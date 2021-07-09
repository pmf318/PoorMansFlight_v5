//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include "connSet.h"
#include "pmfdefines.h"
#include <gfile.hpp>

#include <QGridLayout>
#include <QHeaderView>
#include <QDir>

#include <QGroupBox>
#include <QObject>

#include "gstuff.hpp"
#include "helper.h"
#include "pmf.h"
#include "pmfdefines.h"

#define ConnSetVer 3

static int m_iConnSetInstanceCounter = 0;

ConnSet::ConnSet(GDebug *pGDeb, DSQLPlugin* pIDSQL, QWidget *parent)
  :QDialog(parent)
{    
    m_twMain = NULL;
    m_pGDeb = pGDeb;
    m_strErrorMessages = "";
    deb("connSet start");
    loadFromFile();



    GSeq <GString> list;
    DSQLPlugin::DBTypeNames(&list);
    for( unsigned long i = 1; i <= list.numberOfElements(); ++i)
    {
        deb("connSet, loading databases for "+list.elementAtPosition(i));
        findDatabases(list.elementAtPosition(i));
    }
	m_iChanged = 0;
    m_pMainWdgt = parent;
    m_iConnSetInstanceCounter++;
    m_iMyInstanceCounter = m_iConnSetInstanceCounter;
    deb("connSet: Created instance "+GString(m_iMyInstanceCounter));

    migrateData();
    //This class may be called w/o parent to retrieve connection settings.
	if( !parent ) return;
	
    m_pIDSQL = pIDSQL;
	QGridLayout * pMainGrid = new QGridLayout(this);
	
    this->resize(750, 400);
    ok   = new QPushButton(this);
    esc  = new QPushButton(this);
    add  = new QPushButton(this);
    rem  = new QPushButton(this);
    //test = new QPushButton(this);
    ok->setText("Save");
	add->setText("New");
	rem->setText("Delete");
	esc->setText("Cancel");
    //test->setText("Test");
	//QLabel * hint1 = new QLabel("Set configuration here.", this);
	//QLabel * hint2 = new QLabel("", this);	
	
    connect(ok,   SIGNAL(clicked()), SLOT(save()));
    connect(esc,  SIGNAL(clicked()), SLOT(cancel()));
    connect(add,  SIGNAL(clicked()), SLOT(newEntry()));
    connect(rem,  SIGNAL(clicked()), SLOT(delEntry()));
    //connect(test, SIGNAL(clicked()), SLOT(testEntry()));

	m_twMain = new QTableWidget(this);
    m_twMain->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_twMain->setSortingEnabled(true);
    m_twMain->setColumnCount(7);
	pMainGrid->addWidget(m_twMain, 0 , 0, 1, 3);	

	QGroupBox * lowerBox = new QGroupBox();
	QHBoxLayout *lowerLayout = new QHBoxLayout;
	lowerLayout->addWidget(add);
    lowerLayout->addWidget(rem);
    //lowerLayout->addWidget(test);
	lowerLayout->addWidget(ok);
	lowerLayout->addWidget(esc);

	lowerBox->setLayout(lowerLayout);

    QTableWidgetItem * pItem2 = new QTableWidgetItem("Type");
    m_twMain->setHorizontalHeaderItem(0, pItem2);
    QTableWidgetItem * pItem3 = new QTableWidgetItem("Database");
    m_twMain->setHorizontalHeaderItem(1, pItem3);
    QTableWidgetItem * pItem4 = new QTableWidgetItem("Username");
    m_twMain->setHorizontalHeaderItem(2, pItem4);
    QTableWidgetItem * pItem5 = new QTableWidgetItem("Password");
    m_twMain->setHorizontalHeaderItem(3, pItem5);
    QTableWidgetItem * pItem6 = new QTableWidgetItem("Host");
    m_twMain->setHorizontalHeaderItem(4, pItem6);
    QTableWidgetItem * pItem7 = new QTableWidgetItem("Port");
    m_twMain->setHorizontalHeaderItem(5, pItem7);
    QTableWidgetItem * pItem1 = new QTableWidgetItem("Default");
    m_twMain->setHorizontalHeaderItem(6, pItem1);

	pMainGrid->addWidget(lowerBox, 5 , 0, 1, 3);

	QLabel * warn = new QLabel(this);
    warn->setText("WARNING: Passwords will be saved weakly encrypted in '"+_CONNSET_FILE+"' in your home directory");
	pMainGrid->addWidget(warn, 6, 0);


	//Read CFG and display data
    //loadAll() was called right at the start
    createRows();
    connect((QWidget*)m_twMain->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(sortClicked(int)));
    Helper::setVHeader(m_twMain);
}

ConnSet::~ConnSet()
{
    deb("connSet: destroying instance "+GString(m_iMyInstanceCounter));
    clearSeq(&m_seqCons);
    deb("connSet: destroying instance "+GString(m_iMyInstanceCounter+" done."));
    m_iConnSetInstanceCounter--;
}

void ConnSet::clearSeq(GSeq <CON_SET*> *pSeq)
{
    CON_SET* pCS;
    deb("connSet: clearSeq, elmts: "+GString(pSeq->numberOfElements()));
    while( !pSeq->isEmpty() )
    {
        pCS = pSeq->firstElement();
        delete pCS;
        pSeq->removeFirst();
    }
    deb("connSet: clearSeq ok");
}

int ConnSet::save()
{
    if( m_pMainWdgt) writeToSeq();
    GString out;
    CON_SET *pCS;
    GSeq <GString> seqAll;
    for(int i = 1; i <= (int)m_seqCons.numberOfElements(); ++ i)
    {
        pCS = m_seqCons.elementAtPosition(i);
        pCS->CSVer = ConnSetVer;
        out = "";
        out += GString(pCS->DefDB)+_PMF_PASTE_SEP;
        out += pCS->Type+_PMF_PASTE_SEP;
        out += pCS->DB+_PMF_PASTE_SEP;
        out += pCS->UID+_PMF_PASTE_SEP;
        out += GStuff::encryptToBase64(pCS->PWD)+_PMF_PASTE_SEP;
        out += pCS->Host+_PMF_PASTE_SEP;
        out += pCS->Port+_PMF_PASTE_SEP;
        out += pCS->CSVer;
		
		seqAll.add(out);
	}
    deb("connSet: cfgFileName: "+ cfgFileName());
    Helper::createFileIfNotExist(cfgFileName());
	GFile f( cfgFileName(), GF_OVERWRITE );
    if( f.initOK() )    {
        f.addLine("-- DO NOT CHANGE --");
        f.addLine("Version 2");
        f.addLine("-- END --");

        f.overwrite(&seqAll);
    }
    else deb("connSet: cfgFileName: "+ cfgFileName()+" FAILED");
	m_iChanged = 0;
	close();
    return 0;
}

void ConnSet::writeToSeq()
{

    QCheckBox * pChkBx;
    QLineEdit * pLE;


    CON_SET * pConSet;

    clearSeq(&m_seqCons);
    for(int i = 0; i < m_twMain->rowCount(); ++i )
    {
        pConSet = new CON_SET;

        pChkBx = qobject_cast<QCheckBox*>(m_twMain->cellWidget(i, 6));
        if( pChkBx->checkState() == Qt::Checked ) pConSet->DefDB = 1;
        else pConSet->DefDB = 0;	//Default 0/1

        pConSet->Type = m_twMain->item(i, 0)->text();
        pConSet->DB   = m_twMain->item(i, 1)->text();
        pConSet->UID = m_twMain->item(i, 2)->text();

        pLE = qobject_cast<QLineEdit*>(m_twMain->cellWidget(i, 3));
        pConSet->PWD = pLE->text();//password

        pConSet->Host = m_twMain->item(i, 4)->text();
        pConSet->Port = m_twMain->item(i, 5)->text();
        pConSet->CSVer = ConnSetVer;

        addToSeq(pConSet);
    }
}

void ConnSet::createRows()
{
    for(int i = 1; i <= (int)m_seqCons.numberOfElements(); ++i) createRow(m_seqCons.elementAtPosition(i));
}

void ConnSet::keyPressEvent(QKeyEvent * key)
{
    if( key->key() == Qt::Key_Escape ) 	this->close();
}

void ConnSet::closeEvent(QCloseEvent * event)
{
	if( m_iChanged ) 
	{
		if( QMessageBox::question(this, "PMF", "Quit without saving?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::No ) 
		{
			event->ignore();
			return;
		}
	}
	event->accept();
}

void ConnSet::cancel()
{
	close();
}

int ConnSet::removeFromList(GString dbType, GString dbName)
{
    CON_SET cs;
    cs.Type = dbType;
    cs.DB = dbName;
    ConnSet classCS(NULL, NULL);
    classCS.removeFromSeq(&cs);
    classCS.save();
    return 0;
}

int ConnSet::removeFromSeq(GString dbType, GString dbName)
{
    CON_SET cs;
    cs.Type = dbType;
    cs.DB = dbName;
    return removeFromSeq(&cs);
}

int ConnSet::removeFromSeq(CON_SET *pNewCS)
{
    int pos = findElementPos(pNewCS);
    if( pos < 0 ) return 1;
    m_seqCons.removeAt(pos);
    return 0;
}

int ConnSet::findElementPos(CON_SET *pNewCS)
{
    CON_SET *cs;
    for(int i = 1; i <= (int)m_seqCons.numberOfElements(); ++i)
    {
        cs = m_seqCons.elementAtPosition(i);
        if( cs->DB == pNewCS->DB && cs->Type == pNewCS->Type )
        {
            cs->Host = pNewCS->Host;
            cs->Port = pNewCS->Port;
            cs->UID  = pNewCS->UID;
            cs->PWD  = pNewCS->PWD;
            return i;
        }
    }
    return -1;
}

int ConnSet::updSeq(CON_SET *pNewCS)
{
    CON_SET *cs;
    int pos = findElementPos(pNewCS);
    if( pos < 0 ) return 1;
    cs = m_seqCons.elementAtPosition(pos);
    cs->Host = pNewCS->Host;
    cs->Port = pNewCS->Port;
    cs->UID  = pNewCS->UID;
    cs->PWD  = pNewCS->PWD;
    cs->CSVer  = ConnSetVer;
    return 0;
}

void ConnSet::addToSeq(CON_SET* pCS)
{
    if(!isInList(pCS))
    {
        deb("ConnSet, addingToList ");
        showConnSet(pCS);
        m_seqCons.add(pCS);
    }
}

void ConnSet::getStoredCons(GString dbType, GSeq <CON_SET*> * pSeq)
{

    clearSeq(pSeq);

    CON_SET * pCS;
    for(int i = 1; i <= (int)m_seqCons.numberOfElements(); ++i)
    {
        if( m_seqCons.elementAtPosition(i)->Type != dbType) continue;

        pCS = new CON_SET;
        pCS->DB    = m_seqCons.elementAtPosition(i)->DB;
        pCS->DefDB = m_seqCons.elementAtPosition(i)->DefDB;
        pCS->Host  = m_seqCons.elementAtPosition(i)->Host;
        pCS->Port  = m_seqCons.elementAtPosition(i)->Port;
        pCS->PWD   = m_seqCons.elementAtPosition(i)->PWD;
        pCS->Type  = m_seqCons.elementAtPosition(i)->Type;
        pCS->UID   = m_seqCons.elementAtPosition(i)->UID;
        pCS->CSVer   = m_seqCons.elementAtPosition(i)->CSVer;
        if( m_seqCons.elementAtPosition(i)->Type == dbType) pSeq->add(pCS);
    }
}

int ConnSet::isInList(CON_SET *pCS)
{
    deb("connSet isInList start, elmts: "+GString(m_seqCons.numberOfElements()));
    CON_SET *cs;
    for(int i = 1; i <= (int)m_seqCons.numberOfElements(); ++i)
    {
        cs = m_seqCons.elementAtPosition(i);
        if( cs->DB == pCS->DB && cs->Type == pCS->Type &&
                cs->Host == pCS->Host && cs->UID == pCS->UID &&
                cs->PWD == pCS->PWD)
        {
            deb("connSet isInList: found elmt (1).");
            return 1;
        }
    }
    //In Seq, with different UID/PWD
    for(int i = 1; i <= (int)m_seqCons.numberOfElements(); ++i)
    {
        cs = m_seqCons.elementAtPosition(i);
        if( cs->DB == pCS->DB && cs->Type == pCS->Type )
        {
            deb("connSet isInList: found elmt (2).");
            return 2;
        }
    }
    deb("connSet isInList: elmt not found.");
    return 0;
}

void ConnSet::newEntry()
{
    CON_SET conSet;
    conSet.DefDB = 0;
    conSet.Type = "";
    conSet.DB = "";
    conSet.UID = "";
    conSet.Port = "";
    conSet.Host = "";
    conSet.PWD = "";
    createRow(&conSet);
	m_iChanged = 1;
    //m_twMain->setCurrentItem(m_twMain->itemAt(1, 1));
    m_twMain->scrollToBottom();

}

void ConnSet::delEntry()
{

	QItemSelectionModel* selectionModel = m_twMain->selectionModel();

    QModelIndexList selected = Helper::getSelectedRows(selectionModel);
	if( selected.count() == 0 )
	{
		tm("Nothing selected.\nTo select a row, click on the vertical header.");
		return;
	}
	int count = selected.count();
	for(int i = count-1; i >= 0; i--)
	{
	 	QModelIndex index = selected.at(i);
        //m_twMain->removeRow(index.row());
	}
	m_iChanged = 1;

    tm("Please note: Deleting an entry will only clear password information.\nAll accessible databases will still be shown here.");
}

CON_SET* ConnSet::getDefaultCon()
{
    for(int i = 1; i <= (int)m_seqCons.numberOfElements(); ++i)
    {
        if( m_seqCons.elementAtPosition(i)->DefDB ) return m_seqCons.elementAtPosition(i);
    }
    return NULL;
}

void ConnSet::testEntry()
{
    QItemSelectionModel* selectionModel = m_twMain->selectionModel();
    QModelIndexList selected = Helper::getSelectedRows(selectionModel);

    if( selected.count() == 0 )
    {
        tm("Nothing selected.\nTo select a row, click on the vertical header.");
        return;
    }
    QModelIndex index = selected.at(0);

    GString type = m_twMain->item(index.row(), 1)->text();
    GString db   = m_twMain->item(index.row(), 2)->text();
    GString uid  = m_twMain->item(index.row(), 3)->text();
    GString pwd  = m_twMain->item(index.row(), 4)->text();
    GString host = m_twMain->item(index.row(), 5)->text();
    GString port = m_twMain->item(index.row(), 6)->text();
    DSQLPlugin * plg = new DSQLPlugin(type);
    int erc = plg->connect(db, uid, pwd, host, port);
    if( erc )
    {

    }
}

GString ConnSet::createCfgFile()
{
	QString home = QDir::homePath ();
	if( !home.length() ) return "";
	
	QDir aDir(home);
    #if defined(MAKE_VC) || defined (__MINGW32__)
	aDir.mkdir(_CFG_DIR);

	#else
	aDir.mkdir("."+_CFG_DIR);
	#endif
	QFile cfgFile( cfgFileName() );	
	if (!cfgFile.open(QIODevice::WriteOnly | QIODevice::Text)) return "";
	return home;
}

GString ConnSet::getValue(GString key)
{
	QString home = QDir::homePath ();
	if( !home.length() ) return "";
	GFile cfgFile( cfgFileName() );
	if( !cfgFile.initOK() ) return 2;
	return cfgFile.getKey(key);
}

GString ConnSet::cfgFileName()
{
	QString home = QDir::homePath ();
	if( !home.length() ) return "";
    #if defined(MAKE_VC) || defined (__MINGW32__)
	return GString(home)+"\\"+_CFG_DIR+"\\"+_CONNSET_FILE;
	#else
	return GString(home)+"/."+_CFG_DIR + "/"+_CONNSET_FILE;
	#endif
}



int ConnSet::loadFromFile()
{
    deb("connset, loadFromFile");
    QString home = QDir::homePath ();
    if( !home.length() ) return 1;
    GFile cfgFile( cfgFileName() );
    if( !cfgFile.initOK() ) return 2;

    CON_SET * pConSet;
    GString data;
    for( int i = 1; i <=cfgFile.lines(); ++i )
    {
        pConSet = new CON_SET;
        data = cfgFile.getLine(i);


        GSeq <GString> csElmtSeq;
        if( data.occurrencesOf(_PMF_PASTE_SEP) == 0 ) csElmtSeq = data.split("@");
        else csElmtSeq = data.split(_PMF_PASTE_SEP);
        if( csElmtSeq.numberOfElements() < 7 ) continue;

        //Default: 0 or 1
        pConSet->DefDB = csElmtSeq.elementAtPosition(1).asInt();
        //Type: DB2, SqlServer,...
        pConSet->Type = csElmtSeq.elementAtPosition(2).strip();
        //DBName
        pConSet->DB = csElmtSeq.elementAtPosition(3).strip();
        //UserID
        pConSet->UID = csElmtSeq.elementAtPosition(4).strip();
        //PWD
        pConSet->PWD = csElmtSeq.elementAtPosition(5).strip();
        //Host
        pConSet->Host = csElmtSeq.elementAtPosition(6).strip();
        //Port
        pConSet->Port  = csElmtSeq.elementAtPosition(7).strip();

        if( csElmtSeq.numberOfElements() >= 8 )
        {
            pConSet->CSVer  = csElmtSeq.elementAtPosition(8).asInt();
        }
        else pConSet->CSVer = 0;

        if( pConSet->CSVer >= 2 ) pConSet->PWD = GStuff::decryptFromBase64(pConSet->PWD);
        //Activating this will show ALL db settings ever entered, even if the node is not there anymore.
        //Deactivating this will clear the list from defunct connections.
        deb("connset, loadFromFile, adding ToSeq...");
        addToSeq(pConSet);
    }
    deb("connset, loadFromFile done. elmts: "+GString(m_seqCons.numberOfElements()));
    return 0;
}

void ConnSet::deb(GString msg)
{
    if( !m_pGDeb ) return;
    m_pGDeb->debugMsg("connSet", 1, msg);
}

int ConnSet::findDatabases(GString dbType)
{

    DSQLPlugin * plg = new DSQLPlugin(dbType);

    deb("loadConnnections checking plugin for "+dbType);
    if( !plg || !plg->isOK() ) return 1;
    plg->setGDebug(m_pGDeb);

    GSeq <CON_SET*> dbSeq;
    int rc = plg->getDataBases(&dbSeq);
    if( rc )
    {
        m_strErrorMessages += "No Databases found for Type "+dbType+", sqlCode: "+GString(rc)+"\n";
        return rc;
    }

    deb("loadConnnections getDatabases: "+GString(dbSeq.numberOfElements()));
    //In the plugin, CON_SETs are created with 'new.
    //These elements are zombified when the plugin closes. So we need to copy them.
    //Simply saving the pointers to the created CON_SETs does not work under Windows!
    CON_SET * pCS;

    //Copy the CON_SETs
    for( int i = 1; i <= (int) dbSeq.numberOfElements(); ++i)
    {
        if( !dbSeq.elementAtPosition(i)->DB.length() ) continue;
        pCS = new CON_SET;
        pCS->DB    = dbSeq.elementAtPosition(i)->DB;
        pCS->DefDB = 0; //dbSeq.elementAtPosition(i)->DefDB;
        pCS->Host  = dbSeq.elementAtPosition(i)->Host;
        pCS->Port  = dbSeq.elementAtPosition(i)->Port;
        pCS->PWD   = dbSeq.elementAtPosition(i)->PWD;
        pCS->Type  = dbSeq.elementAtPosition(i)->Type;
        pCS->UID   = dbSeq.elementAtPosition(i)->UID;       
        pCS->CSVer = ConnSetVer;
        //See if connection information (UID, PWD,...) was stored
        setConnectionSettingsFromFile(pCS);

        deb("adding db "+pCS->DB);
        addToSeq(pCS);
    }
    deb("dbList elmts: "+GString(m_seqCons.numberOfElements()));
    //We have to clean this manually
    while( !dbSeq.isEmpty() )
    {
        pCS = dbSeq.firstElement();
        delete pCS;
        pCS  = NULL;
        dbSeq.removeFirst();
    }

    delete plg;
    return 0;
}

int ConnSet::createRow(CON_SET *pConSet)
{
    QTableWidgetItem * pItem;

    QCheckBox * pChkBx   = new QCheckBox(this);
    QLineEdit * pPWDLE   = new QLineEdit(this);

    int rows = m_twMain->rowCount();
    m_twMain->insertRow(rows);


    //Type: SqlSrv, DB2,...
    pItem = new QTableWidgetItem((char*)pConSet->Type);
    m_twMain->setItem(rows, 0, pItem);

    //Database
    pItem = new QTableWidgetItem((char*)pConSet->DB);
    m_twMain->setItem(rows, 1, pItem);

    //UserID
    pItem = new QTableWidgetItem((char*)pConSet->UID);
    m_twMain->setItem(rows, 2, pItem);

    //PWD
    pPWDLE->setText((char*)pConSet->PWD);
    pPWDLE->setEchoMode(QLineEdit::Password);
    m_twMain->setCellWidget(rows, 3, pPWDLE);

    GString host = pConSet->Host;
    if( !host.length() ) host = _HOST_DEFAULT;

    GString port = pConSet->Port;
    if( !port.length() ) port = _PORT_DEFAULT;

    pItem = new QTableWidgetItem((char*)host);
    m_twMain->setItem(rows, 4, pItem);

    pItem = new QTableWidgetItem((char*)port);
    m_twMain->setItem(rows, 5, pItem);

    //DefaultCheckBox
    m_twMain->setCellWidget(rows, 6, pChkBx); //Default

    if( pConSet->DefDB ) pChkBx->setCheckState(Qt::Checked);
    connect(pChkBx, SIGNAL(stateChanged(int)), SLOT(defaultSelected(int)));

/*
    GSeq <GString> list;
    DSQLPlugin::DBTypeNames(&list);
    for(unsigned long i = 1; i <= list.numberOfElements(); ++i)
    {
        pTypeCB->addItem(list.elementAtPosition(i));
        if( pConSet->Type == list.elementAtPosition(i)) pTypeCB->setCurrentIndex(i-1);
    }

    //Currently,, I have no idea how to fetch database instances for non-DB2 databases.
    if( m_pIDSQL->getDBType() == IDSQL::DB2)
    {
        GSeq <CON_SET*> aSeq;
        m_pIDSQL->getDataBases(&aSeq);
        for(unsigned long i = 1; i <= aSeq.numberOfElements(); ++i)
        {
            pDBCB->addItem(aSeq.elementAtPosition(i)->DB);
            if( pConSet->DB == aSeq.elementAtPosition(i)->DB) pDBCB->setCurrentIndex(i-1);
        }
    }
    else if( pConSet->DB.length()) pDBCB->addItem(pConSet->DB);
    pDBCB->setEditable(true);
    pTypeCB->adjustSize();
    connect(pTypeCB, SIGNAL(activated(int)), SLOT(getAllDatabases(int)));

*/

    m_twMain->setRowHeight(rows, QFontMetrics( m_twMain->font()).height()+5);
    return 0;
}


void ConnSet::defaultSelected(int)
{
	//This object was clicked:
	QCheckBox * pChkBx = qobject_cast<QCheckBox *>(sender());	
	//if already unchecked, do nothing.
	//the previously checked sender is already unchecked when this function is called.
	if( pChkBx->checkState() == Qt::Unchecked ) return;

	//uncheck all
	this->blockSignals(true);
	QCheckBox * pUnChkBx;
	for(int i = 0; i < m_twMain->rowCount(); ++i )
	{
        pUnChkBx = qobject_cast<QCheckBox*>(m_twMain->cellWidget(i, 6));
		if( pChkBx != pUnChkBx ) pUnChkBx->setCheckState(Qt::Unchecked);
	}
	//Now set sender checked
	pChkBx->setCheckState(Qt::Checked);
	this->blockSignals(false);
}

int ConnSet::setConnectionSettingsFromFile(CON_SET *pConSet)
{
    QString home = QDir::homePath ();
    if( !home.length() ) return 1;

    GFile cfgFile( cfgFileName() );
    if( !cfgFile.initOK() ) return 2;

    GString data;
    for( int i = 1; i <=cfgFile.lines(); ++i )
    {
        data = cfgFile.getLine(i);
        GSeq <GString> csElmtSeq = data.split(_PMF_PASTE_SEP);

        if( csElmtSeq.numberOfElements() < 7 ) continue;

        //wrong database: DBName or Type different
        if( pConSet->DB != csElmtSeq.elementAtPosition(3).strip() || pConSet->Type != csElmtSeq.elementAtPosition(2).strip() ) continue;

        //Default: 0 or 1
        pConSet->DefDB = csElmtSeq.elementAtPosition(1).strip().asInt();
        //DBName
        pConSet->DB = csElmtSeq.elementAtPosition(3).strip();
        //Type: DB2, SqlServer,...
        pConSet->Type = csElmtSeq.elementAtPosition(2).strip();
        //UserID
        pConSet->UID = csElmtSeq.elementAtPosition(4).strip();
        //PWD
        pConSet->PWD = csElmtSeq.elementAtPosition(5).strip();
        //Host
        pConSet->Host = csElmtSeq.elementAtPosition(6).strip();
        //Port
        pConSet->Port  = csElmtSeq.elementAtPosition(7);
        if( csElmtSeq.numberOfElements() >= 8 )
        {
            pConSet->CSVer = csElmtSeq.elementAtPosition(8);
        }
        else pConSet->CSVer = 0;
    }
    return 0;
}

void ConnSet::getAllDatabases(int pos)
{
    PMF_UNUSED(pos);

    //Neat: Multiple ComboBoxes can signal this slot. QObject::sender()
    //will give us the sender. Heed the warnings, though.
    //In this case we're fine.
    QComboBox *pCB = (QComboBox*)QObject::sender();
}

void ConnSet::migrateData()
{
    for(int i = 1; i <= (int)m_seqCons.numberOfElements(); ++ i)
    {
        if( m_seqCons.elementAtPosition(i)->CSVer < 3)
        {
            this->save();
            break;
        }
    }
}

void ConnSet::sortClicked(int)
{
    Helper::setVHeader(m_twMain);
}



void ConnSet::showConnSet(CON_SET *pCS)
{
    deb("-- ConnSet: ");
    deb("   DB:   "+pCS->DB);
    deb("   Type: "+pCS->Type);
    deb("   UID:  "+pCS->UID);
    deb("   Ver:  "+pCS->CSVer);
    deb("   Host: "+pCS->Host);
    deb("   Port: "+pCS->Port);
}
