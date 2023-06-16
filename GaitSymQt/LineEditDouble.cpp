#include "LineEditDouble.h"
#include "DoubleValidator.h"

#include <QDebug>
#include <QLocale>
#include <QMenu>

#include <limits>
#include <cmath>

LineEditDouble::LineEditDouble(QWidget *parent) :
    QLineEdit(parent)
{
    DoubleValidator *doubleValidator = new DoubleValidator();
    doubleValidator->setNotation(QDoubleValidator::ScientificNotation);
    // not really sure whether I should use max_digits10 [17] or digits_10 [15]
    doubleValidator->setRange(-std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max_digits10);
    this->setValidator(doubleValidator);
    connect(this, SIGNAL(textChanged(const QString &)), this, SLOT(textChangedSlot(const QString &)));

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(menuRequestPath(QPoint)));

    m_defaultStyleSheet = this->styleSheet();
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
    double top = v->top();
    int decimals = v->decimals();
    if (top <= bottom) top = std::nextafter(bottom, std::numeric_limits<double>::max());
    QDoubleValidator::Notation notation = v->notation();
    DoubleValidator *doubleValidator = new DoubleValidator();
    doubleValidator->setNotation(notation);
    doubleValidator->setRange(bottom, top, decimals);
    setValidator(doubleValidator);
}

void LineEditDouble::setTop(double top)
{
    const DoubleValidator *v = dynamic_cast<const DoubleValidator *>(this->validator());
    Q_ASSERT(v);
    double bottom = v->bottom();
    int decimals = v->decimals();
    if (bottom >= top) bottom = std::nextafter(top, -std::numeric_limits<double>::max());
    QDoubleValidator::Notation notation = v->notation();
    DoubleValidator *doubleValidator = new DoubleValidator();
    doubleValidator->setNotation(notation);
    doubleValidator->setRange(bottom, top, decimals);
    setValidator(doubleValidator);
}

void LineEditDouble::setDecimals(int decimals)
{
    const DoubleValidator *v = dynamic_cast<const DoubleValidator *>(this->validator());
    Q_ASSERT(v);
    double bottom = v->bottom();
    double top = v->top();
    QDoubleValidator::Notation notation = v->notation();
    DoubleValidator *doubleValidator = new DoubleValidator();
    doubleValidator->setNotation(notation);
    doubleValidator->setRange(bottom, top, decimals);
    setValidator(doubleValidator);
}

void LineEditDouble::setNotation(QDoubleValidator::Notation notation)
{
    const DoubleValidator *v = dynamic_cast<const DoubleValidator *>(this->validator());
    Q_ASSERT(v);
    double bottom = v->bottom();
    double top = v->top();
    int decimals = v->decimals();
    DoubleValidator *doubleValidator = new DoubleValidator();
    doubleValidator->setNotation(notation);
    doubleValidator->setRange(bottom, top, decimals);
    setValidator(doubleValidator);
}

void LineEditDouble::setValue(double value)
{
    // do some checks for validity
    while (!std::isfinite(value))
    {
        if (value == std::numeric_limits<double>::infinity()) { value = std::numeric_limits<double>::max(); break; }
        if (value == -std::numeric_limits<double>::infinity()) { value = -std::numeric_limits<double>::max(); break; }
        value = 0.0;
        break;
    }
    // there are potential round trip problems here if value is equal to either bottom or top
    const DoubleValidator *validator = static_cast<const DoubleValidator *>(this->validator());
    QString valueString;
    int decimals = validator->decimals();
    valueString.setNum(value, 'g', decimals);
    int pos = 0;
    if (validator->validate(valueString, pos) != QValidator::Acceptable)
    {
        if (decimals > std::numeric_limits<double>::max_digits10) valueString.setNum(value, 'e', std::numeric_limits<double>::max_digits10);
        else valueString.setNum(value, 'e', decimals);
    }
    this->setText(valueString);
}

double LineEditDouble::value()
{
    bool ok;
    double value = QLocale().toDouble(this->text(), &ok);
    while (!ok)
    {
        if (value == std::numeric_limits<double>::infinity()) { value = std::numeric_limits<double>::max(); break; }
        if (value == -std::numeric_limits<double>::infinity()) { value = -std::numeric_limits<double>::max(); break; }
        value = 0.0;
        break;
    }
    return value;
}

void LineEditDouble::menuRequestPath(const QPoint &pos)
{
    QMenu *menu = this->createStandardContextMenu();
    menu->addSeparator();
    menu->addAction(tr("Insert Minimum"));
    menu->addAction(tr("Insert Maximum"));
//    menu->addAction(tr("Scientific Notation"));
//    menu->addAction(tr("Standard Notation"));
    menu->addAction(tr("Float Precision"));
    menu->addAction(tr("Double Precision"));
    QPoint gp = this->mapToGlobal(pos);
    QAction *action = menu->exec(gp);
    while (action)
    {
        if (action->text() == tr("Insert Minimum")) { const DoubleValidator *v = dynamic_cast<const DoubleValidator *>(this->validator()); if (v) setValue(v->bottom()); break; }
        if (action->text() == tr("Insert Maximum")) { const DoubleValidator *v = dynamic_cast<const DoubleValidator *>(this->validator()); if (v) setValue(v->top()); break; }
//        if (action->text() == tr("Scientific Notation")) { setNotation(QDoubleValidator::ScientificNotation); setValue(value()); break; }
//        if (action->text() == tr("Standard Notation")) { setNotation(QDoubleValidator::StandardNotation); setValue(value()); break; }
        if (action->text() == tr("Float Precision")) { setDecimals(std::numeric_limits<float>::max_digits10); setValue(value()); break; }
        if (action->text() == tr("Double Precision")) { setDecimals(std::numeric_limits<double>::max_digits10); setValue(value()); break; }
        break;
    }
    delete menu;
}

