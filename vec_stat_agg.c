
Datum vec_stat_agg_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(vec_stat_agg_transfn);

/**
 * Aggregate transition function for accumulating basic statistics via
 * delegate aggregate transition function and comparison functions.
 */
Datum
vec_stat_agg_transfn(PG_FUNCTION_ARGS)
{
  Oid elemTypeId;
  int16 elemTypeWidth;
  bool elemTypeByValue;
  char elemTypeAlignmentCode;
  int currentLength;
  MemoryContext aggContext;
  VecAggAccumState *state;
  ArrayType *currentArray;
  int arrayLength;
  Datum *currentVals;
  bool *currentNulls;
  int i;
  MemoryContext oldContext;
  FunctionCallInfo transfn_fcinfo;

  if (!AggCheckCallContext(fcinfo, &aggContext)) {
    elog(ERROR, "vec_stat_agg called in non-aggregate context");
  }

  state = PG_ARGISNULL(0) ? NULL : (VecAggAccumState *)PG_GETARG_POINTER(0);

  if (PG_ARGISNULL(1)) {
    // just return the current state unchanged (possibly still NULL)
    PG_RETURN_POINTER(state);
  }
  currentArray = PG_GETARG_ARRAYTYPE_P(1);

  // lazy-init this later if we need (only need on first call to delegate aggregtate state function, to pass NULL as first arg)
  transfn_fcinfo = NULL;
  
  if (state == NULL) {
    elemTypeId = ARR_ELEMTYPE(currentArray);
    if (ARR_NDIM(currentArray) != 1) {
      ereport(ERROR, (errmsg("One-dimensional arrays are required")));
    }
    arrayLength = (ARR_DIMS(currentArray))[0];
    state = initVecAggAccumStateWithNulls(elemTypeId, aggContext, arrayLength);
  } else {
    elemTypeId = state->elementType;
    arrayLength = state->nelems;
  }

  get_typlenbyvalalign(elemTypeId, &elemTypeWidth, &elemTypeByValue, &elemTypeAlignmentCode);
  deconstruct_array(currentArray, elemTypeId, elemTypeWidth, elemTypeByValue, elemTypeAlignmentCode,
      &currentVals, &currentNulls, &currentLength);
  if (currentLength != arrayLength) {
    ereport(ERROR, (errmsg("All arrays must be the same length, but we got %d vs %d", currentLength, arrayLength)));
  }

  // for each input element, delegate to 
  for (i = 0; i < arrayLength; i++) {
    if (currentNulls[i]) {
      // do nothing: nulls can't change the result.
    } else {
      if (state->vec_counts[i] < 1) {
        // first call to delegate aggregate transition, so must init transfn_fcinfo if not already
        if (!transfn_fcinfo) {
          transfn_fcinfo = MemoryContextAlloc(aggContext,  SizeForFunctionCallInfo(2));
         
          // we know the null-ness of the transfn_fcinfo arguments up front, as only call on first non-null element value
          transfn_fcinfo->args[0].isnull = true;
          transfn_fcinfo->args[1].isnull = false;
          
          switch(elemTypeId) {
            // TODO: support other number types
            case NUMERICOID:
              // the numeric_avg_accum supports numeric_avg and numeric_sum final functions
              InitFunctionCallInfoData(*transfn_fcinfo, &numeric_avg_accum_fmgrinfo, 2, fcinfo->fncollation, fcinfo->context, fcinfo->resultinfo);
              break;
            default:
              elog(ERROR, "Unknown array element type");
          }
        }

        // first non-null element set up as initial min/max values
        oldContext = MemoryContextSwitchTo(aggContext); {
          state->vec_mins[i] = datumCopy(currentVals[i], elemTypeByValue, elemTypeWidth);
          state->vec_maxes[i] = datumCopy(currentVals[i], elemTypeByValue, elemTypeWidth);
        } MemoryContextSwitchTo(oldContext);
      } else {
        // execute delegate comparison function for min/max
        switch(elemTypeId) {
          // TODO: support other number types
          case NUMERICOID:
            if (DatumGetInt32(DirectFunctionCall2(numeric_cmp, state->vec_mins[i], currentVals[i])) > 0) {
              oldContext = MemoryContextSwitchTo(aggContext); {
                state->vec_mins[i] = datumCopy(currentVals[i], elemTypeByValue, elemTypeWidth);
              } MemoryContextSwitchTo(oldContext);
            }
            if (DatumGetInt32(DirectFunctionCall2(numeric_cmp, state->vec_maxes[i], currentVals[i])) < 0) {
              oldContext = MemoryContextSwitchTo(aggContext); {
                state->vec_maxes[i] = datumCopy(currentVals[i], elemTypeByValue, elemTypeWidth);
              } MemoryContextSwitchTo(oldContext);
            }
            break;
          default:
            elog(ERROR, "Unknown array element type");
        }
      }
      
      // increment non-null count
      state->vec_counts[i]++;

      // execute delegate transition function
      if (transfn_fcinfo) {
        // first argument passed is null, so can't use DirectFunctionCall2 here
        transfn_fcinfo->args[1].value = currentVals[i];
        transfn_fcinfo->isnull = false;
        state->vec_states[i] = FunctionCallInvoke(transfn_fcinfo);
        if (transfn_fcinfo->isnull) {
          // delegate function returned no state
          ereport(ERROR, (errmsg("The delegate transition function returned a NULL aggregate state on element %d", i)));
        }
      } else {
        switch(state->elementType) {
          // TODO: support other number types
          case NUMERICOID:
            state->vec_states[i] = DirectFunctionCall2(numeric_avg_accum, state->vec_states[i], currentVals[i]);
            break;
          default:
            elog(ERROR, "Unknown array element type");
        }
      }
    }
  }
  PG_RETURN_POINTER(state);
}

Datum vec_stat_agg_finalfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(vec_stat_agg_finalfn);

// Return VecAggStatsType
Datum
vec_stat_agg_finalfn(PG_FUNCTION_ARGS)
{
  VecAggAccumState *state;
  VecAggStatsType *stats;
  int i;
  int16 elementTypeLen;
  bool elementTypeByVal;
  char elementTypeAlign;

  Assert(AggCheckCallContext(fcinfo, NULL));

  state = PG_ARGISNULL(0) ? NULL : (VecAggAccumState *)PG_GETARG_POINTER(0);

  if (state == NULL) {
    PG_RETURN_NULL();
  }

  get_typlenbyvalalign(state->elementType, &elementTypeLen, &elementTypeByVal, &elementTypeAlign);

  stats = initVecAggStatsType(state->elementType, CurrentMemoryContext, state->nelems);

  for (i = 0; i < state->nelems; i++) {
    stats->counts[i] = state->vec_counts[i];
    stats->mins[i] = (state->vec_counts[i] ? datumCopy(state->vec_mins[i], elementTypeByVal, elementTypeLen) : 0);
    stats->maxes[i] = (state->vec_counts[i] ? datumCopy(state->vec_maxes[i], elementTypeByVal, elementTypeLen) : 0);
    switch(state->elementType) {
      // TODO: support other number types
      case NUMERICOID:
        stats->sums[i] = (state->vec_counts[i] ? DirectFunctionCall1(numeric_sum, state->vec_states[i]) : 0);
        stats->means[i] = (state->vec_counts[i] ? DirectFunctionCall1(numeric_avg, state->vec_states[i]) : 0);
        break;
      default:
        elog(ERROR, "Unknown array element type");
    }
  }

  // FIXME: now that we've populated Datum elements, how do we adjust the stats varlena?
  //SET_VARSIZE(stats, newStatsSize);

  PG_RETURN_DATUM(VecAggStatsTypePGetDatum(stats));
}
