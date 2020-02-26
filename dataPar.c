#include <stdio.h>
#include <string.h>
#include "math_functions.h"
#include "map.h"   //all the parallelism happens
#include "fold1.h" //in "map.c" and "fold1.c"
#include "util.h"


float** read_data_points(int nlines, char* filename) {
  FILE* fl = fopen(filename, "r");
  if(!fl) {
    fprintf(stderr, "couldn't find data file %s\n", filename);
    exit(-1);
  }  
  float** points = malloc(sizeof(float*) * nlines);
  
  //skip the first line
  char* line = NULL;
  size_t bufsize = 0;
  getline(&line, &bufsize, fl);
  
  for(int i=0; i<nlines; i++) {   
    ssize_t line_size = getline(&line, &bufsize, fl);
    if(line_size < 0) {
      fprintf(stderr, "getline error\n");
      exit(-1);
    }    
    char* token = strtok(line, ",\n");
    token = strtok(NULL, ",\n");
    points[i] = malloc(sizeof(float) * 2);
    points[i][0] = (float)i;
    points[i][1] = strtod(token, NULL);
  }
  free(line);
  fclose(fl);
  return points;
}

//a1 is y intercept, a2 is slope
typedef struct SAR_arg {
  float a1;
  float a2;
  float** points;
  size_t npoints;
  float sum;
} SAR_arg;

void* SAR(void* argument) {
  SAR_arg* arg = (SAR_arg*) argument;
  arg->sum = 0.0;
  for(int i = 0; i<arg->npoints; i++) {
    float t = arg->points[i][0];
    float d = arg->points[i][1];
    arg->sum += abs_val(d - (arg->a1 + arg->a2*t));
  }
  return arg;
}

void* min_SAR(void* sar1, void* sar2) {
  float n1 = ((SAR_arg*)sar1)->sum;
  float n2 = ((SAR_arg*)sar2)->sum;
  return (n1<n2)? sar1: sar2;
}


//given a set of points, this function finds the L1
//with 1, 4, and 8 threads, and prints results
void L1_test(float** points, size_t npoints) {
  printf("computing L1 of %ld points\n", npoints);

  //math combination function to find the number of pairs
  int nlines = choose(npoints, 2);

  //build up array of lines
  SAR_arg** lines = malloc(sizeof(SAR_arg*) * nlines);
  int k = 0;
  for(int i=0; i<npoints-1; i++) {
    for(int j=i+1; j<npoints; j++) {
      float m =
        slope(points[i][0], points[i][1],
              points[j][0], points[j][1]);
      float b = intercept(points[i][0], points[i][1], m);
      lines[k] = malloc(sizeof(SAR_arg));
      lines[k]->a1 = b;
      lines[k]->a2 = m;
      lines[k]->points = points;
      lines[k]->npoints = npoints;
      k++;      
    }
  }

  //nthreads represents the number of extra threads spawned
  int nthreads_per_test[3] = {0,3,7};

  for(int i=0; i<3; i++) {
    printf("with %d threads\n", nthreads_per_test[i]+1);
  
    size_t t0 = time_ms();
  
    SAR_arg** SAR_arr =     //using map to compute SAR of each line.
      (SAR_arg**)para_map   //this is the O(n^3) operation
      (SAR, (void**)lines, nlines, nthreads_per_test[i]);    
    size_t t1 = time_ms();
    printf("Map: %ld ms\n", t1-t0);

    
    SAR_arg* L1 =          //using fold1 to reduce the array of lines with
      (SAR_arg*)para_fold1 //the min_SAR function, this is only O(n^2)
      (min_SAR, (void**)SAR_arr, nlines, nthreads_per_test[i]);    
    size_t t3 = time_ms();
    printf("fold1: %ld ms\n", t3-t1);  
    printf("L1-> intercept:%f,  slope:%f,  SAR:%f\n\n",
           L1->a1, L1->a2, L1->sum);
    
    free(SAR_arr);    
  }
  
  printf("------------------------\n\n");
  
  for(int i=0; i<nlines; i++) {
    free(lines[i]);
  }
  free(lines); 
}


int main() {
  float** cpi = read_data_points(21, "points.txt");
  float** stremflow = read_data_points(3652, "stremflow_time_series.csv");

  L1_test(cpi, 6);
  L1_test(cpi, 10);
  L1_test(cpi, 14);
  L1_test(cpi, 18);
  L1_test(stremflow, 365);
  L1_test(stremflow, 3652);

  for(int i=0; i<21; i++) {
    free(cpi[i]);
  }
  free(cpi);

  for(int i=0; i<3652; i++) {
    free(stremflow[i]);
  }
  free(stremflow);
  return 0;
}
