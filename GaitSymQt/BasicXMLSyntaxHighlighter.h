#ifndef BASIC_XML_SYNTAX_HIGHLIGHTER_H
#define BASIC_XML_SYNTAX_HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextEdit>

#include <QRegularExpression>

class BasicXMLSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    BasicXMLSyntaxHighlighter(QObject * parent);
    BasicXMLSyntaxHighlighter(QTextDocument * parent);
    BasicXMLSyntaxHighlighter(QTextEdit * parent);

protected:
    virtual void highlightBlock(const QString & text);

private:
    void highlightByRegex(const QTextCharFormat & format,
                          const QRegularExpression & regex, const QString & text);

    void setRegexes();
    void setFormats();

private:
    QTextCharFormat     m_xmlKeywordFormat;
    QTextCharFormat     m_xmlElementFormat;
    QTextCharFormat     m_xmlAttributeFormat;
    QTextCharFormat     m_xmlValueFormat;
    QTextCharFormat     m_xmlCommentFormat;

    QList<QRegularExpression>      m_xmlKeywordRegexes;
    QRegularExpression             m_xmlElementRegex;
    QRegularExpression             m_xmlAttributeRegex;
    QRegularExpression             m_xmlValueRegex;
    QRegularExpression             m_xmlCommentRegex;
};

#endif // BASIC_XML_SYNTAX_HIGHLIGHTER_H
