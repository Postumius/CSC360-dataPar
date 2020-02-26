#include "math_functions.h"

float abs_val(float x) {
  return (x < 0.0 ? -1*x: x);
}


int fac(int n, int r) {
  int prod = 1;
  for(int i = n; i>r; i--) {
    prod *= i;
    }
  return prod;
}

int choose(int n, int r) {
  return fac(n, (n-r)) / fac(r,1);
}

float slope(float x1, float y1, float x2, float y2) {
  return (y2-y1) / (x2-x1);
}

float intercept(float x1, float y1, float m) {
  return y1 - m*x1;
}

