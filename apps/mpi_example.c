/*
┌─────────────────────────────────────────────────────────────────────────────────────┐
│ Usage example of the parallel quicksort functions implemented with the MPI library. │
└─────────────────────────────────────────────────────────────────────────────────────┘
*/

#if defined(__STDC__)
#if (__STDC_VERSION__ >= 199901L)
#define _XOPEN_SOURCE 700
#endif
#endif

// --------------------------------- INCLUDES ---------------------------------

#include "qsort.h"
#include <mpi.h>

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

        // Define data size and data array
		int N = (argc > 1) ? atoi(argv[1]) : N_dflt;
        data_t *data = NULL;

        // Random data generation (only master)
        if (rank == 0) {
            generate_data(&data,N);
        }

        // Define arrays for the local_data
        data_t* local_data = NULL; 
        int local_size = 0;

        // Splitting the data
        MPI_Split(data, N, &local_data, &local_size, rank, size, MPI_DATA_T);

        // Sorting -------------------------------------------------------------


            // Start time -----------------------
            double timer = MPI_Wtime();

            // Sorting algorithms --------------- 
            // Comment / uncomment the desired algorithm

            // Simple parallel qsort ------------
            MPI_Parallel_qsort(&local_data, &local_size,
                               ranks, size, MPI_COMM_WORLD, MPI_DATA_T,
                               compare_ge);
            
            // Hyperquicksort -------------------
            // MPI_Hyperquicksort(&local_data, &local_size, 
            //                    ranks, size, MPI_COMM_WORLD, MPI_DATA_T,
            //                    compare_ge);

            // PSRS -----------------------------
            // MPI_PSRS(&local_data, &local_size, 
            //          N, rank, size,
            //          MPI_DATA_T,
            //          compare_ge);

            // End time -------------------------
            timer = MPI_Wtime() - timer;


        // Merge the sorted arrays in the master process -----------------------
        MPI_Merge(data, local_data, local_size, rank, size, MPI_DATA_T);

        // Verification --------------------------------------------------------

        if (rank==0) {
            fprintf(stdout, "⏱  | Elapsed: %.4f s.\n", timer);
            if (verify_sorting(data, 0, N))
                fprintf(stdout, "✅ | The array is sorted correctly.\n");
            else
                fprintf(stdout, "⛔ | The array is not sorted correctly.\n");

            // Only master frees data
            if (data != NULL) {
                free(data);
                data = NULL;
            }
        }

        // Freeing memory ------------------------------------------------------
        if (local_data != NULL) {
            free(local_data);
            local_data = NULL;
        }
        free(ranks);
        MPI_Type_free(&MPI_DATA_T); // Freeing the MPI data type

        // Finalize MPI --------------------------------------------------------
        MPI_Finalize();

    #else
        int N = (argc > 1) ? atoi(argv[1]) : N_dflt;
        fprintf(stdout, "This program requires both MPI and OpenMP to be enabled to sort the %d elements.\n", N);
    #endif

	return 0;
}