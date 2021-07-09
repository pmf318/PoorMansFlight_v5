//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include <qapplication.h>
#include <qpushbutton.h>
#include <qmainwindow.h>
#include <qlineedit.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>

#include <gstring.hpp>
#include <qfiledialog.h>
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qpainter.h>
#include <qfont.h>
#include <QTextEdit>
#include <QMessageBox>


#include "txtEdit.h"
#include "tabEdit.h"

#ifndef _extSQL_
#define _extSQL_

class ExtSQL : public QDialog
{
   Q_OBJECT
public:
   ExtSQL(GDebug * pGDeb,  DSQLPlugin * pDSQL, TabEdit *parent=0, int fullMode = 1);
   ~ExtSQL();
   GString sqlCmd();
   GString orgCmd();
   void setCmd(GString cmd, GString currentFilter = "");
   void clearCmd();
   void saveGeometry();
   void restoreGeometry();   
   void setCompleterStrings(GSeq <GString>* list);
   bool findText(GString text);
   void callFileDropEvent(QDropEvent * event);

private slots:
   void runClicked();
   void loadClicked();
   void saveClicked();
   void exitClicked();
   void saveAsBookmarkClicked();
   void bookmarkSelected(int index);



private:
   void msg(GString message){QMessageBox::information(this, "PMF", (char*)message);}
   int keepTerminationCharacter(GString cmd);
   void setCursorPosition( unsigned int line );
   void highlightWord(int start, int end);
   //int highWord(QString highlighText, int caseSensitive = 0);
   int findText(int offset = 0);
   void loadBookmarks();

   GString helpTerminatorClicked();
   GString m_gstrSQLCmd;
   GString m_gstrOrgCmd;
   int m_iLastFindPos;
   GString m_strLastSearchString;
   QPushButton * ok;
   QPushButton * exit;
   QPushButton * load;
   QPushButton * save;
   QPushButton * saveAsBookmark;
   QComboBox * bookmarkCB;
   QLineEdit *pStTermLE;
   QLabel * infoLabel;
   //QTextEdit* editLB;   
   TxtEdit * editor;
   TabEdit * m_Master;
   QRect m_qrGeometry;
   void closeEvent(QCloseEvent * event);
   virtual void keyPressEvent(QKeyEvent *event);
   virtual void keyReleaseEvent(QKeyEvent *event);
   QComboBox * m_listCB;
   GString m_strKeys;
   GDebug * m_pGDeb;
   QCheckBox * m_cbCaseSensitive;
   QLineEdit * findLE;

   void deb(GString msg);
   void formatTxt();
   DSQLPlugin * m_pDSQL;

   GString m_strCurrrentFilter;

   GString validPart(GString txt);
   void readAllLines(GString termChar = ";");
   void addWordsToSeq(GString line);
   void createCommands();
   void infoText(int color, GString text = "");
   void createCommandsForTerminator();
   GSeq <GString> m_seqAllWords;
   GSeq <GString> m_seqAllCmds;
   
   //bool event( QEvent * pEvent );
protected:
	//void focusOutEvent(QFocusEvent* e );
    //void focusInEvent(QFocusEvent* e );
};

#endif

