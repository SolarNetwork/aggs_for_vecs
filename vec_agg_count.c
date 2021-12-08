
Datum vec_agg_count(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(vec_agg_count);

/**
 * Extract an array of counts from a VecAggAccumState.
 */
Datum
vec_agg_count(PG_FUNCTION_ARGS)
{
  Datum result;
  ArrayType *stats; // an array of VecAggElementStats elements
  ArrayBuildState *result_build;
  Datum *statsContent;
  bool *statsNulls;
  int statsLength;
  Oid elemTypeId;
  int16 elemTypeWidth;
  bool elemTypeByValue;
  char elemTypeAlignmentCode;
  VecAggElementStatsType *elemStats;
  int dims[1];
  int lbs[1];
  int i;

  stats = PG_ARGISNULL(0) ? NULL : PG_GETARG_ARRAYTYPE_P(0);
  if (stats == NULL || ARR_NDIM(stats) == 0) {
    PG_RETURN_NULL();
  }

  elemTypeId = ARR_ELEMTYPE(stats);

  get_typlenbyvalalign(elemTypeId, &elemTypeWidth, &elemTypeByValue, &elemTypeAlignmentCode);

  deconstruct_array(stats, elemTypeId, elemTypeWidth, elemTypeByValue, elemTypeAlignmentCode,
      &statsContent, &statsNulls, &statsLength);

  result_build = initArrayResultWithNulls(INT8OID, CurrentMemoryContext, statsLength);

  for (i = 0; i < statsLength; i++) {
    if (statsNulls[i]) continue;
    elemStats = DatumGetVecAggElementStatsTypeP(statsContent[i]);
    result_build->dvalues[i] = Int64GetDatum(elemStats->count);
    result_build->dnulls = false;
  }

  dims[0] = result_build->nelems;
  lbs[0] = 1;

  result = makeMdArrayResult(result_build, 1, dims, lbs, CurrentMemoryContext, false);
  PG_RETURN_DATUM(result);
}
