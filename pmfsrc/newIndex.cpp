//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include "newIndex.h"
#include "helper.h"
#include "pmfTable.h"

#include <qlayout.h>
#include <qfont.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QLabel>
#include <gstuff.hpp>
#include <dsqlplugin.hpp>
#include <QCheckBox>
#include <QGroupBox>





NewIndex::NewIndex(DSQLPlugin* pDSQL, QWidget *parent)
    :QDialog(parent) //, f("Charter", 48, QFont::Bold)
{
    this->resize(550, 550);
    m_pDSQL = pDSQL;

    QGridLayout * grid = new QGridLayout( this );

    info = new QLabel(this);

    if( m_pDSQL->getDBTypeName() == _DB2 || m_pDSQL->getDBTypeName() == _DB2ODBC )
    {
        //grid->addWidget(new QLabel("If you do not provide a Schema, "), 0, 0, 1,4);
    }
    info->setText("Index name: ");
    grid->addWidget(info, 1, 0);

    indNameLE = new QLineEdit(this);
    indNameLE->setFixedHeight( indNameLE->sizeHint().height() );

    grid->addWidget(indNameLE, 1, 1, 1, 4);
    //RadioButtons:
    uniRB = new QRadioButton(this);
    dupRB = new QRadioButton(this);
    priRB = new QRadioButton(this);

    uniRB->setText("Unique Index, requires NOT NULL column(s)");
    dupRB->setText("Duplicates Allowed");
    priRB->setText("Primary Key (only one allowed per table)");
    dupRB->setChecked(true);
    grid->addWidget(uniRB, 2, 0, 1, 4);
    grid->addWidget(dupRB, 3, 0, 1, 4);
    grid->addWidget(priRB, 4, 0, 1, 4);
    QLabel *orderTxt = new QLabel("Double-click items in the right box to change ORDER");
    orderTxt->setFixedHeight(20);
    grid->addWidget(orderTxt, 5, 0, 1, 4);


    //ListBoxes and Buttons
    allLB = new QListWidget(this);
    selLB = new QListWidget(this);
    addB = new QPushButton(this);
    addB->setText("->");
    //addB->setGeometry(20, 360, 80, 30);
    connect(addB, SIGNAL(clicked()), SLOT(addClicked()));
    remB = new QPushButton(this);
    remB->setText("<-");
    //remB->setGeometry(20, 360, 80, 30);
    connect(remB, SIGNAL(clicked()), SLOT(remClicked()));


    //OK and New Buttons
    newInd = new QPushButton(this);
    newInd->setDefault(true);
    newInd ->setText("Create");
    //newInd->setGeometry(20, 360, 180, 30);
    connect(newInd, SIGNAL(clicked()), SLOT(newIndClicked()));


    ok = new QPushButton(this);
    ok ->setText("Exit");
    connect(ok, SIGNAL(clicked()), SLOT(OKClicked()));
    //Pity. C++ 11 and above.
    //connect(ok, &QPushButton::clicked, [this]() { this->close(); });


    grid->addWidget(allLB, 6, 0, 5, 2);
    grid->addWidget(addB,  7, 2);
    grid->addWidget(remB,  8, 2);
    grid->addWidget(selLB,  6, 3, 5, 2);


    grid->addWidget(ok, 12, 1);
    grid->addWidget(newInd, 12, 0);

    aThread = NULL;
    tb = NULL;
    timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), this, SLOT(timerEvent()) );
    QObject::connect(selLB, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(selDoubleClicked(QListWidgetItem*)));
}

void NewIndex::OKClicked()
{
    close();
}


NewIndex::~NewIndex()
{
    delete timer;
}

void NewIndex::CreateIndexThread::run()
{
    myNewIndex->createIndex();
}

void NewIndex::createIndex()
{
    GString cols, table, cmd;
    if( m_pDSQL->getDBType() == MARIADB )table = iTabSchema+"."+iTabName;
    else table = GStuff::setDbQuotes(iTabSchema)+"."+GStuff::setDbQuotes(iTabName);

    cols = createIndexCols();
    if( !cols.length() ) return;
    if( priRB->isChecked() ) cmd = "alter table "+table+" add primary key"+cols;
    if( dupRB->isChecked() ) cmd = "create index "+GString(indNameLE->text())+" on "+table+cols;
    if( uniRB->isChecked() ) cmd = "create unique index "+GString(indNameLE->text())+" on "+table+cols;

    errText = m_pDSQL->initAll(cmd);
}

GString NewIndex::createIndexCols()
{
    GString cols, tmp, includeStmt;
    cols = "(";
    for( short i = 0; i < selLB->count(); ++ i )
    {
        tmp = GString(selLB->item(i)->text());
        if( tmp.occurrencesOf("[INC]") )
        {
            includeStmt += tmp.subString(1, tmp.indexOf("[INC]")-1)+",";
        }
        else
        {
            if( priRB->isChecked() )tmp = removeOrder(tmp);
            cols += tmp+",";
        }
    }
    includeStmt.stripTrailing(",");
    if( priRB->isChecked() && includeStmt.length() )
    {
        Helper::msgBox(this, "pmf", "INCLUDE is not allowed in a Primary Key.");
        return "";
    }
    if( includeStmt.length() ) includeStmt = " INCLUDE ("+includeStmt.stripTrailing(",")+")";
    cols.stripTrailing(',');
    cols += ")";
    return cols+includeStmt;
}

void NewIndex::newIndClicked()
{
    if( GString(indNameLE->text() ) == iTabSchema && !priRB->isChecked() )
    {
        if( QMessageBox::warning(this, "PMF", "Really use this index name?", "Yes", "No", 0, 1) ) return;
    }

    if( !GString(indNameLE->text()).length() && !priRB->isChecked() )
    {
        tm("Index needs a name.");
        return;
    }
    GString cols = createIndexCols();
    if( !cols.length() ) return;

    if( QMessageBox::information(this, "PMF", "Creating index "+GString(indNameLE->text())+"\n"+GStuff::format(cols, 80, '+')+"\non table "+iTabSchema+"."+iTabName+"\nContinue?", "Yes", "No", 0, 0, 1) ) return;

    timer->start( 200 );
    aThread = new CreateIndexThread;
    tb = new ThreadBox( this, "Please be patient", "Creating index on "+iTabSchema+"."+iTabName, m_pDSQL->getDBTypeName() );
    aThread->setOwner( this );
    aThread->setBox(tb);
    aThread->start();
    GStuff::dormez(300);
    tb->exec();

    if( errText.length() ) tm(errText);
    else tm("Index created.");

}

GString NewIndex::removeOrder(GString in)
{
    return in.stripTrailing(" ASC").stripTrailing(" DESC").stripTrailing(" RANDOM").stripTrailing(" [INC]");
}

void NewIndex::selDoubleClicked(QListWidgetItem *pItem)
{
    GString txt = removeOrder(pItem->text());
    idxColType *ict = new idxColType(this);
    ict->exec();
    if( ict->selectedSort() != "ESC")
    {
        pItem->setText(txt + " " + GString(ict->selectedSort()));
    }
    delete ict;
}

void NewIndex::timerEvent()
{
    if( !aThread ) return;
    if( !aThread->isAlive() )
    {
        if( tb ) tb->close();
        timer->stop();
    }
} 

void NewIndex::setTableName(GString aTabSchema, GString aTabName)
{
    iTabSchema = aTabSchema;
    iTabName   = aTabName;
    if( m_pDSQL->getDBTypeName() == _DB2 || m_pDSQL->getDBTypeName() == _DB2ODBC )
    {
        indNameLE->setPlaceholderText("SCHEMA.NAME or NAME (SCHEMA will be username)");
    }
    else indNameLE->setPlaceholderText("Set IndexName here");

    fillLV();
}

short NewIndex::fillLV()
{
    GString tabName = GStuff::setDbQuotes(iTabSchema)+"."+GStuff::setDbQuotes(iTabName);
    if( m_pDSQL->getDBType() == MARIADB ) tabName = iTabSchema+"."+iTabName;

    PmfTable pmfTable(m_pDSQL, tabName);

    for(int i = 1; i <= (int)pmfTable.columnCount(); ++i )
    {
        if( m_pDSQL->getDBType() == MARIADB ) allLB->addItem(GString(pmfTable.column(i)->colName()));
        else allLB->addItem(GStuff::setDbQuotes(GString(pmfTable.column(i)->colName())));
    }
    allLB->sortItems();
    return 0;
}

void NewIndex::remClicked()
{
    for(int i = 0; i < selLB->count(); ++i )
    {
        if( selLB->isItemSelected(selLB->item(i)) )
        {
            allLB->addItem( removeOrder(selLB->item(i)->text()) );
            selLB->takeItem(i);
        }
    }
    allLB->sortItems();
}

void NewIndex::addClicked()
{

    for(int i = 0; i < allLB->count(); ++i )
    {
        if( allLB->isItemSelected(allLB->item(i)) )
        {
            idxColType *ict = new idxColType(this);
            //ict.show();
            ict->exec();
            if( ict->selectedSort() != "ESC")
            {
                selLB->addItem( allLB->item(i)->text() + " "+ ict->selectedSort());
                allLB->takeItem(i);
            }
            delete ict;
        }
    }
}

idxColType::idxColType(QWidget *parent) : QDialog(parent)
{

    QGridLayout * grid = new QGridLayout(this);
    QLabel *info = new QLabel("Select sort order.");
    QLabel *info2 = new QLabel("For UNIQUE indexes, you may add columns to be INCLUDED");
    //this->resize(260, 200);
    QPushButton *okButton = new QPushButton("OK", this);
    connect(okButton, SIGNAL(clicked()), SLOT(ok()));
    int row = 0;

    radio0 = new QRadioButton("NONE", this);
    radio1 = new QRadioButton("ASC", this);
    radio2 = new QRadioButton("DESC", this);
    radio3 = new QRadioButton("RANDOM (DB2 only)", this);
    radio4 = new QRadioButton("Add to INCLUDE (UNIQUE indexes only)", this);

    radio0->setChecked(true);
    grid->addWidget(info, row++, 0, 1, 5);
    grid->addWidget(info2, row++, 0, 1, 5);
    row++;
    grid->addWidget(radio0, row, 0);
    grid->addWidget(radio1, row, 1);
    grid->addWidget(radio2, row, 2);
    grid->addWidget(radio3, row, 3);
    row++;
    grid->addWidget(radio4, row++, 0, 1, 4);

    grid->addWidget(okButton, row++, 0);

    //grid->addWidget(createRadioButtons(), row++, 0, 1, 4);

    grid->addWidget(okButton, row++, 0);
    m_Selected = "ESC";
}

QString idxColType::selectedSort()
{
    return m_Selected;
}

void idxColType::ok()
{
    if( radio0->isChecked()) m_Selected = "";
    if( radio1->isChecked()) m_Selected = "ASC";
    if( radio2->isChecked()) m_Selected = "DESC";
    if( radio3->isChecked()) m_Selected = "RANDOM";
    if( radio4->isChecked()) m_Selected = "[INC]";
    this->close();
}

//QGroupBox * idxColType::createRadioButtons()
//{
//    QGroupBox *groupBox = new QGroupBox();

//    QRadioButton *radio0 = new QRadioButton("NONE", this);
//    QRadioButton *radio1 = new QRadioButton("ASC", this);
//    QRadioButton *radio2 = new QRadioButton("DESC", this);
//    QRadioButton *radio3 = new QRadioButton("RANDOM", this);
//    QRadioButton *radio4 = new QRadioButton("add to INCLUDE", this);

//    QHBoxLayout *vbox = new QHBoxLayout;
//    vbox->addWidget(radio0);
//    vbox->addWidget(radio1);
//    vbox->addWidget(radio2);
//    vbox->addWidget(radio3);
//    vbox->addWidget(radio4);
//    vbox->addStretch(0);
//    groupBox->setLayout(vbox);
//    radio1->setChecked(true);
//    return groupBox;
//}
