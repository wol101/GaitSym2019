#ifndef TEXTEDITDIALOG_H
#define TEXTEDITDIALOG_H

#include "ParseXML.h"

#include <QDialog>

class BasicXMLSyntaxHighlighter;
class MainWindow;
class QTableWidget;
class QTableWidgetItem;

namespace Ui {
class TextEditDialog;
}

class TextEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TextEditDialog(QWidget *parent = nullptr);
    ~TextEditDialog() Q_DECL_OVERRIDE;

    QString editorText() const;
    void setEditorText(const QString &editorText);
    void useXMLSyntaxHighlighter();

    void setFileName(const QString &fileName);

    bool isModified() const;
    void setModified(bool modified);

public slots:
    virtual void accept() Q_DECL_OVERRIDE;
    virtual void reject() Q_DECL_OVERRIDE;

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private slots:
    void find();
    void replace();
    void replaceAll();
    void saveAs();
    void attributeMachineInsert();
    void attributeMachineDelete();
    void attributeMachineMoveUp();
    void attributeMachineMoveDown();
    void attributeMachineItemSelectionChanged();
    void attributeMachineSave();
    void attributeMachineLoad();
    void attributeMachineApply();
    void resetPositions();
    void modificationChanged(bool changed);

private:
    bool replace(const QString &findString, const QString &replaceString);
    void loadPreferences();
    void savePreferences();
    void setEditorFonts();
    void enableControls();
    void attributeMachineApplyRow(const std::string &action, const std::string &matchTag, const std::string &matchAttribute, const std::string &matchAttributeValue,
                                  const std::string &changeAttribute, const std::string &tokenNumber, const std::string &search, const std::string &replace, const std::string &arithmetic);
    void attributeMachineApplyRowEdit(const std::string &matchTag, const std::string &matchAttribute, const std::string &matchAttributeValue,
                                      const std::string &changeAttribute, const std::string &tokenNumber, const std::string &search, const std::string &replace, const std::string &arithmetic);
    void attributeMachineApplyRowCreate(const std::string &matchTag, const std::string &matchAttribute, const std::string &matchAttributeValue,
                                        const std::string &changeAttribute, const std::string &tokenNumber, const std::string &search, const std::string &replace, const std::string &arithmetic);
    void attributeMachineApplyRowDelete(const std::string &matchTag, const std::string &matchAttribute, const std::string &matchAttributeValue);
    std::string attributeMachineArithmetic(const std::string &original, const std::string &arithmetic);
    bool attributeMachineMatch(const std::string &refStr, const std::string &findStr);
    std::string attributeMachineReplace(const std::string input, const std::string &before, const std::string &after);
    std::string *validate();
    QString attributeMachineSaveToString();
    void attributeMachineLoadFromString(const QString &string, bool quiet);

    Ui::TextEditDialog *ui;

    QString m_editorText;
    BasicXMLSyntaxHighlighter *m_basicXMLSyntaxHighlighter = nullptr;
    QFont m_editorFont;

    ParseXML m_parseXML;
    std::string m_lastError;
    QString m_fileName;
};

#endif // TEXTEDITDIALOG_H
