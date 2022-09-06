#include "utils.h"

/*returns **squared** distance between two vectors */
double distance_sq(double *vector, double *other_vector, int d){
    double res = 0;
    int i = 0;
    for (i = 0; i < d; i++){
        res+= (vector[i] - other_vector[i]) * (vector[i] - other_vector[i]);
    }
    return res;
}

/*returns the index of closest centroid in centroids to vector */
int  closest_clust(double *vector, double **centroids, int k, int d){
    double curr_min = 0;
    int curr_res = 0;
    int i = 0;
    curr_min = distance_sq(vector, centroids[0], d);

    for (i = 1; i < k; i++){
        double dist = distance_sq(vector, centroids[i], d);
        if (dist < curr_min){
            curr_min = dist;
            curr_res = i;
        }
    }
    return curr_res;
}

/* change to_vector into sum of vector and to_vector */
void add_vec(double *vector, double *to_vector, int d){
    int i = 0;
    for (i = 0; i < d; i++){
        to_vector[i] += vector[i];
    }
    return;
}


/* adds vector to cumulative sum in array sums and increases num of vectors by 1 in clust_sizes array */
void assign_vec(double *vector, double **clust_sums,int *sizes, int index, int d){
    sizes[index]= sizes[index]+ 1;
    add_vec(vector, clust_sums[index], d);
    return;
}

/*calculate the mean vector for all vectors currently in the cluster */
double *calc_mean(double *sum_vector, int num_vectors, int d){
    double *result = calloc(d, sizeof (double ));
    int i = 0;

    for(i = 0; i < d; i++){
        if(num_vectors==0){
            print_error();
            return NULL;
        }
        result[i] = sum_vector[i]/ num_vectors;
    }
    return result;
}

/* checks whether the distance between to vectors is less than constant epsilon */
int is_converged(double *new_centroid, double *old_centroid, int d, double epsilon){
    double dist = 0;
    dist = distance_sq(new_centroid, old_centroid, d);
    return (dist < epsilon*epsilon);
}

/* resets the table of cumulative clusters sums and size to zero */
void reset(double **sums, int *sizes, int k, int d){
    int i = 0;
    int j = 0;

    for (i = 0; i < k; i++){
        sizes[i] = 0;
        for (j = 0; j < d; j++){
            sums[i][j] = 0;
        }
    }
}

int isStrNumeric(char *str){
    int i = 0;
    if (str== NULL || str[0] == 0){
        return False;
    }
    if (str[0] == '0' && (str[1] != 0)){
        return False;
    }
    for (i = 0; str[i] != 0; i++){
        if (str[i] < '0' || str[i]> '9'){
            return False;
        }
    }
    return True;
}


int kmeans(Matrix vectors, int k, int N, int d) {
    int closest_clust_index = 0;
    int i = 0;
    int j = 0;
    int converged = False;
    double **centroids = NULL;
    double **clust_sums = NULL;
    int *clust_sizes = NULL;
    double *new_centroid = NULL;
    char ***centroids_strs = NULL;


    centroids= (double**)calloc(k, (sizeof (double*)));
    if(centroids == NULL){
        print_error();
        return 1;
    }
    for (i = 0; i < k; i++){
        centroids[i] = (double*)calloc(d, (sizeof (double )));
        if(centroids[i] == NULL){
            free_memory_arr(centroids, i-1);
            print_error();
            return 1;
        }
    }

    /* init first vectors from kmeans ++ for clustering ALG */
    for (i = 0; i < k; i++){
        for (j = 0; j < d; j++){
            centroids[i][j] = vectors[i][j];
        }
    }
    clust_sums= (double**)calloc(k,(sizeof (double*)));
    if(clust_sums == NULL){
        free_memory_arr(centroids, k);
        print_error();
        return 1;
    }

    for (i = 0; i < k; i++){
        clust_sums[i] = (double*)calloc(d, (sizeof (double )));
        if(clust_sums[i] == NULL){
            free_memory_arr(centroids, k);
            free_memory_arr(clust_sums, i-1);
            print_error();
            return 1;
        }
    }
    clust_sizes= (int*)calloc(k, (sizeof (int)));
    if(clust_sizes == NULL){
        free_memory_arr(centroids, k);
        free_memory_arr(clust_sums, k);
        print_error();
        return 1;
    }
    if (vectors == NULL || centroids == NULL || clust_sums == NULL || clust_sizes == NULL){
        print_error();
        return 1;
    }

    /* main loop: */
    for(i = 0; (i < MAX_ITER) && !converged; i++){
        converged = True;
        for (j = 0; j < N; j++){
            closest_clust_index = closest_clust(vectors[j], centroids, k, d);
            assign_vec(vectors[j], clust_sums, clust_sizes, closest_clust_index, d);
        }

        for (j = 0; j < k; j++){
            new_centroid = calc_mean(clust_sums[j], clust_sizes[j], d);

            if(new_centroid == NULL){
                free_memory_arr(centroids, k);
                free_memory_arr(clust_sums, k);
                free(clust_sizes);
                return 1;
            }

            if(!is_converged(new_centroid, centroids[j], d, 0)){
                converged = False;
            }
            centroids[j] = new_centroid;
        }

        reset(clust_sums, clust_sizes, k, d);
    }

    centroids_strs = doubleVecsToStr(centroids, d,k);
    if(centroids_strs == NULL){
        free_memory_arr(centroids, k);
        free_memory_arr(clust_sums, k);
        free(clust_sizes);
        return 1;
    }

    writeData("c_output_file.txt", centroids_strs, d, k);

    for (i = 0; i < k; i++){
        free(centroids[i]);
        free(clust_sums[i]);
    }
    free(centroids);
    free(clust_sums);
    free(clust_sizes);
    return 0;
}
