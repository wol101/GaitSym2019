#include "DialogMarkers.h"
#include "ui_DialogMarkers.h"

#include "Preferences.h"
#include "Marker.h"
#include "Simulation.h"
#include "LineEditDouble.h"
#include "LineEditUniqueName.h"
#include "Body.h"
#include "DialogProperties.h"

#include <QDebug>
#include <QSignalBlocker>
#include <QGridLayout>
#include <QMenu>
#include <QAction>

#include <map>

DialogMarkers::DialogMarkers(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogMarkers)
{
    ui->setupUi(this);

    setWindowTitle(tr("Marker Builder"));
#ifdef Q_OS_MACOS
    setWindowFlags(windowFlags() & (~Qt::Dialog) | Qt::Window); // allows the window to be resized on macs
#endif
    restoreGeometry(Preferences::valueQByteArray("DialogMarkersGeometry"));

    QString mirrorAxis = Preferences::valueQString("DialogMarkersMirrorAxis");
    if (mirrorAxis != "X" && mirrorAxis != "Y" && mirrorAxis != "Z") mirrorAxis = "X";
    ui->radioButtonX->setChecked(mirrorAxis == "X");
    ui->radioButtonY->setChecked(mirrorAxis == "Y");
    ui->radioButtonZ->setChecked(mirrorAxis == "Z");

    connect(ui->pushButtonOK, &QPushButton::clicked, this, &DialogMarkers::accept);
    connect(ui->pushButtonCancel, &QPushButton::clicked, this, &DialogMarkers::reject);
    connect(ui->pushButtonProperties, &QPushButton::clicked, this, &DialogMarkers::properties);
    connect(ui->pushButtonCalculatePosition, &QPushButton::clicked, this, &DialogMarkers::calculatePosition);
    connect(ui->pushButtonCopyMarker1, &QPushButton::clicked, this, &DialogMarkers::calculatePositionCopyMarker1);
    connect(ui->pushButtonCopyMarker2, &QPushButton::clicked, this, &DialogMarkers::calculatePositionCopyMarker2);
    connect(ui->pushButtonCalculateOrientation2Marker, &QPushButton::clicked, this, &DialogMarkers::calculateOrientation2Marker);
    connect(ui->pushButtonCalculateOrientation3Marker, &QPushButton::clicked, this, &DialogMarkers::calculateOrientation3Marker);
    connect(ui->pushButtonCalculateMirrorMarker, &QPushButton::clicked, this, &DialogMarkers::calculateMirrorMarker);
    connect(ui->pushButtonCalculatorCalculate, &QPushButton::clicked, this, &DialogMarkers::calculateCalculator);
    connect(ui->pushButtonCalculatorImport, &QPushButton::clicked, this, &DialogMarkers::importCalculator);
    connect(ui->pushButtonMatrixCalculate, &QPushButton::clicked, this, &DialogMarkers::calculateMatrix);
    connect(ui->pushButtonMatrixImport, &QPushButton::clicked, this, &DialogMarkers::importMatrix);
    connect(ui->pushButton3DCursor, &QPushButton::clicked, this, &DialogMarkers::copy3DCursorPosition);
    connect(ui->lineEditMarkerID, &LineEditDouble::textChanged, this, &DialogMarkers::lineEditIDTextChanged);
    connect(ui->lineEditFraction, &LineEditDouble::textChanged, this, &DialogMarkers::lineEditFractionTextChanged);
    connect(ui->lineEditDistance, &LineEditDouble::textChanged, this, &DialogMarkers::lineEditDistanceTextChanged);
    connect(ui->lineEditEulerXAppend, &LineEditDouble::textChanged, this, &DialogMarkers::lineEditEulerAppendTextChanged);
    connect(ui->lineEditEulerYAppend, &LineEditDouble::textChanged, this, &DialogMarkers::lineEditEulerAppendTextChanged);
    connect(ui->lineEditEulerZAppend, &LineEditDouble::textChanged, this, &DialogMarkers::lineEditEulerAppendTextChanged);
    connect(ui->lineEditAngle, &LineEditDouble::textChanged, this, &DialogMarkers::lineEditAxisAngleTextChanged);
    connect(ui->lineEditAxisX, &LineEditDouble::textChanged, this, &DialogMarkers::lineEditAxisAngleTextChanged);
    connect(ui->lineEditAxisY, &LineEditDouble::textChanged, this, &DialogMarkers::lineEditAxisAngleTextChanged);
    connect(ui->lineEditAxisZ, &LineEditDouble::textChanged, this, &DialogMarkers::lineEditAxisAngleTextChanged);
    connect(ui->lineEditQuaternionN, &LineEditDouble::textChanged, this, &DialogMarkers::lineEditQuaternionTextChanged);
    connect(ui->lineEditQuaternionX, &LineEditDouble::textChanged, this, &DialogMarkers::lineEditQuaternionTextChanged);
    connect(ui->lineEditQuaternionY, &LineEditDouble::textChanged, this, &DialogMarkers::lineEditQuaternionTextChanged);
    connect(ui->lineEditQuaternionZ, &LineEditDouble::textChanged, this, &DialogMarkers::lineEditQuaternionTextChanged);
    connect(ui->comboBoxPositionMarker1, &QComboBox::currentTextChanged, this, &DialogMarkers::positionMarkerChanged);
    connect(ui->comboBoxPositionMarker2, &QComboBox::currentTextChanged, this, &DialogMarkers::positionMarkerChanged);
    connect(ui->comboBoxOrientation2Marker1, &QComboBox::currentTextChanged, this, &DialogMarkers::orientation2MarkerChanged);
    connect(ui->comboBoxOrientation2Marker2, &QComboBox::currentTextChanged, this, &DialogMarkers::orientation2MarkerChanged);
    connect(ui->comboBoxOrientation3Marker1, &QComboBox::currentTextChanged, this, &DialogMarkers::orientation3MarkerChanged);
    connect(ui->comboBoxOrientation3Marker2, &QComboBox::currentTextChanged, this, &DialogMarkers::orientation3MarkerChanged);
    connect(ui->comboBoxOrientation3Marker3, &QComboBox::currentTextChanged, this, &DialogMarkers::orientation3MarkerChanged);

    ui->labelAxisAngle->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->labelAxisAngle, &QLabel::customContextMenuRequested, this, &DialogMarkers::labelAxisAngleMenuRequest);
}

DialogMarkers::~DialogMarkers()
{
    delete ui;
}

void DialogMarkers::accept() // this catches OK and return/enter
{
    qDebug() << "DialogMarkers::accept()";

    Marker *markerPtr;
    if (m_inputMarker) markerPtr = m_inputMarker;
    else { m_outputMarker = std::make_unique<Marker>(nullptr); markerPtr = m_outputMarker.get(); }

    markerPtr->setName(ui->lineEditMarkerID->text().toStdString());
    markerPtr->setSimulation(m_simulation);
    if (ui->comboBoxBodyID->currentText() != "World")
    {
        markerPtr->SetBody(m_simulation->GetBodyList()->at(ui->comboBoxBodyID->currentText().toStdString()).get());

        dVector3 pos, result;
        pos[0] = ui->lineEditPositionX->value();
        pos[1] = ui->lineEditPositionY->value();
        pos[2] = ui->lineEditPositionZ->value();
        dBodyGetPosRelPoint(markerPtr->GetBody()->GetBodyID(), pos[0], pos[1], pos[2], result); // convert from world to body
        markerPtr->SetPosition(result[0], result[1], result[2]);

        const double *q = dBodyGetQuaternion(markerPtr->GetBody()->GetBodyID());
        pgd::Quaternion qBody(q[0], q[1], q[2], q[3]);
        double ex = ui->lineEditEulerX->value();
        double ey = ui->lineEditEulerY->value();
        double ez = ui->lineEditEulerZ->value();
        pgd::Quaternion qWorld = pgd::MakeQFromEulerAngles(ex, ey, ez);
        pgd::Quaternion qLocal = ~qBody * qWorld;
        markerPtr->SetQuaternion(qLocal.n, qLocal.x, qLocal.y, qLocal.z);
    }
    else     // world marker
    {
        dVector3 pos;
        pos[0] = ui->lineEditPositionX->value();
        pos[1] = ui->lineEditPositionY->value();
        pos[2] = ui->lineEditPositionZ->value();
        markerPtr->SetPosition(pos[0], pos[1], pos[2]);

        double ex = ui->lineEditEulerX->value();
        double ey = ui->lineEditEulerY->value();
        double ez = ui->lineEditEulerZ->value();
        pgd::Quaternion qWorld = pgd::MakeQFromEulerAngles(ex, ey, ez);
        markerPtr->SetQuaternion(qWorld.n, qWorld.x, qWorld.y, qWorld.z);
    }

    if (m_inputMarker)
    {
        markerPtr->setSize1(m_inputMarker->size1());
        markerPtr->setColour1(m_inputMarker->colour1());
    }
    else
    {
        markerPtr->setSize1(Preferences::valueDouble("MarkerSize"));
        markerPtr->setColour1(Preferences::valueQColor("MarkerColour").name(QColor::HexArgb).toStdString());
    }

    if (m_properties.size() > 0)
    {
        if (m_properties.count("MarkerSize"))
            markerPtr->setSize1(m_properties["MarkerSize"].value.toDouble());
        if (m_properties.count("MarkerColour"))
            markerPtr->setColour1(qvariant_cast<QColor>(m_properties["MarkerColour"].value).name(QColor::HexArgb).toStdString());
    }
    markerPtr->saveToAttributes();
    markerPtr->createFromAttributes();

    QString mirrorAxis = "X";
    if (ui->radioButtonX->isChecked()) mirrorAxis = "X";
    else if (ui->radioButtonY->isChecked()) mirrorAxis = "Y";
    else if (ui->radioButtonZ->isChecked()) mirrorAxis = "Z";
    Preferences::insert("DialogMarkersMirrorAxis", mirrorAxis);
    Preferences::insert("DialogMarkersGeometry", saveGeometry());
    QDialog::accept();
}

void DialogMarkers::reject() // this catches cancel, close and escape key
{
    qDebug() << "DialogMarkers::reject()";
    QString mirrorAxis = "X";
    if (ui->radioButtonX->isChecked()) mirrorAxis = "X";
    else if (ui->radioButtonY->isChecked()) mirrorAxis = "Y";
    else if (ui->radioButtonZ->isChecked()) mirrorAxis = "Z";
    Preferences::insert("DialogMarkersMirrorAxis", mirrorAxis);
    Preferences::insert("DialogMarkersGeometry", saveGeometry());
    QDialog::reject();
}

void DialogMarkers::closeEvent(QCloseEvent *event)
{
    QString mirrorAxis;
    if (ui->radioButtonX->isChecked()) mirrorAxis = "X";
    else if (ui->radioButtonY->isChecked()) mirrorAxis = "Y";
    else if (ui->radioButtonZ->isChecked()) mirrorAxis = "Z";
    Preferences::insert("DialogMarkersMirrorAxis", mirrorAxis);
    Preferences::insert("DialogMarkersGeometry", saveGeometry());

    QDialog::closeEvent(event);
}

void DialogMarkers::lateInitialise()
{
    Q_ASSERT_X(m_simulation, "DialogMarkers::lateInitialise", "simulation undefined");
    auto bodyList = m_simulation->GetBodyList();
    QStringList bodyIDs;
    bodyIDs.append("World");
    for (auto it = bodyList->begin(); it != bodyList->end(); it++) bodyIDs.append(QString::fromStdString(it->first));
    ui->comboBoxBodyID->addItems(bodyIDs);
    ui->comboBoxBodyID->setCurrentText("World");
    auto markerList = m_simulation->GetMarkerList();
    QStringList markerIDs;
    for (auto it = markerList->begin(); it != markerList->end(); it++) markerIDs.append(QString::fromStdString(it->first));
    ui->comboBoxPositionMarker1->addItems(markerIDs);
    ui->comboBoxPositionMarker2->addItems(markerIDs);
    ui->comboBoxOrientation2Marker1->addItems(markerIDs);
    ui->comboBoxOrientation2Marker2->addItems(markerIDs);
    ui->comboBoxOrientation3Marker1->addItems(markerIDs);
    ui->comboBoxOrientation3Marker2->addItems(markerIDs);
    ui->comboBoxOrientation3Marker3->addItems(markerIDs);
    ui->comboBoxMirrorMarker->addItems(markerIDs);
    pgd::Vector3 eulerAngles;
    pgd::Vector3 position(double(m_cursor3DPosition.x()), double(m_cursor3DPosition.y()), double(m_cursor3DPosition.z()));
    if (!m_inputMarker)
    {
        auto nameSet = simulation()->GetNameSet();
        ui->lineEditMarkerID->addStrings(nameSet);
        int initialNameCount = 0;
        QString initialName = QString("Marker%1").arg(initialNameCount, 3, 10, QLatin1Char('0'));
        while (nameSet.count(initialName.toStdString()))
        {
            initialNameCount++;
            initialName = QString("Marker%1").arg(initialNameCount, 3, 10, QLatin1Char('0'));
            if (initialNameCount >= 9999) break; // only do this for the first 9999 markers
        }
        ui->lineEditMarkerID->setText(initialName);
    }
    else
    {
        if (m_inputMarker->GetBody()) ui->comboBoxBodyID->setCurrentText(QString::fromStdString(m_inputMarker->GetBody()->name()));
        else ui->comboBoxBodyID->setCurrentText("World");
        ui->lineEditMarkerID->setText(QString::fromStdString(m_inputMarker->name()));
        ui->lineEditMarkerID->setEnabled(false);
        const pgd::Quaternion q = m_inputMarker->GetWorldQuaternion();
        eulerAngles = pgd::MakeEulerAnglesFromQ(q);
        position = m_inputMarker->GetWorldPosition();
    }
    ui->lineEditPositionX->setValue(position.x);
    ui->lineEditPositionY->setValue(position.y);
    ui->lineEditPositionZ->setValue(position.z);
    ui->lineEditEulerX->setValue(eulerAngles.x);
    ui->lineEditEulerY->setValue(eulerAngles.y);
    ui->lineEditEulerZ->setValue(eulerAngles.z);

    ui->lineEditFraction->setValue(0.5);
}

void DialogMarkers::calculatePosition()
{
    double fraction = ui->lineEditFraction->value();
    auto markerList = m_simulation->GetMarkerList();
    Marker *marker1 = markerList->at(ui->comboBoxPositionMarker1->currentText().toStdString()).get();
    Marker *marker2 = markerList->at(ui->comboBoxPositionMarker2->currentText().toStdString()).get();

    pgd::Vector3 p1 = marker1->GetWorldPosition();
    pgd::Vector3 p2 = marker2->GetWorldPosition();
    pgd::Vector3 p = p1 + (p2 - p1) * fraction;
    ui->lineEditPositionX->setValue(p.x);
    ui->lineEditPositionY->setValue(p.y);
    ui->lineEditPositionZ->setValue(p.z);

    pgd::Quaternion q1 = marker1->GetWorldQuaternion();
    pgd::Quaternion q2 = marker2->GetWorldQuaternion();
    pgd::Quaternion q = slerp(q1, q2, fraction, true);
    pgd::Vector3 e = pgd::MakeEulerAnglesFromQ(q);
    ui->lineEditEulerX->setValue(e.x);
    ui->lineEditEulerY->setValue(e.y);
    ui->lineEditEulerZ->setValue(e.z);

    ui->comboBoxOrientation2Marker1->setCurrentText(ui->comboBoxPositionMarker1->currentText());
    ui->comboBoxOrientation2Marker2->setCurrentText(ui->comboBoxPositionMarker2->currentText());
    ui->comboBoxOrientation3Marker1->setCurrentText(ui->comboBoxPositionMarker1->currentText());
    ui->comboBoxOrientation3Marker2->setCurrentText(ui->comboBoxPositionMarker2->currentText());
}

void DialogMarkers::calculatePositionCopyMarker1()
{
    auto markerList = m_simulation->GetMarkerList();
    Marker *marker = markerList->at(ui->comboBoxPositionMarker1->currentText().toStdString()).get();

    pgd::Vector3 p = marker->GetWorldPosition();
    ui->lineEditPositionX->setValue(p.x);
    ui->lineEditPositionY->setValue(p.y);
    ui->lineEditPositionZ->setValue(p.z);

    pgd::Quaternion q = marker->GetWorldQuaternion();
    pgd::Vector3 e = pgd::MakeEulerAnglesFromQ(q);
    ui->lineEditEulerX->setValue(e.x);
    ui->lineEditEulerY->setValue(e.y);
    ui->lineEditEulerZ->setValue(e.z);
}

void DialogMarkers::calculatePositionCopyMarker2()
{
    auto markerList = m_simulation->GetMarkerList();
    Marker *marker = markerList->at(ui->comboBoxPositionMarker2->currentText().toStdString()).get();

    pgd::Vector3 p = marker->GetWorldPosition();
    ui->lineEditPositionX->setValue(p.x);
    ui->lineEditPositionY->setValue(p.y);
    ui->lineEditPositionZ->setValue(p.z);

    pgd::Quaternion q = marker->GetWorldQuaternion();
    pgd::Vector3 e = pgd::MakeEulerAnglesFromQ(q);
    ui->lineEditEulerX->setValue(e.x);
    ui->lineEditEulerY->setValue(e.y);
    ui->lineEditEulerZ->setValue(e.z);
}

// this calculates the rotation that maps the X axis to the direction from marker 1 to marker 2
void DialogMarkers::calculateOrientation2Marker()
{
    auto markerList = m_simulation->GetMarkerList();
    Marker *marker1 = markerList->at(ui->comboBoxOrientation2Marker1->currentText().toStdString()).get();
    Marker *marker2 = markerList->at(ui->comboBoxOrientation2Marker2->currentText().toStdString()).get();

    pgd::Vector3 v1(1, 0, 0);
    pgd::Vector3 v2 = marker2->GetWorldPosition() - marker1->GetWorldPosition();
    pgd::Quaternion q = pgd::FindRotation(v1, v2);
    pgd::Vector3 e = pgd::MakeEulerAnglesFromQ(q);
    ui->lineEditEulerX->setValue(e.x);
    ui->lineEditEulerY->setValue(e.y);
    ui->lineEditEulerZ->setValue(e.z);

    ui->comboBoxPositionMarker1->setCurrentText(ui->comboBoxOrientation2Marker1->currentText());
    ui->comboBoxPositionMarker2->setCurrentText(ui->comboBoxOrientation2Marker2->currentText());
    ui->comboBoxOrientation3Marker1->setCurrentText(ui->comboBoxOrientation2Marker1->currentText());
    ui->comboBoxOrientation3Marker2->setCurrentText(ui->comboBoxOrientation2Marker2->currentText());
}

// this calculates the basis where marker1 to marker2 is the x axis
// and the z axis is the normal to the plane calculated from the markers in anticlockwise order
// the y axis is normal to the other
void DialogMarkers::calculateOrientation3Marker()
{
    auto markerList = m_simulation->GetMarkerList();
    Marker *marker1 = markerList->at(ui->comboBoxOrientation3Marker1->currentText().toStdString()).get();
    Marker *marker2 = markerList->at(ui->comboBoxOrientation3Marker2->currentText().toStdString()).get();
    Marker *marker3 = markerList->at(ui->comboBoxOrientation3Marker3->currentText().toStdString()).get();

    pgd::Vector3 xAxis = (marker2->GetWorldPosition() - marker1->GetWorldPosition());
    xAxis.Normalize();
    pgd::Vector3 zAxis = xAxis ^ (marker3->GetWorldPosition() - marker2->GetWorldPosition());
    zAxis.Normalize();
    pgd::Vector3 yAxis = zAxis ^ xAxis;
    yAxis.Normalize();

    pgd::Matrix3x3 R(xAxis.x, yAxis.x, zAxis.x,
                     xAxis.y, yAxis.y, zAxis.y,
                     xAxis.z, yAxis.z, zAxis.z);
    pgd::Quaternion q = pgd::MakeQfromM(R);
    pgd::Vector3 e = pgd::MakeEulerAnglesFromQ(q);
    ui->lineEditEulerX->setValue(e.x);
    ui->lineEditEulerY->setValue(e.y);
    ui->lineEditEulerZ->setValue(e.z);

    ui->comboBoxPositionMarker1->setCurrentText(ui->comboBoxOrientation3Marker1->currentText());
    ui->comboBoxPositionMarker2->setCurrentText(ui->comboBoxOrientation3Marker2->currentText());
    ui->comboBoxOrientation2Marker1->setCurrentText(ui->comboBoxOrientation3Marker1->currentText());
    ui->comboBoxOrientation2Marker2->setCurrentText(ui->comboBoxOrientation3Marker2->currentText());
}

void DialogMarkers::calculateMirrorMarker()
{
    auto markerList = m_simulation->GetMarkerList();
    Marker *marker = markerList->at(ui->comboBoxMirrorMarker->currentText().toStdString()).get();

    pgd::Matrix3x3 m;
    if (ui->radioButtonX->isChecked()) m = pgd::Matrix3x3(-1, 0, 0,
                                                          0, 1, 0,
                                                          0, 0, 1);
    else if (ui->radioButtonY->isChecked()) m = pgd::Matrix3x3(1, 0, 0,
                                                               0, -1, 0,
                                                               0, 0, 1);
    else if (ui->radioButtonZ->isChecked()) m = pgd::Matrix3x3(1, 0, 0,
                                                               0, 1, 0,
                                                               0, 0, -1);
    pgd::Vector3 p = m * marker->GetWorldPosition();
    ui->lineEditPositionX->setValue(p.x);
    ui->lineEditPositionY->setValue(p.y);
    ui->lineEditPositionZ->setValue(p.z);

    pgd::Quaternion q = marker->GetWorldQuaternion();
    pgd::Vector3 v = m * q.GetVector();
    q.x = v.x; q.y = v.y; q.z = v.z;
    pgd::Vector3 e = pgd::MakeEulerAnglesFromQ(q);
    ui->lineEditEulerX->setValue(e.x);
    ui->lineEditEulerY->setValue(e.y);
    ui->lineEditEulerZ->setValue(e.z);
}

void DialogMarkers::calculateCalculator()
{
    double ex = ui->lineEditEulerX->value();
    double ey = ui->lineEditEulerY->value();
    double ez = ui->lineEditEulerZ->value();
    pgd::Quaternion qMarker = pgd::MakeQFromEulerAngles(ex, ey, ez);
    pgd::Quaternion qRotation;
    qRotation.n = ui->lineEditQuaternionN->value();
    qRotation.x = ui->lineEditQuaternionX->value();
    qRotation.y = ui->lineEditQuaternionY->value();
    qRotation.z = ui->lineEditQuaternionZ->value();
    pgd::Quaternion qMarker2 = qRotation * qMarker;
    pgd::Vector3 eulerAngles = pgd::MakeEulerAnglesFromQ(qMarker2);
    ui->lineEditEulerX->setValue(eulerAngles.x);
    ui->lineEditEulerY->setValue(eulerAngles.y);
    ui->lineEditEulerZ->setValue(eulerAngles.z);
}

void DialogMarkers::importCalculator()
{
    double ex = ui->lineEditEulerX->value();
    double ey = ui->lineEditEulerY->value();
    double ez = ui->lineEditEulerZ->value();
    ui->lineEditEulerXAppend->setValue(ex);
    ui->lineEditEulerYAppend->setValue(ey);
    ui->lineEditEulerZAppend->setValue(ez);
}

void DialogMarkers::calculateMatrix()
{
    double ex = ui->lineEditEulerX->value();
    double ey = ui->lineEditEulerY->value();
    double ez = ui->lineEditEulerZ->value();
    pgd::Quaternion qMarker = pgd::MakeQFromEulerAngles(ex, ey, ez);
    pgd::Matrix3x3 mMarker = pgd::MakeMFromQ(qMarker);
    pgd::Matrix3x3 matrix;
    matrix.e11 = ui->lineEditMatrixr1c1->value();
    matrix.e12 = ui->lineEditMatrixr1c2->value();
    matrix.e13 = ui->lineEditMatrixr1c3->value();
    matrix.e21 = ui->lineEditMatrixr2c1->value();
    matrix.e22 = ui->lineEditMatrixr2c2->value();
    matrix.e23 = ui->lineEditMatrixr2c3->value();
    matrix.e31 = ui->lineEditMatrixr3c1->value();
    matrix.e32 = ui->lineEditMatrixr3c2->value();
    matrix.e33 = ui->lineEditMatrixr3c3->value();
    pgd::Matrix3x3 mMarker2 = matrix * mMarker;
    pgd::Quaternion qMarker2 = pgd::MakeQfromM(mMarker2);
    pgd::Vector3 eulerAngles = pgd::MakeEulerAnglesFromQ(qMarker2);
    ui->lineEditEulerX->setValue(eulerAngles.x);
    ui->lineEditEulerY->setValue(eulerAngles.y);
    ui->lineEditEulerZ->setValue(eulerAngles.z);
}

void DialogMarkers::importMatrix()
{
    double ex = ui->lineEditEulerX->value();
    double ey = ui->lineEditEulerY->value();
    double ez = ui->lineEditEulerZ->value();
    pgd::Quaternion qMarker = pgd::MakeQFromEulerAngles(ex, ey, ez);
    pgd::Matrix3x3 mMarker = pgd::MakeMFromQ(qMarker);
    ui->lineEditMatrixr1c1->setValue(mMarker.e11);
    ui->lineEditMatrixr1c2->setValue(mMarker.e12);
    ui->lineEditMatrixr1c3->setValue(mMarker.e13);
    ui->lineEditMatrixr2c1->setValue(mMarker.e21);
    ui->lineEditMatrixr2c2->setValue(mMarker.e22);
    ui->lineEditMatrixr2c3->setValue(mMarker.e23);
    ui->lineEditMatrixr3c1->setValue(mMarker.e31);
    ui->lineEditMatrixr3c2->setValue(mMarker.e32);
    ui->lineEditMatrixr3c3->setValue(mMarker.e33);
}

void DialogMarkers::copy3DCursorPosition()
{
    ui->lineEditPositionX->setValue(double(m_cursor3DPosition[0]));
    ui->lineEditPositionY->setValue(double(m_cursor3DPosition[1]));
    ui->lineEditPositionZ->setValue(double(m_cursor3DPosition[2]));
}

void DialogMarkers::setCursor3DPosition(const QVector3D &cursor3DPosition)
{
    m_cursor3DPosition = cursor3DPosition;
}

void DialogMarkers::lineEditIDTextChanged(const QString & /* text */)
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(sender());
    if (lineEdit == nullptr) return;
    QString textCopy = lineEdit->text();
    int pos = lineEdit->cursorPosition();
    ui->pushButtonOK->setEnabled(lineEdit->validator()->validate(textCopy, pos) == QValidator::Acceptable);
}

void DialogMarkers::lineEditFractionTextChanged(const QString & /* text */)
{
    double fraction = ui->lineEditFraction->value();
    auto markerList = m_simulation->GetMarkerList();
    Marker *marker1 = markerList->at(ui->comboBoxPositionMarker1->currentText().toStdString()).get();
    Marker *marker2 = markerList->at(ui->comboBoxPositionMarker2->currentText().toStdString()).get();

    pgd::Vector3 p1 = marker1->GetWorldPosition();
    pgd::Vector3 p2 = marker2->GetWorldPosition();
    pgd::Vector3 p = (p2 - p1) * fraction;
    QSignalBlocker blocker(ui->lineEditDistance);
    ui->lineEditDistance->setValue(p.Magnitude());
}

void DialogMarkers::lineEditDistanceTextChanged(const QString & /* text */)
{
    double distance = ui->lineEditDistance->value();
    auto markerList = m_simulation->GetMarkerList();
    Marker *marker1 = markerList->at(ui->comboBoxPositionMarker1->currentText().toStdString()).get();
    Marker *marker2 = markerList->at(ui->comboBoxPositionMarker2->currentText().toStdString()).get();

    pgd::Vector3 p1 = marker1->GetWorldPosition();
    pgd::Vector3 p2 = marker2->GetWorldPosition();
    pgd::Vector3 p = (p2 - p1) / distance;
    QSignalBlocker blocker(ui->lineEditFraction);
    ui->lineEditFraction->setValue(p.Magnitude());
}

void DialogMarkers::lineEditEulerAppendTextChanged(const QString & /*text*/)
{
    double ex = ui->lineEditEulerXAppend->value();
    double ey = ui->lineEditEulerYAppend->value();
    double ez = ui->lineEditEulerZAppend->value();
    pgd::Quaternion qRotation = pgd::MakeQFromEulerAngles(ex, ey, ez);
    QSignalBlocker blocker1(ui->lineEditQuaternionN);
    QSignalBlocker blocker2(ui->lineEditQuaternionX);
    QSignalBlocker blocker3(ui->lineEditQuaternionY);
    QSignalBlocker blocker4(ui->lineEditQuaternionZ);
    ui->lineEditQuaternionN->setValue(qRotation.n);
    ui->lineEditQuaternionX->setValue(qRotation.x);
    ui->lineEditQuaternionY->setValue(qRotation.y);
    ui->lineEditQuaternionZ->setValue(qRotation.z);
    double angle;
    pgd::Vector3 axis;
    pgd::MakeAxisAngleFromQ(qRotation, &axis.x, &axis.y, &axis.z, &angle);
    angle = pgd::RadiansToDegrees(angle);
    QSignalBlocker blocker5(ui->lineEditAngle);
    QSignalBlocker blocker6(ui->lineEditAxisX);
    QSignalBlocker blocker7(ui->lineEditAxisY);
    QSignalBlocker blocker8(ui->lineEditAxisZ);
    ui->lineEditAngle->setValue(angle);
    ui->lineEditAxisX->setValue(axis.x);
    ui->lineEditAxisY->setValue(axis.y);
    ui->lineEditAxisZ->setValue(axis.z);
}

void DialogMarkers::lineEditAxisAngleTextChanged(const QString & /*text*/)
{
    pgd::Vector3 axis;
    double angle = pgd::DegreesToRadians(ui->lineEditAngle->value());
    axis.x = ui->lineEditAxisX->value();
    axis.y = ui->lineEditAxisY->value();
    axis.z = ui->lineEditAxisZ->value();
    pgd::Quaternion qRotation;
    if (axis.Magnitude2() > std::numeric_limits<double>::epsilon()) qRotation = pgd::MakeQFromAxisAngle(axis, angle, false);
    else qRotation.Set(1, 0, 0, 0);
    QSignalBlocker blocker1(ui->lineEditQuaternionN);
    QSignalBlocker blocker2(ui->lineEditQuaternionX);
    QSignalBlocker blocker3(ui->lineEditQuaternionY);
    QSignalBlocker blocker4(ui->lineEditQuaternionZ);
    ui->lineEditQuaternionN->setValue(qRotation.n);
    ui->lineEditQuaternionX->setValue(qRotation.x);
    ui->lineEditQuaternionY->setValue(qRotation.y);
    ui->lineEditQuaternionZ->setValue(qRotation.z);
    pgd::Vector3 eulerAngles = pgd::MakeEulerAnglesFromQ(qRotation);
    QSignalBlocker blocker5(ui->lineEditEulerXAppend);
    QSignalBlocker blocker6(ui->lineEditEulerYAppend);
    QSignalBlocker blocker7(ui->lineEditEulerZAppend);
    ui->lineEditEulerXAppend->setValue(eulerAngles.x);
    ui->lineEditEulerYAppend->setValue(eulerAngles.y);
    ui->lineEditEulerZAppend->setValue(eulerAngles.z);
}

void DialogMarkers::lineEditQuaternionTextChanged(const QString & /*text*/)
{
    pgd::Quaternion qRotation;
    qRotation.n = ui->lineEditQuaternionN->value();
    qRotation.x = ui->lineEditQuaternionX->value();
    qRotation.y = ui->lineEditQuaternionY->value();
    qRotation.z = ui->lineEditQuaternionZ->value();
    double angle;
    pgd::Vector3 axis;
    pgd::MakeAxisAngleFromQ(qRotation, &axis.x, &axis.y, &axis.z, &angle);
    angle = pgd::RadiansToDegrees(angle);
    QSignalBlocker blocker1(ui->lineEditAngle);
    QSignalBlocker blocker2(ui->lineEditAxisX);
    QSignalBlocker blocker3(ui->lineEditAxisY);
    QSignalBlocker blocker4(ui->lineEditAxisZ);
    ui->lineEditAngle->setValue(angle);
    ui->lineEditAxisX->setValue(axis.x);
    ui->lineEditAxisY->setValue(axis.y);
    ui->lineEditAxisZ->setValue(axis.z);
    pgd::Vector3 eulerAngles = pgd::MakeEulerAnglesFromQ(qRotation);
    QSignalBlocker blocker5(ui->lineEditEulerXAppend);
    QSignalBlocker blocker6(ui->lineEditEulerYAppend);
    QSignalBlocker blocker7(ui->lineEditEulerZAppend);
    ui->lineEditEulerXAppend->setValue(eulerAngles.x);
    ui->lineEditEulerYAppend->setValue(eulerAngles.y);
    ui->lineEditEulerZAppend->setValue(eulerAngles.z);
}

void DialogMarkers::positionMarkerChanged(const QString & /* text */)
{
    double fraction = ui->lineEditFraction->value();
    auto markerList = m_simulation->GetMarkerList();
    if (markerList->find(ui->comboBoxPositionMarker1->currentText().toStdString()) == markerList->end()) return;
    if (markerList->find(ui->comboBoxPositionMarker2->currentText().toStdString()) == markerList->end()) return;
    Marker *marker1 = markerList->at(ui->comboBoxPositionMarker1->currentText().toStdString()).get();
    Marker *marker2 = markerList->at(ui->comboBoxPositionMarker2->currentText().toStdString()).get();

    pgd::Vector3 p1 = marker1->GetWorldPosition();
    pgd::Vector3 p2 = marker2->GetWorldPosition();
    pgd::Vector3 p = (p2 - p1) * fraction;
    QSignalBlocker blocker(ui->lineEditDistance);
    ui->lineEditDistance->setValue(p.Magnitude());
}

void DialogMarkers::orientation2MarkerChanged(const QString & /* text */)
{
    auto markerList = m_simulation->GetMarkerList();
    Marker *marker1, *marker2;
    pgd::Vector3 v2;
    if (markerList->find(ui->comboBoxOrientation2Marker1->currentText().toStdString()) == markerList->end()) goto disable_button;
    if (markerList->find(ui->comboBoxOrientation2Marker2->currentText().toStdString()) == markerList->end()) goto disable_button;
    marker1 = markerList->at(ui->comboBoxOrientation2Marker1->currentText().toStdString()).get();
    marker2 = markerList->at(ui->comboBoxOrientation2Marker2->currentText().toStdString()).get();

    v2 = marker2->GetWorldPosition() - marker1->GetWorldPosition();
    if (v2.Magnitude2() > 1e-10) goto enable_button;
    else goto disable_button;

enable_button:
    ui->pushButtonCalculateOrientation2Marker->setEnabled(true);
    return;
disable_button:
    ui->pushButtonCalculateOrientation2Marker->setEnabled(false);
    return;
}

void DialogMarkers::orientation3MarkerChanged(const QString & /* text */)
{
    auto markerList = m_simulation->GetMarkerList();
    Marker *marker1, *marker2, *marker3;
    pgd::Vector3 v1, v2;
    double d;
    if (markerList->find(ui->comboBoxOrientation3Marker1->currentText().toStdString()) == markerList->end()) goto disable_button;
    if (markerList->find(ui->comboBoxOrientation3Marker2->currentText().toStdString()) == markerList->end()) goto disable_button;
    if (markerList->find(ui->comboBoxOrientation3Marker3->currentText().toStdString()) == markerList->end()) goto disable_button;
    marker1 = markerList->at(ui->comboBoxOrientation3Marker1->currentText().toStdString()).get();
    marker2 = markerList->at(ui->comboBoxOrientation3Marker2->currentText().toStdString()).get();
    marker3 = markerList->at(ui->comboBoxOrientation3Marker3->currentText().toStdString()).get();

    v1 = (marker2->GetWorldPosition() - marker1->GetWorldPosition());
    v2 = (marker3->GetWorldPosition() - marker2->GetWorldPosition());

    if (v1.Magnitude2() < 1e-10 || v2.Magnitude2() < 1e-10) goto disable_button;
    v1.Normalize();
    v2.Normalize();
    d = v1 * v2;
    if (d < 0.9999999999) goto enable_button;
    else goto disable_button;

enable_button:
    ui->pushButtonCalculateOrientation3Marker->setEnabled(true);
    return;
disable_button:
    ui->pushButtonCalculateOrientation3Marker->setEnabled(false);
    return;
}

void DialogMarkers::labelAxisAngleMenuRequest(const QPoint &pos)
{
    QMenu menu(this);
    menu.addAction(tr("Set axis from matrix column 1"));
    menu.addAction(tr("Set axis from matrix column 2"));
    menu.addAction(tr("Set axis from matrix column 3"));

    QPoint gp = ui->labelAxisAngle->mapToGlobal(pos);
    QAction *action = menu.exec(gp);
    if (action)
    {
        if (action->text() == tr("Set axis from matrix column 1"))
        {
            ui->lineEditAxisX->setValue(ui->lineEditMatrixr1c1->value());
            ui->lineEditAxisY->setValue(ui->lineEditMatrixr2c1->value());
            ui->lineEditAxisZ->setValue(ui->lineEditMatrixr3c1->value());
        }
        if (action->text() == tr("Set axis from matrix column 2"))
        {
            ui->lineEditAxisX->setValue(ui->lineEditMatrixr1c2->value());
            ui->lineEditAxisY->setValue(ui->lineEditMatrixr2c2->value());
            ui->lineEditAxisZ->setValue(ui->lineEditMatrixr3c2->value());
        }
        if (action->text() == tr("Set axis from matrix column 3"))
        {
            ui->lineEditAxisX->setValue(ui->lineEditMatrixr1c3->value());
            ui->lineEditAxisY->setValue(ui->lineEditMatrixr2c3->value());
            ui->lineEditAxisZ->setValue(ui->lineEditMatrixr3c3->value());
        }
    }
}

void DialogMarkers::properties()
{
    DialogProperties dialogProperties(this);

#ifdef MARKER_AS_SPHERE
    SettingsItem radius = Preferences::settingsItem("MarkerSize");
    SettingsItem colour = Preferences::settingsItem("MarkerColour");
    if (m_inputMarker)
    {
        radius.value = m_inputMarker->size1();
        colour.value = QColor(QString::fromStdString(m_inputMarker->colour1().GetHexArgb()));
    }
    m_properties.clear();
    m_properties = { { radius.key, radius },
                     { colour.key, colour } };
#else
    SettingsItem radius = Preferences::settingsItem("MarkerSize");
    if (m_inputMarker)
    {
        radius.value = m_inputMarker->size1();
    }
    m_properties.clear();
    m_properties = { { radius.key, radius } };
#endif
    dialogProperties.setInputSettingsItems(m_properties);
    dialogProperties.initialise();

    int status = dialogProperties.exec();
    if (status == QDialog::Accepted)
    {
        dialogProperties.update();
        m_properties = dialogProperties.getOutputSettingsItems();
    }
}

void DialogMarkers::overrideStartPosition(const pgd::Vector3 &position)
{
    ui->lineEditPositionX->setValue(position.x);
    ui->lineEditPositionY->setValue(position.y);
    ui->lineEditPositionZ->setValue(position.z);
}


Simulation *DialogMarkers::simulation() const
{
    return m_simulation;
}

void DialogMarkers::setSimulation(Simulation *simulation)
{
    m_simulation = simulation;
}

std::unique_ptr<Marker> DialogMarkers::outputMarker()
{
    return std::move(m_outputMarker);
}

void DialogMarkers::setInputMarker(Marker *inputMarker)
{
    m_inputMarker = inputMarker;
}


