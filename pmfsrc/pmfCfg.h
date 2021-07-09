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
#include <QListWidget>
#include <QMessageBox>
#include <QGridLayout>

#include <gstring.hpp>
#include <gdebug.hpp>
#include <gseq.hpp>
#include <qfiledialog.h>
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>

#ifndef _PMFCFG_
#define _PMFCFG_

typedef struct{
	GString key;
	GString val;
	GString help;
	QLineEdit *pLE;
} CFG_ROW;
	


class PmfCfg : public QDialog
{
   Q_OBJECT
public:
    PmfCfg(GDebug *pGDeb, QWidget *parent=0);
    ~PmfCfg();
	GString createCfgFile();
	GString getValue(GString key);
private slots:
	int save();
	void cancel();
	GString loadCfgFile(QGridLayout * grid);
	GString cfgFileName();



private:
	QLineEdit *dbLE, *uidLE, *pwdLE;
	QPushButton * ok;
	QPushButton * esc;
	GSeq <CFG_ROW* > m_cfgSeq;
	void addToSeq(GString key, GString help);
	void tm(QString message){QMessageBox::information(this, "PMF config", message);}
	void deb(GString txt);	
    GDebug * m_pGDeb;
};

#endif
