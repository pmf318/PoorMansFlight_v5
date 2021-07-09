//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 
//
/*********************************************************************
*********************************************************************/

#ifndef _DSQLPLUGIN_
#include "dsqlplugin.hpp"
#endif
#include <iostream>
#include <ctype.h>

#ifdef MAKE_VC
    #define _WIN32_WINNT 0x0501
    #include <windows.h>
    #include <io.h>
#elif 	__MINGW32__
#include <windows.h>
#include <io.h>
#else
    #include <unistd.h>
#endif

#include <gseq.hpp>
#include <gstuff.hpp>

static int m_dsqlPluginCounter = 0;

/*********************************************************************
*********************************************************************/
//Declared extern in header
GString m_strDSQLPluginPath;

DSQLPlugin::DSQLPlugin(DSQLPlugin const &plg): m_pIDSQL(plg.m_pIDSQL->clone())
{
    deb("DSQLPlugin::CopyCtor called");
    crt = plg.crt;
    destr = plg.destr;
    m_dsqlPluginCounter++;
    m_iMyInstanceCounter = m_dsqlPluginCounter;
    m_iPluginLoaded = 1;
    dsqlHandle = plg.dsqlHandle;
    m_iLoadError = 0;
    m_pGDeb = plg.m_pGDeb;
    deb("DSQLPlugin::CopyCtor done");
    //Plugin must be loaded already
    //callLoader(plg.m_strDBTypeName);

}
/*
{
    callLoader(plg.m_strDBTypeName);
    if( crtFrom ) m_pIDSQL = crtFrom(plg);

}
*/
DSQLPlugin* DSQLPlugin::clone() const
{
    //printf("Clone called\n");
    return new DSQLPlugin(*this);
}

DSQLPlugin& DSQLPlugin::operator= (DSQLPlugin const& in)
 {
    deb("op = called");
    if (this != &in) // Check for self-assignment
    {
        IDSQL* p2 = in.m_pIDSQL->clone();   // Create the new one FIRST...
        delete m_pIDSQL;                   // ...THEN delete the old one
        m_pIDSQL = p2;
    }
    return *this;
}
DSQLPlugin::DSQLPlugin(GString dbName, GString file )
{
    //setPathEnvironment();
    m_dsqlPluginCounter++;
    m_iMyInstanceCounter = m_dsqlPluginCounter;
    deb("CTor");

    if( !m_strDSQLPluginPath.length() ) m_strDSQLPluginPath = "./plugins/";
    deb("loadPath is "+m_strDSQLPluginPath);

    m_pIDSQL = NULL;
    m_iPluginLoaded = 0;
    m_strDBTypeName = dbName;
    dsqlHandle = 0;    

    m_iLoadError = callLoader(dbName, file);
    if ( m_iLoadError ) return;
    if( crt ) m_pIDSQL = crt();
    if( m_pIDSQL )
	{
		m_iPluginLoaded = 1;
		deb("CTor, plugin loaded, m_pIDSQL valid.");
	}
	else deb("CTor, plugin NOT loaded");
}
DSQLPlugin::~DSQLPlugin()
{
    deb("DTor start, count plugins: "+GString(m_dsqlPluginCounter));
	
    if( m_pIDSQL )
	{
		deb("DTor calling destroy on m_pIDSQL");
        if( destr && m_iPluginLoaded )
        {
            destr(m_pIDSQL);
        }
        else deb("DTor / destr missing");
        deb("DTor deleted m_pIDSQL");
	}
    //unload lib on last instance
    if( dsqlHandle && m_dsqlPluginCounter <= 1 )
    {
        deb("DTor closing libHandle...");

        #ifdef MAKE_VC
        if( dsqlHandle) FreeLibrary(dsqlHandle);
        #elif __MINGW32__
        if( dsqlHandle)
        {
            FreeLibrary((HMODULE)dsqlHandle);
        }
        #else
        if( dsqlHandle) dlclose(dsqlHandle);
        #endif
        dsqlHandle = 0;
        deb("DTor closing libHandle...OK");
    }
    m_pIDSQL = NULL;
    m_iPluginLoaded = 0;
	deb("DTor OK");
    m_dsqlPluginCounter--;
}


int DSQLPlugin::setPathEnvironment()
{
#ifdef MAKE_VC
    char pathVal[65000];
    GetEnvironmentVariable("PATH", pathVal,65000);
    GSeq<GString> pathSeq = GString(pathVal).split(';');
    for(int i = 1; i <= (int)pathSeq.numberOfElements(); ++i )
    {
        GString path = pathSeq.elementAtPosition(i);
        if( path.upperCase().occurrencesOf("\\SQLLIB\\BIN"))
        {
            SetDllDirectory(pathSeq.elementAtPosition(i));
            printf("Setting path: %s\n", (char*) path);
            return 0;
        }
    }
#endif
    return 0;
}

int DSQLPlugin::callLoader(GString dbName, GString file )
{
    deb("callLoader, db: "+dbName+", lib: "+file+", defPath: "+m_strDSQLPluginPath);
    if( file.length() )
    {
        return loadPlugin(file);
    }

    int erc = 1;
    if( dbName == _DB2)
    {
        #if defined(MAKE_VC) || defined (__MINGW32__)
        erc = loadPlugin(m_strDSQLPluginPath+"db2dsql.dll");
        #else        
        erc = loadPlugin(m_strDSQLPluginPath+"libdb2dsql.so");
        #endif
    }
    else if( dbName == _SQLSRV || dbName == "ODBC")
    {
        #if defined(MAKE_VC) || defined (__MINGW32__)
        erc = loadPlugin(m_strDSQLPluginPath+"odbcdsql.dll");
        #else
        erc = loadPlugin(m_strDSQLPluginPath+"libodbcdsql.so");
        #endif
    }
    else if( dbName == _DB2ODBC )
    {
        #if defined(MAKE_VC) || defined (__MINGW32__)
        erc = loadPlugin(m_strDSQLPluginPath+"db2dcli.dll");
        #else
        erc = loadPlugin(m_strDSQLPluginPath+"libdb2dcli.so");
        #endif
    }
    else if( dbName == _MARIADB )
    {
        #if defined(MAKE_VC) || defined (__MINGW32__)
        erc = loadPlugin(m_strDSQLPluginPath+"mariadb.dll");
        #else
        erc = loadPlugin(m_strDSQLPluginPath+"libmariadb.so");
        #endif
    }
    else if( dbName == _POSTGRES )
    {
        #if defined(MAKE_VC) || defined (__MINGW32__)
        erc = loadPlugin(m_strDSQLPluginPath+"postgres.dll");
        #else
        erc = loadPlugin(m_strDSQLPluginPath+"libpostgres.so");
        #endif
    }

    /*
    else if( dbName == _ORAODBC )
    {
        #ifndef MAKE_VC
        erc = loadPlugin(m_strDSQLPluginPath+"liboradsql.so");
        #else
        erc = loadPlugin(m_strDSQLPluginPath+"db2dcli.dll");
        #endif
    }
    */
    return erc;

}
int DSQLPlugin::loadPlugin(GString libPath)
{
    deb("Trying to load "+libPath);

    if( access(libPath, 0) < 0 ) return PluginMissing;

    deb("Trying to load "+libPath+", file exists.");

    #if defined(MAKE_VC) || defined (__MINGW32__)
            DWORD prevErrorMode = 0;
            #if (_WIN32_WINNT >= 0x0600) && !(__GNUC__)
                SetThreadErrorMode(SEM_FAILCRITICALERRORS, &prevErrorMode);
            #else
                prevErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
            #endif
            //SetDllDirectory("c:\\Program Files\\IBM\\SQLLIB\\BIN");
            //dsqlHandle = LoadLibraryEx(libPath,NULL,LOAD_LIBRARY_SEARCH_USER_DIRS);
            dsqlHandle = LoadLibrary( libPath );
            #if (_WIN32_WINNT >= 0x0600) && !(__GNUC__)
                SetThreadErrorMode(prevErrorMode, NULL);
            #else
                SetErrorMode(prevErrorMode);
            #endif
    #else
            deb("Trying dlopen...");
            dsqlHandle = dlopen(libPath, RTLD_NOW);
            deb("Trying dlopen...OK");

    #endif

    deb("checking dsqlHandle...");
    if(!dsqlHandle)
    {
        deb("dlopen/LoadLibrary: Could not load dsqlPlugin in "+libPath);
        return PluginNotLoadable;
    }
    deb("dsqlHandle ok");
    #if defined(MAKE_VC) || defined (__MINGW32__)
        crt = (CREATE) GetProcAddress((HMODULE) dsqlHandle, "create" );
        //crtFrom = (CREATEFROM) GetProcAddress( dsqlHandle, "createFrom" );
        destr = (DESTROY) GetProcAddress( (HMODULE)dsqlHandle, "destroy" );
    #else
        crt=(create_t*)dlsym(dsqlHandle,"create");
        //crtFrom=(createFrom_t*)dlsym(dsqlHandle,"createFrom");
        destr =(destroy_t*)dlsym(dsqlHandle,"destroy");
    #endif

    if( !crt || !destr )
    {
        deb("Plugin not valid: create and/or destroy missing.");
    #if defined(MAKE_VC) || defined(__MINGW32__)
        FreeLibrary((HMODULE)dsqlHandle);
    #else
        dlclose(dsqlHandle);
    #endif
        return PluginNotLoadable;
    }
    deb("Plugin loaded OK.");
    return PluginLoaded;

}
GString DSQLPlugin::pluginPath()
{
    return m_strDSQLPluginPath;
}

void DSQLPlugin::setPluginPath(GString path)
{    
    m_strDSQLPluginPath = path.stripTrailing("/")+"/";
}

int DSQLPlugin::isOK()
{
    return m_iPluginLoaded;
}

void DSQLPlugin::DBTypeNames(GSeq <GString>* list)
{
    list->add(_DB2);
    list->add(_SQLSRV);
    list->add(_DB2ODBC);
    list->add(_MARIADB);
    list->add(_POSTGRES);
//    list->add(_ORAODBC);

}
void DSQLPlugin::init()
{
}
int DSQLPlugin::loadError()
{
    return m_iLoadError;
}

ODBCDB DSQLPlugin::getDBType()
{
    return m_pIDSQL->getDBType();
}
GString DSQLPlugin::getDBTypeName()
{
    return m_pIDSQL->getDBTypeName();
}
GString DSQLPlugin::connect(GString db, GString uid, GString pwd, GString host, GString port)
{
    return m_pIDSQL->connect(db, uid, pwd, host, port);
}
int DSQLPlugin::disconnect()
{
    return m_pIDSQL->disconnect();
}
GString DSQLPlugin::initAll(GString message, unsigned long maxRows, int getLen)
{
    return m_pIDSQL->initAll(message, maxRows, getLen);
}
int DSQLPlugin::commit()
{
    return m_pIDSQL->commit();
}
int DSQLPlugin::rollback()
{
    return m_pIDSQL->rollback();
}
unsigned int DSQLPlugin::numberOfColumns()
{
    return m_pIDSQL->numberOfColumns();
}
unsigned long DSQLPlugin::numberOfRows()
{
    return m_pIDSQL->numberOfRows();
}
GString DSQLPlugin::rowElement(unsigned long row, int col)
{
    return m_pIDSQL->rowElement(row, col);
}
GString DSQLPlugin::rowElement(unsigned long row, GString hostVar)
{
    return m_pIDSQL->rowElement(row, hostVar);
}
GString DSQLPlugin::hostVariable(int col)
{
    return m_pIDSQL->hostVariable(col);
}

int DSQLPlugin::positionOfHostVar(const GString& hostVar)
{
    return m_pIDSQL->positionOfHostVar(hostVar);
}

///virtual	GString currentCursor(GString filter, GString command, long curPos, short commitIt)

int DSQLPlugin::sqlCode()
{
    return m_pIDSQL->sqlCode();
}
GString DSQLPlugin::sqlError()
{
    return m_pIDSQL->sqlError();
}
////virtual	int uploadBlob(GString cmd, GSeq <GString> *fileSeq, GSeq <GString> * lobTypeSeq)
int DSQLPlugin::getTabSchema()
{
    return m_pIDSQL->getTabSchema();
}

int DSQLPlugin::getTables(GString schema)
{
    return m_pIDSQL->getTables(schema);
}

void DSQLPlugin::convToSQL(GString & input)
{
    return m_pIDSQL->convToSQL(input);
}


int DSQLPlugin::getAllTables(GSeq <GString > * tabSeq, GString filter)
{
    return m_pIDSQL->getAllTables(tabSeq, filter);
}

short DSQLPlugin::sqlType(const short & col)
{
    return m_pIDSQL->sqlType(col);
}

short DSQLPlugin::sqlType(const GString & colName)
{
    return m_pIDSQL->sqlType(colName);
}

GString DSQLPlugin::realName(const short & sqlType)
{
    return m_pIDSQL->realName(sqlType);
}

int DSQLPlugin::initRowCrs()
{
    return m_pIDSQL->initRowCrs();
}

int DSQLPlugin::nextRowCrs()
{
    return m_pIDSQL->nextRowCrs();
}

GString DSQLPlugin::dataAtCrs(int col)
{
    return m_pIDSQL->dataAtCrs(col);
}


signed long DSQLPlugin::getCost()
{
    return m_pIDSQL->getCost();
}

//// GString descriptorToFile( GString cmd, GString &blobFile )
void DSQLPlugin::setStopThread(int stop)
{
    return m_pIDSQL->setStopThread(stop);
}

void DSQLPlugin::setDBType(ODBCDB dbType)
{
    return m_pIDSQL->setDBType(dbType);
}
int DSQLPlugin::execByDescriptor( GString cmd, GSeq <GString> *dataSeq)
{
    return m_pIDSQL->execByDescriptor(cmd, dataSeq);
}
int DSQLPlugin::execByDescriptor( GString cmd, GSeq <GString> *dataSeq, GSeq <int> *typeSeq,
                                GSeq <short>* sqlVarLengthSeq, GSeq <int> *forBitSeq )
{
    return m_pIDSQL->execByDescriptor(cmd, dataSeq, typeSeq, sqlVarLengthSeq, forBitSeq);
}


int DSQLPlugin::isNumType(int pos)
{
    return m_pIDSQL->isNumType(pos);
}
int DSQLPlugin::isDateTime(int pos)
{
    return m_pIDSQL->isDateTime(pos);
}

int DSQLPlugin::hasForBitData()
{
    return m_pIDSQL->hasForBitData();
}

int DSQLPlugin::isForBitCol(int i)
{
    return m_pIDSQL->isForBitCol(i);
}
int DSQLPlugin::isBitCol(int i)
{
    return m_pIDSQL->isBitCol(i);
}

int DSQLPlugin::isXMLCol(int i)
{
    return m_pIDSQL->isXMLCol(i);
}
int DSQLPlugin::simpleColType(int i)
{
    return m_pIDSQL->simpleColType(i);
}

int DSQLPlugin::isLOBCol(int i)
{
    return m_pIDSQL->isLOBCol(i);
}

int DSQLPlugin::isNullable(int i)
{
    return m_pIDSQL->isNullable(i);
}

int DSQLPlugin::isFixedChar(int i)
{
    return m_pIDSQL->isFixedChar(i);
}

int DSQLPlugin::isLongTypeCol(int i)
{
    return m_pIDSQL->isLongTypeCol(i);
}

void DSQLPlugin::setCharForBit(int val)
{
    return m_pIDSQL->setCharForBit(val);
}

GString DSQLPlugin::descriptorToFile( GString cmd, GString &blobFile, int * outSize )
{
    return m_pIDSQL->descriptorToFile(cmd, blobFile, outSize);
}

void DSQLPlugin::setReadUncommitted(short readUncommitted)
{
    m_pIDSQL->setReadUncommitted(readUncommitted);
}

short DSQLPlugin::bindIt(GString bndFile, GString database, GString user, GString pwd, GString msgFile)
{
    return m_pIDSQL->bindIt(bndFile, database, user, pwd, msgFile);
}


GString DSQLPlugin::currentCursor(GString filter, GString command, long curPos, short commitIt,
                               GSeq <GString> *fileList, GSeq <long> *lobType)
{
    return m_pIDSQL->currentCursor(filter, command, curPos, commitIt, fileList, lobType);
}

long DSQLPlugin::uploadBlob(GString cmd, GSeq <GString> *fileList, GSeq <long> *lobType)
{
    return m_pIDSQL->uploadBlob(cmd, fileList, lobType);
}

long DSQLPlugin::uploadBlob(GString cmd, char * buffer, long size)
{
    return m_pIDSQL->uploadBlob(cmd, buffer, size);
}

long DSQLPlugin::dataLen(const short & pos)
{
    return m_pIDSQL->dataLen(pos);
}

int  DSQLPlugin::getDataBases(GSeq <CON_SET*> *dbList)
{
    return m_pIDSQL->getDataBases(dbList);
}


//Pack this into helper libs...
int  DSQLPlugin::deleteTable(GString tableName)
{
    return m_pIDSQL->deleteTable(tableName);
}

long DSQLPlugin::retrieveBlob( GString cmd, GString &blobFile, int writeFile )
{
    return m_pIDSQL->retrieveBlob(cmd, blobFile, writeFile);
}


#ifdef  QT4_DSQL
GString DSQLPlugin::getIdenticals(GString table, QWidget* parent, QListWidget *pLB, short autoDel)
{
    return m_pIDSQL->getIdenticals(table, parent, pLB, autoDel);
}
#endif

GString DSQLPlugin::fillChecksView(GString table, int showRaw)
{
    return m_pIDSQL->fillChecksView(table, showRaw);
}

int DSQLPlugin::forceApp(int appID)
{
    return m_pIDSQL->forceApp(appID);
}
void DSQLPlugin::setCLOBReader(short readCLOBData )
{
    return m_pIDSQL->setCLOBReader(readCLOBData);
}


int DSQLPlugin::getSysTables()
{
    return m_pIDSQL->getSysTables();
}

void DSQLPlugin::setCurrentDatabase(GString db)
{
    return m_pIDSQL->setCurrentDatabase(db);
}

GString DSQLPlugin::currentDatabase()
{
    return m_pIDSQL->currentDatabase();
}
unsigned long DSQLPlugin::getResultCount(GString cmd)
{
    return m_pIDSQL->getResultCount(cmd);
}
int DSQLPlugin::getColSpecs(GString table, GSeq<COL_SPEC*> *specSeq)
{
    return m_pIDSQL->getColSpecs(table, specSeq);
}

int DSQLPlugin::getTriggers(GString table, GString *text)
{
    return m_pIDSQL->getTriggers(table, text);
}

GSeq <GString> DSQLPlugin::getTriggerSeq(GString table)
{
    return m_pIDSQL->getTriggerSeq(table);
}

GString DSQLPlugin::getChecks(GString table, GString filter)
{
    return m_pIDSQL->getChecks(table, filter);
}

GSeq <GString> DSQLPlugin::getChecksSeq(GString table, GString filter)
{
    return m_pIDSQL->getChecksSeq(table, filter);
}

GSeq <IDX_INFO*> DSQLPlugin::getIndexeInfo(GString table)
{
    return m_pIDSQL->getIndexeInfo(table);
}


int DSQLPlugin::getUniqueCols(GString table, GSeq <GString> * colSeq)
{
    return m_pIDSQL->getUniqueCols(table, colSeq);
}

void DSQLPlugin::createXMLCastString(GString &xmlData)
{
	m_pIDSQL->createXMLCastString(xmlData);
}
int DSQLPlugin::isBinary(unsigned long row, int col)
{
    return m_pIDSQL->isBinary(row, col);
}
int DSQLPlugin::isTruncated(unsigned long row, int col)
{
    return m_pIDSQL->isTruncated(row, col);
}
void DSQLPlugin::setTruncationLimit(int limit)
{
    m_pIDSQL->setTruncationLimit(limit);
}
int DSQLPlugin::uploadLongBuffer( GString cmd, GString data, int isBinary )
{
    return m_pIDSQL->uploadLongBuffer(cmd, data, isBinary);
}
GString DSQLPlugin::cleanString(GString in)
{
    return m_pIDSQL->cleanString(in);
}
void DSQLPlugin::getResultAsHEX(int asHex)
{
    m_pIDSQL->getResultAsHEX(asHex);
}
void DSQLPlugin::setGDebug(GDebug *pGDB)
{
    m_pIDSQL->setGDebug(pGDB);
}
int DSQLPlugin::exportAsTxt(int mode, GString sqlCmd, GString table, GString outFile, GSeq <GString>* startText, GSeq <GString>* endText, GString *err)
{
    return m_pIDSQL->exportAsTxt(mode, sqlCmd, table, outFile, startText, endText, err);
}
int DSQLPlugin::getRowData(int row, int col, GString * data)
{
    return m_pIDSQL->getRowData(row, col, data);
}
unsigned long DSQLPlugin::getRowDataCount()
{
    return m_pIDSQL->getRowDataCount();
}

int DSQLPlugin::getHeaderData(int pos, GString * data)
{
    return m_pIDSQL->getHeaderData(pos, data);
}
unsigned long DSQLPlugin::getHeaderDataCount()
{
    return m_pIDSQL->getHeaderDataCount();
}
int DSQLPlugin::isTransaction()
{
    return m_pIDSQL->isTransaction();
}
void DSQLPlugin::setDatabaseContext(GString context)
{
    m_pIDSQL->setDatabaseContext(context);
}
int DSQLPlugin::hasUniqueConstraint(GString tableName)
{
    return m_pIDSQL->hasUniqueConstraint(tableName);
}
GString DSQLPlugin::getDdlForView(GString tableName)
{
    return m_pIDSQL->getDdlForView(tableName);
}

void DSQLPlugin::setAutoCommmit(int commit)
{
    return m_pIDSQL->setAutoCommmit(commit);
}
void DSQLPlugin::currentConnectionValues(CON_SET * conSet)
{
    m_pIDSQL->currentConnectionValues(conSet);
}
GString DSQLPlugin::lastSqlSelectCommand()
{
    return m_pIDSQL->lastSqlSelectCommand();
}

TABLE_PROPS DSQLPlugin::getTableProps(GString tableName)
{
    return m_pIDSQL->getTableProps(tableName);
}

void DSQLPlugin::deb(GString msg)
{
    GDebug::debMsg("DSQLPlugin", m_dsqlPluginCounter, msg);
}

int DSQLPlugin::tableIsEmpty(GString tableName)
{
    return m_pIDSQL->tableIsEmpty(tableName);
}

int DSQLPlugin::deleteViaFetch(GString tableName, GSeq<GString> * colSeq, int rowCount, GString whereClause)
{
    return m_pIDSQL->deleteViaFetch(tableName, colSeq, rowCount, whereClause);
}
