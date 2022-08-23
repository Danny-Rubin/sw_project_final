#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "spkmeansmodule.h"

#define DEBUG 1
#define DEFAULT_MAX_ITER 300
#define MAX_ROTATIONS 100
#define EPSILON 0.00001
#define True 1
#define False 0

/* global variables of the program: number of vectors, number of clusters and input vectors dimension */

int n_const = 0;
int k_const = 0;
int d_const = 0;

typedef double** Matrix;
typedef double* Vector;
typedef struct JacobiRes {Vector eigenVals; Matrix eigenVecs;} JacobiRes;



/*
 * python entry point function:
 * only function integrated with python interface
 * params: filename, k, goal
 * what it does: call general entry point after converting python args
 */




/*
 * c entry point function:
 * params: filename, k, goal
 * what it does: reads input file and calls appropriate execution function and then print to screen result
 */



int readData(char *in_file_path, Matrix *vectors);
Matrix executeWam(Matrix vectors);
Matrix executeDdg(Matrix vectors, Matrix wam);
Matrix executeLnorm(Matrix vectors);
JacobiRes executeJacobi(Matrix vectors);
void executeSpk(Matrix vectors);
void printDoubleMatrix(Matrix mat, int rows, int cols);
int PrintData(char ***vec_strs, int rows, int cols);
int validateGoal(char *goal);
int validateMatrixSymmetric(Matrix matrix, int rows, int cols);
int isConvergedJacobi(Matrix mat, Matrix matPrime);


Matrix getIdentityMat(int dim);
// return rotated mat and mutate cumpum to new cumpum
Matrix doJacobiIteration(Matrix mat, Matrix cumPum);
Vector getIandJ(Matrix mat, int dim);
Vector getCandS(Matrix mat, int dim, int i, int j);
void updateCumpum(Matrix cumPum, int dim, int i, int j, double c, double s);
Matrix rotateMat(Matrix mat, int dim, int i, int j, double c, double s);
Matrix copyOf(Matrix mat, int dim);




// "/Users/drubinov/Downloads/sw_project_final/inputs_hw1/input_1.txt"
int cEntryPoint(int k, char* goal, char * fileName){
    Matrix* matrixPtr = (Matrix *) calloc(sizeof (Matrix), 1); // @TODO: check if fails!!!
    JacobiRes jacobiRes;
    Matrix res = NULL;
    if (!validateGoal(goal)){
        return 1;
    }
    if(!readData(fileName, matrixPtr)){
        return 1;
    }
    switch (goal[0]) {
        case 'w':
            res = executeWam(*matrixPtr);
            printDoubleMatrix(res, n_const, n_const);
            break;
        case 'd':
            res = executeDdg(*matrixPtr, NULL);
            printDoubleMatrix(res, n_const, n_const);
            break;
        case 'l':
            res = executeLnorm(*matrixPtr);
            printDoubleMatrix(res, n_const, n_const);
            break;
        case 'j':
            if(!validateMatrixSymmetric(* matrixPtr, n_const, d_const)){ /* input matrix not symmetric*/
                print_invalid_input();
                return 1;
            }
            jacobiRes = executeJacobi(*matrixPtr);
            // @todo: implement and print
            break;
        case 's':
            executeSpk(*matrixPtr);
            // @todo: write to file
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

// @TODO: implement
/* this function validates that the goal input is legal*/
int validateGoal(char *goal);

/* this function takes a matrix and checks if it's symmetric */
int validateMatrixSymmetric(Matrix matrix, int rows, int cols){
    int i = 0, j = 0;
    /* if the matrix is not square: */
    if (rows != cols){
        return False;
    }
    for (i = 0; i < rows; i++){
        for (j = 0; j < cols; j++){
            if (matrix[i][j] != matrix[j][i]){
                return False;
            }
        }
    }
    return True;
}


Vector allocateVector(int n){
    Vector res = (Vector) calloc(n, sizeof (double));
    if (!res){
        print_error();
        return NULL;
    }
    return res;
}


Matrix allocateMatrix(int rows, int cols){
    Matrix res = (Matrix) calloc(rows, sizeof (Vector));
    if (!res){
        print_error();
        return NULL;
    }
    int i = 0;

    for(i = 0; i < rows; i++){
        res[i] = allocateVector(cols);
        if (!res[i]){
            return NULL;
        }
    }
    return res;
}


double sum(Vector vector, int n){
    int i = 0;
    double res = 0;

    for(i = 0; i < n; i++){
        res += vector[i];
    }
    return res;
}


double distance(Vector vector1, Vector vector2){
    double res = 0;
    int i = 0;

    for (i = 0; i < d_const; i++){
        res+= (vector1[i] - vector2[i]) * (vector1[i] - vector2[i]);
    }
    return sqrt(res);
}


Vector getInverseMainDiagonal(Matrix ddg){
    int i = 0;
    Vector res = allocateVector(n_const);

    for(i = 0; i < n_const; i++){
        res[i] = 1 / sqrt(ddg[i][i]);
    }
    return res;
}

Vector getMainDiagonal(Matrix mat){
    int i = 0;
    Vector res = allocateVector(n_const);
    for(i = 0; i < n_const; i++){
        res[i] = mat[i][i];
    }
    return res;
}


void print_error(){
    printf("An Error Has Occurred\n");
}


void print_invalid_input(){
    printf("Invalid Input!\n");
}


/* get a string of filename and returns the number of chars in the first line */
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


/* turns string of comma seperated numbers to array of doubles */
Vector str_to_vec(char *line,int line_len, int d){
    Vector res = NULL;
    char *line_copy = NULL;
    const char s[2] = ",";
    char *token = NULL;
    int i = 0;


    res = (Vector) calloc(d, sizeof(double));
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


int readData(char *in_file_path, Matrix *vectors){
    int line_size = 0;
    int i = 0;
    char *line = NULL;
    int *lines_length = NULL;
    FILE *fp = NULL;
    double *vec = NULL;


    line_size = getLineSize(in_file_path);
    if (!line_size){
        return False;
    }
    n_const = getNumOfLines(in_file_path);
    if (!n_const){
        return False;
    }
    d_const = get_dimension(in_file_path, line_size);
    if(!d_const){
        if (DEBUG == 1){
            printf("in 'readData', invalid input\n");
        }
        print_invalid_input();
        return False;
    }
    *vectors = (double **)calloc(n_const, sizeof(double  *));
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
    lines_length = getLinesLengths(n_const, in_file_path);
    if (!lines_length){
        return False;
    }
    fp = fopen(in_file_path, "r");
    if (fp == NULL){
        if (DEBUG == 1){
            printf("in 'readData', fp is null\n");
        }
        print_error();
        return False;
    }

    for(i=0; i < n_const; i++){

        (fgets(line, lines_length[i] + 1, fp));

        vec = str_to_vec(line, lines_length[i], d_const);

        free(line);
        if (i < (n_const - 1)){
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

/* This function takes a matrix of doubles and converts
* it to a matrix of strings, formatted to 4 decimal places.
*/
char ***doubleVecsToStr(Matrix vectors, int rows, int cols){
    char ***res = NULL;
    int i = 0;
    int j = 0;

    res = (char ***) calloc(k_const, sizeof(char **));
    if (res == NULL){
        if (DEBUG) {
            printf("error in doubleVecsToSt: res is null\n");}
        print_error();
        return NULL;
    }
    for(i=0; i < rows; i++) {
        res[i] = (char **) calloc(cols, sizeof (char*));

        if (res[i] == NULL){
            if (DEBUG) {printf("error in doubleVecsToStr: res[%d] is null\n", i);}

            for(j = 0; j < i; j++){
                free(res[j]);
            }
            free(res);
            print_error();
            return NULL;
        }
    }
    for(i=0; i < rows; i++){
        for(j=0; j < cols; j++) {
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

/* This function takes a matrix of numbers as formatted strings and prints
their value to screen */
int PrintData(char ***vec_strs, int rows, int cols){
    int i = 0; int j = 0;
    for(i=0; i < rows; i++){
        for(j=0; j < cols -1; j++){
            printf("%s,", vec_strs[i][j]);
        }
        printf("%s\n", vec_strs[i][cols-1]);
    }
    return True;
}


void printDoubleMatrix(Matrix mat, int rows, int cols){
    char *** matstr = doubleVecsToStr(mat, rows, cols);
    PrintData(matstr,rows, cols);
}

double offSquared(Matrix mat, int n){
    int i = 0, j = 0;
    double res = 0;

    for (i=0 ; i < n; i++){
        for (j=0;j < n; j++){
            if (i != j)
                res += pow(mat[i][j],2);
        }
    }
    return res;
}

int isConvergedJacobi(Matrix mat, Matrix matPrime){
    // @todo: pud the pudder
    return (offSquared(mat, n_const) - offSquared(matPrime, n_const) <= EPSILON);
}

Matrix getIdentityMat(int dim){
    int i = 0;
    Matrix res = allocateMatrix(dim, dim);
    for(i = 0; i < dim; i++){
        res[i][i] = 1;
    }
    return res;
}


/* execution functions: */

Matrix executeWam(Matrix vectors){
    Matrix res = allocateMatrix(n_const, n_const);
    int i = 0, j =0;
    for(i = 0; i < n_const; i++){
        for(j = 0; j < n_const; j++){
            res[i][j] = (i != j) ? exp( -0.5 * distance(vectors[i], vectors[j]) ) : 0;
        }
    }
    return res;
}


Matrix executeDdg(Matrix vectors, Matrix wam){
    int i = 0;
    Matrix res = allocateMatrix(n_const, n_const);
    if(!wam){
        wam = executeWam(vectors);
    }
    for(i = 0; i < n_const; i++){
        res[i][i] = sum(wam[i], n_const);
    }

    return res;
}


Matrix executeLnorm(Matrix vectors){
    Matrix wam = executeWam(vectors);
    Matrix ddg = executeDdg(vectors, wam);
    Vector inverseMainDiagonal = getInverseMainDiagonal(ddg);

    Matrix res = allocateMatrix(n_const, n_const);


    int i = 0, j =0;
    for(i = 0; i < n_const; i++){
        for(j = 0; j < n_const; j++){
            res[i][j] = ((i != j) ? 1 : 0) - wam[i][j] * inverseMainDiagonal[i] * inverseMainDiagonal[j];
        }
    }
    return res;
}


Matrix getIdentityMat(int dim);

// return rotated mat and mutate cumpum to new cumpum
Matrix doJacobiIteration(Matrix mat, Matrix cumPum);

/*
 * This function calculates the row and column (i and j) of the off-diagonal element
 * with the largest absolute value, and returns them in a double pointer
 */
Vector getIandJ(Matrix mat, int dim){
    int a = 0, b = 0;
    double largestAbsValue = 0;
    Vector res = allocateVector(2); /* the vector that holds the indices i and j */
    for (a = 0; a < dim; a++){
        for (b = 0; b < dim; b++){
            if (a != b && fabs(mat[a][b]) > largestAbsValue){
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
Vector getCandS(Matrix mat, int dim, int i, int j){
    Vector res = allocateVector(2);
    double theta = (mat[j][j]- mat[i][i])/(2 * mat[i][j]);
    int signTheta = theta < 0 ? -1 : 1;
    double t =  signTheta / (fabs(theta) + sqrt(pow(theta,2) + 1));
    double c = 1 / sqrt(pow(t,2) + 1);
    double s = t * c;
    res[0] = c;
    res[1] = s;
    return res;
}
/* creates a copy of a symmetric matrix */
Matrix copyOf(Matrix mat, int dim){
    int i = 0, j = 0;
    Matrix res = allocateMatrix(dim, dim);
    for (i = 0; i < dim; i++){
        for (j = 0; j < dim; j++){
            res[i][j] = mat[i][j];
        }
    }
    return res;
}


Matrix rotateMat(Matrix mat, int dim, int i, int j, double c, double s);

Matrix doJacobiIteration(Matrix mat, Matrix cumPum){
    int i, j;
    double c, s;
    Vector iAndJ = getIandJ(mat, n_const);
    i = (int) iAndJ[0];
    j = (int) iAndJ[1];
    int * cAndS = getCandS(mat, n_const);
    c = cAndS[0];
    s = cAndS[1];
    updateCumpum(cumPum, n_const, i, j, c, s);
    return rotateMat(mat, n_const, i, j, c, s);
}

void updateCumpum(Matrix cumPum, int dim, int i, int j, double c, double s){
    int count = 0;
    Matrix res = cumPum;
    for(count = 0; count < dim; count++){
        res[count][i] = (c * res[count][i]) - (s * res[count][j]);
    }
    for(count = 0; count < dim; count++){
        res[count][j] = (s * res[count][i]) - (s * res[count][j]);
    }

}


JacobiRes executeJacobi(Matrix vectors){
    int i = 0, rotations_converged = 0;
    Matrix A = vectors;
    Matrix A_prime = NULL;
    // main loop:
    for(; i < MAX_ROTATIONS && !rotations_converged; i++){
        A_prime = doJacobiIteration(A);
        rotations_converged = isConvergedJacobi(A, A_prime);
        A = A_prime;
    }



}

void executeSpk(Matrix vectors);



