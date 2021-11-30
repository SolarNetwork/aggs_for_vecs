
Datum vec_add(PG_FUNCTION_ARGS);
PG_FUNCTION_INFO_V1(vec_add);

/**
 * Add elements of two arrays.
 *
 * This function takes two arrays of n-elements and returns an array of
 * n-elements where each element is the sum of the input elements
 * of the same index.
 */
Datum
vec_add(PG_FUNCTION_ARGS)
{
  Oid elemTypeId;
  int16 elemTypeWidth;
  bool elemTypeByValue;
  char elemTypeAlignmentCode;
  int valsLength;
  ArrayType *leftArray, *rightArray, *retArray;
  Datum *leftContent, *rightContent, *retContent;
  bool *leftNulls, *rightNulls, *retNulls;
  int i;
  int dims[1];
  int lbs[1];

  if (PG_ARGISNULL(0) || PG_ARGISNULL(1)) {
    PG_RETURN_NULL();
  }

  leftArray = PG_GETARG_ARRAYTYPE_P(0);
  rightArray = PG_GETARG_ARRAYTYPE_P(1);

  if (ARR_NDIM(leftArray) == 0 || ARR_NDIM(rightArray) == 0) {
    PG_RETURN_NULL();
  }
  if (ARR_NDIM(leftArray) > 1 || ARR_NDIM(rightArray) > 1) {
    ereport(ERROR, (errmsg("vec_add: one-dimensional arrays are required")));
  }

  elemTypeId = ARR_ELEMTYPE(leftArray);

  if (elemTypeId != INT2OID &&
      elemTypeId != INT4OID &&
      elemTypeId != INT8OID &&
      elemTypeId != FLOAT4OID &&
      elemTypeId != FLOAT8OID &&
      elemTypeId != NUMERICOID) {
    ereport(ERROR, (errmsg("vec_add input must be array of SMALLINT, INTEGER, BIGINT, REAL, DOUBLE PRECISION, or NUMERIC")));
  }
  if (rightArray && elemTypeId != ARR_ELEMTYPE(rightArray)) {
    ereport(ERROR, (errmsg("vec_add mins array must be the same type as input array")));
  }

  valsLength = (ARR_DIMS(leftArray))[0];
  if (rightArray && valsLength != (ARR_DIMS(rightArray))[0]) {
    ereport(ERROR, (errmsg("vec_add right array must be the same length as left array")));
  }

  get_typlenbyvalalign(elemTypeId, &elemTypeWidth, &elemTypeByValue, &elemTypeAlignmentCode);

  deconstruct_array(leftArray, elemTypeId, elemTypeWidth, elemTypeByValue, elemTypeAlignmentCode,
      &leftContent, &leftNulls, &valsLength);
  if (rightArray) {
    deconstruct_array(rightArray, elemTypeId, elemTypeWidth, elemTypeByValue, elemTypeAlignmentCode,
        &rightContent, &rightNulls, &valsLength);
  }

  retContent = palloc0(sizeof(Datum) * valsLength);
  retNulls = palloc0(sizeof(bool) * valsLength);

  for (i = 0; i < valsLength; i++) {
    if (leftNulls[i] || rightNulls[i]) {
      retNulls[i] = true;
      continue;
    }
    retNulls[i] = false;
    switch(elemTypeId) {
      case INT2OID:
        retContent[i] = Int16GetDatum(DatumGetInt16(leftContent[i]) + DatumGetInt16(rightContent[i]));
        break;
      case INT4OID:
        retContent[i] = Int32GetDatum(DatumGetInt32(leftContent[i]) + DatumGetInt32(rightContent[i]));
        break;
      case INT8OID:
        retContent[i] = Int64GetDatum(DatumGetInt64(leftContent[i]) + DatumGetInt64(rightContent[i]));
        break;
      case FLOAT4OID:
        retContent[i] = Float4GetDatum(DatumGetFloat4(leftContent[i]) + DatumGetFloat4(rightContent[i]));
        break;
      case FLOAT8OID:
        retContent[i] = Float8GetDatum(DatumGetFloat8(leftContent[i]) + DatumGetFloat8(rightContent[i]));
        break;
      case NUMERICOID:
#if PG_VERSION_NUM < 120000
        retContent[i] = DirectFunctionCall2(numeric_add, leftContent[i], rightContent[i]);
#else
        retContent[i] = NumericGetDatum(numeric_add_opt_error(DatumGetNumeric(leftContent[i]), DatumGetNumeric(rightContent[i]), NULL));
#endif
        break;
    }
  }

  dims[0] = valsLength;
  lbs[0] = 1;

  retArray = construct_md_array(retContent, retNulls, 1, dims, lbs,
      elemTypeId, elemTypeWidth, elemTypeByValue, elemTypeAlignmentCode);

  PG_RETURN_ARRAYTYPE_P(retArray);
}

