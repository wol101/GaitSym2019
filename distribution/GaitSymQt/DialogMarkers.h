#ifndef DIALOGMARKERS_H
#define DIALOGMARKERS_H

#include <QDialog>
#include <QVector3D>

class Marker;
class Simulation;

namespace Ui {
class DialogMarkers;
}

class DialogMarkers : public QDialog
{
    Q_OBJECT

public:
    explicit DialogMarkers(QWidget *parent = nullptr);
    virtual ~DialogMarkers() Q_DECL_OVERRIDE;

    void lateInitialise();

    Marker *marker() const;
    void setMarker(Marker *marker);

    QVector3D cursor3DPosition() const;
    void setCursor3DPosition(const QVector3D &cursor3DPosition);

    Simulation *simulation() const;
    void setSimulation(Simulation *simulation);

    bool createMode() const;
    void setCreateMode(bool createMode);

public slots:
    virtual void accept() Q_DECL_OVERRIDE;
    virtual void reject() Q_DECL_OVERRIDE;
    void calculatePosition();
    void calculatePositionCopyMarker1();
    void calculatePositionCopyMarker2();
    void calculateOrientation2Marker();
    void calculateOrientation3Marker();
    void calculateMirrorMarker();
    void copy3DCursorPosition();
    void lineEditIDTextChanged(const QString &text);
    void orientation2MarkerChanged(const QString &text);
    void orientation3MarkerChanged(const QString &text);

private:
    Ui::DialogMarkers *ui;

    Simulation *m_simulation = nullptr;
    Marker *m_marker = nullptr;
    QVector3D m_cursor3DPosition;
    bool m_createMode = false;
};

#endif // DIALOGMARKERS_H
