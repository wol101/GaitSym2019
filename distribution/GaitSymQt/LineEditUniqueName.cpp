#include "LineEditUniqueName.h"

#include <QDebug>
#include "UniqueNameValidator.h"

LineEditUniqueName::LineEditUniqueName(QWidget *parent) :
    QLineEdit(parent)
{
    m_uniqueNameValidator = new UniqueNameValidator();
    this->setValidator(m_uniqueNameValidator);
    connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(textChangedSlot(const QString &)));

    QString m_defaultStyleSheet = this->styleSheet();
    textChangedSlot(QString());
}

void LineEditUniqueName::textChangedSlot(const QString &text)
{
    QString localCopy(text);
    int pos = this->cursorPosition();
    QValidator::State state = this->validator()->validate(localCopy, pos);
    switch (state)
    {
    case QValidator::Acceptable:
        this->setStyleSheet(m_defaultStyleSheet);
        break;
    case QValidator::Intermediate:
        this->setStyleSheet("QLineEdit { background: rgb(255, 191, 0); }"); // amber
        break;
    case QValidator::Invalid:
        this->setStyleSheet("QLineEdit { background: rgb(255, 0, 0); }"); // red
        break;
    }
}

void LineEditUniqueName::addString(const QString &string)
{
    m_uniqueNameValidator->addString(string);
}

void LineEditUniqueName::addStrings(const QStringList &stringList)
{
    for (const auto &it : stringList) addString(it);
}

