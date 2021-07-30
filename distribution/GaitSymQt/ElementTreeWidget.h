#ifndef ELEMENTTREEWIDGET_H
#define ELEMENTTREEWIDGET_H

#include <QTreeWidget>

class Simulation;
class MainWindow;

class ElementTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit ElementTreeWidget(QWidget *parent = nullptr);

    Simulation *simulation() const;
    void setSimulation(Simulation *simulation);

    MainWindow *mainWindow() const;
    void setMainWindow(MainWindow *mainWindow);

    void clearLists();
    void fillVisibitilityLists(Simulation *simulation);
    void setVisibleSwitchAll(const QString &elementType, bool visibility);
    void setOutputSwitchAll(const QString &elementType, bool output);
    void setVisibleSwitch(const QString &elementType, const QString &elementName, bool visibility);

    static bool BinarySearch(QTreeWidgetItem *A, const QString &value, int *index);

signals:
    void createNewBody();
    void createNewMarker();
    void createNewJoint();
    void createNewGeom();
    void createNewMuscle();
    void createNewDriver();
    void editBody(const QString &body);
    void editMarker(const QString &marker);
    void editJoint(const QString &joint);
    void editGeom(const QString &geom);
    void editMuscle(const QString &muscle);
    void editDriver(const QString &muscle);
    void deleteBody(const QString &body);
    void deleteMarker(const QString &marker);
    void deleteJoint(const QString &joint);
    void deleteGeom(const QString &geom);
    void deleteMuscle(const QString &muscle);
    void deleteDriver(const QString &driver);
    void elementTreeWidgetItemChanged(QTreeWidgetItem *item, int column);
    void infoRequest(const QString &elementType, const QString &elementName);

public slots:
    void elementsItemChanged(QTreeWidgetItem *item, int column);

    int insertBody(const QString &name, bool visible, bool output);
    int insertMarker(const QString &name, bool visible, bool output);
    int insertJoint(const QString &name, bool visible, bool output);
    int insertGeom(const QString &name, bool visible, bool output);
    int insertMuscle(const QString &name, bool visible, bool output);
    int insertFluidSac(const QString &name, bool visible, bool output);
    int insertDriver(const QString &name, bool visible, bool output);
    int insertDataTarget(const QString &name, bool visible, bool output);
    int insertController(const QString &name, bool visible, bool output);

    int removeBody(const QString &name);
    int removeMarker(const QString &name);
    int removeJoint(const QString &name);
    int removeGeom(const QString &name);
    int removeMuscle(const QString &name);
    int removeFluidSac(const QString &name);
    int removeDriver(const QString &name);
    int removeDataTarget(const QString &name);
    int removeController(const QString &name);
    bool removeName(const QString &name);

signals:

private slots:
    void menuRequest(const QPoint &pos);

protected:

private:
    Simulation *m_simulation = nullptr;
    MainWindow *m_mainWindow = nullptr;

    const int ROOT_ITEM_TYPE = 0;
    const int ELEMENT_ITEM_TYPE = 1;

    QTreeWidgetItem *m_bodyTree = nullptr;
    QTreeWidgetItem *m_markerTree = nullptr;
    QTreeWidgetItem *m_jointTree = nullptr;
    QTreeWidgetItem *m_geomTree = nullptr;
    QTreeWidgetItem *m_muscleTree = nullptr;
    QTreeWidgetItem *m_fluidSacTree = nullptr;
    QTreeWidgetItem *m_driverTree = nullptr;
    QTreeWidgetItem *m_dataTargetTree = nullptr;
    QTreeWidgetItem *m_controllerTree = nullptr;
};

#endif // ELEMENTTREEWIDGET_H
