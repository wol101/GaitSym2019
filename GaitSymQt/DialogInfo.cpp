#include "DialogInfo.h"
#include "ui_DialogInfo.h"

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

#include <string>
#include <vector>
#include <set>

using namespace std::literals::string_literals;

DialogInfo::DialogInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogInfo)
{
    ui->setupUi(this);
    setWindowTitle(tr("Element Information"));
#ifdef Q_OS_MACOS
    setWindowFlags(windowFlags() & (~Qt::Dialog) | Qt::Window); // allows the window to be resized on macs
#endif

    loadPreferences();

    connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(accept()));
    setEditorFonts();
}

DialogInfo::~DialogInfo()
{
    delete ui;
}

void DialogInfo::closeEvent(QCloseEvent *event)
{
    savePreferences();
    QDialog::closeEvent(event);
}

void DialogInfo::loadPreferences()
{
    m_editorFont = Preferences::valueQFont("DialogInfoEditorFont");
    restoreGeometry(Preferences::valueQByteArray("DialogInfoGeometry"));
}

void DialogInfo::savePreferences()
{
    Preferences::insert("DialogInfoEditorFont", m_editorFont);
    Preferences::insert("DialogInfoGeometry", saveGeometry());
}

QString DialogInfo::editorText() const
{
    return ui->plainTextEdit->toPlainText();
}

void DialogInfo::setEditorText(const QString &editorText)
{
    ui->plainTextEdit->setPlainText(editorText);
}

void DialogInfo::useXMLSyntaxHighlighter()
{
    m_basicXMLSyntaxHighlighter = new BasicXMLSyntaxHighlighter(ui->plainTextEdit->document());
}

void DialogInfo::setEditorFonts()
{
    QList<QPlainTextEdit *> listQPlainTextEdit = this->findChildren<QPlainTextEdit *>(QString(), Qt::FindChildrenRecursively);
    for (QList<QPlainTextEdit *>::iterator it = listQPlainTextEdit.begin(); it != listQPlainTextEdit.end(); it++) (*it)->setFont(m_editorFont);
}

