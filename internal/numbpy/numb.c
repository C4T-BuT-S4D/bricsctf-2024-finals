#define PY_SSIZE_T_CLEAN
#include "numb.h"
#include <stdio.h>
#include <stdlib.h>

static PyMappingMethods Matrix_mapping_methods = {
    .mp_length = NULL,
    .mp_subscript = (binaryfunc)Matrix_getitem,
    .mp_ass_subscript = (objobjargproc)Matrix_setitem,
};

static PyGetSetDef Matrix_getsetters[] = {
    {"rows", (getter)Matrix_get_rows, NULL, "number of rows", NULL},
    {"cols", (getter)Matrix_get_cols, NULL, "number of columns", NULL},
    {NULL} // Sentinel
};

static PyNumberMethods Matrix_as_number = {
    .nb_add = Matrix_add,      // Overloads the + operator
    .nb_subtract = Matrix_sub, // Overloads the - operator
    .nb_multiply = Matrix_mul, // Overloads the * operator
};

static PyMethodDef Matrix_methods[] = {
    {"dot", (PyCFunction)Matrix_dot, METH_VARARGS,
     "Dot multiply with another matrix or vector"},
    {"get_row", (PyCFunction)Matrix_get_row, METH_VARARGS,
     "Get matrix row as a vector"},
    {NULL} // Sentinel
};

static PyTypeObject MatrixType = {
    PyVarObject_HEAD_INIT(NULL, 0).tp_name = "numb.Matrix",
    .tp_basicsize = sizeof(MatrixObject),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)Matrix_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = Matrix_new,
    .tp_init = (initproc)Matrix_init,
    .tp_methods = Matrix_methods,
    .tp_as_mapping = &Matrix_mapping_methods,
    .tp_getset = Matrix_getsetters, // Register property getters
    .tp_as_number = &Matrix_as_number,
};

static PyMethodDef Vector_methods[] = {
    {"dot", (PyCFunction)Vector_dot, METH_VARARGS,
     "Dot product with another vector"},
    {NULL} // Sentinel
};

static PyNumberMethods Vector_as_number = {
    .nb_add = Vector_add,      // Overloads the + operator
    .nb_subtract = Vector_sub, // Overloads the - operator
    .nb_multiply = Vector_mul, // Overloads the * operator
};

static PySequenceMethods Vector_sequence_methods = {
    .sq_length = Vector_lenfunc,
    .sq_item = (ssizeargfunc)Vector_getitem,
    .sq_ass_item = (ssizeobjargproc)Vector_setitem,
};

static PyGetSetDef Vector_getsetters[] = {
    {"length", (getter)Vector_get_length, NULL, "length of the vector", NULL},
    {NULL} // Sentinel
};

static PyTypeObject VectorType = {
    PyVarObject_HEAD_INIT(NULL, 0).tp_name = "numb.Vector",
    .tp_basicsize = sizeof(VectorObject),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)Vector_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = Vector_new,
    .tp_init = (initproc)Vector_init,
    .tp_methods = Vector_methods,
    .tp_as_sequence = &Vector_sequence_methods,
    .tp_getset = Vector_getsetters, // Register property getters
    .tp_as_number = &Vector_as_number,
};
static void Matrix_dealloc(MatrixObject *self) {
  if (self->data) {
    for (int i = 0; i < self->rows; ++i) {
      free(self->data[i]);
    }
  }
  free(self->data);
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *Matrix_new(PyTypeObject *type, PyObject *args,
                            PyObject *kwds) {
  MatrixObject *self;
  self = (MatrixObject *)type->tp_alloc(type, 0);
  if (self != NULL) {
    self->data = NULL;
    self->rows = 0;
    self->cols = 0;
  }
  return (PyObject *)self;
}

static int Matrix_init(MatrixObject *self, PyObject *args, PyObject *kwds) {
  PyObject *input = NULL;
  if (!PyArg_ParseTuple(args, "O", &input)) {
    return -1;
  }

  if (!PyList_Check(input)) {
    PyErr_SetString(PyExc_TypeError, "Expected a list of lists");
    return -1;
  }

  int rows = PyList_Size(input);
  if (rows == 0) {
    PyErr_SetString(PyExc_ValueError, "List of lists cannot be empty");
    return -1;
  }

  PyObject *first_row = PyList_GetItem(input, 0);
  if (!PyList_Check(first_row)) {
    PyErr_SetString(PyExc_TypeError,
                    "Each element in the main list should be a list");
    return -1;
  }

  int cols = PyList_Size(first_row);

  self->data = (double **)malloc(rows * sizeof(double *));
  for (int i = 0; i < rows; ++i) {
    self->data[i] = (double *)malloc(cols * sizeof(double));
  }
  self->rows = rows;
  self->cols = cols;

  for (int i = 0; i < rows; i++) {
    PyObject *row = PyList_GetItem(input, i);
    if (!PyList_Check(row) || PyList_Size(row) != cols) {
      PyErr_SetString(PyExc_ValueError,
                      "All rows must have the same number of columns");
      return -1;
    }
    for (int j = 0; j < cols; j++) {
      PyObject *item = PyList_GetItem(row, j);
      if (!PyFloat_Check(item)) {
        PyErr_SetString(PyExc_TypeError, "Expected float values");
        return -1;
      }
      self->data[i][j] = PyFloat_AsDouble(item);
    }
  }

  return 0;
}

static PyObject *Matrix_get_rows(MatrixObject *self, void *closure) {
  return PyLong_FromLong(self->rows);
}

static PyObject *Matrix_get_cols(MatrixObject *self, void *closure) {
  return PyLong_FromLong(self->cols);
}

static PyObject *Matrix_getitem(MatrixObject *self, PyObject *key) {
  if (!PyTuple_Check(key) || PyTuple_Size(key) != 2) {
    PyErr_SetString(PyExc_TypeError, "Index must be a tuple of two integers");
    return NULL;
  }

  PyObject *py_row = PyTuple_GetItem(key, 0);
  PyObject *py_col = PyTuple_GetItem(key, 1);

  if (!PyLong_Check(py_row) || !PyLong_Check(py_col)) {
    PyErr_SetString(PyExc_TypeError, "Indices must be integers");
    return NULL;
  }

  int row = (int)PyLong_AsLong(py_row);
  int col = (int)PyLong_AsLong(py_col);

  if (row < 0 || row >= self->rows || col < 0 || col >= self->cols) {
    PyErr_SetString(PyExc_IndexError, "Index out of bounds");
    return NULL;
  }

  return PyFloat_FromDouble(self->data[row][col]);
}

static int Matrix_setitem(MatrixObject *self, PyObject *key, PyObject *value) {
  if (!PyTuple_Check(key) || PyTuple_Size(key) != 2) {
    PyErr_SetString(PyExc_TypeError, "Index must be a tuple of two integers");
    return -1;
  }

  PyObject *py_row = PyTuple_GetItem(key, 0);
  PyObject *py_col = PyTuple_GetItem(key, 1);

  if (!PyLong_Check(py_row) || !PyLong_Check(py_col)) {
    PyErr_SetString(PyExc_TypeError, "Indices must be integers");
    return -1;
  }

  int row = (int)PyLong_AsLong(py_row);
  int col = (int)PyLong_AsLong(py_col);

  if (row < 0 || row >= self->rows || col < 0 || col >= self->cols) {
    PyErr_SetString(PyExc_IndexError, "Index out of bounds");
    return -1;
  }

  if (!PyFloat_Check(value)) {
    PyErr_SetString(PyExc_TypeError, "Assigned value must be a float");
    return -1;
  }

  self->data[row][col] = PyFloat_AsDouble(value);
  return 0;
}

static PyObject *Matrix_add(PyObject *a, PyObject *b) {
  if (!PyObject_TypeCheck(a, &MatrixType) ||
      !PyObject_TypeCheck(b, &MatrixType)) {
    Py_RETURN_NOTIMPLEMENTED;
  }

  MatrixObject *mat1 = (MatrixObject *)a;
  MatrixObject *mat2 = (MatrixObject *)b;

  if (mat1->rows != mat2->rows || mat1->cols != mat2->cols) {
    PyErr_SetString(PyExc_ValueError, "Matrix dimensions must agree");
    return NULL;
  }

  MatrixObject *result = (MatrixObject *)Matrix_new(&MatrixType, NULL, NULL);
  if (!result)
    return NULL;

  result->rows = mat1->rows;
  result->cols = mat1->cols;
  result->data = (double **)malloc(result->rows * sizeof(double *));
  for (int i = 0; i < result->rows; ++i) {
    result->data[i] = NULL;
  }
  for (int i = 0; i < result->rows; ++i) {
    result->data[i] = (double *)malloc(result->cols * sizeof(double));
    if (!result->data[i]) {
      PyErr_SetString(PyExc_MemoryError, "Memory allocation failed");
      // Free previously allocated rows
      for (int k = 0; k < i; ++k) {
        free(result->data[k]);
        result->data[k] = NULL;
      }
      free(result->data);
      result->data = NULL;
      Py_DECREF(result);
      return NULL;
    }
  }

  for (int i = 0; i < mat1->rows; ++i) {
    for (int j = 0; j < mat1->cols; ++j) {
      result->data[i][j] = mat1->data[i][j] + mat2->data[i][j];
    }
  }

  return (PyObject *)result;
}

static PyObject *Matrix_sub(PyObject *a, PyObject *b) {
  if (!PyObject_TypeCheck(a, &MatrixType) ||
      !PyObject_TypeCheck(b, &MatrixType)) {
    Py_RETURN_NOTIMPLEMENTED;
  }

  MatrixObject *mat1 = (MatrixObject *)a;
  MatrixObject *mat2 = (MatrixObject *)b;

  if (mat1->rows != mat2->rows || mat1->cols != mat2->cols) {
    PyErr_SetString(PyExc_ValueError, "Matrix dimensions must agree");
    return NULL;
  }

  MatrixObject *result = (MatrixObject *)Matrix_new(&MatrixType, NULL, NULL);
  if (!result)
    return NULL;

  result->rows = mat1->rows;
  result->cols = mat1->cols;
  result->data = (double **)malloc(result->rows * sizeof(double *));
  for (int i = 0; i < result->rows; ++i) {
    result->data[i] = (double *)malloc(result->cols * sizeof(double));
    if (!result->data[i]) {
      PyErr_SetString(PyExc_MemoryError, "Memory allocation failed");
      for (int k = 0; k < i; ++k)
        free(result->data[k]);
      free(result->data);
      Py_DECREF(result);
      return NULL;
    }
  }

  for (int i = 0; i < mat1->rows; ++i) {
    for (int j = 0; j < mat1->cols; ++j) {
      result->data[i][j] = mat1->data[i][j] - mat2->data[i][j];
    }
  }

  return (PyObject *)result;
}

static PyObject *Matrix_mul(PyObject *a, PyObject *b) {
  MatrixObject *mat;
  double scalar;

  if (PyObject_TypeCheck(a, &MatrixType) && PyFloat_Check(b)) {
    mat = (MatrixObject *)a;
    scalar = PyFloat_AsDouble(b);
  } else if (PyFloat_Check(a) && PyObject_TypeCheck(b, &MatrixType)) {
    mat = (MatrixObject *)b;
    scalar = PyFloat_AsDouble(a);
  } else {
    Py_RETURN_NOTIMPLEMENTED;
  }

  MatrixObject *result = (MatrixObject *)Matrix_new(&MatrixType, NULL, NULL);
  if (!result)
    return NULL;

  result->rows = mat->rows;
  result->cols = mat->cols;
  result->data = (double **)malloc(result->rows * sizeof(double *));
  for (int i = 0; i < result->rows; ++i) {
    result->data[i] = NULL;
  }
  for (int i = 0; i < result->rows; ++i) {
    if (!result->data[i]) {
      PyErr_SetString(PyExc_MemoryError, "Memory allocation failed");
      for (int k = 0; k < i; ++k) {
        free(result->data[k]);
        result->data[k] = NULL;
      }
      free(result->data);
      result->data = NULL;
      Py_DECREF(result);
      return NULL;
    }
  }

  for (int i = 0; i < mat->rows; ++i) {
    for (int j = 0; j < mat->cols; ++j) {
      result->data[i][j] = mat->data[i][j] * scalar;
    }
  }

  return (PyObject *)result;
}

static PyObject *Matrix_dot(MatrixObject *self, PyObject *args) {
  PyObject *other;
  if (!PyArg_ParseTuple(args, "O", &other))
    return NULL;

  if (PyObject_TypeCheck(other, &MatrixType)) {
    MatrixObject *mat2 = (MatrixObject *)other;
    if (self->cols != mat2->rows) {
      PyErr_SetString(PyExc_ValueError,
                      "Matrix dimensions do not allow multiplication");
      return NULL;
    }

    MatrixObject *result = (MatrixObject *)Matrix_new(&MatrixType, NULL, NULL);
    result->rows = self->rows;
    result->cols = mat2->cols;
    result->data = (double **)malloc(result->rows * sizeof(double *));
    for (int i = 0; i < result->rows; ++i) {
      result->data[i] = (double *)malloc(result->cols * sizeof(double));
    }

    for (int i = 0; i < self->rows; ++i) {
      for (int j = 0; j < mat2->cols; ++j) {
        double sum = 0.0;
        for (int k = 0; k < self->cols; ++k) {
          sum += self->data[i][k] * mat2->data[k][j];
        }
        result->data[i][j] = sum;
      }
    }

    return (PyObject *)result;
  } else if (PyObject_TypeCheck(other, &VectorType)) {
    VectorObject *vec = (VectorObject *)other;
    if (self->cols != vec->size) {
      PyErr_SetString(PyExc_ValueError,
                      "Matrix and vector dimensions do not align");
      return NULL;
    }

    VectorObject *result = (VectorObject *)Vector_new(&VectorType, NULL, NULL);
    result->size = self->rows;
    result->data = (double *)malloc(result->size * sizeof(double));

    for (int i = 0; i < self->rows; ++i) {
      double sum = 0.0;
      for (int j = 0; j < self->cols; ++j) {
        sum += self->data[i][j] * vec->data[j];
      }
      result->data[i] = sum;
    }

    return (PyObject *)result;
  } else {
    PyErr_SetString(PyExc_TypeError, "Unsupported type for multiplication");
    return NULL;
  }
}

static PyObject *Vector_get_length(VectorObject *self, void *closure) {
  return PyLong_FromLong(self->size);
}

static void Vector_dealloc(VectorObject *self) {
  if (self->have_to_dealloc) {
    free(self->data);
  }
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *Vector_new(PyTypeObject *type, PyObject *args,
                            PyObject *kwds) {
  VectorObject *self;
  self = (VectorObject *)type->tp_alloc(type, 0);
  if (self != NULL) {
    self->data = NULL;
    self->size = 0;
    self->have_to_dealloc = false;
  }
  return (PyObject *)self;
}

static int Vector_init(VectorObject *self, PyObject *args, PyObject *kwds) {
  PyObject *input = NULL;
  if (!PyArg_ParseTuple(args, "O", &input)) {
    return -1;
  }

  if (!PyList_Check(input)) {
    PyErr_SetString(PyExc_TypeError, "Expected a list");
    return -1;
  }

  int size = PyList_Size(input);
  self->data = (double *)malloc(size * sizeof(double));
  if (!self->data) {
    PyErr_SetString(PyExc_MemoryError, "Could not allocate memory for vector");
    return -1;
  }
  self->have_to_dealloc = true;

  for (int i = 0; i < size; i++) {
    PyObject *item = PyList_GetItem(input, i);
    if (!PyFloat_Check(item)) {
      PyErr_SetString(PyExc_TypeError, "Expected float values");
      free(self->data);
      self->have_to_dealloc = false;
      return -1;
    }
    self->data[i] = PyFloat_AsDouble(item);
  }
  self->size = size;

  return 0;
}

static PyObject *Vector_getitem(VectorObject *self, Py_ssize_t index) {
  if (index < 0 || index >= self->size) {
    PyErr_SetString(PyExc_IndexError, "Index out of bounds");
    return NULL;
  }
  return PyFloat_FromDouble(self->data[index]);
}

static int Vector_setitem(VectorObject *self, Py_ssize_t index,
                          PyObject *value) {
  if (index < 0 || index >= self->size) {
    PyErr_SetString(PyExc_IndexError, "Index out of bounds");
    return -1;
  }
  if (!PyFloat_Check(value)) {
    PyErr_SetString(PyExc_TypeError, "Assigned value must be a float");
    return -1;
  }
  self->data[index] = PyFloat_AsDouble(value);
  return 0;
}

static PyObject *Vector_dot(VectorObject *self, PyObject *args) {
  PyObject *other;
  if (!PyArg_ParseTuple(args, "O!", &VectorType, &other))
    return NULL;

  VectorObject *vec2 = (VectorObject *)other;
  if (self->size != vec2->size) {
    PyErr_SetString(PyExc_ValueError, "Vector lengths do not match");
    return NULL;
  }

  double dot_product = 0.0;
  for (int i = 0; i < self->size; ++i) {
    dot_product += self->data[i] * vec2->data[i];
  }

  return PyFloat_FromDouble(dot_product);
}

static PyObject *Vector_add(PyObject *a, PyObject *b) {
  if (!PyObject_TypeCheck(a, &VectorType) ||
      !PyObject_TypeCheck(b, &VectorType)) {
    Py_RETURN_NOTIMPLEMENTED;
  }

  VectorObject *vec1 = (VectorObject *)a;
  VectorObject *vec2 = (VectorObject *)b;

  if (vec1->size != vec2->size) {
    PyErr_SetString(PyExc_ValueError, "Vector sizes must match");
    return NULL;
  }

  VectorObject *result = (VectorObject *)Vector_new(&VectorType, NULL, NULL);
  if (!result)
    return NULL;

  result->size = vec1->size;
  result->data = (double *)malloc(result->size * sizeof(double));
  if (!result->data) {
    PyErr_SetString(PyExc_MemoryError, "Memory allocation failed");
    Py_DECREF(result);
    return NULL;
  }

  for (int i = 0; i < vec1->size; ++i) {
    result->data[i] = vec1->data[i] + vec2->data[i];
  }

  return (PyObject *)result;
}

static PyObject *Vector_sub(PyObject *a, PyObject *b) {
  if (!PyObject_TypeCheck(a, &VectorType) ||
      !PyObject_TypeCheck(b, &VectorType)) {
    Py_RETURN_NOTIMPLEMENTED;
  }

  VectorObject *vec1 = (VectorObject *)a;
  VectorObject *vec2 = (VectorObject *)b;

  if (vec1->size != vec2->size) {
    PyErr_SetString(PyExc_ValueError, "Vector sizes must match");
    return NULL;
  }

  VectorObject *result = (VectorObject *)Vector_new(&VectorType, NULL, NULL);
  if (!result)
    return NULL;

  result->size = vec1->size;
  result->data = (double *)malloc(result->size * sizeof(double));
  if (!result->data) {
    PyErr_SetString(PyExc_MemoryError, "Memory allocation failed");
    Py_DECREF(result);
    return NULL;
  }

  for (int i = 0; i < vec1->size; ++i) {
    result->data[i] = vec1->data[i] - vec2->data[i];
  }

  return (PyObject *)result;
}

static PyObject *Vector_mul(PyObject *a, PyObject *b) {
  VectorObject *vec;
  double scalar;

  if (PyObject_TypeCheck(a, &VectorType) && PyFloat_Check(b)) {
    vec = (VectorObject *)a;
    scalar = PyFloat_AsDouble(b);
  } else if (PyFloat_Check(a) && PyObject_TypeCheck(b, &VectorType)) {
    vec = (VectorObject *)b;
    scalar = PyFloat_AsDouble(a);
  } else {
    Py_RETURN_NOTIMPLEMENTED;
  }

  VectorObject *result = (VectorObject *)Vector_new(&VectorType, NULL, NULL);
  if (!result)
    return NULL;

  result->size = vec->size;
  result->data = (double *)malloc(result->size * sizeof(double));
  if (!result->data) {
    PyErr_SetString(PyExc_MemoryError, "Memory allocation failed");
    Py_DECREF(result);
    return NULL;
  }
  for (int i = 0; i < vec->size; ++i) {
    result->data[i] = vec->data[i] * scalar;
  }

  return (PyObject *)result;
}

static PyObject *Matrix_get_row(MatrixObject *self, PyObject *args) {
  int row_index;
  if (!PyArg_ParseTuple(args, "i", &row_index)) {
    return NULL;
  }

  if (row_index < 0 || row_index >= self->rows) {
    PyErr_SetString(PyExc_IndexError, "Row index out of bounds");
    return NULL;
  }

  VectorObject *vec = (VectorObject *)Vector_new(&VectorType, NULL, NULL);
  if (!vec) {
    return NULL;
  }

  vec->have_to_dealloc = false;
  vec->size = self->cols;
  vec->data = self->data[row_index];

  return (PyObject *)vec;
}

static Py_ssize_t Vector_lenfunc(PyObject *self) {
  return ((VectorObject *)self)->size;
}

static PyMethodDef NumbMethods[] = {
    {NULL, NULL, 0, NULL} // Sentinel
};

static struct PyModuleDef numbm = {PyModuleDef_HEAD_INIT, "numb", NULL, -1,
                                   NumbMethods};

PyMODINIT_FUNC PyInit_numb(void) {
  PyObject *m;
  if (PyType_Ready(&MatrixType) < 0 || PyType_Ready(&VectorType) < 0)
    return NULL;

  m = PyModule_Create(&numbm);
  if (m == NULL)
    return NULL;

  Py_INCREF(&MatrixType);
  Py_INCREF(&VectorType);

  if (PyModule_AddObject(m, "Matrix", (PyObject *)&MatrixType) < 0 ||
      PyModule_AddObject(m, "Vector", (PyObject *)&VectorType) < 0) {
    Py_DECREF(&MatrixType);
    Py_DECREF(&VectorType);
    Py_DECREF(m);
    return NULL;
  }

  return m;
}
