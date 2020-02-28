void* chunk_fold1(void* argument);

void* para_fold1(void* (*f)(void*, void*), void** inputs,
                 size_t length, unsigned nthreads);

void test_fold1();
