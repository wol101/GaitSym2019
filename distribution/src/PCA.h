/*
 *  PCA.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 4/6/2014.
 *  Copyright 2014 Bill Sellers. All rights reserved.
 *
 */

#ifndef PCA_H
#define PCA_H

#include <algorithm>
#include <iostream>

#define CHECK_COLUMNMAJORARRAY_BOUNDS

class ColumnMajorArray
{
public:
    ColumnMajorArray();
    ColumnMajorArray(int rows, int cols);
    ColumnMajorArray(const ColumnMajorArray &array);
    virtual ~ColumnMajorArray();

    void Init(int rows, int cols);
    void Zero() { std::fill_n(m_data, m_rows * m_cols, 0); }
    void Fill(double v) { std::fill_n(m_data, m_rows * m_cols, v); }
    double *Data() { return m_data; }
    const double *ConstData() const { return m_data; }
    void CopyDataOut(double *dataCopy) const { std::copy(m_data, m_data + (m_rows * m_cols), dataCopy); }
    void CopyDataIn(const double *data) { std::copy(data, data + (m_rows * m_cols), m_data); }
    int Rows() const { return m_rows; }
    int Cols() const { return m_cols; }
    double Sum() const { double sum = 0; double *p = m_data; for (int i = 0; i < (m_rows * m_cols); i++) sum += *p++; return sum; }

    double Get(int row, int col) const
    {
#ifdef CHECK_COLUMNMAJORARRAY_BOUNDS
        if (row < 0 || row >= m_rows) std::cerr << "Warning row out of bounds: row=" << row << " col=" << col << " m_rows=" << m_rows << " m_cols=" << m_cols << "\n";
        if (col < 0 || col >= m_cols) std::cerr << "Warning col out of bounds: row=" << row << " col=" << col << " m_rows=" << m_rows << " m_cols=" << m_cols << "\n";
#endif
        return m_data[col * m_rows + row];
    }
    void Set(int row, int col, double v)
    {
#ifdef CHECK_COLUMNMAJORARRAY_BOUNDS
        if (row < 0 || row >= m_rows) std::cerr << "Warning row out of bounds: row=" << row << " col=" << col << " m_rows=" << m_rows << " m_cols=" << m_cols << "\n";
        if (col < 0 || col >= m_cols) std::cerr << "Warning col out of bounds: row=" << row << " col=" << col << " m_rows=" << m_rows << " m_cols=" << m_cols << "\n";
#endif
        m_data[col * m_rows + row] = v;
    }
    double *Col(int col)
    {
#ifdef CHECK_COLUMNMAJORARRAY_BOUNDS
        if (col < 0 || col >= m_cols) std::cerr << "Warning col out of bounds: col=" << col << " m_rows=" << m_rows << " m_cols=" << m_cols << "\n";
#endif
        return m_data + (col * m_rows);
    }
    const double *ConstCol(int col) const
    {
#ifdef CHECK_COLUMNMAJORARRAY_BOUNDS
        if (col < 0 || col >= m_cols) std::cerr << "Warning col out of bounds: col=" << col << " m_rows=" << m_rows << " m_cols=" << m_cols << "\n";
#endif
        return m_data + (col * m_rows);
    }

    friend std::ostream& operator<<(std::ostream& os, const ColumnMajorArray& array);

private:
    int m_rows;
    int m_cols;
    double *m_data;
};

class PCA
{
public:
    PCA();
    virtual ~PCA();

    // the data is arranged so that each point is a row and each column is a dimension
    void DoPCA(const ColumnMajorArray &data);
    void CalculateScores(ColumnMajorArray &data, int startEigenvector, int endEigenvectors, ColumnMajorArray *scores);

    ColumnMajorArray *EigenVectors() { return &m_eigenvectors; }
    ColumnMajorArray *EigenValues() { return &m_eigenvalues; }
    ColumnMajorArray *Means() { return &m_means; }
    ColumnMajorArray *Scores() { return &m_scores; }

private:
    ColumnMajorArray m_eigenvectors;        // the principal component analysis eigenvectors
    ColumnMajorArray m_eigenvalues;         // the principal component analysis eigenvalues
    ColumnMajorArray m_means;               // the mean values used to calculate the mean centred matrix
    ColumnMajorArray m_scores;              // store the PCA scores calculated from the original warehouse
};

#endif // PCA_H
