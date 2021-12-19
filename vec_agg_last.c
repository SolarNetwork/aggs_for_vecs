
Datum vec_agg_last_finalfn(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(vec_agg_last_finalfn);

/**
 * Extract an array of lasts from a VecAggAccumState.
 */
Datum
vec_agg_last_finalfn(PG_FUNCTION_ARGS)
{
  ArrayType *result;
  VecAggAccumState *state;
  int16 typlen;
  bool typbyval;
  char typalign;
  Datum *dvalues;
  bool *dnulls;
  int dims[1];
  int lbs[1];
  int i;

  state = PG_ARGISNULL(0) ? NULL : (VecAggAccumState *)PG_GETARG_POINTER(0);
  if (state == NULL || state->nelems < 1) {
    PG_RETURN_NULL();
  }

  dvalues = palloc(state->nelems * sizeof(Datum));
  dnulls = palloc(state->nelems * sizeof(bool));

  for (i = 0; i < state->nelems; i++) {
    if (state->vec_counts[i]) {
      dvalues[i] = state->vec_lasts[i]; // TODO: need copy?
      dnulls[i] = false;
    }
  }

  dims[0] = state->nelems;
  lbs[0] = 1;

  get_typlenbyvalalign(state->elementType, &typlen, &typbyval, &typalign);
  result = construct_md_array(dvalues, dnulls, 1, dims, lbs, state->elementType, typlen, typbyval, typalign);  
  PG_RETURN_ARRAYTYPE_P(result);
}
