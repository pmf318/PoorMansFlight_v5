//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include "tableSelector.h"

#include <qlayout.h>

#include <QVBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include "pmfdefines.h"




TableSelector::TableSelector(DSQLPlugin* pDSQL, QWidget *parent, GString currentSchema, int hideSysTabs, int hideListBox )
{
    m_pDSQL = pDSQL;
    iHideSysTabs = hideSysTabs;
    strCurrentSchema = currentSchema;
    int pos = 0;

    QBoxLayout *topLayout = new QVBoxLayout(this);
    QGridLayout * grid = new QGridLayout();
    topLayout->addLayout(grid, 9);

    schemaCB  = new PmfSchemaCB(this);
    tableLB   = new QListWidget(this);    
    tableLB->setSelectionMode(QListWidget::ExtendedSelection);
    tableCB   = new QComboBox(this);

    if( pDSQL->getDBType() == SQLSERVER )
    {
        GString context;
        if( currentSchema.occurrencesOf(".")) context = currentSchema.subString(1, currentSchema.indexOf(".")-1);
        contextCB = new QComboBox(this);
        grid->addWidget(contextCB);
        pDSQL->initAll("SELECT name FROM master.dbo.sysdatabases order by name ");
        contextCB->addItem(_selStringCB); //Add "<Select>" as first entry
        for( int i = 1; i<= (int) pDSQL->numberOfRows(); ++i)
        {
            if( context == pDSQL->rowElement(i,1).strip("'")) pos = i;
            contextCB->addItem(pDSQL->rowElement(i,1).strip("'"));
        }
        connect(contextCB, SIGNAL(activated(int)), SLOT(contextSelected(int)));
        contextCB->setCurrentIndex(pos);
        contextSelected(pos);

    }
    else contextCB = NULL;


    fillSchemaCB(currentSchema);

    grid->addWidget(schemaCB);
    if( hideListBox )
    {
        tableLB->hide();
        grid->addWidget(tableCB);
    }
    else
    {
        tableCB->hide();
        grid->addWidget(tableLB, 2, 0, 1, 4);
    }


    connect(schemaCB, SIGNAL(activated(int)), SLOT(schemaSelected(int)));
    connect(tableCB, SIGNAL(activated(int)), SLOT(tableSelected(int)));

//    if( contextCB) contextSelected(pos);

}
TableSelector::~TableSelector()
{
    delete schemaCB;
    if( contextCB ) delete contextCB;
    delete tableLB;
}
void TableSelector::contextSelected(int index)
{
    PMF_UNUSED(index);
    m_pDSQL->initAll("USE \""+contextCB->currentText()+"\"");
    m_pDSQL->initAll("select db_name()");
    fillSchemaCB(contextCB->currentText());
}
void TableSelector::fillSchemaCB(GString context )
{
    int pos;
    if( context.occurrencesOf(".")) context = context.subString(context.indexOf(".")+1, context.length()).strip();
    pos = schemaCB->fill(m_pDSQL, context, iHideSysTabs);
    schemaSelected(pos);
}

void TableSelector::fillLB(GString schema)
{
    GString name;
    tableLB->clear();
    tableCB->clear();
    if( schema == _selStringCB ) return;
    m_pDSQL->getTables(schema);
    tableCB->addItem(_selStringCB);

    for( unsigned int i=1; i<=m_pDSQL->numberOfRows(); ++i)
    {
        name   = m_pDSQL->rowElement(i,1).strip().strip("'").strip();
        new QListWidgetItem(name, tableLB);
        tableCB->addItem(name);
    }

}

void TableSelector::schemaSelected(int index)
{
    if( index < 0 ) return;
    fillLB(schemaCB->itemText(index));
    emit schemaSelection(schemaCB->itemText(index));
}

void TableSelector::tableSelected(int index)
{
    emit tableSelection(tableCB->itemText(index));
}

QListWidget * TableSelector::getTableHandle()
{
    return tableLB;
}


GString TableSelector::tablePrefix()
{
    GString prefix;
    if( contextCB ) prefix = "\""+contextCB->currentText()+"\".";
    if( m_pDSQL->getDBType() == MARIADB ) return schemaCB->currentText()+".";
    else prefix += "\""+schemaCB->currentText()+"\".";
    return prefix;
}
