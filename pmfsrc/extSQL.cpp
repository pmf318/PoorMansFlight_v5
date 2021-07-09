
//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)
//

#include "extSQL.h"
#include "helper.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QCompleter>
#include "pmfdefines.h"
#include <gstuff.hpp>
#include <QTime>
#include <QList>
#include <QMimeData>
#include "clickLabel.h"
#include "bookmark.h"

#define red 1
#define green 2
#define hideIt 3

ExtSQL::ExtSQL(GDebug* pGDeb, DSQLPlugin * pDSQL, TabEdit *parent, int fullMode)
  :QDialog(parent)
{    
    m_pGDeb = pGDeb;
    m_pDSQL = pDSQL;
	//setStyleSheet("background:transparent");
	//setAttribute(Qt::WA_TranslucentBackground);
	
	QBoxLayout *topLayout = new QVBoxLayout(this);
	QGridLayout * grid = new QGridLayout( );
	topLayout->addLayout(grid, 2);

	m_Master = parent;
	
	this->resize(640, 480);
	this->setWindowTitle("Enter (valid) SQL Command");



	QLabel * pLB = new QLabel(this);    
	pLB->setText("Editor for multilined SQL statements. Use -- (double dash) for comments.");
	QLabel * pLB2 = new QLabel(this);
    pLB2->setText("Hit CTRL+M to toggle comments, CTRL+E for text completion, CTRL+B to format text");
    QLabel * pLB3 = new QLabel(this);
    QFont font  = pLB3->font();
    font.setBold(true);

    pLB3->setText("You can also drag & drop files into the editor below");

	
	/********************************************
	* fullMode: Display two more buttons.
	* extSQL gets called from Snapshot too, 
	* but w/o these buttons.
	*/
	if( fullMode )
	{
		ok = new QPushButton(this);
        ok->setDefault(true);
		ok ->setText("Run");
		//ok->setGeometry(20, 360, btWt, btHt);
		connect(ok, SIGNAL(clicked()), SLOT(runClicked()));

        saveAsBookmark = new QPushButton("Save as bookmark", this);
        connect(saveAsBookmark, SIGNAL(clicked()), SLOT(saveAsBookmarkClicked()));

        bookmarkCB = new QComboBox(this);
        connect(bookmarkCB, SIGNAL(activated(int)), SLOT(bookmarkSelected(int)));


//		load = new QPushButton(this);
//		load->setText("Load From File");
//		connect(load, SIGNAL(clicked()), SLOT(loadClicked()));
	}
    //save = new QPushButton(this);
    //save->setText("Save To File");
    //connect(save, SIGNAL(clicked()), SLOT(saveClicked()));

	exit = new QPushButton(this);
	exit->setText("Exit");
	connect(exit, SIGNAL(clicked()), SLOT(exitClicked()));

    editor = new TxtEdit(m_pGDeb, this);
#if QT_VERSION >= 0x050000
    //editor->setPlaceholderText("Hit CTRL+E for text completion");
#endif

    //editor ->setGeometry( 20, 20, 720, 300);
    setCmd("");

    pStTermLE = new QLineEdit(this);
    pStTermLE->setText(";");
    pStTermLE->setFixedWidth(25);
    pStTermLE->setMaxLength(1);

    infoLabel = new QLabel(this);

    //FIND Box
    QHBoxLayout *findLayout = new QHBoxLayout;
    QGroupBox * findGroupBox = new QGroupBox();
    QLabel * findLabel = new QLabel(this);
    findLabel->setText("Find (next: F3):");
    findLayout->addWidget(findLabel);
    findLE = new QLineEdit(this);
    findLayout->addWidget(findLE);
    m_cbCaseSensitive = new QCheckBox("Case sens.");
    findLayout->addWidget(m_cbCaseSensitive);
    findGroupBox->setLayout(findLayout);
    m_iLastFindPos = 0;



	/*** Add to grid ***/
    grid->addWidget(pLB, 0, 0, 1, 6);
    grid->addWidget(pLB2, 1, 0, 1, 6);
    grid->addWidget(pLB3, 2, 0, 1, 6);
    grid->addWidget(editor, 3, 0, 1, 6);
    //grid->addWidget(save, 5, 4);
    grid->addWidget(exit, 5, 3);
    grid->addWidget(findGroupBox, 6, 0, 1, 6);

    if( fullMode )
    {
        grid->addWidget(ok, 5, 0);
        grid->addWidget(saveAsBookmark, 5, 1);
        grid->addWidget(bookmarkCB, 5, 2);
        //grid->addWidget(load, 5, 3);

        grid->addWidget(infoLabel, 4, 0, 1, 6);
        ClickLabel * clickMe = new ClickLabel("Statement terminator ", this);
        clickMe->setDisplayText(helpTerminatorClicked());
        grid->addWidget(clickMe, 5, 4);
        grid->addWidget(pStTermLE, 5, 5);
    }
    infoLabel->setStyleSheet("QLabel { color : red; }");
    infoLabel->hide();
	m_qrGeometry = this->geometry();
    m_Master->initTxtEditor(editor);
    loadBookmarks();
    editor->setFocus();

}

void ExtSQL::loadBookmarks()
{
    Bookmark bm(m_pGDeb, NULL, "", "");
    bookmarkCB->clear();

    if( bm.count() == 0 )
    {
        bookmarkCB->addItem("<No bookmarks>");
        bookmarkCB->setEnabled(false);
        return;
    }
    bookmarkCB->addItem("<Bookmarks>");
    bookmarkCB->setEnabled(true);
    for( int i = 1; i <= bm.count(); ++i )
    {
        bookmarkCB->addItem(bm.getBookmarkName(i));
    }
}

ExtSQL::~ExtSQL()
{
//	delete ok;
//	delete exit;
//	delete save;
//	delete load;
//    delete editor;
}

void ExtSQL::setCompleterStrings(GSeq<GString> *list)
{

    if( list->numberOfElements() == 0 ) return;
    QStringList completerStrings;
    for(unsigned long  i = 1; i <= list->numberOfElements(); ++i ) completerStrings += list->elementAtPosition(i);

    QCompleter *completer = new QCompleter(completerStrings, this);
    //completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    //completer->setCaseSensitivity(Qt::CaseInsensitive);
    //completer->setWrapAround(false);

    editor->setCompleter(completer);

    SqlHighlighter * sqlHighlighter = new SqlHighlighter(m_Master->colorScheme(), editor->document(), list);
    editor->setSqlHighlighter(sqlHighlighter);

}

bool ExtSQL::findText(GString text)
{
    if( !editor->find(text, QTextDocument::FindBackward | QTextDocument::FindWholeWords) ) return false;

    QList<QTextEdit::ExtraSelection> extras;
    QTextEdit::ExtraSelection highlight;
    int line = editor->textCursor().blockNumber();
    if( line >= 0 )
    {
        setCursorPosition(line);
        highlight.cursor = editor->textCursor();
        highlight.format.setProperty(QTextFormat::FullWidthSelection, true);
        highlight.format.setBackground( QColor(255, 255, 204) );
    }
    extras << highlight;
    editor->setExtraSelections( extras );
    //After highlighting the line we need to re-select the found text:
    return editor->find(text, QTextDocument::FindWholeWords);
}

void ExtSQL::setCursorPosition( unsigned int line )
{
    QTextCursor cursor = editor->textCursor();
    cursor.movePosition( QTextCursor::Start );
    cursor.movePosition( QTextCursor::Down, QTextCursor::MoveAnchor, line  );
    editor->setTextCursor(cursor);
}

void ExtSQL::readAllLines(GString termChar)
{
    deb("readAllLines start");
    m_seqAllWords.removeAll();

    GString line;
    char cr = '\n';

    GString orgData = editor->toPlainText();
    orgData = orgData.stripTrailing(cr);

    orgData = orgData.change(termChar, termChar+"\n");
    while( orgData.occurrencesOf(cr) )
    {
        line = orgData.subString(1, orgData.indexOf(cr)-1);
        addWordsToSeq(validPart(line));
        orgData = orgData.remove(1, orgData.indexOf(cr));
    }
    addWordsToSeq(validPart(orgData));
    deb("readAllLines done");
}

void ExtSQL::addWordsToSeq(GString line)
{
    GString word;
    while(line.occurrencesOf(" "))
    {
        word = line.subString(1, line.indexOf(" ")-1);
        if( word.length()) m_seqAllWords.add(word);
        line = line.remove(1, line.indexOf(" "));

    }
    m_seqAllWords.add(line);
}

int ExtSQL::keepTerminationCharacter(GString cmd)
{
    if( (cmd.upperCase().occurrencesOf("XMLQUERY") || cmd.upperCase().occurrencesOf("XQUERY")) && cmd.upperCase().occurrencesOf("NAMESPACE") ) return 1;
    return 0;
}


void ExtSQL::createCommandsForTerminator()
{
    GString termChar = pStTermLE->text();

    deb("->TermChar was changed, running createCommandsForTerminator()");
    readAllLines(termChar);
    m_seqAllCmds.removeAll();


    GString cmd = "", line;
    for(int i = 1; i <= (int)m_seqAllWords.numberOfElements(); ++i)
    {
        line = m_seqAllWords.elementAtPosition(i);
        if( !line.strip().length() ) continue;
        deb("Word: "+m_seqAllWords.elementAtPosition(i));
        cmd += " "+line;
        if(line.occurrencesOf(termChar) )
        {
            if( cmd.strip().length() )
            {
                m_seqAllCmds.add(cmd);
            }
            deb("Adding cmd: "+cmd);
            cmd = "";
        }
    }
    if( cmd.strip().length() ) m_seqAllCmds.add(cmd);
    deb("Adding last cmd: "+cmd);

}

void ExtSQL::createCommands()
{
    //char cr = 10;
    //char lf = 13;

    GString cmd, line;
    //cmd = editor->toPlainText();
    m_seqAllCmds.removeAll();
    readAllLines();
    cmd = "";
    for(int i = 1; i <= (int)m_seqAllWords.numberOfElements(); ++i)
    {
        cmd += m_seqAllWords.elementAtPosition(i) +" ";
    }

    //To run multiple commands, termination character ';' is used. In namespaced XmlQueries this char is used for namespace-termination.
    if( keepTerminationCharacter(cmd) )
    {
        printf("+++++++++++++CMD1: %s\n", (char*) cmd);
        cmd = cmd.change("\n", " ");
        //cmd = cmd.removeAll(cr);
        //cmd = cmd.removeAll(lf);
        printf("+++++++++++++CMD2: %s\n", (char*) cmd);

        m_Master->setExtSqlCmd(editor->toPlainText());
        m_Master->setCmdText(cmd);
        m_Master->okClick();
        return;
    }

    int beginAtomic = 0;
    cmd = "";
    for(int i = 1; i <= (int)m_seqAllWords.numberOfElements(); ++i)
    {
        line = m_seqAllWords.elementAtPosition(i);
        if( !line.strip().length() ) continue;
        if( GString(line).upperCase() == "BEGIN" ) beginAtomic = 1;
        if( beginAtomic == 1 && GString(line).upperCase() == "ATOMIC") beginAtomic = 2;
        if( beginAtomic == 2 && GString(line).upperCase() == "END") beginAtomic = 3;
        //deb("Line: "+m_seqAllWords.elementAtPosition(i));
        cmd += " "+line;
        if(line.occurrencesOf(";") && (beginAtomic == 0 || beginAtomic == 3) )
        {
            if( cmd.strip().length() > 1 )
            {
                m_seqAllCmds.add(cmd);
            }
            deb("Adding cmd: "+cmd);
            cmd = "";
            if( beginAtomic == 3 ) beginAtomic = 0;
        }        
    }
    if( cmd.strip().length() ) m_seqAllCmds.add(cmd);
    deb("Adding last cmd: "+cmd);
}



GString ExtSQL::validPart(GString txt)
{
    if( txt.occurrencesOf("--") > 0 ) txt = txt.subString(1, txt.indexOf("--") - 1);
    return txt;
}

GString ExtSQL::helpTerminatorClicked()
{
    QString txt = "By default, statements are terminated by ';' (semi-colon)\n\n";
    txt += "However, namespaced XMLQueries or BEGIN...END statements and similar ";
    txt += "are using ';' inside the statement. ";
    txt += "PMF will usually try to figure out where statements end, but in some cases ";
    txt += "you may need to use a different terminator in your statements ";
    txt += "and set this terminator here.\n";
    txt += "NOTE: Mixing XMLQueries with SQL/DDL/DML will usually not work.";
    return txt;
}

void ExtSQL::runClicked()
{
    infoText(green, "Running, please wait...");
    if( GString(pStTermLE->text()) != GString(";") ) createCommandsForTerminator();
    else createCommands();
    m_Master->setExtSqlCmd(editor->toPlainText());
    int errCount = 0;
    for(int i = 1; i <= (int)m_seqAllCmds.numberOfElements(); ++i)
    {
        deb("cmd: "+m_seqAllCmds.elementAtPosition(i));
        infoText(green, "Running command "+GString(i)+" of "+GString(m_seqAllCmds.numberOfElements())+"...");
        m_Master->setCmdText(m_seqAllCmds.elementAtPosition(i).stripTrailing(pStTermLE->text()));
        //helper::msgBox(this, "pmf", "cmd: "+m_seqAllCmds.elementAtPosition(i).stripTrailing(pStTermLE->text()));
        //m_Master->okClick();
        if( m_Master->okClick().length() ) errCount++;
    }
    if( m_seqAllCmds.numberOfElements() > 1  )
    {
        if( errCount > 0) infoText(red, "Multiple statements: "+GString(errCount)+" of "+GString(m_seqAllCmds.numberOfElements())+" statements failed.");
        else infoText(green, "...Done.");
        //QMessageBox::information(this, "pmf", GString(errCount)+" of "+GString(m_seqAllCmds.numberOfElements())+" statements failed.");
    }
    else infoText(hideIt);
}

void ExtSQL::infoText(int color, GString text  )
{
    if( color == hideIt )
    {
        infoLabel->hide();
        return;
    }

    if( color == red ) infoLabel->setStyleSheet("QLabel { color : red; }");
    else if( color == green ) infoLabel->setStyleSheet("QLabel { color : green; }");
    infoLabel->setText(text);
    infoLabel->show();
    infoLabel->repaint();

}

void ExtSQL::exitClicked()
{
    this->close();
}

void ExtSQL::formatTxt()
{
    char cr = 10;
    char lf = 13;
    GString txt = editor->document()->toPlainText();
    txt = txt.removeAll(cr);
    txt = txt.removeAll(lf);
    int pos = 0;
    while( GString(txt).upperCase().indexOf(" WHERE "), pos)
    {
        pos = GString(txt).upperCase().indexOf(" WHERE ")+7;
        txt = txt.insert((char*)"\n", pos);
    }
    while( GString(txt).upperCase().indexOf(" WHERE "), pos)
    {
        pos = GString(txt).upperCase().indexOf(" AND ")+7;
        txt = txt.insert((char*)"\n", pos);
    }

}

void ExtSQL::loadClicked()
{
    infoLabel->hide();
    QString name = QFileDialog::getOpenFileName(this, "Load", extSqlSaveDir());
	if(GString(name).length() == 0) return;
	QFile f(name);
	if( !f.open(QIODevice::ReadOnly ))
	{
		GString txt = "Cannot open file "+GString(name);
		QMessageBox::information(this, "pmf", txt);
		return;
	}
    editor->setText(f.readAll());
	f.close();
}

void ExtSQL::saveAsBookmarkClicked()
{
    Bookmark *bm = new Bookmark(m_pGDeb, this, editor->toPlainText(), m_Master->currentTable(0));
    bm->exec();
    loadBookmarks();
}

void ExtSQL::bookmarkSelected(int index)
{
    Bookmark bm(m_pGDeb, NULL, "", "");
    GString current = editor->toPlainText();
    if( current.length() ) current = "\n\n---------------------\n"+current;
    editor->setText( bm.getBookmarkFormattedSQL(index)+current);
    editor->setFocus();
}


void ExtSQL::saveClicked()
{
    QString name = QFileDialog::getSaveFileName(this, "Save", extSqlSaveDir());
	if(GString(name).length() == 0) return;
	QFile f(name);
	if( !f.open(QIODevice::Unbuffered | QIODevice::WriteOnly | QIODevice::Append))
	{
        msg("Could not open File "+GString(name)+", permission denied.");
		return;
	}
    f.write(GString(editor->toPlainText()));
	f.close();
}
GString ExtSQL::sqlCmd()
{
	return m_gstrSQLCmd;
}
GString ExtSQL::orgCmd()
{
	return m_gstrOrgCmd;
}
void ExtSQL::clearCmd()
{
    editor->clear();
}

void ExtSQL::setCmd(GString cmd, GString currentFilter)
{
	//if( !cmd.length() ) cmd = "-- Editor for multilined SQL statements. Use -- (double dash) for comments.\n";
    m_strCurrrentFilter = currentFilter;
    editor->append(cmd);
}
void ExtSQL::closeEvent(QCloseEvent * event)
{
    m_gstrSQLCmd = editor->toPlainText();
    m_Master->setExtSqlCmd(m_gstrSQLCmd);
    if( !this->isModal() )	m_Master->extSQLClosed();
	event->accept();
}

void ExtSQL::keyReleaseEvent(QKeyEvent *event)
{
    if( event->modifiers().testFlag(Qt::ControlModifier)) return;
    switch (event->key())
    {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_F3:
        case Qt::Key_Escape:
            return;
        default:
            QWidget::keyReleaseEvent(event);
            if( findLE->hasFocus() ) findText();
    }
}

int ExtSQL::findText(int offset)
{    
    findLE->setStyleSheet("");
    GString textToFind = findLE->text();
    if(!textToFind.length())
    {
        m_iLastFindPos = 0;
        return 0;
    }
    if( !editor->findWordOccurrences(textToFind, m_cbCaseSensitive->isChecked()) ) findLE->setStyleSheet("background: red;");
    return 1;
}


void ExtSQL::highlightWord(int start, int end)
{
    QList<QTextEdit::ExtraSelection> extras;
    editor->setExtraSelections( extras );
    QTextCharFormat fmt;
    fmt.setBackground(Qt::yellow);

    QTextCursor cursor(editor->document());
    cursor.setPosition(start, QTextCursor::MoveAnchor);
    cursor.setPosition(end, QTextCursor::KeepAnchor);
    cursor.setCharFormat(fmt);
}


void ExtSQL::keyPressEvent(QKeyEvent *event)
{
//    if( event->key() == Qt::Key_Escape && !this->isModal() )
//    {
//        printf("ExtSQL not modal, closing\n");
//        this->close();
//    }
    if( event->modifiers().testFlag(Qt::ControlModifier))
    {
        if( event->key() == Qt::Key_F )
        {
            findLE->selectAll();
            findLE->setFocus();
            return;
        }
    }
    switch (event->key())
    {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_F3:
            editor->nextFoundWord();
            break;
        case Qt::Key_Escape:
            if( findLE->hasFocus() && findLE->text().length() > 0)
            {
                printf("ExtSQL findLE has focus\n");
                findLE->setText("");
                editor->setFocus();
            }
            else exitClicked();
            break;
        default:
            QWidget::keyPressEvent(event);
    }

}

void ExtSQL::deb(GString msg)
{
    GString s = QTime::currentTime().toString("hh:mm:ss.zzz");
    if( !m_pGDeb ) return;
    m_pGDeb->debugMsg("extSQL", 1, s+": "+msg);
}


void ExtSQL::saveGeometry()
{
	m_qrGeometry = this->geometry();
}
void ExtSQL::restoreGeometry()
{
	this->setGeometry(m_qrGeometry);	
}

void ExtSQL::callFileDropEvent(QDropEvent * event)
{
    editor->readFilesFromDroppedList(event);
}

/*
void extSQL::focusOutEvent(QFocusEvent* e)
{
	////this->setWindowOpacity ( 0.5 );

tm("LostFocus2");
}

void extSQL::focusInEvent(QFocusEvent* e)
{
    //if( m_iHasFocus == 1 ) return;
    //this->setWindowOpacity ( 1.0 );
    tm("GotFocus2");
    //m_iHasFocus = 1;
}

bool extSQL::event( QEvent * pEvent )
{
if ( pEvent->type() == QEvent::ApplicationActivate ){tm("ACT");this->setWindowOpacity ( 1.0 );}
else if ( pEvent->type() == QEvent::ApplicationDeactivate ) {tm("OUT");this->setWindowOpacity ( 0.5 );}

return QDialog::event( pEvent );
}
*/
