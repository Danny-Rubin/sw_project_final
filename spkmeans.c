/* c interface file */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "executeKmeans.h"
#include "spkmeans.h"

#define DEBUG 1
#define EPSILON 0.00001
#define DEFAULT_MAX_ITER 300
#define MAX_ROTATIONS 100
#define EPSILON 0.00001


/* global variables of the program: number of vectors, number of clusters and input vectors dimension */

int n_const = 0;
int d_const = 0;



int main(int argc, char* argv[]){
    /*read argv
     validate cmd args
     call c entry point with args
     capture return value
     return 0 on success and 1 on error */
    if(!validateProgramArgs(argc)){
        return 1;
    }

    return cEntryPoint(-1, argv[1], argv[2], 0);

}



int validateProgramArgs(int argc){
    if (argc != 3){
        print_invalid_input();
        return False;
    }
    return True;
}

/* main entry point for c interface
 * params: filename, k, goal
 * what it does: reads input file and calls appropriate execution function and then print to screen result
 */
int cEntryPoint(int k, char *goal, char *fileName, int stage) {
    Matrix *matrixPtr = allocateVector(1, sizeof(Matrix), True);
    JacobiRes jacobiRes;
    Matrix res = NULL;
    atexit(freeAllMemory);
    if (!readData(fileName, matrixPtr)) {
        return 1;
    }
    if (k >= n_const || !validateGoal(goal)) {
        print_invalid_input();
        exit(1);
    }
    switch (goal[0]) {
        case 'w': /* goal = wam */
            res = executeWam(*matrixPtr);
            printDoubleMatrix(res, n_const, n_const);
            break;
        case 'd': /* goal = ddg */
            res = executeDdg(*matrixPtr, NULL);
            printDoubleMatrix(res, n_const, n_const);
            break;
        case 'l': /* goal = lnorm */
            res = executeLnorm(*matrixPtr);
            printDoubleMatrix(res, n_const, n_const);
            break;
        case 'j': /* goal = jacobi */
            if (!validateMatrixSymmetric(*matrixPtr, n_const, d_const)) { /* input matrix not symmetric*/
                print_invalid_input();
                return 1;
            }
            jacobiRes = executeJacobi(*matrixPtr);
            printDoubleMatrix(&jacobiRes.eigenVals, 1, n_const);
            printDoubleMatrix(jacobiRes.eigenVecs, n_const, n_const);
            break;
        case 's': /* goal = spk */
            if(stage == 1){/* first stage of spkmeans */
                /* calculates the vectors that will be the input for kmeans++ algorithm: */
                projectMatrixSpk(*matrixPtr, k);
                break;
            }
            kmeans(*matrixPtr, k, n_const, d_const); /* second stage of spkmeans - actual call to kmeans */
            break;
        default:
            print_error();
            return 1;
    }
    return 0;
}



/*
 * UTILS:
 */

/*
 * @TODO: add allocation validation
 */

/* this function validates that the goal input is legal*/
int validateGoal(char *goal) {
    if (
            strcmp(goal, "ddg") == 0 ||
            strcmp(goal, "lnorm") == 0 ||
            strcmp(goal, "jacobi") == 0 ||
            strcmp(goal, "wam") == 0 ||
            strcmp(goal, "spk") == 0
            ) {
        return True;
    }
    return False;
}

/* this function takes a matrix and checks if it's symmetric */
int validateMatrixSymmetric(Matrix matrix, int rows, int cols) {
    int i = 0, j = 0;
    /* if the matrix is not square: */
    if (rows != cols) {
        return False;
    }
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            if (matrix[i][j] != matrix[j][i]) {
                return False;
            }
        }
    }
    return True;
}


double sum(Vector vector, int n) {
    int i = 0;
    double res = 0;

    for (i = 0; i < n; i++) {
        res += vector[i];
    }
    return res;
}


double distance(Vector vector1, Vector vector2) {
    double res = 0;
    int i = 0;

    for (i = 0; i < d_const; i++) {
        res += (vector1[i] - vector2[i]) * (vector1[i] - vector2[i]);
    }
    return sqrt(res);
}


Vector getInverseMainDiagonal(Matrix ddg) {
    int i = 0;
    Vector res = allocateVector(n_const, sizeof(double), True);

    for (i = 0; i < n_const; i++) {
        res[i] = 1 / sqrt(ddg[i][i]);
    }
    return res;
}

Vector getMainDiagonal(Matrix mat) {
    int i = 0;
    Vector res = allocateVector(n_const, sizeof(double), True);
    for (i = 0; i < n_const; i++) {
        res[i] = mat[i][i];
    }
    return res;
}


/* get a string of filename and returns the number of chars in the first line */
int getLineSize(char *filename) {
    FILE *fp = NULL;
    int count = 0;
    char c = 'a';

    fp = fopen(filename, "r");
    if (fp == NULL) {
        if (DEBUG) {
            printf("in 'getLineSize', fp is null");
        }
        print_error();
        return False;
    }

    c = fgetc(fp);
    while (c != EOF && c != '\n') {
        c = fgetc(fp);
        count++;
    }
    fclose(fp);
    return count;
}


int getNumOfLines(char *filename) {
    FILE *fp = NULL;
    int count = 0;
    int c = 0;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        if (DEBUG == 1) {
            printf("in 'getNumOfLines', fp is null\n");
        }
        print_error();
        return False;
    }

    while ((c = fgetc(fp)) != EOF) {
        count += (c == '\n'); /* check if c equals '\n' */
    }
    fclose(fp);
    return count;
}


/* returns number of numbers in first line of file */
int get_dimension(char *in_file_path, int line_size) {
    FILE *fp = NULL;
    char *line = NULL;
    int counter = 1;
    int i = 0;

    fp = fopen(in_file_path, "r");
    if (fp == NULL) {
        if (DEBUG == 1) {
            printf("in 'get_dimension', fp is null\n");
        }
        print_error();
        return False;
    }
    line = (char *) calloc(line_size, sizeof(char));
    if (line == NULL) {
        if (DEBUG) {
            printf("in 'get_dimension', line is null\n");
        }
        print_error();
        fclose(fp);
        return False;
    }
    if (fgets(line, line_size, fp) == NULL) {
        fclose(fp);
        if (DEBUG == 1) {
            printf("in 'get_dimension', fgets == null\n");
        }
        print_invalid_input();
        free(line);
        return False;
    }

    for (i = 0; line[i] != '\0'; i++) {
        if (line[i] == ',') {
            counter++;
        }
    }
    fclose(fp);
    free(line);
    return counter;
}


int *getLinesLengths(int num_lines, char *filename) {
    int *linesLengths = NULL;
    FILE *fp = NULL;
    int i = 0;
    char c = 0;
    int line_len = 0;

    linesLengths = (int *) calloc(num_lines, sizeof(int));
    fp = fopen(filename, "r");
    if (fp == NULL) {
        if (DEBUG == 1) {
            printf("in 'getLinesLengths', fp is null\n");
        }
        print_error();
        return NULL;
    }
    if (linesLengths == NULL) {
        if (DEBUG == 1) {
            printf("in 'getLinesLengths', linesLengths is null\n");
        }
        fclose(fp);
        print_error();
        return NULL;
    }
    registerPtr(linesLengths);
    for (i = 0; i < num_lines; i++) {
        c = 0;
        line_len = 1;
        while ((c = fgetc(fp)) != EOF) {
            if (c == '\n') {
                linesLengths[i] = line_len;
                break;
            } else { line_len++; }
        }
    }
    fclose(fp);
    return linesLengths;
}


/* turns string of comma seperated numbers to array of doubles */
Vector str_to_vec(char *line, int line_len, int d) {
    Vector res = NULL;
    char *line_copy = NULL;
    const char s[2] = ",";
    char *token = NULL;
    int i = 0;

    res = allocateVector(d, sizeof(double), True);
    line_copy = (char *) calloc(line_len + 1, sizeof(char));
    if (!line_copy) {
        exit(1);
    }
    strcpy(line_copy, line);
    token = strtok(line_copy, s);
    while (token != NULL) {
        res[i] = atof(token);
        i++;
        token = strtok(NULL, s);
    }
    free(line_copy);
    return res;
}


int readData(char *in_file_path, Matrix *vectors) {
    int line_size = 0;
    int i = 0;
    char *line = NULL;
    int *lines_length = NULL;
    FILE *fp = NULL;
    double *vec = NULL;


    line_size = getLineSize(in_file_path);
    if (!line_size) {
        return False;
    }
    n_const = getNumOfLines(in_file_path);
    if (!n_const) {
        return False;
    }
    d_const = get_dimension(in_file_path, line_size);
    if (!d_const) {
        if (DEBUG == 1) {
            printf("in 'readData', invalid input\n");
        }
        print_invalid_input();
        return False;
    }
    *vectors = (double **) calloc(n_const, sizeof(double *));
    registerPtr(*vectors);
    if (*vectors == NULL) {
        if (DEBUG == 1) {
            printf("in 'readData', *vectors == NULL");
        }
        print_error();
        return False;
    }
    line = (char *) calloc(line_size + 1, sizeof(char));
    if (line == NULL) {
        if (DEBUG) {
            printf("in 'get_dimension', line is null\n");
        }
        print_error();
        return False;
    }
    lines_length = getLinesLengths(n_const, in_file_path);
    if (!lines_length) {
        return False;
    }
    fp = fopen(in_file_path, "r");
    if (fp == NULL) {
        if (DEBUG == 1) {
            printf("in 'readData', fp is null\n");
        }
        print_error();
        return False;
    }

    for (i = 0; i < n_const; i++) {

        (fgets(line, lines_length[i] + 1, fp));

        vec = str_to_vec(line, lines_length[i], d_const);

        free(line);
        if (i < (n_const - 1)) {
            line = (char *) calloc(lines_length[i + 1] + 1, sizeof(char));

        }
        (*vectors)[i] = vec;
    }
    fclose(fp);
    return True;
}

double offSquared(Matrix mat, int n) {
    int i = 0, j = 0;
    double res = 0;

    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            if (i != j) {
                res += pow(mat[i][j], 2);
            }
        }
    }
    return res;
}

int isConvergedJacobi(Matrix mat, Matrix matPrime) {
    return (offSquared(mat, n_const) - offSquared(matPrime, n_const) <= EPSILON);
}

Matrix getIdentityMat(int dim) {
    int i = 0;
    Matrix res = allocateMatrix(dim, dim, True);
    for (i = 0; i < dim; i++) {
        res[i][i] = 1;
    }
    return res;
}

int determineK(Vector eigenVals){
    int i = 0, res = 0;
    double maxDiff = 0.0, currDiff = 0.0;
    Vector sortedEigenVals = allocateVector(n_const, sizeof(double), True);
    int* SortedIndices = getSortedIndices(eigenVals);
    for (i = 0; i < n_const; i++){
        sortedEigenVals[i] = eigenVals[SortedIndices[i]];
    }

    for (i = 0; i < n_const/2 ; i++){
        currDiff = fabs(sortedEigenVals[i] - sortedEigenVals[i+1]);
        if (currDiff > maxDiff){
            maxDiff = currDiff;
            res = i + 1;
        }
    }
    return res;
}


/* execution functions: */

Matrix executeWam(Matrix vectors) {
    Matrix res = allocateMatrix(n_const, n_const, True);
    int i = 0, j = 0;
    for (i = 0; i < n_const; i++) {
        for (j = 0; j < n_const; j++) {
            res[i][j] = (i != j) ? exp(-0.5 * distance(vectors[i], vectors[j])) : 0;
        }
    }
    return res;
}


Matrix executeDdg(Matrix vectors, Matrix wam) {
    int i = 0;
    Matrix res = allocateMatrix(n_const, n_const, True);
    if (!wam) {
        wam = executeWam(vectors);
    }
    for (i = 0; i < n_const; i++) {
        res[i][i] = sum(wam[i], n_const);
    }

    return res;
}


Matrix executeLnorm(Matrix vectors) {
    Matrix wam = executeWam(vectors);
    Matrix ddg = executeDdg(vectors, wam);
    Vector inverseMainDiagonal = getInverseMainDiagonal(ddg);

    Matrix res = allocateMatrix(n_const, n_const, True);


    int i = 0, j = 0;
    for (i = 0; i < n_const; i++) {
        for (j = 0; j < n_const; j++) {
            res[i][j] = ((i == j) ? 1 : 0) - wam[i][j] * inverseMainDiagonal[i] * inverseMainDiagonal[j];
        }
    }
    return res;
}


/* return rotated mat and update (in place) cumulative p to new cumulative p */
void doJacobiIteration(Matrix mat, Matrix cumPum);

/*
 * This function calculates the row and column (i and j) of the off-diagonal element
 * with the largest absolute value, and returns them in a double pointer
 */
Vector getIandJ(Matrix mat, int dim) {
    int a = 0, b = 0;
    double largestAbsValue = -1;
    Vector res = allocateVector(2, sizeof(double), True); /* the vector that holds the indices i and j */
    for (a = 0; a < dim; a++) {
        for (b = a+1; b < dim; b++) {
            if (a != b && fabs(mat[a][b]) > largestAbsValue) {
                largestAbsValue = fabs(mat[a][b]);
                /* assign the indices to i and j respectively: */
                res[0] = a;
                res[1] = b;
            }
        }
    }
    return res;
}

/*
 * this function calculates the values of c and s parameters of the rotation matrix
 * based on the known formulas, and returns them in a double pointer
 */
Vector getCandS(Matrix mat, int i, int j) {
    Vector res = allocateVector(2, sizeof(double), True);

    double theta = (mat[j][j] - mat[i][i]) / (2 * mat[i][j]);
    double signTheta = theta < 0 ? -1 : 1;
    double t = signTheta / (fabs(theta) + sqrt(pow(theta, 2) + 1));
    double c = 1.0 / sqrt(pow(t, 2) + 1);
    double s = t * c;
    res[0] = c;
    res[1] = s;
    return res;
}

/* creates a new copy of a matrix */
Matrix copyOf(Matrix mat, int dim) {
    int i = 0, j = 0;
    Matrix res = allocateMatrix(dim, dim, False);
    for (i = 0; i < dim; i++) {
        for (j = 0; j < dim; j++) {
            res[i][j] = mat[i][j];
        }
    }
    return res;
}


void rotateMat(Matrix mat, int dim, int i, int j, double c, double s) {
    int r = 0;
    Matrix copy = copyOf(mat, dim);
    for (r = 0; r < dim; r++) {
        mat[r][i] = mat[i][r] = c * copy[r][i] - s * copy[r][j];
        mat[r][j] = mat[j][r] = c * copy[r][j] + s * copy[r][i];
    }
    mat[i][i] = pow(c, 2) * copy[i][i] + pow(s, 2) * copy[j][j] - 2 * s * c * copy[i][j];
    mat[j][j] = pow(s, 2) * copy[i][i] + pow(c, 2) * copy[j][j] + 2 * s * c * copy[i][j];
    mat[i][j] = mat[j][i] = 0;
    freeMatrix(copy, dim);
}

void doJacobiIteration(Matrix mat, Matrix cumPum) {
    int i, j;
    double c, s;
    Vector cAndS = NULL;
    Vector iAndJ = getIandJ(mat, n_const);
    i = (int) iAndJ[0];
    j = (int) iAndJ[1];
    cAndS = getCandS(mat, i, j);
    c = cAndS[0];
    s = cAndS[1];
    updateCumP(cumPum, n_const, i, j, c, s);
    rotateMat(mat, n_const, i, j, c, s);
    return;
}

/* updates by mutation cumulative multiplication of P's */
void updateCumP(Matrix cumP, int dim, int i, int j, double c, double s) {
    int count = 0;
    Matrix copy = copyOf(cumP, dim);
    for (count = 0; count < dim; count++) {
        cumP[count][i] = (c * copy[count][i]) - (s * copy[count][j]);
    }
    for (count = 0; count < dim; count++) {
        cumP[count][j] = (s * copy[count][i]) + (c * copy[count][j]);
    }
    freeMatrix(copy, dim);
}

/*
* This function takes two matrices and copies the content of one to the other
*/
void copyInPlace(Matrix mat, Matrix copiedMat, int dim) {
    int i = 0, j = 0;
    for (i = 0; i < dim; i++) {
        for (j = 0; j < dim; j++) {
            mat[i][j] = copiedMat[i][j];
        }
    }
}

/* swaps two ints */
void swap(int *xp, int *yp)
{
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}

/* return vector of indices sorted by decreasing order of keys vector */
int* getSortedIndices(Vector keys ){
    int i = 0, j = 0;
    int* indices = allocateVector(n_const, sizeof (int), True);
    for (i = 0; i < n_const; i++){
        indices[i] = i;
    }
    for (i = 0; i < n_const; i++){ /* stable bubble sort */
        for (j = 0; j < n_const - 1; j++){
            if (keys[(int)indices[j]] < keys[(int)indices[j+1]]){
                swap((int*) &indices[j], (int*) &indices[j+1]);
            }
        }
    }
    return indices;
}

Matrix getSubMatrix(JacobiRes jacobi, int k){
    int i = 0, j = 0;
    Matrix res = allocateMatrix(n_const, k, True);
    int * sortedIndices = getSortedIndices(jacobi.eigenVals);
    for (i = 0; i < n_const ; i++){
        for (j = 0; j < k; j++){
            res[i][j] = jacobi.eigenVecs[i][sortedIndices[j]];
        }
    }
    return res;
}




JacobiRes executeJacobi(Matrix vectors) {
    int i = 0;
    int rotations_converged = (offSquared(vectors, n_const) == 0); /* determines if mat is diagonal */
    JacobiRes res = {.eigenVals = NULL, .eigenVecs = NULL};
    Matrix A = vectors;
    Matrix A_prime = copyOf(A, n_const);
    Matrix cumPum = getIdentityMat(n_const);
    registerPtr(A_prime);

    /* main loop: */
    for (i = 0; i < MAX_ROTATIONS && !rotations_converged; i++) {
        doJacobiIteration(A_prime, cumPum);
        rotations_converged = isConvergedJacobi(A, A_prime);
        copyInPlace(A, A_prime, n_const);
    }
    res.eigenVals = getMainDiagonal(A_prime);
    res.eigenVecs = cumPum;
    return res;
}

void reNormalize(Matrix vectors, int rows, int cols){
    int i = 0, j = 0;
    double rowNorm = 0.0;
    for (i = 0; i < rows; i++){
        rowNorm = norm(vectors[i], cols);

        for (j = 0; j < cols; j++){
            vectors[i][j] = rowNorm == 0? 0 : vectors[i][j]/rowNorm;
        }
    }
}

void projectMatrixSpk(Matrix vectors, int k) {
    Matrix res = NULL, subMatrix = NULL;
    JacobiRes jacobi = {NULL, NULL};
    res = executeLnorm(vectors);
    jacobi = executeJacobi(res);
    if (!k){
        k = determineK(jacobi.eigenVals);
    }
    subMatrix = getSubMatrix(jacobi, k);
    reNormalize(subMatrix, n_const, k);
    writeMatrixToFile(subMatrix, "c_output_file.txt", n_const, k);
    return;
}
