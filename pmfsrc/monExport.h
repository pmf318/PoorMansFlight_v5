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
#include <QListWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QMessageBox>

#include <gstring.hpp>
#include <qfiledialog.h>
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>
#include <dsqlplugin.hpp>
#include <dbapiplugin.hpp>

#include <gstring.hpp>
#include "txtEdit.h"
#include "pmfTable.h"

#ifndef _MONEXPORT_
#define _MONEXPORT_

class MonExport : public QDialog
{
   Q_OBJECT
public:
   MonExport(DSQLPlugin * pDSQL, GString monitorName, QTableWidget *pTabWdgt, QWidget *parent=0);
   ~MonExport();

private slots:
   void okClicked();
   void escClicked();
   void radioBtClicked();


private:
   void msg(GString message){QMessageBox::information(this, "PMF", (char*)message);}
   void reject();
   int exportToDel();
   int exportToTable();

   int exportToXml();
   GString getFileName(GString extension);
   void writeToUtf8File(QFile* fileOut, QString txt, int setBOM = 0);

   GString createStmt(int asDdl = 0);
   void exportAsXml(GString fileName);
   int insertIntoTable(int row, PmfTable * table);
   GString columnDdl(int col);
   void exportAsDel(GString fileName);
   QTableWidget *m_pTabWdgt;
   QPushButton * okBt;
   QPushButton * escBt;
   GString m_gstrMonitorName;
   QRadioButton *asXmlRB, *toTableRB, *asDelRB;
   QLineEdit *tabNameLE;
   DSQLPlugin * m_pDSQL;
};

#endif
