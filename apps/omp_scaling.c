/*
┌───────────────────────────────────────────────────────────────────────────────────────┐
│ Script to collect data for strong and weak scaling analysis of the OpenMP algorithms. │
└───────────────────────────────────────────────────────────────────────────────────────┘
*/

#if defined(__STDC__)
#if (__STDC_VERSION__ >= 199901L)
#define _XOPEN_SOURCE 700
#endif
#endif
#define T_dflt 5

// --------------------------------- INCLUDES ---------------------------------

#include "qsort.h"
#include <string.h>

// --------------------------------- MAIN -------------------------------------

int main(int argc, char **argv) {

	#if defined(_OPENMP)

		#if defined(DEBUG)
			fprintf(stdout, "🚧 Running in DEBUG mode\n");
		#endif

		// Enable nested parallelism
		omp_set_nested(1);

        // Variables --------------------------------------------------------------
        int N = (argc > 1) ? atoi(argv[1]) : N_dflt;		        // Number of elements
        char *method = (argc > 2) ? argv[2] : "serial";		        // Sorting method
		data_t *data = NULL;                                        // Array of data
        int trials = (argc > 3) ? atoi(argv[3]) : T_dflt;		    // Number of trials
        double *times = (double *)malloc(trials * sizeof(double));  // Array of times
        struct timespec ts;                                         // Time struct
        double timer = 0.0;                                         // Timer
        int correctly_sorted = 1;                                   // Correctly sorted boolean
        int nthreads = 1;                                           // Number of threads

        // Open a csv file to store the times ---------------------------------
        FILE *file = fopen("datasets/omp_scaling.csv", "a+");
        if (file != NULL) {
            // Move to the end of the file
            fseek(file, 0, SEEK_END);

            // If the file position indicator is 0, the file was just created
            if (ftell(file) == 0) {
                fprintf(file, "\"Method\",\"Sorted\",\"Threads\",\"Elements\",\"Elements (Scientific)\","
                            "\"Average time (s)\",\"St. Deviation (s)\",\"Min (s)\",\"Max (s)\"\n");
                fflush(file);
            }
        }

        // Sorting trials --------------------------------------------------------


            // Serial trials
            if (strcmp(method, "serial") == 0) {
                for (int i = 0; i < trials; i++) {
                    generate_data(&data, N);                 // Generate data
                    timer = CPU_TIME;                 		// Start timer
                    serial_qsort(data, 0, N, compare_ge);   // Sort
                    times[i] = CPU_TIME - timer;            // Stop timer
                    if (!verify_sorting(data, 0, N))        // Verify sorting
                        correctly_sorted = 0; // Not correctly sorted !
                    // Free memory
                    if (data != NULL) {
                        free(data);
                        data = NULL;
                    }
                }
            } else {
            // Parallel trials
                #pragma omp parallel
                #pragma omp single
                nthreads = omp_get_num_threads();

                for (int i = 0; i < trials; i++) {
                    generate_data(&data, N);                     // Generate data

                    // Timed sorting
                    if (strcmp(method, "task") == 0) {

                        timer = CPU_TIME;                       // Start timer
                        #pragma omp parallel
                        #pragma omp single
                        omp_task_qsort(data, 0, N, compare_ge);

                        times[i] = CPU_TIME - timer;            // Stop timer

                    } else if (strcmp(method, "simple") == 0) {

                        timer = CPU_TIME;                       // Start timer
                        omp_parallel_qsort(data, 0, N, compare_ge, 0);
                        times[i] = CPU_TIME - timer;            // Stop timer

                    } else if (strcmp(method, "hyper") == 0) {

                        timer = CPU_TIME;                       // Start timer
                        omp_hyperquicksort(data, 0, N, compare_ge, 0);
                        times[i] = CPU_TIME - timer;            // Stop timer

                    } else if (strcmp(method, "psrs") == 0) {

                        timer = CPU_TIME;                       // Start timer
                        omp_psrs(data, 0, N, compare_ge);
                        times[i] = CPU_TIME - timer;            // Stop timer

                    } else {
                        fprintf(stderr, "ERROR: The sorting method named %s is not available.\n", method);
                        exit(EXIT_FAILURE);
                    }

                    if (!verify_sorting(data, 0, N))            // Verify sorting
                        correctly_sorted = 0; // Not correctly sorted !
                    // Free memory
                    if (data != NULL) {
                        free(data);
                        data = NULL;
                    }
                }
            }

            if (correctly_sorted) {
                fprintf(file, "%s, %s,%d,%d,%e,%.6f,%.6f,%.6f,%.6f\n", 
                        method, "Yes", nthreads, N, (double)N,
                        mean(times, trials), stdev(times, trials),
                        min(times, trials), max(times, trials));
            } else {
                fprintf(file, "%s,%s,%d,%d,%e,%.6f,%.6f,%.6f,%.6f\n",
                        method, "No", nthreads, N, (double)N,
                        mean(times, trials), stdev(times, trials),
                        min(times, trials), max(times, trials));
            }


        // Close the csv file -----------------------------------------------------
        fclose(file);

		// Memory deallocation -----------------------------------------------------
		free(times);
        if (data != NULL) {
            free(data);
            data = NULL;
        }

	#endif

	return 0;
}
