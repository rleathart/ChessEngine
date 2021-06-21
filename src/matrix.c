#include "matrix.h"
#include <math.h>
#include <assert.h>
#include <stdio.h>

Matrix matrix_new(size_t m, size_t n)
{
  Matrix mat = {
      .m = m,
      .n = n,
      .data = calloc(n * m, sizeof(Scalar)),
  };
  return mat;
}

Matrix matrix_new_from_array(size_t m, size_t n, Scalar* arr)
{
  Matrix mat = matrix_new(m, n);
  for (size_t i = 0; i < m; i++)
    for (size_t j = 0; j < n; j++)
      matrix_set_elem(&mat, i, j, arr[i * n + j]);
  return mat;
}

Matrix matrix_new_rotation(Scalar theta)
{
  // clang-format off
  Scalar rotation[] = {
    cos(theta), -sin(theta),
    sin(theta), cos(theta),
  }; // clang-format on
  Matrix rv = matrix_new_from_array(2, 2, rotation);
  return rv;
}

void matrix_print(Matrix mat)
{
  for (int i = 0; i < mat.m; i++)
  {
    for (int j = 0; j < mat.n; j++)
      printf("%f ", mat.data[i * mat.n + j]);
    printf("\n");
  }
}

void matrix_set_elem(Matrix* mat, size_t i, size_t j, Scalar value)
{
  assert(i < mat->m);
  assert(j < mat->n);
  mat->data[i * mat->n + j] = value;
}

Scalar matrix_get_elem(Matrix mat, size_t i, size_t j)
{
  assert(i < mat.m);
  assert(j < mat.n);
  return mat.data[i * mat.n + j];
}

Matrix matrix_mul(Matrix a, Matrix b)
{
  assert(a.n == b.m);
  Matrix rv = matrix_new(a.m, b.n);

  for (int i = 0; i < a.m; i++)
    for (int j = 0; j < b.n; j++)
    {
      Scalar tot = 0;
      for (int k = 0; k < b.m; k++)
        tot += matrix_get_elem(a, i, k) * matrix_get_elem(b, k, j);
      matrix_set_elem(&rv, i, j, tot);
    }
  return rv;
}

Matrix matrix_mul_scalar(Scalar x, Matrix mat)
{
  Matrix rv = matrix_new(mat.m, mat.n);
  for (size_t i = 0; i < mat.m * mat.n; i++)
    rv.data[i] = x * mat.data[i];
  return rv;
}
