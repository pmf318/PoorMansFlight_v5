//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include "createCheck.h"
#include "helper.h"
#include <qlayout.h>
#include <QGridLayout>
#include <gstuff.hpp>
#include <QCloseEvent>
#include <QHeaderView>
#include <QTime>
#include <QDate>


#define CREATECHECK_GROUPNAME "CreateNewCheck"
#define CREATECHECK_NAME "Name"
#define CREATECHECK_CHECK "Statement"



CreateCheck::CreateCheck(DSQLPlugin* pDSQL, GString tableName, QWidget * parent)  :QDialog(parent)
{
    m_pDSQL = new DSQLPlugin(*pDSQL);
    m_strTableName = tableName;

    this->resize(500, 200);

    this->setWindowTitle("pmf");
    okB = new QPushButton(this);
    okB->setDefault(true);
    okB->setText("Save");
    connect(okB, SIGNAL(clicked()), SLOT(OKClicked()));

    closeB = new QPushButton(this);
    closeB->setDefault(true);
    closeB->setText("Exit");
    connect(closeB, SIGNAL(clicked()), SLOT(closeClicked()));

    QGridLayout * mainGrid = new QGridLayout(this);

    //OptionsTab * pFlds = createFields(CREATECHECK_GROUPNAME, this);
    QWidget * pWdgt = createFields(CREATECHECK_GROUPNAME);
    mainGrid->addWidget(pWdgt, 0, 0, 1, 4);
    mainGrid->addWidget(okB, 1, 0);
    mainGrid->addWidget(closeB, 1, 3);
}


CreateCheck::~CreateCheck()
{
    if( m_pDSQL ) delete m_pDSQL;
}


QWidget* CreateCheck::createFields(GString groupName)
{
    m_pTab = new OptionsTab(this, "CreateNewCheck");
    m_pTab->addRow( groupName, "To create a new check:");
    m_pTab->addRow( groupName, " - Name is case-sensitive.");
    m_pTab->addRow( groupName, " - Do not put statement in brackets.");
    m_pTab->addRow( groupName, " - Statement ex.: SomeCol > 0 and SomeCol < 1000");
    m_pTab->addRow( groupName, "");
    m_pTab->addRow( groupName, CREATECHECK_NAME, ROW_LINEEDIT, "", "", 0);
    m_pTab->addRow( groupName, CREATECHECK_CHECK, ROW_TEXTBOX, "", "", 3);
    return m_pTab->createGridWidget(groupName);
}

void CreateCheck::OKClicked()
{
    GString name = m_pTab->getFieldValue(CREATECHECK_GROUPNAME, CREATECHECK_NAME).strip();
    GString check = m_pTab->getTextBoxValue(CREATECHECK_GROUPNAME, CREATECHECK_CHECK).strip();
    if( !name.length() || !check.length() )
    {
        msg("Please fill all fields.");
        return;
    }

    GString cmd = "ALTER TABLE "+m_strTableName +" ADD CONSTRAINT \""+name+"\" CHECK ("+check+")";

    if( QMessageBox::question(this, "PMF", "Running command\n"+cmd+"\nContinue?", QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes )
    {
        return;
    }

    GString err = m_pDSQL->initAll(cmd);
    if( err.length() ) msg(err);
    else msg("Check created.");
}

void CreateCheck::closeClicked()
{
    close();
}

void CreateCheck::msg(GString txt)
{
    Helper::msgBox(this, "pmf", txt);
}
void CreateCheck::saveGeometry()
{
    m_qrGeometry = this->geometry();
}
void CreateCheck::restoreGeometry()
{
    this->setGeometry(m_qrGeometry);
}
void CreateCheck::closeEvent(QCloseEvent * event)
{
    event->accept();
}
void CreateCheck::keyPressEvent(QKeyEvent *event)
{
    if( event->key() == Qt::Key_Escape )
    {
        close();
    }
}
