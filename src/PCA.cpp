/*
 *  PCA.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 4/6/2014.
 *  Copyright 2014 Bill Sellers. All rights reserved.
 *
 */

#include "PCA.h"

#include <cfloat>

#ifdef USE_PCA
  #ifdef USE_CBLAS // note, on mac we use an edited version of cblas.h as a local copy
    #include <cblas.h>
    #include <clapack.h>
  #else
    #ifdef USE_ESSL
      #include <essl.h>
    #else
      #include <blas.h>
      #include <lapack.h>
    #endif
  #endif
#endif

ColumnMajorArray::ColumnMajorArray()
{
    m_rows = 0;
    m_cols = 0;
    m_data = 0;
}

ColumnMajorArray::ColumnMajorArray(int rows, int cols)
{
    m_rows = rows;
    m_cols = cols;
    m_data = new double[m_rows * m_cols];
}

ColumnMajorArray::ColumnMajorArray(const ColumnMajorArray &array)
{
    m_rows = array.Rows();
    m_cols = array.Cols();
    m_data = new double[m_rows * m_cols];
    array.CopyDataOut(m_data);
}

ColumnMajorArray::~ColumnMajorArray()
{
    if (m_data) delete [] m_data;
}

void ColumnMajorArray::Init(int rows, int cols)
{
    if (m_data) delete [] m_data;
    m_rows = rows;
    m_cols = cols;
    m_data = new double[m_rows * m_cols];
}

std::ostream& operator<<(std::ostream& os, const ColumnMajorArray& array)
{
    os << "m_rows = " << array.m_rows << " m_cols = " << array.m_cols << "\n";
    for (int r = 0; r < array.m_rows; r++)
    {
        for (int c = 0; c < array.m_cols; c++)
        {
            if (c) os << " " << array.Get(r, c);
            else os << array.Get(r, c);
        }
        os << "\n";
    }
    return os;
}

PCA::PCA()
{
}

PCA::~PCA()
{
}

// the data is arranged so that each point is a row and each column is a dimension
void PCA::DoPCA(const ColumnMajorArray &data)
{
#ifdef USE_PCA
    int r, c;
    double sum, mean;
    // calculate the mean centred covariance matrix
    // this routine does not currently do unit variance scaling because this is often not required
    m_means.Init(1, data.Cols());
    ColumnMajorArray meanCentredData(data.Rows(), data.Cols());
    for (c = 0; c < data.Cols(); c++)
    {
        sum = 0;
        for (r = 0; r < data.Rows(); r++) sum += data.Get(r, c);
        mean = sum / data.Rows();
        m_means.Set(0, c, mean);
        for (r = 0; r < data.Rows(); r++) meanCentredData.Set(r, c, data.Get(r, c) - mean);
    }

    // Load m-by-n mean centred data matrix C
    // Compute Cov=1/(m-1)*CT*C
    // BLAS Level 3 routine DSYRK computes:
    // a*A*AT+b*v
    // a*AT*A+b*v

    ColumnMajorArray covarianceMatrix(data.Cols(), data.Cols());

#ifdef USE_CBLAS // note, on mac we use an edited version of cblas.h as a local copy
    cblas_dsyrk(CblasColMajor, CblasUpper, CblasTrans, covarianceMatrix.Cols(),
                meanCentredData.Rows(), 1.0/(data.Rows() - 1), meanCentredData.Data(), meanCentredData.Rows(),
                0, covarianceMatrix.Data(), covarianceMatrix.Cols());
#else
#ifdef USE_ESSL
    char UL = 'U';
    char TR = 'T';
    int N = covarianceMatrix.Cols();
    int K = meanCentredData.Rows();
    double alpha = 1.0/(data.Rows() - 1);
    double *A = meanCentredData.Data();
    double lda = meanCentredData.Rows();
    double beta = 0;
    double *C = covarianceMatrix.Data();
    int ldc = covarianceMatrix.Cols();
    dsyrk(&UL, &TR, N, K, alpha, A, lda, beta, C, ldc);
#else
    char UL = 'U';
    char TR = 'T';
    int N = covarianceMatrix.Cols();
    int K = meanCentredData.Rows();
    double alpha = 1.0/(data.Rows() - 1);
    double *A = meanCentredData.Data();
    double lda = meanCentredData.Rows();
    double beta = 0;
    double *C = covarianceMatrix.Data();
    int ldc = covarianceMatrix.Cols();
    dsyrk(&UL, &TR, &N, &K, &alpha, A, &lda, &beta, C, &ldc);
#endif
#endif

    // now calculate the eigenvectors and eigenvalues of the covariance matrix
    m_eigenvalues.Init(1, data.Cols());
    m_eigenvectors.Init(data.Cols(), data.Cols());

#ifdef USE_CBLAS
    m_eigenvectors.CopyDataIn(covarianceMatrix.Data());
    char jobz = 'V';
    char uplo = 'U';
    int cols = data.Cols();
    int lwork = cols * cols; // I think this is big enough
    double *work = new double [lwork];
    int info;
    dsyev_(&jobz, &uplo, &cols, m_eigenvectors.Data(), &cols, m_eigenvalues.Data(), work, &lwork, &info);
#else
#ifdef USE_ESSL
    char jobz = 'V';
    char range = 'A';
    char uplo = 'U';
    int n = covarianceMatrix.Cols();
    double *a = covarianceMatrix.Data();
    lda = n;
    int vl = 0;
    int vu = 0;
    int il = 0;
    int iu = 0;
    double abstol = DBL_MIN;
    int m;
    double *w = m_eigenvalues.Data();
    double *z = m_eigenvectors.Data();
    int ldz = n;
    double *work = new double[8 * n];
    int lwork = 8 * n;
    int *iwork = new int[5 * n];
    int ifail;
    int info;
    dsyevx(&jobz, &range, &uplo, n, a, lda, vl, vu, il, iu, abstol, &m, w, z, ldz, work, lwork, iwork, &ifail, info);
#else
    m_eigenvectors.CopyDataIn(covarianceMatrix.Data());
    char jobz = 'V';
    char uplo = 'U';
    int cols = data.Cols();
    int lwork = cols * cols; // I think this is big enough
    double *work = new double [lwork];
    int info;
    dsyev_(&jobz, &uplo, &cols, m_eigenvectors.Data(), &cols, m_eigenvalues.Data(), work, &lwork, &info);
#endif
#endif

    delete [] work;

    m_scores.Init(data.Rows(), data.Cols());
    CalculateScores(meanCentredData, 0, meanCentredData.Cols(), &m_scores);
#endif
}

void PCA::CalculateScores(ColumnMajorArray &data, int startEigenvector, int endEigenvectors, ColumnMajorArray *scores)
{
#ifdef USE_PCA
    for (int i = startEigenvector; i < endEigenvectors; i++)
    {
#ifdef USE_CBLAS // note, on mac we use an edited version of cblas.h as a local copy
        cblas_dgemv(CblasColMajor, CblasNoTrans, data.Rows(), data.Cols(), 1.0, data.ConstData(), data.Rows(), m_eigenvectors.Col(i), 1, 0, scores->Col(i), 1);
#else
#ifdef USE_ESSL
        char TA = 'N';
        int M = data.Rows();
        int N = data.Cols();
        double alpha = 1.0;
        double *A = data.Data();
        int lda = data.Rows();
        double *X = m_eigenvectors.Col(i);
        int incX = 1;
        double beta = 0;
        double *Y = scores->Col(i);
        int incY = 1;
        dgemv(&TA, M, N, alpha, A, lda, X, incX, beta, Y, incY);
#else
        char TA = 'N';
        int M = data.Rows();
        int N = data.Cols();
        double alpha = 1.0;
        double *A = data.Data();
        int lda = data.Rows();
        double *X = m_eigenvectors.Col(i);
        int incX = 1;
        double beta = 0;
        double *Y = scores->Col(i);
        int incY = 1;
        dgemv(&TA, &M, &N, &alpha, A, &lda, X, &incX, &beta, Y, &incY);
#endif
#endif
    }
#endif
}

