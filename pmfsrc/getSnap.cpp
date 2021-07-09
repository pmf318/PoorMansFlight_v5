//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//


#include <qlayout.h>
#include <qfont.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QSettings>


#ifndef _getSnap_
#include "getSnap.h"
#endif

#ifdef MAKE_VC
#include <windows.h>
#endif

#include "threadBox.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include <gstuff.hpp>
#include "simpleShow.h"
//#include <udbapi.hpp>
#include "helper.h"
#include "monExport.h"
#include "pmfTableItem.h"

#ifndef pmf_H
#include "pmf.h"
#endif


#ifndef _extSQL_
#include "extSQL.h"
#endif


#if defined(MAKE_VC) || defined (__MINGW32__)
//HINSTANCE hDLL = 0;
#else
#include <dlfcn.h>
#endif



GetSnap::GetSnap(DSQLPlugin * pDSQL, Pmf *parent)
  :QDialog(parent)
{

    int buttonWidth = 100;

    m_pPmf = parent;
    QGridLayout * pMainGrid = new QGridLayout( this );

    QGroupBox * pRadioButtonGroupBox = new QGroupBox(this);

    m_pDSQL = pDSQL;

	this->resize(640, 480);
	//RadioButtons:
	databaseRB  = new QRadioButton("Database", this);
	connect(databaseRB, SIGNAL(clicked()), SLOT(OKClicked()));

	buffRB  = new QRadioButton("Buffer pool", this);
	connect(buffRB, SIGNAL(clicked()), SLOT(OKClicked()));

	lockRB  = new QRadioButton("Locks", this);
	connect(lockRB, SIGNAL(clicked()), SLOT(OKClicked()));

	dSQLRB  = new QRadioButton("Dynamic SQL", this);
	connect(dSQLRB, SIGNAL(clicked()), SLOT(OKClicked()));


	tableRB = new QRadioButton("Table Activity", this);
	connect(tableRB, SIGNAL(clicked()), SLOT(OKClicked()));

    resetBt = new QPushButton("Reset Monitor", this);
    connect(resetBt, SIGNAL(clicked()), SLOT(resetClicked()));

    hadrRB = new QRadioButton("HADR state", this);
    connect(hadrRB, SIGNAL(clicked()), SLOT(OKClicked()));
    hadrRB->setHidden(true);

    databaseRB->setChecked(true);

    ok = new QPushButton("Refresh", this);
    ok->setDefault(true);	
	connect(ok, SIGNAL(clicked()), SLOT(OKClicked()));
    ok->setMaximumWidth(buttonWidth);
    ok->setMinimumWidth(buttonWidth);

	exitBt = new QPushButton("Exit", this);
	connect(exitBt, SIGNAL(clicked()), SLOT(ExitClicked()));
    exitBt->setMaximumWidth(buttonWidth);
    exitBt->setMinimumWidth(buttonWidth);

    exportBt = new QPushButton("Export", this);
    connect(exportBt, SIGNAL(clicked()), SLOT(exportClicked()));
    exportBt->setMaximumWidth(buttonWidth);
    exportBt->setMinimumWidth(buttonWidth);


	info = new QLabel( "", this);

    filterLE = new QLineEdit(this);
    filterLE->setPlaceholderText("Filter for SQL Stmts");

	mainLV = new QTableWidget(this);
    mainLV->setSortingEnabled(true);
    mainLV->setWordWrap(false);
#if QT_VERSION >= 0x050000
    mainLV->horizontalHeader()->setSectionsMovable(true);
#else
    mainLV->horizontalHeader()->setMovable(true);
#endif
    mainLV->setAlternatingRowColors(true);

    QHBoxLayout *radioButtonLayout = new QHBoxLayout;
    radioButtonLayout->addWidget(databaseRB);
    radioButtonLayout->addWidget(buffRB);
    radioButtonLayout->addWidget(lockRB);
    radioButtonLayout->addWidget(dSQLRB);
    radioButtonLayout->addWidget(tableRB);
    //radioButtonLayout->addWidget(hadrRB);
    radioButtonLayout->addWidget(resetBt);
    pRadioButtonGroupBox->setLayout(radioButtonLayout);

    pMainGrid->addWidget(pRadioButtonGroupBox, 0, 0, 1, 5);
    pMainGrid->addWidget(mainLV, 1, 0, 1, 5);
    pMainGrid->addWidget(ok, 2, 0);
    pMainGrid->addWidget(exportBt, 2, 1);
    pMainGrid->addWidget(exitBt, 2, 2);
    pMainGrid->addWidget(info, 2, 3);
    pMainGrid->addWidget(filterLE, 2, 4);
    filterLE->hide();
	pluginLoaded = 0;

    QSettings settings(_CFG_DIR, "pmf5");
    this->restoreGeometry(settings.value("showMonitorGeometry").toByteArray());

	
	m_actRightClick= new QAction( "<Nothing to do here.>", this );
	m_actRightClick->setEnabled(false);
    connect( m_actRightClick, SIGNAL( triggered() ), this, SLOT( slotRightClick() ) );
	setContextMenuPolicy( Qt::ActionsContextMenu );

	addAction( m_actRightClick );
    connect((QWidget*)mainLV->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(sortClicked(int)));

    int rc = 0;
    m_pApiPlg = new DBAPIPlugin(m_pDSQL->getDBTypeName());
    if( !m_pApiPlg )
    {
        tm("Could not loas plugin, sorry");
        m_pApiPlg = NULL;
        rc = 1;
    }
    else
    {
        CON_SET conSet;
        m_pDSQL->currentConnectionValues(&conSet);
        rc = m_pApiPlg->initMonitor(conSet.DB, conSet.Host, conSet.UID, conSet.PWD);
        if( !rc ) rc = m_pApiPlg->startMonitor();
        if( rc ) tm("Could not initialize monitors: "+m_pApiPlg->SQLError());
    }
    if( rc )
    {
        databaseRB->setEnabled(false);
        buffRB->setEnabled(false);
        lockRB->setEnabled(false);
        dSQLRB->setEnabled(false);
        tableRB->setEnabled(false);
        resetBt->setEnabled(false);
        ok->setEnabled(false);
        exportBt->setEnabled(false);
    }
    exportBt->setEnabled(false);
}
GetSnap::~GetSnap()
{
//   udbapi aAPI;
//   aAPI.resetMonitor();
#ifdef MAKE_VC
   //FreeLibrary( hDLL );
#else
#endif
    QSettings settings(_CFG_DIR, "pmf5");
    settings.setValue("showMonitorGeometry", this->saveGeometry());
    if( m_pApiPlg ) delete m_pApiPlg;
}


void GetSnap::slotRightClick()
{
	
	if( 0 == mainLV->selectedItems().count() )
	{
		tm("Nothing selected");
		return;
	}
	QTableWidgetItem* pItem = mainLV->currentItem();
	if( !pItem ) return;

	if( dSQLRB->isChecked() )
	{
		GString txt = mainLV->item(mainLV->currentItem()->row(), 0)->text();
        SimpleShow *smp = new SimpleShow("SnapShot", this, true);
        smp->setText(txt);
        smp->setSqlHighlighter(m_pPmf->getColorScheme(), m_pPmf->sqlCmdSeq());
        smp->exec();
        delete smp;
	}
	if( lockRB->isChecked() )
	{
		GString appID, appName;
		//AgentID is in the first column
		//pItem = mainLV->item(mainLV->currentItem()->row(), 0);

		appID = mainLV->item(mainLV->currentItem()->row(), 0)->text();
		appName = mainLV->item(mainLV->currentItem()->row(), 1)->text();
		        
		if( QMessageBox::information(this, "Force", "Force Application "+appName+" (AgentID "+appID+")?", "Yes", "No", 0, 0, 1) ) return;
        int erc = m_pDSQL->forceApp(appID.asInt());

		if( erc ) tm("Force failed, erc: "+GString(erc));
		else OKClicked();
	}

}

void GetSnap::startThread()
{
   GetSnapThread * aThread = new GetSnapThread;
   ThreadBox * tb = new ThreadBox( this, "Please be patient", "Getting Snapshot", m_pDSQL->getDBTypeName() );
   aThread->setOwner( this );
   aThread->setBox(tb);
   aThread->start();
   #ifdef MAKE_VC
   tb->show();
   #else
   tb->exec();
   #endif
   aThread->setDone();
//   delete aThread;
//   tm("Done");
}

void GetSnap::GetSnapThread::run()
{
   //mySnap->OKClicked();
}

void GetSnap::OKClicked()
{
/*
	delete mainLV;
	mainLV = new QTableWidget(this);
	grid->addWidget(mainLV, 1, 0, 1, 5);
*/
   long erc;


   info->setText("");
   filterLE->hide();
   if( dSQLRB->isChecked() )
   {
       info->setText("Right click a row to see statement");
       filterLE->show();
   }
   if( lockRB->isChecked() ) info->setText("Right click a row to force application");


   m_actRightClick->setEnabled(false);
   if( dSQLRB->isChecked() )
   {
        m_actRightClick->setEnabled(true);
        m_actRightClick->setText("Show full statement");
        
   }
   if( lockRB->isChecked() )
   {
        m_actRightClick->setEnabled(true);
		m_actRightClick->setText("Force application / Delete connection");
   }
   fillLV();
   if( mainLV->rowCount() == 0 ) exportBt->setEnabled(false);
   else exportBt->setEnabled(true);
}

void GetSnap::AdvancedClicked()
{
   //dsqlapi aAPI;
//   aAPI.resetMonitor(dbName, 0);
}

void GetSnap::ExitClicked()
{
    close();
}

void GetSnap::resetClicked()
{
    if( QMessageBox::information(this, "pmf", "Reset Monitors for "+m_pDSQL->currentDatabase()+"?", "Yes", "No", 0, 0, 1) ) return;    
    int rc = m_pApiPlg->resetMonitor();
    if( rc ) tm("Reset failed, rc: "+GString(rc)+", "+m_pApiPlg->SQLError());
    else OKClicked();
}

short GetSnap::fillLV()
{
	if( dSQLRB->isChecked() ) getSQLData();
    if( buffRB->isChecked() ) getData(1);
    if( databaseRB->isChecked() ) getData(2);
    if( lockRB->isChecked() ) getData(3);
	//   if( sortRB->isChecked() ) getData(0);
    if( tableRB->isChecked() ) getData(4);
    if( hadrRB->isChecked() ) getData(5);
	//   if( UOWRB->isChecked() ) getData(0);
    Helper::setVHeader(mainLV);
	return 0;
}

short GetSnap::getData(int type)
{

    printf("getSnap::getData for type %i\n", type);
    /* Types:
    1: SQLMA_DBASE_BUFFERPOOLS
    2: SQLMA_DBASE
    3: SQLMA_DBASE_LOCKS
    4: SQLMA_DBASE_TABLES
    5: HADR
    */
	if( !type ) return 0;
	int erc;

    if( type == 5 )
    {
        erc = m_pApiPlg->getSnapshotData(type);
        if( erc ) m_pApiPlg->getSnapshotData(6);
        if( erc ) m_pApiPlg->getSnapshotData(7);
    }
    else erc = m_pApiPlg->getSnapshotData(type);
    if( erc )
    {
        tm("ErrCode: "+GString(erc)+" "+m_pApiPlg->SQLError());
        return 1;
    }
    fillListViewFromAPI(m_pApiPlg);
	return 0;
}
void GetSnap::fillListViewFromAPI(DBAPIPlugin *pAPI, GString dynSqlFilter)
{
    mainLV->setSortingEnabled(false);
    mainLV->setUpdatesEnabled(false);
    mainLV->clear();
    mainLV->setRowCount(0);

    if( !dSQLRB->isChecked() ) dynSqlFilter = "";

    GString data;
    //QTableWidgetItem * pItem;
    PmfTableItem * pItem;
    mainLV->setColumnCount(pAPI->getHeaderDataCount());
    for( int i = 1; i <= (int)pAPI->getHeaderDataCount(); ++i)
    {
        pAPI->getHeaderData(i, &data);
        pItem = new PmfTableItem((char*)data);
        mainLV->setHorizontalHeaderItem(i-1, pItem);
    }
    //mainLV->setRowCount(pAPI->getRowDataCount());
    int rows = 1;
    printf("--- Start fill\n");
    for(int i = 1; i <= (int)pAPI->getRowDataCount(); ++i)
    {

        if( dynSqlFilter.length() )
        {
            pAPI->getRowData(i, 1, &data);
            if( !data.upperCase().occurrencesOf(dynSqlFilter.upperCase()) ) continue;
        }
        mainLV->setRowCount(rows);

        for(int j = 1; j <= (int)pAPI->getHeaderDataCount(); ++j)
        {
            GString head;
            pAPI->getHeaderData(j, &head);
            pAPI->getRowData(i, j, &data);
            //printf("Head: %s, Data is: %s\n", (char*) head, (char*) data);

              //pItem = new QTableWidgetItem((char*)data);
            pItem = new PmfTableItem();
            if( data.isDigits())
            {
                pItem->setData(Qt::DisplayRole, qlonglong(data.asLongLong()));
                pItem->setTextAlignment( Qt::AlignRight | Qt::AlignVCenter );
            }
            else if( GString(data).removeAll('.').isDigits() )
            {
                pItem->setData(Qt::EditRole, QVariant((char*)data));
                pItem->setTextAlignment( Qt::AlignRight | Qt::AlignVCenter);
            }
            else pItem->setData(Qt::DisplayRole, QString::fromLocal8Bit(data));
            mainLV->setItem(rows-1, j-1, pItem);
        }
        rows++;
    }
    mainLV->setSortingEnabled(true);
    mainLV->setUpdatesEnabled(true);
    Helper::setVHeader(mainLV);
}

short GetSnap::getSQLData()
{
	int erc;
   
    CON_SET conSet;
    m_pDSQL->currentConnectionValues(&conSet);
    erc = m_pApiPlg->getDynSQLSnapshotData();
    if( !erc )
    {
        fillListViewFromAPI(m_pApiPlg, filterLE->text());
    }
    else tm("ErrCode: "+GString(erc)+" "+m_pApiPlg->SQLError());
    return 0;
}

void GetSnap::sortClicked(int)
{
    Helper::setVHeader(mainLV);
}

void GetSnap::exportClicked()
{
    MonExport foo(m_pDSQL, getTitle(), mainLV, this);
    foo.exec();
}

GString GetSnap::getTitle()
{
    if( dSQLRB->isChecked() ) return "DynamicSQL";
    if( buffRB->isChecked() ) return "BufferPool";
    if( databaseRB->isChecked() ) return "Database";
    if( lockRB->isChecked() ) return "Locks";
    if( tableRB->isChecked() ) return "TableActivity";
    if( hadrRB->isChecked() ) return "HADR";
    return "<NotSet>";
}

//void GetSnap::keyPressEvent(QKeyEvent *event)
//{

//    switch (event->key())
//    {

//    case Qt::Key_F5:
//        OKClicked();
//        break;
//    }
//    QWidget::keyPressEvent(event);
//}
