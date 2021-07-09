//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#ifndef _BOOKMARK_
#define _BOOKMARK_

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>

#include <gstring.hpp>
#include <gseq.hpp>
#include <gdebug.hpp>

class Bookmark : public QDialog
{
    typedef struct{
        GString Name;
        GString Table;
        GString SqlCmd;
        void init(){Name = SqlCmd = Table = ""; }
    } BOOKMARK;

	Q_OBJECT
	GString dbName, userName, passWord, nodeName;
	
	public:
        Bookmark( GDebug *pGDeb, QWidget* parent = NULL, GString sql = "", GString table = "");
        ~Bookmark();
		void tm(QString message){QMessageBox::information(this, "db-Connect", message);}
		int count();
		int save();		
		int clearBookmarkSeq();			
        int addBookmark(GString name, GString table, GString sqlCmd);
		GString getBookmarkName(int i);
        GString getBookmarkFormattedSQL(int i);
        GString getBookmarkFlatSQL(int i);
		GString getBookmarkTable(int i);		
		
private:
        GSeq <BOOKMARK*> m_seqBookmark;
//		GSeq <GString> m_seqName;
//		GSeq <GString> m_seqSQL;
//		GSeq <GString> m_seqTable;
		GString m_strSQL;
		GString m_strTable;
		int readAll();
		void msg(GString txt);
		GString bmFileName();
		void deb(GString msg);
        GDebug *m_pGDeb;
	protected slots:
		virtual void cancelClicked();
		virtual void okClicked();
		void closeEvent( QCloseEvent*);
		void keyPressEvent(QKeyEvent *event);
		
	protected:
		QComboBox* dbNameCB;
		QLineEdit* nameLE;
		QLineEdit* tableLE;
		QLineEdit* sqlLE;
		QPushButton* okB;
		QPushButton* cancelB;
		
};

#endif
