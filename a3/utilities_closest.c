#include <stdio.h>
#include <float.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "point.h"


/*
 * The following two functions are needed for library function qsort().
 * Refer: http://www.cplusplus.com/reference/clibrary/cstdlib/qsort/
 */

// Needed to sort array of points according to X coordinate.
int compare_x(const void *a, const void *b) {
    struct Point *p1 = (struct Point *)a;
    struct Point *p2 = (struct Point *)b;
    return p1->x - p2->x;
}

// Needed to sort array of points according to Y coordinate.
int compare_y(const void* a, const void* b) {
    struct Point *p1 = (struct Point *)a;
    struct Point *p2 = (struct Point *)b;
    return p1->y - p2->y;
}

// A utility function to find the distance between two points.
double dist(struct Point p1, struct Point p2) {
    /*
     * Cast the values from int to long to avoid signed integer overflow when
     * multiplying these numbers (-fsanitize=undefined was useful for catching this).
     * RAND_MAX seems to generally be set to INT_MAX. So as long as LONG_MAX is greater
     * than 2*INT_MAX^2 we are safe.
     * -Furkan
     */
    return sqrt(((long) p1.x - (long) p2.x) * ((long) p1.x - (long) p2.x) +
                ((long) p1.y - (long) p2.y) * ((long) p1.y - (long) p2.y));
}

/*
 * Brute Force method to find minimal distance between two points in an
 * array p of size n.
 */
double brute_force(struct Point *p, int n) {
    double min = DBL_MAX;

    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (dist(p[i], p[j]) < min) {
                min = dist(p[i], p[j]);
            }
        }
    }

    return min;
}

// Return smallest of two double values 
double min(double x, double y) {
    return (x < y) ? x : y;
}

/*
 * Find the distance between the closest points of array strip of size d.
 * All points in array strip are sorted according to y coordinate. They all
 * have an upper bound on minimum distance as d. Note that this method
 * seems to be a O(n^2) method, but it's a O(n) method as the inner loop
 * runs at most 6 times.
 */
double strip_closest(struct Point *strip, int size, double d) {
    double min = d;  // Initialize the minimum distance as d

    qsort(strip, size, sizeof(struct Point), compare_y);

    /*
     * Pick all points one by one and try the next points until the difference
     * between y coordinates is smaller than d. This is a proven fact that this
     * loop runs at most 6 times.
     */
    for (int i = 0; i < size; ++i) {
        for (int j = i + 1; j < size && (strip[j].y - strip[i].y) < min; ++j) {
            if (dist(strip[i], strip[j]) < min) {
                min = dist(strip[i], strip[j]);
            }
        }
    }

    return min;
}

/*
 * Return the total number of points stored in the specified file.
 */
int total_points(char *f_name) {
    struct stat st_buf;

    if (stat(f_name, &st_buf) != 0) {
        perror("stat");
        exit(1);
    }

    // Make sure that the file has a valid size.
    if ((st_buf.st_size - sizeof(int)) % sizeof(struct Point) != 0) {
        fprintf(stderr, "The size of the specified file is invalid!\n");
        exit(1);
    }

    return (st_buf.st_size - sizeof(int)) / sizeof(struct Point);
}

/*
 * Return all input points from the specified file and populate *n with the
 * number of points read.
 */
void read_points(char *f_name, struct Point *points_arr) {
    int total, bytes_read;
    FILE *fp;

    fp = fopen(f_name, "rb");
    if (fp == NULL) {
        perror(f_name);
        exit(1);
    }

    bytes_read = fread(&total, sizeof(total), 1, fp);
    if (bytes_read == 0 || total < 0) {
        fprintf(stderr, "Failed to read number of points from %s.\n", f_name);
        exit(1);
    }

    bytes_read = fread(points_arr, sizeof(struct Point), total, fp);
    if (bytes_read != total) {
        fprintf(stderr, "Error reading %s.\n", f_name);
        exit(1);
    }

    if (fclose(fp)) {
        fprintf(stderr, "Error closing %s.\n", f_name);
        exit(1);
    }
}
