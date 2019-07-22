/****************************************************************************
**
** Copyright (C) 2015 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "QForwardRendererCustom.h"

#include <Qt3DCore/qentity.h>
#include <Qt3DRender/qviewport.h>
#include <Qt3DRender/qcameraselector.h>
#include <Qt3DRender/qclearbuffers.h>
#include <Qt3DRender/qfilterkey.h>
#include <Qt3DRender/qfrustumculling.h>
#include <Qt3DRender/qrendersurfaceselector.h>
#include <Qt3DRender/qclearbuffers.h>
#include <Qt3DRender/qnodraw.h>

//static void initResources()
//{
//#ifdef QT_STATIC
//    Q_INIT_RESOURCE(extras);
//#endif
//}

void QForwardRendererCustom::init()
{
//    initResources();

    m_frustumCulling->setParent(m_clearBuffer);
    m_clearBuffer->setParent(m_cameraSelector);
    m_cameraSelector->setParent(m_viewport);
    m_viewport->setParent(m_surfaceSelector);
    m_surfaceSelector->setParent(this);

    m_viewport->setNormalizedRect(QRectF(0.0, 0.0, 1.0, 1.0));
    m_clearBuffer->setClearColor(Qt::white);
    m_clearBuffer->setBuffers(Qt3DRender::QClearBuffers::ColorDepthBuffer);
    m_noDraw = new Qt3DRender::QNoDraw(m_clearBuffer);

    Qt3DRender::QFilterKey *forwardRenderingStyle = new Qt3DRender::QFilterKey(this);
    forwardRenderingStyle->setName(QStringLiteral("renderingStyle"));
    forwardRenderingStyle->setValue(QStringLiteral("forward"));
    this->addMatch(forwardRenderingStyle);
}

/*!
    \class Qt3DExtras::QForwardRendererCustom
    \brief The QForwardRendererCustom provides a default \l{Qt 3D Render Framegraph}{FrameGraph}
    implementation of a forward renderer.
    \inmodule Qt3DExtras
    \since 5.7
    \inherits Qt3DRender::QTechniqueFilter

    Forward rendering is what OpenGL traditionally uses. It renders directly to the backbuffer
    one object at a time shading each one as it goes.

    QForwardRendererCustom is a single leaf \l{Qt 3D Render Framegraph}{FrameGraph} tree which contains
    a Qt3DRender::QViewport, a Qt3DRender::QCameraSelector, and a Qt3DRender::QClearBuffers.
    The QForwardRendererCustom has a default requirement filter key whose name is "renderingStyle" and
    value "forward".
    If you need to filter out your techniques, you should do so based on that filter key.

    By default the viewport occupies the whole screen and the clear color is white.
    Frustum culling is also enabled.
*/
/*!
    \qmltype ForwardRenderer
    \brief The ForwardRenderer provides a default \l{Qt 3D Render Framegraph}{FrameGraph}
    implementation of a forward renderer.
    \since 5.7
    \inqmlmodule Qt3D.Extras
    \instantiates Qt3DExtras::QForwardRendererCustom

    Forward rendering is what OpenGL traditionally uses. It renders directly to the backbuffer
    one object at a time shading each one as it goes.

    ForwardRenderer is a single leaf \l{Qt 3D Render Framegraph}{FrameGraph} tree which contains
    a Viewport, a CameraSelector, and a ClearBuffers.
    The ForwardRenderer has a default requirement filter key whose name is "renderingStyle" and
    value "forward".
    If you need to filter out your techniques, you should do so based on that filter key.

    By default the viewport occupies the whole screen and the clear color is white.
    Frustum culling is also enabled.
 */

QForwardRendererCustom::QForwardRendererCustom(QNode *parent)
    : QTechniqueFilter(parent)
    , m_surfaceSelector(new Qt3DRender::QRenderSurfaceSelector)
    , m_viewport(new Qt3DRender::QViewport())
    , m_cameraSelector(new Qt3DRender::QCameraSelector())
    , m_clearBuffer(new Qt3DRender::QClearBuffers())
    , m_frustumCulling(new Qt3DRender::QFrustumCulling())

{
    QObject::connect(m_clearBuffer, &Qt3DRender::QClearBuffers::clearColorChanged, this,
                     &QForwardRendererCustom::clearColorChanged);
    QObject::connect(m_viewport, &Qt3DRender::QViewport::normalizedRectChanged, this,
                     &QForwardRendererCustom::viewportRectChanged);
    QObject::connect(m_cameraSelector, &Qt3DRender::QCameraSelector::cameraChanged, this,
                     &QForwardRendererCustom::cameraChanged);
    QObject::connect(m_surfaceSelector, &Qt3DRender::QRenderSurfaceSelector::surfaceChanged, this,
                     &QForwardRendererCustom::surfaceChanged);
    QObject::connect(m_surfaceSelector,
                     &Qt3DRender::QRenderSurfaceSelector::externalRenderTargetSizeChanged, this,
                     &QForwardRendererCustom::externalRenderTargetSizeChanged);
    QObject::connect(m_frustumCulling, &Qt3DRender::QFrustumCulling::enabledChanged, this,
                     &QForwardRendererCustom::frustumCullingEnabledChanged);
    QObject::connect(m_viewport, &Qt3DRender::QViewport::gammaChanged, this,
                     &QForwardRendererCustom::gammaChanged);
    init();
}

QForwardRendererCustom::~QForwardRendererCustom()
{
}

void QForwardRendererCustom::setViewportRect(const QRectF &viewportRect)
{
    m_viewport->setNormalizedRect(viewportRect);
}

void QForwardRendererCustom::setClearColor(const QColor &clearColor)
{
    m_clearBuffer->setClearColor(clearColor);
}

void QForwardRendererCustom::setCamera(Qt3DCore::QEntity *camera)
{
    m_cameraSelector->setCamera(camera);
}

void QForwardRendererCustom::setSurface(QObject *surface)
{
    m_surfaceSelector->setSurface(surface);
}

void QForwardRendererCustom::setExternalRenderTargetSize(const QSize &size)
{
    m_surfaceSelector->setExternalRenderTargetSize(size);
}

void QForwardRendererCustom::setFrustumCullingEnabled(bool enabled)
{
    m_frustumCulling->setEnabled(enabled);
}

void QForwardRendererCustom::setGamma(float gamma)
{
    m_viewport->setGamma(gamma);
}

Qt3DRender::QNoDraw *QForwardRendererCustom::noDraw() const
{
    return m_noDraw;
}

/*!
    \qmlproperty rect ForwardRenderer::viewportRect

    Holds the current normalized viewport rectangle.
*/
/*!
    \property QForwardRendererCustom::viewportRect

    Holds the current normalized viewport rectangle.
*/
QRectF QForwardRendererCustom::viewportRect() const
{
    return m_viewport->normalizedRect();
}

/*!
    \qmlproperty color ForwardRenderer::clearColor

    Holds the current clear color of the scene. The frame buffer is initialized to the clear color
    before rendering.
*/
/*!
    \property QForwardRendererCustom::clearColor

    Holds the current clear color of the scene. The frame buffer is initialized to the clear color
    before rendering.
*/
QColor QForwardRendererCustom::clearColor() const
{
    return m_clearBuffer->clearColor();
}

/*!
    \qmlproperty Entity ForwardRenderer::camera

    Holds the current camera entity used to render the scene.

    \note A camera is an Entity that has a CameraLens as one of its components.
*/
/*!
    \property QForwardRendererCustom::camera

    Holds the current camera entity used to render the scene.

    \note A camera is a QEntity that has a QCameraLens as one of its components.
*/
Qt3DCore::QEntity *QForwardRendererCustom::camera() const
{
    return m_cameraSelector->camera();
}

/*!
    \qmlproperty Object ForwardRenderer::window

    Holds the current render surface.

    \deprecated
*/
/*!
    \property QForwardRendererCustom::window

    Holds the current render surface.

    \deprecated
*/

/*!
    \qmlproperty Object ForwardRenderer::surface

    Holds the current render surface.
*/
/*!
    \property QForwardRendererCustom::surface

    Holds the current render surface.
*/
QObject *QForwardRendererCustom::surface() const
{
    return m_surfaceSelector->surface();
}

/*!
    \property QForwardRendererCustom::externalRenderTargetSize

    Contains the size of the external render target. External render
    targets are relevant when rendering does not target a window
    surface (as set in \l {QForwardRendererCustom::surface()}{surface()}).
*/
QSize QForwardRendererCustom::externalRenderTargetSize() const
{
    return m_surfaceSelector->externalRenderTargetSize();
}

/*!
    \qmlproperty bool ForwardRenderer::frustumCulling

    Indicates if the renderer applies frustum culling to the scene.
*/
/*!
    \property QForwardRendererCustom::frustumCulling

    Indicates if the renderer applies frustum culling to the scene.
*/
bool QForwardRendererCustom::isFrustumCullingEnabled() const
{
    return m_frustumCulling->isEnabled();
}

/*!
    \qmlproperty real ForwardRenderer::gamma

    Holds the gamma value the renderer applies to the scene.
*/
/*!
    \property QForwardRendererCustom::gamma

    Holds the gamma value the renderer applies to the scene.
*/
float QForwardRendererCustom::gamma() const
{
    return m_viewport->gamma();
}

