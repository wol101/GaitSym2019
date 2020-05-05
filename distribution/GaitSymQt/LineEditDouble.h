#ifndef LINEEDITDOUBLE_H
#define LINEEDITDOUBLE_H

#include <QLineEdit>

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

public slots:
    void textChangedSlot(const QString &text);

private:
    QString m_defaultStyleSheet;
};

#endif // LINEEDITDOUBLE_H
