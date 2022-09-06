#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DEFAULT_MAX_ITER 100
#define EPSILON 0.001
#define True 1
#define False 0
#define DEBUG 0

void print_error(){
    printf("An Error Has Occurred\n");
}

void print_invalid_input(){
    printf("Invalid Input!\n");
}

void free_memory_arr(double** arr, int n){
    int i = 0;
    for(i = 0; i < n; i++){
        free(arr[i]);
    }
    free(arr);
}

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
int is_converged(double *new_centroid, double *old_centroid, int d){
    double dist = 0;
    dist = distance_sq(new_centroid, old_centroid, d);
    return (dist < EPSILON*EPSILON);
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

int getProgramArgs(int argc, char *argv[], int *k, int *max_iter, char **in_file_path, char **out_file_path){
    int max_iter_provided = 0;

    if (argc < 4 || argc > 5){
        print_invalid_input();
        return False;
    }
    if (!isStrNumeric(argv[1])){
        print_invalid_input();
        return False;
    }
    *k = atoi(argv[1]);
    max_iter_provided = (argc== 5);
    if (max_iter_provided && !isStrNumeric(argv[2])){
        print_invalid_input();
        return False;
    }
    *max_iter = max_iter_provided ? atoi(argv[2]) : DEFAULT_MAX_ITER;
    *in_file_path = argv[2 + max_iter_provided];
    *out_file_path = argv[3 + max_iter_provided];
    return True;
}
/*
 * This function gets a string of filename and returns the number of chars in the first line
 */
int getLineSize(char *filename){
    FILE *fp = NULL;
    int count = 0;
    char c = 'a';

    fp = fopen(filename, "r");
    if (fp == NULL){
        if (DEBUG ){
            printf("in 'getLineSize', fp is null");
        }
        print_error();
        return False;
    }

    c = fgetc(fp);
    while (c != EOF && c!= '\n'){
        c = fgetc(fp);
        count++;
    }
    fclose(fp);
    return count;
}

int getNumOfLines(char *filename){
    FILE *fp = NULL;
    int count = 0;
    int c = 0;

    fp = fopen(filename, "r");
    if (fp == NULL){
        if (DEBUG == 1){
            printf("in 'getNumOfLines', fp is null\n");
        }
        print_error();
        return False;
    }

    while ( ( c = fgetc( fp ) ) != EOF )
    {
        count+= (c == '\n'); /* check if c equals '\n' */
    }
    fclose(fp);
    return count;
}

int *getLinesLengths(int num_lines, char *filename){
    int *linesLengths = NULL;
    FILE *fp = NULL;
    int i = 0;
    char c = 0;
    int line_len = 0;

    linesLengths = (int *)calloc(num_lines,sizeof(int));
    fp = fopen(filename, "r");
    if (fp == NULL){

        if (DEBUG == 1){
            printf("in 'getLinesLengths', fp is null\n");
        }
        print_error();
        return NULL;
    }
    if (linesLengths == NULL){
        if (DEBUG == 1){
            printf("in 'getLinesLengths', linesLengths is null\n");
        }
        fclose(fp);
        print_error();
        return NULL;
    }
    for(i = 0; i < num_lines; i++){
        c = 0;
        line_len = 1;
        while((c = fgetc(fp)) != EOF){
            if (c == '\n'){
                linesLengths[i] = line_len;
                break;
            }
            else{ line_len++;}
        }
    }
    fclose(fp);
    return linesLengths;
}

/* returns number of numbers in first line of file */
int get_dimension(char *in_file_path, int line_size){
    FILE *fp = NULL;
    char *line = NULL;
    int counter = 1;
    int i = 0;

    fp = fopen(in_file_path, "r");
    if (fp == NULL){
        if (DEBUG == 1){
            printf("in 'get_dimension', fp is null\n");
        }
        print_error();
        return False;
    }
    line = (char  *) calloc(line_size, sizeof (char));
    if (line == NULL){
        if (DEBUG){
            printf("in 'get_dimension', line is null\n");
        }
        print_error();
        fclose(fp);
        return False;
    }
    if(fgets(line, line_size, fp) == NULL){
        fclose(fp);
        if (DEBUG == 1){
            printf("in 'get_dimension', fgets == null\n");
        }
        print_invalid_input();
        free(line);
        return False;
    }

    for(i = 0; line[i] != '\0'; i++ ){
        if(line[i] == ','){
            counter++;
        }
    }
    fclose(fp);
    free(line);
    return counter;
}

/* turns string of comma seperated numbers to array of doubles */
double *str_to_vec(char *line,int line_len, int d){
    double *res = NULL;
    char *line_copy = NULL;
    const char s[2] = ",";
    char *token = NULL;
    int i = 0;


    res = (double *) calloc(d, sizeof(double));
    line_copy = (char *) calloc(line_len+1, sizeof(char ));
    strcpy(line_copy, line);
    if(res == NULL){
        if (DEBUG == 1){
            printf("in 'str_to_vec', res is null\n");
        }
        print_error();
        return NULL;
    }


    token = strtok(line_copy, s);
    while( token != NULL ) {
        res[i] = atof(token);
        i++;
        token = strtok(NULL, s);
    }
    free(line_copy);
    return res;
}

int readData(char *in_file_path, double ***vectors, int *N, int *d){
    int line_size = 0;
    int i = 0;
    char *line = NULL;
    int *lines_length = NULL;
    FILE *fp = NULL;
    double *vec = NULL;


    line_size = getLineSize(in_file_path);
    *N = getNumOfLines(in_file_path);
    *vectors = (double **)calloc(*N, sizeof(double  *));
    *d = get_dimension(in_file_path, line_size);
    if( *N == 0 || line_size == 0 || *d == 0){
        if (DEBUG == 1){
            printf("in 'readData', invalid input\n");
        }
        print_invalid_input();
        return False;
    }
    if (*vectors == NULL){
        if (DEBUG == 1){
            printf("in 'readData', *vectors == NULL");
        }
        print_error();
        return False;
    }
    line = (char  *) calloc(line_size + 1, sizeof (char));
    if (line == NULL){
        if (DEBUG){
            printf("in 'get_dimension', line is null\n");
        }
        print_error();
        return False;
    }
    lines_length = getLinesLengths(*N, in_file_path);
    fp = fopen(in_file_path, "r");
    if (fp == NULL){
        if (DEBUG == 1){
            printf("in 'readData', fp is null\n");
        }
        print_error();
        return False;
    }

    for(i=0; i < *N; i++){

        (fgets(line, lines_length[i] + 1, fp));

        vec = str_to_vec(line, lines_length[i], *d);

        free(line);
        if (i < (*N - 1)){
            line =   (char *) calloc(lines_length[i+1] + 1, sizeof(char));

        }

        if(vec == NULL){
            if (DEBUG == 1){
                printf("in 'readData', vec==NULL");
            }
            print_error();
            fclose(fp);
            free(lines_length);
            free(line);
            return False;
        }
        (*vectors)[i] = vec;
    }
    fclose(fp);
    free(lines_length);
    return True;
}

char *doubleToRoundStr(double num){
    char *res = NULL;

    res = (char *) calloc(400, sizeof (char));
    if(res == NULL){
        return NULL;
    }
    sprintf(res, "%.4f", num);
    return res;
}

char ***doubleVecsToStr(double **vectors, int d, int k){
    char ***res = NULL;
    int i = 0;
    int j = 0;


    res = (char ***) calloc(k, sizeof(char **));
    if (res == NULL){
        if (DEBUG) {
            printf("error in doubleVecsToSt: res is null\n");}
        print_error();
        return NULL;
    }
    for(i=0; i < k; i++) {
        res[i] = (char **) calloc(d, sizeof (char*));

        if (res[i] == NULL){
            if (DEBUG) {printf("error in doubleVecsToSt: res[%d] is null\n", i);}

            for(j = 0; j < i; j++){
                free(res[j]);
            }
            free(res);
            print_error();
            return NULL;
        }
    }
    for(i=0; i < k; i++){
        for(j=0; j <d; j++) {
            char *val= doubleToRoundStr(vectors[i][j]);
            if(val == NULL){
                print_error();
                return NULL;
            }
            res[i][j] = val;
        }
    }
    return res;
}



int writeData(char *out_file, char ***vec_strs, int d, int k){
    FILE *fp = NULL;
    int i = 0; int j = 0;

    fp = fopen(out_file, "w");
    if (fp ==NULL){
        if (DEBUG) {
            printf("error in writeData: fp is null\n");}
        print_error();
        return False;
        }
    for(i=0; i < k; i++){
        for(j=0; j <d -1; j++){
            fprintf(fp, "%s,", vec_strs[i][j]);
        }
        fprintf(fp, "%s\n", vec_strs[i][d-1]);
    }
    fclose(fp);
    return True;
}

int main(int argc, char *argv[]) {
    int k = 0;
    int max_iter = 0;
    int closest_clust_index = 0;
    int N = 0;
    int d = 0;
    int res = 0;
    int i = 0;
    int j = 0;
    int converged = False;

    char *in_file = NULL;
    char *out_file = NULL;
    double **vectors = NULL;
    double **centroids = NULL;
    double **clust_sums = NULL;
    int *clust_sizes = NULL;
    double *new_centroid = NULL;
    char ***centroids_strs = NULL;




    res = getProgramArgs(argc, argv, &k, &max_iter, &in_file, &out_file);
    if (!res){
        return 1;
    }

    if(readData(in_file, &vectors, &N, &d) == 0){
        if(vectors != NULL){
            free_memory_arr(vectors, N);
        }
        return 1;
    }
    if(k < 2 || k >= N){
        print_invalid_input();
        free_memory_arr(vectors, N);
        return 1;
    }


    centroids= (double**)calloc(k, (sizeof (double*)));
    if(centroids == NULL){
        free_memory_arr(vectors, N);
        print_error();
        return 1;
    }
    for (i = 0; i < k; i++){
        centroids[i] = (double*)calloc(d, (sizeof (double )));
        if(centroids[i] == NULL){
            free_memory_arr(vectors, N);
            free_memory_arr(centroids, i-1);
            print_error();
            return 1;
        }
    }
    for (i = 0; i < k; i++){
        for (j = 0; j < d; j++){
            centroids[i][j] = vectors[i][j];
        }
    }
    clust_sums= (double**)calloc(k,(sizeof (double*)));
    if(clust_sums == NULL){
        free_memory_arr(vectors, N);
        free_memory_arr(centroids, k);

        print_error();
        return 1;
    }

    for (i = 0; i < k; i++){
        clust_sums[i] = (double*)calloc(d, (sizeof (double )));
        if(clust_sums[i] == NULL){
            free_memory_arr(vectors, N);
            free_memory_arr(centroids, k);
            free_memory_arr(clust_sums, i-1);
            print_error();
            return 1;
        }
    }
    clust_sizes= (int*)calloc(k, (sizeof (int)));
    if(clust_sizes == NULL){
        free_memory_arr(vectors, N);
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
    for(i = 0; (i < max_iter) && !converged; i++){
        converged = True;
        for (j = 0; j < N; j++){
            closest_clust_index = closest_clust(vectors[j], centroids, k, d);
            assign_vec(vectors[j], clust_sums, clust_sizes, closest_clust_index, d);
        }

        for (j = 0; j < k; j++){
            new_centroid = calc_mean(clust_sums[j], clust_sizes[j], d);

            if(new_centroid == NULL){
                free_memory_arr(vectors, N);
                free_memory_arr(centroids, k);
                free_memory_arr(clust_sums, k);
                free(clust_sizes);
                return 1;
            }

            if(!is_converged(new_centroid, centroids[j], d)){
                converged = False;
            }
            centroids[j] = new_centroid;
        }

        reset(clust_sums, clust_sizes, k, d);
    }

    centroids_strs = doubleVecsToStr(centroids, d,k);
    if(centroids_strs == NULL){
        free_memory_arr(vectors, N);
        free_memory_arr(centroids, k);
        free_memory_arr(clust_sums, k);
        free(clust_sizes);
        return 1;
    }

    writeData(out_file, centroids_strs, d, k);

    for (i = 0; i < k; i++){
        free(centroids[i]);
        free(clust_sums[i]);
    }
    free(centroids);
    free(clust_sums);
    free(clust_sizes);
    for (i = 0; i < N; i++){
        free(vectors[i]);
    }
    free(vectors);
    return 0;
}
