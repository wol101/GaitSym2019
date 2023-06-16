#ifndef UNIQUENAMEVALIDATOR_H
#define UNIQUENAMEVALIDATOR_H

#include <QValidator>
#include <QRegularExpressionValidator>
#include <QSet>
#include <QString>

class UniqueNameValidator : public QValidator
{
    Q_OBJECT
public:
    explicit UniqueNameValidator(QObject *parent = nullptr);

    virtual QValidator::State validate(QString &input, int &pos) const;

    void addString(const QString &string);

signals:

public slots:

private:
    QSet<QString> m_setOfExistingStrings;
    QRegularExpressionValidator m_internalValidator;
};

#endif // UNIQUENAMEVALIDATOR_H
