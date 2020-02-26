#include <stdio.h>
#include <string.h>
#include "math_functions.h"
#include "map.h"
#include "fold1.h"
#include "util.h"


double** read_data_points(int nlines, char* filename) {
  FILE* fl = fopen(filename, "r");
  if(!fl) {
    fprintf(stderr, "couldn't find data file %s\n", filename);
    exit(-1);
  }  
  double** nums = malloc(sizeof(double*) * nlines);
  
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
    nums[i] = malloc(sizeof(double) * 2);
    nums[i][0] = (double)i;
    nums[i][1] = strtod(token, NULL);    
  }
  return nums;
}


/*
double points[7][2] =
  {{1.0, 87.6},
   {2.0, 88.9},
   {3.0, 90.4},
   {4.0, 91.3},
   {5.0, 92.9},
   {6.0, 95.4},
   {7.0, 97.8}};
*/

typedef struct SAR_arg {
  double a1;
  double a2;
  double** points;
  size_t npoints;
} SAR_arg;

void* SAR(void* argument) {
  SAR_arg* arg = (SAR_arg*) argument;
  double* sum = (double*)malloc(sizeof(double));
  *sum = 0;
  for(int i = 0; i<arg->npoints; i++) {
    double t = arg->points[i][0];
    double d = arg->points[i][1];
    *sum += abs_val(d - (arg->a1 + arg->a2*t));
  }
  return sum;
}
  
void SAR_test(double** points, size_t npoints) {
  printf("computing min SAR on %ld points\n", npoints);
  int nlines = choose(npoints, 2);

 
  SAR_arg** lines = malloc(sizeof(SAR_arg*) * nlines);
  int k = 0;
  for(int i=0; i<npoints-1; i++) {
    for(int j=i+1; j<npoints; j++) {
      double m =
        slope(points[i][0], points[i][1],
              points[j][0], points[j][1]);
      double b = intercept(points[i][0], points[i][1], m);
      lines[k] = malloc(sizeof(SAR_arg));
      lines[k]->a1 = b;
      lines[k]->a2 = m;
      lines[k]->points = points;
      lines[k]->npoints = npoints;
      k++;      
    }
  }
  
  int nthreads_per_test[3] = {0,3,7};

  for(int i=0; i<3; i++) {
    printf("with %d threads\n", nthreads_per_test[i]+1);
  
    size_t t0 = time_ms();
  
    double** SAR_arr =
      (double**)para_map
      (SAR, (void**)lines, nlines, nthreads_per_test[i]);    
    size_t t1 = time_ms();
    printf("Map: %ld ms\n", t1-t0);

    /*for(int j=0; j<nlines; j++) {
      printf("%f\n", *SAR_arr[j]);
    }*/
    double* min_SAR =
      (double*)para_fold1
      (min, (void**)SAR_arr, nlines, nthreads_per_test[i]);    
    size_t t3 = time_ms();
    printf("fold1: %ld ms\n", t3-t1);  
    printf("minimum SAR: %f\n\n", *min_SAR);    
  }
  /*
  free(min_SAR);
  for(int j=0; j<nlines; j++) {
    free(SAR_arr[j]);
  }
  free(SAR_arr);
  */
  printf("------------------------\n\n");
  
  for(int i=0; i<nlines; i++) {
    free(lines[i]);
  }
  free(lines); 
}



int main() {
  double** points1 = read_data_points(21, "points.txt");
  //double** points2 = read_data_points(3651, "stremflow_time_series.csv");

  SAR_test(points1, 6);
  SAR_test(points1, 10);
  SAR_test(points1, 14);
  SAR_test(points1, 18);
  //SAR_test(points2, 100);
  
  return 0;
}
