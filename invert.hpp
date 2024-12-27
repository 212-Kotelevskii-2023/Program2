#ifndef invert_hpp
#define invert_hpp

#include <stdio.h>
#include <pthread.h>

void invert(double *a, double *a_inv, int n, int thread_num, int threads_count, int *return_flag, double *max_elem, int *max_ind);
void synchronize(int total_threads);

#endif /* invert_hpp */
