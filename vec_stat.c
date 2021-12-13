typedef struct VecAggAccumState {
  ArrayBuildState *astate;
  uint32          *vec_counts;     // Element non-null count.
  Datum           *vec_states;     // Element aggregate state.
  pgnum           *vec_mins;       // Element min value seen.
  pgnum           *vec_maxes;      // Element max value seen.
} VecAggAccumState;

VecAggAccumState *
initVecAggAccumStateWithNulls(Oid element_type, MemoryContext rcontext, int alen);

VecAggAccumState *
initVecAggAccumStateWithNulls(Oid element_type, MemoryContext rcontext, int alen) {
  VecAggAccumState *state;

  state = (VecAggAccumState *)MemoryContextAlloc(rcontext, sizeof(VecAggAccumState));
  state->astate = initArrayResultWithNulls(element_type, rcontext, alen);
  state->vec_counts = (uint32 *)MemoryContextAllocZero(rcontext, alen * sizeof(uint32)); // set counts to 0 with AllocZero
  state->vec_states = (Datum *)MemoryContextAlloc(rcontext, alen * sizeof(Datum));
  state->vec_mins = (pgnum *)MemoryContextAlloc(rcontext, alen * sizeof(pgnum));
  state->vec_maxes = (pgnum *)MemoryContextAlloc(rcontext, alen * sizeof(pgnum));
  
  return state;
}
