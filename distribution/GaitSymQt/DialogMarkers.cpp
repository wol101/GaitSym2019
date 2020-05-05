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

#include <map>

DialogMarkers::DialogMarkers(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogMarkers)
{
    ui->setupUi(this);

    setWindowTitle(tr("Marker Builder"));
#ifdef Q_OS_MACOS
    setWindowFlags(windowFlags() & (~Qt::Dialog) |
                   Qt::Window); // allows the window to be resized on macs
#endif
    restoreGeometry(Preferences::valueQByteArray("DialogMarkersGeometry"));

    QString mirrorAxis = Preferences::valueQString("DialogMarkersMirrorAxis");
    ui->radioButtonX->setChecked(mirrorAxis == "X");
    ui->radioButtonY->setChecked(mirrorAxis == "Y");
    ui->radioButtonZ->setChecked(mirrorAxis == "Z");

    ui->lineEditFraction->setBottom(0.0);
    ui->lineEditFraction->setTop(1.0);
    ui->lineEditFraction->setValue(0.5);

    connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->pushButtonProperties, SIGNAL(clicked()), this, SLOT(properties()));
    connect(ui->pushButtonCalculatePosition, SIGNAL(clicked()), this, SLOT(calculatePosition()));
    connect(ui->pushButtonCopyMarker1, SIGNAL(clicked()), this, SLOT(calculatePositionCopyMarker1()));
    connect(ui->pushButtonCopyMarker2, SIGNAL(clicked()), this, SLOT(calculatePositionCopyMarker2()));
    connect(ui->pushButtonCalculateOrientation2Marker, SIGNAL(clicked()), this, SLOT(calculateOrientation2Marker()));
    connect(ui->pushButtonCalculateOrientation3Marker, SIGNAL(clicked()), this, SLOT(calculateOrientation3Marker()));
    connect(ui->pushButtonCalculateMirrorMarker, SIGNAL(clicked()), this, SLOT(calculateMirrorMarker()));
    connect(ui->pushButton3DCursor, SIGNAL(clicked()), this, SLOT(copy3DCursorPosition()));
    connect(ui->lineEditMarkerID, SIGNAL(textChanged(const QString &)), this, SLOT(lineEditIDTextChanged(const QString &)));
    connect(ui->comboBoxOrientation2Marker1, SIGNAL(currentTextChanged(const QString &)), this, SLOT(orientation2MarkerChanged(const QString &)));
    connect(ui->comboBoxOrientation2Marker2, SIGNAL(currentTextChanged(const QString &)), this, SLOT(orientation2MarkerChanged(const QString &)));
    connect(ui->comboBoxOrientation3Marker1, SIGNAL(currentTextChanged(const QString &)), this, SLOT(orientation3MarkerChanged(const QString &)));
    connect(ui->comboBoxOrientation3Marker2, SIGNAL(currentTextChanged(const QString &)), this, SLOT(orientation3MarkerChanged(const QString &)));
    connect(ui->comboBoxOrientation3Marker3, SIGNAL(currentTextChanged(const QString &)), this, SLOT(orientation3MarkerChanged(const QString &)));
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
        markerPtr->SetQuaternion(qLocal.n, qLocal.v.x, qLocal.v.y, qLocal.v.z);
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
        markerPtr->SetQuaternion(qWorld.n, qWorld.v.x, qWorld.v.y, qWorld.v.z);
    }

    if (m_inputMarker)
    {
        markerPtr->setSize1(m_inputMarker->size1());
        markerPtr->setColour1(m_inputMarker->colour1());
        for (auto it : *m_inputMarker->dependentList())
        {
            markerPtr->addDependent(it);
        }
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

    QString mirrorAxis;
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
    QString mirrorAxis;
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
    pgd::Vector eulerAngles;
    pgd::Vector position(double(m_cursor3DPosition.x()), double(m_cursor3DPosition.y()), double(m_cursor3DPosition.z()));
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
}

void DialogMarkers::calculatePosition()
{
    double fraction = ui->lineEditFraction->value();
    auto markerList = m_simulation->GetMarkerList();
    Marker *marker1 = markerList->at(ui->comboBoxPositionMarker1->currentText().toStdString()).get();
    Marker *marker2 = markerList->at(ui->comboBoxPositionMarker2->currentText().toStdString()).get();

    pgd::Vector p1 = marker1->GetWorldPosition();
    pgd::Vector p2 = marker2->GetWorldPosition();
    pgd::Vector p = p1 + (p2 - p1) * fraction;
    ui->lineEditPositionX->setValue(p.x);
    ui->lineEditPositionY->setValue(p.y);
    ui->lineEditPositionZ->setValue(p.z);

    pgd::Quaternion q1 = marker1->GetWorldQuaternion();
    pgd::Quaternion q2 = marker2->GetWorldQuaternion();
    pgd::Quaternion q = slerp(q1, q2, fraction, true);
    pgd::Vector e = pgd::MakeEulerAnglesFromQ(q);
    ui->lineEditEulerX->setValue(e.x);
    ui->lineEditEulerY->setValue(e.y);
    ui->lineEditEulerZ->setValue(e.z);
}

void DialogMarkers::calculatePositionCopyMarker1()
{
    auto markerList = m_simulation->GetMarkerList();
    Marker *marker = markerList->at(ui->comboBoxPositionMarker1->currentText().toStdString()).get();

    pgd::Vector p = marker->GetWorldPosition();
    ui->lineEditPositionX->setValue(p.x);
    ui->lineEditPositionY->setValue(p.y);
    ui->lineEditPositionZ->setValue(p.z);

    pgd::Quaternion q = marker->GetWorldQuaternion();
    pgd::Vector e = pgd::MakeEulerAnglesFromQ(q);
    ui->lineEditEulerX->setValue(e.x);
    ui->lineEditEulerY->setValue(e.y);
    ui->lineEditEulerZ->setValue(e.z);
}

void DialogMarkers::calculatePositionCopyMarker2()
{
    auto markerList = m_simulation->GetMarkerList();
    Marker *marker = markerList->at(ui->comboBoxPositionMarker2->currentText().toStdString()).get();

    pgd::Vector p = marker->GetWorldPosition();
    ui->lineEditPositionX->setValue(p.x);
    ui->lineEditPositionY->setValue(p.y);
    ui->lineEditPositionZ->setValue(p.z);

    pgd::Quaternion q = marker->GetWorldQuaternion();
    pgd::Vector e = pgd::MakeEulerAnglesFromQ(q);
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

    pgd::Vector v1(1, 0, 0);
    pgd::Vector v2 = marker2->GetWorldPosition() - marker1->GetWorldPosition();
    pgd::Quaternion q = pgd::FindRotation(v1, v2);
    pgd::Vector e = pgd::MakeEulerAnglesFromQ(q);
    ui->lineEditEulerX->setValue(e.x);
    ui->lineEditEulerY->setValue(e.y);
    ui->lineEditEulerZ->setValue(e.z);
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

    pgd::Vector xAxis = (marker2->GetWorldPosition() - marker1->GetWorldPosition());
    xAxis.Normalize();
    pgd::Vector zAxis = xAxis ^ (marker3->GetWorldPosition() - marker2->GetWorldPosition());
    zAxis.Normalize();
    pgd::Vector yAxis = zAxis ^ xAxis;
    yAxis.Normalize();

    pgd::Matrix3x3 R(xAxis.x, yAxis.x, zAxis.x,
                     xAxis.y, yAxis.y, zAxis.y,
                     xAxis.z, yAxis.z, zAxis.z);
    pgd::Quaternion q = pgd::MakeQfromM(R);
    pgd::Vector e = pgd::MakeEulerAnglesFromQ(q);
    ui->lineEditEulerX->setValue(e.x);
    ui->lineEditEulerY->setValue(e.y);
    ui->lineEditEulerZ->setValue(e.z);
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
    pgd::Vector p = m * marker->GetWorldPosition();
    ui->lineEditPositionX->setValue(p.x);
    ui->lineEditPositionY->setValue(p.y);
    ui->lineEditPositionZ->setValue(p.z);

    pgd::Quaternion q = marker->GetWorldQuaternion();
    q.v = m * q.v;
    pgd::Vector e = pgd::MakeEulerAnglesFromQ(q);
    ui->lineEditEulerX->setValue(e.x);
    ui->lineEditEulerY->setValue(e.y);
    ui->lineEditEulerZ->setValue(e.z);
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

void DialogMarkers::orientation2MarkerChanged(const QString & /* text */)
{
    auto markerList = m_simulation->GetMarkerList();
    Marker *marker1, *marker2;
    pgd::Vector v2;
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
    pgd::Vector v1, v2;
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

void DialogMarkers::overrideStartPosition(const pgd::Vector &position)
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


