#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include "spkmeans.h"

int translate_vectors(PyObject *data, vector **datapoints, int amount, int d);

PyObject *pack(double **datapoints, int amount, int d);

static PyObject *fit_spk_capi(PyObject *self, PyObject *args);

static PyObject *fit_kms_capi(PyObject *self, PyObject *args);

static PyObject *fit_wam_capi(PyObject *self, PyObject *args);

static PyObject *fit_ddg_capi(PyObject *self, PyObject *args);

static PyObject *fit_lnorm_capi(PyObject *self, PyObject *args);

static PyObject *fit_jacobi_capi(PyObject *self, PyObject *args);


int translate_vectors(PyObject *data, vector **datapoints, int amount, int d) {
    /* this method receives a python object containing the datapoints in python form
     * and a pointer to a c object that is an array of vectors
     * amount is the number of datapoints in data
     * and d is the length of each datapoint
     * the method takes each datapoint from data and translates it into a c form
     * which it then inserts to the array datapoints
     * it returns 1 is an error occurred and 0 if not */
    double *args = NULL;
    PyObject *point = NULL;
    int i, j;
    for (i = 0; i < amount; i++) {  /* amount = n for points call and k for centroids call*/
        /* this loop goes through each datapoint in data and extracts it */
        point = PyList_GetItem(data, i);
        args = (double *) calloc(d, sizeof(double));
        /* allocation for the arguments of current datapoint being translated */
        if (args == NULL) {
            /* an error in memory allocation */
            return 1;
        }
        for (j = 0; j < d; j++) {
            /* each argument of the current datapoint is extracted from the python object
             * and inserted to its respective spot in a c object */
            args[j] = PyFloat_AsDouble(PyList_GetItem(point, j));
        }
        (*datapoints)[i].args = args;
    }
    /* if code got this far and nothing went wrong, the process is finished */
    /*free(args);*/
    return 0;
}

PyObject *pack(double **datapoints, int rows, int cols) {
    /* this method receives a c object and translates it into a python format
     * in order to send it back to python
     * datapoints are placed in rows and eigen vectors are placed in columns
     * so for datapoints and centroids, each row is one vector
     * the amount of vectors is rows and each vector is of length cols
     * but for eigen vectors, each column is one vector
     * the amount of vectors is cols and each vector is of length rows */
    int i = 0;
    int j = 0;
    PyObject *packed_datapoints = PyList_New((Py_ssize_t) rows);
    PyObject *datapoint;
    if (packed_datapoints == NULL) {
        /* in case an error occurred */
        return NULL;
    }
    for (i = 0; i < rows; i++) {
        /* each datapoint is translated into python form */
        datapoint = PyList_New((Py_ssize_t) cols);
        if (datapoint == NULL) {
            Py_XDECREF(packed_datapoints);
            return NULL;
        }
        for (j = 0; j < cols; j++) {
            PyList_SetItem(datapoint, j, PyFloat_FromDouble(datapoints[i][j]));
        }
        PyList_SetItem(packed_datapoints, i, datapoint);
    }
    return packed_datapoints;
}


static PyObject *fit_spk_capi(PyObject *self, PyObject *args) {
    /* this method receives the datapoints from python
     * and creates matrix T from the normalized spectral clustring algorithm
     * it returns matrix T as a python object */
    PyObject *python_datapoints = NULL, *python_t_mat = NULL;
    vector *datapoints = NULL;
    double **t_mat = NULL;
    int n, d, k, flag;
    /* flag helps to determine whether the translation from python to c object was successful */
    if (!PyArg_ParseTuple(args, "Oiii", &python_datapoints, &n, &d, &k)) {
        return NULL;
    }
    /* memory allocation for datapoints */
    datapoints = (vector *) calloc(n, sizeof(vector));
    if (datapoints == NULL) {
        return NULL;
    }
    flag = translate_vectors(python_datapoints, &datapoints, n, d);
    /* translating python_datapoints from python object to c object */
    /* and inserting it to the c object called datapoints */
    if (flag == 1) {
        /* if flag == 1, an error occurred and the program is terminated */
        free_vectors(&datapoints, n);
        return NULL;
    }
    t_mat = fit_spk_c(datapoints, &k, n, d);
    /* calls the method that finds T */
    if (t_mat == NULL) {
        free_vectors(&datapoints, n);
        return NULL;
    }
    python_t_mat = pack(t_mat, n, k);
    /* packs t_mat into a format of python object */
    if (python_t_mat == NULL) {
        free_vectors(&datapoints, n);
        free_double(&t_mat, n);
        return NULL;
    }
    free_vectors(&datapoints, n);
    free_double(&t_mat, n);
    return python_t_mat;
}

static PyObject *fit_kms_capi(PyObject *self, PyObject *args) {
    /* this method calls the k_means method */
    int flag;
    /* flag helps to determine whether the translation from python to c object was successful */
    PyObject *python_datapoints = NULL;
    PyObject *python_centpoints = NULL;
    /* python_datapoints is going to hold the python object datapoints from the fit_kms input and
     * python_centpoints is going to hold the python object first_centroids in fit_kms input
     * before "translation" into c objects */
    vector *datapoints = NULL;
    vector *centpoints = NULL;
    /* datapoints is going to hold the vectors translated from python_datapoints
     * centpoints is going to hold the vectors translated from python_centpoints
     * both as c objects after translation from python objects */
    double **centroids = NULL;
    /* centroids is going to hold the output of the c function k_means */
    int d, k, n, max_iter;
    /* d is the length of each datapoint, k is the number of clusters,
     * n is the number of datapoints, and max_iter is the maximal number of iterations
     * as received in the input from fit_kms */
    double epsilon;
    PyObject *packed_centroids = NULL;

    if (!PyArg_ParseTuple(args, "OidiiiO", &python_datapoints, &max_iter, &epsilon, &n, &d, &k, &python_centpoints)) {
        /* checking whether there's any error with receiving the input from the python program */
        /* and inserting them into respective c variables */
        return NULL;
    }
    /* memory allocation for datapoints and centpoints */
    datapoints = (vector *) calloc(n, sizeof(vector));
    if (datapoints == NULL) {
        return NULL;
    }
    centpoints = (vector *) calloc(k, sizeof(vector));
    if (centpoints == NULL) {
        free_vectors(&datapoints, n);
        return NULL;
    }
    flag = translate_vectors(python_datapoints, &datapoints, n, d);
    /* translating python_datapoints from python object to c object */
    /* and inserting it to the c object called datapoints */
    if (flag == 1) {
        /* if flag == 1, an error occurred and the program is terminated */
        free_vectors(&datapoints, n);
        free_vectors(&centpoints, k);
        return NULL;
    }
    flag = translate_vectors(python_centpoints, &centpoints, k, d);
    /* translating python_centpoints from python object to c object */
    /* and inserting it to the c object called centpoints */
    if (flag == 1) {
        /* if flag == 1, an error occurred and the program is terminated */
        free_vectors(&datapoints, n);
        free_vectors(&centpoints, k);
        return NULL;
    }
    centroids = k_means(datapoints, k, max_iter, epsilon, n, d, centpoints);
    /* calling the method k_means to obtain the centroids according to the specified algorithm */
    free_vectors(&datapoints, n);
    free_vectors(&centpoints, k);
    /* free memory of datapoints and centpoints before proceeding since they are no longer needed */
    if (centroids == NULL) {
        /* making sure no error has occurred */
        return NULL;
    }

    packed_centroids = pack(centroids, k, d);
    /* packaging the final centroids of this method into a python object to be outputted */
    if (packed_centroids == NULL) {
        /* if there was an error, all memory allocations should be freed before termination */
        free_double(&centroids, k);
        return NULL;
    }
    /* free memory */
    free_double(&centroids, k);
    return packed_centroids;
}

static PyObject *fit_wam_capi(PyObject *self, PyObject *args) {
    PyObject *python_datapoints = NULL, *python_w_mat = NULL;
    vector *datapoints = NULL;
    double **w_mat = NULL;
    int n, d, flag;
    /* flag helps to determine whether the translation from python to c object was successful */
    if (!PyArg_ParseTuple(args, "Oii", &python_datapoints, &n, &d)) {
        return NULL;
    }
    /* memory allocation for datapoints */
    datapoints = (vector *) calloc(n, sizeof(vector));
    if (datapoints == NULL) {
        return NULL;
    }
    flag = translate_vectors(python_datapoints, &datapoints, n, d);
    /* translating python_datapoints from python object to c object */
    /* and inserting it to the c object called datapoints */
    if (flag == 1) {
        /* if flag == 1, an error occurred and the program is terminated */
        free_vectors(&datapoints, n);
        return NULL;
    }
    w_mat = find_w(datapoints, n, d);
    /* calls a method to find wam */
    if (w_mat == NULL) {
        /* in case something went wrong */
        free_vectors(&datapoints, n);
        return NULL;
    }
    python_w_mat = pack(w_mat, n, n);
    /* packs w_mat into a format of python object */
    free_vectors(&datapoints, n);
    free_double(&w_mat, n);
    if (python_w_mat == NULL) {
        return NULL;
    }
    return python_w_mat;
}

static PyObject *fit_ddg_capi(PyObject *self, PyObject *args) {
    PyObject *python_datapoints = NULL, *python_d_mat = NULL;
    vector *datapoints = NULL;
    double **d_mat = NULL, **w_mat = NULL;
    int n, d, flag;
    /* flag helps to determine whether the translation from python to c object was successful */
    if (!PyArg_ParseTuple(args, "Oii", &python_datapoints, &n, &d)) {
        return NULL;
    }
    /* memory allocation for datapoints */
    datapoints = (vector *) calloc(n, sizeof(vector));
    if (datapoints == NULL) {
        return NULL;
    }
    flag = translate_vectors(python_datapoints, &datapoints, n, d);
    /* translating python_datapoints from python object to c object */
    /* and inserting it to the c object called datapoints */
    if (flag == 1) {
        /* if flag == 1, an error occurred and the program is terminated */
        free_vectors(&datapoints, n);
        return NULL;
    }
    w_mat = find_w(datapoints, n, d);
    /* finds wam in order to use it to find ddg */
    if (w_mat == NULL) {
        /* in case something went wrong */
        free_vectors(&datapoints, n);
        return NULL;
    }
    d_mat = find_d(w_mat, n);
    /* finds ddg */
    if (d_mat == NULL) {
        /* in case something went wrong */
        free_double(&w_mat, n);
        free_vectors(&datapoints, n);
        return NULL;
    }
    python_d_mat = pack(d_mat, n, n);
    /* packs d_mat into a form of python object */
    free_double(&w_mat, n);
    free_vectors(&datapoints, n);
    free_double(&d_mat, n);
    if (python_d_mat == NULL) {
        return NULL;
    }
    return python_d_mat;
}

static PyObject *fit_lnorm_capi(PyObject *self, PyObject *args) {
    PyObject *python_datapoints = NULL, *python_l_mat = NULL;
    vector *datapoints = NULL;
    double **l_mat = NULL;
    int n, d, flag;
    /* flag helps to determine whether the translation from python to c object was successful */
    if (!PyArg_ParseTuple(args, "Oii", &python_datapoints, &n, &d)) {
        return NULL;
    }
    /* memory allocation for datapoints */
    datapoints = (vector *) calloc(n, sizeof(vector));
    if (datapoints == NULL) {
        return NULL;
    }
    flag = translate_vectors(python_datapoints, &datapoints, n, d);
    /* translating python_datapoints from python object to c object */
    /* and inserting it to the c object called datapoints */
    if (flag == 1) {
        /* if flag == 1, an error occurred and the program is terminated */
        free_vectors(&datapoints, n);
        return NULL;
    }
    l_mat = find_l(datapoints, n, d);
    if (l_mat == NULL) {
        free_vectors(&datapoints, n);
        return NULL;
    }
    python_l_mat = pack(l_mat, n, n);
    free_vectors(&datapoints, n);
    free_double(&l_mat, n);
    if (python_l_mat == NULL) {
        return NULL;
    }
    return python_l_mat;
}


static PyObject *fit_jacobi_capi(PyObject *self, PyObject *args) {
    PyObject *python_datapoints = NULL, *python_eigenvalues = NULL, *python_eigenvectors = NULL, *python_eigen = NULL;
    vector *datapoints = NULL;
    eigen_vector *jacobi_eigen = NULL;
    double **data_mat = NULL, **ex1 = NULL, **ex2 = NULL;
    int n, d, flag;
    /* flag helps to determine whether the translation from python to c object was successful */
    if (!PyArg_ParseTuple(args, "Oii", &python_datapoints, &n, &d)) {
        /* checking whether there's any error with receiving the input from the python program */
        /* and inserting them into respective c variables */
        return NULL;
    }
    /* memory allocation for datapoints */
    datapoints = (vector *) calloc(n, sizeof(vector));
    if (datapoints == NULL) {
        return NULL;
    }
    flag = translate_vectors(python_datapoints, &datapoints, n, d);
    /* translating python_datapoints from python object to c object */
    /* and inserting it to the c object called datapoints */
    if (flag == 1) {
        /* if flag == 1, an error occurred and the program is terminated */
        free_vectors(&datapoints, n);
        return NULL;
    }
    data_mat = extract_mat_from_dp(datapoints, n, d);
    /* translates datapoints into a format of a 2-dim array */
    if (data_mat == NULL) {
        free_vectors(&datapoints, n);
        return NULL;
    }
    jacobi_eigen = find_j(data_mat, n);
    /* finds the eigen values and eigen vectors of data_mat */
    if (jacobi_eigen == NULL) {
        free_double(&data_mat, n);
        free_vectors(&datapoints, n);
        return NULL;
    }
    ex1 = extract_values(jacobi_eigen, n);
    if (ex1 == NULL) {
        free_double(&data_mat, n);
        free_vectors(&datapoints, n);
        free_eigens(&jacobi_eigen, n);
        return NULL;
    }
    python_eigenvalues = pack(ex1, n, 1);
    free_double(&ex1, n);
    /* packs the eigen values found in the jacobi algorithm into a python object */
    if (python_eigenvalues == NULL) {
        free_double(&data_mat, n);
        free_vectors(&datapoints, n);
        free_eigens(&jacobi_eigen, n);
        return NULL;
    }
    ex2 = extract_mat_from_vectors(jacobi_eigen, n, n);
    if (ex2 == NULL) {
        Py_XDECREF(python_eigenvalues);
        free_double(&data_mat, n);
        free_vectors(&datapoints, n);
        free_eigens(&jacobi_eigen, n);
        return NULL;
    }
    python_eigenvectors = pack(ex2, n, n);
    free_double(&ex2, n);
    /* packs the eigen vectors found in the jacobi algorithm into a python object */
    if (python_eigenvectors == NULL) {
        Py_XDECREF(python_eigenvalues);
        free_double(&data_mat, n);
        free_vectors(&datapoints, n);
        free_eigens(&jacobi_eigen, n);
        return NULL;
    }

    /* this part combines the eigen values and eigen structures previously found and packed
     * and puts them in one data structure that will be outputted back to python */
    python_eigen = PyTuple_New(2);
    if (python_eigen == NULL) {
        Py_XDECREF(python_eigenvalues);
        Py_XDECREF(python_eigenvectors);
        free_double(&data_mat, n);
        free_vectors(&datapoints, n);
        free_eigens(&jacobi_eigen, n);
        return NULL;
    }

    PyTuple_SetItem(python_eigen, 0, python_eigenvalues);
    PyTuple_SetItem(python_eigen, 1, python_eigenvectors);
    free_double(&data_mat, n);
    free_vectors(&datapoints, n);
    free_eigens(&jacobi_eigen, n);

    return python_eigen;
}

static PyMethodDef ckmeansMethods[] = {
        {"fit_spk",
                (PyCFunction) fit_spk_capi,
                     METH_VARARGS},
        {"fit_kms",
                (PyCFunction) fit_kms_capi,
                     METH_VARARGS},
        {"fit_wam",
                (PyCFunction) fit_wam_capi,
                     METH_VARARGS},
        {"fit_ddg",
                (PyCFunction) fit_ddg_capi,
                     METH_VARARGS},
        {"fit_lnorm",
                (PyCFunction) fit_lnorm_capi,
                     METH_VARARGS},
        {"fit_jacobi",
                (PyCFunction) fit_jacobi_capi,
                     METH_VARARGS},
        {NULL, NULL, 0, NULL}};

static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "mykmeanssp",
        NULL,
        -1,
        ckmeansMethods};

PyMODINIT_FUNC
PyInit_mykmeanssp(void) {
    PyObject *m;
    m = PyModule_Create(&moduledef);
    if (!m) {
        return NULL;
    }
    return m;
}


