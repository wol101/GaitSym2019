#include "TextEditDialog.h"
#include "ui_TextEditDialog.h"

#include "BasicXMLSyntaxHighlighter.h"
#include "Preferences.h"
#include "Simulation.h"

#include "pystring.h"
#include "exprtk.hpp"

#include <QPlainTextEdit>
#include <QMessageBox>
#include <QCheckBox>
#include <QDebug>
#include <QToolButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QFileDialog>
#include <QComboBox>

#include <QRegularExpression>

#include <string>
#include <sstream>
#include <regex>
#include <iomanip>

using namespace std::literals::string_literals;

TextEditDialog::TextEditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextEditDialog)
{
    ui->setupUi(this);
    setWindowTitle(tr("Raw XML Editor"));
#ifdef Q_OS_MACOS
    setWindowFlags(windowFlags() & (~Qt::Dialog) | Qt::Window); // allows the window to be resized on macs
#endif

    loadPreferences();

    connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->pushButtonFind, SIGNAL(clicked()), this, SLOT(find()));
    connect(ui->pushButtonReplace, SIGNAL(clicked()), this, SLOT(replace()));
    connect(ui->pushButtonReplaceAll, SIGNAL(clicked()), this, SLOT(replaceAll()));
    connect(ui->pushButtonSaveAs, SIGNAL(clicked()), this, SLOT(saveAs()));
    connect(ui->pushButtonSave, SIGNAL(clicked()), this, SLOT(attributeMachineSave()));
    connect(ui->pushButtonLoad, SIGNAL(clicked()), this, SLOT(attributeMachineLoad()));
    connect(ui->pushButtonApply, SIGNAL(clicked()), this, SLOT(attributeMachineApply()));
    connect(ui->toolButtonInsert, SIGNAL(clicked()), this, SLOT(attributeMachineInsert()));
    connect(ui->toolButtonDelete, SIGNAL(clicked()), this, SLOT(attributeMachineDelete()));
    connect(ui->toolButtonMoveUp, SIGNAL(clicked()), this, SLOT(attributeMachineMoveUp()));
    connect(ui->toolButtonMoveDown, SIGNAL(clicked()), this, SLOT(attributeMachineMoveDown()));
    connect(ui->pushButtonResetPositions, SIGNAL(clicked(bool)), this, SLOT(resetPositions()));
    connect(ui->tableWidget, SIGNAL(itemSelectionChanged()), this, SLOT(attributeMachineItemSelectionChanged()));
    connect(ui->plainTextEdit, SIGNAL(modificationChanged(bool)), this, SLOT(modificationChanged(bool)));

    setEditorFonts();
    enableControls();
}

TextEditDialog::~TextEditDialog()
{
    delete ui;
}

void TextEditDialog::accept()
{
    qDebug() << "TextEditDialog::accept()";

    std::string *errorMessage = validate();
    if (errorMessage)
    {
        QMessageBox::warning(this, "Error validating XML file", QString("Error message:\n%1\nSuggest either fix error or Save As and Cancel").arg(QString::fromStdString(*errorMessage)));
        return;
    }

    savePreferences();
    QDialog::accept();
}

void TextEditDialog::reject() // this catches cancel, close and escape key
{
    qDebug() << "TextEditDialog::reject()";

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

void TextEditDialog::closeEvent(QCloseEvent *event)
{
    qDebug() << "TextEditDialog::closeEvent()";
    // I don't think I need to do anything - accept and reject are always called
    // savePreferences();
    QDialog::closeEvent(event);
}

void TextEditDialog::loadPreferences()
{
    m_editorFont = Preferences::valueQFont("TextEditDialogEditorFont");
    ui->checkBoxCaseSensitive->setChecked(Preferences::valueBool("TextEditDialogCaseSensitiveSearch"));
    ui->checkBoxRegularExpression->setChecked(Preferences::valueBool("TextEditDialogRegularExpressionSearch"));
    ui->lineEditFind->setText(Preferences::valueQString("TextEditDialogFind"));
    ui->lineEditReplace->setText(Preferences::valueQString("TextEditDialogReplace"));
    restoreGeometry(Preferences::valueQByteArray("TextEditDialogGeometry"));
    ui->splitter->restoreState(Preferences::valueQByteArray("TextEditDialogSplitterState"));
    attributeMachineLoadFromString(Preferences::valueQString("TextEditDialogAttributeMachineCode"), true);
}

void TextEditDialog::savePreferences()
{
    Preferences::insert("TextEditDialogCaseSensitiveSearch", ui->checkBoxCaseSensitive->isChecked());
    Preferences::insert("TextEditDialogRegularExpressionSearch", ui->checkBoxRegularExpression->isChecked());
    Preferences::insert("TextEditDialogEditorFont", m_editorFont);
    Preferences::insert("TextEditDialogFind", ui->lineEditFind->text());
    Preferences::insert("TextEditDialogReplace", ui->lineEditReplace->text());
    Preferences::insert("TextEditDialogAttributeMachineCode", attributeMachineSaveToString());
    Preferences::insert("TextEditDialogGeometry", saveGeometry());
    Preferences::insert("TextEditDialogSplitterState", ui->splitter->saveState());
}

QString TextEditDialog::editorText() const
{
    return ui->plainTextEdit->toPlainText();
}

void TextEditDialog::setEditorText(const QString &editorText)
{
    ui->plainTextEdit->setPlainText(editorText);
}

void TextEditDialog::useXMLSyntaxHighlighter()
{
    m_basicXMLSyntaxHighlighter = new BasicXMLSyntaxHighlighter(ui->plainTextEdit->document());
}

void TextEditDialog::find()
{
    QString findString = ui->lineEditFind->text();
    if (findString.isEmpty()) return;

    bool result = false;
    if (ui->checkBoxRegularExpression->isChecked())
    {
        QRegularExpression regExp(findString, ui->checkBoxCaseSensitive->isChecked() ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
        result = ui->plainTextEdit->find(regExp);
    }
    else
    {
        QTextDocument::FindFlags options = {};
        if (ui->checkBoxCaseSensitive->isChecked()) options |= QTextDocument::FindCaseSensitively;
        result = ui->plainTextEdit->find(findString, options);
    }
    if (result == false)
    {
        QMessageBox::warning(this, tr("Find Error"), QString("Unable to find:\n%1").arg(findString));
        return;
    }
}

void TextEditDialog::replace()
{
    QString findString = ui->lineEditFind->text();
    if (findString.isEmpty()) return;

    // these 3 lines move the cursor to the beginning of the selection (i.e. the anchor position)
    QTextCursor textCursor = ui->plainTextEdit->textCursor();
    textCursor.setPosition(textCursor.anchor());
    ui->plainTextEdit->setTextCursor(textCursor);

    QString replaceString = ui->lineEditReplace->text();
    if (!replace(findString, replaceString))
    {
        QMessageBox::warning(this, tr("Find Error"), QString("Unable to find:\n%1").arg(findString));
        return;
    }
}

void TextEditDialog::replaceAll()
{
    QString findString = ui->lineEditFind->text();
    if (findString.isEmpty()) return;

    // these 3 lines move the cursor to the beginning of the selection (i.e. the anchor position)
    QTextCursor textCursor = ui->plainTextEdit->textCursor();
    textCursor.setPosition(textCursor.anchor());
    ui->plainTextEdit->setTextCursor(textCursor);

    QString replaceString = ui->lineEditReplace->text();
    int replaceCount = 0;
    while (replace(findString, replaceString)) { replaceCount++; };
    if (replaceCount == 0)
    {
        QMessageBox::warning(this, QString("Find Error"), QString("Unable to find:\n%1").arg(findString));
        return;
    }
    QMessageBox::information(this, QString("Replacing \"%1\"").arg(findString), QString("%1 replacements made").arg(replaceCount));
}

bool TextEditDialog::replace(const QString &findString, const QString &replaceString)
{
    bool result = false;
    if (ui->checkBoxRegularExpression->isChecked())
    {
        QRegularExpression regExp(findString, ui->checkBoxCaseSensitive->isChecked() ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
        result = ui->plainTextEdit->find(regExp);
    }
    else
    {
        QTextDocument::FindFlags options;
        if (ui->checkBoxCaseSensitive->isChecked()) options |= QTextDocument::FindCaseSensitively;
        result = ui->plainTextEdit->find(findString, options);
    }
    if (result == false)
    {
        return false;
    }

    if (ui->checkBoxRegularExpression->isChecked())
    {
        QTextCursor cursor = ui->plainTextEdit->textCursor();
        QString selectedString;
        selectedString.setUtf16(cursor.selectedText().utf16(), cursor.selectedText().size()); // this forces a deep copy
        QRegularExpression regExp(findString, ui->checkBoxCaseSensitive->isChecked() ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
        selectedString.replace(regExp, replaceString);
        cursor.insertText(cursor.selectedText().replace(cursor.selectedText(), selectedString));
    }
    else
    {
        QTextCursor cursor = ui->plainTextEdit->textCursor();
        cursor.insertText(cursor.selectedText().replace(cursor.selectedText(), replaceString));
    }
    return true;
}

void TextEditDialog::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Model State File"), m_fileName, tr("Config Files (*.gaitsym);;XML files (*.xml)"), nullptr);
    if (fileName.isNull() == false)
    {
        QString textData = ui->plainTextEdit->toPlainText();
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            file.write(textData.toUtf8());
            file.close();
        }
        else
        {
            QMessageBox::warning(this, "File Save As error", QString("Could not write '%1'\n%2\n").arg(fileName).arg(file.errorString()));
        }
    }
}

void TextEditDialog::setEditorFonts()
{
    QList<QLineEdit *> listQLineEdit = this->findChildren<QLineEdit *>(QString(), Qt::FindChildrenRecursively);
    for (QList<QLineEdit *>::iterator it = listQLineEdit.begin(); it != listQLineEdit.end(); it++) (*it)->setFont(m_editorFont);

    QList<QPlainTextEdit *> listQPlainTextEdit = this->findChildren<QPlainTextEdit *>(QString(), Qt::FindChildrenRecursively);
    for (QList<QPlainTextEdit *>::iterator it = listQPlainTextEdit.begin(); it != listQPlainTextEdit.end(); it++) (*it)->setFont(m_editorFont);
}

void TextEditDialog::enableControls()
{
    ui->toolButtonDelete->setEnabled(ui->tableWidget->rowCount() > 0);
    ui->toolButtonInsert->setEnabled(true);
    ui->toolButtonMoveUp->setEnabled(ui->tableWidget->rowCount() > 1 && ui->tableWidget->currentRow() > 0);
    ui->toolButtonMoveDown->setEnabled(ui->tableWidget->rowCount() > 1 && ui->tableWidget->currentRow() < ui->tableWidget->rowCount() - 1);
    ui->pushButtonOK->setEnabled(isModified());
}

void TextEditDialog::attributeMachineInsert()
{
    int row = ui->tableWidget->currentRow();
    ui->tableWidget->insertRow(row + 1);
    QComboBox* combo = new QComboBox();
    combo->addItem(QString("Edit"));
    combo->addItem(QString("Create"));
    combo->addItem(QString("Delete"));
    combo->setCurrentIndex(-1);
    ui->tableWidget->setCellWidget(row + 1, 0, combo);
    enableControls();
}

void TextEditDialog::attributeMachineDelete()
{
    if (ui->tableWidget->rowCount() == 0) return;
    int row = ui->tableWidget->currentRow();
    ui->tableWidget->removeRow(row);
    enableControls();
}

void TextEditDialog::attributeMachineMoveUp()
{
    if (ui->tableWidget->rowCount() < 2) return;
    int row = ui->tableWidget->currentRow();
    if (row == 0) return;
    ui->tableWidget->insertRow(row + 1);
    for (int column = 0; column < ui->tableWidget->columnCount(); column++)
    {
        if (column != 0)
        {
            QTableWidgetItem *item = ui->tableWidget->takeItem(row - 1, column);
            ui->tableWidget->setItem(row + 1, column, item);
        }
        else
        {
            QComboBox* combo = qobject_cast<QComboBox *>(ui->tableWidget->cellWidget(row - 1, column));
            if (combo) ui->tableWidget->setCellWidget(row + 1, column, combo);
        }
    }
    ui->tableWidget->removeRow(row - 1);
    enableControls();
}

void TextEditDialog::attributeMachineMoveDown()
{
    if (ui->tableWidget->rowCount() < 2) return;
    int row = ui->tableWidget->currentRow();
    if (row >= ui->tableWidget->rowCount() - 1) return;
    ui->tableWidget->insertRow(row);
    for (int column = 0; column < ui->tableWidget->columnCount(); column++)
    {
        if (column != 0)
        {
            QTableWidgetItem *item = ui->tableWidget->takeItem(row + 2, column);
            ui->tableWidget->setItem(row, column, item);
        }
        else
        {
            QComboBox* combo = qobject_cast<QComboBox *>(ui->tableWidget->cellWidget(row + 2, column));
            if (combo) ui->tableWidget->setCellWidget(row, column, combo);
        }
    }
    ui->tableWidget->removeRow(row + 2);
    enableControls();
}

void TextEditDialog::attributeMachineItemSelectionChanged()
{
    enableControls();
}

void TextEditDialog::attributeMachineSave()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save XML attribute machine"), QString(), tr("Tab Delimited Files (*.tab);;Text files (*.txt);;All files (*.*)"), nullptr);
    if (fileName.isNull() == false)
    {
        QString textData = attributeMachineSaveToString();
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            file.write(textData.toUtf8());
            file.close();
        }
        else
        {
            QMessageBox::warning(this, "File save error", QString("Could not write '%1'\n%2\n").arg(fileName).arg(file.errorString()));
        }
    }
}

QString TextEditDialog::attributeMachineSaveToString()
{
    QString textData;
    for (int col = 0; col < ui->tableWidget->columnCount(); col++)
    {
        textData += ui->tableWidget->horizontalHeaderItem(col)->text();
        if (col < ui->tableWidget->columnCount() - 1) textData += "\t";
        else textData += "\n";
    }
    for (int row = 0; row < ui->tableWidget->rowCount(); row++)
    {
        QComboBox *combo = qobject_cast<QComboBox *>(ui->tableWidget->cellWidget(row, 0));
        if (combo) textData += combo->currentText();
        for (int col = 1; col < ui->tableWidget->columnCount(); col++)
        {
            textData += "\t";
            QTableWidgetItem *item = ui->tableWidget->item(row, col);
            if (item) textData += item->text();
        }
        textData += "\n";
    }
    return textData;
}

void TextEditDialog::attributeMachineLoad()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open XML attribute machine"), QString(), tr("Tab Delimited Files (*.tab);;Text files (*.txt);;All files (*.*)"), nullptr);
    if (fileName.isNull() == false)
    {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::warning(this, "File read error", QString("Could not read '%1'\n%2\n").arg(fileName).arg(file.errorString()));
            return;
        }
        QByteArray data = file.readAll();
        QString text = QString::fromUtf8(data);
        attributeMachineLoadFromString(text, false);
    }
}

void TextEditDialog::attributeMachineLoadFromString(const QString &string, bool quiet)
{
    QStringList lines = string.split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts);
    QStringList tokenList;
    QString line;
    int iLine = 0;
    while (iLine < lines.size())
    {
        line = lines[iLine++];
        tokenList = line.split(QLatin1Char('\t'));
        break;
    }
    if (tokenList.size() < ui->tableWidget->columnCount())
    {
        if (!quiet) QMessageBox::warning(this, "File format error", QString("Wrong number of columns\n"));
        return;
    }
    for (int col = 0; col < ui->tableWidget->columnCount(); col++)
    {
        if (tokenList[col] != ui->tableWidget->horizontalHeaderItem(col)->text())
        {
            if (!quiet) QMessageBox::warning(this, "File format error", QString("Column names incorrect\n"));
            return;
        }
    }
    for (int row = ui->tableWidget->rowCount() - 1; row >= 0; row--)
    {
        ui->tableWidget->removeRow(row);
    }
    while (iLine < lines.size())
    {
        line = lines[iLine++];
        tokenList = line.split(QLatin1Char('\t'));
        if (tokenList.size() >= ui->tableWidget->columnCount())
        {
            int row = ui->tableWidget->rowCount();
            ui->tableWidget->insertRow(row);
            QComboBox* combo = new QComboBox();
            combo->addItem(QString("Edit"));
            combo->addItem(QString("Create"));
            combo->addItem(QString("Delete"));
            combo->setCurrentText(tokenList[0]);
            ui->tableWidget->setCellWidget(row, 0, combo);
            for (int col = 1; col < ui->tableWidget->columnCount(); col++)
            {
                QTableWidgetItem *item = new QTableWidgetItem(tokenList[col]);
                ui->tableWidget->setItem(row, col, item);
            }
        }
    }
}

void TextEditDialog::attributeMachineApply()
{
    bool localModified = ui->plainTextEdit->document()->isModified();
    std::string *lastError;
    std::string xml = ui->plainTextEdit->toPlainText().toStdString();
    std::string rootNodeTag = "GAITSYM2019"s;
    lastError = m_parseXML.LoadModel(xml.c_str(), xml.size(), rootNodeTag);
    if (lastError)
    {
        QMessageBox::warning(this, "XML parse error", QString("'%1'").arg(QString::fromStdString(*lastError)));
        return;
    }

    for (int row = 0; row < ui->tableWidget->rowCount(); row++)
    {
        QStringList tokens;
        QComboBox *combo = qobject_cast<QComboBox *>(ui->tableWidget->cellWidget(row, 0));
        if (combo) tokens.append(combo->currentText());
        else tokens.append(QString());
        for (int col = 1; col < ui->tableWidget->columnCount(); col++)
        {
            QTableWidgetItem *item = ui->tableWidget->item(row, col);
            if (item) tokens.append(item->text());
            else tokens.append(QString());
        }
        attributeMachineApplyRow(tokens[0].toStdString(), tokens[1].toStdString(), tokens[2].toStdString(), tokens[3].toStdString(), tokens[4].toStdString(),
                tokens[5].toStdString(), tokens[6].toStdString(), tokens[7].toStdString(), tokens[8].toStdString());
    }

    std::string newXML = m_parseXML.SaveModel("GAITSYM2019"s, "Created from TextEditDialog::attributeMachineApply"s);
    ui->plainTextEdit->setPlainText(QString::fromStdString(newXML));
    if (localModified || (xml != newXML)) setModified(true);
}


void TextEditDialog::attributeMachineApplyRow(const std::string &action, const std::string &matchTag, const std::string &matchAttribute, const std::string &matchAttributeValue,
                                              const std::string &changeAttribute, const std::string &tokenNumber, const std::string &search, const std::string &replace, const std::string &arithmetic)
{
    if (action == "Edit")
        attributeMachineApplyRowEdit(matchTag, matchAttribute, matchAttributeValue, changeAttribute, tokenNumber, search, replace, arithmetic);
    if (action == "Create")
        attributeMachineApplyRowCreate(matchTag, matchAttribute, matchAttributeValue, changeAttribute, tokenNumber, search, replace, arithmetic);
    if (action == "Delete")
        attributeMachineApplyRowDelete(matchTag, matchAttribute, matchAttributeValue);
}

void TextEditDialog::attributeMachineApplyRowEdit(const std::string &matchTag, const std::string &matchAttribute, const std::string &matchAttributeValue,
                                                  const std::string &changeAttribute, const std::string &tokenNumber, const std::string &search, const std::string &replace, const std::string &arithmetic)
{

    std::regex regularExpression("^\\s*(0|[1-9]\\d*)\\s*$"s);
    std::smatch match;
    bool tokenNumberDefined = std::regex_search(tokenNumber, match, regularExpression);
    size_t tokenNumberValue = size_t(std::atoi(tokenNumber.c_str()));
    auto elementList = m_parseXML.elementList();
    for (auto &&tagElementIt : *elementList)
    {
        if (attributeMachineMatch(tagElementIt->tag, matchTag))
        {
            auto attributes = &tagElementIt->attributes;
            for (auto &&matchAttributeIt : *attributes)
            {
                if (attributeMachineMatch(matchAttributeIt.first, matchAttribute) && attributeMachineMatch(matchAttributeIt.second, matchAttributeValue))
                {
                    for (auto &&changeAttributeIt : *attributes)
                    {
                        if (attributeMachineMatch(changeAttributeIt.first, changeAttribute))
                        {
                            if (search.size())
                            {
                                if (!tokenNumberDefined)
                                {
                                    changeAttributeIt.second = attributeMachineReplace(changeAttributeIt.second, search, replace);
                                }
                                else
                                {
                                    std::vector<std::string> result;
                                    pystring::split(changeAttributeIt.second, result);
                                    if (result.size() > tokenNumberValue) result[tokenNumberValue] = attributeMachineReplace(result[tokenNumberValue], search, replace);
                                    changeAttributeIt.second = pystring::join(" "s, result);
                                }
                            }
                            if (pystring::strip(arithmetic).size())
                            {
                                if (!tokenNumberDefined)
                                {
                                    changeAttributeIt.second = attributeMachineArithmetic(changeAttributeIt.second, arithmetic);
                                }
                                else
                                {
                                    std::vector<std::string> result;
                                    pystring::split(changeAttributeIt.second, result);
                                    if (result.size() > tokenNumberValue) result[tokenNumberValue] = attributeMachineArithmetic(result[tokenNumberValue], arithmetic);
                                    changeAttributeIt.second = pystring::join(" "s, result);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void TextEditDialog::attributeMachineApplyRowCreate(const std::string &matchTag, const std::string &matchAttribute, const std::string &matchAttributeValue,
                                                    const std::string &changeAttribute, const std::string &tokenNumber, const std::string &search, const std::string &replace, const std::string &arithmetic)
{

    std::regex regularExpression("^\\s*(0|[1-9]\\d*)\\s*$"s);
    std::smatch match;
    bool tokenNumberDefined = std::regex_search(tokenNumber, match, regularExpression);
    size_t tokenNumberValue = size_t(std::atoi(tokenNumber.c_str()));
    std::vector<std::unique_ptr<ParseXML::XMLElement>> newElements;
    std::vector<std::unique_ptr<ParseXML::XMLElement>> *elementList = m_parseXML.elementList();
    for (auto &&tagElementIt : *elementList)
    {
        if (attributeMachineMatch(tagElementIt->tag, matchTag))
        {
            auto attributes = &tagElementIt->attributes;
            for (auto &&matchAttributeIt : *attributes)
            {
                if (attributeMachineMatch(matchAttributeIt.first, matchAttribute) && attributeMachineMatch(matchAttributeIt.second, matchAttributeValue))
                {
                    std::unique_ptr<ParseXML::XMLElement> newElement = std::make_unique<ParseXML::XMLElement>();
                    newElement->tag = tagElementIt->tag;
                    newElement->attributes = tagElementIt->attributes;
                    for (auto &&changeAttributeIt : newElement->attributes)
                    {
                        if (attributeMachineMatch(changeAttributeIt.first, changeAttribute))
                        {
                            if (search.size())
                            {
                                if (!tokenNumberDefined)
                                {
                                    changeAttributeIt.second = attributeMachineReplace(changeAttributeIt.second, search, replace);
                                }
                                else
                                {
                                    std::vector<std::string> result;
                                    pystring::split(changeAttributeIt.second, result);
                                    if (result.size() > tokenNumberValue) result[tokenNumberValue] = attributeMachineReplace(result[tokenNumberValue], search, replace);
                                    changeAttributeIt.second = pystring::join(" "s, result);
                                }
                            }
                            if (pystring::strip(arithmetic).size())
                            {
                                if (!tokenNumberDefined)
                                {
                                    changeAttributeIt.second = attributeMachineArithmetic(changeAttributeIt.second, arithmetic);
                                }
                                else
                                {
                                    std::vector<std::string> result;
                                    pystring::split(changeAttributeIt.second, result);
                                    if (result.size() > tokenNumberValue) result[tokenNumberValue] = attributeMachineArithmetic(result[tokenNumberValue], arithmetic);
                                    changeAttributeIt.second = pystring::join(" "s, result);
                                }
                            }
                        }
                    }
                    newElements.push_back(std::move(newElement));
                }
            }
        }
    }
    for (auto &&newElementIt : newElements)
    {
        elementList->push_back(std::move(newElementIt));
    }
}

void TextEditDialog::attributeMachineApplyRowDelete(const std::string &matchTag, const std::string &matchAttribute, const std::string &matchAttributeValue)
{

    std::regex regularExpression("^\\s*(0|[1-9]\\d*)\\s*$"s);
    std::smatch match;
    auto elementList = m_parseXML.elementList();
    for (auto tagElementIt = elementList->begin(); tagElementIt != elementList->end();)
    {
        bool deleteFlag = false;
        if (attributeMachineMatch(tagElementIt->get()->tag, matchTag))
        {
            auto attributes = &tagElementIt->get()->attributes;
            for (auto &&matchAttributeIt : *attributes)
            {
                if (attributeMachineMatch(matchAttributeIt.first, matchAttribute) && attributeMachineMatch(matchAttributeIt.second, matchAttributeValue))
                {
                    deleteFlag = true;
                    break;
                }
            }
        }
        if (deleteFlag)
            tagElementIt =elementList->erase(tagElementIt);
        else
            tagElementIt++;
    }
}

bool TextEditDialog::attributeMachineMatch(const std::string &refStr, const std::string &findStr)
{
    bool caseSensitive = false;
    bool useRegex = false;
    if (ui->checkBoxCaseSensitive->isChecked()) caseSensitive = true;
    if (ui->checkBoxRegularExpression->isChecked()) useRegex = true;
    QString qRefStr = QString::fromStdString(refStr);
    QString qFindStr = QString::fromStdString(findStr);
    if (!useRegex)
    {
        int compare = qRefStr.compare(qFindStr, caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
        if (compare == 0) return true;
        return false;
    }

    QRegularExpression regExp(qFindStr, caseSensitive ? QRegularExpression::NoPatternOption : QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = regExp.match(qRefStr);
    return match.hasMatch();
}

std::string TextEditDialog::attributeMachineReplace(const std::string input, const std::string &before, const std::string &after)
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

// this needs to be customised depending on how the genome interacts with
// the XML file specifying the simulation
std::string TextEditDialog::attributeMachineArithmetic(const std::string &original, const std::string &arithmetic)
{
    std::string newString;
    exprtk::symbol_table<double> *symbol_table;
    exprtk::expression<double> *expression;
    exprtk::parser<double> *parser;

    symbol_table = new exprtk::symbol_table<double>();
    expression = new exprtk::expression<double>();
    parser = new exprtk::parser<double>();

    symbol_table->add_constant("v", std::atof(original.c_str()));
    symbol_table->add_constants(); // this adds pi, epsilon, inf

    expression->register_symbol_table(*symbol_table);
    bool success = parser->compile(arithmetic, *expression);
    if (success)
    {
        double newValue = expression->value();
        std::stringstream ss;
        ss << std::setprecision(18) << newValue; // this defaults to %.18g format if neither fixed nor scientific is set
        newString = ss.str();
    }
    return newString;
}

void TextEditDialog::resetPositions()
{
    bool localModified = ui->plainTextEdit->document()->isModified();
    std::string *lastError;
    std::string xml = ui->plainTextEdit->toPlainText().toStdString();
    std::string rootNodeTag = "GAITSYM2019"s;
    lastError = m_parseXML.LoadModel(xml.c_str(), xml.size(), rootNodeTag);
    if (lastError)
    {
        QMessageBox::warning(this, "XML parse error", QString("'%1'").arg(QString::fromStdString(*lastError)));
        return;
    }

    auto elementList = m_parseXML.elementList();
    for (auto tagElementIt = elementList->begin(); tagElementIt != elementList->end(); tagElementIt++)
    {
        if (tagElementIt->get()->tag == "BODY"s)
        {
            auto constructionPosition = tagElementIt->get()->attributes.find("ConstructionPosition"s);
            auto position = tagElementIt->get()->attributes.find("Position"s);
            auto quaternion = tagElementIt->get()->attributes.find("Quaternion"s);
            if (constructionPosition == tagElementIt->get()->attributes.end() || position == tagElementIt->get()->attributes.end() || quaternion == tagElementIt->get()->attributes.end())
            {
                auto name = tagElementIt->get()->attributes.find("ID"s);
                if (name == tagElementIt->get()->attributes.end())
                    QMessageBox::warning(this, tr("Reset Positions Error"), QString("Unable to find unnamed BODY"));
                else
                    QMessageBox::warning(this, tr("Reset Positions Error"), QString("Unable to find BODY:\n%1").arg(name->second.c_str()));
                return;
            }
            position->second = constructionPosition->second;
            quaternion->second = "1 0 0 0"s;
        }
    }
    std::string newXML = m_parseXML.SaveModel("GAITSYM2019"s, "Created from TextEditDialog::resetPositions"s);
    ui->plainTextEdit->setPlainText(QString::fromStdString(newXML));
    if (localModified || (xml != newXML)) setModified(true);
}

std::string *TextEditDialog::validate()
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

void TextEditDialog::modificationChanged(bool /*changed*/)
{
    enableControls();
}

bool TextEditDialog::isModified() const
{
    return ui->plainTextEdit->document()->isModified();
}

void TextEditDialog::setFileName(const QString &fileName)
{
   m_fileName = fileName;
}

void TextEditDialog::setModified(bool modified)
{
    ui->plainTextEdit->document()->setModified(modified);
    enableControls();
}


