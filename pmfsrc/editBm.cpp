//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//


#include <qlayout.h>
#include <qfont.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QLabel>

#include "bookmark.h"
#include "helper.h"

#ifndef _editBm_
#include "editBm.h"
#endif
EditBm::EditBm(GDebug *pGDeb, QWidget *parent)
  :QDialog(parent) //, f("Charter", 48, QFont::Bold)
{
    m_pGDeb = pGDeb;
	this->resize(480, 250);
	QGridLayout * grid = new QGridLayout(this);

    info = new QLabel(this);
    info->setText("Use Bookmarks->Add bookmark to add bookmarks");
    grid->addWidget(info, 0, 0, 1, 4);


    mainLV = new QTableWidget(this);
	grid->addWidget(mainLV, 2, 0, 1, 4);

	mainLV->setSelectionBehavior(QAbstractItemView::SelectRows);
	mainLV->setSelectionMode(QAbstractItemView::SingleSelection);

	exitButton = new QPushButton(this);
	exitButton->setText("Exit");
	connect(exitButton, SIGNAL(clicked()), SLOT(exitClicked()));


	saveButton = new QPushButton(this);
	saveButton->setText("Save");
	connect(saveButton, SIGNAL(clicked()), SLOT(OKClicked()));

	delButton = new QPushButton(this);
	delButton->setText("Delete");
	connect(delButton, SIGNAL(clicked()), SLOT(delClicked()));

	grid->addWidget(delButton, 3, 1);
	grid->addWidget(saveButton, 3, 2);
	grid->addWidget(exitButton, 3, 3);
    connect((QWidget*)mainLV->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(sortClicked(int)));
	fillLV();

}
EditBm::~EditBm()
{
}

void EditBm::OKClicked()
{
    Bookmark * bm = new Bookmark(m_pGDeb, NULL, "", "");
	bm->clearBookmarkSeq();
	for( int i = 0; i < mainLV->rowCount(); ++i )
	{
        bm->addBookmark(mainLV->item(i, 0)->text(), mainLV->item(i, 1)->text(), mainLV->item(i, 2)->text());
	}
	bm->save();
}
void EditBm::exitClicked()
{
	close();
}


short EditBm::fillLV()
{
    Bookmark * bm = new Bookmark(m_pGDeb, NULL, "", "");
	QTableWidgetItem * pItem;
	mainLV->setRowCount(bm->count());
    mainLV->setColumnCount(3);

	mainLV->setHorizontalHeaderItem(0, new QTableWidgetItem("Name"));
    mainLV->setHorizontalHeaderItem(1, new QTableWidgetItem("Table"));
    mainLV->setHorizontalHeaderItem(2, new QTableWidgetItem("SQL-Statement"));
	for( int i = 0; i < bm->count(); ++i )
	{
		pItem = new QTableWidgetItem((char*) bm->getBookmarkName(i+1));
		mainLV->setItem(i, 0, pItem);
        pItem = new QTableWidgetItem((char*) bm->getBookmarkTable(i+1));
        mainLV->setItem(i, 1, pItem);
        pItem = new QTableWidgetItem((char*) bm->getBookmarkFlatSQL(i+1));
        mainLV->setItem(i, 2, pItem);
	}
	for(int i = 0; i < mainLV->columnCount(); ++i)
	{
		mainLV->resizeColumnToContents ( i );
	}

    Helper::setVHeader(mainLV);
	return 0;
}

void EditBm::delClicked()
{
	int pos = mainLV->currentRow();
	if( pos < 0 ) return;

	if( QMessageBox::question(this, "PMF", "Delete bokkmark '"+mainLV->item(pos, 0)->text()+"' ?", "Yes", "No", 0, 1) != 0 ) return;
	mainLV->removeRow(mainLV->currentRow());
	OKClicked();
}

void EditBm::sortClicked(int)
{
    Helper::setVHeader(mainLV);
}
