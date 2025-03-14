#include <iostream>
#include <chrono>
#include <omp.h>
#include <cmath>

int nsteps = 40000000;
int THREADS = 1;
double a = -4.0, b = 4.0;

double func(double x)
{
    return cos(x);
}

double integrate_serial()
{
    double sum = 0.0;
    double h = (b - a) / nsteps;

    double start = omp_get_wtime();

    for (int i = 0; i < nsteps; i++)
        sum += func(a + (h * (i + 0.5)));

    double end = omp_get_wtime();
    double time = end - start;

    return time;
}

double integrate_parallel()
{
    double sum = 0.0;
    double h = (b - a) / nsteps;

    double start = omp_get_wtime();

    #pragma omp parallel num_threads(THREADS)
    {
        int nthreads = omp_get_num_threads();
        int threadid = omp_get_thread_num();
        int items_per_thread = nsteps / nthreads;
        int lb = threadid * items_per_thread;
        int ub = (threadid == nthreads - 1) ? nsteps : (lb + items_per_thread);
        double local_sum = 0.0;

        for (int i = lb; i < ub; i++)
            local_sum += func(a + (h * (i + 0.5))); // исправлено смещение

        #pragma omp atomic
        sum += local_sum;
    }

    double end = omp_get_wtime();
    double time = end - start;


    return time;
}

int main(int argc, char** argv)
{
    if (argc > 1) {
        THREADS = std::atoi(argv[1]);
    }
    
    int arr[] = {1, 2, 4, 7, 8, 16, 20, 40};
    double res[10];
    double serial = integrate_serial();
    for(int i=0;i<8;i++){
        THREADS =arr[i];
        double parallel = integrate_parallel();
        res[i]=parallel;
        std::cout << serial/parallel << ", ";
    }
    std::cout << "\n";
    for(int i=0;i<8;i++){
        std::cout << res[i] << ", ";
    }
    

    return 0;
}
