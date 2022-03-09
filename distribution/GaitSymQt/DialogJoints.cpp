#include "DialogJoints.h"
#include "ui_DialogJoints.h"

#include "Preferences.h"
#include "Simulation.h"
#include "Joint.h"
#include "Body.h"
#include "Marker.h"
#include "HingeJoint.h"
#include "UniversalJoint.h"
#include "FixedJoint.h"
#include "BallJoint.h"
#include "FloatingHingeJoint.h"
#include "GSUtil.h"
#include "DialogProperties.h"
#include "Filter.h"

#include <QDebug>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QFileInfo>
#include <QMessageBox>

DialogJoints::DialogJoints(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogJoints)
{
    ui->setupUi(this);

    setWindowTitle(tr("Joint Builder"));
#ifdef Q_OS_MACOS
    setWindowFlags(windowFlags() & (~Qt::Dialog) | Qt::Window); // allows the window to be resized on macs
#endif
    restoreGeometry(Preferences::valueQByteArray("DialogJointsGeometry"));

    connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->pushButtonProperties, SIGNAL(clicked()), this, SLOT(properties()));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

    // this logic monitors for changing values
    QList<QWidget *> widgets = this->findChildren<QWidget *>();
    for (auto it = widgets.begin(); it != widgets.end(); it++)
    {
        QComboBox *comboBox = dynamic_cast<QComboBox *>(*it);
        if (comboBox) connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxChanged(int)));
        QLineEdit *lineEdit = dynamic_cast<QLineEdit *>(*it);
        if (lineEdit) connect(lineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(lineEditChanged(const QString &)));
        QSpinBox *spinBox = dynamic_cast<QSpinBox *>(*it);
        if (spinBox) connect(spinBox, SIGNAL(valueChanged(int)), this, SLOT(spinBoxChanged(int)));
        QCheckBox *checkBox = dynamic_cast<QCheckBox *>(*it);
        if (checkBox) connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(checkBoxChanged(int)));
    }

    ui->pushButtonOK->setEnabled(false);
}

DialogJoints::~DialogJoints()
{
    delete ui;
}

void DialogJoints::accept() // this catches OK and return/enter
{
    qDebug() << "DialogJoints::accept()";

    std::map<std::string, std::unique_ptr<Marker>> *markerList = m_simulation->GetMarkerList();
    QString tab = ui->tabWidget->tabText(ui->tabWidget->currentIndex());
    if (tab == "Hinge")
    {
        std::unique_ptr<HingeJoint> joint = std::make_unique<HingeJoint>(m_simulation->GetWorldID());
        joint->setName(ui->lineEditJointID->text().toStdString());
        joint->setSimulation(m_simulation);
        joint->setBody1Marker(markerList->at(ui->comboBoxMarker1->currentText().toStdString()).get());
        joint->setBody2Marker(markerList->at(ui->comboBoxMarker2->currentText().toStdString()).get());
        joint->Attach();
        pgd::Vector3 anchor = joint->body1Marker()->GetWorldPosition();
        pgd::Vector3 axis = joint->body1Marker()->GetWorldAxis(Marker::Axis::X);
        joint->SetHingeAnchor(anchor.x, anchor.y, anchor.z);
        joint->SetHingeAxis(axis.x, axis.y, axis.z);
        if (ui->lineEditCFM->text().size()) joint->setCFM(ui->lineEditCFM->value());
        if (ui->lineEditERP->text().size()) joint->setERP(ui->lineEditERP->value());
        if (ui->lineEditHingeLowStop->text().size() && ui->lineEditHingeHighStop->text().size())
            joint->SetJointStops(pgd::DegreesToRadians(ui->lineEditHingeLowStop->value()), pgd::DegreesToRadians(ui->lineEditHingeHighStop->value()));
        if (ui->lineEditHingeLowStopTorqueLimit->text().size() && ui->lineEditHingeHighStopTorqueLimit->text().size())
            joint->SetTorqueLimits(ui->lineEditHingeLowStopTorqueLimit->value(), ui->lineEditHingeHighStopTorqueLimit->value());
        joint->SetStopTorqueWindow(ui->spinBoxHingeTorqueWindow->value());
        if (ui->lineEditHingeStopCFM->text().size()) joint->SetStopCFM(ui->lineEditHingeStopCFM->value());
        if (ui->lineEditHingeStopERP->text().size()) joint->SetStopERP(ui->lineEditHingeStopERP->value());
        joint->SetStopBounce(ui->lineEditHingeStopBounce->value());
        m_outputJoint = std::move(joint);
    }

    else if (tab == "Ball")
    {
        std::unique_ptr<BallJoint> joint;
        QString mode = ui->comboBoxBallMode->currentText();
        BallJoint::Mode iMode = BallJoint::NoStops;
        if (mode == "No Stops") iMode = BallJoint::NoStops;
        else if (mode == "Fixed Euler") iMode = BallJoint::AMotorEuler;
        else if (mode == "User Euler") iMode = BallJoint::AMotorUser;
        joint = std::make_unique<BallJoint>(m_simulation->GetWorldID(), iMode);
        joint->setName(ui->lineEditJointID->text().toStdString());
        joint->setSimulation(m_simulation);
        joint->setBody1Marker(markerList->at(ui->comboBoxMarker1->currentText().toStdString()).get());
        joint->setBody2Marker(markerList->at(ui->comboBoxMarker2->currentText().toStdString()).get());
        joint->Attach();
        pgd::Vector3 anchor = joint->body1Marker()->GetWorldPosition();
        pgd::Vector3 x, y, z;
        joint->body1Marker()->GetWorldBasis(&x, &y, &z);
        joint->SetBallAnchor(anchor.x, anchor.y, anchor.z);
        joint->SetAxes(x.x, x.y, x.z, y.x, y.y, y.z, z.x, z.y, z.z, static_cast<int>(iMode));
        if (ui->lineEditCFM->text().size()) joint->setCFM(ui->lineEditCFM->value());
        if (ui->lineEditERP->text().size()) joint->setERP(ui->lineEditERP->value());
        if (iMode == BallJoint::AMotorEuler || iMode == BallJoint::AMotorUser)
        {
            joint->SetStops(pgd::DegreesToRadians(ui->lineEditBallLowStop0->value()),
                            pgd::DegreesToRadians(ui->lineEditBallHighStop0->value()),
                            pgd::DegreesToRadians(ui->lineEditBallLowStop1->value()),
                            pgd::DegreesToRadians(ui->lineEditBallHighStop1->value()),
                            pgd::DegreesToRadians(ui->lineEditBallLowStop2->value()),
                            pgd::DegreesToRadians(ui->lineEditBallHighStop2->value()));
        }
        m_outputJoint = std::move(joint);
    }

    else if (tab == "Floating Hinge")
    {
        std::unique_ptr<FloatingHingeJoint> joint = std::make_unique<FloatingHingeJoint>(m_simulation->GetWorldID());
        joint->setName(ui->lineEditJointID->text().toStdString());
        joint->setSimulation(m_simulation);
        joint->setBody1Marker(markerList->at(ui->comboBoxMarker1->currentText().toStdString()).get());
        joint->setBody2Marker(markerList->at(ui->comboBoxMarker2->currentText().toStdString()).get());
        joint->Attach();
        pgd::Vector3 axis = joint->body1Marker()->GetWorldAxis(Marker::Axis::X);
        joint->SetFloatingHingeAxis(axis.x, axis.y, axis.z);
        if (ui->lineEditCFM->text().size()) joint->setCFM(ui->lineEditCFM->value());
        if (ui->lineEditERP->text().size()) joint->setERP(ui->lineEditERP->value());
        if (ui->lineEditFloatingHingeLowStop->text().size() && ui->lineEditFloatingHingeHighStop->text().size())
            joint->SetJointStops(pgd::DegreesToRadians(ui->lineEditFloatingHingeLowStop->value()), pgd::DegreesToRadians(ui->lineEditFloatingHingeHighStop->value()));
        m_outputJoint = std::move(joint);
    }

    else if (tab == "Fixed")
    {
        std::unique_ptr<FixedJoint> joint = std::make_unique<FixedJoint>(m_simulation->GetWorldID());
        joint->setName(ui->lineEditJointID->text().toStdString());
        joint->setSimulation(m_simulation);
        joint->setBody1Marker(markerList->at(ui->comboBoxMarker1->currentText().toStdString()).get());
        joint->setBody2Marker(markerList->at(ui->comboBoxMarker2->currentText().toStdString()).get());
        joint->Attach();
        joint->SetFixed();
        joint->setLateFix(ui->checkBoxLateFix->isChecked());
        if (ui->lineEditCFM->text().size()) joint->setCFM(ui->lineEditCFM->value());
        if (ui->lineEditERP->text().size()) joint->setERP(ui->lineEditERP->value());
        QString mode = ui->comboBoxFixedStressCalculationType->currentText();
        if (mode != "None")
        {
            QImage image;
            if (image.load(ui->lineEditFixedStressBitmap->text()) == false || image.height() == 0 || image.width() == 0)
            {
                QMessageBox::warning(this, tr("Create Joint"), QString("Error loading %1\nUnable to create joint.").arg(ui->lineEditFixedStressBitmap->text()));
                return;
            }
            std::vector<unsigned char> stiffnessMap(size_t(image.height()) * size_t(image.width()));
            size_t count = 0;
            for (int iy = 0; iy < image.height(); iy++)
            {
                for (int ix = 0; ix < image.width(); ix++)
                {
                    // this is slow but robust
                    int grey = qGray(image.pixel(ix, iy));
                    stiffnessMap[count++] = (grey == 0 ? 0 : 1);
                }
            }
            // remember that SetCrossSection takes ownership of the stiffnessMap
            joint->SetCrossSection(stiffnessMap, size_t(image.width()), size_t(image.height()),
                                   ui->lineEditFixedStressBitmapPixelSize->value(), ui->lineEditFixedStressBitmapPixelSize->value());

            if (mode == "Beam Low Pass")
            {
                joint->SetStressCalculationType(FixedJoint::beam);
                joint->SetLowPassType(FixedJoint::Butterworth2ndOrderLowPass);
            }
            else if (mode == "Spring Low Pass")
            {
                joint->SetStressCalculationType(FixedJoint::spring);
                joint->SetLowPassType(FixedJoint::Butterworth2ndOrderLowPass);
            }
            else if (mode == "Beam Moving Average")
            {
                joint->SetStressCalculationType(FixedJoint::beam);
                joint->SetLowPassType(FixedJoint::MovingAverageLowPass);
            }
            else if (mode == "Spring Moving Average")
            {
                joint->SetStressCalculationType(FixedJoint::spring);
                joint->SetLowPassType(FixedJoint::MovingAverageLowPass);
            }
            joint->SetWindow(size_t(ui->spinBoxFixedStressWindow->value()));
            joint->SetCutoffFrequency(ui->lineEditFixedStressCutoffFrequency->value());
            joint->SetStressLimit(ui->lineEditFixedStressLimit->value());
        }
        m_outputJoint = std::move(joint);
    }

    else if (tab == "Universal")
    {
        std::unique_ptr<UniversalJoint> joint = std::make_unique<UniversalJoint>(m_simulation->GetWorldID());
        joint->setName(ui->lineEditJointID->text().toStdString());
        joint->setSimulation(m_simulation);
        joint->setBody1Marker(markerList->at(ui->comboBoxMarker1->currentText().toStdString()).get());
        joint->setBody2Marker(markerList->at(ui->comboBoxMarker2->currentText().toStdString()).get());
        joint->Attach();
        pgd::Vector3 anchor = joint->body1Marker()->GetWorldPosition();
        pgd::Vector3 x, y, z;
        joint->body1Marker()->GetWorldBasis(&x, &y, &z);
        joint->SetUniversalAnchor(anchor.x, anchor.y, anchor.z);
        joint->SetUniversalAxis1(x.x, x.y, x.z);
        joint->SetUniversalAxis2(y.x, y.y, y.z);
        if (ui->lineEditCFM->text().size()) joint->setCFM(ui->lineEditCFM->value());
        if (ui->lineEditERP->text().size()) joint->setERP(ui->lineEditERP->value());
        if (ui->lineEditUniversalLowStop1->text().size() && ui->lineEditUniversalHighStop1->text().size())
            joint->SetJointStops1(pgd::DegreesToRadians(ui->lineEditUniversalLowStop1->value()), pgd::DegreesToRadians(ui->lineEditUniversalHighStop1->value()));
        if (ui->lineEditUniversalStopCFM1->text().size()) joint->SetStopCFM1(ui->lineEditUniversalStopCFM1->value());
        if (ui->lineEditUniversalStopERP1->text().size()) joint->SetStopERP1(ui->lineEditUniversalStopERP1->value());
        joint->SetStopBounce1(ui->lineEditUniversalStopBounce1->value());
        if (ui->lineEditUniversalLowStop2->text().size() && ui->lineEditUniversalHighStop2->text().size())
            joint->SetJointStops2(pgd::DegreesToRadians(ui->lineEditUniversalLowStop2->value()), pgd::DegreesToRadians(ui->lineEditUniversalHighStop2->value()));
        if (ui->lineEditUniversalStopCFM2->value() != 0.0) joint->SetStopCFM2(ui->lineEditUniversalStopCFM2->value());
        if (ui->lineEditUniversalStopERP2->value() != 0.0) joint->SetStopERP2(ui->lineEditUniversalStopERP2->value());
        joint->SetStopBounce2(ui->lineEditUniversalStopBounce2->value());
        m_outputJoint = std::move(joint);
    }

    if (m_inputJoint)
    {
        m_outputJoint->setColour1(m_inputJoint->colour1());
        m_outputJoint->setSize1(m_inputJoint->size1());
    }
    else
    {
        m_outputJoint->setColour1(Preferences::valueQColor("JointColour").name(QColor::HexArgb).toStdString());
        m_outputJoint->setSize1(Preferences::valueDouble("JointAxesSize"));
    }

    m_outputJoint->saveToAttributes();
    m_outputJoint->createFromAttributes();

    if (m_properties.size() > 0)
    {
        if (m_properties.count("JointColour"))
            m_outputJoint->setColour1(qvariant_cast<QColor>(m_properties["JointColour"].value).name(QColor::HexArgb).toStdString());
        if (m_properties.count("JointAxesSize"))
            m_outputJoint->setSize1(m_properties["JointAxesSize"].value.toDouble());
    }


    Preferences::insert("DialogJointsGeometry", saveGeometry());
    QDialog::accept();
}

void DialogJoints::reject() // this catches cancel, close and escape key
{
    qDebug() << "DialogJoints::reject()";
    Preferences::insert("DialogJointsGeometry", saveGeometry());
    QDialog::reject();
}

void DialogJoints::closeEvent(QCloseEvent *event)
{
    qDebug() << "DialogJoints::closeEvent()";
    Preferences::insert("DialogJointsGeometry", saveGeometry());

    QDialog::closeEvent(event);
}

void DialogJoints::lateInitialise()
{
    Q_ASSERT_X(m_simulation, "DialogJoints::lateInitialise", "m_simulation undefined");

    // get the lists in the right formats
    std::map<std::string, std::unique_ptr<Marker>> *markerList = m_simulation->GetMarkerList();
    QStringList markerIDs;
    for (auto it = markerList->begin(); it != markerList->end(); it++)
        markerIDs.append(QString::fromStdString(it->first));

    // set the marker lists
    ui->comboBoxMarker1->addItems(markerIDs);
    ui->comboBoxMarker2->addItems(markerIDs);

    // now set some sensible defaults
    ui->lineEditHingeLowStopTorqueLimit->setBottom(0);
    ui->lineEditHingeHighStopTorqueLimit->setBottom(0);

    ui->lineEditFixedStressBitmapPixelSize->setBottom(1e-100);
    ui->lineEditFixedStressLimit->setBottom(0);
    ui->lineEditFixedStressCutoffFrequency->setBottom(0);

    ui->lineEditHingeLowStop->setValue(-std::numeric_limits<double>::max());
    ui->lineEditHingeHighStop->setValue(+std::numeric_limits<double>::max());
    ui->lineEditFloatingHingeLowStop->setValue(-std::numeric_limits<double>::max());
    ui->lineEditFloatingHingeHighStop->setValue(+std::numeric_limits<double>::max());
    ui->lineEditUniversalLowStop1->setValue(-std::numeric_limits<double>::max());
    ui->lineEditUniversalHighStop1->setValue(+std::numeric_limits<double>::max());
    ui->lineEditUniversalLowStop2->setValue(-std::numeric_limits<double>::max());
    ui->lineEditUniversalHighStop2->setValue(+std::numeric_limits<double>::max());

    if (!m_inputJoint)
    {
        // set default new name
        auto nameSet = simulation()->GetNameSet();
        ui->lineEditJointID->addStrings(nameSet);
        int initialNameCount = 0;
        QString initialName = QString("Joint%1").arg(initialNameCount, 3, 10, QLatin1Char('0'));
        while (nameSet.count(initialName.toStdString()))
        {
            initialNameCount++;
            initialName = QString("Joint%1").arg(initialNameCount, 3, 10, QLatin1Char('0'));
            if (initialNameCount >= 999) break; // only do this for the first 999 markers
        }
        ui->lineEditJointID->setText(initialName);
        return;
    }

    QStringList tabNames;
    for (int i = 0; i < ui->tabWidget->count(); i++) tabNames.push_back(ui->tabWidget->tabText(i));

    std::string s;
    m_inputJoint->saveToAttributes();
    ui->lineEditJointID->setText(QString::fromStdString(m_inputJoint->findAttribute("ID"s)));
    ui->lineEditJointID->setEnabled(false);
    ui->comboBoxMarker1->setCurrentText(QString::fromStdString(m_inputJoint->findAttribute("Body1MarkerID"s)));
    ui->comboBoxMarker2->setCurrentText(QString::fromStdString(m_inputJoint->findAttribute("Body2MarkerID"s)));
    if ((s = m_inputJoint->findAttribute("CFM"s)).size()) ui->lineEditCFM->setValue(GSUtil::Double(s));
    if ((s = m_inputJoint->findAttribute("ERP"s)).size()) ui->lineEditERP->setValue(GSUtil::Double(s));

    HingeJoint *hingeJoint = dynamic_cast<HingeJoint *>(m_inputJoint);
    if (hingeJoint)
    {
        if ((s = m_inputJoint->findAttribute("LowStop"s)).size()) ui->lineEditHingeLowStop->setValue(pgd::RadiansToDegrees(GSUtil::Double(s)));
        if ((s = m_inputJoint->findAttribute("HighStop"s)).size()) ui->lineEditHingeHighStop->setValue(pgd::RadiansToDegrees(GSUtil::Double(s)));
        if ((s = m_inputJoint->findAttribute("LowStopTorqueLimit"s)).size()) ui->lineEditHingeLowStopTorqueLimit->setValue(GSUtil::Double(s));
        if ((s = m_inputJoint->findAttribute("HighStopTorqueLimit"s)).size()) ui->lineEditHingeHighStopTorqueLimit->setValue(GSUtil::Double(s));
        if ((s = m_inputJoint->findAttribute("StopTorqueWindow"s)).size()) ui->spinBoxHingeTorqueWindow->setValue(GSUtil::Int(s));
        if ((s = m_inputJoint->findAttribute("StopCFM"s)).size()) ui->lineEditHingeStopCFM->setValue(GSUtil::Double(s));
        if ((s = m_inputJoint->findAttribute("StopERP"s)).size()) ui->lineEditHingeStopERP->setValue(GSUtil::Double(s));
        if ((s = m_inputJoint->findAttribute("StopBounce"s)).size()) ui->lineEditHingeStopBounce->setValue(GSUtil::Double(s));
        ui->tabWidget->setCurrentIndex(tabNames.indexOf("Hinge"));
    }

    BallJoint *ballJoint = dynamic_cast<BallJoint *>(m_inputJoint);
    if (ballJoint)
    {
        switch (ballJoint->GetMode())
        {
        case BallJoint::NoStops:
            ui->comboBoxBallMode->setCurrentText("No Stops");
            break;
        case BallJoint::AMotorEuler:
            ui->comboBoxBallMode->setCurrentText("Fixed Euler");
            break;
        case BallJoint::AMotorUser:
            ui->comboBoxBallMode->setCurrentText("User Euler");
            break;
        }
        if ((s = m_inputJoint->findAttribute("LowStop0"s)).size()) ui->lineEditBallLowStop0->setValue(pgd::RadiansToDegrees(GSUtil::Double(s)));
        if ((s = m_inputJoint->findAttribute("HighStop0"s)).size()) ui->lineEditBallHighStop0->setValue(pgd::RadiansToDegrees(GSUtil::Double(s)));
        if ((s = m_inputJoint->findAttribute("LowStop1"s)).size()) ui->lineEditBallLowStop1->setValue(pgd::RadiansToDegrees(GSUtil::Double(s)));
        if ((s = m_inputJoint->findAttribute("HighStop1"s)).size()) ui->lineEditBallHighStop1->setValue(pgd::RadiansToDegrees(GSUtil::Double(s)));
        if ((s = m_inputJoint->findAttribute("LowStop2"s)).size()) ui->lineEditBallLowStop2->setValue(pgd::RadiansToDegrees(GSUtil::Double(s)));
        if ((s = m_inputJoint->findAttribute("HighStop2"s)).size()) ui->lineEditBallHighStop2->setValue(pgd::RadiansToDegrees(GSUtil::Double(s)));
        ui->tabWidget->setCurrentIndex(tabNames.indexOf("Ball"));
    }

    FloatingHingeJoint *floatingHingeJoint = dynamic_cast<FloatingHingeJoint *>(m_inputJoint);
    if (floatingHingeJoint)
    {
        if ((s = m_inputJoint->findAttribute("LowStop"s)).size()) ui->lineEditFloatingHingeLowStop->setValue(pgd::RadiansToDegrees(GSUtil::Double(s)));
        if ((s = m_inputJoint->findAttribute("HighStop"s)).size()) ui->lineEditFloatingHingeHighStop->setValue(pgd::RadiansToDegrees(GSUtil::Double(s)));
        ui->tabWidget->setCurrentIndex(tabNames.indexOf("Floating Hinge"));
    }

    FixedJoint *fixedJoint = dynamic_cast<FixedJoint *>(m_inputJoint);
    if (fixedJoint)
    {
        ui->checkBoxLateFix->setChecked(fixedJoint->lateFix());
        std::string stressCalculationType = m_inputJoint->findAttribute("StressCalculationType"s);
        std::string lowPassType = m_inputJoint->findAttribute("LowPassType"s);
        QString text;
        if (stressCalculationType == "None"s) text = "None";
        else
        {
            if (stressCalculationType == "Beam"s) text = "Beam";
            if (stressCalculationType == "Spring"s) text = "Spring";
            if (lowPassType == "Butterworth2ndOrderLowPass"s) text.append(" Low Pass");
            if (lowPassType == "MovingAverageLowPass"s) text.append(" Moving Average");
        }
        ui->comboBoxFixedStressCalculationType->setCurrentIndex(ui->comboBoxFixedStressCalculationType->findText(text));
        if ((s = m_inputJoint->findAttribute("CutoffFrequency"s)).size()) ui->lineEditFixedStressCutoffFrequency->setValue(GSUtil::Double(s));
        if ((s = m_inputJoint->findAttribute("Window"s)).size()) ui->spinBoxFixedStressWindow->setValue(GSUtil::Int(s));
        if ((s = m_inputJoint->findAttribute("StressLimit"s)).size()) ui->lineEditFixedStressLimit->setValue(GSUtil::Double(s));
        if ((s = m_inputJoint->findAttribute("StressBitmapPixelSize"s)).size()) ui->lineEditFixedStressBitmapPixelSize->setValue(GSUtil::Double(s));
        if ((s = m_inputJoint->findAttribute("StressBitmap"s)).size()) ui->lineEditFixedStressBitmap->setText(QString::fromStdString(s));
        ui->tabWidget->setCurrentIndex(tabNames.indexOf("Fixed"));
    }

    UniversalJoint *universalJoint = dynamic_cast<UniversalJoint *>(m_inputJoint);
    if (universalJoint)
    {
        if ((s = m_inputJoint->findAttribute("LowStop1"s)).size()) ui->lineEditUniversalLowStop1->setValue(pgd::RadiansToDegrees(GSUtil::Double(s)));
        if ((s = m_inputJoint->findAttribute("HighStop1"s)).size()) ui->lineEditUniversalHighStop1->setValue(pgd::RadiansToDegrees(GSUtil::Double(s)));
        if ((s = m_inputJoint->findAttribute("StopCFM1"s)).size()) ui->lineEditUniversalStopCFM1->setValue(GSUtil::Double(s));
        if ((s = m_inputJoint->findAttribute("StopERP1"s)).size()) ui->lineEditUniversalStopERP1->setValue(GSUtil::Double(s));
        if ((s = m_inputJoint->findAttribute("StopBounce1"s)).size()) ui->lineEditUniversalStopBounce1->setValue(GSUtil::Double(s));
        if ((s = m_inputJoint->findAttribute("LowStop2"s)).size()) ui->lineEditUniversalLowStop2->setValue(pgd::RadiansToDegrees(GSUtil::Double(s)));
        if ((s = m_inputJoint->findAttribute("HighStop2"s)).size()) ui->lineEditUniversalHighStop2->setValue(pgd::RadiansToDegrees(GSUtil::Double(s)));
        if ((s = m_inputJoint->findAttribute("StopCFM2"s)).size()) ui->lineEditUniversalStopCFM2->setValue(GSUtil::Double(s));
        if ((s = m_inputJoint->findAttribute("StopERP2"s)).size()) ui->lineEditUniversalStopERP2->setValue(GSUtil::Double(s));
        if ((s = m_inputJoint->findAttribute("StopBounce2"s)).size()) ui->lineEditUniversalStopBounce2->setValue(GSUtil::Double(s));
        ui->tabWidget->setCurrentIndex(tabNames.indexOf("Universal"));
    }
}

void DialogJoints::tabChanged(int /*index*/)
{
    updateActivation();
}

void DialogJoints::comboBoxChanged(int /*index*/)
{
    updateActivation();
}

void DialogJoints::lineEditChanged(const QString & /*text*/)
{
    updateActivation();
}

void DialogJoints::spinBoxChanged(int /*value*/)
{
    updateActivation();
}

void DialogJoints::checkBoxChanged(int /*index*/)
{
    updateActivation();
}


void DialogJoints::updateActivation()
{
    bool okEnable = true;
    QString tab = ui->tabWidget->tabText(ui->tabWidget->currentIndex());
    QString textCopy = ui->lineEditJointID->text();
    int pos = ui->lineEditJointID->cursorPosition();
    if (ui->lineEditJointID->validator()->validate(textCopy, pos) != QValidator::Acceptable) okEnable = false;
    auto markerList = m_simulation->GetMarkerList();

    if (markerList->count(ui->comboBoxMarker1->currentText().toStdString()) == 0
            || markerList->count(ui->comboBoxMarker2->currentText().toStdString()) == 0) okEnable = false;
    else if (markerList->at(ui->comboBoxMarker1->currentText().toStdString())->GetBody() ==
             markerList->at(ui->comboBoxMarker2->currentText().toStdString())->GetBody()) okEnable = false;

    if (tab == "Hinge")
    {
        if (ui->lineEditHingeLowStop->value() >= ui->lineEditHingeHighStop->value())
            okEnable = false;
    }

    else if (tab == "Ball")
    {
        QString mode = ui->comboBoxBallMode->currentText();
        if (mode == "No Stops")
        {
            ui->lineEditBallLowStop0->setEnabled(false);
            ui->lineEditBallLowStop1->setEnabled(false);
            ui->lineEditBallLowStop2->setEnabled(false);
            ui->lineEditBallHighStop0->setEnabled(false);
            ui->lineEditBallHighStop1->setEnabled(false);
            ui->lineEditBallHighStop2->setEnabled(false);
        }
        else if (mode == "Fixed Euler")
        {
            ui->lineEditBallLowStop0->setEnabled(true);
            ui->lineEditBallLowStop1->setEnabled(true);
            ui->lineEditBallLowStop2->setEnabled(true);
            ui->lineEditBallHighStop0->setEnabled(true);
            ui->lineEditBallHighStop1->setEnabled(true);
            ui->lineEditBallHighStop2->setEnabled(true);
            if (ui->lineEditBallLowStop0->value() >= ui->lineEditBallHighStop0->value() ||
                    ui->lineEditBallLowStop1->value() >= ui->lineEditBallHighStop1->value() ||
                    ui->lineEditBallLowStop2->value() >= ui->lineEditBallHighStop2->value()) okEnable = false;
        }
        else if (mode == "User Euler")
        {
            ui->lineEditBallLowStop0->setEnabled(true);
            ui->lineEditBallLowStop1->setEnabled(true);
            ui->lineEditBallLowStop2->setEnabled(true);
            ui->lineEditBallHighStop0->setEnabled(true);
            ui->lineEditBallHighStop1->setEnabled(true);
            ui->lineEditBallHighStop2->setEnabled(true);
            if (ui->lineEditBallLowStop0->value() >= ui->lineEditBallHighStop0->value() ||
                    ui->lineEditBallLowStop1->value() >= ui->lineEditBallHighStop1->value() ||
                    ui->lineEditBallLowStop2->value() >= ui->lineEditBallHighStop2->value()) okEnable = false;
        }
    }

    else if (tab == "Floating Hinge")
    {
        if (ui->lineEditFloatingHingeLowStop->value() >= ui->lineEditFloatingHingeHighStop->value())
            okEnable = false;
    }

    else if (tab == "Fixed")
    {
        QString mode = ui->comboBoxFixedStressCalculationType->currentText();
        if (mode == "None")
        {
            ui->lineEditFixedStressBitmap->setEnabled(false);
            ui->lineEditFixedStressBitmapPixelSize->setEnabled(false);
            ui->lineEditFixedStressLimit->setEnabled(false);
            ui->lineEditFixedStressCutoffFrequency->setEnabled(false);
            ui->spinBoxFixedStressWindow->setEnabled(false);
        }
        else
        {
            ui->lineEditFixedStressBitmap->setEnabled(true);
            ui->lineEditFixedStressBitmapPixelSize->setEnabled(true);
            ui->lineEditFixedStressLimit->setEnabled(true);
            if (mode == "Beam Low Pass" || mode == "Spring Low Pass")
            {
                ui->lineEditFixedStressCutoffFrequency->setEnabled(true);
                ui->spinBoxFixedStressWindow->setEnabled(false);
            }
            else
            {
                ui->lineEditFixedStressCutoffFrequency->setEnabled(false);
                ui->spinBoxFixedStressWindow->setEnabled(true);
            }
            QFileInfo fi(ui->lineEditFixedStressBitmap->text());
            if (!fi.isFile()) okEnable = false;
        }
    }

    else if (tab == "Universal")
    {
        if (ui->lineEditUniversalLowStop1->value() >= ui->lineEditUniversalHighStop1->value() ||
                ui->lineEditUniversalLowStop2->value() >= ui->lineEditUniversalHighStop2->value()) okEnable = false;
    }

    ui->pushButtonOK->setEnabled(okEnable);
}

void DialogJoints::properties()
{
    DialogProperties dialogProperties(this);

    SettingsItem jointAxesSize = Preferences::settingsItem("JointAxesSize");
    SettingsItem jointColour = Preferences::settingsItem("JointColour");
    if (m_inputJoint)
    {
        jointAxesSize.value = m_inputJoint->size1();
        jointColour.value = QColor(QString::fromStdString(m_inputJoint->colour1().GetHexArgb()));
    }
    m_properties.clear();
    m_properties = { { jointAxesSize.key, jointAxesSize },
                     { jointColour.key, jointColour } };

    dialogProperties.setInputSettingsItems(m_properties);
    dialogProperties.initialise();

    int status = dialogProperties.exec();
    if (status == QDialog::Accepted)
    {
        dialogProperties.update();
        m_properties = dialogProperties.getOutputSettingsItems();
    }
}

Simulation *DialogJoints::simulation() const
{
    return m_simulation;
}

void DialogJoints::setSimulation(Simulation *simulation)
{
    m_simulation = simulation;
}

std::unique_ptr<Joint> DialogJoints::outputJoint()
{
    return std::move(m_outputJoint);
}

void DialogJoints::setInputJoint(Joint *inputJoint)
{
    m_inputJoint = inputJoint;
}

