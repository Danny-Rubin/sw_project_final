#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "utils.c"
#include "executeKmeans.c"
#include "spkmeans.c"



/*
 * python entry point function:
 * only function integrated with python interface
 * params: filename, k, goal
 * what it does: call general entry point after converting python args
 */
static PyObject* pythonEntryPoint(PyObject *self, PyObject *args){
    int k = 0, res = 0, stage =0;
    char *goal;
    char* fileName;

    if(!PyArg_ParseTuple(args, "issi", &k, &goal, &fileName, &stage)){
        return  NULL;
    }
    res = cEntryPoint(k, goal, fileName, stage);
    return Py_BuildValue("i", res);
}

/* Python module functions */
static PyMethodDef capi_methods[] = {
        {"pythonEntryPoint",
                (PyCFunction) pythonEntryPoint,
                     METH_VARARGS,
                PyDoc_STR("Python entry point function")
        },
        {NULL, NULL, 0, NULL}};

static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "mykmeanssp",   /* name of module */
        NULL, /* module documentation, may be NULL */
        -1,       /* size of per-interpreter state of the module,
         or -1 if the module keeps state in global variables. */
        capi_methods
};


PyMODINIT_FUNC PyInit_mykmeanssp(void)
{
    PyObject *m;
    m = PyModule_Create(&moduledef);
    if (!m){
        return NULL;
    }
    return m;
}
