// Aggregate element statistics to use in two-step aggregation functions.
typedef struct VecAggElementStatsType {
  char    vl_len_[4];
  Oid     elemTypeId;
  uint32  count;
  Datum   sum;
  Datum   min;
  Datum   max;
  Datum   mean;
} VecAggElementStatsType;

/*
 * fmgr macros for vecaggstats type objects
 */
#define DatumGetVecAggElementStatsTypeP(X)      ((VecAggElementStatsType *) PG_DETOAST_DATUM(X))
#define DatumGetVecAggElementStatsTypePCopy(X)  ((VecAggElementStatsType *) PG_DETOAST_DATUM_COPY(X))
#define VecAggElementStatsTypePGetDatum(X)      PointerGetDatum(X)
#define PG_GETARG_VECAGGSTATS_P(n)              DatumGetVecAggElementStatsTypeP(PG_GETARG_DATUM(n))
#define PG_GETARG_VECAGGSTATS_P_COPY(n)         DatumGetVecAggElementStatsTypePCopy(PG_GETARG_DATUM(n))
#define PG_RETURN_VECAGGSTATS_P(x)              return VecAggElementStatsTypePGetDatum(x)


static Size
vecaggstats_size(Oid elemTypeId, Datum *datums, int32 datumCount) {
  Size size;
  int16 elemTypeWidth;
  bool elemTypeByValue;
  char elemTypeAlignmentCode;
  int  i;

  get_typlenbyvalalign(elemTypeId, &elemTypeWidth, &elemTypeByValue, &elemTypeAlignmentCode);
  
  size = att_align_nominal(sizeof(VecAggElementStatsType), elemTypeAlignmentCode);
  for (i = 0; i < datumCount; i++) {
    size += att_align_nominal(VARSIZE(datums[i]), elemTypeAlignmentCode);
  }

  return size;
}

/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(vecaggstats_in);

Datum
vecaggstats_in(PG_FUNCTION_ARGS)
{
	char                   *str = PG_GETARG_CSTRING(0);
  int32                   varlen;
	VecAggElementStatsType *result;
  Size		                size;

  varlen = 4;

  // TODO: parse text form, e.g. (oid,count,sum,min,max,mean);
  //       all the d_* are themselves varlena structures to support Numeric
	if (false) {
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
      errmsg("invalid input syntax for type %s: \"%s\"", "vecaggelementstats", str)));
  }

	result = (VecAggElementStatsType *) palloc0(sizeof(VecAggElementStatsType));
	// TODO: populate result

	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(vecaggstats_out);

Datum
vecaggstats_out(PG_FUNCTION_ARGS)
{
	VecAggElementStatsType *stats = (VecAggElementStatsType *) PG_GETARG_POINTER(0);
	char	             *result;

	// TODO: format to actual string
  result = palloc(6);
  strcpy(result, "hello");

	PG_RETURN_CSTRING(result);
}
