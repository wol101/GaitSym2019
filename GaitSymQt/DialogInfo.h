#ifndef DIALOGINFO_H
#define DIALOGINFO_H

#include "ParseXML.h"

#include <QDialog>

class BasicXMLSyntaxHighlighter;

namespace Ui {
class DialogInfo;
}

class DialogInfo : public QDialog
{
    Q_OBJECT

public:
    explicit DialogInfo(QWidget *parent = nullptr);
    virtual ~DialogInfo() Q_DECL_OVERRIDE;

    QString editorText() const;
    void setEditorText(const QString &editorText);
    void useXMLSyntaxHighlighter();

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    void loadPreferences();
    void savePreferences();
    void setEditorFonts();

    Ui::DialogInfo *ui;

    QString m_editorText;
    BasicXMLSyntaxHighlighter *m_basicXMLSyntaxHighlighter = nullptr;
    QFont m_editorFont;

};


#endif // DIALOGINFO_H
