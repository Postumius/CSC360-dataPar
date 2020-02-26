#include "math_functions.h"

double abs_val(double x) {
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

double slope(double x1, double y1, double x2, double y2) {
  return (y2-y1) / (x2-x1);
}

double intercept(double x1, double y1, double m) {
  return y1 - m*x1;
}

