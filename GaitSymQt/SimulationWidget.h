#ifndef SIMULATIONWIDGET_H
#define SIMULATIONWIDGET_H

#ifndef USE_QT3

#include "StrokeFont.h"
#include "IntersectionHits.h"
#include "DrawBody.h"
#include "DrawJoint.h"
#include "DrawMuscle.h"
#include "DrawMarker.h"
#include "DrawFluidSac.h"
#include "DrawGeom.h"
#include "FacetedSphere.h"
#include "AVIWriter.h"
#include "TrackBall.h"

#include <QOpenGLWidget>
#include <QElapsedTimer>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions_3_3_Core>
#include <QMatrix4x4>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QMouseEvent>

#include <memory>
#include <map>

class Simulation;
class MainWindow;

struct SimpleLight
{
    GLfloat ambient[4];
    GLfloat diffuse[4];
    GLfloat specular[4];
    GLfloat position[4];

    void SetAmbient(GLfloat r, GLfloat g, GLfloat b, GLfloat a) { ambient[0] = r; ambient[1] = g; ambient[2] = b; ambient[3] = a; }
    void SetDiffuse(GLfloat r, GLfloat g, GLfloat b, GLfloat a) { diffuse[0] = r; diffuse[1] = g; diffuse[2] = b; diffuse[3] = a; }
    void SetSpecular(GLfloat r, GLfloat g, GLfloat b, GLfloat a) { specular[0] = r; specular[1] = g; specular[2] = b; specular[3] = a; }
    void SetPosition(GLfloat x, GLfloat y, GLfloat z, GLfloat a) { position[0] = x; position[1] = y; position[2] = z; position[3] = a; }
};

class SimulationWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    SimulationWidget(QWidget *parent = nullptr);

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
    int WriteUSDFrame(const QString &pathname);
    int StartAVISave(const QString &filename);
    int StopAVISave();

//    void AddExtraObjectToDraw(const std::string &name, std::shared_ptr<FacetedObject> object);
//    size_t DeleteExtraObjectToDraw(const std::string &name);
//    std::shared_ptr<FacetedObject> GetExtraObjectToDraw(const std::string &name);

    bool DeleteDrawBody(const std::string &bodyName);

    AVIWriter *aviWriter() const;

    int aviQuality() const;
    void setAviQuality(int aviQuality);

    QColor backgroundColour() const;
    void setBackgroundColour(const QColor &backgroundColour);

    float axesScale() const;
    void setAxesScale(float axesScale);

    MainWindow *getMainWindow() const;
    void setMainWindow(MainWindow *mainWindow);

    QOpenGLShaderProgram *facetedObjectShader() const;
    QOpenGLShaderProgram *fixedColourObjectShader() const;
    QMatrix4x4 proj() const;
    QMatrix4x4 view() const;

    bool getDrawBodyMesh1() const;
    void setDrawBodyMesh1(bool drawBodyMesh1);

    bool getDrawBodyMesh2() const;
    void setDrawBodyMesh2(bool drawBodyMesh2);

    bool getDrawBodyMesh3() const;
    void setDrawBodyMesh3(bool drawBodyMesh3);

    const IntersectionHits *getClosestHit() const;

    QString getLastMenuItem() const;

    std::map<std::string, std::unique_ptr<DrawBody>> *getDrawBodyMap();
    std::map<std::string, std::unique_ptr<DrawJoint>> *getDrawJointMap();
    std::map<std::string, std::unique_ptr<DrawGeom>> *getDrawGeomMap();
    std::map<std::string, std::unique_ptr<DrawMuscle>> *getDrawMuscleMap();
    std::map<std::string, std::unique_ptr<DrawFluidSac>> *getDrawFluidSacMap();
    std::map<std::string, std::unique_ptr<DrawMarker>> *getDrawMarkerMap();

public slots:
    void SetCameraVec(float x, float y, float z);
    void SetCameraVec(double x, double y, double z);
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
//    void EmitEditFluidSacRequest(const QString &s);
    void EmitDeleteMarkerRequest(const QString &s);
    void EmitDeleteBodyRequest(const QString &s);
    void EmitDeleteGeomRequest(const QString &s);
    void EmitDeleteJointRequest(const QString &s);
    void EmitDeleteMuscleRequest(const QString &s);
//    void EmitDeleteFluidSacRequest(const QString &s);
    void EmitInfoRequest(const QString &elementType, const QString &elementName);
    void EmitHideRequest(const QString &elementType, const QString &elementName);
    void EmitResize(int width, int height);

protected:
    virtual void initializeGL() Q_DECL_OVERRIDE;
    virtual void paintGL() Q_DECL_OVERRIDE;
    virtual void resizeGL(int width, int height) Q_DECL_OVERRIDE;
    virtual void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    virtual void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

private:
    void SetupLights();
    void drawModel();
    bool intersectModel(float winX, float winY);

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
    float m_cameraVecX = 0, m_cameraVecY = 1, m_cameraVecZ = 0;
    float m_COIx = 0, m_COIy = 0, m_COIz = 0;
    float m_frontClip = 1;
    float m_backClip = 1000;
    float m_upX = 0, m_upY = 0, m_upZ = 1;

    std::unique_ptr<Trackball> m_trackball;
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

    std::unique_ptr<FacetedSphere> m_cursor3D;
    float m_cursorRadius = 0.1f;
    float m_cursor3DNudge = 0.1f;
    QVector3D m_cursor3DPosition;
    QColor m_cursorColour;
    size_t m_cursorLevel = 4;
    QColor m_backgroundColour;
    float m_axesScale = 1;
    std::unique_ptr<FacetedObject> m_globalAxes;

//    std::map<std::string, std::shared_ptr<FacetedObject>> m_extraObjectsToDrawMap;

    std::unique_ptr<AVIWriter> m_aviWriter;
    int m_aviQuality = 80;
    unsigned int m_fps = 25;

    std::map<std::string, std::unique_ptr<DrawBody>> m_drawBodyMap;
    std::map<std::string, std::unique_ptr<DrawJoint>> m_drawJointMap;
    std::map<std::string, std::unique_ptr<DrawGeom>> m_drawGeomMap;
    std::map<std::string, std::unique_ptr<DrawMuscle>> m_drawMuscleMap;
    std::map<std::string, std::unique_ptr<DrawFluidSac>> m_drawFluidSacMap;
    std::map<std::string, std::unique_ptr<DrawMarker>> m_drawMarkerMap;
    std::vector<Drawable *> m_drawables;
    bool m_drawBodyMesh1 = true;
    bool m_drawBodyMesh2 = false;
    bool m_drawBodyMesh3 = false;

    StrokeFont m_StrokeFont;
    GLuint m_LineBuffer;
    GLuint m_LineColourBuffer;

    bool m_moveMarkerMode = false;
    std::string m_moveMarkerName;
    std::vector<std::unique_ptr<IntersectionHits>> m_hits;
    std::vector<size_t> m_hitsIndexByZ;
    QString m_lastMenuItem;

    QOpenGLVertexArrayObject *m_vao = nullptr;
    QOpenGLShaderProgram *m_facetedObjectShader = nullptr;
    QOpenGLShaderProgram *m_fixedColourObjectShader = nullptr;
    QMatrix4x4 m_proj;
    QMatrix4x4 m_view;
    QMatrix4x4 m_model;

};

#endif

#endif // SIMULATIONWIDGET_H
