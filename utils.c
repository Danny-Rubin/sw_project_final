#include "utils.h"



/* linked list for memory management */
struct node *memoryListHead = NULL;


/* Memory management functions: */


/* add memory ptr at first position at list */
void registerPtr(int key, void *memory) {
    struct node *link = (struct node *) malloc(sizeof(struct node));
    if (!link) { freeAllMemory(memoryListHead); }
    link->key = key;
    link->memory = memory;
    link->next = memoryListHead;
    memoryListHead = link;
    memoryListHead = link;
}


void freeAllMemory() {
    struct node *tmp;

    while (memoryListHead != NULL) {
        tmp = memoryListHead;
        memoryListHead = memoryListHead->next;
        free(tmp->memory); /* free memory */
        free(tmp); /* free list link  */
    }
    // system("leaks executablename");
}
/*
 * This function allocates a vector in memory
 * Params:
 * n- vector dimension,
 * elementSize- size of each element in the vector
 * shouldRegister (boolean)- determines if the vector will be saved in the general memory linked list
 */
void *allocateVector(int n, size_t elementSize, int shouldRegister) {
    Vector res = (Vector) calloc(n, elementSize);
    if (!res) {
        print_error();
        exit(1);
    }
    if (shouldRegister) {
        registerPtr(0, res);
    }
    return res;
}

/*
 * This function allocates a matrix in memory
 * Params:
 * rows and cols - desired mat dimensions,
 * shouldRegister (boolean)- determines if the matrix will be saved in the general memory linked list
 */
Matrix allocateMatrix(int rows, int cols, int shouldRegister) {
    int i = 0;
    Matrix res = (Matrix) calloc(rows, sizeof(Vector));
    if (!res) {
        print_error();
        exit(1);
    }
    if (shouldRegister) {
        registerPtr(0, res);
    }
    for (i = 0; i < rows; i++) {
        res[i] = allocateVector(cols, sizeof(double), shouldRegister);
        if (!res[i]) {
            return NULL;
        }
    }
    return res;
}

void freeMatrix(Matrix mat, int n) {
    int i = 0;
    for (i = 0; i < n; i++) {
        free(mat[i]);
    }
    free(mat);
}

void free_memory_arr(double** arr, int n){
    int i = 0;
    for(i = 0; i < n; i++){
        free(arr[i]);
    }
    free(arr);
}

/* Priniting, writing and formatting: */

void print_error() {
    printf("An Error Has Occurred\n");
}


void print_invalid_input() {
    printf("Invalid Input!\n");
}


int writeData(char *out_file, char ***vecStrs, int cols, int rows){
    FILE *fp = NULL;
    int i = 0; int j = 0;

    fp = fopen(out_file, "w");
    if (fp ==NULL){
        if (DEBUG) {
            printf("error in writeData: fp is null\n");}
        print_error();
        exit(1);
    }
    for(i=0; i < rows; i++){
        for(j=0; j < cols - 1; j++){
            fprintf(fp, "%s,", vecStrs[i][j]);
        }
        fprintf(fp, "%s\n", vecStrs[i][cols - 1]);
    }
    fclose(fp);
    return True;
}

char *doubleToRoundStr(double num) {
    char *res = allocateVector(400, sizeof(char), True);
    sprintf(res, "%.4f", num);
    return res;
}

/* This function takes a matrix of doubles and converts
* it to a matrix of strings, formatted to 4 decimal places.
*/
char ***doubleVecsToStr(Matrix vectors, int rows, int cols) {
    char ***res = NULL;
    int i = 0;
    int j = 0;

    res = (char ***) calloc(rows, sizeof(char **));
    if (res == NULL) {
        if (DEBUG) {
            printf("error in doubleVecsToSt: res is null\n");
        }
        print_error();
        exit(1);
    }
    registerPtr(0, res);
    for (i = 0; i < rows; i++) {
        res[i] = allocateVector(cols, sizeof(char *), True);
    }
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            char *val = doubleToRoundStr(vectors[i][j]);
            res[i][j] = val;
        }
    }
    return res;
}

void writeMatrixToFile(Matrix vectors, char *outFileName, int rows, int cols){
    char ***StrMat = doubleVecsToStr(vectors, rows, cols);
//     printf("finished double vecs to str \n");
        if(!StrMat){
            exit(1);
        }
    writeData(outFileName, StrMat, cols, rows);
    // printf("finished write data\n");
}

/* This function takes a matrix of numbers as formatted strings and prints
their value to screen */
int PrintData(char ***vec_strs, int rows, int cols) {
    int i = 0;
    int j = 0;
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols - 1; j++) {
            printf("%s,", vec_strs[i][j]);
        }
        printf("%s\n", vec_strs[i][cols - 1]);
    }
    return True;
}


void printDoubleMatrix(Matrix mat, int rows, int cols) {
    // printf("in printDOUBLE %f %f %f %f\n", mat[0][0],mat[0][1],mat[1][0],mat[1][1]);
    char ***matstr = doubleVecsToStr(mat, rows, cols);
    // printf("in mATSTR %s %s %s %s\n", matstr[0][0],matstr[0][1],matstr[1][0],matstr[1][1]);
    PrintData(matstr, rows, cols);
}

double norm(Vector vec, int n){
    int i = 0;
    double sum = 0;
    for(i = 0; i < n; i++){
        sum += pow(vec[i], 2);
    }
    return sqrt(sum);
}