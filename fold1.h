void* chunk_fold1(void* argument);

void* para_fold1(void* (*f)(void*, void*), void** inputs,
               int length, int nthreads);
