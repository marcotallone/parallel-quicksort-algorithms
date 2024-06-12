#include "qsort.h"

void serial_qsort(data_t *data, int start, int end, compare_t cmp_ge) {

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
	#else
	#define CHECK
	#endif

		int size = end - start;
		if (size > 2) { // if the size is >2 we recursively sort the two halves
			int mid = partitioning(data, start, end, cmp_ge);

			CHECK; // check the partitioning only if DEBUG is defined

			serial_qsort(data, start, mid, cmp_ge);   // sort the left half
			serial_qsort(data, mid + 1, end, cmp_ge); // sort the right half
		} else { // if the size is 2 we swap the two elements if necessary
			if ((size == 2) && cmp_ge((void *)&data[start], (void *)&data[end - 1]))
				SWAP((void *)&data[start], (void *)&data[end - 1], sizeof(data_t));
		}
}
