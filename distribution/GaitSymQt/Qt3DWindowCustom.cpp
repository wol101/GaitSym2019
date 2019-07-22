/****************************************************************************
**
** Copyright (C) 2016 Klaralvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "Qt3DWindowCustom.h"
#include "QForwardRendererCustom.h"

#include <QAspectEngine>
#include <Qt3DCore/qentity.h>
#include <Qt3DRender/qrendersettings.h>
#include <Qt3DRender/qrenderaspect.h>
#include <Qt3DInput/qinputaspect.h>
#include <Qt3DInput/qinputsettings.h>
#include <Qt3DLogic/qlogicaspect.h>
#include <Qt3DRender/qcamera.h>
#include <QtGui/qopenglcontext.h>

#include <QEvent>
#include <QResizeEvent>

//static void initResources()
//{
//#ifdef QT_STATIC
//    Q_INIT_RESOURCE(extras);
//#endif
//}

Qt3DWindowCustom::Qt3DWindowCustom(QScreen *screen)
    : QWindow(screen)
    , m_aspectEngine(new Qt3DCore::QAspectEngine)
    , m_renderAspect(new Qt3DRender::QRenderAspect(Qt3DRender::QRenderAspect::Threaded))
    , m_inputAspect(new Qt3DInput::QInputAspect)
    , m_logicAspect(new Qt3DLogic::QLogicAspect)
    , m_renderSettings(new Qt3DRender::QRenderSettings)
    , m_forwardRenderer(new QForwardRendererCustom)
    , m_defaultCamera(new Qt3DRender::QCamera)
    , m_inputSettings(new Qt3DInput::QInputSettings)
    , m_root(new Qt3DCore::QEntity)
    , m_userRoot(nullptr)
    , m_initialized(false)
{
//    initResources();

//    if (!d->parentWindow)
//        d->connectToScreen(screen ? screen : d->topLevelScreen.data());

    setSurfaceType(QSurface::OpenGLSurface);

    resize(1024, 768);

    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
#ifdef QT_OPENGL_ES_2
    format.setRenderableType(QSurfaceFormat::OpenGLES);
#else
    if (QOpenGLContext::openGLModuleType() == QOpenGLContext::LibGL)
    {
        format.setVersion(4, 3);
        format.setProfile(QSurfaceFormat::CoreProfile);
    }
#endif
    format.setDepthBufferSize(24);
    format.setSamples(4);
    format.setStencilBufferSize(8);
    setFormat(format);
    QSurfaceFormat::setDefaultFormat(format);

    m_aspectEngine->registerAspect(m_renderAspect);
    m_aspectEngine->registerAspect(m_inputAspect);
    m_aspectEngine->registerAspect(m_logicAspect);

    m_defaultCamera->setParent(m_root);
    m_forwardRenderer->setCamera(m_defaultCamera);
    m_forwardRenderer->setSurface(this);
    m_renderSettings->setActiveFrameGraph(m_forwardRenderer);
    m_inputSettings->setEventSource(this);
}

Qt3DWindowCustom::~Qt3DWindowCustom()
{
    delete m_aspectEngine;
}

/*!
    Registers the specified \a aspect.
*/
void Qt3DWindowCustom::registerAspect(Qt3DCore::QAbstractAspect *aspect)
{
    Q_ASSERT(!isVisible());
    m_aspectEngine->registerAspect(aspect);
}

/*!
    Registers the specified aspect \a name.
*/
void Qt3DWindowCustom::registerAspect(const QString &name)
{
    Q_ASSERT(!isVisible());
    m_aspectEngine->registerAspect(name);
}

/*!
    Sets the specified \a root entity of the scene.
*/
void Qt3DWindowCustom::setRootEntity(Qt3DCore::QEntity *root)
{
    if (m_userRoot != root)
    {
        if (m_userRoot != nullptr)
            m_userRoot->setParent(static_cast<Qt3DCore::QNode *>(nullptr));
        if (root != nullptr)
            root->setParent(m_root);
        m_userRoot = root;
    }
}

/*!
    Activates the specified \a activeFrameGraph.
*/
void Qt3DWindowCustom::setActiveFrameGraph(Qt3DRender::QFrameGraphNode *activeFrameGraph)
{
    m_renderSettings->setActiveFrameGraph(activeFrameGraph);
}

/*!
    Returns the node of the active frame graph.
*/
Qt3DRender::QFrameGraphNode *Qt3DWindowCustom::activeFrameGraph() const
{
    return m_renderSettings->activeFrameGraph();
}

/*!
    Returns the node of the default framegraph
*/
QForwardRendererCustom *Qt3DWindowCustom::defaultFrameGraph() const
{
    return m_forwardRenderer;
}

Qt3DRender::QCamera *Qt3DWindowCustom::camera() const
{
    return m_defaultCamera;
}

/*!
    Returns the render settings of the 3D Window.
*/
Qt3DRender::QRenderSettings *Qt3DWindowCustom::renderSettings() const
{
    return m_renderSettings;
}

/*!
    Manages the display events specified in \a e.
*/
void Qt3DWindowCustom::showEvent(QShowEvent *e)
{
    if (!m_initialized)
    {
        m_root->addComponent(m_renderSettings);
        m_root->addComponent(m_inputSettings);
        m_aspectEngine->setRootEntity(Qt3DCore::QEntityPtr(m_root));

        m_initialized = true;
    }

    QWindow::showEvent(e);
}

/*!
    Resets the aspect ratio of the 3D window.
*/
void Qt3DWindowCustom::resizeEvent(QResizeEvent *event)
{
    qDebug() << "Qt3DWindowCustom::resizeEvent width=" << event->size().width() << " height=" <<
             event->size().height();
    m_defaultCamera->setAspectRatio(float(width()) / float(height()));
}

/*!
    \reimp

    Requests renderer to redraw if we are using OnDemand render policy.
*/
bool Qt3DWindowCustom::event(QEvent *e)
{
    const bool needsRedraw = (e->type() == QEvent::Expose || e->type() == QEvent::UpdateRequest);
    if (needsRedraw && m_renderSettings->renderPolicy() == Qt3DRender::QRenderSettings::OnDemand)
        m_renderSettings->sendCommand(QLatin1Literal("InvalidateFrame"));
    return QWindow::event(e);
}


