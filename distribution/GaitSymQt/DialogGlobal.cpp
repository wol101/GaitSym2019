#include "DialogGlobal.h"
#include "ui_DialogGlobal.h"

#include "Global.h"
#include "Preferences.h"
#include "Body.h"
#include "LineEditDouble.h"
#include "LineEditPath.h"

#include <QtGlobal>
#include <QDebug>

DialogGlobal::DialogGlobal(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogGlobal)
{
    ui->setupUi(this);

    m_global = nullptr;
    m_existingBodies = nullptr;

    m_fitnessTypeMap.insert("Distance Travelled", Global::DistanceTravelled);
    m_fitnessTypeMap.insert("Kinematic Match LS", Global::KinematicMatch);
    m_fitnessTypeMap.insert("Kinematic Match Mini-Max", Global::KinematicMatchMiniMax);
    m_fitnessTypeMap.insert("Kinematic Match Continuous LS", Global::KinematicMatchContinuous);
    m_fitnessTypeMap.insert("Kinematic Match Continuous Mini-Max", Global::KinematicMatchContinuousMiniMax);
    m_fitnessTypeMap.insert("Closest Warehouse", Global::ClosestWarehouse);
    ui->comboBoxFitnessType->addItems(QStringList(m_fitnessTypeMap.keys()));

    m_stepTypeMap.insert("WorldStep", Global::WorldStep);
    m_stepTypeMap.insert("QuickStep", Global::QuickStep);
    ui->comboBoxStepType->addItems(QStringList(m_stepTypeMap.keys()));

    ui->lineEditCurrentWarehouseFile->setPathType(LineEditPath::FileForOpen);

    ui->groupBoxWarehouse->setHidden(true);
#ifdef EXPERIMENTAL
    ui->groupBoxWarehouse->setHidden(false);
#endif

    connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->checkBoxSpringDamping, SIGNAL(stateChanged(int)), this, SLOT(checkBoxSpringDampingStateChanged(int)));

    restoreGeometry(Preferences::valueQByteArray("DialogGlobalGeometry"));

}

DialogGlobal::~DialogGlobal()
{
    delete ui;
}

Global *DialogGlobal::global() const
{
    return m_global;
}

void DialogGlobal::accept() // this catches OK and return/enter
{
    qDebug() << "DialogGlobal::accept()";
    readValues();
    Preferences::insert("DialogGlobalGeometry", saveGeometry());
    QDialog::accept();
}

void DialogGlobal::reject() // this catches cancel, close and escape key
{
    qDebug() << "DialogGlobal::reject()";
    Preferences::insert("DialogGlobalGeometry", saveGeometry());
    QDialog::reject();
}

void DialogGlobal::setExistingBodies(const std::map<std::string, Body *> *existingBodies)
{
    Q_ASSERT_X(existingBodies, "DialogGlobal::setExistingBodies", "existingBodies == nullptr");
    m_existingBodies = existingBodies;
    for (auto iter : *m_existingBodies)
    {
        ui->comboBoxDistanceTravelledBodyIDName->addItem(QString::fromStdString(iter.first));
    }
}

void DialogGlobal::setGlobal(Global *global)
{
    Q_ASSERT_X(global, "DialogGlobal::setGlobal", "global not set");
    m_global = global;

    ui->comboBoxFitnessType->setCurrentIndex(static_cast<int>(m_global->fitnessType()));
    ui->comboBoxStepType->setCurrentIndex(static_cast<int>(m_global->stepType()));
    int index = ui->comboBoxDistanceTravelledBodyIDName->findData(QString::fromStdString(m_global->DistanceTravelledBodyIDName()));
    if ( index != -1 ) ui->comboBoxDistanceTravelledBodyIDName->setCurrentIndex(index); // -1 for not found

    ui->lineEditCFM->setValue(m_global->CFM());
    ui->lineEditContactMaxCorrectingVel->setValue(m_global->ContactMaxCorrectingVel());
    ui->lineEditERP->setValue(m_global->ERP());
    ui->lineEditERPContactSurfaceLayer->setValue(m_global->ContactSurfaceLayer());
    ui->lineEditFailDistanceAbort->setValue(m_global->WarehouseFailDistanceAbort());
    ui->lineEditGravityX->setValue(m_global->Gravity().x);
    ui->lineEditGravityY->setValue(m_global->Gravity().y);
    ui->lineEditGravityZ->setValue(m_global->Gravity().z);
    ui->lineEditMechanicalEnergyLimit->setValue(m_global->MechanicalEnergyLimit());
    ui->lineEditMetabolicEnergyLimit->setValue(m_global->MetabolicEnergyLimit());
    ui->lineEditStepSize->setValue(m_global->StepSize());
    ui->lineEditTimeLimit->setValue(m_global->TimeLimit());
    ui->lineEditUnitIncreaseDistanceThreshold->setValue(
        m_global->WarehouseUnitIncreaseDistanceThreshold());
    ui->lineEditWarehouseDecreaseThresholdFactor->setValue(
        m_global->WarehouseDecreaseThresholdFactor());
    ui->lineEditCurrentWarehouseFile->setText(QString::fromStdString(m_global->CurrentWarehouseFile()));
    ui->checkBoxAllowConnectedCollisions->setChecked(m_global->AllowConnectedCollisions());
    ui->checkBoxAllowInternalCollisions->setChecked(m_global->AllowInternalCollisions());
}

void DialogGlobal::readValues()
{
    m_global->setFitnessType(static_cast<Global::FitnessType>(ui->comboBoxFitnessType->currentIndex()));
    m_global->setStepType(static_cast<Global::StepType>(ui->comboBoxStepType->currentIndex()));
    m_global->setDistanceTravelledBodyIDName(
        ui->comboBoxDistanceTravelledBodyIDName->currentText().toStdString());
    m_global->setContactMaxCorrectingVel(ui->lineEditContactMaxCorrectingVel->value());
    m_global->setContactSurfaceLayer(ui->lineEditERPContactSurfaceLayer->value());
    m_global->setWarehouseFailDistanceAbort(ui->lineEditFailDistanceAbort->value());
    m_global->setGravity(ui->lineEditGravityX->value(), ui->lineEditGravityY->value(),
                         ui->lineEditGravityZ->value());
    m_global->setMechanicalEnergyLimit(ui->lineEditMechanicalEnergyLimit->value());
    m_global->setMetabolicEnergyLimit(ui->lineEditMetabolicEnergyLimit->value());
    m_global->setStepSize(ui->lineEditStepSize->value());
    m_global->setTimeLimit(ui->lineEditTimeLimit->value());
    m_global->setWarehouseUnitIncreaseDistanceThreshold(
        ui->lineEditUnitIncreaseDistanceThreshold->value());
    m_global->setWarehouseDecreaseThresholdFactor(
        ui->lineEditWarehouseDecreaseThresholdFactor->value());
    m_global->setCurrentWarehouseFile(ui->lineEditCurrentWarehouseFile->text().toStdString());
    m_global->setAllowConnectedCollisions(ui->checkBoxAllowConnectedCollisions->isChecked());
    m_global->setAllowInternalCollisions(ui->checkBoxAllowInternalCollisions->isChecked());

    if (ui->checkBoxSpringDamping->isChecked())
    {
        double spring_constant = ui->lineEditCFM->value();
        double damping_constant = ui->lineEditERP->value();
        double integration_stepsize = ui->lineEditStepSize->value();
        double cfm, erp;
        ConvertToCFMERP(spring_constant, damping_constant, integration_stepsize, &cfm, &erp);
        m_global->setCFM(cfm);
        m_global->setERP(erp);
        m_global->setSpringConstant(spring_constant);
        m_global->setDampingConstant(damping_constant);
    }
    else
    {
        double cfm = ui->lineEditCFM->value();
        double erp = ui->lineEditERP->value();
        double integration_stepsize = ui->lineEditStepSize->value();
        double spring_constant, damping_constant;
        ConvertToSpringAndDampingConstants(erp, cfm, integration_stepsize, &spring_constant,
                                           &damping_constant);
        m_global->setCFM(cfm);
        m_global->setERP(erp);
        m_global->setSpringConstant(spring_constant);
        m_global->setDampingConstant(damping_constant);
    }
}

void DialogGlobal::checkBoxSpringDampingStateChanged(int /* state */)
{
    if (ui->checkBoxSpringDamping->isChecked())
    {
        ui->labelCFM->setText("Spring");
        ui->labelERP->setText("Damp");
        double cfm = ui->lineEditCFM->value();
        double erp = ui->lineEditERP->value();
        double integration_stepsize = ui->lineEditStepSize->value();
        double spring_constant, damping_constant;
        ConvertToSpringAndDampingConstants(erp, cfm, integration_stepsize, &spring_constant,
                                           &damping_constant);
        ui->lineEditCFM->setValue(spring_constant);
        ui->lineEditERP->setValue(damping_constant);
    }
    else
    {
        ui->labelCFM->setText("CFM");
        ui->labelERP->setText("ERP");
        double spring_constant = ui->lineEditCFM->value();
        double damping_constant = ui->lineEditERP->value();
        double integration_stepsize = ui->lineEditStepSize->value();
        double cfm, erp;
        ConvertToCFMERP(spring_constant, damping_constant, integration_stepsize, &cfm, &erp);
        ui->lineEditCFM->setValue(cfm);
        ui->lineEditERP->setValue(erp);
    }
}

void DialogGlobal::ConvertToCFMERP(double spring_constant, double damping_constant,
                                   double integration_stepsize, double *cfm, double *erp)
{
    // naive version could cause divide by zero errors
    // *erp = (integration_stepsize * spring_constant) / ((integration_stepsize * spring_constant) + damping_constant);
    // *cfm = 1.0 / ((integration_stepsize * spring_constant) + damping_constant);
    double erp_denom = ((integration_stepsize * spring_constant) + damping_constant);
    if (std::abs(erp_denom) > std::numeric_limits<double>::min())
    {
        *erp = (integration_stepsize * spring_constant) / ((integration_stepsize * spring_constant) +
                damping_constant);
        *cfm = 1.0 / ((integration_stepsize * spring_constant) + damping_constant);
    }
    else
    {
        *erp = Preferences::valueDouble("GlobalDefaultERP");
        *cfm = Preferences::valueDouble("GlobalDefaultCFM");
    }
    return;
}

void DialogGlobal::ConvertToSpringAndDampingConstants(double erp, double cfm,
        double integration_stepsize, double *spring_constant, double *damping_constant)
{
    // naive version could cause divide by zero errors
    // *spring_constant = erp / (cfm * integration_stepsize);
    // *damping_constant = (1.0 - erp) / cfm;
    if (std::abs(cfm * integration_stepsize) > std::numeric_limits<double>::min())
    {
        *spring_constant = erp / (cfm * integration_stepsize);
        *damping_constant = (1.0 - erp) / cfm;
    }
    else
    {
        integration_stepsize = Preferences::valueDouble("GlobalDefaultStepSize");
        erp = Preferences::valueDouble("GlobalDefaultERP");
        cfm = Preferences::valueDouble("GlobalDefaultCFM");
        *spring_constant = erp / (cfm * integration_stepsize);
        *damping_constant = (1.0 - erp) / cfm;
    }
    return;
}


