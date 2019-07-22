#ifndef DIALOGBODYBUILDER_H
#define DIALOGBODYBUILDER_H

#include <QDialog>

class QLineEdit;
class FacetedObject;
class SimulationWindow;
class Body;
class LineEditPath;
class Simulation;

namespace Ui {
class DialogBodyBuilder;
}

class DialogBodyBuilder : public QDialog
{
    Q_OBJECT

public:
    explicit DialogBodyBuilder(QWidget *parent = nullptr);
    virtual ~DialogBodyBuilder() Q_DECL_OVERRIDE;

    void lateInitialise();

    void setBody(Body *body);
    Body *body() const;

    Simulation *simulation() const;
    void setSimulation(Simulation *simulation);

    bool createMode() const;
    void setCreateMode(bool createMode);

public slots:
    virtual void accept() Q_DECL_OVERRIDE;
    virtual void reject() Q_DECL_OVERRIDE;
    void calculate();
    void lineEditMeshClicked();
    void lineEditIDTextChanged(const QString &text);

private:
    void displayMesh(const QString &filename);
    void lineEditMeshActivated(LineEditPath *lineEdit);

    Ui::DialogBodyBuilder *ui = nullptr;

    SimulationWindow *m_simulationWindow = nullptr;
    QString m_displayFileName;

    FacetedObject *m_referenceObject = nullptr;
    Body *m_body = nullptr;
    Simulation *m_simulation = nullptr;

    const std::map<std::string, Body *> *m_existingBodies = nullptr;
    bool m_createMode = false;
};

#endif // DIALOGBODYBUILDER_H
