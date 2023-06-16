/*
 *  Warehouse.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 24/4/2014.
 *  Copyright 2014 Bill Sellers. All rights reserved.
 *
 */

#ifdef EXPERIMENTAL

#ifndef WAREHOUSE_H
#define WAREHOUSE_H

#include "NamedObject.h"

#include "PCA.h"

#include <ANN/ANN.h>

#include <vector>
#include <string>
#include <map>
#include <memory>

class Body;

class WarehouseUnit : public NamedObject
{
public:
    WarehouseUnit();
    virtual ~WarehouseUnit();

    int ImportWarehouseUnit(const char *filename, bool appendFlag);
    int ImportWarehouseUnit(char *fileData, int fileDataLen, bool appendFlag);
    void InitaliseWarehouse();
    int DoSearch();

    std::vector<std::string> *GetDriverNames() { return &m_driverNames; }
    std::vector<std::string> *GetBodyNames() { return &m_bodyNames; }
    int GetNumDrivers() { return m_numDrivers; }
    int GetNumBodies() { return m_numBodies; }
    int GetNumValuesPerBody() { return m_numValuesPerBody; }
    int GetNumPoints() { return m_nPts; }
    int GetDimensionality() { return m_nDim; }
    int GetDimensionsWanted() { return m_numDimensionsWanted; }
    double *GetActivations(int *numPoints, int *numDrivers) { *numPoints = m_nPts; *numDrivers = m_numDrivers; return m_activations; }
    double *GetBodyData(int *numPoints, int *numBodies, int *numValuesPerBody) { *numPoints = m_nPts; *numBodies = m_numBodies; *numValuesPerBody = m_numValuesPerBody; return m_bodyData; }
    double *GetWeights(int *numBodies, int *numValuesPerBody) { *numBodies = m_numBodies; *numValuesPerBody = m_numValuesPerBody; return m_weights; }
    double *GetBodyQueryData() { return m_bodyQueryData; }

    double* GetCurrentActivations();
    double GetNearestNeighbourDistance() { return m_dists[0]; }
    int GetNearestNeighbourIndex() { return m_nnIdx[0]; }

    void SetUsePCA(bool usePCA) { m_usePCA = usePCA; }
    void SetDriverIDs(const char *nameList);
    void SetBodyIDs(const char *nameList);
    void SetActivations(int numPoints, int numDrivers, double *activations);
    void SetBodyData(int numPoints, int numBodies, int numValuesPerBody, double *bodyData);
    void SetWeights(int numBodies, int numValuesPerBody, double *weights);
    void SetBodyQueryData(const std::map<std::string, std::unique_ptr<Body>> &bodyMap);
    void SetBodyQueryData(double *bodyQueryData);

private:
    int                 m_nPts;                         // actual number of data points
    int                 m_nDim;                         // dimensionality of search space
    int                 m_nNN;                          // number of nearest neighbours to return
    ANNpointArray       m_dataPts;                      // data points
    ANNpoint            m_queryPt;                      // query point
    ANNidxArray         m_nnIdx;                        // near neighbor indices
    ANNdistArray        m_dists;                        // near neighbor distances
    ANNkd_tree*         m_kdTree;                       // search structure
    double              m_eps;                          // error bound

    double *m_bodyQueryData;                            // body query data before dimension reduction
    double *m_activations;                              // activations list
    double *m_bodyData;                                 // raw warehouse body data list
    int m_numDrivers;                                   // number of drivers
    int m_numBodies;                                    // number of bodies
    int m_numValuesPerBody;                             // number of floating point values per body
    double *m_weights;                                  // weights for the body values
    bool m_usePCA;                                      // switch to turn PCA dimension reduction on and off

    std::vector<std::string> m_driverNames;             // list of driver names
    std::vector<std::string> m_bodyNames;               // list of bodies

    PCA m_pca;                                          // store the PCA values
    int m_numDimensionsWanted;                          // the number of dimensions wanted
    double m_eigenvalueThreshold;                       // the threshold for the minimum wanted eigenvalue
};

class Warehouse : public NamedObject
{
public:
    Warehouse()
    {
        m_UnitIncreaseThreshold = 1e10;
        m_UnitDecreaseThresholdFactor = 0.5;
        m_CurrentUnit = 0;
        m_NearestNeighbourDistance = DBL_MAX;
        m_UsePCA = false;
        m_LastSearchResult = -1;
    }
    ~Warehouse() { for (unsigned int i = 0; i < m_warehouseList.size(); i++) delete m_warehouseList[i]; }

    int DoSearch(const std::map<std::string, std::unique_ptr<Body> > &bodyMap);
    double GetNearestNeighbourDistance() { return m_warehouseList[m_CurrentUnit]->GetNearestNeighbourDistance(); }
    double *GetCurrentActivations() { return m_warehouseList[m_CurrentUnit]->GetCurrentActivations(); }
    std::vector<std::string> *GetDriverNames() { return m_warehouseList[m_CurrentUnit]->GetDriverNames(); }
    std::vector<std::string> *GetBodyNames() { return m_warehouseList[m_CurrentUnit]->GetBodyNames(); }

    WarehouseUnit *NewWarehouseUnit(unsigned int index);
    WarehouseUnit *GetWarehouseUnit(unsigned int index) { if (index >= m_warehouseList.size()) return nullptr; return m_warehouseList[index]; }
    size_t GetNumWarehouseUnits() { return m_warehouseList.size(); }

    void SetUnitIncreaseThreshold(double unitIncreaseThreshold) { m_UnitIncreaseThreshold = unitIncreaseThreshold; }
    void SetUnitDecreaseThresholdFactor(double unitDecreaseThresholdFactor) { m_UnitDecreaseThresholdFactor = unitDecreaseThresholdFactor; }

    virtual std::string dumpToString();
    virtual std::string *createFromAttributes();
    virtual void saveToAttributes();

private:
    std::vector <WarehouseUnit *> m_warehouseList;
    double m_UnitIncreaseThreshold;
    double m_UnitDecreaseThresholdFactor;
    unsigned int m_CurrentUnit;
    int m_LastSearchResult;
    double m_NearestNeighbourDistance;
    bool m_UsePCA;
};

#endif // WAREHOUSE_H

#else

class Warehouse: public NamedObject
{
public:
    Warehouse() {}

};

#endif
