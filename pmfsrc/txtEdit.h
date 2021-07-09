
#ifndef _txtEdit_
#define _txtEdit_

#include <QTextEdit>

#include "sqlHighlighter.h"
#include <gstring.hpp>
#include <gdebug.hpp>
#include "gseq.hpp"


class QCompleter;

class TxtEdit : public QTextEdit
{
    Q_OBJECT


public:
    TxtEdit(GDebug * pGDeb, QWidget *parent = 0, int singleLine = 0 );
    ~TxtEdit();

    void setCompleter(QCompleter *cmpl);
    void setSqlHighlighter(SqlHighlighter * pSqlHighlighter);

    void insertNewLine(GString  &txt, GString token, bool after = true);
    void fillLineSeq(GString txt, GSeq<GString> *outSeq);
    void readFilesFromDroppedList(QDropEvent *event);
    int highlightWord(QString highlighText, int caseSensitive = 0, int offset = 0);
    int findWordOccurrences(QString textToFind, int caseSensitive);
    int nextFoundWord();

protected:
    void keyPressEvent(QKeyEvent *e);
    void focusInEvent(QFocusEvent *e);

protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);



private slots:
    void insertCompletion(QString completion);
    void txtEdCursorPositionChanged();

private:
    QString textUnderCursor() const;
    QSize sizeHint() const;
    void wheelEvent(QWheelEvent* event);
    QTextCursor bracketBeginCursor, bracketEndCursor;
    void createParenthesisSelection(int pos);
    QList<QTextEdit::ExtraSelection> m_extraSelections;

private:

    QCompleter *m_pCompleter;
    SqlHighlighter * m_pSqlHighlighter;
    void deb(GString msg);
    void formatTxt();
    void formatTxt2();
    int m_iSingleLineMode;
    void insertFromMimeData(const QMimeData *source);
    GDebug *m_pGdeb;
    QWidget * m_pParent;
    void toggleComment();
    int setComment(QString &s);
    int m_iCurrentWordPos;
};

#endif
