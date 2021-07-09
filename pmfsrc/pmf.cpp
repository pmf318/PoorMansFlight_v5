#include "pmf.h"

#if QT_VERSION >= 0x050000
#else
#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QAction>
#include <QMenuItem>
#endif

#include <QGridLayout>
#include <QDesktopServices>


#include <QColorDialog>
#include <QTabWidget>
#include <QFileDialog>
#include <QFontDialog>
#include <QSettings>
#include <QShortcut>
#include <QTextBrowser>
#include <QStyleFactory>
#include  <QKeyEvent>
#include <QProcess>
#include <QToolButton>


#include <sys/stat.h>

 
#include <gstring.hpp>
#include <gsocket.hpp>
#include <gstuff.hpp>
#include "loginbox.h"
#include "exportBox.h"
#include "importBox.h"
#include "simpleShow.h"
#include "deltab.h"
#include "allTabDDL.h"
#include "finddbl.h"
#include "pmfdefines.h"
#include "tabSpace.h"
#include "tbSize.h"
#include "getSnap.h"
#include "pmfCfg.h"
#include "getclp.h"
#include "querydb.h"
#include "bookmark.h"
#include "editBm.h"
#include "connSet.h"
#include "gfile.hpp"
#include "helper.h"
#include "downloader.h"
#include "catalogInfo.h"
#include "catalogDB.h"

#include "QResource"

#include "reorgAll.h"

#include "db2menu.h"
#include <qvarlengtharray.h>


//#include <string>
//using namespace std;

#ifdef MAKE_VC
#include <QWindow>
//#include <qpa/qplatformnativeinterface.h>
#endif

#ifdef MSVC_STATIC_BUILD
extern "C" int _except_handler4_common() {
    return 0; // whatever, I don't know what this is
}
#endif


Pmf::Pmf(GDebug *pGDeb, int threaded)
{	
    aThread = NULL;
    m_pGDeb = pGDeb;
    m_iThreaded = threaded;
    m_iForceClose = 0;
	deb("ctor start");
	
	m_iTabIDCounter = 0;
	m_iShowing = 0;
    m_pDownloader = 0;
    m_iColorScheme = 0;

	createGUI();
    m_tabWdgt = new QTabWidget;
    m_tabWdgt->setMovable(true);
    m_tabWdgt->setTabsClosable(true);
    connect(m_tabWdgt, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTabClicked(int)));


    m_mnuMainMenu = new QMenu("Menu", this);
    m_mnuAdmMenu = new QMenu("Administration", this);
    m_mnuTableMenu = new QMenu("Table", this);
    m_mnuBookmarkMenu = new QMenu("Bookmarks", this);
    m_mnuSettingsMenu = new QMenu("Settings", this);
    m_mnuMelpMenu = new QMenu("Help", this);

    //m_pDB2Menu = NULL;
    m_pIDSQL = NULL;
    connect( m_tabWdgt, SIGNAL( currentChanged ( int ) ), this, SLOT( curTabChg(int) ) );

	m_iCurrentTab = -1;
	    

    QVBoxLayout *mainVBox = new QVBoxLayout;
    createDownloadInfo();
    mainVBox->addWidget(downloadInfoBox);
    mainVBox->addWidget(m_tabWdgt);
    QWidget *widget = new QWidget();
    widget->setLayout(mainVBox);
    setCentralWidget(widget);

    deb("ctor: Disabling WinErrMsgBox on LoadLibrary failure. Set once in the application.");
    #ifdef MAKE_VC
    //LPDWORD lpOldMode;
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
    //SetThreadErrorMode(SEM_FAILCRITICALERRORS, lpOldMode);
    #endif


    QToolButton *addTabButton = new QToolButton(this);
    m_tabWdgt->setCornerWidget(addTabButton, Qt::TopRightCorner);
//    addTabButton->setCursor(Qt::ArrowCursor);
//    addTabButton->setAutoRaise(true);
    QIcon icoB;
    icoB.addPixmap(QPixmap(":addTab.png"), QIcon::Normal, QIcon::On);
    addTabButton->setIcon(icoB);
    connect(addTabButton, SIGNAL(clicked()), SLOT(addNewTab()));
    addTabButton->setToolTip(tr("Add new tab"));    

    //connect(m_tabWdgt, SIGNAL(tabOrderWasChanged()), SLOT(pmfTabOrderWasChanged()));
}

Pmf::~Pmf()
{
    deb("closing ::pmf...");
    if( m_pIDSQL )
    {
        delete m_pIDSQL;
        m_pIDSQL = NULL;
    }
    QDir tmpDir(Helper::tempPath());
    QStringList filters;
    filters << "PMF5_LOB*.*";
    tmpDir.setNameFilters(filters);
    QStringList filesList = tmpDir.entryList(filters);
    for(int i = 0; i < filesList.count(); ++i)remove(Helper::tempPath()+ GString(filesList.at(i)));

    CHECKBOX_ACTION * pCbAction;
    while( m_cbMenuActionSeq.numberOfElements() )
    {
        pCbAction = m_cbMenuActionSeq.firstElement();
        delete pCbAction;
        m_cbMenuActionSeq.removeFirst();
    }
    deb("closing ::pmf done.");
}

void Pmf::createDownloadInfo()
{
    downloadInfoBox = new QGroupBox();
    QGridLayout *downloadInfoLayout = new QGridLayout(this);
    QSpacerItem * spacer = new QSpacerItem(10, 10);

    //downloadInfoLayout->setColumnStretch(0, 1);
    //downloadInfoLayout->setColumnStretch(1, 1);
    downloadInfoBox->setLayout(downloadInfoLayout);
    downloadInfoLE = new QLineEdit(this);
    downloadInfoLE->setReadOnly(true);
    downloadInfoLE->setMinimumWidth(500);
    downloadInfoLE->setStyleSheet("background:#F6FA82;");

    downloadCancelButton = new QPushButton("Cancel");
    //downloadCancelButton ->setMaximumWidth(200);
    connect(downloadCancelButton, SIGNAL(clicked()), SLOT(downloadCancelled()));
    downloadInfoLayout->addWidget(downloadInfoLE, 0, 0);
    downloadInfoLayout->addWidget(downloadCancelButton, 0, 1);
    downloadInfoLayout->addItem(spacer, 0, 2);
    downloadInfoLayout->setColumnStretch(2, 3);

    downloadCancelButton->setEnabled(false);
    downloadInfoBox->setLayout(downloadInfoLayout);
    downloadInfoBox->hide();
}

void Pmf::timerEvent()
{
    if( !aThread ) return;

    if( !aThread->isAlive())
    {
        timer->stop();
    }
    if( m_gstrCurrentVersion.length() )
    {
        timer->stop();
        if( m_gstrCurrentVersion == "?" )
        {
            QSettings dateSettings(_CFG_DIR, "pmf5");
            int reply = QMessageBox::question(this, "Versioncheck", "Do you want to allow PMF to check for updates?\n"
                                              "(to change this later, go to menu->Settings)",  QMessageBox::Yes|QMessageBox::No);
            if( reply == QMessageBox::No)
            {
                dateSettings.setValue("checksEnabled", "N");
                m_actVersionCheck->setChecked(false);
                return;
            }
            else dateSettings.setValue("checksEnabled", "Y");
            return;
        }
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("New version");
//#ifdef MAKE_VC
//        msgBox.setInformativeText("There is a new version ("+m_gstrCurrentVersion+") available. \nDo you want to install it? ");
//        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
//        msgBox.setDefaultButton(QMessageBox::Yes);
//        int ret = msgBox.exec();
//        if( ret == QMessageBox::Yes)
//        {
//            getNewVersion();
//        }
//#else
        msgBox.setTextFormat(Qt::RichText);   //this is what makes the links clickable
        msgBox.setText("Hint: A new version ("+m_gstrCurrentVersion+") is available at <a href='http://leipelt.org/downld.html'>www.leipelt.org</a>");
        msgBox.exec();
//#endif

    }
}

void Pmf::MyThread::run()
{
   GStuff::dormez(12000);
   myPmf->m_gstrCurrentVersion = myPmf->checkForUpdate(1);   
}


QMenu * Pmf::createStyleMenu()
{
    QStringList styles;
    /* Obsolete (not working anymore), replaced by QStyleFactory::keys()
#ifndef MAKE_VC
    styles << "plastique" << "motif" << "cde" << "windows" << "cleanlooks" << "Fusion";
#else    
    styles << "plastique" << "windowsxp" << "windowsvista";
#endif
    */
    styles = QStyleFactory::keys();
    m_stylesMenu = new QMenu("Style", this);
    QAction *pLightAction, *pDarkAction;
    QSettings settings(_CFG_DIR, "pmf5");
    QString prevStyle;
#ifndef MAKE_VC
    prevStyle = settings.value("style", "Fusion").toString();
#else
    prevStyle = settings.value("style", "windowsvista").toString();
#endif
    GString styleName;
    for (int i = 0; i < styles.size(); ++i)
    {	
        styleName = (styles.at(i).toLocal8Bit()).data();
        pLightAction = new QAction(styleName, this);        
        pLightAction->setCheckable(true);
        pLightAction->setChecked(false);
        if( GString(prevStyle) == styleName)
		{
            pLightAction->setChecked(true);

            QApplication::setStyle(QStyleFactory::create(prevStyle));
            //qApp->setStyleSheet("QTabBar::close-button { image: url(:/pmf5.1/pmfsrc/icons/addTab.png) subcontrol-position: left; }");
            qApp->setStyleSheet("QTabBar::close-button { image: url(:closeTab.png); subcontrol-position: right;} QTabBar::close-button:hover {image: url(:closeTab_hover.png)}");
		}
        m_stylesMenu->addAction(pLightAction);
        connect(pLightAction, SIGNAL(triggered()), this, SLOT(setStyle()));
    }
    /*
    m_stylesMenu->addSeparator();
    for (int i = 0; i < styles.size(); ++i)
    {
        styleName = (styles.at(i).toLocal8Bit()).data();
        pDarkAction = new QAction(styleName + _DARK_THEME, this);
        pDarkAction->setCheckable(true);
        pDarkAction->setChecked(false);
        if( GString(prevStyle) == styleName + _DARK_THEME )
        {
            pDarkAction->setChecked(true);
            QApplication::setStyle(QStyleFactory::create(prevStyle));
        }
        m_stylesMenu->addAction(pDarkAction);
        connect(pDarkAction, SIGNAL(triggered()), this, SLOT(setStyle()));
    }
    */

    return m_stylesMenu;
}
void Pmf::createMenu(GString dbTypeName)
{
	deb("createMenu start");
	int dbt;
	if( dbTypeName == "DB2" ) dbt = 1;
	else dbt = 2;
	

    menuBar()->clear();
    m_mnuMainMenu->clear();
    m_mnuAdmMenu->clear();
    m_mnuTableMenu->clear();
    m_mnuBookmarkMenu->clear();
    m_mnuSettingsMenu->clear();
    m_mnuMelpMenu->clear();


    m_cbMenuActionSeq.deleteAll();

    //Settings
    m_mnuSettingsMenu->addAction("Configuration", this, SLOT(setConfig()));
    m_mnuSettingsMenu->addSeparator();
    m_mnuSettingsMenu->addAction("Set Font", this, SLOT(setPmfFont()));
    m_mnuSettingsMenu->addAction("Reset Font to default", this, SLOT(resetPmfFont()));
    m_mnuSettingsMenu->addSeparator();
    m_mnuSettingsMenu->addAction("Connection profiles", this, SLOT(setConnections()));

    // MainMenu
    m_mnuMainMenu->addAction("Connect",this,SLOT(loginClicked()));
    if( !dbTypeName.length() )
    {        
        if( m_pIDSQL == NULL )
        {
            DBAPIPlugin * pApi = new DBAPIPlugin(_DB2);
            if( pApi->isValid() )m_mnuMainMenu->addAction("Catalog DBs and Nodes", this, SLOT(catalogDBs()));
            delete pApi;
        }
        m_mnuMainMenu->addAction("Exit",this,SLOT(quitPMF()));
        menuBar()->addMenu(m_mnuMainMenu);
        menuBar()->addMenu(m_mnuSettingsMenu);
        return;
    }

    m_mnuMainMenu->addSeparator();
    m_mnuMainMenu->addAction("Next Tab", this, SLOT(nextTab()), QKeySequence(QKeySequence::NextChild));
    m_mnuMainMenu->addAction("Previous Tab", this, SLOT(prevTab()), QKeySequence(QKeySequence::PreviousChild));

    //QShortcut* scPrev = new QShortcut(QKeySequence::PreviousChild, this);
    //connect(scPrev, SIGNAL(activated()), this, SLOT(prevTab()));

    m_mnuMainMenu->addAction("New Tab",this,SLOT(createNewTab()), QKeySequence(Qt::CTRL + Qt::Key_T) );
    m_mnuMainMenu->addAction("Close Tab", this, SLOT(closeCurrentTab()), QKeySequence(Qt::CTRL + Qt::Key_W));
    m_mnuMainMenu->addSeparator();
    if( dbt == 1 ) m_mnuMainMenu->addAction("Catalog DBs and Nodes", this, SLOT(catalogDBs()));
    m_mnuMainMenu->addAction("Query DB", this, SLOT(queryDB()));
    m_mnuMainMenu->addAction("Drop tables", this, SLOT(deleteTable()));
    m_mnuMainMenu->addAction("Create DDLs", this, SLOT(createDDLs()));
    m_mnuMainMenu->addAction("Find identical rows", this, SLOT(findIdenticals()));

    if( dbt == 1 ) m_mnuMainMenu->addAction("Table spaces", this, SLOT(getTabSpace()));
    if( dbt == 1 ) m_mnuMainMenu->addAction("Table sizes", this, SLOT(tableSizes()));
    if( dbt == 1 ) m_mnuMainMenu->addAction("Database info", this, SLOT(showDatabaseInfo()));
    m_mnuMainMenu->addSeparator();
    m_mnuMainMenu->addAction("Exit",this,SLOT(quitPMF()));

    //Administration
    const char * nought = ""; //resolution probs on Manjaro
    if( dbt == 1 ) m_mnuAdmMenu->addAction("Snapshot", this, SLOT(snapShot()));
    else m_mnuAdmMenu->addAction("<DB2 only>", this, nought);

    //Table stuff
    m_mnuTableMenu->addAction("Export", this, SLOT(exportData()));
    if( dbt == 1 || m_pIDSQL->getDBType() == MARIADB ) m_mnuTableMenu->addAction("Import and Load", this, SLOT(importData()));
    m_mnuTableMenu->addSeparator();
    //m_mnuTableMenu->addAction("Create DDL", this, SLOT(createDDL()));
    m_mnuTableMenu->addAction("Table properties", this, SLOT(runstats()));

    //Bookmarks
    m_mnuBookmarkMenu->addAction("Add bookmark", this, SLOT(addBookmark()));
    m_mnuBookmarkMenu->addAction("Edit bookmarks", this, SLOT(editBookmark()));
    m_mnuBookmarkMenu->addSeparator();


    m_mnuSettingsMenu->addMenu(createStyleMenu());
    m_mnuSettingsMenu->addMenu(createCharForBitMenu());
    m_mnuSettingsMenu->addMenu(createRestoreMenu());

    createCheckBoxActions();



    //Help
    m_mnuMelpMenu->addAction("Index", this, SLOT(showHelp()));
    m_mnuMelpMenu->addAction("Check Updates", this, SLOT(checkForUpdate()));
	m_mnuMelpMenu->addAction("Show Debug", this, SLOT(showDebug()));
    m_mnuMelpMenu->addAction("Info", this, SLOT(showInfo()));


    menuBar()->addMenu(m_mnuMainMenu);
    menuBar()->addMenu(m_mnuTableMenu);
    menuBar()->addMenu(m_mnuAdmMenu);
    menuBar()->addMenu(m_mnuSettingsMenu);
    QAction* m_menuBarBookmarkAction = menuBar()->addMenu(m_mnuBookmarkMenu);
    connect(m_mnuBookmarkMenu, SIGNAL(aboutToShow()), this, SLOT(bookmarkMenuClicked()))  ;



    menuBar()->addMenu(m_mnuMelpMenu);

    if( m_actShowCloseOnTabs->isChecked() ) m_tabWdgt->setTabsClosable(true);
    else m_tabWdgt->setTabsClosable(false);
    getBookmarks();
	deb("createMenu done");
//    menuBar()->setFont(this->font());
//    m_mnuMainMenu->setFont(this->font());
//    m_mnuAdmMenu->setFont(this->font());
//    m_mnuTableMenu->setFont(this->font());
//    m_mnuBookmarkMenu->setFont(this->font());
//    m_mnuSettingsMenu->setFont(this->font());
//    m_mnuMelpMenu->setFont(this->font());

}
	

void Pmf::setStyle()
{  
	QList<QAction*> actionList = m_stylesMenu->actions();
	if (actionList.empty()) return;
	//Deselect all 
	for( int i = 0; i < actionList.count(); ++i ) 
    {
	    actionList.at(i)->setChecked(false);
	}

	QAction *action = qobject_cast<QAction *>(sender());
	if( !action ) return;	
	action->setChecked(true);
	//Changing style works flawless under Linux,
	//under Windows PMF gets resized and manual resizing is disabled...strange
	//NOTE: The above Windows problem occurs only when a tabEdit object exists.
    #if defined(MAKE_VC) || defined (__MINGW32__)
	msg("Changes take effect after restart.");
	#else
    msg("Changes take effect after restart.");
//    qApp->setStyle(action->text());
//    qApp->setStyle(QStyleFactory::create(action->text()));
	#endif
    QSettings settings(_CFG_DIR, "pmf5");
	settings.setValue("style", action->text());	

    QList<QWidget*> widgets = this->findChildren<QWidget*>();
    foreach (QWidget* w, widgets)
    {
        w->setPalette(m_qPalette);
    }
}
QMenu * Pmf::createCharForBitMenu()
{
    QStringList charForBit;
    charForBit << "Hide data" << "Show as binary" << "Show as HEX" << "Auto" ;
    m_charForBitMenu = new QMenu("CharForBit Data", this);
    QAction * ac;
    QSettings settings(_CFG_DIR, "pmf5");

    //The 2nd param is the default value if nothing is found
    QString prev = settings.value("charForBit", "Auto").toString();
    m_iCharForBit = DATA_AUTO;
    for (int i = 0; i < charForBit.size(); ++i)
    {	
        ac = new QAction(charForBit.at(i).toLocal8Bit(), this);
        ac->setCheckable(true);
        if( prev == QString(charForBit.at(i).toLocal8Bit()) )
        {
            ac->setChecked(true);
            if( prev == QString("Hide data") ) m_iCharForBit = DATA_HIDE;
            else if( prev == QString("Show as binary") ) m_iCharForBit = DATA_BIN;
            else if( prev == QString("Show as HEX") ) m_iCharForBit = DATA_HEX;
        }
        else ac->setChecked(false);
        m_charForBitMenu->addAction(ac);
        connect(ac, SIGNAL(triggered()), this, SLOT(setCharForBit()));
    }
    return m_charForBitMenu;
}
void Pmf::setCharForBit()
{  
	QList<QAction*> actionList = m_charForBitMenu->actions();
	if (actionList.empty()) return;
	for( int i = 0; i < actionList.count(); ++i ) actionList.at(i)->setChecked(false);

	QAction *action = qobject_cast<QAction *>(sender());
	if( !action ) return;	
	action->setChecked(true);
    QSettings settings(_CFG_DIR, "pmf5");
	settings.setValue("charForBit", action->text());	
    if( action->text() == "Hide data" ) m_iCharForBit = DATA_HIDE ;
    else if( action->text() == "Show as binary" ) m_iCharForBit = DATA_BIN;
    else if( action->text() == "Show as HEX" ) m_iCharForBit = DATA_HEX;
    else m_iCharForBit = DATA_AUTO;
	msg("Click 'Open' or 'Run Cmd' to refresh the view.");
	//msg("text: "+GString(action->text().toStdString().c_str())+", cfB: "+GString(m_iCharForBit));
}
QMenu * Pmf::createRestoreMenu()
{
    QStringList restore;
    restore << "Ask" << "Restore" << "Never" ;
    m_restoreMenu = new QMenu("Restore prev. session");
    QAction * ac;
    QSettings settings(_CFG_DIR, "pmf5");
    QString prev = settings.value("restore", "Ask").toString();

    m_iRestore = 1;
    for (int i = 0; i < restore.size(); ++i)
    {
        ac = new QAction(restore.at(i).toLocal8Bit(), this);
        ac->setCheckable(true);
        if( prev == QString(restore.at(i).toLocal8Bit()) )
        {
            ac->setChecked(true);            
            if( prev == QString("Restore") ) m_iRestore = 2;
            else if( prev == QString("Never") ) m_iRestore = 3;
        }
        else ac->setChecked(false);
        m_restoreMenu->addAction(ac);
        connect(ac, SIGNAL(triggered()), this, SLOT(setRestore()));
    }
    return m_restoreMenu;
}
void Pmf::setRestore()
{
    QList<QAction*> actionList = m_restoreMenu->actions();
    if (actionList.empty()) return;
    for( int i = 0; i < actionList.count(); ++i ) actionList.at(i)->setChecked(false);

    QAction *action = qobject_cast<QAction *>(sender());
    if( !action ) return;
    action->setChecked(true);
    QSettings settings(_CFG_DIR, "pmf5");
    settings.setValue("restore", action->text());
    if( action->text() == "Restore" ) m_iRestore = 2;
    else if( action->text() == "Never" ) m_iRestore = 3;
    else m_iRestore = 1;
}

void Pmf::curTabChg(int index)
{
    PMF_UNUSED(index);

	//This tab just got clicked.
	//Notify previously selected tab that is has lost focus.
	if( m_iCurrentTab >= 0 )
	{
		QWidget* pWdgt = m_tabWdgt->widget(m_iCurrentTab);
		if( pWdgt ) 
		{
			TabEdit * pTE = (TabEdit*) pWdgt;
			if( pTE ) pTE->lostFocus();	
		}
	}
	//Store the newly selected tab's index
	m_iCurrentTab = m_tabWdgt->currentIndex();
	if( m_iCurrentTab < 0 ) return;
	
	//Notify the tab that it has the focus now
	QWidget* pWdgt = m_tabWdgt->widget(m_iCurrentTab);
	if( pWdgt ) 
	{
		TabEdit * pTE = (TabEdit*) pWdgt;
		if( pTE ) pTE->gotFocus();	
	}
}
void Pmf::prevTab()
{
	if( !m_tabWdgt->count() ) return;
	if( m_tabWdgt->currentIndex() == 0 ) m_tabWdgt->setCurrentIndex(m_tabWdgt->count()-1);
	else m_tabWdgt->setCurrentIndex(m_iCurrentTab-1);
}
void Pmf::nextTab()
{
	if( !m_tabWdgt->count() ) return;
	if( m_iCurrentTab >= m_tabWdgt->count()-1 ) m_tabWdgt->setCurrentIndex(0);
	else m_tabWdgt->setCurrentIndex(m_iCurrentTab+1);

}
void Pmf::getGeometry(int *x, int *y, int *w, int *h )
{
	*x = this->x();
	*y = this->y();
	*w = this->width();
	*h = this->height();
}

void Pmf::addNewTab()
{
    createNewTab("", 0);
}

void Pmf::setColorScheme(int scheme, QPalette palette)
{
    m_iColorScheme = scheme;
    m_qPalette = palette;
}
int Pmf::getColorScheme()
{
    return m_iColorScheme;
}

void Pmf::closeTabClicked(int index)
{
    closeTab(index);
}

void Pmf::createNewTab(GString cmd, int asNext)
{

    deb("::createNewTab start ");
    deb("::createNewTab, asNext: "+GString(asNext)+", cmd: "+cmd);
    m_iTabIDCounter++;
    TabEdit * pTE = new TabEdit(this, m_tabWdgt, m_iTabIDCounter, m_pGDeb, m_iThreaded);
    pTE->setGDebug(m_pGDeb);
    pTE->setCheckBoxValues( &m_cbMenuActionSeq );    

    if( asNext )
    {
        if( m_iCurrentTab >= 0 )
        {
            TabEdit * pCurTE =  (TabEdit*) m_tabWdgt->widget(m_iCurrentTab);
            m_strLastSelectedContext = pCurTE->currentContext();
            m_strLastSelectedSchema = pCurTE->currentSchema();
        }

        m_tabWdgt->insertTab(m_iCurrentTab+1, pTE, _newTabString);
        m_tabWdgt->setCurrentIndex(m_iCurrentTab+1);
    }
    else
    {
        m_tabWdgt->addTab(pTE, _newTabString);
        m_tabWdgt->setCurrentIndex(m_tabWdgt->count()-1);
    }
    pTE->setFocus();
    m_iCurrentTab = m_tabWdgt->currentIndex();

    pTE->setDBName(m_gstrDBName);

    pTE->fillSchemaCB( m_strLastSelectedContext, m_strLastSelectedSchema);

    pTE->setHistTableName(m_strHistTableName);

    //connect( m_tabWdgt, SIGNAL( currentChanged ( int ) ), this, SLOT( curTabChg(int) ) );


    //cmd is set when we restore a previous session.
    //Do NOT run cmd if it's UPDATE, DELETE, INSERT, ....
    //...and do not set cmd to uppercase...
    if( cmd.strip().length() && GString(cmd).upperCase().subString(1, 6) == "SELECT")
    {
        pTE->setCmdLine(cmd);
        pTE->okClick();
        pTE->fillSelectCBs();
    }
    pTE->loadCmdHist();
    deb("::createNewTab, installing evtFilters...");
    pTE->installEventFilter(this);
    if( !cmd.length() ) pTE->popupTableCB();


//    QWidget * tabCloseButton;
//    for(int i = 0; i < m_tabWdgt->count(); ++i)
//    {
//        tabCloseButton = m_tabWdgt->tabBar()->tabButton(i, QTabBar::RightSide);
//        if( tabCloseButton != 0 )
//        {
//            tabCloseButton->resize(15, 15);
//        }
//    }
}

int Pmf::closeTab(int index)
{
    if( index < 0 ) return 0;
    QWidget* pWdgt = m_tabWdgt->widget(index);

    TabEdit * pTE = (TabEdit*) pWdgt;
    if( !pTE->canClose() ) return 1;

    //pWdgt->close();
    m_tabWdgt->removeTab(index);
    delete pWdgt;
    pWdgt = NULL;
    return 0;
}

int Pmf::closeCurrentTab()
{
    //msg("Current: "+GString(m_tabWdgt->currentIndex()));
    int index = m_tabWdgt->currentIndex();
    return closeTab(index);
}

int Pmf::closeDBConn()
{
    if( !m_gstrDBName.length() || m_gstrDBName == _NO_DB ) return 0;
		
	if( QMessageBox::question(this, "PMF", "Close this connection?", QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes ) return 1;

    setWindowTitle( Helper::pmfNameAndVersion() );
    disconnect( m_tabWdgt, SIGNAL( currentChanged ( int ) ), this, SLOT( curTabChg(int) ) );
    savePrevious();
	while(m_tabWdgt->count())
    {	        
        if( closeCurrentTab() )
        {
            connect( m_tabWdgt, SIGNAL( currentChanged ( int ) ), this, SLOT( curTabChg(int) ) );
            return 1;
        }
    }
    if( m_pIDSQL )
    {
        m_pIDSQL->disconnect();
        delete m_pIDSQL;
        m_pIDSQL = NULL;
    }
    m_iCurrentTab = -1;
    connect( m_tabWdgt, SIGNAL( currentChanged ( int ) ), this, SLOT( curTabChg(int) ) );
    return 0;
}

int Pmf::loginClicked()
{
    deb("loginClicked start");
    checkMigration();
    if( closeDBConn() > 0 ) return 0;
    //if( m_pDB2Menu ) delete m_pDB2Menu;

    LoginBox * lb = new LoginBox(m_pGDeb, this);
    lb->exec();
	m_gstrDBName = lb->DBName();

	deb("loginClicked, box done");
    m_pIDSQL = lb->getConnection();
	deb("loginClicked, creating menu...");
    if( !m_pIDSQL ) 
	{
		createMenu("");
		deb("loginClicked: m_pIDSQL is NULL.");
		return 1;
	}
    else createMenu(m_pIDSQL->getDBTypeName());
    deb("loginClicked, check for _NO_DB...");
	
    if( m_gstrDBName == _NO_DB ) return 1;
    setWindowTitle( Helper::pmfNameAndVersion(m_gstrDBName) );


    m_strHistTableName = histTableName();
	deb("loginClicked, restoring...");
    setFontFromSettings();
    if( !restorePrevious() )
    {
        if( m_tabWdgt->count() == 0 )createNewTab();

		deb("loginClicked, restoring "+GString(m_tabWdgt->count())+" tabs...");
        for( int i = 0; i < m_tabWdgt->count(); ++i )
        {
            QWidget* pWdgt = m_tabWdgt->widget(i);
            TabEdit * pTE = (TabEdit*) pWdgt;
            pTE->setDBName(m_gstrDBName);
            pTE->fillSchemaCB(m_actHideSysTabs->isChecked() ? 1 : 0 );
            pTE->loadCmdHist();
        }

    }
	m_gstrUID =  lb->UserName();
	m_gstrPWD =  lb->PassWord();
    m_gstrNODE = lb->HostName();
	deb("loginClicked, got connInfo.");
    lb->close();
    delete lb;
	
    timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), this, SLOT(timerEvent()) );

    timer->start( 1000 );
    aThread = new MyThread;
    aThread->setOwner( this );
    aThread->start();

	Helper::showHintMessage(this, 1002);
    Helper::showHintMessage(this, 1003);
	deb("loginClicked OK");
    return 0;
}

void Pmf::createGUI()
{

}

void Pmf::msg(GString txt)
{
	QMessageBox::information(this, "pmf", txt);
}

GString Pmf::currentSchema()
{
	if( m_tabWdgt->currentIndex() < 0 ) return "";
	GString s = m_tabWdgt->tabText(m_tabWdgt->currentIndex());
    if( s == _newTabString )
    {
         return "";
    }
    return s.subString(1, s.lastIndexOf(".")-1);
}
GString Pmf::currentTable()
{
	if( m_tabWdgt->currentIndex() < 0 ) return "";
	
	QWidget* pWdgt = m_tabWdgt->widget(m_tabWdgt->currentIndex());
	TabEdit * pTE = (TabEdit*) pWdgt;
	
	return pTE->currentTable(1);
	
	/*
	GString s = m_tabWdgt->tabText(m_tabWdgt->currentIndex());
	if( s == _newTabString ) return "";
	return s.subString(s.indexOf(".")+1, s.length()).strip();
	*/
}
void Pmf::setLastSelectedSchema(GString context, GString schema)
{
	m_strLastSelectedContext = context;
    m_strLastSelectedSchema = schema;
}

/***********************************************************************
*
* SLOTS
*
***********************************************************************/
void Pmf::setConfig()
{
    PmfCfg * foo = new PmfCfg(m_pGDeb, this);
	foo->exec();
    PmfCfg aCFG(m_pGDeb);
	GString mr = aCFG.getValue("MaxRows");
	for( int i = 0; i < m_tabWdgt->count(); ++i )
	{
		QWidget* pWdgt = m_tabWdgt->widget(i);
		TabEdit * pTE = (TabEdit*) pWdgt;
		pTE->setMaxRows(mr);
	}
}

void Pmf::setConnections()
{
    ConnSet * foo = new ConnSet(m_pGDeb, m_pIDSQL, this);
	foo->exec();
}

void Pmf::getNewVersion()
{
    if( m_pDownloader ) return;

    m_pDownloader = new Downloader(m_pGDeb);

    connect(m_pDownloader, SIGNAL (downloadCompleted()), SLOT (handleDownloadResult()));
    //connect(m_pDownloader, SIGNAL (downloadFailed()), SLOT (downloadFailed));
    disconnect( timer, SIGNAL(timeout()), this, SLOT(timerEvent()) );
    timer->start(500);
    connect( timer, SIGNAL(timeout()), this, SLOT(checkDownloadSize()) );
    downloadInfoBox->show();
    downloadInfoLE->setText("Dowloading");
    remove( newVersionFileLocation() );
    m_pDownloader->getPmfSetup();
}

void Pmf::downloadCancelled()
{
    printf("pmf::cancelClicked\n");
    if( m_pDownloader )
    {
        m_pDownloader->cancelDownload();
    }    
}


void Pmf::checkDownloadSize()
{
    printf("In chckDlD size\n");
    int size = m_pDownloader->downloadedSize();    
    int perc = size / 120000;
    if( size > 0 )
    {
        downloadCancelButton->setEnabled(true);
        downloadInfoLE->setText("Downloading: "+GString(perc)+"% ");
    }
    else downloadInfoLE->setText("Connecting (this may take a while)...");
}

void Pmf::handleDownloadResult()
{
    timer->stop();
    downloadInfoBox->hide();
    if( m_pDownloader )
    {
        if( m_pDownloader->downloadedSize() < 0 )
        {
            msg("Download failed.");
            delete m_pDownloader;
            m_pDownloader = 0;
            return;
        }
    }
    downloadInfoBox->hide();
    if( m_pDownloader ) delete m_pDownloader;
    m_pDownloader = 0;

#if defined(MAKE_VC)

    GString fileName = newVersionFileLocation();
    int rc = (int)ShellExecute(0, 0, fileName, 0, 0 , SW_SHOW );
    if( rc < 32 )
    {
        QProcess * qp = new QProcess(this);
        QString prog = "RUNDLL32.EXE SHELL32.DLL,OpenAs_RunDLL "+fileName;
        qp->start(prog);
    }
#endif
}

void Pmf::showInfo()
{
    GString s = "PMF (Poor Man's Flight) "+Helper::pmfVersion();
    s += "\n(C) Gregor Leipelt \n\nSee www.leipelt.net for updates and documentation";
	msg(s);    
}
void Pmf::exportData()
{
    QWidget* pWdgt = m_tabWdgt->widget(m_tabWdgt->currentIndex());
    TabEdit * pTE = (TabEdit*) pWdgt;
    QSettings settings(_CFG_DIR, "pmf5");
    m_gstrPrevExportPath = settings.value("exportPath", "").toString();

    if( !m_gstrPrevExportPath.length() ) m_gstrPrevExportPath = m_gstrPrevImportPath;

    ExportBox * foo = new ExportBox(m_pIDSQL, this, &m_gstrPrevExportPath);
    foo->setSelect(pTE->lastSqlSelectCmd(), pTE->currentTable() );
    foo->exec();
}

//NOT IN USE
void Pmf::importData()
{
    QSettings settings(_CFG_DIR, "pmf5");
    m_gstrPrevImportPath = settings.value("importPath", "").toString();

	if( !m_gstrPrevImportPath.length() ) m_gstrPrevImportPath = m_gstrPrevExportPath;
    ImportBox * foo = new ImportBox(m_pIDSQL, this, currentSchema(), currentTable(), &m_gstrPrevImportPath);
    foo->setGDebug(m_pGDeb);
	foo->exec();
}

void Pmf::catalogDBs()
{
    CatalogInfo *foo = new CatalogInfo(m_pIDSQL, this);
    foo->exec();
//    SimpleShow * foo = new SimpleShow("DBInfo", this);
//    DBAPIPlugin* pApi = new DBAPIPlugin(m_pIDSQL->getDBTypeName());
//    if( !pApi )
//    {
//        foo->setText("Could not load required plugin, sorry.");
//        return;
//    }
//    GSeq <DB_INFO*> dataSeq;
//    dataSeq = pApi->dbInfo();
//    GString out;
//    DB_INFO * pInfo;
//    for (int i = 1; i <= (int)dataSeq.numberOfElements(); ++i )
//    {
//        pInfo = dataSeq.elementAtPosition(i);
//        out += "Name: "+pInfo->Name+"\n";
//        out += "Alias: "+pInfo->Alias+"\n";
//        out += "Drive: "+pInfo->Drive+"\n";
//        //out += "Directory: "+pInfo->Directory+"\n";
//        out += "NodeName: "+pInfo->NodeName+"\n";
//        out += "Release Level: "+pInfo->ReleaseLevel+"\n";
//        out += "Comment: "+pInfo->Comment+"\n";
//        out += "DB Type: "+pInfo->DbType+"\n";
//        out += "*************************\n";
//    }
//    dataSeq.deleteAll();
//    delete pApi;
//    foo->setText(out);
//    foo->exec();
}

void Pmf::showDatabaseInfo()
{
    SimpleShow * foo = new SimpleShow("DBInfo", this);
    DBAPIPlugin* pApi = new DBAPIPlugin(m_pIDSQL->getDBTypeName());
    if( !pApi )
    {
        foo->setText("Could not load required plugin, sorry.");
        return;
    }
    GSeq <DB_INFO*> dataSeq;
    dataSeq = pApi->dbInfo();
    GString out;
    DB_INFO * pInfo;
    for (int i = 1; i <= (int)dataSeq.numberOfElements(); ++i )
    {
        pInfo = dataSeq.elementAtPosition(i);
        //out += "Name: "+pInfo->Name+"\n";
        out += "Alias: "+pInfo->Alias+"\n";
        out += "Drive: "+pInfo->Drive+"\n";
        //out += "Directory: "+pInfo->Directory+"\n";
        out += "NodeName: "+pInfo->NodeName+"\n";
        //out += "Release Level: "+pInfo->ReleaseLevel+"\n";
        out += "Comment: "+pInfo->Comment+"\n";
        out += "DB Type: "+pInfo->DbType+"\n";
        out += "*************************\n";
    }
    dataSeq.deleteAll();
    delete pApi;
    foo->setText(out);
    foo->exec();
}

void Pmf::createDDLs()
{
    AllTabDDL * foo = new AllTabDDL(m_pIDSQL, this, currentSchema(), m_actHideSysTabs->isChecked() ? 1 : 0 );
    foo->exec();
    delete foo;
}

void Pmf::deleteTable()
{
    Deltab * foo = new Deltab(m_pIDSQL, this, currentSchema(), m_actHideSysTabs->isChecked() ? 1 : 0 );
	foo->exec();
    delete foo;
}
void Pmf::findIdenticals()
{	
    Finddbl * foo = new Finddbl(m_pGDeb, m_pIDSQL, this, currentSchema(), m_actHideSysTabs->isChecked() ? 1 : 0);
	foo->exec();
    delete foo;
}
void Pmf::runstats()
{
	if( checkTableSet() ) return;	

    if( m_iCurrentTab >= 0 )
    {
        QWidget* pWdgt = m_tabWdgt->widget(m_iCurrentTab);
        if( pWdgt )
        {
            TabEdit * pTE = (TabEdit*) pWdgt;
            ReorgAll * foo = new ReorgAll(m_pIDSQL, pTE, currentTable(), m_actHideSysTabs->isChecked() ? 1 : 0);
            foo->exec();
            delete foo;
        }
    }
}
void Pmf::tableSizes()
{
    TableSize * foo = new TableSize(m_pIDSQL, this, currentSchema(), m_actHideSysTabs->isChecked() ? 1 : 0);
	foo->exec();
    delete foo;
}
void Pmf::getTabSpace()
{
    TabSpace * foo = new TabSpace(m_pIDSQL, this);
	foo->setTableName(currentSchema()+"."+currentTable());
	foo->exec();	
    delete foo;
}
void Pmf::snapShot()
{
    GetSnap * foo = new GetSnap(m_pIDSQL, this);
    this->getColorScheme();

	foo->setDBName(m_gstrDBName);
	if( !m_gstrNODE.length() ) m_gstrNODE = "DB2";
	foo->exec();	
    delete foo;
}
void Pmf::createDDL()
{
    if( checkTableSet() ) return;

    if( m_iCurrentTab >= 0 )
    {
        QWidget* pWdgt = m_tabWdgt->widget(m_iCurrentTab);
        if( pWdgt )
        {
            TabEdit * pTE = (TabEdit*) pWdgt;
            pTE->runGetClp();
        }
    }
}
void Pmf::queryDB()
{
    Querydb * foo = new Querydb(m_pIDSQL, this,currentSchema(), m_actHideSysTabs->isChecked() ? 1 : 0);
    foo->show();
    //foo->exec();
}
void Pmf::keyPressEvent(QKeyEvent * key)
{
    //The events are currently eaten in tabEdit.
    //if( key->key() == Qt::Key_Escape ) { key->ignore();}	///this->close();
    if( key->key() == Qt::Key_Escape ) this->close();
    else QMainWindow::keyPressEvent(key);
}
void Pmf::closePMF()
{
    if( QMessageBox::question(m_tabWdgt, "PMF", "Quit PMF?", QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes )
    {
        return;
    }
    disconnect( m_tabWdgt, SIGNAL( currentChanged ( int ) ), this, SLOT( curTabChg(int) ) );
    this->close();
}

void Pmf::closeEvent(QCloseEvent * event)
{
    deb("got close evt");
    disconnect( m_tabWdgt, SIGNAL( currentChanged ( int ) ), this, SLOT( curTabChg(int) ) );
    if( m_iForceClose )
    {
        while(m_tabWdgt->count())  closeCurrentTab();
        deb("close forced");
        event->accept();
        return;
    }
    /*
    if( QMessageBox::question(m_tabWdgt, "PMF", "Quit PMF?", QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes )
    {
        event->ignore();
        return;
    }
    */


    deb("Closing...");
    savePrevious();
	while(m_tabWdgt->count())
	{	
        if( closeCurrentTab() )
		{
			event->ignore();
			return;
		}
	}

    deb("got evt");
	event->accept();
    deb("Deleting IDSQL");
}

void Pmf::quitPMF()
{
	this->close();
}

void Pmf::resetPmfFont()
{
    QSettings settings(_CFG_DIR, "pmf5");
    settings.remove("font");
    msg("Please restart PMF for changes to take effect.");
}
void Pmf::setPmfFont()
{

	bool ok;
	QFont orgFont = this->font();
	QFont font = QFontDialog::getFont(&ok, orgFont, this);
	if (ok) 
	{
        QApplication::setFont(font);
        this->setFont(font);
        this->repaint();
        QCoreApplication::processEvents();
        msg("Please restart PMF for changes to take effect.");
        for( int i = 0; i < m_tabWdgt->count(); ++i )
        {
            QWidget* pWdgt = m_tabWdgt->widget(i);
            TabEdit * pTE = (TabEdit*) pWdgt;
            //pTE->setCellFont(&font);
        }
        QSettings settings(_CFG_DIR, "pmf5");
        settings.setValue("font", font.toString());
//        m_mnuMainMenu->setFont(font);
//        m_mnuAdmMenu->setFont(font);
//        m_mnuTableMenu->setFont(font);
//        m_mnuBookmarkMenu->setFont(font);
//        m_mnuSettingsMenu->setFont(font);
//        m_mnuMelpMenu->setFont(font);
	}

}
int Pmf::checkTableSet()
{
	if( !currentSchema().length() || !currentTable().length() )
	{
		msg("Please select/open a table.");
		return 1;
	}
	return 0;
}
void Pmf::addBookmark()
{
	QWidget* pWdgt = m_tabWdgt->widget(m_tabWdgt->currentIndex());
	TabEdit * pTE = (TabEdit*) pWdgt;
	if( pTE->currentTable().strip(".") == _selStringCB && 0 == pTE->getSQL().length()  )
	{
		msg("There is nothing to bookmark. At least open a table.");
		return;
	}
	
    Bookmark * foo = new Bookmark(m_pGDeb, this, pTE->getSQL(), pTE->currentTable());
	foo->exec();
	getBookmarks();
}
void Pmf::editBookmark()
{	
    EditBm * foo = new EditBm(m_pGDeb, this);
	foo->exec();
	getBookmarks();
}

void Pmf::setBookmarkData()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if( !action ) return;
	int pos = action->data().toInt();
	
    Bookmark bm(m_pGDeb);
	QWidget* pWdgt = m_tabWdgt->widget(m_tabWdgt->currentIndex());
	TabEdit * pTE = (TabEdit*) pWdgt;
    pTE->loadBookmark(bm.getBookmarkTable(pos), bm.getBookmarkFlatSQL(pos));
}
void Pmf::getBookmarks()
{
	deb("getBookmarks, start.");
	/***************************************************
	* This is rather nice: The menu-items are created during
	* run-time, the items are connected to the same slot.
	* See http://doc.trolltech.com/4.6/mainwindows-recentfiles-mainwindow-cpp.html
	*/
    m_mnuBookmarkMenu->clear();
    m_mnuBookmarkMenu->addAction("Add bookmark", this, SLOT(addBookmark()));
    m_mnuBookmarkMenu->addAction("Edit bookmarks", this, SLOT(editBookmark()));
    m_mnuBookmarkMenu->addSeparator();
	
	deb("calling bm ctor...");
    Bookmark * bm = new Bookmark(m_pGDeb);
    deb("getBookmark");
	//QAction * bmActs = new QAction[bm->count()];
	for( int i = 1; i <= bm->count() && i < MaxBookmarkActs; ++i )
	{
		bmActs[i-1] = new QAction(this);
		bmActs[i-1]->setText(bm->getBookmarkName(i));
		bmActs[i-1]->setData(i);
        m_mnuBookmarkMenu->addAction(bmActs[i-1]);
		connect(bmActs[i-1], SIGNAL(triggered()), this, SLOT(setBookmarkData()));		
	}
	deb("bookmarks done, calling dtor...");
	delete bm;
	deb("bookmarks OK.");
}
GString Pmf::restoreFileName()
{
    QString home = QDir::homePath ();
    if( !home.length() ) return "";
    #if defined(MAKE_VC) || defined (__MINGW32__)
    return GString(home)+"\\"+_CFG_DIR+"\\"+_RESTORE_FILE+m_gstrDBName;
    #else
    return GString(home)+"/."+_CFG_DIR + "/"+_RESTORE_FILE+m_gstrDBName;
    #endif
}

int Pmf::restorePrevious()
{
    int i = 0;
    QString home = QDir::homePath ();
    if( !home.length() ) return 0;


    GFile f(restoreFileName());
    if( !f.initOK() ) return 0;
    if( m_iRestore == 3 ) return 0;

    if( m_iRestore == 1 &&  f.lines() > 0 )
    {
        if( QMessageBox::question(this, "PMF", "Restore previous session?\n\nTired of this? Go to Menu->Settings->Restore prev. session", QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes ) return 0;
    }


    for( i = 0; i < f.lines(); ++i )
    {
        createNewTab(f.getLine(i+1));
    }
    return i;
}
void Pmf::savePrevious()
{
    GString line;
    QWidget* pWdgt;
    TabEdit * pTE;
    int haveLines = 0;
    for( int i = 0; i < m_tabWdgt->count(); ++i )
    {
        pWdgt = m_tabWdgt->widget(i);
        pTE = (TabEdit*) pWdgt;
        line = pTE->getLastSelect();
        if( line.length() ) haveLines++;
    }
    if( !haveLines ) return;

    GFile f(restoreFileName(), GF_OVERWRITE);

    for( int i = 0; i < m_tabWdgt->count(); ++i )
    {
        pWdgt = m_tabWdgt->widget(i);
        pTE = (TabEdit*) pWdgt;
        line = pTE->getLastSelect();
        if( line.length() )f.addLine(line);
    }
}

void Pmf::createCheckBoxActions()
{
    QSettings *settings = new QSettings(_CFG_DIR, "pmf5");

    m_actTextCompleter = new QAction("Enable text completer", this);
    setAndConnectActions(settings, m_actTextCompleter, _TEXTCOMPLETER, "Y");

    m_actCountAllRows = new QAction("Count rows", this);
    setAndConnectActions(settings, m_actCountAllRows, _COUNTROWS);


    m_actReadUncommitted = new QAction("Read uncommitted rows", this);
    setAndConnectActions(settings, m_actReadUncommitted, _READUNCOMMITTED);

    m_actHideSysTabs = new QAction("Hide system tables", this);
    setAndConnectActions(settings, m_actHideSysTabs, _HIDESYSTABS);

    m_actRefreshOnFocus = new QAction("Refresh Tabs on focus", this);
    setAndConnectActions(settings, m_actRefreshOnFocus, _REFRESHONFOCUS, "N");

    m_actShowCloseOnTabs = new QAction("Show close button on tabs", this);
    setAndConnectActions(settings, m_actShowCloseOnTabs, _SHOWCLOSEONTABS, "N");

    m_actConvertGuid = new QAction("Convert GUID", this);
    setAndConnectActions(settings, m_actConvertGuid, _CONVERTGUID, "Y");

    m_mnuSettingsMenu->addSeparator();

    m_actEnterToSave = new QAction("ENTER to Save/Update", this);
    setAndConnectActions(settings, m_actEnterToSave, _ENTERTOSAVE, "Y");

    m_actUseEscKey = new QAction("ESC closes PMF", this);
    setAndConnectActions(settings, m_actUseEscKey, _USEESCTOCLOSE);

    m_actVersionCheck = new QAction("Warn if new version is available", this);
    setAndConnectActions(settings, m_actVersionCheck, _CHECKSENABLED, "Y");

    m_mnuSettingsMenu->addSeparator();

}
void Pmf::setAndConnectActions(QSettings *settings,  QAction* action, QString name, QString defVal)
{
    QString prev;
    connect(action, SIGNAL(triggered()), SLOT(checkBoxAction()) );
    action->setCheckable(true);
    if( !defVal.length() ) defVal = name;
    prev = settings->value(name, defVal).toString();
    if( prev == "Y") action->setChecked(true);
    else action->setChecked(false);
    m_mnuSettingsMenu->addAction(action);

    CHECKBOX_ACTION * pCbAction = new CHECKBOX_ACTION;
    pCbAction->Action = action;
    pCbAction->Default = defVal;
    pCbAction->Name = name;
    m_cbMenuActionSeq.add(pCbAction);
}

void Pmf::checkBoxAction()
{
    QAction *action = qobject_cast<QAction *>(sender());
    for( int i = 0; i < m_tabWdgt->count(); ++i )
    {        
        QWidget* pWdgt = m_tabWdgt->widget(i);
        TabEdit * pTE = (TabEdit*) pWdgt;
        if( action == m_actHideSysTabs) pTE->fillSchemaCB();
        pTE->setCheckBoxValues(&m_cbMenuActionSeq);
    }
    QSettings settings(_CFG_DIR, "pmf5");

    if( action == m_actTextCompleter) msg("Changes take effect after restart");
    if( action == m_actCountAllRows && action->isChecked()) msg("Enabling this will do a \nSELECT COUNT(*) FROM <table>\nthe result will be displayed at the bottom in the 'info' field.\n\nBe aware that this may take a lot of time." );
    if( action == m_actConvertGuid && action->isChecked()) msg("Enabling this will convert GUIDs between DB2 and SqlServer format." );

    if( action == m_actShowCloseOnTabs)m_tabWdgt->setTabsClosable(m_tabWdgt->tabsClosable() ? false : true );

    //Save in settings
    CHECKBOX_ACTION* pAction;
    for(int i = 1; i <= m_cbMenuActionSeq.numberOfElements(); ++i )
    {
        pAction = m_cbMenuActionSeq.elementAtPosition(i);
        settings.setValue( pAction->Name , pAction->Action->isChecked() ? "Y" : "N" );
    }
}

void Pmf::deb(GString msg)
{    
    m_pGDeb->debugMsg("pmf", 1, msg);
}
void Pmf::showEvent( QShowEvent * evt)
{
   if( m_iShowing ) return;
#ifndef MAKE_VC
        return;
#endif
    QMainWindow::showEvent(evt);
    QTimer::singleShot(20, this, SLOT(callLogin()));
}
void Pmf::callLogin()
{    
    loginClicked();
    deb("callLogin done");
    m_iShowing = 1;
}
void Pmf::showDebug()
{
    m_pGDeb->setParent(this);
    for( int i = 0; i < m_tabWdgt->count(); ++i )
    {
        QWidget* pWdgt = m_tabWdgt->widget(i);
        TabEdit * pTE = (TabEdit*) pWdgt;
        //!!pTE->setGDebug(m_pGDeb);
    }
}
void Pmf::showHelp()
{
	
    QDialog * helpVw = new QDialog(this);

    QTextBrowser* browser = new QTextBrowser(helpVw);
    //QString path = ".";
    //browser->setSearchPaths(QStringList() << path );
	
    //QUrl::toLocalFile()
    browser->resize(800,500);

	helpVw->setWindowTitle("PMF Help - ESC to close");
    browser->setSource( QUrl("qrc:///pmfhelp.htm") );
	helpVw->show();	
			
}
DSQLPlugin* Pmf::getConnection()
{
    return m_pIDSQL;
}
GString Pmf::histTableName()
{
    deb("histTableName, start");
    DSQLPlugin plg(*m_pIDSQL);
    plg.initAll("SELECT COUNT(*) FROM SYSCAT.TABLES WHERE TABSCHEMA='PMF' AND TABNAME='HISTORY'");
    if( plg.rowElement(1,1).asInt() == 1 ) return "PMF.HISTORY";
    deb("histTableName, done");
    return "";
}


GString Pmf::checkForUpdate(int dailyCheck)
{
    //This runs in a Thread. Do not call deb(...) here while the DebOutputWindow is open.
    //deb("checkForUpdate start");
    QSettings dateSettings(_CFG_DIR, "pmf5");
    GString lastDate  = dateSettings.value("lastUpdCheck", "").toString();
    GString lastVer   = dateSettings.value("lastVerCheck", "").toString();
    GString checksEnabled = dateSettings.value("checksEnabled", "?").toString();

    //Do not check.
    //deb("Enabled: "+checksEnabled+", daily: "+GString(dailyCheck));
    if( checksEnabled == "N" && dailyCheck) return "";
    if( checksEnabled == "?" )
    {
        return checksEnabled;
    }
    //deb("checkForUpdate lastVer: "+lastVer+", PMF_VER: "+GString(PMF_VER));
    if(!lastVer.length())lastVer = GString(PMF_VER);

    GString today = QDate::currentDate().toString("yyyyMMdd");

	
    //deb("LastCheck: "+GString(lastDate.asInt())+", today: "+GString(today.asInt()));
    if( lastDate.asInt() >= today.asInt() && dailyCheck ) return "";

    dateSettings.setValue("lastUpdCheck", QDate::currentDate().toString("yyyyMMdd"));


    GString server = "leipelt.de";
    int port = 80;
    GString verSite = "/curver5.html";

    GSocket sock;
    GString data, ver, fullVer;
    sock.set_non_blocking(true);
    GStuff::startStopwatch();
    int rc = sock.connect(server, port);
    int seconds = GStuff::getStopwatch();
    //deb("Error from connect: "+GString(rc)+", waitTime: "+GString(seconds)+" seconds.");

    if( !rc )
    {
        rc = sock.sendRaw("GET "+verSite+" HTTP/1.0\r\n");
        //deb("Rc send1: "+GString(rc));
        rc = sock.sendRaw("Host: "+server+":"+GString(port)+"\r\n");
        //deb("Rc send2: "+GString(rc));
        rc = sock.sendRaw("\r\n"); //<- always close with \r\n, this signals end of http request
        //deb("Rc send3: "+GString(rc));
        rc = sock.recvRawText(&data);
        //deb("Rc recev: "+GString(rc));
    }
    if( rc )
    {
        if( !dailyCheck ) msg("Sorry, cannot check for new version. Please go to \nwww.leipelt.org");
        return "";
    }
    seconds = GStuff::getStopwatch();
    //deb("Error from recRaw: "+GString(rc)+", total waitTime: "+GString(seconds)+" seconds, data: "+data);


    ver = fullVer = getVersionFromHTML(data);
    ver = ver.removeAll('.');

    //deb("Ver: "+ver+", full: "+fullVer);
    //store lastVer: So we don't nag daily.

    if( ver.asInt() > GString(PMF_VER).asInt())
    {
        return fullVer;
    }
    else
    {
        if( !dailyCheck )msg("This version is up to date.\nTo enable or disable automatic check, click\nmenu->Settings ");
    }
    return "";
}
GString Pmf::getVersionFromHTML(GString data)
{
    GString ver;
    if( data.occurrencesOf("ver=")  )
    {
        ver = data.subString(data.indexOf("ver=")+4, data.length());
        if( ver.occurrencesOf("<") ) ver = ver.subString(1, ver.indexOf("<")-1);
        ver = ver.stripTrailing("\n");
    }
    return ver;
}

TabEdit * Pmf::currentTabEdit()
{
    if( checkTableSet() ) return NULL;
    if( m_iCurrentTab >= 0 )
    {
        QWidget* pWdgt = m_tabWdgt->widget(m_iCurrentTab);
        if( pWdgt )
        {
            return (TabEdit*) pWdgt;
        }
    }
    return NULL;
}

void Pmf::refreshTabOrder()
{
    for( int i = 0; i < m_tabWdgt->count(); ++i )
    {
        TabEdit * pTE = (TabEdit*) m_tabWdgt->widget(i);
        pTE->setNewIndex(i);
    }
}

void Pmf::checkMigration()
{
    GString home = GString(QDir::homePath());
    GString oldSettings;
    GString newSettings;
#if defined(MAKE_VC) || defined (__MINGW32__)
    oldSettings = home+"\\pmf4";
    newSettings = home+"\\"+_CFG_DIR;
#else
    oldSettings = home+"/.pmf4";
    newSettings = home+"/."+_CFG_DIR;
#endif

    if( QDir().exists(newSettings)) return;
    GString msg = "Do you want to move your settings from\n"+oldSettings+" to\n"+newSettings+" ?\n\n";
    msg += "If you are using pmf4 along with pmf5, click No.\n";
    msg += "If in doubt, click Yes.";
    if( QMessageBox::question(this, "PMF", msg, QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes ) return;

    rename(oldSettings, newSettings);
}
void Pmf::addToHostVarSeq(GSeq <GString> * pSeq)
{
    GString elmt;
    int found = 0;
    for(int j=1; j <= (int)pSeq->numberOfElements(); ++j)
    {
        found = 0;
        elmt = pSeq->elementAtPosition(j);
        for(int i=1; i <= (int) m_hostVarSeq.numberOfElements(); ++i)
        {
            if( m_hostVarSeq.elementAtPosition(i) == elmt )
            {
                found = 1;
                break;
            }
        }
        if( !found )m_hostVarSeq.add(elmt);
    }
    m_hostVarSeq.sort();
}
GSeq <GString>* Pmf::sqlCmdSeq()
{
    if( m_sqlCmdSeq.numberOfElements() == 0 )
    {
        m_sqlCmdSeq.add("select");
        m_sqlCmdSeq.add("update");
        m_sqlCmdSeq.add("insert");
        m_sqlCmdSeq.add("delete");
        m_sqlCmdSeq.add("from");
        m_sqlCmdSeq.add("where");
        m_sqlCmdSeq.add("and");
        m_sqlCmdSeq.add("or");
        m_sqlCmdSeq.add("order");
        m_sqlCmdSeq.add("by");
        m_sqlCmdSeq.add("not");
        m_sqlCmdSeq.add("in");
        m_sqlCmdSeq.add("right");
        m_sqlCmdSeq.add("left");
        m_sqlCmdSeq.add("outer");
        m_sqlCmdSeq.add("inner");
        m_sqlCmdSeq.add("upper");
        m_sqlCmdSeq.add("lower");
        m_sqlCmdSeq.add("coalesce");
        m_sqlCmdSeq.add("like");

        m_sqlCmdSeq.add("@DATA@");
        m_sqlCmdSeq.add("@COL@");
        m_sqlCmdSeq.add("@TABLE@");
    }
    return &m_sqlCmdSeq;
}

void Pmf::setFontFromSettings()
{
    QFont f;
    QSettings settings(_CFG_DIR, "pmf5");
    if( settings.value("font", -1).toInt() >= 0 )
    {
        f.fromString(settings.value("font").toString());
        QApplication::setFont(f);
    }
}


GSeq <GString>* Pmf::hostVarSeq()
{
    return &m_hostVarSeq;
}

QStringList Pmf::completerStringList()
{
    sqlCmdSeq();
    QStringList completerStrings;
    for(int i = 1; i <= (int)m_hostVarSeq.numberOfElements(); ++i) completerStrings += m_hostVarSeq.elementAtPosition(i);
    for(int i = 1; i <= (int)m_sqlCmdSeq.numberOfElements(); ++i) completerStrings += m_sqlCmdSeq.elementAtPosition(i);
    return completerStrings;
}

void Pmf::bookmarkMenuClicked()
{
    getBookmarks();
}


/*
bool pmf::eventFilter(QObject* object, QEvent* event)
{

    if( event->type() == QEvent::WindowActivate )
    {
        for( int i = 0; i < m_tabWdgt->count(); ++i )
        {
            QWidget* pWdgt = m_tabWdgt->widget(i);
            tabEdit * pTE = (tabEdit*) pWdgt;
            if( pTE == object)
            {
                printf("Activate\n");
                if( pTE->isChecked(_REFRESHONFOCUS) ) pTE->okClick();
            }
        }
    }
    if( event->type() == QEvent::WindowDeactivate )
    {
        printf("DeActivate\n");
    }

    return false;
}
*/
