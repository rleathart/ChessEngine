#pragma once

#include <stdlib.h>

typedef double Scalar;

typedef struct
{
  size_t m, n;
  Scalar* data;
} Matrix;

void matrix_set_elem(Matrix* mat, size_t i, size_t j, Scalar value);
Matrix matrix_new(size_t m, size_t n);
Matrix matrix_new_from_array(size_t m, size_t n, Scalar* arr);
Matrix matrix_new_rotation(Scalar theta);
void matrix_print(Matrix mat);
void matrix_set_elem(Matrix* mat, size_t i, size_t j, Scalar value);
Scalar matrix_get_elem(Matrix mat, size_t i, size_t j);
Matrix matrix_mul(Matrix a, Matrix b);
Matrix matrix_mul_scalar(Scalar x, Matrix mat);
