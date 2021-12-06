// Aggregate element statistics to use in two-step aggregation functions.
typedef struct VecAggElementStats {
  char		vl_len_[4];
  Oid     elementType;
  uint32  count;
  Datum   sum;
  Datum   min;
  Datum   max;
} VecAggElementStats;

/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/

PG_FUNCTION_INFO_V1(vecaggelementstats_in);

Datum
vecaggelementstats_in(PG_FUNCTION_ARGS)
{
	char               *str = PG_GETARG_CSTRING(0);
  int32               varlen;
	VecAggElementStats *result;

  varlen = 4;

  // TODO: parse text form, e.g. (len,oid,count,d_sum,d_sumX2,d_min,d_max);
  //       all the d_* are themselves varlena structures to support Numeric
	if (false) {
		ereport(ERROR, (errcode(ERRCODE_INVALID_TEXT_REPRESENTATION), 
      errmsg("invalid input syntax for type %s: \"%s\"", "vecaggelementstats", str)));
  }

	result = (VecAggElementStats *) palloc(sizeof(varlen));
	// TODO: populate result
	PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(vecaggelementstats_out);

Datum
vecaggelementstats_out(PG_FUNCTION_ARGS)
{
	VecAggElementStats *stats = (VecAggElementStats *) PG_GETARG_POINTER(0);
	char	             *result;

	// TODO: format to string
	PG_RETURN_CSTRING(result);
}
