/*
 *  FacetedObject.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 13/09/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

#include "FacetedObject.h"
#include "FacetedSphere.h"
#include "DataFile.h"
#include "GSUtil.h"
#include "GLUtils.h"
#include "PGDMath.h"
#include "DeferredRenderer/sceneeffect.h"

#define TINYPLY_IMPLEMENTATION
#include "tinyply.h"
#include "ode/ode.h"

#include <QFileInfo>
#include <QDir>
#include <QMaterial>
#include <QTransform>
#include <QGeometryRenderer>
#include <QBuffer>
#include <QPerVertexColorMaterial>
#include <QAttribute>
#include <QDebug>
#include <QLayer>
#include <QParameter>

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cmath>
#include <stdlib.h>
#include <string.h>
#include <cfloat>
#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <cstdlib>


// create object
FacetedObject::FacetedObject(Qt3DCore::QNode *parent)
    : Qt3DCore::QEntity(parent)
{
}

// destroy object
FacetedObject::~FacetedObject()
{
}

// parse an OBJ file to a FacetedObject
// returns 0 on success
int FacetedObject::ParseOBJFile(const std::string &filename)
{
    m_filename = filename;
    MeshStoreObject *meshStoreObject = m_meshStore.getMesh(filename);
    if (meshStoreObject)
    {
        mVertexList = meshStoreObject->vertexList;
        mNormalList = meshStoreObject->normalList;
        mColourList = meshStoreObject->colourList;
        m_lowerBound[0] = meshStoreObject->lowerBound[0];
        m_lowerBound[1] = meshStoreObject->lowerBound[1];
        m_lowerBound[2] = meshStoreObject->lowerBound[2];
        m_upperBound[0] = meshStoreObject->upperBound[0];
        m_upperBound[1] = meshStoreObject->upperBound[1];
        m_upperBound[2] = meshStoreObject->upperBound[2];
        return 0;
    }

    DataFile theFile;
    if (theFile.ReadFile(filename) == true) return __LINE__;
    size_t bufferSizeLimit = theFile.GetSize(); // if the file has any content it will have fewer lines than this
    std::vector<char *>linePtrs;
    linePtrs.reserve(size_t(bufferSizeLimit));
    size_t lineCount = DataFile::ReturnLines(theFile.GetRawData(), linePtrs.data(), bufferSizeLimit);

    std::vector<pgd::Vector> vertexList;
    vertexList.reserve(bufferSizeLimit);
    std::vector<pgd::Vector> normalList;
    normalList.reserve(bufferSizeLimit);
    struct Triangle
    {
        size_t vertex[3];
        size_t normal[3];
    };
    std::vector<Triangle> triangleList;
    triangleList.reserve(bufferSizeLimit);
    std::vector<char *> tokens;
    tokens.reserve(bufferSizeLimit);
    pgd::Vector vertex;
    Triangle triangle;

    // parse the lines
    for  (size_t i = 0; i < lineCount; i++)
    {
        // vertices
        if (linePtrs.data()[i][0] == 'v' && linePtrs.data()[i][1] == ' ')
        {
            size_t numTokens = DataFile::ReturnTokens(linePtrs.data()[i], tokens.data(), 4);
            if (numTokens > 3)
            {
                vertex.x = atof(tokens.data()[1]);
                vertex.y = atof(tokens.data()[2]);
                vertex.z = atof(tokens.data()[3]);
                vertexList.push_back(vertex);
                if (vertex.x < m_lowerBound[0]) m_lowerBound[0] = vertex.x;
                if (vertex.y < m_lowerBound[1]) m_lowerBound[1] = vertex.y;
                if (vertex.z < m_lowerBound[2]) m_lowerBound[2] = vertex.z;
                if (vertex.x > m_upperBound[0]) m_upperBound[0] = vertex.x;
                if (vertex.y > m_upperBound[1]) m_upperBound[1] = vertex.y;
                if (vertex.z > m_upperBound[2]) m_upperBound[2] = vertex.z;
            }
        }

        // normals
        if (linePtrs.data()[i][0] == 'v' && linePtrs.data()[i][1] == 'n')
        {
            size_t numTokens = DataFile::ReturnTokens(linePtrs.data()[i], tokens.data(), 4);
            if (numTokens > 3)
            {
                vertex.x = atof(tokens.data()[1]);
                vertex.y = atof(tokens.data()[2]);
                vertex.z = atof(tokens.data()[3]);
                normalList.push_back(vertex);
            }
        }

        // faces
        if (linePtrs.data()[i][0] == 'f' && linePtrs.data()[i][1] == ' ')
        {
            size_t numTokens = DataFile::ReturnTokens(linePtrs.data()[i], tokens.data(), bufferSizeLimit);
            size_t faceElementCount;
            size_t output[3];
            if (numTokens >= 4)
            {
                // do first triangle
                faceElementCount = OBJFaceVertexDecode(tokens.data()[1], output);
                if (faceElementCount == 1)
                {
                    triangle.vertex[0] = output[0] - 1;
                    triangle.vertex[1] = strtoull(tokens.data()[2], nullptr, 10) - 1;
                    triangle.vertex[2] = strtoull(tokens.data()[3], nullptr, 10) - 1;
                    triangle.normal[0] = SIZE_MAX;
                    triangle.normal[1] = SIZE_MAX;
                    triangle.normal[2] = SIZE_MAX;
                }
                else if (faceElementCount == 3)
                {
                    triangle.vertex[0] = output[0] - 1;
                    triangle.normal[0] = output[2] - 1;
                    OBJFaceVertexDecode(tokens.data()[2], output);
                    triangle.vertex[1] = output[0] - 1;
                    triangle.normal[1] = output[2] - 1;
                    OBJFaceVertexDecode(tokens.data()[3], output);
                    triangle.vertex[2] = output[0] - 1;
                    triangle.normal[2] = output[2] - 1;
                }
                else
                {
                    continue;
                }
                triangleList.push_back(triangle);
                if (m_BadMesh)   // currently duplicate the polygon but with reversed winding but this could be improved
                {
                    std::swap(triangle.vertex[1], triangle.vertex[2]);
                    std::swap(triangle.normal[1], triangle.normal[2]);
                    triangleList.push_back(triangle);
                }
                for (size_t j = 3; j < size_t(numTokens - 1); j++)
                {
                    // do subsequent triangles
                    if (faceElementCount == 1)
                    {
                        triangle.vertex[1] = strtoull(tokens.data()[j], nullptr, 10) - 1;
                        triangle.vertex[2] = strtoull(tokens.data()[j + 1], nullptr, 10) - 1;
                        triangle.normal[1] = SIZE_MAX;
                        triangle.normal[2] = SIZE_MAX;
                    }
                    else if (faceElementCount == 3)
                    {
                        OBJFaceVertexDecode(tokens.data()[j], output);
                        triangle.vertex[1] = output[0] - 1;
                        triangle.normal[1] = output[2] - 1;
                        OBJFaceVertexDecode(tokens.data()[j + 1], output);
                        triangle.vertex[2] = output[0] - 1;
                        triangle.normal[2] = output[2] - 1;
                    }
                    else
                    {
                        continue;
                    }
                    triangleList.push_back(triangle);
                    if (m_BadMesh)   // currently duplicate the polygon but with reversed winding but this could be improved
                    {
                        std::swap(triangle.vertex[1], triangle.vertex[2]);
                        std::swap(triangle.normal[1], triangle.normal[2]);
                        triangleList.push_back(triangle);
                    }
                }
            }
        }
    }

//    std::cerr << filename << "\n";
//    std::cerr << vertexList.size() << "  vertices loaded\n";
//    std::cerr << normalList.size() << " normals loaded\n";
//    std::cerr << triangleList.size() << " triangles loaded\n";

    mVertexList.clear();
    mVertexList.reserve(triangleList.size() * 9);
    mNormalList.clear();
    mNormalList.reserve(triangleList.size() * 9);
    mColourList.clear();
    mColourList.reserve(triangleList.size() * 9);
    double colour[3] = {m_colour.redF(), m_colour.greenF(), m_colour.blueF() };
    pgd::Vector normal;
    for (auto it : triangleList)
    {
        mVertexList.push_back(vertexList[it.vertex[0]].x);
        mVertexList.push_back(vertexList[it.vertex[0]].y);
        mVertexList.push_back(vertexList[it.vertex[0]].z);
        mVertexList.push_back(vertexList[it.vertex[1]].x);
        mVertexList.push_back(vertexList[it.vertex[1]].y);
        mVertexList.push_back(vertexList[it.vertex[1]].z);
        mVertexList.push_back(vertexList[it.vertex[2]].x);
        mVertexList.push_back(vertexList[it.vertex[2]].y);
        mVertexList.push_back(vertexList[it.vertex[2]].z);
        if (it.normal[0] != SIZE_MAX)
        {
            mNormalList.push_back(normalList[it.normal[0]].x);
            mNormalList.push_back(normalList[it.normal[0]].y);
            mNormalList.push_back(normalList[it.normal[0]].z);
            mNormalList.push_back(normalList[it.normal[1]].x);
            mNormalList.push_back(normalList[it.normal[1]].y);
            mNormalList.push_back(normalList[it.normal[1]].z);
            mNormalList.push_back(normalList[it.normal[2]].x);
            mNormalList.push_back(normalList[it.normal[2]].y);
            mNormalList.push_back(normalList[it.normal[2]].z);
        }
        else
        {
            ComputeFaceNormal(vertexList[it.vertex[0]].data(), vertexList[it.vertex[1]].data(),
                              vertexList[it.vertex[2]].data(), normal.data());
            mNormalList.push_back(normal.x);
            mNormalList.push_back(normal.y);
            mNormalList.push_back(normal.z);
            mNormalList.push_back(normal.x);
            mNormalList.push_back(normal.y);
            mNormalList.push_back(normal.z);
            mNormalList.push_back(normal.x);
            mNormalList.push_back(normal.y);
            mNormalList.push_back(normal.z);
        }
        for (size_t i = 0; i < 9; i++) mColourList.push_back(colour[i % 3]);
    }

    m_meshStore.setTargetMemory(0.5);
    m_meshStore.addMesh(filename, mVertexList, mNormalList, mColourList, m_lowerBound, m_upperBound);

    return 0;
}

int FacetedObject::ParsePLYFile(const std::string &filename)
{
    MeshStoreObject *meshStoreObject = m_meshStore.getMesh(filename);
    if (meshStoreObject)
    {
        mVertexList = meshStoreObject->vertexList;
        mNormalList = meshStoreObject->normalList;
        mColourList = meshStoreObject->colourList;
        m_lowerBound[0] = meshStoreObject->lowerBound[0];
        m_lowerBound[1] = meshStoreObject->lowerBound[1];
        m_lowerBound[2] = meshStoreObject->lowerBound[2];
        m_upperBound[0] = meshStoreObject->upperBound[0];
        m_upperBound[1] = meshStoreObject->upperBound[1];
        m_upperBound[2] = meshStoreObject->upperBound[2];
        return 0;
    }
    try
    {
#if (defined(_WIN32) || defined(WIN32)) && !defined(__MINGW32__)
        std::ifstream ss(DataFile::ConvertUTF8ToWide(filename), std::ios::binary);
#else
        std::ifstream ss(filename, std::ios::binary);
#endif
        if (ss.fail()) throw std::runtime_error("failed to open " + filename);

        tinyply::PlyFile file;
        file.parse_header(ss);

        for (auto c : file.get_comments()) std::cout << "Comment: " << c << std::endl;
        std::vector<tinyply::PlyElement> elementVector = file.get_elements();
        std::map<std::string, tinyply::PlyProperty *> vertexProperties;
        std::map<std::string, tinyply::PlyProperty *> faceProperties;
        for (auto e : elementVector)
        {
            std::cout << "element - " << e.name << " (" << e.size << ")" << std::endl;
            if (e.name == "vertex") for (auto p : e.properties) vertexProperties[p.name] = &p;
            else if (e.name == "face") for (auto p : e.properties) faceProperties[p.name] = &p;
            for (auto p : e.properties)
            {
                if (p.isList) std::cout << "\tproperty - " << p.name << " (" <<
                                            tinyply::PropertyTable[p.listType].str << ")"
                                            << " (" << tinyply::PropertyTable[p.propertyType].str << ")" << std::endl;
                else
                    std::cout << "\tproperty - " << p.name << " (" << tinyply::PropertyTable[p.propertyType].str << ")"
                              << std::endl;
            }
        }

        // Tinyply treats parsed data as untyped byte buffers. See below for examples.
        std::shared_ptr<tinyply::PlyData> vertices, normals, faces, texcoords, colours;

        // The header information can be used to programmatically extract properties on elements
        // known to exist in the header prior to reading the data. For brevity of this sample, properties
        // like vertex position are hard-coded:
        if (vertexProperties.find("x") != vertexProperties.end())
        {
            try
            {
                vertices = file.request_properties_from_element("vertex", { "x", "y", "z" });
            }
            catch (const std::exception &e)
            {
                std::cerr << "tinyply exception: " << e.what() << std::endl;
            }
        }

        if (vertexProperties.find("nx") != vertexProperties.end())
        {
            try
            {
                normals = file.request_properties_from_element("vertex", { "nx", "ny", "nz" });
            }
            catch (const std::exception &e)
            {
                std::cerr << "tinyply exception: " << e.what() << std::endl;
            }
        }

        if (vertexProperties.find("u") != vertexProperties.end())
        {
            try
            {
                texcoords = file.request_properties_from_element("vertex", { "u", "v" });
            }
            catch (const std::exception &e)
            {
                std::cerr << "tinyply exception: " << e.what() << std::endl;
            }
        }

        if (vertexProperties.find("red") != vertexProperties.end())
        {
            try
            {
                colours = file.request_properties_from_element("vertex", { "red", "green", "blue", "alpha" });
            }
            catch (const std::exception &e)
            {
                std::cerr << "tinyply exception: " << e.what() << std::endl;
            }
        }

        // Providing a list size hint (the last argument) is a 2x performance improvement. If you have
        // arbitrary ply files, it is best to leave this 0.
        // note: tinyply does not cope with varying numbers of vertices in each face and GaitSym wants triangles
        if (faceProperties.find("vertex_indices") != faceProperties.end())
        {
            try
            {
                faces = file.request_properties_from_element("face", { "vertex_indices" }, 3);
            }
            catch (const std::exception &e)
            {
                std::cerr << "tinyply exception: " << e.what() << std::endl;
            }
        }

        file.read(ss);

        if (vertices) std::cout << "\tRead " << vertices->count << " total vertices " << std::endl;
        if (normals) std::cout << "\tRead " << normals->count << " total vertex normals " << std::endl;
        if (texcoords) std::cout << "\tRead " << texcoords->count << " total vertex texcoords " <<
                                     std::endl;
        if (colours) std::cout << "\tRead " << colours->count << " total vertex colours " << std::endl;
        if (faces) std::cout << "\tRead " << faces->count << " total faces " << std::endl;

        AllocateMemory(faces->count);
        if (vertices->t == tinyply::Type::FLOAT32)
        {
            int32_t *vertexIndexPtr = reinterpret_cast<int32_t *>(faces->buffer.get());
            float *vertexPtr = reinterpret_cast<float *>(vertices->buffer.get());
            double triangle[9];
            for (size_t i = 0; i < faces->count; i++)
            {
                for (size_t j = 0; j < 3;
                        j++) (&triangle[0])[j] = double((&vertexPtr[3 * vertexIndexPtr[i * 3 + 0]])[j]);
                for (size_t j = 0; j < 3;
                        j++) (&triangle[3])[j] = double((&vertexPtr[3 * vertexIndexPtr[i * 3 + 1]])[j]);
                for (size_t j = 0; j < 3;
                        j++) (&triangle[6])[j] = double((&vertexPtr[3 * vertexIndexPtr[i * 3 + 2]])[j]);
                AddTriangle(triangle);
//                qDebug("Triangle %d (%d,%d,%d)", i, vertexIndexPtr[i * 3 + 0], vertexIndexPtr[i * 3 + 1], vertexIndexPtr[i * 3 + 2]);

            }
        }
        if (vertices->t == tinyply::Type::FLOAT64)
        {
            int32_t *vertexIndexPtr = reinterpret_cast<int32_t *>(faces->buffer.get());
            double *vertexPtr = reinterpret_cast<double *>(vertices->buffer.get());
            double triangle[9];
            for (size_t i = 0; i < faces->count; i++)
            {
                std::copy_n(&triangle[0], 3, &vertexPtr[3 * vertexIndexPtr[i * 3 + 0]]);
                std::copy_n(&triangle[3], 3, &vertexPtr[3 * vertexIndexPtr[i * 3 + 1]]);
                std::copy_n(&triangle[6], 3, &vertexPtr[3 * vertexIndexPtr[i * 3 + 2]]);
                AddTriangle(triangle);
            }
        }

        m_meshStore.setTargetMemory(0.5);
        m_meshStore.addMesh(filename, mVertexList, mNormalList, mColourList, m_lowerBound, m_upperBound);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Caught tinyply exception: " << e.what() << std::endl;
        return __LINE__;
    }
    return 0;
}

int FacetedObject::ReadFromMemory(const char *data, size_t len, bool binary, const std::string &meshName)
{
    MeshStoreObject *meshStoreObject = m_meshStore.getMesh(meshName);
    if (meshStoreObject)
    {
        mVertexList = meshStoreObject->vertexList;
        mNormalList = meshStoreObject->normalList;
        mColourList = meshStoreObject->colourList;
        m_lowerBound[0] = meshStoreObject->lowerBound[0];
        m_lowerBound[1] = meshStoreObject->lowerBound[1];
        m_lowerBound[2] = meshStoreObject->lowerBound[2];
        m_upperBound[0] = meshStoreObject->upperBound[0];
        m_upperBound[1] = meshStoreObject->upperBound[1];
        m_upperBound[2] = meshStoreObject->upperBound[2];
        return 0;
    }

    mVertexList.clear();
    mNormalList.clear();
    mColourList.clear();
    if (binary)
    {
        if (len < sizeof(size_t) + 6 * sizeof(double)) return __LINE__;
        const char *ptr = data;
        size_t numVertices = *reinterpret_cast<size_t *>(*ptr);
        ptr += sizeof(size_t);
        if (len < sizeof(size_t) + 6 * sizeof(double) + numVertices * 9 * sizeof(double)) return __LINE__;
        m_lowerBound[0] = *reinterpret_cast<double *>(*ptr);
        ptr += sizeof(double);
        m_lowerBound[1] = *reinterpret_cast<double *>(*ptr);
        ptr += sizeof(double);
        m_lowerBound[2] = *reinterpret_cast<double *>(*ptr);
        ptr += sizeof(double);
        m_upperBound[0] = *reinterpret_cast<double *>(*ptr);
        ptr += sizeof(double);
        m_upperBound[1] = *reinterpret_cast<double *>(*ptr);
        ptr += sizeof(double);
        m_upperBound[2] = *reinterpret_cast<double *>(*ptr);
        ptr += sizeof(double);
        size_t numElements = numVertices * 3;
        mVertexList.resize(numElements);
        mNormalList.resize(numElements);
        mColourList.resize(numElements);
        for (size_t i = 0; i < numVertices; i++)
        {
            mVertexList.push_back(*reinterpret_cast<double *>(*ptr));
            ptr += sizeof(double);
        }
        for (size_t i = 0; i < numVertices; i++)
        {
            mNormalList.push_back(*reinterpret_cast<double *>(*ptr));
            ptr += sizeof(double);
        }
        for (size_t i = 0; i < numVertices; i++)
        {
            mColourList.push_back(*reinterpret_cast<double *>(*ptr));
            ptr += sizeof(double);
        }
        return 0;
    }
    else
    {
        if (data[len]) return __LINE__; // must be null terminated for ASCII case
        char *endPtr;
        size_t numVertices = std::strtoull(data, &endPtr, 0);
        m_lowerBound[0] = std::strtod(endPtr, &endPtr);
        m_lowerBound[1] = std::strtod(endPtr, &endPtr);
        m_lowerBound[2] = std::strtod(endPtr, &endPtr);
        m_upperBound[0] = std::strtod(endPtr, &endPtr);
        m_upperBound[1] = std::strtod(endPtr, &endPtr);
        m_upperBound[2] = std::strtod(endPtr, &endPtr);
        size_t numElements = numVertices * 3;
        mVertexList.resize(numElements);
        mNormalList.resize(numElements);
        mColourList.resize(numElements);
        for (size_t i = 0; i < numVertices; i++) mVertexList.push_back(std::strtod(endPtr, &endPtr));
        for (size_t i = 0; i < numVertices; i++) mNormalList.push_back(std::strtod(endPtr, &endPtr));
        for (size_t i = 0; i < numVertices; i++) mColourList.push_back(std::strtod(endPtr, &endPtr));
        return 0;
    }
}

void FacetedObject::SaveToMemory(std::vector<char> *data, bool binary)
{
    if (binary)
    {
        data->clear();
        data->resize(sizeof(size_t) + 6 * sizeof(double) + mVertexList.size() * 3 * sizeof(double));
        size_t *ptr = reinterpret_cast<size_t *>(data->data());
        *ptr++ = mVertexList.size() / 3;
        double *dPtr = reinterpret_cast<double *>(ptr);
        *dPtr++ = m_lowerBound[0];
        *dPtr++ = m_lowerBound[1];
        *dPtr++ = m_lowerBound[2];
        *dPtr++ = m_upperBound[0];
        *dPtr++ = m_upperBound[1];
        *dPtr++ = m_upperBound[2];
        for (size_t i = 0; i < mVertexList.size(); i++) *dPtr++ = mVertexList[i];
        for (size_t i = 0; i < mNormalList.size(); i++) *dPtr++ = mNormalList[i];
        for (size_t i = 0; i < mColourList.size(); i++) *dPtr++ = mColourList[i];
    }
    else
    {
        data->clear();
        char buf[32];
        int l = std::sprintf(buf, "%zud\n", mVertexList.size() / 3);
        std::copy_n(buf, l, std::back_inserter(*data));
        l = std::sprintf(buf, "%.18g\n", m_lowerBound[0]);
        std::copy_n(buf, l, std::back_inserter(*data));
        l = std::sprintf(buf, "%.18g\n", m_lowerBound[1]);
        std::copy_n(buf, l, std::back_inserter(*data));
        l = std::sprintf(buf, "%.18g\n", m_lowerBound[2]);
        std::copy_n(buf, l, std::back_inserter(*data));
        l = std::sprintf(buf, "%.18g\n", m_upperBound[0]);
        std::copy_n(buf, l, std::back_inserter(*data));
        l = std::sprintf(buf, "%.18g\n", m_upperBound[1]);
        std::copy_n(buf, l, std::back_inserter(*data));
        l = std::sprintf(buf, "%.18g\n", m_upperBound[2]);
        std::copy_n(buf, l, std::back_inserter(*data));
        for (size_t i = 0; i < mVertexList.size(); i++)
        {
            l = std::sprintf(buf, "%.18g\n", mVertexList[i]);
            std::copy_n(buf, l, std::back_inserter(*data));
        }
        for (size_t i = 0; i < mNormalList.size(); i++)
        {
            l = std::sprintf(buf, "%.18g\n", mNormalList[i]);
            std::copy_n(buf, l, std::back_inserter(*data));
        }
        for (size_t i = 0; i < mColourList.size(); i++)
        {
            l = std::sprintf(buf, "%.18g\n", mColourList[i]);
            std::copy_n(buf, l, std::back_inserter(*data));
        }
        data->push_back('\0');
    }
}

void FacetedObject::InitialiseEntity()
{
    // Material
    Qt3DRender::QMaterial *material;
    if (m_effect)
    {
        material = new Qt3DRender::QMaterial(this);
        material->setEffect(m_effect);
        material->addParameter(new Qt3DRender::QParameter(QStringLiteral("meshColor"), QColor(Qt::blue)));
    }
    else
    {
        material = new Qt3DExtras::QPerVertexColorMaterial(this);
    }

    // Transform
    m_transform = new Qt3DCore::QTransform(this);

    // Custom Mesh
    Qt3DRender::QGeometryRenderer *customMeshRenderer = new Qt3DRender::QGeometryRenderer(this);
    Qt3DRender::QGeometry *customGeometry = new Qt3DRender::QGeometry(customMeshRenderer);

    // the vertex data is interleaved
    // vec3 for position
    // vec3 for normals
    // vec3 for colors
    size_t numVertices = mVertexList.size() / 3;
    Q_ASSERT_X(numVertices == mNormalList.size() / 3, "FacetedObject::InitialiseEntity()",
               "numVertices != mNormalList.size() / 3");
    QByteArray vertexData;
    vertexData.resize(int(numVertices * (3 + 3 + 3) * sizeof(float)));
    float *vertexDataPtr = reinterpret_cast<float *>(vertexData.data());
    for (size_t i = 0; i < numVertices; i++)
    {
        vertexDataPtr[i * (3 + 3 + 3) + 0] = float(mVertexList[i * 3 + 0]);
        vertexDataPtr[i * (3 + 3 + 3) + 1] = float(mVertexList[i * 3 + 1]);
        vertexDataPtr[i * (3 + 3 + 3) + 2] = float(mVertexList[i * 3 + 2]);
        vertexDataPtr[i * (3 + 3 + 3) + 3] = float(mNormalList[i * 3 + 0]);
        vertexDataPtr[i * (3 + 3 + 3) + 4] = float(mNormalList[i * 3 + 1]);
        vertexDataPtr[i * (3 + 3 + 3) + 5] = float(mNormalList[i * 3 + 2]);
        vertexDataPtr[i * (3 + 3 + 3) + 6] = float(mColourList[i * 3 + 0]);
        vertexDataPtr[i * (3 + 3 + 3) + 7] = float(mColourList[i * 3 + 1]);
        vertexDataPtr[i * (3 + 3 + 3) + 8] = float(mColourList[i * 3 + 2]);
//        qDebug("Vertex %d (%f,%f,%f) (%f,%f,%f) (%f,%f,%f)", i,
//               double(vertexDataPtr[i * (3 + 3 + 3) + 0]),
//                double(vertexDataPtr[i * (3 + 3 + 3) + 1]),
//                double(vertexDataPtr[i * (3 + 3 + 3) + 2]),
//                double(vertexDataPtr[i * (3 + 3 + 3) + 3]),
//                double(vertexDataPtr[i * (3 + 3 + 3) + 4]),
//                double(vertexDataPtr[i * (3 + 3 + 3) + 5]),
//                double(vertexDataPtr[i * (3 + 3 + 3) + 6]),
//                double(vertexDataPtr[i * (3 + 3 + 3) + 7]),
//                double(vertexDataPtr[i * (3 + 3 + 3) + 8]));
    }

    // the index data is just a list of uint32_t values
    // and is the same length as the number of vertices because vertices are not shared
    QByteArray vertexIndexData;
    vertexIndexData.resize(int(numVertices * sizeof(uint32_t)));
    uint32_t *vertexIndexDataPtr = reinterpret_cast<uint32_t *>(vertexIndexData.data());
    for (size_t i = 0; i < numVertices; i++) vertexIndexDataPtr[i] = uint32_t(i);

    Qt3DRender::QBuffer *vertexDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::VertexBuffer,
            customGeometry);
    Qt3DRender::QBuffer *indexDataBuffer = new Qt3DRender::QBuffer(Qt3DRender::QBuffer::IndexBuffer,
            customGeometry);
    vertexDataBuffer->setUsage(Qt3DRender::QBuffer::StaticDraw);
    indexDataBuffer->setUsage(Qt3DRender::QBuffer::StaticDraw);
    vertexDataBuffer->setData(vertexData);
    indexDataBuffer->setData(vertexIndexData);

    // Attributes
    Qt3DRender::QAttribute *positionAttribute = new Qt3DRender::QAttribute(this);
    positionAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    positionAttribute->setBuffer(vertexDataBuffer);
    positionAttribute->setDataType(Qt3DRender::QAttribute::Float);
    positionAttribute->setDataSize(3);
    positionAttribute->setByteOffset(0);
    positionAttribute->setByteStride(9 * sizeof(float));
    positionAttribute->setCount(uint(numVertices));
    positionAttribute->setName(Qt3DRender::QAttribute::defaultPositionAttributeName());

    Qt3DRender::QAttribute *normalAttribute = new Qt3DRender::QAttribute(this);
    normalAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    normalAttribute->setBuffer(vertexDataBuffer);
    normalAttribute->setDataType(Qt3DRender::QAttribute::Float);
    normalAttribute->setDataSize(3);
    normalAttribute->setByteOffset(3 * sizeof(float));
    normalAttribute->setByteStride(9 * sizeof(float));
    normalAttribute->setCount(uint(numVertices));
    normalAttribute->setName(Qt3DRender::QAttribute::defaultNormalAttributeName());

    Qt3DRender::QAttribute *colorAttribute = new Qt3DRender::QAttribute(this);
    colorAttribute->setAttributeType(Qt3DRender::QAttribute::VertexAttribute);
    colorAttribute->setBuffer(vertexDataBuffer);
    colorAttribute->setDataType(Qt3DRender::QAttribute::Float);
    colorAttribute->setDataSize(3);
    colorAttribute->setByteOffset(6 * sizeof(float));
    colorAttribute->setByteStride(9 * sizeof(float));
    colorAttribute->setCount(uint(numVertices));
    colorAttribute->setName(Qt3DRender::QAttribute::defaultColorAttributeName());

    Qt3DRender::QAttribute *indexAttribute = new Qt3DRender::QAttribute(this);
    indexAttribute->setAttributeType(Qt3DRender::QAttribute::IndexAttribute);
    indexAttribute->setBuffer(indexDataBuffer);
    indexAttribute->setDataType(Qt3DRender::QAttribute::UnsignedInt);
    indexAttribute->setDataSize(1);
    indexAttribute->setByteOffset(0);
    indexAttribute->setByteStride(sizeof(uint32_t));
    indexAttribute->setCount(uint(numVertices));

    customGeometry->addAttribute(positionAttribute);
    customGeometry->addAttribute(normalAttribute);
    customGeometry->addAttribute(colorAttribute);
    customGeometry->addAttribute(indexAttribute);

    customMeshRenderer->setInstanceCount(1);
    customMeshRenderer->setIndexOffset(0);
    customMeshRenderer->setFirstInstance(0);
    customMeshRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);
    customMeshRenderer->setGeometry(customGeometry);
    customMeshRenderer->setVertexCount(int(numVertices));

    this->addComponent(customMeshRenderer);
    this->addComponent(m_transform);
    this->addComponent(material);
    if (m_layer) this->addComponent(m_layer);


//    std::cerr << numVertices << " initialised\n";
}

// Write a FacetedObject out as a POVRay file
void FacetedObject::WritePOVRay(std::string filename)
{
    std::ostringstream objData;
    WritePOVRay(objData);
#if (defined(_WIN32) || defined(WIN32)) && !defined(__MINGW32__)
    std::ofstream f(DataFile::ConvertUTF8ToWide(filename));
#else
    std::ofstream f(filename);
#endif
    f << objData.str();
    f.close();
}

// write the object out as a POVRay string
// currently assumes all faces are triangles (call Triangulate if conversion is necessary)
void FacetedObject::WritePOVRay(std::ostringstream &theString)
{
    size_t i, j;
    dVector3 prel, p, result;

    theString.precision(7); // should be plenty

    theString << "object {\n";
    theString << "  mesh {\n";

    // first faces
    for (i = 0; i < mVertexList.size() / 9; i++)
    {
        theString << "    triangle {\n";
        for (j = 0; j < 3; j++)
        {
            prel[0] = mVertexList[i * 9 + j * 3];
            prel[1] = mVertexList[i * 9 + j * 3 + 1];
            prel[2] = mVertexList[i * 9 + j * 3 + 2];
            prel[3] = 0;
            dMULTIPLY0_331(p, m_DisplayRotation, prel);
            result[0] = p[0] + m_DisplayPosition[0];
            result[1] = p[1] + m_DisplayPosition[1];
            result[2] = p[2] + m_DisplayPosition[2];

            theString << "      <" << result[0] << "," << result[1] << "," << result[2] << ">\n";
        }
        theString << "    }\n";
    }

    // now colour
    theString << "    pigment {\n";
    theString << "      color rgbf<" << m_colour.redF() << "," << m_colour.greenF() << "," <<
              m_colour.blueF() << "," << 1 - m_colour.alphaF() << ">\n";
    theString << "    }\n";
    theString << "  }\n";
    theString << "}\n\n";
}

// Write a FacetedObject out as an OBJ file
void FacetedObject::WriteOBJFile(std::string filename)
{
    std::ostringstream objData;
    WriteOBJFile(objData);
#if (defined(_WIN32) || defined(WIN32)) && !defined(__MINGW32__)
    std::ofstream f(DataFile::ConvertUTF8ToWide(filename));
#else
    std::ofstream f(filename);
#endif
    f << objData.str();
    f.close();
}


SceneEffect *FacetedObject::effect() const
{
    return m_effect;
}

void FacetedObject::setEffect(SceneEffect *effect)
{
    m_effect = effect;
}

Qt3DRender::QLayer *FacetedObject::layer() const
{
    return m_layer;
}

void FacetedObject::setLayer(Qt3DRender::QLayer *layer)
{
    m_layer = layer;
}

double *FacetedObject::upperBound()
{
    return m_upperBound;
}

double *FacetedObject::lowerBound()
{
    return m_lowerBound;
}

// Write a FacetedObject out as a OBJ
void FacetedObject::WriteOBJFile(std::ostringstream &out)
{
    size_t i, j;
    dVector3 prel, p, result;
    static unsigned long counter = 0;

    out.precision(7); // should be plenty

    for (i = 0; i < m_OBJName.size(); i++)
        if (m_OBJName[i] <= ' ') m_OBJName[i] = '_';
    out << "o " << m_OBJName << counter << "\n";
    counter++;

    if (m_UseRelativeOBJ)
    {
        // write out the vertices, faces, groups and objects
        // this is the relative version - inefficient but allows concatenation of objects
        for (i = 0; i < mVertexList.size() / 9; i++)
        {
            for (j = 0; j < 3; j++)
            {
                prel[0] = mVertexList[i * 9 + j * 3];
                prel[1] = mVertexList[i * 9 + j * 3 + 1];
                prel[2] = mVertexList[i * 9 + j * 3 + 2];
                prel[3] = 0;
                dMULTIPLY0_331(p, m_DisplayRotation, prel);
                result[0] = p[0] + m_DisplayPosition[0];
                result[1] = p[1] + m_DisplayPosition[1];
                result[2] = p[2] + m_DisplayPosition[2];
                out << "v " << result[0] << " " << result[1] << " " << result[2] << "\n";
            }

            out << "f ";
            for (j = 0; j < 3; j++)
            {
                if (j == 3)
                    out << j - 3 << "\n";
                else
                    out << j - 3 << " ";
            }
        }
    }
    else
    {
        for (i = 0; i < mVertexList.size() / 9; i++)
        {
            for (j = 0; j < 3; j++)
            {
                prel[0] = mVertexList[i * 9 + j * 3];
                prel[1] = mVertexList[i * 9 + j * 3 + 1];
                prel[2] = mVertexList[i * 9 + j * 3 + 2];
                prel[3] = 0;
                dMULTIPLY0_331(p, m_DisplayRotation, prel);
                result[0] = p[0] + m_DisplayPosition[0];
                result[1] = p[1] + m_DisplayPosition[1];
                result[2] = p[2] + m_DisplayPosition[2];
                out << "v " << result[0] << " " << result[1] << " " << result[2] << "\n";
            }
        }

        for (i = 0; i < mVertexList.size() / 9; i++)
        {
            out << "f ";
            for (j = 0; j < 3; j++)
            {
                // note this files vertex list start at 1 not zero
                if (j == 2)
                    out << i * 3 + j + 1 + m_vertexOffset << "\n";
                else
                    out << i * 3 + j + 1 + m_vertexOffset << " ";
            }
        }
        m_vertexOffset += mVertexList.size() / 3;
    }
}

void FacetedObject::SetDisplayPosition(double x, double y, double z)
{
    m_DisplayPosition[0] = x;
    m_DisplayPosition[1] = y;
    m_DisplayPosition[2] = z;
    m_transform->setTranslation(QVector3D(float(x), float(y), float(z)));
}

void FacetedObject::SetDisplayRotation(const dMatrix3 R)
{
    std::copy_n(R, dM3E__MAX, m_DisplayRotation);
    dQuaternion q;
    dRtoQ (R, q);
    m_transform->setRotation(QQuaternion(float(q[0]), float(q[1]), float(q[2]), float(q[3])));
}

// dQuaternion q [ w, x, y, z ], where w is the real part and (x, y, z) form the vector part.
void FacetedObject::SetDisplayRotationFromQuaternion(const dQuaternion q)
{
    dQtoR(q, m_DisplayRotation);
    m_transform->setRotation(QQuaternion(float(q[0]), float(q[1]), float(q[2]), float(q[3])));
}

// this routine rotates the Z axis to point in a specified direction
void FacetedObject::SetDisplayRotationFromAxis(double x, double y, double z)
{
    // calculate the rotation needed to get the axis pointing the right way
    dVector3 axis;
    axis[0] = x;
    axis[1] = y;
    axis[2] = z;
    dVector3 p, q;
    // calculate 2 perpendicular vectors
    dPlaneSpace(axis, p, q);
    // assemble the matrix
    m_DisplayRotation[3] = m_DisplayRotation[7] = m_DisplayRotation[11] = 0;

    m_DisplayRotation[0] =    p[0];
    m_DisplayRotation[4] =    p[1];
    m_DisplayRotation[8] =     p[2];
    m_DisplayRotation[1] =    q[0];
    m_DisplayRotation[5] =    q[1];
    m_DisplayRotation[9] =     q[2];
    m_DisplayRotation[2] = axis[0];
    m_DisplayRotation[6] = axis[1];
    m_DisplayRotation[10] = axis[2];

    dQuaternion qu;
    dRtoQ (m_DisplayRotation, qu);
    m_transform->setRotation(QQuaternion(float(qu[0]), float(qu[1]), float(qu[2]), float(qu[3])));
}

// utility to calculate a face normal
// this assumes anticlockwise winding
void FacetedObject::ComputeFaceNormal(const double *v1, const double *v2, const double *v3,
                                      double normal[3])
{
    double a[3], b[3];

    // calculate in plane vectors
    a[0] = v2[0] - v1[0];
    a[1] = v2[1] - v1[1];
    a[2] = v2[2] - v1[2];
    b[0] = v3[0] - v1[0];
    b[1] = v3[1] - v1[1];
    b[2] = v3[2] - v1[2];

    // cross(a, b, normal);
    normal[0] = a[1] * b[2] - a[2] * b[1];
    normal[1] = a[2] * b[0] - a[0] * b[2];
    normal[2] = a[0] * b[1] - a[1] * b[0];

    // normalize(normal);
    double norm = sqrt(normal[0] * normal[0] +
                       normal[1] * normal[1] +
                       normal[2] * normal[2]);

    if (norm > 0.0)
    {
        normal[0] /= norm;
        normal[1] /= norm;
        normal[2] /= norm;
    }
}

// move the object
// note this must be used before first draw call
void FacetedObject::Move(double x, double y, double z)
{
    if (x == 0.0 && y == 0.0 && z == 0.0) return;
    for (size_t i = 0; i < mVertexList.size() / 3; i++)
    {
        mVertexList[i * 3] += x;
        mVertexList[i * 3 + 1] += y;
        mVertexList[i * 3 + 2] += z;
    }
}

// scale the object
// note this must be used before first draw call
void FacetedObject::Scale(double x, double y, double z)
{
    if (x == 1.0 && y == 1.0 && z == 1.0) return;
    for (size_t i = 0; i < mVertexList.size() / 3; i++)
    {
        mVertexList[i * 3] *= x;
        mVertexList[i * 3 + 1] *= y;
        mVertexList[i * 3 + 2] *= z;
    }
}

// this routine triangulates the polygon and calls AddTriangle to do the actual data adding
// vertices are a packed list of floating point numbers
// x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4
void FacetedObject::AddPolygon(const double *vertices, size_t nSides, const double *normals)
{
    if (normals == nullptr)
    {
        // calculate the normals
        double triNormal[9];
        ComputeFaceNormal(vertices, vertices + 3, vertices + 6, triNormal);
        for (size_t i = 3; i < 9; i++) triNormal[i] = triNormal[i % 3];
        // add faces as triangles
        double triangle[9];
        triangle[0] = vertices[0];
        triangle[1] = vertices[1];
        triangle[2] = vertices[2];
        for (size_t j = 2; j < nSides; j++)
        {
            triangle[3] = vertices[(j - 1) * 3];
            triangle[4] = vertices[(j - 1) * 3 + 1];
            triangle[5] = vertices[(j - 1) * 3 + 2];
            triangle[6] = vertices[(j * 3)];
            triangle[7] = vertices[(j * 3) + 1];
            triangle[8] = vertices[(j * 3) + 2];
            AddTriangle(triangle, triNormal);
        }
    }
    else
    {
        // add faces as triangles
        double triangle[9];
        double triNormal[9];
        triangle[0] = vertices[0];
        triangle[1] = vertices[1];
        triangle[2] = vertices[2];
        triNormal[0] = normals[0];
        triNormal[1] = normals[1];
        triNormal[2] = normals[2];
        for (size_t j = 2; j < nSides; j++)
        {
            triangle[3] = vertices[(j - 1) * 3];
            triangle[4] = vertices[(j - 1) * 3 + 1];
            triangle[5] = vertices[(j - 1) * 3 + 2];
            triangle[6] = vertices[(j * 3)];
            triangle[7] = vertices[(j * 3) + 1];
            triangle[8] = vertices[(j * 3) + 2];
            triNormal[3] = normals[(j - 1) * 3];
            triNormal[4] = normals[(j - 1) * 3 + 1];
            triNormal[5] = normals[(j - 1) * 3 + 2];
            triNormal[6] = normals[(j * 3)];
            triNormal[7] = normals[(j * 3) + 1];
            triNormal[8] = normals[(j * 3) + 2];
            AddTriangle(triangle, triNormal);
        }

    }
}


// this is the only routine that actually adds data to the faceted object
// it gets called by add polygon
// vertices is a packed list of floating point numbers
// x1, y1, z1, x2, y2, z2, x3, y3, z3
void FacetedObject::AddTriangle(const double *vertices, const double *normals)
{
    Q_ASSERT_X(mVertexList.capacity() - mVertexList.size() >= 9, "FacetedObject::AddTriangle",
               "Warning: not enough triangle space reserved");
    pgd::Vector vertex;
    for (size_t i = 0; i < 3; i++)
    {
        vertex.x = vertices[i * 3 + 0];
        vertex.y = vertices[i * 3 + 1];
        vertex.z = vertices[i * 3 + 2];
        mVertexList.push_back(vertex.x);
        mVertexList.push_back(vertex.y);
        mVertexList.push_back(vertex.z);
        if (vertex.x < m_lowerBound[0]) m_lowerBound[0] = vertex.x;
        if (vertex.y < m_lowerBound[1]) m_lowerBound[1] = vertex.y;
        if (vertex.z < m_lowerBound[2]) m_lowerBound[2] = vertex.z;
        if (vertex.x > m_upperBound[0]) m_upperBound[0] = vertex.x;
        if (vertex.y > m_upperBound[1]) m_upperBound[1] = vertex.y;
        if (vertex.z > m_upperBound[2]) m_upperBound[2] = vertex.z;
    }
    if (normals)
    {
        for (size_t i = 0; i < 9; i++) mNormalList.push_back(normals[i]);
    }
    else
    {
        // calculate the normals
        double normal[3];
        ComputeFaceNormal(vertices, vertices + 3, vertices + 6, normal);
        for (size_t i = 0; i < 9; i++) mNormalList.push_back(normal[i % 3]);
    }
    double colour[3] = {m_colour.redF(), m_colour.greenF(), m_colour.blueF() };
    for (size_t i = 0; i < 9; i++) mColourList.push_back(colour[i % 3]);
}

// this routine triangulates the polygon and calls AddTriangle to do the actual data adding
// vertices are a packed list of floating point numbers
// x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4
void FacetedObject::AddPolygon(const float *floatVertices, size_t nSides, const float *floatNormals)
{
    std::vector<double> doubleVertices;
    doubleVertices.reserve(nSides);
    for (size_t i = 0; i < nSides; i++) doubleVertices.data()[i] = double(floatVertices[i]);
    if (floatNormals)
    {
        std::vector<double> normals;
        normals.reserve(nSides);
        for (size_t i = 0; i < nSides; i++) normals.data()[i] = double(floatNormals[i]);
        AddPolygon(doubleVertices.data(), nSides, normals.data());
    }
    else
    {
        AddPolygon(doubleVertices.data(), nSides, nullptr);
    }


//    // add faces as triangles
//    float triangle[9];
//    triangle[0] = vertices[0];
//    triangle[1] = vertices[1];
//    triangle[2] = vertices[2];
//    for (size_t j = 2; j < nSides; j++)
//    {
//        triangle[3] = vertices[(j - 1) * 3];
//        triangle[4] = vertices[(j - 1) * 3 + 1];
//        triangle[5] = vertices[(j - 1) * 3 + 2];
//        triangle[6] = vertices[(j * 3)];
//        triangle[7] = vertices[(j * 3) + 1];
//        triangle[8] = vertices[(j * 3) + 2];
//        AddTriangle(triangle);
//    }
}


// this is the only routine that actually adds data to the facetted object
// it gets called by add polygon
// vertices is a packed list of floating point numbers
// x1, y1, z1, x2, y2, z2, x3, y3, z3
void FacetedObject::AddTriangle(const float *floatVertices, const float *floatNormals)
{
    double vertices[9];
    for (int i = 0; i < 9; i++) vertices[i] = double(floatVertices[i]);
    if (floatNormals)
    {
        double normals[9];
        for (int i = 0; i < 9; i++) normals[i] = double(floatNormals[i]);
        AddTriangle(vertices, normals);
    }
    else
    {
        AddTriangle(vertices, nullptr);
    }
}

// this routine handles the memory allocation
// allocation is the number of vertices to store
void FacetedObject::AllocateMemory(size_t numTriangles)
{
//    qDebug() << "Allocated " << numTriangles << " triangles\n";
    mVertexList.reserve(numTriangles * 9);
    mNormalList.reserve(numTriangles * 9);
    mColourList.reserve(numTriangles * 9);
}

// return an ODE style trimesh
// note memory is allocated by this routine and will need to be released elsewhere
// warning - this routine will not cope with very big meshes because it uses ints
void FacetedObject::CalculateTrimesh(double **vertices, int *numVertices, int *vertexStride,
                                     dTriIndex **triIndexes, int *numTriIndexes, int *triStride)
{
    size_t i;
    *vertexStride = 3 * sizeof(double);
    *triStride = 3 * sizeof(dTriIndex);

    *numVertices = int((mVertexList.size() / 3));
    *numTriIndexes = int((mVertexList.size() / 3));

    *vertices = new double[(mVertexList.size() / 3) * 3];
    *triIndexes = new dTriIndex[(mVertexList.size() / 3)];

    for (i = 0; i < (mVertexList.size() / 3); i++)
    {
        (*vertices)[i * 3] = mVertexList[i * 3];
        (*vertices)[i * 3 + 1] = mVertexList[i * 3 + 1];
        (*vertices)[i * 3 + 2] = mVertexList[i * 3 + 2];
    }

    for (i = 0; i < (mVertexList.size() / 3); i++)
    {
        (*triIndexes)[i] = dTriIndex(i);
    }
}

// return an ODE style trimesh
// note memory is allocated by this routine and will need to be released elsewhere
// warning - this routine will not cope with very big meshes because it uses ints
void FacetedObject::CalculateTrimesh(float **vertices, int *numVertices, int *vertexStride,
                                     dTriIndex **triIndexes, int *numTriIndexes, int *triStride)
{
    size_t i;
    *vertexStride = 3 * sizeof(float);
    *triStride = 3 * sizeof(dTriIndex);

    *numVertices = int((mVertexList.size() / 3));
    *numTriIndexes = int((mVertexList.size() / 3));

    *vertices = new float[(mVertexList.size() / 3) * 3];
    *triIndexes = new dTriIndex[(mVertexList.size() / 3)];

    for (i = 0; i < (mVertexList.size() / 3); i++)
    {
        (*vertices)[i * 3] = float(mVertexList[i * 3]);
        (*vertices)[i * 3 + 1] = float(mVertexList[i * 3 + 1]);
        (*vertices)[i * 3 + 2] = float(mVertexList[i * 3 + 2]);
    }

    for (i = 0; i < (mVertexList.size() / 3); i++)
    {
        (*triIndexes)[i] = dTriIndex(i);
    }
}

// calculate mass properties
// based on dMassSetTrimesh
/*
 * dMassSetTrimesh, implementation by Gero Mueller.
 * Based on Brian Mirtich, "Fast and Accurate Computation of
 * Polyhedral Mass Properties," journal of graphics tools, volume 1,
 * number 2, 1996.
 */

#define SQR(x)          ((x)*(x))                       //!< Returns x square
#define CUBE(x)         ((x)*(x)*(x))                   //!< Returns x cube


void FacetedObject::CalculateMassProperties(dMass *m, double density, bool clockwise)
{
    dMassSetZero (m);

    // assumes anticlockwise winding

    unsigned int triangles = static_cast<unsigned int>((mVertexList.size() / 3) / 3);

    double nx, ny, nz;
    unsigned int i, j, A, B, C;
    // face integrals
    double Fa, Fb, Fc, Faa, Fbb, Fcc, Faaa, Fbbb, Fccc, Faab, Fbbc, Fcca;

    // projection integrals
    double P1, Pa, Pb, Paa, Pab, Pbb, Paaa, Paab, Pabb, Pbbb;

    double T0 = 0;
    double T1[3] = {0., 0., 0.};
    double T2[3] = {0., 0., 0.};
    double TP[3] = {0., 0., 0.};

    dVector3 v[3];
    for ( i = 0; i < triangles; i++ )
    {
        if (clockwise == false)
        {
            for (j = 0; j < 3; j++)
            {
                v[j][0] = mVertexList[i * 9 + j * 3];
                v[j][1] = mVertexList[i * 9 + j * 3 + 1];
                v[j][2] = mVertexList[i * 9 + j * 3 + 2];
            }
        }
        else
        {
            for (j = 0; j < 3; j++)
            {
                v[j][2] = mVertexList[i * 9 + j * 3];
                v[j][1] = mVertexList[i * 9 + j * 3 + 1];
                v[j][0] = mVertexList[i * 9 + j * 3 + 2];
            }
        }

        dVector3 n, a, b;
        dOP( a, -, v[1], v[0] );
        dOP( b, -, v[2], v[0] );
        dCROSS( n, =, b, a );
        nx = fabs(n[0]);
        ny = fabs(n[1]);
        nz = fabs(n[2]);

        if ( nx > ny && nx > nz )
            C = 0;
        else
            C = (ny > nz) ? 1 : 2;

        // Even though all triangles might be initially valid,
        // a triangle may degenerate into a segment after applying
        // space transformation.
        if (n[C] != REAL(0.0))
        {
            A = (C + 1) % 3;
            B = (A + 1) % 3;

            // calculate face integrals
            {
                double w;
                double k1, k2, k3, k4;

                //compProjectionIntegrals(f);
                {
                    double a0 = 0.0, a1 = 0.0, da;
                    double b0 = 0.0, b1 = 0.0, db;
                    double a0_2, a0_3, a0_4, b0_2, b0_3, b0_4;
                    double a1_2, a1_3, b1_2, b1_3;
                    double C1, Ca, Caa, Caaa, Cb, Cbb, Cbbb;
                    double Cab, Kab, Caab, Kaab, Cabb, Kabb;

                    P1 = Pa = Pb = Paa = Pab = Pbb = Paaa = Paab = Pabb = Pbbb = 0.0;

                    for ( j = 0; j < 3; j++)
                    {
                        switch (j)
                        {
                        case 0:
                            a0 = v[0][A];
                            b0 = v[0][B];
                            a1 = v[1][A];
                            b1 = v[1][B];
                            break;
                        case 1:
                            a0 = v[1][A];
                            b0 = v[1][B];
                            a1 = v[2][A];
                            b1 = v[2][B];
                            break;
                        case 2:
                            a0 = v[2][A];
                            b0 = v[2][B];
                            a1 = v[0][A];
                            b1 = v[0][B];
                            break;
                        }
                        da = a1 - a0;
                        db = b1 - b0;
                        a0_2 = a0 * a0;
                        a0_3 = a0_2 * a0;
                        a0_4 = a0_3 * a0;
                        b0_2 = b0 * b0;
                        b0_3 = b0_2 * b0;
                        b0_4 = b0_3 * b0;
                        a1_2 = a1 * a1;
                        a1_3 = a1_2 * a1;
                        b1_2 = b1 * b1;
                        b1_3 = b1_2 * b1;

                        C1 = a1 + a0;
                        Ca = a1 * C1 + a0_2;
                        Caa = a1 * Ca + a0_3;
                        Caaa = a1 * Caa + a0_4;
                        Cb = b1 * (b1 + b0) + b0_2;
                        Cbb = b1 * Cb + b0_3;
                        Cbbb = b1 * Cbb + b0_4;
                        Cab = 3 * a1_2 + 2 * a1 * a0 + a0_2;
                        Kab = a1_2 + 2 * a1 * a0 + 3 * a0_2;
                        Caab = a0 * Cab + 4 * a1_3;
                        Kaab = a1 * Kab + 4 * a0_3;
                        Cabb = 4 * b1_3 + 3 * b1_2 * b0 + 2 * b1 * b0_2 + b0_3;
                        Kabb = b1_3 + 2 * b1_2 * b0 + 3 * b1 * b0_2 + 4 * b0_3;

                        P1 += db * C1;
                        Pa += db * Ca;
                        Paa += db * Caa;
                        Paaa += db * Caaa;
                        Pb += da * Cb;
                        Pbb += da * Cbb;
                        Pbbb += da * Cbbb;
                        Pab += db * (b1 * Cab + b0 * Kab);
                        Paab += db * (b1 * Caab + b0 * Kaab);
                        Pabb += da * (a1 * Cabb + a0 * Kabb);
                    }

                    P1 /= 2.0;
                    Pa /= 6.0;
                    Paa /= 12.0;
                    Paaa /= 20.0;
                    Pb /= -6.0;
                    Pbb /= -12.0;
                    Pbbb /= -20.0;
                    Pab /= 24.0;
                    Paab /= 60.0;
                    Pabb /= -60.0;
                }

                w = - dDOT(n, v[0]);

                k1 = 1 / n[C];
                k2 = k1 * k1;
                k3 = k2 * k1;
                k4 = k3 * k1;

                Fa = k1 * Pa;
                Fb = k1 * Pb;
                Fc = -k2 * (n[A] * Pa + n[B] * Pb + w * P1);

                Faa = k1 * Paa;
                Fbb = k1 * Pbb;
                Fcc = k3 * (SQR(n[A]) * Paa + 2 * n[A] * n[B] * Pab + SQR(n[B]) * Pbb +
                            w * (2 * (n[A] * Pa + n[B] * Pb) + w * P1));

                Faaa = k1 * Paaa;
                Fbbb = k1 * Pbbb;
                Fccc = -k4 * (CUBE(n[A]) * Paaa + 3 * SQR(n[A]) * n[B] * Paab
                              + 3 * n[A] * SQR(n[B]) * Pabb + CUBE(n[B]) * Pbbb
                              + 3 * w * (SQR(n[A]) * Paa + 2 * n[A] * n[B] * Pab + SQR(n[B]) * Pbb)
                              + w * w * (3 * (n[A] * Pa + n[B] * Pb) + w * P1));

                Faab = k1 * Paab;
                Fbbc = -k2 * (n[A] * Pabb + n[B] * Pbbb + w * Pbb);
                Fcca = k3 * (SQR(n[A]) * Paaa + 2 * n[A] * n[B] * Paab + SQR(n[B]) * Pabb
                             + w * (2 * (n[A] * Paa + n[B] * Pab) + w * Pa));
            }


            T0 += n[0] * ((A == 0) ? Fa : ((B == 0) ? Fb : Fc));

            T1[A] += n[A] * Faa;
            T1[B] += n[B] * Fbb;
            T1[C] += n[C] * Fcc;
            T2[A] += n[A] * Faaa;
            T2[B] += n[B] * Fbbb;
            T2[C] += n[C] * Fccc;
            TP[A] += n[A] * Faab;
            TP[B] += n[B] * Fbbc;
            TP[C] += n[C] * Fcca;
        }
    }

    T1[0] /= 2;
    T1[1] /= 2;
    T1[2] /= 2;
    T2[0] /= 3;
    T2[1] /= 3;
    T2[2] /= 3;
    TP[0] /= 2;
    TP[1] /= 2;
    TP[2] /= 2;

    m->mass = density * T0;
// #define _I(i,j) I[(i)*4+(j)]
// regex _I\(([0-9]+),([0-9]+)\) to I[(\1)*4+(\2)]
//    m->_I(0,0) = density * (T2[1] + T2[2]);
//    m->_I(1,1) = density * (T2[2] + T2[0]);
//    m->_I(2,2) = density * (T2[0] + T2[1]);
//    m->_I(0,1) = - density * TP[0];
//    m->_I(1,0) = - density * TP[0];
//    m->_I(2,1) = - density * TP[1];
//    m->_I(1,2) = - density * TP[1];
//    m->_I(2,0) = - density * TP[2];
//    m->_I(0,2) = - density * TP[2];
    m->I[0 * 4 + 0] = density * (T2[1] + T2[2]);
    m->I[1 * 4 + 1] = density * (T2[2] + T2[0]);
    m->I[2 * 4 + 2] = density * (T2[0] + T2[1]);
    m->I[0 * 4 + 1] = - density * TP[0];
    m->I[1 * 4 + 0] = - density * TP[0];
    m->I[2 * 4 + 1] = - density * TP[1];
    m->I[1 * 4 + 2] = - density * TP[1];
    m->I[2 * 4 + 0] = - density * TP[2];
    m->I[0 * 4 + 2] = - density * TP[2];

    m->c[0] = T1[0] / T0;
    m->c[1] = T1[1] / T0;
    m->c[2] = T1[2] / T0;
}

// reverse the face winding
void FacetedObject::ReverseWinding()
{
    double t;
    size_t numTriangles = (mVertexList.size() / 3) / 3;
    size_t i, j;
    for (i = 0; i < numTriangles; i++)
    {
        for (j = 0; j < 3; j++)
        {
            t = mVertexList[i * 9 + 3 + j];
            mVertexList[i * 9 + 3 + j] = mVertexList[i * 9 + 6 + j];
            mVertexList[i * 9 + 6 + j] = t;
        }
    }
    for (i = 0; i < mNormalList.size(); i++) mNormalList[i] = -mNormalList[i];
}

// add the faces from one faceted object to another
void FacetedObject::AddFacetedObject(FacetedObject *object, bool useDisplayRotation)
{
    size_t numTriangles = object->GetNumTriangles();
    double *triangle, *p1;
    double triangle2[9];
    dVector3 v1, v1r;

    if (useDisplayRotation)
    {
        for (size_t i = 0; i < numTriangles; i++)
        {
            triangle = object->GetTriangle(i);
            for (int j = 0; j < 3; j++)
            {
                p1 = triangle + 3 * j;
                v1[0] = p1[0];
                v1[1] = p1[1];
                v1[2] = p1[2];
                v1[3] = 0;
                dMULTIPLY0_331(v1r, m_DisplayRotation, v1);
                p1 = triangle2 + 3 * j;
                p1[0] = v1r[0] + m_DisplayPosition[0];
                p1[1] = v1r[1] + m_DisplayPosition[1];
                p1[2] = v1r[2] + m_DisplayPosition[2];
            }
            AddTriangle(triangle2);
        }
    }
    else
    {
        for (size_t i = 0; i < numTriangles; i++)
        {
            triangle = object->GetTriangle(i);
            AddTriangle(triangle);
        }
    }
}

size_t FacetedObject::OBJFaceVertexDecode(char *string, size_t *output)
{
    char *p = string;
    char *lastP = p;
    size_t count = 0;
    while (*p != 0)
    {
        if (*p == '/')
        {
            *p = 0;
            if (lastP) output[count++] = strtoull(lastP, nullptr, 10);
            else output[count++] = SIZE_MAX;
            lastP = p + 1;
        }
        p++;
    }
    output[count++] = strtoull(lastP, nullptr, 10);
    return count;
}

