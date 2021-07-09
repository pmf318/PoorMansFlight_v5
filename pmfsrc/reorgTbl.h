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
#include <q3listbox.h>
#include <gstring.hpp>
#include <dsqlobj.hpp>
#include <qfiledialog.h>
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>

#ifndef _REORGTBL_
#define _REORGTBL_

class reorgTbl : public QDialog
{
   Q_OBJECT
public:
   reorgTbl(QWidget *parent=0, const char* name=0, GSeq <GString> * tableNameSeq = NULL, const char* currentSchema = 0);
   ~reorgTbl();
   short fillLB(GSeq <GString> * tableNameSeq);

private slots:
   void OKClicked();
   void CancelClicked();

private:

   QPushButton * ok, * cancel;
   Q3ListBox* tableLB;
   GString currentSchema;
};

#endif
