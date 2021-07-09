//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include "catalogInfo.h"
#include "helper.h"
#include <qlayout.h>
#include <qfont.h>
#include <QGridLayout>
#include <QLabel>


#ifndef _catalogInfo_
#include <catalogInfo.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>


#include <dsqlplugin.hpp>
#include "catalogDB.h"
#include "connSet.h"

#define COL_DBNAME 1
#define COL_NODE   3

CatalogInfo::CatalogInfo(DSQLPlugin* pDSQL, QWidget *parent)  :QDialog(parent) //, f("Charter", 48, QFont::Bold)
{
    m_pIDSQL = pDSQL;

    this->resize(680, 400);
    setWindowTitle("Databases");
    QGridLayout * grid = new QGridLayout( this );
    info = new QLabel(this);

    grid->addWidget(info, 0, 0, 1, 5);
    ok = new QPushButton("Exit", this);
    ok->setDefault(true);
    newB = new QPushButton("Catalog DB/Node", this);
    uncNodeB = new QPushButton("Uncatalog Node", this);
    uncDatabaseB = new QPushButton("Uncatalog DB", this);

    connect(newB, SIGNAL(clicked()), SLOT(newClicked()));
    connect(uncNodeB, SIGNAL(clicked()), SLOT(uncNodeClicked()));
    connect(uncDatabaseB, SIGNAL(clicked()), SLOT(uncDatabaseClicked()));
    connect(ok, SIGNAL(clicked()), SLOT(OKClicked()));
    grid->addWidget(newB, 2, 0);
    grid->addWidget(uncNodeB, 2, 1);
    grid->addWidget(uncDatabaseB, 2, 2);
    grid->addWidget(ok, 2, 4);

    mainLV = new QTableWidget(this);
    mainLV->setGeometry( 20, 20, 420, 300);
    mainLV->setSelectionBehavior(QAbstractItemView::SelectRows);
    grid->addWidget(mainLV, 1, 0, 1, 5);

    if( m_pIDSQL == NULL )pApi = new DBAPIPlugin(_DB2);
    else pApi = new DBAPIPlugin(m_pIDSQL->getDBTypeName());
    if( !pApi->isValid() )
    {
        QMessageBox::information(0, "UDB API", "Could not load the required plugin, sorry.");        
        return;
    }
    connect((QWidget*)mainLV->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(sortClicked(int)));
    fillLV();
}
CatalogInfo::~CatalogInfo()
{
    if( pApi ) delete pApi;
}


void CatalogInfo::OKClicked()
{
    close();
}

void CatalogInfo::newClicked()
{
    CatalogDB foo(m_pIDSQL, this);
    foo.exec();
    if( foo.catalogChanged() ) fillLV();
}

void CatalogInfo::uncatalogItem(int colNr)
{
    QItemSelectionModel* selectionModel = mainLV->selectionModel();
    QModelIndexList selected = Helper::getSelectedRows(selectionModel);

    if( selected.count() == 0 )
    {
        tm("Nothing selected.");
        return;
    }

    if( colNr == COL_DBNAME )
    {
        if( QMessageBox::question(this, "PMF", "Uncatalog selected databases?", "Yes", "No", 0, 1) != 0 ) return;
    }
    else if( colNr == COL_NODE )
    {
        if( QMessageBox::question(this, "PMF", "Uncatalog selected nodes?\n(Note:Databases will be uncatalogued too)", "Yes", "No", 0, 1) != 0 ) return;
    }
    else
    {
        return;
    }
    int erc = 0;
    for(int i= 0; i< selected.count();i++)
    {
        QModelIndex index = selected.at(i);
        int pos = index.row();
        GString database = GString(mainLV->item(pos, COL_DBNAME)->text());
        GString node     = GString(mainLV->item(pos, COL_NODE)->text());
        if( colNr == COL_DBNAME ) erc = uncatalogDB(pApi, database, node);
        else if( colNr == COL_NODE )  erc = uncatalogNode(pApi, node);
        if( erc == 123 ) return;
    }    

//    CON_SET conSet;
//    m_pIDSQL->currentConnectionValues(&conSet);
    refreshDirectory();
//    if( !erc && QMessageBox::question(this, "PMF", "Do you want to refresh the local node cache (i.e. DB2 TERMINATE)?", "Yes", "No", 0, 1) == 0 )
//    {
//        if( pApi->dbRestart(conSet.DB, conSet.UID, conSet.PWD) )
//        {
//            if( pApi->dbRestart(conSet.DB, "", "") ) tm("Could not restart: "+pApi->SQLError());
//        }
//    }
    fillLV();
}

int CatalogInfo::uncatalogNode(DBAPIPlugin* pApi, GString name)
{
    if( !name.strip().length() )
    {
        if( QMessageBox::question(this, "PMF", "This appears to be a local node, uncatalog anyway (not recommended)?", "Yes", "No", 0, 1) != 0 ) return 123;
    }
    GSeq <DB_INFO*> dataSeq;
    dataSeq = pApi->dbInfo();
    int erc;
    for( int i = 1; i <= (int)dataSeq.numberOfElements(); ++i)
    {
        if( name != dataSeq.elementAtPosition(i)->NodeName ) continue;
        erc = pApi->uncatalogDatabase(dataSeq.elementAtPosition(i)->Alias);
        if( erc ) tm("Could not uncatalog database "+dataSeq.elementAtPosition(i)->Alias+", error: "+pApi->SQLError());
    }
    erc = pApi->uncatalogNode(name);
    erc = erc == -1021 ? 0 : erc;
    if( erc ) tm("Could not uncatalog node "+name+", error: "+pApi->SQLError());
    return erc;
}


int CatalogInfo::uncatalogDB(DBAPIPlugin* pApi, GString name, GString node)
{
    if( !node.strip().length() )
    {
        if( QMessageBox::question(this, "PMF", "This appears to be a local database, uncatalog anyway (not recommended)?", "Yes", "No", 0, 1) != 0 ) return 123;
    }
    int erc = pApi->uncatalogDatabase(name);    
    if( erc ) tm("Could not uncatalog database "+name+", error: "+pApi->SQLError());
    else
    {
        ConnSet::removeFromList(_DB2ODBC, name);
        ConnSet::removeFromList(_DB2, name);
    }
    return erc;
}


void CatalogInfo::uncNodeClicked()
{
    uncatalogItem(COL_NODE);
}

void CatalogInfo::uncDatabaseClicked()
{
    uncatalogItem(COL_DBNAME);
}

short CatalogInfo::fillLV()
{
    mainLV->clear();
    mainLV->setRowCount( 0);
    mainLV->setColumnCount(0);
    NODE_INFO* pNode;

    GSeq <DB_INFO*> dataSeq;
    dataSeq = pApi->dbInfo();

    GSeq<NODE_INFO*> nodeSeq;
    nodeSeq = pApi->getNodeInfo();

    QTableWidgetItem * pItem;
    mainLV->setColumnCount(7);
    pItem = new QTableWidgetItem("Database");
    mainLV->setHorizontalHeaderItem(0, pItem);
    pItem = new QTableWidgetItem("Alias");
    mainLV->setHorizontalHeaderItem(1, pItem);
    pItem = new QTableWidgetItem("Drive");
    mainLV->setHorizontalHeaderItem(2, pItem);
    pItem = new QTableWidgetItem("NodeName");
    mainLV->setHorizontalHeaderItem(3, pItem);
    pItem = new QTableWidgetItem("Host");
    mainLV->setHorizontalHeaderItem(4, pItem);
    pItem = new QTableWidgetItem("Comment");
    mainLV->setHorizontalHeaderItem(5, pItem);
    pItem = new QTableWidgetItem("DB Type");
    mainLV->setHorizontalHeaderItem(6, pItem);

    mainLV->setRowCount(dataSeq.numberOfElements());
    for( int i = 1; i <= (int)dataSeq.numberOfElements(); ++i)
    {
        pNode = nodeFromList( &nodeSeq, dataSeq.elementAtPosition(i)->NodeName);


        mainLV->setItem(i-1, 0, createItem(dataSeq.elementAtPosition(i)->Database));
        mainLV->setItem(i-1, 1, createItem(dataSeq.elementAtPosition(i)->Alias));
        mainLV->setItem(i-1, 2, createItem(dataSeq.elementAtPosition(i)->Drive));
        mainLV->setItem(i-1, 3, createItem(dataSeq.elementAtPosition(i)->NodeName));
        mainLV->setItem(i-1, 4, createItem(pNode->HostName));
        mainLV->setItem(i-1, 5, createItem(dataSeq.elementAtPosition(i)->Comment));
        mainLV->setItem(i-1, 6, createItem(dataSeq.elementAtPosition(i)->DbType));
    }
    Helper::setVHeader(mainLV);

    DB_INFO * DbInfo;
    while(dataSeq.numberOfElements())
    {
        DbInfo = dataSeq.elementAtPosition(1);
        delete DbInfo;
        dataSeq.removeFirst();
    }
    while(nodeSeq.numberOfElements())
    {
        pNode = nodeSeq.elementAtPosition(1);
        delete pNode;
        nodeSeq.removeFirst();
    }
    return 0;
}

QTableWidgetItem * CatalogInfo::createItem(GString text)
{
    QTableWidgetItem* pItem = new QTableWidgetItem();
    pItem->setFlags(pItem->flags() ^ Qt::ItemIsEditable);
    pItem->setData(Qt::DisplayRole, QString::fromLocal8Bit(text));
    return pItem;
}

NODE_INFO * CatalogInfo::nodeFromList(GSeq<NODE_INFO*> *nodeSeq,  GString nodeName)
{
    for(int i = 1; i <= nodeSeq->numberOfElements(); ++i )
    {
        if( nodeSeq->elementAtPosition(i)->NodeName == nodeName ) return nodeSeq->elementAtPosition(i);
    }
    return new NODE_INFO;
}

void CatalogInfo::sortClicked(int)
{
    Helper::setVHeader(mainLV);
}

void CatalogInfo::refreshDirectory()
{
    tm("Please restart PMF to refresh diretory cache.");
//    system("db2 terminate");
//    pApi->dbInfo();
//    pApi->getNodeInfo();
}


