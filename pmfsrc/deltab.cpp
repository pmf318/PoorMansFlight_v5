//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include "deltab.h"

#include <qlayout.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QGridLayout>
#include <QMessageBox>



Deltab::Deltab(DSQLPlugin* pDSQL, QWidget *parent, GString currentSchema, int hideSysTabs )
  :QDialog(parent) //, f("Charter", 48, QFont::Bold)
{
	this->resize(290, 500);
	this->setWindowTitle("Drop Table");
    m_pDSQL = pDSQL;

	QBoxLayout *topLayout = new QVBoxLayout(this);
	QGridLayout * grid = new QGridLayout();
    topLayout->addLayout(grid, 9);

	ok = new QPushButton(this);

    ok->setDefault(true);

	cancel = new QPushButton(this);
	ok ->setText("Drop");
	ok->setFixedHeight( ok->sizeHint().height());
    cancel->setText("Close");
	cancel->setFixedHeight( cancel->sizeHint().height());
	connect(ok, SIGNAL(clicked()), SLOT(OKClicked()));
	connect(cancel, SIGNAL(clicked()), SLOT(CancelClicked()));

    tbSel = new TableSelector(pDSQL, parent, currentSchema, hideSysTabs);

    grid->addWidget(tbSel, 1, 0, 1, 4);
	grid->addWidget(ok, 2, 0);
	grid->addWidget(cancel, 2, 1);
	
}
Deltab::~Deltab()
{
	delete ok;
	delete cancel;
}
void Deltab::OKClicked()
{
    QListWidget *pLB = tbSel->getTableHandle();

	long i, erc, count = 0;	
    for( i=0; i < pLB->count(); ++i)
	{
        if( pLB->isItemSelected(pLB->item(i)) ) count++;
	}
	if( !count )
	{
		QMessageBox::information(this, "PMF", "Select tables(s) to drop");
		return;
	}
	if( QMessageBox::information(this, "PMF", "Dropping "+GString(count)+" table(s), continue?", "Yes", "No", 0, 0, 1) ) return;
	
    GString tabName;
    for( i=0; i < pLB->count(); ++i)
	{
        if( pLB->isItemSelected(pLB->item(i)) )
		{
            if( m_pDSQL->getDBTypeName() == _MARIADB ) tabName = tbSel->tablePrefix()+GString(pLB->item(i)->text());
            else tabName = tbSel->tablePrefix()+ "\""+GString(pLB->item(i)->text())+"\"";
            erc = m_pDSQL->deleteTable(tabName);
			if( !erc )
			{
                pLB->takeItem(i);
				i--; 
			}
            else
            {
                msg("Could not drop table "+tabName+"\nSQLCode: "+GString(erc)+", SQLError: "+m_pDSQL->sqlError());
            }
            m_pDSQL->commit();
		}
    }
}
void Deltab::msg(GString message)
{
    QMessageBox::information(this, "PMF", message);
}

void Deltab::CancelClicked()
{
    close();
}
void Deltab::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_Delete:
            OKClicked();
            break;
        case Qt::Key_Escape:
            CancelClicked();
            break;
        default:
            QWidget::keyPressEvent(event);
    }
}

