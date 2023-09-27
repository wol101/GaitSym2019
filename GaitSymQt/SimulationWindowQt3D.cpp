/*
 *  SimulationWindowQt3D.cpp
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 08/10/2018.
 *  Copyright 2018 Bill Sellers. All rights reserved.
 *
 */

#ifdef USE_QT3D

#include "SimulationWindowQt3D.h"
#include "Simulation.h"
#include "Body.h"
#include "Joint.h"
#include "Geom.h"
#include "Muscle.h"
#include "Marker.h"
#include "FluidSac.h"
#include "FacetedObject.h"
#include "TrackBall.h"
#include "AVIWriter.h"
#include "DrawBody.h"
#include "DrawJoint.h"
#include "DrawGeom.h"
#include "DrawMuscle.h"
#include "DrawMarker.h"
#include "DrawFluidSac.h"
#include "FacetedSphere.h"
#include "Preferences.h"
#include "MainWindow.h"
#include "FacetedAxes.h"
#include "PGDMath.h"

#include <QForwardRenderer>
#include <QApplication>
#include <QClipboard>
#include <QWheelEvent>
#include <QElapsedTimer>
#include <QDir>
#include <QMessageBox>
#include <QDirIterator>
#include <QDebug>
#include <QBuffer>
#include <QtGlobal>
#include <QInputAspect>
#include <QFirstPersonCameraController>
#include <QOrbitCameraController>
#include <QPointLight>
#include <QDirectionalLight>
#include <QTransform>
#include <QScreenRayCaster>
#include <QCamera>
#include <QPickingSettings>
#include <QRenderSettings>
#include <QLayer>
#include <QObjectPicker>
#include <QTimer>
#include <QDiffuseSpecularMaterial>
#include <QMenu>
#include <QAction>
#include <QParameter>

#include <cmath>

#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

SimulationWindowQt3D::SimulationWindowQt3D(QScreen *screen)
//    : Qt3DWindowCustom(screen),
    : Qt3DWindow(screen),
    m_mouseClickEvent(QEvent::Type::None, QPointF(),  QPointF(), Qt::MouseButton::NoButton, Qt::NoButton, Qt::NoModifier)
{
    m_trackball = new Trackball();
    m_cursorColour = Preferences::valueQColor("CursorColour");
    m_backgroundColour = Preferences::valueQColor("BackgroundColour");
    m_axesScale = Preferences::valueFloat("GlobalAxesSize");
    m_mouseClickEventPtr = &m_mouseClickEvent;
}

SimulationWindowQt3D::~SimulationWindowQt3D()
{
    if (m_trackball) delete m_trackball;
    if (m_aviWriter) delete m_aviWriter;
}

void SimulationWindowQt3D::initialiseScene()
{
    /* this is a rather brute force way of clearing the root entity */
    // create the root entity
    Qt3DCore::QEntity *tempEntity = m_rootEntity;
    m_rootEntity = new Qt3DCore::QEntity();
    this->setRootEntity(m_rootEntity);
    if (tempEntity) delete
        tempEntity; // must delete the old root QEntity after if has been replaced because setRootEntity calls setParent(nullptr) the old root
    /* I'd prefer it if this worked but deleteChildrenRecursively crashes
        // create the root entity and the required child entities
        if (m_rootEntity)
        {
            deleteChildrenRecursively(m_rootEntity->childNodes());
        }
        else
        {
            m_rootEntity = new Qt3DCore::QEntity();
            this->setRootEntity(m_rootEntity);
        }
    */

    // background
    this->defaultFrameGraph()->setClearColor(m_backgroundColour);
    // this->defaultFrameGraph()->noDraw()->setEnabled(false);

    // deleting the rootEntity deletes all the child entities but I need to clear them from the lists too
    m_drawBodyMap.clear();
    m_drawJointMap.clear();
    m_drawGeomMap.clear();
    m_drawMuscleMap.clear();
    m_drawMarkerMap.clear();
    m_drawFluidSacMap.clear();

    // and the ray caster
    m_screenRayCaster = new Qt3DRender::QScreenRayCaster(m_rootEntity);
    m_screenRayCaster->setRunMode(Qt3DRender::QAbstractRayCaster::SingleShot);
    m_rootEntity->addComponent(m_screenRayCaster);
    connect(m_screenRayCaster, SIGNAL(hitsChanged(const Qt3DRender::QAbstractRayCaster::Hits &)), this, SLOT(rayCasterHitsChanged(const Qt3DRender::QAbstractRayCaster::Hits &)));

    // and the 3D cursor location
    m_cursor3DTransform = new Qt3DCore::QTransform(m_rootEntity);

    // camera
    updateCamera();

    // lights
//    Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity(m_rootEntity);
//    Qt3DRender::QPointLight *light = new Qt3DRender::QPointLight(lightEntity);
//    light->setColor("white");
//    light->setIntensity(1);
//    lightEntity->addComponent(light);
//    Qt3DCore::QTransform *lightTransform = new Qt3DCore::QTransform(lightEntity);
//    lightTransform->setTranslation(QVector3D(0, -100, 0));
//    lightEntity->addComponent(lightTransform);
    Qt3DCore::QEntity *lightEntity1 = new Qt3DCore::QEntity(m_rootEntity);
    Qt3DRender::QDirectionalLight *light1 = new Qt3DRender::QDirectionalLight(lightEntity1);
    light1->setColor("yellow");
    light1->setIntensity(1);
    light1->setWorldDirection(QVector3D(0, -1, 0));
    lightEntity1->addComponent(light1);
    Qt3DCore::QEntity *lightEntity2 = new Qt3DCore::QEntity(m_rootEntity);
    Qt3DRender::QDirectionalLight *light2 = new Qt3DRender::QDirectionalLight(lightEntity2);
    light2->setColor("cyan");
    light2->setIntensity(1);
    light2->setWorldDirection(QVector3D(-0.8660f, 0.5f, 0));
    lightEntity2->addComponent(light2);
    Qt3DCore::QEntity *lightEntity3 = new Qt3DCore::QEntity(m_rootEntity);
    Qt3DRender::QDirectionalLight *light3 = new Qt3DRender::QDirectionalLight(lightEntity3);
    light3->setColor("magenta");
    light3->setIntensity(1);
    light3->setWorldDirection(QVector3D(0.8660f, 0.5f, 0));
    lightEntity3->addComponent(light3);

    // picking
    Qt3DRender::QRenderSettings *renderSettings = this->renderSettings();
    renderSettings->setRenderPolicy(Qt3DRender::QRenderSettings::OnDemand); // Qt3DRender::QRenderSettings::Always uses a lot of CPU when nothing is happening
    // renderSettings->setRenderPolicy(Qt3DRender::QRenderSettings::Always); // Qt3DRender::QRenderSettings::Always uses a lot of CPU when nothing is happening
    Qt3DRender::QPickingSettings *pickingSettings = renderSettings->pickingSettings();
    pickingSettings->setFaceOrientationPickingMode(Qt3DRender::QPickingSettings::FrontFace);
    pickingSettings->setPickMethod(Qt3DRender::QPickingSettings::PrimitivePicking);
    pickingSettings->setPickResultMode(Qt3DRender::QPickingSettings::AllPicks);
    pickingSettings->setWorldSpaceTolerance(1e-10f);
    m_pickableLayer = new Qt3DRender::QLayer(m_rootEntity);
    m_pickableLayer->setRecursive(true);
    m_screenRayCaster->addLayer(m_pickableLayer);

    // create the 3D cursor
    m_cursor3D = new FacetedSphere(m_cursorRadius, 4, m_cursorColour, 1.0, m_rootEntity);
    m_cursor3D->InitialiseEntity();
    m_cursor3D->addComponent(m_cursor3DTransform);

    // create the axes
    FacetedAxes *axes = new FacetedAxes(m_rootEntity);
    axes->InitialiseEntity();
    Qt3DCore::QTransform *axesTransform = new Qt3DCore::QTransform(axes);
    axesTransform->setScale(m_axesScale);
    axes->addComponent(axesTransform);
    axes->addComponent(m_pickableLayer); // everything except the 3D cursor is pickable

    // set up the simulation object containers
    m_staticEntities = new Qt3DCore::QEntity(m_rootEntity);
    m_staticEntities->addComponent(m_pickableLayer);
    m_dynamicEntities = new Qt3DCore::QEntity(m_rootEntity);
    m_dynamicEntities->addComponent(m_pickableLayer);

}

void SimulationWindowQt3D::updateCamera()
{
    Qt3DRender::QCamera *camera = this->camera();
    float aspectRatio = float(width()) / float(height());
    if (m_orthographicProjection)
    {
        camera->setProjectionType(Qt3DRender::QCameraLens::OrthographicProjection);
        float viewHeight = 2.0f * std::sin((m_FOV / 2.0f) * float(M_PI) / 180.0f) * m_cameraDistance;
        float viewWidth = viewHeight * aspectRatio;
        camera->setLeft(-viewWidth);
        camera->setRight(viewWidth);
        camera->setBottom(-viewHeight);
        camera->setTop(viewHeight);
        camera->setNearPlane(m_frontClip);
        camera->setFarPlane(m_backClip);
    }
    else
    {
        camera->setProjectionType(Qt3DRender::QCameraLens::PerspectiveProjection);
        camera->setFieldOfView(m_FOV);
        camera->setAspectRatio(aspectRatio);
        camera->setNearPlane(m_frontClip);
        camera->setFarPlane(m_backClip);
    }
    QVector3D position(m_COIx - (m_cameraVecX * m_cameraDistance), m_COIy - (m_cameraVecY * m_cameraDistance), m_COIz - (m_cameraVecZ * m_cameraDistance));
    QVector3D target(m_COIx, m_COIy, m_COIz);
    QVector3D upVector(m_upX, m_upY, m_upZ);
    camera->setUpVector(upVector);
    camera->setViewCenter(target);
    camera->setPosition(position);

    m_projectionMatrix = camera->lens()->projectionMatrix();
    m_viewMatrix = camera->viewMatrix();
    requestUpdate();
}

void SimulationWindowQt3D::mousePressEvent(QMouseEvent *event)
{
    m_mouseClickEventPtr = event;
    if (m_moveMarkerMode)
    {
        m_moveMarkerMode = false;
        if (m_mouseClickEventPtr->modifiers() == Qt::NoModifier)
        {
            QVector3D position = m_cursor3DTransform->translation();
            emit EmitMoveMarkerRequest(QString::fromStdString(m_moveMarkerName), position);
        }
        return;
    }
    if (m_screenRayCaster) m_screenRayCaster->trigger(event->pos());
}

void SimulationWindowQt3D::rayCasterHitsChanged(const Qt3DRender::QAbstractRayCaster::Hits &hits)
{
    if (m_moveMarkerMode)
    {
        int index = closestHit(hits);
        if (index < 0) return;
        QVector3D worldIntersection = hits.at(index).worldIntersection();
        m_cursor3DTransform->setTranslation(worldIntersection);
        updateCamera();
        return;
    }
    m_trackballFlag = false;
    if (m_mouseClickEventPtr->buttons() & Qt::LeftButton)
    {
        if (m_mouseClickEventPtr->modifiers() == Qt::NoModifier)
        {
            int trackballRadius;
            if (width() < height()) trackballRadius = int(width() / 2.2f);
            else trackballRadius = int(height() / 2.2f);
            m_trackballStartCameraVec = QVector3D(m_cameraVecX, m_cameraVecY, m_cameraVecZ);
            m_trackballStartUp = QVector3D(m_upX, m_upY, m_upZ);
            m_trackball->StartTrackball(m_mouseClickEventPtr->pos().x(), m_mouseClickEventPtr->pos().y(), width() / 2,
                                        height() / 2, trackballRadius,
                                        pgd::Vector3(double(m_trackballStartUp.x()), double(m_trackballStartUp.y()), double(m_trackballStartUp.z())),
                                        pgd::Vector3(double(-m_trackballStartCameraVec.x()), double(-m_trackballStartCameraVec.y()), double(-m_trackballStartCameraVec.z())));
            m_trackballFlag = true;
            emit EmitStatusString(tr("Rotate"), 2);
            updateCamera();
        }
        else if (m_mouseClickEventPtr->modifiers() & Qt::ShiftModifier)
        {
            // detect the collision point of the mouse click
            int index = closestHit(hits);
            if (index >= 0)
            {
                QVector3D worldIntersection = hits.at(index).worldIntersection();
                m_cursor3DTransform->setTranslation(worldIntersection);
                QClipboard *clipboard = QApplication::clipboard();
                clipboard->setText(QString("%1\t%2\t%3").arg(double(worldIntersection.x())).arg(double(
                                       worldIntersection.y())).arg(double(worldIntersection.z())), QClipboard::Clipboard);
                emit EmitStatusString(QString("3D Cursor %1\t%2\t%3").arg(double(worldIntersection.x())).arg(double(
                                          worldIntersection.y())).arg(double(worldIntersection.z())), 2);
                updateCamera();
            }
        }
    }
    else if (m_mouseClickEventPtr->buttons() & Qt::MiddleButton)
    {
        if (m_mouseClickEventPtr->modifiers() == Qt::NoModifier)
        {
            m_panFlag = true;

            m_panStartScreenPoint = QVector3D((2.0f * m_mouseClickEventPtr->pos().x()) / width() - 1.0f, 1.0f - (2.0f * m_mouseClickEventPtr->pos().y()) / height(), 0);
            m_panStartCOI = QVector3D(m_COIx, m_COIy, m_COIz);
            m_projectPanMatrix = m_projectionMatrix *
                                 m_viewMatrix; // model would be identity so mvpMatrix isn't needed
            m_unprojectPanMatrix = m_projectPanMatrix.inverted(); // we need the unproject matrix

            // detect the collision point of the mouse click
            int index = closestHit(hits);
            if (index >= 0)
            {
                QVector3D worldIntersection = hits.at(index).worldIntersection();
                m_panStartPoint = worldIntersection;
                QVector3D screenStartPoint = m_projectPanMatrix * m_panStartPoint;
                m_panStartScreenPoint.setZ(screenStartPoint.z());
            }
            else     // this is harder since we don't know the screen Z were are interested in.
            {
                // generate a screen Z from projecting the COI into screen coordinates (-1 to 1 box)
                QVector3D screenStartPoint = m_projectPanMatrix * m_panStartCOI;
                m_panStartScreenPoint.setZ(screenStartPoint.z());
                // now unproject this point to get the pan start point
                m_panStartPoint = m_unprojectPanMatrix * m_panStartScreenPoint;
            }
            emit EmitStatusString(tr("Pan"), 2);
            updateCamera();
        }
        else if (m_mouseClickEventPtr->modifiers() & Qt::AltModifier)
        {
            int index = closestHit(hits);
            if (index >= 0)
            {
                QVector3D worldIntersection = hits.at(index).worldIntersection();
                m_COIx = worldIntersection.x();
                m_COIy = worldIntersection.y();
                m_COIz = worldIntersection.z();
                QClipboard *clipboard = QApplication::clipboard();
                clipboard->setText(QString("%1\t%2\t%3").arg(double(worldIntersection.x())).arg(double( worldIntersection.y())).arg(double(worldIntersection.z())), QClipboard::Clipboard);
                emit EmitStatusString(QString("Centre of Interest %1\t%2\t%3").arg(double( worldIntersection.x())).arg(double(worldIntersection.y())).arg(double(worldIntersection.z())), 2);
                emit EmitCOI(worldIntersection.x(), worldIntersection.y(), worldIntersection.z());
                updateCamera();
            }
        }
    }
    else if (m_mouseClickEventPtr->buttons() & Qt::RightButton)
    {
        if (m_mouseClickEventPtr->modifiers() == Qt::NoModifier)
        {
            m_hits = hits;
            m_closestHitIndex = closestHit(m_hits);
            menuRequest(m_mouseClickEventPtr->pos());
        }
    }
}

void SimulationWindowQt3D::mouseMoveEvent(QMouseEvent *event)
{
    if (m_moveMarkerMode)
    {
        if (event->modifiers() != Qt::NoModifier) m_moveMarkerMode = false;
        else m_screenRayCaster->trigger(event->pos());
        return;
    }
    if (event->buttons() & Qt::LeftButton)
    {
        if (m_trackballFlag)
        {
            pgd::Quaternion pgdRotation;
            m_trackball->RollTrackballToClick(event->pos().x(), event->pos().y(), &pgdRotation);
            QQuaternion rotation(float(pgdRotation.n), float(pgdRotation.x), float(pgdRotation.y), float(pgdRotation.z));
            rotation = rotation.conjugated();
            QVector3D newCameraVec = rotation * m_trackballStartCameraVec;
            m_cameraVecX = newCameraVec.x();
            m_cameraVecY = newCameraVec.y();
            m_cameraVecZ = newCameraVec.z();
            QVector3D newUp = rotation *  m_trackballStartUp;
            m_upX = newUp.x();
            m_upY = newUp.y();
            m_upZ = newUp.z();
            updateCamera();

            emit EmitStatusString(QString("Camera %1 %2 %3 Up %4 %5 %6").arg(double(m_cameraVecX)).arg(double( m_cameraVecY)).arg(double(m_cameraVecZ)).arg(double(m_upX)).arg(double(m_upY)).arg(double(m_upZ)),
                                  2);
        }
    }
    else if (event->buttons() & Qt::MiddleButton)
    {
        if (m_panFlag)
        {
            QVector3D screenPoint((2.0f * event->pos().x()) / width() - 1.0f, 1.0f - (2.0f * event->pos().y()) / height(), m_panStartScreenPoint.z());
            QVector3D panCurrentPoint = m_unprojectPanMatrix * screenPoint;
            m_COIx = m_panStartCOI.x() - (panCurrentPoint.x() - m_panStartPoint.x());
            m_COIy = m_panStartCOI.y() - (panCurrentPoint.y() - m_panStartPoint.y());
            m_COIz = m_panStartCOI.z() - (panCurrentPoint.z() - m_panStartPoint.z());
            // qDebug("panCurrentPoint=%f,%f,%f,%f", panCurrentPoint[0], panCurrentPoint[1], panCurrentPoint[2], panCurrentPoint[3]);

            emit EmitStatusString(QString("COI %1 %2 %3").arg(double(m_COIx)).arg(double(m_COIy)).arg(double(
                                      m_COIz)), 2);
            emit EmitCOI(m_COIx, m_COIy, m_COIz);
            updateCamera();
        }
    }
}

void SimulationWindowQt3D::mouseReleaseEvent(QMouseEvent * /* event */)
{
    m_trackballFlag = false;
    m_panFlag = false;
    updateCamera();
}

void SimulationWindowQt3D::wheelEvent(QWheelEvent *event)
{
    // assume each ratchet of the wheel gives a score of 120 (8 * 15 degrees)
    float sensitivity = 2400;
    float scale = 1.0f + float(event->delta()) / sensitivity;
    m_FOV *= scale;
    if (m_FOV > 170) m_FOV = 170;
    else if (m_FOV < 0.001f) m_FOV = 0.001f;
    updateCamera();

    emit EmitStatusString(QString("FOV %1").arg(double(m_FOV)), 2);
    emit EmitFoV(m_FOV);
}

// handle key presses
void SimulationWindowQt3D::keyPressEvent(QKeyEvent *event)
{
    QVector3D newPosition = m_cursor3DTransform->translation();
    switch ( event->key() )
    {

    // X, Y and Z move the cursor
    case Qt::Key_X:
        if (event->modifiers() == Qt::NoModifier)
            newPosition.setX(newPosition.x() + m_cursor3DNudge);
        else
            newPosition.setX(newPosition.x() - m_cursor3DNudge);
        break;

    case Qt::Key_Y:
        if (event->modifiers() == Qt::NoModifier)
            newPosition.setY(newPosition.y() + m_cursor3DNudge);
        else
            newPosition.setY(newPosition.y() - m_cursor3DNudge);
        break;

    case Qt::Key_Z:
        if (event->modifiers() == Qt::NoModifier)
            newPosition.setZ(newPosition.z() + m_cursor3DNudge);
        else
            newPosition.setZ(newPosition.z() - m_cursor3DNudge);
        break;

    // S snaps the cursor to the nearest whole number multiple of the nudge value
    case Qt::Key_S:
        newPosition.setX(round(newPosition.x() / m_cursor3DNudge) * m_cursor3DNudge);
        newPosition.setY(round(newPosition.y() / m_cursor3DNudge) * m_cursor3DNudge);
        newPosition.setZ(round(newPosition.z() / m_cursor3DNudge) * m_cursor3DNudge);
        break;
    }

    if (newPosition != m_cursor3DTransform->translation())
    {
        m_cursor3DTransform->setTranslation(newPosition);
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(QString("%1\t%2\t%3").arg(double(newPosition.x())).arg(double( newPosition.y())).arg(double(newPosition.z())), QClipboard::Clipboard);
        emit EmitStatusString(QString("3D Cursor %1\t%2\t%3").arg(double(newPosition.x())).arg(double( newPosition.y())).arg(double(newPosition.z())), 2);
        updateCamera();
    }
}

void SimulationWindowQt3D::menuRequest(const QPoint &pos)
{
    if (!m_simulation) return;
    if (m_closestHitIndex < 0) return;

    QMenu menu;
    QAction *action = menu.addAction(tr("Centre View"));
    menu.addSeparator();
    Qt3DCore::QEntity *entity = nullptr;
    if (m_mainWindow->mode() == MainWindow::constructionMode)
    {
        action = menu.addAction(tr("Create Marker..."));
        menu.addSeparator();

        entity = m_hits.at(m_closestHitIndex).entity();
        EntityType entityType;
        entity = getParentDrawType(entity, &entityType);
        if (entityType == DrawBodyType) action = menu.addAction(tr("Edit Body..."));
        else if (entityType == DrawFluidSacType) action = menu.addAction(tr("Edit Fluid Sac..."));
        else if (entityType == DrawGeomType) action = menu.addAction(tr("Edit Geom..."));
        else if (entityType == DrawJointType) action = menu.addAction(tr("Edit Joint..."));
        else if (entityType == DrawMarkerType)
        {
            action = menu.addAction(tr("Edit Marker..."));
            action = menu.addAction(tr("Move Marker"));
        }
        else if (entityType == DrawMuscleType) action = menu.addAction(tr("Edit Muscle..."));
        if (action->text() == tr("Edit Fluid Sac...")) action->setEnabled(
                false); // not implemented any time soon
    }

    QPoint gp = this->mapToGlobal(pos);
    action = menu.exec(gp);
    if (action)
    {
        if (action->text() == tr("Centre View"))
        {
            QVector3D worldIntersection = m_hits.at(m_closestHitIndex).worldIntersection();
            m_COIx = worldIntersection.x();
            m_COIy = worldIntersection.y();
            m_COIz = worldIntersection.z();
            QClipboard *clipboard = QApplication::clipboard();
            clipboard->setText(QString("%1\t%2\t%3").arg(double(worldIntersection.x())).arg(double(
                                   worldIntersection.y())).arg(double(worldIntersection.z())), QClipboard::Clipboard);
            emit EmitStatusString(QString("Centre of Interest %1\t%2\t%3").arg(double(
                                      worldIntersection.x())).arg(double(worldIntersection.y())).arg(double(worldIntersection.z())), 2);
            emit EmitCOI(worldIntersection.x(), worldIntersection.y(), worldIntersection.z());
            updateCamera();
        }
        else if (action->text() == tr("Create Marker..."))
        {
            emit EmitCreateMarkerRequest();
        }
        else if (action->text() == tr("Edit Marker..."))
        {
            DrawMarker *drawMarker = dynamic_cast<DrawMarker *>(entity);
            emit EmitEditMarkerRequest(QString::fromStdString(drawMarker->marker()->name()));
        }
        else if (action->text() == tr("Edit Body..."))
        {
            DrawBody *drawBody = dynamic_cast<DrawBody *>(entity);
            emit EmitEditBodyRequest(QString::fromStdString(drawBody->body()->name()));
        }
        else if (action->text() == tr("Edit Geom..."))
        {
            DrawGeom *drawGeom = dynamic_cast<DrawGeom *>(entity);
            emit EmitEditGeomRequest(QString::fromStdString(drawGeom->geom()->name()));
        }
        else if (action->text() == tr("Edit Joint..."))
        {
            DrawJoint *drawJoint = dynamic_cast<DrawJoint *>(entity);
            emit EmitEditJointRequest(QString::fromStdString(drawJoint->joint()->name()));
        }
        else if (action->text() == tr("Edit Muscle..."))
        {
            DrawMuscle *drawMuscle = dynamic_cast<DrawMuscle *>(entity);
            emit EmitEditMuscleRequest(QString::fromStdString(drawMuscle->muscle()->name()));
        }
        else if (action->text() == tr("Move Marker"))
        {
            m_moveMarkerMode = true;
            DrawMarker *drawMarker = dynamic_cast<DrawMarker *>(entity);
            m_moveMarkerName = drawMarker->marker()->name();
        }
    }
}

Qt3DCore::QEntity *SimulationWindowQt3D::getParentDrawType(Qt3DCore::QEntity *entity,
        EntityType *entityType)
{
    DrawBody *drawBody = dynamic_cast<DrawBody *>(entity);
    if (drawBody)
    {
        *entityType = DrawBodyType;
        return drawBody;
    }
    DrawFluidSac *drawFluidSac = dynamic_cast<DrawFluidSac *>(entity);
    if (drawFluidSac)
    {
        *entityType = DrawFluidSacType;
        return drawFluidSac;
    }
    DrawGeom *drawGeom = dynamic_cast<DrawGeom *>(entity);
    if (drawGeom)
    {
        *entityType = DrawGeomType;
        return drawGeom;
    }
    DrawJoint *drawJoint = dynamic_cast<DrawJoint *>(entity);
    if (drawJoint)
    {
        *entityType = DrawJointType;
        return drawJoint;
    }
    DrawMarker *drawMarker = dynamic_cast<DrawMarker *>(entity);
    if (drawMarker)
    {
        *entityType = DrawMarkerType;
        return drawMarker;
    }
    DrawMuscle *drawMuscle = dynamic_cast<DrawMuscle *>(entity);
    if (drawMuscle)
    {
        *entityType = DrawMuscleType;
        return drawMuscle;
    }

    if (!entity->parentEntity())
    {
        *entityType = NotRecognised;
        return nullptr;
    }
    return getParentDrawType(entity->parentEntity(), entityType);
}

MainWindow *SimulationWindowQt3D::getMainWindow() const
{
    return m_mainWindow;
}

void SimulationWindowQt3D::setMainWindow(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

int SimulationWindowQt3D::getClosestHitIndex() const
{
    return m_closestHitIndex;
}

const Qt3DRender::QAbstractRayCaster::Hits &SimulationWindowQt3D::getHits() const
{
    return m_hits;
}

void SimulationWindowQt3D::resizeEvent ( QResizeEvent *event)
{
    qDebug() << "SimulationWindowQt3D::resizeEvent width=" << event->size().width() << " height=" << event->size().height();
    updateCamera();
}

float SimulationWindowQt3D::axesScale() const
{
    return m_axesScale;
}

void SimulationWindowQt3D::setAxesScale(float axesScale)
{
    m_axesScale = axesScale;
}

QVector3D SimulationWindowQt3D::cursor3DPosition() const
{
    return m_cursor3DTransform->translation();
}

void SimulationWindowQt3D::setCursor3DPosition(const QVector3D &cursor3DPosition)
{
    m_cursor3DTransform->setTranslation(cursor3DPosition);
    updateCamera();
}

int SimulationWindowQt3D::bodyMeshNumber() const
{
    return m_bodyMeshNumber;
}

void SimulationWindowQt3D::setBodyMeshNumber(int bodyMeshNumber)
{
    m_bodyMeshNumber = bodyMeshNumber;
}

Qt3DCore::QEntity *SimulationWindowQt3D::rootEntity() const
{
    return m_rootEntity;
}

QColor SimulationWindowQt3D::backgroundColour() const
{
    return m_backgroundColour;
}

void SimulationWindowQt3D::setBackgroundColour(const QColor &backgroundColour)
{
    m_backgroundColour = backgroundColour;
}

int SimulationWindowQt3D::aviQuality() const
{
    return m_aviQuality;
}

void SimulationWindowQt3D::setAviQuality(int aviQuality)
{
    m_aviQuality = aviQuality;
}

AVIWriter *SimulationWindowQt3D::aviWriter() const
{
    return m_aviWriter;
}

void SimulationWindowQt3D::setAviWriter(AVIWriter *aviWriter)
{
    m_aviWriter = aviWriter;
}

QColor SimulationWindowQt3D::cursorColour() const
{
    return m_cursorColour;
}

void SimulationWindowQt3D::setCursorColour(const QColor &cursorColour)
{
    m_cursorColour = cursorColour;
}

// write the current frame out to a file
int SimulationWindowQt3D::WriteStillFrame(const QString &filename)
{
    /*
    // get image from the last rendered frame
    irr::video::IImage* const image = videoDriver()->createScreenShot();
    if (image == 0)
    {
        qDebug("Error: SimulationWindowQt3D::WriteStillFrame(%s) cannot createScreenShot\n", qPrintable(filename));
        return __LINE__;
    }
    // write screenshot to file
    QString filenameWithExtension = filename + ".png";
    #ifdef _IRR_WCHAR_FILESYSTEM
    if (!videoDriver()->writeImageToFile(image, filenameWithExtension.toStdWString().c_str()))
    #else
    if (!videoDriver()->writeImageToFile(image, filenameWithExtension.toUtf8().data()))
    #endif
    {
        qDebug("Error: SimulationWindowQt3D::WriteStillFrame(%s) cannot writeImageToFile\n", qPrintable(filename));
        return __LINE__;
    }
    image->drop();
    */
    return 0;
}

// write the current frame out to a file
int SimulationWindowQt3D::WriteMovieFrame()
{
    Q_ASSERT(m_aviWriter);
    /*
    // get image from the last rendered frame
    irr::video::IImage* const image = videoDriver()->createScreenShot();
    if (image) //should always be true, but you never know. ;)
    {
        irr::core::dimension2d<irr::u32>size =  image->getDimension();
        unsigned char *rgb = new unsigned char[size.Width * size.Height * 3];
        unsigned char *ptr = rgb;
        QColor c;
        for (unsigned int iy = 0; iy < size.Height; iy++)
        {
            for (unsigned int ix = 0; ix < size.Width; ix++)
            {
                c = image->getPixel(ix, iy);
                *ptr++ = c.getRed();
                *ptr++ = c.getGreen();
                *ptr++ = c.getBlue();
            }
        }
        image->drop();
        m_aviWriter->WriteAVI(size.Width, size.Height, rgb, m_aviQuality);
        delete [] rgb;
    }
    */
    return 0;
}

int SimulationWindowQt3D::StartAVISave(const QString &fileName)
{
    /*
    if (m_aviWriter) delete m_aviWriter;
    m_aviWriter = new AVIWriter();
    if (m_aviQuality == 0) return __LINE__; // should always be true
    irr::video::IImage* const image = videoDriver()->createScreenShot();
    if (image == 0) return __LINE__; //should always be true, but you never know. ;)
    irr::core::dimension2d<irr::u32>size =  image->getDimension();
    unsigned char *rgb = new unsigned char[size.Width * size.Height * 3];
    unsigned char *ptr = rgb;
    QColor c;
    for (unsigned int iy = 0; iy < size.Height; iy++)
    {
        for (unsigned int ix = 0; ix < size.Width; ix++)
        {
            c = image->getPixel(ix, iy);
            *ptr++ = c.getRed();
            *ptr++ = c.getGreen();
            *ptr++ = c.getBlue();
        }
    }
    image->drop();
    m_aviWriter->InitialiseFile(fileName, size.Width, size.Height, m_fps);

    m_aviWriter->WriteAVI(size.Width, size.Height, rgb, m_aviQuality);
    delete [] rgb;
    */
    return 0;
}

int SimulationWindowQt3D::StopAVISave()
{
    if (m_aviWriter == nullptr) return __LINE__;
    delete m_aviWriter;
    m_aviWriter = nullptr;
    return 0;
}

// write the scene as a series of OBJ files in a folder
int SimulationWindowQt3D::WriteCADFrame(const QString &pathname)
{
    QString workingFolder = QDir::currentPath();
    if (QDir(pathname).exists() == false)
    {
        if (QDir().mkdir(pathname) == false)
        {
            QMessageBox::warning(nullptr, "Snapshot Error", QString("Could not create folder '%1' for OBJ files\n").arg(pathname), "Click button to return to simulation");
            return __LINE__;
        }
    }
    QDir::setCurrent(pathname);
    /*
        irr::scene::ISceneNode *rootSceneNode = sceneManager()->getRootSceneNode();
        irr::scene::ISceneNodeList allSceneNodes;
        GetAllChildren(rootSceneNode, &allSceneNodes);

        irr::scene::IMeshWriter *meshWriter = sceneManager()->createMeshWriter(irr::scene::EMWT_OBJ);
        irr::io::IFileSystem *fs = sceneManager()->getFileSystem();
        irr::scene::IMeshManipulator *mm = sceneManager()->getMeshManipulator(); // careful because only some IMeshManipulator functions work on IDynamicMesh and EIT_32BIT index types

        irr::core::array<irr::scene::IMesh *> meshList;
        irr::core::array<const QMatrix4x4 *> transformationList;
        irr::core::array<const irr:: c8 *> nameList;
        for (irr::scene::ISceneNodeList::ConstIterator it = allSceneNodes.begin(); it != allSceneNodes.end(); it++)
        {
            irr::scene::IMeshSceneNode *meshSceneNode = dynamic_cast<irr::scene::IMeshSceneNode *>(*it);
            if (meshSceneNode)
            {
                meshList.push_back(meshSceneNode->getMesh());
                meshSceneNode->updateAbsolutePosition();
                transformationList.push_back(&meshSceneNode->getAbsoluteTransformation());
                continue;
            }
            irr::scene::IAnimatedMeshSceneNode *animatedMeshSceneNode = dynamic_cast<irr::scene::IAnimatedMeshSceneNode *>(*it);
            if (animatedMeshSceneNode)
            {
                meshList.push_back(animatedMeshSceneNode->getMesh()->getMesh(0));
                animatedMeshSceneNode->updateAbsolutePosition();
                transformationList.push_back(&animatedMeshSceneNode->getAbsoluteTransformation());
                continue;
            }
        }

        for (irr::u32 meshCount = 0; meshCount < meshList.size(); meshCount++)
        {

            QString meshFileName = QString("Mesh%1.obj").arg(meshCount, 5, 10, QChar('0'));
            irr::io::IWriteFile *file = fs->createAndWriteFile(meshFileName.toUtf8().constData());
            QMatrix4x4 transformation = *transformationList[meshCount];
            mm->transform(meshList[meshCount], transformation); // transform the mesh in place to avoid duplicating a big mesh
            meshWriter->writeMesh(file, meshList[meshCount]);
            transformation.makeInverse();
            mm->transform(meshList[meshCount], transformation); // and reverse the transformation
            file->drop();

            for (irr::u32 meshBufferIndex = 0; meshBufferIndex <  meshList[meshCount]->getMeshBufferCount(); meshBufferIndex++)
            {
                const irr::video::SMaterial &material =  meshList[meshCount]->getMeshBuffer(meshBufferIndex)->getMaterial();
                irr::video::ITexture *texture = material.getTexture(0);
                if (texture)
                {
                    irr::io::path materialPath = texture->getName();
                    qDebug() << "materialPath " << materialPath.c_str() << "\n";
                    irr::io::path textureBasename = fs->getFileBasename(materialPath);
                    irr::video::ECOLOR_FORMAT format = texture->getColorFormat();
                    if (format == irr::video::ECF_A8R8G8B8)
                    {
                        const irr::core::dimension2du &textureSize = texture->getSize();
                        QImage qImage(textureSize.Width, textureSize.Height, QImage::Format_ARGB32);
                        irr::u8 *texturePtr = static_cast<irr::u8 *>(texture->lock(irr::video::ETLM_READ_ONLY, 0));
                        memcpy(qImage.bits(), texturePtr, qImage.width() * qImage.height() * 4);
                        texture->unlock();
                        QImage flippedImage = qImage.mirrored(false, true);
    #ifdef _IRR_WCHAR_FILESYSTEM
                        flippedImage.save(QString::fromWCharArray(textureBasename.c_str()));
    #else
                        flippedImage.save(QString(textureBasename.c_str()));
    #endif
                    }
                    else
                    {
                        QMessageBox::warning(0, "Snapshot Error", QString("Could note generate texture for format \1\n").arg(format), "Click button to continue");
                    }
                }
            }
        }

        meshWriter->drop();
        */
    QDir::setCurrent(workingFolder);
    return 0;
}

void SimulationWindowQt3D::SetCameraVec(double x, double y, double z)
{
    SetCameraVec(float(x), float(y), float(z));
}

void SimulationWindowQt3D::SetCameraVec(float x, float y, float z)
{
    m_cameraVecX = x;
    m_cameraVecY = y;
    m_cameraVecZ = z;
    if (z > 0.999f || z < -0.999f)
    {
        m_upX = 0;
        m_upY = 1;
        m_upZ = 0;
    }
    else
    {
        m_upX = 0;
        m_upY = 0;
        m_upZ = 1;
    }
    updateCamera();
}


void SimulationWindowQt3D::updateModel()
{
    if (!m_simulation) return;
    std::map<std::string, Body *> *bodyList = m_simulation->GetBodyList();
    for (auto &&iter : *bodyList)
    {
        std::map<std::string, DrawBody *>::iterator it = m_drawBodyMap.find(iter.first);
        if (it != m_drawBodyMap.end() && it->second->body() == iter.second)
        {
            it->second->updateEntityPose(); // this body already has an associated entity so just update the position
        }
        else
        {
            if (it != m_drawBodyMap.end()) delete it->second;
            DrawBody *drawBody = new DrawBody();
            drawBody->initialise(iter.second;
            drawBody->setParent(m_staticEntities);
            m_drawBodyMap[iter.first] = drawBody;
        }
        m_drawBodyMap[iter.first]->setEnabled(iter.second->visible());
        m_drawBodyMap[iter.first]->meshEntity1()->setEnabled(m_bodyMeshNumber == 1);
        m_drawBodyMap[iter.first]->meshEntity2()->setEnabled(m_bodyMeshNumber == 2);
        m_drawBodyMap[iter.first]->meshEntity3()->setEnabled(m_bodyMeshNumber == 3);
    }

    std::map<std::string, Joint *> *jointList = m_simulation->GetJointList();
    for (auto &&iter : *jointList)
    {
        std::map<std::string, DrawJoint *>::iterator it = m_drawJointMap.find(iter.first);
        if (it != m_drawJointMap.end() && it->second->joint() == iter.second)
        {
            it->second->updateEntityPose(); // this joint already has an associated entity so just update the position
        }
        else
        {
            if (it != m_drawJointMap.end()) delete it->second;
            DrawJoint *drawJoint = new DrawJoint();
            drawJoint->initialise(iter.second);
            drawJoint->setParent(m_staticEntities);
            m_drawJointMap[iter.first] = drawJoint;
        }
        m_drawJointMap[iter.first]->setEnabled(iter.second->visible());
    }

    std::map<std::string, Geom *> *geomList = m_simulation->GetGeomList();
    for (auto &&iter : *geomList)
    {
        auto it = m_drawGeomMap.find(iter.first);
        if (it != m_drawGeomMap.end() && it->second->geom() == iter.second)
        {
            it->second->updateEntityPose(); // this geom already has an associated entity so just update the position
        }
        else
        {
            if (it != m_drawGeomMap.end()) delete it->second;
            DrawGeom *drawGeom = new DrawGeom();
             drawGeom->initialise(iter.second);
            drawGeom->setParent(m_staticEntities);
            m_drawGeomMap[iter.first] = drawGeom;
        }
        m_drawGeomMap[iter.first]->setEnabled(iter.second->visible());
    }

    auto *markerList = m_simulation->GetMarkerList();
    for (auto &&iter : *markerList)
    {
        auto it = m_drawMarkerMap.find(iter.first);
        if (it != m_drawMarkerMap.end() && it->second->marker() == iter.second)
        {
            it->second->updateEntityPose();
        }
        else
        {
            if (it != m_drawMarkerMap.end()) delete it->second;
            DrawMarker *drawMarker = new DrawMarker();
            drawMarker->initialise(iter.second);
            drawMarker->setParent(m_staticEntities);
            m_drawMarkerMap[iter.first] = drawMarker;
        }
        m_drawMarkerMap[iter.first]->setEnabled(iter.second->visible());
    }

    auto *muscleList = m_simulation->GetMuscleList();
    for (auto &&iter : *muscleList)
    {
        auto it = m_drawMuscleMap.find(iter.first);
        if (it != m_drawMuscleMap.end()) delete it->second;
        DrawMuscle *drawMuscle = new DrawMuscle();
        drawMuscle->initialise(iter.second);
        drawMuscle->setParent(m_dynamicEntities);
        m_drawMuscleMap[iter.first] = drawMuscle;
        m_drawMuscleMap[iter.first]->setEnabled(iter.second->visible());
    }

    std::map<std::string, FluidSac *> *fluidSacList = m_simulation->GetFluidSacList();
    for (auto &&iter : *fluidSacList)
    {
        auto it = m_drawFluidSacMap.find(iter.first);
        if (it != m_drawFluidSacMap.end()) delete it->second;
        DrawFluidSac *drawFluidSac = new DrawFluidSac();
         drawFluidSac->initialise(iter.second);
        drawFluidSac->setParent(m_dynamicEntities);
        m_drawFluidSacMap[iter.first] = drawFluidSac;
        m_drawFluidSacMap[iter.first]->setEnabled(iter.second->visible());
    }

    requestUpdate();
}

int SimulationWindowQt3D::closestHit(const Qt3DRender::QAbstractRayCaster::Hits &hits)
{
    float distance = FLT_MAX;
    int index = -1;
    for (int i = 0; i < hits.size(); i++)
    {
        if (hits.at(i).distance() < distance && isEntityVisible(hits.at(i).entity()))
        {
            distance = hits.at(i).distance();
            index = i;
        }
    }
    return index;
}

bool SimulationWindowQt3D::isEntityVisible(Qt3DCore::QNode *node)
{
    if (!node) return true;
    if (!node->isEnabled()) return false;
    return (isEntityVisible(node->parentNode()));
}


void SimulationWindowQt3D::deleteChildrenRecursively(Qt3DCore::QNodeVector vector)
{
    for (int i = vector.size() - 1; i >= 0; i--)
    {
        Qt3DCore::QEntity *entity = dynamic_cast<Qt3DCore::QEntity *>(vector[i]);
        if (entity)
        {
            for (int j = entity->components().size() - 1; j >= 0; j--)
            {
                auto ptr = entity->components()[j];
                entity->removeComponent(ptr);
                delete (ptr);
            }
        }
        deleteChildrenRecursively(vector[i]->childNodes());
        delete (vector[i]);
    }
}

bool SimulationWindowQt3D::halfTransparency() const
{
    return m_halfTransparency;
}

void SimulationWindowQt3D::setHalfTransparency(bool halfTransparency)
{
    m_halfTransparency = halfTransparency;
}

bool SimulationWindowQt3D::normals() const
{
    return m_normals;
}

void SimulationWindowQt3D::setNormals(bool normals)
{
    m_normals = normals;
}

float SimulationWindowQt3D::cursor3DNudge() const
{
    return m_cursor3DNudge;
}

void SimulationWindowQt3D::setCursor3DNudge(float cursor3DNudge)
{
    m_cursor3DNudge = cursor3DNudge;
}

float SimulationWindowQt3D::cursorRadius() const
{
    return m_cursorRadius;
}

void SimulationWindowQt3D::setCursorRadius(float cursorRadius)
{
    m_cursorRadius = cursorRadius;
}

float SimulationWindowQt3D::upZ() const
{
    return m_upZ;
}

void SimulationWindowQt3D::setUpZ(float upZ)
{
    m_upZ = upZ;
}

float SimulationWindowQt3D::upY() const
{
    return m_upY;
}

void SimulationWindowQt3D::setUpY(float upY)
{
    m_upY = upY;
}

float SimulationWindowQt3D::upX() const
{
    return m_upX;
}

void SimulationWindowQt3D::setUpX(float upX)
{
    m_upX = upX;
}

Simulation *SimulationWindowQt3D::simulation() const
{
    return m_simulation;
}

void SimulationWindowQt3D::setSimulation(Simulation *simulation)
{
    m_simulation = simulation;
}

bool SimulationWindowQt3D::wireFrame() const
{
    return m_wireFrame;
}

void SimulationWindowQt3D::setWireFrame(bool wireFrame)
{
    m_wireFrame = wireFrame;
}

bool SimulationWindowQt3D::boundingBox() const
{
    return m_boundingBox;
}

void SimulationWindowQt3D::setBoundingBox(bool boundingBox)
{
    m_boundingBox = boundingBox;
}

bool SimulationWindowQt3D::orthographicProjection() const
{
    return m_orthographicProjection;
}

void SimulationWindowQt3D::setOrthographicProjection(bool orthographicProjection)
{
    m_orthographicProjection = orthographicProjection;
}

float SimulationWindowQt3D::cameraDistance() const
{
    return m_cameraDistance;
}

void SimulationWindowQt3D::setCameraDistance(float cameraDistance)
{
    m_cameraDistance = cameraDistance;
}

float SimulationWindowQt3D::FOV() const
{
    return m_FOV;
}

void SimulationWindowQt3D::setFOV(float FOV)
{
    m_FOV = FOV;
}

float SimulationWindowQt3D::cameraVecX() const
{
    return m_cameraVecX;
}

void SimulationWindowQt3D::setCameraVecX(float cameraVecX)
{
    m_cameraVecX = cameraVecX;
}

float SimulationWindowQt3D::cameraVecY() const
{
    return m_cameraVecY;
}

void SimulationWindowQt3D::setCameraVecY(float cameraVecY)
{
    m_cameraVecY = cameraVecY;
}

float SimulationWindowQt3D::cameraVecZ() const
{
    return m_cameraVecZ;
}

void SimulationWindowQt3D::setCameraVecZ(float cameraVecZ)
{
    m_cameraVecZ = cameraVecZ;
}

float SimulationWindowQt3D::COIx() const
{
    return m_COIx;
}

void SimulationWindowQt3D::setCOIx(float COIx)
{
    m_COIx = COIx;
}

float SimulationWindowQt3D::COIy() const
{
    return m_COIy;
}

void SimulationWindowQt3D::setCOIy(float COIy)
{
    m_COIy = COIy;
}

float SimulationWindowQt3D::COIz() const
{
    return m_COIz;
}

void SimulationWindowQt3D::setCOIz(float COIz)
{
    m_COIz = COIz;
}

float SimulationWindowQt3D::frontClip() const
{
    return m_frontClip;
}

void SimulationWindowQt3D::setFrontClip(float frontClip)
{
    m_frontClip = frontClip;
}

float SimulationWindowQt3D::backClip() const
{
    return m_backClip;
}

void SimulationWindowQt3D::setBackClip(float backClip)
{
    m_backClip = backClip;
}

std::map<std::string, DrawMarker *> *SimulationWindowQt3D::getDrawMarkerMap()
{
    return &m_drawMarkerMap;
}

std::map<std::string, DrawFluidSac *> *SimulationWindowQt3D::getDrawFluidSacMap()
{
    return &m_drawFluidSacMap;
}

std::map<std::string, DrawMuscle *> *SimulationWindowQt3D::getDrawMuscleMap()
{
    return &m_drawMuscleMap;
}

std::map<std::string, DrawGeom *> *SimulationWindowQt3D::getDrawGeomMap()
{
    return &m_drawGeomMap;
}

std::map<std::string, DrawJoint *> *SimulationWindowQt3D::getDrawJointMap()
{
    return &m_drawJointMap;
}

std::map<std::string, DrawBody *> *SimulationWindowQt3D::getDrawBodyMap()
{
    return &m_drawBodyMap;
}

bool SimulationWindowQt3D::getDrawBodyMesh3() const
{
    return m_drawBodyMesh3;
}

void SimulationWindowQt3D::setDrawBodyMesh3(bool drawBodyMesh3)
{
    m_drawBodyMesh3 = drawBodyMesh3;
}

bool SimulationWindowQt3D::getDrawBodyMesh2() const
{
    return m_drawBodyMesh2;
}

void SimulationWindowQt3D::setDrawBodyMesh2(bool drawBodyMesh2)
{
    m_drawBodyMesh2 = drawBodyMesh2;
}

bool SimulationWindowQt3D::getDrawBodyMesh1() const
{
    return m_drawBodyMesh1;
}

void SimulationWindowQt3D::setDrawBodyMesh1(bool drawBodyMesh1)
{
    m_drawBodyMesh1 = drawBodyMesh1;
}

#endif
