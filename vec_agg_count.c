
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
  int dims[1];
  int lbs[1];
  int i;

  stats = PG_ARGISNULL(0) ? NULL : PG_GETARG_ARRAYTYPE_P(0);

  if (stats == NULL)
    PG_RETURN_NULL();

  elemTypeId = ARR_ELEMTYPE(stats);

  get_typlenbyvalalign(elemTypeId, &elemTypeWidth, &elemTypeByValue, &elemTypeAlignmentCode);

  deconstruct_array(stats, elemTypeId, elemTypeWidth, elemTypeByValue, elemTypeAlignmentCode,
      &statsContent, &statsNulls, &statsLength);

  result_build = initArrayResultWithNulls(elemTypeId, CurrentMemoryContext, statsLength);

  for (i = 0; i < statsLength; i++) {
    VecAggElementStats *stats = NULL; // FIXME get from stats[i]
    result_build->dvalues[i] = Int64GetDatum(stats->count);
  }

  dims[0] = result_build->nelems;
  lbs[0] = 1;

  result = makeMdArrayResult(result_build, 1, dims, lbs, CurrentMemoryContext, false);
  PG_RETURN_DATUM(result);
}
