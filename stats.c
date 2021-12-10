// Aggregate state for use in two-step aggregate functions.
typedef struct VecAggAccumState {
  Oid              elementType;
  int              nelems;         // number of elements
  Datum           *vec_states;     // Element aggregate state.
  Datum           *vec_mins;       // Element min value seen.
  Datum           *vec_maxes;      // Element max value seen.
  uint32          *vec_counts;     // Element non-null count.
} VecAggAccumState;

// Aggregate element statistics to use in two-step aggregation functions.
typedef struct VecAggStatsType {
  char   vl_len_[4];
  Oid    elemTypeId; // type of Datum used below
  int    nelems;     // number of elements
  int64 *counts;
  Datum *sums;
  Datum *mins;
  Datum *maxes;
  Datum *means;
} VecAggStatsType;

/*
 * fmgr macros for vecaggstats type objects
 */
#define DatumGetVecAggStatsTypeP(X)             ((VecAggStatsType *) PG_DETOAST_DATUM(X))
#define DatumGetVecAggStatsTypePCopy(X)         ((VecAggStatsType *) PG_DETOAST_DATUM_COPY(X))
#define VecAggStatsTypePGetDatum(X)             PointerGetDatum(X)
#define PG_GETARG_VECAGGSTATS_P(n)              DatumGetVecAggStatsTypeP(PG_GETARG_DATUM(n))
#define PG_GETARG_VECAGGSTATS_P_COPY(n)         DatumGetVecAggStatsTypePCopy(PG_GETARG_DATUM(n))
#define PG_RETURN_VECAGGSTATS_P(x)              return VecAggStatsTypePGetDatum(x)

VecAggAccumState *
initVecAggAccumStateWithNulls(Oid element_type, MemoryContext rcontext, int arLen);

VecAggAccumState *
initVecAggAccumStateWithNulls(Oid element_type, MemoryContext rcontext, int arLen) {
  VecAggAccumState *astate;

  astate = (VecAggAccumState *)MemoryContextAlloc(rcontext, sizeof(VecAggAccumState));
  astate->nelems = arLen;
  astate->elementType = element_type;
  astate->vec_states = (Datum *)MemoryContextAlloc(rcontext, arLen * sizeof(Datum));
  astate->vec_mins = (Datum *)MemoryContextAlloc(rcontext, arLen * sizeof(Datum));
  astate->vec_maxes = (Datum *)MemoryContextAlloc(rcontext, arLen * sizeof(Datum));
  astate->vec_counts = (uint32 *)MemoryContextAllocZero(rcontext, arLen * sizeof(uint32)); // set counts to 0
  
  return astate;
}

VecAggStatsType *
initVecAggStatsType(Oid element_type, MemoryContext rcontext, int nelems);

VecAggStatsType *
initVecAggStatsType(Oid element_type, MemoryContext rcontext, int nelems) {
  VecAggStatsType *stats;

  stats = (VecAggStatsType *)MemoryContextAlloc(rcontext, sizeof(VecAggStatsType));

  stats->elemTypeId = element_type;
  stats->nelems = nelems;
  stats->counts = (nelems < 1 ? 0 : MemoryContextAllocZero(rcontext, nelems * sizeof(int64))); // set counts to 0
  stats->sums = (nelems < 1 ? 0 : MemoryContextAlloc(rcontext, nelems * sizeof(Datum)));
  stats->mins = (nelems < 1 ? 0 : MemoryContextAlloc(rcontext, nelems * sizeof(Datum)));
  stats->maxes = (nelems < 1 ? 0 : MemoryContextAlloc(rcontext, nelems * sizeof(Datum)));
  stats->means = (nelems < 1 ? 0 : MemoryContextAlloc(rcontext, nelems * sizeof(Datum)));

  SET_VARSIZE(stats, sizeof(VecAggStatsType) 
                  + (nelems * sizeof(int64)) 
                  + (nelems * 4 * sizeof(Datum)));

  return stats;
}

static Size
vecaggstats_size(Oid elemTypeId, Datum *datums, int32 datumCount) {
  Size size;
  int16 elemTypeWidth;
  bool elemTypeByValue;
  char elemTypeAlignmentCode;
  int  i;

  get_typlenbyvalalign(elemTypeId, &elemTypeWidth, &elemTypeByValue, &elemTypeAlignmentCode);
  
  size = att_align_nominal(sizeof(VecAggStatsType), elemTypeAlignmentCode);
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
	char            *str = PG_GETARG_CSTRING(0);
  int32            varlen;
	VecAggStatsType *result;
  Size		         size;

  varlen = 4;

  // TODO: parse text form, e.g. (oid,count,sum,min,max,mean);
  //       all the d_* are themselves varlena structures to support Numeric
	if (false) {
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
      errmsg("invalid input syntax for type %s: \"%s\"", "vecaggelementstats", str)));
  }

	result = (VecAggStatsType *) palloc0(sizeof(VecAggStatsType));
	// TODO: populate result

	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(vecaggstats_out);

Datum
vecaggstats_out(PG_FUNCTION_ARGS)
{
	VecAggStatsType *stats = (VecAggStatsType *) PG_GETARG_POINTER(0);
	char	          *result;

	// TODO: format to actual string
  result = palloc(5);
  strcpy(result, "TODO");

	PG_RETURN_CSTRING(result);
}
