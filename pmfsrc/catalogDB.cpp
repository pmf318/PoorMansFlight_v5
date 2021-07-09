//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include "catalogDB.h"
#include "helper.h"


#include <qlayout.h>
#include <QGridLayout>
#include <gstuff.hpp>
#include <QCloseEvent>
#include <QHeaderView>
#include <QTime>
#include <QDate>


#define CATALOGDB_GROUPNAME "CreateNewCheck"
#define CATALOGDB_NAME "Name"
#define CATALOGDB_CHECK "Statement"


#define CATDB_DB_AND_NODE_EXIST 0
#define CATDB_DB_EXISTS 1
#define CATDB_DB_NOT_FOUND 2

CatalogDB::CatalogDB(DSQLPlugin* pDSQL, QWidget * parent)  :QDialog(parent)
{
    qApp->installEventFilter(this);
    if( !pDSQL ) _dbTypeName = _DB2;
    else _dbTypeName = pDSQL->getDBTypeName();


    m_iCatalogChanged = 0;

    this->resize(400, 200);

    this->setWindowTitle("pmf");
    okB = new QPushButton(this);
    okB->setDefault(true);
    okB->setText("Save");
    connect(okB, SIGNAL(clicked()), SLOT(OKClicked()));
    okB->setDefault(true);

    closeB = new QPushButton(this);
    closeB->setDefault(true);
    closeB->setText("Exit");
    connect(closeB, SIGNAL(clicked()), SLOT(closeClicked()));

    QGridLayout * mainGrid = new QGridLayout(this);


    nameLE     = new QLineEdit(this);
    hostLE     = new QLineEdit(this);
    commentLE  = new QLineEdit(this);
    portLE     = new QLineEdit(this);
    databaseLE = new QLineEdit(this);
    aliasLE    = new QLineEdit(this);

    useExistingRB = new QRadioButton("Use existing node", this);
    createNewRB   = new QRadioButton("Catalog new node", this);
    nodesCB       = new QComboBox(this);
    //protocolCB    = new QComboBox(this);

    int row = 0;

    row++;
    mainGrid->addWidget( new QLabel("Catalog a new database. You can create a new node"), row, 0, 1, 2);
    row++;
    mainGrid->addWidget( new QLabel("or chose an existing node."), row, 0, 1, 2);
//    row++;
//    mainGrid->addWidget( new QLabel("Manage entries in 'Menu->Catalog DBs and Nodes'"), row, 0, 1, 2);

    row++;
    mainGrid->addWidget( new QLabel(""), row, 0, 1, 2);


    row++;
    mainGrid->addWidget(new QLabel("Database:", this), row, 0);
    mainGrid->addWidget(databaseLE, row, 1, 1, 2);

    row++;
    mainGrid->addWidget(new QLabel("Alias:", this), row, 0);
    mainGrid->addWidget(aliasLE, row, 1, 1, 2);

    row++;
    mainGrid->addWidget(new QLabel(""), row, 0, 1, 2);

//    row++;
//    QLabel *info1 = new QLabel("Chose existing node or create a new node");
//    mainGrid->addWidget(info1, row, 0, 1, 2);

    row++;
    mainGrid->addWidget(useExistingRB, row, 0);
    row++;
    mainGrid->addWidget(nodesCB, row, 0, 1, 3);

    row++;
    mainGrid->addWidget(new QLabel(""), row, 0, 1, 2);

    row++;
    mainGrid->addWidget(createNewRB, row, 0);

    row++;
    mainGrid->addWidget(new QLabel("Nodename:", this), row, 0);
    mainGrid->addWidget(nameLE, row, 1, 1, 2);

    row++;
    mainGrid->addWidget(new QLabel("Host/Server:", this), row, 0);
    mainGrid->addWidget(hostLE, row, 1, 1, 2);

    row++;
    mainGrid->addWidget(new QLabel("Comment:", this), row, 0);
    mainGrid->addWidget(commentLE, row, 1, 1, 2);

    row++;
    mainGrid->addWidget(new QLabel("Port or Service:", this), row, 0);
    mainGrid->addWidget(portLE, row, 1, 1, 2);

//    mainGrid->addWidget(new QLabel("Protocol:", this), 8, 0);
//    mainGrid->addWidget(protocolCB, 8, 1, 1, 2);
    row++;
    mainGrid->addWidget(okB, row, 0);
    mainGrid->addWidget(closeB, row, 2);

    connect(useExistingRB, SIGNAL(clicked()), SLOT(selectionToggled()));
    connect(createNewRB, SIGNAL(clicked()), SLOT(selectionToggled()));
    fillAllFields();    
}


CatalogDB::~CatalogDB()
{    
}

void CatalogDB::selectionToggled()
{
    nodesCB->setEnabled(useExistingRB->isChecked());
    nameLE->setEnabled(!useExistingRB->isChecked());
    hostLE->setEnabled(!useExistingRB->isChecked());
    commentLE->setEnabled(!useExistingRB->isChecked());
    portLE->setEnabled(!useExistingRB->isChecked());
}

void CatalogDB::fillAllFields()
{
    DBAPIPlugin* pApi = new DBAPIPlugin(_dbTypeName);
    if( !pApi->isValid() )
    {
        msg("Could not load the required plugin, sorry.");
        return;
    }
    GSeq<NODE_INFO*> nodeSeq = fillNodesCB(pApi);
    delete pApi;
    portLE->setText("50000");
    if( nodeSeq.numberOfElements() ) useExistingRB->setChecked(true);
    else createNewRB->setChecked(true);
    selectionToggled();
    aliasLE->setPlaceholderText("Default: same as Database");
    hostLE->setPlaceholderText("Server name or IP");
    databaseLE->setFocus();
}

GSeq<NODE_INFO*>  CatalogDB::fillNodesCB(DBAPIPlugin *pApi)
{
    nodesCB->clear();
    GSeq<NODE_INFO*> nodeSeq;
    nodeSeq = pApi->getNodeInfo();
    NODE_INFO* pInfo;
    GString data;
    for( int i = 1; i <= nodeSeq.numberOfElements(); ++i )
    {
        pInfo = nodeSeq.elementAtPosition(i);
        data = pInfo->NodeName+" ("+pInfo->HostName+"@"+pInfo->ServiceName+")";
        nodesCB->addItem(data);
    }
    return nodeSeq;
}

void CatalogDB::OKClicked()
{
    int erc;
    DBAPIPlugin* pApi = new DBAPIPlugin(_dbTypeName);
    if( !pApi->isValid() )
    {
        msg("Could not load the required plugin, sorry.");
        return;
    }
    GString database = databaseLE->text();
    GString alias = aliasLE->text();
    if( alias.strip().length() == 0 ) alias = database;
    if( !database.strip().length())
    {
        msg("Set database name.");
        return;
    }

    GString hostName, nodeName, port, comment, data;
    if( useExistingRB->isChecked() )
    {
        data = nodesCB->currentText();
        nodeName = data.subString(1, data.indexOf("(")-1).strip();
        data = data.remove(1, data.indexOf("("));
        hostName = data.subString(1, data.indexOf("@")-1).strip();
        data = data.remove(1, data.indexOf("@"));
        port = data.subString(1, data.indexOf(")")-1).strip();
    }
    else
    {
        nodeName = nameLE->text();
        hostName = hostLE->text();
        port     = portLE->text();
        comment  = commentLE->text();
        erc = pApi->createNode(hostName, nodeName, port, comment);
        if( erc )
        {
            msg("CATALOG NODE failed: "+GString(erc)+", "+pApi->SQLError());
            return;
        }
    }
    if( database.length() > 8 || nodeName.length() > 8 )
    {
        msg("DATABASE and NODENAME must not exceed 8 characters.");
        return;
    }


    erc = pApi->catalogDatabase(database, alias, '1', nodeName, NULL, comment, 0 );
    if( erc )
    {
        msg("CATALOG DATABASE failed: "+GString(erc)+", "+pApi->SQLError());
        return;
    }
    m_iCatalogChanged = 1;

    fillNodesCB(pApi);
    delete pApi;
    msg("Success: Database " + database + " is now catalogued. You may need to restart PMF to refresh the node cache!");
}

int CatalogDB::catalogChanged()
{
    return m_iCatalogChanged;
}

void CatalogDB::closeClicked()
{
    close();
}

void CatalogDB::msg(GString txt)
{
    Helper::msgBox(this, "pmf", txt);
}
void CatalogDB::saveGeometry()
{
    m_qrGeometry = this->geometry();
}
void CatalogDB::restoreGeometry()
{
    this->setGeometry(m_qrGeometry);
}
void CatalogDB::closeEvent(QCloseEvent * event)
{
    event->accept();
}
void CatalogDB::keyPressEvent(QKeyEvent *event)
{    
    if( event->key() == Qt::Key_Escape )
    {
        close();
    }
}

int CatalogDB::catalogDbAndNode(CATALOG_DB * pCatalogDb)
{
    int erc;
    DBAPIPlugin* pApi = new DBAPIPlugin(_DB2);
    if( !pApi->isValid() )
    {
        delete pApi;
        return 1001;
    }

    GSeq <DB_INFO*> dbSeq;
    dbSeq = pApi->dbInfo();

    GSeq<NODE_INFO*> nodeSeq;
    nodeSeq = pApi->getNodeInfo();

    int ret = CatalogDB::dbAliasExistsOnNode(pCatalogDb, &dbSeq, &nodeSeq);

    if( ret == CATDB_DB_AND_NODE_EXIST ) return 0;
    else if( ret == CATDB_DB_EXISTS ) //Alias exists, but on different node
    {
        erc = uncatalogDb(pCatalogDb);
        if( erc ) return erc;
    }

    if( !nodeExists(pCatalogDb, &nodeSeq) )
    {
        erc = pApi->createNode(pCatalogDb->Host, pCatalogDb->NodeName, pCatalogDb->Port, "PMF: AutoCreate");
        if( erc ) return erc;
    }
    //Catalog DB
    erc = pApi->catalogDatabase(pCatalogDb->Database, pCatalogDb->Alias, '1', pCatalogDb->NodeName, NULL, "PMF: AutoCreate", 0 );
    delete pApi;
    return erc;
}

int CatalogDB::nodeExists(CATALOG_DB * pCatalogDb, GSeq<NODE_INFO*>* nodeSeq)
{
    NODE_INFO* nodeInfo;
    for( int i = 1; i <= nodeSeq->numberOfElements(); ++i )
    {
        nodeInfo = nodeSeq->elementAtPosition(i);
        if( pCatalogDb->NodeName == nodeInfo->NodeName )
        {
            return 1;
        }
    }
    return 0;
}

int CatalogDB::dbAliasExistsOnNode(CATALOG_DB * pCatalogDb, GSeq <DB_INFO*> *dbSeq, GSeq<NODE_INFO*> *nodeSeq )
{
    DB_INFO* dbInfo;
    for( int i = 1; i <= dbSeq->numberOfElements(); ++i )
    {
        dbInfo = dbSeq->elementAtPosition(i);
        if( dbInfo->Alias == pCatalogDb->Alias && dbInfo->NodeName == pCatalogDb->NodeName )
        {
            return CATDB_DB_AND_NODE_EXIST;
        }
    }
    for( int i = 1; i <= dbSeq->numberOfElements(); ++i )
    {
        dbInfo = dbSeq->elementAtPosition(i);
        if( dbInfo->Alias == pCatalogDb->Alias )
        {
            return CATDB_DB_EXISTS;
        }
    }
    return CATDB_DB_NOT_FOUND;

}


int CatalogDB::uncatalogDb(CATALOG_DB * pCatalogDb)
{
    if( pCatalogDb->Alias.length() == 0 ) return 0;


    DBAPIPlugin* pApi = new DBAPIPlugin(_DB2);
    if( !pApi->isValid() )
    {
        delete pApi;
        return 1001;
    }
    GSeq <DB_INFO*> dataSeq;
    dataSeq = pApi->dbInfo();
    DB_INFO* pInfo;
    for( int i = 1; i <= (int)dataSeq.numberOfElements(); ++i)
    {
        pInfo = dataSeq.elementAtPosition(i);
        if( pInfo->Alias == pCatalogDb->Alias && !pInfo->NodeName.length() )
        {
            printf("uncatalog: DB is local\n");
            delete pApi;
            return 1002;
        }
    }

    int erc = pApi->uncatalogDatabase(pCatalogDb->Alias);
    delete pApi;
    return erc;
}


bool CatalogDB::eventFilter(QObject *obj, QEvent *event)
{
//    if (obj == databaseLE && event->type() == QEvent::KeyPress)
//    {
//        QKeyEvent *key = static_cast<QKeyEvent *>(event);

//        aliasLE->setText(databaseLE->text().append(key->key()));

//        //qApp->postEvent((QObject*)aliasLE,(QEvent *)event);
//    }
    return QObject::eventFilter(obj, event);
}
