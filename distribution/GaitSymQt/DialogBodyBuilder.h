#ifndef DIALOGBODYBUILDER_H
#define DIALOGBODYBUILDER_H

#include "Preferences.h"
#include "Body.h"

#include <QDialog>

#include <memory>

class QLineEdit;
class FacetedObject;
class SimulationWidget;
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

    Simulation *simulation() const;
    void setSimulation(Simulation *simulation);

    void setInputBody(Body *inputBody);

    std::unique_ptr<Body> outputBody();

public slots:
    virtual void accept() Q_DECL_OVERRIDE;
    virtual void reject() Q_DECL_OVERRIDE;
    void properties();
    void calculate();
    void lineEditMeshClicked();
    void lineEditIDTextChanged(const QString &text);

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    void lineEditMeshActivated(LineEditPath *lineEdit);

    Ui::DialogBodyBuilder *ui = nullptr;

    QString m_displayFileName;

    std::shared_ptr<FacetedObject> m_referenceObject;
    Body *m_inputBody = nullptr;
    std::unique_ptr<Body> m_outputBody;
    Simulation *m_simulation = nullptr;

    const std::map<std::string, Body *> *m_existingBodies = nullptr;
    QMap<QString, SettingsItem> m_properties;

};

#endif // DIALOGBODYBUILDER_H
