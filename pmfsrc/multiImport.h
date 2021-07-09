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
#include <gseq.hpp>
#include <qfiledialog.h>
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>

#include <QTableWidget>
#include <QComboBox>

#include <dsqlplugin.hpp>

#include <gseq.hpp>


#ifndef _MULTIIMPORT_
#define _MULTIIMPORT_

typedef struct {
    GString FILENAME;
    GString TABLE;
    GString IMPTYPE;
}  MULTI_IMP;


class MultiImport : public QDialog
{
   Q_OBJECT
public:
    MultiImport(GDebug *pGDeb, DSQLPlugin* pIDSQL, QWidget *parent=0);
    ~MultiImport();
	void closeEvent(QCloseEvent * event);
    int createRow(GString fileName, GString schema, GString table);

public slots:
	void keyPressEvent(QKeyEvent *event);	
private slots:
	void cancel();
    void okClicked();

private:
	QLineEdit *dbLE, *uidLE, *pwdLE;
    QPushButton * ok, *esc, *add, *rem, *test;
	QTableWidget * m_twMain;
    QWidget * m_pMainWdgt;

    void tm(QString message){QMessageBox::information(this, "PMF MultiImport ", message);}
    void deb(GString msg);
    DSQLPlugin * m_pIDSQL;
    GDebug * m_pGDeb;


};

#endif
