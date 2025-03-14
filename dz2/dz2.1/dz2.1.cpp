#include <iostream>
#include <omp.h>

int SIZE, THREADS = 1;

void init_arrays(double* a, double* b, int n, int m) {
    #pragma omp parallel
    {
        int nthreads = omp_get_num_threads();
        int threadid = omp_get_thread_num();
        int items_per_thread = n / nthreads;
        int lb = threadid * items_per_thread;
        int ub = (threadid == nthreads - 1) ? (n - 1) : (lb + items_per_thread - 1);

        for (int i = lb; i <= ub; i++) {
            for (int j = 0; j < m; j++)
                a[i * m + j] = i + j;  // Используем m для индексации
            b[i] = i;
        }
    }
}

void run_serial(const double a[], const double b[], int n, int m) {
    auto* res = new double[n]();
    double start = omp_get_wtime();

    for (int i = 0; i < n; i++) {
        res[i] = 0.0;
        for (int j = 0; j < m; j++) {
            res[i] += a[i * m + j] * b[j];  // Используем m для индексации
        }
    }

    double end = omp_get_wtime();
    double time = end - start;

    std::cout << time << ", ";
    delete[] res;
}

void run_parallel(const double a[], const double b[], int n, int m) {
    auto* res = new double[n]();
    double start = omp_get_wtime();

    #pragma omp parallel num_threads(THREADS)
    {
        int nthreads = omp_get_num_threads();
        int threadid = omp_get_thread_num();
        int items_per_thread = n / nthreads;
        int lb = threadid * items_per_thread;
        int ub = (threadid == nthreads - 1) ? (n - 1) : (lb + items_per_thread - 1);

        for (int i = lb; i <= ub; i++) {
            res[i] = 0.0;
            for (int j = 0; j < m; j++) {
                res[i] += a[i * m + j] * b[j];  // Используем m для индексации
            }
        }
    }

    double end = omp_get_wtime();
    double time = end - start;

    std::cout << time << ", ";
    delete[] res;
}

int main() {
    int arr1[] = {20000, 40000};
    int arr2[] = {1, 2, 4, 7, 8, 16, 20, 40};

    for (int i = 0; i < 2; i++) {
        for (int j = 1; j < 8; j++) {  // Исправил количество итераций
            SIZE = arr1[i];
            THREADS = arr2[j];

            auto* a = new double[SIZE * SIZE];
            auto* b = new double[SIZE];

            init_arrays(a, b, SIZE, SIZE);

            if (THREADS == 1) 
                run_serial(a, b, SIZE, SIZE);
            else 
                run_parallel(a, b, SIZE, SIZE);

            delete[] a;
            delete[] b;
        }
        std::cout << "\n";
    }
    return 0;
}
