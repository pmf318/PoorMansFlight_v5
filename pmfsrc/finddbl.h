//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#ifndef _FINDDBL_
#define _FINDDBL_

#include <qdialog.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <QListWidget>
#include <QComboBox>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qmessagebox.h>

#include <gstring.hpp>
#include <gseq.hpp>

#include <dsqlplugin.hpp>
#include "pmfSchemaCB.h"
#include "tableSelector.h"

class  Finddbl: public QDialog
{
    Q_OBJECT
public:

    Finddbl( GDebug *pGDeb, DSQLPlugin * pDSQL, QWidget* parent = NULL, GString currentSchema = "", int hideSysTabs = 0 );
    ~Finddbl();
	void tm(GString message);
	int findInTable(GString table);
    void checkTables(QListWidget *pLB);
	void addToLB(GString txt);
    

private slots:
	virtual void okClicked();
	virtual void cancelClicked();

private:
	QListWidget* resultLB;
	QRadioButton* displayRB;
	QRadioButton* deleteRB;
	QPushButton* okB;
	QPushButton* cancelB;
	QLabel* infoTXT;
	QFont font;
	short iStop;
	GString currentSchema;
	GSeq <GString> identRowSeq;
    DSQLPlugin * m_pDSQL;
    TableSelector * tbSel;
    GDebug* m_pGDeb;

public:
	QMessageBox * stopIt;
};

#endif // _included
