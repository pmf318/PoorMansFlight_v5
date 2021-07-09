//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include "odbcMdf.h"

#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QSortFilterProxyModel>
#include <QSettings>
#include <QTextBrowser>
#include <QProcess>

#include <qlayout.h>
#include "pmfdefines.h"
#include "connSet.h"
#include <gstuff.hpp>
#include <qfont.h>
#include <gfile.hpp>

#include <dsqlplugin.hpp>
#include <dbapiplugin.hpp>

#ifdef MAKE_VC
#include <io.h>
#include <ODBCINST.H>
#else
#include <unistd.h>
#endif




OdbcMdf::OdbcMdf( GDebug *pGDeb, QWidget* parent )
: QDialog(parent)
{
    m_pGDeb = pGDeb;
    this->setWindowTitle("Connect");
	QVBoxLayout *topLayout = new QVBoxLayout( );
	
    m_pMainGrid = new QGridLayout(this);
    topLayout->addLayout( m_pMainGrid, 10 );
	
	QLabel* tmpQLabel;

    int startRow = 0;
    tmpQLabel = new QLabel( this);
    tmpQLabel->setText( "Driver" );
    tmpQLabel->setFixedHeight( tmpQLabel->sizeHint().height() );
    m_pMainGrid->addWidget(tmpQLabel, startRow+0, 0);

	tmpQLabel = new QLabel( this);
    tmpQLabel->setText( "DSN" );
	tmpQLabel->setFixedHeight( tmpQLabel->sizeHint().height() );
    m_pMainGrid->addWidget(tmpQLabel, startRow+1, 0);

	
	tmpQLabel = new QLabel( this );
    tmpQLabel->setText( "File" );
	tmpQLabel->setFixedHeight( tmpQLabel->sizeHint().height() );
    m_pMainGrid->addWidget(tmpQLabel, startRow+2, 0);
	
	tmpQLabel = new QLabel( this );
    tmpQLabel->setText( "Description" );
	tmpQLabel->setFixedHeight( tmpQLabel->sizeHint().height() );
    m_pMainGrid->addWidget(tmpQLabel, startRow+3, 0);

    driverCB = new QComboBox( );
    driverCB->setFixedHeight( driverCB->sizeHint().height() );
    driverCB->setEditable(true);

    m_pMainGrid->addWidget(driverCB, startRow+0, 1, 1, 2);
    driverCB->setInsertPolicy(QComboBox::InsertAlphabetically);
    driverCB->addItem("SQL Server Native Client 10.0");
    driverCB->addItem("SQL Server Native Client 11.0");

	
    dataSourceNameLE = new QLineEdit(  );
    dataSourceNameLE->setText( "Test" );
    dataSourceNameLE->setFixedHeight( dataSourceNameLE->sizeHint().height() );
    m_pMainGrid->addWidget(dataSourceNameLE, startRow+1, 1, 1, 2);
	
	
    fileNameLE = new QLineEdit();
    fileNameLE->setText( "c:\\pmf5.1\\ScanDB.mdf" );
    fileNameLE->setFixedHeight( fileNameLE->sizeHint().height() );
    m_pMainGrid->addWidget(fileNameLE, startRow+2, 1, 1, 2);
	
    descriptionLE = new QLineEdit();
    descriptionLE->setText( "" );
    descriptionLE->setFixedHeight( descriptionLE->sizeHint().height() );
    m_pMainGrid->addWidget(descriptionLE, startRow+3, 1, 1, 2);


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

    resize( 270, 180 );
}

OdbcMdf::~OdbcMdf()
{
}

void OdbcMdf::okClicked()
{
    GString driver      = driverCB->currentText();
    GString dsn         = dataSourceNameLE->text();
    GString fileName    = fileNameLE->text();
    GString description = descriptionLE->text();

    bool rc;
    /*
    boolean rc = SQLConfigDataSource (NULL, ODBC_CONFIG_SYS_DSN, driver, "DSN="+dsn+"\0CREATE_DB="+fileName+"\00");
    if( rc )   QMessageBox::information(this, "Missing plugins", "fail(1)");
    else return;
    rc = SQLConfigDataSource (NULL, ODBC_ADD_SYS_DSN, dsn, "DSN="+dsn+"\00DBQ="+fileName+"\ 00FIL=MSAccess\00Description="+description+"\00UID=\00");
    if( rc )   QMessageBox::information(this, "Missing plugins", "fail(2)");
    */
    /*
    rc = SQLConfigDataSource (NULL, ODBC_CONFIG_SYS_DSN, "Microsoft Access Driver (*.mdb)", "DSN=TESTDSN_DSN\0CREATE_DB=c:\\TESTDSN_DB.mdb\00");
    if( !rc )   QMessageBox::information(this, "Missing plugins", "fail(1)");
    rc = SQLConfigDataSource (NULL, ODBC_ADD_SYS_DSN, "MicrosoftAccess Driver (*.mdb)", "DSN=TESTDSN_DSN\00DBQ=c:\\TESTDSN_DB.mdb\ 00FIL=MSAccess\00Description=TESTDSN_database\00UID=\00");
    if( !rc )   QMessageBox::information(this, "Missing plugins", "fail(2)");
    */
    /* SQLInstallerError. *pfErrorCode*/

//    rc = SQLConfigDataSource(NULL,ODBC_ADD_SYS_DSN,"SQL Server",
//                       /*"DSN="+dsn+"\0"*/
//                       /*"Description="+description+"\0"*/
//                       "SourceType=MDF\\0"
//                       "AttachDBFileName="+fileName+"\\0"
//                       /*"Server=localhost\SQLExpress\0"*/
//                       "Address=localhost\\00");
//    if( rc )   QMessageBox::information(this, "Missing plugins", "fail(2)");
//    InstallerError();
#ifdef MAKE_VC
    QMessageBox::information(this, "Missing plugins", "start(1)");
    SQLConfigDataSource ((HWND)QWidget::winId(), ODBC_ADD_SYS_DSN, "SQL Server", "DSN=Test\0Server=localhost\\SQLExpress\00");
    InstallerError();
    QMessageBox::information(this, "Missing plugins", "start(2)");
    GString attr = "DSN=Test\0";
    attr += "Server=localhost\\SQLExpress\0";
    attr += "Description=this is a sample\0";
    attr += "Database=Sample\0";
    attr += "Trusted_Connection=YES\0";
    attr += "AttachDBFileName="+fileName+"\0";
    rc = SQLConfigDataSource((HWND)QWidget::winId(),ODBC_CONFIG_SYS_DSN, "SQL Server", attr);
    if( rc )   QMessageBox::information(this, "Missing plugins", "fail(2)");
    InstallerError();
    //ODBC_ERROR_INVALID_REQUEST_TYPE
    //SQLGetInstalledDrivers

//    p = ODBC_ADD_SYS_DSN; //value of ODBC_ADD_SYS_DSN enumeration constant
//    s1= SystemFolder ^"sqlsvr32.dll";
//    s2 = "DSN=dsnNAME\\0 SourceDB=" + szDBPath ^ "dsnNAME_DATA.mdf";

//    str1 = &s1;
//    str2 = &s2;
//    nReturn = SQLConfigDataSource (0, ODBC_ADD_SYS_DSN, , s2);
#endif
}

void OdbcMdf::InstallerError()
{
#ifdef MAKE_VC
    DWORD pErr;
    //LPSTR szErrMsg;
    SQLCHAR szErrMsg[512];
    WORD cbMsgBuffer=512;
    WORD cbRet;
    WORD wErrNum=1;
    printf("Start DEB\n");
    while(SQLInstallerError(wErrNum,&pErr,(LPSTR)szErrMsg,cbMsgBuffer,&cbRet)!=SQL_NO_DATA)
    {
        printf("%u\t%u\t%s\n",wErrNum,pErr,szErrMsg);
        wErrNum++;
    };
    printf("End \n");
#endif
}

void OdbcMdf::closeEvent(QCloseEvent * event)
{
	event->accept();
}

void OdbcMdf::helpClicked()
{

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

void OdbcMdf::cancelClicked()
{
    this->close();
}

void OdbcMdf::deb(GString msg)
{
    m_pGDeb->debugMsg("OdbcMdf", 1, msg);
}
