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
#include "DialogProperties.h"
#include "Marker.h"

#include "pystring.h"

#include <QDebug>
#include <QScrollArea>
#include <QSpacerItem>
#include <QGridLayout>

#include <string>
#include <map>
using namespace std::string_literals; // enables s-suffix for std::string literals

DialogMuscles::DialogMuscles(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogMuscles)
{
    ui->setupUi(this);

    setWindowTitle(tr("Muscle Builder"));
#ifdef Q_OS_MACOS
    setWindowFlags(windowFlags() & (~Qt::Dialog) | Qt::Window); // allows the window to be resized on macs
#endif
    restoreGeometry(Preferences::valueQByteArray("DialogMusclesGeometry"));

    QVBoxLayout *verticalLayoutViaPoints;
    QScrollArea *scrollAreaViaPoints;
    QWidget *scrollAreaWidgetContentsViaPoints;
    verticalLayoutViaPoints = new QVBoxLayout();
    verticalLayoutViaPoints->setSpacing(6);
    verticalLayoutViaPoints->setContentsMargins(11, 11, 11, 11);
    verticalLayoutViaPoints->setObjectName(QStringLiteral("verticalLayout"));
    scrollAreaViaPoints = new QScrollArea();
    scrollAreaViaPoints->setObjectName(QStringLiteral("scrollArea"));
    scrollAreaViaPoints->setWidgetResizable(true);
    scrollAreaWidgetContentsViaPoints = new QWidget();
    scrollAreaWidgetContentsViaPoints->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
    m_gridLayoutViaPoints = new QGridLayout();
    m_gridLayoutViaPoints->setSpacing(6);
    m_gridLayoutViaPoints->setContentsMargins(11, 11, 11, 11);
    m_gridLayoutViaPoints->setObjectName(QStringLiteral("gridLayout"));
    scrollAreaWidgetContentsViaPoints->setLayout(m_gridLayoutViaPoints);
    scrollAreaViaPoints->setWidget(scrollAreaWidgetContentsViaPoints);
    verticalLayoutViaPoints->addWidget(scrollAreaViaPoints);
    ui->widgetViaPointsPlaceholder->setLayout(verticalLayoutViaPoints);

    QVBoxLayout *verticalLayoutTorqueMarkers;
    QScrollArea *scrollAreaTorqueMarkers;
    QWidget *scrollAreaWidgetContentsTorqueMarkers;
    verticalLayoutTorqueMarkers = new QVBoxLayout();
    verticalLayoutTorqueMarkers->setSpacing(6);
    verticalLayoutTorqueMarkers->setContentsMargins(11, 11, 11, 11);
    verticalLayoutTorqueMarkers->setObjectName(QStringLiteral("verticalLayout"));
    scrollAreaTorqueMarkers = new QScrollArea();
    scrollAreaTorqueMarkers->setObjectName(QStringLiteral("scrollArea"));
    scrollAreaTorqueMarkers->setWidgetResizable(true);
    scrollAreaWidgetContentsTorqueMarkers = new QWidget();
    scrollAreaWidgetContentsTorqueMarkers->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
    m_gridLayoutTorqueMarkers = new QGridLayout();
    m_gridLayoutTorqueMarkers->setSpacing(6);
    m_gridLayoutTorqueMarkers->setContentsMargins(11, 11, 11, 11);
    m_gridLayoutTorqueMarkers->setObjectName(QStringLiteral("gridLayout"));
    scrollAreaWidgetContentsTorqueMarkers->setLayout(m_gridLayoutTorqueMarkers);
    scrollAreaTorqueMarkers->setWidget(scrollAreaWidgetContentsTorqueMarkers);
    verticalLayoutTorqueMarkers->addWidget(scrollAreaTorqueMarkers);
    ui->widgetTorqueMarkersPlaceholder->setLayout(verticalLayoutTorqueMarkers);

    connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->pushButtonProperties, SIGNAL(clicked()), this, SLOT(properties()));
    connect(ui->tabWidgetMuscle, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
    connect(ui->tabWidgetStrap, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

    // this logic monitors for changing values
    QList<QWidget *> widgets = this->findChildren<QWidget *>();
    for (auto it = widgets.begin(); it != widgets.end(); it++)
    {
        QComboBox *comboBox = dynamic_cast<QComboBox *>(*it);
        if (comboBox)
            connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxChanged(int)));
        QLineEdit *lineEdit = dynamic_cast<QLineEdit *>(*it);
        if (lineEdit)
            connect(lineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(lineEditChanged(const QString &)));
        QSpinBox *spinBox = dynamic_cast<QSpinBox *>(*it);
        if (spinBox)
            connect(spinBox, SIGNAL(textChanged(const QString &)), this, SLOT(spinBoxChanged(const QString &)));
        QCheckBox *checkBox = dynamic_cast<QCheckBox *>(*it);
        if (checkBox)
            connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(checkBoxChanged(int)));
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
    auto markerList = m_simulation->GetMarkerList();
    QString strapTab = ui->tabWidgetStrap->tabText(ui->tabWidgetStrap->currentIndex());
    std::unique_ptr<Strap> strap;
    if (strapTab == "N-Point")
    {
        if (ui->spinBoxNViaPoints->value() == 0)
        {
            strap = std::make_unique<TwoPointStrap>();
            Marker *originMarker = markerList->at(ui->comboBoxOriginMarker->currentText().toStdString()).get();
            Marker *insertionMarker = markerList->at(ui->comboBoxInsertionMarker->currentText().toStdString()).get();
            reinterpret_cast<TwoPointStrap *>(strap.get())->SetOrigin(originMarker);
            reinterpret_cast<TwoPointStrap *>(strap.get())->SetInsertion(insertionMarker);
        }
        else
        {
            strap =  std::make_unique<NPointStrap>();
            Marker *originMarker = markerList->at(ui->comboBoxOriginMarker->currentText().toStdString()).get();
            Marker *insertionMarker = markerList->at(ui->comboBoxInsertionMarker->currentText().toStdString()).get();
            reinterpret_cast<NPointStrap *>(strap.get())->SetOrigin(originMarker);
            reinterpret_cast<NPointStrap *>(strap.get())->SetInsertion(insertionMarker);
            std::vector<Marker *>viaPointMarkerList;
            viaPointMarkerList.reserve(size_t(ui->spinBoxNViaPoints->value()));
            for (int i = 0; i < ui->spinBoxNViaPoints->value(); i++) viaPointMarkerList.push_back(markerList->at(m_viaPointComboBoxList[i]->currentText().toStdString()).get());
            reinterpret_cast<NPointStrap *>(strap.get())->SetViaPoints(&viaPointMarkerList);
        }
    }
    else if (strapTab == "Cylinder")
    {
        strap =  std::make_unique<CylinderWrapStrap>();
        Marker *originMarker = markerList->at(ui->comboBoxOriginMarker->currentText().toStdString()).get();
        Marker *insertionMarker = markerList->at(ui->comboBoxInsertionMarker->currentText().toStdString()).get();
        reinterpret_cast<CylinderWrapStrap *>(strap.get())->SetOrigin(originMarker);
        reinterpret_cast<CylinderWrapStrap *>(strap.get())->SetInsertion(insertionMarker);
        Marker *cylinderMarker = markerList->at(ui->comboBoxCylinderMarker->currentText().toStdString()).get();
        reinterpret_cast<CylinderWrapStrap *>(strap.get())->SetCylinder(cylinderMarker);
        reinterpret_cast<CylinderWrapStrap *>(strap.get())->SetCylinderRadius(ui->lineEditCylinderRadius->value());
    }
    else if (strapTab == "2-Cylinder")
    {
        strap =  std::make_unique<TwoCylinderWrapStrap>();
        Marker *originMarker = markerList->at(ui->comboBoxOriginMarker->currentText().toStdString()).get();
        Marker *insertionMarker = markerList->at(ui->comboBoxInsertionMarker->currentText().toStdString()).get();
        reinterpret_cast<TwoCylinderWrapStrap *>(strap.get())->SetOrigin(originMarker);
        reinterpret_cast<TwoCylinderWrapStrap *>(strap.get())->SetInsertion(insertionMarker);
        Marker *cylinder1Marker = markerList->at(ui->comboBox2Cylinder1Marker->currentText().toStdString()).get();
        reinterpret_cast<TwoCylinderWrapStrap *>(strap.get())->SetCylinder1(cylinder1Marker);
        reinterpret_cast<TwoCylinderWrapStrap *>(strap.get())->SetCylinder1Radius(ui->lineEdit2Cylinder1Radius->value());
        Marker *cylinder2Marker = markerList->at(ui->comboBox2Cylinder2Marker->currentText().toStdString()).get();
        reinterpret_cast<TwoCylinderWrapStrap *>(strap.get())->SetCylinder2(cylinder2Marker);
        reinterpret_cast<TwoCylinderWrapStrap *>(strap.get())->SetCylinder2Radius(ui->lineEdit2Cylinder2Radius->value());
    }
    Q_ASSERT_X(strap, "DialogMuscles::accept", "strap undefined");
    strap->setSimulation(m_simulation);
    strap->setLength(ui->lineEditLength->value());
    if (ui->spinBoxNTorqueMarkers->value())
    {
        std::vector<Marker *>torqueMarkerList;
        torqueMarkerList.reserve(size_t(ui->spinBoxNTorqueMarkers->value()));
        for (int i = 0; i < ui->spinBoxNTorqueMarkers->value(); i++) torqueMarkerList.push_back(markerList->at(m_torqueMarkerComboBoxList[i]->currentText().toStdString()).get());
        strap->setTorqueMarkerList(torqueMarkerList);
    }

    QString muscleTab = ui->tabWidgetMuscle->tabText(ui->tabWidgetMuscle->currentIndex());
    if (muscleTab == "Minetti-Alexander")
    {
        std::unique_ptr<MAMuscle> muscle = std::make_unique<MAMuscle>();
        muscle->SetStrap(strap.get());
        double forcePerUnitArea = ui->lineEditForcePerUnitArea->value();
        double vMaxFactor = ui->lineEditVMaxFactor->value();
        double pca = ui->lineEditPCA->value();
        double fibreLength = ui->lineEditFibreLength->value();
        double activationK = ui->lineEditActivationK->value();
        muscle->setForcePerUnitArea(forcePerUnitArea);
        muscle->setVMaxFactor(vMaxFactor);
        muscle->setPca(pca);
        muscle->setFibreLength(fibreLength);
        muscle->SetF0(pca * forcePerUnitArea);
        muscle->SetVMax(fibreLength * vMaxFactor);
        muscle->SetK(activationK);
        m_outputMuscle = std::move(muscle);
    }
    else if (muscleTab == "Minetti-Alexander Elastic")
    {
        std::unique_ptr<MAMuscleComplete> muscle = std::make_unique<MAMuscleComplete>();
        muscle->SetStrap(strap.get());
        double forcePerUnitArea = ui->lineEditForcePerUnitArea_2->value();
        double vMaxFactor = ui->lineEditVMaxFactor_2->value();
        double pca = ui->lineEditPCA_2->value();
        double fibreLength = ui->lineEditFibreLength_2->value();
        double activationK = ui->lineEditActivationK_2->value();
        double width = ui->lineEditWidth->value();
        double tendonLength = ui->lineEditTendonLength->value();
        double serialStrainAtFmax = ui->lineEditSerialStrainAtFMax->value();
        double serialStrainRateAtFmax = ui->lineEditSerialStrainRateAtFMax->value();
        QString serialStrainModel = ui->comboBoxSerialStrainModel->currentText();
        double parallelStrainAtFmax = ui->lineEditParallelStrainAtFMax->value();
        double parallelStrainRateAtFmax = ui->lineEditParallelStrainRateAtFMax->value();
        QString parallelStrainModel = ui->comboBoxParallelStrainModel->currentText();
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
        muscle->setSerialStrainModel(serialStrainModel.toStdString());
        muscle->setParallelStrainModel(parallelStrainModel.toStdString());
        muscle->setActivationK(activationK);
        muscle->setAkFastTwitchProportion(akFastTwitchProportion);
        muscle->setAkTActivationA(akTActivationA);
        muscle->setAkTActivationB(akTActivationB);
        muscle->setAkTDeactivationA(akTDeactivationA);
        muscle->setAkTDeactivationB(akTDeactivationB);
        muscle->setFibreLength(fibreLength);
        muscle->setForcePerUnitArea(forcePerUnitArea);
        muscle->setInitialFibreLength(initialFibreLength);
        muscle->setPca(pca);
        muscle->setStartActivation(startActivation);
        muscle->setTendonLength(tendonLength);
        muscle->setVMaxFactor(vMaxFactor);
        muscle->setWidth(width);

        muscle->SetSerialElasticProperties(serialStrainAtFmax, serialStrainRateAtFmax, tendonLength, muscle->serialStrainModel());
        muscle->SetParallelElasticProperties(parallelStrainAtFmax, parallelStrainRateAtFmax, parallelElementLength, muscle->parallelStrainModel());
        muscle->SetMuscleProperties(vMax, fMax, activationK, width);
        muscle->SetActivationKinetics(activationKinetics, akFastTwitchProportion, akTActivationA, akTActivationB, akTDeactivationA, akTDeactivationB);
        muscle->SetInitialFibreLength(initialFibreLength);
        muscle->SetActivationRate(activationRate);
        muscle->SetStartActivation(startActivation);
        muscle->SetMinimumActivation(minimumActivation);
        m_outputMuscle = std::move(muscle);
    }
    else if (muscleTab == "Damped Spring")
    {
        std::unique_ptr<DampedSpringMuscle> muscle = std::make_unique<DampedSpringMuscle>();
        muscle->SetStrap(strap.get());
        double unloadedLength = ui->lineEditUnloadedLength->value();
        double springConstant = ui->lineEditSpringConstant->value();
        double area = ui->lineEditArea->value();
        double damping = ui->lineEditDamping->value();
        muscle->SetUnloadedLength(unloadedLength);
        muscle->SetSpringConstant(springConstant);
        muscle->SetArea(area);
        muscle->SetDamping(damping);
        if (ui->lineEditBreakingStrain->text().size()) muscle->SetBreakingStrain(ui->lineEditBreakingStrain->value());
        m_outputMuscle = std::move(muscle);
    }
    Q_ASSERT_X(m_outputMuscle, "DialogMuscles::accept", "m_outputMuscle undefined");
    m_outputMuscle->setName(ui->lineEditMuscleID->text().toStdString());
    m_outputMuscle->setSimulation(m_simulation);

    m_outputStrap = std::move(strap);

    if (m_inputMuscle)
    {
        m_outputMuscle->GetStrap()->setColour1(m_inputMuscle->GetStrap()->colour1());
        m_outputMuscle->GetStrap()->setColour2(m_inputMuscle->GetStrap()->colour2());
        m_outputMuscle->setColour1(m_inputMuscle->colour1());
        m_outputMuscle->GetStrap()->setSize1(m_inputMuscle->GetStrap()->size1());
        m_outputMuscle->GetStrap()->setSize2(m_inputMuscle->GetStrap()->size2());
        m_outputMuscle->setSize1(m_inputMuscle->size1());
        m_outputMuscle->setSize2(m_inputMuscle->size2());
        m_outputMuscle->GetStrap()->setName(m_inputMuscle->GetStrap()->name());
    }
    else
    {
        m_outputMuscle->GetStrap()->setColour1(Preferences::valueQColor("StrapColour").name(QColor::HexArgb).toStdString());
        m_outputMuscle->GetStrap()->setColour2(Preferences::valueQColor("StrapCylinderColour").name(QColor::HexArgb).toStdString());
        m_outputMuscle->setColour1(Preferences::valueQColor("StrapForceColour").name(QColor::HexArgb).toStdString());
        m_outputMuscle->GetStrap()->setSize1(Preferences::valueDouble("StrapRadius"));
        m_outputMuscle->GetStrap()->setSize2(Preferences::valueDouble("StrapCylinderLength"));
        m_outputMuscle->setSize1(Preferences::valueDouble("StrapForceRadius"));
        m_outputMuscle->setSize2(Preferences::valueDouble("StrapForceScale"));
        m_outputMuscle->GetStrap()->setName(m_outputMuscle->name() + "_strap"s);
    }

    if (m_properties.size() > 0)
    {
        if (m_properties.count("StrapColour"))
            m_outputMuscle->GetStrap()->setColour1(qvariant_cast<QColor>(m_properties["StrapColour"].value).name(QColor::HexArgb).toStdString());
        if (m_properties.count("StrapCylinderColour"))
            m_outputMuscle->GetStrap()->setColour2(qvariant_cast<QColor>(m_properties["StrapCylinderColour"].value).name(QColor::HexArgb).toStdString());
        if (m_properties.count("StrapForceColour"))
            m_outputMuscle->setColour1(qvariant_cast<QColor>(m_properties["StrapForceColour"].value).name(QColor::HexArgb).toStdString());
        if (m_properties.count("StrapRadius"))
            m_outputMuscle->GetStrap()->setSize1(m_properties["StrapRadius"].value.toDouble());
        if (m_properties.count("StrapCylinderLength"))
            m_outputMuscle->GetStrap()->setSize2(m_properties["StrapCylinderLength"].value.toDouble());
        if (m_properties.count("StrapForceRadius"))
            m_outputMuscle->setSize1(m_properties["StrapForceRadius"].value.toDouble());
        if (m_properties.count("StrapForceScale"))
            m_outputMuscle->setSize2(m_properties["StrapForceScale"].value.toDouble());
    }

    m_outputStrap->saveToAttributes();
    m_outputStrap->createFromAttributes();
    // muscle has no dependencies so it does not need saving and loading

    Preferences::insert("DialogMusclesGeometry", saveGeometry());
    QDialog::accept();
}

void DialogMuscles::reject() // this catches cancel, close and escape key
{
    qDebug() << "DialogMuscles::reject()";
    Preferences::insert("DialogMusclesGeometry", saveGeometry());
    QDialog::reject();
}

void DialogMuscles::closeEvent(QCloseEvent *event)
{
    Preferences::insert("DialogMusclesGeometry", saveGeometry());

    QDialog::closeEvent(event);
}

std::unique_ptr<Strap> DialogMuscles::outputStrap()
{
    return std::move(m_outputStrap);
}

std::unique_ptr<Muscle> DialogMuscles::outputMuscle()
{
    return std::move(m_outputMuscle);
}

void DialogMuscles::setInputMuscle(Muscle *inputMuscle)
{
    m_inputMuscle = inputMuscle;
}

void DialogMuscles::lateInitialise()
{
    Q_ASSERT_X(m_simulation, "DialogMuscles::lateInitialise", "m_simulation undefined");

    // get the lists in the right formats
    auto markerList = m_simulation->GetMarkerList();
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

    ui->lineEditLength->setValue(-1);

    if (!m_inputMuscle)
    {
        // set default new name
        auto nameSet = m_simulation->GetNameSet();
        ui->lineEditMuscleID->addStrings(nameSet);
        int initialNameCount = 0;
        QString initialName = QString("Muscle%1").arg(initialNameCount, 3, 10, QLatin1Char('0'));
        while (nameSet.count(initialName.toStdString()))
        {
            initialNameCount++;
            initialName = QString("Muscle%1").arg(initialNameCount, 3, 10, QLatin1Char('0'));
            if (initialNameCount >= 999) break; // only do this for the first 999 markers
        }
        ui->lineEditMuscleID->setText(initialName);
        return;
    }

    QStringList tabNamesMuscle, tabNamesStrap;
    for (int i = 0; i < ui->tabWidgetMuscle->count(); i++) tabNamesMuscle.push_back(ui->tabWidgetMuscle->tabText(i));
    for (int i = 0; i < ui->tabWidgetStrap->count(); i++) tabNamesStrap.push_back(ui->tabWidgetStrap->tabText(i));
    std::string s;
    m_inputMuscle->saveToAttributes();
    m_inputMuscle->GetStrap()->saveToAttributes();
    ui->lineEditMuscleID->setText(QString::fromStdString(m_inputMuscle->findAttribute("ID"s)));
    ui->lineEditMuscleID->setEnabled(false);
    ui->comboBoxOriginMarker->setCurrentText(QString::fromStdString(m_inputMuscle->GetStrap()->findAttribute("OriginMarkerID"s)));
    ui->comboBoxInsertionMarker->setCurrentText(QString::fromStdString(m_inputMuscle->GetStrap()->findAttribute("InsertionMarkerID"s)));
    if ((s = m_inputMuscle->GetStrap()->findAttribute("Length"s)).size()) ui->lineEditLength->setValue(GSUtil::Double(s));

    MAMuscle *maMuscle = dynamic_cast<MAMuscle *>(m_inputMuscle);
    if (maMuscle)
    {
        if ((s = maMuscle->findAttribute("ForcePerUnitArea"s)).size()) ui->lineEditForcePerUnitArea->setValue(GSUtil::Double(s));
        if ((s = maMuscle->findAttribute("VMaxFactor"s)).size()) ui->lineEditVMaxFactor->setValue(GSUtil::Double(s));
        if ((s = maMuscle->findAttribute("PCA"s)).size()) ui->lineEditPCA->setValue(GSUtil::Double(s));
        if ((s = maMuscle->findAttribute("FibreLength"s)).size()) ui->lineEditFibreLength->setValue(GSUtil::Double(s));
        if ((s = maMuscle->findAttribute("ActivationK"s)).size()) ui->lineEditActivationK->setValue(GSUtil::Double(s));
        ui->tabWidgetMuscle->setCurrentIndex(tabNamesMuscle.indexOf("Minetti-Alexander"));
    }

    MAMuscleComplete *maMuscleComplete = dynamic_cast<MAMuscleComplete *>(m_inputMuscle);
    if (maMuscleComplete)
    {
        if ((s = maMuscleComplete->findAttribute("ForcePerUnitArea"s)).size()) ui->lineEditForcePerUnitArea_2->setValue(GSUtil::Double(s));
        if ((s = maMuscleComplete->findAttribute("VMaxFactor"s)).size()) ui->lineEditVMaxFactor_2->setValue(GSUtil::Double(s));
        if ((s = maMuscleComplete->findAttribute("PCA"s)).size()) ui->lineEditPCA_2->setValue(GSUtil::Double(s));
        if ((s = maMuscleComplete->findAttribute("FibreLength"s)).size()) ui->lineEditFibreLength_2->setValue(GSUtil::Double(s));
        if ((s = maMuscleComplete->findAttribute("ActivationK"s)).size()) ui->lineEditActivationK_2->setValue(GSUtil::Double(s));
        if ((s = maMuscleComplete->findAttribute("Width"s)).size()) ui->lineEditWidth->setValue(GSUtil::Double(s));
        if ((s = maMuscleComplete->findAttribute("TendonLength"s)).size()) ui->lineEditTendonLength->setValue(GSUtil::Double(s));
        if ((s = maMuscleComplete->findAttribute("SerialStrainAtFmax"s)).size()) ui->lineEditSerialStrainAtFMax->setValue(GSUtil::Double(s));
        if ((s = maMuscleComplete->findAttribute("SerialStrainRateAtFmax"s)).size()) ui->lineEditSerialStrainRateAtFMax->setValue(GSUtil::Double(s));
        if ((s = maMuscleComplete->findAttribute("ParallelStrainAtFmax"s)).size()) ui->lineEditParallelStrainAtFMax->setValue(GSUtil::Double(s));
        if ((s = maMuscleComplete->findAttribute("ParallelStrainRateAtFmax"s)).size()) ui->lineEditParallelStrainRateAtFMax->setValue(GSUtil::Double(s));
        if ((s = maMuscleComplete->findAttribute("InitialFibreLength"s)).size()) ui->lineEditInitialFibreLength->setValue(GSUtil::Double(s));
        if ((s = maMuscleComplete->findAttribute("FastTwitchProportion"s)).size()) ui->lineEditFastTwitchProportion->setValue(GSUtil::Double(s));
        if ((s = maMuscleComplete->findAttribute("TActivationA"s)).size()) ui->lineEditTActivationA->setValue(GSUtil::Double(s));
        if ((s = maMuscleComplete->findAttribute("TActivationB"s)).size()) ui->lineEditTActivationB->setValue(GSUtil::Double(s));
        if ((s = maMuscleComplete->findAttribute("TDeactivationA"s)).size()) ui->lineEditTDeactivationA->setValue(GSUtil::Double(s));
        if ((s = maMuscleComplete->findAttribute("TDeactivationB"s)).size()) ui->lineEditTDeactivationB->setValue(GSUtil::Double(s));
        if ((s = maMuscleComplete->findAttribute("ActivationRate"s)).size()) ui->lineEditActivationRate->setValue(GSUtil::Double(s));
        if ((s = maMuscleComplete->findAttribute("StartActivation"s)).size()) ui->lineEditInitialActivation->setValue(GSUtil::Double(s));
        if ((s = maMuscleComplete->findAttribute("MinimumActivation"s)).size()) ui->lineEditMinimumActivation->setValue(GSUtil::Double(s));
        if ((s = maMuscleComplete->findAttribute("ActivationKinetics"s)).size()) ui->checkBoxUseActivation->setChecked(GSUtil::Bool(s));
        if ((s = maMuscleComplete->findAttribute("SerialStrainModel"s)).size()) ui->comboBoxSerialStrainModel->setCurrentIndex(ui->comboBoxSerialStrainModel->findText(QString::fromStdString(s)));
        if ((s = maMuscleComplete->findAttribute("ParallelStrainModel"s)).size()) ui->comboBoxParallelStrainModel->setCurrentIndex(ui->comboBoxParallelStrainModel->findText(QString::fromStdString(s)));
        ui->tabWidgetMuscle->setCurrentIndex(tabNamesMuscle.indexOf("Minetti-Alexander Elastic"));
    }

    DampedSpringMuscle *dampedSpringMuscle = dynamic_cast<DampedSpringMuscle *>(m_inputMuscle);
    if (dampedSpringMuscle)
    {
        if ((s = dampedSpringMuscle->findAttribute("UnloadedLength"s)).size()) ui->lineEditUnloadedLength->setValue(GSUtil::Double(s));
        if ((s = dampedSpringMuscle->findAttribute("SpringConstant"s)).size()) ui->lineEditSpringConstant->setValue(GSUtil::Double(s));
        if ((s = dampedSpringMuscle->findAttribute("Area"s)).size()) ui->lineEditArea->setValue(GSUtil::Double(s));
        if ((s = dampedSpringMuscle->findAttribute("Damping"s)).size()) ui->lineEditDamping->setValue(GSUtil::Double(s));
        if ((s = dampedSpringMuscle->findAttribute("BreakingStrain"s)).size()) ui->lineEditBreakingStrain->setValue(GSUtil::Double(s));
        ui->tabWidgetMuscle->setCurrentIndex(tabNamesMuscle.indexOf("Damped Spring"));
    }

    NPointStrap *nPointStrap = dynamic_cast<NPointStrap *>(m_inputMuscle->GetStrap());
    if (nPointStrap)
    {
        s = nPointStrap->findAttribute("ViaPointMarkerIDList"s);
        if (s.size())
        {
            std::vector<std::string> result;
            pystring::split(s, result);
            if (result.size())
            {
                const QSignalBlocker blocker(ui->spinBoxNViaPoints);
                ui->spinBoxNViaPoints->setValue(int(result.size()));
                for (int i = 0; i < ui->spinBoxNViaPoints->value(); i++)
                {
                    QLabel *label = new QLabel();
                    label->setText(QString("Via Point %1").arg(i));
                    m_gridLayoutViaPoints->addWidget(label, i, 0, Qt::AlignTop);
                    QComboBox *comboBoxMarker = new QComboBox();
                    comboBoxMarker->addItems(markerIDs);
                    comboBoxMarker->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
                    m_gridLayoutViaPoints->addWidget(comboBoxMarker, i, 1, Qt::AlignTop);
                    m_viaPointLabelList.push_back(label);
                    m_viaPointComboBoxList.push_back(comboBoxMarker);
                    comboBoxMarker->setCurrentText(QString::fromStdString(result[size_t(i)]));
                }
                QSpacerItem *gridSpacerViaPoints = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
                m_gridLayoutViaPoints->addItem(gridSpacerViaPoints, ui->spinBoxNViaPoints->value(), 0);
            }
        }
        ui->tabWidgetStrap->setCurrentIndex(tabNamesStrap.indexOf("N-Point"));
    }

    CylinderWrapStrap *cylinderWrapStrap = dynamic_cast<CylinderWrapStrap *>(m_inputMuscle->GetStrap());
    if (cylinderWrapStrap)
    {
        if ((s = cylinderWrapStrap->findAttribute("CylinderMarkerID"s)).size()) ui->comboBoxCylinderMarker->setCurrentText(QString::fromStdString(s));
        if ((s = cylinderWrapStrap->findAttribute("CylinderRadius"s)).size()) ui->lineEditCylinderRadius->setValue(GSUtil::Double(s));
        ui->tabWidgetStrap->setCurrentIndex(tabNamesStrap.indexOf("Cylinder"));
    }

    TwoCylinderWrapStrap *twoCylinderWrapStrap = dynamic_cast<TwoCylinderWrapStrap *>(m_inputMuscle->GetStrap());
    if (twoCylinderWrapStrap)
    {
        if ((s = twoCylinderWrapStrap->findAttribute("Cylinder1MarkerID"s)).size()) ui->comboBox2Cylinder1Marker->setCurrentText(QString::fromStdString(s));
        if ((s = twoCylinderWrapStrap->findAttribute("Cylinder1Radius"s)).size()) ui->lineEdit2Cylinder1Radius->setValue(GSUtil::Double(s));
        if ((s = twoCylinderWrapStrap->findAttribute("Cylinder2MarkerID"s)).size()) ui->comboBox2Cylinder2Marker->setCurrentText(QString::fromStdString(s));
        if ((s = twoCylinderWrapStrap->findAttribute("Cylinder2Radius"s)).size()) ui->lineEdit2Cylinder2Radius->setValue(GSUtil::Double(s));
        ui->tabWidgetStrap->setCurrentIndex(tabNamesStrap.indexOf("2-Cylinder"));
    }

    if (m_inputMuscle->GetStrap()->torqueMarkerList().size())
    {
        const QSignalBlocker blocker(ui->spinBoxNTorqueMarkers);
        ui->spinBoxNTorqueMarkers->setValue(int(m_inputMuscle->GetStrap()->torqueMarkerList().size()));
        for (int i = 0; i < ui->spinBoxNTorqueMarkers->value(); i++)
        {
            QLabel *label = new QLabel();
            label->setText(QString("Torque Marker %1").arg(i));
            m_gridLayoutTorqueMarkers->addWidget(label, i, 0, Qt::AlignTop);
            QComboBox *comboBoxMarker = new QComboBox();
            comboBoxMarker->addItems(markerIDs);
            comboBoxMarker->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            m_gridLayoutTorqueMarkers->addWidget(comboBoxMarker, i, 1, Qt::AlignTop);
            m_torqueMarkerLabelList.push_back(label);
            m_torqueMarkerComboBoxList.push_back(comboBoxMarker);
            comboBoxMarker->setCurrentText(QString::fromStdString(m_inputMuscle->GetStrap()->torqueMarkerList().at(i)->name()));
        }
        QSpacerItem *gridSpacerTorqueMarkers = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        m_gridLayoutTorqueMarkers->addItem(gridSpacerTorqueMarkers, ui->spinBoxNTorqueMarkers->value(), 0);
    }

}

void DialogMuscles::tabChanged(int /*index*/)
{
    updateActivation();
}

void DialogMuscles::comboBoxChanged(int /*index*/)
{
    updateActivation();
}

void DialogMuscles::lineEditChanged(const QString &/*text*/)
{
    updateActivation();
}

void DialogMuscles::spinBoxChanged(const QString &/*text*/)
{

    if (this->sender() == ui->spinBoxNViaPoints)
    {
        // get the lists in the right formats
        auto markerList = m_simulation->GetMarkerList();
        QStringList markerIDs;
        for (auto it = markerList->begin(); it != markerList->end(); it++) markerIDs.append(QString::fromStdString(it->first));

        // store the current values in the list
        QVector<QString> oldValues(m_viaPointComboBoxList.size());
        for (int i = 0; i < m_viaPointComboBoxList.size(); i++) oldValues[i] = m_viaPointComboBoxList[i]->currentText();

        // delete all the existing widgets in the layout
        QLayoutItem *child;
        while ((child = m_gridLayoutViaPoints->takeAt(0)) != nullptr)
        {
            delete child->widget(); // delete the widget
            delete child;   // delete the layout item
        }
        m_viaPointLabelList.clear();
        m_viaPointComboBoxList.clear();

        // now create a new set
        int requiredViaPoints = ui->spinBoxNViaPoints->value();
        for (int i = 0; i < requiredViaPoints; i++)
        {
            QLabel *label = new QLabel();
            label->setText(QString("Via Point %1").arg(i));
            m_gridLayoutViaPoints->addWidget(label, i, 0, Qt::AlignTop);
            QComboBox *comboBoxMarker = new QComboBox();
            comboBoxMarker->addItems(markerIDs);
            comboBoxMarker->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            if (i < oldValues.size()) comboBoxMarker->setCurrentText(oldValues[i]);
            m_gridLayoutViaPoints->addWidget(comboBoxMarker, i, 1, Qt::AlignTop);
            m_viaPointLabelList.push_back(label);
            m_viaPointComboBoxList.push_back(comboBoxMarker);
        }
        QSpacerItem *gridSpacerViaPoints = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        m_gridLayoutViaPoints->addItem(gridSpacerViaPoints, requiredViaPoints, 0);
    }

    if (this->sender() == ui->spinBoxNTorqueMarkers)
    {
        // get the lists in the right formats
        auto markerList = m_simulation->GetMarkerList();
        QStringList markerIDs;
        for (auto it = markerList->begin(); it != markerList->end(); it++) markerIDs.append(QString::fromStdString(it->first));

        // store the current values in the list
        QVector<QString> oldValues(m_torqueMarkerComboBoxList.size());
        for (int i = 0; i < m_torqueMarkerComboBoxList.size(); i++) oldValues[i] = m_torqueMarkerComboBoxList[i]->currentText();

        // delete all the existing widgets in the layout
        QLayoutItem *child;
        while ((child = m_gridLayoutTorqueMarkers->takeAt(0)) != nullptr)
        {
            delete child->widget(); // delete the widget
            delete child;   // delete the layout item
        }
        m_torqueMarkerLabelList.clear();
        m_torqueMarkerComboBoxList.clear();

        // now create a new set
        int requiredTorqueMarkers = ui->spinBoxNTorqueMarkers->value();
        for (int i = 0; i < requiredTorqueMarkers; i++)
        {
            QLabel *label = new QLabel();
            label->setText(QString("Torque Marker %1").arg(i));
            m_gridLayoutTorqueMarkers->addWidget(label, i, 0, Qt::AlignTop);
            QComboBox *comboBoxMarker = new QComboBox();
            comboBoxMarker->addItems(markerIDs);
            comboBoxMarker->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            if (i < oldValues.size()) comboBoxMarker->setCurrentText(oldValues[i]);
            m_gridLayoutTorqueMarkers->addWidget(comboBoxMarker, i, 1, Qt::AlignTop);
            m_torqueMarkerLabelList.push_back(label);
            m_torqueMarkerComboBoxList.push_back(comboBoxMarker);
        }
        QSpacerItem *gridSpacerTorqueMarkers = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        m_gridLayoutTorqueMarkers->addItem(gridSpacerTorqueMarkers, requiredTorqueMarkers, 0);
    }

    updateActivation();
}

void DialogMuscles::checkBoxChanged(int /*index*/)
{
    updateActivation();
}


void DialogMuscles::updateActivation()
{
    bool okEnable = true;
    QString textCopy = ui->lineEditMuscleID->text();
    int pos = ui->lineEditMuscleID->cursorPosition();
    if (ui->lineEditMuscleID->validator()->validate(textCopy, pos) != QValidator::Acceptable) okEnable = false;

    ui->pushButtonOK->setEnabled(okEnable);
}

void DialogMuscles::properties()
{
    DialogProperties dialogProperties(this);

    SettingsItem strapColour = Preferences::settingsItem("StrapColour");
    SettingsItem strapCylinderColour = Preferences::settingsItem("StrapCylinderColour");
    SettingsItem strapForceColour = Preferences::settingsItem("StrapForceColour");
    SettingsItem strapRadius = Preferences::settingsItem("StrapRadius");
    SettingsItem strapCylinderLength = Preferences::settingsItem("StrapCylinderLength");
    SettingsItem strapForceRadius = Preferences::settingsItem("StrapForceRadius");
    SettingsItem strapForceScale = Preferences::settingsItem("StrapForceScale");
    if (m_inputMuscle)
    {
        strapColour.value = QColor(QString::fromStdString(m_inputMuscle->GetStrap()->colour1().GetHexArgb()));
        strapCylinderColour.value = QColor(QString::fromStdString(m_inputMuscle->GetStrap()->colour2().GetHexArgb()));
        strapForceColour.value = QColor(QString::fromStdString(m_inputMuscle->colour1().GetHexArgb()));
        strapRadius.value = m_inputMuscle->GetStrap()->size1();
        strapCylinderLength.value = m_inputMuscle->GetStrap()->size2();
        strapForceRadius.value = m_inputMuscle->size1();
        strapForceScale.value = m_inputMuscle->size2();
    }
    m_properties.clear();
    m_properties = { { strapColour.key, strapColour },
                     { strapCylinderColour.key, strapCylinderColour},
                     { strapForceColour.key, strapForceColour},
                     { strapRadius.key, strapRadius },
                     { strapCylinderLength.key, strapCylinderLength },
                     { strapForceRadius.key, strapForceRadius },
                     { strapForceScale.key, strapForceScale } };
    dialogProperties.setInputSettingsItems(m_properties);
    dialogProperties.initialise();

    int status = dialogProperties.exec();
    if (status == QDialog::Accepted)
    {
        dialogProperties.update();
        m_properties = dialogProperties.getOutputSettingsItems();
    }
}

Simulation *DialogMuscles::simulation() const
{
    return m_simulation;
}

void DialogMuscles::setSimulation(Simulation *simulation)
{
    m_simulation = simulation;
}

