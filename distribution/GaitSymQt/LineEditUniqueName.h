#ifndef LINEEDITUNIQUENAME_H
#define LINEEDITUNIQUENAME_H

#include <QLineEdit>

class UniqueNameValidator;

class LineEditUniqueName : public QLineEdit
{
    Q_OBJECT
public:
    LineEditUniqueName(QWidget *parent = nullptr);

    void addString(const QString &string);
    void addStrings(const QStringList &stringList);

public slots:
    void textChangedSlot(const QString &text);

private:
    QString m_defaultStyleSheet;
    UniqueNameValidator *m_uniqueNameValidator;
};

#endif // LINEEDITUNIQUENAME_H
