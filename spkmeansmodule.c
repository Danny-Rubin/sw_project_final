#include <math.h>
#include <stdlib.h>

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

