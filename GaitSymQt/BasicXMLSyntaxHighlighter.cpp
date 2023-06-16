#include "BasicXMLSyntaxHighlighter.h"

BasicXMLSyntaxHighlighter::BasicXMLSyntaxHighlighter(QObject * parent) :
    QSyntaxHighlighter(parent)
{
    setRegexes();
    setFormats();
}

BasicXMLSyntaxHighlighter::BasicXMLSyntaxHighlighter(QTextDocument * parent) :
    QSyntaxHighlighter(parent)
{
    setRegexes();
    setFormats();
}

BasicXMLSyntaxHighlighter::BasicXMLSyntaxHighlighter(QTextEdit * parent) :
    QSyntaxHighlighter(parent)
{
    setRegexes();
    setFormats();
}

void BasicXMLSyntaxHighlighter::highlightBlock(const QString & text)
{
    // Special treatment for xml element regex as we use captured text to emulate lookbehind
    QRegularExpressionMatch match = m_xmlElementRegex.match(text);
    int xmlElementIndex = match.capturedStart(0);
    while(xmlElementIndex >= 0)
    {
        int matchedPos =  match.capturedStart(1);
        int matchedLength = match.captured(1).length();
        setFormat(matchedPos, matchedLength, m_xmlElementFormat);

        match = m_xmlElementRegex.match(text, matchedPos + matchedLength);
        xmlElementIndex = match.capturedStart(0);
    }

    // Highlight xml keywords *after* xml elements to fix any occasional / captured into the enclosing element
    typedef QList<QRegularExpression>::const_iterator Iter;
    Iter xmlKeywordRegexesEnd = m_xmlKeywordRegexes.end();
    for(Iter it = m_xmlKeywordRegexes.begin(); it != xmlKeywordRegexesEnd; ++it)
    {
        const QRegularExpression & regex = *it;
        highlightByRegex(m_xmlKeywordFormat, regex, text);
    }

    highlightByRegex(m_xmlAttributeFormat, m_xmlAttributeRegex, text);
    highlightByRegex(m_xmlCommentFormat, m_xmlCommentRegex, text);
    highlightByRegex(m_xmlValueFormat, m_xmlValueRegex, text);
}

void BasicXMLSyntaxHighlighter::highlightByRegex(const QTextCharFormat & format,
                                                 const QRegularExpression & regex, const QString & text)
{
    QRegularExpressionMatch match = regex.match(text);
    int index = match.capturedStart(0);

    while(index >= 0)
    {
        int matchedLength = match.captured(0).length();
        setFormat(index, matchedLength, format);

        match = regex.match(text, index + matchedLength);
        index = match.capturedStart(0);
    }
}

void BasicXMLSyntaxHighlighter::setRegexes()
{
    m_xmlElementRegex.setPattern("<[?\\s]*[/]?[\\s]*([^\\n][^>]*)(?=[\\s/>]|$)");
    m_xmlAttributeRegex.setPattern("\\w+(?=\\=)");
    m_xmlValueRegex.setPattern("\"[^\\n\"]+\"(?=[?\\s/>]|$)");
    m_xmlCommentRegex.setPattern("<!--[^\\n]*-->");

    m_xmlKeywordRegexes = QList<QRegularExpression>() << QRegularExpression("<\\?") << QRegularExpression("/>")
                                                      << QRegularExpression(">") << QRegularExpression("<")
                                                      << QRegularExpression("</") << QRegularExpression("\\?>");
}

void BasicXMLSyntaxHighlighter::setFormats()
{
    m_xmlKeywordFormat.setForeground(Qt::blue);
    m_xmlKeywordFormat.setFontWeight(QFont::Bold);

    m_xmlElementFormat.setForeground(Qt::darkMagenta);
    m_xmlElementFormat.setFontWeight(QFont::Bold);

    m_xmlAttributeFormat.setForeground(Qt::darkGreen);
    m_xmlAttributeFormat.setFontWeight(QFont::Bold);
    m_xmlAttributeFormat.setFontItalic(true);

    m_xmlValueFormat.setForeground(Qt::darkRed);

    m_xmlCommentFormat.setForeground(Qt::gray);
}

