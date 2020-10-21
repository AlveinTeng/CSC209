/*
 * A C implementation of the divide and conquer algorithm for the closest pair of points problem.
 *
 * Single-process solution borrowed with slight modifications from:
 * https://www.geeksforgeeks.org/closest-pair-of-points-using-divide-and-conquer-algorithm/
 */

#include <stdio.h>
#include <stdlib.h>

#include "point.h"
#include "utilities_closest.h"


/*
 * Recursive divide-and-conquer implementation to find the minimal distance
 * between any two pair of points in array p. Assumes that the array P[] is sorted
 * according to x coordinate.
 */
double closest_serial(struct Point *p, int n) {
    // If there are 2 or 3 points, then use brute force.
    if (n <= 3)
        return brute_force(p, n);

    // Find the middle point.
    int mid = n / 2;
    struct Point mid_point = p[mid];

    /*
     * Consider the vertical line passing through the middle point;
     * calculate the smallest distance dl on left of middle point and
     * dr on right side.
     */
    double dl = closest_serial(p, mid);
    double dr = closest_serial(p + mid, n - mid);

    // Find the smaller of two distances 
    double d = min(dl, dr);

    // Build an array strip[] that contains points close (closer than d) to the line passing through the middle point.
    struct Point *strip = malloc(sizeof(struct Point) * n);
    if (strip == NULL) {
        perror("malloc");
        exit(1);
    }

    int j = 0;
    for (int i = 0; i < n; i++) {
        if (abs(p[i].x - mid_point.x) < d) {
            strip[j] = p[i], j++;
        }
    }

    // Find the closest points in strip.  Return the minimum of d and closest distance in strip[].
    double new_min = min(d, strip_closest(strip, j, d));
    free(strip);

    return new_min;
}
