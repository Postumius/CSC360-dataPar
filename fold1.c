#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
//#include "fold1.h"
#include "util.h"

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

void split_evenly(size_t* chunk_sizes, size_t n, unsigned nthreads) {
  size_t quot = n / nthreads;
  size_t rem = n % nthreads;

  for(int i=0; i<nthreads; i++) {
    if (rem > 0) {
      chunk_sizes[i] = quot + 1;
      rem--;
    } else {
      chunk_sizes[i] = quot;
    }
  }
}

void* para_fold1(void* (*f)(void*, void*), void** inputs,
               size_t length, unsigned nthreads) {

  //for the single-threaded test
  if(nthreads == 0) {
    fold_arg arg = {f, inputs, NULL, 0, length};
    chunk_fold1((void*)&arg);
    return arg.acc;

      
  } else {    
    pthread_t threads[nthreads];
    size_t chunk_sizes[nthreads];
    split_evenly(chunk_sizes, length, nthreads);
    /*
      after each thread has folded up its section, the result will go into
      the array "intermeds".  */
    fold_arg args[nthreads];
    void* intermeds[nthreads];
  
    //spawn new threads to fold
    size_t j = 0;
    for(int i=0; i<nthreads; i++) {
      fold_arg arg = {f, inputs, NULL,
                      j, j + chunk_sizes[i]};
      args[i] = arg;
      pthread_create(&threads[i], NULL, chunk_fold1, (void*)&args[i]);
      j += chunk_sizes[i];
    }
  
    //join the threads back together
    for(int i=0; i<nthreads; i++) {
      pthread_join(threads[i], NULL);
    }  

    //put the results of the intermediate folds into the array, then fold that up  
    for(int i=0; i<nthreads; i++) {
      intermeds[i] = args[i].acc;    
    }  
  
    fold_arg final_arg = {f, intermeds, NULL, 0, nthreads};
    chunk_fold1((void*)&final_arg);  

    //note that we never mallocated final_arg.acc, since it just points
    //to an element in the input.
    return final_arg.acc;
  }
  
}

void* pointless(void* acc, void* num) {
  int n = *(unsigned*)num;
  for(unsigned i=0; i<n; i++) {
    NULL;
  }
  return num;
}

void test(unsigned** nums, size_t length, unsigned nthreads) {
 
  size_t t0 = time_ms();
  unsigned ignored = *(unsigned*)para_fold1(pointless, (void**)nums, length, nthreads);
  size_t t1 = time_ms();
  printf("%ld ms with %d threads\n", t1-t0, nthreads);
}

int main() {
  unsigned big = 100000;
  unsigned** rising = malloc(sizeof(int*) * big);
  unsigned** flat = malloc(sizeof(int*) * big); 
  
  for(unsigned i=0; i<big; i++) {
    rising[i] = malloc(sizeof(int));
    *rising[i] = i;

    flat[i] = malloc(sizeof(int));
    *flat[i] = big/2;
  }
  printf("made arrays\ncommencing O(n^2) operation on %d elements\n\n", big);
  printf("first with workload distributed unevenly over threads:\n");
  test(rising, big, 0);
  test(rising, big, 4);
  test(rising, big, 8);
  test(rising, big, 12);

  printf("\nnow with workload distributed evenly over threads:\n");
  test(flat, big, 0);
  test(flat, big, 4);
  test(flat, big, 8);
  test(flat, big, 12);
}
  
