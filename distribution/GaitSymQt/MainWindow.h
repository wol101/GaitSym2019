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
class SimulationWindow;
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
    SimulationWindow *simulationWindow() const;

public slots:
    void setInterfaceValues();
    void processOneThing();

    void about();
    void buttonCameraFront();
    void buttonCameraRight();
    void buttonCameraTop();
    void buttonCameraBack();
    void buttonCameraLeft();
    void buttonCameraBottom();
    void checkboxActivationColours(int);
    void checkboxContactForce(int);
    void checkboxMuscleForce(int);
    void checkboxTracking(int);
    void menu1280x720();
    void menu1920x1080();
    void menu640x480();
    void menu800x600();
    void menuDefaultView();
    void menuImportWarehouse();
    void menuOutputs();
    void menuPreferences();
    void menuRecordMovie();
    void menuSaveDefaultView();
    void menuStartAVISave();
    void menuStartOBJSequenceSave();
    void menuStartWarehouseExport();
    void menuStopAVISave();
    void menuStopOBJSequenceSave();
    void menuStopWarehouseExport();
    void menuToggleFullScreen();
    void menuGeneticAlgorithm();
    void menuNextAscentHillclimbing();
    void menuRandomAscentHillclimbing();
    void menuSimplexSearch();
    void menuSimulatedAnnealing();
    void menuTabuSearch();
    void menuNew();
    void menuCreateBody();
    void menuCreateMarker();
    void menuCreateJoint();
    void menuCreateMuscle();
    void menuCreateGeom();
    void menuCreateDriver();
    void menuEditGlobal();
    void open();
    void resizeAndCentre(int w, int h);
    void restart();
    void run();
    void saveas();
    void save();
    void snapshot();
    void objSnapshot();
    void spinboxCOIXChanged(double);
    void spinboxCOIYChanged(double);
    void spinboxCOIZChanged(double);
    void spinboxCursorNudgeChanged(double);
    void spinboxCursorSizeChanged(double);
    void spinboxDistanceChanged(double);
    void spinboxFarChanged(double);
    void spinboxFoVChanged(double);
    void spinboxFPSChanged(double);
    void spinboxNearChanged(double);
    void spinboxSkip(int);
    void spinboxTimeMax(double);
    void spinboxTrackingOffsetChanged(double v);
    void step();
    void comboBoxCurrentTextChanged(const QString &text);
    void deleteExistingBody(const QString &);
    void deleteExistingMarker(const QString &);
    void deleteExistingJoint(const QString &);
    void deleteExistingMuscle(const QString &);
    void deleteExistingGeom(const QString &);
    void deleteExistingDriver(const QString &name);
    void editExistingBody(const QString &);
    void editExistingMarker(const QString &);
    void editExistingJoint(const QString &);
    void editExistingMuscle(const QString &);
    void editExistingGeom(const QString &);
    void editExistingDriver(const QString &);
    void moveExistingMarker(const QString &s, const QVector3D &p);
    void handleElementTreeWidgetItemChanged(QTreeWidgetItem *item, int column);
    void enterRunMode();
    void enterConstructionMode();


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

    void open(const QString &fileName);
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

    Ui::MainWindow *ui = nullptr;

    QFileInfo m_configFile;

    bool m_movieFlag = false;
    bool m_saveOBJFileSequenceFlag = false;
    QString m_objFileSequenceFolder;
    bool m_stepFlag = false;
    uint64_t m_stepCount = 0;
    int m_logLevel = 1;

    QTimer *m_timer = nullptr;
    SimulationWindow *m_simulationWindow = nullptr;
    Simulation *m_simulation = nullptr;

    Mode m_mode = constructionMode;
    bool m_noName = true;
    bool m_saveRequired = false;
};

#endif // MAINWINDOW_H
