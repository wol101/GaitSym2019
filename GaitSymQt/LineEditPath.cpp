#include "LineEditPath.h"

#include <QMenu>
#include <QFileDialog>
#include <QFocusEvent>
#include <QValidator>

LineEditPath::LineEditPath(QWidget *parent) :
    QLineEdit(parent)
{
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(menuRequestPath(QPoint)));
    connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(textChangedSlot(const QString &)));
    m_pathType = FileForOpen;
    this->setText("");
}

void LineEditPath::menuRequestPath(const QPoint &pos)
{
    QMenu *menu = this->createStandardContextMenu();
    menu->addSeparator();
    switch (m_pathType)
    {
    case FileForOpen:
        menu->addAction(tr("Select File to Open..."));
        break;
    case FileForSave:
        menu->addAction(tr("Select File to Save..."));
        break;
    case Folder:
        menu->addAction(tr("Select Folder..."));
        break;
    }
    QPoint gp = this->mapToGlobal(pos);
    QAction *action = menu->exec(gp);
    if (action)
    {
        if (action->text() == tr("Select Folder..."))
        {
            QString dir = QFileDialog::getExistingDirectory(this, "Select required folder", this->text());
            if (!dir.isEmpty()) this->setText(dir);
        }
        if (action->text() == tr("Select File to Open..."))
        {
            QString file = QFileDialog::getOpenFileName(this, "Select required file to open", this->text());
            if (!file.isEmpty()) this->setText(file);
        }
        if (action->text() == tr("Select File to Save..."))
        {
            QString file = QFileDialog::getSaveFileName(this, "Select required file to save", this->text());
            if (!file.isEmpty()) this->setText(file);
        }
    }
    delete menu;
}

void LineEditPath::focusInEvent(QFocusEvent *e)
{
    QLineEdit::focusInEvent(e);
    emit(focussed(true));
}

void LineEditPath::focusOutEvent(QFocusEvent *e)
{
    QLineEdit::focusOutEvent(e);
    emit(focussed(false));
}

LineEditPath::PathType LineEditPath::pathType() const
{
    return m_pathType;
}

void LineEditPath::setPathType(const PathType &pathType)
{
    m_pathType = pathType;
}

void LineEditPath::setHighlighted(bool highlight)
{
    if (highlight)
    {
        m_backgroundStyle = "rgb(0, 255, 0)"; // green
    }
    else
    {
        m_backgroundStyle = QString();
    }
    generateLocalStyleSheet();
}

void LineEditPath::textChangedSlot(const QString &text)
{
    QFileInfo fileInfo(text);
    QValidator::State state = QValidator::Invalid;
    if (m_pathType == FileForOpen && fileInfo.isFile()) state = QValidator::Acceptable;
    if (m_pathType == FileForSave)
    {
        if (!fileInfo.exists()) state = QValidator::Acceptable;
        else if (fileInfo.isFile()) state = QValidator::Acceptable;
    }
    if (m_pathType == Folder && fileInfo.isDir()) state = QValidator::Acceptable;
    switch (state)
    {
    case QValidator::Acceptable:
        m_foregroundStyle = QString();
        break;
    case QValidator::Intermediate:
        m_foregroundStyle = "rgb(255, 191, 0)"; // amber
        break;
    case QValidator::Invalid:
        m_foregroundStyle = "rgb(255, 0, 0)"; // red
        break;
    }
    generateLocalStyleSheet();
}

void LineEditPath::generateLocalStyleSheet()
{
    if (m_foregroundStyle.size() && m_backgroundStyle.size())
        this->setStyleSheet(QString("QLineEdit { background: %1 ; color: %2 }").arg(m_backgroundStyle).arg(m_foregroundStyle));
    if (m_foregroundStyle.size() && !m_backgroundStyle.size())
        this->setStyleSheet(QString("QLineEdit { color: %1 }").arg(m_foregroundStyle));
    if (!m_foregroundStyle.size() && m_backgroundStyle.size())
        this->setStyleSheet(QString("QLineEdit { background: %1 }").arg(m_backgroundStyle));
    if (!m_foregroundStyle.size() && !m_backgroundStyle.size())
        this->setStyleSheet(QString());
}

