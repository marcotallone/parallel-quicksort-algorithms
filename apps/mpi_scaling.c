/*
┌────────────────────────────────────────────────────────────────────────────────────┐
│ Script to collect data for strong and weak scaling analysis of the MPI algorithms. │
└────────────────────────────────────────────────────────────────────────────────────┘
*/

#if defined(__STDC__)
#if (__STDC_VERSION__ >= 199901L)
#define _XOPEN_SOURCE 700
#endif
#endif
#define T_dflt 10
#define MG_dflt 0 // Default data generation: - 0 (each process generates its own data)
                  //                          - 1 (master generates and collect the data)

// --------------------------------- INCLUDES ----------------------------------

#include "qsort.h"
#include <string.h>

// --------------------------------- MAIN --------------------------------------

int main(int argc, char **argv) {

    #if defined(MPI_VERSION) && defined(_OPENMP)

		#if defined(DEBUG)
			fprintf(stdout, "🚧 Running in DEBUG mode\n");
		#endif

        // Initialize MPI ------------------------------------------------------
        int mpi_provided_thread_level;
        MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &mpi_provided_thread_level);
        if (mpi_provided_thread_level < MPI_THREAD_FUNNELED)
        {
            printf("ERROR: A problem arise when asking for MPI_THREAD_FUNNELED level.\n");
            MPI_Finalize();
            exit(1);
        }

        // Enable nested parallelism
		omp_set_nested(1);

        // Repeated trials variables -------------------------------------------
        char *method = (argc > 2) ? argv[2] : "serial";		        // Sorting method
        const int trials = (argc > 3) ? atoi(argv[3]) : T_dflt;     // Number of trials
        const int mastergen = (argc > 4) ? atoi(argv[4]) : MG_dflt; // Master data generation
        double *times = (double *)malloc(trials * sizeof(double));  // Array of times

        // Variables -----------------------------------------------------------

        // MPI variables
        int rank, size;
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);              // Process rank (ID)
        MPI_Comm_size(MPI_COMM_WORLD, &size);              // Number of processes
        int *ranks = (int *)malloc(size * sizeof(int));    // Array of ranks
        for (int i = 0; i < size; i++) {
            ranks[i] = i;
        }
        MPI_Datatype MPI_DATA_T;                           // MPI data_t type
        MPI_Type_contiguous(DATA_SIZE, MPI_DOUBLE, &MPI_DATA_T);
        MPI_Type_commit(&MPI_DATA_T);

        // Variables for the sorting trials
        int nthreads = 1;                                   // Number of threads
        int processes = 1;                                  // Number of processes
        double timer = 0.0;                                 // Timer
        int correctly_sorted = 1;                           // Correctly sorted boolean

        // Define data size and data array
		int N = (argc > 1) ? atoi(argv[1]) : N_dflt;        // Number of elements
        data_t *data = NULL;                                // Array of data

        // Define arrays for the local_data
        data_t* local_data = NULL; 
        int local_size = 0;

        // Open a csv file to store the times ----------------------------------
        FILE *file = fopen("datasets/mpi_scaling.csv", "a+");
        if (rank == 0) {
            if (file != NULL) {
                // Move to the end of the file
                fseek(file, 0, SEEK_END);

                // If the file position indicator is 0, the file was just created
                if (ftell(file) == 0) {
                    fprintf(file, "\"Method\",\"Sorted\",\"Processes\",\"Threads\",\"Elements\",\"Elements (Scientific)\","
                                  "\"Average time (s)\",\"St. Deviation (s)\",\"Min (s)\",\"Max (s)\"\n");
                    fflush(file);
                }
            }
        }

        // Sorting trials ------------------------------------------------------


            // Serial trials (only master process)
            if (strcmp(method, "serial") == 0) {

                if (rank == 0) {
                    for (int i = 0; i < trials; i++) {
                        generate_data(&data, N);

                        // Timed sorting
                        timer = MPI_Wtime();                    // Start timer
                        serial_qsort(data, 0, N, compare_ge);   // Sorting
                        times[i] = MPI_Wtime() - timer;         // Stop timer
                        if (!verify_sorting(data, 0, N))        // Verify sorting
                            correctly_sorted = 0; // Not correctly sorted !
                        // Free memory
                        if (data != NULL) {
                            free(data);
                            data = NULL;
                        }
                    }

                    // Master appends the results to the csv file
                    if (correctly_sorted) {
                        fprintf(file, "%s,%s,%d,%d,%d,%e,%.6f,%.6f,%.6f,%.6f\n", 
                                method, "Yes", processes, nthreads, N, (double)N,
                                mean(times, trials), stdev(times, trials), 
                                min(times, trials), max(times, trials));
                    } else {
                        fprintf(file, "%s,%s,%d,%d,%d,%e,%.6f,%.6f,%.6f,%.6f\n", 
                                method, "No", processes, nthreads, N, (double)N,
                                mean(times, trials), stdev(times, trials), 
                                min(times, trials), max(times, trials));
                    }
                }

            } else {
                // Parallel trials (all processes)
                processes = size;

                #pragma omp parallel
                #pragma omp single
                nthreads = omp_get_num_threads();

                for (int i = 0; i < trials; i++) {

                    if (mastergen) {
                        // Random data generation and broadcast by the master
                        if (rank == 0) {
                            generate_data(&data,N);
                        }
                        // Splitting and sending the data
                        MPI_Split(data, N, &local_data, &local_size, rank, size, MPI_DATA_T);
                    } else {
                        // Each process generates its own data
                        chunk_t chunk = split(0, N, size, rank);
                        local_size = chunk.size;
                        generate_data(&local_data, local_size);
                    }

                    // Timed sorting
                    if (strcmp(method, "simple") == 0) {
                        timer = MPI_Wtime();                        // Start timer
                        MPI_Parallel_qsort(&local_data, &local_size,
                                           ranks, size, MPI_COMM_WORLD, MPI_DATA_T,
                                           compare_ge);
                        times[i] = MPI_Wtime() - timer;             // Stop timer
                    } else if (strcmp(method, "hyper") == 0) {
                        timer = MPI_Wtime();                        // Start timer
                        MPI_Hyperquicksort(&local_data, &local_size, 
                                           ranks, size, MPI_COMM_WORLD, MPI_DATA_T,
                                           compare_ge);
                        times[i] = MPI_Wtime() - timer;             // Stop timer
                    } else if (strcmp(method, "psrs") == 0) {
                        timer = MPI_Wtime();                        // Start timer
                        MPI_PSRS(&local_data, &local_size, 
                                 N, rank, size,
                                 MPI_DATA_T,
                                 compare_ge);
                        times[i] = MPI_Wtime() - timer;             // Stop timer
                    } else {
                        fprintf(stderr, "ERROR: The sorting method named %s is not available.\n", method);
                        MPI_Finalize();
                        exit(1);
                    }

                    if (mastergen) {
                        // // Merge the sorted arrays in the master process to verify
                        MPI_Merge(data, local_data, local_size, rank, size, MPI_DATA_T);
                        if (rank == 0) {
                            if (!verify_sorting(data, 0, N))           // Verify sorting
                                correctly_sorted = 0; // Not correctly sorted !
                            
                            //Free memory
                            if (data != NULL) {
                                free(data);
                                data = NULL;
                            }
                        }
                    }

                    if (local_data != NULL) {
                        free(local_data);
                        local_data = NULL;
                    }
                }

                // Master appends the results to the csv file
                if (rank == 0) {
                    if (correctly_sorted) {
                        fprintf(file, "%s,%s,%d,%d,%d,%e,%.6f,%.6f,%.6f,%.6f\n", 
                                method, "Yes", processes, nthreads, N, (double)N,
                                mean(times, trials), stdev(times, trials), 
                                min(times, trials), max(times, trials));
                    } else {
                        fprintf(file, "%s,%s,%d,%d,%d,%e,%.6f,%.6f,%.6f,%.6f\n", 
                                method, "No", processes, nthreads, N, (double)N,
                                mean(times, trials), stdev(times, trials), 
                                min(times, trials), max(times, trials));
                    }
                }
            }


        // Close the csv file --------------------------------------------------
        fclose(file);

		// Memory deallocation -------------------------------------------------
		free(times);
        free(ranks);
        MPI_Type_free(&MPI_DATA_T); // Freeing the MPI data type

        // Finalize MPI --------------------------------------------------------
        MPI_Finalize();

	#endif

	return 0;
}
