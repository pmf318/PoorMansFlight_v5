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

#include <dsqlplugin.hpp>

#ifndef _newIndex_
#define _newIndex_

class idxColType : public QDialog
{
    Q_OBJECT
public:
    idxColType(QWidget* parent);
    QString selectedSort();
private:
    QRadioButton *radio0;
    QRadioButton *radio1;
    QRadioButton *radio2;
    QRadioButton *radio3;
    QRadioButton *radio4;
    QString m_Selected;

    //QGroupBox* createRadioButtons();
private slots:
    void ok();
};



class NewIndex : public QDialog
{

    class CreateIndexThread : public GThread
    {
    public:
        virtual void run();
        void setOwner( NewIndex * aNewIndex ) { myNewIndex = aNewIndex; }
    private:
        NewIndex * myNewIndex;
    };

    Q_OBJECT
public:
    void createIndex();
    NewIndex(DSQLPlugin* pDSQL, QWidget *parent=0);
    ~NewIndex();
    short fillLV();
    void setTableName(GString aTabSchema, GString aTabName);
private slots:
    void newIndClicked();
    void addClicked();
    void remClicked();
    void timerEvent();
    void OKClicked();
    void selDoubleClicked(QListWidgetItem* pItem);

private:
    void tm(GString message){QMessageBox::information(this, "INDEX", message);}
    GString iTabName, iTabSchema, errText;
    QPushButton * ok, * newInd, *addB, *remB;
    QLineEdit * indNameLE;
    QLabel * info;
    QListWidget * allLB, *selLB;
    QRadioButton * uniRB, *dupRB, *priRB;

    CreateIndexThread * aThread;
    ThreadBox * tb;
    QTimer *timer;
    DSQLPlugin * m_pDSQL;

    QGroupBox * createRadioButtons();
    GString removeOrder(GString in);
    GString createIndexCols();
};

#endif
