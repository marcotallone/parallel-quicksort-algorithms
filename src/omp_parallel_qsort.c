#include "qsort.h"
#include <math.h>

#if defined(_OPENMP)

void omp_parallel_qsort(data_t *data, int start, int end, compare_t cmp_ge, int depth) {

	// Shared variables
	double common_pivot = 0;
	int id_counter = 0;
	int nthreads;
	int *low_sum = NULL;
	int *high_sum = NULL;
	int *index = NULL;
	int array_size = end - start;

	#pragma omp parallel
	nthreads = omp_get_num_threads();

	if (depth < log2(nthreads)) {
		#pragma omp parallel
		{
			// Each thread gets an "artificial" ordered id
			// depending on the order in which they enter the recursive call
			int id;
			#pragma omp critical
			id = id_counter++;

			#pragma omp single nowait
			index = (int *)malloc(array_size * sizeof(int));

			#pragma omp single nowait
			{
				low_sum = (int *)malloc((nthreads+1) * sizeof(int));
				low_sum[0] = 0; // the first element is always 0
			}
			
			#pragma omp single nowait
			{
				high_sum = (int *)malloc((nthreads+1) * sizeof(int));
				high_sum[0] = 0; // the first element is always 0
			}

			// Each thread picks a chunk of the array
			chunk_t chunk = split(start, end, nthreads, id);

			// One thread randomly decides a pivot for all from its partition
			#pragma omp single
			common_pivot = data[chunk.start + (chunk.end - chunk.start)/2].data[HOT];
		
			// Partition the chunk and find the mid, i.e. index of the first element >= common_pivot 
			int mid = partitioning_low_high(data, chunk.start, chunk.end+1, common_pivot) - chunk.start;

			// Each thread writes the count of low and high elements in its location of the sum lists
			low_sum[id+1] = mid;                       // mid is the index the first element >= pivot
			high_sum[id+1] = chunk.size - mid;         // total elements - number of elements < pivot

			#pragma omp barrier     // wait for all threads to finish before computing the prefix sum

			// One thread computes the prefix sum of the low array
			#pragma omp single nowait
			for (int i = 1; i < nthreads+1; i++)
				low_sum[i] += low_sum[i - 1];

			// One thread computes the prefix sum of the high array
			#pragma omp single 
			for (int i = 1; i < nthreads+1; i++)
				high_sum[i] += high_sum[i - 1];

			// Each thread writes the new element positions in the shared index array
			const int total_low_elements = low_sum[nthreads];
			const int low_sum_id = low_sum[id];
			const int high_sum_id = high_sum[id];

			for (int i = 0; i < mid; i++)
				index[chunk.start - start + i] = start + low_sum_id + i;

			for (int i = mid; i < chunk.size; i++)
				index[chunk.start - start + i] = start + total_low_elements + high_sum_id + i - mid;

			#pragma omp barrier // wait for all the threads to write their indexes before swapping

			// One thread takes the job of reordering the elements and launching the recursive calls
			#pragma omp single
			{
				for (int i = 0; i < array_size; i++) {
					while (index[i] != start + i) {
						SWAP((void *)&data[start + i], (void *)&data[index[i]], sizeof(data_t));
						SWAP((void *)&index[i], (void *)&index[index[i] - start], sizeof(int));
					}
				}

				// Recursive call of the function to sort the two halves
				#pragma omp task
				if (start < start + low_sum[nthreads])
					omp_parallel_qsort(data, start, start + low_sum[nthreads], cmp_ge, depth+1);

				#pragma omp task
				if (start + low_sum[nthreads] < end)
					omp_parallel_qsort(data, start + low_sum[nthreads], end, cmp_ge, depth+1);
			}

			// Free the memory of the shared variables
			#pragma omp single nowait
			free(low_sum);

			#pragma omp single nowait
			free(high_sum);

			#pragma omp single nowait
			free(index);
		}

	} else { // if depth >= log2(nthreads) we sort serially
		serial_qsort(data, start, end, cmp_ge);
	}
}

#endif
