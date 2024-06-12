/*
┌────────────────────────────────────────────────────────────────────────────┐
│ Simple script to display the results of the scaling analysis for the qsort │
│ algorithms implemented in OpenMP and MPI in a nice way.                    │   
│ Note: Before running this script colect the data using the `omp_scaling.c` │
│ and `mpi_scaling.c` scripts. Then run this script from the root directory. │
│ More info on the README.md file.                                           │
└────────────────────────────────────────────────────────────────────────────┘
*/

#if defined(__STDC__)
#if (__STDC_VERSION__ >= 199901L)
#define _XOPEN_SOURCE 700
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    char *implementation = (argc > 1) ? argv[1] : "mpi";
    char method[32] = "";
    char sorted[32] = "";
    int processes = 0;
    int threads = 0;
    int elements = 0;
    double elements_sci = 0.0;
    double average_time = 0.0;
    double st_dev = 0.0;
    double min = 0.0;
    double max = 0.0;

    if (strcmp(implementation, "mpi") == 0) {

        // Print table header
        fprintf(stdout, "┌──────────────────────┐\n");
        fprintf(stdout, "│ MPI Scaling Analysis │\n");
        fprintf(stdout, "├────────┬────────┬────┴──────┬─────────┬────────────┬────────────────┬──────────────────┬──────────────┬─────────┬─────────┐\n");
        fprintf(stdout, "│ Method │ Sorted │ Processes │ Threads │  Elements  │ Elements (Sci) │ Average time (s) │ St. Dev. (s) │ Min (s) │ Max (s) │\n");
        fprintf(stdout, "├────────┼────────┼───────────┼─────────┼────────────┼────────────────┼──────────────────┼──────────────┼─────────┼─────────┤\n");

        // Open the csv file
        FILE *file = fopen("datasets/mpi_scaling.csv", "r");

        // If the file does not exist
        if (file == NULL) {
            fprintf(stderr, "ERROR: mpi_scaling.csv file not found.\n");
            fprintf(stderr, "       Try running this script from the root directory.\n");
            return 1;
        }

        // Line buffer
        char line[1024];

        // Skip the first line
        fgets(line, 1024, file);

        // Parse the csv file
        while (fgets(line, 1024, file)) {
            sscanf(line, "%[^,],%[^,],%d,%d,%d,%lf,%lf,%lf,%lf,%lf", method, sorted, &processes, &threads, &elements, &elements_sci, &average_time, &st_dev, &min, &max);
            fprintf(stdout, "│ %-6s │   %-4s │ %6d    │ %4d    │ %10d │ %10.0e     │ %12.4f     │ %10.4f   │ %7.4f │ %7.4f │\n",
                    method, sorted, processes, threads, elements, elements_sci, average_time, st_dev, min, max);
        }

        // Close the file
        fclose(file);

        // Print table footer
        fprintf(stdout, "└────────┴────────┴───────────┴─────────┴────────────┴────────────────┴──────────────────┴──────────────┴─────────┴─────────┘\n");

    } else if (strcmp(implementation, "omp") == 0) {

        // Print table header
        fprintf(stdout, "┌─────────────────────────┐\n");
        fprintf(stdout, "│ OpenMP Scaling Analysis │\n");
        fprintf(stdout, "├────────┬────────┬───────┴──┬────────────┬────────────────┬──────────────────┬──────────────┬─────────┬─────────┐\n");
        fprintf(stdout, "│ Method │ Sorted │  Threads │  Elements  │ Elements (Sci) │ Average time (s) │ St. Dev. (s) │ Min (s) │ Max (s) │\n");
        fprintf(stdout, "├────────┼────────┼──────────┼────────────┼────────────────┼──────────────────┼──────────────┼─────────┼─────────┤\n");

        // Open the csv file
        FILE *file = fopen("datasets/omp_scaling.csv", "r");

        // If the file does not exist
        if (file == NULL) {
            fprintf(stderr, "ERROR: omp_scaling.csv file not found.\n");
            fprintf(stderr, "       Try running this script from the root directory.\n");
            return 1;
        }

        // Line buffer
        char line[1024];

        // Skip the first line
        fgets(line, 1024, file);

        // Parse the csv file
        while (fgets(line, 1024, file)) {
            sscanf(line, "%[^,],%[^,],%d,%d,%lf,%lf,%lf,%lf,%lf", method, sorted, &threads, &elements, &elements_sci, &average_time, &st_dev, &min, &max);
            fprintf(stdout, "│ %-6s │   %-4s │ %5d    │ %10d │ %10.0e     │ %12.4f     │ %10.4f   │ %7.4f │ %7.4f │\n",
                    method, sorted, threads, elements, elements_sci, average_time, st_dev, min, max);
        }

        // Close the file
        fclose(file);

        // Print table footer
        fprintf(stdout, "└────────┴────────┴──────────┴────────────┴────────────────┴──────────────────┴──────────────┴─────────┴─────────┘\n");
    } else {
        fprintf(stderr, "ERROR: Invalid implementation. Use 'mpi' or 'omp'.\n");
        return 1;
    }

    return 0;
}