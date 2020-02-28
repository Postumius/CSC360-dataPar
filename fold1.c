#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include "fold1.h"
#include "util.h"
/*
This is loosely based on the map implementation from here:
http:www.datastuff.tech/programming/how-to-code-mapreduce-in-c-from-scratch-using-threads-pt-1-map/

Fold1 is like fold (or reduce) but instead of taking an accumulator
as a parameter it uses the first element in the array as the accumulator.
this is more appropriate for finding the minimum or maximum of a list

Contrary to what the above article states, the operation only needs to be
associative, not commutative
*/

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

//calculates the most even split of n over nthreads, stores answer in chunk_sizes
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

  //for the single-threaded case
  if(nthreads == 0) {
    fold_arg arg = {f, inputs, NULL, 0, length};
    chunk_fold1((void*)&arg);
    return arg.acc;

      
  } else {    
    pthread_t threads[nthreads];
    size_t chunk_sizes[nthreads];
    split_evenly(chunk_sizes, length, nthreads);
    /* after each thread has folded up its chunk, the result will go into
       the array "intermeds". */
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

    /*Note that we never mallocated final_arg.acc, since it just points
      to an element in the input. This unfortunately means that fold1
      could have side-effects on the input, depending on f. */
    return final_arg.acc;
  }
  
}

/*everything past here isn't actually needed for dataPar, it's just
  additional testing for the performance evaluation.

  The gist is that we have this "pointless" function that takes num
  and loops num times. We fold with it over one array with elements
  rising from 1 to 10^5, and another array with (10^5)/2 in every spot.
  test_fold1 will get called from main in dataPar.c*/

void* pointless(void* acc, void* num) {
  int n = *(unsigned*)num;
  for(unsigned i=0; i<n; i++) {
    NULL;
  }
  return num;
}

void test(unsigned** nums, size_t length, unsigned nthreads) {
 
  size_t t0 = time_ms();
  //the if is just so I don't get a warning for unused value
  if(*(unsigned*)para_fold1(pointless, (void**)nums, length, nthreads));
  size_t t1 = time_ms();
  printf("%ld ms with %d threads\n", t1-t0, nthreads);
}

void test_fold1() {
  unsigned big = 100000;
  unsigned** rising = malloc(sizeof(int*) * big);
  unsigned** flat = malloc(sizeof(int*) * big); 
  
  for(unsigned i=0; i<big; i++) {
    rising[i] = malloc(sizeof(int));
    *rising[i] = i;

    flat[i] = malloc(sizeof(int));
    *flat[i] = big/2;
  }
  printf("extra test!\ncommencing O(n^2) operation on %d elements\n\n", big);
  printf("first with workload distributed unevenly over threads:\n");
  for(int i=1; i<17; i++) {
    test(rising, big, i);
  }

  printf("\nnow with workload distributed evenly over threads:\n");
  for(int i=1; i<17; i++) {
    test(flat, big, i);
  }

  for(unsigned i=0; i<big; i++) {
    free(flat[i]);
    free(rising[i]);
  }
  free(flat);
  free(rising);
}
  
