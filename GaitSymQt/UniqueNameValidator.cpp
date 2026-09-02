#include "UniqueNameValidator.h"

UniqueNameValidator::UniqueNameValidator(QObject *parent) : QValidator(parent)
{
    QRegularExpression regExp("[a-zA-Z0-9_]+");
    m_internalValidator.setRegularExpression(regExp);
}

QValidator::State UniqueNameValidator::validate(QString &input, int &pos) const
{
    QValidator::State state = m_internalValidator.validate(input, pos);
    if (state != QValidator::Acceptable) return state;
    if (m_setOfExistingStrings.contains(input)) return QValidator::Intermediate;
    return QValidator::Acceptable;
}

void UniqueNameValidator::addString(const QString &string)
{
    m_setOfExistingStrings.insert(string);
}

