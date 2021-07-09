#ifndef SQLHIGHLIGHTER_H
#define SQLHIGHLIGHTER_H


#include<QSyntaxHighlighter>
#include <gseq.hpp>
#include <gstring.hpp>

class SqlHighlighter : public QSyntaxHighlighter
{


public:
    SqlHighlighter( int colorScheme, QTextDocument *parent = 0, GSeq<GString> *sqlCmdList = NULL, GSeq<GString> *hostVarList = NULL);

protected:
    void highlightBlock(const QString &text);

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    int m_iColorScheme;
    QVector<HighlightingRule> highlightingRules;

    QRegExp commentStartExpression;
    QRegExp commentEndExpression;

    QRegExp sqlValStartExpression;
    QRegExp sqlValEndExpression;

    QRegExp txtValStartExpression;
    QRegExp txtValEndExpression;

    QTextCharFormat sqlKeywordFormat;
    QTextCharFormat colNameFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
    void highlightThisBlock(const QString &text, QRegExp startExp, QRegExp stopExp, QColor color);

    void deb(GString msg);
};

#endif // SQLHIGHLIGHTER_H
