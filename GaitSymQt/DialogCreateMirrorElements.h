#ifndef DIALOGCREATEMIRRORELEMENTS_H
#define DIALOGCREATEMIRRORELEMENTS_H

#include "ParseXML.h"

#include <QDialog>

class BasicXMLSyntaxHighlighter;

namespace Ui {
class DialogCreateMirrorElements;
}

class DialogCreateMirrorElements : public QDialog
{
    Q_OBJECT

public:
    explicit DialogCreateMirrorElements(QWidget *parent = nullptr);
    virtual ~DialogCreateMirrorElements() Q_DECL_OVERRIDE;

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
    void applyMirrorCreate(const std::string &fromString, const std::string &toString, size_t axis);
    bool attributeFind(const std::string &refStr, const std::string &findStr);
    std::string attributeReplace(const std::string input, const std::string &before, const std::string &after);
    std::string mirrorString(const std::string &inputString, size_t tokenNumber);
    std::string mirrorMOIString(const std::string &inputString, size_t axis);
    std::string mirrorRotationRange(const std::string &inputString);

    Ui::DialogCreateMirrorElements *ui;

    QString m_editorText;
    BasicXMLSyntaxHighlighter *m_basicXMLSyntaxHighlighter = nullptr;
    QFont m_editorFont;

    ParseXML m_parseXML;
    std::string m_lastError;

};

#endif // DIALOGCREATEMIRRORELEMENTS_H
