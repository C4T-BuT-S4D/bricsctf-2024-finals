#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdbool.h>

typedef struct {
  PyObject_HEAD double **data;
  int rows;
  int cols;
} MatrixObject;

typedef struct {
  PyObject_HEAD double *data;
  int size;
  bool have_to_dealloc;
} VectorObject;

static void Matrix_dealloc(MatrixObject *self);
static PyObject *Matrix_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static int Matrix_init(MatrixObject *self, PyObject *args, PyObject *kwds);
static PyObject *Matrix_get_rows(MatrixObject *self, void *closure);
static PyObject *Matrix_get_cols(MatrixObject *self, void *closure);
static PyObject *Matrix_getitem(MatrixObject *self, PyObject *key);
static int Matrix_setitem(MatrixObject *self, PyObject *key, PyObject *value);
static PyObject *Matrix_dot(MatrixObject *self, PyObject *args);
static PyObject *Matrix_add(PyObject *a, PyObject *b);
static PyObject *Matrix_mul(PyObject *a, PyObject *b);
static PyObject *Matrix_sub(PyObject *a, PyObject *b);
static PyObject *Vector_get_length(VectorObject *self, void *closure);
static Py_ssize_t Vector_lenfunc(PyObject *self);

static void Vector_dealloc(VectorObject *self);
static PyObject *Vector_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
static int Vector_init(VectorObject *self, PyObject *args, PyObject *kwds);
static PyObject *Vector_getitem(VectorObject *self, Py_ssize_t index);
static int Vector_setitem(VectorObject *self, Py_ssize_t index,
                          PyObject *value);
static PyObject *Vector_add(PyObject *a, PyObject *b);
static PyObject *Vector_sub(PyObject *a, PyObject *b);
static PyObject *Vector_mul(PyObject *a, PyObject *b);
static PyObject *Vector_dot(VectorObject *self, PyObject *args);
static PyObject *Matrix_get_row(MatrixObject *self, PyObject *args);
PyMODINIT_FUNC PyInit_numb(void);
