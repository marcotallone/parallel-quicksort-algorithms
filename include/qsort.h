#ifndef QSORT_H__
#define QSORT_H__

// --------------------------------- INCLUDES ---------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>

#if defined(_OPENMP)
	#include <omp.h>
#endif

#if defined(USE_MPI)
    #include <mpi.h>
#endif

// --------------------------------- MACROS ------------------------------------

#if defined(_OPENMP)
	// Measure the wall-clock time
	#define CPU_TIME (clock_gettime(CLOCK_REALTIME, &ts), \
					(double)ts.tv_sec + (double)ts.tv_nsec * 1e-9)

	// Measure the cpu thread time
	#define CPU_TIME_th (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &myts), \
						(double)myts.tv_sec + (double)myts.tv_nsec * 1e-9)
#else
	// Measure the cpu process time
	#define CPU_TIME (clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts), \
					(double)ts.tv_sec + (double)ts.tv_nsec * 1e-9)
#endif

#if defined(DEBUG)
	#define VERBOSE
#endif

#if defined(VERBOSE)
	#define PRINTF(...) printf(__VA_ARGS__)
#else
	#define PRINTF(...)
#endif

#if !defined(DATA_SIZE)
	#define DATA_SIZE 8
#endif
#define HOT 0

// Dfault number of elements to be sorted
#if (!defined(DEBUG) || defined(_OPENMP))
	#define N_dflt 100000
#else
	#define N_dflt 10000
#endif

// --------------------------------- STRUCTS ----------------------------------
// The data to be sorted consists in a structure, just to mimic that you may have
// to sort not basic types, whch may have some effects on the memory efficiency.
// The structure, defined below as data_t is just an array of DATA_SIZE double,
// where DATA_SIZE is defined here below.
// However, for the sake of simplicity, we assume that we sort with respect one
// of the double in each structure, the HOTst one, where HOT is defined below (my
// choice was to set HOT to 0, so the code will use the first double to sort the
// data).
// Note that you can chnage the data structure and the ordering as you like.

// Data "data_t" type (simulates a complex data structure)
typedef struct {
	double data[DATA_SIZE];
} data_t;

// Chunk type to represent a portion of the array
typedef struct {
    int start;
    int end;
	int size;
} chunk_t;

// Macros for max and min between two data_t objects
#define MAX(a, b) ((a)->data[HOT] > (b)->data[HOT] ? (a) : (b));
#define MIN(a, b) ((a)->data[HOT] < (b)->data[HOT] ? (a) : (b));

// Macro for swapping two data_t objects
#define SWAP(A, B, SIZE)     \
	do                       \
	{                        \
		int sz = (SIZE);     \
		char *a = (A);       \
		char *b = (B);       \
		do                   \
		{                    \
			char _temp = *a; \
			*a++ = *b;       \
			*b++ = _temp;    \
		} while (--sz);      \
	} while (0)

// --------------------------------- PROTOTYPES --------------------------------

// Signatures
typedef int(compare_t)(const void *, const void *);
typedef int(verify_t)(data_t *, int, int, int);


// Inline functions declarations
static inline compare_t compare;		// compare function
static inline compare_t compare_ge; 	// compare for "greater or equal"
static inline compare_t compare_double; // compare for double
verify_t verify_partitioning;			// verify partitioning
int verify_sorting(data_t *, int, int); // verify sorting
void show_array(data_t *, int, int); 	// print the array
void show_int_array(int *, int, int); 	// print the int array
void show_double_array(double *, int, int); 	// print the double array

// Utility functions
double mean(double *, int);			    // average of an array
double min(double *, int);	         	// minimum of an array
double max(double *, int);				// maximum of an array
double stdev(double *, int);			// standard deviation of an array

// Partitioning functions
static inline int partitioning(data_t *, int, int, compare_t);
static inline int partitioning_low_high(data_t *, int, int, double);
static inline int binary_search(data_t*, int, int, double);
static inline int* p_partitioning(data_t *, int, int, double *, int);

// Splitting function
static inline chunk_t split(int, int, int, int);

// Merging function
static inline data_t* merge(data_t *, int, int, data_t *, int, int);

// Data generation
void generate_data(data_t **, int); 	// generate random data


// Sorting functions declaration (serial and parallel)

// Serial quicksort function
void serial_qsort(data_t *, int, int, compare_t);

// OpenMP quicksort functions
#if defined(_OPENMP)
	// Tasks parallel quicksort function
	void omp_task_qsort(data_t *, int, int, compare_t);

	// Parallel quicksort function
	void omp_parallel_qsort(data_t *, int, int, compare_t, int);

	// Hyperquicksort function
	void omp_hyperquicksort(data_t *, int, int, compare_t, int);

	// Parallel Sort by Regular Sampling (PSRS) function
	void omp_psrs(data_t *, int, int, compare_t);

#endif

// MPI quicksort functions
#if defined(MPI_VERSION)

	// Parallel quicksort function (MPI)
	void MPI_Parallel_qsort(data_t **, int *, int *, int, MPI_Comm, MPI_Datatype, compare_t);

	// Hyperquicksort function (MPI)
	void MPI_Hyperquicksort(data_t **, int *, int *, int, MPI_Comm, MPI_Datatype, compare_t);

	// Parallel Sort by Regular Sampling (PSRS) function (MPI)
	void MPI_PSRS(data_t **, int *, int, int, int, MPI_Datatype, compare_t);

	// Splitting function (MPI)
	void MPI_Split(data_t *, int, data_t **, int *, int, int, MPI_Datatype);

	// Merging function (MPI)
	void MPI_Merge(data_t *, data_t *, int , int, int, MPI_Datatype);

#endif

// ----------------------------- INLINE FUNCTIONS ------------------------------

// Partitionin functions
inline int partitioning(data_t *data, int start, int end, compare_t cmp_ge)
{
	// Here end is past the last element
	--end;

	// Swap the elements so that the meadian of [starts], [mid] and [end]
	// end up in the end place and is later picked as pivot
	int mid = start + (end - start) / 2;
	if (cmp_ge((void *)&data[start], (void *)&data[mid]))
		SWAP((void *)&data[start], (void *)&data[mid], sizeof(data_t));
	if (cmp_ge((void *)&data[start], (void *)&data[end]))
		SWAP((void *)&data[start], (void *)&data[end], sizeof(data_t));
	if (cmp_ge((void *)&data[end], (void *)&data[mid]))
		SWAP((void *)&data[mid], (void *)&data[end], sizeof(data_t));

	// The last element is the pivot
	void *pivot = (void *)&data[end];

	// Pointbreak is the index of the semi-last element
	int pointbreak = end - 1;

	for (int i = start; i <= pointbreak; i++) {
		// If the element is greater than or equal to the pivot
		if (cmp_ge((void *)&data[i], pivot)) {
			// Find the first element from the end that is less than the pivot
			while ((pointbreak > i) && cmp_ge((void *)&data[pointbreak], pivot))
				pointbreak--;

			// Swap the elements
			if (pointbreak > i)
				SWAP((void *)&data[i], (void *)&data[pointbreak--], sizeof(data_t));
		}
	}

	// In the end we need to adjust the position of the pivot
	pointbreak += !cmp_ge((void *)&data[pointbreak], pivot);
	SWAP((void *)&data[pointbreak], pivot, sizeof(data_t));

	return pointbreak;
}

inline int partitioning_low_high(data_t *data, int start, int end, double threshold) {
	// The idea is to maintain two pointers: one starting from the left (low) and moving towards right,
	// and the other starting from the right (high) and moving towards left. The left pointer finds an element
	// greater than or equal to the pivot (threshold), and the right pointer finds one less than it, then they swap elements.
	// This continues until the pointers cross

	int low = start - 1;
	int high = end;

	while (1) {
		// Find the first element greater than or equal to the pivot
		do {
			++low;
		} while (low < high && data[low].data[HOT] < threshold);

		// Find the first element less than the pivot
		do {
			--high;
		} while (high > low && data[high].data[HOT] >= threshold);

		// If the pointers cross, the partitioning is done.
		// It returns the index position of the first element that is 
		// greater than or equal to the pivot, which is "low" when the function stops
		if (low >= high)
			return low;

		// Swap the elements
		SWAP((void *)&data[low], (void *)&data[high], sizeof(data_t));
	}
}

inline int* p_partitioning(data_t *data, int start, int end, double *pivots, int p) {
	// Partitioning function that, given a data_t*, a start, a end, a compare_t and an array of p pivots,
	// returns an array of p+1 integers such that the first index is 0 and the i-th integer is the index 
	// of the first element in the array that is greater or equal to the i-th pivot
	int *indexes = (int *)malloc((p+1) * sizeof(int));

	// Finding the indexes (with a data array already sorted)
	indexes[0] = 0;
	for (int i = 0; i < p; i++) {
		indexes[i+1] = binary_search(data, start, end, pivots[i]) - start;
	}

	return indexes;
}

inline int binary_search(data_t *data, int start, int end, double threshold) {
    int result = -1;
	while (start <= end) {
		int mid = start + (end - start) / 2;
		if (data[mid].data[HOT] >= threshold) {
			result = mid;
			end = mid - 1;
		} else {
			start = mid + 1;
		}
	}
    return result;
}

// Comparing functions
inline int compare(const void *A, const void *B) {
	data_t *a = (data_t *)A;
	data_t *b = (data_t *)B;
	double diff = a->data[HOT] - b->data[HOT];
	return ((diff > 0) - (diff < 0));
}

inline int compare_ge(const void *A, const void *B) {
	data_t *a = (data_t *)A;
	data_t *b = (data_t *)B;
	return (a->data[HOT] >= b->data[HOT]);
}

inline int compare_double(const void* a, const void* b) {
    double arg1 = *(const double*) a;
    double arg2 = *(const double*) b;
    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

// Split / Merge functions

// Splits an array from start to end in p chunks depending on the id of the process
inline chunk_t split(int start, int end, int p, int id) {
	int size = (end - start) / p;
	int remainder = (end - start) % p;
	chunk_t chunk;
	chunk.start = start + id * size + (id < remainder ? id : remainder);
	chunk.end = chunk.start + size - (id < remainder ? 0 : 1);
	chunk.size = 1 + chunk.end - chunk.start;
	return chunk;
}

// Merges two arraya a from start_a to end_a and b from start_b to end_b into a single one
static inline data_t* merge(data_t *a, int start_a, int end_a, data_t *b, int start_b, int end_b) {
	int size_a = end_a - start_a;
    int size_b = end_b - start_b;
    data_t *merged = (data_t *)malloc((size_a + size_b) * sizeof(data_t));
    memcpy(merged, a + start_a, size_a * sizeof(data_t));
    memcpy(merged + size_a, b + start_b, size_b * sizeof(data_t));
    return merged;
}

#endif // QSORT_H__