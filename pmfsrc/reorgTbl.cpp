//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include <reorgTbl.h>
#include <dsqlapi.hpp>

#include <qlayout.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3GridLayout>

reorgTbl::reorgTbl(QWidget *parent, const char* name, GSeq <GString> * tableNameSeq, const char* currentSchema )
  :QDialog(parent, name, TRUE) //, f("Charter", 48, QFont::Bold)
{

  Q3BoxLayout *topLayout = new Q3VBoxLayout(this, 9);
  Q3GridLayout * grid = new Q3GridLayout( 2, 3 );
//        grid->setColStretch( 1, 1 );
  topLayout->addLayout(grid, 2);

  this->resize(260, 560);
  this->setCaption("Reorg Tables");
 
  ok = new QPushButton(this, "Go!");
  ok->setDefault(TRUE);
  cancel = new QPushButton(this, "CANC");
  ok ->setText("Reorg Table");
  cancel->setText("Cancel");
  ok->setGeometry(20, 520, 80, 30);
  cancel->setGeometry(120, 520, 50, 30);
  connect(ok, SIGNAL(clicked()), SLOT(OKClicked()));
  connect(cancel, SIGNAL(clicked()), SLOT(CancelClicked()));
  grid->addWidget(ok, 1, 0);
  grid->addWidget(cancel, 1, 2);

  tableLB = new Q3ListBox(this);
  tableLB->setGeometry( 20, 20, 220, 480);
  tableLB->setMultiSelection(TRUE);
  grid->addMultiCellWidget(tableLB, 0, 0, 0, 2);
  this->currentSchema = currentSchema;
  fillLB(tableNameSeq);
}
reorgTbl::~reorgTbl()
{
  delete ok;
  delete cancel;
  delete tableLB;
}
void reorgTbl::OKClicked()
{
  short i;
  dsqlapi aAPI;
  for( i=0; i<tableLB->count(); ++i)
  {
     if( tableLB->isSelected(i) ) 
         aAPI.reorgTable(currentSchema+"."+GString(tableLB->text(i)));
  }
}

void reorgTbl::CancelClicked()
{
  close();  
}

short reorgTbl::fillLB(GSeq <GString> * tableNameSeq)
{
  for( long i=1; i<=tableNameSeq->numberOfElements(); ++i)
  {
    tableLB->insertItem(tableNameSeq->elementAtPosition(i));
  }
  return 0;
}


