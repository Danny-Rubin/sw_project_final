#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DEBUG 0
#define DEFAULT_MAX_ITER 300
#define EPSILON 0.00001
#define True 1
#define False 0

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

/* global variables of the program: number of vectors, number of clusters and input vectors dimension */

int n_const = 0;
int k_const = 0;
int d_const = 0;

typedef double** Matrix;
typedef double* Vector;


/*
 * UTILS:
 */

/*
 * @TODO: add allocation validation
 */


Vector allocateVector(int n){
    Vector res = (Vector) calloc(n, sizeof (double));
    return res;
}


Matrix allocateMatrix(int rows, int cols){
    Matrix res = (Matrix) calloc(rows, sizeof (Vector));
    int i = 0;

    for(i = 0; i < rows; i++){
        res[i] = allocateVector(cols);
    }
    return res;
}


double sum(Vector vector, int n){
    int i = 0, res = 0;
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


int readData(char *in_file_path, double ***vectors){
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
char ***doubleVecsToStr(Matrix vectors, int d, int k){
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
    for(i=0; i < k_const; i++) {
        res[i] = (char **) calloc(d_const, sizeof (char*));

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
    for(i=0; i < k_const; i++){
        for(j=0; j < d_const; j++) {
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
int PrintData(char ***vec_strs){
    int i = 0; int j = 0;
    for(i=0; i < k_const; i++){
        for(j=0; j < d_const -1; j++){
            printf("%s,", vec_strs[i][j]);
        }
        printf("%s\n", vec_strs[i][d_const-1]);
    }
    return True;
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
    if(!wam){
        wam = executeWam(vectors);
    }

    Matrix res = allocateMatrix(n_const, n_const);
    int i = 0;

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

