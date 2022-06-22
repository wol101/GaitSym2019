#include "DialogRename.h"
#include "ui_DialogRename.h"

#include "Preferences.h"
#include "BasicXMLSyntaxHighlighter.h"
#include "Simulation.h"
#include "GSUtil.h"
#include "PGDMath.h"

#include "pystring.h"

#include <QPlainTextEdit>
#include <QMessageBox>
#include <QCheckBox>
#include <QDebug>
#include <QRegularExpression>

#include <string>
#include <vector>
#include <set>

using namespace std::literals::string_literals;

DialogRename::DialogRename(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogRename)
{
    ui->setupUi(this);
    setWindowTitle(tr("Create Mirror Elements"));
#ifdef Q_OS_MACOS
    setWindowFlags(windowFlags() & (~Qt::Dialog) | Qt::Window); // allows the window to be resized on macs
#endif

    loadPreferences();

    connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->pushButtonApply, SIGNAL(clicked()), this, SLOT(apply()));
    connect(ui->lineEditFrom, SIGNAL(textChanged(const QString &)), this, SLOT(textChanged(const QString &)));
    connect(ui->lineEditTo, SIGNAL(textChanged(const QString &)), this, SLOT(textChanged(const QString &)));
    connect(ui->plainTextEdit, SIGNAL(modificationChanged(bool)), this, SLOT(modificationChanged(bool)));
    connect(ui->comboBoxFrom, SIGNAL(currentTextChanged(const QString &)), this, SLOT(currentTextChanged(const QString &)));

    setEditorFonts();
    enableControls();
}

DialogRename::~DialogRename()
{
    delete ui;
}

void DialogRename::accept()
{
    qDebug() << "DialogRename::accept()";

    std::string *errorMessage = validate();
    if (errorMessage)
    {
        QMessageBox::warning(this, "Error validating XML file", QString("Error message:\n%1\nSuggest either fix error or Cancel").arg(QString::fromStdString(*errorMessage)));
        return;
    }

    savePreferences();
    QDialog::accept();
}

void DialogRename::reject() // this catches cancel, close and escape key
{
    qDebug() << "DialogRename::reject()";

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

void DialogRename::loadPreferences()
{
    m_editorFont = Preferences::valueQFont("DialogRenameEditorFont");
    ui->checkBoxCaseSensitive->setChecked(Preferences::valueBool("DialogRenameCaseSensitiveSearch"));
    ui->checkBoxRegularExpression->setChecked(Preferences::valueBool("DialogRenameRegularExpressionSearch"));
    ui->checkBoxWholeWord->setChecked(Preferences::valueBool("DialogRenameWholeWordSearch"));
    ui->lineEditFrom->setText(Preferences::valueQString("DialogRenameFrom"));
    ui->lineEditTo->setText(Preferences::valueQString("DialogRenameTo"));
    restoreGeometry(Preferences::valueQByteArray("DialogRenameGeometry"));
}

void DialogRename::savePreferences()
{
    Preferences::insert("DialogRenameCaseSensitiveSearch", ui->checkBoxCaseSensitive->isChecked());
    Preferences::insert("DialogRenameRegularExpressionSearch", ui->checkBoxRegularExpression->isChecked());
    Preferences::insert("DialogRenameWholeWordSearch", ui->checkBoxWholeWord->isChecked());
    Preferences::insert("DialogRenameEditorFont", m_editorFont);
    Preferences::insert("DialogRenameFrom", ui->lineEditFrom->text());
    Preferences::insert("DialogRenameTo", ui->lineEditTo->text());
    Preferences::insert("DialogRenameGeometry", saveGeometry());
}

QString DialogRename::editorText() const
{
    return ui->plainTextEdit->toPlainText();
}

void DialogRename::setEditorText(const QString &editorText)
{
    ui->plainTextEdit->setPlainText(editorText);
}

void DialogRename::useXMLSyntaxHighlighter()
{
    m_basicXMLSyntaxHighlighter = new BasicXMLSyntaxHighlighter(ui->plainTextEdit->document());
}

void DialogRename::setEditorFonts()
{
    QList<QLineEdit *> listQLineEdit = this->findChildren<QLineEdit *>(QString(), Qt::FindChildrenRecursively);
    for (QList<QLineEdit *>::iterator it = listQLineEdit.begin(); it != listQLineEdit.end(); it++) (*it)->setFont(m_editorFont);

    QList<QPlainTextEdit *> listQPlainTextEdit = this->findChildren<QPlainTextEdit *>(QString(), Qt::FindChildrenRecursively);
    for (QList<QPlainTextEdit *>::iterator it = listQPlainTextEdit.begin(); it != listQPlainTextEdit.end(); it++) (*it)->setFont(m_editorFont);
}

void DialogRename::enableControls()
{
    ui->pushButtonApply->setEnabled(ui->lineEditTo->text().size() > 0 && ui->lineEditFrom->text().size() > 0);
    ui->pushButtonOK->setEnabled(isModified());
}

std::string *DialogRename::validate()
{
    Simulation simulation;
    QByteArray editFileData = ui->plainTextEdit->toPlainText().toUtf8();
    std::string *errorMessage = simulation.LoadModel(editFileData.constData(), size_t(editFileData.size()));
    if (errorMessage)
    {
        m_lastError = *errorMessage;
        return &m_lastError;
    }
    return nullptr;
}

void DialogRename::textChanged(const QString & /*text*/)
{
    enableControls();
}

bool DialogRename::isModified() const
{
    return ui->plainTextEdit->document()->isModified();
}

void DialogRename::setModified(bool modified)
{
    ui->plainTextEdit->document()->setModified(modified);
    enableControls();
}

void DialogRename::apply()
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
    std::string fromString = ui->lineEditFrom->text().toStdString();
    std::string toString = ui->lineEditTo->text().toStdString();

    applyRename(fromString, toString);

    std::string newXML = m_parseXML.SaveModel("GAITSYM2019"s, "Created from DialogRename::apply"s);
    ui->plainTextEdit->setPlainText(QString::fromStdString(newXML));
    if (localModified || (xml != newXML)) setModified(true);
}

void DialogRename::applyRename(const std::string &fromString, const std::string &toString)
{
    std::vector<std::unique_ptr<ParseXML::XMLElement>> *elementList = m_parseXML.elementList();

    // loop through the elements changinf matching IDs
    for (auto &&tagElementIt : *elementList)
    {
        for (auto &&attributeIT : tagElementIt->attributes)
        {
            // single name IDs allways have an attribute ending in ID
            if (pystring::endswith(attributeIT.first, "ID"s))
            {
                if (attributeFind(attributeIT.second, fromString))
                {
                    attributeIT.second = attributeReplace(attributeIT.second, fromString, toString);
                }
            }
            // multiple naame IDs always have an attribute ending in IDList, IDList1, IDList2
            if (pystring::endswith(attributeIT.first, "IDList"s) || pystring::endswith(attributeIT.first, "IDList1"s) || pystring::endswith(attributeIT.first, "IDList2"s))
            {
                bool changed = false;
                std::vector<std::string> result;
                pystring::split(attributeIT.second, result);
                for (auto &&resultIt : result)
                {
                    if (attributeFind(resultIt, fromString))
                    {
                        resultIt = attributeReplace(resultIt, fromString, toString);
                        changed = true;
                    }
                }
                if (changed)
                    attributeIT.second = pystring::join(" "s, result);
            }
        }
    }
}

bool DialogRename::attributeFind(const std::string &refStr, const std::string &findStr)
{
    bool caseSensitive = false;
    bool useRegex = false;
    bool wholeWord = false;
    if (ui->checkBoxCaseSensitive->isChecked()) caseSensitive = true;
    if (ui->checkBoxRegularExpression->isChecked()) useRegex = true;
    if (ui->checkBoxWholeWord->isChecked()) wholeWord = true;
    if (wholeWord == true && useRegex == false && refStr.size() != findStr.size()) return false;
    QString qRefStr = QString::fromStdString(refStr);
    QString qFindStr = QString::fromStdString(findStr);
    if (!useRegex) return qRefStr.contains(qFindStr, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);

    QRegularExpression regExp(qFindStr, caseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = regExp.match(qRefStr);
    if (match.capturedStart(0) == -1) return false;
    return true;
}

std::string DialogRename::attributeReplace(const std::string input, const std::string &before, const std::string &after)
{
    bool caseSensitive = false;
    bool useRegex = false;
    if (ui->checkBoxCaseSensitive->isChecked()) caseSensitive = true;
    if (ui->checkBoxRegularExpression->isChecked()) useRegex = true;
    QString qInput = QString::fromStdString(input);
    QString qBefore = QString::fromStdString(before);
    QString qAfter = QString::fromStdString(after);
    if (!useRegex)
    {
        QString qReplaceStr = qInput.replace(qBefore, qAfter, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
        return qReplaceStr.toStdString();
    }

    QRegularExpression regExp(qBefore, caseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
    QString qReplaceStr = qInput.replace(regExp, qAfter);
    return qReplaceStr.toStdString();
}

void DialogRename::setNameList(std::vector<NamedObject *> *nameList)
{
    QSignalBlocker blocker(ui->lineEditFrom);
    m_nameList = nameList;
    QStringList names;
    for (auto &&it : *m_nameList) names.append(QString::fromStdString(it->name()));
    names.sort(Qt::CaseInsensitive);
    ui->comboBoxFrom->clear();
    ui->comboBoxFrom->addItems(names);
    ui->comboBoxFrom->setCurrentIndex(-1);
}

void DialogRename::currentTextChanged(const QString &text)
{
    ui->lineEditFrom->setText(text);
}

void DialogRename::modificationChanged(bool /*changed*/)
{
    enableControls();
}



