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

#ifndef _EXPIMPOPTIONS_
#define _EXPIMPOPTIONS_


typedef struct{
   int FileType;
   int WdgType;
   GString Title;
   int MaxLen;
   QWidget * TheWdgt;
   GString DefaultValue;
   GString DefaultText;
   GString OldValue;
   int PmfInternal;
   } OPTIONSROW;

class ExpImpOptions : public QDialog
{
    public:
    enum FileType
    {
        TYP_DEL = 0,
        TYP_IXF,
        TYP_LOB,
        TYP_ALL,
        TYP_LOAD
    };
    enum Mode
    {
        MODE_EXPORT,
        MODE_IMPORT,
        MODE_LOAD,
        MODE_LOAD_HADR
    };

	Q_OBJECT
	public:
        ExpImpOptions(QWidget *parent, Mode mode, GString dbName);
        ~ExpImpOptions();
        GString modifiedString();
        void createEntries(Mode mode);
        void displayAllRows();
        GString createModifiedString(FileType type);
        int setFieldValue(FileType type, GString fieldName, GString data);
        GString getFieldValue(FileType type, GString fieldName);
        int getCheckBoxValue(FileType type, GString fieldName);
        int setCheckBoxValue(FileType type, GString fieldName, int checked);
        void setRowsToDefault();
        int settingsChanged();
        void setHadrPath(int overwrite = 0);


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
        GSeq <OPTIONSROW*> m_rowsSeq;
        QGridLayout * m_pMainGrid;
        void displayRow(OPTIONSROW * pRow, QGridLayout *pGrid, int row );

        void createRow(int mod, GString title, int type, GString defaultValue, GString defaultText, int maxLen = 1, int pmfInternal = 0);
        void createRow(int mod, GString text);
        void createTab( GString tabTitle, FileType fileType);
        GString optionsrowToString(OPTIONSROW * pRow);
        void loadPreviousSettings();
        void saveSettingsForAllRows();
        OPTIONSROW* getOptionsRow(FileType type, GString fieldName);
        GString m_strSettingsFileName;


        void mb(GString message);
        Mode m_Mode;
};

#endif

