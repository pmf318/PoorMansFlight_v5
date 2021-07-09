//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include "querydb.h"
#include "helper.h"

#include <qframe.h>
#include <qlabel.h>
#include <QGridLayout>
#include <QVBoxLayout>
#include <gstring.hpp>
#include <qlayout.h>
#include "pmfSchemaCB.h"
#include "pmf.h"

Querydb::Querydb( DSQLPlugin* pDSQL, Pmf* parent, GString currentSchema, int hideSysTabs )
 :QDialog(parent)
{
    //QBoxLayout *topLayout = new QVBoxLayout(this);
    QGridLayout * grid = new QGridLayout( this );
    m_pPmf = parent;
    m_pDSQL = pDSQL;
	QLabel* tmpQLabel;
	tmpQLabel = new QLabel( this );
    tmpQLabel->setText( "Select table(s) to query" );

//Create a ButtonGroup
    bGroup = new QButtonGroup( this);

	hostRB = new QRadioButton( this );
	hostRB->setText( "HostVariable" );
    hostRB->setAutoRepeat( false );


    exactMatchCB = new QCheckBox( this );
    exactMatchCB->setText( "Exact match" );


	valueRB = new QRadioButton( this );
    valueRB->setText( "Value (numerical or alpha-num.)" );
    valueRB->setAutoRepeat( false );
    valueRB->setChecked( true );
    //valueRB->setAutoResize( false );
//	connect( valueRB, SIGNAL(clicked()), SLOT(valClicked()) );


	QWidget * pane = new QWidget(this);
	QGridLayout *btGrid = new QGridLayout(pane);	
	okB = new QPushButton(pane);
	connect( okB, SIGNAL(clicked()), SLOT(okClicked()) );
	okB->setText( "Go!" );
    okB->setAutoRepeat( false );
    //okB->setAutoResize( false );

	cancelB = new QPushButton(pane);
	connect( cancelB, SIGNAL(clicked()), SLOT(cancelClicked()) );
	cancelB->setText( "Cancel" );
    cancelB->setAutoRepeat( false );
    //cancelB->setAutoResize( false );
	btGrid->addWidget(okB, 0, 0);
	btGrid->addWidget(cancelB, 0, 1);

    tbSel = new TableSelector(pDSQL, parent, currentSchema, hideSysTabs);


	findLE = new QLineEdit( this );
	findLE->setText( "" );
	findLE->setEchoMode( QLineEdit::Normal );
    findLE->setFrame( true );
    findLE->setPlaceholderText("Do NOT put quotes around strings");

        //grid->addWidget(bGroup, 1, 1, 1, 1);

	resultLB = new QListWidget( this);
	resultLB->setFrameStyle( 51 );
	resultLB->setLineWidth( 2 );
    connect(resultLB, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(listDoubleClicked(QListWidgetItem*)));
	QLabel* tmpQLabel2;
	tmpQLabel2 = new QLabel( this);
    tmpQLabel2->setText( "Results (double-click to view result)" );

    grid->addWidget(tbSel, 1, 0, 8, 1);
	grid->addWidget(tmpQLabel, 0, 0);
    grid->addWidget(valueRB, 1, 1);
    grid->addWidget(hostRB, 2, 1);
    grid->addWidget(exactMatchCB, 3, 1);
    grid->addWidget(findLE, 4, 1);
    grid->addWidget(tmpQLabel2, 5, 1);
    grid->addWidget(resultLB, 6, 1, 2, 1);
    grid->addWidget(pane, 8, 1);
    this->resize( 660, 430 );
}


Querydb::~Querydb()
{
}

void Querydb::listDoubleClicked(QListWidgetItem * pItem)
{
    for(int i = 0; i < resultLB->count(); ++i)
    {
        if( resultLB->item(i) == pItem )
        {
            m_pPmf->createNewTab(m_cmdSeq.elementAtPosition(i+1), 0);
        }
    }
}

void Querydb::okClicked()
{
    m_cmdSeq.removeAll();

	if( GString(findLE->text()).length() == 0 )
	{
		tm("Enter a string to search for.");
		return;
    }
    QListWidget *pLB = tbSel->getTableHandle();

	resultLB->clear();
	short i, cnt, numTables = 0;
	cnt = 0;

    for( i=0; i<pLB->count(); ++i )
	{
        if( pLB->isItemSelected(pLB->item(i)) ) numTables++;
	}
	GString title = "Tables querried:";
	
	QProgressDialog apd(title, "Cancel", 0, numTables, this); 
	
	apd.setWindowModality(Qt::WindowModal);
	apd.setValue(1);

	cnt = 0;
    GString tabName;
    for( i=0; i<pLB->count(); ++i )
	{
        if( pLB->isItemSelected(pLB->item(i)) )
		{
				apd.setValue(cnt);
				cnt++;
                if( m_pDSQL->getDBTypeName() == _MARIADB ) tabName = tbSel->tablePrefix()+GString(pLB->item(i)->text());
                else tabName = tbSel->tablePrefix()+ "\""+GString(pLB->item(i)->text())+"\"";

				apd.setWindowTitle(tabName);
                searchInTable( tabName, &apd );				
				if( apd.wasCanceled() ) break;
		}
	}
	if( cnt == 0 )
	{
		tm("Select a table.");
		return;
	}
	if( resultLB->count() == 0 ) resultLB->addItem("<No Match Found>");
	apd.setValue(numTables);
}

void Querydb::searchInTable(GString table, QProgressDialog * apd)
{
    GString sel, result, findStr, err;
	unsigned long i;
    int found;

    if( hostRB->isChecked() )
	{
        sel = m_pDSQL->initAll("SELECT colname FROM syscat.columns where tabschema='"+Helper::tableSchema(table, DB2) +
              "' and tabname='"+Helper::tableName(table, DB2)+"'");
        for( i=1; i<=m_pDSQL->numberOfRows(); ++i )
        {
            found = 0;
            if( apd->wasCanceled() ) break;
            findStr = GString(findLE->text());
            if( exactMatchCB->isChecked())
            {
                if( findStr == m_pDSQL->rowElement(i, 1).strip("'") ) found = 1;
            }
            else if( m_pDSQL->rowElement(i, 1).upperCase().occurrencesOf(findStr.upperCase()) > 0 ) found = 1;

            if(found)
            {
                result = "Found Column "+m_pDSQL->rowElement(i, 1).strip()+" In Table "+table;
                resultLB->addItem( result );
                m_cmdSeq.add("SELECT * FROM "+table);
            }
        }

//        sel = m_pDSQL->initAll("SELECT * FROM "+table, 1);

//        for( i=1; i<=m_pDSQL->numberOfColumns(); ++i )
//		{
//            found = 0;
//			if( apd->wasCanceled() ) break;
//            findStr = GString(findLE->text());
//            if( exactMatchCB->isChecked())
//            {
//                if(  findStr == m_pDSQL->hostVariable(i).strip() ) found = 1;
//            }
//            else if( m_pDSQL->hostVariable(i).upperCase().occurrencesOf(findStr.upperCase()) > 0 ) found = 1;

//            if(found)
//            {
//                result = "Found Column "+m_pDSQL->hostVariable(i).strip()+" In Table "+table;
//                resultLB->addItem( result );
//                m_cmdSeq.add("SELECT * FROM "+table);
//            }
//		}
	}
	else
	{
        GString val, constraint;
        sel = m_pDSQL->initAll("SELECT * FROM "+table, 1);

        GSeq<COL_SPEC*> colDescSeq;
        DSQLPlugin * qDSQL = new DSQLPlugin(*m_pDSQL);
        qDSQL->getColSpecs(table, &colDescSeq);
        delete qDSQL;

        val  = findLE->text();
        DSQLPlugin *tmpDSQL = new DSQLPlugin(*m_pDSQL);
        for( i=1; i<=m_pDSQL->numberOfColumns(); ++i )
		{
			if( apd->wasCanceled() ) break;
            constraint = Helper::createSearchConstraint(m_pDSQL, &colDescSeq,  val, i, exactMatchCB->isChecked() );
            if( !constraint.length() ) continue;
            err = tmpDSQL->initAll("SELECT COUNT(*) FROM "+table+" WHERE "+constraint);
            GString cmd = "SELECT COUNT(*) FROM "+table+" WHERE "+constraint;
            printf("CMD: %s\n",(char*)cmd);
            if( tmpDSQL->rowElement(1, 1).strip("'").asInt() > 0 )
            {
                result = table+": Found '"+val+"' "+tmpDSQL->rowElement(1, 1)+" time(s) in column "+m_pDSQL->hostVariable(i);
                resultLB->addItem(result);                
                m_cmdSeq.add("SELECT * FROM "+table+" WHERE "+constraint);
            }

//            host = m_pDSQL->hostVariable(i).strip();
//			val  = findLE->text();
//            if( val.indexOf("'") > 0 )
//            {
//                if( exactMatchCB->isChecked()  ) sel  = "SELECT COUNT(*) FROM "+table+" WHERE "+host+" = "+val;
//                else sel  = "SELECT COUNT(*) FROM "+table+" WHERE UPPER("+host+") LIKE %"+val.upperCase()+"%";
//            }
//			else sel  = "SELECT COUNT(*) FROM "+table+" WHERE "+host+" = "+val;

//            tmpDSQL->initAll(sel);
//            cnt = tmpDSQL->rowElement(1, 1);
//			if( cnt.asInt() > 0 )
//			{
//				result = table+": Found "+val+" "+cnt+" Time(s) In Column "+host;
//				resultLB->addItem(result);
//			}
		} //end for
        delete tmpDSQL;
	} //end if Value or Host
}

void Querydb::cancelClicked()
{
	close();
}


