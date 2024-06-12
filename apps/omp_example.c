/*
┌────────────────────────────────────────────────────────────────────────────┐
│ Usage example of the parallel quicksort functions implemented with OpenMP. │
└────────────────────────────────────────────────────────────────────────────┘
*/

#if defined(__STDC__)
#if (__STDC_VERSION__ >= 199901L)
#define _XOPEN_SOURCE 700
#endif
#endif

// --------------------------------- INCLUDES ---------------------------------

#include "qsort.h"

// --------------------------------- MAIN --------------------------------------

int main(int argc, char **argv) {

    #if defined(_OPENMP)

        #if defined(DEBUG)
            fprintf(stdout, "🚧 Running in DEBUG mode\n");
        #endif

        // Enable nested parallelism
        omp_set_nested(1);

        // Variables -----------------------------------------------------------
        struct timespec ts;
		int N = (argc > 1) ? atoi(argv[1]) : N_dflt;
        data_t *data = (data_t *)malloc(N * sizeof(data_t));
        int nthreads = 1;

        #pragma omp master
        {
            nthreads = omp_get_max_threads();
            fprintf(stdout, "🚀 | Running with %d threads\n", nthreads);
        }

        // Random data generation (only master)
        generate_data(&data,N);

        // Sorting -------------------------------------------------------------


            // Start time -----------------------
            double timer = CPU_TIME;

            // Sorting algorithms ---------------
            // Comment / uncomment the desired algorithm

            // Task parallel qsort --------------
            // #pragma omp parallel
            // #pragma omp single
            // omp_task_qsort(data, 0, N, compare_ge);

            // Simple parallel qsort ------------
            // omp_parallel_qsort(data, 0, N, compare_ge, 0);

            // Hyperquicksort -------------------
            omp_hyperquicksort(data, 0, N, compare_ge, 0);

            // PSRS -----------------------------
            // omp_psrs(data, 0, N, compare_ge);

            // End time -------------------------
            timer = CPU_TIME - timer;

        // Verification --------------------------------------------------------
        fprintf(stdout, "⏱  | Elapsed: %.4f s.\n", timer);
        if (verify_sorting(data, 0, N))
            fprintf(stdout, "✅ | The array is sorted correctly.\n");
        else
            fprintf(stdout, "⛔ | The array is not sorted correctly.\n");

        // Freeing memory ------------------------------------------------------
        free(data);

    #else
        int N = (argc > 1) ? atoi(argv[1]) : N_dflt;
        fprintf(stdout, "This program requires OpenMP to be enabled to sort the %d elements.\n", N);
    #endif

	return 0;
}
