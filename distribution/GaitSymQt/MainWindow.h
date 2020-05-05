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

namespace Ui {
class MainWindow;
}

class GLWidget;
class DialogVisibility;
class QBoxLayout;
class QListWidgetItem;
class ViewControlWidget;
class DialogLog;
class Preferences;
class SimulationWidget;
class Simulation;
class AVIWriter;
class ElementTreeModel;
class QTreeWidgetItem;
class Marker;
class Body;
class Joint;
class Muscle;
class Geom;
class Driver;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow() Q_DECL_OVERRIDE;

    enum Mode { constructionMode, runMode };

    Mode mode() const;
    Simulation *simulation() const;
    SimulationWidget *simulationWidget() const;

public slots:
    void setInterfaceValues();
    void processOneThing();

    void buttonCameraBack();
    void buttonCameraBottom();
    void buttonCameraFront();
    void buttonCameraLeft();
    void buttonCameraRight();
    void buttonCameraTop();
    void comboBoxMeshDisplayMapCurrentTextChanged(const QString &text);
    void comboBoxMuscleColourMapCurrentTextChanged(const QString &text);
    void comboBoxTrackingBodyCurrentTextChanged(const QString &text);
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
    void enterConstructionMode();
    void enterRunMode();
    void handleElementTreeWidgetItemChanged(QTreeWidgetItem *item, int column);
    void menu1280x720();
    void menu1920x1080();
    void menu640x480();
    void menu800x600();
    void menuAbout();
    void menuCreateAssembly();
    void menuCreateBody();
    void menuCreateDriver();
    void menuCreateGeom();
    void menuCreateJoint();
    void menuCreateMarker();
    void menuCreateMuscle();
    void menuDefaultView();
    void menuDeleteAssembly();
    void menuEditGlobal();
    void menuExportMarkers();
    void menuGeneticAlgorithm();
    void menuImportWarehouse();
    void menuNew();
    void menuNextAscentHillclimbing();
    void menuOpen();
    void menuOutputs();
    void menuPreferences();
    void menuRandomAscentHillclimbing();
    void menuRecordMovie();
    void menuResetView();
    void menuRestart();
    void menuSave();
    void menuSaveAs();
    void menuSaveDefaultView();
    void menuSimplexSearch();
    void menuSimulatedAnnealing();
    void menuStartAVISave();
    void menuStartOBJSequenceSave();
    void menuStartWarehouseExport();
    void menuStopAVISave();
    void menuStopOBJSequenceSave();
    void menuStopWarehouseExport();
    void menuTabuSearch();
    void menuToggleFullScreen();
    void moveExistingMarker(const QString &s, const QVector3D &p);
    void objSnapshot();
    void resizeAndCentre(int w, int h);
    void radioButtonTrackingNone(int);
    void radioButtonTrackingX(int);
    void radioButtonTrackingY(int);
    void radioButtonTrackingZ(int);
    void run();
    void snapshot();
    void spinboxCOIXChanged(double);
    void spinboxCOIYChanged(double);
    void spinboxCOIZChanged(double);
    void spinboxCursorNudgeChanged(double);
    void spinboxCursorSizeChanged(double);
    void spinboxDistanceChanged(double);
    void spinboxFPSChanged(double);
    void spinboxFarChanged(double);
    void spinboxFoVChanged(double);
    void spinboxNearChanged(double);
    void spinboxSkip(int);
    void spinboxTimeMax(double);
    void spinboxTrackingOffsetChanged(double v);
    void step();

    void copy();
    void cut();
    void paste();
    void selectAll();

    void setStatusString(const QString &s, int logLevel);
    void setUICOI(float x, float y, float z);
    void setUIFoV(float v);

    void log(const QString &text);

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:

    void menuOpen(const QString &fileName);
    void writeSettings();
    static QByteArray readResource(const QString &resource);
    void updateEnable();
    void menuCreateEditMarker(Marker *marker);
    void menuCreateEditBody(Body *body);
    void menuCreateEditJoint(Joint *joint);
    void menuCreateEditGeom(Geom *geom);
    void menuCreateEditMuscle(Muscle *muscle);
    void menuCreateEditDriver(Driver *driver);
    void resizeSimulationWindow(int w, int h);
    void updateComboBoxTrackingBody();


    Ui::MainWindow *ui = nullptr;

    QFileInfo m_configFile;

    bool m_movieFlag = false;
    bool m_saveOBJFileSequenceFlag = false;
    QString m_objFileSequenceFolder;
    bool m_stepFlag = false;
    uint64_t m_stepCount = 0;
    int m_logLevel = 1;

    QTimer *m_timer = nullptr;
    Simulation *m_simulation = nullptr;

    Mode m_mode = constructionMode;
    bool m_noName = true;
    bool m_saveRequired = false;
};

#endif // MAINWINDOW_H
