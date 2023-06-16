#include "DialogGlobal.h"
#include "ui_DialogGlobal.h"

#include "Global.h"
#include "Preferences.h"
#include "Body.h"
#include "LineEditDouble.h"
#include "LineEditPath.h"
#include "DialogProperties.h"

#include "pystring.h"

#include <QtGlobal>
#include <QDebug>
#include <QStandardItemModel>

using namespace std::string_literals;

DialogGlobal::DialogGlobal(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogGlobal)
{
    ui->setupUi(this);
    setWindowTitle(tr("Global Builder"));
#ifdef Q_OS_MACOS
    setWindowFlags(windowFlags() & (~Qt::Dialog) | Qt::Window); // allows the window to be resized on macs
#endif

    initialiseDefaultGlobal();
    ui->lineEditCurrentWarehouseFile->setPathType(LineEditPath::FileForOpen);

    ui->groupBoxWarehouse->setHidden(true);
#ifdef EXPERIMENTAL
    ui->groupBoxWarehouse->setHidden(false);
#endif

    connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->pushButtonProperties, SIGNAL(clicked()), this, SLOT(properties()));
    connect(ui->pushButtonDefaults, SIGNAL(clicked()), this, SLOT(setDefaults()));
    connect(ui->checkBoxSpringDamping, SIGNAL(stateChanged(int)), this, SLOT(checkBoxSpringDampingStateChanged(int)));

    restoreGeometry(Preferences::valueQByteArray("DialogGlobalGeometry"));

}

DialogGlobal::~DialogGlobal()
{
    delete ui;
}

void DialogGlobal::accept() // this catches OK and return/enter
{
    qDebug() << "DialogGlobal::accept()";
    m_outputGlobal = std::make_unique<Global>();
    m_outputGlobal->setFitnessType(static_cast<Global::FitnessType>(ui->comboBoxFitnessType->currentIndex()));
    m_outputGlobal->setStepType(static_cast<Global::StepType>(ui->comboBoxStepType->currentIndex()));
    m_outputGlobal->setDistanceTravelledBodyIDName(ui->comboBoxDistanceTravelledBodyIDName->currentText().toStdString());
    m_outputGlobal->setContactMaxCorrectingVel(ui->lineEditContactMaxCorrectingVel->value());
    m_outputGlobal->setContactSurfaceLayer(ui->lineEditContactSurfaceLayer->value());
    m_outputGlobal->setWarehouseFailDistanceAbort(ui->lineEditFailDistanceAbort->value());
    m_outputGlobal->setGravity(ui->lineEditGravityX->value(), ui->lineEditGravityY->value(), ui->lineEditGravityZ->value());
    m_outputGlobal->setMechanicalEnergyLimit(ui->lineEditMechanicalEnergyLimit->value());
    m_outputGlobal->setMetabolicEnergyLimit(ui->lineEditMetabolicEnergyLimit->value());
    m_outputGlobal->setStepSize(ui->lineEditStepSize->value());
    m_outputGlobal->setTimeLimit(ui->lineEditTimeLimit->value());
    m_outputGlobal->setNumericalErrorsScore(ui->lineEditNumericalErrorScore->value());
    m_outputGlobal->setWarehouseUnitIncreaseDistanceThreshold(ui->lineEditUnitIncreaseDistanceThreshold->value());
    m_outputGlobal->setWarehouseDecreaseThresholdFactor(ui->lineEditWarehouseDecreaseThresholdFactor->value());
    m_outputGlobal->setLinearDamping(ui->lineEditLinearDamping->value());
    m_outputGlobal->setAngularDamping(ui->lineEditAngularDamping->value());
    m_outputGlobal->setCurrentWarehouseFile(ui->lineEditCurrentWarehouseFile->text().toStdString());
    m_outputGlobal->setAllowConnectedCollisions(ui->checkBoxAllowConnectedCollisions->isChecked());
    m_outputGlobal->setAllowInternalCollisions(ui->checkBoxAllowInternalCollisions->isChecked());
    m_outputGlobal->setPermittedNumericalErrors(ui->spinBoxPermittedErrorCount->value());

    m_outputGlobal->setName("Global");

    if (ui->checkBoxSpringDamping->isChecked())
    {
        double spring_constant = ui->lineEditCFM->value();
        double damping_constant = ui->lineEditERP->value();
        double integration_stepsize = ui->lineEditStepSize->value();
        double cfm, erp;
        ConvertToCFMERP(spring_constant, damping_constant, integration_stepsize, &cfm, &erp);
        m_outputGlobal->setCFM(cfm);
        m_outputGlobal->setERP(erp);
        m_outputGlobal->setSpringConstant(spring_constant);
        m_outputGlobal->setDampingConstant(damping_constant);
    }
    else
    {
        double cfm = ui->lineEditCFM->value();
        double erp = ui->lineEditERP->value();
        double integration_stepsize = ui->lineEditStepSize->value();
        double spring_constant, damping_constant;
        ConvertToSpringAndDampingConstants(erp, cfm, integration_stepsize, &spring_constant, &damping_constant);
        m_outputGlobal->setCFM(cfm);
        m_outputGlobal->setERP(erp);
        m_outputGlobal->setSpringConstant(spring_constant);
        m_outputGlobal->setDampingConstant(damping_constant);
    }

    int count = ui->listWidgetMeshPath->count();
    m_outputGlobal->MeshSearchPath()->clear();
    for (int i = 0; i < count; i++)
    {
        QString itemText = ui->listWidgetMeshPath->item(i)->text();
        if (itemText.size()) m_outputGlobal->MeshSearchPath()->push_back(itemText.toStdString());
    }

    if (m_inputGlobal)
    {
        m_outputGlobal->setColour1(m_inputGlobal->colour1());
        m_outputGlobal->setSize1(m_inputGlobal->size1());
    }
    else
    {
        m_outputGlobal->setColour1(Preferences::valueQColor("BackgroundColour").name(QColor::HexArgb).toStdString());
        m_outputGlobal->setSize1(Preferences::valueDouble("GlobalAxesSize"));
    }

    if (m_properties.size() > 0)
    {
        if (m_properties.count("BackgroundColour"))
            m_outputGlobal->setColour1(qvariant_cast<QColor>(m_properties["BackgroundColour"].value).name(QColor::HexArgb).toStdString());
        if (m_properties.count("GlobalAxesSize"))
            m_outputGlobal->setSize1(m_properties["GlobalAxesSize"].value.toDouble());
    }

    Preferences::insert("DialogGlobalGeometry", saveGeometry());
    QDialog::accept();
}

void DialogGlobal::reject() // this catches cancel, close and escape key
{
    qDebug() << "DialogGlobal::reject()";
    Preferences::insert("DialogGlobalGeometry", saveGeometry());
    QDialog::reject();
}

void DialogGlobal::closeEvent(QCloseEvent *event)
{
    Preferences::insert("DialogGlobalGeometry", saveGeometry());

    QDialog::closeEvent(event);
}

void DialogGlobal::lateInitialise()
{
    if (m_inputGlobal) updateUI(m_inputGlobal);
    else updateUI(&m_defaultGlobal);
}

void DialogGlobal::setDefaults()
{
    updateUI(&m_defaultGlobal);
}

void DialogGlobal::updateUI(const Global *globalPtr)
{
    // assign the QComboBox items
    for (size_t i = 0; i < Global::fitnessTypeCount; i++) ui->comboBoxFitnessType->addItem(globalPtr->fitnessTypeStrings(i));
    for (size_t i = 0; i < Global::stepTypeCount; i++) ui->comboBoxStepType->addItem(globalPtr->stepTypeStrings(i));

    if (m_existingBodies == nullptr || m_existingBodies->size() == 0)
    {
        // disable incompatible items
        QStandardItemModel *model = qobject_cast<QStandardItemModel *>(ui->comboBoxFitnessType->model());
        Q_ASSERT_X(model != nullptr, "DialogGlobal::lateInitialise", "qobject_cast<QStandardItemModel *> failed");
        bool disabled = true;
        QStandardItem *item = model->item(int(Global::KinematicMatch));
        item->setFlags(disabled ? item->flags() & ~Qt::ItemIsEnabled : item->flags() | Qt::ItemIsEnabled);
    }
    else
    {
        for (auto &&iter : *m_existingBodies) ui->comboBoxDistanceTravelledBodyIDName->addItem(QString::fromStdString(iter.first));
        QString distanceTravelledBodyID = QString::fromStdString(globalPtr->DistanceTravelledBodyIDName());
        ui->comboBoxDistanceTravelledBodyIDName->setCurrentText(distanceTravelledBodyID);
    }

    ui->comboBoxFitnessType->setCurrentIndex(static_cast<int>(globalPtr->fitnessType()));
    ui->comboBoxStepType->setCurrentIndex(static_cast<int>(globalPtr->stepType()));

    ui->lineEditCFM->setValue(globalPtr->CFM());
    ui->lineEditContactMaxCorrectingVel->setValue(globalPtr->ContactMaxCorrectingVel());
    ui->lineEditERP->setValue(globalPtr->ERP());
    ui->lineEditContactSurfaceLayer->setValue(globalPtr->ContactSurfaceLayer());
    ui->lineEditFailDistanceAbort->setValue(globalPtr->WarehouseFailDistanceAbort());
    ui->lineEditGravityX->setValue(globalPtr->Gravity().x);
    ui->lineEditGravityY->setValue(globalPtr->Gravity().y);
    ui->lineEditGravityZ->setValue(globalPtr->Gravity().z);
    ui->lineEditMechanicalEnergyLimit->setValue(globalPtr->MechanicalEnergyLimit());
    ui->lineEditMetabolicEnergyLimit->setValue(globalPtr->MetabolicEnergyLimit());
    ui->lineEditStepSize->setValue(globalPtr->StepSize());
    ui->lineEditTimeLimit->setValue(globalPtr->TimeLimit());
    ui->lineEditNumericalErrorScore->setValue(globalPtr->NumericalErrorsScore());
    ui->lineEditUnitIncreaseDistanceThreshold->setValue(globalPtr->WarehouseUnitIncreaseDistanceThreshold());
    ui->lineEditWarehouseDecreaseThresholdFactor->setValue(globalPtr->WarehouseDecreaseThresholdFactor());
    ui->lineEditLinearDamping->setValue(globalPtr->LinearDamping());
    ui->lineEditAngularDamping->setValue(globalPtr->AngularDamping());
    ui->lineEditCurrentWarehouseFile->setText(QString::fromStdString(globalPtr->CurrentWarehouseFile()));
    ui->checkBoxAllowConnectedCollisions->setChecked(globalPtr->AllowConnectedCollisions());
    ui->checkBoxAllowInternalCollisions->setChecked(globalPtr->AllowInternalCollisions());
    ui->spinBoxPermittedErrorCount->setValue(globalPtr->PermittedNumericalErrors());

    ui->listWidgetMeshPath->clear();
    for (size_t i = 0; i < globalPtr->ConstMeshSearchPath()->size(); i++)
    {
        QListWidgetItem *item = new QListWidgetItem(QString::fromStdString(globalPtr->ConstMeshSearchPath()->at(i)));
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->listWidgetMeshPath->addItem(item);
    }
    for (size_t i = globalPtr->ConstMeshSearchPath()->size(); i < 100; i++)
    {
        QListWidgetItem *item = new QListWidgetItem(QString());
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->listWidgetMeshPath->addItem(item);
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
        ConvertToSpringAndDampingConstants(erp, cfm, integration_stepsize, &spring_constant, &damping_constant);
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

void DialogGlobal::ConvertToCFMERP(double spring_constant, double damping_constant, double integration_stepsize, double *cfm, double *erp)
{
    // naive version could cause divide by zero errors
    // *erp = (integration_stepsize * spring_constant) / ((integration_stepsize * spring_constant) + damping_constant);
    // *cfm = 1.0 / ((integration_stepsize * spring_constant) + damping_constant);
    double erp_denom = ((integration_stepsize * spring_constant) + damping_constant);
    if (std::abs(erp_denom) > std::numeric_limits<double>::min())
    {
        *erp = (integration_stepsize * spring_constant) / ((integration_stepsize * spring_constant) + damping_constant);
        *cfm = 1.0 / ((integration_stepsize * spring_constant) + damping_constant);
    }
    else
    {
        *erp = Preferences::valueDouble("GlobalDefaultERP");
        *cfm = Preferences::valueDouble("GlobalDefaultCFM");
    }
    return;
}

void DialogGlobal::ConvertToSpringAndDampingConstants(double erp, double cfm, double integration_stepsize, double *spring_constant, double *damping_constant)
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

void DialogGlobal::properties()
{
    DialogProperties dialogProperties(this);

    SettingsItem globalAxesSize = Preferences::settingsItem("GlobalAxesSize");
    SettingsItem backgroundColour = Preferences::settingsItem("BackgroundColour");
    if (m_inputGlobal)
    {
        globalAxesSize.value = m_inputGlobal->size1();
        backgroundColour.value = QColor(QString::fromStdString(m_inputGlobal->colour1().GetHexArgb()));
    }
    m_properties.clear();
    m_properties = { { globalAxesSize.key, globalAxesSize },
                     { backgroundColour.key, backgroundColour } };

    dialogProperties.setInputSettingsItems(m_properties);
    dialogProperties.initialise();

    int status = dialogProperties.exec();
    if (status == QDialog::Accepted)
    {
        dialogProperties.update();
        m_properties = dialogProperties.getOutputSettingsItems();
    }
}

std::unique_ptr<Global> DialogGlobal::outputGlobal()
{
    return std::move(m_outputGlobal);
}

void DialogGlobal::setInputGlobal(const Global *inputGlobal)
{
    m_inputGlobal = inputGlobal;
}

void DialogGlobal::setExistingBodies(const std::map<std::string, std::unique_ptr<Body>> *existingBodies)
{
    m_existingBodies = existingBodies;
}

void DialogGlobal::initialiseDefaultGlobal()
{
    for (size_t i = 0; i < Global::fitnessTypeCount; i++)
    {
        if (Preferences::valueQString("GlobalDefaultFitnessType") == Global::fitnessTypeStrings(i))
        {
            m_defaultGlobal.setFitnessType(static_cast<Global::FitnessType>(i));
            break;
        }
    }
    for (size_t i = 0; i < Global::stepTypeCount; i++)
    {
        if (Preferences::valueQString("GlobalDefaultStepType") == Global::stepTypeStrings(i))
        {
            m_defaultGlobal.setStepType(static_cast<Global::StepType>(i));
            break;
        }
    }
    m_defaultGlobal.setAllowConnectedCollisions(Preferences::valueBool("GlobalDefaultAllowConnectedCollisions"));
    m_defaultGlobal.setAllowInternalCollisions(Preferences::valueBool("GlobalDefaultAllowInternalCollisions"));
    m_defaultGlobal.setPermittedNumericalErrors(Preferences::valueBool("GlobalDefaultPermittedNumericalErrors"));
    m_defaultGlobal.setGravity(Preferences::valueDouble("GlobalDefaultGravityX"), Preferences::valueDouble("GlobalDefaultGravityY"), Preferences::valueDouble("GlobalDefaultGravityZ"));
    m_defaultGlobal.setBMR(Preferences::valueDouble("GlobalDefaultBMR"));
    m_defaultGlobal.setCFM(Preferences::valueDouble("GlobalDefaultCFM"));
    m_defaultGlobal.setContactMaxCorrectingVel(Preferences::valueDouble("GlobalDefaultContactMaxCorrectingVel"));
    m_defaultGlobal.setContactSurfaceLayer(Preferences::valueDouble("GlobalDefaultContactSurfaceLayer"));
    m_defaultGlobal.setDampingConstant(Preferences::valueDouble("GlobalDefaultDampingConstant"));
    m_defaultGlobal.setERP(Preferences::valueDouble("GlobalDefaultERP"));
    m_defaultGlobal.setMechanicalEnergyLimit(Preferences::valueDouble("GlobalDefaultMechanicalEnergyLimit"));
    m_defaultGlobal.setMetabolicEnergyLimit(Preferences::valueDouble("GlobalDefaultMetabolicEnergyLimit"));
    m_defaultGlobal.setSpringConstant(Preferences::valueDouble("GlobalDefaultSpringConstant"));
    m_defaultGlobal.setStepSize(Preferences::valueDouble("GlobalDefaultStepSize"));
    m_defaultGlobal.setTimeLimit(Preferences::valueDouble("GlobalDefaultTimeLimit"));
    m_defaultGlobal.setWarehouseDecreaseThresholdFactor(Preferences::valueDouble("GlobalDefaultWarehouseDecreaseThresholdFactor"));
    m_defaultGlobal.setWarehouseFailDistanceAbort(Preferences::valueDouble("GlobalDefaultWarehouseFailDistanceAbort"));
    m_defaultGlobal.setWarehouseUnitIncreaseDistanceThreshold(Preferences::valueDouble("GlobalDefaultWarehouseUnitIncreaseDistanceThreshold"));
    m_defaultGlobal.setLinearDamping(Preferences::valueDouble("GlobalDefaultLinearDamping"));
    m_defaultGlobal.setAngularDamping(Preferences::valueDouble("GlobalDefaultAngularDamping"));
    m_defaultGlobal.setNumericalErrorsScore(Preferences::valueDouble("GlobalDefaultNumericalErrorsScore"));
    m_defaultGlobal.setCurrentWarehouseFile(Preferences::valueQString("GlobalDefaultCurrentWarehouseFile").toStdString());
    m_defaultGlobal.setDistanceTravelledBodyIDName(Preferences::valueQString("GlobalDefaultDistanceTravelledBodyIDName").toStdString());

    m_defaultGlobal.MeshSearchPath()->clear();
    std::string buf = Preferences::valueQString("GlobalDefaultMeshSearchPath").toStdString();
    std::vector<std::string> encodedMeshSearchPath;
    if (buf.size())
    {
        pystring::split(buf, encodedMeshSearchPath, ":"s);
        for (size_t i = 0; i < encodedMeshSearchPath.size(); i++) m_defaultGlobal.MeshSearchPath()->push_back(Global::percentDecode(encodedMeshSearchPath[i]));
    }
}

