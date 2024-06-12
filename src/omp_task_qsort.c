#include "qsort.h"

#if defined(_OPENMP)

void omp_task_qsort(data_t *data, int start, int end, compare_t cmp_ge) {

	#if defined(DEBUG)
	#define CHECK                                                                   \
	{                                                                               \
		if (verify_partitioning(data, start, end, mid))                             \
		{                                                                           \
			printf("partitioning is wrong\n");                                      \
			printf("%4d, %4d (%4d, %g) -> %4d, %4d  +  %4d, %4d\n",                 \
				start, end, mid, data[mid].data[HOT], start, mid, mid + 1, end);    \
			show_array(data, start, end);                                           \
		}                                                                           \
	}
	#endif

		int size = end - start;
		if (size > 2) { // if the size is >2 we recursively sort the two halves
			int mid = partitioning(data, start, end, cmp_ge);

		#if defined(DEBUG)
				CHECK; // check the partitioning only if DEBUG is defined
		#endif

			#pragma omp task
			omp_task_qsort(data, start, mid, cmp_ge);

	        #pragma omp task
			omp_task_qsort(data, mid + 1, end, cmp_ge);

		} else { // if the size is 2 we swap the two elements if necessary
			if ((size == 2) && cmp_ge((void *)&data[start], (void *)&data[end - 1]))
				SWAP((void *)&data[start], (void *)&data[end - 1], sizeof(data_t));
		}
}

#endif