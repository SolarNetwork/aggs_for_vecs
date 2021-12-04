
Datum vec_to_mean_numeric_transfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(vec_to_mean_numeric_transfn);

/**
 * Returns an array of n elements,
 * which each element is the mean value found in that position
 * from all input arrays.
 *
 * by Paul A. Jungwirth
 */
Datum
vec_to_mean_numeric_transfn(PG_FUNCTION_ARGS)
{
  Oid elemTypeId;
  int16 elemTypeWidth;
  bool elemTypeByValue;
  char elemTypeAlignmentCode;
  int currentLength;
  MemoryContext aggContext;
  VecArrayBuildState *state = NULL;
  ArrayType *currentArray;
  int arrayLength;
  Datum *currentVals;
  bool *currentNulls;
  int i;
  MemoryContext old;

  if (!AggCheckCallContext(fcinfo, &aggContext)) {
    elog(ERROR, "vec_to_mean_numeric_transfn called in non-aggregate context");
  }

  // PG_ARGISNULL tests for SQL NULL,
  // but after the first pass we can have a
  // value that is non-SQL-NULL but still is C NULL.
  if (!PG_ARGISNULL(0)) {
    state = (VecArrayBuildState *)PG_GETARG_POINTER(0);
  }

  if (PG_ARGISNULL(1)) {
    // just return the current state unchanged (possibly still NULL)
    PG_RETURN_POINTER(state);
  }
  currentArray = PG_GETARG_ARRAYTYPE_P(1);

  if (state == NULL) {
    // Since we have our first not-null argument
    // we can initialize the state to match its length.
    elemTypeId = ARR_ELEMTYPE(currentArray);
    if (ARR_NDIM(currentArray) != 1) {
      ereport(ERROR, (errmsg("One-dimensional arrays are required")));
    }
    arrayLength = (ARR_DIMS(currentArray))[0];
    // Just start with all NULLs and let the comparisons below replace them:
    state = initVecArrayResultWithNulls(elemTypeId, NUMERICOID, aggContext, arrayLength);

    state->vec_accum_fcinfo = MemoryContextAllocZero(aggContext,  SizeForFunctionCallInfo(2));
    InitFunctionCallInfoData(*state->vec_accum_fcinfo, &numeric_avg_accum_fmgrinfo, 2, fcinfo->fncollation, fcinfo->context, fcinfo->resultinfo);
  } else {
    elemTypeId = state->inputElementType;
    arrayLength = state->state.nelems;
  }

  get_typlenbyvalalign(elemTypeId, &elemTypeWidth, &elemTypeByValue, &elemTypeAlignmentCode);
  deconstruct_array(currentArray, elemTypeId, elemTypeWidth, elemTypeByValue, elemTypeAlignmentCode,
      &currentVals, &currentNulls, &currentLength);
  if (currentLength != arrayLength) {
    ereport(ERROR, (errmsg("All arrays must be the same length, but we got %d vs %d", currentLength, arrayLength)));
  }

  state->vec_accum_fcinfo->args[1].isnull = false;

  old = MemoryContextSwitchTo(aggContext);
  for (i = 0; i < arrayLength; i++) {
    if (currentNulls[i]) {
      // do nothing: nulls can't change the result.
    } else {
      if (state->state.dnulls[i]) {
        state->state.dnulls[i] = false;
        state->vec_accum_fcinfo->args[0].isnull = true;
      } else {
        state->vec_accum_fcinfo->args[0].isnull = false;
      }
      
      state->vec_accum_fcinfo->args[0].value = state->vecvalues[i].datum;
      state->vec_accum_fcinfo->args[1].value = currentVals[i];
      state->vec_accum_fcinfo->isnull = false;
      state->vecvalues[i].datum = FunctionCallInvoke(state->vec_accum_fcinfo);
      if (state->vec_accum_fcinfo->isnull) {
        // accumulator returned no state
        ereport(ERROR, (errmsg("The numeric_avg_accum function returned a NULL aggregate state when invoked with %s",
          DatumGetCString(DirectFunctionCall1(numeric_out, currentVals[i])))));
      }
    }
  }
  MemoryContextSwitchTo(old);
  PG_RETURN_POINTER(state);
}

Datum vec_to_mean_numeric_finalfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(vec_to_mean_numeric_finalfn);

Datum
vec_to_mean_numeric_finalfn(PG_FUNCTION_ARGS)
{
  Datum result;
  VecArrayBuildState *state;
  int dims[1];
  int lbs[1];
  int i;
  Datum div;

  Assert(AggCheckCallContext(fcinfo, NULL));

  state = PG_ARGISNULL(0) ? NULL : (VecArrayBuildState *)PG_GETARG_POINTER(0);

  if (state == NULL)
    PG_RETURN_NULL();

  LOCAL_FCINFO(proxy_fcinfo, 1);
  InitFunctionCallInfoData(*proxy_fcinfo, &numeric_avg_fmgrinfo, 1, fcinfo->fncollation, fcinfo->context, fcinfo->resultinfo);

  // Convert from our pgnums to Datums:
  for (i = 0; i < state->state.nelems; i++) {
    if (state->state.dnulls[i]) continue;
    proxy_fcinfo->args[0].isnull = (state->vecvalues[i].datum == 0);
    proxy_fcinfo->args[0].value = state->vecvalues[i].datum;
    proxy_fcinfo->isnull = false;
    div = FunctionCallInvoke(proxy_fcinfo);
    if (proxy_fcinfo->isnull) {
      // this isn't really expected; should this be an error condition?
      state->state.dnulls[i] = true;
    } else {
      state->state.dvalues[i] = div;
    }
  }

  dims[0] = state->state.nelems;
  lbs[0] = 1;

  result = makeMdArrayResult(&state->state, 1, dims, lbs, CurrentMemoryContext, false);
  PG_RETURN_DATUM(result);
}
