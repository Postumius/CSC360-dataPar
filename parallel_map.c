#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

typedef struct map_arg {
  void* (*f)(void*);
  void** inputs;
  void** outputs;
  int from;
  int to;
} map_arg;

void* chunk_map(void* argument) {
  map_arg* arg = (map_arg*) argument;
  for(int i = arg->from; i < arg->to; i++) {
    arg->outputs[i] = (*(arg->f))(arg->inputs[i]);
  }
  return NULL;
}

void* para_map(void* (*f)(void*), void** inputs,
              int length, int nthreads) {

  void** outputs = malloc(sizeof(void*) * length);
  map_arg args[nthreads];
  pthread_t threads[nthreads];
  int chunk_size = (nthreads > 0)? length / nthreads: 0;

  for(int i=0; i<nthreads; i++) {
    map_arg arg = {f, inputs, outputs, 
                   i*chunk_size, (i+1) * chunk_size};
    args[i] = arg;
    pthread_create(&threads[i], NULL, chunk_map, (void*)&args[i]);
  }
  
  map_arg arg = {f, inputs, outputs,
                 chunk_size*nthreads, length};
  chunk_map((void*)&arg);
  
  
  for(int i=0; i<nthreads; i++) {
    pthread_join(threads[i], NULL);
  }
  return outputs;
}

void* doub(void* number) {
  int* n = malloc(sizeof(int));
  *n = *((int*)number) * 2;
  return n;
}
/*
int main() {
  int** arr = malloc(sizeof(int*) * 14);
  for(int i = 0; i<14; i++) {
    int* n = malloc(sizeof(int));
    *n = i;
    arr[i] = n;
  }
  //void* doubled[7];
  //map_arg arg = {(void**)arr, doubled, doub, 0, 7};
  //chunk_map((void*)&arg);
  //printf("%d\n", *(int*)doub((void*)&n));
  void** doubled = para_map(doub, (void**)arr, 14, 2);
  for (int i = 0; i<14; i++)
    printf("%d\n", (*(int*)doubled[i]));
}
*/
