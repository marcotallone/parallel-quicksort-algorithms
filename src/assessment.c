#include "qsort.h"

int verify_sorting(data_t *data, int start, int end) {
	int i = start;
	while ((++i < end) && (data[i].data[HOT] >= data[i - 1].data[HOT]));
	return (i == end);
}

int verify_partitioning(data_t *data, int start, int end, int mid) {
	int failure = 0;
	int fail = 0;

	for (int i = start; i < mid; i++)
		if (compare((void *)&data[i], (void *)&data[mid]) >= 0)
			fail++;

	failure += fail;
	if (fail) {
		printf("failure in first half\n");
		fail = 0;
	}

	for (int i = mid + 1; i < end; i++)
		if (compare((void *)&data[i], (void *)&data[mid]) < 0)
			fail++;

	failure += fail;
	if (fail)
		printf("failure in second half\n");

	return failure;
}

void show_array(data_t *data, int start, int end) {
	printf("[");
	printf("\033[1;30m");
	for (int i = start; i < end-1; i++)
		printf("%2.0f, ", data[i].data[HOT]);
	printf("%2.0f", data[end-1].data[HOT]);
	printf("\033[0m");	
	printf("]\n");
	
	// Print indexes below the numbers in gray color
	printf("\033[1;34m ");
	for (int i = start; i < end-1; i++)
		printf("%2d  ", i);
	printf("%2d\033[0m\n", end-1);	
}

void show_int_array(int *data, int start, int end) {
	printf("[");
	for (int i = start; i < end-1; i++)
		printf("%d, ", data[i]);
	printf("%d]\n", data[end-1]);

	// Print indexes below the numbers in gray color
	// printf("\033[1;30m");
	// for (int i = start; i < end-1; i++)
	// 	printf("%2d, ", i);
	// printf("%2d\033[0m\n", end-1);
}

void show_double_array(double *data, int start, int end) {
	printf("[");
	for (int i = start; i < end-1; i++)
		printf("%f, ", data[i]);
	printf("%f]\n", data[end-1]);

	// Print indexes below the numbers in gray color
	// printf("\033[1;30m");
	// for (int i = start; i < end-1; i++)
	// 	printf("%2d, ", i);
	// printf("%2d\033[0m\n", end-1);
}

double mean(double *data, int size) {

	double sum = 0;
	for (int i = 0; i < size; i++)
		sum += data[i];
	return sum / size;
}

double min(double *data, int size) {
	double min = data[0];
	for (int i = 1; i < size; i++)
		if (data[i] < min)
			min = data[i];
	return min;
}

double max(double *data, int size) {
	double max = data[0];
	for (int i = 1; i < size; i++)
		if (data[i] > max)
			max = data[i];
	return max;
}

double stdev(double *data, int size) {
	double avg = mean(data, size);
	double sum = 0;
	for (int i = 0; i < size; i++)
		sum += (data[i] - avg) * (data[i] - avg);
	return sqrt(sum / size);
}

void generate_data(data_t **data, int N) {
    *data = (data_t *)malloc(N * sizeof(data_t));

	#if defined(_OPENMP)
        // Parallel initialization
        #pragma omp parallel
        {
            int me = omp_get_thread_num();
            short unsigned int seed = time(NULL) % ((1 << sizeof(short int)) - 1);
            short unsigned int seeds[3] = {seed - me, seed + me, seed + me * 2};

            #pragma omp for
            for (int i = 0; i < N; i++)
                (*data)[i].data[HOT] = erand48(seeds);
        }
    #else
        // Serial initialization
        long int seed = time(NULL);
        srand48(seed);

        for (int i = 0; i < N; i++)
            (*data)[i].data[HOT] = drand48();
    #endif
}

#if defined(MPI_VERSION) && defined(_OPENMP)

void MPI_Split(data_t *data, int N, data_t **local_data, int *local_size, int rank, int size, MPI_Datatype MPI_DATA_T) {

	if (size > 1) {
		// Splitting the data -> each process computes its chunk dimensions
		chunk_t chunk = split(0, N, size, rank);

		// Each process will have a local copy of its data
		*local_data = (data_t *)malloc(chunk.size * sizeof(data_t));

		// The master process sends the chunks to the other processes
		if (rank == 0) {
			for (int i = 1; i < size; i++) {
				chunk_t other_chunk = split(0, N, size, i);
				MPI_Send(data + other_chunk.start, other_chunk.size, MPI_DATA_T, i, 0, MPI_COMM_WORLD);

				// The master process also copies its local data
				memcpy(*local_data, data + chunk.start, chunk.size * sizeof(*data));
			}
		} else {
			MPI_Recv(*local_data, chunk.size, MPI_DATA_T, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}

		*local_size = chunk.size;
	} else {
		// if there is only one process, the local data is a copy of the global data
		*local_size = N;
		*local_data = (data_t *)malloc(N * sizeof(data_t));
		memcpy(*local_data, data, N * sizeof(*data));
	}
}

void MPI_Merge(data_t *data, data_t *sorted_data, int sorted_size, int rank, int size, MPI_Datatype MPI_DATA_T) {

	if (size > 1) {
        // The root process writes the size of its sorted array in a array of sizes
        int *sorted_sizes = NULL;
        if (rank == 0) {
            sorted_sizes = (int *)malloc(size * sizeof(int));
        }

        // Gather the size of the sorted arrays from the other processes
        MPI_Gather(&sorted_size, 1, MPI_INT, sorted_sizes, 1, MPI_INT, 0, MPI_COMM_WORLD);

        // The root process gaters the sorted arrays from the other processes and fills the final sorted array
        if (rank == 0) {

            // Copy the sorted data from the root process
            #pragma omp parallel for
            for (int i = 0; i < sorted_size; i++) {
                data[i] = sorted_data[i];
            }

            // Gather the sorted data from the other processes
            int *displs = (int *)malloc(size * sizeof(int));
            displs[0] = 0;
            for (int i = 1; i < size; i++) {
                displs[i] = displs[i - 1] + sorted_sizes[i - 1];
            }

			// Gatherv is needed because the size of the sorted arrays can differ in each process
            MPI_Gatherv(MPI_IN_PLACE, 0, MPI_DATA_T, data, sorted_sizes, displs, MPI_DATA_T, 0, MPI_COMM_WORLD);

            // Freeing memory
            free(displs);
            free(sorted_sizes);
        } else {
            MPI_Gatherv(sorted_data, sorted_size, MPI_DATA_T, NULL, NULL, NULL, MPI_DATA_T, 0, MPI_COMM_WORLD);
        }
	} else {
		// Copy the sorted data from the root process
		#pragma omp parallel for
		for (int i = 0; i < sorted_size; i++) {
			data[i] = sorted_data[i];
		}
	}
}

#endif