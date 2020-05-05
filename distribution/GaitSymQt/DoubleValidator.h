#ifndef DOUBLEVALIDATOR_H
#define DOUBLEVALIDATOR_H

#include <QDoubleValidator>

class DoubleValidator : public QDoubleValidator
{
    Q_OBJECT
public:
    explicit DoubleValidator(QObject *parent = nullptr);
    explicit DoubleValidator(double bottom, double top, int decimals, QObject *parent = nullptr);

    virtual QValidator::State validate(QString &input, int &pos) const;

signals:

public slots:
};

#endif // DOUBLEVALIDATOR_H
