//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include "multiImport.h"
#include "pmfdefines.h"
#include <gfile.hpp>

#include <QGridLayout>
#include <QHeaderView>
#include <QDir>

#include <QGroupBox>
#include <QObject>

#include "gstuff.hpp"
#include "helper.h"
#include "pmf.h"
#include "pmfSchemaCB.h"


MultiImport::MultiImport(GDebug *pGDeb, DSQLPlugin* pIDSQL, QWidget *parent)
  :QDialog(parent)
{
    m_pGDeb = pGDeb;

    deb("MultiImport start");
    m_pMainWdgt = parent;
	
    m_pIDSQL = pIDSQL;
	QGridLayout * pMainGrid = new QGridLayout(this);
	
    QLabel *inf1 = new QLabel("Tables and schemas were guessed from filenames.", this);
    QLabel *inf2 = new QLabel("Check if the mapping 'Filename to Table' is correct", this);
    pMainGrid->addWidget(inf1, 0 , 0, 1, 3);
    pMainGrid->addWidget(inf2, 1 , 0, 1, 3);

    this->resize(450, 400);
    ok   = new QPushButton(this);
    esc  = new QPushButton(this);
    ok->setText("OK");
	esc->setText("Cancel");
	
    connect(ok,   SIGNAL(clicked()), SLOT(okClicked()));
    connect(esc,  SIGNAL(clicked()), SLOT(cancel()));

	m_twMain = new QTableWidget(this);
	m_twMain->setSortingEnabled(true);
    m_twMain->setColumnCount(2);
    pMainGrid->addWidget(m_twMain, 2 , 0, 1, 3);


	QGroupBox * lowerBox = new QGroupBox();
	QHBoxLayout *lowerLayout = new QHBoxLayout;
	lowerLayout->addWidget(ok);
	lowerLayout->addWidget(esc);

	lowerBox->setLayout(lowerLayout);

    QTableWidgetItem * pItem2 = new QTableWidgetItem("Filename (source)");
    m_twMain->setHorizontalHeaderItem(0, pItem2);
    QTableWidgetItem * pItem3 = new QTableWidgetItem("Table (target)");
    m_twMain->setHorizontalHeaderItem(1, pItem3);
//    QTableWidgetItem * pItem4 = new QTableWidgetItem("Table");
//    m_twMain->setHorizontalHeaderItem(2, pItem4);

    pMainGrid->addWidget(lowerBox, 5 , 0, 1, 3);

}
MultiImport::~MultiImport()
{
}



void MultiImport::keyPressEvent(QKeyEvent * key)
{
    if( key->key() == Qt::Key_Escape ) 	this->close();
}

void MultiImport::closeEvent(QCloseEvent * event)
{
    this->setResult(QDialog::Rejected);
	event->accept();
}

void MultiImport::cancel()
{   
	close();
    this->setResult(QDialog::Rejected);
}

void MultiImport::okClicked()
{
    close();
    this->setResult(QDialog::Accepted);
}


void MultiImport::deb(GString msg)
{
    m_pGDeb->debugMsg("MultiImport", 1, msg);
}


int MultiImport::createRow(GString fileName, GString schema, GString table)
{    
    schema += "."+table;
    int rows = m_twMain->rowCount();
    m_twMain->insertRow(rows);

    /*
    QCheckBox * pChkBx   = new QCheckBox(this);
    GString cmd = "Select count(*) from syscat.tables where translate(tabschema)='"+schema.upperCase()+"' and translate(tabname)='"+table.upperCase()+"'";
    m_pIDSQL->initAll(cmd);
    if( m_pIDSQL->rowElement(1,1).asInt() != 1 )
    {
        schema = "<Not found>";
        table  = "<Not found>";
        pChkBx->setEnabled(false);
    }
    else pChkBx->setChecked(true);
    m_twMain->setCellWidget(rows, 0, pChkBx);
    */

    m_twMain->setItem(rows, 0, new QTableWidgetItem((char*)fileName));
    m_twMain->setItem(rows, 1, new QTableWidgetItem((char*)schema));
    //m_twMain->setItem(rows, 2, new QTableWidgetItem((char*)table));

    m_twMain->setRowHeight(rows, QFontMetrics( m_twMain->font()).height()+5);
    m_twMain->resizeColumnsToContents();
    return 0;
}




