/*
 *  DialogProperties.cpp
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 16/03/2020.
 *  Copyright 2017 Bill Sellers. All rights reserved.
 *
 */

#include <QScrollArea>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QColorDialog>

#include "Preferences.h"
#include "LineEditDouble.h"

#include "DialogProperties.h"
#include "ui_DialogProperties.h"

DialogProperties::DialogProperties(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogProperties)
{
    ui->setupUi(this);

    setWindowTitle(tr("Object Parameters"));
#ifdef Q_OS_MACOS
    // on MacOS High Sierra (and possibly others), the Qt::Dialog flag disables resizing
    // if I clear the Qt::Dialog flag and make sure the Qt::Window flag is set (it should be already)
    // then it seems to let me have resizing without any other side effects
    setWindowFlags(windowFlags() & (~Qt::Dialog) | Qt::Window);
#endif

    connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(acceptButtonClicked()));
    connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(rejectButtonClicked()));

    restoreGeometry(Preferences::valueQByteArray("DialogPropertiesGeometry"));

}

DialogProperties::~DialogProperties()
{
    delete ui;
}

void DialogProperties::initialise()
{
    QVector<SettingsItem> intSettings;
    QVector<SettingsItem> fpSettings;
    QVector<SettingsItem> boolSettings;
    QVector<SettingsItem> stringSettings;
    QVector<SettingsItem> colourSettings;
    QVector<SettingsItem> vector2DSettings;
    QVector<SettingsItem> vector3DSettings;
    for (auto &&item : m_inputSettingsItems)
    {
        if (item.display && item.type == QMetaType::Int) intSettings.push_back(item);
        if (item.display && (item.type == QMetaType::Float || item.type == QMetaType::Double)) fpSettings.push_back(item);
        if (item.display && item.type == QMetaType::Bool) boolSettings.push_back(item);
        if (item.display && item.type == QMetaType::QString) stringSettings.push_back(item);
        if (item.display && item.type == QMetaType::QColor) colourSettings.push_back(item);
        if (item.display && item.type == QMetaType::QVector2D) vector2DSettings.push_back(item);
        if (item.display && item.type == QMetaType::QVector3D) vector3DSettings.push_back(item);
    }

    if (intSettings.size()) initialiseTab("Integers", intSettings);
    if (fpSettings.size()) initialiseTab("Floating Point", fpSettings);
    if (boolSettings.size()) initialiseTab("Booleans", boolSettings);
    if (stringSettings.size()) initialiseTab("Strings", stringSettings);
    if (colourSettings.size()) initialiseTab("Colours", colourSettings);
    if (vector2DSettings.size()) initialiseTab("2D Vectors", vector2DSettings);
    if (vector3DSettings.size()) initialiseTab("3D Vectors", vector3DSettings);

}

void DialogProperties::initialiseTab(const QString &tabName, const QVector<SettingsItem> &settingItems)
{
    QWidget *widget = new QWidget();
    widget->setObjectName(tabName);
    ui->tabWidget->addTab(widget, tabName);

    const QString COLOUR_STYLE("QPushButton { background-color : %1; color : %2; border: 4px solid %3; }");
    QVBoxLayout *verticalLayout;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QGridLayout *gridLayout;

    verticalLayout = new QVBoxLayout(widget);
    verticalLayout->setSpacing(6);
    verticalLayout->setContentsMargins(9, 9, 9, 9);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    scrollArea = new QScrollArea(this);
    scrollArea->setObjectName(QStringLiteral("scrollArea"));
    scrollArea->setWidgetResizable(true);
    scrollAreaWidgetContents = new QWidget();
    scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
    // scrollAreaWidgetContents->setGeometry(QRect(0, 0, 245, 154));
    gridLayout = new QGridLayout(scrollAreaWidgetContents);
    gridLayout->setSpacing(6);
    gridLayout->setContentsMargins(9, 9, 9, 9);
    gridLayout->setObjectName(QStringLiteral("gridLayout"));


    int row = 0;
    SettingsWidget settingsWidget;
    for (QVector<SettingsItem>::const_iterator i = settingItems.constBegin(); i != settingItems.constEnd(); i++)
    {
        SettingsItem item = *i;

        QLabel *label = new QLabel();
        label->setText(item.label);
        gridLayout->addWidget(label, row, 0);

        settingsWidget.item = item;
        QMetaType::Type type = item.type;
        if (type == QMetaType::Int)
        {
            QSpinBox *entryWidget = new QSpinBox();
            entryWidget->setMinimumWidth(200);
            entryWidget->setRange(item.minimumValue.toInt(), item.maximumValue.toInt());
            entryWidget->setValue(item.value.toInt());
            gridLayout->addWidget(entryWidget, row, 1);
            settingsWidget.widget = entryWidget;
            m_SettingsWidgetList.append(settingsWidget);
        }

        if (type == QMetaType::Double)
        {
            LineEditDouble *entryWidget = new LineEditDouble();
            entryWidget->setMinimumWidth(200);
            entryWidget->setBottom(item.minimumValue.toDouble());
            entryWidget->setTop(item.maximumValue.toDouble());
            entryWidget->setValue(item.value.toDouble());
            gridLayout->addWidget(entryWidget, row, 1);
            settingsWidget.widget = entryWidget;
            m_SettingsWidgetList.append(settingsWidget);
        }

        if (type == QMetaType::Float)
        {
            LineEditDouble *entryWidget = new LineEditDouble();
            entryWidget->setMinimumWidth(200);
            entryWidget->setBottom(item.minimumValue.toDouble());
            entryWidget->setTop(item.maximumValue.toDouble());
            entryWidget->setValue(item.value.toDouble());
            gridLayout->addWidget(entryWidget, row, 1);
            settingsWidget.widget = entryWidget;
            m_SettingsWidgetList.append(settingsWidget);
        }

        if (type == QMetaType::QString)
        {
            QLineEdit *entryWidget = new QLineEdit();
            entryWidget->setMinimumWidth(200);
            entryWidget->setText(QString("%1").arg(item.value.toString()));
            gridLayout->addWidget(entryWidget, row, 1);
            settingsWidget.widget = entryWidget;
            m_SettingsWidgetList.append(settingsWidget);
            // this adds an extra context menu to the line edit
            entryWidget->setContextMenuPolicy(Qt::CustomContextMenu);
            QObject::connect(entryWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(menuRequestPath(QPoint)));
        }

        if (type == QMetaType::Bool)
        {
            QCheckBox *checkBox = new QCheckBox();
            checkBox->setChecked(item.value.toBool());
            checkBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            gridLayout->addWidget(checkBox, row, 1);
//            QSpacerItem *horizSpacer = new QSpacerItem(20, 40, QSizePolicy::Expanding, QSizePolicy::Minimum);
//            gridLayout->addItem(horizSpacer, row, 2);
            settingsWidget.widget = checkBox;
            m_SettingsWidgetList.append(settingsWidget);
        }

        if (type == QMetaType::QColor)
        {
            QPushButton *pushButton = new QPushButton();
            pushButton->setText("Colour");
            QColor color = qvariant_cast<QColor>(item.value);
            pushButton->setStyleSheet(COLOUR_STYLE.arg(color.name()).arg(getIdealTextColour(color).name()).arg(getAlphaColourHint(color).name()));
            pushButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            connect(pushButton, SIGNAL(clicked()), this, SLOT(colourButtonClicked()));
            gridLayout->addWidget(pushButton, row, 1);
            settingsWidget.widget = pushButton;
            m_SettingsWidgetList.append(settingsWidget);
        }

        if (type == QMetaType::QVector2D)
        {
            QVector2D v = qvariant_cast<QVector2D>(item.value);
            QWidget *nestedWidget = new QWidget();
            nestedWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            gridLayout->addWidget(nestedWidget, row, 1);
            QGridLayout *nestedGridLayout = new QGridLayout(nestedWidget);
            nestedGridLayout->setSpacing(6);
            nestedGridLayout->setContentsMargins(0, 0, 0, 0);
            LineEditDouble *x = new LineEditDouble();
            x->setValue(double(v.x()));
            nestedGridLayout->addWidget(x, 0, 0);
            LineEditDouble *y = new LineEditDouble();
            y->setValue(double(v.y()));
            nestedGridLayout->addWidget(y, 1, 0);
            settingsWidget.widget = nestedWidget;
            m_SettingsWidgetList.append(settingsWidget);

        }

        if (type == QMetaType::QVector3D)
        {
            QVector3D v = qvariant_cast<QVector3D>(item.value);
            QWidget *nestedWidget = new QWidget();
            nestedWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            gridLayout->addWidget(nestedWidget, row, 1);
            QGridLayout *nestedGridLayout = new QGridLayout(nestedWidget);
            nestedGridLayout->setSpacing(6);
            nestedGridLayout->setContentsMargins(0, 0, 0, 0);
            LineEditDouble *x = new LineEditDouble();
            x->setValue(double(v.x()));
            nestedGridLayout->addWidget(x, 0, 0);
            LineEditDouble *y = new LineEditDouble();
            y->setValue(double(v.y()));
            nestedGridLayout->addWidget(y, 1, 0);
            LineEditDouble *z = new LineEditDouble();
            z->setValue(double(v.z()));
            nestedGridLayout->addWidget(z, 2, 0);
            settingsWidget.widget = nestedWidget;
            m_SettingsWidgetList.append(settingsWidget);
        }

        row++;

    }

    QSpacerItem *vertSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    gridLayout->addItem(vertSpacer, row, 0);

    scrollArea->setWidget(scrollAreaWidgetContents);
    verticalLayout->addWidget(scrollArea);
}

void DialogProperties::setInputSettingsItems(QMap<QString, SettingsItem> &inputSettingsItems)
{
    m_inputSettingsItems = inputSettingsItems;
}

QMap<QString, SettingsItem> DialogProperties::getOutputSettingsItems() const
{
    return m_outputSettingsItems;
}

void DialogProperties::colourButtonClicked()
{
    const QString COLOUR_STYLE("QPushButton { background-color : %1; color : %2; border: 4px solid %3; }");
    int i;
    QPushButton *pushButton = dynamic_cast<QPushButton *>(QObject::sender());
    for (i = 0; i < m_SettingsWidgetList.size(); i++)
        if (m_SettingsWidgetList[i].widget == pushButton) break;
    if (i >= m_SettingsWidgetList.size()) return;

    QColor colour = QColorDialog::getColor(qvariant_cast<QColor>(m_SettingsWidgetList[i].item.value), this, "Select Color", QColorDialog::ShowAlphaChannel /*| QColorDialog::DontUseNativeDialog*/);
    if (colour.isValid())
    {
        pushButton->setStyleSheet(COLOUR_STYLE.arg(colour.name()).arg(getIdealTextColour(colour).name()).arg(getAlphaColourHint(colour).name()));
        m_SettingsWidgetList[i].item.value = colour;
    }
}


// return an ideal label colour, based on the given background colour.
// Based on http://www.codeproject.com/cs/media/IdealTextColor.asp
QColor DialogProperties::getIdealTextColour(const QColor &rBackgroundColour)
{
    const int THRESHOLD = 105;
    int BackgroundDelta = int((rBackgroundColour.red() * 0.299f) + (rBackgroundColour.green() * 0.587f) + (rBackgroundColour.blue() * 0.114f));
    return QColor((255 - BackgroundDelta < THRESHOLD) ? Qt::black : Qt::white);
}

QColor DialogProperties::getAlphaColourHint(const QColor &colour)
{
    // (source * Blend.SourceAlpha) + (background * Blend.InvSourceAlpha)
    QColor background;
    background.setRgbF(1.0, 1.0, 1.0);
    QColor hint;
    hint.setRedF((colour.redF() * colour.alphaF()) + (background.redF() * (1 - colour.alphaF())));
    hint.setGreenF((colour.greenF() * colour.alphaF()) + (background.greenF() * (1 - colour.alphaF())));
    hint.setBlueF((colour.blueF() * colour.alphaF()) + (background.blueF() * (1 - colour.alphaF())));
    return hint;
}

void DialogProperties::acceptButtonClicked()
{
    Preferences::insert("DialogPropertiesGeometry", saveGeometry());
    accept();
}

void DialogProperties::rejectButtonClicked()
{
    Preferences::insert("DialogPropertiesGeometry", saveGeometry());
    reject();
}

void DialogProperties::closeEvent(QCloseEvent *event)
{
    Preferences::insert("DialogPropertiesGeometry", saveGeometry());

    QDialog::closeEvent(event);
}

void DialogProperties::update()
{
    m_outputSettingsItems.clear();

    for (int i = 0; i < m_SettingsWidgetList.size(); i++)
    {
        SettingsItem item = m_SettingsWidgetList[i].item;
        QWidget *widget = m_SettingsWidgetList[i].widget;

        QMetaType::Type type = static_cast<QMetaType::Type>(item.value.type());
        if (type == QMetaType::Int) item.value = dynamic_cast<QSpinBox *>(widget)->value();
        if (type == QMetaType::Double) item.value = dynamic_cast<LineEditDouble *>(widget)->value();
        if (type == QMetaType::Float) item.value = float(dynamic_cast<LineEditDouble *>(widget)->value());
        if (type == QMetaType::QString) item.value = dynamic_cast<QLineEdit *>(widget)->text();
        if (type == QMetaType::Bool) item.value = dynamic_cast<QCheckBox *>(widget)->isChecked();

        if (type == QMetaType::QVector2D)
        {
            QVector2D v;
            LineEditDouble *x = dynamic_cast<LineEditDouble *>(widget->layout()->itemAt(0));
            LineEditDouble *y = dynamic_cast<LineEditDouble *>(widget->layout()->itemAt(1));
            v.setX(float(x->value()));
            v.setY(float(y->value()));
            item.value = v;
        }

        if (type == QMetaType::QVector3D)
        {
            QVector3D v;
            LineEditDouble *x = dynamic_cast<LineEditDouble *>(widget->layout()->itemAt(0));
            LineEditDouble *y = dynamic_cast<LineEditDouble *>(widget->layout()->itemAt(1));
            LineEditDouble *z = dynamic_cast<LineEditDouble *>(widget->layout()->itemAt(2));
            v.setX(float(x->value()));
            v.setY(float(y->value()));
            v.setZ(float(z->value()));
            item.value = v;
        }

        m_outputSettingsItems[item.key] = item;
    }
}
