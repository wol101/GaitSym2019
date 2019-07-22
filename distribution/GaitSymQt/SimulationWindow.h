/*
 *  SimulationWindow.h
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 08/10/2018.
 *  Copyright 2018 Bill Sellers. All rights reserved.
 *
 */

#ifndef SIMULATIONWINDOW_H
#define SIMULATIONWINDOW_H

// #include <Qt3DWindow>
#include <QMatrix4x4>
#include <QVector3D>
#include <QScreenRayCaster>
#include <QTransform>
#include <QLayer>
#include <QMouseEvent>
#include <QNoDraw>

#include <map>

#include "Qt3DWindowCustom.h"

class Simulation;
class FacetedObject;
class Trackball;
class AVIWriter;
class DrawBody;
class DrawJoint;
class DrawMuscle;
class DrawMarker;
class DrawFluidSac;
class DrawGeom;
class MainWindow;
class FacetedSphere;

namespace Qt3DRender {
class QLayer;
}
class SceneEffect;

class SimulationWindow : public Qt3DWindowCustom
{
    Q_OBJECT
public:
    SimulationWindow(QScreen *screen = nullptr);
    virtual ~SimulationWindow() Q_DECL_OVERRIDE;

    void initialiseScene();
//    void clearRootEntity();

    void updateCamera();
    void updateModel();

    Simulation *simulation() const;
    void setSimulation(Simulation *simulation);

    bool wireFrame() const;
    void setWireFrame(bool wireFrame);

    bool boundingBox() const;
    void setBoundingBox(bool boundingBox);

    bool orthographicProjection() const;
    void setOrthographicProjection(bool orthographicProjection);

    float cameraDistance() const;
    void setCameraDistance(float cameraDistance);

    float FOV() const;
    void setFOV(float FOV);

    float cameraVecX() const;
    void setCameraVecX(float cameraVecX);

    float cameraVecY() const;
    void setCameraVecY(float cameraVecY);

    float cameraVecZ() const;
    void setCameraVecZ(float cameraVecZ);

    float COIx() const;
    void setCOIx(float COIx);

    float COIy() const;
    void setCOIy(float COIy);

    float COIz() const;
    void setCOIz(float COIz);

    float frontClip() const;
    void setFrontClip(float frontClip);

    float backClip() const;
    void setBackClip(float backClip);

    float upX() const;
    void setUpX(float upX);

    float upY() const;
    void setUpY(float upY);

    float upZ() const;
    void setUpZ(float upZ);

    float cursorRadius() const;
    void setCursorRadius(float cursorRadius);

    float cursor3DNudge() const;
    void setCursor3DNudge(float cursor3DNudge);

    QVector3D cursor3DPosition() const;
    void setCursor3DPosition(const QVector3D &cursor3DPosition);

    QColor cursorColour() const;
    void setCursorColour(const QColor &cursorColour);

    bool normals() const;
    void setNormals(bool normals);

    bool halfTransparency() const;
    void setHalfTransparency(bool halfTransparency);

    int WriteStillFrame(const QString &filename);
    int WriteMovieFrame();
    int WriteCADFrame(const QString &pathname);
    int StartAVISave(const QString &fileName);
    int StopAVISave();

    AVIWriter *aviWriter() const;
    void setAviWriter(AVIWriter *aviWriter);

    int aviQuality() const;
    void setAviQuality(int aviQuality);

    QColor backgroundColour() const;
    void setBackgroundColour(const QColor &backgroundColour);

    Qt3DCore::QEntity *rootEntity() const;

    int bodyMeshNumber() const;
    void setBodyMeshNumber(int bodyMeshNumber);

    float axesScale() const;
    void setAxesScale(float axesScale);

    const Qt3DRender::QAbstractRayCaster::Hits &getHits() const;
    int getClosestHitIndex() const;

    bool getUseDeferredRenderer() const;
    void setUseDeferredRenderer(bool useDeferredRenderer);

    MainWindow *getMainWindow() const;
    void setMainWindow(MainWindow *mainWindow);

public slots:
    void SetCameraVec(float x, float y, float z);
    void SetCameraVec(double x, double y, double z);
    void rayCasterHitsChanged(const Qt3DRender::QAbstractRayCaster::Hits &hits);
    void menuRequest(const QPoint &pos);

signals:
    void EmitStatusString(const QString &s, int logLevel);
    void EmitCOI(float x, float y, float z);
    void EmitFoV(float v);
    void EmitCreateMarkerRequest();
    void EmitEditMarkerRequest(const QString &s);
    void EmitMoveMarkerRequest(const QString &s, const QVector3D &p);
    void EmitEditBodyRequest(const QString &s);
    void EmitEditGeomRequest(const QString &s);
    void EmitEditJointRequest(const QString &s);
    void EmitEditMuscleRequest(const QString &s);

protected:
    virtual void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    virtual void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    virtual void resizeEvent ( QResizeEvent *event ) Q_DECL_OVERRIDE;

private:
    enum EntityType {NotRecognised, DrawBodyType, DrawFluidSacType, DrawGeomType, DrawJointType, DrawMarkerType, DrawMuscleType};
    Qt3DCore::QEntity *getParentDrawType(Qt3DCore::QEntity *entity, EntityType *entityType);
    int closestHit(const Qt3DRender::QAbstractRayCaster::Hits &hits);
    bool isEntityVisible(Qt3DCore::QNode *node);
    void deleteChildrenRecursively(Qt3DCore::QNodeVector vector);

    Simulation *m_simulation = nullptr;
    MainWindow *m_mainWindow = nullptr;

    bool m_wireFrame = false;
    bool m_boundingBox = false;
    bool m_boundingBoxBuffers = false;
    bool m_normals = false;
    bool m_halfTransparency = false;

    bool m_orthographicProjection = true;
    float m_cameraDistance = 50;
    float m_FOV = 5;
    float m_cameraVecX = 0, m_cameraVecY = -1, m_cameraVecZ = 0;
    float m_COIx = 0, m_COIy = 0, m_COIz = 0;
    float m_frontClip = 1;
    float m_backClip = 1000;
    float m_upX = 0, m_upY = 0, m_upZ = 1;

    QMatrix4x4 m_projectionMatrix;
    QMatrix4x4 m_viewMatrix;

    Trackball *m_trackball = nullptr;
    bool m_trackballFlag = false;
    QVector3D m_trackballStartCameraVec;
    QVector3D m_trackballStartUp;

    bool m_panFlag = false;
    QMatrix4x4 m_projectPanMatrix;
    QMatrix4x4 m_unprojectPanMatrix;
    QVector3D m_panStartPoint;
    QVector3D m_panStartCOI;
    QVector3D m_panStartScreenPoint;

    float m_zoomDistance = 0;
    float m_zoomStartFOV = 0;

    Qt3DCore::QEntity *m_rootEntity = nullptr;
    Qt3DCore::QEntity *m_staticEntities = nullptr;
    Qt3DCore::QEntity *m_dynamicEntities = nullptr;
    Qt3DCore::QTransform *m_cursor3DTransform = nullptr;
    FacetedSphere *m_cursor3D;
    float m_cursorRadius = 0.1f;
    float m_cursor3DNudge = 0.1f;
    QColor m_cursorColour;
    QColor m_backgroundColour;
    Qt3DRender::QScreenRayCaster *m_screenRayCaster = nullptr;
    QMouseEvent m_mouseClickEvent;
    Qt3DRender::QAbstractRayCaster::Hits m_hits;
    int m_closestHitIndex = 0;
    Qt3DRender::QLayer *m_pickableLayer;
    float m_axesScale = 1;


    AVIWriter *m_aviWriter = nullptr;
    int m_aviQuality = 80;
    unsigned int m_fps = 25;

    std::map<std::string, DrawBody *> m_drawBodyMap;
    std::map<std::string, DrawJoint *> m_drawJointMap;
    std::map<std::string, DrawGeom *> m_drawGeomMap;
    std::map<std::string, DrawMuscle *> m_drawMuscleMap;
    std::map<std::string, DrawFluidSac *> m_drawFluidSacMap;
    std::map<std::string, DrawMarker *> m_drawMarkerMap;
    int m_bodyMeshNumber = 1;

    Qt3DRender::QLayer *m_sceneLayer = nullptr;
    SceneEffect *m_sceneEffect = nullptr;
    bool m_useDeferredRenderer = false;
    bool m_moveMarkerMode = false;
    std::string m_moveMarkerName;
};

#endif // SIMULATIONWINDOW_H
