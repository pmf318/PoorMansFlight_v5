//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2004
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
#include <qmessagebox.h>
#include <QTableWidget>


#include <gthread.hpp>


#include <dsqlplugin.hpp>
#include <dbapiplugin.hpp>

class Pmf;

#ifndef _getSnap_
#define _getSnap_

class GetSnap : public QDialog
{
   class GetSnapThread : public GThread
   {
      public:
         virtual void run();
         void setOwner( GetSnap * aSnap ) { mySnap = aSnap; }
      private:
         GetSnap * mySnap;
   };



   Q_OBJECT
public:
   GetSnap(DSQLPlugin * pDSQL, Pmf *parent=0);
   ~GetSnap();
   short fillLV();
   void setDBName(GString name){dbName = name;}
   void startThread();
   void fillListViewFromAPI(DBAPIPlugin *pAPI, GString dynSqlFilter = "");

//   void OKClicked();

private slots:
   void AdvancedClicked();
   void ExitClicked();
//   void startThread();
   void OKClicked();
   void slotRightClick();
   void sortClicked(int);
   void exportClicked();
   void resetClicked();

private:
   //void keyPressEvent(QKeyEvent *event);
   void tm(QString message){QMessageBox::information(this, "Snapshot", message);}
   GString getTitle();
   GString iTabName, iTabSchema;
   QPushButton * ok;
   QPushButton * resetBt;
   QPushButton * exitBt;
   QPushButton * exportBt;
   QPushButton * advBt;
   QTableWidget* mainLV;
   QLabel * info;

   QRadioButton * databaseRB;
   QRadioButton * buffRB;
   QRadioButton * lockRB;
   QRadioButton * sortRB;
   QRadioButton * dSQLRB;
   QRadioButton * tableRB;
   QRadioButton * hadrRB;
   QRadioButton * UOWRB;
   QLineEdit    * filterLE;
   short getSQLData();
   short getData(int type);
   GString dbName, m_gstrNodeName, m_gstrUser, m_gstrPwd;
   int pluginLoaded;
   QAction *m_actRightClick; 
   DSQLPlugin * m_pDSQL;
   GSeq <GString> _filterSeq;
   Pmf* m_pPmf;

   DBAPIPlugin *m_pApiPlg;


   typedef int (*PLUGIN) (char *, char*, char *, char*, int,  QTableWidget *);
   PLUGIN  plugIn;

};

#endif
