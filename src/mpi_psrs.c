#include "qsort.h"

#if defined(_OPENMP) && defined(MPI_VERSION)

// Parallel Sort by Regular Sampling (PSRS) function (MPI)
void MPI_PSRS(data_t **local_data, int *local_size,
              int global_size, int rank, int size,
              MPI_Datatype MPI_DATA_T,
              compare_t cmp_ge) {

    // Each process sorts its local data
    #pragma omp parallel
    #pragma omp single
    omp_task_qsort(*local_data, 0, *local_size, cmp_ge);

    // Just check in case there is only 1 process
    if (size == 1) { return; }

    // Each process samples its local data
    double *local_samples = (double *)malloc(size * sizeof(double));
    int step = (global_size) / (size * size);
    #pragma omp parallel for
    for (int i = 0; i < size; i++) {
        local_samples[i] = (*local_data)[i * step].data[HOT];
    }    

    // Gather all the samples in the root process
    double *samples = NULL;
    if (rank == 0)
        samples = (double *)malloc(size * size * sizeof(double));
    
    MPI_Gather(local_samples, size, MPI_DOUBLE,
               samples, size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    free(local_samples);

    // The root process sorts the samples and selects the pivots
    double *pivots = (double *)malloc((size - 1) * sizeof(double));
    if (rank == 0) {
        qsort(samples, size * size, sizeof(double), compare_double);
        #pragma omp parallel for
        for (int i = 0; i < size - 1; i++)
            pivots[i] = samples[(i + 1) * size];
    }

    // Broadcast the pivots to all the processes
    MPI_Bcast(pivots, size - 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    if (rank == 0)
        free(samples);

    // Each process partitions its chunk in p parts using the pivots
    int *local_mids = p_partitioning(*local_data, 0, *local_size, pivots, size - 1);

    // Each process measures the size of each partition
    int *local_partitions_counts = (int *)malloc(size * sizeof(int)); // Sizes of the local partitions
    #pragma omp parallel for
    for (int i = 0; i < size-1; i++)
        local_partitions_counts[i] = local_mids[i+1] - local_mids[i];
    local_partitions_counts[size-1] = *local_size - local_mids[size-1];

    // Each process sends and receives from the others the sizes of the partition
    int *counts = (int *)malloc(size * sizeof(int));    // Sizes of the partitions from the alltoall
    MPI_Alltoall(local_partitions_counts, 1, MPI_INT,   // Send buffer, number of elements to send, send data type
                 counts, 1, MPI_INT,                    // Receive buffer, number of elements to receive, receive data type
                 MPI_COMM_WORLD);                       // Communicator

    // Each process computes the rcvdispls for the alltoallv from the counts received
    int *rcvdispls = (int *)malloc(size * sizeof(int)); // Displacements for the alltoallv from the alltoall
    rcvdispls[0] = 0;
    for (int i = 1; i < size; i++)
        rcvdispls[i] = rcvdispls[i-1] + counts[i-1];

    // Each process computes how many elements it's going to be storing based on the counts
    int local_sorted_size = 0;
    #pragma omp parallel for reduction(+:local_sorted_size)
    for (int i = 0; i < size; i++)
        local_sorted_size += counts[i];

    *local_size = local_sorted_size;

    // Each process sends and receives the elements of the partitions in a alltoallv operation
    data_t *sorted_data = (data_t *)malloc((local_sorted_size) * sizeof(data_t));
    MPI_Alltoallv(*local_data, local_partitions_counts, local_mids, MPI_DATA_T, // Send buffer, number of elements to send, displacements, send data type
                  sorted_data, counts, rcvdispls, MPI_DATA_T,                   // Receive buffer, number of elements to receive, displacements, receive data type
                  MPI_COMM_WORLD);                                              // Communicator
    
    // Free the orirginal local_data and assign the sorted_data to it
    free(*local_data);
    *local_data = sorted_data;

    // Each process sorts the received data
    #pragma omp parallel
    #pragma omp single
    omp_task_qsort(*local_data, 0, *local_size, cmp_ge);

    // Freeing memory
    free(local_mids);
    free(local_partitions_counts);
    free(counts);
    free(rcvdispls);
}

#endif
