//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#ifndef _QUERYDB_
#define _QUERYDB_

#include <qdialog.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <QListWidget>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <gstring.hpp>
#include <qmessagebox.h>
#include <qprogressdialog.h>

#include <gseq.hpp>
#include "pmfSchemaCB.h"
#include "tableSelector.h"

#include <dsqlplugin.hpp>

class Pmf;


class Querydb : public QDialog
{
    Q_OBJECT
private:
	GString currentSchema;
    DSQLPlugin * m_pDSQL;
    TableSelector * tbSel;
    Pmf* m_pPmf;
    GSeq <GString> m_cmdSeq;

public:

    Querydb( DSQLPlugin* pSDQL,  Pmf* parent, GString currentSchema, int hideSysTabs = 0 );
    virtual ~Querydb();
	void searchInTable(GString table, QProgressDialog * apd);
	void tm(GString message){QMessageBox::information(this, "QUERY DB", (char*)message);  return;}

protected slots:

	//    virtual void fillTableLB(GSeq <GString> * tableNameSeq);
	virtual void okClicked();
    virtual void cancelClicked();
    void listDoubleClicked(QListWidgetItem*);


protected:
	QCheckBox* systabCB;
	QLineEdit* findLE;
	QRadioButton* hostRB;
    QCheckBox* exactMatchCB;
	QRadioButton* valueRB;
	QPushButton* okB;
	QPushButton* cancelB;
	QListWidget* resultLB;
	QButtonGroup * bGroup;
};

#endif

