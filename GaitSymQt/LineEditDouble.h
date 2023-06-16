#ifndef LINEEDITDOUBLE_H
#define LINEEDITDOUBLE_H

#include <QLineEdit>
#include <QDoubleValidator>

class LineEditDouble : public QLineEdit
{
    Q_OBJECT
public:
    LineEditDouble(QWidget *parent = nullptr);

    void setValue(double value);
    double value();

    void setBottom(double bottom);
    void setTop(double top);
    void setDecimals(int decimals);
    void setNotation(QDoubleValidator::Notation notation);

public slots:
    void textChangedSlot(const QString &text);
    void menuRequestPath(const QPoint &pos);

private:
    QString m_defaultStyleSheet;
};

#endif // LINEEDITDOUBLE_H
