//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include <qapplication.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlistwidget.h>
#include <QTextEdit>
#include <QTextBrowser>
#include <QTabWidget>
#include <QTableWidget>
#include <gstring.hpp>
#include <qfiledialog.h>
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qpainter.h>
#include <qfont.h>
#include <dsqlplugin.hpp>

#ifndef _OPTIONSTAB_
#include "optionsTab.h"
#endif
#include "txtEdit.h"

#include <gfile.hpp>
#ifndef _getclp_
#define _getclp_

//avoid revcursive include
class TabEdit;


class Getclp : public QDialog
{
    Q_OBJECT
public:
    Getclp(DSQLPlugin* pDSQL, TabEdit *parent, GString tableName = "");
    ~Getclp();

    //   void tm(GString message){QMessageBox::information(this, "PMF", message);}
    void saveGeometry();
    void restoreGeometry();
    void closeEvent(QCloseEvent *event);
    int saveAsXml(GString fileName, int overwrite = 0);
    int saveAsPlainText(GString fileName, int overwrite = 0);
    void writeForeignKeys(GFile outFile, GString entry);
    void showOptionsDialog(QWidget* parent);

    
    void createTableStmt(GString length, GString out, GString defVal, GSeq<COL_SPEC*> specSeq);
    

    
private slots:
    void OKClicked();
    void saveAsPlainTextClicked();
    void saveAsXmlClicked();
    void sortClicked(int);
public slots:
    void optionsClicked();

private:
    GSeq <GString> m_colSeq;
    QTabWidget *  m_mainWdgt;
    short fillLB();

    //QTextBrowser * mainEditor;
    TxtEdit * mainEditor;

    QTableWidget * tabWdgt;
    GString iTableName;
    QPushButton * ok;
    QPushButton * saveAsPlainTextBt;
    QPushButton * saveAsXmlBt;
    QPushButton * optionsBt;

    GString m_strCreateTabStmt;

    QRect m_qrGeometry;
    void msg(GString txt);
    TabEdit * m_pParent;
    virtual void keyPressEvent(QKeyEvent *event);
    void addTableItem(int col, GString txt);
    void writeChecks(DSQLPlugin *pDSQL, QFile *outFile, OptionsTab *pOptTab);
    void writeIdxInfo(GSeq <IDX_INFO*> * idxSeq, QFile *outFile, OptionsTab *pOptTab);
    void writeForKeyInfo(GSeq <IDX_INFO*> * idxSeq, QFile* outFile, OptionsTab *pOptTab);
    void writePrimKeyInfo(GSeq <IDX_INFO*> * idxSeq, QFile* outFile, OptionsTab *pOptTab);
    void writeToUtf8File(QFile* fileOut, QString txt, int setBOM = 0);
    void writeColumns(QFile *outFile);
    OptionsTab* createOptionsTab(GString groupName, QWidget *parent);
    void writeTable(QFile *outFile, OptionsTab* pOptTab);
    void writeView(QFile *outFile, OptionsTab*);
    void writeAlias(QFile *outFile, OptionsTab* pOptTab, TABLE_PROPS * pProps);
    void writeMQT(QFile *outFile, OptionsTab* pOptTab);
    GString formatColumnsInStmt(GString cols, OptionsTab *pOptTab);


    void addTextLB();
    void addColsWdgt();
    void writeSeqToFile(QFile *outFile, GSeq <GString> *entry, GString tag);
    GString tableSpace(GString tableName);
    //GString createTableStmt(GString tableName, GSeq<COL_SPEC*> *specSeq);
    GString linesToSingleLine(GString in);
    int isSysIdx(GString idxName);

    DSQLPlugin * m_pDSQL;
};

#endif
