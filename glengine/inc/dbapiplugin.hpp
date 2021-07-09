//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 
//


#ifndef _DBAPIPLUGIN_
#define _DBAPIPLUGIN_

#ifdef  __IBMCPP__
#define MAKE_VA
#define NO_QT
#endif

#ifdef  _MSC_VER
#ifndef MAKE_VC
#define MAKE_VC
#endif
#endif

//************** Visual C++ ********************
#ifdef MAKE_VC
#define DllImport   __declspec( dllimport )
#define DllExport   __declspec( dllexport )
#define VCExport DllExport
#else
#define VCExport
#endif
//(************** Visual C++ END *****************



#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <gstring.hpp>
#include <gseq.hpp>

#include <idbapi.hpp>
#include <idsql.hpp>

//This is global
extern GString m_strDBAPIPluginPath;

#ifdef MakePlugin
class DBAPIPluginExp_Imp DBAPIPlugin
        #else
class DBAPIPlugin : IDBAPI
        #endif
{
private:
#ifdef MAKE_VC
    HINSTANCE dbapiHandle;
    typedef IDBAPI* (*CREATE) ();
    typedef void   (*DESTROY)(IDBAPI*);
    CREATE crt;
    DESTROY destr;
#elif __MINGW32__
    void* dbapiHandle;
    typedef IDBAPI* (*CREATE) ();
    typedef void   (*DESTROY)(IDBAPI*);
    CREATE crt;
    DESTROY destr;
#else
    void *dbapiHandle;
    typedef IDBAPI* create_t();
    typedef void destroy_t(IDBAPI*);
    create_t* crt;
    destroy_t* destr;
#endif

    void init();

    GString m_strDBTypeName;
    void init(GString dbTypeName);
    
    int loadPlugin(GString libPath);
    IDBAPI* m_pIDBAPI;
    int m_iMyInstanceCounter;
    int m_iPluginLoaded;
    int callLoader(GString dbName, GString file = "");
    void deb(GString msg);

public:

    VCExport   DBAPIPlugin(GString dbTypeName, GString fileName = "");
    VCExport   ~DBAPIPlugin();
    VCExport   DBAPIPlugin& operator= (DBAPIPlugin const& in);
    VCExport   DBAPIPlugin(DBAPIPlugin const & plg);
    VCExport   int isOK();

	VCExport static void setPluginPath(GString path);
    VCExport static GString pluginPath();

    VCExport DBAPIPlugin* clone() const;


    VCExport GString SQLError();


    VCExport int isValid(){return m_pIDBAPI == NULL ? 0 : 1;}
    VCExport int getSnapshotData(int type);
    VCExport int initMonitor(GString database, GString node, GString user, GString pwd);
    VCExport int startMonitor();
    VCExport int resetMonitor();
    VCExport int stopMonitor();
    VCExport int initTabSpaceLV(GSeq <TAB_SPACE*> *dataSeq);

    VCExport int importTable(GString dataFile, GSeq<GString> *pathSeq, GString format, GString statement, GString msgFile, GString modifierString = "");
    VCExport int importTableNew(GString dataFile, GSeq<GString> *pathSeq, GString format, GString statement, GString msgFile, GString modifierString = "");
    VCExport int exportTable(GString dataFile, GString format, GString statement, GString msgFile, GString modified);
    VCExport int exportTable(GString format, GString statement, GString msgFile, GSeq<GString> *pathSeq, GString dataFile, int sessions, GString modified );
    VCExport int exportTableNew(GString format, GString statement, GString msgFile, GSeq<GString> *pathSeq, GString dataFile, int sessions, GString modified );
    VCExport GString getDbCfgForToken(GString nodeName, GString user, GString pwd, GString dbAlias, int token, int isNumToken);

    VCExport GString reorgTable(GString table, GString indexName="", GString tabSpace = "");
    VCExport GString runStats(GString table, GSeq <GString> * indList, unsigned char statsOpt, unsigned char shareLevel );

    VCExport GString rebind(GString bindFile);


    VCExport int getDBVersion(GString alias);
    VCExport int stopReq();

    VCExport int loadFromFile(GString dataFile, GSeq<GString> *pathSeq, GString format, GString statement, GString msgFile, GString modifierString);
    VCExport int loadFromFileNew(GString dataFile, GSeq<GString> *pathSeq, GString format, GString statement, GString msgFile, GString modifierString, GString copyTarget = "");
    VCExport GSeq<DB_INFO*> dbInfo();
    VCExport int getDynSQLSnapshotData();
    VCExport int getRowData(int row, int col, GString * data);
    VCExport int getHeaderData(int pos, GString * data);
    VCExport unsigned long getHeaderDataCount();
    VCExport unsigned long getRowDataCount();
    VCExport void setGDebug(GDebug *pGDB);
    VCExport int createNode(GString hostName, GString nodeName, GString port, GString comment = "");
    VCExport GSeq<NODE_INFO*> getNodeInfo();
    VCExport int catalogDatabase(GString name, GString alias, GString type, GString nodeName, GString path, GString comment, int authentication );
    VCExport int uncatalogNode(GString nodeName);
    VCExport int uncatalogDatabase(GString alias);
    VCExport int dbRestart(GString alias, GString user, GString pwd);

};
#endif


