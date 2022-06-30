/*
 *  DialogAssembly.cpp
 *  GaitSymODE2019
 *
 *  Created by Bill Sellers on 19/10/2019.
 *  Copyright 2019 Bill Sellers. All rights reserved.
 *
 */

#include "DialogAssembly.h"
#include "ui_DialogAssembly.h"

#include "Preferences.h"
#include "Simulation.h"
#include "HingeJoint.h"
#include "BallJoint.h"
#include "UniversalJoint.h"
#include "AMotorJoint.h"
#include "LMotorJoint.h"
#include "LineEditDouble.h"
#include "Body.h"
#include "PGDMath.h"
#include "Marker.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QScrollArea>

using namespace std::string_literals;

DialogAssembly::DialogAssembly(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAssembly)
{
    ui->setupUi(this);
    setWindowTitle(tr("Assembly Builder"));
#ifdef Q_OS_MACOS
    setWindowFlags(windowFlags() & (~Qt::Dialog) | Qt::Window); // allows the window to be resized on macs
#endif
    restoreGeometry(Preferences::valueQByteArray("DialogAssemblyGeometry"));

    connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->pushButtonReset, SIGNAL(clicked()), this, SLOT(reset()));
    connect(ui->comboBoxBodyList, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(comboBoxBodyListCurrentIndexChanged(const QString &)));

    // make all the labels the same width as the largest
    QList<QLabel *> list = this->findChildren<QLabel *>();
    int maxWidth = INT_MIN;
    for (auto &&iter : list) maxWidth = std::max(maxWidth, iter->width());
    for (auto &&iter : list) iter->resize(maxWidth, iter->height());
}

DialogAssembly::~DialogAssembly()
{
    delete ui;
}

void DialogAssembly::closeEvent(QCloseEvent *event)
{
    Preferences::insert("DialogAssemblyGeometry", saveGeometry());
    QDialog::closeEvent(event);
}

void DialogAssembly::accept() // this catches OK and return/enter
{
    for (auto &&iter = m_simulation->GetJointList()->begin(); iter != m_simulation->GetJointList()->end(); /* no increment */)
    {
        if (iter->second->group() == "assembly"s)
        {
            emit jointDeleted(QString::fromStdString(iter->second->name()));
            iter = m_simulation->GetJointList()->erase(iter);
        }
        else { iter++; }
    }
    for (auto &&iter = m_simulation->GetMarkerList()->begin(); iter != m_simulation->GetMarkerList()->end(); /* no increment */)
    {
        if (iter->second->group() == "assembly"s)
        {
            emit jointDeleted(QString::fromStdString(iter->second->name()));
            iter = m_simulation->GetMarkerList()->erase(iter);
        }
        else { iter++; }
    }

    QString bodyName = ui->comboBoxBodyList->currentText();
    if (!bodyName.isEmpty())
    {
        Body *body = m_simulation->GetBody(bodyName.toStdString());
        // markers needed
        std::unique_ptr<Marker> worldMarker = std::make_unique<Marker>(nullptr);
        Marker *worldMarkerPtr = worldMarker.get();
        worldMarker->setName("World"s + body->name()+ "Joint_Body1"s + m_assemblyMarkerSuffix);
        worldMarker->setGroup("assembly"s);
        worldMarker->setSimulation(m_simulation);
        (*m_simulation->GetMarkerList())[worldMarkerPtr->name()] = std::move(worldMarker);
        emit markerCreated(QString::fromStdString(worldMarkerPtr->name()));
        std::unique_ptr<Marker> bodyMarker = std::make_unique<Marker>(body);
        Marker *bodyMarkerPtr = bodyMarker.get();
        bodyMarker->setName("World"s + body->name()+ "Joint_Body2"s + m_assemblyMarkerSuffix);
        bodyMarker->setGroup("assembly"s);
        bodyMarker->setSimulation(m_simulation);
        bodyMarker->saveToAttributes();
        bodyMarker->createFromAttributes();
        (*m_simulation->GetMarkerList())[bodyMarkerPtr->name()] = std::move(bodyMarker);
        emit markerCreated(QString::fromStdString(bodyMarkerPtr->name()));

        // now the linear motor joint
        std::unique_ptr<LMotorJoint> lMotorJoint = std::make_unique<LMotorJoint>(m_simulation->GetWorldID());
        LMotorJoint *lMotorJointPtr = lMotorJoint.get();
        lMotorJoint->setSimulation(m_simulation);
        lMotorJoint->setName("World"s + body->name()+ "Joint"s + m_assemblyLinearMotorSuffix);
        lMotorJoint->setGroup("assembly"s);
        lMotorJoint->setBody1Marker(worldMarkerPtr);
        lMotorJoint->setBody2Marker(bodyMarkerPtr);
        lMotorJoint->Attach();
        lMotorJoint->SetNumAxes(3);
        lMotorJoint->SetTargetPosition(0, ui->lineEditX->value());
        lMotorJoint->SetTargetPosition(1, ui->lineEditY->value());
        lMotorJoint->SetTargetPosition(2, ui->lineEditZ->value());
        lMotorJoint->SetMaxForce(0, ui->lineEditMaxForce->value());
        lMotorJoint->SetMaxForce(1, ui->lineEditMaxForce->value());
        lMotorJoint->SetMaxForce(2, ui->lineEditMaxForce->value());
        lMotorJoint->SetTargetPositionGain(0, ui->lineEditPositionGain->value());
        lMotorJoint->SetTargetPositionGain(1, ui->lineEditPositionGain->value());
        lMotorJoint->SetTargetPositionGain(2, ui->lineEditPositionGain->value());
        lMotorJoint->saveToAttributes();
        lMotorJoint->createFromAttributes();
        (*m_simulation->GetJointList())[lMotorJoint->name()] = std::move(lMotorJoint);
        emit jointCreated(QString::fromStdString(lMotorJointPtr->name()));

        // now the angular motor joint
        std::unique_ptr<AMotorJoint> aMotorJoint = std::make_unique<AMotorJoint>(m_simulation->GetWorldID());
        AMotorJoint *aMotorJointPtr = aMotorJoint.get();
        aMotorJoint->setSimulation(m_simulation);
        aMotorJoint->setName("World"s + body->name() + "Joint"s + m_assemblyAngularMotorSuffix);
        aMotorJoint->setGroup("assembly"s);
        aMotorJoint->setBody1Marker(worldMarkerPtr);
        aMotorJoint->setBody2Marker(bodyMarkerPtr);
        aMotorJoint->Attach();
        aMotorJoint->SetTargetAngles(pgd::DegreesToRadians(ui->lineEditXR->value()),
                                     pgd::DegreesToRadians(ui->lineEditYR->value()),
                                     pgd::DegreesToRadians(ui->lineEditZR->value()));
        aMotorJoint->SetMaxTorque(ui->lineEditMaxTorque->value());
        aMotorJoint->SetTargetAngleGain(ui->lineEditAngleGain->value());
        aMotorJoint->setReverseBodyOrderInCalculations(true);
        aMotorJoint->saveToAttributes();
        aMotorJoint->createFromAttributes();
        (*m_simulation->GetJointList())[aMotorJoint->name()] = std::move(aMotorJoint);
        emit jointCreated(QString::fromStdString(aMotorJointPtr->name()));
    }

    std::map<std::string, std::unique_ptr<Joint>> *jointsMap = m_simulation->GetJointList();
    for (size_t i = 0; i < m_jointList.size(); i++)
    {
        HingeJoint *hingeJoint = dynamic_cast<HingeJoint *>(m_jointList[i]);
        if (hingeJoint)
        {
            LineEditDouble *lineEdit = dynamic_cast<LineEditDouble *>(m_gridLayout->itemAtPosition(int(i), 2)->widget());
            Q_ASSERT_X(lineEdit, "DialogAssembly::accept", "lineEdit not set");
            std::unique_ptr<AMotorJoint> joint = std::make_unique<AMotorJoint>(m_simulation->GetWorldID());
            AMotorJoint *jointPtr = joint.get();
            joint->setSimulation(m_simulation);
            joint->setName(hingeJoint->name() + m_assemblyAngularMotorSuffix);
            joint->setGroup("assembly"s);
            joint->setBody1Marker(hingeJoint->body1Marker());
            joint->setBody2Marker(hingeJoint->body2Marker());
            joint->Attach();
            joint->SetTargetAngles(pgd::DegreesToRadians(lineEdit->value()));
            joint->SetMaxTorque(ui->lineEditMaxTorque->value());
            joint->SetTargetAngleGain(ui->lineEditAngleGain->value());
            joint->saveToAttributes();
            joint->createFromAttributes();
            (*jointsMap)[joint->name()] = std::move(joint);
            emit jointCreated(QString::fromStdString(jointPtr->name()));
        }

        UniversalJoint *universalJoint = dynamic_cast<UniversalJoint *>(m_jointList[i]);
        if (universalJoint)
        {
            LineEditDouble *lineEdit1 = dynamic_cast<LineEditDouble *>(m_gridLayout->itemAtPosition(int(i), 2)->widget());
            LineEditDouble *lineEdit2 = dynamic_cast<LineEditDouble *>(m_gridLayout->itemAtPosition(int(i), 3)->widget());
            Q_ASSERT_X(lineEdit1, "DialogAssembly::accept", "lineEdit1 not set");
            Q_ASSERT_X(lineEdit2, "DialogAssembly::accept", "lineEdit2 not set");
            std::unique_ptr<AMotorJoint> joint = std::make_unique<AMotorJoint>(m_simulation->GetWorldID());
            AMotorJoint *jointPtr = joint.get();
            joint->setSimulation(m_simulation);
            joint->setName(universalJoint->name() + m_assemblyAngularMotorSuffix);
            joint->setGroup("assembly"s);
            joint->setBody1Marker(universalJoint->body1Marker());
            joint->setBody2Marker(universalJoint->body2Marker());
            joint->Attach();
            joint->SetTargetAngles(pgd::DegreesToRadians(lineEdit1->value()),
                                   pgd::DegreesToRadians(lineEdit2->value()));
            joint->SetMaxTorque(ui->lineEditMaxTorque->value());
            joint->SetTargetAngleGain(ui->lineEditAngleGain->value());
            joint->saveToAttributes();
            joint->createFromAttributes();
            (*jointsMap)[joint->name()] = std::move(joint);
            emit jointCreated(QString::fromStdString(jointPtr->name()));
        }

        BallJoint *ballJoint = dynamic_cast<BallJoint *>(m_jointList[i]);
        if (ballJoint)
        {
            LineEditDouble *lineEdit1 = dynamic_cast<LineEditDouble *>(m_gridLayout->itemAtPosition(int(i), 2)->widget());
            LineEditDouble *lineEdit2 = dynamic_cast<LineEditDouble *>(m_gridLayout->itemAtPosition(int(i), 3)->widget());
            LineEditDouble *lineEdit3 = dynamic_cast<LineEditDouble *>(m_gridLayout->itemAtPosition(int(i), 4)->widget());
            Q_ASSERT_X(lineEdit1, "DialogAssembly::accept", "lineEdit1 not set");
            Q_ASSERT_X(lineEdit2, "DialogAssembly::accept", "lineEdit2 not set");
            Q_ASSERT_X(lineEdit3, "DialogAssembly::accept", "lineEdit3 not set");
            std::unique_ptr<AMotorJoint> joint = std::make_unique<AMotorJoint>(m_simulation->GetWorldID());
            AMotorJoint *jointPtr = joint.get();
            joint->setSimulation(m_simulation);
            joint->setName(ballJoint->name() + m_assemblyAngularMotorSuffix);
            joint->setGroup("assembly"s);
            joint->setBody1Marker(ballJoint->body1Marker());
            joint->setBody2Marker(ballJoint->body2Marker());
            joint->Attach();
            joint->SetTargetAngles(pgd::DegreesToRadians(lineEdit1->value()),
                                   pgd::DegreesToRadians(lineEdit2->value()),
                                   pgd::DegreesToRadians(lineEdit3->value()));
            joint->SetMaxTorque(ui->lineEditMaxTorque->value());
            joint->SetTargetAngleGain(ui->lineEditAngleGain->value());
            joint->saveToAttributes();
            joint->createFromAttributes();
            (*jointsMap)[ joint->name()] = std::move(joint);
            emit jointCreated(QString::fromStdString(jointPtr->name()));
        }
    }

    Preferences::insert("DialogAssemblyMaxForce", ui->lineEditMaxForce->value());
    Preferences::insert("DialogAssemblyMaxTorque", ui->lineEditMaxTorque->value());
    Preferences::insert("DialogAssemblyPositionGain", ui->lineEditPositionGain->value());
    Preferences::insert("DialogAssemblyAngleGain", ui->lineEditAngleGain->value());

    Preferences::insert("DialogAssemblyGeometry", saveGeometry());
    QDialog::accept();
}

void DialogAssembly::reject() // this catches cancel, close and escape key
{
    Preferences::insert("DialogAssemblyGeometry", saveGeometry());
    QDialog::reject();
}

void DialogAssembly::comboBoxBodyListCurrentIndexChanged(const QString &text)
{
    if (!m_simulation) return;
    Body *body = m_simulation->GetBody(text.toStdString());
    if (body)
    {
        pgd::Vector3 position(body->GetPosition());
        pgd::Quaternion quaternion(body->GetQuaternion());
        pgd::Vector3 euler = MakeEulerAnglesFromQ(quaternion);
        ui->lineEditX->setValue(position.x);
        ui->lineEditY->setValue(position.y);
        ui->lineEditZ->setValue(position.z);
        ui->lineEditXR->setValue(euler.x);
        ui->lineEditYR->setValue(euler.y);
        ui->lineEditZR->setValue(euler.z);
    }
}

void DialogAssembly::initialise()
{
    Q_ASSERT_X(m_simulation, "DialogAssembly::initialiseEdit", "m_simulation not set");

    ui->lineEditMaxForce->setValue(Preferences::valueDouble("DialogAssemblyMaxForce", 10000));
    ui->lineEditMaxTorque->setValue(Preferences::valueDouble("DialogAssemblyMaxTorque", 10000));
    ui->lineEditPositionGain->setValue(Preferences::valueDouble("DialogAssemblyPositionGain", 100));
    ui->lineEditAngleGain->setValue(Preferences::valueDouble("DialogAssemblyAngleGain", 100));

    std::map<std::string, Joint *> assemblyJoints;
    for (auto &&iter : *m_simulation->GetJointList())
        if (iter.second->group() == "assembly"s) assemblyJoints[iter.second->name()] = iter.second.get();

    // fill out the body area
    for (auto &&iter : *m_simulation->GetBodyList())
    {
        ui->comboBoxBodyList->addItem(QString::fromStdString(iter.first));
    }
    ui->comboBoxBodyList->setCurrentIndex(-1); // -1 means that nothing is selected
    ui->lineEditX->setValue(0);
    ui->lineEditY->setValue(0);
    ui->lineEditZ->setValue(0);
    ui->lineEditXR->setValue(0);
    ui->lineEditYR->setValue(0);
    ui->lineEditZR->setValue(0);

    std::string bodyName;
    for (auto &&iter : assemblyJoints)
    {
        LMotorJoint *lMotorJoint = dynamic_cast<LMotorJoint *>(iter.second);
        if (lMotorJoint)
        {
            bodyName = lMotorJoint->GetBody2()->name();
            ui->comboBoxBodyList->setCurrentText(QString::fromStdString(bodyName));
            ui->lineEditX->setValue(lMotorJoint->GetTargetPosition(0));
            ui->lineEditY->setValue(lMotorJoint->GetTargetPosition(1));
            ui->lineEditZ->setValue(lMotorJoint->GetTargetPosition(2));
            break;
        }
    }
    for (auto &&iter : assemblyJoints)
    {
        AMotorJoint *aMotorJoint = dynamic_cast<AMotorJoint *>(iter.second);
        if (aMotorJoint && aMotorJoint->name() == "World"s + bodyName + "Joint"s + m_assemblyAngularMotorSuffix)
        {
            if (aMotorJoint->targetAnglesList().size() == 3)
            {
                ui->lineEditXR->setValue(pgd::RadiansToDegrees(aMotorJoint->targetAnglesList()[0]));
                ui->lineEditYR->setValue(pgd::RadiansToDegrees(aMotorJoint->targetAnglesList()[1]));
                ui->lineEditZR->setValue(pgd::RadiansToDegrees(aMotorJoint->targetAnglesList()[2]));
            }
            break;
        }
    }

    // fill out the joint area
    QVBoxLayout *verticalLayout;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;

    verticalLayout = new QVBoxLayout(ui->groupBoxJoints);
    verticalLayout->setSpacing(6);
    verticalLayout->setContentsMargins(9, 9, 9, 9);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    scrollArea = new QScrollArea(this);
    scrollArea->setObjectName(QStringLiteral("scrollArea"));
    scrollArea->setWidgetResizable(true);
    scrollAreaWidgetContents = new QWidget();
    scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
    // scrollAreaWidgetContents->setGeometry(QRect(0, 0, 245, 154));
    m_gridLayout = new QGridLayout(scrollAreaWidgetContents);
    m_gridLayout->setSpacing(6);
    m_gridLayout->setContentsMargins(9, 9, 9, 9);
    m_gridLayout->setObjectName(QStringLiteral("gridLayout"));

    int row = 0;
    for (auto &&iter : *m_simulation->GetJointList())
    {
        HingeJoint *hingeJoint = dynamic_cast<HingeJoint *>(iter.second.get());
        if (hingeJoint)
        {
            QLabel *label = new QLabel();
            label->setText(QString::fromStdString(iter.first));
            m_gridLayout->addWidget(label, row, 0);
            label = new QLabel();
            label->setText(QString::fromUtf8(u8"Hinge angle (\u00B0)"));
            m_gridLayout->addWidget(label, row, 1);
            LineEditDouble *lineEdit = new LineEditDouble();
            auto jointIter = assemblyJoints.find(iter.first + m_assemblyAngularMotorSuffix);
            if (jointIter != assemblyJoints.end())
            {
                if (dynamic_cast<AMotorJoint *>(jointIter->second))
                    lineEdit->setValue(pgd::RadiansToDegrees(dynamic_cast<AMotorJoint *>(jointIter->second)->targetAnglesList()[0]));
            }
            else { lineEdit->setValue(pgd::RadiansToDegrees(hingeJoint->GetHingeAngle())); }
            m_gridLayout->addWidget(lineEdit, row, 2);
            m_jointList.push_back(iter.second.get());
            row++;
        }

        UniversalJoint *universalJoint = dynamic_cast<UniversalJoint *>(iter.second.get());
        if (universalJoint)
        {
            QLabel *label = new QLabel();
            label->setText(QString::fromStdString(iter.first));
            m_gridLayout->addWidget(label, row, 0);
            label = new QLabel();
            label->setText(QString::fromUtf8(u8"Universal angles (\u00B0)"));
            m_gridLayout->addWidget(label, row, 1);
            LineEditDouble *lineEdit = new LineEditDouble();
            auto jointIter = assemblyJoints.find(iter.first + m_assemblyAngularMotorSuffix);
            if (jointIter != assemblyJoints.end())
            {
                if (dynamic_cast<AMotorJoint *>(jointIter->second))
                    lineEdit->setValue(pgd::RadiansToDegrees(dynamic_cast<AMotorJoint *>(jointIter->second)->targetAnglesList()[0]));
            }
            else { lineEdit->setValue(pgd::RadiansToDegrees(universalJoint->GetUniversalAngle1())); }
            m_gridLayout->addWidget(lineEdit, row, 2);
            lineEdit = new LineEditDouble();
            if (jointIter != assemblyJoints.end())
            {
                if (dynamic_cast<AMotorJoint *>(jointIter->second))
                    lineEdit->setValue(pgd::RadiansToDegrees(dynamic_cast<AMotorJoint *>(jointIter->second)->targetAnglesList()[1]));
            }
            else { lineEdit->setValue(pgd::RadiansToDegrees(universalJoint->GetUniversalAngle2())); }
            m_gridLayout->addWidget(lineEdit, row, 3);
            m_jointList.push_back(iter.second.get());
            row++;
        }

        BallJoint *ballJoint = dynamic_cast<BallJoint *>(iter.second.get());
        if (ballJoint)
        {
            QLabel *label = new QLabel();
            label->setText(QString::fromStdString(iter.first));
            m_gridLayout->addWidget(label, row, 0);
            label = new QLabel();
            label->setText(QString::fromUtf8(u8"Ball angles (\u00B0)"));
            m_gridLayout->addWidget(label, row, 1);
            pgd::Quaternion jointAngle = ballJoint->GetQuaternion();
            pgd::Vector3 euler = pgd::MakeEulerAnglesFromQ(jointAngle);
            LineEditDouble *lineEdit = new LineEditDouble();
            auto jointIter = assemblyJoints.find(iter.first + m_assemblyAngularMotorSuffix);
            if (jointIter != assemblyJoints.end())
            {
                if (dynamic_cast<AMotorJoint *>(jointIter->second))
                    lineEdit->setValue(pgd::RadiansToDegrees(dynamic_cast<AMotorJoint *>(jointIter->second)->targetAnglesList()[0]));
            }
            else { lineEdit->setValue(euler.x); }
            m_gridLayout->addWidget(lineEdit, row, 2);
            lineEdit = new LineEditDouble();
            if (jointIter != assemblyJoints.end())
            {
                if (dynamic_cast<AMotorJoint *>(jointIter->second))
                    lineEdit->setValue(pgd::RadiansToDegrees(dynamic_cast<AMotorJoint *>(jointIter->second)->targetAnglesList()[1]));
            }
            else { lineEdit->setValue(euler.y); }
            m_gridLayout->addWidget(lineEdit, row, 3);
            lineEdit = new LineEditDouble();
            if (jointIter != assemblyJoints.end())
            {
                if (dynamic_cast<AMotorJoint *>(jointIter->second))
                    lineEdit->setValue(pgd::RadiansToDegrees(dynamic_cast<AMotorJoint *>(jointIter->second)->targetAnglesList()[2]));
            }
            else { lineEdit->setValue(euler.z); }
            m_gridLayout->addWidget(lineEdit, row, 4);
            m_jointList.push_back(iter.second.get());
            row++;
        }

    }

    QSpacerItem *vertSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    m_gridLayout->addItem(vertSpacer, row, 0);

    scrollArea->setWidget(scrollAreaWidgetContents);
    verticalLayout->addWidget(scrollArea);
}

void DialogAssembly::reset()
{
    ui->comboBoxBodyList->setCurrentIndex(-1); // -1 means that nothing is selected
    ui->lineEditX->setValue(0);
    ui->lineEditY->setValue(0);
    ui->lineEditZ->setValue(0);
    ui->lineEditXR->setValue(0);
    ui->lineEditYR->setValue(0);
    ui->lineEditZR->setValue(0);

    for (size_t i = 0; i < m_jointList.size(); i++)
    {
        HingeJoint *hingeJoint = dynamic_cast<HingeJoint *>(m_jointList[i]);
        if (hingeJoint)
        {
            LineEditDouble *lineEdit = dynamic_cast<LineEditDouble *>(m_gridLayout->itemAtPosition(int(i), 2)->widget());
            Q_ASSERT_X(lineEdit, "DialogAssembly::accept", "lineEdit not set");
            lineEdit->setValue(pgd::RadiansToDegrees(hingeJoint->GetHingeAngle()));
        }

        UniversalJoint *universalJoint = dynamic_cast<UniversalJoint *>(m_jointList[i]);
        if (universalJoint)
        {
            LineEditDouble *lineEdit1 = dynamic_cast<LineEditDouble *>(m_gridLayout->itemAtPosition(int(i), 2)->widget());
            LineEditDouble *lineEdit2 = dynamic_cast<LineEditDouble *>(m_gridLayout->itemAtPosition(int(i), 3)->widget());
            Q_ASSERT_X(lineEdit1, "DialogAssembly::accept", "lineEdit1 not set");
            Q_ASSERT_X(lineEdit2, "DialogAssembly::accept", "lineEdit2 not set");
            lineEdit1->setValue(pgd::RadiansToDegrees(universalJoint->GetUniversalAngle1()));
            lineEdit2->setValue(pgd::RadiansToDegrees(universalJoint->GetUniversalAngle2()));
        }

        BallJoint *ballJoint = dynamic_cast<BallJoint *>(m_jointList[i]);
        if (ballJoint)
        {
            LineEditDouble *lineEdit1 = dynamic_cast<LineEditDouble *>(m_gridLayout->itemAtPosition(int(i), 2)->widget());
            LineEditDouble *lineEdit2 = dynamic_cast<LineEditDouble *>(m_gridLayout->itemAtPosition(int(i), 3)->widget());
            LineEditDouble *lineEdit3 = dynamic_cast<LineEditDouble *>(m_gridLayout->itemAtPosition(int(i), 4)->widget());
            Q_ASSERT_X(lineEdit1, "DialogAssembly::accept", "lineEdit1 not set");
            Q_ASSERT_X(lineEdit2, "DialogAssembly::accept", "lineEdit2 not set");
            Q_ASSERT_X(lineEdit3, "DialogAssembly::accept", "lineEdit3 not set");
            pgd::Quaternion quaternion = ballJoint->GetQuaternion();
            pgd::Vector3 euler = pgd::MakeEulerAnglesFromQ(quaternion);
            lineEdit1->setValue(euler.x);
            lineEdit2->setValue(euler.y);
            lineEdit3->setValue(euler.z);
        }
    }
}

Simulation *DialogAssembly::simulation() const
{
    return m_simulation;
}

void DialogAssembly::setSimulation(Simulation *simulation)
{
    m_simulation = simulation;
}

