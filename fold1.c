#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include "fold1.h"

//this is based on the map implementation from here:
//http://www.datastuff.tech/programming/how-to-code-mapreduce-in-c-from-scratch-using-threads-pt-1-map/

//fold1 is like fold (or reduce) but instead of taking an accumulator
//as a parameter it uses the first element in the array as the accumulator
//this is more appropriate for finding the minimum or maximum of a list

//contrary to what the above article states, the operation only needs to be
//associative, not commutative

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

  //make sure there aren't more threads than elements in the input
  if (nthreads > length) nthreads = length;
  
  pthread_t threads[nthreads];

  //prevent division by zero, so we can run it single-threaded
  int chunk_size = (nthreads > 0)? length / nthreads: 0;
  /*
  after each thread has folded up its section, the result will go into
  the array "intermeds". If there is no remainder for the main thread
  to fold, intermeds will need to be 1 shorter
  needs to be 1 shorter */
  int nintermeds = (length - nthreads*chunk_size != 0)? nthreads+1: nthreads;
  fold_arg args[nintermeds];
  void* intermeds[nintermeds];
  
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

  //put the results of the intermediate folds into the array, then fold that up  
  for(int i=0; i<nintermeds; i++) {
    intermeds[i] = args[i].acc;    
  }
  fold_arg final_arg = {f, intermeds, NULL, 0, nintermeds};
  chunk_fold1((void*)&final_arg);  

  //note that we never mallocated final_arg.acc, since it just points
  //to an element in the input.
  return final_arg.acc;
}
