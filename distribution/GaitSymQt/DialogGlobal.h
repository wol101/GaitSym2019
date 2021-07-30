#ifndef DIALOGGLOBAL_H
#define DIALOGGLOBAL_H

#include "Preferences.h"
#include "Global.h"

#include <QDialog>
#include <QMap>

#include <map>
#include <string>
#include <memory>

class Body;

namespace Ui {
class DialogGlobal;
}

#include "Global.h"

class DialogGlobal : public QDialog
{
    Q_OBJECT

public:
    explicit DialogGlobal(QWidget *parent = nullptr);
    virtual ~DialogGlobal() Q_DECL_OVERRIDE;

    void lateInitialise();

    void setExistingBodies(const std::map<std::string, std::unique_ptr<Body>> *existingBodies);

    void setInputGlobal(const Global *inputGlobal);

    std::unique_ptr<Global> outputGlobal();

public slots:
    virtual void accept() Q_DECL_OVERRIDE;
    virtual void reject() Q_DECL_OVERRIDE;
    void properties();
    void checkBoxSpringDampingStateChanged(int state);
    void setDefaults();

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    static void ConvertToCFMERP(double spring_constant, double damping_constant,
                                double integration_stepsize, double *cfm, double *erp);
    static void ConvertToSpringAndDampingConstants(double erp, double cfm, double integration_stepsize,
                                                   double *spring_constant, double *damping_constant);
    void updateUI(const Global *globalPtr);
    void initialiseDefaultGlobal();

    Ui::DialogGlobal *ui;

    const Global *m_inputGlobal = nullptr;
    std::unique_ptr<Global> m_outputGlobal;
    const std::map<std::string, std::unique_ptr<Body>> *m_existingBodies = nullptr;
    Global m_defaultGlobal;

    QMap<QString, SettingsItem> m_properties;

};

#endif // DIALOGGLOBAL_H
