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
#include <gstring.hpp>
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qmessagebox.h>
#include <QTableWidget>
#include "pmfSchemaCB.h"
#include <dsqlplugin.hpp>

#ifndef _tbSize_
#define _tbSize_

class TableSize : public QDialog
{
   Q_OBJECT
public:
    TableSize(DSQLPlugin* pDSQL, QWidget *parent=0, GString currentSchema ="", int hideSysTabs = 0);
   ~TableSize();
   short fillLV();
   void setSchema(GString aTabSchema);
private slots:
   void OKClicked();
   void tabSpaceClicked();
   void runStatsClicked();
	void schemaSelected(int index);
    void sortClicked(int);
private:
   void tm(GString message);
   GString iTabName, iTabSchema;
   QPushButton * ok, * rstatB, *tbsDetB;
   QTableWidget* mainLV;
   QLabel * info;
   PmfSchemaCB * schemaCB;
   void setVHeader();
   DSQLPlugin* m_pDSQL;
   int m_iHideSysTabs;

};

#endif
