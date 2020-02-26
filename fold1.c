#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include "fold1.h"

typedef struct fold_arg {
  void* (*f)(void*, void*);
  void** inputs;
  void* acc;
  int from;
  int to;
} fold_arg;

void* chunk_fold1(void* argument) {
  fold_arg* arg = (fold_arg*) argument;
  
  if(arg->from == arg->to) {
    fprintf(stderr, "chunk_fold1 received an empty array\n");
    exit(-1);
  }
  arg->acc = arg->inputs[arg->from];
  for(int i = arg->from+1; i < arg->to; i++) {
    arg->acc = (*(arg->f))(arg->acc, arg->inputs[i]);
  }
  return NULL;
}

void* para_fold1(void* (*f)(void*, void*), void** inputs,
               int length, int nthreads) {

  if (nthreads > length) nthreads = length;
  pthread_t threads[nthreads];
  int chunk_size = (nthreads > 0)? length / nthreads: 0;
  int nintermeds = (length - nthreads*chunk_size != 0)? nthreads+1: nthreads;
  fold_arg args[nintermeds];
  
  //spawn new threads to fold
  for(int i=0; i<nthreads; i++) {
    fold_arg arg = {f, inputs, NULL,
                    i*chunk_size, (i+1) * chunk_size};
    args[i] = arg;
    pthread_create(&threads[i], NULL, chunk_fold1, (void*)&args[i]);
  }
  //if there's a remainder, fold it in the main thread
  if(nintermeds > nthreads) {
    fold_arg arg = {f, inputs, NULL,
                    chunk_size*nthreads, length};
    args[nthreads] = arg;
    chunk_fold1((void*)&args[nthreads]);
  }

  //join the threads back together
  for(int i=0; i<nthreads; i++) {
    pthread_join(threads[i], NULL);
  }

  //put the results of the intermediate folds in an array, then fold that up
  void* intermeds[nintermeds];
  for(int i=0; i<nintermeds; i++) {
    intermeds[i] = args[i].acc;    
  }
  fold_arg final_arg = {f, intermeds, NULL, 0, nintermeds};
  final_arg.acc = malloc(sizeof(void));
  chunk_fold1((void*)&final_arg);  
  
  return final_arg.acc;
}

void* plus(void* num1, void* num2) {
  *(int*)num1 += *(int*)num2;
  return num1;
}

void* min(void* num1, void* num2) {
  double n1 = *(double*)num1;
  double n2 = *(double*)num2;
  return (n1<n2)? num1: num2;
  /*
  double* smaller = malloc(sizeof(double));
  *smaller = (n1 < n2)? n1: n2;
  return smaller;
  */
}
/*
int main() {
  
  int** ns = malloc(sizeof(int*) * 15);
  for(int i = 0; i < 15; i++) {
    int* n = malloc(sizeof(int));
    *n = i;
    ns[i] = n;
  }
  
  
  //fold_arg arg = {plus, (void**)ns, NULL, 0, 15};
  //chunk_fold1((void*)&arg);
  //printf("%d\n", *(int*)arg.acc);
  
  
  
  printf("%d\n",
         *(int*)para_fold1
         (plus, (void**)ns, 15, 2));
         
  return 0;
  
}
*/
