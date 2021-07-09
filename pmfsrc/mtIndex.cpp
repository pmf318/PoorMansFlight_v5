//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include "mtIndex.h"
#include <sqlenv.h>
#include <qlayout.h>
#include <qfont.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <dsqlobj.hpp>

#ifndef _newIndex_
#include "newIndex.h"
#endif

mtIndex::mtIndex(QWidget *parent)
  :QDialog(parent) //, f("Charter", 48, QFont::Bold)
{
  this->resize(680, 400);
  //QBoxLayout *topLayout = new QVBoxLayout(this, 9);

  QGridLayout * grid = new QGridLayout();
  //topLayout->addLayout(grid, 2);
  //grid->setColStretch( 0, 0 );

  info = new QLabel(this);

  grid->addWidget(info, 0, 0, 1, 4);


  newInd = new QPushButton(this);
  newInd->setDefault(TRUE);
  newInd ->setText("New Index");
  ///newInd->setGeometry(20, 360, 180, 30);
  connect(newInd, SIGNAL(clicked()), SLOT(newIndClicked()));
  grid->addWidget(newInd, 2, 0);



  del = new QPushButton(this);
  del ->setText("Drop Index");
  ///del->setGeometry(20, 360, 180, 30);
  connect(del, SIGNAL(clicked()), SLOT(delClicked()));
  grid->addWidget(del, 2, 1);

  ok = new QPushButton(this);
  ok ->setText("Exit");
  ok->setGeometry(20, 360, 80, 30);
  connect(ok, SIGNAL(clicked()), SLOT(OKClicked()));
  grid->addWidget(ok, 2, 2);

  mainLV = new QTableWidget(this);
  mainLV->setGeometry( 20, 20, 420, 300);
  mainLV->setSelectionMode(QTableWidget::ExtendedSelection);
  mainLV->setSelectionBehavior(QAbstractItemView::SelectRows);  
  
  grid->addWidget(mainLV, 1, 1, 1, 4);
//  infoLB->setFont(QFont("courier"));

}
mtIndex::~mtIndex()
{
}

void mtIndex::newIndClicked()
{
   newIndex * ni = new newIndex(this);
   ni->setTableName(iTabSchema, iTabName);
   ni->setFont(this->font());
   //!!ni->setFont(font);
   ni->exec();
   fillLV();
}
void mtIndex::OKClicked()
{
  close();
}
void mtIndex::setTableName(GString aTable)
{
   if( !aTable.length() )
   {
      info->setText("Please select a table first.");
      return;
   }
   iTabSchema = aTable.subString(1, aTable.indexOf(".") - 1);
   iTabName   = aTable.subString(aTable.indexOf(".") + 1, aTable.length()).strip();
   info->setText("Indexes for table "+aTable);
   fillLV();
}

short mtIndex::fillLV()
{
   Q3ListViewItem * lvItem;
   dsqlobj aDSQL;
   int i, j;

   mainLV->clear();


   GString err = aDSQL.initAll("SELECT NAME As IndexName, COLNAMES as Columns, UNIQUERULE, CREATE_TIME, Creator, STATS_TIME FROM"
                 " SYSIBM.SYSINDEXES WHERE TBCREATOR='"+iTabSchema+"' AND TBNAME='"+iTabName+"'");
   if( aDSQL.numberOfRows() == 0 )
   {
        aDSQL.initAll("SELECT INAME, ICREATOR, COLNAMES FROM SYSTEM.SYSINDEXES WHERE "
                       "CREATOR='"+iTabSchema+"' AND TNAME='"+iTabName+"'");
   }

   if( !mainLV->columns() )
   {
       for( i = 1; i <= aDSQL.numberOfColumns(); ++i ) mainLV->addColumn(aDSQL.hostVariable(i));
   }

   for( i = 1; i <= aDSQL.numberOfRows(); ++i )
   {
      lvItem = new Q3ListViewItem(mainLV);
      for( j = 1; j <= aDSQL.numberOfColumns(); ++ j )
      {
         lvItem->setText( j-1, aDSQL.rowElement(i, j).strip("'").strip() );
      }
   }
   return 0;
}

void mtIndex::delClicked()
{
   GString table = iTabSchema+"."+iTabName;
   GString cmd;
   Q3ListViewItem *lvItem = mainLV->selectedItem();
   if( lvItem == NULL ) return;
   if( lvItem->text(2) == "'P'" ) cmd = "alter table "+table+" drop primary key"; //Primary Key
   else cmd = "drop index "+GString(lvItem->text(4))+"."+GString(lvItem->text(0)); //"Ordinary" Index
   dsqlobj aSQL;
   GString err = aSQL.initAll(cmd);
   if( err.length() ) tm(err);
   else fillLV();
}
