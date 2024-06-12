#include "qsort.h"

#if defined(MPI_VERSION) && defined(_OPENMP)

// Parallel quicksort function (MPI)
void MPI_Parallel_qsort(data_t **local_data, int *local_size, 
                        int *ranks, int size,
                        MPI_Comm comm, MPI_Datatype MPI_DATA_T,
                        compare_t cmp_ge) {

    if (size > 1) {

        if(size % 2 != 0) {
            fprintf(stdout, "ERROR: Odd number of processes in MPI_Parallel_qsort.\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }

        // Divide the processes in the communicator in 2 groups
        int rank; // rank in the recursive call
        MPI_Comm_rank(comm, &rank);
        int *low_processes  = (int *)malloc((size/2) * sizeof(int));
        int *high_processes = (int *)malloc((size/2) * sizeof(int));
        for (int i = 0; i < size/2; i++) {
            low_processes[i]  = ranks[i];          // rank <  size/2
            high_processes[i] = ranks[i + size/2]; // rank >= size/2
        }

        // Master process picks median pivot in its chunk and broadcasts it
        double pivot = 0;
        if (rank == 0) {
            double a = (*local_data)[0].data[HOT];
            double b = (*local_data)[(*local_size)/2].data[HOT];
            double c = (*local_data)[*local_size-1].data[HOT];
            pivot = a < b ? (b < c ? b : (a < c ? c : a)) 
                    : (a < c ? a : (b < c ? c : b));       
        }
        MPI_Bcast(&pivot, 1, MPI_DOUBLE, 0, comm);

        // Each process partitions its chunk according to the pivot
        int mid = partitioning_low_high(*local_data, 0, *local_size, pivot);

        // Each process in the low group sends the size of its "high" partition (from mid included to end excluded)
        // to one process of the high group and receives from it the size of the "low" partition (from start included
        // to mid excluded) in a sendrecv operation
        int new_size  = 0;
        int low_size  = mid;
        int high_size = *local_size - mid;

        if (rank < size/2) {
            MPI_Sendrecv(&high_size,    1,   MPI_INT,        // adress send buffer, count send elements, type of send elements
                         rank + size/2, 0,                   // rank of the process to send to, tag
                         &new_size,     1,   MPI_INT,        // adress receive buffer, count receive elements, type of receive elements
                         rank + size/2, 0,                   // rank of the process to receive from, tag
                         comm, MPI_STATUS_IGNORE);           // communicator, status
        } else {
            MPI_Sendrecv(&low_size,    1,   MPI_INT,         // adress send buffer, count send elements, type of send elements
                         rank - size/2, 0,                   // rank of the process to send to, tag
                         &new_size,     1,   MPI_INT,        // adress receive buffer, count receive elements, type of receive elements
                         rank - size/2, 0,                   // rank of the process to receive from, tag
                         comm, MPI_STATUS_IGNORE);           // communicator, status
        }

        // Allocate memory for the incoming data
        data_t *incoming_data = (data_t *)malloc(new_size * sizeof(data_t));

        // Each process of the low group send its "high" partition to one process of the high group
        // and receives the "low" partition from that same other process in a sendrecv operation
        if (rank < size/2) {
            MPI_Sendrecv(&(*local_data)[mid], high_size, MPI_DATA_T,
                         rank + size/2, 0,
                         incoming_data, new_size, MPI_DATA_T,
                         rank + size/2, 0,
                         comm, MPI_STATUS_IGNORE);
        } else {
            MPI_Sendrecv(&(*local_data)[0], low_size, MPI_DATA_T,
                         rank - size/2, 0,
                         incoming_data, new_size, MPI_DATA_T,
                         rank - size/2, 0,
                         comm, MPI_STATUS_IGNORE);
        }

        // Merge the incoming data with the local data
        data_t *merged = NULL;
        if (rank < size/2) {
            merged = merge(*local_data, 0, low_size, incoming_data, 0, new_size);
            *local_size = low_size + new_size;
            free(incoming_data);
        } else {
            merged = merge(incoming_data, 0, new_size, *local_data, mid, *local_size);
            *local_size = high_size + new_size;
            free(incoming_data);
        }

        // Free the initial local data and update the pointer to the new merged data
        free(*local_data);
        *local_data = merged;

        // Define 2 new communicators for the recursive calls
        MPI_Comm low_comm, high_comm;
        MPI_Comm_split(comm, rank < size/2, rank, &low_comm);
        MPI_Comm_split(comm, rank >= size/2, rank, &high_comm);

        // Recursive calls
        if (rank < size/2) {
            // Low group processes sort the low partition
            MPI_Parallel_qsort(local_data, local_size,
                               low_processes, size/2,
                               low_comm, MPI_DATA_T,
                               cmp_ge);
        } else {
            // High group processes sort the high partition
            MPI_Parallel_qsort(local_data, local_size,
                               high_processes, size/2, 
                               high_comm, MPI_DATA_T,
                               cmp_ge);
        }

        // Free memory
        free(low_processes);
        free(high_processes);

    } else { // If only one process is entering the recursive call we sort locally 

        #pragma omp parallel
        #pragma omp single
        omp_task_qsort(*local_data, 0, *local_size, cmp_ge);

    }
}

#endif
