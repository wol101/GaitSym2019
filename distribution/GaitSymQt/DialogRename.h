#ifndef DIALOGRENAME_H
#define DIALOGRENAME_H

#include "ParseXML.h"

#include <QDialog>

class BasicXMLSyntaxHighlighter;
class NamedObject;

namespace Ui {
class DialogRename;
}

class DialogRename : public QDialog
{
    Q_OBJECT

public:
    explicit DialogRename(QWidget *parent = nullptr);
    virtual ~DialogRename() Q_DECL_OVERRIDE;

    QString editorText() const;
    void setEditorText(const QString &editorText);
    void useXMLSyntaxHighlighter();

    void setFileName(const QString &fileName);

    bool isModified() const;
    void setModified(bool modified);

    void setNameList(std::vector<NamedObject *> *nameList);

public slots:
    virtual void accept() Q_DECL_OVERRIDE;
    virtual void reject() Q_DECL_OVERRIDE;

private slots:
    void apply();
    void textChanged(const QString &text);
    void modificationChanged(bool changed);
    void currentTextChanged(const QString &text);

private:
    void loadPreferences();
    void savePreferences();
    void setEditorFonts();
    void enableControls();
    std::string *validate();
    void applyRename(const std::string &fromString, const std::string &toString);
    bool attributeFind(const std::string &refStr, const std::string &findStr);
    std::string attributeReplace(const std::string input, const std::string &before, const std::string &after);

    Ui::DialogRename *ui;

    QString m_editorText;
    BasicXMLSyntaxHighlighter *m_basicXMLSyntaxHighlighter = nullptr;
    QFont m_editorFont;

    std::vector<NamedObject *> *m_nameList = nullptr;

    ParseXML m_parseXML;
    std::string m_lastError;

};

#endif // DIALOGRENAME_H
