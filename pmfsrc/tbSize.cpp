//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//
#include <QModelIndex>

#include <qlayout.h>
#include <qfont.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QLabel>


#include "finddbl.h"
#include "helper.h"


#ifndef _tbSize_
#include "tbSize.h"
#endif
#ifndef _tabSpace_
#include "tabSpace.h"
#endif
#ifndef _reorgAll_
#include "reorgAll.h"
#endif
TableSize::TableSize(DSQLPlugin* pDSQL, QWidget *parent, GString currentSchema, int hideSysTabs )
  :QDialog(parent) //, f("Charter", 48, QFont::Bold)
{
    m_pDSQL = new DSQLPlugin(*pDSQL);
	this->resize(680, 400);
	QGridLayout * grid = new QGridLayout(this);

	info = new QLabel(this);
	grid->addWidget(info, 0, 0, 1, 4);

    schemaCB = new PmfSchemaCB(this, currentSchema);
	grid->addWidget(schemaCB, 1, 0);
	
	mainLV = new QTableWidget(this);
	grid->addWidget(mainLV, 2, 0, 1, 4);

	mainLV->setSelectionBehavior(QAbstractItemView::SelectRows);
	mainLV->setSelectionMode(QAbstractItemView::SingleSelection);
	rstatB = new QPushButton(this);
    rstatB->setText("Reorg+Runstats");
	connect(rstatB, SIGNAL(clicked()), SLOT(runStatsClicked()));
	grid->addWidget(rstatB, 3, 1);
	

	ok = new QPushButton(this);
	ok->setText("Exit");
	connect(ok, SIGNAL(clicked()), SLOT(OKClicked()));
	grid->addWidget(ok, 3, 2);
	
    m_iHideSysTabs = hideSysTabs;
    schemaCB->fill(pDSQL, currentSchema, hideSysTabs);
	connect(schemaCB, SIGNAL(activated(int)), SLOT(schemaSelected(int)));	
    connect((QWidget*)mainLV->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(sortClicked(int)));
    setSchema(currentSchema);
}

TableSize::~TableSize()
{
	delete m_pDSQL;
}

void TableSize::OKClicked()
{
	close();
}
void TableSize::setSchema(GString aTabSchema)
{	
   if( !aTabSchema.length() )
   {
      info->setText("Please select a table schema first.");
      return;
   }
   iTabSchema = aTabSchema;
   QFont font  = info->font();
   font.setBold(true);
   info->setText("Tables in schema "+iTabSchema+"    Hint: Do REORG+RUNSTATS on tables with negative size.");
   fillLV();
}

short TableSize::fillLV()
{
 
	QTableWidgetItem * pItem;

	unsigned int i, j;

	mainLV->clear();

    GString err = m_pDSQL->initAll("select substr(tabname,1,32) TABNAME,npages*pagesize / 1024  as \"Used (kB)\", "
				"pagesize*fpages / 1024 as \"Allocated (kB)\", a.TBSPACE from syscat.tables a, "
					"syscat.tablespaces b where a.tbspace=b.tbspace and a.tabschema = '"+iTabSchema+"' order by tabname");
	if( err.length() )
	{
		tm("I'm guessing that your database is on some AS400/VM/VMS/... system. Please contact me (pmf@leipelt.net), I need your help to get this working on your system.");
	}
    mainLV->setColumnCount(m_pDSQL->numberOfColumns());
    mainLV->setRowCount(m_pDSQL->numberOfRows());
    for( i = 1; i <= m_pDSQL->numberOfColumns(); ++i )
	{
        pItem = new QTableWidgetItem((char*)m_pDSQL->hostVariable(i));
		mainLV->setHorizontalHeaderItem(i-1, pItem);
	}
    GString data;
    for( i = 1; i <= m_pDSQL->numberOfRows(); ++i )
	{		
        for( j = 1; j <= m_pDSQL->numberOfColumns(); ++ j )
		{
            data = m_pDSQL->rowElement(i, j).strip("'").strip();
            pItem = new QTableWidgetItem();
            if( data.isDigits() ) pItem->setData(Qt::DisplayRole, qlonglong(data.asLong()));
            else pItem->setData(Qt::DisplayRole, QString::fromLocal8Bit(data));
            pItem->setFlags(pItem->flags() ^ Qt::ItemIsEditable);
			mainLV->setItem(i-1, j-1, pItem);
		}
	}
    Helper::setVHeader(mainLV);
	return 0;
}

void TableSize::tabSpaceClicked()
{
    TabSpace* tb = new TabSpace(m_pDSQL,this);
	//tb->setTableName(iTableName);
	#ifdef MAKE_VC
	tb->show();
	#else
	tb->exec();
	#endif
}
void TableSize::runStatsClicked()
{
	GString tabName;
	QItemSelectionModel* selectionModel = mainLV->selectionModel();
    QModelIndexList selected = Helper::getSelectedRows(selectionModel);
	
	//QModelIndexList selected = selectionModel->selectedRows();
	QTableWidgetItem * pItem;
	if( selected.count() == 0 )
	{
		tm("Select a table");
		return;
	}
	
	for(int i= 0; i< selected.count();i++)
	{
	 	QModelIndex index = selected.at(i);
		pItem = mainLV->item(index.row(), 0);
		tabName = iTabSchema+"."+GString(pItem->text());
	}	
	
    DSQLPlugin *pDSQL = new DSQLPlugin(*m_pDSQL);
    ReorgAll ra(pDSQL, this, tabName, m_iHideSysTabs);
    ra.setTabIndex(4); //jump to RUNSTATS page
	ra.exec();
	delete pDSQL;
	fillLV();
    
}
void TableSize::schemaSelected(int index)
{
	GString schema = schemaCB->itemText(index);
	setSchema(schema);
}   
void TableSize::tm(GString message)
{
    #ifdef MAKE_VC
    _flushall();
	#endif
	QMessageBox::information(this, "PMF", message);
}

void TableSize::sortClicked(int)
{
    Helper::setVHeader(mainLV);
}

