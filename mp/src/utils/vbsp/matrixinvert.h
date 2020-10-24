// By Jason Yu-Tseh Chi
// From http://chi3x10.wordpress.com/2008/05/28/calculate-matrix-inversion-in-c/
// modified to work with valve's matrix_3x4_t

#pragma once
#include "mathlib/mathlib.h"

// calculate the cofactor of element (row,col)
inline int GetMatrixMinor(float **src, float **dest, int row, int col, int order)
{
    // indicate which col and row is being copied to dest
    int colCount = 0, rowCount = 0;

    for (int i = 0; i < order; i++)
    {
        if (i != row)
        {
            colCount = 0;
            for (int j = 0; j < order; j++)
            {
                // when j is not the element
                if (j != col)
                {
                    dest[rowCount][colCount] = src[i][j];
                    colCount++;
                }
            }
            rowCount++;
        }
    }

    return 1;
}

// Calculate the determinant recursively.
inline double CalcMatrixDeterminant(float **mat, int order)
{
    // order must be >= 0
    // stop the recursion when matrix is a single element
    if (order == 1)
        return mat[0][0];

    // the determinant value
    float det = 0;

    // allocate the cofactor matrix
    float **minor;
    minor = new float *[order - 1];
    for (int i = 0; i < order - 1; i++)
        minor[i] = new float[order - 1];

    for (int i = 0; i < order; i++)
    {
        // get minor of element (0,i)
        GetMatrixMinor(mat, minor, 0, i, order);
        // the recursion is here!

        det += (i % 2 == 1 ? -1.0 : 1.0) * mat[0][i] * CalcMatrixDeterminant(minor, order - 1);
        // det += pow( -1.0, i ) * mat[0][i] * CalcMatrixDeterminant( minor,order-1 );
    }

    // release memory
    for (int i = 0; i < order - 1; i++)
        delete[] minor[i];
    delete[] minor;

    return det;
}

// matrix inversion
inline void MatrixInversion(matrix3x4_t &in, matrix3x4_t &out)
{
    float **A;
    A = new float *[4];
    for (int i = 0; i < 4; i++)
        A[i] = new float[4];
    int order = 4;

    // load in into A
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            A[i][j] = in[i][j];
        }
    }
    A[3][0] = A[3][1] = A[3][2] = 0;
    A[3][3] = 1;

    // get the determinant of a
    double det = 1.0 / CalcMatrixDeterminant(static_cast<float **>(A), order);

    // memory allocation
    float *temp = new float[(order - 1) * (order - 1)];
    float **minor = new float *[order - 1];
    for (int i = 0; i < order - 1; i++)
        minor[i] = temp + (i * (order - 1));

    for (int j = 0; j < order; j++)
    {
        for (int i = 0; i < order; i++)
        {
            // get the co-factor (matrix) of A(j,i)
            GetMatrixMinor(static_cast<float **>(A), minor, j, i, order);
            out[i][j] = det * CalcMatrixDeterminant(minor, order - 1);
            if ((i + j) % 2 == 1)
                out[i][j] = -out[i][j];
        }
    }

    // release memory
    for (int i = 0; i < 4; i++)
        delete A[i];
    delete A;
    // delete [] minor[0];
    delete[] temp;
    delete[] minor;
}
