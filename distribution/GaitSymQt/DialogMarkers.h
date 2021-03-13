#ifndef DIALOGMARKERS_H
#define DIALOGMARKERS_H

#include "Preferences.h"

#include <QDialog>
#include <QVector3D>

#include <memory>

class Marker;
class Simulation;

namespace pgd {
class Vector3;
}

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
    void overrideStartPosition(const pgd::Vector3 &position);

    void setCursor3DPosition(const QVector3D &cursor3DPosition);

    Simulation *simulation() const;
    void setSimulation(Simulation *simulation);

    void setInputMarker(Marker *inputMarker);

    std::unique_ptr<Marker> outputMarker();

public slots:
    virtual void accept() Q_DECL_OVERRIDE;
    virtual void reject() Q_DECL_OVERRIDE;
    void properties();
    void calculatePosition();
    void calculatePositionCopyMarker1();
    void calculatePositionCopyMarker2();
    void calculateOrientation2Marker();
    void calculateOrientation3Marker();
    void calculateMirrorMarker();
    void calculateCalculator();
    void calculateMatrix();
    void copy3DCursorPosition();
    void importCalculator();
    void importMatrix();
    void lineEditIDTextChanged(const QString &text);
    void lineEditFractionTextChanged(const QString &text);
    void lineEditDistanceTextChanged(const QString &text);
    void lineEditEulerAppendTextChanged(const QString &text);
    void lineEditAxisAngleTextChanged(const QString &text);
    void lineEditQuaternionTextChanged(const QString &text);
    void positionMarkerChanged(const QString &text);
    void orientation2MarkerChanged(const QString &text);
    void orientation3MarkerChanged(const QString &text);
    void labelAxisAngleMenuRequest(const QPoint &pos);

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    Ui::DialogMarkers *ui;

    Simulation *m_simulation = nullptr;
    Marker *m_inputMarker = nullptr;
    std::unique_ptr<Marker> m_outputMarker;
    QVector3D m_cursor3DPosition;

    QMap<QString, SettingsItem> m_properties;
};

#endif // DIALOGMARKERS_H
