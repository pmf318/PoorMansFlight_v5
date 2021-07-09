//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//
#include "finddbl.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h> 
//#include <windows.h> 

#include <qbuttongroup.h>
#include <qlabel.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <gstring.hpp>
#include <qlayout.h>
#include <gstuff.hpp>
#include <QProgressDialog>




Finddbl::Finddbl(GDebug *pGDeb, DSQLPlugin * pDSQL, QWidget *parent, GString currentSchema, int hideSysTabs )
 :QDialog(parent)
{
        QBoxLayout *topLayout = new QVBoxLayout(this);
        QGridLayout * grid = new QGridLayout();
        topLayout->addLayout(grid, 2);

    m_pGDeb = pGDeb;

    m_pDSQL = pDSQL;
	QLabel* tmpQLabel;
	tmpQLabel = new QLabel( this);
	tmpQLabel->setGeometry( 20, 10, 10, 30 );
	tmpQLabel->setText( "Select Table(s)" );
	//tmpQLabel->setAlignment( 289 );
	grid->addWidget(tmpQLabel, 0, 0);

    tbSel = new TableSelector(pDSQL, parent, currentSchema, hideSysTabs);
    grid->addWidget(tbSel, 1, 0, 3, 2);
	
	displayRB = new QRadioButton( this );
	displayRB->setText( "Display Identical Rows" );
    displayRB->setAutoRepeat( false );
    displayRB->setChecked( true );
	grid->addWidget(displayRB, 1, 4);
	
	deleteRB = new QRadioButton( this );
	deleteRB->setText( "Display And DELETE Identical Rows" );
    deleteRB->setAutoRepeat( false );
	grid->addWidget(deleteRB, 2, 4);
	
	resultLB = new QListWidget( this );
    resultLB->setGeometry( 240, 210, 400, 240 );
	resultLB->setFrameStyle( 51 );
	resultLB->setLineWidth( 2 );
    grid->addWidget(resultLB, 3, 4, 1, 1);

	okB = new QPushButton( this );
	connect( okB, SIGNAL(clicked()), SLOT(okClicked()) );
	okB->setText( "Go!" );
	grid->addWidget(okB, 5, 0);

	cancelB = new QPushButton( this );
	connect( cancelB, SIGNAL(clicked()), SLOT(cancelClicked()) );
	cancelB->setText( "Cancel" );
    cancelB->setAutoRepeat( false );
	grid->addWidget(cancelB, 5, 1);
	
    //cancelB->setAutoResize( false );

	tmpQLabel = new QLabel( this );
	tmpQLabel->setText( "Results:" );
	//tmpQLabel->setAlignment( 289 );
	
    grid->addWidget(tmpQLabel, 0, 4);
	
	
    resize(800, 400);

	/******************** Not nice
	QGroupBox *leftGroupBox = new QGroupBox(tr("Title goes here"));
	QVBoxLayout *leftVbox = new QVBoxLayout;
	leftVbox->addWidget(tmpQLabel);
	leftVbox->addWidget(schemaCB);
	leftVbox->addWidget(tableLB);
	leftVbox->addStretch(2);
	leftGroupBox->setLayout(leftVbox);
	grid->addWidget(leftGroupBox, 0, 0, 5, 3);


	
	QGroupBox *rightGroupBox = new QGroupBox(tr("Title goes here"));
	QVBoxLayout *rightVbox = new QVBoxLayout;
	rightVbox->addWidget(displayRB);
	rightVbox->addWidget(deleteRB);
	rightVbox->addWidget(resultLB);
	rightVbox->addStretch(2);
	rightGroupBox->setLayout(rightVbox);
	grid->addWidget(rightGroupBox, 0, 7, 5, 3);
        stopIt = 0;
	resize( 660, 500 );
	fillSchemaCB(currentSchema);
	**********************/
}


Finddbl::~Finddbl()
{
}

void Finddbl::okClicked()
{
    QListWidget *pLB = tbSel->getTableHandle();
	iStop = 0;
	short i, count = 0;
	resultLB->clear();
    for( i=0; i < pLB->count(); ++i)
	{
        if( pLB->isItemSelected(pLB->item(i)) ) count++;
	}
	if( !count )
	{
		QMessageBox::information(this, "PMF", "No tables selected.");
		return;
	}

	if( deleteRB->isChecked() )
	{
        GString text =  "You selected to delete identical Rows.\nBackup your table(s) FIRST!\n\nContinue?" ;
        if( QMessageBox::warning(this, "CAUTION", text, "Yes", "No", 0, 1) != 0 ) return;
	}
    checkTables(pLB);
 
}
void Finddbl::checkTables(QListWidget *pLB)
{
	int erc;
    GString tabName;
    for( int i=0; i < pLB->count(); ++i)
	{
        if( pLB->isItemSelected(pLB->item(i)) )
		{
            if( m_pDSQL->getDBTypeName() == _MARIADB ) tabName = tbSel->tablePrefix()+GString(pLB->item(i)->text());
            else tabName = tbSel->tablePrefix()+"\""+GString(pLB->item(i)->text())+"\"";
            erc = findInTable( tabName );
			if( erc ) break;
            pLB->item(i)->setSelected(false);
            pLB->scrollToItem(pLB->item(i));
		}
	}
}
int Finddbl::findInTable(GString table)
{
    m_pDSQL->setStopThread(0);

	short autoDel;
	if( deleteRB->isChecked() ) autoDel = 1;
	else autoDel = 0;
	identRowSeq.removeAll();

    DSQLPlugin * pDSQL = new DSQLPlugin(*m_pDSQL);
    pDSQL->setGDebug(m_pGDeb);
    GSeq <GString> unqColSeq;

    pDSQL->getUniqueCols(table, &unqColSeq);

    if( unqColSeq.numberOfElements() > 0)
    {
        GString msg = "["+table+" has unique key(s), not checking]";
        new QListWidgetItem(msg, resultLB);
        return 0;
    }
	GString err = pDSQL->getIdenticals(table, this, resultLB, autoDel);
    if( err == "QUIT" ) return 1;
	else if( err.length() ) 
	{
        tm("Could not check table "+table+"\n"+err);
        //return 1;
	}
    delete pDSQL;

   return 0;
}
void Finddbl::addToLB(GString message)
{
	for( int i = 0; i < resultLB->count(); ++i )
		if( GString(resultLB->item(i)->text()) == message ) return;
	new QListWidgetItem(message, resultLB);
}
void Finddbl::cancelClicked()
{
   iStop = 1;
   m_pDSQL->setStopThread(1);
   close();
}


void Finddbl::tm(GString message)
{
   QMessageBox::information(this, "Find", message);
}


