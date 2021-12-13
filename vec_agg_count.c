Datum vec_agg_count_finalfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(vec_agg_count_finalfn);

/**
 * Extract an array of counts from a VecAggAccumState.
 */
Datum
vec_agg_count_finalfn(PG_FUNCTION_ARGS)
{
  Datum result;
  MemoryContext aggContext;
  VecAggAccumState *state;
  ArrayBuildState *astate;
  int dims[1];
  int lbs[1];
  int i;

  if (!AggCheckCallContext(fcinfo, &aggContext)) {
    elog(ERROR, "Function called in non-aggregate context");
  }

  state = PG_ARGISNULL(0) ? NULL : (VecAggAccumState *)PG_GETARG_POINTER(0);
  if (state == NULL || state->astate->alen < 1) {
    PG_RETURN_NULL();
  }

  if (state->astate->element_type == INT8OID) {
    astate = state->astate;
  } else {
    astate = initArrayResultWithNulls(INT8OID, aggContext, state->astate->alen);
  }

  for (i = 0; i < astate->alen; i++) {
    astate->dvalues[i] = Int64GetDatum((int64)state->vec_counts[i]);
    astate->dnulls[i] = false;
  }

  dims[0] = astate->alen;
  lbs[0] = 1;

  result = makeMdArrayResult(astate, 1, dims, lbs, CurrentMemoryContext, false);
  PG_RETURN_DATUM(result);
}
