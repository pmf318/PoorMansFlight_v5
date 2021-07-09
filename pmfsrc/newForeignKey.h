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
#include <qlistwidget.h>
#include <gstring.hpp>
#include <qfiledialog.h>
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qlistwidget.h>
#include <qmessagebox.h>
#include <QGroupBox>
#include <gthread.hpp>
#include "threadBox.h"
#include "tableSelector.h"

#include <dsqlplugin.hpp>

#ifndef _newForeignKey_
#define _newForeignKey_


class NewForeignKey : public QDialog
{

    class CreateForeignKeyThread : public GThread
    {
    public:
        virtual void run();
        void setOwner( NewForeignKey * aNewForeignKey ) { myNewForeignKey = aNewForeignKey; }
    private:
        NewForeignKey * myNewForeignKey;
    };

    Q_OBJECT
public:
    void createForeignKey();
    NewForeignKey(DSQLPlugin* pDSQL, QWidget *parent, GString fullTableName, int hideSystemTables);
    ~NewForeignKey();
    void fillLV();
    void setTableName(GString aTabSchema, GString aTabName);
private slots:
    void newForeignKeyClicked();
    void addClicked();
    void remClicked();
    void addFkClicked();
    void remFkClicked();

    void timerEvent();
    void OKClicked();
    void tableSelected(QString table);

private:
    void tm(GString message){QMessageBox::information(this, "INDEX", message);}

    QGroupBox * createRadioButtons();
    GString iTabName, iTabSchema, errText;
    QPushButton * ok, * newInd, *addB, *remB, *addFkB, *remFkB;
    QLineEdit * indNameLE;
    QLabel * info;
    QListWidget * allLB, *selLB, * allFkLB, *selFkLB;

    CreateForeignKeyThread * aThread;
    ThreadBox * tb;
    QTimer *timer;
    DSQLPlugin * m_pDSQL;
    GString m_strFullTableName;
    GString m_strRefTableName;
    int m_iHideSystemTables;
    TableSelector * m_pTabSel;
    QRadioButton *cascadeRB;
    QRadioButton *noActionRB;
    QRadioButton *restrictRB;
    QRadioButton *setNullRB;
    QRadioButton *noneRB;



    void msg(GString message){QMessageBox::information(this, "Foreign Key", message);}
};

#endif
