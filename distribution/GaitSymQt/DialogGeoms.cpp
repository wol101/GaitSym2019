#include "DialogGeoms.h"
#include "ui_DialogGeoms.h"

#include "Geom.h"
#include "Simulation.h"
#include "Preferences.h"
#include "SphereGeom.h"
#include "CappedCylinderGeom.h"
#include "BoxGeom.h"
#include "PlaneGeom.h"
#include "GSUtil.h"
#include "Marker.h"

#include <QDebug>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>


DialogGeoms::DialogGeoms(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogGeoms)
{
    ui->setupUi(this);

    setWindowTitle(tr("Geom Builder"));
#ifdef Q_OS_MACOS
    setWindowFlags(windowFlags() & (~Qt::Dialog) | Qt::Window); // allows the window to be resized on macs
#endif
    restoreGeometry(Preferences::valueQByteArray("DialogGeomsGeometry"));

    connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));

    // this logic monitors for changing values
    QList<QWidget *> widgets = this->findChildren<QWidget *>();
    for (auto it = widgets.begin(); it != widgets.end(); it++)
    {
        QComboBox *comboBox = dynamic_cast<QComboBox *>(*it);
        if (comboBox) connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxChanged(int)));
        QLineEdit *lineEdit = dynamic_cast<QLineEdit *>(*it);
        if (lineEdit) connect(lineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(lineEditChanged(const QString &)));
        QSpinBox *spinBox = dynamic_cast<QSpinBox *>(*it);
        if (spinBox) connect(spinBox, SIGNAL(valueChanged(const QString &)), this, SLOT(spinBoxChanged(const QString &)));
        QCheckBox *checkBox = dynamic_cast<QCheckBox *>(*it);
        if (checkBox) connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(checkBoxChanged(int)));
    }

}

DialogGeoms::~DialogGeoms()
{
    delete ui;
}

void DialogGeoms::accept() // this catches OK and return/enter
{
    qDebug() << "DialogGeoms::accept()";
    if (m_origGeom) delete m_origGeom;
    std::map<std::string, Marker *> *markerList = m_simulation->GetMarkerList();
    QString strapTab = ui->tabWidget->tabText(ui->tabWidget->currentIndex());
    if (strapTab == "Sphere")
    {
        SphereGeom *geom = new SphereGeom(m_simulation->GetSpaceID(), ui->lineEditSphereRadius->value());
        m_geom = geom;
    }
    else if (strapTab == "Capsule")
    {
        CappedCylinderGeom *geom = new CappedCylinderGeom(m_simulation->GetSpaceID(),
                ui->lineEditCapsuleRadius->value(), ui->lineEditCapsuleLength->value());
        m_geom = geom;
    }
    else if (strapTab == "Box")
    {
        BoxGeom *geom = new BoxGeom(m_simulation->GetSpaceID(), ui->lineEditBoxLengthX->value(),
                                    ui->lineEditBoxLengthY->value(), ui->lineEditBoxLengthZ->value());
        m_geom = geom;
    }
    else if (strapTab == "Plane")
    {
        Marker *geomMarker = markerList->at(ui->comboBoxGeomMarker->currentText().toStdString());
        pgd::Vector normal = geomMarker->GetAxis(Marker::Axis::Z);
        pgd::Vector point = geomMarker->GetPosition();
        double a = normal.x;
        double b = normal.y;
        double c = normal.z;
        double d = normal.Dot(point);
        PlaneGeom *geom = new PlaneGeom(m_simulation->GetSpaceID(), a, b, c, d);
        m_geom = geom;
    }

    m_geom->SetName(ui->lineEditGeomID->text().toStdString());
    m_geom->setSimulation(m_simulation);
    m_geom->setGeomMarker(markerList->at(ui->comboBoxGeomMarker->currentText().toStdString()));
    m_geom->SetSpringDamp(ui->lineEditSpring->value(), ui->lineEditDamp->value(),
                          m_simulation->GetTimeIncrement());
    m_geom->SetContactMu(ui->lineEditMu->value());
    m_geom->SetContactBounce(ui->lineEditBounce->value());
    m_geom->SetAbort(ui->checkBoxAbort->isChecked());

    Preferences::insert("DialogGeomsGeometry", saveGeometry());
    QDialog::accept();
}

void DialogGeoms::reject() // this catches cancel, close and escape key
{
    qDebug() << "DialogGeoms::reject()";
    Preferences::insert("DialogGeomsGeometry", saveGeometry());
    QDialog::reject();
}

void DialogGeoms::lateInitialise()
{
    Q_ASSERT_X(m_simulation, "DialogGeoms::lateInitialise", "simulation undefined");
    m_origGeom = m_geom;

    // set the marker lists
    std::map<std::string, Marker *> *markerList = m_simulation->GetMarkerList();
    QStringList markerIDs;
    for (auto it = markerList->begin(); it != markerList->end(); it++) markerIDs.append(QString::fromStdString(it->first));
    ui->comboBoxGeomMarker->addItems(markerIDs);

    // now set some sensible defaults
    ui->lineEditSpring->setBottom(std::numeric_limits<double>::min());
    ui->lineEditSpring->setValue(Preferences::valueDouble("GlobalDefaultSpringConstant"));
    ui->lineEditDamp->setBottom(std::numeric_limits<double>::min());
    ui->lineEditDamp->setValue(Preferences::valueDouble("GlobalDefaultDampingConstant"));
    ui->lineEditBounce->setBottom(0);
    ui->lineEditBounce->setValue(0);
    ui->lineEditMu->setBottom(0);
    ui->lineEditMu->setValue(1);
    ui->checkBoxAbort->setChecked(false);
    ui->checkBoxAdhesion->setChecked(false);

    ui->lineEditSphereRadius->setBottom(std::numeric_limits<double>::min());
    ui->lineEditSphereRadius->setValue(1);
    ui->lineEditCapsuleRadius->setBottom(std::numeric_limits<double>::min());
    ui->lineEditCapsuleRadius->setValue(1);
    ui->lineEditCapsuleLength->setBottom(std::numeric_limits<double>::min());
    ui->lineEditCapsuleLength->setValue(1);
    ui->lineEditBoxLengthX->setBottom(std::numeric_limits<double>::min());
    ui->lineEditBoxLengthX->setValue(1);
    ui->lineEditBoxLengthY->setBottom(std::numeric_limits<double>::min());
    ui->lineEditBoxLengthY->setValue(1);
    ui->lineEditBoxLengthZ->setBottom(std::numeric_limits<double>::min());
    ui->lineEditBoxLengthZ->setValue(1);

    if (m_geom)
    {
        QStringList tabNames;
        for (int i = 0; i < ui->tabWidget->count(); i++) tabNames.push_back(ui->tabWidget->tabText(i));
        std::string s;
        m_geom->SaveToAttributes();
        ui->lineEditGeomID->setText(QString::fromStdString(m_geom->GetAttribute("ID"s)));
        ui->lineEditGeomID->setEnabled(false);
        ui->comboBoxGeomMarker->setCurrentText(QString::fromStdString(m_geom->GetAttribute("MarkerID"s)));
        if ((s = m_geom->GetAttribute("SpringConstant"s)).size()) ui->lineEditSpring->setValue(GSUtil::Double(s));
        if ((s = m_geom->GetAttribute("DampingConstant"s)).size()) ui->lineEditDamp->setValue(GSUtil::Double(s));
        if ((s = m_geom->GetAttribute("Bounce"s)).size()) ui->lineEditBounce->setValue(GSUtil::Double(s));
        if ((s = m_geom->GetAttribute("Mu"s)).size()) ui->lineEditMu->setValue(GSUtil::Double(s));
        if ((s = m_geom->GetAttribute("Abort"s)).size()) ui->checkBoxAbort->setChecked(GSUtil::Bool(s));
        if ((s = m_geom->GetAttribute("Adhesion"s)).size()) ui->checkBoxAdhesion->setChecked(GSUtil::Bool(s));

        SphereGeom *sphereGeom = dynamic_cast<SphereGeom *>(m_geom);
        if (sphereGeom)
        {
            if ((s = sphereGeom->GetAttribute("Radius"s)).size()) ui->lineEditSphereRadius->setValue(GSUtil::Double(s));
            ui->tabWidget->setCurrentIndex(tabNames.indexOf("Sphere"));
        }

        CappedCylinderGeom *cappedCylinderGeom = dynamic_cast<CappedCylinderGeom *>(m_geom);
        if (cappedCylinderGeom)
        {
            if ((s = cappedCylinderGeom->GetAttribute("Radius"s)).size()) ui->lineEditCapsuleRadius->setValue(GSUtil::Double(s));
            if ((s = cappedCylinderGeom->GetAttribute("Length"s)).size()) ui->lineEditCapsuleLength->setValue(GSUtil::Double(s));
            ui->tabWidget->setCurrentIndex(tabNames.indexOf("Capsule"));
        }

        BoxGeom *boxGeom = dynamic_cast<BoxGeom *>(m_geom);
        if (boxGeom)
        {
            if ((s = boxGeom->GetAttribute("LengthX"s)).size()) ui->lineEditBoxLengthX->setValue(GSUtil::Double(s));
            if ((s = boxGeom->GetAttribute("LengthY"s)).size()) ui->lineEditBoxLengthY->setValue(GSUtil::Double(s));
            if ((s = boxGeom->GetAttribute("LengthZ"s)).size()) ui->lineEditBoxLengthZ->setValue(GSUtil::Double(s));
            ui->tabWidget->setCurrentIndex(tabNames.indexOf("Box"));
        }

        PlaneGeom *planeGeom = dynamic_cast<PlaneGeom *>(m_geom);
        if (planeGeom)
        {
            ui->tabWidget->setCurrentIndex(tabNames.indexOf("Plane"));
        }

    }
    else
    {
        std::map<std::string, Geom *> *geomList = m_simulation->GetGeomList();
        QStringList geomIDs;
        for (auto it = geomList->begin(); it != geomList->end(); it++) geomIDs.append(QString::fromStdString(it->first));

        // set default new name
        ui->lineEditGeomID->addStrings(geomIDs);
        int initialNameCount = 0;
        QString initialName = QString("Geom%1").arg(initialNameCount, 3, 10, QLatin1Char('0'));
        while (geomList->count(initialName.toStdString()))
        {
            initialNameCount++;
            initialName = QString("Geom%1").arg(initialNameCount, 3, 10, QLatin1Char('0'));
            if (initialNameCount >= 999) break; // only do this for the first 999 markers
        }
        ui->lineEditGeomID->setText(initialName);
    }
}

void DialogGeoms::comboBoxChanged(int index)
{
    Q_UNUSED(index);
    updateActivation();
}

void DialogGeoms::lineEditChanged(const QString &text)
{
    Q_UNUSED(text);
    updateActivation();
}

void DialogGeoms::spinBoxChanged(const QString &text)
{
    Q_UNUSED(text);
    updateActivation();
}

void DialogGeoms::checkBoxChanged(int index)
{
    Q_UNUSED(index);
    updateActivation();
}

void DialogGeoms::updateActivation()
{
    bool okEnable = true;
    QString textCopy = ui->lineEditGeomID->text();
    int pos = ui->lineEditGeomID->cursorPosition();
    if (ui->lineEditGeomID->validator()->validate(textCopy, pos) != QValidator::Acceptable) okEnable = false;

    ui->pushButtonOK->setEnabled(okEnable);
}

Simulation *DialogGeoms::simulation() const
{
    return m_simulation;
}

void DialogGeoms::setSimulation(Simulation *simulation)
{
    m_simulation = simulation;
}

Geom *DialogGeoms::geom() const
{
    return m_geom;
}

void DialogGeoms::setGeom(Geom *geom)
{
    m_geom = geom;
}

