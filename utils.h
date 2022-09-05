#ifndef UTILS
#define UTILS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define DEBUG 1
#define True 1
#define False 0
#define MAX_ITER 3000

typedef double **Matrix;
typedef double *Vector;
typedef struct JacobiRes {
    Vector eigenVals;
    Matrix eigenVecs;
} JacobiRes;


struct node {
    void *memory;
    struct node *next;
};


/* Functions declartions: */
void freeAllMemory();

void print_error();

void print_invalid_input();

void registerPtr(void *memory);

void *allocateVector(int n, size_t elementSize, int shouldRegister);

Matrix allocateMatrix(int rows, int cols, int shouldRegister);

void freeMatrix(Matrix mat, int n);

void print_invalid_input();

int writeData(char *out_file, char ***vecStrs, int cols, int rows);

char *doubleToRoundStr(double num);

char ***doubleVecsToStr(Matrix vectors, int rows, int cols);

void writeMatrixToFile(Matrix vectors, char *outFileName, int rows, int cols);

int PrintData(char ***vec_strs, int rows, int cols);

void printDoubleMatrix(Matrix mat, int rows, int cols);

void free_memory_arr(double** arr, int n);

double norm(Vector vec, int n);



#endif
