//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include "loginbox.h"
#include "catalogDB.h"

#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QSortFilterProxyModel>
#include <QSettings>
#include <QTextBrowser>
#include <QProcess>
#include <QDomComment>

#include <qlayout.h>
#include "pmfdefines.h"
#include "connSet.h"
#include "odbcMdf.h"
#include <gstuff.hpp>
#include <qfont.h>
#include <gfile.hpp>
#include "helper.h"
#include "gxml.hpp"

#include <dsqlplugin.hpp>
#include <dbapiplugin.hpp>

#if defined(MAKE_VC) || defined (__MINGW32__)
#include <io.h>
#else
#include <unistd.h>
#endif

#define LGNBOX_HELP "Hint: Click <Help>!"
#define LGNBOX_MARIADB "<optional>"
#define LGNBOX_CREATE_NODE "<Catalog DB>"
#define LGNBOX_RUN_STARTUP "<Run Catalog Cmds>"
#define STARTUP_FILE_NAME "pmf_startup.xml"


LoginBox::LoginBox( GDebug *pGDeb, QWidget* parent ): QDialog(parent)
{

    m_pGDeb = pGDeb;
    this->setWindowTitle("Connect");
	QVBoxLayout *topLayout = new QVBoxLayout( );
	
    m_pMainGrid = new QGridLayout(this);
    topLayout->addLayout( m_pMainGrid, 10 );
	
	QLabel* tmpQLabel;

    int startRow = 0;
    tmpQLabel = new QLabel( this);
    tmpQLabel->setText( "Type" );
    tmpQLabel->setFixedHeight( tmpQLabel->sizeHint().height() );
    m_pMainGrid->addWidget(tmpQLabel, startRow+0, 0);

	tmpQLabel = new QLabel( this);
	tmpQLabel->setText( "Database" );
	tmpQLabel->setFixedHeight( tmpQLabel->sizeHint().height() );
    m_pMainGrid->addWidget(tmpQLabel, startRow+1, 0);
	
	tmpQLabel = new QLabel( this );
	tmpQLabel->setText( "UserID" );
	tmpQLabel->setFixedHeight( tmpQLabel->sizeHint().height() );
    m_pMainGrid->addWidget(tmpQLabel, startRow+2, 0);
	
	tmpQLabel = new QLabel( this );
	tmpQLabel->setText( "Password" );
	tmpQLabel->setFixedHeight( tmpQLabel->sizeHint().height() );
    m_pMainGrid->addWidget(tmpQLabel, startRow+3, 0);

    tmpQLabel = new QLabel( this );
    tmpQLabel->setText( "Host" );
    tmpQLabel->setFixedHeight( tmpQLabel->sizeHint().height() );
    m_pMainGrid->addWidget(tmpQLabel, startRow+4, 0);

    tmpQLabel = new QLabel( this );
    tmpQLabel->setText( "Port" );
    tmpQLabel->setFixedHeight( tmpQLabel->sizeHint().height() );
    m_pMainGrid->addWidget(tmpQLabel, startRow+5, 0);



    dbTypeCB = new QComboBox( );
    dbTypeCB->setFixedHeight( dbTypeCB->sizeHint().height() );
    m_pMainGrid->addWidget(dbTypeCB, startRow+0, 1, 1, 2);

	dbNameCB = new QComboBox( );
	dbNameCB->setFixedHeight( dbNameCB->sizeHint().height() );
    dbNameCB->setEditable(true);
    m_pMainGrid->addWidget(dbNameCB, startRow+1, 1, 1, 2);
    dbNameCB->setInsertPolicy(QComboBox::InsertAlphabetically);

	
	userNameLE = new QLineEdit(  );
	userNameLE->setText( "" );
	userNameLE->setFixedHeight( userNameLE->sizeHint().height() );
    m_pMainGrid->addWidget(userNameLE, startRow+2, 1, 1, 2);
	
	
	passWordLE = new QLineEdit();
	passWordLE->setText( "" );
	passWordLE->setEchoMode(QLineEdit::Password);
	passWordLE->setFixedHeight( passWordLE->sizeHint().height() );
    m_pMainGrid->addWidget(passWordLE, startRow+3, 1, 1, 2);
	
    hostNameLE = new QLineEdit();
    hostNameLE->setText( "" );
    hostNameLE->setFixedHeight( hostNameLE->sizeHint().height() );
    m_pMainGrid->addWidget(hostNameLE, startRow+4, 1, 1, 2);

    portLE = new QLineEdit();
    portLE->setText( "" );
    portLE->setFixedHeight( portLE->sizeHint().height() );
    m_pMainGrid->addWidget(portLE, startRow+5, 1, 1, 2);


    //Button widget
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    QWidget * buttonWdiget = new QWidget();
    buttonWdiget->setLayout(buttonLayout);
	okB = new QPushButton();
	connect( okB, SIGNAL(clicked()), SLOT(okClicked()) );
	okB->setText( "OK" );
    okB->setAutoRepeat( false );
    okB->setDefault(true);
	okB->setFixedHeight( okB->sizeHint().height() );
    buttonLayout->addWidget(okB);

	cancelB = new QPushButton();
	connect( cancelB, SIGNAL(clicked()), SLOT(cancelClicked()) );
	cancelB->setText( "Cancel" );
    cancelB->setAutoRepeat( false );
	cancelB->setFixedHeight( cancelB->sizeHint().height() );
    buttonLayout->addWidget(cancelB);

    helpB = new QPushButton();
    connect( helpB, SIGNAL(clicked()), SLOT(helpClicked() ) );
    helpB->setText( "Help" );
    helpB->setAutoRepeat( false );
    helpB->setFixedHeight( helpB->sizeHint().height() );
    buttonLayout->addWidget(helpB);
    m_pMainGrid->addWidget(buttonWdiget, startRow+6, 0, 1, 3);


    runPluginCheck();
    createPluginInfo();

    resize( 270, 180 );

    m_pIDSQL = NULL;

}

LoginBox::~LoginBox()
{
    dbName = _NO_DB;
    m_seqDBList.deleteAll();;
}

void LoginBox::runPluginCheck()
{
    QSettings settings(_CFG_DIR, "pmf5");
    GString pluginPath = "./plugins/";
    pluginPath = QCoreApplication::applicationDirPath()+"/plugins/";
    if( checkPlugins(pluginPath) == 0 )
    {
        DSQLPlugin::setPluginPath(pluginPath);
        DBAPIPlugin::setPluginPath(pluginPath);
        settings.setValue("PluginPath", QString((char*)DSQLPlugin::pluginPath()));
        return;
    }

    if( settings.value("PluginPath", -1).toInt() >= 0 )
    {
        pluginPath = settings.value("PluginPath").toString();
    }
    while( checkPlugins(pluginPath) )
    {
        GString msg = "No plugins found in "+pluginPath+",\nplease navigate to the directory where the plugins are located.";
        #if defined(MAKE_VC) || defined (__MINGW32__)
        msg += "\n\nValid plugins are db2dsql.dll, odbcdsql.dll, db2dcli.dll";
        #else
        msg += "\n\nValid plugins are libdb2dsql.so, libodbcdsql.so, libdb2dcli.so";
        #endif
        QMessageBox::information(this, "Missing plugins", msg);
        QString path = QFileDialog::getExistingDirectory (this, "Plugin path");
        if ( path.isNull()  ) return;
        pluginPath = path;        
    }
    settings.setValue("PluginPath", QString((char*)pluginPath));
}

void LoginBox::initBox()
{
    disconnect(dbNameCB, SIGNAL(activated(int)));
    disconnect(dbTypeCB, SIGNAL(activated(int)));
    dbNameCB->clear();

    if(dbTypeCB->count() > 1 )
    {        
        if( GString(dbTypeCB->itemText(0)) != _selStringCB) dbTypeCB->insertItem(0, _selStringCB);
        dbTypeCB->setCurrentIndex(0);
    }
    else if(dbTypeCB->count() == 1 )
    {
        deb("Only one dbType");
        getAllDatabases(0);
        dbTypeCB->setCurrentIndex(0);
        dbNameCB->setFocus();
    }
    dbTypeCB->setFocus();
    setDefaultCon();    
    connect(dbNameCB, SIGNAL(activated(int)), SLOT(getConnData(int)));
    connect(dbTypeCB, SIGNAL(activated(int)), SLOT(getAllDatabases(int)));
}

void LoginBox::createPluginInfo()
{
    deb("createPluginInfo start");
    GSeq <GString> list;
    GString dbType;
    int rc;
    DSQLPlugin::DBTypeNames(&list);

    for( int i = 1; i <= (int)list.numberOfElements(); ++i)
    {
        dbType = list.elementAtPosition(i);
        deb(dbType);
    }
    pInfoLBL = new QLabel("Binding....please wait...", this);
    pInfoLBL->setStyleSheet("color: red;");
    pInfoLBL->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    pInfoLBL->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
    m_pMainGrid->addWidget(pInfoLBL, m_pMainGrid->rowCount(), 0, 1, 3);
    pInfoLBL->setHidden(true);

    QSettings settings(_CFG_DIR, "pmf5");
    GString pluginPath = QCoreApplication::applicationDirPath() +"/plugins/";
    if( settings.value("PluginPath", -1).toInt() >= 0 )
    {
        pluginPath = settings.value("PluginPath").toString();
    }

    DSQLPlugin::setPluginPath(pluginPath);

    deb("createPluginInfo listCount: "+GString(list.numberOfElements()));
    for( int i = 1; i <= (int)list.numberOfElements(); ++i)
    {
        QLabel *pLBL;
        dbType = list.elementAtPosition(i);
        DSQLPlugin dsql(dbType);

        rc = dsql.loadError();
        deb("Loading dbType "+dbType+", rc: "+GString(rc));
        if( rc == PluginLoaded )
        {
            dbTypeCB->addItem(dbType);
            pLBL = new QLabel(dbType+": Plugin OK.", this);
        }
        else if( dbType == _POSTGRES || dbType == _MARIADB ) continue;
        else if( rc == PluginMissing ) pLBL = new QLabel(dbType+": Plugin missing.", this);
        else pLBL = new QLabel(dbType+": Plugin not loadable.", this);

        pLBL->setFrameStyle( QFrame::Panel | QFrame::Sunken );
        pLBL->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
        //pLBL->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        //Place info at top or bottom of GUI
        m_pMainGrid->addWidget(pLBL, m_pMainGrid->rowCount()+1, 0, 1, 3);
        //m_pMainGrid->addWidget(pLBL, i-1, 0, 1, 3);
    }
    initBox();
}



void LoginBox::okClicked()
{
    deb("LoginBox, okClicked Start");
    if( GString(dbTypeCB->currentText()) == _selStringCB)
    {        
        dbTypeCB->setFocus();
        return;
    }
    m_pIDSQL = new DSQLPlugin(dbTypeCB->currentText());
    m_pIDSQL->setGDebug(m_pGDeb);
    if( dbNameCB->currentText() == LGNBOX_CREATE_NODE )
    {
        CatalogDB foo(m_pIDSQL, this);
        foo.exec();
        if( foo.catalogChanged() ) initBox();
        return;
    }
    if( dbNameCB->currentText() == LGNBOX_RUN_STARTUP )
    {
        runAutoCatalog();
        return;
    }

    if( dbNameCB->currentText() == LGNBOX_HELP) return;

    if(m_pIDSQL == NULL || !m_pIDSQL->isOK() )
    {
        tm("Could not load plugin for this database, sorry.");
        return;
    }	

    dbName   = dbNameCB->currentText();
	userName = userNameLE->text();
	passWord = passWordLE->text();
    hostName = hostNameLE->text();
    port     = portLE->text();

    if( dbName.length() == 0 && GString(dbTypeCB->currentText()) != _MARIADB ) return;

    m_pIDSQL->disconnect();
    m_pIDSQL->setGDebug(m_pGDeb);

    if( nodeNameHasChanged(dbName, hostName) )
    {
        hostName = hostNameLE->text();
        return;
    }

    int erc = bindAndConnect(m_pIDSQL, dbName, userName, passWord, hostName, port);
    if( erc  )
    {
        if( (erc == -1097 ||erc == -1027) && (m_pIDSQL->getDBType() == DB2 || m_pIDSQL->getDBType() == DB2ODBC ) )
        {
            if( QMessageBox::question(this, "PMF", "The NODE "+hostName+" appears to not exist anymore. Do you want to uncatalog this databases?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes )
            {
                DBAPIPlugin* pApi = new DBAPIPlugin(m_pIDSQL->getDBTypeName());
                if( pApi->isValid() )
                {
                    pApi->uncatalogDatabase(dbName);
                }
                delete pApi;
                ConnSet::removeFromList(_DB2ODBC, dbName);
                ConnSet::removeFromList(_DB2, dbName);
                initBox();
            }
        }

        if( (erc == -1013 )  && (m_pIDSQL->getDBType() == DB2 || m_pIDSQL->getDBType() == DB2ODBC ) )
        {
            if( QMessageBox::question(this, "PMF", "Do you want to remove this entry from the list of known databases?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes )
            {
                ConnSet::removeFromList(_DB2ODBC, dbName);
                ConnSet::removeFromList(_DB2, dbName);
                initBox();
            }
        }
        dbName = _NO_DB;
        userName = "";
        passWord = "";
    }
    else if( dbName != LGNBOX_HELP )
    {
        CON_SET * pCS = new CON_SET;
        pCS->DefDB = 0;
        pCS->Type = dbTypeCB->currentText();
        pCS->DB = dbName;
        pCS->Host = hostName;
        pCS->UID = userName;
        pCS->Port = port;
        pCS->PWD = passWord;


        ConnSet cSet(m_pGDeb, NULL);
        int rc = cSet.isInList(pCS);
        if(rc == 0)
        {
            if( QMessageBox::question(this, "PMF", "Connected. Save connection information?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes )
            {
                cSet.addToSeq(pCS);
                cSet.save();
                deb("LoginBox, okClicked connset saved ok");
            }
            //else delete pCS;
        }
        else if(rc == 2) //Update UID, PWD
        {
            if( QMessageBox::question(this, "PMF", "Connected. Update connection information?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes )
            {
                cSet.updSeq(pCS);
                cSet.save();
            }
            //else delete pCS;
        }
        //else delete pCS;
        deb("LoginBox, deleting connset...");
        deb("LoginBox, deleting closing...");
        close();
    }
    deb("LoginBox, okClicked end");
}


void LoginBox::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_Escape:
            cancelClicked();
            break;
			
        case Qt::Key_Return:
            okClicked();
            break;

        default:
        event->accept();
			
    }
}

void LoginBox::closeEvent(QCloseEvent * event)
{
	event->accept();
}
void LoginBox::helpClicked()
{

    /*
    QByteArray systemRoot = qgetenv("SystemRoot");
    if (!systemRoot.isEmpty())
    {
        QString e1 = QString::fromLocal8Bit(systemRoot);
        QMessageBox::information(this, "pmf", e1);
        QProcess * qp = new QProcess(this);
        //QString prog = "%systemroot%\\SysWOW64\\odbcad32.exe";
        QString prog = e1+"\\SysWOW64\\odbcad32.exe";
        qp->start(prog);
        qp->waitForStarted();
        return;
    }
    */
//    OdbcMdf * pOdbcMdf = new OdbcMdf(m_pGDeb, this);
//    pOdbcMdf->show();
//    return;

    QDialog * helpVw = new QDialog(this);
    QTextBrowser* browser = new QTextBrowser();


    QHBoxLayout *helpLayout = new QHBoxLayout;
    helpLayout->addWidget(browser);
    helpVw->setLayout(helpLayout);


    helpVw->setFixedSize(800,500);
    helpVw->setWindowTitle("PMF Help - ESC to close");
    browser->setSource( QUrl("qrc:///pmfHelp_con.html") );
    helpVw->exec();
}

void LoginBox::cancelClicked()
{
    dbName = _NO_DB;
    hostName = _NO_DB;
    if( m_pIDSQL ) delete m_pIDSQL;
    m_pIDSQL = NULL;
    this->close();
}

void LoginBox::getAllDatabases(int pos)
{
    PMF_UNUSED(pos);
    deb("getAllDatabases start");
    userNameLE->setText("");
    passWordLE->setText("");
    hostNameLE->setText("");
    portLE->setText("");
    dbNameCB->clear();

    if( GString(dbTypeCB->currentText()) == _selStringCB)
    {
        okB->setEnabled(false);
        return;
    }
    else okB->setEnabled(true);

    hostNameLE->setEnabled(true);
    portLE->setEnabled(true);
    portLE->setText("");

    dbNameCB->clear();
    if( GString(dbTypeCB->currentText()) == _DB2)
    {
        //hostNameLE->setEnabled(false);
        //portLE->setEnabled(false);
        dbNameCB->setEditable(false);
    }
    else if( GString(dbTypeCB->currentText()) == _SQLSRV)
    {
		dbNameCB->setEditable(true);
    }
    loadConnnections(dbTypeCB->currentText());
    if( m_seqDBList.numberOfElements() ) getConnData(0);
}
void LoginBox::loadConnnections(GString dbType)
{
    ConnSet cSet(m_pGDeb, m_pIDSQL);
    cSet.getStoredCons(dbType, &m_seqDBList);
    fillCB();
}

void LoginBox::fillCB()
{
    dbNameCB->clear();
    GSeq <GString>sortSeq;
    for( unsigned long i = 1; i <= m_seqDBList.numberOfElements(); ++i )
    {
        sortSeq.add(m_seqDBList.elementAtPosition(i)->DB);
        deb("in seq: "+m_seqDBList.elementAtPosition(i)->DB);

    }
    sortSeq.sort();
    for( unsigned long i = 1; i <= sortSeq.numberOfElements(); ++i )
    {
        dbNameCB->addItem(sortSeq.elementAtPosition(i));
        deb("to CB: "+sortSeq.elementAtPosition(i));
    }
    if( m_seqDBList.numberOfElements() == 0 )
    {
        if( GString(dbTypeCB->currentText()) == _MARIADB )
        {
            dbNameCB->addItem(LGNBOX_MARIADB);
            portLE->setText("3306");
        }
        else if( GString(dbTypeCB->currentText()) == _POSTGRES )
        {
            portLE->setText("5432");
        }
        else dbNameCB->addItem(LGNBOX_HELP);
        hostNameLE->setText(_HOST_DEFAULT);        
    }
    if( GString(dbTypeCB->currentText()) == _DB2 )
    {
        DBAPIPlugin* pApi = new DBAPIPlugin(dbTypeCB->currentText());
        if( pApi->isValid() )
        {
            dbNameCB->addItem(LGNBOX_CREATE_NODE);
            if( haveStartupFile() ) dbNameCB->addItem(LGNBOX_RUN_STARTUP);
        }
        delete pApi;
    }
}

int LoginBox::nodeNameHasChanged(GString alias, GString hostName)
{
    if( GString(dbTypeCB->currentText()) != _DB2) return 0;
    GSeq <CON_SET*> csSeq;
    int rc = m_pIDSQL->getDataBases(&csSeq);
    if( rc )
    {
        msg("Could not determine databases, SqlCode: "+GString(m_pIDSQL->sqlCode())+", ErrorText: "+m_pIDSQL->sqlError());
        return 1;
    }
    CON_SET* pCS;
    for( int i = 1; i <= (int) csSeq.numberOfElements(); ++i )
    {
        pCS = csSeq.elementAtPosition(i);
        if( pCS->DB == alias && pCS->Host != hostName )
        {
            if( QMessageBox::question(this, "PMF", "The node for this database appears to have changed to '"+pCS->Host+"'\nSet this node?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes )
            {
                hostNameLE->setText(pCS->Host);
                userNameLE->setText("");
                passWordLE->setText("");
                userNameLE->setFocus();
                return 1;
            }
            break;
        }
    }
    return 0;
}


void LoginBox::setDefaultCon()
{
    deb("setDefaultCon");
    ConnSet cSet(m_pGDeb, NULL);
    int pos;
    if( cSet.errorMessages().length() ) msg(cSet.errorMessages()+"\n\nCheck your client configuration, maybe try connecting in a shell.");

    getConnData(0);

    CON_SET* pCS = cSet.getDefaultCon();
    deb("setDefaultCon, checking pCS for NULL");
    if( pCS == NULL ) return;

    deb("setDefaultCon, pCS OK. Type: "+pCS->Type);
    if( pCS->Type.length() )
    {
        pos = dbTypeCB->findText(pCS->Type);
        deb("DBType "+pCS->Type+", found at pos "+GString(pos));
        if( pos < 0 ) pos = 0;
        dbTypeCB->setCurrentIndex(pos);
    }
    getAllDatabases(0);
    if( pCS->DB.length() )
    {
        pos = dbNameCB->findText(pCS->DB);
        deb("DBName "+pCS->DB+", found at pos "+GString(pos));
        if( pos < 0 ) pos = 0;
        dbNameCB->setCurrentIndex(pos);
    }
    if( pCS->UID.length() ) userNameLE->setText(pCS->UID);
    else userNameLE->setText("");
    if( pCS->PWD.length() ) passWordLE->setText(pCS->PWD);
    else passWordLE->setText("");


    hostNameLE->setText(pCS->Host);
    portLE->setText(pCS->Port);
    dbTypeCB->blockSignals(false);
    dbNameCB->blockSignals(false);

}

void LoginBox::getConnData(int pos)
{
    PMF_UNUSED(pos);
	GString db = dbNameCB->currentText();
    userNameLE->setText("");
    passWordLE->setText("");
    hostNameLE->setText("");
    portLE->setText("");

    for( unsigned long i = 1; i <= m_seqDBList.numberOfElements(); ++i )
    {
        if( db != m_seqDBList.elementAtPosition(i)->DB ) continue;
        hostNameLE->setText(m_seqDBList.elementAtPosition(i)->Host);
        portLE->setText(m_seqDBList.elementAtPosition(i)->Port);
        userNameLE->setText(m_seqDBList.elementAtPosition(i)->UID);
        passWordLE->setText(m_seqDBList.elementAtPosition(i)->PWD);
        return;
    }
    //Nothing found, clear comboBox
    //dbNameCB->clear();
}
GString LoginBox::Port()
{
    return port;
}


GString LoginBox::HostName()
{
    return hostName;
}

GString LoginBox::DBName()
{
    if( GString(dbTypeCB->currentText()) == _MARIADB) return hostName;
	return dbName;
}
GString LoginBox::UserName()
{
	return userName;
}

GString LoginBox::PassWord()
{
	return passWord;
}
void LoginBox::deb(GString msg)
{
    m_pGDeb->debugMsg("loginBox", 1, msg);
}
int LoginBox::checkPlugins(GString pluginPath)
{
	pluginPath = pluginPath.stripTrailing("/") + "/";
    #if defined(MAKE_VC) || defined (__MINGW32__)
    if( _access(pluginPath+"db2dsql.dll", 0) >= 0 ) return 0;
    else if( _access(pluginPath+"odbcdsql.dll", 0) >= 0 ) return 0;
	#else
	if( access(pluginPath+"libdb2dsql.so", 0) >= 0 ) return 0;
	else if( access(pluginPath+"libodbcdsql.so", 0) >= 0 ) return 0;
    else if( access(pluginPath+"liboradsql.so", 0) >= 0 ) return 0;
	#endif    
	return 1;
}


int LoginBox::bindAndConnect(DSQLPlugin *pDSQL, GString db, GString uid, GString pwd, GString node, GString port)
{
    if( db == LGNBOX_MARIADB ) db = "";
    deb("bindAndConnect start.");
    if( !pDSQL ) return 1;
    deb("bindAndConnect db: "+db+", uid: "+uid+", node: "+node+", port: "+port);
    GString err = pDSQL->connect(db, uid, pwd, node, port);
    pDSQL->commit();
    if( err.length() )
    {
        msg(err);
        return pDSQL->sqlCode() == 0 ? -1 : pDSQL->sqlCode();
    }
    deb("bindAndConnect ret on NOT DB...");
    if( pDSQL->getDBType() != DB2 ) return 0;

    deb("bindAndConnect getSysTables...");
    //Now check if BIND is necessary:
    pDSQL->getSysTables();
    if( pDSQL->numberOfRows() > 0 ) return 0;

    pInfoLBL->setHidden(false);
    pInfoLBL->repaint();
    this->repaint();
    //msg("No Tables Found...Maybe BIND will help.\nPLEASE NOTE:\nThis should take only a few seconds. If PMF appears to hang, another instance of PMF is running somewhere.\nStop this instance and restart PMF.");

    //We need to disconnect before rebind:
    pDSQL->disconnect();

    GString bndFile;
    #if defined(MAKE_VC) || defined (__MINGW32__)
        bndFile = DBAPIPlugin::pluginPath()+"PMF"+GString(PMF_VER)+"W.bnd";
        bndFile = bndFile.change("/", "\\");
    #else
         bndFile = DBAPIPlugin::pluginPath()+"PMF"+GString(PMF_VER)+"L.bnd";
    #endif
    deb("loginBox, bndFile is "+bndFile);

    bndFile = checkBindFile(bndFile);
    pInfoLBL->setHidden(true);

    if( !bndFile.length() )
    {
        msg("Cannot bind: Bindfile is missing.");
        return 1;
    }
    int erc = pDSQL->bindIt(bndFile, db, uid, pwd, Helper::tempPath()+"BND.MSG");
    if( erc  )
    {
#if defined(MAKE_VC) || defined (__MINGW32__)
        msg("Could not bind, Error: "+GString(erc)+"\n"+pDSQL->sqlError()+"\n\nYou might try restarting PMF as Admin (required only once for BINDING)");
#else
        msg("Could not bind, Error: "+GString(erc)+"\n"+pDSQL->sqlError());
#endif
        return pDSQL->sqlCode();
    }
    //Check if BIND was successful:
    deb("Connecting ....");
    err = pDSQL->connect(db, uid, pwd, node, port);
    deb("getting SysTables ....");
    pDSQL->getSysTables();
    //if( pDSQL->numberOfRows() > 0 ) msg("Good news: Bind OK, Reconnect Too.");
    if( pDSQL->numberOfRows() == 0 )
    {
        msg("No luck. Possibly wrong version of BIND file (pmf5***.bnd) - OR - insufficient rights: Last errors are\n"+GString(err)+ "\nand\n"+pDSQL->sqlError());
        return 1;
    }
    deb("bindAndConnect done.");
    return 0;
}


/********************************
 *Check or get bndFile
 *******************************/
GString LoginBox::checkBindFile(GString bndFile)
{
    GFile gf(bndFile, GF_READONLY);
    if( gf.initOK() ) return bndFile;
     QString newName = "";
     msg("Could not find file '"+bndFile+"'\nThis file is needed for BINDing PMF to your database.\n"
        "Please enter the file's position in the following dialog.\n\n"
        "OR: bind "+bndFile+" manually ('db2 bind [path...]\\"+bndFile+"')");

     while( newName.indexOf(GStuff::fileFromPath(bndFile)) < 0 )
     {

            newName = QFileDialog::getOpenFileName(this, "Find: "+bndFile, ".");
            if( !newName.length() ) return "";
     }
#if defined(MAKE_VC) || defined (__MINGW32__)
     return GString(newName).translate('/', '\\');
#else
    return  GString(newName);
#endif
}

void LoginBox::msg(GString txt)
{
    QMessageBox::information(this, "pmf", txt);
}

void LoginBox::runAutoCatalog()
{
    QFile file(autoCatalogFilePath()+STARTUP_FILE_NAME);
    file.open(QFile::ReadOnly|QFile::Text);

    QDomDocument dom;
    QString error;
    int line, column;

    if(!dom.setContent(&file, &error, &line, &column))
    {
        msg("pmf_startup.xml: "+GString(error)+", line "+GString(line)+", column: "+GString(column));
        file.close();
        return;
    }
    file.close();

    GXml fXml;
    fXml.readFromFile(autoCatalogFilePath()+STARTUP_FILE_NAME);

    GXml outXml = fXml.getBlocksFromXPath("/Commands/CatalogList/Add/");
    int count = outXml.countBlocks("Add");
    for(int i=1; i <= count; ++i )
    {
        printf("Block at pos %i: %s\n", i, (char*) outXml.getBlockAtPosition("Add", i) .toString());
        CATALOG_DB cDB;
        cDB.Alias = outXml.getBlockAtPosition("Add", i).getAttribute("dbAlias");
        cDB.Database = outXml.getBlockAtPosition("Add", i).getAttribute("dbName");
        cDB.Host = outXml.getBlockAtPosition("Add", i).getAttribute("nodeHost");
        cDB.NodeName = outXml.getBlockAtPosition("Add", i).getAttribute("nodeName");
        cDB.Port = outXml.getBlockAtPosition("Add", i).getAttribute("nodePort");
        int erc = CatalogDB::catalogDbAndNode(&cDB);
        if( erc ) msg("Error "+GString(erc)+" on catalog dbAlias "+cDB.Alias);
    }

    outXml = fXml.getBlocksFromXPath("/Commands/CatalogList/Remove/");
    count = outXml.countBlocks("Remove");
    for(int i=1; i <= count; ++i )
    {
        printf("Block at pos %i: %s\n", i, (char*) outXml.getBlockAtPosition("Remove", i) .toString());
        CATALOG_DB cDB;
        cDB.Alias = outXml.getBlockAtPosition("Remove", i).getAttribute("dbAlias");
        int erc = CatalogDB::uncatalogDb(&cDB);
        if( erc ) msg("Error "+GString(erc)+" on uncatalog dbAlias "+cDB.Alias);
    }

    msg("Please restart PMF for changes to take effect.");
    return;
}

int LoginBox::haveStartupFile()
{
    if( QDir().exists(autoCatalogFilePath()+STARTUP_FILE_NAME)) return 1;
    return 0;
}

DSQLPlugin * LoginBox::getConnection()
{
    deb("GetConnection:");
    return m_pIDSQL;
}
