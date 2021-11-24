/*
 *  PlaneGeom.cpp
 *  GaitSymODE
 *
 *  Created by Bill Sellers on 13/09/2005.
 *  Copyright 2005 Bill Sellers. All rights reserved.
 *
 */

#include "PlaneGeom.h"
#include "Simulation.h"
#include "Marker.h"
#include "GSUtil.h"
#include "Body.h"


#include "ode/ode.h"

#include <string>
#include <algorithm>
#include <limits>

using namespace std::string_literals;

// The plane equation is:
// a * x + b * y + c * z = d
// The plane's normal vector is (a, b, c), and it must have length 1.

// Note: planes are non placeable so do not try and place them!

PlaneGeom::PlaneGeom(dSpaceID space, double a, double b, double c, double d)
{
    // create the geom
    double length = std::sqrt(a * a + b * b + c * c);
    if (length < std::numeric_limits<double>::min()) // standard fixup
    {
        a = 0;
        b = 0;
        c = 1;
        d = 0;
    }
    else
    {
        a = a / length;
        b = b / length;
        c = c / length;
    }
    setGeomID(dCreatePlane(space, a, b, c, d));
    dGeomSetData(GetGeomID(), this);

#ifdef USE_QT_IRRLICHT
    m_floorSceneNode = 0;

    //glGenTextures(1, &m_textureID);
    m_texture = 0;
    m_trackDepth = 0;
    m_lastDisplayTime = -1;
    m_nx = 0;
    m_ny = 0;
    Colour mappedColour;
    for (int i = 0; i < 256; i++)
    {
        GLUtils::SetColourFromMap(float(i) / 255.0, JetColourMap, &mappedColour);
        m_colourMap[i][0] = int(0.5 + mappedColour.r * 255.0); // red
        m_colourMap[i][1] = int(0.5 + mappedColour.g * 255.0); // green
        m_colourMap[i][2] = int(0.5 + mappedColour.b * 255.0); // blue
        m_colourMap[i][3] = int(0.5 + mappedColour.alpha * 255.0); // alpha
    }

    m_planeOrigin = pgd::Vector3(a * d, b * d, c * d); // this is in world coordinates
    // now find a rotation that aligns the world Z axis with the plane normal
    m_planeOrientation = FindRotation(pgd::Vector3(0, 0, 1), pgd::Vector3(a, b, c));
    m_checkerboardLow = 0;
    m_checkerboardHigh = 0;
    m_trackDrawThreshold = 0;

    m_floorTexture = ":/images/Floor Textures/floor_checker_2.png";
#endif

}

PlaneGeom::~PlaneGeom()
{
#ifdef USE_QT_IRRLICHT
    //glDeleteTextures(1, &m_textureID);
    if (m_texture) delete m_texture;
    if (m_trackDepth) delete m_trackDepth;
#endif
}

std::string *PlaneGeom::createFromAttributes()
{
    if (Geom::createFromAttributes()) return lastErrorPtr();
    std::string buf;

    if (geomMarker()->GetBody())
    {
        setLastError("GEOM ID=\""s + name() +"\" PlaneGeom must be attached to the World"s);
        return lastErrorPtr();
    }

    // because planes are non-placeable we need to calculate the plane equation directly from the marker
    //
    // we want the plane in its cartesian form a*x+b*y+c*z = d
    // the marker gives a point on the plane, 2 vectors in the plane (x & y axes)
    // and the plane normal (z axes)
    // the normal is (a,b,c)
    // d is the dot product of the normal at the point on the plane

    pgd::Vector3 normal = geomMarker()->GetWorldAxis(Marker::Axis::Z);
    pgd::Vector3 point = geomMarker()->GetWorldPosition();
    double a = normal.x;
    double b = normal.y;
    double c = normal.z;
    double d = normal.Dot(point);
    dGeomPlaneSetParams(GetGeomID(), a, b, c, d);
    return nullptr;
}

void PlaneGeom::appendToAttributes()
{
    Geom::appendToAttributes();
    std::string buf;

    setAttribute("Type"s, "Plane"s);
    // this is for information only
//    dVector4 result;
//    dGeomPlaneGetParams(GetGeomID(), result);
//    setAttribute("A"s, *GSUtil::ToString(result[0], &buf));
//    setAttribute("B"s, *GSUtil::ToString(result[1], &buf));
//    setAttribute("C"s, *GSUtil::ToString(result[2], &buf));
//    setAttribute("D"s, *GSUtil::ToString(result[3], &buf));

    return;
}

#ifdef USE_QT_IRRLICHT
// this routine sets the limits and resolution of the coloured patch used for the footprint
void PlaneGeom::SetTrackPatch(double trackPatchStartX, double trackPatchStartY, double trackPatchEndX, double trackPatchEndY, double trackPatchResolutionX, double trackPatchResolutionY)
{
    m_trackPatchStartX = trackPatchStartX;
    m_trackPatchEndX = trackPatchEndX;
    m_trackPatchResolutionX = trackPatchResolutionX;
    m_trackPatchStartY = trackPatchStartY;
    m_trackPatchEndY = trackPatchEndY;
    m_trackPatchResolutionY = trackPatchResolutionY;

    m_nx = (int)(0.00001 + (m_trackPatchEndX - m_trackPatchStartX) / m_trackPatchResolutionX);
    m_ny = (int)(0.00001 + (m_trackPatchEndY - m_trackPatchStartY) / m_trackPatchResolutionY);
    int textureSize = m_nx * m_ny * 4; // RGBA format
    if (textureSize)
    {
        m_texture = new unsigned char[textureSize];
        std::fill_n(m_texture, textureSize, 0);
        m_trackDepth = new double[textureSize];
        if (m_checkerboardHigh == m_checkerboardLow)
        {
            std::fill_n(m_trackDepth, textureSize, m_checkerboardLow);
        }
        else
        {
            double *trackDepthPtr = m_trackDepth;
            for (int iy = 0; iy < m_ny; iy++)
            {
                for (int ix = 0; ix < m_nx; ix++)
                {
                    if (iy % 2)
                    {
                        if (ix %2) *trackDepthPtr++ = m_checkerboardHigh;
                        else *trackDepthPtr++ = m_checkerboardLow;
                    }
                    else
                    {
                        if (ix %2) *trackDepthPtr++ = m_checkerboardLow;
                        else *trackDepthPtr++ = m_checkerboardHigh;
                    }
                }
            }
        }
    }
}

// this routine adds a impulse to the plane impulse store to emulate the appearance of footprints
//.the values are in world coordinates
void PlaneGeom::AddImpulse(double x, double y, double z, double fx, double fy, double fz, double time)
{
    if (m_trackDepth == 0) return;
    pgd::Vector3 offsetVec =  pgd::Vector3(x, y, z) - m_planeOrigin;
    pgd::Vector3 posOnPlane = QVRotate(m_planeOrientation, offsetVec);
    pgd::Vector3 force = QVRotate(m_planeOrientation, pgd::Vector3(fx, fy, fz));
    pgd::Vector3 impulse = force * time;
    int ix = (int)((posOnPlane.x - m_trackPatchStartX) / m_trackPatchResolutionX);
    int iy = (int)((posOnPlane.y - m_trackPatchStartY) / m_trackPatchResolutionY);
    if (ix >= 0 && x < m_nx && iy >= 0 && y < m_ny)
    {
        double *ptr = m_trackDepth + iy * m_nx + ix;
        *ptr += impulse.z;
        // std::cerr << "ix = " << ix << " iy = " << iy << " Impulse = " << *ptr << "\n";
    }
}


void PlaneGeom::Draw(SimulationWindow *window)
{
    if (m_floorSceneNode == 0)
    {
        // create meshes for the axes
        irr::scene::IMesh *mesh;
        const irr::scene::IGeometryCreator *geometryCreator = window->sceneManager()->getGeometryCreator();

        // load the floor texture - more complicated than it should be because we want to use resources
        // if we used files then this single line would work
        // irr::video::ITexture *floorTexture = window->videoDriver()->getTexture("/Users/wis/Unix/cvs/GaitSym2016/GaitSymQt/images/Floor Textures/test.png");
#ifdef _IRR_COMPILE_WITH_PNG_LOADER_ // this version uses the irrlicht PNG loader
        QFile file(m_floorTexture);
        bool ok = file.open(QIODevice::ReadOnly);
        Q_ASSERT(ok);
        QByteArray textureData = file.readAll();
        irr::io::IFileSystem *fs = window->irrlichtDevice()->getFileSystem();
        irr::io::IReadFile* readFile = fs->createMemoryReadFile(textureData.data(), textureData.size(), QFileInfo(m_floorTexture).fileName().toUtf8().constData(), false);
        irr::video::ITexture *floorTexture = window->videoDriver()->getTexture(readFile);
        readFile->drop();
#else // this version uses the Qt PNG loader and copies the memory to a locked texture
        QImage qImage;
        qImage.load(m_floorTexture);
        if (qImage.format() != QImage::Format_ARGB32)
            qImage = qImage.convertToFormat(QImage::Format_ARGB32);
        irr::video::ITexture *floorTexture = window->videoDriver()->addTexture(irr::core::dimension2d<irr::u32>(qImage.width(), qImage.height()),
                                                                               QFileInfo(m_floorTexture).fileName().toUtf8().constData(), irr::video::ECF_A8R8G8B8);
        irr::u8 *texturePtr = static_cast<irr::u8 *>(floorTexture->lock(irr::video::ETLM_WRITE_ONLY, 0));
        memcpy(texturePtr, qImage.bits(), qImage.width() * qImage.height() * 4);
        floorTexture->unlock();
#endif

        irr::u32 xRange = 100, yRange = 100;
        irr::core::dimension2d<irr::f32> tileSize(2.f, 2.f); // for a 2x2 checkerboard
        irr::core::dimension2d<irr::u32> tileCount(xRange, yRange);
        irr::video::SMaterial material;
        material.setTexture(0, floorTexture);
        irr::core::dimension2df textureRepeatCount(xRange, yRange);
        mesh = geometryCreator->createPlaneMesh(tileSize, tileCount, &material, textureRepeatCount);
        // the plane is Y = 0, and we want Z = 0
        m_floorSceneNode = window->sceneManager()->addEmptySceneNode();
        irr::scene::IMeshSceneNode *meshSceneNode = window->sceneManager()->addMeshSceneNode(mesh, m_floorSceneNode);
        meshSceneNode->setMaterialFlag(irr::video::EMF_BACK_FACE_CULLING, false);
        meshSceneNode->setMaterialFlag(irr::video::EMF_FRONT_FACE_CULLING, true);
        meshSceneNode->setMaterialFlag(irr::video::EMF_LIGHTING, false); //this might be better false to give a nice bright floor
        meshSceneNode->setAutomaticCulling(irr::scene::EAC_OFF); // no object based culling wanted
        meshSceneNode->setRotation(irr::core::vector3df(90, 0, 0));

        // meshSceneNode->setDebugDataVisible(irr::scene::EDS_MESH_WIRE_OVERLAY | irr::scene::EDS_NORMALS);
    }

    irr::core::quaternion q(m_planeOrientation.v.x, m_planeOrientation.v.y, m_planeOrientation.v.z, m_planeOrientation.n);
    irr::core::vector3df euler;
    q.toEuler(euler);
    m_floorSceneNode->setRotation(euler);
    m_floorSceneNode->setPosition(irr::core::vector3df(m_planeOrigin.x, m_planeOrigin.y, m_planeOrigin.z));
    m_floorSceneNode->setVisible(m_Visible);

    /* old style opengl
    if (m_trackDepth == 0 || m_texture == 0) return;

    if (m_lastDisplayTime != simulation()->GetTime())
    {
        m_lastDisplayTime = simulation()->GetTime();
        glBindTexture(GL_TEXTURE_2D, m_textureID);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        // fill the texture values from the track depth map
        unsigned char *texturePtr = m_texture;
        int ix, iy;
        double *trackDepthPtr = m_trackDepth;
        double v;
        int idx;
        int trackDrawThresholdIdx = int(255.99999 * (m_trackDrawThreshold - m_lowRange) / (m_highRange - m_lowRange));
        for (iy = 0; iy < m_ny; iy++)
        {
            for (ix = 0; ix < m_nx; ix++)
            {
                v = (*trackDepthPtr++ - m_lowRange) / (m_highRange - m_lowRange);
                idx = (int)(255.99999 * v);
                if (idx < 0) idx = 0;
                else if (idx > 255) idx = 255;
                *texturePtr++ = m_colourMap[idx][0];
                *texturePtr++ = m_colourMap[idx][1];
                *texturePtr++ = m_colourMap[idx][2];
                if (idx <= trackDrawThresholdIdx) *texturePtr++ = m_colourMap[idx][3];
                else *texturePtr++ = 0;
            }
        }

        glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, m_nx, m_ny, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_texture); // glTexSubmImage2D may be quicker

    }

    GLfloat xmin = m_trackPatchStartX;
    GLfloat ymin = m_trackPatchStartY;
    GLfloat xmax = m_trackPatchEndX;
    GLfloat ymax = m_trackPatchEndY;

    glPushMatrix();

    glTranslatef(m_planeOrigin.x, m_planeOrigin.y, m_planeOrigin.z);
    glRotatef(pgd::RadiansToDegrees(2*acos(m_planeOrientation.n)), m_planeOrientation.v.x, m_planeOrientation.v.y, m_planeOrientation.v.z);

    glEnable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glBegin (GL_QUADS);
    glTexCoord2f (0.0, 0.0);
    glVertex3f (xmin, ymin, 0.0);
    glTexCoord2f (1.0, 0.0);
    glVertex3f (xmax, ymin, 0.0);
    glTexCoord2f (1.0, 1.0);
    glVertex3f (xmax, ymax, 0.0);
    glTexCoord2f (0.0, 1.0);
    glVertex3f (xmin, ymax, 0.0);
    glEnd ();

    glEnable(GL_CULL_FACE);
    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
    */
}
#endif
