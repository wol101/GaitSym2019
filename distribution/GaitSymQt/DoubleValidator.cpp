#include "DoubleValidator.h"

DoubleValidator::DoubleValidator(QObject *parent)
    : QDoubleValidator (parent)
{

}

DoubleValidator::DoubleValidator(double bottom, double top, int decimals, QObject *parent)
    : QDoubleValidator(bottom, top, decimals, parent)
{
}

// the only change is that we allow the empty string as a valid value;

QValidator::State DoubleValidator::validate(QString &input, int &pos) const
{
    if (input.isEmpty()) return QValidator::Acceptable;

    return QDoubleValidator::validate(input, pos);
}
