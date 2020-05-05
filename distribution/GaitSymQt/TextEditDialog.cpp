#include "TextEditDialog.h"
#include "ui_TextEditDialog.h"

#include "BasicXMLSyntaxHighlighter.h"
#include "Preferences.h"

#include <QPlainTextEdit>
#include <QMessageBox>
#include <QCheckBox>
#include <QRegularExpression>
#include <QDebug>

TextEditDialog::TextEditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TextEditDialog)
{
    ui->setupUi(this);

    connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->pushButtonFind, SIGNAL(clicked()), this, SLOT(find()));
    connect(ui->pushButtonReplace, SIGNAL(clicked()), this, SLOT(replace()));

    m_editorFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    m_editorFont = Preferences::valueQFont("EditorFont");
    restoreGeometry(Preferences::valueQByteArray("TextEditDialogGeometry"));

    setEditorFonts();
}

TextEditDialog::~TextEditDialog()
{
    delete ui;
}

void TextEditDialog::accept() // this catches cancel, close and escape key
{
    qDebug() << "TextEditDialog::accept()";
    Preferences::insert("TextEditDialogGeometry", saveGeometry());
    QDialog::reject();
}

void TextEditDialog::reject() // this catches cancel, close and escape key
{
    qDebug() << "TextEditDialog::reject()";
    Preferences::insert("TextEditDialogGeometry", saveGeometry());
    QDialog::reject();
}

void TextEditDialog::closeEvent(QCloseEvent *event)
{
    Preferences::insert("DialogDriversGeometry", saveGeometry());
    QDialog::closeEvent(event);
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

    QTextDocument::FindFlags options = nullptr;
    if (ui->checkBoxCaseSensitive->isChecked()) options |= QTextDocument::FindCaseSensitively;
    bool result = ui->plainTextEdit->find(findString, options);
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
    QString replaceString = ui->lineEditReplace->text();

    // check to see if we have already found something
    QTextCursor cursor = ui->plainTextEdit->textCursor();
    if (cursor.hasSelection() &&  cursor.selectedText() == findString)
    {
        cursor.insertText(cursor.selectedText().replace(cursor.selectedText(), replaceString));
        return;
    }

    // find a new thing and replace it
    QTextDocument::FindFlags options = nullptr;
    if (ui->checkBoxCaseSensitive->isChecked()) options |= QTextDocument::FindCaseSensitively;
    bool result = ui->plainTextEdit->find(findString, options);
    if (result)
    {
        cursor = ui->plainTextEdit->textCursor();
        cursor.insertText(cursor.selectedText().replace(cursor.selectedText(), replaceString));
    }
    else
    {
        QMessageBox::warning(this, tr("Replace Error"), QString("Unable to find:\n%1").arg(findString));
        return;
    }
}

void TextEditDialog::setEditorFonts()
{
    QList<QLineEdit *> listQLineEdit = this->findChildren<QLineEdit *>(QRegularExpression(".*", QRegularExpression::CaseInsensitiveOption), Qt::FindChildrenRecursively);
    for (QList<QLineEdit *>::iterator it = listQLineEdit.begin(); it != listQLineEdit.end(); it++) (*it)->setFont(m_editorFont);

    QList<QPlainTextEdit *> listQPlainTextEdit = this->findChildren<QPlainTextEdit *>(QRegularExpression(".*", QRegularExpression::CaseInsensitiveOption), Qt::FindChildrenRecursively);
    for (QList<QPlainTextEdit *>::iterator it = listQPlainTextEdit.begin(); it != listQPlainTextEdit.end(); it++) (*it)->setFont(m_editorFont);
}




