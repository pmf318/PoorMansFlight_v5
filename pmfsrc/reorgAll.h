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
#include <qlistwidget.h>

#include <QTableWidget>
#include <QGridLayout>
#include <gstring.hpp>
#include <qfiledialog.h>
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qtabwidget.h>
#include <qcheckbox.h>
#include <qcombobox.h>

#include <gthread.hpp>
#include "threadBox.h"
#include <dsqlplugin.hpp>
#include <idsql.hpp>

#include "txtEdit.h"
#ifndef _OPTIONSTAB_
#include "optionsTab.h"
#endif

#ifndef _REORGALL_
#define _REORGALL_

    typedef struct{
       int rowType;
       GString name;
       QRadioButton * typeRB;
       QCheckBox *nnCB;
       QLineEdit *defaultLE;
       QLabel *sizeTxt;
       QLineEdit *sizeLE;
       QComboBox *mbCB;
       QCheckBox *logCB;
       QCheckBox *compCB;
       } NEWROW;

//avoid revcursive include
class TabEdit;

class ReorgAll : public QDialog
{

   class TheThread : public GThread
   {
      public:
         virtual void run();
         void setOwner( ReorgAll * aReorg ) { myReorg = aReorg; }
         void setMode( int mode ) { myMode = mode; }
      private:
         ReorgAll * myReorg;
         int myMode; //Either RUNSTATS (0) or REORG (1)
   };



private:
   Q_OBJECT

public:
    ReorgAll(DSQLPlugin* pDSQL, QWidget *parent, GString tableName, int hideSystables);
    ~ReorgAll();
    void startReorg();
    void startRunstats();
    void setTabIndex(int pos);
    void tm(QString message){QMessageBox::information(this, "Runstats & al", message);}

private slots:
    void getBindFiles();
    void takeIt();
    void callReorg();
    void refreshRunstatsListView();
    void callRunstats();
    void callRebind();
    void refreshReorgListView();
    void saveList();
    void loadList();
    void newIndClicked();
    void newForeignKeyClicked();
    void sortClicked(int);
    void sortColumnsClicked(int);

    void newChecksClicked();
    void delChecksClicked();

    void delClicked();
    void alterTableClicked();
    void getPackages();
    void endIt();
    void dropCols();
    void saveChanges();
    void timerEvent();
    short fillIndLV();
    short fillChecksLV();
    void fillColLV();
    void itemChg(QTableWidgetItem*);
    void indexClicked(QTableWidgetItem*);
    void checksClicked(QTableWidgetItem*);
    void saveAsPlainTextClicked();
    void saveAsXmlClicked();
    void optionsClicked();

private:
    QTabWidget * m_mainWdgt;
    QWidget * m_pParent;
    GString reorgRunstatsErr;
    void addTextLB();
    void addColsWdgt();
    void indexTab();
    void reorgTab();
    void runstatsTab();
    void rebindTab();
    void alterTab();
    void dropTab();
    void checksTab();
    void showOptionsDialog(QWidget* parent);
    int fillLV( QTableWidget * aLV, GString cmd );
    void fillColumnsTab();
    void addTableItem(int col, GString txt);
    GString formTabName();
    QTableWidgetItem * createReadOnlyItem(GString txt);
    int saveAsPlainText(GString fileName, int overwrite);
    OptionsTab* createOptionsTab(GString groupName, QWidget * parent);
    int saveAsXml(GString fileName, int overwrite = 0);
    void writeToUtf8File(QFile* fileOut, QString txt, int setBOM = 0);
    void writeView(QFile *outFile, OptionsTab*);
    void writeAlias(QFile *outFile, OptionsTab* pOptTab, TABLE_PROPS * pProps);
    void writeMQT(QFile *outFile, OptionsTab* pOptTab);
    void writeColumns(QFile *outFile);
    void writeTable(QFile *outFile, OptionsTab* pOptTab);
    void writeChecks(DSQLPlugin *pDSQL, QFile *outFile, OptionsTab *pOptTab);
    void writeIdxInfo(GSeq <IDX_INFO*> * idxSeq, QFile *outFile, OptionsTab *pOptTab);
    void writeForKeyInfo(GSeq <IDX_INFO*> * idxSeq, QFile* outFile, OptionsTab *pOptTab);
    void writePrimKeyInfo(GSeq <IDX_INFO*> * idxSeq, QFile* outFile, OptionsTab *pOptTab);
    void writeSeqToFile(QFile *outFile, GSeq <GString> *entry, GString tag);
    GString linesToSingleLine(GString in);
    int isSysIdx(GString idxName);
    GString formatColumnsInStmt(GString cols, OptionsTab *pOptTab);
    GString tableSpace(GString tableName);
    short fillCreateTabLB();


    void setVHeader(QTableWidget * pLV);

    void alterTable(NEWROW* aRow);
    GSeq <NEWROW * > rowSeq;
    NEWROW * createRow(GString buttonName, int type, QWidget * pWdgt);
    void addToGrid(NEWROW * aRow, QGridLayout * aGrid, int row);

    GString iTabName, iTabSchema;

    QPushButton * runB, *newInd, *newFKey, *delInd, *altTab, *refreshB, *refreshIdxB, *refreshChecksB, *newCheck, *delCheck;
    QListWidget * bindLB, *tabspcLB;
    void takeItem();
    QTableWidget * reorgLV, *runstatsLV, *indexLV, *checksLV, *dropLV;
    QLineEdit * colNameLE;

    QRadioButton *tableRB;
    QRadioButton *extTabOnlyRB;
    QRadioButton *bothRB;
    QRadioButton *extTabIndexRB;
    QRadioButton *indexRB;
    QRadioButton *extIndexRB;
    QRadioButton *extIndexTabRB;
    QRadioButton *statsAllRB;
    QCheckBox * shareCB;
    QCheckBox *rawIndexCB;

    QPushButton * saveAsPlainTextBt;
    QPushButton * saveAsXmlBt;
    QPushButton * optionsBt;
    TxtEdit* mainEditor;


    TheThread * theThread;
    ThreadBox * tb;
    QTimer *timer;
    DSQLPlugin* m_pDSQL;
    GString iFullTabName;
    QWidget * pWdgtReorg;
    int m_iHideSysTables;
    QTableWidget* tabWdgt;
    void fillListViewFromAPI(DSQLPlugin *pDSQL, QTableWidget *pLV);
    GString m_strCreateTabStmt;
};

#endif
