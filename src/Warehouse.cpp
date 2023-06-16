/*
 *  Warehouse.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 24/4/2014.
 *  Copyright 2014 Bill Sellers. All rights reserved.
 *
 */

#ifdef EXPERIMENTAL

#include "Warehouse.h"
#include "DataFile.h"
#include "GSUtil.h"
#include "Body.h"
#include "PGDMath.h"
#include "Simulation.h"
#include "PCA.h"

#include "ANN/ANN.h"

#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <sstream>

using namespace std::string_literals;

WarehouseUnit::WarehouseUnit()
{
    m_nPts = 0;                   // actual number of data points
    m_nDim = 0;                   // dimensionality of search space
    m_nNN = 0;                    // number of nearest neighbours to return
    m_dataPts = nullptr;          // data points
    m_queryPt = nullptr;          // query point
    m_nnIdx = nullptr;            // near neighbor indices
    m_dists = nullptr;            // near neighbor distances
    m_kdTree = nullptr;           // search structure
    m_eps = 0;                    // error bound
    m_activations = nullptr;      // activations list
    m_bodyData = nullptr;         // raw warehouse body data list
    m_numDrivers = 0;             // number of drivers
    m_numBodies = 0;              // number of bodies
    m_numValuesPerBody = 0;       // number of floating point values per body (currently hardwired)
    m_numDimensionsWanted = 0;    // the number of dimensions wanted
    m_eigenvalueThreshold = 0.001;// the threshold for the minimum wanted eigenvalue
    m_bodyQueryData = nullptr;    // body query data before dimension reduction
    m_weights = nullptr;          // weights for the body values
    m_usePCA = false;             // switch to turn PCA dimension reduction on and off
}

WarehouseUnit::~WarehouseUnit()
{
    if (m_dataPts) annDeallocPts(m_dataPts);
    if (m_queryPt) annDeallocPt(m_queryPt);
    if (m_bodyQueryData) delete [] m_bodyQueryData;
    if (m_nnIdx) delete [] m_nnIdx;
    if (m_dists) delete [] m_dists;
    if (m_kdTree) delete m_kdTree;
    if (m_activations) delete [] m_activations;
    if (m_bodyData) delete [] m_bodyData;
    if (m_weights) delete [] m_weights;
}

// read the warehouse data file and create the ANN structures
int WarehouseUnit::ImportWarehouseUnit(const char *filename, bool appendFlag)
{
    DataFile file;
    if (file.ReadFile(filename)) return __LINE__;
    char *fileData = file.GetRawData();
    int fileDataLen = file.GetSize();
    return ImportWarehouseUnit(fileData, fileDataLen, appendFlag);
}

int WarehouseUnit::ImportWarehouseUnit(char *fileData, int fileDataLen, bool appendFlag)
{
    char **ptrs = new char *[fileDataLen / 2]; // this must be big enough and these files aren't so large that I need to worry about space
    int numTokens = DataFile::ReturnTokens(fileData, ptrs, fileDataLen / 2); // this should make the whole thing reasonably tolerant

    if (appendFlag == false)
    {
        m_numDrivers = GSUtil::Int(ptrs[0]);
        m_numBodies = GSUtil::Int(ptrs[1 + m_numDrivers]);
        m_numValuesPerBody = 13; // hardwired for import
        m_nDim = m_numBodies * m_numValuesPerBody;
        int numValues = (numTokens - (2 + m_numDrivers + m_numBodies));
        int lineLength = 1 + m_numDrivers + m_nDim;
        //std::cerr << "m_numDrivers=" << m_numDrivers << " m_numBodies=" << m_numBodies << " numTokens=" << numTokens << " numValues=" << numValues << " lineLength=" << lineLength << "\n";
        if (numValues % lineLength) return __LINE__;
        m_nPts = numValues / lineLength;
        int index = 1;
        int i, j;
        m_driverNames.clear();
        for (i = 0; i < m_numDrivers; i++) m_driverNames.push_back(ptrs[index++]);
        index++;
        m_bodyNames.clear();
        for (i = 0; i < m_numBodies; i++) m_bodyNames.push_back(ptrs[index++]);
        //double *times = new double[m_nPts];
        if (m_activations) delete [] m_activations;
        m_activations = new double[m_nPts * m_numDrivers];
        if (m_bodyData) delete [] m_bodyData;
        m_bodyData = new double[m_nPts * m_nDim];
        for (i = 0; i < m_nPts; i++)
        {
            index++; //times[i] = GSUtil::Double(ptrs[index++]);
            for (j = 0; j < m_numDrivers; j++) m_activations[i * m_numDrivers + j] = GSUtil::Double(ptrs[index++]);
            for (j = 0; j < m_nDim; j++) m_bodyData[i * m_nDim + j] = GSUtil::Double(ptrs[index++]);
        }
        //delete [] times;
        if (m_weights) delete [] m_weights;
        m_weights = new double[m_nDim];
        std::fill_n(m_weights, m_nDim, 1.0);
    }
    else
    {
        int numValues = (numTokens - (2 + m_numDrivers + m_numBodies));
        int lineLength = 1 + m_numDrivers + m_nDim;
        if (numValues % lineLength) return __LINE__;
        int nPts = numValues / lineLength;
        int old_nPts = m_nPts;
        m_nPts += nPts;
        int index = 1 + m_numDrivers + 1 + m_numBodies;
        int i, j;
        //double *times = new double[m_nPts];
        double *old_activations = m_activations;
        double *old_bodyData = m_bodyData;
        m_activations = new double[m_nPts * m_numDrivers];
        m_bodyData = new double[m_nPts * m_nDim];
        for (i = 0; i < old_nPts; i++)
        {
            for (j = 0; j < m_numDrivers; j++) m_activations[i * m_numDrivers + j] = old_activations[i * m_numDrivers + j];
            for (j = 0; j < m_nDim; j++) m_bodyData[i * m_nDim + j] = old_bodyData[i * m_nDim + j];
        }
        if (old_activations) delete [] old_activations;
        if (old_bodyData) delete [] old_bodyData;
        for (i = old_nPts; i < m_nPts; i++)
        {
            index++; //times[i] = GSUtil::Double(ptrs[index++]);
            for (j = 0; j < m_numDrivers; j++) m_activations[i * m_numDrivers + j] = GSUtil::Double(ptrs[index++]);
            for (j = 0; j < m_nDim; j++) m_bodyData[i * m_nDim + j] = GSUtil::Double(ptrs[index++]);
        }
        //delete [] times;
    }

    InitaliseWarehouse();

    delete [] ptrs;
    return 0;
}

void WarehouseUnit::SetDriverIDs(const char *nameList)
{
    int len = strlen(nameList);
    char *newNameList = new char[len];
    strcpy(newNameList, nameList);
    char **ptrs = new char *[len];
    m_numDrivers = DataFile::ReturnTokens(newNameList, ptrs, len);
    int index = 0;
    m_driverNames.clear();
    for (int i = 0; i < m_numDrivers; i++) m_driverNames.push_back(ptrs[index++]);
    delete [] ptrs;
    delete [] newNameList;
}

void WarehouseUnit::SetBodyIDs(const char *nameList)
{
    int len = strlen(nameList);
    char *newNameList = new char[len];
    strcpy(newNameList, nameList);
    char **ptrs = new char *[len];
    m_numBodies = DataFile::ReturnTokens(newNameList, ptrs, len);
    int index = 0;
    m_bodyNames.clear();
    for (int i = 0; i < m_numBodies; i++) m_bodyNames.push_back(ptrs[index++]);
    delete [] ptrs;
    delete [] newNameList;
}

void WarehouseUnit::SetActivations(int numPoints, int numDrivers, double *activations)
{
    m_nPts = numPoints;
    m_numDrivers = numDrivers;
    if (m_activations) delete [] m_activations;
    m_activations = new double[m_nPts * m_numDrivers];
     for (int i = 0; i < m_nPts; i++)
    {
        for (int j = 0; j < m_numDrivers; j++) m_activations[i * m_numDrivers + j] = activations[i * m_numDrivers + j];
    }
}

void WarehouseUnit::SetBodyData(int numPoints, int numBodies, int numValuesPerBody, double *bodyData)
{
    m_nPts = numPoints;
    m_numBodies = numBodies;
    m_numValuesPerBody = numValuesPerBody;
    m_nDim = m_numBodies * m_numValuesPerBody;
    if (m_bodyData) delete [] m_bodyData;
    m_bodyData = new double[m_nPts * m_nDim];
    for (int i = 0; i < m_nPts; i++)
    {
        for (int j = 0; j < m_nDim; j++) m_bodyData[i * m_nDim + j] = bodyData[i * m_nDim + j];
    }
}

void WarehouseUnit::SetWeights(int numBodies, int numValuesPerBody, double *weights)
{
    m_numBodies = numBodies;
    m_numValuesPerBody = numValuesPerBody;
    m_nDim = m_numBodies * m_numValuesPerBody;
    if (m_weights) delete [] m_weights;
    m_weights = new double[m_nDim];
    std::copy(weights, weights + m_nDim, m_weights);
}
// initalise the data search structures using the pre-loaded data
void WarehouseUnit::InitaliseWarehouse()
{
    int i, j;

    if (m_usePCA)
    {
        // do the PCA to find out how many dimensions we need
        ColumnMajorArray dataMatrix(m_nPts, m_nDim);
        for (i = 0; i < m_nPts; i++)
        {
            for (j = 0; j < m_nDim; j++)
            {
                dataMatrix.Set(i, j, m_bodyData[i * m_nDim + j] * m_weights[j]);
            }
        }
        m_pca.DoPCA(dataMatrix);

        m_numDimensionsWanted = m_nDim;
        ColumnMajorArray *eigenValues = m_pca.EigenValues();
        double threshold = m_eigenvalueThreshold * eigenValues->Sum();
        for (i = 0; i < m_nDim; i++)
        {
            if (eigenValues->Get(0, i) < threshold)
            {
                m_numDimensionsWanted = i;
                break;
            }
        }
        if (m_numDimensionsWanted < 1) m_numDimensionsWanted = 1;
        std::cerr << "m_nDim = " << m_nDim << " m_numDimensionsWanted = " << m_numDimensionsWanted << "\n";

        // do the dimension reduction
        if (m_dataPts) annDeallocPts(m_dataPts);
        m_dataPts = annAllocPts(m_nPts, m_numDimensionsWanted); // allocate data points
        ColumnMajorArray *pca_scores = m_pca.Scores();
        for (i = 0; i < m_nPts; i++)
        {
            for (j = 0; j < m_numDimensionsWanted; j++)
            {
                m_dataPts[i][j] = pca_scores->Get(i, j);
            }
        }
    }
    else
    {
        m_numDimensionsWanted = m_nDim;
        if (m_dataPts) annDeallocPts(m_dataPts);
        m_dataPts = annAllocPts(m_nPts, m_numDimensionsWanted); // allocate data points
        for (i = 0; i < m_nPts; i++)
        {
            for (j = 0; j < m_numDimensionsWanted; j++)
            {
                m_dataPts[i][j] = m_bodyData[i * m_nDim + j] * m_weights[j];
            }
        }

    }

    m_kdTree = new ANNkd_tree(                               // build search structure
                              m_dataPts,                     // the data points
                              m_nPts,                        // number of points
                              m_numDimensionsWanted);        // dimension of space

    // allocate space for the query items
    if (m_bodyQueryData) delete [] m_bodyQueryData;
    m_bodyQueryData = new double [m_nDim];
    std::fill_n(m_bodyQueryData, m_nDim, 0);
    if (m_queryPt) annDeallocPt(m_queryPt);
    m_queryPt = annAllocPt(m_numDimensionsWanted);
    m_nNN = 1; // only one want one nearest point
    if (m_nnIdx) delete [] m_nnIdx;
    m_nnIdx = new ANNidx[m_nNN];
    if (m_dists) delete [] m_dists;
    m_dists = new ANNdist[m_nNN];
}

// set up the query from the current body positions
void WarehouseUnit::SetBodyQueryData(const std::map<std::string, std::unique_ptr<Body> > &bodyMap)
{
    int i = 0;
    Body *rootBody, *body;

    // this code is modified from the WarehouseSave function to fill out the query routine
    rootBody = bodyMap.at(m_bodyNames[0]).get();
    pgd::Vector3 pos, vel, avel;
    pgd::Quaternion quat;
    rootBody->GetRelativePosition(nullptr, &pos);
    rootBody->GetRelativeQuaternion(nullptr, &quat);
    rootBody->GetRelativeLinearVelocity(nullptr, &vel);
    rootBody->GetRelativeAngularVelocity(nullptr, &avel);
    double angle = QGetAngle(quat);
    pgd::Vector3 axis = QGetAxis(quat);
    m_bodyQueryData[i++] = pos.x; m_bodyQueryData[i++] = pos.y; m_bodyQueryData[i++] = pos.z;
    m_bodyQueryData[i++] = angle; m_bodyQueryData[i++] = axis.x; m_bodyQueryData[i++] = axis.y; m_bodyQueryData[i++] = axis.z;
    m_bodyQueryData[i++] = vel.x; m_bodyQueryData[i++] = vel.y; m_bodyQueryData[i++] = vel.z;
    m_bodyQueryData[i++] = avel.x; m_bodyQueryData[i++] = avel.y; m_bodyQueryData[i++] = avel.z;
    // and now the rest of the bodies
    for (unsigned int j = 1; j < m_bodyNames.size(); j++)
    {
        body = bodyMap.at(m_bodyNames[j]).get();
        body->GetRelativePosition(rootBody, &pos);
        body->GetRelativeQuaternion(rootBody, &quat);
        body->GetRelativeLinearVelocity(rootBody, &vel);
        body->GetRelativeAngularVelocity(rootBody, &avel);
        angle = QGetAngle(quat);
        axis = QGetAxis(quat);
        m_bodyQueryData[i++] = pos.x; m_bodyQueryData[i++] = pos.y; m_bodyQueryData[i++] = pos.z;
        m_bodyQueryData[i++] = angle; m_bodyQueryData[i++] = axis.x; m_bodyQueryData[i++] = axis.y; m_bodyQueryData[i++] = axis.z;
        m_bodyQueryData[i++] = vel.x; m_bodyQueryData[i++] = vel.y; m_bodyQueryData[i++] = vel.z;
        m_bodyQueryData[i++] = avel.x; m_bodyQueryData[i++] = avel.y; m_bodyQueryData[i++] = avel.z;
    }
}

// set up the query from an existing body query array
void WarehouseUnit::SetBodyQueryData(double *bodyQueryData)
{
    for (int iDim = 0; iDim < m_nDim; iDim++) m_bodyQueryData[iDim] = bodyQueryData[iDim];
}

// do the search
int WarehouseUnit::DoSearch()
{
    int j;
    if (m_usePCA)
    {
        ColumnMajorArray data(1, m_nDim);
        ColumnMajorArray scores(1, m_numDimensionsWanted);
        for (j = 0; j < m_nDim; j++) data.Set(0, j, m_bodyQueryData[j] * m_weights[j]);
        m_pca.CalculateScores(data, 0, m_numDimensionsWanted, &scores);
        for (j = 0; j < m_numDimensionsWanted; j++) m_queryPt[j] = scores.Get(0, j);
    }
    else
    {
        for (j = 0; j < m_nDim; j++) m_queryPt[j] = m_bodyQueryData[j] * m_weights[j];
    }

    // do the query

    m_kdTree->annkSearch(                     // search
                         m_queryPt,           // query point
                         m_nNN,               // number of near neighbors
                         m_nnIdx,             // nearest neighbors (returned)
                         m_dists,             // distance (returned)
                         m_eps);              // error bound

    // std::cerr << "m_dists[0] = " << m_dists[0] << "\n";

    return m_nNN;
}

// returns the pointer to the currently selected group of activations
double* WarehouseUnit::GetCurrentActivations()
{
    return m_activations + m_nnIdx[0] * m_numDrivers;
}


// do an ANN search using the current body positions
int Warehouse::DoSearch(const std::map<std::string, std::unique_ptr<Body>> &bodyMap)
{
    double distance, primaryDistance = 0;
    unsigned int i;
    // are we currently using the primary unit?
    if (m_CurrentUnit == 0)
    {
        m_warehouseList[m_CurrentUnit]->SetBodyQueryData(bodyMap);
        m_warehouseList[m_CurrentUnit]->DoSearch();
        primaryDistance = m_warehouseList[m_CurrentUnit]->GetNearestNeighbourDistance();
        if (primaryDistance <= m_UnitIncreaseThreshold)
        {
            m_NearestNeighbourDistance = primaryDistance; // business as usual
            m_LastSearchResult = 0;
            return m_LastSearchResult;
        }
        // poor match to primary so try the other units in turn
        for (i = m_CurrentUnit + 1; i < m_warehouseList.size(); i++)
        {
            m_warehouseList[i]->SetBodyQueryData(bodyMap);
            m_warehouseList[i]->DoSearch();
            distance = m_warehouseList[i]->GetNearestNeighbourDistance();
            if (distance <= m_UnitIncreaseThreshold)
            {
                m_NearestNeighbourDistance = distance; //change to a new unit
                m_CurrentUnit = i;
                m_LastSearchResult = 1;
                return m_LastSearchResult;
            }
        }
        // nothing good so fall back on primary
        m_NearestNeighbourDistance = primaryDistance;
        m_LastSearchResult = 2;
        return m_LastSearchResult;
    }

    // check to see whether we should move back to a higher priority unit
    for (i = 0; i < m_CurrentUnit; i++)
    {
        m_warehouseList[i]->SetBodyQueryData(bodyMap);
        m_warehouseList[i]->DoSearch();
        distance = m_warehouseList[i]->GetNearestNeighbourDistance();
        if (i == 0) primaryDistance = distance;
        if (distance <= m_UnitIncreaseThreshold * m_UnitDecreaseThresholdFactor)
        {
            m_NearestNeighbourDistance = distance; //change to a new unit
            m_CurrentUnit = i;
            m_LastSearchResult = 3;
            return m_LastSearchResult;
        }
    }

    // check whether we should stay where we are
    m_warehouseList[m_CurrentUnit]->SetBodyQueryData(bodyMap);
    m_warehouseList[m_CurrentUnit]->DoSearch();
    distance = m_warehouseList[m_CurrentUnit]->GetNearestNeighbourDistance();
    if (distance <= m_UnitIncreaseThreshold)
    {
        m_NearestNeighbourDistance = distance; // business as usual
        m_LastSearchResult = 4;
        return m_LastSearchResult;
    }

    // poor match to current so try the other units in turn
    for (i = m_CurrentUnit + 1; i < m_warehouseList.size(); i++)
    {
        m_warehouseList[i]->SetBodyQueryData(bodyMap);
        m_warehouseList[i]->DoSearch();
        distance = m_warehouseList[i]->GetNearestNeighbourDistance();
        if (distance <= m_UnitIncreaseThreshold)
        {
            m_NearestNeighbourDistance = distance; //change to a new unit
            m_CurrentUnit = i;
            m_LastSearchResult = 5;
            return m_LastSearchResult;
        }
    }
    // nothing good so fall back on primary
    m_NearestNeighbourDistance = primaryDistance;
    m_CurrentUnit = 0;
    m_LastSearchResult = 6;
    return m_LastSearchResult;
}

WarehouseUnit *Warehouse::NewWarehouseUnit(unsigned int index)
{
    if (index != m_warehouseList.size()) return nullptr;
    m_warehouseList.push_back(new WarehouseUnit());
    m_warehouseList[index]->SetUsePCA(m_UsePCA);
    return m_warehouseList[index];
}

// this function initialises the data in the object based on the contents
// of an xml_node node. It uses information from the simulation as required
// to satisfy dependencies
// it returns nullptr on success and a pointer to lastError() on failure
std::string *Warehouse::createFromAttributes()
{
//    char buffer[512];
//    std::string ID, driverIDs, bodyIDs, UsePCA;
//    unsigned int numWarehouseUnits = 0, numDrivers, numBodies, numPoints, n, numValuesPerBody;
//    rapidxml::xml_attribute<char> * attr = nullptr;
//    double *activations, *bodyData, *weights;
//    WarehouseUnit *unit;

//    if (getAttribute("ID"s, &ID) == nullptr) return lastErrorPtr();
//    this->SetName(ID);

//    if (getAttribute("UsePCA"s, &UsePCA) == nullptr) return lastErrorPtr();
//    m_UsePCA = GSUtil::Bool(UsePCA.c_str());

//    if (getAttribute("DriverIDs"s, &driverIDs) == nullptr) return lastErrorPtr();
//    if (getAttribute("BodyIDs"s, &bodyIDs) == nullptr) return lastErrorPtr();

//    while (true)
//    {
//        sprintf(buffer, "Activations%d", numWarehouseUnits);
//        attr = FindXMLAttribute(node, buffer);
//        if (attr == nullptr) break;

//        unit = NewWarehouseUnit(numWarehouseUnits);
//        unit->SetDriverIDs(driverIDs.c_str());
//        unit->SetBodyIDs(bodyIDs.c_str());
//        n = GSUtil::CountTokens(attr->value());
//        activations = new double[n];
//        GSUtil::Double(attr->value(), n, activations);
//        numDrivers = unit->GetNumDrivers();
//        numBodies = unit->GetNumBodies();
//        numPoints = n / numDrivers;
//        unit->SetActivations(numPoints, numDrivers, activations);
//        delete activations;
//        if (n % numDrivers) { setLastError("Warehouse ID=\""s + name() +"\" n % numDrivers != 0"s); return lastErrorPtr(); }

//        sprintf(buffer, "BodyData%d", numWarehouseUnits);
//        attr = FindXMLAttribute(node, buffer);
//        if (attr == 0) return lastErrorPtr();
//        n = GSUtil::CountTokens(attr->value());
//        bodyData = new double[n];
//        GSUtil::Double(attr->value(), n, bodyData);
//        numValuesPerBody = n / (numPoints * numBodies);
//        unit->SetBodyData(numPoints, numBodies, numValuesPerBody, bodyData);
//        delete bodyData;
//        if (n % (numPoints * numBodies)) { setLastError("Warehouse ID=\""s + name() +"\" n % (numPoints * numBodies) != 0"s); return lastErrorPtr(); }

//        sprintf(buffer, "Weights%d", numWarehouseUnits);
//        attr = FindXMLAttribute(node, buffer);
//        if (attr == nullptr) return lastErrorPtr();
//        n = GSUtil::CountTokens(attr->value());
//        if (n != numBodies * numValuesPerBody) { setLastError("Warehouse ID=\""s + name() +"\" n != numBodies * numValuesPerBody"s); return lastErrorPtr(); }
//        weights = new double[n];
//        GSUtil::Double(attr->value(), n, weights);
//        unit->SetWeights(numBodies, numValuesPerBody, weights);
//        delete weights;

//        unit->SetUsePCA(m_UsePCA);
//        unit->InitaliseWarehouse();
//        numWarehouseUnits++;
//    }
//    if (numWarehouseUnits == 0) { setLastError("Warehouse ID=\""s + name() +"\" numWarehouseUnits == 0"s); return lastErrorPtr(); }

    return nullptr;
}

void Warehouse::saveToAttributes()
{
//    rapidxml::xml_node<char> *newNode = parent->document()->allocate_node(rapidxml::node_element, "WAREHOUSE");
//    rapidxml::xml_attribute<char> *newAttr = parent->document()->allocate_attribute("ID", name().c_str());
//    newNode->append_attribute(newAttr);
//    newAttr = parent->document()->allocate_attribute("UsePCA", m_UsePCA ? "true" : "false");
//    newNode->append_attribute(newAttr);

//    WarehouseUnit *unit = GetWarehouseUnit(0);
//    std::stringstream ss, ss2;
//    ss.precision(7);
//    ss << std::scientific;
//    std::vector<std::string> *driverNames = unit->GetDriverNames();
//    for (unsigned int i = 0; i < driverNames->size(); i++)
//    {
//        if (i) ss << ' ';
//        ss << driverNames->at(i);
//    }
//    newAttr = parent->document()->allocate_attribute("DriverIDs", ss.str().c_str());
//    newNode->append_attribute(newAttr);

//    std::vector<std::string> *bodyNames = unit->GetBodyNames();
//    ss.str(""); ss.clear(); // clear the string and then clear any error status
//    for (unsigned int i = 0; i < bodyNames->size(); i++)
//    {
//        if (i) ss << ' ';
//        ss << bodyNames->at(i);
//    }
//    newAttr = parent->document()->allocate_attribute("BodyIDs", ss.str().c_str());
//    newNode->append_attribute(newAttr);

//    for (unsigned int i = 0; i < GetNumWarehouseUnits(); i++)
//    {
//        unit = GetWarehouseUnit(i);
//        int numPoints, numDrivers, numBodies, numValuesPerBody;
//        double *activations = unit->GetActivations(&numPoints, &numDrivers);
//        double *bodyData = unit->GetBodyData(&numPoints, &numBodies, &numValuesPerBody);
//        double *weights = unit->GetWeights(&numBodies, &numValuesPerBody);
//        ss.str(""); ss.clear(); // clear the string and then clear any error status
//        for (int iPoints = 0; iPoints < numPoints; iPoints++)
//        {
//            for (int iDrivers = 0; iDrivers < numDrivers; iDrivers++)
//            {
//                if (iDrivers || iPoints) ss << ' ';
//                ss << *activations++;
//            }
//        }
//        ss2.str(""); ss2.clear();
//        ss2 << "Activations" << i;
//        newAttr = parent->document()->allocate_attribute(ss2.str().c_str(), ss.str().c_str());
//        newNode->append_attribute(newAttr);

//        ss.str(""); ss.clear(); // clear the string and then clear any error status
//        for (int iPoints = 0; iPoints < numPoints; iPoints++)
//        {
//            for (int iDim = 0; iDim < (numBodies * numValuesPerBody); iDim++)
//            {
//                if (iDim || iPoints) ss << ' ';
//                ss << *bodyData++;
//            }
//        }
//        ss2.str(""); ss2.clear();
//        ss2 << "BodyData" << i;
//        newAttr = parent->document()->allocate_attribute(ss2.str().c_str(), ss.str().c_str());
//        newNode->append_attribute(newAttr);

//        ss.str(""); ss.clear(); // clear the string and then clear any error status
//        for (int iDim = 0; iDim < (numBodies * numValuesPerBody); iDim++)
//        {
//            if (iDim) ss << ' ';
//            ss << *weights++;
//        }
//        ss2.str(""); ss2.clear();
//        ss2 << "Weights" << i;
//        newAttr = parent->document()->allocate_attribute(ss2.str().c_str(), ss.str().c_str());
//        newNode->append_attribute(newAttr);

//    }
}

std::string Warehouse::dumpToString()
{
    std::stringstream ss;
    ss.precision(17);
    ss.setf(std::ios::scientific);
    if (firstDump())
    {
        setFirstDump(false);
        ss << "Time\tCurrentUnit\tSearchResult";
        for (unsigned int i = 0; i < m_warehouseList.size(); i++)
        {
            ss << "\tUnit" << i << "Distance\tUnit" << i << "Index";
        }
        ss << "\n";
    }
    ss << simulation()->GetTime() << "\t" << m_CurrentUnit << "\t" << m_LastSearchResult;
    double *bodyQueryData = m_warehouseList[0]->GetBodyQueryData();
    for (unsigned int i = 0; i < m_warehouseList.size(); i++)
    {
        if (i) m_warehouseList[i]->SetBodyQueryData(bodyQueryData);
        m_warehouseList[i]->DoSearch(); // need to do this because it may not have happened to all units
        ss << "\t" << m_warehouseList[i]->GetNearestNeighbourDistance()
           << "\t" << m_warehouseList[i]->GetNearestNeighbourIndex();
    }
    ss << "\n";
    return ss.str();
}


#endif
