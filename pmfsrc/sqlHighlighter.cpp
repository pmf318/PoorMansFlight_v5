

#include "sqlHighlighter.h"
#include <gseq.hpp>

#include <QMessageBox>

SqlHighlighter::SqlHighlighter( int colorScheme, QTextDocument *parent, GSeq<GString> *sqlCmdList, GSeq<GString> *hostVarList)  : QSyntaxHighlighter(parent)
{
    m_iColorScheme = colorScheme;
    HighlightingRule sqlSyntaxRule;
    HighlightingRule colNamesRule;

    if(m_iColorScheme) sqlKeywordFormat.setForeground(Qt::darkBlue);
    //sqlKeywordFormat.setFontWeight(QFont::Bold);

    if(m_iColorScheme) colNameFormat.setForeground(Qt::darkGreen);
    //colNameFormat.setFontWeight(QFont::Bold);


    QStringList sqlKeywordPatterns;
    QStringList colNamesPatterns;


    if( hostVarList != NULL)
    {
        for( int i = 1; i <= (int)hostVarList->numberOfElements(); ++i)
        {
            if( !hostVarList->elementAtPosition(i).strip().length() ) continue;
            colNamesPatterns << "\\b"+hostVarList->elementAtPosition(i).strip("\"")+"\\b";
        }
    }

    if( sqlCmdList != NULL )
    {
        for( int i = 1; i <= (int)sqlCmdList->numberOfElements(); ++i)
        {
            if( !sqlCmdList->elementAtPosition(i).strip().length() ) continue;
            sqlKeywordPatterns << "\\b"+sqlCmdList->elementAtPosition(i)+"\\b";
        }

    }

    foreach (const QString &pattern, sqlKeywordPatterns)
    {
        sqlSyntaxRule.pattern = QRegExp(pattern,  Qt::CaseInsensitive);
        sqlSyntaxRule.format = sqlKeywordFormat;
        highlightingRules.append(sqlSyntaxRule);
    }

    foreach (const QString &pattern, colNamesPatterns)
    {
        colNamesRule.pattern = QRegExp(pattern,  Qt::CaseInsensitive);
        colNamesRule.format = colNameFormat;
        highlightingRules.append(colNamesRule);
    }
    /*
    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(Qt::darkMagenta);
    sqlSyntaxRule.pattern = QRegExp("\\b[A-Za-z0-9_]+\\b");
    sqlSyntaxRule.format = classFormat;
    highlightingRules.append(sqlSyntaxRule);

    quotationFormat.setForeground(Qt::darkGreen);
    sqlSyntaxRule.pattern = QRegExp("\".*\"");
    sqlSyntaxRule.format = quotationFormat;
    highlightingRules.append(sqlSyntaxRule);


    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
    sqlSyntaxRule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    sqlSyntaxRule.format = functionFormat;
    highlightingRules.append(sqlSyntaxRule);
    */
    if(m_iColorScheme) singleLineCommentFormat.setForeground(Qt::darkGray);
    sqlSyntaxRule.pattern = QRegExp("--[^\n]*");
    sqlSyntaxRule.format = singleLineCommentFormat;
    highlightingRules.append(sqlSyntaxRule);

    if(m_iColorScheme) multiLineCommentFormat.setForeground(Qt::red);

    sqlValStartExpression= QRegExp("'");
    sqlValEndExpression= QRegExp("'");

    txtValStartExpression= QRegExp("\"");
    txtValEndExpression= QRegExp("\"");

    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");
}
void SqlHighlighter::deb(GString msg)
{
    return;
    printf("SqlHighlighter: %s\n", (char*) msg);
#if defined(MAKE_VC) || defined (__MINGW32__)
    _flushall();
#endif
}

void SqlHighlighter::highlightBlock(const QString &text)
{
    if( m_iColorScheme )
    {
        highlightThisBlock(text, sqlValStartExpression, sqlValEndExpression, Qt::darkRed );
        highlightThisBlock(text, txtValStartExpression, txtValEndExpression, Qt::darkGreen );
    }
    else
    {
        highlightThisBlock(text, sqlValStartExpression, sqlValEndExpression, QColor(204, 51, 153));
        highlightThisBlock(text, txtValStartExpression, txtValEndExpression, QColor(204, 0, 0) );
    }
}

void SqlHighlighter::highlightThisBlock(const QString &text, QRegExp startExp, QRegExp stopExp, QColor color)
{
    deb("highlightThisBlock start");
    foreach (const HighlightingRule &rule, highlightingRules)
    {
        QRegExp expression(rule.pattern);
        int index = expression.indexIn(text);
        deb("highlightThisBlock hlRule index: "+GString(index));
        while (index >= 0)
        {
            int length = expression.matchedLength();
            if( length <= 0 ) break;
            setFormat(index, length, rule.format);
            index = expression.indexIn(text, index + length);
        }
    }
    setCurrentBlockState(0);
    int startIndex = 0;
    if (previousBlockState() != 1) startIndex = startExp.indexIn(text);
    deb("highlightThisBlock start while...");
    while (startIndex >= 0)
    {
        int endIndex = stopExp.indexIn(text, startIndex+1);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        }
        else
        {
            commentLength = endIndex - startIndex  + stopExp.matchedLength();
        }
        //setFormat(startIndex, commentLength, multiLineCommentFormat);
        setFormat(startIndex, commentLength, color);
        startIndex = startExp.indexIn(text, startIndex + commentLength);
    }
    deb("highlightThisBlock done");

}



