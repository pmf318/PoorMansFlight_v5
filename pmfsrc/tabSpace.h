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
#include <QTableWidget>
#include <gstring.hpp>
#include <qfiledialog.h>
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qmessagebox.h>

#include <dsqlplugin.hpp>
#include <dbapiplugin.hpp>
#ifndef _tabSpace_
#define _tabSpace_

class TabSpace : public QDialog
{
   Q_OBJECT
public:
   TabSpace(DSQLPlugin * pDSQL, QWidget *parent=0);
   ~TabSpace();
   short fillLV();
   void setTableName(GString aTableName);
private slots:
   void OKClicked();
   void sortClicked(int);
   void reduceMaxClicked();

private:
   void tm(GString message){QMessageBox::information(this, "PMF", message);}
   GString iTabName, iTabSchema;
   QPushButton * ok, *reduceMaxBt;
   QTableWidget* mainLV;
   QLabel * info;
   DSQLPlugin * m_pIDSQL;
   DBAPIPlugin * m_pApi;
//   void fill (struct SQLB_TBSPQRY_DATA *dataP, sqluint32 num);
};

#endif
