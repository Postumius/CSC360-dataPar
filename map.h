void* chunk_map(void* argument);

void* para_map(void* (*f)(void*), void** inputs,
              int length, int nthreads);

