//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include "tabSpace.h"
#include "helper.h"
#include <qlayout.h>
#include <qfont.h>
#include <QGridLayout>
#include <QLabel>


#ifndef _tabSpace_
#include <tabSpace.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>


#include <dsqlplugin.hpp>

TabSpace::TabSpace(DSQLPlugin* pDSQL, QWidget *parent)
  :QDialog(parent) //, f("Charter", 48, QFont::Bold)
{
    m_pIDSQL = pDSQL;

	this->resize(680, 400);
	QGridLayout * grid = new QGridLayout( this );

	info = new QLabel(this);

	grid->addWidget(info, 0, 0, 1, 5);

    ok = new QPushButton("Exit", this);
    ok->setDefault(true);

    reduceMaxBt = new QPushButton("Reduce MAX", this);

	connect(ok, SIGNAL(clicked()), SLOT(OKClicked()));
    connect(reduceMaxBt, SIGNAL(clicked()), SLOT(reduceMaxClicked()));
    grid->addWidget(ok, 2, 4);
    grid->addWidget(reduceMaxBt, 2, 0);

	mainLV = new QTableWidget(this);

	mainLV->setGeometry( 20, 20, 420, 300);
    mainLV->setSelectionBehavior(QAbstractItemView::SelectRows);
	grid->addWidget(mainLV, 1, 0, 1, 5);

    m_pApi = new DBAPIPlugin(m_pIDSQL->getDBTypeName());
    if( !m_pApi->isValid() )
    {
        QMessageBox::information(0, "UDB API", "Could not load the required plugin, sorry.");
        return;
    }
    connect((QWidget*)mainLV->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(sortClicked(int)));
    fillLV();
}
TabSpace::~TabSpace()
{
	if( m_pApi ) delete m_pApi;
}

void TabSpace::reduceMaxClicked()
{
    QItemSelectionModel* selectionModel = mainLV->selectionModel();
    QModelIndexList selected = Helper::getSelectedRows(selectionModel);
    if( selected.count() == 0 )
    {
        Helper::msgBox(this, "PMF", "Select tablespace(s)");
        return;
    }
    GString msg = "This will run 'ALTER TABLESPACE [TableSpace] REDUCE MAX' which should reclaim unused space.\nContinue?";
    if( QMessageBox::question(this, "PMF", msg, "Yes", "No", 0, 1) != 0 ) return;
    for(int i= 0; i< selected.count();i++)
    {
        QModelIndex index = selected.at(i);
        int pos = index.row();
        GString tabSpace = GString(mainLV->item(pos,1)->text());

        GString cmd = "ALTER TABLESPACE "+tabSpace+" REDUCE MAX";
        GString err = m_pIDSQL->initAll(cmd);
        if( err.length() ) Helper::msgBox(this, "PMF", err);
        else Helper::msgBox(this, "PMF", "Done. It may take several minutes to see changes.");
    }
}

void TabSpace::OKClicked()
{
    close();
}
void TabSpace::setTableName(GString aTableName)
{
	return;
	if( !aTableName.length() )
	{
		info->setText("Please select a table schema first.");
		return;
	}
	iTabSchema = aTableName.subString(1, aTableName.indexOf('.') - 1);
	iTabName   = aTableName.subString(aTableName.indexOf('.') + 1, aTableName.length()).strip();
	info->setText("Table spaces:");
	fillLV();
}


short TabSpace::fillLV()
{
	if( !m_pApi ) return 1;
	
	QTableWidgetItem * pItem;	
	mainLV->setColumnCount(10);
	pItem = new QTableWidgetItem("ID");
	mainLV->setHorizontalHeaderItem(0, pItem);
	pItem = new QTableWidgetItem("Name");
	mainLV->setHorizontalHeaderItem(1, pItem);
	pItem = new QTableWidgetItem("Type");
	mainLV->setHorizontalHeaderItem(2, pItem);
	pItem = new QTableWidgetItem("Contents");
	mainLV->setHorizontalHeaderItem(3, pItem);
	pItem = new QTableWidgetItem("State");
	mainLV->setHorizontalHeaderItem(4, pItem);
	pItem = new QTableWidgetItem("Total Pages");
	mainLV->setHorizontalHeaderItem(5, pItem);
	pItem = new QTableWidgetItem("Usable Pages");
	mainLV->setHorizontalHeaderItem(6, pItem);
	pItem = new QTableWidgetItem("Used Pages");
	mainLV->setHorizontalHeaderItem(7, pItem);
	pItem = new QTableWidgetItem("Free Pages");
	mainLV->setHorizontalHeaderItem(8, pItem);
	pItem = new QTableWidgetItem("High Water Mark ");
	mainLV->setHorizontalHeaderItem(9, pItem);
	
    GSeq <TAB_SPACE*> dataSeq;
    m_pApi->initTabSpaceLV(&dataSeq);

    mainLV->setRowCount(dataSeq.numberOfElements());
    for( int i = 1; i <= (int)dataSeq.numberOfElements(); ++i)
    {
        pItem = new QTableWidgetItem((char*) dataSeq.elementAtPosition(i)->Id);
        pItem->setFlags(pItem->flags() ^ Qt::ItemIsEditable);
        mainLV->setItem(i-1, 0, pItem);        
        pItem = new QTableWidgetItem((char*) dataSeq.elementAtPosition(i)->Name);
        pItem->setFlags(pItem->flags() ^ Qt::ItemIsEditable);
        mainLV->setItem(i-1, 1, pItem);
        pItem = new QTableWidgetItem((char*) dataSeq.elementAtPosition(i)->Type);
        pItem->setFlags(pItem->flags() ^ Qt::ItemIsEditable);
        mainLV->setItem(i-1, 2, pItem);
        pItem = new QTableWidgetItem((char*) dataSeq.elementAtPosition(i)->Contents);
        pItem->setFlags(pItem->flags() ^ Qt::ItemIsEditable);
        mainLV->setItem(i-1, 3, pItem);
        pItem = new QTableWidgetItem((char*) dataSeq.elementAtPosition(i)->State);
        pItem->setFlags(pItem->flags() ^ Qt::ItemIsEditable);
        mainLV->setItem(i-1, 4, pItem);
        pItem = new QTableWidgetItem((char*) dataSeq.elementAtPosition(i)->TotalPages);
        pItem->setFlags(pItem->flags() ^ Qt::ItemIsEditable);
        mainLV->setItem(i-1, 5, pItem);
        pItem = new QTableWidgetItem((char*) dataSeq.elementAtPosition(i)->UsablePages);
        pItem->setFlags(pItem->flags() ^ Qt::ItemIsEditable);
        mainLV->setItem(i-1, 6, pItem);
        pItem = new QTableWidgetItem((char*) dataSeq.elementAtPosition(i)->UsedPages);
        pItem->setFlags(pItem->flags() ^ Qt::ItemIsEditable);
        mainLV->setItem(i-1, 7, pItem);
        pItem = new QTableWidgetItem((char*) dataSeq.elementAtPosition(i)->FreePages);
        pItem->setFlags(pItem->flags() ^ Qt::ItemIsEditable);
        mainLV->setItem(i-1, 8, pItem);
        pItem = new QTableWidgetItem((char*) dataSeq.elementAtPosition(i)->HighWaterMark);
        pItem->setFlags(pItem->flags() ^ Qt::ItemIsEditable);
        mainLV->setItem(i-1, 9, pItem);


    }

    Helper::setVHeader(mainLV);
    TAB_SPACE * tabSpace;
    while(dataSeq.numberOfElements())
    {
        tabSpace = dataSeq.elementAtPosition(1);
        delete tabSpace;
        dataSeq.removeFirst();
    }
	return 0;
}

void TabSpace::sortClicked(int)
{
    Helper::setVHeader(mainLV);
}
