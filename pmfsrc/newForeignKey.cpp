//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include "newForeignKey.h"
#include "helper.h"
#include <qlayout.h>
#include <qfont.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QLabel>
#include <gstuff.hpp>
#include <dsqlplugin.hpp>
#include <QCheckBox>
#include <QGroupBox>





NewForeignKey::NewForeignKey(DSQLPlugin* pDSQL, QWidget *parent, GString fullTableName, int hideSystemTables)
    :QDialog(parent) //, f("Charter", 48, QFont::Bold)
{
    m_iHideSystemTables = hideSystemTables;
    m_strFullTableName = fullTableName;
    this->resize(550, 550);
    m_pDSQL = pDSQL;

    QGridLayout * grid = new QGridLayout( this );

    info = new QLabel(this);

    if( m_pDSQL->getDBTypeName() == _DB2 || m_pDSQL->getDBTypeName() == _DB2ODBC )
    {
        //grid->addWidget(new QLabel("If you do not provide a Schema, "), 0, 0, 1,4);
    }
    info->setText("Key name: ");
    grid->addWidget(info, 1, 0);

    indNameLE = new QLineEdit(this);
    indNameLE->setFixedHeight( indNameLE->sizeHint().height() );

    grid->addWidget(indNameLE, 1, 1, 1, 4);


//    noneRB = new QRadioButton("<None>", this);
//    cascadeRB = new QRadioButton("CASCADE", this);
//    noActionRB = new QRadioButton("NO ACTION", this);
//    if( pDSQL->getDBType() == DB2 || pDSQL->getDBType() == DB2ODBC )
//    {
//        restrictRB = new QRadioButton("RESTRICT", this);
//        setNullRB = new QRadioButton("SET DEFAULT", this);
//    }
//    else if( pDSQL->getDBType() == SQLSERVER  )
//    {
//       restrictRB = new QRadioButton("SET DEFAULT", this);
//       setNullRB  = new QRadioButton("SET NULL", this);;
//    }
//    noneRB->setChecked(true);

//    grid->addWidget(new QLabel("On Delete:"), 2, 0);
//    grid->addWidget(noneRB, 2, 1);
//    grid->addWidget(cascadeRB, 2, 2);
//    grid->addWidget(noActionRB, 2, 3);
//    grid->addWidget(restrictRB, 2, 4);
//    grid->addWidget(setNullRB, 2, 5);
    grid->addWidget(createRadioButtons() , 2, 0, 1,5);
    grid->addWidget(new QLabel("Select columns for Foreign Key"), 3, 0, 1,4);



    //ListBoxes and Buttons
    allLB = new QListWidget(this);
    selLB = new QListWidget(this);
    addB = new QPushButton(this);
    addB->setText("->");

    allFkLB = new QListWidget(this);
    selFkLB = new QListWidget(this);
    addFkB = new QPushButton(this);
    addFkB->setText("->");


    connect(addB, SIGNAL(clicked()), SLOT(addClicked()));
    remB = new QPushButton(this);
    remB->setText("<-");
    connect(remB, SIGNAL(clicked()), SLOT(remClicked()));

    connect(addFkB, SIGNAL(clicked()), SLOT(addFkClicked()));
    remFkB = new QPushButton(this);
    remFkB->setText("<-");
    connect(remFkB, SIGNAL(clicked()), SLOT(remFkClicked()));

    //OK and New Buttons
    newInd = new QPushButton(this);
    newInd->setDefault(true);
    newInd ->setText("Create");
    //newInd->setGeometry(20, 360, 180, 30);
    connect(newInd, SIGNAL(clicked()), SLOT(newForeignKeyClicked()));


    ok = new QPushButton(this);
    ok ->setText("Exit");
    connect(ok, SIGNAL(clicked()), SLOT(OKClicked()));
    //Pity. C++ 11 and above.
    //connect(ok, &QPushButton::clicked, [this]() { this->close(); });


    grid->addWidget(allLB, 6, 0, 4, 2);
    grid->addWidget(addB,  7, 2);
    grid->addWidget(remB,  8, 2);
    grid->addWidget(selLB,  6, 3, 4, 2);

    QFrame* pFrame = new QFrame();
    pFrame->setFrameShape(QFrame::HLine);
    grid->addWidget(pFrame, 11, 0, 1,5);

    grid->addWidget(new QLabel("Select reference table and columns"), 12, 0, 1,5);
    m_pTabSel = new TableSelector(pDSQL, this, "", hideSystemTables, 1 );
    grid->addWidget(m_pTabSel, 13, 0, 1, 5);
    grid->addWidget(allFkLB, 14, 0, 4, 2);
    grid->addWidget(addFkB,  15, 2);
    grid->addWidget(remFkB,  16, 2);
    grid->addWidget(selFkLB,  14, 3, 4, 2);

    grid->addWidget(ok, 18, 1);
    grid->addWidget(newInd, 18, 0);

    aThread = NULL;
    tb = NULL;
    timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), this, SLOT(timerEvent()) );
    connect( m_pTabSel, SIGNAL(tableSelection(QString )), this, SLOT(tableSelected(QString )));
    connect( m_pTabSel, SIGNAL(schemaSelection(QString )), this, SLOT(tableSelected(QString )));
}

void NewForeignKey::tableSelected(QString table)
{
    if( m_pDSQL->getDBTypeName() == _MARIADB ) m_strRefTableName = m_pTabSel->tablePrefix()+GString(table);
    else m_strRefTableName = m_pTabSel->tablePrefix() + GStuff::wrap(table);
    m_pDSQL->initAll("Select * from "+m_strRefTableName, 1);

    allFkLB->clear();
    selFkLB->clear();
    for( unsigned int i = 1; i <= m_pDSQL->numberOfColumns(); ++ i )
    {
        allFkLB->addItem("\""+m_pDSQL->hostVariable(i)+"\"");
    }

}

void NewForeignKey::OKClicked()
{
    close();
}


NewForeignKey::~NewForeignKey()
{
    delete timer;
}

void NewForeignKey::CreateForeignKeyThread::run()
{
    myNewForeignKey->createForeignKey();
}

void NewForeignKey::createForeignKey()
{
    GString fkCols, refCols;
    for( int i = 0; i < selLB->count() ; ++i )
    {
        fkCols += GString(selLB->item(i)->text())+",";
    }
    fkCols.stripTrailing(",");
    for( int i = 0; i < selFkLB->count() ; ++i )
    {
        refCols += GString(selFkLB->item(i)->text())+",";
    }
    refCols.stripTrailing(",");


    GString cmd = "ALTER TABLE "+m_strFullTableName+" ADD CONSTRAINT "+GString(indNameLE->text())+" FOREIGN KEY ("+fkCols+") REFERENCES \n";
    cmd += m_strRefTableName+"("+refCols+")";

    if( cascadeRB->isChecked() )cmd += " ON DELETE "+cascadeRB->text();
    if( noActionRB->isChecked() )cmd += " ON DELETE "+noActionRB->text();
    if( restrictRB->isChecked() )cmd += " ON DELETE "+restrictRB->text();
    if( setNullRB != NULL && setNullRB->isChecked() )cmd += " ON DELETE "+setNullRB->text();
    errText = m_pDSQL->initAll(cmd);
    if( errText.length() ) errText += "\n\nCommand was:\n"+cmd;
}

void NewForeignKey::newForeignKeyClicked()
{

    if( GString(indNameLE->text()).strip().length() == 0 )
    {
        msg("Enter a name for the Foreign Key.");
        return;
    }
    if( selLB->count() == 0 )
    {
        msg("No columns selected");
        return;
    }
    if( selFkLB->count() == 0 )
    {
        msg("No reference columns selected");
        return;
    }

    if( QMessageBox::information(this, "PMF", "Creating Foreign Key "+GString(indNameLE->text())+"\non table "+m_strFullTableName+"\nContinue?", "Yes", "No", 0, 0, 1) ) return;

    timer->start( 200 );
    aThread = new CreateForeignKeyThread;
    tb = new ThreadBox( this, "Please be patient", "Creating Foreign Key on "+m_strFullTableName, m_pDSQL->getDBTypeName() );
    aThread->setOwner( this );
    aThread->setBox(tb);
    aThread->start();
    GStuff::dormez(300);
    tb->exec();

    if( errText.length() ) tm(errText);
    else tm("Foreign Key created.");

}

void NewForeignKey::timerEvent()
{
    if( !aThread ) return;
    if( !aThread->isAlive() )
    {
        if( tb ) tb->close();
        timer->stop();
    }
} 

void NewForeignKey::setTableName(GString aTabSchema, GString aTabName)
{
    iTabSchema = aTabSchema;
    iTabName   = aTabName;
    if( m_pDSQL->getDBTypeName() == _DB2 || m_pDSQL->getDBTypeName() == _DB2ODBC )
    {
        indNameLE->setPlaceholderText("SCHEMA.NAME or NAME (SCHEMA will be username)");
    }
    else indNameLE->setPlaceholderText("Set name here");

    fillLV();
}

void NewForeignKey::fillLV()
{
    m_pDSQL->initAll("Select * from "+GStuff::setDbQuotes(iTabSchema)+"."+GStuff::setDbQuotes(iTabName), 1);

    for( unsigned int i = 1; i <= m_pDSQL->numberOfColumns(); ++ i )
    {
        allLB->addItem("\""+m_pDSQL->hostVariable(i)+"\"");
    }
}

void NewForeignKey::remClicked()
{
    for(int i = 0; i < selLB->count(); ++i )
    {
        if( selLB->isItemSelected(selLB->item(i)) )
        {
            allLB->addItem( selLB->item(i)->text() );
            selLB->takeItem(i);
        }
    }
}

void NewForeignKey::addClicked()
{

    for(int i = 0; i < allLB->count(); ++i )
    {
        if( allLB->isItemSelected(allLB->item(i)) )
        {
            selLB->addItem( allLB->item(i)->text());
            allLB->takeItem(i);
        }
    }
}

void NewForeignKey::remFkClicked()
{
    for(int i = 0; i < selFkLB->count(); ++i )
    {
        if( selFkLB->isItemSelected(selFkLB->item(i)) )
        {
            allFkLB->addItem( selFkLB->item(i)->text() );
            selFkLB->takeItem(i);
        }
    }
}

void NewForeignKey::addFkClicked()
{

    for(int i = 0; i < allFkLB->count(); ++i )
    {
        if( allFkLB->isItemSelected(allFkLB->item(i)) )
        {
            selFkLB->addItem( allFkLB->item(i)->text());
            allFkLB->takeItem(i);
        }
    }
}


QGroupBox * NewForeignKey::createRadioButtons()
{
    QGroupBox *groupBox = new QGroupBox();

    noneRB = new QRadioButton("[Nothing]", this);
    cascadeRB = new QRadioButton("CASCADE", this);
    noActionRB = new QRadioButton("NO ACTION", this);
    setNullRB  = new QRadioButton("SET NULL", this);
    if( m_pDSQL->getDBType() == SQLSERVER  )
    {
       restrictRB = new QRadioButton("SET DEFAULT", this);       
    }
    else //( m_pDSQL->getDBType() == DB2 || m_pDSQL->getDBType() == DB2ODBC )
    {
        restrictRB = new QRadioButton("RESTRICT", this);
    }

    noneRB->setChecked(true);
    QHBoxLayout *vbox = new QHBoxLayout;
    vbox->addWidget(new QLabel("On Delete:"));
    vbox->addWidget(noneRB);
    vbox->addWidget(cascadeRB);
    vbox->addWidget(noActionRB);
    vbox->addWidget(restrictRB);
    vbox->addWidget(setNullRB);
    vbox->addStretch(0);
    groupBox->setLayout(vbox);
    return groupBox;
}
