#ifndef PROJECT_SPKMEANS_H
#define PROJECT_SPKMEANS_H

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

typedef struct {
    int cluster;
    double *args;
} vector;
/* an object of this type represents a datapoint that the algorithm operates on
 * it's defined by an array of the datapoint arguments and an integer that determines the cluster
 * it's currently in
 * it can also represent a centroid */

typedef struct {
    double lambda;
    double *args;
} eigen_vector;
/* an object of this type represents an eigen vector
 * with lambda as its eigen value and args as its arguments */

typedef enum {
    wam,
    ddg,
    lnorm,
    jacobi,
    dflt
} GOAL;

/* declarations */

GOAL enumerate_goal(char *goal_name);

int extract_n(FILE *inp);

int extract_d(FILE *inp);

int vector_reader(FILE *inp, vector *datapoints, int d, int n);

vector *organize(char *file_name, int *n, int *d);

void free_double(double ***values, int len);

void free_vectors(vector **arr, int count);

void free_eigens(eigen_vector **arr, int count);

void print_matrix(double **mat, int n);

void print_values(double **mat, int n);

void print_jacobi(eigen_vector *j_mat, int n, int d);

double **multiply(double **a_mat, double **b_mat, int n, int m);

int sum_sub(vector *sumsub, vector v1, vector v2, char *sign, int d);

void divide(vector *quo, vector v, int divisor, int d);

double euc_norm(vector v, int d);

double **find_w(vector *datapoints, int n, int d);

double **find_d(double **w_mat, int n);

double **find_l(vector *datapoints, int n, int d);

double **find_p(double **a_mat, int i, int j, int n);

eigen_vector *find_j(double **mat, int n);

double **find_t(eigen_vector *eigens, int n, int k);

double **extract_mat_from_dp(vector *datapoints, int n, int d);

double **extract_values(eigen_vector *eigens, int n);

double **extract_mat_from_vectors(eigen_vector *eigens, int n, int d);

double off(double **a_mat, int n);

void find_pivot(double **a_mat, int *p1, int *p2, int n);

void rotate(double **a_mat, double **p_mat, int i, int j, int n);

void normalize(double **mat, int n, int k);

void terminate(vector *centroids_prev, int *counts, vector *sums, int k);

int eigen_comparator(const void *p1, const void *p2);

int k_heuristic(eigen_vector *eigens, int n);

double **fit_spk_c(vector *datapoints, int *k, int n, int d);

double **k_means(vector *datapoints, int k, int max_iter, double epsilon, int n, int d, vector *centroids);

int convergence(vector *centroids, vector *centroids_prev, int iter, int k, int d, double epsilon);

int change_cluster(vector *v, vector const *centroids, int k, int d);

int prev(vector *centroid_prev, vector centroid, int d);

int invalid();

int error();

#endif

