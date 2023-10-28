/*
 *  MainWindow.h
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 08/10/2018.
 *  Copyright 2018 Bill Sellers. All rights reserved.
 *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileInfo>

#ifdef USE_QT3D
class SimulationWindowQt3D;
#else
class SimulationWidget;
#endif

namespace Ui {
class MainWindow;
}

class Simulation;
class QTreeWidgetItem;
class MainWindowActions;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow() Q_DECL_OVERRIDE;

   friend MainWindowActions;

    enum Mode { constructionMode, runMode };

    Mode mode() const;
    Simulation *simulation() const;
#ifdef USE_QT3D
    SimulationWindowQt3D *simulationWidget() const;
#else
    SimulationWidget *simulationWidget() const;
#endif

    const QFileInfo &configFile() const;

public slots:
    void processOneThing();
    void handleCommandLineArguments();

    void comboBoxMeshDisplayMapCurrentTextChanged(const QString &text);
    void comboBoxMuscleColourMapCurrentTextChanged(const QString &text);
    void comboBoxProjectionCurrentTextChanged(const QString &text);
    void comboBoxTrackingMarkerCurrentTextChanged(const QString &text);
    void deleteExistingBody(const QString &, bool force = false);
    void deleteExistingDriver(const QString &name, bool force = false);
    void deleteExistingGeom(const QString &, bool force = false);
    void deleteExistingJoint(const QString &, bool force = false);
    void deleteExistingMarker(const QString &, bool force = false);
    void deleteExistingMuscle(const QString &, bool force = false);
    void editExistingBody(const QString &);
    void editExistingDriver(const QString &);
    void editExistingGeom(const QString &);
    void editExistingJoint(const QString &);
    void editExistingMarker(const QString &);
    void editExistingMuscle(const QString &);
    void handleElementTreeWidgetItemChanged(QTreeWidgetItem *item, int column);

    void moveExistingMarker(const QString &s, const QVector3D &p);
    void radioButtonTracking();
    void reportOpenGLSize(int width, int height);
    void resizeAndCentre(int w, int h);
    void spinboxCOIXChanged(double v);
    void spinboxCOIYChanged(double v);
    void spinboxCOIZChanged(double v);
    void spinboxCursorNudgeChanged(double v);
    void spinboxCursorSizeChanged(double v);
    void spinboxDistanceChanged(double v);
    void spinboxFPSChanged(double v);
    void spinboxFarChanged(double v);
    void spinboxFoVChanged(double v);
    void spinboxNearChanged(double v);
    void spinboxSkip(int i);
    void spinboxTimeMax(double v);
    void spinboxTrackingOffsetChanged(double v);

    void setStatusString(const QString &s, int logLevel);
    void setUICOI(float x, float y, float z);
    void setUIFoV(float v);

    void log(const QString &text);

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:

    void setInterfaceValues();
    void writeSettings();
    static QByteArray readResource(const QString &resource);
    void updateEnable();
    void resizeSimulationWindow(int openGLWidth, int openGLHeight);
    void updateComboBoxTrackingMarker();
    void handleTracking();

    Ui::MainWindow *ui = nullptr;
#ifdef USE_QT3D
    SimulationWindowQt3D *m_simulationWidget = nullptr;
#else
    SimulationWidget *m_simulationWidget = nullptr;
#endif

    QFileInfo m_configFile; // maybe use windowFilePath() and windowTitle() instead

    bool m_movieFlag = false;
    bool m_saveOBJFileSequenceFlag = false;
    QString m_objFileSequenceFolder;
    enum objFileFormat { obj, usda };
    enum objFileFormat m_objFileFormat = obj;
    bool m_stepFlag = false;
    uint64_t m_stepCount = 0;
    int m_logLevel = 1;

    QTimer *m_timer = nullptr;
    Simulation *m_simulation = nullptr;

    Mode m_mode = constructionMode;
    bool m_noName = true;

    MainWindowActions *m_mainWindowActions = nullptr;
};

#endif // MAINWINDOW_H
