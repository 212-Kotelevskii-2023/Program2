#ifndef matrix_hpp
#define matrix_hpp

#include <stdio.h>

double f(int k, int n, int i, int j);
int enter_matrix(double* a, int n, int k, FILE* fin);
void print_matrix(double* a, int n, int m);
void norm1(double* a, double* a_inv, int n, int thread_num, int threads_count, double &max_norm);

#endif /* matrix_hpp */
