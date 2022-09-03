#include "spkmeans.h"

int main(int argc, char *argv[]) {
    GOAL goal = dflt;
    char *file_name = NULL;
    vector *datapoints = NULL;
    double **out_mat = NULL, **w_mat = NULL, **data_mat = NULL;
    eigen_vector *out_jacobi = NULL;
    int n, d;
    if (argc != 3) {
        /* this program should only receive 2 arguments, a goal and a filename */
        return invalid();
    }
    goal = enumerate_goal(argv[1]);
    /* the first argument from the input is a string representing the wanted goal */
    /* and enumerate_goal translates it from string to the fitting GOAL enum */
    if (goal == dflt) {
        /* the translation failed, which means the insertted goal is invalid */
        return invalid();
    }
    file_name = argv[2];
    /* the second argument from the input is a string representing the name of the selected file */
    datapoints = organize(file_name, &n, &d);
    /* the method organize extract the datapoints from the file using the file name given
     * and finds the values of both n and d from the file
     * so datapoints now holds n datapoints with length d, and n and d hold their respective values */
    if (datapoints == NULL) {
        /* something went wrong with organization of the file */
        return error();
    }
    /* now all the data is set and ready to be operated on
     * the code performs on the data according to the specified goal */
    switch (goal) {
        case wam:
            /* in this case the Weighted Adjacency Matrix is calculated
             * as requested by the user
             * using the method find_w which calculates the wam
             * and puts it in the output matrix */
            out_mat = find_w(datapoints, n, d);
            break;
        case ddg:
            /* in this case the Diagonal Degree Matrix is calculated
             * using the wam calculated by find_w
             * and the method find_d which calculates the ddg */
            w_mat = find_w(datapoints, n, d);
            if (w_mat != NULL) {
                /* if w_mat == NULL something went wrong and the code does not procceed
                 * to calculate the ddg as the output matrix */
                out_mat = find_d(w_mat, n);
                free_double(&w_mat, n);
            }
            break;
        case lnorm:
            /* in this case the Normalized Graph Laplacian is calculated
             * using the method find_l which calculates the lnorm
             * and puts it in the output matrix */
            out_mat = find_l(datapoints, n, d);
            break;
        case jacobi:
            /* in this case the Jacobi graph is calculated
             * using the method find_j which calculates the eigen values and eigen vectors of the matrix
             * and puts it in the output eigen object */
            if (n != d) {
                /* in this case the number of datapoints and their length is not the same
                 * which means the matrix is not symmetric
                 * and thus a jacobi matrix can't be calculated
                 * so the input in the file is invalid and does not fit the specified goal */
                free_vectors(&datapoints, n);
                return invalid();
            }
            data_mat = extract_mat_from_dp(datapoints, n, d);
            /* translates datapoints into a format of a 2-dim array */
            if (data_mat != NULL) {
                out_jacobi = find_j(data_mat, n);
            }
            free_double(&data_mat, n);
            break;
        default:
            /* in this case the goal does not fit any of the above
             * and thus it is invalid */
            free_vectors(&datapoints, n);
            return invalid();
    }
    if (goal != jacobi) {
        /* for all goals but jacobi, the output is a matrix to be printed
         * and out_mat should contain this matrix */
        if (out_mat == NULL) {
            /* in this case something went wrong in the previous operations */
            free_vectors(&datapoints, n);
            return error();
        } else {
            /* if no error has occurred, the output matrix is printed
             * after which the memory of it is freed */
            print_matrix(out_mat, n);
            free_double(&out_mat, n);
        }
    } else {
        /* this is the only case in which the output is not the same format and object as the others
         * when the goal is jacobi, instead of a matrix, eigen values and eigen vectors are to be printed
         * both of which are supposed to be present in out_jacobi */
        if (out_jacobi == NULL) {
            free_vectors(&datapoints, n);
            return error();
        } else {
            /* if no error has occurred, the output eigen values and eigen vectors are printed
             * after which the memory of them is freed */
            print_jacobi(out_jacobi, n, d);
            free_eigens(&out_jacobi, n);
        }
    }
    free_vectors(&datapoints, n);
    return 0;
}

GOAL enumerate_goal(char *goal_name) {
    /* this method receives a string representing a name of a certain goal
     * and returns the fitting GOAL enum according to its specification */
    if (strcmp(goal_name, "wam") == 0) {
        return wam;
    }
    if (strcmp(goal_name, "ddg") == 0) {
        return ddg;
    }
    if (strcmp(goal_name, "lnorm") == 0) {
        return lnorm;
    }
    if (strcmp(goal_name, "jacobi") == 0) {
        return jacobi;
    }
    /* in case the string was none of the above, the default GOAL is returned */
    return dflt;
}

int extract_n(FILE *inp) {
    /* this method receives a file of datapoints and calculates n for it
     * such that n represents the total number of datapoints in the file */
    int n = 0;
    char c;
    while ((c = fgetc(inp)) != EOF) {
        /* extract_n is only called upon once, and starts to read the file from the beginning
         * each datapoint is represented by a line of arguments
         * so n counts the number of lines */
        if (c == '\n') {
            /* after every line there's a new line '\n' */
            n++;
        }
    }
    return n;
}

int extract_d(FILE *inp) {
    /* this method receives a file of datapoints and calculates d for it
     * such that d represents the number of arguments in each datapoint (length) in the file */
    int d = 1;
    char c;
    fseek(inp, 0, SEEK_SET);
    /* extract_d is only called upon after prior mending with the file
     * so it needs to go back to the beginning of the file */
    c = fgetc(inp);
    while (c != '\n') {
        /* calculating the length of the first datapoint should suffice
         * because the length of all datapoints in a file is the same
         * when a new line '\n' is found, it represents the end of a datapoint */
        if (c == ',') {
            /* the arguments of a datapoint are seperated by one comma ','
             * and there's no comma after the last argument
             * so d counts the number of commas beginning at 1
             * which is similar to counting the number of arguments */
            d++;
        }
        c = fgetc(inp);
    }
    return d;
}

int vector_reader(FILE *inp, vector *datapoints, int d, int n) {
    /* this method receive a file and an array of n datapoints with length d each
     * and reads the datapoints from the file into the array of vectors */
    double point;
    int i = 0, j = 0, m;
    /* k here does not represent a cluster but is used as an index */
    fseek(inp, 0, SEEK_SET);
    /* extract_d is only called upon after prior mending with the file
     * so it needs to go back to the beginning of the file */
    for (m = 0; m < n * d; m++) {
        /* this loop scans all arguments in the file, representing all the datapoints */
        fscanf(inp, "%lf,", &point);
        /* each argument is read from the file */
        if (i == d) {
            /* in this case, a full datapoint has been read
             * and i is set to be 0 in order to represent the beginning of the next datapoint
             * in this method i represents the number of argument read in current datapoint */
            i = 0;
            j++;
        }
        datapoints[j].args[i] = point;
        /* the argument read at the beginning of the iteration
         * is put in the index representing the current argument
         * of the array representing the arguments of the current datapoint being read */
        i++;
    }
    return 0;
}

vector *organize(char *file_name, int *n, int *d) {
    /* this method receives a string and two pointers of integers
     * it opens the file with the name given in the string
     * and puts the values of n and d according to the datapoints in that file
     * it returns the datapoints in the file as an array of vector objects */
    int i;
    FILE *input = fopen(file_name, "r");
    vector *datapoints = NULL;
    *n = extract_n(input);
    *d = extract_d(input);
    if (input == NULL) {
        return NULL;
    }
    datapoints = (vector *) calloc(*n, sizeof(vector));
    if (datapoints == NULL) {
        fclose(input);
        return NULL;
    }
    for (i = 0; i < *n; i++) {
        datapoints[i].args = NULL;
        datapoints[i].args = (double *) calloc(*d, sizeof(double));
        if (datapoints[i].args == NULL) {
            fclose(input);
            free_vectors(&datapoints, *n);
            return NULL;
        }
    }
    if (vector_reader(input, datapoints, *d, *n) == 2) {
        /* vector reader takes care of the reading of datapoints into the array */
        free_vectors(&datapoints, *n);
        fclose(input);
        return NULL;
    }
    fclose(input);
    return datapoints;
}

void free_double(double ***values, int len) {
    /* this method frees a 2-dim array of doubles
     * values is a pointer to the array to be freed with len rows */
    int i;
    for (i = 0; i < len; i++) {
        /* frees memory of each row in the array */
        free((*values)[i]);
    }
    free(*values);
    /* frees the whole array */
}

void free_vectors(vector **arr, int count) {
    /* this method frees an array of vectors
     * arr is a pointer to the array to be freed with count arguments */
    int i;
    for (i = 0; i < count; i++) {
        /* frees memory of each args array of each vector in arr */
        free((*arr)[i].args);
    }
    free(*arr);
    /* frees the vectors */
}

void free_eigens(eigen_vector **arr, int count) {
    /* this method frees an array of eigen vectors
     * arr is a pointer to the array to be freed with count arguments */
    int i;
    for (i = 0; i < count; i++) {
        /* frees memory of each args array of each vector in arr */
        free((*arr)[i].args);
    }
    free(*arr);
    /* frees the eigen_vectors */
}

void print_matrix(double **mat, int n) {
    /* this method prints a matrix of size nxn
     * it prints each row of the matrix as a line
     * such that different lines are seperated with a new line '\n'
     * and different arguments are seperated with a comma ','
     * the format of each value is 4 digits after the decimal point */
    int i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n - 1; j++) {
            /* the last argument in each line isn't followed by a comma
             * but a new line instead */
            printf("%.4f,", mat[i][j]);
        }
        printf("%.4f\n", mat[i][n - 1]);
    }
}


void print_asym_matrix(double **mat, int rows, int cols) {
    /* this method prints a matrix of size nxn
     * it prints each row of the matrix as a line
     * such that different lines are seperated with a new line '\n'
     * and different arguments are seperated with a comma ','
     * the format of each value is 4 digits after the decimal point */
    int i, j;
    for (i = 0; i < rows ; i++) {
        for (j = 0; j < cols - 1; j++) {
            /* the last argument in each line isn't followed by a comma
             * but a new line instead */
            printf("%.4f,", mat[i][j]);
        }
        printf("%.4f\n", mat[i][cols - 1]);
    }
}
void print_values(double **mat, int n) {
    /* this method prints a matrix of size nx1 as a line of values
     * seperated by commas ','
     * the format of each value is 4 digits after the decimal point */
    int i;
    for (i = 0; i < n - 1; i++) {
        /* the last argument isn't followed by a comma
         * but a new line instead */
        printf("%.4f,", mat[i][0]);
    }
    printf("%.4f\n", mat[n - 1][0]);
}

void print_jacobi(eigen_vector *j_mat, int n, int d) {
    /* this method prints the eigen values of j_mat as one line
     * and then prints the eigen vectors of j_mat, each vector as a seperate line */
    double **values = NULL, **vectors = NULL;
    values = extract_values(j_mat, n);
    vectors = extract_mat_from_vectors(j_mat, n, d);
    if (values == NULL || vectors == NULL) {
        free_double(&values, n);
        free_double(&vectors, n);
        error();
        return;
    }
    print_values(values, n);
    print_matrix(vectors, n);
    free_double(&values, n);
    free_double(&vectors, n);
}

double **multiply(double **a_mat, double **b_mat, int n, int m) {
    /* this method receives two matrices a_mat and b_mat
     * a_mat is a nxm matrix and b_mat is a mxn matrix
     * it calculates their multiplication product, a nxn matrix
     * and returns it */
    int i, j, k;
    /* k here does not represent a cluster but is used as an index */
    double **ab_mat = NULL;
    ab_mat = (double **) calloc(n, sizeof(double *));
    if (ab_mat == NULL) {
        return NULL;
    }
    for (i = 0; i < n; i++) {
        ab_mat[i] = (double *) calloc(n, sizeof(double));
        if (ab_mat[i] == NULL) {
            free_double(&ab_mat, i);
            return NULL;
        }
        for (j = 0; j < n; j++) {
            for (k = 0; k < m; k++) {
                /* each argument of the result matrix ab_mat is calculated
                 * according to the rules of matrix multiplication */
                ab_mat[i][j] += a_mat[i][k] * b_mat[k][j];
            }
        }
    }
    return ab_mat;
}

int sum_sub(vector *sumsub, vector v1, vector v2, char *sign, int d) {
    /* this method receives vector v1 and vector v2, both of length d
     * and either sums or subtracts them
     * if sign == '+' sumsub points at a vector with values representing the args of v1+v2
     * if sign == '-' sumsub points at a vector with values representing the args of v1-v2
     * the output is 2 if an error occurred or 0 if no errors occurred */
    int i;
    double *sum = NULL;
    sum = (double *) calloc(d, sizeof(double));
    if (sum == NULL) {
        return 2;
    }
    for (i = 0; i < d; i++) {
        if (*sign == '+') {
            /* in this case the final vector is going to represent the sum of the two input vectors */
            sum[i] = v1.args[i] + v2.args[i];
        }
        if (*sign == '-') {
            /* in this case the final vector is going to represent the subtraction of the two input vectors */
            sum[i] = v1.args[i] - v2.args[i];
        }
    }
    for (i = 0; i < d; i++) {
        /* inserting the arguments of sum into the vector sumsub is pointing at */
        sumsub->args[i] = sum[i];
    }
    free(sum);
    return 0;
}

void divide(vector *quo, vector v, int divisor, int d) {
    /* this method receives a vector v of length d
     * and a pointer quo to a vector of length d
     * and sets the arguments of the vector quo points at to be the result
     * of the division of v's arguments by divisor */
    int i;
    for (i = 0; i < d; i++) {
        quo->args[i] = v.args[i] / divisor;
    }
}

double euc_norm(vector v, int d) {
    /* this method calculates the euclidean norm of vector v of length d
     * and returns the result */
    int i;
    double sum = 0;
    for (i = 0; i < d; i++) {
        sum += (v.args[i]) * (v.args[i]);
    }
    return sqrt(sum);
}

double **find_w(vector *datapoints, int n, int d) {
    /* this method calculates the Weighted Adjacency Matrix of given datapoints */
    int i, j;
    double norm;
    double **w_mat = NULL;
    vector sub;
    double *sub_args = NULL;
    /* initializations and memory allocations */
    sub_args = (double *) calloc(d, sizeof(double));
    if (sub_args == NULL) {
        return NULL;
    }
    sub.args = sub_args;
    w_mat = (double **) calloc(n, sizeof(double *));
    if (w_mat == NULL) {
        free(sub_args);
        return NULL;
    }
    for (i = 0; i < n; i++) {
        w_mat[i] = (double *) calloc(n, sizeof(double));
        if (w_mat[i] == NULL) {
            free(sub_args);
            free_double(&w_mat, i);
            return NULL;
        }
        for (j = i + 1; j < n; j++) {
            if (sum_sub(&sub, datapoints[i], datapoints[j], "-", d) == 2) {
                free(sub_args);
                free_double(&w_mat, i);
                return NULL;
            }
            /* for each pair of datapoints i and j from the data, their subtraction is calculated
             * and the euclidean norm of the result is then calculated
             * according to the norm, the [i][j] value of the wam is calculated
             * as specified in the instructions */
            norm = euc_norm(sub, d);
            w_mat[i][j] = exp(norm * (-0.5));
        }
    }
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            /* since the matrix is symmetric, the other values where j <= i are filled
             * according to the values calculated before where i > j */
            w_mat[j][i] = w_mat[i][j];
        }
    }
    free(sub_args);
    return w_mat;
}

double **find_d(double **w_mat, int n) {
    /* this method calculates the Diagonal Degree Matrix of given datapoints
     * using their pre-calculated Weighted Adjacency Matrix */
    int i, j;
    double **d_mat = NULL;
    /* initializations and memory allocations */
    d_mat = (double **) calloc(n, sizeof(double *));
    if (d_mat == NULL) {
        return NULL;
    }
    for (i = 0; i < n; i++) {
        d_mat[i] = (double *) calloc(n, sizeof(double));
        if (d_mat[i] == NULL) {
            free_double(&d_mat, i);
            return NULL;
        }
        for (j = 0; j < n; j++) {
            /* as specified in the instructions */
            d_mat[i][i] += w_mat[i][j];
        }
    }
    return d_mat;
}

double **find_l(vector *datapoints, int n, int d) {
    /* this method calculates the Normalized Graph Laplacian of given datapoints */
    int i, j;
    double **l_mat = NULL, **d_mat = NULL, **w_mat = NULL, **dw = NULL, **dwd = NULL;
    /* initializations and memory allocations */
    w_mat = find_w(datapoints, n, d);
    /* calculation of the wam of datapoints */
    if (w_mat == NULL) {
        return NULL;
    }
    d_mat = find_d(w_mat, n);
    /* calculation of the ddg of datapoints using w_mat */
    if (d_mat == NULL) {
        free_double(&w_mat, n);
        return NULL;
    }
    for (i = 0; i < n; i++) {
        d_mat[i][i] = 1 / sqrt(d_mat[i][i]);
    }
    /* change from d_mat to d_mat^(-1/2) */
    dw = multiply(d_mat, w_mat, n, n);
    if (dw == NULL) {
        free_double(&d_mat, n);
        free_double(&w_mat, n);
        return NULL;
    }
    dwd = multiply(dw, d_mat, n, n);
    free_double(&d_mat, n);
    free_double(&w_mat, n);
    free_double(&dw, n);
    if (dwd == NULL) {
        return NULL;
    }
    /* memory allocation for the lnorm */
    l_mat = (double **) calloc(n, sizeof(double *));
    if (l_mat == NULL) {
        free_double(&dwd, n);
        return NULL;
    }
    /* lnorm has been calculated using the pre-calculated matrices d_mat^(-1/2) and w_mat
     * and now it's values are fitted from the previous calculations */
    for (i = 0; i < n; i++) {
        l_mat[i] = (double *) calloc(n, sizeof(double));
        if (l_mat[i] == NULL) {
            free_double(&dwd, n);
            free_double(&l_mat, i);
            return NULL;
        }
        for (j = 0; j < n; j++) {
            l_mat[i][j] = -dwd[i][j];
            if (i == j) {
                l_mat[i][j] += 1;
            }
        }
    }
    free_double(&dwd, n);
    return l_mat;
}

double **find_p(double **a_mat, int i, int j, int n) {
    /* this method builds the rotation matrix P */
    double theta, t, c, s;
    int sign_theta, k;
    /* k here does not represent a cluster but is used as an index */
    double **p_mat = NULL;
    p_mat = (double **) calloc(n, sizeof(double *));
    if (p_mat == NULL) {
        return NULL;
    }
    for (k = 0; k < n; k++) {
        p_mat[k] = (double *) calloc(n, sizeof(double));
        if (p_mat[k] == NULL) {
            free_double(&p_mat, k);
            return NULL;
        }
        /* building the identity matrix of size nxn */
        p_mat[k][k] = 1;
    }
    theta = (a_mat[j][j] - a_mat[i][i]) / (2 * a_mat[i][j]);
    sign_theta = (theta < 0) ? -1 : 1;
    /* theta and theta's sign are calculated according to the instructions */
    t = sign_theta / (fabs(theta) + sqrt(pow(theta, 2) + 1));
    c = 1 / sqrt(pow(t, 2) + 1);
    p_mat[i][i] = c;
    p_mat[j][j] = c;
    s = t * c;
    p_mat[i][j] = s;
    p_mat[j][i] = -1 * s;
    /* all calculations according to the specified instructions */
    return p_mat;
}

eigen_vector *find_j(double **a_mat, int n) {
    /* this method calculates the eigen values and eigen vectors of a_mat
     * by using the jacobi algorithm */
    int i = 0, j = 1, k, l, iter = 0, i1, i2;
    /* k here does not represent a cluster but is used as an index */
    double off_a, off_a_tag;
    double **v_mat = NULL, **p_mat = NULL, **vp = NULL;
    eigen_vector *j_mat = NULL;
    v_mat = (double **) calloc(n, sizeof(double *));
    if (v_mat == NULL) {
        return NULL;
    }
    for (k = 0; k < n; k++) {
        v_mat[k] = (double *) calloc(n, sizeof(double));
        if (v_mat[k] == NULL) {
            free_double(&v_mat, k);
            return NULL;
        }
        v_mat[k][k] = 1;
        /* building the identity matrix of size nxn */
    }
    if (off(a_mat, n) != 0) {
        /* if off(a_mat, n) == 0, a_mat is already diagonal
         * no further need to diagonalize it
         * if not, the jacobi algorithm is performed to diagonalize a_mat */
        do {
            iter++;
            find_pivot(a_mat, &i, &j, n);
            off_a = off(a_mat, n);
            p_mat = find_p(a_mat, i, j, n);
            if (p_mat == NULL) {
                free_double(&v_mat, n);
                return NULL;
            }
            rotate(a_mat, p_mat, i, j, n);
            /* changes a_mat in place */
            off_a_tag = off(a_mat, n);
            vp = multiply(v_mat, p_mat, n, n);
            free_double(&p_mat, n);
            if (vp == NULL) {
                free_double(&v_mat, n);
                return NULL;
            }
            for (i1 = 0; i1 < n; i1++) {
                for (i2 = 0; i2 < n; i2++) {
                    /* inserting values such that v_mat = vp */
                    v_mat[i1][i2] = vp[i1][i2];
                }
            }
            free_double(&vp, n);
        } while ((off_a - off_a_tag > 0.00001) && (iter < 100));
        /* as long as convergence doesn't happen and the number of iterations is lower than the maximal
         * the loop keeps going */
    }
    j_mat = (eigen_vector *) calloc(n, sizeof(eigen_vector));
    if (j_mat == NULL) {
        free_double(&v_mat, n);
        return NULL;
    }
    for (k = 0; k < n; k++) {
        j_mat[k].args = (double *) calloc(n, sizeof(double));
        if (j_mat[k].args == NULL) {
            free_double(&v_mat, n);
            free_eigens(&j_mat, n);
            return NULL;
        }
        j_mat[k].lambda = a_mat[k][k];
        /* the values of the jacobi matrix diagonal are inserted in j_mat according to the ones calculated in a_mat */
        for (l = 0; l < n; l++) {
            j_mat[k].args[l] = v_mat[l][k];
            /* the rest of the values of the jacobi matrix are inserted in j_mat according to the ones calculated in a_mat */
        }
    }
    free_double(&v_mat, n);
    return j_mat;
}

double **find_t(eigen_vector *eigen_vectors, int n, int k) {
    /* this method receives n eigen vectors with length k each
     * and returns a 2-dim array with n rows and k columns that is constructed from them */
    double **u_mat = NULL;
    int i, j;
    u_mat = (double **) calloc(n, sizeof(double *));
    if (u_mat == NULL) {
        return NULL;
    }
    for (i = 0; i < n; i++) {
        /* first the method calculates the U matrix containing the eigen vectors as columns */
        u_mat[i] = (double *) calloc(k, sizeof(double));
        if (u_mat[i] == NULL) {
            free_double(&u_mat, i);
            return NULL;
        }
        for (j = 0; j < k; j++) {
            u_mat[i][j] = eigen_vectors[j].args[i];
        }
    }
    printf("submatrix: \n");
    print_asym_matrix(u_mat, n, k);
    printf("end submatrix \n");

    /* normalizes u_mat to become matrix T, which is the normalized version of U
     * the change happens in place */
    normalize(u_mat, n, k);
    printf("normal submatrix: \n");
    print_asym_matrix(u_mat, n, k);
    printf("end normal submatrix \n");
    /* T, already normalized, is returned */
    return u_mat;
}

double **extract_mat_from_dp(vector *datapoints, int n, int d) {
    /* this method receives datapoints as an array of vectors
     * and returns a 2-dim double array representation of them
     * such that each row in the matrix is an array of arguments of respective vector
     * from the datapoints structure */
    double **vectors = NULL;
    int i, j;
    vectors = (double **) calloc(n, sizeof(double *));
    if (vectors == NULL) {
        return NULL;
    }
    for (i = 0; i < n; i++) {
        vectors[i] = (double *) calloc(d, sizeof(double));
        if (vectors[i] == NULL) {
            free_double(&vectors, i);
            return NULL;
        }
        for (j = 0; j < d; j++) {
            vectors[i][j] = datapoints[i].args[j];
            /* each value of vectors is picked according to the value in datapoints */
        }
    }
    return vectors;
}

double **extract_values(eigen_vector *eigens, int n) {
    /* this method creates a 2-dim array nx1
     * such that each row has one value that represents an eigen value found in eigens */
    double **values = NULL;
    int i;
    values = (double **) calloc(n, sizeof(double *));
    if (values == NULL) {
        return NULL;
    }
    for (i = 0; i < n; i++) {
        values[i] = (double *) calloc(1, sizeof(double));
        if (values[i] == NULL) {
            free_double(&values, i);
            return NULL;
        }
        values[i][0] = eigens[i].lambda;
    }
    return values;
}

double **extract_mat_from_vectors(eigen_vector *eigens, int n, int d) {
    /* this method creates a 2-dim array dxn
     * such that each column has values that represent an eigen vector found in eigens */
    double **vectors = NULL;
    int i, j;
    vectors = (double **) calloc(d, sizeof(double *));
    if (vectors == NULL) {
        return NULL;
    }
    for (i = 0; i < d; i++) {
        vectors[i] = (double *) calloc(n, sizeof(double));
        if (vectors[i] == NULL) {
            free_double(&vectors, i);
            return NULL;
        }
        for (j = 0; j < n; j++) {
            vectors[i][j] = eigens[j].args[i];
        }
    }
    return vectors;
}

double off(double **a_mat, int n) {
    /* this method receives a_mat, a matrix of size nxn
     * and calculates off(a_mat)^2 a values that is defined to be
     * the sum of squares of all off-diagonal elemnts of a_mat
     * it returns the result */
    double sum = 0;
    int i, j;
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            if (i != j) {
                /* the value of off only considers the elemnts that are not on the diagonal
                 * so the elemnt adds to the total sum if and only if it's not on diagonal
                 * meaning, it's not located on the row and column of same index */
                sum += pow(a_mat[i][j], 2);
            }
        }
    }
    return sum;
}

void find_pivot(double **a_mat, int *p1, int *p2, int n) {
    /* this method finds the indices p1, p2 of matrix a_mat of size nxn
     * for which a_mat[p1][p2] has the largest absolute value
     * also called pivot */
    int i, j;
    double max = 0, curr;
    *p1 = 0, *p2 = 1;
    for (i = 0; i < n; i++) {
        for (j = i + 1; j < n; j++) {
            curr = fabs(a_mat[i][j]);
            /* calculates the current absolute value of a_mat[i][j] */
            *p1 = (curr > max) ? i : *p1;
            *p2 = (curr > max) ? j : *p2;
            max = (curr > max) ? curr : max;
            /* if curr is greater than the current max, changes the indices p1 and p2
             * to point at current i and j respectively
             * and updates the new max to be curr
             * if not, no further changes are made this iteration */
        }
    }
}


void rotate(double **a_mat, double **p_mat, int i, int j, int n) {
    /* this method rotates the matrix a_mat
     * using the equations given in the assignment
     * expressing the relation between a_mat and a_mat', the version of a_mat post-rotation */
    double c = p_mat[i][i], s = p_mat[i][j], temp_ri, temp_ii, temp_jj;
    int r;
    for (r = 0; r < n; r++) {
        if (r != i && r != j) {
            /* all calculations according to the instructions */
            temp_ri = a_mat[r][i];
            a_mat[r][i] = c * temp_ri - s * a_mat[r][j];
            a_mat[i][r] = a_mat[r][i];
            a_mat[r][j] = c * a_mat[r][j] + s * temp_ri;
            a_mat[j][r] = a_mat[r][j];
        }
    }
    /* all calculations according to the instructions */
    temp_ii = a_mat[i][i];
    temp_jj = a_mat[j][j];
    a_mat[i][i] = pow(c, 2) * temp_ii + pow(s, 2) * temp_jj - 2 * s * c * a_mat[i][j];
    a_mat[j][j] = pow(s, 2) * temp_ii + pow(c, 2) * temp_jj + 2 * s * c * a_mat[i][j];
    a_mat[i][j] = 0;
    a_mat[j][i] = 0;
}

void normalize(double **mat, int n, int k) {
    /* this method receives a matrix mat of size nxk
     * and normalizes it in place */
    int i, j;
    double sum = 0;
    for (i = 0; i < n; i++) {
        for (j = 0; j < k; j++) {
            /* first the sum of squared values of all arguments in the row is calculated */
            sum += pow(mat[i][j], 2);
        }
        sum = sqrt(sum);
        for (j = 0; j < k; j++) {
            /* all values in the row are normalized according to sum */
            mat[i][j] = sum > 0 ? mat[i][j] / sum : 0;
        }
        sum = 0;
    }
}

void terminate(vector *centroids_prev, int *counts, vector *sums, int k) {
    /* this method rids of the memory allocations of all specified arrays in the input
     * and frees all of them before the program is terminated
     * in the spk goal only */
    if (centroids_prev != NULL) {
        free_vectors(&centroids_prev, k);
    }
    free(counts);
    if (sums != NULL) {
        free_vectors(&sums, k);
    }
}

int eigen_comparator(const void *p1, const void *p2) {
    /* this comparator compares between two eigen vectors
     * it returns 1 if the first eigen vector has the smaller eigen value
     * or -1 if the first eigen vector has the bigger eigen value
     * and 0 if the eigen values of both vectors are the same */
    const eigen_vector *eigen1 = p1, *eigen2 = p2;
    double comp = eigen1->lambda - eigen2->lambda;
    if (comp > 0) {
        return -1;
    }
    if (comp < 0) {
        return 1;
    }
    return 0;
}

int k_heuristic(eigen_vector *eigens, int n) {
    /* this method calculates the optimal number of clusters k
     * for the k_means algorithm
     * according to the k heuristic */
    double delta, max = 0;
    int i, k = 1;
    for (i = 0; i < n / 2; i++) {
        delta = eigens[i].lambda - eigens[i + 1].lambda;
        delta = fabs(delta);
        k = (delta > max) ? i + 1 : k;
        max = (delta > max) ? delta : max;
    }
    return k;
}

double **fit_spk_c(vector *datapoints, int *k, int n, int d) {
    /* this method is called upon for the spk goal from the c module
     * it receives datapoints which is an array of vectors
     * it also receives n the amount of datapoints and d the length of each
     * and a pointer k to a number that will represent the number of wanted clusters */
    double **t_mat = NULL, **l_mat = NULL;
    eigen_vector *jacobi_eigen = NULL;
    l_mat = find_l(datapoints, n, d);
    print_matrix(l_mat, n);
    /* computes the normalized graph laplacian lnorm */
    if (l_mat == NULL) {
        return NULL;
    }
    jacobi_eigen = find_j(l_mat, n);
    printf("\n\n");
    print_jacobi(jacobi_eigen, n, d);
    /* calculates the eigen values and eigen vectors of l_mat */
    if (jacobi_eigen == NULL) {
        free_double(&l_mat, n);
        return NULL;
    }
    qsort(jacobi_eigen, n, sizeof(eigen_vector), eigen_comparator);
    printf("le jacobian after sort: \n");
    print_jacobi(jacobi_eigen, n, d);
    printf("\n");
    /* sorts the found eigen vectors according to the given comparator */
    if (*k == 0) {
        /* in that case, no valid k was given as an input, so the best fitted k is calculated
         * to be the number of clusters for the algorithm */
        *k = k_heuristic(jacobi_eigen, n);
    }
    t_mat = find_t(jacobi_eigen, n, *k);
    /* U is calculated inside find_t and normalized to form T
     * essentially in this stage T is calculated according to the specified instructions */
    free_double(&l_mat, n);
    free_eigens(&jacobi_eigen, n);
    if (t_mat == NULL) {
        return NULL;
    }
    return t_mat;
}

double **k_means(vector *datapoints, int k, int max_iter, double epsilon, int n, int d, vector *centroids) {
    /* this method performs full k_means */
    int iter = 0, i, j;
    vector *centroids_prev = NULL, *sums = NULL;
    int *counts = NULL;
    double **res = NULL;
    centroids_prev = (vector *) calloc(k, sizeof(vector));
    /* this array of vectors will hold the previous centroids of each iteration of the algorithm */
    if (centroids_prev == NULL) {
        /* in case memory allocation went wrong, all previous successful memory allocations
         * should be freed before the program is terminated */
        terminate(centroids_prev, counts, sums, k);
        return NULL;
    }
    sums = (vector *) calloc(k, sizeof(vector));
    /* this array of vectors will hold the sum of vectors of each cluster of k clusters
     * such that vector i of sums is the sum of all vectors of cluster i in datapoints during this iteration */
    if (sums == NULL) {
        terminate(centroids_prev, counts, sums, k);
        return NULL;
    }
    counts = (int *) calloc(k, sizeof(int));
    /* this array will hold the amount of vectors of each cluster of k clusters
     * such that integer i of counts is the amount of vectors of cluster i in datapoints during this iteration */
    if (counts == NULL) {
        terminate(centroids_prev, counts, sums, k);
        return NULL;
    }
    for (i = 0; i < k; i++) {
        sums[i].args = NULL;
        sums[i].args = (double *) calloc(d, sizeof(double));
        if (sums[i].args == NULL) {
            terminate(centroids_prev, counts, sums, k);
            return NULL;
        }
        centroids_prev[i].args = NULL;
        centroids_prev[i].args = (double *) calloc(d, sizeof(double));
        if (centroids_prev[i].args == NULL) {
            terminate(centroids_prev, counts, sums, k);
            return NULL;
        }
    }

    while (convergence(centroids, centroids_prev, iter, k, d, epsilon) == 0 && iter < max_iter) {
        /* the loop goes on until the centroids converge, as long as it doesn't do more iterations than the maximum */
        for (i = 0; i < k; i++) {
            for (j = 0; j < d; j++) {
                sums[i].args[j] = 0;
            }
        }
        for (i = 0; i < k; i++) {
            counts[i] = 0;
        }
        /* all sums and counts of clusters are initialized at the beginning of every iteration
         * because the datapoints are about to be arranged in new clusters */
        for (i = 0; i < n; i++) {
            if (change_cluster(&datapoints[i], centroids, k, d) == 2) {
                terminate(centroids_prev, counts, sums, k);
                return NULL;
            }
        }
        /* change_cluster arranges the datapoints in clusters
         * and gives each datapoint its updated most fitting centroid */
        for (i = 0; i < n; i++) {
            /* the new sums and counts are calculated according to the current cluster organization */
            sum_sub(&sums[datapoints[i].cluster], sums[datapoints[i].cluster], datapoints[i], "+", d);
            counts[datapoints[i].cluster]++;
        }
        for (i = 0; i < k; i++) {
            prev(&centroids_prev[i], centroids[i], d);
            /* prev puts the current saved centroids in centroids_prev
             * such that they'll be ready for the use as the previous centroids
             * before calculation of the new centroids */
            divide(&centroids[i], sums[i], counts[i], d);
            /* after the previous centroids have been saved
             * the new ones are calculated using the current sums and counts of the clusters found */
        }
        iter++;
    }
    res = (double **) calloc(k, sizeof(double *));
    /* res is going to hold the final centroids to be outputted */
    if (res == NULL) {
        terminate(centroids_prev, counts, sums, k);
        return NULL;
    }
    for (i = 0; i < k; i++) {
        res[i] = (double *) calloc(d, sizeof(double));
        if (res[i] == NULL) {
            terminate(centroids_prev, counts, sums, k);
            free_double(&res, i);
            return NULL;
        }
    }
    for (i = 0; i < k; i++) {
        for (j = 0; j < d; j++) {
            /* each value of res is found according to the values of the most recent centroids
             * such that centroid i is represented by row i of res */
            res[i][j] = centroids[i].args[j];
        }

    }
    terminate(centroids_prev, counts, sums, k);
    return res;
}

int convergence(vector *centroids, vector *centroids_prev, int iter, int k, int d, double epsilon) {
    /* this method receives current centroids and previous centroids of certain clustered datapoints
     * of length d and with k clusters
     * and checks if the centroids are converged with accuracy of epsilon
     * it returns 0 if they aren't, 1 if they are and 2 if an error occurred */
    int m;
    /* represents an index of the clusters */
    double norm;
    vector sub;
    double *sub_args = NULL;
    sub_args = (double *) calloc(d, sizeof(double));
    if (sub_args == NULL) {
        return 2;
    }
    sub.args = sub_args;
    if (iter == 0) {
        /* if iter == 0 then it's the first iteration of the algorithm
         * so there are no previous centroids yet
         * and a conversion couldn't occur */
        free(sub_args);
        return 0;
    }
    for (m = 0; m < k; m++) {
        sum_sub(&sub, centroids[m], centroids_prev[m], "-", d);
        /* for each pair of centroid & prev_centroid, they're subtracted
         * and the result of their subtraction is put in sub */
        norm = euc_norm(sub, d);
        /* the norm of sub is calculated */
        if (norm >= epsilon) {
            /* in this case, the current centroid compared to its respective previous centroid
             * did not converge, so a full convergence did not occur */
            free(sub_args);
            return 0;
        }
    }
    /* if all centroids have been checked and none of made the method return 0
     * then all of them converged
     * meaning the centroids are converged */
    free(sub_args);
    return 1;
}

int change_cluster(vector *v, vector const *centroids, int k, int d) {
    /* this method receives an array of k centroids of length d
     * and a datapoint v of length d
     * and puts v in a new clusters best suited for it according to the current centroids */
    int m;
    /* represents an index of the centroids */
    double min = INT_MAX;
    vector sub;
    double *sub_args = NULL;
    sub_args = (double *) calloc(d, sizeof(double));
    if (sub_args == NULL) {
        return 2;
    }
    sub.args = sub_args;
    for (m = 0; m < k; m++) {
        sum_sub(&sub, *v, centroids[m], "-", d);
        /* the distance between v to each centroid is calculated via their subtraction */
        if (euc_norm(sub, d) < min) {
            /* if a centroid closer to v was find, its cluster is v's new cluster */
            min = euc_norm(sub, d);
            (*v).cluster = m;
        }
    }
    free(sub_args);
    return 0;
}

int prev(vector *centroid_prev, vector centroid, int d) {
    /* this method receives a pointer to a vector centroid_prev
     * and a vector centroid
     * and inserts the values of centroid's arguments to the arguments of the vector
     * that centroid_prev is pointing at
     * such that after the method is done, centroid_prev is pointing at a vector
     * that equals centroid in terms of their arguments */
    int i;
    double *prev = NULL;
    prev = (double *) calloc(d, sizeof(double));
    if (prev == NULL) {
        return 2;
    }
    for (i = 0; i < d; i++) {
        prev[i] = centroid.args[i];
    }
    for (i = 0; i < d; i++) {
        centroid_prev->args[i] = prev[i];
    }
    free(prev);
    return 0;
}

int invalid() {
    /* this method is called upon if an invalid value has been inputted to the program
     * it prints a message and terminates */
    printf("Invalid Input!\n");
    return 1;
}

int error() {
    /* this method is called upon if an error occurred during the run of the program
     * it prints a message and terminates */
    printf("An Error Has Occurred\n");
    return 1;
}
