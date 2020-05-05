/*
 *  FacetedObject.h
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 13/09/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

#ifndef FacetedObject_h
#define FacetedObject_h

#include "NamedObject.h"
#include "MeshStore.h"
#include "PGDMath.h"

#include "ode/ode.h"

#include <QColor>
#include <QOpenGLBuffer>
#include <QMatrix4x4>

class SimulationWidget;
class DataFile;
class TrimeshGeom;

class FacetedObject
{
public:
    FacetedObject();
    virtual ~FacetedObject();

    // destructor is needed to make sure it is virtual in subclasses but I don't need the rest of the rule of 5
    FacetedObject(const FacetedObject&) = delete;
    FacetedObject(FacetedObject&&) = delete;
    FacetedObject& operator=(const FacetedObject&) = delete;
    FacetedObject& operator=(FacetedObject&&) = delete;

    struct OBJMaterial
    {
        double Ns = {0};
        double Ka[3] = {0, 0, 0};
        double Kd[3] = {0, 0, 0};
        double Ks[3] = {0, 0, 0};
        double Ke[3] = {0, 0, 0};
        double Ni = {0};
        double d = {0};
        int illum = {0};
    };

    virtual void Draw();

    int ParseOBJFile(const std::string &filename);
    int ParseOBJMaterialFile(const std::string &filename, std::map<std::string, OBJMaterial> *materialMap);
    int ParsePLYFile(const std::string &filename);

    int ReadFromMemory(const char *data, size_t len, bool binary, const std::string &meshName);
    void SaveToMemory(std::vector<char> *data, bool binary);

    int ReadFromResource(const QString &resourceName);

    virtual void WritePOVRay(std::ostringstream &theString);
    virtual void WritePOVRay(std::string filename);
    virtual void WriteOBJFile(std::ostringstream &out);
    virtual void WriteOBJFile(std::string filename);

    size_t GetNumVertices() const;
    const double *GetVertex(size_t i) const;
    const double *GetNormal(size_t i) const;
    const double *GetVertexList() const;
    const double *GetNormalList() const;
    const double *GetColourList() const;
    size_t GetNumTriangles() const;
    const double *GetTriangle(size_t i) const;
    const double *GetDisplayPosition() const;
    const double *GetDisplayRotation() const;
    const double *GetDisplayScale() const;

    void AddPolygon(const double *vertices, size_t nSides, const double *normals = nullptr);
    void AddTriangle(const double *vertices, const double *normals = nullptr);
    void AddPolygon(const float *floatVertices, size_t nSides, const float *floatNormals = nullptr);
    void AddTriangle(const float *floatVertices, const float *floatNormals = nullptr);
    void AddFacetedObject(const FacetedObject *object, bool useDisplayRotation, bool useDirectAccess);

    // static utilities
    static void ComputeFaceNormal(const double *v1, const double *v2, const double *v3, double normal[3]);

    // manipulation functions
    void Move(double x, double y, double z);
    void Scale(double x, double y, double z);
    void Mirror(bool x, bool y, bool z);
    void SwapAxes(int axis1, int axis2);
    void Rotate(double x, double y, double z, double angleDegrees);

    // utility
    void ReverseWinding();
    void AllocateMemory(size_t numTriangles);
    void ApplyDisplayTransformation(const pgd::Vector inVec, pgd::Vector *outVec);
    void ApplyDisplayRotation(const pgd::Vector inVec, pgd::Vector *outVec);

    // ODE link
    void CalculateTrimesh(double **vertices, int *numVertices, int *vertexStride, dTriIndex **triIndexes, int *numTriIndexes, int *triStride);
    void CalculateTrimesh(float **vertices, int *numVertices, int *vertexStride, dTriIndex **triIndexes, int *numTriIndexes, int *triStride);
    void CalculateMassProperties(dMass *m, double density, bool clockwise);

    double *lowerBound();
    double *upperBound();

    SimulationWidget *simulationWidget() const;
    void setSimulationWidget(SimulationWidget *simulationWidget);

    void setBlendColour(const QColor &blendColour, double blendFraction);
    QColor blendColour() const;
    double blendFraction() const;

    int FindIntersection(const pgd::Vector &rayOrigin, const pgd::Vector &rayVector, std::vector<pgd::Vector> *intersectionCoordList, std::vector<size_t> *intersectionIndexList) const;
    static bool RayIntersectsTriangle(const pgd::Vector &rayOrigin, const pgd::Vector &rayVector, const pgd::Vector &vertex0, const pgd::Vector &vertex1, const pgd::Vector &vertex2, pgd::Vector *outIntersectionPoint);
    static bool HitBoundingBox(const double minB[3], const double maxB[3], const double origin[3], const double dir[3], double coord[3]);

    void SetDisplayPosition(double x, double y, double z);
    void SetDisplayScale(double x, double y, double z);
    void SetDisplayRotation(const dMatrix3 R);
    void SetDisplayRotationFromQuaternion(const dQuaternion q);

    bool visible() const;
    void setVisible(bool visible);

    const QMatrix4x4 &model();
    void setModel(const QMatrix4x4 &model);


private:

    std::vector<double> m_vertexList;
    std::vector<double> m_normalList;
    std::vector<double> m_colourList;
    bool m_useRelativeOBJ = false;
    bool m_badMesh = false;
    dVector3 m_lowerBound = {DBL_MAX, DBL_MAX, DBL_MAX, 0};
    dVector3 m_upperBound = {-DBL_MAX, -DBL_MAX, -DBL_MAX, 0};

    dVector3 m_displayPosition = {0, 0, 0, 0};
    dVector3 m_displayScale = {1, 1, 1, 0};
    dMatrix3 m_displayRotation = {1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0};
    bool m_visible = true;
    QMatrix4x4 m_model;
    bool m_modelValid = false;

    std::ofstream *m_POVRayFile = nullptr;
    std::ofstream *m_OBJFile = nullptr;
    std::string m_OBJName;
    std::string m_filename;
    size_t m_vertexOffset = 0;

    QColor m_blendColour = {255, 255, 255, 255};
    double m_blendFraction = 0;
    SimulationWidget *m_simulationWidget = nullptr;
    QOpenGLBuffer m_VBO;
    bool m_VBOAllocated = false;

    static MeshStore m_meshStore;

};

#endif
