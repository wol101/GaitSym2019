#include "DialogCreateTestingDrivers.h"
#include "ui_DialogCreateTestingDrivers.h"

#include "Preferences.h"
#include "BasicXMLSyntaxHighlighter.h"
#include "Simulation.h"

#include "pystring.h"

#include <QPlainTextEdit>
#include <QMessageBox>
#include <QCheckBox>
#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

#include <string>
#include <vector>
#include <limits>

using namespace std::literals::string_literals;

DialogCreateTestingDrivers::DialogCreateTestingDrivers(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogCreateTestingDrivers)
{
    ui->setupUi(this);
    setWindowTitle(tr("Create Testing Drivers"));
#ifdef Q_OS_MACOS
    setWindowFlags(windowFlags() & (~Qt::Dialog) | Qt::Window); // allows the window to be resized on macs
#endif

    loadPreferences();

    connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->pushButtonApply, SIGNAL(clicked()), this, SLOT(apply()));
    connect(ui->lineEditSuffix, SIGNAL(textChanged(const QString &)), this, SLOT(textChanged(const QString &)));
    connect(ui->lineEditActivationTime, SIGNAL(textChanged(const QString &)), this, SLOT(textChanged(const QString &)));
    connect(ui->lineEditActivationValue, SIGNAL(textChanged(const QString &)), this, SLOT(textChanged(const QString &)));
    connect(ui->plainTextEdit, SIGNAL(modificationChanged(bool)), this, SLOT(modificationChanged(bool)));

    ui->lineEditActivationTime->setBottom(std::numeric_limits<double>::epsilon());
    ui->lineEditActivationValue->setBottom(0);
    ui->lineEditActivationValue->setTop(1.0);
    QRegularExpression rx("[0-9A-Za-z_]+");
    QValidator *validator = new QRegularExpressionValidator(rx, this);
    ui->lineEditSuffix->setValidator(validator);

    setEditorFonts();
    enableControls();
}

DialogCreateTestingDrivers::~DialogCreateTestingDrivers()
{
    delete ui;
}

void DialogCreateTestingDrivers::accept()
{
    qDebug() << "DialogCreateTestingDrivers::accept()";

    std::string *errorMessage = validate();
    if (errorMessage)
    {
        QMessageBox::warning(this, "Error validating XML file", QString("Error message:\n%1\nSuggest either fix error or Cancel").arg(QString::fromStdString(*errorMessage)));
        return;
    }

    savePreferences();
    QDialog::accept();
}

void DialogCreateTestingDrivers::reject() // this catches cancel, close and escape key
{
    qDebug() << "DialogCreateTestingDrivers::reject()";

    if (isModified())
    {
        QMessageBox msgBox;
        msgBox.setText("The document has been modified.");
        msgBox.setInformativeText("Click OK to quit, and Cancel to continue working on the document");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Cancel) return;
    }

    savePreferences();
    QDialog::reject();
}

void DialogCreateTestingDrivers::loadPreferences()
{
    m_editorFont = Preferences::valueQFont("DialogCreateTestingDriversEditorFont");
    ui->lineEditSuffix->setText(Preferences::valueQString("DialogCreateTestingDriversSuffix"));
    ui->lineEditActivationTime->setValue(Preferences::valueDouble("DialogCreateTestingDriversActivationTime"));
    ui->lineEditActivationValue->setValue(Preferences::valueDouble("DialogCreateTestingDriversActivationValue"));
    restoreGeometry(Preferences::valueQByteArray("DialogCreateTestingDriversGeometry"));
}

void DialogCreateTestingDrivers::savePreferences()
{
    Preferences::insert("DialogCreateTestingDriversEditorFont", m_editorFont);
    Preferences::insert("DialogCreateTestingDriversSuffix", ui->lineEditSuffix->text());
    Preferences::insert("DialogCreateTestingDriversActivationTime", ui->lineEditActivationTime->value());
    Preferences::insert("DialogCreateTestingDriversActivationValue", ui->lineEditActivationValue->value());
    Preferences::insert("DialogCreateTestingDriversGeometry", saveGeometry());
}

QString DialogCreateTestingDrivers::editorText() const
{
    return ui->plainTextEdit->toPlainText();
}

void DialogCreateTestingDrivers::setEditorText(const QString &editorText)
{
    ui->plainTextEdit->setPlainText(editorText);
}

void DialogCreateTestingDrivers::useXMLSyntaxHighlighter()
{
    m_basicXMLSyntaxHighlighter = new BasicXMLSyntaxHighlighter(ui->plainTextEdit->document());
}

void DialogCreateTestingDrivers::setEditorFonts()
{
    QList<QLineEdit *> listQLineEdit = this->findChildren<QLineEdit *>(QString(), Qt::FindChildrenRecursively);
    for (QList<QLineEdit *>::iterator it = listQLineEdit.begin(); it != listQLineEdit.end(); it++) (*it)->setFont(m_editorFont);

    QList<QPlainTextEdit *> listQPlainTextEdit = this->findChildren<QPlainTextEdit *>(QString(), Qt::FindChildrenRecursively);
    for (QList<QPlainTextEdit *>::iterator it = listQPlainTextEdit.begin(); it != listQPlainTextEdit.end(); it++) (*it)->setFont(m_editorFont);
}

void DialogCreateTestingDrivers::enableControls()
{
    ui->pushButtonApply->setEnabled(ui->lineEditActivationTime->hasAcceptableInput() &&
                                    ui->lineEditActivationValue->hasAcceptableInput() &&
                                    ui->lineEditSuffix->hasAcceptableInput());
    ui->pushButtonOK->setEnabled(isModified());
}

std::string *DialogCreateTestingDrivers::validate()
{
    Simulation simulation;
    QByteArray editFileData = ui->plainTextEdit->toPlainText().toUtf8();
    std::string *errorMessage = simulation.LoadModel(editFileData.constData(), editFileData.size());
    if (errorMessage)
    {
        m_lastError = *errorMessage;
        return &m_lastError;
    }
    return nullptr;
}

void DialogCreateTestingDrivers::textChanged(const QString & /*text*/)
{
    enableControls();
}

bool DialogCreateTestingDrivers::isModified() const
{
    return ui->plainTextEdit->document()->isModified();
}

void DialogCreateTestingDrivers::setModified(bool modified)
{
    ui->plainTextEdit->document()->setModified(modified);
    enableControls();
}

void DialogCreateTestingDrivers::apply()
{
    bool localModified = isModified();
    std::string *lastError;
    std::string xml = ui->plainTextEdit->toPlainText().toStdString();
    std::string rootNodeTag = "GAITSYM2019"s;
    lastError = m_parseXML.LoadModel(xml.c_str(), xml.size(), rootNodeTag);
    if (lastError)
    {
        QMessageBox::warning(this, "XML parse error", QString("'%1'").arg(QString::fromStdString(*lastError)));
        return;
    }
    std::string suffix = ui->lineEditSuffix->text().toStdString();
    double activationTime = ui->lineEditActivationTime->value();
    double activationValue = ui->lineEditActivationValue->value();

    applyCreateTestingDrivers(activationTime, activationValue, suffix);

    std::string newXML = m_parseXML.SaveModel("GAITSYM2019"s, "Created from DialogCreateTestingDrivers::apply"s);
    ui->plainTextEdit->setPlainText(QString::fromStdString(newXML));
    if (localModified || (xml != newXML)) setModified(true);
}

void DialogCreateTestingDrivers::applyCreateTestingDrivers(double activationTime, double activationValue, const std::string &suffix)
{
    std::vector<std::unique_ptr<ParseXML::XMLElement>> newElements;
    std::vector<std::unique_ptr<ParseXML::XMLElement>> *elementList = m_parseXML.elementList();
    // get the muscles first
    std::vector<std::string> muscleNames;
    for (auto &&tagElementIt : *elementList)
    {
        if (tagElementIt->tag == "MUSCLE"s)
        {
            auto findID = tagElementIt->attributes.find("ID"s);
            if (findID != tagElementIt->attributes.end())
            {
                muscleNames.push_back(findID->second);
            }
        }
    }
    // now create the new DRIVERS
    for (size_t i = 0; i < muscleNames.size(); i++)
    {
        std::unique_ptr<ParseXML::XMLElement> newElement = std::make_unique<ParseXML::XMLElement>();
        newElement->tag = "DRIVER"s;
        newElement->attributes["ID"s] = muscleNames[i] + suffix;
        newElement->attributes["TargetIDList"s] = muscleNames[i];
        newElement->attributes["Type"s] = "StackedBoxcar"s;
        newElement->attributes["StackSize"s] = "1"s;
        newElement->attributes["CycleTime"s] = std::to_string(double(muscleNames.size() - 1) * activationTime);
        newElement->attributes["Widths"s] = std::to_string(1.0 / double(muscleNames.size() - 1));
        newElement->attributes["Delays"s] = std::to_string(double(i) / double(muscleNames.size() - 1));
        newElement->attributes["Heights"s] = std::to_string(activationValue);
        newElement->attributes["LinearInterpolation"s] = "false"s;
        newElement->attributes["DriverRange"s] = "0 1"s;
        newElement->attributes["Group"s] = "TestingDriver"s;
        newElement->attributes["Colour1"s] = "0 0 0 255"s;
        newElement->attributes["Colour2"s] = "0 0 0 255"s;
        newElement->attributes["Colour3"s] = "0 0 0 255"s;
        newElement->attributes["Size1"s] = "0"s;
        newElement->attributes["Size2"s] = "0"s;
        newElement->attributes["Size3"s] = "0"s;
        newElements.push_back(std::move(newElement));
    }

    // and set the run time
    for (auto &&tagElementIt : *elementList)
    {
        if (tagElementIt->tag == "GLOBAL"s)
        {
            auto timeLimit = tagElementIt->attributes.find("TimeLimit"s);
            if (timeLimit != tagElementIt->attributes.end())
            {
                timeLimit->second = std::to_string(double(muscleNames.size() - 1) * activationTime);
            }
        }
    }
    // and add them to m_parseXML
    for (auto &&newElementIt : newElements)
    {
        elementList->push_back(std::move(newElementIt));
    }
}

void DialogCreateTestingDrivers::modificationChanged(bool /*changed*/)
{
    enableControls();
}



