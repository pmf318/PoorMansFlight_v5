//
//  This e is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 
//
/*********************************************************************
*********************************************************************/

#ifndef _DBAPIPLUGIN_
#include "dbapiplugin.hpp"
#endif

#include <iostream>
#include <ctype.h>

#ifdef MAKE_VC
#elif __MINGW32__
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include <gstuff.hpp>


static int m_dbapiPluginCounter = 0;

//Declared extern in header
GString m_strDBAPIPluginPath;
/*********************************************************************
*********************************************************************/

DBAPIPlugin::DBAPIPlugin(DBAPIPlugin const & plg): m_pIDBAPI(plg.m_pIDBAPI->clone())
{
    crt = plg.crt;
    destr = plg.destr;
    m_dbapiPluginCounter++;
    m_iMyInstanceCounter = m_dbapiPluginCounter;
    m_iPluginLoaded = 1;
    dbapiHandle = plg.dbapiHandle;
    deb("CopyCtor called");
    //Plugin must be loaded already
    //callLoader(plg.m_strDBTypeName);

}
DBAPIPlugin* DBAPIPlugin::clone() const
{
    //printf("Clone called\n");
    return new DBAPIPlugin(*this);
}

DBAPIPlugin& DBAPIPlugin::operator= (DBAPIPlugin const& in)
  {
    deb("op = called");
    if (this != &in) // Check for self-assignment
    {
        IDBAPI* p2 = in.m_pIDBAPI->clone();   // Create the new one FIRST...
        delete m_pIDBAPI;                   // ...THEN delete the old one
        m_pIDBAPI = p2;
    }
    return *this;
}
DBAPIPlugin::DBAPIPlugin(GString dbName, GString file )
{
    m_dbapiPluginCounter++;
    m_iMyInstanceCounter = m_dbapiPluginCounter;
    deb("CTor");
    if( !m_strDBAPIPluginPath.length() ) m_strDBAPIPluginPath = "./plugins/";

    m_pIDBAPI = NULL;
    m_iPluginLoaded = 0;
    m_strDBTypeName = dbName;
    dbapiHandle = 0;

    if (callLoader(dbName, file) ) return;
    if( crt ) m_pIDBAPI = crt();
    if( m_pIDBAPI )
    {
        m_iPluginLoaded = 1;
        deb("CTor, plugin loaded, m_pIDSQL valid.");
    }
    else deb("CTor, plugin NOT loaded");
}
DBAPIPlugin::~DBAPIPlugin()
{
    deb("DTor start, count plugins: "+GString(m_dbapiPluginCounter));

    if( m_pIDBAPI )
    {
        deb("DTor calling destroy on m_pIDSQL");
        if( destr && m_iPluginLoaded )
        {
            destr(m_pIDBAPI);
        }
        else deb("DTor / destr missing");
        deb("DTor deleted m_pIDSQL");
    }
    //unload lib on last instance
    if( dbapiHandle && m_dbapiPluginCounter <= 1 )
    {
        deb("DTor closing libHandle...");

        #if defined(MAKE_VC) || defined (__MINGW32__)
        if( dbapiHandle) FreeLibrary((HMODULE)dbapiHandle);
        #else
        if( dbapiHandle) dlclose(dbapiHandle);
        #endif
        dbapiHandle = 0;
        deb("DTor closing libHandle...OK");
    }
    m_pIDBAPI = NULL;
    m_iPluginLoaded = 0;
    deb("DTor OK, m_dbapiPluginCounter: "+GString(m_dbapiPluginCounter));
    m_dbapiPluginCounter--;
}

int DBAPIPlugin::isOK()
{
    return m_iPluginLoaded;
}

int DBAPIPlugin::callLoader(GString dbName, GString file )
{
    if( file.length() )
    {
        return loadPlugin(file);
    }
    int erc = 1;
    if( dbName == _DB2)
    {
        #if defined(MAKE_VC) || defined (__MINGW32__)
        erc = loadPlugin(m_strDBAPIPluginPath+"db2dapi.dll");
        #else
        erc = loadPlugin(m_strDBAPIPluginPath+"libdb2dapi.so");
        #endif
    }
    else if( dbName == _SQLSRV || dbName == "ODBC")
    {
        erc = 1; //Not implemented.
    }
    return erc;

}
int DBAPIPlugin::loadPlugin(GString libPath)
{
    deb("Trying to load "+libPath);
    #if defined(MAKE_VC) || defined(__MINGW32__)
        DWORD prevErrorMode = 0;
        #if (_WIN32_WINNT >= 0x0600) && !(__GNUC__)
            SetThreadErrorMode(SEM_FAILCRITICALERRORS, &prevErrorMode);
        #else
            prevErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
        #endif
        //dsqlHandle = LoadLibraryExW(libPath,NULL,0);
        dbapiHandle = LoadLibrary( libPath );
        #if (_WIN32_WINNT >= 0x0600) && !(__GNUC__)
            SetThreadErrorMode(prevErrorMode, NULL);
        #else
            SetErrorMode(prevErrorMode);
        #endif
    #else
        deb("Trying dlopen...");
        dbapiHandle = dlopen(libPath, RTLD_NOW);
        deb("Trying dlopen...OK");
        #endif
    deb("checking dbapiHandle...");
    if(!dbapiHandle)
    {
        deb("dlopen/LoadLibrary: Could not load dsqlPlugin in "+libPath);
        return 1;
    }
    deb("dsqlHandle ok");
    #if defined(MAKE_VC) || defined (__MINGW32__)
        crt = (CREATE) GetProcAddress( (HMODULE)dbapiHandle, "create" );;
        destr = (DESTROY) GetProcAddress( (HMODULE)dbapiHandle, "destroy" );
    #else
        crt=(create_t*)dlsym(dbapiHandle,"create");
        destr =(destroy_t*)dlsym(dbapiHandle,"destroy");
    #endif

    if( !crt || !destr )
    {
        deb("Plugin not valid: create and/or destroy missing.");
    #if defined(MAKE_VC) || defined(__MINGW32__)
        FreeLibrary((HMODULE)dbapiHandle);
    #else
        dlclose(dbapiHandle);
    #endif
        return 1;
    }
    deb("Plugin loaded OK.");
    return 0;

}
GString DBAPIPlugin::pluginPath()
{
    return m_strDBAPIPluginPath;
}

void DBAPIPlugin::setPluginPath(GString path)
{
    m_strDBAPIPluginPath = path.stripTrailing("/")+"/";
}

GString DBAPIPlugin::SQLError()
{
    return m_pIDBAPI->SQLError();
}	

int DBAPIPlugin::getSnapshotData(int type)
{    
    return m_pIDBAPI->getSnapshotData(type);
}	
int DBAPIPlugin::initMonitor(GString database, GString node, GString user, GString pwd)
{
    return m_pIDBAPI->initMonitor(database, node, user, pwd);
}

int DBAPIPlugin::startMonitor()
{
    return m_pIDBAPI->startMonitor();
}	

int DBAPIPlugin::stopMonitor()
{
    return m_pIDBAPI->stopMonitor();
}

int DBAPIPlugin::resetMonitor()
{
    return m_pIDBAPI->resetMonitor();
}	

int DBAPIPlugin::initTabSpaceLV(GSeq <TAB_SPACE*> *dataSeq)
{
    return m_pIDBAPI->initTabSpaceLV(dataSeq);
}

int DBAPIPlugin::importTable(GString dataFile, GSeq<GString> *pathSeq, GString format, GString statement, GString msgFile, GString modifierString)
{
    return m_pIDBAPI->importTable(dataFile, pathSeq, format, statement, msgFile, modifierString);
}

int DBAPIPlugin::importTableNew(GString dataFile, GSeq<GString> *pathSeq, GString format, GString statement, GString msgFile, GString modifierString)
{
    return m_pIDBAPI->importTableNew(dataFile, pathSeq, format, statement, msgFile, modifierString);
}

int DBAPIPlugin::exportTable(GString dataFile, GString format, GString statement, GString msgFile, GString modified)
{
    return m_pIDBAPI->exportTable(dataFile, format, statement, msgFile, modified);
}

int DBAPIPlugin::exportTable(GString format, GString statement, GString msgFile, GSeq<GString>* pathSeq, GString dataFile, int sessions,GString modified )
{
    return m_pIDBAPI->exportTable(format, statement, msgFile, pathSeq, dataFile, sessions, modified);
}

int DBAPIPlugin::exportTableNew(GString format, GString statement, GString msgFile, GSeq<GString>* pathSeq, GString dataFile, int sessions,GString modified )
{
    return m_pIDBAPI->exportTableNew(format, statement, msgFile, pathSeq, dataFile, sessions, modified);
}

GString DBAPIPlugin::reorgTable(GString table, GString indexName, GString tabSpace)
{
    return m_pIDBAPI->reorgTable(table, indexName, tabSpace);
}	
GString DBAPIPlugin::runStats(GString table, GSeq <GString> * indList, unsigned char statsOpt, unsigned char shareLevel )
{
    return m_pIDBAPI->runStats(table, indList, statsOpt, shareLevel);
}	

GString DBAPIPlugin::rebind(GString bindFile)
{
    return m_pIDBAPI->rebind(bindFile);
}	

int DBAPIPlugin::getDBVersion(GString alias)
{
    return m_pIDBAPI->getDBVersion(alias);
}	

int DBAPIPlugin::stopReq()
{
    return m_pIDBAPI->stopReq();
}	

int DBAPIPlugin::loadFromFile(GString dataFile, GSeq<GString> *pathSeq, GString format, GString statement, GString msgFile, GString modifierString)
{
    return m_pIDBAPI->loadFromFile(dataFile, pathSeq, format, statement, msgFile, modifierString);
}	

int DBAPIPlugin::loadFromFileNew(GString dataFile, GSeq<GString> *pathSeq, GString format, GString statement, GString msgFile, GString modifierString, GString copyTarget )
{
    return m_pIDBAPI->loadFromFileNew(dataFile, pathSeq, format, statement, msgFile, modifierString, copyTarget);
}


GString DBAPIPlugin::getDbCfgForToken(GString nodeName, GString user, GString pwd, GString dbAlias, int token, int isNumToken)
{
    return m_pIDBAPI->getDbCfgForToken(nodeName, user, pwd, dbAlias, token, isNumToken);
}

GSeq <DB_INFO*> DBAPIPlugin::dbInfo()
{
    return m_pIDBAPI->dbInfo();
}

int DBAPIPlugin::getDynSQLSnapshotData()
{
    return m_pIDBAPI->getDynSQLSnapshotData();
}
int DBAPIPlugin::getRowData(int row, int col, GString * data)
{
    return m_pIDBAPI->getRowData(row, col, data);
}
unsigned long DBAPIPlugin::getRowDataCount()
{
    return m_pIDBAPI->getRowDataCount();
}

int DBAPIPlugin::getHeaderData(int pos, GString * data)
{
    return m_pIDBAPI->getHeaderData(pos, data);
}
unsigned long DBAPIPlugin::getHeaderDataCount()
{
    return m_pIDBAPI->getHeaderDataCount();
}

int DBAPIPlugin::createNode(GString hostName, GString nodeName, GString port, GString comment)
{
    return m_pIDBAPI->createNode(hostName, nodeName, port, comment);
}

GSeq<NODE_INFO*> DBAPIPlugin::getNodeInfo()
{
    return m_pIDBAPI->getNodeInfo();
}

int DBAPIPlugin::catalogDatabase(GString name, GString alias, GString type, GString nodeName, GString path, GString comment, int authentication  )
{
    return m_pIDBAPI->catalogDatabase(name, alias, type, nodeName, path, comment, authentication);
}

int DBAPIPlugin::uncatalogNode(GString nodeName)
{
    return m_pIDBAPI->uncatalogNode(nodeName);
}

int DBAPIPlugin::uncatalogDatabase(GString alias)
{
    return m_pIDBAPI->uncatalogDatabase(alias);
}

int DBAPIPlugin::dbRestart(GString alias, GString user, GString pwd)
{
    return m_pIDBAPI->dbRestart(alias, user, pwd);
}

void DBAPIPlugin::setGDebug(GDebug *pGDB)
{
    m_pIDBAPI->setGDebug(pGDB);
}
void DBAPIPlugin::deb(GString msg)
{
    GDebug::debMsg("DBAPIPlugin", m_dbapiPluginCounter, msg);
}

