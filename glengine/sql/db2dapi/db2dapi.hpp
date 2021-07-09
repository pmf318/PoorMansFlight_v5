//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 
//


#ifndef _DB2DAPI_
#define _DB2DAPI_

#ifdef  __IBMCPP__
#define MAKE_VA
#define NO_QT
#endif

#ifdef  _MSC_VER
#ifndef MAKE_VC
#define MAKE_VC
#endif
#endif



#include <sqlmon.h>
#ifdef SQLM_DBMON_VERSION7
#include <db2ApiDf.h>
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

//************** Visual Age *********************
#ifdef MAKE_VA
#ifdef Makedb2dapi
#define db2dapiExp_Imp _Export
#else
#define db2dapiExp_Imp _Import
//  #pragma library( "db2dapi.LIB" )
#pragma library( "GLEngine.LIB" )
#endif
#endif
//************** Visual Age END *****************

#include <gstring.hpp>
#include <string.h>
#include <stdio.h>
#ifndef _GSEQ_
#include <gseq.hpp>
#endif
#include <gthread.hpp>
#include <idbapi.hpp>
#include <dbapiplugin.hpp>



// ******************************* CLASS **************************** //
#ifdef MakeGString
class db2dapiExp_Imp db2dapi
#else
//Explicitly deriving from interface is surprisingly important on Windows (see mem-management on runtime [crt]
//and compiler option /MT) and even more surprisingly, apparently works perfectly well on Linux
//without referencing the interface.
class db2dapi : public IDBAPI
#endif
{

    class RowData
    {
        private:
            GSeq <GString> rowDataSeq;
        public:
            RowData(){}
            ~RowData();
            void add(GString data);
            GString elementAt(int pos);
            unsigned long numberOfElements();
    };

    class MainThread : public GThread
    {
        public:
            virtual void run();
            void setOwner( db2dapi * aUDB ) { mydb2dapi = aUDB; }
        private:
            db2dapi * mydb2dapi;
    };

private:
    MainThread * mainThread;

    void tm(GString msg);
    void mb(GString msg);
    int fillLV();
    int snType;

    QMessageBox * runInfo;

    GSeq<GString> headerSeq;
    GSeq<RowData*> rowSeq;

    void clearSequences();
    GDebug * m_pGDB;


public:
    VCExport  db2dapi();
    VCExport void setGDebug(GDebug *pGDB);
    //VCExport  db2dapi(GString database, GString ndName, GString user, GString pwd, int type, QTableWidget * pLV);
    VCExport  ~db2dapi();
    VCExport  db2dapi* clone() const;
    //VCExport  db2dapi(db2dapi & aDBAPI);
    VCExport  db2dapi( db2dapi const & o);
    VCExport  GString SQLError();
    VCExport  void startMainThread();


    VCExport  int importTable(GString dataFile, GSeq<GString> *pathSeq, GString format, GString statement, GString msgFile, GString modifierString = "");
    VCExport  int importTableNew(GString dataFile, GSeq<GString> *pathSeq, GString format, GString statement, GString msgFile, GString modifierString = "");

    VCExport  GString reorgTable(GString table, GString indexName="", GString tabSpace = "");
    VCExport  GString runStats(GString table, GSeq <GString> * indList, unsigned char statsOpt, unsigned char shareLevel );


    VCExport  GString rebind(GString bindFile);

    VCExport  int exportTable(GString dataFile, GString format, GString statement, GString msgFile, GString modified);
    VCExport  int exportTable(GString format, GString statement, GString msgFile, GSeq<GString> *pathSeq, GString dataFile, int sessions, GString modified );
    VCExport  int exportTableNew(GString format, GString statement, GString msgFile, GSeq<GString> *pathSeq, GString dataFile, int sessions, GString modified );

    VCExport  int getDBVersion(GString alias);
    VCExport  int stopReq();

    VCExport  int loadFromFile(GString dataFile, GSeq<GString> *pathSeq, GString format, GString statement, GString msgFile, GString modifierString);
    VCExport  int loadFromFileNew(GString dataFile, GSeq<GString> *pathSeq, GString format, GString statement, GString msgFile, GString modifierString, GString copyTarget = "");
    VCExport  GString getDbCfgForToken(GString nodeName, GString user, GString pwd, GString dbAlias, int token, int isNumToken);
    VCExport  unsigned long getResultCount(GString cmd);
    VCExport  GSeq <DB_INFO*> dbInfo();
    VCExport  int getDynSQLSnapshotData();
    VCExport  int getHeaderData(int pos, GString * data);
    VCExport  int getRowData(int row, int col, GString * data);
    VCExport unsigned long getRowDataCount();
    VCExport unsigned long getHeaderDataCount();    

    VCExport int createNode(GString hostName, GString nodeName, GString port, GString comment = "");
    VCExport GSeq<NODE_INFO*> getNodeInfo();
    VCExport int catalogDatabase(GString name, GString alias, GString type, GString nodeName, GString path, GString comment, int authentication );
    VCExport int uncatalogNode(GString nodeName);
    VCExport int uncatalogDatabase(GString alias);
    VCExport int dbRestart(GString alias, GString user, GString pwd);


    /* STUFF FOR SNAPSHOT */
#ifdef SQLM_DBMON_VERSION7
public:
    VCExport  int getSnapshotData(int type);
    VCExport  int initMonitor(GString database, GString node, GString user, GString pwd);
    VCExport  int stopMonitor();
    VCExport  int startMonitor();
    VCExport  int resetMonitor();
    VCExport  int initTabSpaceLV(GSeq <TAB_SPACE*> *dataSeq);
    
private:
    int iMonVersion;
    sqlm_header_info* jumpToKey(int element, char* start, GString & data);
    int processData(char * buffer_ptr, int type);
    GString getData(sqlm_header_info * pHeader, char* data);
    GString m_gstrDbName, m_gstrNodeName, m_gstrUser, m_gstrPwd;
    int readMonitors();


    int iGetTime;
    int fillTabList(char * pStart);

    int fillDSQL(char * pStart);


    int fillLock(char * pStart);
    int initSnapshot(struct sqlma *pSqlMA, int type);
    int FreeMemory(struct sqlma *ma_ptr, char *buffer_ptr);
    int initELMArray();
    void processBuffer(sqlm_header_info *datastream);
    void  fillTabSpaceLV (struct SQLB_TBSPQRY_DATA *dataP, sqluint32 num, GSeq <TAB_SPACE*> *dataSeq);

    int fillLV(char * pStart, int key, GSeq <int> * keySeq, GSeq <GString> * nameSeq);

#endif
};

#endif





