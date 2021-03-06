/*
 *  FacetedObject.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 13/09/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

// #pragma warning( disable : 4100 )

#include "FacetedObject.h"
#include "FacetedSphere.h"
#include "DataFile.h"
#include "GSUtil.h"
#include "GLUtils.h"
#include "PGDMath.h"
#include "SimulationWidget.h"
#include "Preferences.h"
#include "pystring.h"

#define TINYPLY_IMPLEMENTATION
#include "tinyply.h"
#include "ode/ode.h"

#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLTexture>
#if QT_VERSION >= 0x060000
#include <QOpenGLVersionFunctionsFactory>
#endif


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
#include <regex>

using namespace std::literals::string_literals;

// create object
FacetedObject::FacetedObject()
{
}

// destroy object
FacetedObject::~FacetedObject()
{
}

// utility front end to try and parse different sorts of mesh files
int FacetedObject::ParseMeshFile(const std::string &filename)
{
    std::string lowercase = pystring::lower(filename);
    if (pystring::endswith(lowercase, ".obj"s)) return ParseOBJFile(filename);
    if (pystring::endswith(lowercase, ".ply"s)) return ParsePLYFile(filename);
    return __LINE__;
}

// parse an OBJ file to a FacetedObject
// returns 0 on success
int FacetedObject::ParseOBJFile(const std::string &filename)
{
    m_filename = filename;
    MeshStoreObject *meshStoreObject = m_meshStore.getMesh(filename);
    if (meshStoreObject)
    {
        m_vertexList = meshStoreObject->vertexList;
        m_normalList = meshStoreObject->normalList;
        m_colourList = meshStoreObject->colourList;
        m_uvList = meshStoreObject->uvList;
        m_lowerBound[0] = meshStoreObject->lowerBound[0];
        m_lowerBound[1] = meshStoreObject->lowerBound[1];
        m_lowerBound[2] = meshStoreObject->lowerBound[2];
        m_upperBound[0] = meshStoreObject->upperBound[0];
        m_upperBound[1] = meshStoreObject->upperBound[1];
        m_upperBound[2] = meshStoreObject->upperBound[2];
        return 0;
    }

    // read the whole file into memory
    DataFile theFile;
    if (theFile.ReadFile(filename) == true) return __LINE__;
    char *ptr = theFile.GetRawData();
    char *endPtr = ptr + theFile.GetSize();

    pgd::Vector3 vertex;
    std::vector<pgd::Vector3> vertexList;
    pgd::Vector3 normal;
    std::vector<pgd::Vector3> normalList;
    pgd::Vector2 uv;
    std::vector<pgd::Vector2> uvList;
    struct Triangle
    {
        size_t vertex[3];
        size_t normal[3];
        size_t uv[3];
        OBJMaterial *material;
    };
    Triangle triangle;
    std::vector<Triangle> triangleList;
    // having checked a few files ball park is 20M file, 100k vertices, 200k faces, 200k normals
    size_t estimated_max_capacity = theFile.GetSize() / 25; // this will give a factor of 4 to 8 leeway
    vertexList.reserve(estimated_max_capacity);
    normalList.reserve(estimated_max_capacity);
    uvList.reserve(estimated_max_capacity);
    triangleList.reserve(estimated_max_capacity);

    enum FaceFormat { unknown, vertex_only, vertex_texture, vertex_normal, vertex_texture_normal };
    FaceFormat faceFormat = unknown;

    std::map<std::string, OBJMaterial> materialMap;
    OBJMaterial *currentMaterial = nullptr;
    std::string line;
    line.reserve(1024);
    while (ptr < endPtr)
    {
        // non content (whitespace)
        if (*ptr < 32)
        {
            while (*ptr < 32 && ptr < endPtr) { ptr++; }
            continue;
        }

        // comments
        if (*ptr == '#')
        {
            ptr++;
            while (*ptr != '\n' && *ptr != '\r' && ptr < endPtr) { ptr++; }
            ptr++;
            continue;
        }

        // materials definitions
        if (*ptr == 'm')
        {
            // first extract the line
            line.clear();
            while (*ptr != '\n' && *ptr != '\r' && ptr < endPtr)
            {
                line.append(1, *ptr);
                ptr++;
            }
            if (pystring::startswith(line, "mtllib "s))
            {
                std::string materialsFile = pystring::os::path::join(pystring::os::path::dirname(filename), line.substr("mtllib "s.size(), std::string::npos));
                if (ParseOBJMaterialFile(materialsFile, &materialMap))
                {
                    qDebug() << "Error reading material file \"" << materialsFile.c_str() << "\"";
                }
            }
            continue;
        }

        // materials use
        if (*ptr == 'u')
        {
            // first extract the line
            line.clear();
            while (*ptr != '\n' && *ptr != '\r' && ptr < endPtr)
            {
                line.append(1, *ptr);
                ptr++;
            }
            if (pystring::startswith(line, "usemtl "s))
            {
                std::string material = pystring::strip(line.substr("usemtl "s.size(), std::string::npos));
                auto it = materialMap.find(material);
                if (it == materialMap.end()) currentMaterial = nullptr;
                else currentMaterial = &it->second;
            }
            continue;
        }

        // vertices and normals and texture coords
        if (*ptr == 'v')
        {
            ptr++;
            if (ptr >= endPtr) continue;
            if (*ptr == ' ')
            {
                vertex.x = strtod(ptr, &ptr);
                if (ptr >= endPtr) continue;
                vertex.y = strtod(ptr, &ptr);
                if (ptr >= endPtr) continue;
                vertex.z = strtod(ptr, &ptr);
                vertexList.push_back(vertex);
                if (vertex.x < m_lowerBound[0]) m_lowerBound[0] = vertex.x;
                if (vertex.y < m_lowerBound[1]) m_lowerBound[1] = vertex.y;
                if (vertex.z < m_lowerBound[2]) m_lowerBound[2] = vertex.z;
                if (vertex.x > m_upperBound[0]) m_upperBound[0] = vertex.x;
                if (vertex.y > m_upperBound[1]) m_upperBound[1] = vertex.y;
                if (vertex.z > m_upperBound[2]) m_upperBound[2] = vertex.z;
                ptr++;
                continue;
            }
            if (*ptr == 'n')
            {
                ptr++;
                if (ptr >= endPtr) continue;
                normal.x = strtod(ptr, &ptr);
                if (ptr >= endPtr) continue;
                normal.y = strtod(ptr, &ptr);
                if (ptr >= endPtr) continue;
                normal.z = strtod(ptr, &ptr);
                normalList.push_back(normal);
                ptr++;
                continue;
            }
            if (*ptr == 't')
            {
                ptr++;
                if (ptr >= endPtr) continue;
                uv.x = strtod(ptr, &ptr);
                if (ptr >= endPtr) continue;
                uv.y = strtod(ptr, &ptr);
                uvList.push_back(uv);
                ptr++;
                continue;
            }
            // not recognised so skip to the end of line
            while (*ptr != '\n' && *ptr != '\r' && ptr < endPtr) { ptr++; }
            ptr++;
            continue;
        }

        // faces (only triangular faces supported)
        if (*ptr == 'f')
        {
            ptr++;
            while (faceFormat == unknown)
            {
                const std::regex vertex_only_tester("[ \t]*[0-9]+[ \t]+[0-9]+[ \t]+[0-9]+[ \t]*"s);
                const std::regex vertex_texture_tester("[ \t]*[0-9]+/[0-9]+[ \t]+[0-9]+/[0-9]+[ \t]+[0-9]+/[0-9]+[ \t]*"s);
                const std::regex vertex_normal_tester("[ \t]*[0-9]+//[0-9]+[ \t]+[0-9]+//[0-9]+[ \t]+[0-9]+//[0-9]+[ \t]*"s);
                const std::regex vertex_texture_normal_tester("[ \t]*[0-9]+/[0-9]+/[0-9]+[ \t]+[0-9]+/[0-9]+/[0-9]+[ \t]+[0-9]+/[0-9]+/[0-9]+[ \t]*"s);
                char *localEndPtr = ptr + 1;
                while (*localEndPtr != '\0' && *localEndPtr != '\r' && *localEndPtr != '\n' && localEndPtr < endPtr) localEndPtr++;
                std::string testStr(ptr, size_t(localEndPtr - ptr));
                std::smatch sm;
                if (std::regex_match(testStr, sm, vertex_only_tester)) { faceFormat = vertex_only; break; }
                if (std::regex_match(testStr, sm, vertex_texture_tester)) { faceFormat = vertex_texture; break; }
                if (std::regex_match(testStr, sm, vertex_normal_tester)) { faceFormat = vertex_normal; break; }
                if (std::regex_match(testStr, sm, vertex_texture_normal_tester)) { faceFormat = vertex_texture_normal; break; }
                break;
            }

            if (faceFormat == vertex_only)
            {
                triangle.vertex[0] = strtoull(ptr, &ptr, 10) - 1;
                if (ptr >= endPtr) continue;
                triangle.vertex[1] = strtoull(ptr, &ptr, 10) - 1;
                if (ptr >= endPtr) continue;
                triangle.vertex[2] = strtoull(ptr, &ptr, 10) - 1;
                triangle.normal[0] = SIZE_MAX;
                triangle.normal[1] = SIZE_MAX;
                triangle.normal[2] = SIZE_MAX;
                triangle.uv[0] = SIZE_MAX;
                triangle.uv[1] = SIZE_MAX;
                triangle.material = currentMaterial;
                triangleList.push_back(triangle);
                ptr++;
                if (m_badMesh)   // currently duplicate the triangle but with reversed winding but this could be improved
                {

                    std::swap(triangle.vertex[1], triangle.vertex[2]);
                    std::swap(triangle.normal[1], triangle.normal[2]);
                    std::swap(triangle.uv[1], triangle.uv[2]);
                    triangleList.push_back(triangle);
                }
                continue;
            }

            if (faceFormat == vertex_normal)
            {
                triangle.vertex[0] = strtoull(ptr, &ptr, 10) - 1;
                ptr += 2;
                if (ptr >= endPtr) continue;
                triangle.normal[0] = strtoull(ptr, &ptr, 10) - 1;
                if (ptr >= endPtr) continue;
                triangle.vertex[1] = strtoull(ptr, &ptr, 10) - 1;
                ptr += 2;
                if (ptr >= endPtr) continue;
                triangle.normal[1] = strtoull(ptr, &ptr, 10) - 1;
                if (ptr >= endPtr) continue;
                triangle.vertex[2] = strtoull(ptr, &ptr, 10) - 1;
                ptr += 2;
                if (ptr >= endPtr) continue;
                triangle.normal[2] = strtoull(ptr, &ptr, 10) - 1;
                triangle.uv[0] = SIZE_MAX;
                triangle.uv[1] = SIZE_MAX;
                triangle.material = currentMaterial;
                triangleList.push_back(triangle);
                ptr++;
                if (m_badMesh)   // currently duplicate the triangle but with reversed winding but this could be improved
                {

                    std::swap(triangle.vertex[1], triangle.vertex[2]);
                    std::swap(triangle.normal[1], triangle.normal[2]);
                    std::swap(triangle.uv[1], triangle.uv[2]);
                    triangleList.push_back(triangle);
                }
                continue;
            }

            if (faceFormat == vertex_texture)
            {
                triangle.vertex[0] = strtoull(ptr, &ptr, 10) - 1;
                ptr++;
                if (ptr >= endPtr) continue;
                triangle.uv[0] = strtoull(ptr, &ptr, 10) - 1;
                if (ptr >= endPtr) continue;
                triangle.normal[0] = SIZE_MAX;
                triangle.vertex[1] = strtoull(ptr, &ptr, 10) - 1;
                ptr++;
                if (ptr >= endPtr) continue;
                triangle.uv[1] = strtoull(ptr, &ptr, 10) - 1;
                if (ptr >= endPtr) continue;
                triangle.normal[1] = SIZE_MAX;
                triangle.vertex[2] = strtoull(ptr, &ptr, 10) - 1;
                ptr++;
                if (ptr >= endPtr) continue;
                triangle.uv[2] = strtoull(ptr, &ptr, 10) - 1;
                triangle.normal[2] = SIZE_MAX;
                triangle.material = currentMaterial;
                triangleList.push_back(triangle);
                ptr++;
                if (m_badMesh)   // currently duplicate the triangle but with reversed winding but this could be improved
                {

                    std::swap(triangle.vertex[1], triangle.vertex[2]);
                    std::swap(triangle.normal[1], triangle.normal[2]);
                    std::swap(triangle.uv[1], triangle.uv[2]);
                    triangleList.push_back(triangle);
                }
                continue;
            }

            if (faceFormat == vertex_texture_normal)
            {
                triangle.vertex[0] = strtoull(ptr, &ptr, 10) - 1;
                ptr++;
                if (ptr >= endPtr) continue;
                triangle.uv[0] = strtoull(ptr, &ptr, 10) - 1;
                ptr++;
                if (ptr >= endPtr) continue;
                triangle.normal[0] = strtoull(ptr, &ptr, 10) - 1;
                if (ptr >= endPtr) continue;
                triangle.vertex[1] = strtoull(ptr, &ptr, 10) - 1;
                ptr++;
                if (ptr >= endPtr) continue;
                triangle.uv[1] = strtoull(ptr, &ptr, 10) - 1;
                ptr++;
                if (ptr >= endPtr) continue;
                triangle.normal[1] = strtoull(ptr, &ptr, 10) - 1;
                if (ptr >= endPtr) continue;
                triangle.vertex[2] = strtoull(ptr, &ptr, 10) - 1;
                ptr++;
                if (ptr >= endPtr) continue;
                triangle.uv[2] = strtoull(ptr, &ptr, 10) - 1;
                ptr++;
                if (ptr >= endPtr) continue;
                triangle.normal[2] = strtoull(ptr, &ptr, 10) - 1;
                triangle.material = currentMaterial;
                triangleList.push_back(triangle);
                ptr++;
                if (m_badMesh)   // currently duplicate the triangle but with reversed winding but this could be improved
                {
                    std::swap(triangle.vertex[1], triangle.vertex[2]);
                    std::swap(triangle.normal[1], triangle.normal[2]);
                    std::swap(triangle.uv[1], triangle.uv[2]);
                    triangleList.push_back(triangle);
                }
                continue;
            }
        }

        // skip till end of line
        while (ptr < endPtr && *ptr != '\n' && *ptr != '\r') { ptr++; }
        ptr++;
    }

    m_vertexList.clear();
    m_vertexList.reserve(triangleList.size() * 9);
    m_normalList.clear();
    m_normalList.reserve(triangleList.size() * 9);
    m_colourList.clear();
    m_colourList.reserve(triangleList.size() * 9);
    m_uvList.clear();
    m_uvList.reserve(triangleList.size() * 6);
    double colour[3] = {m_blendColour.redF(), m_blendColour.greenF(), m_blendColour.blueF() };
    for (auto it : triangleList)
    {
        m_vertexList.push_back(vertexList[it.vertex[0]].x);
        m_vertexList.push_back(vertexList[it.vertex[0]].y);
        m_vertexList.push_back(vertexList[it.vertex[0]].z);
        m_vertexList.push_back(vertexList[it.vertex[1]].x);
        m_vertexList.push_back(vertexList[it.vertex[1]].y);
        m_vertexList.push_back(vertexList[it.vertex[1]].z);
        m_vertexList.push_back(vertexList[it.vertex[2]].x);
        m_vertexList.push_back(vertexList[it.vertex[2]].y);
        m_vertexList.push_back(vertexList[it.vertex[2]].z);
        if (it.normal[0] != SIZE_MAX)
        {
            m_normalList.push_back(normalList[it.normal[0]].x);
            m_normalList.push_back(normalList[it.normal[0]].y);
            m_normalList.push_back(normalList[it.normal[0]].z);
            m_normalList.push_back(normalList[it.normal[1]].x);
            m_normalList.push_back(normalList[it.normal[1]].y);
            m_normalList.push_back(normalList[it.normal[1]].z);
            m_normalList.push_back(normalList[it.normal[2]].x);
            m_normalList.push_back(normalList[it.normal[2]].y);
            m_normalList.push_back(normalList[it.normal[2]].z);
        }
        else
        {
            ComputeFaceNormal(vertexList[it.vertex[0]].data(), vertexList[it.vertex[1]].data(), vertexList[it.vertex[2]].data(), normal.data());
            m_normalList.push_back(normal.x);
            m_normalList.push_back(normal.y);
            m_normalList.push_back(normal.z);
            m_normalList.push_back(normal.x);
            m_normalList.push_back(normal.y);
            m_normalList.push_back(normal.z);
            m_normalList.push_back(normal.x);
            m_normalList.push_back(normal.y);
            m_normalList.push_back(normal.z);
        }
        if (it.material)
        {
            for (size_t i = 0; i < 9; i++)
            {
                m_colourList.push_back(it.material->Kd[i % 3]);
            }
        }
        else
        {
            for (size_t i = 0; i < 9; i++) m_colourList.push_back(colour[i % 3]);
        }
        if (it.uv[0] != SIZE_MAX)
        {
            m_uvList.push_back(uvList[it.uv[0]].x);
            m_uvList.push_back(uvList[it.uv[0]].y);
            m_uvList.push_back(uvList[it.uv[1]].x);
            m_uvList.push_back(uvList[it.uv[1]].y);
            m_uvList.push_back(uvList[it.uv[2]].x);
            m_uvList.push_back(uvList[it.uv[2]].y);
        }
        else
        {
            m_uvList.push_back(0);
            m_uvList.push_back(0);
            m_uvList.push_back(0);
            m_uvList.push_back(0);
            m_uvList.push_back(0);
            m_uvList.push_back(0);
        }
    }

    m_meshStore.setTargetMemory(Preferences::valueDouble("MeshStoreMemoryFraction"));
    m_meshStore.addMesh(filename, m_vertexList, m_normalList, m_colourList, m_uvList, m_lowerBound, m_upperBound);

    return 0;
}

int FacetedObject::ParseOBJMaterialFile(const std::string &filename, std::map<std::string, OBJMaterial> *materialMap)
{
    DataFile materialsFile;
    if (materialsFile.ReadFile(filename))
    {
        std::string strippedFilename = pystring::strip(filename);
        if (materialsFile.ReadFile(strippedFilename))
        {
            if (strippedFilename[0] == '"' && strippedFilename.back() == '"')
            {
                if (materialsFile.ReadFile(strippedFilename.substr(1, strippedFilename.size() - 2)))
                    return __LINE__;
            }
            else
            {
                return __LINE__;
            }
        }
    }

    std::string materialsData(materialsFile.GetRawData(), materialsFile.GetSize());
    std::vector<std::string> lines;
    pystring::splitlines(materialsData, lines);
    std::string currentMaterialName;
    OBJMaterial currentMaterial = {};
    for (size_t i =0; i < lines.size(); i++)
    {
        std::string line = pystring::strip(lines[i]);
        if (pystring::startswith(line, "#"s)) continue;
        if (pystring::startswith(line, "newmtl "s))
        {
            if (currentMaterialName.size())
            {
                (*materialMap)[currentMaterialName] = currentMaterial;
                currentMaterial = {};
            }
            currentMaterialName = pystring::strip(line.substr("newmtl "s.size(), std::string::npos));
            continue;
        }

        std::vector<std::string> tokens;
        pystring::split(line, tokens);
        if (tokens.size() >= 2 && tokens[0] == "Ns"s)
        {
            currentMaterial.Ns = GSUtil::Double(tokens[1]);
            continue;
        }
        if (tokens.size() >= 4 && tokens[0] == "Ka"s)
        {
            currentMaterial.Ka[0] =GSUtil::Double(tokens[1]);
            currentMaterial.Ka[1] =GSUtil::Double(tokens[2]);
            currentMaterial.Ka[2] =GSUtil::Double(tokens[3]);
            continue;
        }
        if (tokens.size() >= 4 && tokens[0] == "Kd"s)
        {
            currentMaterial.Kd[0] =GSUtil::Double(tokens[1]);
            currentMaterial.Kd[1] =GSUtil::Double(tokens[2]);
            currentMaterial.Kd[2] =GSUtil::Double(tokens[3]);
            continue;
        }
        if (tokens.size() >= 4 && tokens[0] == "Ks"s)
        {
            currentMaterial.Ks[0] =GSUtil::Double(tokens[1]);
            currentMaterial.Ks[1] =GSUtil::Double(tokens[2]);
            currentMaterial.Ks[2] =GSUtil::Double(tokens[3]);
            continue;
        }
        if (tokens.size() >= 4 && tokens[0] == "Ke"s)
        {
            currentMaterial.Ke[0] =GSUtil::Double(tokens[1]);
            currentMaterial.Ke[1] =GSUtil::Double(tokens[2]);
            currentMaterial.Ke[2] =GSUtil::Double(tokens[3]);
            continue;
        }
        if (tokens.size() >= 2 && tokens[0] == "Ni"s)
        {
            currentMaterial.Ni = GSUtil::Double(tokens[1]);
            continue;
        }
        if (tokens.size() >= 2 && tokens[0] == "d"s)
        {
            currentMaterial.d = GSUtil::Double(tokens[1]);
            continue;
        }
        if (tokens.size() >= 2 && tokens[0] == "illum"s)
        {
            currentMaterial.illum = GSUtil::Int(tokens[1]);
            continue;
        }
    }
    if (currentMaterialName.size())
    {
        (*materialMap)[currentMaterialName] = currentMaterial;
        currentMaterial = {};
    }
    return 0;
}

int FacetedObject::ParsePLYFile(const std::string &filename)
{
    m_filename = filename;
    MeshStoreObject *meshStoreObject = m_meshStore.getMesh(filename);
    if (meshStoreObject)
    {
        m_vertexList = meshStoreObject->vertexList;
        m_normalList = meshStoreObject->normalList;
        m_colourList = meshStoreObject->colourList;
        m_uvList = meshStoreObject->uvList;
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

        for (auto &&c : file.get_comments()) qDebug() << "Comment: " << c.c_str() << "\n";
        std::vector<tinyply::PlyElement> elementVector = file.get_elements();
        std::map<std::string, tinyply::PlyProperty *> vertexProperties;
        std::map<std::string, tinyply::PlyProperty *> faceProperties;
        for (auto &&e : elementVector)
        {
            qDebug() << "element - " << e.name.c_str() << " (" << e.size << ")" << "\n";
            if (e.name == "vertex") for (auto &&p : e.properties) vertexProperties[p.name] = &p;
            else if (e.name == "face") for (auto &&p : e.properties) faceProperties[p.name] = &p;
            for (auto &&p : e.properties)
            {
                if (p.isList) qDebug() << "\tproperty - " << p.name.c_str() << " (" << tinyply::PropertyTable[p.listType].str.c_str() << ")" << " (" << tinyply::PropertyTable[p.propertyType].str.c_str() << ")" << "\n";
                else
                    qDebug() << "\tproperty - " << p.name.c_str() << " (" << tinyply::PropertyTable[p.propertyType].str.c_str() << ")" << "\n";
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
                qDebug() << "tinyply exception: " << e.what() << "\n";
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
                qDebug() << "tinyply exception: " << e.what() << "\n";
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
                qDebug() << "tinyply exception: " << e.what() << "\n";
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
                qDebug() << "tinyply exception: " << e.what() << "\n";
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
                qDebug() << "tinyply exception: " << e.what() << "\n";
            }
        }

        file.read(ss);

        if (vertices) qDebug() << "\tRead " << vertices->count << " total vertices " << "\n";
        if (normals) qDebug() << "\tRead " << normals->count << " total vertex normals " << "\n";
        if (texcoords) qDebug() << "\tRead " << texcoords->count << " total vertex texcoords " << "\n";
        if (colours) qDebug() << "\tRead " << colours->count << " total vertex colours " << "\n";
        if (faces) qDebug() << "\tRead " << faces->count << " total faces " << "\n";

        AllocateMemory(faces->count);
        if (vertices->t == tinyply::Type::FLOAT32)
        {
            int32_t *vertexIndexPtr = reinterpret_cast<int32_t *>(faces->buffer.get());
            float *vertexPtr = reinterpret_cast<float *>(vertices->buffer.get());
            double triangle[9];
            for (size_t i = 0; i < faces->count; i++)
            {
                for (size_t j = 0; j < 3; j++) (&triangle[0])[j] = double((&vertexPtr[3 * vertexIndexPtr[i * 3 + 0]])[j]);
                for (size_t j = 0; j < 3; j++) (&triangle[3])[j] = double((&vertexPtr[3 * vertexIndexPtr[i * 3 + 1]])[j]);
                for (size_t j = 0; j < 3; j++) (&triangle[6])[j] = double((&vertexPtr[3 * vertexIndexPtr[i * 3 + 2]])[j]);
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

        m_meshStore.setTargetMemory(Preferences::valueDouble("MeshStoreMemoryFraction"));
        m_meshStore.addMesh(filename, m_vertexList, m_normalList, m_colourList, m_uvList, m_lowerBound, m_upperBound);
    }
    catch (const std::exception &e)
    {
        qDebug() << "Caught tinyply exception: " << e.what() << "\n";
        return __LINE__;
    }
    return 0;
}

int FacetedObject::ReadFromMemory(const char *data, size_t len, bool binary, const std::string &meshName)
{
    m_filename = meshName;
    MeshStoreObject *meshStoreObject = m_meshStore.getMesh(meshName);
    if (meshStoreObject)
    {
        m_vertexList = meshStoreObject->vertexList;
        m_normalList = meshStoreObject->normalList;
        m_colourList = meshStoreObject->colourList;
        m_uvList = meshStoreObject->uvList;
        m_lowerBound[0] = meshStoreObject->lowerBound[0];
        m_lowerBound[1] = meshStoreObject->lowerBound[1];
        m_lowerBound[2] = meshStoreObject->lowerBound[2];
        m_upperBound[0] = meshStoreObject->upperBound[0];
        m_upperBound[1] = meshStoreObject->upperBound[1];
        m_upperBound[2] = meshStoreObject->upperBound[2];
        return 0;
    }

    m_vertexList.clear();
    m_normalList.clear();
    m_colourList.clear();
    m_uvList.clear();
    if (binary)
    {
        if (len < sizeof(size_t) + 6 * sizeof(double)) return __LINE__;
        const char *ptr = data;
        size_t numTriangles = *reinterpret_cast<size_t *>(*ptr);
        ptr += sizeof(size_t);
        if (len < sizeof(size_t) + 6 * sizeof(double) + numTriangles * 3 * 11 * sizeof(double)) return __LINE__;
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
        size_t numElements = numTriangles * 9;
        m_vertexList.reserve(numElements);
        m_normalList.reserve(numElements);
        m_colourList.reserve(numElements);
        m_uvList.reserve(numTriangles * 6);
        for (size_t i = 0; i < numElements; i++)
        {
            m_vertexList.push_back(*reinterpret_cast<double *>(*ptr));
            ptr += sizeof(double);
        }
        for (size_t i = 0; i < numElements; i++)
        {
            m_normalList.push_back(*reinterpret_cast<double *>(*ptr));
            ptr += sizeof(double);
        }
        for (size_t i = 0; i < numElements; i++)
        {
            m_colourList.push_back(*reinterpret_cast<double *>(*ptr));
            ptr += sizeof(double);
        }
        for (size_t i = 0; i < numTriangles * 6; i++)
        {
            m_uvList.push_back(*reinterpret_cast<double *>(*ptr));
            ptr += sizeof(double);
        }
    }
    else
    {
        if (data[len]) return __LINE__; // must be null terminated for ASCII case
        char *endPtr;
        size_t numTriangles = std::strtoull(data, &endPtr, 0);
        m_lowerBound[0] = std::strtod(endPtr, &endPtr);
        m_lowerBound[1] = std::strtod(endPtr, &endPtr);
        m_lowerBound[2] = std::strtod(endPtr, &endPtr);
        m_upperBound[0] = std::strtod(endPtr, &endPtr);
        m_upperBound[1] = std::strtod(endPtr, &endPtr);
        m_upperBound[2] = std::strtod(endPtr, &endPtr);
        size_t numElements = numTriangles * 9;
        m_vertexList.reserve(numElements);
        m_normalList.reserve(numElements);
        m_colourList.reserve(numElements);
        m_uvList.reserve(numTriangles * 6);
        for (size_t i = 0; i < numElements; i++) m_vertexList.push_back(std::strtod(endPtr, &endPtr));
        for (size_t i = 0; i < numElements; i++) m_normalList.push_back(std::strtod(endPtr, &endPtr));
        for (size_t i = 0; i < numElements; i++) m_colourList.push_back(std::strtod(endPtr, &endPtr));
        for (size_t i = 0; i < numTriangles * 6; i++) m_uvList.push_back(std::strtod(endPtr, &endPtr));
    }
    m_meshStore.setTargetMemory(Preferences::valueDouble("MeshStoreMemoryFraction"));
    m_meshStore.addMesh(meshName, m_vertexList, m_normalList, m_colourList, m_uvList, m_lowerBound, m_upperBound);
    return 0;
}

void FacetedObject::SaveToMemory(std::vector<char> *data, bool binary)
{
    if (binary)
    {
        data->clear();
        data->resize(sizeof(size_t) + 6 * sizeof(double) + m_vertexList.size() * sizeof(double) +
                     m_normalList.size() * sizeof(double) +
                     m_colourList.size() * sizeof(double) +
                     m_uvList.size() * sizeof(double));
        size_t *ptr = reinterpret_cast<size_t *>(data->data());
        *ptr++ = m_vertexList.size() / 3;
        double *dPtr = reinterpret_cast<double *>(ptr);
        *dPtr++ = m_lowerBound[0];
        *dPtr++ = m_lowerBound[1];
        *dPtr++ = m_lowerBound[2];
        *dPtr++ = m_upperBound[0];
        *dPtr++ = m_upperBound[1];
        *dPtr++ = m_upperBound[2];
        for (size_t i = 0; i < m_vertexList.size(); i++) *dPtr++ = m_vertexList[i];
        for (size_t i = 0; i < m_normalList.size(); i++) *dPtr++ = m_normalList[i];
        for (size_t i = 0; i < m_colourList.size(); i++) *dPtr++ = m_colourList[i];
        for (size_t i = 0; i < m_uvList.size(); i++) *dPtr++ = m_uvList[i];
    }
    else
    {
        data->clear();
        char buf[32];
        int l = std::sprintf(buf, "%zud\n", m_vertexList.size() / 3);
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
        for (size_t i = 0; i < m_vertexList.size(); i++)
        {
            l = std::sprintf(buf, "%.18g\n", m_vertexList[i]);
            std::copy_n(buf, l, std::back_inserter(*data));
        }
        for (size_t i = 0; i < m_normalList.size(); i++)
        {
            l = std::sprintf(buf, "%.18g\n", m_normalList[i]);
            std::copy_n(buf, l, std::back_inserter(*data));
        }
        for (size_t i = 0; i < m_colourList.size(); i++)
        {
            l = std::sprintf(buf, "%.18g\n", m_colourList[i]);
            std::copy_n(buf, l, std::back_inserter(*data));
        }
        for (size_t i = 0; i < m_uvList.size(); i++)
        {
            l = std::sprintf(buf, "%.18g\n", m_uvList[i]);
            std::copy_n(buf, l, std::back_inserter(*data));
        }
        data->push_back('\0');
    }
}

int FacetedObject::ReadFromResource(const QString &resourceName)
{
    m_filename = resourceName.toStdString();
    MeshStoreObject *meshStoreObject = m_meshStore.getMesh(resourceName.toStdString());
    if (meshStoreObject)
    {
        m_vertexList = meshStoreObject->vertexList;
        m_normalList = meshStoreObject->normalList;
        m_colourList = meshStoreObject->colourList;
        m_uvList = meshStoreObject->uvList;
        m_lowerBound[0] = meshStoreObject->lowerBound[0];
        m_lowerBound[1] = meshStoreObject->lowerBound[1];
        m_lowerBound[2] = meshStoreObject->lowerBound[2];
        m_upperBound[0] = meshStoreObject->upperBound[0];
        m_upperBound[1] = meshStoreObject->upperBound[1];
        m_upperBound[2] = meshStoreObject->upperBound[2];
        return 0;
    }

    QFile file(resourceName);
    if (!file.open(QIODevice::ReadOnly)) return __LINE__;
    QByteArray data = file.readAll();
    file.close();
    QTextStream dataStream(data, QIODevice::ReadOnly);

    m_vertexList.clear();
    m_normalList.clear();
    m_colourList.clear();
    m_uvList.clear();

    QString key;
    dataStream >> key;
    if (key != "xyzxnynznrgb") return __LINE__;
    size_t numTriangles;
    dataStream >> numTriangles;
    if (numTriangles <= 0) return __LINE__;
    size_t numVertices = numTriangles * 3;
    size_t numElements = numVertices * 3;
    dataStream >> m_lowerBound[0] >> m_lowerBound[1] >> m_lowerBound[2] >> m_upperBound[0] >> m_upperBound[1] >> m_upperBound[2];

    m_vertexList.reserve(numElements);
    m_normalList.reserve(numElements);
    m_colourList.reserve(numElements);
    m_uvList.reserve(numVertices * 2);
    double x, y, z, nx, ny, nz, r, g, b;
    for (size_t i = 0; i < numVertices; i++)
    {
        dataStream >> x >> y >> z >> nx >> ny >> nz >> r >> g >> b;
        m_vertexList.push_back(x); m_vertexList.push_back(y); m_vertexList.push_back(z);
        m_normalList.push_back(nx); m_normalList.push_back(ny); m_normalList.push_back(nz);
        m_colourList.push_back(r); m_colourList.push_back(g); m_colourList.push_back(b);
        m_uvList.push_back(0); m_uvList.push_back(0);
    }
    m_meshStore.setTargetMemory(Preferences::valueDouble("MeshStoreMemoryFraction"));
    m_meshStore.addMesh(resourceName.toStdString(), m_vertexList, m_normalList, m_colourList, m_uvList, m_lowerBound, m_upperBound);
    return 0;
}

void FacetedObject::Draw()
{
#if QT_VERSION >= 0x060000
        QOpenGLFunctions_3_3_Core *f = QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_3_3_Core>(QOpenGLContext::currentContext());
#else
    QOpenGLFunctions_3_3_Core *f = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
#endif

    if (m_VBOAllocated == false)
    {
        m_VBOAllocated = true;
        // order vertex data as x, y, z, xn, yn, zn, r, g, b, u, v
        size_t vertBufSize = m_vertexList.size() + m_normalList.size() + m_colourList.size()  + m_uvList.size();
        std::unique_ptr<GLfloat []> vertBuf = std::make_unique<GLfloat []>(vertBufSize);
        GLfloat *vertBufPtr = vertBuf.get();
        double *vertexListPtr = m_vertexList.data();
        double *normalListPtr = m_normalList.data();
        double *colourListPtr = m_colourList.data();
        double *uvListPtr = m_uvList.data();
        for (size_t i = 0; i < m_vertexList.size() / 3; i++)
        {
            *vertBufPtr++ = GLfloat(*vertexListPtr++);
            *vertBufPtr++ = GLfloat(*vertexListPtr++);
            *vertBufPtr++ = GLfloat(*vertexListPtr++);
            *vertBufPtr++ = GLfloat(*normalListPtr++);
            *vertBufPtr++ = GLfloat(*normalListPtr++);
            *vertBufPtr++ = GLfloat(*normalListPtr++);
            *vertBufPtr++ = GLfloat(*colourListPtr++);
            *vertBufPtr++ = GLfloat(*colourListPtr++);
            *vertBufPtr++ = GLfloat(*colourListPtr++);
            *vertBufPtr++ = GLfloat(*uvListPtr++);
            *vertBufPtr++ = GLfloat(*uvListPtr++);
        }

        // Setup our vertex buffer object.
        m_VBO.create();
        m_VBO.bind();
        m_VBO.allocate(vertBuf.get(), int(vertBufSize * sizeof(GLfloat)));
    }
    if (m_visible == false) return;

    // select the rendering program
    m_simulationWidget->facetedObjectShader()->bind();

    // select the vertex attribute bindings for the program.
    m_VBO.bind();
    int stride = 11 * sizeof(GLfloat);
    int offset = 0;
    m_simulationWidget->facetedObjectShader()->enableAttributeArray("vertex");
    m_simulationWidget->facetedObjectShader()->setAttributeBuffer("vertex", GL_FLOAT, offset, 3, stride);
    offset += 3 * sizeof(GLfloat);
    m_simulationWidget->facetedObjectShader()->enableAttributeArray("vertexNormal");
    m_simulationWidget->facetedObjectShader()->setAttributeBuffer("vertexNormal", GL_FLOAT, offset, 3, stride);
    offset += 3 * sizeof(GLfloat);
    m_simulationWidget->facetedObjectShader()->enableAttributeArray("vertexColour");
    m_simulationWidget->facetedObjectShader()->setAttributeBuffer("vertexColour", GL_FLOAT, offset, 3, stride);
    offset += 3 * sizeof(GLfloat);
    m_simulationWidget->facetedObjectShader()->enableAttributeArray("vertexUV");
    m_simulationWidget->facetedObjectShader()->setAttributeBuffer("vertexUV", GL_FLOAT, offset, 2, stride);

    m_VBO.release();

    // set the uniforms
    QMatrix4x4 model = this->model(); // thios recalculates the model matrix
    QMatrix4x4 modelView = m_simulationWidget->view() * model;
    m_simulationWidget->facetedObjectShader()->setUniformValue("mvMatrix", modelView);
    QMatrix4x4 modelViewProjection = m_simulationWidget->proj() * modelView;
    m_simulationWidget->facetedObjectShader()->setUniformValue("mvpMatrix", modelViewProjection);
    QMatrix3x3 normalMatrix = modelView.normalMatrix();
    m_simulationWidget->facetedObjectShader()->setUniformValue("normalMatrix", normalMatrix);

    GLfloat r = GLfloat(m_blendColour.redF());
    GLfloat g = GLfloat(m_blendColour.greenF());
    GLfloat b = GLfloat(m_blendColour.blueF());
    GLfloat alpha = GLfloat(m_blendColour.alphaF());
    GLfloat blendFraction = GLfloat(m_blendFraction);
    GLfloat ambientProportion = 0.2f; // new for per vertex colour
    GLfloat diffuseProportion = 0.8f;
    GLfloat specularProportion = 0.3f;
    GLfloat specularPower = 20.0f;
    QVector4D ambient(ambientProportion, ambientProportion, ambientProportion, alpha);
    QVector4D diffuse(diffuseProportion, diffuseProportion, diffuseProportion, alpha);
    QVector4D specular(specularProportion, specularProportion, specularProportion, alpha);
    QVector4D blendColour(r, g, b, alpha);
    m_simulationWidget->facetedObjectShader()->setUniformValue("ambient", ambient);
    m_simulationWidget->facetedObjectShader()->setUniformValue("diffuse", diffuse);
    m_simulationWidget->facetedObjectShader()->setUniformValue("specular", specular);
    m_simulationWidget->facetedObjectShader()->setUniformValue("shininess", specularPower);
    m_simulationWidget->facetedObjectShader()->setUniformValue("blendColour", blendColour);
    m_simulationWidget->facetedObjectShader()->setUniformValue("blendFraction", blendFraction);
    // decal is set to avoid z-fighting for decals (0 is normal, 1 and higher will be put in front)
    m_simulationWidget->facetedObjectShader()->setUniformValue("decal", GLfloat(m_decal));

    if (m_texture)
    {
        f->glActiveTexture(GL_TEXTURE0);
        m_simulationWidget->facetedObjectShader()->setUniformValue("textureSampler", 0);
        m_simulationWidget->facetedObjectShader()->setUniformValue("hasTexture", true);
        m_texture->bind();
        f->glDrawArrays(GL_TRIANGLES, 0, GLsizei(m_vertexList.size() / 3));
        m_texture->release();
    }
    else
    {
        m_simulationWidget->facetedObjectShader()->setUniformValue("hasTexture", false);
        f->glDrawArrays(GL_TRIANGLES, 0, GLsizei(m_vertexList.size() / 3));
    }

    m_simulationWidget->facetedObjectShader()->disableAttributeArray("vertex");
    m_simulationWidget->facetedObjectShader()->disableAttributeArray("vertexNormal");
    m_simulationWidget->facetedObjectShader()->disableAttributeArray("vertexColour");
    m_simulationWidget->facetedObjectShader()->disableAttributeArray("vertexUV");

    m_simulationWidget->facetedObjectShader()->release();
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
    for (i = 0; i < m_vertexList.size() / 9; i++)
    {
        theString << "    triangle {\n";
        for (j = 0; j < 3; j++)
        {
            prel[0] = m_vertexList[i * 9 + j * 3];
            prel[1] = m_vertexList[i * 9 + j * 3 + 1];
            prel[2] = m_vertexList[i * 9 + j * 3 + 2];
            prel[3] = 0;
            dMULTIPLY0_331(p, m_displayRotation, prel);
            result[0] = p[0] + m_displayPosition[0];
            result[1] = p[1] + m_displayPosition[1];
            result[2] = p[2] + m_displayPosition[2];

            theString << "      <" << result[0] << "," << result[1] << "," << result[2] << ">\n";
        }
        theString << "    }\n";
    }

    // now colour
    theString << "    pigment {\n";
    theString << "      color rgbf<" << m_blendColour.redF() << "," << m_blendColour.greenF() << "," <<
              m_blendColour.blueF() << "," << 1 - m_blendColour.alphaF() << ">\n";
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

const double *FacetedObject::GetDisplayPosition() const
{
    return m_displayPosition;
}

const double *FacetedObject::GetDisplayRotation() const
{
    return m_displayRotation;
}

const double *FacetedObject::GetDisplayScale() const
{
    return m_displayScale;
}

size_t FacetedObject::GetNumVertices() const
{
    return m_vertexList.size() / 3;
}

const double *FacetedObject::GetVertex(size_t i) const
{
    return &m_vertexList.at(3 * i);
}

const double *FacetedObject::GetNormal(size_t i) const
{
    return &m_normalList.at(3 * i);
}

const double *FacetedObject::GetVertexList() const
{
    return m_vertexList.data();
}

const double *FacetedObject::GetNormalList() const
{
    return m_normalList.data();
}

const double *FacetedObject::GetColourList() const
{
    return m_colourList.data();
}

size_t FacetedObject::GetNumTriangles() const
{
    return m_vertexList.size() / 9;
}

const double *FacetedObject::GetTriangle(size_t i) const
{
    return &m_vertexList.at(9 * i);
}

double *FacetedObject::upperBound()
{
    return m_upperBound;
}

SimulationWidget *FacetedObject::simulationWidget() const
{
    return m_simulationWidget;
}

void FacetedObject::setSimulationWidget(SimulationWidget *glWidget)
{
    m_simulationWidget = glWidget;
}


void FacetedObject::setBlendColour(const QColor &blendColour, double blendFraction)
{
    if (m_blendColour == blendColour && m_blendFraction == blendFraction) return;
    m_blendColour = blendColour;
    m_blendFraction = blendFraction;
}

double *FacetedObject::lowerBound()
{
    return m_lowerBound;
}

// Write a FacetedObject out as a OBJ
void FacetedObject::WriteOBJFile(std::ostringstream &out)
{
    size_t i, j;
    dVector3 prel, result;

    out.precision(7); // should be plenty

    if (m_useRelativeOBJ)
    {
        // write out the vertices, faces, groups and objects
        // this is the relative version - inefficient but allows concatenation of objects
        for (i = 0; i < m_vertexList.size() / 9; i++)
        {
            for (j = 0; j < 3; j++)
            {
                prel[0] = m_vertexList[i * 9 + j * 3];
                prel[1] = m_vertexList[i * 9 + j * 3 + 1];
                prel[2] = m_vertexList[i * 9 + j * 3 + 2];
                ApplyDisplayTransformation(*reinterpret_cast<pgd::Vector3 *>(prel), reinterpret_cast<pgd::Vector3 *>(result));
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
        for (i = 0; i < m_vertexList.size() / 9; i++)
        {
            for (j = 0; j < 3; j++)
            {
                prel[0] = m_vertexList[i * 9 + j * 3];
                prel[1] = m_vertexList[i * 9 + j * 3 + 1];
                prel[2] = m_vertexList[i * 9 + j * 3 + 2];
                ApplyDisplayTransformation(*reinterpret_cast<pgd::Vector3 *>(prel), reinterpret_cast<pgd::Vector3 *>(result));
                out << "v " << result[0] << " " << result[1] << " " << result[2] << "\n";
            }
        }

        for (i = 0; i < m_vertexList.size() / 9; i++)
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
        m_vertexOffset += m_vertexList.size() / 3;
    }
}

// utility to calculate a face normal
// this assumes anticlockwise winding
void FacetedObject::ComputeFaceNormal(const double *v1, const double *v2, const double *v3, double normal[3])
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
// note this must be used before the first Draw() call
void FacetedObject::Move(double x, double y, double z)
{
    if (x == 0.0 && y == 0.0 && z == 0.0) return;
    for (size_t i = 0; i < m_vertexList.size() / 3; i++)
    {
        m_vertexList[i * 3] += x;
        m_vertexList[i * 3 + 1] += y;
        m_vertexList[i * 3 + 2] += z;
    }
    m_lowerBound[0] += x;
    m_lowerBound[1] += y;
    m_lowerBound[2] += z;
    m_upperBound[0] += x;
    m_upperBound[1] += y;
    m_upperBound[2] += z;
}

// scale the object
// note this must be used before the first Draw() call
void FacetedObject::Scale(double x, double y, double z)
{
    if (x == 1.0 && y == 1.0 && z == 1.0) return;
    for (size_t i = 0; i < m_vertexList.size() / 3; i++)
    {
        m_vertexList[i * 3] *= x;
        m_vertexList[i * 3 + 1] *= y;
        m_vertexList[i * 3 + 2] *= z;
    }
    m_lowerBound[0] *= x;
    m_lowerBound[1] *= y;
    m_lowerBound[2] *= z;
    m_upperBound[0] *= x;
    m_upperBound[1] *= y;
    m_upperBound[2] *= z;
}

// rotate the object
// note this must be used before the first Draw() call
void FacetedObject::Rotate(double x, double y, double z, double angleDegrees)
{
    Q_ASSERT_X(x != 0 || y != 0 || z != 0, "Axis must be non-zero", "FacetedObject::Rotate");
    if (angleDegrees == 0) return;
    pgd::Quaternion q = pgd::MakeQFromAxisAngle(x, y, z, pgd::DegreesToRadians(angleDegrees));
    pgd::Vector3 v;
    for (size_t i = 0; i < m_vertexList.size() / 3; i++)
    {
        v = pgd::QVRotate(q, pgd::Vector3(m_vertexList[i * 3], m_vertexList[i * 3 + 1], m_vertexList[i * 3 + 2]));
        m_vertexList[i * 3] = v.x;
        m_vertexList[i * 3 + 1] = v.y;
        m_vertexList[i * 3 + 2] = v.z;
    }
    v = pgd::QVRotate(q, m_lowerBound);
    m_lowerBound[0] = v.x;
    m_lowerBound[1] = v.y;
    m_lowerBound[2] = v.z;
    v = pgd::QVRotate(q, m_upperBound);
    m_upperBound[0] = v.x;
    m_upperBound[1] = v.y;
    m_upperBound[2] = v.z;
}


// this routine triangulates the polygon and calls AddTriangle to do the actual data adding
// vertices are a packed list of floating point numbers
// x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4
void FacetedObject::AddPolygon(const double *vertices, size_t nSides, const double *normals, const double *UVs)
{
    if (UVs == nullptr)
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
                AddTriangle(triangle, triNormal, nullptr);
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
                AddTriangle(triangle, triNormal, nullptr);
            }
        }
    }
    else
    {
        if (normals == nullptr)
        {
            // calculate the normals
            double triNormal[9];
            double triUVs[6];
            ComputeFaceNormal(vertices, vertices + 3, vertices + 6, triNormal);
            for (size_t i = 3; i < 9; i++) triNormal[i] = triNormal[i % 3];
            // add faces as triangles
            double triangle[9];
            triangle[0] = vertices[0];
            triangle[1] = vertices[1];
            triangle[2] = vertices[2];
            triUVs[0] = UVs[0];
            triUVs[1] = UVs[1];
            for (size_t j = 2; j < nSides; j++)
            {
                triangle[3] = vertices[(j - 1) * 3];
                triangle[4] = vertices[(j - 1) * 3 + 1];
                triangle[5] = vertices[(j - 1) * 3 + 2];
                triangle[6] = vertices[(j * 3)];
                triangle[7] = vertices[(j * 3) + 1];
                triangle[8] = vertices[(j * 3) + 2];
                triUVs[2] = UVs[(j - 1) * 2];
                triUVs[3] = UVs[(j - 1) * 2 + 1];
                triUVs[4] = UVs[(j * 2)];
                triUVs[5] = UVs[(j * 2) + 1];
                AddTriangle(triangle, triNormal, triUVs);
            }
        }
        else
        {
            // add faces as triangles
            double triangle[9];
            double triNormal[9];
            double triUVs[6];
            triangle[0] = vertices[0];
            triangle[1] = vertices[1];
            triangle[2] = vertices[2];
            triNormal[0] = normals[0];
            triNormal[1] = normals[1];
            triNormal[2] = normals[2];
            triUVs[0] = UVs[0];
            triUVs[1] = UVs[1];
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
                triUVs[2] = UVs[(j - 1) * 2];
                triUVs[3] = UVs[(j - 1) * 2 + 1];
                triUVs[4] = UVs[(j * 2)];
                triUVs[5] = UVs[(j * 2) + 1];
                AddTriangle(triangle, triNormal, triUVs);
            }
        }
    }
}


// this is the only Add* routine that actually adds data to the faceted object
// except for AddFacetedObject with the useDirectAccess option
// it gets called by add polygon
// vertices is a packed list of floating point numbers
// x1, y1, z1, x2, y2, z2, x3, y3, z3
void FacetedObject::AddTriangle(const double *vertices, const double *normals, const double *UVs)
{
    Q_ASSERT_X(m_vertexList.capacity() - m_vertexList.size() >= 9, "FacetedObject::AddTriangle", "Warning: not enough triangle space reserved");
    pgd::Vector3 vertex;
    for (size_t i = 0; i < 3; i++)
    {
        vertex.x = vertices[i * 3 + 0];
        vertex.y = vertices[i * 3 + 1];
        vertex.z = vertices[i * 3 + 2];
        m_vertexList.push_back(vertex.x);
        m_vertexList.push_back(vertex.y);
        m_vertexList.push_back(vertex.z);
        if (vertex.x < m_lowerBound[0]) m_lowerBound[0] = vertex.x;
        if (vertex.y < m_lowerBound[1]) m_lowerBound[1] = vertex.y;
        if (vertex.z < m_lowerBound[2]) m_lowerBound[2] = vertex.z;
        if (vertex.x > m_upperBound[0]) m_upperBound[0] = vertex.x;
        if (vertex.y > m_upperBound[1]) m_upperBound[1] = vertex.y;
        if (vertex.z > m_upperBound[2]) m_upperBound[2] = vertex.z;
    }
    if (normals)
    {
        for (size_t i = 0; i < 9; i++) m_normalList.push_back(normals[i]);
    }
    else
    {
        // calculate the normals
        double normal[3];
        ComputeFaceNormal(vertices, vertices + 3, vertices + 6, normal);
        for (size_t i = 0; i < 9; i++) m_normalList.push_back(normal[i % 3]);
    }
    if (UVs)
    {
        for (size_t i = 0; i < 6; i++) m_uvList.push_back(UVs[i]);
    }
    else
    {
        for (size_t i = 0; i < 6; i++) m_uvList.push_back(0);
    }
    double colour[3] = {m_blendColour.redF(), m_blendColour.greenF(), m_blendColour.blueF() };
    for (size_t i = 0; i < 9; i++) m_colourList.push_back(colour[i % 3]);
}

// this routine triangulates the polygon and calls AddTriangle to do the actual data adding
// vertices are a packed list of floating point numbers
// x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4
void FacetedObject::AddPolygon(const float *floatVertices, size_t nSides, const float *floatNormals, const float *floatUVs)
{
    if (floatUVs == nullptr)
    {
        size_t nVertices = nSides * 3;
        std::vector<double> doubleVertices;
        doubleVertices.reserve(nVertices);
        for (size_t i = 0; i < nVertices; i++) doubleVertices.push_back(double(floatVertices[i]));
        if (floatNormals)
        {
            std::vector<double> normals;
            normals.reserve(nVertices);
            for (size_t i = 0; i < nVertices; i++) normals.push_back(double(floatNormals[i]));
            AddPolygon(doubleVertices.data(), nSides, normals.data(), nullptr);
        }
        else
        {
            AddPolygon(doubleVertices.data(), nSides, nullptr, nullptr);
        }
    }
    else
    {
        size_t nUVs = nSides * 2;
        std::vector<double> doubleUVs;
        doubleUVs.reserve(nUVs);
        for (size_t i = 0; i < nUVs; i++) doubleUVs.push_back(double(floatUVs[i]));
        size_t nVertices = nSides * 3;
        std::vector<double> doubleVertices;
        doubleVertices.reserve(nVertices);
        for (size_t i = 0; i < nVertices; i++) doubleVertices.push_back(double(floatVertices[i]));
        if (floatNormals)
        {
            std::vector<double> normals;
            normals.reserve(nVertices);
            for (size_t i = 0; i < nVertices; i++) normals.push_back(double(floatNormals[i]));
            AddPolygon(doubleVertices.data(), nSides, normals.data(), doubleUVs.data());
        }
        else
        {
            AddPolygon(doubleVertices.data(), nSides, nullptr, doubleUVs.data());
        }
    }
}


// this is just a convenience function to allow AddTraingles to be called with floats rather than doubles
// vertices is a packed list of floating point numbers
// x1, y1, z1, x2, y2, z2, x3, y3, z3
void FacetedObject::AddTriangle(const float *floatVertices, const float *floatNormals, const float *floatUVs)
{
    if (floatUVs == nullptr)
    {
        double vertices[9];
        for (int i = 0; i < 9; i++) vertices[i] = double(floatVertices[i]);
        if (floatNormals)
        {
            double normals[9];
            for (int i = 0; i < 9; i++) normals[i] = double(floatNormals[i]);
            AddTriangle(vertices, normals, nullptr);
        }
        else
        {
            AddTriangle(vertices, nullptr, nullptr);
        }
    }
    else
    {
        double UVs[6];
        for (int i = 0; i < 6; i++) UVs[i] = double(floatUVs[i]);
        double vertices[9];
        for (int i = 0; i < 9; i++) vertices[i] = double(floatVertices[i]);
        if (floatNormals)
        {
            double normals[9];
            for (int i = 0; i < 9; i++) normals[i] = double(floatNormals[i]);
            AddTriangle(vertices, normals, UVs);
        }
        else
        {
            AddTriangle(vertices, nullptr, UVs);
        }
    }
}

// this routine handles the memory allocation
// allocation is the number of vertices to store
void FacetedObject::AllocateMemory(size_t numTriangles)
{
//    qDebug() << "Allocated " << numTriangles << " triangles\n";
    m_vertexList.reserve(numTriangles * 9);
    m_normalList.reserve(numTriangles * 9);
    m_colourList.reserve(numTriangles * 9);
    m_uvList.reserve(numTriangles * 6);
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

    *numVertices = int((m_vertexList.size() / 3));
    *numTriIndexes = int((m_vertexList.size() / 3));

    *vertices = new double[(m_vertexList.size() / 3) * 3];
    *triIndexes = new dTriIndex[(m_vertexList.size() / 3)];

    for (i = 0; i < (m_vertexList.size() / 3); i++)
    {
        (*vertices)[i * 3] = m_vertexList[i * 3];
        (*vertices)[i * 3 + 1] = m_vertexList[i * 3 + 1];
        (*vertices)[i * 3 + 2] = m_vertexList[i * 3 + 2];
    }

    for (i = 0; i < (m_vertexList.size() / 3); i++)
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

    *numVertices = int((m_vertexList.size() / 3));
    *numTriIndexes = int((m_vertexList.size() / 3));

    *vertices = new float[(m_vertexList.size() / 3) * 3];
    *triIndexes = new dTriIndex[(m_vertexList.size() / 3)];

    for (i = 0; i < (m_vertexList.size() / 3); i++)
    {
        (*vertices)[i * 3] = float(m_vertexList[i * 3]);
        (*vertices)[i * 3 + 1] = float(m_vertexList[i * 3 + 1]);
        (*vertices)[i * 3 + 2] = float(m_vertexList[i * 3 + 2]);
    }

    for (i = 0; i < (m_vertexList.size() / 3); i++)
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

    unsigned int triangles = static_cast<unsigned int>((m_vertexList.size() / 3) / 3);

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
                v[j][0] = m_vertexList[i * 9 + j * 3];
                v[j][1] = m_vertexList[i * 9 + j * 3 + 1];
                v[j][2] = m_vertexList[i * 9 + j * 3 + 2];
            }
        }
        else
        {
            for (j = 0; j < 3; j++)
            {
                v[j][2] = m_vertexList[i * 9 + j * 3];
                v[j][1] = m_vertexList[i * 9 + j * 3 + 1];
                v[j][0] = m_vertexList[i * 9 + j * 3 + 2];
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
    size_t numTriangles = (m_vertexList.size() / 3) / 3;
    size_t i, j;
    for (i = 0; i < numTriangles; i++)
    {
        for (j = 0; j < 3; j++)
        {
            t = m_vertexList[i * 9 + 3 + j];
            m_vertexList[i * 9 + 3 + j] = m_vertexList[i * 9 + 6 + j];
            m_vertexList[i * 9 + 6 + j] = t;
        }
    }
    for (i = 0; i < m_normalList.size(); i++) m_normalList[i] = -m_normalList[i];
}

// add the faces from one faceted object to another
// useDirectAccess version does not use AddTriangle
// there is probably no good reason currently not to use useDirectAccess
void FacetedObject::AddFacetedObject(const FacetedObject *object, bool useDisplayRotation, bool useDirectAccess)
{
    if (useDirectAccess)
    {
        size_t offset = m_vertexList.size();
        m_vertexList.resize(offset + object->m_vertexList.size());
        m_normalList.resize(offset + object->m_normalList.size());
        m_colourList.resize(offset + object->m_colourList.size());
        size_t offsetUV = m_uvList.size();
        m_uvList.resize(offsetUV + object->m_uvList.size());
        if (useDisplayRotation)
        {
            for (size_t i = 0; i < object->m_vertexList.size(); i += 3)
            {
                ApplyDisplayTransformation(&object->m_vertexList[i], reinterpret_cast<pgd::Vector3 *>(&m_vertexList[i + offset]));
                ApplyDisplayRotation(&object->m_normalList[i], reinterpret_cast<pgd::Vector3 *>(&m_normalList[i + offset]));
            }
            for (size_t i = 0; i < object->m_colourList.size(); i++) m_colourList[i + offset] = object->m_colourList[i];
            for (size_t i = 0; i < object->m_uvList.size(); i++) m_uvList[i + offsetUV] = object->m_uvList[i];
            pgd::Vector3 lower, upper;
            ApplyDisplayTransformation(m_lowerBound, &lower);
            ApplyDisplayTransformation(m_upperBound, &upper);
            if (lower.x < m_lowerBound[0]) m_lowerBound[0] = lower.x;
            if (lower.y < m_lowerBound[1]) m_lowerBound[1] = lower.y;
            if (lower.z < m_lowerBound[2]) m_lowerBound[2] = lower.z;
            if (upper.x > m_upperBound[0]) m_upperBound[0] = upper.x;
            if (upper.y > m_upperBound[1]) m_upperBound[1] = upper.y;
            if (upper.z > m_upperBound[2]) m_upperBound[2] = upper.z;
        }
        else
        {
            for (size_t i = 0; i < object->m_vertexList.size(); i++)
            {
                m_vertexList[i + offset] = object->m_vertexList[i];
                m_normalList[i + offset] = object->m_normalList[i];
                m_colourList[i + offset] = object->m_colourList[i];
            }
            for (size_t i = 0; i < object->m_uvList.size(); i++) m_uvList[i + offsetUV] = object->m_uvList[i];
            if (object->m_lowerBound[0] < m_lowerBound[0]) m_lowerBound[0] = object->m_lowerBound[0];
            if (object->m_lowerBound[1] < m_lowerBound[1]) m_lowerBound[1] = object->m_lowerBound[1];
            if (object->m_lowerBound[2] < m_lowerBound[2]) m_lowerBound[2] = object->m_lowerBound[2];
            if (object->m_upperBound[0] > m_upperBound[0]) m_upperBound[0] = object->m_upperBound[0];
            if (object->m_upperBound[1] > m_upperBound[1]) m_upperBound[1] = object->m_upperBound[1];
            if (object->m_upperBound[2] > m_upperBound[2]) m_upperBound[2] = object->m_upperBound[2];
        }
    }
    else
    {
        size_t numTriangles = object->GetNumTriangles();
        const double *triangle;
        const double *p1;
        double *p2;
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
                    dMULTIPLY0_331(v1r, m_displayRotation, v1);
                    p2 = triangle2 + 3 * j;
                    p2[0] = v1r[0] + m_displayPosition[0];
                    p2[1] = v1r[1] + m_displayPosition[1];
                    p2[2] = v1r[2] + m_displayPosition[2];
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
}

// modified from
// Möller–Trumbore intersection algorithm
// From Wikipedia, the free encyclopedia
// The Möller–Trumbore ray-triangle intersection algorithm, named after its inventors Tomas Möller and Ben Trumbore,
// is a fast method for calculating the intersection of a ray and a triangle in three dimensions without needing
// precomputation of the plane equation of the plane containing the triangle.

bool FacetedObject::RayIntersectsTriangle(const pgd::Vector3 &rayOrigin,
                                          const pgd::Vector3 &rayVector,
                                          const pgd::Vector3 &vertex0,
                                          const pgd::Vector3 &vertex1,
                                          const pgd::Vector3 &vertex2,
                                          pgd::Vector3 *outIntersectionPoint)
{
    const double EPSILON = 1e-10;
    pgd::Vector3 edge1, edge2, h, s, q;
    double a,f,u,v;
    edge1 = vertex1 - vertex0;
    edge2 = vertex2 - vertex0;
    h = rayVector.Cross(edge2);
    a = edge1.Dot(h);
    if (a > -EPSILON && a < EPSILON)
        return false;    // This ray is parallel to this triangle.
    f = 1.0/a;
    s = rayOrigin - vertex0;
    u = f * s.Dot(h);
    if (u < 0.0 || u > 1.0)
        return false;
    q = s.Cross(edge1);
    v = f * rayVector.Dot(q);
    if (v < 0.0 || u + v > 1.0)
        return false;
    // At this stage we can compute t to find out where the intersection point is on the line.
    double t = f * edge2.Dot(q);
    if (t > EPSILON && t < 1/EPSILON) // ray intersection
    {
        *outIntersectionPoint = rayOrigin + rayVector * t;
        return true;
    }
    else // This means that there is a line intersection but not a ray intersection.
        return false;
}

// modified from
// Fast Ray-Box Intersection
// by Andrew Woo
// from "Graphics Gems", Academic Press, 1990

#define HITBOUNDINBOX_NUMDIM  3
bool FacetedObject::HitBoundingBox(const double minB[HITBOUNDINBOX_NUMDIM], const double maxB[HITBOUNDINBOX_NUMDIM],      /*box */
                                   const double rayOrigin[HITBOUNDINBOX_NUMDIM], const double rayVector[HITBOUNDINBOX_NUMDIM],     /*ray */
                                   double coord[HITBOUNDINBOX_NUMDIM])                                                    /* hit point */
{
    bool inside = true;
    char quadrant[HITBOUNDINBOX_NUMDIM];
    int i;
    int whichPlane;
    double maxT[HITBOUNDINBOX_NUMDIM];
    double candidatePlane[HITBOUNDINBOX_NUMDIM];
    const char RIGHT = 0;
    const char LEFT = 1;
    const char MIDDLE = 2;

    /* Find candidate planes; this loop can be avoided if
    rays cast all from the eye (assume perpsective view) */
    for (i = 0; i < HITBOUNDINBOX_NUMDIM; i++)
    {
        if (rayOrigin[i] < minB[i])
        {
            quadrant[i] = LEFT;
            candidatePlane[i] = minB[i];
            inside = false;
        }
        else if (rayOrigin[i] > maxB[i])
        {
            quadrant[i] = RIGHT;
            candidatePlane[i] = maxB[i];
            inside = false;
        }
        else
        {
            quadrant[i] = MIDDLE;
        }
    }

    /* Ray origin inside bounding box */
    if (inside)
    {
        std::copy(rayOrigin, rayOrigin + HITBOUNDINBOX_NUMDIM, coord);
        return (true);
    }


    /* Calculate T distances to candidate planes */
    for (i = 0; i < HITBOUNDINBOX_NUMDIM; i++)
    {
        if (quadrant[i] != MIDDLE && rayVector[i] !=0.)
        {
            maxT[i] = (candidatePlane[i] - rayOrigin[i]) / rayVector[i];
        }
        else
        {
            maxT[i] = -1.;
        }
    }

    /* Get largest of the maxT's for final choice of intersection */
    whichPlane = 0;
    for (i = 1; i < HITBOUNDINBOX_NUMDIM; i++)
    {
        if (maxT[whichPlane] < maxT[i]) whichPlane = i;
    }

    /* Check final candidate actually inside box */
    if (maxT[whichPlane] < 0.) return (false);
    for (i = 0; i < HITBOUNDINBOX_NUMDIM; i++)
    {
        if (whichPlane != i)
        {
            coord[i] = rayOrigin[i] + maxT[whichPlane] * rayVector[i];
            if (coord[i] < minB[i] || coord[i] > maxB[i]) return (false);
        }
        else
        {
            coord[i] = candidatePlane[i];
        }
    }

    return (true);              /* ray hits box */
}

// this routine works in model coordinates and rayVector must be unit length
int FacetedObject::FindIntersection(const pgd::Vector3 &rayOrigin, const pgd::Vector3 &rayVector, std::vector<pgd::Vector3> *intersectionCoordList, std::vector<size_t> *intersectionIndexList) const
{
    if (!m_visible || !m_vertexList.size()) return 0;

    // first check bounding box
    double coord[3];
    bool bbHit = HitBoundingBox(m_lowerBound, m_upperBound, rayOrigin.constData(), rayVector.constData(), coord);
    if (!bbHit) return 0;

    // now loop through the triangles
    bool triHit;
    int hitCount = 0;
    pgd::Vector3 outIntersectionPoint;
    for (size_t i = 0; i < m_vertexList.size(); i += 9)
    {
#ifdef PRECHECK_BOUNDINGBOX
        double lowerBound[3] = {std::min({mVertexList[i], mVertexList[i + 3], mVertexList[i + 6]}), std::min({mVertexList[i + 1], mVertexList[i + 4], mVertexList[i + 7]}), std::min({mVertexList[i + 2], mVertexList[i + 5], mVertexList[i + 8]})};
        double upperBound[3] = {std::max({mVertexList[i], mVertexList[i + 3], mVertexList[i + 6]}), std::max({mVertexList[i + 1], mVertexList[i + 4], mVertexList[i + 7]}), std::max({mVertexList[i + 2], mVertexList[i + 5], mVertexList[i + 8]})};
        bbHit = HitBoundingBox(lowerBound, upperBound, rayOrigin.constData(), rayVector.constData(), coord);
        if (bbHit)
        {
            triHit = RayIntersectsTriangle(rayOrigin, rayVectorNorm, &mVertexList[i], &mVertexList[i + 3], &mVertexList[i + 6], &outIntersectionPoint);
            if (triHit)
            {
                hitCount++;;
                if (intersectionCoordList) intersectionCoordList->push_back(outIntersectionPoint);
                if (intersectionIndexList) intersectionIndexList->push_back(i);
            }
        }
#else
        triHit = RayIntersectsTriangle(rayOrigin, rayVector, &m_vertexList[i], &m_vertexList[i + 3], &m_vertexList[i + 6], &outIntersectionPoint);
        if (triHit)
        {
            hitCount++;
            if (intersectionCoordList) intersectionCoordList->push_back(outIntersectionPoint);
            if (intersectionIndexList) intersectionIndexList->push_back(i);
        }
#endif

    }
    return hitCount;
}

void FacetedObject::ApplyDisplayTransformation(const pgd::Vector3 inVec, pgd::Vector3 *outVec)
{
    dVector3 v1r;
    pgd::Vector3 scaled(inVec.x * m_displayScale[0], inVec.y * m_displayScale[1], inVec.z * m_displayScale[2]);
    dMULTIPLY0_331(v1r, m_displayRotation, scaled.constData());
    *outVec = *reinterpret_cast<pgd::Vector3 *>(v1r) + *reinterpret_cast<pgd::Vector3 *>(m_displayPosition);
}

void FacetedObject::ApplyDisplayRotation(const pgd::Vector3 inVec, pgd::Vector3 *outVec)
{
    dMULTIPLY0_331(outVec->data(), m_displayRotation, inVec.constData());
}

void FacetedObject::SetDisplayPosition(double x, double y, double z)
{
    m_displayPosition[0] = x;
    m_displayPosition[1] = y;
    m_displayPosition[2] = z;
    m_modelValid = false;
}

void FacetedObject::SetDisplayScale(double x, double y, double z)
{
    m_displayScale[0] = x;
    m_displayScale[1] = y;
    m_displayScale[2] = z;
    m_modelValid = false;
}

void FacetedObject::SetDisplayRotation(const dMatrix3 R)
{
    std::copy_n(R, dM3E__MAX, m_displayRotation);
    m_modelValid = false;
}

// dQuaternion q [ w, x, y, z ], where w is the real part and (x, y, z) form the vector part.
void FacetedObject::SetDisplayRotationFromQuaternion(const dQuaternion q)
{
    dQtoR(q, m_displayRotation);
    m_modelValid = false;
}

void FacetedObject::ClearMeshStore()
{
    m_meshStore.clear();
}

bool FacetedObject::visible() const
{
    return m_visible;
}

void FacetedObject::setVisible(bool visible)
{
    m_visible = visible;
}

const QMatrix4x4 &FacetedObject::model()
{
    if (!m_modelValid)
    {
        m_modelValid = true;
        QMatrix4x4 translationRotation(
                    static_cast<float>(m_displayRotation[0]), static_cast<float>(m_displayRotation[1]), static_cast<float>(m_displayRotation[2]),  static_cast<float>(m_displayPosition[0]),
                    static_cast<float>(m_displayRotation[4]), static_cast<float>(m_displayRotation[5]), static_cast<float>(m_displayRotation[6]),  static_cast<float>(m_displayPosition[1]),
                    static_cast<float>(m_displayRotation[8]), static_cast<float>(m_displayRotation[9]), static_cast<float>(m_displayRotation[10]), static_cast<float>(m_displayPosition[2]),
                    0,                                        0,                                        0,                                         1);
        QMatrix4x4 scale(
                    static_cast<float>(m_displayScale[0]),  0,                                      0,                                      0,
                    0,                                      static_cast<float>(m_displayScale[1]),  0,                                      0,
                    0,                                      0,                                      static_cast<float>(m_displayScale[2]),  0,
                    0,                                      0,                                      0,                                      1);
        // ModelMatrix = Translation * Rotation * Scale
        m_model = translationRotation * scale;
    }
    return m_model;
}

void FacetedObject::setModel(const QMatrix4x4 &model)
{
    m_model = model;
}

QOpenGLTexture *FacetedObject::texture() const
{
    return m_texture.get();
}

void FacetedObject::setTexture(std::unique_ptr<QOpenGLTexture> &&texture)
{
    m_texture = std::move(texture);
}

double FacetedObject::decal() const
{
    return m_decal;
}

void FacetedObject::setDecal(double decal)
{
    m_decal = decal;
}

std::string FacetedObject::filename() const
{
    return m_filename;
}

QColor FacetedObject::blendColour() const
{
    return m_blendColour;
}

double FacetedObject::blendFraction() const
{
    return m_blendFraction;
}

