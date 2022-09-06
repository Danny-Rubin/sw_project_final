#include "utils.h"

/* function signatures: */

int cEntryPoint(int k, char *goal, char *fileName, int stage);

int readData(char *in_file_path, Matrix *vectors);

void writeMatrixToFile(Matrix vectors, char *outFileName, int rows, int cols);

int determineK(Vector eigenVals);

Matrix executeWam(Matrix vectors);

Matrix executeDdg(Matrix vectors, Matrix wam);

Matrix executeLnorm(Matrix vectors);

JacobiRes executeJacobi(Matrix vectors);

void reNormalize(Matrix vectors, int rows, int cols);

void projectMatrixSpk(Matrix vectors, int k);

int PrintData(char ***vec_strs, int rows, int cols);

int validateGoal(char *goal);

int validateMatrixSymmetric(Matrix matrix, int rows, int cols);

int isConvergedJacobi(Matrix mat, Matrix matPrime);

Matrix getIdentityMat(int dim);

void doJacobiIteration(Matrix mat, Matrix cumPum);

Vector getIandJ(Matrix mat, int dim);

Vector getCandS(Matrix mat, int i, int j);

void updateCumP(Matrix cumP, int dim, int i, int j, double c, double s);

void rotateMat(Matrix mat, int dim, int i, int j, double c, double s);

Matrix copyOf(Matrix mat, int dim);

Matrix getSubMatrix(JacobiRes jacobi, int k);

int* getSortedIndices(Vector keys );

int validateProgramArgs(int argc);
