//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include <qapplication.h>
#include <qtabwidget.h>
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
#include <gseq.hpp>
#include <dsqlplugin.hpp>

#include <qfiledialog.h>
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qcheckbox.h>

#ifndef _OPTIONSTAB_
#define _OPTIONSTAB_

#define ROW_CHECKBOX 1
#define ROW_LINEEDIT 2
#define ROW_COMMENT 3
#define ROW_TEXTBOX 4


typedef struct{
   GString GroupName;
   int WdgType;
   GString Title;
   int MaxLen;
   QWidget * TheWdgt;
   GString Value;
   GString HelpText;
   GString OldValue;
   } OPTIONS_TAB_ROW;

class OptionsTab : public QDialog
{
    public:

	Q_OBJECT
	public:
        OptionsTab(QWidget *parent, GString settingsName);
        ~OptionsTab();
        int setFieldValue(GString groupName, GString fieldName, GString data);
        GString getFieldValue(GString groupName, GString fieldName);
        GString getTextBoxValue(GString groupName, GString fieldName);
        int getCheckBoxValue(GString groupName, GString fieldName);
        int setCheckBoxValue(GString groupName, GString fieldName, int checked);
        void setRowsToDefault();
        int settingsChanged();
        void addRow(GString groupName, GString title, int type, GString defaultValue, GString defaultText, int maxLen = 1);
        void addRow(GString groupName, GString text);
        void displayRowsInTab(GString groupName, GString tabTitle);
        QWidget * createGridWidget(GString groupName) ;
        int  rowCount(GString groupName);
        void clearRows(GString groupName = "");
        int cfgFileVersion();


	private slots:
		void OKClicked();
		void CancelClicked();
        void saveSettingsClicked();
        void clearSettingsClicked();
		
	private:
        QTabWidget * m_mainWdgt;
		QFont f;
		QPushButton * ok, * cancel, *getFileB;
        GString m_gstrModified;
        GSeq <OPTIONS_TAB_ROW*> m_rowsSeq;
        QGridLayout * m_pMainGrid;
        void displayRow(OPTIONS_TAB_ROW * pRow, QGridLayout *pGrid, int row );
        void displayAllRows();
        int isInSeq(GString groupName, GString title);

        GString optionsrowToString(OPTIONS_TAB_ROW * pRow);
        void loadPreviousSettings();
        void saveSettingsForAllRows();
        OPTIONS_TAB_ROW* getOptionsRow(GString groupName, GString fieldName);
        GString m_strSettingsFileName;
        int m_iCfgFileVer;
        void mb(GString message);
};

#endif

