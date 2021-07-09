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
#include <QComboBox>
#include <gstring.hpp>
#include <qfiledialog.h>
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>

#include <gseq.hpp>


#include <dsqlplugin.hpp>
#include "pmfSchemaCB.h"
#include "tableSelector.h"
#include "getclp.h"
#include "optionsTab.h"

#ifndef _ALLTABDDL_
#define _ALLTABDDL_

class AllTabDDL : public QDialog
{
   Q_OBJECT
public:
   AllTabDDL(DSQLPlugin* pDSQL, QWidget *parent=0, GString currentSchema = "", int hideSysTabs = 0);
   ~AllTabDDL();

private slots:
   void okClicked();
   void cancelClicked();
   void optionsClicked();


private:
   QPushButton * ok, * cancel, *optionsBt;
   DSQLPlugin * m_pDSQL;
   TableSelector * tbSel;
   void msg(GString message);
   int exportToFile(GString tabName, long overwrite);
   void setInfo(GString msg);
   QString m_qstrPrevDir;
   int askOverwrite(GString fileName);
   QRadioButton *asXmlRB;
   QRadioButton *asTxtRB;
   QLineEdit *infoLE;
   QWidget *m_pParent;
   int overwriteFiles(QListWidget* pLB, GString path, GString schema);
};

#endif
