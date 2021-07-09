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
#include <gthread.hpp>
#include "threadBox.h"
#include <idsql.hpp>
#include <dsqlplugin.hpp>
#include "expImpOptions.h"
#include <qfiledialog.h>
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qcheckbox.h>

#ifndef _EXPORTBOX_
#define _EXPORTBOX_

class ExportBox : public QDialog
{
	class ExportThread : public GThread
	{
		public:
			virtual void run();
            void setOwner( ExportBox * aExport ) { myExport = aExport; }
		private:
            ExportBox * myExport;
	};
	
	Q_OBJECT
	public:
        ExportBox(DSQLPlugin *pDSQL, QWidget *parent, GString *prevDir);
        ~ExportBox();
		void setSelect(GString sel, GString table);
        void mb(GString message);
		void callExport();
        void startExport();
		
	private slots:
		void OKClicked();
		void CancelClicked();
		int getFileClicked();
		void timerEvent();
        void optionsClicked();
        void modeClicked();
		
	private:
        void askLobExport();
        int getNumberOfLobFiles(DSQLPlugin * pDSQL, GString table, GString constraint = "");
		QFont f;
		QLineEdit *fileNameLE;
        QRadioButton* ixfRB, *delRB, *wsfRB, *txtRB, *selRB, *allRB, *ddlRB;
        QPushButton *optionsB;
		QPushButton * ok, * cancel, *getFileB;
		GString iAction;
		GString iTableName;
		QCheckBox * exportLobCB;
		ExportThread * aThread;
        ThreadBox * tb;
   		GString *m_gstrPrevDir;
		QTimer * timer;
        DSQLPlugin * m_pIDSQL;
        unsigned long getResultCount(GString cmd);
        GString m_gstrExpErr;
        ExpImpOptions * m_pExpImpOptions;
        QGridLayout * m_pMainGrid;
        int m_iFullSelectLobCount;
        int m_iFullSelectLobFiles;
        int m_iCurrentSelectLobCount;
        int m_iCurrentSelectLobFiles;
		
};

#endif

