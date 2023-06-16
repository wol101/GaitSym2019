/*
 *  DialogPreferences.cpp
 *  GaitSym2019
 *
 *  Created by Bill Sellers on 12/02/2017.
 *  Copyright 2017 Bill Sellers. All rights reserved.
 *
 */

#include <QColorDialog>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QPushButton>
#include <QMultiMap>
#include <QScrollArea>
#include <QFileDialog>
#include <QMenu>
#include <QAction>
#include <QDebug>
#include <QMessageBox>
#include <QVector>
#include <QSpacerItem>
#include <QFont>
#include <QFontDialog>

#include "Preferences.h"
#include "LineEditDouble.h"
#include "LineEditPath.h"

#include "DialogPreferences.h"
#include "ui_DialogPreferences.h"

DialogPreferences::DialogPreferences(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPreferences)
{
    ui->setupUi(this);

    setWindowTitle(tr("Preferences"));
#ifdef Q_OS_MACOS
    // on MacOS High Sierra (and possibly others), the Qt::Dialog flag disables resizing
    // if I clear the Qt::Dialog flag and make sure the Qt::Window flag is set (it should be already)
    // then it seems to let me have resizing without any other side effects
    setWindowFlags(windowFlags() & (~Qt::Dialog) | Qt::Window);
#endif

    connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(acceptButtonClicked()));
    connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(rejectButtonClicked()));
    connect(ui->pushButtonImport, SIGNAL(clicked()), this, SLOT(importButtonClicked()));
    connect(ui->pushButtonExport, SIGNAL(clicked()), this, SLOT(exportButtonClicked()));
    connect(ui->pushButtonDefaults, SIGNAL(clicked()), this, SLOT(defaultsButtonClicked()));

    restoreGeometry(Preferences::valueQByteArray("DialogPreferencesGeometry"));
}

DialogPreferences::~DialogPreferences()
{
    delete ui;
}

void DialogPreferences::initialise()
{
    QStringList keys = Preferences::keys();
    keys.sort(Qt::CaseInsensitive);
    QVector<SettingsItem> intSettings;
    QVector<SettingsItem> fpSettings;
    QVector<SettingsItem> boolSettings;
    QVector<SettingsItem> stringSettings;
    QVector<SettingsItem> colourSettings;
    QVector<SettingsItem> fontSettings;
    QVector<SettingsItem> vector2DSettings;
    QVector<SettingsItem> vector3DSettings;
    for (int i = 0; i < keys.size(); i++)
    {
        SettingsItem item = Preferences::settingsItem(keys[i]);
        if (item.display && item.type == QMetaType::Int) intSettings.push_back(item);
        if (item.display && (item.type == QMetaType::Float || item.type == QMetaType::Double)) fpSettings.push_back(item);
        if (item.display && item.type == QMetaType::Bool) boolSettings.push_back(item);
        if (item.display && item.type == QMetaType::QString) stringSettings.push_back(item);
        if (item.display && item.type == QMetaType::QColor) colourSettings.push_back(item);
        if (item.display && item.type == QMetaType::QFont) fontSettings.push_back(item);
        if (item.display && item.type == QMetaType::QVector2D) vector2DSettings.push_back(item);
        if (item.display && item.type == QMetaType::QVector3D) vector3DSettings.push_back(item);
    }

    if (intSettings.size()) initialiseTab("Integers", intSettings);
    if (fpSettings.size()) initialiseTab("Floating Point", fpSettings);
    if (boolSettings.size()) initialiseTab("Booleans", boolSettings);
    if (stringSettings.size()) initialiseTab("Strings", stringSettings);
    if (colourSettings.size()) initialiseTab("Colours", colourSettings);
    if (fontSettings.size()) initialiseTab("Fonts", fontSettings);
    if (vector2DSettings.size()) initialiseTab("2D Vectors", vector2DSettings);
    if (vector3DSettings.size()) initialiseTab("3D Vectors", vector3DSettings);

}

void DialogPreferences::initialiseTab(const QString &tabName, const QVector<SettingsItem> &settingItems)
{
    QWidget *widget = new QWidget();
    widget->setObjectName(tabName);
    ui->tabWidget->addTab(widget, tabName);

    const QString COLOUR_STYLE("QPushButton { background-color : %1; color : %2; border: 4px solid %3; }");
    const QString FONT_STYLE("QPushButton { font: %1; font-size: %2; }");

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
            pushButton->setStyleSheet(COLOUR_STYLE.arg(color.name()).arg(getIdealTextColour(color).name()).arg( getAlphaColourHint(color).name()));
            pushButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            connect(pushButton, SIGNAL(clicked()), this, SLOT(colourButtonClicked()));
            gridLayout->addWidget(pushButton, row, 1);
            settingsWidget.widget = pushButton;
            m_SettingsWidgetList.append(settingsWidget);
        }

        if (type == QMetaType::QFont)
        {
            QPushButton *pushButton = new QPushButton();
            pushButton->setText("Font");
            QFont font = qvariant_cast<QFont>(item.value);
            pushButton->setText(font.family() + QString(" %1 point").arg(font.pointSize()));
            pushButton->setFont(font);
            pushButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            connect(pushButton, SIGNAL(clicked()), this, SLOT(fontButtonClicked()));
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

    qDebug() << "Preferences loaded from \"" << Preferences::fileName();

}

void DialogPreferences::update()
{
    for (int i = 0; i < m_SettingsWidgetList.size(); i++)
    {
        SettingsItem item = m_SettingsWidgetList[i].item;
        QWidget *widget = m_SettingsWidgetList[i].widget;

        QMetaType::Type type = static_cast<QMetaType::Type>(item.type);
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

        Preferences::insert(item);
    }
}

void DialogPreferences::colourButtonClicked()
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

void DialogPreferences::fontButtonClicked()
{
    int i;
    QPushButton *pushButton = dynamic_cast<QPushButton *>(QObject::sender());
    for (i = 0; i < m_SettingsWidgetList.size(); i++)
        if (m_SettingsWidgetList[i].widget == pushButton) break;
    if (i >= m_SettingsWidgetList.size()) return;

    QFont oldFont;
    oldFont.fromString(m_SettingsWidgetList[i].item.value.toString());
    bool ok;
    QFont newFont = QFontDialog::getFont(&ok, oldFont);
    if (ok)
    {
        pushButton->setFont(newFont);
        pushButton->setText(newFont.family() + QString(" %1 point").arg(newFont.pointSize()));
        m_SettingsWidgetList[i].item.value = newFont;
    }
}


// return an ideal label colour, based on the given background colour.
// Based on http://www.codeproject.com/cs/media/IdealTextColor.asp
QColor DialogPreferences::getIdealTextColour(const QColor &rBackgroundColour)
{
    const int THRESHOLD = 105;
    int BackgroundDelta = int((rBackgroundColour.red() * 0.299f) + (rBackgroundColour.green() * 0.587f) + (rBackgroundColour.blue() * 0.114f));
    return QColor((255 - BackgroundDelta < THRESHOLD) ? Qt::black : Qt::white);
}

QColor DialogPreferences::getAlphaColourHint(const QColor &colour)
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

void DialogPreferences::importButtonClicked()
{
    QString lastImportedFile = Preferences::valueQString(tr("LastImportedFile"));
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import Settings File"), lastImportedFile, tr("Exported Settings Files (*.xml);;Any File (*.* *)"), nullptr);
    if (fileName.isNull() == false)
    {
        Preferences::Import(fileName);
        Preferences::insert("LastImportedFile", fileName);
        for (int i = 0; i < m_SettingsWidgetList.size(); i++)
        {
            SettingsItem item = m_SettingsWidgetList[i].item;
            QWidget *widget = m_SettingsWidgetList[i].widget;
            QMetaType::Type type = static_cast<QMetaType::Type>(item.value.type());
            if (type == QMetaType::Int) dynamic_cast<QSpinBox *>(widget)->setValue(Preferences::valueInt(item.key));
            if (type == QMetaType::Double) dynamic_cast<QLineEdit *>(widget)->setText(Preferences::valueQString(item.key));
            if (type == QMetaType::QString) dynamic_cast<QLineEdit *>(widget)->setText( Preferences::valueQString(item.key));
            if (type == QMetaType::Bool) dynamic_cast<QCheckBox *>(widget)->setChecked(Preferences::valueBool(item.key));
            if (type == QMetaType::QVector2D)
            {
                QVector2D v = Preferences::valueQVector2D(item.key);
                LineEditDouble *x = dynamic_cast<LineEditDouble *>(widget->layout()->itemAt(0));
                LineEditDouble *y = dynamic_cast<LineEditDouble *>(widget->layout()->itemAt(1));
                x->setValue(double(v.x()));
                y->setValue(double(v.y()));
            }
            if (type == QMetaType::QVector3D)
            {
                QVector3D v = Preferences::valueQVector3D(item.key);
                LineEditDouble *x = dynamic_cast<LineEditDouble *>(widget->layout()->itemAt(0));
                LineEditDouble *y = dynamic_cast<LineEditDouble *>(widget->layout()->itemAt(1));
                LineEditDouble *z = dynamic_cast<LineEditDouble *>(widget->layout()->itemAt(2));
                x->setValue(double(v.x()));
                y->setValue(double(v.y()));
                z->setValue(double(v.z()));
            }
        }
    }
}

void DialogPreferences::exportButtonClicked()
{
    QString lastExportedFile = Preferences::valueQString(tr("LastExportedFile"));
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export Settings File"), lastExportedFile, tr("Exported Settings Files (*.xml)"), nullptr);
    if (fileName.isNull() == false)
    {
        Preferences::insert("LastExportedFile", fileName);
        Preferences::Export(fileName);
    }
}

void DialogPreferences::defaultsButtonClicked()
{
    QMessageBox msgBox;
    msgBox.setText("The values for all the preferences will be set to their defaults.");
    msgBox.setInformativeText("Click OK to proceed, and Cancel to abort this action.");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    int ret = msgBox.exec();
    if (ret == QMessageBox::Ok)
    {
        for (int i = 0; i < m_SettingsWidgetList.size(); i++)
        {
            Preferences::LoadDefaults();
            SettingsItem item = m_SettingsWidgetList[i].item;
            QWidget *widget = m_SettingsWidgetList[i].widget;
            QMetaType::Type type = static_cast<QMetaType::Type>(item.value.type());
            if (type == QMetaType::Int) dynamic_cast<QSpinBox *>(widget)->setValue(Preferences::valueInt(item.key));
            if (type == QMetaType::Double) dynamic_cast<LineEditDouble *>(widget)->setText(Preferences::valueQString(item.key));
            if (type == QMetaType::QString) dynamic_cast<QLineEdit *>(widget)->setText( Preferences::valueQString(item.key));
            if (type == QMetaType::Bool) dynamic_cast<QCheckBox *>(widget)->setChecked(Preferences::valueBool(item.key));
            if (type == QMetaType::QVector2D)
            {
                QVector2D v = Preferences::valueQVector2D(item.key);
                LineEditDouble *x = dynamic_cast<LineEditDouble *>(widget->layout()->itemAt(0));
                LineEditDouble *y = dynamic_cast<LineEditDouble *>(widget->layout()->itemAt(1));
                x->setValue(double(v.x()));
                y->setValue(double(v.y()));
            }
            if (type == QMetaType::QVector3D)
            {
                QVector3D v = Preferences::valueQVector3D(item.key);
                LineEditDouble *x = dynamic_cast<LineEditDouble *>(widget->layout()->itemAt(0));
                LineEditDouble *y = dynamic_cast<LineEditDouble *>(widget->layout()->itemAt(1));
                LineEditDouble *z = dynamic_cast<LineEditDouble *>(widget->layout()->itemAt(2));
                x->setValue(double(v.x()));
                y->setValue(double(v.y()));
                z->setValue(double(v.z()));
            }
        }
    }
}

void DialogPreferences::acceptButtonClicked()
{
    Preferences::insert("DialogPreferencesGeometry", saveGeometry());
    accept();
}

void DialogPreferences::rejectButtonClicked()
{
    Preferences::insert("DialogPreferencesGeometry", saveGeometry());
    reject();
}

void DialogPreferences::closeEvent(QCloseEvent *event)
{
    Preferences::insert("DialogPreferencesGeometry", saveGeometry());

    QDialog::closeEvent(event);
}

void DialogPreferences::menuRequestPath(QPoint pos)
{
    // this should always come from a QLineEdit
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(sender());
    if (lineEdit == nullptr) return;

    QMenu *menu = lineEdit->createStandardContextMenu();
    menu->addSeparator();
    menu->addAction(tr("Select Folder..."));


    QPoint gp = lineEdit->mapToGlobal(pos);

    QAction *action = menu->exec(gp);
    if (action)
    {
        if (action->text() == tr("Select Folder..."))
        {
            QString dir = QFileDialog::getExistingDirectory(this, "Select required folder", lineEdit->text());
            if (!dir.isEmpty())
            {
                lineEdit->setText(dir);
            }
        }
    }
    delete menu;
}

