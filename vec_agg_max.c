
Datum vec_agg_max(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(vec_agg_max);

/**
 * Extract an array of maxes from a VecAggStatsType.
 */
Datum
vec_agg_max(PG_FUNCTION_ARGS)
{
  Datum result;
  ArrayBuildState *result_build;
  VecAggStatsType *stats;
  int dims[1];
  int lbs[1];
  int i;

  stats = PG_ARGISNULL(0) ? NULL : PG_GETARG_VECAGGSTATS_P(0);
  if (stats == NULL || stats->nelems < 1) {
    PG_RETURN_NULL();
  }

  result_build = initArrayResultWithNulls(stats->elemTypeId, CurrentMemoryContext, stats->nelems);

  for (i = 0; i < stats->nelems; i++) {
    if (stats->sums[i]) {
      result_build->dvalues[i] = stats->maxes[i]; // TODO: need copy?
      result_build->dnulls = false;
    }
  }

  dims[0] = result_build->nelems;
  lbs[0] = 1;

  result = makeMdArrayResult(result_build, 1, dims, lbs, CurrentMemoryContext, false);
  PG_RETURN_DATUM(result);
}
