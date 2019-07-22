#include "DialogDrivers.h"
#include "ui_DialogDrivers.h"

#include "Driver.h"
#include "Simulation.h"
#include "Preferences.h"
#include "CyclicDriver.h"
#include "FixedDriver.h"
#include "StackedBoxCarDriver.h"
#include "StepDriver.h"
#include "GSUtil.h"
#include "Muscle.h"
#include "Controller.h"

#include "pystring.h"

#include <QDebug>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QScrollArea>

DialogDrivers::DialogDrivers(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogDrivers)
{
    ui->setupUi(this);

    setWindowTitle(tr("Driver Builder"));
#ifdef Q_OS_MACOS
    setWindowFlags(windowFlags() & (~Qt::Dialog) | Qt::Window); // allows the window to be resized on macs
#endif
    restoreGeometry(Preferences::valueQByteArray("DialogDriversGeometry"));

    // set up the targets area
    QVBoxLayout *verticalLayout;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    verticalLayout = new QVBoxLayout();
    verticalLayout->setSpacing(6);
    verticalLayout->setContentsMargins(11, 11, 11, 11);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout1"));
    scrollArea = new QScrollArea();
    scrollArea->setObjectName(QStringLiteral("scrollArea1"));
    scrollArea->setWidgetResizable(true);
    scrollAreaWidgetContents = new QWidget();
    scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents1"));
    m_targetGridLayout = new QGridLayout();
    m_targetGridLayout->setSpacing(6);
    m_targetGridLayout->setContentsMargins(11, 11, 11, 11);
    m_targetGridLayout->setObjectName(QStringLiteral("gridLayout1"));
    scrollAreaWidgetContents->setLayout(m_targetGridLayout);
    scrollArea->setWidget(scrollAreaWidgetContents);
    verticalLayout->addWidget(scrollArea);
    ui->widgetTargetsPlaceholder->setLayout(verticalLayout);

    verticalLayout = new QVBoxLayout();
    verticalLayout->setSpacing(6);
    verticalLayout->setContentsMargins(11, 11, 11, 11);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout2"));
    scrollArea = new QScrollArea();
    scrollArea->setObjectName(QStringLiteral("scrollArea2"));
    scrollArea->setWidgetResizable(true);
    scrollAreaWidgetContents = new QWidget();
    scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents2"));
    m_boxcarGridLayout = new QGridLayout();
    m_boxcarGridLayout->setSpacing(6);
    m_boxcarGridLayout->setContentsMargins(11, 11, 11, 11);
    m_boxcarGridLayout->setObjectName(QStringLiteral("gridLayout2"));
    scrollAreaWidgetContents->setLayout(m_boxcarGridLayout);
    scrollArea->setWidget(scrollAreaWidgetContents);
    verticalLayout->addWidget(scrollArea);
    ui->widgetBoxcarPlaceholder->setLayout(verticalLayout);

    connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

    // this logic monitors for changing values
    QList<QWidget *> widgets = this->findChildren<QWidget *>();
    for (auto it = widgets.begin(); it != widgets.end(); it++)
    {
        QComboBox *comboBox = dynamic_cast<QComboBox *>(*it);
        if (comboBox) connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxChanged(int)));
        QLineEdit *lineEdit = dynamic_cast<QLineEdit *>(*it);
        if (lineEdit) connect(lineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(lineEditChanged(const QString &)));
//        commented out because the spin boxes are handled explicitly
//        QSpinBox *spinBox = dynamic_cast<QSpinBox *>(*it);
//        if (spinBox) connect(spinBox, SIGNAL(valueChanged(const QString &)), this, SLOT(spinBoxChanged(const QString &)));
        QCheckBox *checkBox = dynamic_cast<QCheckBox *>(*it);
        if (checkBox) connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(checkBoxChanged(int)));
    }

    connect(ui->spinBoxTargets, SIGNAL(valueChanged(const QString &)), this, SLOT(spinBoxChangedTargets(const QString &)));
    connect(ui->spinBoxSteps, SIGNAL(valueChanged(const QString &)), this, SLOT(spinBoxChangedSteps(const QString &)));
    connect(ui->spinBoxStepsPerCycle, SIGNAL(valueChanged(const QString &)), this, SLOT(spinBoxChangedStepsPerCycle(const QString &)));
    connect(ui->spinBoxBoxcarStackSize, SIGNAL(valueChanged(const QString &)), this, SLOT(spinBoxChangedBoxcarStackSize(const QString &)));

    ui->pushButtonOK->setEnabled(false);

}

DialogDrivers::~DialogDrivers()
{
    delete ui;
}

void DialogDrivers::closeEvent(QCloseEvent *event)
{
    Preferences::insert("DialogDriversGeometry", saveGeometry());
    QDialog::closeEvent(event);
}

void DialogDrivers::accept() // this catches OK and return/enter
{
    qDebug() << "DialogDrivers::accept()";
    if (m_origDriver) delete m_origDriver;

    QString tab = ui->tabWidget->tabText(ui->tabWidget->currentIndex());
    if (tab == "Fixed")
    {
        FixedDriver *driver = new FixedDriver();
        driver->SetValue(ui->lineEditFixedValue->value());
        m_driver = driver;
    }
    else if (tab == "Step")
    {
        StepDriver *driver = new StepDriver();
        size_t steps = static_cast<size_t>(ui->spinBoxSteps->value());
        std::vector<double> durations;
        durations.reserve(steps);
        std::vector<double> values;
        values.reserve(steps);
        for (int i = 0; i < ui->spinBoxSteps->value(); i++)
        {
            durations.push_back(ui->tableWidgetStep->itemAt(0, i)->text().toDouble());
            values.push_back(ui->tableWidgetStep->itemAt(1, i)->text().toDouble());
        }
        driver->SetValuesAndDurations(steps, values.data(), durations.data());
        m_driver = driver;
    }

    else if (tab == "Cyclic")
    {
        CyclicDriver *driver = new CyclicDriver();
        size_t steps = static_cast<size_t>(ui->spinBoxStepsPerCycle->value());
        std::vector<double> durations;
        durations.reserve(steps);
        std::vector<double> values;
        values.reserve(steps);
        for (int i = 0; i < ui->spinBoxStepsPerCycle->value(); i++)
        {
            durations.push_back(ui->tableWidgetStep->itemAt(0, i)->text().toDouble());
            values.push_back(ui->tableWidgetStep->itemAt(1, i)->text().toDouble());
        }
        driver->SetValuesAndDurations(int(steps), values.data(), durations.data());
        m_driver = driver;
    }

    else if (tab == "Boxcar")
    {
        StackedBoxcarDriver *driver = new StackedBoxcarDriver();
        driver->SetCycleTime(ui->lineEditBoxcarCycleTime->value());
        size_t stackSize = static_cast<size_t>(ui->spinBoxBoxcarStackSize->value());
        driver->SetStackSize(stackSize);
        std::vector<double> delays;
        delays.reserve(stackSize);
        std::vector<double> widths;
        widths.reserve(stackSize);
        std::vector<double> heights;
        heights.reserve(stackSize);
        for (int i = 0; i < ui->spinBoxBoxcarStackSize->value(); i++)
        {
            delays.push_back(m_boxcarLineEditDoubleList[i * 3 + 0]->value());
            widths.push_back(m_boxcarLineEditDoubleList[i * 3 + 1]->value());
            heights.push_back(m_boxcarLineEditDoubleList[i * 3 + 2]->value());
        }
        driver->SetDelays(delays.data());
        driver->SetWidths(widths.data());
        driver->SetHeights(heights.data());
        m_driver = driver;
    }

    m_driver->setSimulation(m_simulation);
    m_driver->SetName(ui->lineEditDriverID->text().toStdString());
    m_driver->setMinValue(ui->lineEditMinimum->value());
    m_driver->setMaxValue(ui->lineEditMaximum->value());
    m_driver->setInterp(ui->checkBoxInterpolate->isChecked());
    for (int i = 0; i < m_targetComboBoxList.size(); i++)
    {
        std::string name = m_targetComboBoxList[i]->currentText().toStdString();
        Muscle *muscle = m_simulation->GetMuscle(name);
        if (muscle) { m_driver->AddTarget(muscle); continue; }
        Controller *controller = m_simulation->GetController(name);
        if (controller) { m_driver->AddTarget(controller); continue; }
    }

    QDialog::accept();
}

void DialogDrivers::reject() // this catches cancel, close and escape key
{
    qDebug() << "DialogDrivers::reject()";
    QDialog::reject();
}

void DialogDrivers::lateInitialise()
{
    Q_ASSERT_X(m_simulation, "DialogDrivers::lateInitialise", "simulation undefined");
    m_origDriver = m_driver;

    const QSignalBlocker blocker1(ui->spinBoxSteps);
    const QSignalBlocker blocker2(ui->spinBoxStepsPerCycle);
    const QSignalBlocker blocker3(ui->spinBoxTargets);
    const QSignalBlocker blocker4(ui->spinBoxBoxcarStackSize);

    // set the lists
    for (auto it = m_simulation->GetMuscleList()->begin(); it != m_simulation->GetMuscleList()->end(); it++)
        m_drivableIDs.append(QString::fromStdString(it->first));
    for (auto it = m_simulation->GetControllerList()->begin(); it != m_simulation->GetControllerList()->end(); it++)
        m_drivableIDs.append(QString::fromStdString(it->first));
    QStringList tabNames;
    for (int i = 0; i < ui->tabWidget->count(); i++) tabNames.push_back(ui->tabWidget->tabText(i));

    // now set some sensible defaults
    ui->lineEditMinimum->setValue(0);
    ui->lineEditMaximum->setValue(1);
    ui->checkBoxInterpolate->setChecked(false);
    ui->spinBoxTargets->setValue(1);

    ui->lineEditFixedValue->setValue(1);

    ui->tableWidgetStep->setColumnCount(2);
    ui->tableWidgetStep->setRowCount(1);
    QStringList labels;
    labels << "Duration" << "Value";
    ui->tableWidgetStep->setHorizontalHeaderLabels(labels);
    ui->spinBoxSteps->setValue(1);
    ui->tableWidgetStep->setItem(0, 0, new QTableWidgetItem("1"));
    ui->tableWidgetStep->setItem(0, 1, new QTableWidgetItem("1"));

    ui->tableWidgetCyclic->setColumnCount(2);
    ui->tableWidgetCyclic->setRowCount(1);
    labels.clear();
    labels << "Duration" << "Value";
    ui->tableWidgetCyclic->setHorizontalHeaderLabels(labels);
    ui->spinBoxStepsPerCycle->setValue(1);
    ui->tableWidgetCyclic->setItem(0, 0, new QTableWidgetItem("1"));
    ui->tableWidgetCyclic->setItem(0, 1, new QTableWidgetItem("1"));

    ui->tabWidget->setCurrentIndex(tabNames.indexOf("Fixed"));

    if (m_driver)
    {
        std::string s;
        m_driver->SaveToAttributes();
        ui->lineEditDriverID->setText(QString::fromStdString(m_driver->GetAttribute("ID"s)));
        ui->lineEditDriverID->setEnabled(false);

        s = m_driver->GetAttribute("TargetIDList"s);
        std::vector<std::string> targetNames;
        pystring::split(s, targetNames);
        for (size_t i = 0; i < targetNames.size(); i++)
        {
            QLabel *label = new QLabel();
            label->setText(QString("Target %1").arg(1));
            m_targetGridLayout->addWidget(label, 0, 0, Qt::AlignTop);
            QComboBox *comboBoxDrivable = new QComboBox();
            comboBoxDrivable->addItems(m_drivableIDs);
            comboBoxDrivable->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            comboBoxDrivable->setCurrentText(QString::fromStdString(targetNames[i]));
            m_targetGridLayout->addWidget(comboBoxDrivable, 0, 1, Qt::AlignTop);
            m_targetLabelList.push_back(label);
            m_targetComboBoxList.push_back(comboBoxDrivable);
        }
        m_targetGridSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        m_targetGridLayout->addItem(m_targetGridSpacer, ui->spinBoxTargets->value(), 0);

        FixedDriver *fixedDriver = dynamic_cast<FixedDriver *>(m_driver);
        if (fixedDriver)
        {
            if ((s = fixedDriver->GetAttribute("Value"s)).size()) ui->lineEditFixedValue->setValue(GSUtil::Double(s));
            ui->tabWidget->setCurrentIndex(tabNames.indexOf("Fixed"));
        }

        StackedBoxcarDriver *stackedBoxcarDriver = dynamic_cast<StackedBoxcarDriver *>(m_driver);
        if (stackedBoxcarDriver)
        {
            int stackSize = GSUtil::Int(stackedBoxcarDriver->GetAttribute("StackSize"s));
            std::vector<double> delays(static_cast<size_t>(stackSize));
            std::vector<double> widths(static_cast<size_t>(stackSize));
            std::vector<double> heights(static_cast<size_t>(stackSize));
            GSUtil::Double(stackedBoxcarDriver->GetAttribute("Delays"s), stackSize, delays.data());
            GSUtil::Double(stackedBoxcarDriver->GetAttribute("Widths"s), stackSize, widths.data());
            GSUtil::Double(stackedBoxcarDriver->GetAttribute("Heights"s), stackSize, heights.data());
            for (int i = 0; i < stackSize; i++)
            {
                QLabel *label = new QLabel();
                label->setText(QString("Delay %1").arg(i + 1));
                m_boxcarGridLayout->addWidget(label, i, 0, Qt::AlignTop);
                m_boxcarLabelList.push_back(label);
                LineEditDouble *lineEditDelay = new LineEditDouble();
                lineEditDelay->setValue(delays[size_t(i)]);
                m_boxcarGridLayout->addWidget(lineEditDelay, i, 1, Qt::AlignTop);
                m_boxcarLineEditDoubleList.push_back(lineEditDelay);
                label = new QLabel();
                label->setText(QString("Width %1").arg(i + 1));
                m_boxcarGridLayout->addWidget(label, i, 2, Qt::AlignTop);
                m_boxcarLabelList.push_back(label);
                LineEditDouble *lineEditWidth = new LineEditDouble();
                lineEditWidth->setValue(widths[size_t(i)]);
                m_boxcarGridLayout->addWidget(lineEditWidth, i, 3, Qt::AlignTop);
                m_boxcarLineEditDoubleList.push_back(lineEditWidth);
                label = new QLabel();
                label->setText(QString("Height %1").arg(i + 1));
                m_boxcarGridLayout->addWidget(label, i, 4, Qt::AlignTop);
                m_boxcarLabelList.push_back(label);
                LineEditDouble *lineEditHeight = new LineEditDouble();
                lineEditHeight->setValue(heights[size_t(i)]);
                m_boxcarGridLayout->addWidget(lineEditHeight, i, 5, Qt::AlignTop);
                m_boxcarLineEditDoubleList.push_back(lineEditHeight);
            }
            m_boxcarGridSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
            m_boxcarGridLayout->addItem(m_boxcarGridSpacer, ui->spinBoxTargets->value(), 0);

            ui->spinBoxBoxcarStackSize->setValue(stackSize);
            ui->lineEditBoxcarCycleTime->setValue(GSUtil::Double(stackedBoxcarDriver->GetAttribute("CycleTime"s)));
            ui->tabWidget->setCurrentIndex(tabNames.indexOf("Boxcar"));
        }

    }
    else
    {
        std::map<std::string, Driver *> *driverList = m_simulation->GetDriverList();
        QStringList driverIDs;
        for (auto it = driverList->begin(); it != driverList->end(); it++) driverIDs.append(QString::fromStdString(it->first));
        ui->lineEditDriverID->addStrings(driverIDs);
        int initialNameCount = 0;
        QString initialName = QString("Driver%1").arg(initialNameCount, 3, 10, QLatin1Char('0'));
        while (driverList->count(initialName.toStdString()))
        {
            initialNameCount++;
            initialName = QString("Driver%1").arg(initialNameCount, 3, 10, QLatin1Char('0'));
            if (initialNameCount >= 999) break; // only do this for the first 999 markers
        }
        ui->lineEditDriverID->setText(initialName);

        QLabel *label = new QLabel();
        label->setText(QString("Target %1").arg(1));
        m_targetGridLayout->addWidget(label, 0, 0, Qt::AlignTop);
        QComboBox *comboBoxDrivable = new QComboBox();
        comboBoxDrivable->addItems(m_drivableIDs);
        comboBoxDrivable->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        m_targetGridLayout->addWidget(comboBoxDrivable, 0, 1, Qt::AlignTop);
        m_targetLabelList.push_back(label);
        m_targetComboBoxList.push_back(comboBoxDrivable);
        m_targetGridSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        m_targetGridLayout->addItem(m_targetGridSpacer, ui->spinBoxTargets->value(), 0);

        int stackSize = 1;
        std::vector<double> delays = { 0.5 };
        std::vector<double> widths = { 0.5 };
        std::vector<double> heights = { 1.0 };
        for (int i = 0; i < stackSize; i++)
        {
            label = new QLabel();
            label->setText(QString("Delay %1").arg(i + 1));
            m_boxcarGridLayout->addWidget(label, i, 0, Qt::AlignTop);
            m_boxcarLabelList.push_back(label);
            LineEditDouble *lineEditDelay = new LineEditDouble();
            lineEditDelay->setValue(delays[size_t(i)]);
            m_boxcarGridLayout->addWidget(lineEditDelay, i, 1, Qt::AlignTop);
            m_boxcarLineEditDoubleList.push_back(lineEditDelay);
            label = new QLabel();
            label->setText(QString("Width %1").arg(i + 1));
            m_boxcarGridLayout->addWidget(label, i, 2, Qt::AlignTop);
            m_boxcarLabelList.push_back(label);
            LineEditDouble *lineEditWidth = new LineEditDouble();
            lineEditWidth->setValue(widths[size_t(i)]);
            m_boxcarGridLayout->addWidget(lineEditWidth, i, 3, Qt::AlignTop);
            m_boxcarLineEditDoubleList.push_back(lineEditWidth);
            label = new QLabel();
            label->setText(QString("Height %1").arg(i + 1));
            m_boxcarGridLayout->addWidget(label, i, 4, Qt::AlignTop);
            m_boxcarLabelList.push_back(label);
            LineEditDouble *lineEditHeight = new LineEditDouble();
            lineEditHeight->setValue(heights[size_t(i)]);
            m_boxcarGridLayout->addWidget(lineEditHeight, i, 5, Qt::AlignTop);
            m_boxcarLineEditDoubleList.push_back(lineEditHeight);
        }
        m_boxcarGridSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
        m_boxcarGridLayout->addItem(m_boxcarGridSpacer, ui->spinBoxTargets->value(), 0);

        ui->spinBoxBoxcarStackSize->setValue(stackSize);
        ui->lineEditBoxcarCycleTime->setValue(1.0);
        ui->tabWidget->setCurrentIndex(tabNames.indexOf("Boxcar"));
    }
}

void DialogDrivers::tabChanged(int index)
{
    Q_UNUSED(index);
    updateActivation();
}

void DialogDrivers::comboBoxChanged(int index)
{
    Q_UNUSED(index);
    updateActivation();
}

void DialogDrivers::lineEditChanged(const QString &text)
{
    Q_UNUSED(text);
    updateActivation();
}

void DialogDrivers::spinBoxChangedTargets(const QString &text)
{
    Q_UNUSED(text);

    // store the current values in the list
    QVector<QString> oldValues(m_targetComboBoxList.size());
    for (int i = 0; i < m_targetComboBoxList.size(); i++) oldValues[i] = m_targetComboBoxList[i]->currentText();

    // delete all the existing widgets in the layout
    if (m_targetGridSpacer)
    {
        m_targetGridLayout->removeItem(m_targetGridSpacer);
        delete m_targetGridSpacer;
    }
    for (auto it = m_targetLabelList.rbegin(); it != m_targetLabelList.rend(); it++)
    {
        m_targetGridLayout->removeWidget(*it);
        delete *it;
    }
    m_targetLabelList.clear();
    for (auto it = m_targetComboBoxList.rbegin(); it != m_targetComboBoxList.rend(); it++)
    {
        m_targetGridLayout->removeWidget(*it);
        delete *it;
    }
    m_targetComboBoxList.clear();

    // now create a new set
    for (int i = 0; i < ui->spinBoxTargets->value(); i++)
    {
        QLabel *label = new QLabel();
        label->setText(QString("Target %1").arg(i + 1));
        m_targetGridLayout->addWidget(label, i, 0, Qt::AlignTop);
        QComboBox *comboBoxDrivable = new QComboBox();
        comboBoxDrivable->addItems(m_drivableIDs);
        comboBoxDrivable->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        m_targetGridLayout->addWidget(comboBoxDrivable, i, 1, Qt::AlignTop);
        m_targetLabelList.push_back(label);
        m_targetComboBoxList.push_back(comboBoxDrivable);
        if (i < oldValues.size()) comboBoxDrivable->setCurrentText(oldValues[i]);
    }
    m_targetGridSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    m_targetGridLayout->addItem(m_targetGridSpacer, ui->spinBoxTargets->value(), 0);

    updateActivation();
}
void DialogDrivers::spinBoxChangedSteps(const QString &text)
{
    Q_UNUSED(text);
    ui->tableWidgetStep->setRowCount(ui->spinBoxSteps->value());
    updateActivation();
}

void DialogDrivers::spinBoxChangedStepsPerCycle(const QString &text)
{
    Q_UNUSED(text);
    ui->tableWidgetCyclic->setRowCount(ui->spinBoxStepsPerCycle->value());
    updateActivation();
}

void DialogDrivers::spinBoxChangedBoxcarStackSize(const QString &text)
{
    Q_UNUSED(text);

    // store the current values in the list
    QVector<QString> oldValues(m_boxcarLineEditDoubleList.size());
    for (int i = 0; i < m_boxcarLineEditDoubleList.size(); i++) oldValues[i] = m_boxcarLineEditDoubleList[i]->text();

    // delete all the existing widgets in the layout
    if (m_boxcarGridSpacer)
    {
        m_boxcarGridLayout->removeItem(m_boxcarGridSpacer);
        delete m_boxcarGridSpacer;
    }
    for (auto it = m_boxcarLabelList.rbegin(); it != m_boxcarLabelList.rend(); it++)
    {
        m_boxcarGridLayout->removeWidget(*it);
        delete *it;
    }
    m_boxcarLabelList.clear();
    for (auto it = m_boxcarLineEditDoubleList.rbegin(); it != m_boxcarLineEditDoubleList.rend(); it++)
    {
        m_boxcarGridLayout->removeWidget(*it);
        delete *it;
    }
    m_boxcarLineEditDoubleList.clear();

    // now create a new set
    int stackSize = ui->spinBoxBoxcarStackSize->value();
    QLabel *label;
    for (int i = 0; i < stackSize; i++)
    {
        label = new QLabel();
        label->setText(QString("Delay %1").arg(i + 1));
        m_boxcarGridLayout->addWidget(label, i, 0, Qt::AlignTop);
        m_boxcarLabelList.push_back(label);
        LineEditDouble *lineEditDelay = new LineEditDouble();
        if (i < oldValues.size() / 3) lineEditDelay->setText(oldValues[i * 3 + 0]);
        m_boxcarGridLayout->addWidget(lineEditDelay, i, 1, Qt::AlignTop);
        m_boxcarLineEditDoubleList.push_back(lineEditDelay);
        label = new QLabel();
        label->setText(QString("Width %1").arg(i + 1));
        m_boxcarGridLayout->addWidget(label, i, 2, Qt::AlignTop);
        m_boxcarLabelList.push_back(label);
        LineEditDouble *lineEditWidth = new LineEditDouble();
        if (i < oldValues.size() / 3) lineEditWidth->setText(oldValues[i * 3 + 1]);
        m_boxcarGridLayout->addWidget(lineEditWidth, i, 3, Qt::AlignTop);
        m_boxcarLineEditDoubleList.push_back(lineEditWidth);
        label = new QLabel();
        label->setText(QString("Height %1").arg(i + 1));
        m_boxcarGridLayout->addWidget(label, i, 4, Qt::AlignTop);
        m_boxcarLabelList.push_back(label);
        LineEditDouble *lineEditHeight = new LineEditDouble();
        if (i < oldValues.size() / 3) lineEditHeight->setText(oldValues[i * 3 + 2]);
        m_boxcarGridLayout->addWidget(lineEditHeight, i, 5, Qt::AlignTop);
        m_boxcarLineEditDoubleList.push_back(lineEditHeight);
    }
    m_boxcarGridSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    m_boxcarGridLayout->addItem(m_boxcarGridSpacer, ui->spinBoxTargets->value(), 0);

    updateActivation();
}

void DialogDrivers::checkBoxChanged(int index)
{
    Q_UNUSED(index);
    updateActivation();
}


void DialogDrivers::updateActivation()
{
    bool okEnable = true;
    QString textCopy = ui->lineEditDriverID->text();
    int pos = ui->lineEditDriverID->cursorPosition();
    if (ui->lineEditDriverID->validator()->validate(textCopy, pos) != QValidator::Acceptable) okEnable = false;

    ui->pushButtonOK->setEnabled(okEnable);
}

Simulation *DialogDrivers::simulation() const
{
    return m_simulation;
}

void DialogDrivers::setSimulation(Simulation *simulation)
{
    m_simulation = simulation;
}

Driver *DialogDrivers::driver() const
{
    return m_driver;
}

void DialogDrivers::setDriver(Driver *driver)
{
    m_driver = driver;
}

