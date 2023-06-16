#ifndef DIALOGCREATETESTINGDRIVERS_H
#define DIALOGCREATETESTINGDRIVERS_H

#include "ParseXML.h"

#include <QDialog>

class BasicXMLSyntaxHighlighter;

namespace Ui {
class DialogCreateTestingDrivers;
}

class DialogCreateTestingDrivers : public QDialog
{
    Q_OBJECT

public:
    explicit DialogCreateTestingDrivers(QWidget *parent = nullptr);
    virtual ~DialogCreateTestingDrivers() Q_DECL_OVERRIDE;

    QString editorText() const;
    void setEditorText(const QString &editorText);
    void useXMLSyntaxHighlighter();

    void setFileName(const QString &fileName);

    bool isModified() const;
    void setModified(bool modified);

public slots:
    virtual void accept() Q_DECL_OVERRIDE;
    virtual void reject() Q_DECL_OVERRIDE;

private slots:
    void apply();
    void textChanged(const QString &text);
    void modificationChanged(bool changed);

private:
    void loadPreferences();
    void savePreferences();
    void setEditorFonts();
    void enableControls();
    std::string *validate();
    void applyCreateTestingDrivers(double activationTime, double activationValue, const std::string &suffix);

    Ui::DialogCreateTestingDrivers *ui;

    QString m_editorText;
    BasicXMLSyntaxHighlighter *m_basicXMLSyntaxHighlighter = nullptr;
    QFont m_editorFont;

    ParseXML m_parseXML;
    std::string m_lastError;

};

#endif // DIALOGCREATETESTINGDRIVERS_H
