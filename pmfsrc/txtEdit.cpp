#include "txtEdit.h"
#include <QCompleter>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QtDebug>
#include <QApplication>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QScrollBar>
#include <QMimeData>
#include <QTableWidget>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QTextDocumentFragment>

#include <QUrl>


#include <gstring.hpp>
#include <gdebug.hpp>
#include <gstuff.hpp>


#ifndef max
  #define max(A,B) ((A) >(B) ? (A):(B))
#endif

#ifndef min
  #define min(A,B) ((A) <(B) ? (A):(B))
#endif


/************************************************************************************
 *
 * txtEdit class
 *
 *************************************************************************************/


TxtEdit::TxtEdit(GDebug *pGDeb, QWidget *parent, int singleLine ) : QTextEdit(parent), m_pCompleter(0)
{
    //QObject::connect(this, SIGNAL(cursorPositionChanged(int, int)),this, SLOT(txtEdCursorPositionChanged(int, int)));
    QObject::connect(this, SIGNAL(cursorPositionChanged()),this, SLOT(txtEdCursorPositionChanged()));    
    m_pGdeb = pGDeb;
    m_iSingleLineMode = singleLine;
    setLineWrapMode(NoWrap);
    if( !m_iSingleLineMode ) return;

    setTabChangesFocus(true);
    if( singleLine )
    {
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFixedHeight(sizeHint().height());
    m_iCurrentWordPos = -1;
}

TxtEdit::~TxtEdit()
{
    if(m_pCompleter)
    {
        m_pCompleter->popup()->hide();
    }

}
void TxtEdit::insertFromMimeData(const QMimeData *source)
{
    //insertFromMimeData is called when something is pasted.
    //SingleLine: Use txtEdit as QLineEdit. If pasted text has CR/LF, remove it.

    this->setAcceptRichText(false);
    QString s = source->text();
    if( m_iSingleLineMode && source->hasText() ) s = s.remove("\n").remove("\r");
    QTextEdit::insertPlainText(s); ///  :insertFromMimeData(source);
}

void TxtEdit::setSqlHighlighter(SqlHighlighter * pSqlHighlighter)
{
    m_pSqlHighlighter = pSqlHighlighter;
}
void TxtEdit::setCompleter(QCompleter *completer)
{
    if (m_pCompleter) QObject::disconnect(m_pCompleter, 0, this, 0);

    m_pCompleter = completer;
    m_pCompleter->setWidget(this);
    m_pCompleter->setCompletionMode(QCompleter::UnfilteredPopupCompletion);

    m_pCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    QObject::connect(m_pCompleter, SIGNAL(activated(QString)),this, SLOT(insertCompletion(QString)));

}

void TxtEdit::insertCompletion(QString completion)
{

    if (m_pCompleter->widget() != this)  return;
    if( completion.length() == 0 ) return;
    QTextCursor tc = textCursor();
    deb("orgpos: "+GString(tc.position()));
    tc.movePosition(QTextCursor::StartOfWord);

    int doIt = 1;
    GString selTxt;
    GString cur = textUnderCursor();
    int orgPos = tc.position();
    if( tc.movePosition(QTextCursor::Left) )
    {
        tc.select(QTextCursor::WordUnderCursor);
        selTxt = GString(tc.selectedText());
        deb("checkSel: "+GString(tc.selectedText())+"<-" );    
        deb("pos: "+GString(tc.position()));
        if( tc.position() - orgPos > (int)cur.length() ) doIt = 0;
        if( GString(tc.selectedText()) == "\"" && completion[0] == '\"' && doIt )
        {
            completion = "\" "+completion;
            tc.movePosition(QTextCursor::Right,QTextCursor::KeepAnchor);
            tc.movePosition(QTextCursor::EndOfWord,QTextCursor::KeepAnchor );

            deb("checkSel, have quote: "+GString(tc.selectedText())+"<-" );
        }

        else if( GString(tc.selectedText()) == "\"" && completion[0] != '\"' && doIt )
        {
            completion = "\" "+completion;
            tc.movePosition(QTextCursor::Right,QTextCursor::KeepAnchor);
            tc.movePosition(QTextCursor::EndOfWord,QTextCursor::KeepAnchor );

            deb("checkSel, have quote: "+GString(tc.selectedText())+"<-" );
        }

        else if( GString(tc.selectedText()).length() == 1 && doIt )
        {
            completion = tc.selectedText() + " "+ completion;
            tc.movePosition(QTextCursor::Right,QTextCursor::KeepAnchor);
            tc.movePosition(QTextCursor::EndOfWord,QTextCursor::KeepAnchor );
            deb("checkSel, have quote: "+GString(tc.selectedText())+"<-" );
        }
        else
        {
            tc.movePosition(QTextCursor::NextWord);
            tc.movePosition(QTextCursor::EndOfWord,QTextCursor::KeepAnchor );
            deb("checkSel, no quote: "+GString(tc.selectedText()) );
        }
    }
    else
    {
        tc.movePosition(QTextCursor::EndOfWord,QTextCursor::KeepAnchor );
        deb("checkSel, no left: "+GString(tc.selectedText()) );
    }

    //tc.select(QTextCursor::WordUnderCursor);
    deb("sel: "+GString(tc.selectedText()) );

    tc.removeSelectedText();
    ////tc.insertText(completion.toUpper());
    tc.insertText(completion+" ");

    //setTextCursor(tc);
    //tc.movePosition(QTextCursor::EndOfWord );
}

void TxtEdit::wheelEvent(QWheelEvent* event)
{
    if( m_iSingleLineMode) event->ignore();
    else QTextEdit::wheelEvent(event);

}

QSize TxtEdit::sizeHint() const
{

        QFontMetrics fm(font());
        int h = qMax(fm.height(), 14) + 4;
        int w = fm.width(QLatin1Char('x')) * 17 + 4;
        QStyleOptionFrameV2 opt;
        opt.initFrom(this);
        return (style()->sizeFromContents(QStyle::CT_LineEdit, &opt, QSize(w, h).expandedTo(QApplication::globalStrut()), this));
}
QString TxtEdit::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

void TxtEdit::focusInEvent(QFocusEvent *e)
{
    if (m_pCompleter) m_pCompleter->setWidget(this);
    QTextEdit::focusInEvent(e);
}

void TxtEdit::keyPressEvent(QKeyEvent *e)
{    
    deb("txtEdit, keyPress");
    if( m_iSingleLineMode)
    {
        if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
        {
            e->ignore();
            return;
        }
    }
    if( (e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_B && !m_iSingleLineMode)
    {
        formatTxt2();
        e->ignore();
        return;
    }

    if( (e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_M  && !m_iSingleLineMode)
    {
        toggleComment();
        e->ignore();
        return;
    }

    if (m_pCompleter && m_pCompleter->popup()->isVisible())
    {
        // The following keys are forwarded by the completer to the widget
        switch (e->key())
        {
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Escape:
            case Qt::Key_Tab:
            case Qt::Key_Backtab:
                e->ignore();
                return;

            default:
                break;
        }
    }

    bool isShortcut = ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_E); // CTRL+E
    if (!m_pCompleter || !isShortcut)
    {
        // do not process the shortcut when we have a completer
        QTextEdit::keyPressEvent(e);
    }

    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!m_pCompleter || (ctrlOrShift && e->text().isEmpty()))  return;

    //static QString eow("~!@#$%^&*()+{}|:\"<>?,/;'[]\\-="); // end of word
    //static QString eow("~!@#$%^&*()+{}|:<>?,/;'[]\\-="); // end of word
    static QString eow(""); // end of word
    bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 2
                      || eow.contains(e->text().right(1))))
    {
        m_pCompleter->popup()->hide();
        //deb("Ret on (1), eText: "+GString(e->text()));

        //m_pParent->keyPressEvent(e);
        //QTextEdit::keyPressEvent(e);
        //e->ignore(); //Setting this: TAB key jumps to buttons.
        return;
    }
    int pos;

    for ( pos = 0; m_pCompleter->setCurrentRow(pos); pos++)
    {
        GString cur = completionPrefix;
        GString txt = GString(m_pCompleter->currentCompletion()).strip("\"").subString(1, cur.length());
        //deb("List at "+GString(pos)+", text: "+GString(m_pCompleter->currentCompletion())+", txt: "+txt+", cur: "+cur);

        if( cur.upperCase() == txt.upperCase()  )
        {
            QRect cr = cursorRect();
            m_pCompleter->popup()->setCurrentIndex(m_pCompleter->completionModel()->index(pos, 0));
            cr.setWidth(m_pCompleter->popup()->sizeHintForColumn(0) + m_pCompleter->popup()->verticalScrollBar()->sizeHint().width());
            m_pCompleter->complete(cr); // pop it up!
            return;
        }
        else m_pCompleter->popup()->close();
    }
    return;

    // -----------------------DEAD CODE----------------------
    deb("ComplPref: "+GString(completionPrefix)+", m_pCompleter->completionPrefix(): "+GString(m_pCompleter->completionPrefix()));
    if (completionPrefix != m_pCompleter->completionPrefix())
    {
        m_pCompleter->setCompletionPrefix(completionPrefix);
        m_pCompleter->popup()->setCurrentIndex(m_pCompleter->completionModel()->index(0, 0));
    }
    if( !m_pCompleter->currentCompletion().length() )
    {
        deb("Ret on (2), curCompl:"+GString(m_pCompleter->currentCompletion())+"<-");
        return;
    }
    deb("TextUC: "+completionPrefix+", compPref: "+m_pCompleter->completionPrefix()+", currCompl: "+m_pCompleter->currentCompletion()+"<-");
    QRect cr = cursorRect();
    cr.setWidth(m_pCompleter->popup()->sizeHintForColumn(0) + m_pCompleter->popup()->verticalScrollBar()->sizeHint().width());
    m_pCompleter->complete(cr); // pop it up!
    QTextEdit::keyPressEvent(e);

}

int TxtEdit::setComment(QString &s)
{
    GString out = s;
    if( out.strip().subString(1, 2) == "--")
    {
        out = s;
        int idx = out.indexOf("--") - 1;
        s.remove(idx, 2);
        return -1;
    }
    else
    {
        s.insert(0,"--");
        return 1;
    }
    return 0;
}

void TxtEdit::toggleComment()
{
    QTextCursor cur = textCursor();
    if(!cur.hasSelection())
    {
        cur.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        cur.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        setTextCursor(cur);
    }

    int anchor  = cur.anchor();
    int pcurPos = cur.position();

    cur.setPosition(anchor);
    cur.movePosition(QTextCursor::StartOfBlock,QTextCursor::MoveAnchor);
    anchor = cur.position();
    // save a new anchor at the beginning of the line of the selected text
    cur.setPosition(anchor);
    cur.setPosition(pcurPos, QTextCursor::KeepAnchor);
    // set a new selection with the new beginning
    QString str = cur.selection().toPlainText();
    QStringList list = str.split("\n");
    // get the selected text and split into lines

    int moveCrs = 0;
    for (int i = 0; i < list.count(); i++)
    {
        if( list[i].length() == 0 ) continue;
        moveCrs += setComment(list[i]);
//        if( list[i].startsWith("--") )
//        {
//            moveCrs--;
//            list[i] = list[i].remove(0,2);
//        }
//        else
//        {
//            list[i].insert(0,"--");
//            moveCrs++;
//        }
    }
    str = list.join("\n");
    cur.removeSelectedText();
    cur.insertText(str);
    // put the new text back
    cur.setPosition(anchor);
    cur.setPosition(pcurPos+2*moveCrs, QTextCursor::KeepAnchor);
    // reselect the text for more indents

    setTextCursor(cur);
    return;
/***
    QTextCursor curs = textCursor();


    if(!curs.hasSelection())
    {
        curs.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        curs.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        setTextCursor(curs);
    }

    // Get the first and count of lines to indent.

    int spos = curs.anchor();
    int epos = curs.position();

    if (spos > epos)
    {
        std::swap(spos, epos);
    }
    curs.setPosition(spos, QTextCursor::MoveAnchor);
    int sblock = curs.block().blockNumber();

    curs.setPosition(epos, QTextCursor::MoveAnchor);
    int eblock = curs.block().blockNumber();


    curs.setPosition(spos, QTextCursor::MoveAnchor);
    curs.beginEditBlock();
    if(eblock == sblock) curs.insertText("--");

    for(int i = 0; i < (eblock - sblock); ++i)
    {
        curs.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
        if( curs.selectedText().startsWith("--") ) curs.selectedText().remove(0,2);
        else curs.insertText("--");
        curs.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor);
    }
    curs.endEditBlock();

    // Set our cursor's selection to span all of the involved lines.

    curs.setPosition(spos, QTextCursor::MoveAnchor);
    curs.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);

    while(curs.block().blockNumber() < eblock)
    {
        curs.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
    }

    curs.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    setTextCursor(curs);
    */
}


void TxtEdit::formatTxt2()
{
    deb("formatTxt2 start");
    GSeq <GString> formattedSeq;
    GString txt;

    QTextBlock qtxt;
    for(int i = 0; i < this->document()->blockCount(); ++i)
    {
        qtxt = this->document()->findBlockByLineNumber(i);        
        txt += qtxt.text()+"\n";
    }
    insertNewLine(txt, "CREATE", true);
    insertNewLine(txt, "FROM", false);
    insertNewLine(txt, "WHERE");
    insertNewLine(txt, "ORDER");
    insertNewLine(txt, "AND");
    insertNewLine(txt, "OR");
    insertNewLine(txt, "GROUP", false);
    insertNewLine(txt, "JOIN", false);

    fillLineSeq(txt, &formattedSeq);

    GString out;
    for(int i= 1; i <= formattedSeq.numberOfElements(); ++i )
    {
        txt = formattedSeq.elementAtPosition(i);
        out += GStuff::breakLongLine(txt, 70);
    }

    this->setText(out);
    deb("formatTxt2 end");
    return;
}

void TxtEdit::fillLineSeq(GString txt, GSeq<GString> *outSeq)
{
    outSeq->removeAll();

    while (txt.occurrencesOf("\n"))
    {        
        outSeq->add(txt.subString(1, txt.indexOf("\n")-1));
        txt = txt.remove(1, txt.indexOf("\n"));
    }
    outSeq->add(txt);
}


void TxtEdit::insertNewLine(GString &txt, GString token, bool after)
{
    int pos = 0;
    token = " "+token.upperCase()+" ";
    while( GString(txt).upperCase().indexOf(token, pos))
    {
        if( after ) pos = GString(txt).upperCase().indexOf(token, pos) + token.length();
        else pos  = GString(txt).upperCase().indexOf(token, pos);
        txt = txt.insert((char*)"\n", pos);
        if( !after ) pos +=  + token.length();
    }
}

void TxtEdit::formatTxt()
{
    QTextBlock qtxt;

    GString txt; // = this->document()->toPlainText();
    for(int i = 0; i < this->document()->blockCount(); ++i)
    {
        qtxt = this->document()->findBlockByLineNumber(i);
        txt += qtxt.text().trimmed()+" ";
    }
    deb("formatTxt: in: "+txt);

    //This is not case-sensitive enough, otherwise it would be perfect:
    //txt = txt.change(" WHERE ", " WHERE\n");
    //txt = txt.change(" AND ", " AND\n");
    insertNewLine(txt, "FROM", false);
    insertNewLine(txt, "WHERE");
    insertNewLine(txt, "AND");
    insertNewLine(txt, "OR");
    insertNewLine(txt, "ORDER");
    insertNewLine(txt, "GROUP", false);
    insertNewLine(txt, "JOIN", false);
    insertNewLine(txt, "CREATE", false);
    this->setText(txt);

    //txt = breakLongLines(70);
    deb("formatTxt: out: "+txt);
    //this->setText(txt);
    this->setAlignment(Qt::AlignLeft);
}


void TxtEdit::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void TxtEdit::dragMoveEvent(QDragMoveEvent *event)
{
    event->accept();
}

void TxtEdit::readFilesFromDroppedList(QDropEvent *event)
{
    const QMimeData* pMimeData = event->mimeData();
    QList< QUrl > urlList = pMimeData->urls();

    if ( !pMimeData->hasUrls() )
    {
        QMessageBox::information(this, "pmf", "This is not a file.");
        return;
    }
    //event->acceptProposedAction();
    if( urlList.size() > 0 )
    {
        //QMessageBox::information(this, "pmf", "Cannot process more than one file at a time.");
        //return;
        this->clear();
    }
    foreach (QUrl fileName, urlList)
    {
        QFile file(fileName.toLocalFile());
        file.open(QFile::ReadOnly | QFile::Text);

        QTextStream ReadFile(&file);
        this->append(ReadFile.readAll());

    }
}

void TxtEdit::dropEvent(QDropEvent *event)
{
    readFilesFromDroppedList(event);
}

void TxtEdit::deb(GString msg)
{
    if( !m_pGdeb ) return;
    m_pGdeb->debugMsg("txtEdit", 1, msg);
}

void TxtEdit::txtEdCursorPositionChanged()
{
    QTextDocument * doc = document();

    QTextCursor cursor = textCursor();
    QTextCharFormat plainFormat(cursor.charFormat());
    QTextCharFormat qtFormat = plainFormat;


    QTextCharFormat bracketMismatchFormat= plainFormat;


    qtFormat.setForeground(QColor("#F0182E"));
    QTextCharFormat bracketMatchFormat= qtFormat;


    if (!bracketBeginCursor.isNull() || !bracketEndCursor.isNull()) {
        bracketBeginCursor.setCharFormat(QTextCharFormat());
        bracketEndCursor.setCharFormat(QTextCharFormat());

        bracketBeginCursor = bracketEndCursor = QTextCursor();
    }

    int position = cursor.position();
    QTextCharFormat format = bracketMismatchFormat;

    if ((!cursor.atBlockEnd()   && doc->characterAt(position) == '(') ||
        (!cursor.atBlockStart() && doc->characterAt(position - 1) == ')')) {

        bool forward = doc->characterAt(position) == '(';

        QTextCursor::MoveOperation move;

        QChar c, begin, end;

        if (forward) { //bool forward =
            position++;
            move = QTextCursor::NextCharacter;
            begin = '(';
            end   = ')';

        } else {
            position -= 2;
            move = QTextCursor::PreviousCharacter;
            begin = ')';
            end   = '(';
        }

        bracketBeginCursor = QTextCursor(cursor);
        bracketBeginCursor.movePosition(move, QTextCursor::KeepAnchor);


        int braceDepth = 1;

        while (!(c = doc->characterAt(position)).isNull()) {
            if (c == begin) {
                braceDepth++;
            } else if (c == end) {
                braceDepth--;

                if (!braceDepth) {
                    bracketEndCursor = QTextCursor(doc);
                    bracketEndCursor.setPosition(position);
                    bracketEndCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
                    bracketEndCursor.setCharFormat(bracketMatchFormat);

                    format = bracketMatchFormat;
                    break;
                }
            }
            forward ? position++ : position--;
        }
        bracketBeginCursor.setCharFormat(format);
    }
}

void TxtEdit::createParenthesisSelection(int pos)
{
    QList<QTextEdit::ExtraSelection> selections = extraSelections();

    QTextEdit::ExtraSelection selection;
    QTextCharFormat format = selection.format;
    format.setBackground(Qt::green);
    selection.format = format;

    QTextCursor cursor = textCursor();
    cursor.setPosition(pos);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    selection.cursor = cursor;

    selections.append(selection);
    setExtraSelections(selections);
}

int TxtEdit::nextFoundWord()
{
    if( m_extraSelections.count() == 0 || m_iCurrentWordPos < 0 ) return 1;

    extraSelections().clear();

    QTextEdit::ExtraSelection extra;
    if( m_iCurrentWordPos > m_extraSelections.count()-1 ) m_iCurrentWordPos = 0;
    extra = m_extraSelections.at(m_iCurrentWordPos);
    extra.format.setBackground(Qt::yellow);

    QList<QTextEdit::ExtraSelection> extras;
    extras << extra;
    setExtraSelections( extras );
    this->ensureCursorVisible() ;
    this->setTextCursor(extra.cursor);
    m_iCurrentWordPos++;
    return 0;


//    QTextCursor cursor = textCursor();
//    //extra.format.setBackground(color);
//    cursor.setPosition(extra.cursor.anchor(), QTextCursor::MoveAnchor);
//    cursor.setPosition(extra.cursor.position(), QTextCursor::KeepAnchor);
//    printf("SetCRS to %i\n", extra.cursor.position());

//    QTextCharFormat fmt;
//    fmt.setBackground(Qt::yellow);
//    cursor.setCharFormat(fmt);


////    QTextCharFormat fmt;
////    fmt.setBackground(Qt::yellow);

////    cursor.setPosition(begin, QTextCursor::MoveAnchor);
////    cursor.setPosition(end, QTextCursor::KeepAnchor);
////    cursor.setCharFormat(fmt);
}

int TxtEdit::findWordOccurrences(QString textToFind, int caseSensitive)
{

    m_extraSelections.clear();
    if (document()) {
        QTextDocument::FindFlags flags;
        if( caseSensitive ) flags = QTextDocument::FindCaseSensitively;
       QTextCursor cursor(document());
       cursor.setPosition(0);
       cursor = document() -> find(textToFind, cursor, flags);

       while (! cursor.isNull()) {
          QTextEdit::ExtraSelection extra;
          extra.cursor = cursor;
          m_extraSelections.append(extra);

          cursor = document() -> find(textToFind, cursor, flags);
       }
       //setExtraSelections(m_extraSelections);
       if( m_extraSelections.count() )
       {
           m_iCurrentWordPos = 0;
           nextFoundWord();
       }
       return m_extraSelections.count();
    }
    return 0;
}

int TxtEdit::highlightWord(QString highlighText, int caseSensitive, int offset )
{

    if (document()) {
       //QList<QTextEdit::ExtraSelection> extraSelections;
       QColor color(Qt::lightGray);

       QTextCursor cursor(document());
       cursor.setPosition(offset);
       //cursor = document() -> find(highlighText, cursor, flags);
       cursor = document() -> find(highlighText, cursor);

       while (! cursor.isNull())
       {
          QTextEdit::ExtraSelection extra;
          extra.format.setBackground(color);
          extra.cursor = cursor;
          m_extraSelections.append(extra);

          //cursor = document() -> find(txt, cursor, flags);
          cursor = document() -> find(highlighText, cursor);
       }
       setExtraSelections(m_extraSelections);
       return m_extraSelections.count();
    }
    return 0;

/*

    if( !caseSensitive ) highlighText = highlighText.toUpper();
    QList<QTextEdit::ExtraSelection> extraSelections;

    QBrush backBrush( Qt::yellow );
    QBrush textBrush( Qt::black );
    QPen outlinePen( Qt::gray, 1 );
    int pos = -1;

    for( int i=0; i < this->document()->blockCount(); i++ )
    {
        QTextBlock block = this->document()->findBlockByNumber( i );
        if( block.isValid() )
        {
            QString text = block.text();
            int p;
            if( !caseSensitive ) p = text.toUpper().indexOf(highlighText, offset);
            else p = text.indexOf(highlighText);
            printf("txtEdit: offset: %i, p: %i\n", offset, p);

            if( p != -1 )
            {
                pos = block.position() + p;

                QTextEdit::ExtraSelection selection;
                selection.cursor = QTextCursor(this->document());
                selection.cursor.setPosition( pos );
                selection.cursor.setPosition( pos+highlighText.length(), QTextCursor::KeepAnchor );
                selection.format.setBackground( backBrush );
                selection.format.setForeground( textBrush );
                selection.format.setProperty( QTextFormat::OutlinePen, outlinePen );

                extraSelections.append( selection );
                this->ensureCursorVisible() ;
                this->setTextCursor(selection.cursor);
                if (offset >= 0 ) return p;

            }
        }
    }
    this->setExtraSelections( extraSelections );
    return pos;
*/
}

