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

#ifndef _editBm_
#define _editBm_

class EditBm : public QDialog
{
   Q_OBJECT
public:
   EditBm(GDebug*pGDeb, QWidget *parent=0);
   ~EditBm();
   short fillLV();

private slots:
   void OKClicked();
	void exitClicked();
	void delClicked();	
    void sortClicked(int);
private:
   void tm(QString message){QMessageBox::information(this, "Table sizes", message);}
   GString iTabName, iTabSchema;
   QPushButton * exitButton, *saveButton, *delButton;
   QTableWidget* mainLV;
   QLabel * info;
   GDebug * m_pGDeb;
};

#endif
