#include "DialogBodyBuilder.h"
#include "ui_DialogBodyBuilder.h"

#include "SimulationWindow.h"
#include "Preferences.h"
#include "FacetedObject.h"
#include "Body.h"
#include "LineEditUniqueName.h"

#include "ode/ode.h"

#include <QBoxLayout>
#include <QFileInfo>
#include <QProgressDialog>
#include <QThread>
#include <QValidator>


DialogBodyBuilder::DialogBodyBuilder(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogBodyBuilder)
{
    ui->setupUi(this);

    setWindowTitle(tr("Body Builder"));
#ifdef Q_OS_MACOS
    setWindowFlags(windowFlags() & (~Qt::Dialog) |
                   Qt::Window); // allows the window to be resized on macs
#endif
    restoreGeometry(Preferences::valueQByteArray("DialogBodyBuilderGeometry"));
    ui->splitter->restoreState(Preferences::valueQByteArray("DialogBodyBuilderSplitterState"));

    // put SimulationWindow into existing widgetGLWidget
    QBoxLayout *boxLayout = new QBoxLayout(QBoxLayout::LeftToRight, ui->widget3DWindow);
    boxLayout->setContentsMargins(0, 0, 0, 0);
    m_simulationWindow = new SimulationWindow();
    QWidget *container = QWidget::createWindowContainer(m_simulationWindow);
    boxLayout->addWidget(container);
    m_simulationWindow->initialiseScene();
    m_simulationWindow->requestUpdate();

    connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->pushButtonCalculate, SIGNAL(clicked()), this, SLOT(calculate()));
    connect(ui->lineEditMesh1, SIGNAL(focussed(bool)), this, SLOT(lineEditMeshClicked()));
    connect(ui->lineEditMesh2, SIGNAL(focussed(bool)), this, SLOT(lineEditMeshClicked()));
    connect(ui->lineEditMesh3, SIGNAL(focussed(bool)), this, SLOT(lineEditMeshClicked()));
    connect(ui->lineEditMesh1, SIGNAL(editingFinished()), this, SLOT(lineEditMeshClicked()));
    connect(ui->lineEditMesh2, SIGNAL(editingFinished()), this, SLOT(lineEditMeshClicked()));
    connect(ui->lineEditMesh3, SIGNAL(editingFinished()), this, SLOT(lineEditMeshClicked()));
    connect(ui->lineEditID, SIGNAL(textChanged(const QString &)), this,
            SLOT(lineEditIDTextChanged(const QString &)));

    ui->pushButtonCalculate->setEnabled(false);
    ui->pushButtonOK->setEnabled(false);

    ui->lineEditDensity->setValue(1.0);
    ui->lineEditMass->setValue(1.0);
    ui->lineEditI11->setValue(1.0);
    ui->lineEditI22->setValue(1.0);
    ui->lineEditI33->setValue(1.0);
    ui->lineEditDensity->setBottom(DBL_MIN);
    ui->lineEditMass->setBottom(DBL_MIN);
    ui->lineEditI11->setBottom(DBL_MIN);
    ui->lineEditI22->setBottom(DBL_MIN);
    ui->lineEditI33->setBottom(DBL_MIN);

    qobject_cast<LineEditPath *>(ui->lineEditMesh1)->setPathType(LineEditPath::FileForOpen);
    qobject_cast<LineEditPath *>(ui->lineEditMesh2)->setPathType(LineEditPath::FileForOpen);
    qobject_cast<LineEditPath *>(ui->lineEditMesh3)->setPathType(LineEditPath::FileForOpen);
}

DialogBodyBuilder::~DialogBodyBuilder()
{
    delete ui;
}

void DialogBodyBuilder::setBody(Body *body)
{
    Q_ASSERT_X(body, "DialogBodyBuilder::setBody", "body not defined");
    m_body = body;
    dMass mass;
    m_body->GetMass(&mass);
    ui->lineEditMass->setValue(mass.mass);
    ui->lineEditI11->setValue(mass.I[0 * 4 + 0]);
    ui->lineEditI22->setValue(mass.I[1 * 4 + 1]);
    ui->lineEditI33->setValue(mass.I[2 * 4 + 2]);
    ui->lineEditI12->setValue(mass.I[0 * 4 + 1]);
    ui->lineEditI13->setValue(mass.I[0 * 4 + 2]);
    ui->lineEditI23->setValue(mass.I[1 * 4 + 2]);
    ui->lineEditDensity->setValue(m_body->GetConstructionDensity());
    const double *constructionPosition = m_body->GetConstructionPosition();
    ui->lineEditX->setValue(constructionPosition[0]);
    ui->lineEditY->setValue(constructionPosition[1]);
    ui->lineEditZ->setValue(constructionPosition[2]);
    const double *positionHighBound = m_body->GetPositionHighBound();
    ui->lineEditHighX->setValue(positionHighBound[0]);
    ui->lineEditHighY->setValue(positionHighBound[1]);
    ui->lineEditHighZ->setValue(positionHighBound[2]);
    const double *positionLowBound = m_body->GetPositionLowBound();
    ui->lineEditLowX->setValue(positionLowBound[0]);
    ui->lineEditLowY->setValue(positionLowBound[1]);
    ui->lineEditLowZ->setValue(positionLowBound[2]);
    const double *velocityHighBound = m_body->GetLinearVelocityHighBound();
    ui->lineEditHighVX->setValue(velocityHighBound[0]);
    ui->lineEditHighVY->setValue(velocityHighBound[1]);
    ui->lineEditHighVZ->setValue(velocityHighBound[2]);
    const double *velocityLowBound = m_body->GetLinearVelocityLowBound();
    ui->lineEditLowVX->setValue(velocityLowBound[0]);
    ui->lineEditLowVY->setValue(velocityLowBound[1]);
    ui->lineEditLowVZ->setValue(velocityLowBound[2]);
}

void DialogBodyBuilder::lateInitialise()
{
    Q_ASSERT_X(m_simulation, "DialogBodyBuilder::lateInitialise", "m_simulation not defined");
    if (m_createMode)
    {
        auto existingBodies = m_simulation->GetBodyList();
        QString name("World"); // World always exists
        ui->lineEditID->addString(name);
        for (auto iter : *existingBodies)
        {
            name = QString::fromStdString(iter.first);
            ui->lineEditID->addString(name);
        }
        int initialNameCount = 0;
        QString initialName = QString("Body%1").arg(initialNameCount, 3, 10, QLatin1Char('0'));
        while (existingBodies->count(initialName.toStdString()))
        {
            initialNameCount++;
            initialName = QString("Body%1").arg(initialNameCount, 3, 10, QLatin1Char('0'));
            if (initialNameCount >= 999) break; // only do this for the first 999 bodies
        }
        ui->lineEditID->setText(initialName);
    }
    else
    {
        ui->lineEditID->setText(QString::fromStdString(m_body->GetName()));
        ui->lineEditID->setEnabled(false);
        dMass mass;
        m_body->GetMass(&mass);
        ui->lineEditMass->setValue(mass.mass);
        ui->lineEditI11->setValue(mass.I[0 * 4 + 0]);
        ui->lineEditI22->setValue(mass.I[1 * 4 + 1]);
        ui->lineEditI33->setValue(mass.I[2 * 4 + 2]);
        ui->lineEditI12->setValue(mass.I[0 * 4 + 1]);
        ui->lineEditI13->setValue(mass.I[0 * 4 + 2]);
        ui->lineEditI23->setValue(mass.I[1 * 4 + 2]);
        ui->lineEditDensity->setValue(1000);
        const double *constructionPosition = m_body->GetConstructionPosition();
        ui->lineEditX->setValue(constructionPosition[0]);
        ui->lineEditY->setValue(constructionPosition[1]);
        ui->lineEditZ->setValue(constructionPosition[2]);
        const double *positionHighBound = m_body->GetPositionHighBound();;
        ui->lineEditHighX->setValue(positionHighBound[0]);
        ui->lineEditHighY->setValue(positionHighBound[1]);
        ui->lineEditHighZ->setValue(positionHighBound[2]);
        const double *positionLowBound = m_body->GetPositionLowBound();;
        ui->lineEditLowX->setValue(positionLowBound[0]);
        ui->lineEditLowY->setValue(positionLowBound[1]);
        ui->lineEditLowZ->setValue(positionLowBound[2]);
        const double *velocityHighBound = m_body->GetLinearVelocityHighBound();
        ui->lineEditHighVX->setValue(velocityHighBound[0]);
        ui->lineEditHighVY->setValue(velocityHighBound[1]);
        ui->lineEditHighVZ->setValue(velocityHighBound[2]);
        const double *velocityLowBound = m_body->GetLinearVelocityLowBound();
        ui->lineEditLowVX->setValue(velocityLowBound[0]);
        ui->lineEditLowVY->setValue(velocityLowBound[1]);
        ui->lineEditLowVZ->setValue(velocityLowBound[2]);
        ui->lineEditMesh1->setText(QString::fromStdString(m_body->GetGraphicFile1()));
        ui->lineEditMesh2->setText(QString::fromStdString(m_body->GetGraphicFile2()));
        ui->lineEditMesh3->setText(QString::fromStdString(m_body->GetGraphicFile3()));
        lineEditMeshActivated(ui->lineEditMesh1);
    }
}

void DialogBodyBuilder::accept() // this catches OK and return/enter
{
    qDebug() << "DialogBodyBuilder::accept()";

    if (!m_body) m_body = new Body(m_simulation->GetWorldID());
    m_body->SetName(ui->lineEditID->text().toStdString());
    m_body->SetGraphicFile1(ui->lineEditMesh1->text().toStdString());
    m_body->SetGraphicFile2(ui->lineEditMesh2->text().toStdString());
    m_body->SetGraphicFile3(ui->lineEditMesh3->text().toStdString());

    dMass mass;
    mass.c[0] = mass.c[1] = mass.c[2] = 0;
    mass.mass = ui->lineEditMass->value();
    mass.I[0 * 4 + 0] = ui->lineEditI11->value();
    mass.I[1 * 4 + 1] = ui->lineEditI22->value();
    mass.I[2 * 4 + 2] = ui->lineEditI33->value();
    mass.I[0 * 4 + 1] = ui->lineEditI12->value();
    mass.I[0 * 4 + 2] = ui->lineEditI13->value();
    mass.I[1 * 4 + 2] = ui->lineEditI23->value();
    m_body->SetMass(&mass);
    m_body->SetConstructionDensity(ui->lineEditDensity->value());
    dVector3 constructionPosition;
    constructionPosition[0] = ui->lineEditX->value();
    constructionPosition[1] = ui->lineEditY->value();
    constructionPosition[2] = ui->lineEditZ->value();
    m_body->SetConstructionPosition(constructionPosition[0], constructionPosition[1],
                                    constructionPosition[2]);
    m_body->SetPosition(constructionPosition[0], constructionPosition[1], constructionPosition[2]);
    dVector3 positionHighBound;
    positionHighBound[0] = ui->lineEditHighX->value();
    positionHighBound[1] = ui->lineEditHighY->value();
    positionHighBound[2] = ui->lineEditHighZ->value();
    m_body->SetPositionHighBound(positionHighBound[0], positionHighBound[1], positionHighBound[2]);
    dVector3 positionLowBound;
    positionLowBound[0] = ui->lineEditLowX->value();
    positionLowBound[1] = ui->lineEditLowY->value();
    positionLowBound[2] = ui->lineEditLowZ->value();
    m_body->SetPositionLowBound(positionLowBound[0], positionLowBound[1], positionLowBound[2]);
    dVector3 velocityHighBound;
    velocityHighBound[0] = ui->lineEditHighVX->value();
    velocityHighBound[1] = ui->lineEditHighVY->value();
    velocityHighBound[2] = ui->lineEditHighVZ->value();
    m_body->SetLinearVelocityHighBound(velocityHighBound[0], velocityHighBound[1],
                                       velocityHighBound[2]);
    dVector3 velocityLowBound;
    velocityLowBound[0] = ui->lineEditLowVX->value();
    velocityLowBound[1] = ui->lineEditLowVY->value();
    velocityLowBound[2] = ui->lineEditLowVZ->value();
    m_body->SetLinearVelocityLowBound(velocityLowBound[0], velocityLowBound[1], velocityLowBound[2]);

    Preferences::insert("DialogBodyBuilderGeometry", saveGeometry());
    Preferences::insert("DialogBodyBuilderSplitterState", ui->splitter->saveState());

    QDialog::accept();
}

void DialogBodyBuilder::reject() // this catches cancel, close and escapem key
{
    qDebug() << "DialogBodyBuilder::reject()";
    Preferences::insert("DialogBodyBuilderGeometry", saveGeometry());
    Preferences::insert("DialogBodyBuilderSplitterState", ui->splitter->saveState());

    QDialog::reject();
}

void DialogBodyBuilder::calculate()
{
    if (!m_referenceObject) return;
    dMass mass;
    double density = ui->lineEditDensity->text().toDouble();
    bool clockwise = false;
    m_referenceObject->CalculateMassProperties(&mass, density, clockwise);
    ui->lineEditMass->setValue(mass.mass);
    ui->lineEditX->setValue(mass.c[0]);
    ui->lineEditY->setValue(mass.c[1]);
    ui->lineEditZ->setValue(mass.c[2]);
// #define _I(i,j) I[(i)*4+(j)]
// regex _I\(([0-9]+),([0-9]+)\) to I[(\1)*4+(\2)]
//    mass->_I(0,0) = I11;
//    mass->_I(1,1) = I22;
//    mass->_I(2,2) = I33;
//    mass->_I(0,1) = I12;
//    mass->_I(0,2) = I13;
//    mass->_I(1,2) = I23;
//    mass->_I(1,0) = I12;
//    mass->_I(2,0) = I13;
//    mass->_I(2,1) = I23;
    ui->lineEditI11->setValue(mass.I[0 * 4 + 0]);
    ui->lineEditI22->setValue(mass.I[1 * 4 + 1]);
    ui->lineEditI33->setValue(mass.I[2 * 4 + 2]);
    ui->lineEditI12->setValue(mass.I[0 * 4 + 1]);
    ui->lineEditI13->setValue(mass.I[0 * 4 + 2]);
    ui->lineEditI23->setValue(mass.I[1 * 4 + 2]);
}

void DialogBodyBuilder::lineEditMeshClicked()
{
    LineEditPath *lineEdit = qobject_cast<LineEditPath *>(sender());
    if (lineEdit == nullptr) return;
    lineEditMeshActivated(lineEdit);
}

void DialogBodyBuilder::lineEditMeshActivated(LineEditPath *lineEdit)
{
    if (lineEdit->text().size() && lineEdit->text() != m_displayFileName)
    {
        QString filename = lineEdit->text();
        qDebug() << "DialogBodyBuilder::displayMesh " << filename;
        m_referenceObject = new FacetedObject(nullptr);
        int err = 1;
        if (filename.toLower().endsWith(".obj")) err = m_referenceObject->ParseOBJFile(
                        filename.toStdString());
        if (filename.toLower().endsWith(".ply")) err = m_referenceObject->ParsePLYFile(
                        filename.toStdString());
        if (err)
        {
            delete m_referenceObject;
            return;
        }
        m_displayFileName = filename;
        m_simulationWindow->initialiseScene();
        Qt3DCore::QEntity *rootEntity = m_simulationWindow->rootEntity();
        m_referenceObject->InitialiseEntity();
        m_referenceObject->setParent(rootEntity);
        if (lineEdit != ui->lineEditMesh1) ui->lineEditMesh1->setHighlighted(false);
        if (lineEdit != ui->lineEditMesh2) ui->lineEditMesh2->setHighlighted(false);
        if (lineEdit != ui->lineEditMesh3) ui->lineEditMesh3->setHighlighted(false);
        lineEdit->setHighlighted(true);
        ui->pushButtonCalculate->setEnabled(true);
    }
}

bool DialogBodyBuilder::createMode() const
{
    return m_createMode;
}

void DialogBodyBuilder::setCreateMode(bool createMode)
{
    m_createMode = createMode;
}

Simulation *DialogBodyBuilder::simulation() const
{
    return m_simulation;
}

void DialogBodyBuilder::setSimulation(Simulation *simulation)
{
    m_simulation = simulation;
}

Body *DialogBodyBuilder::body() const
{
    return m_body;
}


void DialogBodyBuilder::lineEditIDTextChanged(const QString & /* text */)
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(sender());
    if (lineEdit == nullptr) return;
    QString textCopy = lineEdit->text();
    int pos = lineEdit->cursorPosition();
    ui->pushButtonOK->setEnabled(lineEdit->validator()->validate(textCopy,
                                 pos) == QValidator::Acceptable);
}

void DialogBodyBuilder::displayMesh(const QString &filename)
{
    qDebug() << "DialogBodyBuilder::displayMesh " << filename;
    FacetedObject *facetedObject = new FacetedObject(nullptr);
    int err = 1;
    if (filename.toLower().endsWith(".obj")) err = facetedObject->ParseOBJFile(filename.toStdString());
    if (filename.toLower().endsWith(".ply")) err = facetedObject->ParsePLYFile(filename.toStdString());
    if (err)
    {
        delete facetedObject;
        return;
    }
    m_displayFileName = filename;
    m_simulationWindow->initialiseScene();
    Qt3DCore::QEntity *rootEntity = m_simulationWindow->rootEntity();
    facetedObject->InitialiseEntity();
    facetedObject->setParent(rootEntity);
}


