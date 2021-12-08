
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
  Datum compareResult;

  if (!AggCheckCallContext(fcinfo, &aggContext)) {
    elog(ERROR, "vec_stat_agg called in non-aggregate context");
  }

  state = PG_ARGISNULL(0) ? NULL : (VecAggAccumState *)PG_GETARG_POINTER(0);

  if (PG_ARGISNULL(1)) {
    // just return the current state unchanged (possibly still NULL)
    PG_RETURN_POINTER(state);
  }
  currentArray = PG_GETARG_ARRAYTYPE_P(1);

  if (state == NULL) {
    elemTypeId = ARR_ELEMTYPE(currentArray);
    if (ARR_NDIM(currentArray) != 1) {
      ereport(ERROR, (errmsg("One-dimensional arrays are required")));
    }
    arrayLength = (ARR_DIMS(currentArray))[0];
    state = initVecAggAccumStateWithNulls(elemTypeId, aggContext, arrayLength);

    // Set up the delegate aggregate transition/compare function calls
    state->transfn_fcinfo = MemoryContextAllocZero(aggContext,  SizeForFunctionCallInfo(2));
    state->cmp_fcinfo = MemoryContextAllocZero(aggContext,  SizeForFunctionCallInfo(2));
    switch(elemTypeId) {
      // TODO: support other number types
      case NUMERICOID:
        // TODO: the numeric_avg_accum supports numeric_avg and numeric_sum final functions
        InitFunctionCallInfoData(*state->transfn_fcinfo, &numeric_avg_accum_fmgrinfo, 2, fcinfo->fncollation, fcinfo->context, fcinfo->resultinfo);
        InitFunctionCallInfoData(*state->cmp_fcinfo, &numeric_cmp_fmgrinfo, 2, InvalidOid, NULL, NULL);
        break;
      default:
        elog(ERROR, "Unknown array element type");
    }
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

  // we can set isnull to false up front because we won't call the delgate function when an element is NULL
  state->transfn_fcinfo->args[1].isnull = false;
  state->cmp_fcinfo->args[0].isnull = false;
  state->cmp_fcinfo->args[1].isnull = false;

  // for each input element, delegate to 
  for (i = 0; i < arrayLength; i++) {
    if (currentNulls[i]) {
      // do nothing: nulls can't change the result.
    } else {
      if (state->vec_counts[i] < 1) {
        // first non-null element; we can use this as initial min/max values
        state->vec_mins[i] = currentVals[i];
        state->vec_maxes[i] = currentVals[i];
        state->transfn_fcinfo->args[0].isnull = true;
      } else {
        state->transfn_fcinfo->args[0].isnull = false;

        // execute delegate comparison function for min
        state->cmp_fcinfo->args[0].value = state->vec_mins[i];
        state->cmp_fcinfo->args[1].value = currentVals[i];
        state->cmp_fcinfo->isnull = false;
        compareResult = FunctionCallInvoke(state->cmp_fcinfo);
        if (state->cmp_fcinfo->isnull) {
          // delegate function returned no result
          ereport(ERROR, (errmsg("The delegate comparison function returned a NULL result on element %d", i)));
        } else if (DatumGetInt32(compareResult) > 0) {
          state->vec_mins[i] = currentVals[i];
        }

        // execute delegate comparison function for max
        state->cmp_fcinfo->args[0].value = state->vec_maxes[i];
        state->cmp_fcinfo->args[1].value = currentVals[i];
        state->cmp_fcinfo->isnull = false;
        compareResult = FunctionCallInvoke(state->cmp_fcinfo);
        if (state->cmp_fcinfo->isnull) {
          // delegate function returned no result
          ereport(ERROR, (errmsg("The delegate comparison function returned a NULL result on element %d", i)));
        } else if (DatumGetInt32(compareResult) < 0) {
          state->vec_maxes[i] = currentVals[i];
        }
      }
      
      // increment non-null count
      state->vec_counts[i]++;

      // execute delegate transition function
      state->transfn_fcinfo->args[0].value = state->vec_states[i];
      state->transfn_fcinfo->args[1].value = currentVals[i];
      state->transfn_fcinfo->isnull = false;
      state->vec_states[i] = FunctionCallInvoke(state->transfn_fcinfo);
      if (state->transfn_fcinfo->isnull) {
        // delegate function returned no state
        ereport(ERROR, (errmsg("The delegate transition function returned a NULL aggregate state on element %d", i)));
      }
    }
  }
  PG_RETURN_POINTER(state);
}

Datum vec_stat_agg_finalfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(vec_stat_agg_finalfn);

// Return array of VecAggElementStats elements
Datum
vec_stat_agg_finalfn(PG_FUNCTION_ARGS)
{
  VecAggAccumState *state;
  Datum result;
  ArrayBuildState *result_build;
  Oid statsOid;
  VecAggElementStatsType *stats;
  int dims[1];
  int lbs[1];
  int i;
  int16 elementTypeLen;
  bool elementTypeByVal;
  char elementTypeAlign;


  Assert(AggCheckCallContext(fcinfo, NULL));

  state = PG_ARGISNULL(0) ? NULL : (VecAggAccumState *)PG_GETARG_POINTER(0);

  if (state == NULL)
    PG_RETURN_NULL();

  statsOid = typenameTypeId(NULL, typeStringToTypeName("vecaggstats"));
  result_build = initArrayResultWithNulls(statsOid, CurrentMemoryContext, state->nelems);

  get_typlenbyvalalign(state->elementType, &elementTypeLen, &elementTypeByVal, &elementTypeAlign);

  for (i = 0; i < state->nelems; i++) {
    if (state->nelems < 1) continue;
    stats = palloc(sizeof(VecAggElementStatsType));
    stats->elemTypeId = state->elementType;
    stats->count = state->vec_counts[i];
    stats->min = datumCopy(state->vec_mins[i], elementTypeByVal, elementTypeLen);
    stats->max = datumCopy(state->vec_maxes[i], elementTypeByVal, elementTypeLen);
    stats->sum = 0; // TODO: extract sum out of stats->vec_states[i], i.e. call numeric_sum
    result_build->dvalues[i] = VecAggElementStatsTypePGetDatum(stats);
    result_build->dnulls[i] = false;
  }

  dims[0] = state->nelems;
  lbs[0] = 1;
  result = makeMdArrayResult(result_build, 1, dims, lbs, CurrentMemoryContext, false);
  PG_RETURN_DATUM(result);
}
