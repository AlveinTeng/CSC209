#include <stdio.h>
#include <stdlib.h> 

#include "point.h"


int main(int argc, char *argv[]) {
    struct Point *p;
    FILE *f;
    int n, bytes_written;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s filename total_points\n", argv[0]);
        exit(1);
    }

    n = strtol(argv[2], NULL, 10);
    p = malloc(n * sizeof(struct Point));
    if (p == NULL) {
        perror("malloc");
        exit(1);
    }

    printf("Generating %d random points with x- and y-coordinate range between %d and %d...\n", n, 0, RAND_MAX);
    for (int i = 0; i < n; i++) {
        p[i].x = rand();
        p[i].y = rand();
        // printf("P[%d] = (%d, %d)\n", i, p[i].x, p[i].y);
    }

    f = fopen(argv[1], "wb");
    if (f == NULL) {
        perror(argv[1]);
        exit(1);
    }

    bytes_written = fwrite(&n, sizeof(n), 1, f);
    if (bytes_written != 1) {
        fprintf(stderr, "Error writing n to data file.\n");
        exit(1);
    }
    bytes_written = fwrite(p, sizeof(struct Point), n, f);
    if (bytes_written != n) {
        fprintf(stderr, "Error writing Point structs to data file.\n");
        exit(1);
    }

    if (fclose(f)) {
        fprintf(stderr, "Error closing data file.\n");
        exit(1);
    }

    printf("Done.\n");
    return 0;
}
