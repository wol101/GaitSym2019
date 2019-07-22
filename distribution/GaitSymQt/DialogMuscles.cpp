#include "DialogMuscles.h"
#include "ui_DialogMuscles.h"

#include "Simulation.h"
#include "Muscle.h"
#include "MAMuscle.h"
#include "MAMuscleComplete.h"
#include "DampedSpringMuscle.h"
#include "TwoPointStrap.h"
#include "NPointStrap.h"
#include "CylinderWrapStrap.h"
#include "TwoCylinderWrapStrap.h"
#include "Preferences.h"
#include "GSUtil.h"

#include "pystring.h"

#include <QDebug>
#include <QScrollArea>
#include <QSpacerItem>

#include <string>
using namespace std::string_literals; // enables s-suffix for std::string literals

DialogMuscles::DialogMuscles(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogMuscles)
{
    ui->setupUi(this);

    setWindowTitle(tr("Muscle Builder"));
#ifdef Q_OS_MACOS
    setWindowFlags(windowFlags() & (~Qt::Dialog) |
                   Qt::Window); // allows the window to be resized on macs
#endif
    restoreGeometry(Preferences::valueQByteArray("DialogMusclesGeometry"));

    QVBoxLayout *verticalLayout;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    verticalLayout = new QVBoxLayout();
    verticalLayout->setSpacing(6);
    verticalLayout->setContentsMargins(11, 11, 11, 11);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    scrollArea = new QScrollArea();
    scrollArea->setObjectName(QStringLiteral("scrollArea"));
    scrollArea->setWidgetResizable(true);
    scrollAreaWidgetContents = new QWidget();
    scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
    m_gridLayout = new QGridLayout();
    m_gridLayout->setSpacing(6);
    m_gridLayout->setContentsMargins(11, 11, 11, 11);
    m_gridLayout->setObjectName(QStringLiteral("gridLayout"));
    scrollAreaWidgetContents->setLayout(m_gridLayout);
    scrollArea->setWidget(scrollAreaWidgetContents);
    verticalLayout->addWidget(scrollArea);
    ui->widgetViaPointsPlaceholder->setLayout(verticalLayout);

    connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->tabWidgetMuscle, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
    connect(ui->tabWidgetStrap, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

    // this logic monitors for changing values
    QList<QWidget *> widgets = this->findChildren<QWidget *>();
    for (auto it = widgets.begin(); it != widgets.end(); it++)
    {
        QComboBox *comboBox = dynamic_cast<QComboBox *>(*it);
        if (comboBox) connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxChanged(int)));
        QLineEdit *lineEdit = dynamic_cast<QLineEdit *>(*it);
        if (lineEdit) connect(lineEdit, SIGNAL(textChanged(const QString &)), this,
                                  SLOT(lineEditChanged(const QString &)));
        QSpinBox *spinBox = dynamic_cast<QSpinBox *>(*it);
        if (spinBox) connect(spinBox, SIGNAL(valueChanged(const QString &)), this,
                                 SLOT(spinBoxChanged(const QString &)));
        QCheckBox *checkBox = dynamic_cast<QCheckBox *>(*it);
        if (checkBox) connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(checkBoxChanged(int)));
    }

    ui->pushButtonOK->setEnabled(false);
}

DialogMuscles::~DialogMuscles()
{
    delete ui;
}

void DialogMuscles::accept() // this catches OK and return/enter
{
    qDebug() << "DialogMuscles::accept()";
    if (m_origMuscle)
    {
        delete m_origMuscle->GetStrap();
        delete m_origMuscle;
    }
    std::map<std::string, Marker *> *markerList = m_simulation->GetMarkerList();
    QString strapTab = ui->tabWidgetStrap->tabText(ui->tabWidgetStrap->currentIndex());
    Strap *strap = nullptr;
    if (strapTab == "N-Point")
    {
        if (ui->spinBoxNViaPoints->value() == 0)
        {
            strap = new TwoPointStrap();
            Marker *originMarker = markerList->at(ui->comboBoxOriginMarker->currentText().toStdString());
            Marker *insertionMarker = markerList->at(ui->comboBoxInsertionMarker->currentText().toStdString());
            reinterpret_cast<TwoPointStrap *>(strap)->SetOrigin(originMarker);
            reinterpret_cast<TwoPointStrap *>(strap)->SetInsertion(insertionMarker);
        }
        else
        {
            strap = new NPointStrap();
            Marker *originMarker = markerList->at(ui->comboBoxOriginMarker->currentText().toStdString());
            Marker *insertionMarker = markerList->at(ui->comboBoxInsertionMarker->currentText().toStdString());
            reinterpret_cast<NPointStrap *>(strap)->SetOrigin(originMarker);
            reinterpret_cast<NPointStrap *>(strap)->SetInsertion(insertionMarker);
            std::vector<Marker *>viaPointMarkerList;
            viaPointMarkerList.reserve(size_t(ui->spinBoxNViaPoints->value()));
            for (int i = 0; i < ui->spinBoxNViaPoints->value();
                    i++) viaPointMarkerList.push_back(markerList->at(
                                    m_viaPointComboBoxList[i]->currentText().toStdString()));
            reinterpret_cast<NPointStrap *>(strap)->SetViaPoints(&viaPointMarkerList);
        }
    }
    else if (strapTab == "Cylinder")
    {
        strap = new CylinderWrapStrap();
        Marker *originMarker = markerList->at(ui->comboBoxOriginMarker->currentText().toStdString());
        Marker *insertionMarker = markerList->at(ui->comboBoxInsertionMarker->currentText().toStdString());
        reinterpret_cast<CylinderWrapStrap *>(strap)->SetOrigin(originMarker);
        reinterpret_cast<CylinderWrapStrap *>(strap)->SetInsertion(insertionMarker);
        Marker *cylinderMarker = markerList->at(ui->comboBoxCylinderMarker->currentText().toStdString());
        reinterpret_cast<CylinderWrapStrap *>(strap)->SetCylinder(cylinderMarker);
        reinterpret_cast<CylinderWrapStrap *>(strap)->SetCylinderRadius(
            ui->lineEditCylinderRadius->value());
    }
    else if (strapTab == "2-Cylinder")
    {
        strap = new TwoCylinderWrapStrap();
        Marker *originMarker = markerList->at(ui->comboBoxOriginMarker->currentText().toStdString());
        Marker *insertionMarker = markerList->at(ui->comboBoxInsertionMarker->currentText().toStdString());
        reinterpret_cast<TwoCylinderWrapStrap *>(strap)->SetOrigin(originMarker);
        reinterpret_cast<TwoCylinderWrapStrap *>(strap)->SetInsertion(insertionMarker);
        Marker *cylinder1Marker = markerList->at(ui->comboBox2Cylinder1Marker->currentText().toStdString());
        reinterpret_cast<TwoCylinderWrapStrap *>(strap)->SetCylinder1(cylinder1Marker);
        reinterpret_cast<TwoCylinderWrapStrap *>(strap)->SetCylinder1Radius(
            ui->lineEdit2Cylinder1Radius->value());
        Marker *cylinder2Marker = markerList->at(ui->comboBox2Cylinder2Marker->currentText().toStdString());
        reinterpret_cast<TwoCylinderWrapStrap *>(strap)->SetCylinder2(cylinder2Marker);
        reinterpret_cast<TwoCylinderWrapStrap *>(strap)->SetCylinder2Radius(
            ui->lineEdit2Cylinder2Radius->value());
    }

    QString muscleTab = ui->tabWidgetMuscle->tabText(ui->tabWidgetMuscle->currentIndex());
    if (muscleTab == "Minetti-Alexander")
    {
        MAMuscle *muscle = new MAMuscle();
        muscle->SetStrap(strap);
        double forcePerUnitArea = ui->lineEditForcePerUnitArea->value();
        double vMaxFactor = ui->lineEditVMaxFactor->value();
        double pca = ui->lineEditPCA->value();
        double fibreLength = ui->lineEditFibreLength->value();
        double activationK = ui->lineEditActivationK->value();
        muscle->SetF0(pca * forcePerUnitArea);
        muscle->SetVMax(fibreLength * vMaxFactor);
        muscle->SetK(activationK);
        m_muscle = muscle;
    }
    else if (muscleTab == "Minetti-Alexander Elastic")
    {
        MAMuscleComplete *muscle = new MAMuscleComplete();
        muscle->SetStrap(strap);
        double forcePerUnitArea = ui->lineEditForcePerUnitArea_2->value();
        double vMaxFactor = ui->lineEditVMaxFactor_2->value();
        double pca = ui->lineEditPCA_2->value();
        double fibreLength = ui->lineEditFibreLength_2->value();
        double activationK = ui->lineEditActivationK_2->value();
        double width = ui->lineEditWidth->value();
        double tendonLength = ui->lineEditTendonLength->value();
        double serialStrainAtFmax = ui->lineEditSerialStrainAtFMax->value();
        double serialStrainRateAtFmax = ui->lineEditSerialStrainRateAtFMax->value();
        QMap<QString, MAMuscleComplete::StrainModel> strainModelMap{{"Linear", MAMuscleComplete::linear}, {"Square", MAMuscleComplete::square}};
        MAMuscleComplete::StrainModel serialStrainModel =
            strainModelMap[ui->comboBoxSerialStrainModel->currentText()];
        double parallelStrainAtFmax = ui->lineEditParallelStrainAtFMax->value();
        double parallelStrainRateAtFmax = ui->lineEditParallelStrainRateAtFMax->value();
        MAMuscleComplete::StrainModel parallelStrainModel =
            strainModelMap[ui->comboBoxParallelStrainModel->currentText()];
        double parallelElementLength = fibreLength;
        double vMax = fibreLength * vMaxFactor;
        double fMax = pca * forcePerUnitArea;
        double initialFibreLength = ui->lineEditInitialFibreLength->value();
        bool activationKinetics = ui->checkBoxUseActivation->isChecked();
        double akFastTwitchProportion = ui->lineEditFastTwitchProportion->value();
        double akTActivationA = ui->lineEditTActivationA->value();
        double akTActivationB = ui->lineEditTActivationB->value();
        double akTDeactivationA = ui->lineEditTDeactivationA->value();
        double akTDeactivationB = ui->lineEditTDeactivationB->value();
        double activationRate = ui->lineEditActivationRate->value();
        double startActivation = ui->lineEditInitialActivation->value();
        double minimumActivation = ui->lineEditMinimumActivation->value();

        muscle->SetSerialElasticProperties(serialStrainAtFmax, serialStrainRateAtFmax, tendonLength,
                                           serialStrainModel);
        muscle->SetParallelElasticProperties(parallelStrainAtFmax, parallelStrainRateAtFmax,
                                             parallelElementLength, parallelStrainModel);
        muscle->SetMuscleProperties(vMax, fMax, activationK, width);
        muscle->SetActivationKinetics(activationKinetics, akFastTwitchProportion, akTActivationA,
                                      akTActivationB, akTDeactivationA, akTDeactivationB);
        muscle->SetInitialFibreLength(initialFibreLength);
        muscle->SetActivationRate(activationRate);
        muscle->SetStartActivation(startActivation);
        muscle->SetMinimumActivation(minimumActivation);
        m_muscle = muscle;
    }
    else if (muscleTab == "Damped Spring")
    {
        DampedSpringMuscle *muscle = new DampedSpringMuscle();
        muscle->SetStrap(strap);
        double unloadedLength = ui->lineEditUnloadedLength->value();
        double springConstant = ui->lineEditSpringConstant->value();
        double area = ui->lineEditArea->value();
        double damping = ui->lineEditBreakingStrain->value();
        muscle->SetUnloadedLength(unloadedLength);
        muscle->SetSpringConstant(springConstant);
        muscle->SetArea(area);
        muscle->SetDamping(damping);
        if (ui->lineEditDamping->text().size()) muscle->SetBreakingStrain(ui->lineEditDamping->value());
        m_muscle = muscle;
    }

    m_muscle->SetName(ui->lineEditMuscleID->text().toStdString());
    m_muscle->GetStrap()->SetName(ui->lineEditMuscleID->text().toStdString() + "_strap"s);
    m_muscle->setSimulation(m_simulation);

    Preferences::insert("DialogMusclesGeometry", saveGeometry());
    QDialog::accept();
}

void DialogMuscles::reject() // this catches cancel, close and escape key
{
    qDebug() << "DialogMuscles::reject()";
    Preferences::insert("DialogMusclesGeometry", saveGeometry());
    QDialog::reject();
}

void DialogMuscles::lateInitialise()
{
    Q_ASSERT_X(m_simulation, "DialogMuscles::lateInitialise", "simulation undefined");
    m_origMuscle = m_muscle;
    // get the lists in the right formats
    std::map<std::string, Marker *> *markerList = m_simulation->GetMarkerList();
    QStringList markerIDs;
    for (auto it = markerList->begin(); it != markerList->end(); it++)
        markerIDs.append(QString::fromStdString(it->first));
    // set the marker lists
    ui->comboBoxOriginMarker->addItems(markerIDs);
    ui->comboBoxInsertionMarker->addItems(markerIDs);
    ui->comboBoxCylinderMarker->addItems(markerIDs);
    ui->comboBox2Cylinder1Marker->addItems(markerIDs);
    ui->comboBox2Cylinder2Marker->addItems(markerIDs);

    // now set some sensible defaults
    ui->lineEditForcePerUnitArea->setBottom(0);
    ui->lineEditForcePerUnitArea->setValue(300000);
    ui->lineEditVMaxFactor->setBottom(0);
    ui->lineEditVMaxFactor->setValue(8.4);
    ui->lineEditPCA->setBottom(0);
    ui->lineEditPCA->setValue(1);
    ui->lineEditFibreLength->setBottom(0);
    ui->lineEditFibreLength->setValue(1);
    ui->lineEditActivationK->setBottom(0);
    ui->lineEditActivationK->setValue(0.17);

    ui->lineEditForcePerUnitArea_2->setBottom(0);
    ui->lineEditForcePerUnitArea_2->setValue(300000);
    ui->lineEditVMaxFactor_2->setBottom(0);
    ui->lineEditVMaxFactor_2->setValue(8.4);
    ui->lineEditPCA_2->setBottom(0);
    ui->lineEditPCA_2->setValue(1);
    ui->lineEditFibreLength_2->setBottom(0);
    ui->lineEditFibreLength_2->setValue(1);
    ui->lineEditActivationK_2->setBottom(0);
    ui->lineEditActivationK_2->setValue(0.17);
    ui->lineEditWidth->setBottom(0);
    ui->lineEditWidth->setValue(0.5);
    ui->lineEditTendonLength->setBottom(0);
    ui->lineEditTendonLength->setValue(1);
    ui->lineEditSerialStrainAtFMax->setBottom(0);
    ui->lineEditSerialStrainAtFMax->setValue(0.06);
    ui->lineEditSerialStrainRateAtFMax->setBottom(0);
    ui->lineEditSerialStrainRateAtFMax->setValue(0);
    ui->lineEditParallelStrainAtFMax->setBottom(0);
    ui->lineEditParallelStrainAtFMax->setValue(0.6);
    ui->lineEditParallelStrainRateAtFMax->setBottom(0);
    ui->lineEditParallelStrainRateAtFMax->setValue(0);
    ui->lineEditInitialFibreLength->setBottom(0);
    ui->lineEditInitialFibreLength->setValue(1);
    ui->checkBoxUseActivation->setChecked(false);
    ui->lineEditFastTwitchProportion->setBottom(0);
    ui->lineEditFastTwitchProportion->setValue(0.5);
    ui->lineEditTActivationA->setBottom(0);
    ui->lineEditTActivationA->setValue(8.00E-02);
    ui->lineEditTActivationB->setBottom(0);
    ui->lineEditTActivationB->setValue(4.70E-04);
    ui->lineEditTDeactivationA->setBottom(0);
    ui->lineEditTDeactivationA->setValue(9.00E-02);
    ui->lineEditTDeactivationB->setBottom(0);
    ui->lineEditTDeactivationB->setValue(5.60E-04);
    ui->lineEditActivationRate->setBottom(0);
    ui->lineEditActivationRate->setValue(500);
    ui->lineEditInitialActivation->setBottom(0);
    ui->lineEditInitialActivation->setValue(0);
    ui->lineEditMinimumActivation->setBottom(0);
    ui->lineEditMinimumActivation->setValue(1e-5);

    ui->lineEditUnloadedLength->setBottom(0);
    ui->lineEditUnloadedLength->setValue(1);
    ui->lineEditSpringConstant->setBottom(0);
    ui->lineEditSpringConstant->setValue(1);
    ui->lineEditArea->setBottom(0);
    ui->lineEditArea->setValue(1);
    ui->lineEditDamping->setBottom(0);
    ui->lineEditDamping->setValue(1);

    ui->lineEditCylinderRadius->setBottom(0);
    ui->lineEditCylinderRadius->setValue(1);

    ui->lineEdit2Cylinder1Radius->setBottom(0);
    ui->lineEdit2Cylinder1Radius->setValue(1);
    ui->lineEdit2Cylinder2Radius->setBottom(0);
    ui->lineEdit2Cylinder2Radius->setValue(1);

    if (m_muscle)
    {
        QStringList tabNamesMuscle, tabNamesStrap;
        for (int i = 0; i < ui->tabWidgetMuscle->count(); i++) tabNamesMuscle.push_back(ui->tabWidgetMuscle->tabText(i));
        for (int i = 0; i < ui->tabWidgetStrap->count(); i++) tabNamesStrap.push_back(ui->tabWidgetStrap->tabText(i));
        std::string s;
        m_muscle->SaveToAttributes();
        m_muscle->GetStrap()->SaveToAttributes();
        ui->lineEditMuscleID->setText(QString::fromStdString(m_muscle->GetAttribute("ID"s)));
        ui->lineEditMuscleID->setEnabled(false);
        ui->comboBoxOriginMarker->setCurrentText(QString::fromStdString(m_muscle->GetStrap()->GetAttribute("OriginMarkerID"s)));
        ui->comboBoxInsertionMarker->setCurrentText(QString::fromStdString(m_muscle->GetStrap()->GetAttribute("InsertionMarkerID"s)));
        if ((s = m_muscle->GetStrap()->GetAttribute("Length"s)).size()) ui->lineEditLength->setValue(GSUtil::Double(s));

        MAMuscle *maMuscle = dynamic_cast<MAMuscle *>(m_muscle);
        if (maMuscle)
        {
            if ((s = maMuscle->GetAttribute("ForcePerUnitArea"s)).size()) ui->lineEditForcePerUnitArea->setValue(GSUtil::Double(s));
            if ((s = maMuscle->GetAttribute("VMaxFactor"s)).size()) ui->lineEditVMaxFactor->setValue(GSUtil::Double(s));
            if ((s = maMuscle->GetAttribute("PCA"s)).size()) ui->lineEditPCA->setValue(GSUtil::Double(s));
            if ((s = maMuscle->GetAttribute("FibreLength"s)).size()) ui->lineEditFibreLength->setValue(GSUtil::Double(s));
            if ((s = maMuscle->GetAttribute("ActivationK"s)).size()) ui->lineEditActivationK->setValue(GSUtil::Double(s));
            ui->tabWidgetMuscle->setCurrentIndex(tabNamesMuscle.indexOf("Minetti-Alexander"));
        }

        MAMuscleComplete *maMuscleComplete = dynamic_cast<MAMuscleComplete *>(m_muscle);
        if (maMuscleComplete)
        {
            if ((s = maMuscleComplete->GetAttribute("ForcePerUnitArea"s)).size()) ui->lineEditForcePerUnitArea_2->setValue(GSUtil::Double(s));
            if ((s = maMuscleComplete->GetAttribute("VMaxFactor"s)).size()) ui->lineEditVMaxFactor_2->setValue(GSUtil::Double(s));
            if ((s = maMuscleComplete->GetAttribute("PCA"s)).size()) ui->lineEditPCA_2->setValue(GSUtil::Double(s));
            if ((s = maMuscleComplete->GetAttribute("FibreLength"s)).size()) ui->lineEditFibreLength_2->setValue(GSUtil::Double(s));
            if ((s = maMuscleComplete->GetAttribute("ActivationK"s)).size()) ui->lineEditActivationK_2->setValue(GSUtil::Double(s));
            if ((s = maMuscleComplete->GetAttribute("Width"s)).size()) ui->lineEditWidth->setValue(GSUtil::Double(s));
            if ((s = maMuscleComplete->GetAttribute("TendonLength"s)).size()) ui->lineEditTendonLength->setValue(GSUtil::Double(s));
            if ((s = maMuscleComplete->GetAttribute("SerialStrainAtFmax"s)).size()) ui->lineEditSerialStrainAtFMax->setValue(GSUtil::Double(s));
            if ((s = maMuscleComplete->GetAttribute("SerialStrainRateAtFmax"s)).size()) ui->lineEditSerialStrainRateAtFMax->setValue(GSUtil::Double(s));
            if ((s = maMuscleComplete->GetAttribute("ParallelStrainAtFmax"s)).size()) ui->lineEditParallelStrainAtFMax->setValue(GSUtil::Double(s));
            if ((s = maMuscleComplete->GetAttribute("ParallelStrainRateAtFmax"s)).size()) ui->lineEditParallelStrainRateAtFMax->setValue(GSUtil::Double(s));
            if ((s = maMuscleComplete->GetAttribute("InitialFibreLength"s)).size()) ui->lineEditInitialFibreLength->setValue(GSUtil::Double(s));
            if ((s = maMuscleComplete->GetAttribute("FastTwitchProportion"s)).size()) ui->lineEditFastTwitchProportion->setValue(GSUtil::Double(s));
            if ((s = maMuscleComplete->GetAttribute("TActivationA"s)).size()) ui->lineEditTActivationA->setValue(GSUtil::Double(s));
            if ((s = maMuscleComplete->GetAttribute("TActivationB"s)).size()) ui->lineEditTActivationB->setValue(GSUtil::Double(s));
            if ((s = maMuscleComplete->GetAttribute("TDeactivationA"s)).size()) ui->lineEditTDeactivationA->setValue(GSUtil::Double(s));
            if ((s = maMuscleComplete->GetAttribute("TDeactivationB"s)).size()) ui->lineEditTDeactivationB->setValue(GSUtil::Double(s));
            if ((s = maMuscleComplete->GetAttribute("ActivationRate"s)).size()) ui->lineEditActivationRate->setValue(GSUtil::Double(s));
            if ((s = maMuscleComplete->GetAttribute("StartActivation"s)).size()) ui->lineEditInitialActivation->setValue(GSUtil::Double(s));
            if ((s = maMuscleComplete->GetAttribute("MinimumActivation"s)).size()) ui->lineEditMinimumActivation->setValue(GSUtil::Double(s));
            if ((s = maMuscleComplete->GetAttribute("ActivationKinetics"s)).size()) ui->checkBoxUseActivation->setChecked(GSUtil::Bool(s));
            if ((s = maMuscleComplete->GetAttribute("SerialStrainModel"s)).size())
                ui->comboBoxSerialStrainModel->setCurrentIndex(ui->comboBoxSerialStrainModel->findText(QString::fromStdString(s)));
            if ((s = maMuscleComplete->GetAttribute("ParallelStrainModel"s)).size())
                ui->comboBoxParallelStrainModel->setCurrentIndex(ui->comboBoxParallelStrainModel->findText(QString::fromStdString(s)));
            ui->tabWidgetMuscle->setCurrentIndex(tabNamesMuscle.indexOf("Minetti-Alexander Elastic"));
        }

        DampedSpringMuscle *dampedSpringMuscle = dynamic_cast<DampedSpringMuscle *>(m_muscle);
        if (dampedSpringMuscle)
        {
            if ((s = dampedSpringMuscle->GetAttribute("UnloadedLength"s)).size()) ui->lineEditUnloadedLength->setValue(GSUtil::Double(s));
            if ((s = dampedSpringMuscle->GetAttribute("SpringConstant"s)).size()) ui->lineEditSpringConstant->setValue(GSUtil::Double(s));
            if ((s = dampedSpringMuscle->GetAttribute("Area"s)).size()) ui->lineEditArea->setValue(GSUtil::Double(s));
            if ((s = dampedSpringMuscle->GetAttribute("Damping"s)).size()) ui->lineEditDamping->setValue(GSUtil::Double(s));
            if ((s = dampedSpringMuscle->GetAttribute("BreakingStrain"s)).size()) ui->lineEditBreakingStrain->setValue(GSUtil::Double(s));
            ui->tabWidgetMuscle->setCurrentIndex(tabNamesMuscle.indexOf("Damped Spring"));
        }

        NPointStrap *nPointStrap = dynamic_cast<NPointStrap *>(m_muscle->GetStrap());
        if (nPointStrap)
        {
            if ((s = nPointStrap->GetAttribute("ViaPointMarkerIDList"s)).size())
            {
                std::vector<std::string> result;
                pystring::split(s, result);
                if (result.empty())
                {
                    ui->spinBoxNViaPoints->setValue(int(result.size()));
                    for (int i = 0; i < ui->spinBoxNViaPoints->value(); i++)
                    {
                        QLabel *label = new QLabel();
                        label->setText(QString("Via Point %1").arg(i));
                        m_gridLayout->addWidget(label, i, 0, Qt::AlignTop);
                        QComboBox *comboBoxMarker = new QComboBox();
                        comboBoxMarker->addItems(markerIDs);
                        comboBoxMarker->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
                        m_gridLayout->addWidget(comboBoxMarker, i, 2, Qt::AlignTop);
                        m_viaPointLabelList.push_back(label);
                        m_viaPointComboBoxList.push_back(comboBoxMarker);
                        comboBoxMarker->setCurrentText(QString::fromStdString(result[size_t(i)]));
                    }
                    m_gridSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
                    m_gridLayout->addItem(m_gridSpacer, ui->spinBoxNViaPoints->value(), 0);
                }
            }
            ui->tabWidgetStrap->setCurrentIndex(tabNamesStrap.indexOf("N-Point"));
        }

        CylinderWrapStrap *cylinderWrapStrap = dynamic_cast<CylinderWrapStrap *>(m_muscle->GetStrap());
        if (cylinderWrapStrap)
        {
            if ((s = cylinderWrapStrap->GetAttribute("CylinderMarkerID"s)).size()) ui->comboBoxCylinderMarker->setCurrentText(QString::fromStdString(s));
            if ((s = cylinderWrapStrap->GetAttribute("CylinderRadius"s)).size()) ui->lineEditCylinderRadius->setValue(GSUtil::Double(s));
            ui->tabWidgetStrap->setCurrentIndex(tabNamesStrap.indexOf("Cylinder"));
        }

        TwoCylinderWrapStrap *twoCylinderWrapStrap = dynamic_cast<TwoCylinderWrapStrap *>(m_muscle->GetStrap());
        if (twoCylinderWrapStrap)
        {
            if ((s = twoCylinderWrapStrap->GetAttribute("Cylinder1MarkerID"s)).size()) ui->comboBox2Cylinder1Marker->setCurrentText(QString::fromStdString(s));
            if ((s = twoCylinderWrapStrap->GetAttribute("Cylinder1Radius"s)).size()) ui->lineEdit2Cylinder1Radius->setValue(GSUtil::Double(s));
            if ((s = twoCylinderWrapStrap->GetAttribute("Cylinder2MarkerID"s)).size()) ui->comboBox2Cylinder2Marker->setCurrentText(QString::fromStdString(s));
            if ((s = twoCylinderWrapStrap->GetAttribute("Cylinder2Radius"s)).size()) ui->lineEdit2Cylinder2Radius->setValue(GSUtil::Double(s));
            ui->tabWidgetStrap->setCurrentIndex(tabNamesStrap.indexOf("2-Cylinder"));
        }
    }
    else
    {
        std::map<std::string, Muscle *> *muscleList = m_simulation->GetMuscleList();
        QStringList muscleIDs;
        for (auto it = muscleList->begin(); it != muscleList->end();
                it++) muscleIDs.append(QString::fromStdString(it->first));

        // set default new name
        ui->lineEditMuscleID->addStrings(muscleIDs);
        int initialNameCount = 0;
        QString initialName = QString("Muscle%1").arg(initialNameCount, 3, 10, QLatin1Char('0'));
        while (muscleList->count(initialName.toStdString()))
        {
            initialNameCount++;
            initialName = QString("Muscle%1").arg(initialNameCount, 3, 10, QLatin1Char('0'));
            if (initialNameCount >= 999) break; // only do this for the first 999 markers
        }
        ui->lineEditMuscleID->setText(initialName);
    }
}

void DialogMuscles::tabChanged(int index)
{
    Q_UNUSED(index);
    updateActivation();
}

void DialogMuscles::comboBoxChanged(int index)
{
    Q_UNUSED(index);
    updateActivation();
}

void DialogMuscles::lineEditChanged(const QString &text)
{
    Q_UNUSED(text);
    updateActivation();
}

void DialogMuscles::spinBoxChanged(const QString &text)
{
    Q_UNUSED(text);

    if (this->sender() == ui->spinBoxNViaPoints)
    {
        // get the lists in the right formats
//        std::map<std::string, Body *> *bodyList = m_simulation->GetBodyList();
//        QStringList bodyIDs;
//        for (auto it = bodyList->begin(); it != bodyList->end(); it++) bodyIDs.append(QString::fromStdString(it->first));
        std::map<std::string, Marker *> *markerList = m_simulation->GetMarkerList();
        QStringList markerIDs;
        for (auto it = markerList->begin(); it != markerList->end();
                it++) markerIDs.append(QString::fromStdString(it->first));

        // store the current values in the list
        QVector<QString> oldValues(m_viaPointComboBoxList.size());
        for (int i = 0; i < m_viaPointComboBoxList.size();
                i++) oldValues[i] = m_viaPointComboBoxList[i]->currentText();

        // delete all the existing widgets in the layout
        if (m_gridSpacer)
        {
            m_gridLayout->removeItem(m_gridSpacer);
            delete m_gridSpacer;
        }
        for (auto it = m_viaPointLabelList.rbegin(); it != m_viaPointLabelList.rend(); it++)
        {
            m_gridLayout->removeWidget(*it);
            delete *it;
        }
        m_viaPointLabelList.clear();
        for (auto it = m_viaPointComboBoxList.rbegin(); it != m_viaPointComboBoxList.rend(); it++)
        {
            m_gridLayout->removeWidget(*it);
            delete *it;
        }
        m_viaPointComboBoxList.clear();

        // now create a new set
        for (int i = 0; i < ui->spinBoxNViaPoints->value(); i++)
        {
            QLabel *label = new QLabel();
            label->setText(QString("Via Point %1").arg(i));
            m_gridLayout->addWidget(label, i, 0, Qt::AlignTop);
            QComboBox *comboBoxMarker = new QComboBox();
            comboBoxMarker->addItems(markerIDs);
            comboBoxMarker->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            m_gridLayout->addWidget(comboBoxMarker, i, 2, Qt::AlignTop);
            m_viaPointLabelList.push_back(label);
            m_viaPointComboBoxList.push_back(comboBoxMarker);
            if (i < oldValues.size()) comboBoxMarker->setCurrentText(oldValues[i]);
        }
        m_gridSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        m_gridLayout->addItem(m_gridSpacer, ui->spinBoxNViaPoints->value(), 0);
    }

    updateActivation();
}

void DialogMuscles::checkBoxChanged(int index)
{
    Q_UNUSED(index);
    updateActivation();
}


void DialogMuscles::updateActivation()
{
    bool okEnable = true;
    QString textCopy = ui->lineEditMuscleID->text();
    int pos = ui->lineEditMuscleID->cursorPosition();
    if (ui->lineEditMuscleID->validator()->validate(textCopy,
            pos) != QValidator::Acceptable) okEnable = false;

    ui->pushButtonOK->setEnabled(okEnable);
}

Simulation *DialogMuscles::simulation() const
{
    return m_simulation;
}

void DialogMuscles::setSimulation(Simulation *simulation)
{
    m_simulation = simulation;
}

Muscle *DialogMuscles::muscle() const
{
    return m_muscle;
}

void DialogMuscles::setMuscle(Muscle *muscle)
{
    m_muscle = muscle;
}
