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

#include "MeshStore.h"

#include "ode/ode.h"

#include <QEntity>
#include <QTransform>
#include <QColor>

namespace Qt3DRender {
class QLayer;
}
class SceneEffect;

class DataFile;
class TrimeshGeom;
class ostringstream;
namespace pgd {
class Vector;
}

class FacetedObject: public Qt3DCore::QEntity
{
    Q_OBJECT
public:
    FacetedObject(Qt3DCore::QNode *parent);
    virtual ~FacetedObject() Q_DECL_OVERRIDE;

    void InitialiseEntity();

    int ParseOBJFile(const std::string &filename);
    int ParsePLYFile(const std::string &filename);

    int ReadFromMemory(const char *data, size_t len, bool binary, const std::string &meshName);
    void SaveToMemory(std::vector<char> *data, bool binary);

    virtual void WritePOVRay(std::ostringstream &theString);
    virtual void WritePOVRay(std::string filename);
    virtual void WriteOBJFile(std::ostringstream &out);
    virtual void WriteOBJFile(std::string filename);

    size_t GetNumVertices()
    {
        return mVertexList.size() / 3;
    }
    double *GetVertex(size_t i)
    {
        return &mVertexList.at(3 * i);
    }
    double *GetNormal(size_t i)
    {
        return &mNormalList.at(3 * i);
    }
    double *GetVertexList()
    {
        return mVertexList.data();
    }
    double *GetNormalList()
    {
        return mNormalList.data();
    }

    void AddPolygon(const double *vertices, size_t nSides, const double *normals = nullptr);
    void AddTriangle(const double *vertices, const double *normals = nullptr);
    void AddPolygon(const float *floatVertices, size_t nSides, const float *floatNormals = nullptr);
    void AddTriangle(const float *floatVertices, const float *floatNormals = nullptr);
    void AddFacetedObject(FacetedObject *object, bool useDisplayRotation);

    size_t GetNumTriangles()
    {
        return mVertexList.size() / 9;
    }
    double *GetTriangle(size_t i)
    {
        return &mVertexList.at(9 * i);
    }

    void SetDisplayPosition(double x, double y, double z);
    void SetDisplayRotation(const dMatrix3 R);
    void SetDisplayRotationFromQuaternion(const dQuaternion q);
    void SetDisplayRotationFromAxis(double x, double y, double z);

    const double *GetDisplayPosition()
    {
        return m_DisplayPosition;
    }
    const double *GetDisplayRotation()
    {
        return m_DisplayRotation;
    }

    // static utilities
    static void ComputeFaceNormal(const double *v1, const double *v2, const double *v3,
                                  double normal[3]);
    static size_t OBJFaceVertexDecode(char *string, size_t *output);

    // manipulation functions
    void Move(double x, double y, double z);
    void Scale(double x, double y, double z);
    void Mirror(bool x, bool y, bool z);
    void SwapAxes(int axis1, int axis2);
    void RotateAxes(int axis0, int axis1, int axis2);

    // utility
    void ReverseWinding();
    void AllocateMemory(size_t numTriangles);

    // ODE link
    void CalculateTrimesh(double **vertices, int *numVertices, int *vertexStride,
                          dTriIndex **triIndexes, int *numTriIndexes, int *triStride);
    void CalculateTrimesh(float **vertices, int *numVertices, int *vertexStride, dTriIndex **triIndexes,
                          int *numTriIndexes, int *triStride);
    void CalculateMassProperties(dMass *m, double density, bool clockwise);

    void SetBadMesh(bool v)
    {
        m_BadMesh = v;
    }

    QColor GetColour() const
    {
        return m_colour;
    }
    void SetColour(const QColor &colour)
    {
        m_colour = colour;
    }

    double *lowerBound();
    double *upperBound();


    Qt3DRender::QLayer *layer() const;
    void setLayer(Qt3DRender::QLayer *layer);

    SceneEffect *effect() const;
    void setEffect(SceneEffect *effect);

private:

    std::vector<double> mVertexList;
    std::vector<double> mNormalList;
    std::vector<double> mColourList;
    bool m_UseRelativeOBJ = false;
    bool m_BadMesh = false;
    dVector3 m_lowerBound = {DBL_MAX, DBL_MAX, DBL_MAX, 0};
    dVector3 m_upperBound = {-DBL_MAX, -DBL_MAX, -DBL_MAX, 0};

    dVector3 m_DisplayPosition = {0, 0, 0, 0};
    dMatrix3 m_DisplayRotation = {1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0};

    std::ofstream *m_POVRayFile = nullptr;
    std::ofstream *m_OBJFile = nullptr;
    std::string m_OBJName;
    std::string m_filename;
    size_t m_vertexOffset = 0;

    Qt3DCore::QTransform *m_transform = nullptr;
    QColor m_colour;

    static MeshStore m_meshStore;

    Qt3DRender::QLayer *m_layer = nullptr;
    SceneEffect *m_effect = nullptr;

};

#endif
