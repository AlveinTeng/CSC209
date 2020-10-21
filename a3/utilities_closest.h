#ifndef _UTILITIES_H
#define _UTILITIES_H

// Needed to sort array of points according to X coordinate.
int compare_x(const void *a, const void *b);

// Needed to sort array of points according to Y coordinate.
int compare_y(const void* a, const void* b);

// A utility function to find the distance between two points.
double dist(struct Point p1, struct Point p2);

/*
 * Brute Force method to find minimal distance between two points in an
 * array p of size n.
 */
double brute_force(struct Point *p, int n);

// Return smallest of two double values 
double min(double x, double y);

/*
 * Find the distance between the closest points of array strip of size d.
 * All points in array strip are sorted according to y coordinate. They all
 * have an upper bound on minimum distance as d. Note that this method
 * seems to be a O(n^2) method, but it's a O(n) method as the inner loop
 * runs at most 6 times.
 */
double strip_closest(struct Point *strip, int size, double d);

/*
 * Return the total number of points stored in the specified file.
 */
int total_points(char *f_name);

/*
 * Return all input points from the specified file and populate *n with the
 * number of points read.
 */
void read_points(char *f_name, struct Point *points_arr);

#endif /* _UTILITIES_H */
