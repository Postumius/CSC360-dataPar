#include <stdio.h>
#include <stdlib.h>

#define M 4

double points[7][2] =
  {{1.0, 87.6},
   {2.0, 88.9},
   {3.0, 90.4},
   {4.0, 91.3},
   {5.0, 92.9},
   {6.0, 95.4},
   {7.0, 97.8}};

double abs_val(double x) {
  return (x < 0.0 ? -1*x: x);
}

double SAR(double a1, double a2) {
  double sum = 0.0;
  for(int i = 0; i<M; i++) {
    double t = points[i][0];
    double d = points[i][1];
    sum += abs_val(d - (a1 + a2*t));
  }
  return sum;
}

int fac(int n) {
  int prod = 1;
  for(int i = 1; i<=n; i++) {
    prod *= i;
    }
  return prod;
}

int choose(int n, int r) {
  return fac(n) / (fac(r)*fac(n-r));
}

double slope(double x1, double y1, double x2, double y2) {
  return (y2-y1) / (x2-x1);
}

double intercept(double x1, double y1, double m) {
  return y1 - m*x1;
}

int main() {
  int size = choose(M,2);
  double A[size][2];
  int k = 0;
  for(int i=0; i<M-1; i++) {
    for(int j=i+1; j<M; j++) {
      double m =
        slope(points[i][0], points[i][1],
              points[j][0], points[j][1]);
      double b = intercept(points[i][0], points[i][1], m);
      A[k][0] = b;
      A[k][1] = m;
      printf("%f %f %f\n", b, m, SAR(b, m));
      k++;
    }
  }  
  return 0;
}
