#include <stdio.h>
#include <iostream>
#include <time.h>
#include <math.h>
#include "matrix.hpp"
#include "invert.hpp"
#include <sys/time.h>
#include <pthread.h>

using namespace std;

long double get_time();

typedef struct
{
    double *a;
    double *a_inv;
    int n;
    int thread_num;
    int threads_count;
    int *return_flag;
    double *max_elem;
    int *max_ind;
    double norm;
    long double time;
} Args;

void *invert(void *Arg)
{
    Args *arg = (Args*)Arg;
    long double t;

    synchronize(arg->threads_count);
    t = get_time();

    invert(arg->a, arg->a_inv, arg->n, arg->thread_num, arg->threads_count, arg->return_flag, arg->max_elem, arg->max_ind);

    synchronize(arg->threads_count);
    arg->time = get_time() - t;

    return NULL;
}

void *norm1(void *Arg)
{
    Args *arg = (Args*)Arg;
    long double t;

    synchronize(arg->threads_count);
    t = get_time();

    norm1(arg->a, arg->a_inv, arg->n, arg->thread_num, arg->threads_count, arg->norm);

    synchronize(arg->threads_count);
    arg->time = get_time() - t;

    return NULL;
}

int main(int argc, char **argv)
{
    int n, m, k, i;
    double *a;
    double *a_inv;
    double *max_elem;
    int *max_ind;
    int return_flag = 0;
    char filename[120];
    FILE* fin = nullptr;
    long double t, t_norm;
    double norm;
    int threads_count;
    pthread_t *threads;
    Args *args;
    int flag;

    if (argc < 5)
    {
        printf("Data is incorrect. Right format:\n./a.out n m threads k *filename (если k != 0)");
        return -1;
    }

    if (sscanf(argv[1], "%d", &n) != 1 || sscanf(argv[2], "%d", &m) != 1 || sscanf(argv[3], "%d", &threads_count) != 1 || sscanf(argv[4], "%d", &k) != 1)
    {
        printf("Data is incorrect.\n");
        return -1;
    }

    if ((k == 0 && argc != 6) || (k != 0 && argc != 5))
    {
        printf("Data is incorrect.\n");
        return -1;
    }

    if (n < 0 || m < 0 || m > n || k < 0 || k > 4 || threads_count < 1)
    {
        printf("Data is incorrect.\n");
        return -1;
    }

    if (k == 0)
    {
        if(sscanf(argv[5], "%s", filename) != 1)
        {
            printf("Data is incorrect.\n");
            return -1;
        }

        fin = fopen(filename, "r");

        if (!fin)
        {
            printf("File is not exist.\n");
            fclose(fin);
            return -2;
        }
    }

    try
    {
        a = new double [n*n];
        a_inv = new double [n*n];
        max_elem = new double [threads_count];
        max_ind = new int [threads_count];
        args = new Args [threads_count];
        threads = new pthread_t [threads_count];
    }
    catch (bad_alloc&)
    {
        printf("Not enough memory.\n");

        if (k == 0)
            fclose(fin);

        return -2;
    }

    flag = enter_matrix(a, n, k, fin);

    if (flag < 0)
    {
        printf("Matrix is incorrect.\n");

        if (k == 0)
            fclose(fin);

        delete []a;
        delete []a_inv;
        delete []args;
        delete []threads;
        delete []max_elem;
        delete []max_ind;

        return -2;
    }

    printf("\nInitial matrix:\n");
    print_matrix(a, n, m);

    for (i = 0; i < threads_count; ++i)
    {
        args[i].a = a;
        args[i].a_inv = a_inv;
        args[i].n = n;
        args[i].thread_num = i;
        args[i].threads_count = threads_count;
        args[i].return_flag = &return_flag;
        args[i].max_elem = max_elem;
        args[i].max_ind = max_ind;
    }

    for (i = 0; i < threads_count; ++i)
    {
        if (pthread_create(threads+i, 0, invert, args+i))
        {
            printf("Thread is not created.\n");

            if (k == 0)
                fclose(fin);

            delete []a;
            delete []a_inv;
            delete []args;
            delete []threads;
            delete []max_elem;
            delete []max_ind;


            return -1;
        }
    }

    for (i = 0; i < threads_count; ++i)
    {
        if (pthread_join(threads[i], 0))
        {
            printf("Thread did not started\n");

            if (k == 0)
                fclose(fin);

            delete []a;
            delete []a_inv;
            delete []args;
            delete []threads;
            delete []max_elem;
            delete []max_ind;


            return -1;
        }
    }

    if(return_flag)
    {
        printf("Matrix is singular.\n");

        if (k == 0)
            fclose(fin);

        delete []a;
        delete []a_inv;
        delete []args;
        delete []threads;
        delete []max_elem;
        delete []max_ind;

        return -1;
    }

    t = args[0].time;

    for (i = 1; i < threads_count; ++i)
    {
        if (t < args[i].time)
            t = args[i].time;
    }

    printf("\nInvert matrix:\n");
    print_matrix(a_inv, n, m);

    if (k == 0)
        fseek(fin, 0, SEEK_SET);

    flag = enter_matrix(a, n, k, fin);

    for (i = 0; i < threads_count; ++i)
    {
        if (pthread_create(threads+i, 0, norm1, args+i))
        {
            printf("Thread is not created.\n");

            if (k == 0)
                fclose(fin);

            delete []a;
            delete []a_inv;
            delete []args;
            delete []threads;
            delete []max_elem;
            delete []max_ind;

            return -1;
        }
    }

    for (i = 0; i < threads_count; ++i)
    {
        if (pthread_join(threads[i], 0))
        {
            printf("Thread did not started\n");

            if (k == 0)
                fclose(fin);

            delete []a;
            delete []a_inv;
            delete []args;
            delete []threads;
            delete []max_elem;
            delete []max_ind;

            return -1;
        }
    }

    norm = args[0].norm;
    for (i = 1; i < threads_count; ++i)
    {
        if (norm < args[i].norm)
            norm = args[i].norm;
    }

    t_norm = args[0].time;
    for (i = 1; i < threads_count; ++i)
    {
        if (t_norm < args[i].time)
            t_norm = args[i].time;
    }

    printf("\nDimension = %d, Number of threads = %d\n", n, threads_count);
    printf("Error norm: %10.3e\n", norm);
    printf("Time of invert: %Lf s.\n", t);
    printf("Time of norm: %Lf s.\n", t_norm);

    if (k == 0)
        fclose(fin);

    delete []a;
    delete []a_inv;
    delete []args;
    delete []threads;
    delete []max_elem;
    delete []max_ind;

    return 0;
}

long double get_time()
{
    struct timeval t;
    gettimeofday(&t, 0);
    return t.tv_sec + t.tv_usec/1000000.0;
}
