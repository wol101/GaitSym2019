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
#include <QDebug>

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

    connect(ui->spinBoxTargets, SIGNAL(valueChanged(int)), this, SLOT(spinBoxChangedTargets(int)));
    connect(ui->spinBoxSteps, SIGNAL(valueChanged(int)), this, SLOT(spinBoxChangedSteps(int)));
    connect(ui->spinBoxStepsPerCycle, SIGNAL(valueChanged(int)), this, SLOT(spinBoxChangedStepsPerCycle(int)));
    connect(ui->spinBoxBoxcarStackSize, SIGNAL(valueChanged(int)), this, SLOT(spinBoxChangedBoxcarStackSize(int)));

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

    QString tab = ui->tabWidget->tabText(ui->tabWidget->currentIndex());
    if (tab == "Fixed")
    {
        std::unique_ptr<FixedDriver> driver = std::make_unique<FixedDriver>();
        driver->setValue(ui->lineEditFixedValue->value());
        m_outputDriver = std::move(driver);
    }
    else if (tab == "Step")
    {
        std::unique_ptr<StepDriver> driver = std::make_unique<StepDriver>();
        size_t steps = static_cast<size_t>(ui->spinBoxSteps->value());
        std::vector<double> durations;
        durations.reserve(steps);
        std::vector<double> values;
        values.reserve(steps);
        for (int i = 0; i < ui->spinBoxSteps->value(); i++)
        {
            durations.push_back(ui->tableWidgetStep->item(i, 0)->text().toDouble());
            values.push_back(ui->tableWidgetStep->item(i, 1)->text().toDouble());
        }
        driver->setDurationList(durations);
        driver->setValueList(values);
        m_outputDriver = std::move(driver);
    }

    else if (tab == "Cyclic")
    {
        std::unique_ptr<CyclicDriver> driver = std::make_unique<CyclicDriver>();
        size_t steps = static_cast<size_t>(ui->spinBoxStepsPerCycle->value());
        std::vector<double> durations;
        durations.reserve(steps);
        std::vector<double> values;
        values.reserve(steps);
        for (int i = 0; i < ui->spinBoxStepsPerCycle->value(); i++)
        {
            durations.push_back(ui->tableWidgetCyclic->item(i, 0)->text().toDouble());
            values.push_back(ui->tableWidgetCyclic->item(i, 1)->text().toDouble());
        }
        driver->setDurationList(durations);
        driver->setValueList(values);
        m_outputDriver = std::move(driver);
    }

    else if (tab == "Boxcar")
    {
        std::unique_ptr<StackedBoxcarDriver> driver = std::make_unique<StackedBoxcarDriver>();
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
        m_outputDriver = std::move(driver);
    }

    m_outputDriver->setSimulation(m_simulation);
    m_outputDriver->setName(ui->lineEditDriverID->text().toStdString());
    m_outputDriver->setMinValue(ui->lineEditMinimum->value());
    m_outputDriver->setMaxValue(ui->lineEditMaximum->value());
    m_outputDriver->setInterp(ui->checkBoxInterpolate->isChecked());
    for (int i = 0; i < m_targetComboBoxList.size(); i++)
    {
        std::string name = m_targetComboBoxList[i]->currentText().toStdString();
        Muscle *muscle = m_simulation->GetMuscle(name);
        if (muscle) { m_outputDriver->AddTarget(muscle); continue; }
        Controller *controller = m_simulation->GetController(name);
        if (controller) { m_outputDriver->AddTarget(controller); continue; }
    }

    m_outputDriver->saveToAttributes();
    m_outputDriver->createFromAttributes();

    Preferences::insert("DialogDriversGeometry", saveGeometry());
    QDialog::accept();
}

void DialogDrivers::reject() // this catches cancel, close and escape key
{
    qDebug() << "DialogDrivers::reject()";
    Preferences::insert("DialogDriversGeometry", saveGeometry());
    QDialog::reject();
}

void DialogDrivers::lateInitialise()
{
    Q_ASSERT_X(m_simulation, "DialogDrivers::lateInitialise", "m_simulation undefined");

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
    ui->spinBoxTargets->setValue(0);

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

    if (!m_inputDriver)
    {
        auto nameSet = m_simulation->GetNameSet();
        ui->lineEditDriverID->addStrings(nameSet);
        int initialNameCount = 0;
        QString initialName = QString("Driver%1").arg(initialNameCount, 3, 10, QLatin1Char('0'));
        while (nameSet.count(initialName.toStdString()))
        {
            initialNameCount++;
            initialName = QString("Driver%1").arg(initialNameCount, 3, 10, QLatin1Char('0'));
            if (initialNameCount >= 999) break; // only do this for the first 999 markers
        }
        ui->lineEditDriverID->setText(initialName);
        spinBoxChangedTargets(1);
        spinBoxChangedBoxcarStackSize(1);
        return;
    }

    ui->lineEditDriverID->setText(QString::fromStdString(m_inputDriver->name()));
    ui->lineEditDriverID->setEnabled(false);

    m_inputDriver->saveToAttributes();
    std::string s = m_inputDriver->findAttribute("TargetIDList"s);
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
    m_targetGridLayout->addItem(m_targetGridSpacer, int(targetNames.size()), 0);
    ui->spinBoxTargets->setValue(int(targetNames.size()));

    ui->lineEditMinimum->setValue(m_inputDriver->MinValue());
    ui->lineEditMaximum->setValue(m_inputDriver->MaxValue());
    ui->checkBoxInterpolate->setChecked(m_inputDriver->Interp());

    while (true)
    {
        FixedDriver *fixedDriver = dynamic_cast<FixedDriver *>(m_inputDriver);
        if (fixedDriver)
        {
            if ((s = fixedDriver->findAttribute("Value"s)).size()) ui->lineEditFixedValue->setValue(GSUtil::Double(s));
            ui->tabWidget->setCurrentIndex(tabNames.indexOf("Fixed"));
            spinBoxChangedBoxcarStackSize(1);
            break;
        }

        StepDriver *stepDriver = dynamic_cast<StepDriver *>(m_inputDriver);
        if (stepDriver)
        {
            std::vector<double> valueList = stepDriver->valueList();
            std::vector<double> durationList = stepDriver->durationList();
            int steps = int(std::min(valueList.size(), durationList.size()));
            ui->spinBoxSteps->setValue(steps);
            ui->tableWidgetStep->setRowCount(steps);
            ui->tableWidgetStep->setColumnCount(2);
            for (int i = 0; i < steps; i++)
            {
                ui->tableWidgetStep->setItem(i, 0, new QTableWidgetItem(QString("%1").arg(durationList[i])));
                ui->tableWidgetStep->setItem(i, 1, new QTableWidgetItem(QString("%1").arg(valueList[i])));
            }
            ui->tabWidget->setCurrentIndex(tabNames.indexOf("Step"));
            spinBoxChangedBoxcarStackSize(1);
            break;
        }

        CyclicDriver *cyclicDriver = dynamic_cast<CyclicDriver *>(m_inputDriver);
        if (cyclicDriver)
        {
            std::vector<double> valueList = cyclicDriver->valueList();
            std::vector<double> durationList = cyclicDriver->durationList();
            int steps = int(std::min(valueList.size(), durationList.size()));
            ui->spinBoxStepsPerCycle->setValue(steps);
            ui->tableWidgetCyclic->setRowCount(steps);
            ui->tableWidgetCyclic->setColumnCount(2);
            for (int i = 0; i < steps; i++)
            {
                ui->tableWidgetCyclic->setItem(i, 0, new QTableWidgetItem(QString("%1").arg(durationList[i])));
                ui->tableWidgetCyclic->setItem(i, 1, new QTableWidgetItem(QString("%1").arg(valueList[i])));
            }
            ui->tabWidget->setCurrentIndex(tabNames.indexOf("Cyclic"));
            spinBoxChangedBoxcarStackSize(1);
            break;
        }

        StackedBoxcarDriver *stackedBoxcarDriver = dynamic_cast<StackedBoxcarDriver *>(m_inputDriver);
        if (stackedBoxcarDriver)
        {
            int stackSize = GSUtil::Int(stackedBoxcarDriver->findAttribute("StackSize"s));
            std::vector<double> delays(static_cast<size_t>(stackSize));
            std::vector<double> widths(static_cast<size_t>(stackSize));
            std::vector<double> heights(static_cast<size_t>(stackSize));
            GSUtil::Double(stackedBoxcarDriver->findAttribute("Delays"s), stackSize, delays.data());
            GSUtil::Double(stackedBoxcarDriver->findAttribute("Widths"s), stackSize, widths.data());
            GSUtil::Double(stackedBoxcarDriver->findAttribute("Heights"s), stackSize, heights.data());
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
            m_boxcarGridLayout->addItem(m_boxcarGridSpacer, stackSize, 0);

            ui->spinBoxBoxcarStackSize->setValue(stackSize);
            ui->lineEditBoxcarCycleTime->setValue(GSUtil::Double(stackedBoxcarDriver->findAttribute("CycleTime"s)));
            ui->tabWidget->setCurrentIndex(tabNames.indexOf("Boxcar"));
            break;
        }
        qDebug() << "Unsupported DRIVER";
        break;
    }
}

void DialogDrivers::tabChanged(int /* index */)
{
    updateActivation();
}

void DialogDrivers::comboBoxChanged(int /* index */)
{
    updateActivation();
}

void DialogDrivers::lineEditChanged(const QString & /* text */)
{
    updateActivation();
}

void DialogDrivers::spinBoxChangedTargets(int /* value */)
{
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
void DialogDrivers::spinBoxChangedSteps(int value)
{
    if (ui->tableWidgetStep->rowCount() > value)
    {
        ui->tableWidgetStep->setRowCount(value);
    }
    else
    {
        ui->tableWidgetStep->setRowCount(value);
        ui->tableWidgetStep->setItem(value - 1, 0, new QTableWidgetItem(QString("1")));
        ui->tableWidgetStep->setItem(value - 1, 1, new QTableWidgetItem(QString("0")));
    }
    updateActivation();
}

void DialogDrivers::spinBoxChangedStepsPerCycle(int value)
{
    if (ui->tableWidgetCyclic->rowCount() > value)
    {
        ui->tableWidgetCyclic->setRowCount(value);
    }
    else
    {
        ui->tableWidgetCyclic->setRowCount(value);
        ui->tableWidgetCyclic->setItem(value - 1, 0, new QTableWidgetItem(QString("1")));
        ui->tableWidgetCyclic->setItem(value - 1, 1, new QTableWidgetItem(QString("0")));
    }
    updateActivation();
}

void DialogDrivers::spinBoxChangedBoxcarStackSize(int /* value */)
{
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

void DialogDrivers::checkBoxChanged(int /* index */)
{
    updateActivation();
}


void DialogDrivers::updateActivation()
{
    bool okEnable = true;
    QString textCopy = ui->lineEditDriverID->text();
    int pos = ui->lineEditDriverID->cursorPosition();
    if (ui->lineEditDriverID->validator()->validate(textCopy, pos) != QValidator::Acceptable) okEnable = false;
    if (ui->spinBoxTargets->value() < 1) okEnable = false;
    if (ui->lineEditMinimum->value() >= ui->lineEditMaximum->value()) okEnable = false;

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

void DialogDrivers::setInputDriver(Driver *inputDriver)
{
    m_inputDriver = inputDriver;
}

std::unique_ptr<Driver> DialogDrivers::outputDriver()
{
    return std::move(m_outputDriver);
}




