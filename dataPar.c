#include <stdio.h>
#include <string.h>
#include "math_functions.c"
#include "parallel_map.c"
#include "parallel_fold1.c"
#include "util.c"

#define M 3651

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
} SAR_arg;

void* SAR(void* argument) {
  SAR_arg* arg = (SAR_arg*) argument;
  double* sum = (double*)malloc(sizeof(double));
  *sum = 0;
  for(int i = 0; i<M; i++) {
    double t = arg->points[i][0];
    double d = arg->points[i][1];
    *sum += abs_val(d - (arg->a1 + arg->a2*t));
  }
  return sum;
}
  
/*
void* SAR(void* pair) {
  double* sum = (double*)malloc(sizeof(double));
  *sum = 0;
  double a1 = *((double**)pair)[0];
  double a2 = *((double**)pair)[1];
  for(int i = 0; i<M; i++) {
    double t = points[i][0];
    double d = points[i][1];
    *sum += abs_val(d - (a1 + a2*t));
  }
  return sum;
}
*/

int main() {
  double** points = read_data_points(M, "stremflow_time_series.csv");
  
  int size = choose(M,2);
  printf("M choose 2: %d\n", size);

 
  SAR_arg** A = malloc(sizeof(SAR_arg*) * size);
  int k = 0;
  for(int i=0; i<M-1; i++) {
    for(int j=i+1; j<M; j++) {
      double m =
        slope(points[i][0], points[i][1],
              points[j][0], points[j][1]);
      double b = intercept(points[i][0], points[i][1], m);
      /*
      A[k] = malloc(sizeof(double*) *2);
      A[k][0] = malloc(sizeof(double));
      A[k][1] = malloc(sizeof(double));
      *A[k][0] = b;
      *A[k][1] = m;
      */
      A[k] = malloc(sizeof(SAR_arg));
      A[k]->a1 = b;
      A[k]->a2 = m;
      //A[k]->points = malloc(sizeof(double*));
      A[k]->points = points;            
      //printf("%f %f\n", A[k]->a1, A[k]->a2);
      k++;      
    }
  }
  printf("built A\n");

  size_t t0 = time_ms();
  double** SAR_arr = (double**)para_map(SAR, (void**)A, size, 31);
  size_t t1 = time_ms();
  printf("Map: %ld ms\n", t1-t0);
  
  double* minSAR = (double*)para_fold1(min, (void**)SAR_arr, size, 31);
  size_t t3 = time_ms();
  printf("fold: %ld ms\n", t3-t1);
  
  printf("minimum SAR: %f\n", *minSAR);
  return 0;
}
