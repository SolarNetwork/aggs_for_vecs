Datum vec_agg_sum_finalfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(vec_agg_sum_finalfn);

/**
 * Extract an array of means from a VecAggAccumState.
 */
Datum
vec_agg_sum_finalfn(PG_FUNCTION_ARGS)
{
  Datum result;
  MemoryContext aggContext;
  VecAggAccumState *state;
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

  for (i = 0; i < state->astate->alen; i++) {
    if (state->vec_counts[i]) {
      switch (state->astate->element_type) {
        // TODO: support other number types
        case NUMERICOID:
          state->astate->dvalues[i] = DirectFunctionCall1(numeric_sum, state->vec_states[i]);
          break;

        default:
          elog(ERROR, "Unknown array element type");
      }
      state->astate->dnulls[i] = false;
    }
  }

  dims[0] = state->astate->alen;
  lbs[0] = 1;

  result = makeMdArrayResult(state->astate, 1, dims, lbs, CurrentMemoryContext, false);
  PG_RETURN_DATUM(result);
}
