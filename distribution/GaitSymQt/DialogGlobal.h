#ifndef DIALOGGLOBAL_H
#define DIALOGGLOBAL_H

#include <QDialog>
#include <QMap>

#include <map>
#include <string>

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

    Global *global() const;
    void setGlobal(Global *global);

    void setExistingBodies(const std::map<std::string, Body *> *existingBodies);

public slots:
    virtual void accept() Q_DECL_OVERRIDE;
    virtual void reject() Q_DECL_OVERRIDE;
    void checkBoxSpringDampingStateChanged(int state);

private:
    void readValues();
    static void ConvertToCFMERP(double spring_constant, double damping_constant,
                                double integration_stepsize, double *cfm, double *erp);
    static void ConvertToSpringAndDampingConstants(double erp, double cfm, double integration_stepsize,
            double *spring_constant, double *damping_constant);

    Ui::DialogGlobal *ui;

    Global *m_global;
    const std::map<std::string, Body *> *m_existingBodies;

    QMap<QString, Global::FitnessType> m_fitnessTypeMap;
    QMap<QString, Global::StepType> m_stepTypeMap;

};

#endif // DIALOGGLOBAL_H
