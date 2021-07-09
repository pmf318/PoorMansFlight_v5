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

#include <gstring.hpp>
#include <qfiledialog.h>
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>
#include <dsqlplugin.hpp>
#include <dbapiplugin.hpp>

#include <gstring.hpp>
#include "txtEdit.h"

#ifndef _SIMPLESHOW_
#define _SIMPLESHOW_

class SimpleShow : public QDialog
{
   Q_OBJECT
public:
   SimpleShow(QString name, QWidget *parent=0, bool showFormatCB = false, bool readOnly = true);
   ~SimpleShow();
   //short fillLB();
   void setText(GString text);
   void setSqlHighlighter(int colorScheme, GSeq <GString>* list);
   void setLineWrapping(QTextEdit::LineWrapMode mode);

private slots:
   void OKClicked();
   void displayData();

private:

   void reject();
   QPushButton * ok;
   QListWidget* infoLB;
   TxtEdit * infoTE;
   QCheckBox * m_cbFormat;
   GString m_gstrOrgText;
   QString m_gstrSettingsName;
};

#endif
