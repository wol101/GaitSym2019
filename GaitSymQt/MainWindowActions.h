/*
 *  MainWindowActions.h
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 31/08/2020.
 *  Copyright 2020 Bill Sellers. All rights reserved.
 *
 */

#ifndef MAINWINDOWACTIONS_H
#define MAINWINDOWACTIONS_H

#include <QObject>

class Marker;
class Body;
class Joint;
class Muscle;
class Geom;
class Driver;
class MainWindow;
class NamedObject;
class QAction;

class MainWindowActions : public QObject
{
    Q_OBJECT
public:
    explicit MainWindowActions(QObject *parent = nullptr);

public slots:
    void buttonCameraBack();
    void buttonCameraBottom();
    void buttonCameraFront();
    void buttonCameraLeft();
    void buttonCameraRight();
    void buttonCameraTop();
    void enterConstructionMode();
    void enterRunMode();
    void menu1280x720();
    void menu1920x1080();
    void menu640x480();
    void menu800x600();
    void menuAbout();
    void menuClearMeshCache();
    void menuCreateAssembly();
    void menuCreateBody();
    void menuCreateDriver();
    void menuCreateGeom();
    void menuCreateJoint();
    void menuCreateMarker();
    void menuCreateMuscle();
    void menuCreateMirrorElements();
    void menuCreateTestingDrivers();
    void menuLoadDefaultView();
    void menuDeleteAssembly();
    void menuEditGlobal();
    void menuExportMarkers();
    void menuImportMeshes();
    void menuImportWarehouse();
    void menuNew();
    void menuOpen();
    void menuOutputs();
    void menuPreferences();
    void menuRawXMLEditor();
    void menuRecordMovie();
    void menuRename();
    void menuResetView();
    void menuRestart();
    void menuSave();
    void menuSaveAs();
    void menuSaveDefaultView();
    void menuStartOBJSequenceSave();
    void menuStartUSDSequenceSave();
    void menuStartWarehouseExport();
    void menuStopAVISave();
    void menuStopOBJSequenceSave();
    void menuStopUSDSequenceSave();
    void menuStopWarehouseExport();
    void menuToggleFullScreen();

    void menuOpen(const QString &fileName, const QByteArray *fileData);
    void menuCreateEditMarker(Marker *marker);
    void menuCreateEditBody(Body *body);
    void menuCreateEditJoint(Joint *joint);
    void menuCreateEditGeom(Geom *geom);
    void menuCreateEditMuscle(Muscle *muscle);
    void menuCreateEditDriver(Driver *driver);

    void menuOpenRecent(QAction *action);

    void elementInfo(const QString &elementType, const QString &elementName);
    void elementHide(const QString &elementType, const QString &elementName);

    void usdSnapshot();
    void objSnapshot();
    void run();
    void snapshot();
    void step();

    void copy();
    void cut();
    void paste();
    void selectAll();

private:
    MainWindow *m_mainWindow = nullptr;

    void updateRecentFiles(const QString &recentFile);
    QStringList m_recentFileList;
    int m_maxRecentFiles = 20;
};

#endif // MAINWINDOWACTIONS_H
