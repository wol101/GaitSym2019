#ifndef TEXTEDITDIALOG_H
#define TEXTEDITDIALOG_H

#include <QDialog>

class BasicXMLSyntaxHighlighter;

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

public slots:
    virtual void accept() Q_DECL_OVERRIDE;
    virtual void reject() Q_DECL_OVERRIDE;

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private slots:
    void find();
    void replace();

private:
    void setEditorFonts();

    Ui::TextEditDialog *ui;

    QString m_editorText;
    BasicXMLSyntaxHighlighter *m_basicXMLSyntaxHighlighter = nullptr;
    QFont m_editorFont;
};

#endif // TEXTEDITDIALOG_H
