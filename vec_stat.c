typedef struct VecAggAccumState {
  Oid              elementType;    // input element type
  int              nelems;         // number of elements
  uint32          *vec_counts;     // Element non-null count.
  Datum           *vec_states;     // Element aggregate state.
  Datum           *vec_mins;       // Element min value seen.
  Datum           *vec_maxes;      // Element max value seen.
  Datum           *vec_firsts;     // Element first value seen.
  Datum           *vec_lasts;      // Element last value seen.
  FunctionCallInfo transfn_fcinfo; // Cached function call for invoking aggregate function.
  FunctionCallInfo cmp_fcinfo;     // Cached function call for invoking comparison function.
} VecAggAccumState;

VecAggAccumState *
initVecAggAccumState(Oid element_type, MemoryContext rcontext, int nelems);

VecAggAccumState *
initVecAggAccumState(Oid element_type, MemoryContext rcontext, int nelems) {
  VecAggAccumState *astate;

  astate = (VecAggAccumState *)MemoryContextAlloc(rcontext, sizeof(VecAggAccumState));
  astate->nelems = nelems;
  astate->elementType = element_type;
  astate->vec_counts = (uint32 *)MemoryContextAllocZero(rcontext, nelems * sizeof(uint32)); // set counts to 0 with AllocZero
  astate->vec_states = (Datum *)MemoryContextAlloc(rcontext, nelems * sizeof(Datum));
  astate->vec_mins = (Datum *)MemoryContextAlloc(rcontext, nelems * sizeof(Datum));
  astate->vec_maxes = (Datum *)MemoryContextAlloc(rcontext, nelems * sizeof(Datum));
  astate->vec_firsts = (Datum *)MemoryContextAlloc(rcontext, nelems * sizeof(Datum));
  astate->vec_lasts = (Datum *)MemoryContextAlloc(rcontext, nelems * sizeof(Datum));
  
  return astate;
}
