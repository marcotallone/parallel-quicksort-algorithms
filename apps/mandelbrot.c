/*
┌────────────────────────────────────────────────────────────────────────────┐
│ Program to generate a Mandelbrot set image                                 │
└────────────────────────────────────────────────────────────────────────────┘
*/

#if defined(__STDC__)
#if (__STDC_VERSION__ >= 199901L)
#define _XOPEN_SOURCE 700
#endif
#endif
#define T_dflt 3

// --------------------------------- INCLUDES ----------------------------------

#include "mandel.h"

// --------------------------------- MAIN --------------------------------------

int main(int argc, char *argv[]) {

    // Variables                                                // ┌──────────┐ 
    // Command line variables                                   // │ Inputs:  │
                                                                // ├──────────┤
    const int n_x    = (argc > 1) ? atoi(argv[1]) : W_dflt;     // │ x-pixels │
    const int n_y    = (argc > 2) ? atoi(argv[2]) : H_dflt;     // │ y-pixels │
    const double x_l = (argc > 3) ? atof(argv[3]) : Xmin_dflt;  // │ x-min    │
    const double y_l = (argc > 4) ? atof(argv[4]) : Ymin_dflt;  // │ y-min    │
    const double x_r = (argc > 5) ? atof(argv[5]) : Xmax_dflt;  // │ x-max    │
    const double y_r = (argc > 6) ? atof(argv[6]) : Ymax_dflt;  // │ y-max    │
    const int I_max  = (argc > 7) ? atoi(argv[7]) : Imax_dflt;  // │ max iter │
    const int trials = (argc > 8) ? atoi(argv[8]) : T_dflt;     // │ trials   │
                                                                // └──────────┘
                                                                // ┌──────────┐
    // Other variables                                          // │ Others:  │
                                                                // ├──────────┤
    unsigned short int **M = NULL;                              // │ Image    │
    const double delta_x = (x_r - x_l) / n_x;                   // │ x-step   │
    const double delta_y = (y_r - y_l) / n_y;                   // │ y-step   │
    struct timespec ts;                                         // │ timeStr. │
    double *times = (double *)malloc(trials * sizeof(double));  // │ times    │
    double timer = 0.0;                                         // │ timer    │
    int nthreads = 1;                                           // │ threads  │
                                                                // └──────────┘

    // Open a csv file to store the times --------------------------------------
    FILE *file = fopen("datasets/mandelbrot_scaling.csv", "a+");
    if (file != NULL) {
        // Move to the end of the file
        fseek(file, 0, SEEK_END);

        // If the file position indicator is 0, the file was just created
        if (ftell(file) == 0) {
            fprintf(file, "\"Threads\",\"Max Iterations\",\"Width\",\"Height\","
                          "\"Average time (s)\",\"St. Deviation (s)\","
                          "\"Min (s)\",\"Max (s)\"\n");
            fflush(file);
        }
    }

    // Measure nthreads
    #pragma omp parallel
    #pragma omp master
    nthreads = omp_get_max_threads();

    // Work by row horizontally ------------------------------------------------
    
   for (int t = 0; t < trials; t++) {
        M = malloc(n_y * sizeof(unsigned short int *));
        for (int i = 0; i < n_y; i++) {
            M[i] = malloc(n_x * sizeof(unsigned short int));
        }

        // Timer
        timer = CPU_TIME;

        // Calculate the number of iterations for each pixel
        #pragma omp parallel for schedule(dynamic, 1)
        for (int i = 0; i < n_y; i++) {
            const complex double im_c = (y_l + i * delta_y) * I;
            for (int j = 0; j < n_x; j++) {
                complex double c = (x_l + j * delta_x) + im_c;
                M[i][j] = mandelbrot(c, I_max);
            }
        }
        times[t] = CPU_TIME - timer;

        // Write the image to a pgm file (uncomment to execute)
        printf("Time horizontally: %f\n", timer);
        fprintf(stdout, "Time horizontally: %f\n", timer);
        fflush(stdout);
        write_pgm_image(M, I_max, n_x, n_y, "mandelbrot_h.pgm");
        
        // Free memory
        for (int i = 0; i < n_y; i++) {
            free(M[i]);
        }
        free(M);
   }

    // Write the times to the csv file
    fprintf(file, "%d,%d,%d,%d,%.6f,%.6f,%.6f,%.6f\n", 
            nthreads, I_max, n_x, n_y, 
            mean(times, trials), 
            stdev(times, trials), 
            min(times, trials), 
            max(times, trials));
    fflush(file);
    fclose(file);

    // Free memory
    free(times);

    return 0;
}
