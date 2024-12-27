#include "matrix.hpp"
#include <stdio.h>
#include <math.h>

double f(int k, int n, int i, int j)
{
    switch (k)
    {
        case 1:
            return n-fmax(i, j)+1;
        case 2:
            return fmax(i, j);
        case 3:
            return fabs(i-j);
        case 4:
            return 1.0/(i+j+1);
        default:
            return -1;
    }
}

int enter_matrix(double* a, int n, int k, FILE* fin)
{
    int i, j;
    
    if (fin)
    {
        for (i = 0; i < n; ++i)
        {
            for (j = 0; j < n; ++j)
            {
                if (fscanf(fin, "%lf", &a[i*n+j]) != 1)
                    return -1;
            }
        }
    }
    else
    {
        for (i = 0; i < n; ++i)
        {
            for (j = 0; j < n; ++j)
            {
                a[i*n+j] = f(k, n, i, j);
            }
        }
    }
    
    return 0;
}

void print_matrix(double* a, int n, int m)
{
    int i, j;
    
    for (i = 0; i < m; ++i)
    {
        for (j = 0; j < m; ++j)
        {
            printf("%10.3e ", a[i*n+j]);
        }
        printf("\n");
    }
}

void norm1(double* a, double* a_inv, int n, int thread_num, int threads_count, double &max_norm)
{
    int i, j, k;
    double temp, sum = 0.0;
    int first_row, last_row;

    max_norm = 0.0;

    first_row = n * thread_num;
    first_row = first_row/threads_count;
    last_row = n * (thread_num + 1);
    last_row = last_row/threads_count;

    for (i = first_row; i < last_row; ++i)
    {
        sum = 0.0;

        for (j = 0; j < n; ++j)
        {
            temp = 0.0;

            for (k = 0; k < n; ++k)
                temp += a[i*n+k] * a_inv[k*n+j];

            if (i == j)
                temp -= 1.0;

            sum += fabs(temp);
        }

        if (sum > max_norm)
            max_norm = sum;
    }
}
