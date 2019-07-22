#include "LineEditDouble.h"
#include "DoubleValidator.h"

#include <QDebug>
#include <QLocale>

#include <limits>

LineEditDouble::LineEditDouble(QWidget *parent) :
    QLineEdit(parent)
{
    DoubleValidator *doubleValidator = new DoubleValidator();
    doubleValidator->setNotation(QDoubleValidator::ScientificNotation);
    // doubleValidator->setRange(-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(), std::numeric_limits<double>::digits10);
    // doubleValidator->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::digits10);
    doubleValidator->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), 17);
    this->setValidator(doubleValidator);
    connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(textChangedSlot(const QString &)));

    QString m_defaultStyleSheet = this->styleSheet();
    this->setText("");
}

void LineEditDouble::textChangedSlot(const QString &text)
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
void LineEditDouble::setBottom(double bottom)
{
    const DoubleValidator *v = dynamic_cast<const DoubleValidator *>(this->validator());
    Q_ASSERT(v);
//    double bottom = v->bottom();
    double top = v->top();
    int decimals = v->decimals();
    setValidator(new DoubleValidator(bottom, top, decimals));
}

void LineEditDouble::setTop(double top)
{
    const DoubleValidator *v = dynamic_cast<const DoubleValidator *>(this->validator());
    Q_ASSERT(v);
    double bottom = v->bottom();
//    double top = v->top();
    int decimals = v->decimals();
    setValidator(new DoubleValidator(bottom, top, decimals));
}

void LineEditDouble::setDecimals(int decimals)
{
    const DoubleValidator *v = dynamic_cast<const DoubleValidator *>(this->validator());
    Q_ASSERT(v);
    double bottom = v->bottom();
    double top = v->top();
//    int decimals = v->decimals();
    setValidator(new DoubleValidator(bottom, top, decimals));
}

void LineEditDouble::setValue(double value)
{
    // there are potential round trip problems here if value is equal to either bottom or top
    const DoubleValidator *validator = static_cast<const DoubleValidator *>(this->validator());
    QString valueString;
    int decimals = validator->decimals();
    valueString.setNum(value, 'g', decimals);
    int pos = 0;
    if (validator->validate(valueString, pos) != QValidator::Acceptable)
    {
        if (decimals > std::numeric_limits<double>::digits10) valueString.setNum(value, 'e', std::numeric_limits<double>::digits10);
        else valueString.setNum(value, 'e', decimals);
    }
    this->setText(valueString);
}

double LineEditDouble::value()
{
    return QLocale().toDouble(this->text());
}

