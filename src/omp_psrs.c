#include "qsort.h"

#if defined(_OPENMP)

void omp_psrs(data_t *data, int start, int end, compare_t cmp_ge) {

	// Shared variables
	double *samples = NULL;
	int **prefix_matrix = NULL;
	int *index = NULL;
	int array_size = end - start;

	#pragma omp parallel
	{
		// Number of threads and id
		int nthreads = omp_get_num_threads();
		int id = omp_get_thread_num();

		// Each thread picks a chunk of the array
		chunk_t chunk = split(start, end, nthreads, id);

		// Each thread sorts its chunk serially
		serial_qsort(data, chunk.start, chunk.end+1, cmp_ge);

		// One thread allocates memory for the index array
		#pragma omp single nowait
		index = (int *)malloc(array_size * sizeof(int));

		// One thread allocates memory for the samples array
		#pragma omp single nowait
		samples = (double *)malloc(nthreads * nthreads * sizeof(double));

		// Memory allocation for the (nthreads+1)*(nthreads+1) prefix_matrix
		#pragma omp single
		{
			prefix_matrix = (int **)malloc((nthreads+1) * sizeof(int *));
			prefix_matrix[0] = (int *)malloc((nthreads+1) * sizeof(int));
		}
		prefix_matrix[id+1] = (int *)malloc((nthreads+1) * sizeof(int));
		
		// Filling first row and first column with zeros
		#pragma omp single nowait
		prefix_matrix[0][0] = 0;

		prefix_matrix[0][id+1] = 0;
		prefix_matrix[id+1][0] = 0;

		// Each thread samples "nthreads" pivots at regular intervals 
		int step = array_size / (nthreads * nthreads);
		#pragma omp parallel for
		for (int i = 0; i < nthreads; i++) {
			int sample_index = id * nthreads + i;
			int data_index = chunk.start + i * step;
			if (sample_index >= nthreads*nthreads || data_index >= array_size) {
				fprintf(stderr, " ERROR: Index out of bounds in OMP PSRS sampling.\n");
				exit(1);
			}
			samples[sample_index] = data[data_index].data[HOT];
		}

		#pragma omp barrier // wait for all the threads to fill the samples array before sorting it

		// One thread sorts the samples array
		#pragma omp single
		qsort(samples, nthreads * nthreads, sizeof(double), compare_double);

		// All the threads select the same nthreads-1 pivots from the samples array
		double *pivots = (double *)malloc((nthreads - 1) * sizeof(double));

		// #pragma omp parallel for
		for (int i = 0; i < nthreads - 1; i++) {
			pivots[i] = samples[(i + 1) * nthreads];
		} 

		// Each thread partitions its chunk using the pivots (the mids array goes from index 0 to nthreads-1)
		int *mids = p_partitioning(data, chunk.start, chunk.end+1, pivots, nthreads-1);

		// Each thread writes the number of elements in the other partitions except the last
		#pragma omp parallel for
		for (int i = 0; i < nthreads-1; i++) {
			prefix_matrix[i+1][id+1] = mids[i+1] - mids[i];
		}

		// Each thread writes the number of elements in the last partition
		prefix_matrix[nthreads][id+1] = chunk.size - mids[nthreads - 1];

		#pragma omp barrier // wait for all the threads to finish before computing the prefix sum of rows

		// Each thread computes the prefix sum of a row of the prefix matrix
		for (int j = 1; j < nthreads+1; j++) {
			prefix_matrix[id+1][j] += prefix_matrix[id+1][j-1];
		}

		#pragma omp barrier // wait for all prefix sums of rows to finish before prefix sum of last column

		// Compute and store in an array the prefix sum of the last column of the prefix_matrix
		int* prefix_sum = (int *)malloc((nthreads + 1) * sizeof(int));
		prefix_sum[0] = 0;
		for (int i = 1; i < nthreads+1; i++) {
			prefix_sum[i] = prefix_sum[i-1] + prefix_matrix[i][nthreads];
		}

		// Each thread writes the new element positions in the shared index array
		for (int k = 1; k < nthreads; k++) {
			for (int i = mids[k-1]; i < mids[k]; i++) {
				index[chunk.start + i] = prefix_sum[k-1] + prefix_matrix[k][id] + (i - mids[k-1]);
			}

			// Last partition
			for ( int i = mids[nthreads-1]; i < chunk.size; i++) {
				index[chunk.start + i] = prefix_sum[nthreads-1] + prefix_matrix[nthreads][id] + (i - mids[nthreads-1]);
			}
		}

		#pragma omp barrier // wait for all the threads to write their indexes before swapping

		// One thread takes the job of reordering the elements
		#pragma omp single
		{
			for (int i = 0; i < array_size; i++) {
				while (index[i] != start + i) {
					SWAP((void *)&data[start + i], (void *)&data[index[i]], sizeof(data_t));
					SWAP((void *)&index[i], (void *)&index[index[i] - start], sizeof(int));
				}
			}
		}

		// Finally each thread sorts serially a portion of the array that,
		// looking at the last column, goes
		// from:
		int sstart = prefix_sum[id];
		// to:
		int send = prefix_sum[id] + prefix_matrix[id+1][nthreads];

		serial_qsort(data, sstart, send, cmp_ge);

		// Free memory
		free(pivots);
		free(mids);
		free(prefix_sum);
		
		// Freeing the prefix matrix
		free(prefix_matrix[id+1]);

		// Freeing the shared variables
		#pragma omp barrier
		#pragma omp single nowait
		{
			free(prefix_matrix[0]);
			free(prefix_matrix);
		}

		#pragma omp single nowait
		free(samples);

		#pragma omp single nowait
		free(index);
	}
}

#endif
