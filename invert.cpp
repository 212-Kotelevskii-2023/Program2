#include "invert.hpp"
#include "matrix.hpp"
#include <math.h>
#include <algorithm>

using namespace std;

void invert(double *a, double *a_inv, int n, int thread_num, int threads_count, int *return_flag, double *max_elem, int *max_ind)
{
    int i, j, k;
    int maxind;
    double value, maxelem;
    int first_row, last_row;

    double eps = 1e-30;

    first_row = n * thread_num;
	first_row = first_row/threads_count;
	last_row = n * (thread_num + 1);
	last_row = last_row/threads_count;

    for (i = first_row; i < last_row; ++i)
    {
        for (j = 0; j < n; ++j)
        {
            a_inv[j+i*n] = 0;
        }
        a_inv[i+i*n] = 1;
    }

    //основный цикл
    for (i = 0; i < n; ++i)
    {
        synchronize(threads_count);

        first_row = (n - i) * thread_num;
		first_row = first_row/threads_count + i;
		last_row = (n - i) * (thread_num + 1);
		last_row = last_row/threads_count + i;

        max_elem[thread_num] = a[first_row * n + i];
        max_ind[thread_num] = first_row;

        for (j = first_row + 1; j < last_row; ++j)
        {
            if (fabs(a[j*n+i]) > fabs(max_elem[thread_num]))
            {
                max_elem[thread_num] = a[j*n+i];
                max_ind[thread_num] = j;
            }
        }

        synchronize(threads_count);

        if (thread_num == 0)
        {
            maxelem = max_elem[0];
            maxind = max_ind[0];

            for (k = 1; k < threads_count; ++k)
            {
                if (fabs(maxelem) < fabs(max_elem[k]))
                {
                    maxelem = max_elem[k];
                    maxind = max_ind[k];
                }
            }

            if (fabs(maxelem) <= eps)
            {
                *return_flag = 1;
            }

            max_ind[0] = maxind;
        }

        synchronize(threads_count);

        if (*return_flag == 1)
            return;

        maxind = max_ind[0];

        if (maxind != i)
        {
            first_row = n * thread_num;
            first_row = first_row/threads_count;
            last_row = n * (thread_num + 1);
            last_row = last_row/threads_count;

            // пофиксили гонку
            max_ind[thread_num] = maxind;

            for (j = first_row; j < last_row; ++j)
            {
                swap(a[max_ind[thread_num]*n+j], a[i*n+j]);
                swap(a_inv[max_ind[thread_num]*n+j], a_inv[i*n+j]);
            }
        }

        synchronize(threads_count);

        first_row = n * thread_num;
        first_row = first_row/threads_count;
        last_row = n * (thread_num + 1);
        last_row = last_row/threads_count;

        max_elem[thread_num] = 1.0 / a[i*n+i];

        synchronize(threads_count);

        for (j = first_row; j < last_row; ++j)
        {
            a[i*n+j] *= max_elem[thread_num];
            a_inv[i*n+j] *= max_elem[thread_num];
        }

        synchronize(threads_count);

        first_row = (n - i - 1) * thread_num;
		first_row = first_row/threads_count + i + 1;
		last_row = (n - i - 1) * (thread_num + 1);
		last_row = last_row/threads_count + i + 1;

        for (j = first_row; j < last_row; ++j)
        {
            value = a[j*n+i];

            for (k = 0; k < n; ++k)
            {
                a[j*n+k] -= a[i*n+k] * value;
                a_inv[j*n+k] -= a_inv[i*n+k] * value;
            }
        }
    }

    synchronize(threads_count);

    first_row = n * thread_num;
	first_row = first_row/threads_count;
	last_row = n * (thread_num + 1);
	last_row = last_row/threads_count;

    // обратный ход
    for (k = first_row; k < last_row; ++k) // выбор столбца
    {
        for (i = n-1; i >= 0; --i) // выбор строки
        {
            value = a_inv[i*n+k]; // зафиксировали элемент обратной

            for (j = i+1; j < n; ++j) // бежим
                value -= a[i*n+j] * a_inv[j*n+k];

            a_inv[i*n+k] = value;
        }
    }
}

void synchronize(int threads_count)
{
    static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    static pthread_cond_t condvar_in = PTHREAD_COND_INITIALIZER;
    static pthread_cond_t condvar_out = PTHREAD_COND_INITIALIZER;
    static int threads_in = 0;
    static int threads_out = 0;

    pthread_mutex_lock(&mutex);

    threads_in++;
    if (threads_in >= threads_count)
    {
        threads_out = 0;
        pthread_cond_broadcast(&condvar_in);
    } else
        while (threads_in < threads_count)
            pthread_cond_wait(&condvar_in,&mutex);

    threads_out++;
    if (threads_out >= threads_count)
    {
        threads_in = 0;
        pthread_cond_broadcast(&condvar_out);
    } else
        while (threads_out < threads_count)
            pthread_cond_wait(&condvar_out,&mutex);

    pthread_mutex_unlock(&mutex);
}
