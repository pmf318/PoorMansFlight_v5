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

#ifndef _mtIndex_
#define _mtIndex_

class mtIndex : public QDialog
{
   Q_OBJECT
public:
   mtIndex(QWidget *parent=0);
   ~mtIndex();
   short fillLV();
   void setTableName(GString aTable);
private slots:
   void OKClicked();
   void newIndClicked();
   void delClicked();

private:
   void tm(QString message){QMessageBox::information(this, "INDEX", message);}
   GString iTabName, iTabSchema;
   QPushButton * ok, * newInd, *del;
   QTableWidget* mainLV;
   QLabel * info;
};

#endif
