//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt
//

#define SNAPSHOT_BUFFER_UNIT_SZ 1024
#define NUMELEMENTS 779

//#include <stdio.h>

#include <db2dapi.hpp>

#include "sqlca.h"
#include "sqlda.h"
#include <sqlenv.h>
#include "sqlutil.h"

#include <idsql.hpp>
#include <gstuff.hpp>


#if(defined(DB2NT))
  #define PATH_SEP "\\"
#else /* UNIX */
  #define PATH_SEP "/"
#endif

#define PMF_HADR 318

#include <sqlmon.h>

static  struct sqlca    sqlca;
static  char            SqlMsg[1024];


/***********************************************************************
 * This class can either be instatiated or loaded via dlopen/loadlibrary
 ***********************************************************************/
//Define functions with C symbols (create/destroy instance).
#ifndef MAKE_VC
extern "C" db2dapi* create()
{
    //printf("Calling db2dapi creat()\n");
    return new db2dapi();
}
extern "C" void destroy(db2dapi* pDb2api)
{
    if( pDb2api ) delete pDb2api;
}
#else
extern "C" __declspec( dllexport ) db2dapi* create()
{
    //printf("Calling db2dapi creat()\n");
    //flushall();
    return new db2dapi();
}	
extern "C" __declspec( dllexport ) void destroy(db2dapi* pDb2api)
{
    if( pDb2api ) delete pDb2api ;
}	
#endif

GString db2dapi::SQLError()
{
    //if( !sqlca.sqlcode && !SQLCODE ) return "";
    sqlaintp( SqlMsg, sizeof(SqlMsg), 1023, &sqlca );
    return( GString(&SqlMsg[0]) );
}

db2dapi* db2dapi::clone() const
{
    //printf("clone called\n");
    return new db2dapi(*this);
}

db2dapi::db2dapi()
{
    m_pGDB = NULL;
}
db2dapi::db2dapi(const db2dapi & aDBAPI)
{
    m_pGDB = aDBAPI.m_pGDB;
}
db2dapi::~db2dapi()
{
    clearSequences();
    if( m_gstrNodeName.length() ) sqledtin(&sqlca);
}

/****************************************************************
*
*
*  Stuff for snapshot
*  THIS ONLY GETS COMPILED ON DB2 v 7 and above
*
*****************************************************************/
#ifdef SQLM_DBMON_VERSION7

int db2dapi::getSnapshotData(int type)
{

    clearSequences();
    tm("Start getSN, node: "+m_gstrNodeName+", type: "+GString(type));

    int snType = 0;
    if(type == 1 ) snType = SQLMA_DBASE_BUFFERPOOLS;
    else if( type == 2 ) snType = SQLMA_DBASE;
    else if( type == 3 ) snType = SQLMA_DBASE_LOCKS;
    else if( type == 4 ) snType = SQLMA_DBASE_TABLES;
    else if( type == 5 ) snType = SQLMA_DB2;
    else if( type == 6 ) snType = SQLMA_DBASE;
    else if( type == 7 ) snType = SQLMA_DBASE_ALL;
    //SQLMA_DBASE, SQLMA_DBASE_ALL

    unsigned int obj_num = 1;    /* # of objects to monitor */

    struct sqlma *pSqlMA = NULL;  /* sqlma structure pointer */
    unsigned int ma_sz;           /* size of sqlma structure */


    ma_sz = SQLMASIZE(obj_num);
    pSqlMA = (struct sqlma *) malloc(ma_sz);
    if ( pSqlMA == NULL)
    {
        mb("Error allocating sqlma. Don't know what's wrong. Really.\nExiting.");
        return(99);
    }
    memset(pSqlMA, '\0', ma_sz);

    pSqlMA->obj_num = obj_num;
    pSqlMA->obj_var[0].obj_type = snType;
    strncpy((char *)pSqlMA->obj_var[0].object, (char*)m_gstrDbName.strip(), SQLM_OBJECT_SZ);


    /* Possible values for type:
  SQLMA_DBASE_ALL;
  SQLMA_BUFFERPOOLS_ALL;
  SQLMA_DBASE_REMOTE_ALL;
  SQLMA_DBASE;
  SQLMA_DBASE_APPLS;
  SQLMA_DBASE_TABLESPACES;
  SQLMA_DBASE_LOCKS;
  SQLMA_DBASE_BUFFERPOOLS;
  SQLMA_DBASE_TABLES;
  SQLMA_DYNAMIC_SQL;
*/

    return initSnapshot(pSqlMA, snType);

    //  runInfo->close();

}
int db2dapi::getDynSQLSnapshotData()
{

    clearSequences();
    int snType = SQLMA_DYNAMIC_SQL;
    tm("Start getSN, node: "+m_gstrNodeName);

    unsigned int obj_num = 1;    /* # of objects to monitor */

    struct sqlma *pSqlMA = NULL;  /* sqlma structure pointer */
    unsigned int ma_sz;           /* size of sqlma structure */


    ma_sz = SQLMASIZE(obj_num);
    pSqlMA = (struct sqlma *) malloc(ma_sz);
    if ( pSqlMA == NULL)
    {
        mb("Error allocating sqlma. Don't know what's wrong. Really.\nExiting.");
        return(99);
    }
    memset(pSqlMA, '\0', ma_sz);

    pSqlMA->obj_num = obj_num;
    pSqlMA->obj_var[0].obj_type = SQLMA_DYNAMIC_SQL;
    strncpy((char *)pSqlMA->obj_var[0].object, (char*)m_gstrDbName.strip(), SQLM_OBJECT_SZ);

    return initSnapshot(pSqlMA, snType);
}
void db2dapi::setGDebug(GDebug *pGDB)
{
    m_pGDB = pGDB;
}

/****************************************************************
*
*   INIT SNAPSHOT
*
****************************************************************/
int db2dapi::initSnapshot(struct sqlma *pSqlMA, int type)
{

//    if( !nodeName.length() || nodeName == _HOST_DEFAULT )
//    {
//        nodeName = getenv("DB2INSTANCE");
//        if( !nodeName.length() )  nodeName = "DB2"; //We're guessing here...
//    }
    //This apparently confuses the monitor results for dynSQL on remote instances...
//    if( nodeName != "DB2" ) sqleatin(nodeName, user, pwd, &sqlca);
//    if( sqlca.sqlcode  )
//    {
//        mb("Tried to attach to instance "+nodeName+". It failed with error-code "+GString(sqlca.sqlcode));
//        return 1;
//    }
    //Done attach
    int erc;
    struct sqlm_collected collected;
    //struct sqlca sqlca;
    char *buffer_ptr = NULL;
    sqluint32 buffer_sz;
    sqluint32 outputFormat;
    db2GetSnapshotData getSnapshotParam;
    db2GetSnapshotSizeData getSnapshotSizeParam;


    memset(&collected, '\0', sizeof(struct sqlm_collected));

    getSnapshotSizeParam.piSqlmaData = pSqlMA;
    getSnapshotSizeParam.poBufferSize = &buffer_sz;
    getSnapshotSizeParam.iVersion = SQLM_CURRENT_VERSION;
    getSnapshotSizeParam.iNodeNumber = SQLM_CURRENT_NODE;
    //From v 8.1    
#ifdef SQLM_DBMON_VERSION9
    getSnapshotSizeParam.iSnapshotClass = SQLM_CLASS_DEFAULT;
#elif SQLM_DBMON_VERSION8
    getSnapshotSizeParam.iSnapshotClass = SQLM_CLASS_DEFAULT;
#endif

    //!!
#ifdef SQLM_DBMON_VERSION9
    erc = db2GetSnapshotSize(db2Version970, &getSnapshotSizeParam, &sqlca);
#elif SQLM_DBMON_VERSION8
    erc = db2GetSnapshotSize(db2Version810, &getSnapshotSizeParam, &sqlca);
#else
    erc = db2GetSnapshotSize(db2Version710, &getSnapshotSizeParam, &sqlca);
#endif

    tm("getsn1: erc "+GString(erc));

    if( erc )
    {
        FreeMemory(pSqlMA, buffer_ptr);
        mb("db2GetSnapshotSize returned error-code "+GString(erc)+"\nand assumes you'll figure out what's wrong.\nExiting");
        return erc;
    }

    //Attach to remote instance if necessary
    erc = 0;


    if (buffer_sz == 0)
    {
        tm("getsn3: ercNoMEM");
        FreeMemory(pSqlMA, buffer_ptr);
        mb("There is something seriously wrong with some internal buffers. I'll quit. Sorry.");
        return(2);
    }
    buffer_ptr = (char *) malloc(buffer_sz);
    if (buffer_ptr == NULL)
    {
        tm("getsn4: erc NoBuff");
        mb("Could not allocate required memory (DB2 says it should be "+GString(buffer_sz)+"). I'll quit. Sorry.");
        FreeMemory(pSqlMA, buffer_ptr);
        return(3);
    }
    memset(buffer_ptr, '\0', buffer_sz);
    tm("initSN2");
    getSnapshotParam.piSqlmaData = pSqlMA;
    getSnapshotParam.poCollectedData = &collected;
    getSnapshotParam.iBufferSize = buffer_sz;
    getSnapshotParam.poBuffer = buffer_ptr;
    getSnapshotParam.iVersion = SQLM_CURRENT_VERSION;
    getSnapshotParam.iStoreResult = 0;
    getSnapshotParam.iNodeNumber = SQLM_CURRENT_NODE;
    getSnapshotParam.poOutputFormat = &outputFormat;

    erc = -1;
#ifdef SQLM_DBMON_VERSION9
    getSnapshotParam.iSnapshotClass = SQLM_CLASS_DEFAULT;
    erc = db2GetSnapshot(db2Version970, &getSnapshotParam, &sqlca);
#elif SQLM_DBMON_VERSION8
    getSnapshotParam.iSnapshotClass = SQLM_CLASS_DEFAULT;
    erc = db2GetSnapshot(db2Version810, &getSnapshotParam, &sqlca);
#else
    erc = db2GetSnapshot(db2Version710, &getSnapshotParam, &sqlca);
#endif
    tm("getsn5: erc "+GString(erc));
    while (sqlca.sqlcode == 1606)
    {
        FreeMemory(NULL, buffer_ptr);
        buffer_sz = buffer_sz + SNAPSHOT_BUFFER_UNIT_SZ;

        buffer_ptr = (char *) malloc(buffer_sz);
        if (buffer_ptr == NULL)
        {
            FreeMemory(pSqlMA, buffer_ptr);
            mb("Could not allocate memory for snapshot. I'll quit. Sorry.");
            return(4);
        }
        memset(buffer_ptr, '\0', buffer_sz);

        getSnapshotParam.iBufferSize = buffer_sz;
        getSnapshotParam.poBuffer = buffer_ptr;
        //!!
        erc = 0;
#ifdef SQLM_DBMON_VERSION9
        erc = db2GetSnapshot(db2Version970, &getSnapshotParam, &sqlca);
#elif SQLM_DBMON_VERSION8
        erc = db2GetSnapshot(db2Version810, &getSnapshotParam, &sqlca);
#else
        erc = db2GetSnapshot(db2Version710, &getSnapshotParam, &sqlca);
#endif

        tm("getsn6: erc "+GString(erc));
    }
    if (sqlca.sqlcode == 1611)
    {
        tm("Got 1611");
    }

    if (sqlca.sqlcode < 0L )
    {
        sqlaintp( SqlMsg, sizeof(SqlMsg), 1023, &sqlca );
        tm("err "+GString(sqlca.sqlcode)+", "+GString(SqlMsg));
        FreeMemory(pSqlMA, buffer_ptr);
        mb("db2GetSnapshot returned SQLerror "+GString(sqlca.sqlcode)+"\n"+GString(SqlMsg));
        return((int)sqlca.sqlcode);
    }

    tm("initSN3, erc: "+GString(erc));
    if ( erc )
    {
        FreeMemory(pSqlMA, buffer_ptr);
        return erc;
    }
    processData(buffer_ptr, type);
    FreeMemory(pSqlMA, buffer_ptr);
    tm("initSN5");
    //  if( nodeName != "DB2" ) sqledtin(&sqlca);
    return erc;
}


int db2dapi::initMonitor(GString database, GString node, GString user, GString pwd)
{
    int rc = 0;
    m_gstrDbName = database;
    m_gstrNodeName = node;
    m_gstrUser = user;
    m_gstrPwd = pwd;
    if( m_gstrNodeName == _HOST_DEFAULT ) m_gstrNodeName= "";

//    if( !m_gstrNodeName.length() || m_gstrNodeName == _HOST_DEFAULT )
//    {
//        m_gstrNodeName = getenv("DB2INSTANCE");
//        if( !m_gstrNodeName.length() )  m_gstrNodeName = "DB2"; //We're guessing here...
//    }
    //This apparently confuses the monitor results for dynSQL on remote instances...
//    if( nodeName != "DB2" ) sqleatin(nodeName, user, pwd, &sqlca);
//    if( sqlca.sqlcode  )
//    {
//        mb("Tried to attach to instance "+nodeName+". It failed with error-code "+GString(sqlca.sqlcode));
//        return 1;
//    }

    rc = sqleatin(m_gstrNodeName, m_gstrUser, m_gstrPwd, &sqlca);
    getDBVersion(m_gstrDbName);
//    if( m_gstrNodeName == _HOST_DEFAULT ) m_gstrNodeName= "";
//    else rc = sqleatin(m_gstrNodeName, m_gstrNodeName, m_gstrNodeName, &sqlca);
    return sqlca.sqlcode;
}

/****************************************************************
*
*   START MONITORS
*
****************************************************************/
int db2dapi::startMonitor()
{

    tm("Start Mon...");
    int rc = 0;
    db2MonitorSwitchesData switchesData;

    struct sqlm_recording_group switchesList[SQLM_NUM_GROUPS];
    sqluint32 outputFormat;
    /************
  tm("dbVers: "+GString(getDBVersion(database)));
  switch( getDBVersion(database) )
  {
     case 9:  //DB2 v7
        iMonVersion = db2Version710;
        break;
     case 10: //DB2 v8
        //iMonVersion = db2Version810;
        break;

     default:
        iMonVersion = db2Version710;
        break;
  }
**************/

#ifdef SQLM_DBMON_VERSION9
    rc = db2MonitorSwitches(db2Version970, &switchesData, &sqlca);
#elif SQLM_DBMON_VERSION8
    db2MonitorSwitches(db2Version810, &switchesData, &sqlca);
#else
    db2MonitorSwitches(db2Version710, &switchesData, &sqlca);
#endif

    switchesList[SQLM_UOW_SW].input_state = SQLM_ON;
    switchesList[SQLM_STATEMENT_SW].input_state = SQLM_ON;
    switchesList[SQLM_TABLE_SW].input_state = SQLM_ON;
    switchesList[SQLM_BUFFER_POOL_SW].input_state = SQLM_ON;
    switchesList[SQLM_LOCK_SW].input_state = SQLM_ON;
    switchesList[SQLM_SORT_SW].input_state = SQLM_ON;
#ifdef SQLM_DBMON_VERSION8
    switchesList[SQLM_TIMESTAMP_SW].input_state = SQLM_ON;
#endif


    switchesData.piGroupStates = switchesList;
    switchesData.poBuffer = NULL;
    switchesData.iVersion = SQLM_CURRENT_VERSION;
    switchesData.iBufferSize = 0;
    switchesData.iReturnData = 0;
    switchesData.iNodeNumber = SQLM_CURRENT_NODE;
    switchesData.poOutputFormat = &outputFormat;

#ifdef SQLM_DBMON_VERSION9
    rc = db2MonitorSwitches(db2Version970, &switchesData, &sqlca);
#elif SQLM_DBMON_VERSION8
    db2MonitorSwitches(db2Version810, &switchesData, &sqlca);
#else     
    db2MonitorSwitches(db2Version710, &switchesData, &sqlca);
#endif     

    switchesData.iReturnData = 0;
    db2MonitorSwitches(db2Version970, &switchesData, &sqlca);
    switchesData.poBuffer = NULL;
    int state;
    state = switchesList[SQLM_UOW_SW].output_state;
    state = switchesList[SQLM_STATEMENT_SW].output_state;
    state = switchesList[SQLM_TABLE_SW].output_state;
    state = switchesList[SQLM_BUFFER_POOL_SW].output_state;
    state = switchesList[SQLM_LOCK_SW].output_state;
    state = switchesList[SQLM_SORT_SW].output_state;


    if (sqlca.sqlcode < 0L)
    {
        return((int)sqlca.sqlcode);
    }
    tm("Mon started");


    return rc;
}

int db2dapi::readMonitors()
{
    int SWITCHES_BUFFER_UNIT_SZ = 1024;
    tm("Start Mon...");
    int rc = 0;
    db2MonitorSwitchesData switchesData;
    memset (&switchesData, '\0', sizeof(switchesData));
    struct sqlm_recording_group switchesList[SQLM_NUM_GROUPS];
    memset(switchesList, '\0', sizeof(switchesList));
    sqluint32 outputFormat = SQLM_STREAM_STATIC_FORMAT;
    static sqluint32 switchesBufferSize = SWITCHES_BUFFER_UNIT_SZ;
    char *switchesBuffer;

    switchesBuffer = (char *)malloc(switchesBufferSize);
    memset(switchesBuffer, '\0', switchesBufferSize);

    switchesData.piGroupStates = switchesList;
    switchesData.poBuffer = switchesBuffer;
    switchesData.iVersion = SQLM_DBMON_VERSION9_5;
    switchesData.iBufferSize = switchesBufferSize;
    switchesData.iReturnData = 1;
    switchesData.iNodeNumber = SQLM_CURRENT_NODE;
    switchesData.poOutputFormat = &outputFormat;
    rc = db2MonitorSwitches(db2Version970, &switchesData, &sqlca);

    free(switchesBuffer);
    return rc;

    /************
  tm("dbVers: "+GString(getDBVersion(database)));
  switch( getDBVersion(database) )
  {
     case 9:  //DB2 v7
        iMonVersion = db2Version710;
        break;
     case 10: //DB2 v8
        //iMonVersion = db2Version810;
        break;

     default:
        iMonVersion = db2Version710;
        break;
  }
**************/
/*
#ifdef SQLM_DBMON_VERSION9
    rc = db2MonitorSwitches(db2Version970, &switchesData, &sqlca);
#elif SQLM_DBMON_VERSION8
    db2MonitorSwitches(db2Version810, &switchesData, &sqlca);
#else
    db2MonitorSwitches(db2Version710, &switchesData, &sqlca);
#endif

    switchesList[SQLM_UOW_SW].input_state = SQLM_HOLD;
    switchesList[SQLM_STATEMENT_SW].input_state = SQLM_HOLD;
    switchesList[SQLM_TABLE_SW].input_state = SQLM_HOLD;
    switchesList[SQLM_BUFFER_POOL_SW].input_state = SQLM_HOLD;
    switchesList[SQLM_LOCK_SW].input_state = SQLM_HOLD;
    switchesList[SQLM_SORT_SW].input_state = SQLM_HOLD;
#ifdef SQLM_DBMON_VERSION8
    switchesList[SQLM_TIMESTAMP_SW].input_state = SQLM_ON;
#endif

    sqluint32 buffer_sz = 405;
    char *buffer_ptr = NULL;
    buffer_ptr = (char *) malloc(buffer_sz);

    printf("size: %i\n", sizeof(switchesList));
    switchesData.piGroupStates = switchesList;
    switchesData.poBuffer = buffer_ptr;
    switchesData.iVersion = SQLM_CURRENT_VERSION;
    switchesData.iBufferSize = buffer_sz;
    switchesData.iReturnData = 1;
    switchesData.iNodeNumber = SQLM_CURRENT_NODE;
    switchesData.poOutputFormat = &outputFormat;

#ifdef SQLM_DBMON_VERSION9
    rc = db2MonitorSwitches(db2Version970, &switchesData, &sqlca);
#elif SQLM_DBMON_VERSION8
    db2MonitorSwitches(db2Version810, &switchesData, &sqlca);
#else
    db2MonitorSwitches(db2Version710, &switchesData, &sqlca);
#endif

    db2MonitorSwitches(db2Version970, &switchesData, &sqlca);
    int state;

    printf("BUF: %s\n", buffer_ptr);
    state = switchesList[SQLM_UOW_SW].output_state;
    if( switchesList[SQLM_STATEMENT_SW].input_state == SQLM_OFF) printf("readMonitors: IS OFF\n");
    else printf("readMonitors: IS ON\n");
    state = switchesList[SQLM_STATEMENT_SW].output_state;
    state = switchesList[SQLM_TABLE_SW].output_state;
    state = switchesList[SQLM_BUFFER_POOL_SW].output_state;
    state = switchesList[SQLM_LOCK_SW].output_state;
    state = switchesList[SQLM_SORT_SW].output_state;
    state = switchesList[SQLM_TIMESTAMP_SW].output_state;

mb(this->SQLError());
    if (sqlca.sqlcode < 0L)
    {
        tm(this->SQLError());
        return((int)sqlca.sqlcode);
    }
    tm("Mon started");
    return rc;
*/
/*
    tm("Start Mon...");
    int rc = 0;


    //struct sqlca sqlca;
    db2MonitorSwitchesData switchesData;

    struct sqlm_recording_group switchesList[SQLM_NUM_GROUPS];
    sqluint32 outputFormat = SQLM_STREAM_STATIC_FORMAT;
    switchesData.piGroupStates = switchesList;
    switchesData.poBuffer = NULL;
    switchesData.iVersion = SQLM_CURRENT_VERSION;
    switchesData.iBufferSize = 0;
    switchesData.iReturnData = 0;
    switchesData.iNodeNumber = SQLM_CURRENT_NODE;
    switchesData.poOutputFormat = &outputFormat;

    rc = db2MonitorSwitches(db2Version970, &switchesData, &sqlca);


    switchesList[SQLM_UOW_SW].input_state = SQLM_HOLD;
    switchesList[SQLM_STATEMENT_SW].input_state = SQLM_HOLD;
    switchesList[SQLM_TABLE_SW].input_state = SQLM_HOLD;
    switchesList[SQLM_BUFFER_POOL_SW].input_state = SQLM_HOLD;
    switchesList[SQLM_LOCK_SW].input_state = SQLM_HOLD;
    switchesList[SQLM_SORT_SW].input_state = SQLM_HOLD;


#ifdef SQLM_DBMON_VERSION9
    db2MonitorSwitches(db2Version970, &switchesData, &sqlca);
#elif SQLM_DBMON_VERSION8
    db2MonitorSwitches(db2Version810, &switchesData, &sqlca);
#else
    db2MonitorSwitches(db2Version710, &switchesData, &sqlca);
#endif

    int state;
    state = switchesList[SQLM_UOW_SW].output_state;
    state = switchesList[SQLM_STATEMENT_SW].output_state;
    state = switchesList[SQLM_TABLE_SW].output_state;
    state = switchesList[SQLM_BUFFER_POOL_SW].output_state;
    state = switchesList[SQLM_LOCK_SW].output_state;
    state = switchesList[SQLM_SORT_SW].output_state;


    if (sqlca.sqlcode != 0L)
    {
        if (sqlca.sqlcode < 0L)
        {
            return((int)sqlca.sqlcode);
        }
    }
    tm("Mon started");



    return rc;
    */
}


int db2dapi::stopMonitor()
{
    resetMonitor();
    return 0;
}



/****************************************************************
*
*   RESET MONITORS
*
****************************************************************/
int db2dapi::resetMonitor()
{
    tm("Start ResetMon...");
    int rc = 0;
    //struct sqlca sqlca;
    db2MonitorSwitchesData switchesData;

    struct sqlm_recording_group switchesList[SQLM_NUM_GROUPS];

    tm("Resetting...");

    switchesList[SQLM_UOW_SW].input_state = SQLM_OFF;
    switchesList[SQLM_STATEMENT_SW].input_state = SQLM_OFF;
    switchesList[SQLM_TABLE_SW].input_state = SQLM_OFF;
    switchesList[SQLM_BUFFER_POOL_SW].input_state = SQLM_OFF;
    switchesList[SQLM_LOCK_SW].input_state = SQLM_OFF;
    switchesList[SQLM_SORT_SW].input_state = SQLM_OFF;
#ifdef SQLM_DBMON_VERSION8
    switchesList[SQLM_TIMESTAMP_SW].input_state = SQLM_OFF;
#endif
    db2ResetMonitorData resetData;
    resetData.iNodeNumber = SQLM_CURRENT_NODE;
    resetData.iResetAll = 1;
    resetData.iVersion = SQLM_CURRENT_VERSION;
    resetData.piDbAlias = (char*) m_gstrDbName;

    switchesData.iVersion = SQLM_CURRENT_VERSION;
    tm("Resetting...DONE");
#ifdef SQLM_DBMON_VERSION9
    rc = db2ResetMonitor(db2Version970, &switchesData, &sqlca);
#elif SQLM_DBMON_VERSION8
    //     db2MonitorSwitches(db2Version810, &switchesData, &sqlca);
    rc= db2ResetMonitor(db2Version810, &switchesData, &sqlca);
#else
    //     db2MonitorSwitches(db2Version710, &switchesData, &sqlca);
    rc = db2ResetMonitor(db2Version710, &switchesData, &sqlca);
#endif     
    rc = db2ResetMonitor(db2Version970, &resetData, &sqlca);
    if (sqlca.sqlcode != 0L)
    {
        if (sqlca.sqlcode < 0L)
        {
            return((int)sqlca.sqlcode);
        }
    }
    tm("Mon resetted");
    return rc;
}

/****************************************************************
*
*   FREE MEM
*
****************************************************************/
int db2dapi::FreeMemory(struct sqlma *pSqlMA, char *buffer_ptr)
{
    tm("Start FreeMem....");
    if (buffer_ptr != NULL) free(buffer_ptr);
    if (pSqlMA != NULL) free(pSqlMA);
    tm("Done FreeMem");
    return 0;
}


/****************************************************************
*
*   GET DATA
*
****************************************************************/
GString db2dapi::getData(sqlm_header_info * pHeader, char* pData)
{
    char buff[23];
    if (pHeader->type == SQLM_TYPE_U32BIT)
    {
        unsigned int i = *(unsigned int*)pData;
        sprintf(buff,"%d",i);
        return GString(i);
        //printf("%d  (0x%x)\n",i,i);
    }
    else if (pHeader->type == SQLM_TYPE_32BIT)
    {
        signed int i = *(signed int*)pData;
        return GString(i);
    }
    else if (pHeader->type == SQLM_TYPE_64BIT)
    {
        #ifdef MAKE_VC
        int64_t i = *(int64_t*)pData;
        #else
        long i = *(long*)pData;
        #endif
        return GString(i);
    }
    else if (pHeader->type == SQLM_TYPE_U64BIT)
    {
        #ifdef MAKE_VC
        uint64_t i = *(uint64_t*)pData;
        #else
        unsigned long i = *(unsigned long*)pData;
        #endif
        return GString(i);
    }
    else if (pHeader->type == SQLM_TYPE_STRING)
    {
        return GString(pData, pHeader->size);
    }
    else if (pHeader->type == SQLM_TYPE_U16BIT)
    {
        // print out the data (4 bytes long and unsigned)
        unsigned int i = *(unsigned short*)pData;
        return GString(i);
    }
    else if (pHeader->type == SQLM_TYPE_16BIT)
    {
        signed int i = *(signed short*)pData;
        return GString(i);
    }
    else
    {
        GString d;
        if( pHeader->size == 8 )
        {
            d = GString(*(signed long*)pData);
            return d;
        }
        //if( pHeader->size == 8 ) d = "";
        d = "0x";
        unsigned int i, j;
        if( pHeader->size > 64 ) return "<Programmers fault (probably)>";
        for (i = 0; i<pHeader->size; i++)
        {
            j = (char)*(pData + i);
            d += j;
        }
        return d;
    }
}
/****************************************************************
*
*   JUMP TO KEY
*
****************************************************************/
sqlm_header_info* db2dapi::jumpToKey(int element, char* pStart, GString& data)
{
    tm("Start jumpToKey");
    if( !pStart )
    {
        tm("pStart is NULL");
        return NULL;
    }
    sqlm_header_info * pHeader = (sqlm_header_info *)pStart;
    char * pData;
    GString elm, sec;
    data = "<NotFound>";
    char * pEnd = pStart + pHeader->size + sizeof(sqlm_header_info);

LabA:
    while ((char*)pHeader < pEnd)
    {
        if (pHeader->type == SQLM_TYPE_HEADER )
        {
            if( pHeader->element == element )
            {
                return pHeader;
            }
            else
            {
                pHeader++;
                goto LabA;
            }
        }
        else
        {
            pData = (char*)pHeader + sizeof(sqlm_header_info);
            data = getData(pHeader, pData);
            tm("Data: "+data+", header: "+GString(pHeader->element));
            if( pHeader->element == element )
            {
                tm("Found it as DATA: "+data);
                return pHeader;
            }
            pHeader = (sqlm_header_info *)(pData + pHeader->size);
        }
    }
    tm("End Jump, returning");
    return NULL;
}

/****************************************************************
*
*   PROCESS DATA
*
****************************************************************/
int db2dapi::processData(char * pStart, int type)
{

    tm("Start ProcData for type "+GString(type));
    GSeq <int> keySeq;
    GSeq <GString> nameSeq;
    int erc;
    tm("+++++++++++++++++++++++ BUFPTR +++++++++++++++++++++++++++");
    tm(pStart);
    tm("+++++++++++++++++++++++ BUFPTR DONE+++++++++++++++++++++++++++");

    clearSequences();

    switch( type )
    {
    case  SQLMA_DYNAMIC_SQL:
        erc = fillDSQL(pStart);
        break;

    case  SQLMA_DBASE_TABLES:
        erc = fillTabList(pStart);
        break;

    case  SQLMA_DBASE_LOCKS:
        erc = fillLock(pStart);
        break;

    case  SQLMA_DBASE_BUFFERPOOLS:
        keySeq.add(SQLM_ELM_POOL_DATA_L_READS);
        keySeq.add(SQLM_ELM_POOL_DATA_P_READS);
        keySeq.add(SQLM_ELM_POOL_DATA_WRITES);
        keySeq.add(SQLM_ELM_POOL_INDEX_L_READS);
        keySeq.add(SQLM_ELM_POOL_INDEX_P_READS);
        keySeq.add(SQLM_ELM_POOL_INDEX_WRITES);
        keySeq.add(SQLM_ELM_POOL_READ_TIME);
        keySeq.add(SQLM_ELM_POOL_WRITE_TIME);
        keySeq.add(SQLM_ELM_POOL_ASYNC_DATA_READS);
        keySeq.add(SQLM_ELM_POOL_ASYNC_DATA_WRITES);
        keySeq.add(SQLM_ELM_POOL_ASYNC_INDEX_WRITES);
        keySeq.add(SQLM_ELM_POOL_ASYNC_READ_TIME);
        keySeq.add(SQLM_ELM_POOL_ASYNC_WRITE_TIME);
        keySeq.add(SQLM_ELM_POOL_ASYNC_DATA_READ_REQS);
        keySeq.add(SQLM_ELM_DIRECT_READS);
        keySeq.add(SQLM_ELM_DIRECT_WRITES);
        keySeq.add(SQLM_ELM_DIRECT_READ_REQS);
        keySeq.add(SQLM_ELM_DIRECT_WRITE_REQS);
        keySeq.add(SQLM_ELM_DIRECT_READ_TIME);
        keySeq.add(SQLM_ELM_DIRECT_WRITE_TIME);
        keySeq.add(SQLM_ELM_POOL_ASYNC_INDEX_READS);
        keySeq.add(SQLM_ELM_POOL_DATA_TO_ESTORE);
        keySeq.add(SQLM_ELM_POOL_INDEX_TO_ESTORE);
        keySeq.add(SQLM_ELM_POOL_INDEX_FROM_ESTORE);
        keySeq.add(SQLM_ELM_POOL_DATA_FROM_ESTORE);
        keySeq.add(SQLM_ELM_UNREAD_PREFETCH_PAGES);
        keySeq.add(SQLM_ELM_FILES_CLOSED);
        keySeq.add(SQLM_ELM_BP_NAME);
        keySeq.add(SQLM_ELM_DB_NAME);
        keySeq.add(SQLM_ELM_DB_PATH);
        keySeq.add(SQLM_ELM_INPUT_DB_ALIAS);

        nameSeq.add("POOL_DATA_L_READS");
        nameSeq.add("POOL_DATA_P_READS");
        nameSeq.add("POOL_DATA_WRITES");
        nameSeq.add("POOL_INDEX_L_READS");
        nameSeq.add("POOL_INDEX_P_READS");
        nameSeq.add("POOL_INDEX_WRITES");
        nameSeq.add("POOL_READ_TIME");
        nameSeq.add("POOL_WRITE_TIME");
        nameSeq.add("POOL_ASYNC_DATA_READS");
        nameSeq.add("POOL_ASYNC_DATA_WRITES");
        nameSeq.add("POOL_ASYNC_INDEX_WRITES");
        nameSeq.add("POOL_ASYNC_READ_TIME");
        nameSeq.add("POOL_ASYNC_WRITE_TIME");
        nameSeq.add("POOL_ASYNC_DATA_READ_REQS");
        nameSeq.add("DIRECT_READS");
        nameSeq.add("DIRECT_WRITES");
        nameSeq.add("DIRECT_READ_REQS");
        nameSeq.add("DIRECT_WRITE_REQS");
        nameSeq.add("DIRECT_READ_TIME");
        nameSeq.add("DIRECT_WRITE_TIME");
        nameSeq.add("POOL_ASYNC_INDEX_READS");
        nameSeq.add("POOL_DATA_TO_ESTORE");
        nameSeq.add("POOL_INDEX_TO_ESTORE");
        nameSeq.add("POOL_INDEX_FROM_ESTORE");
        nameSeq.add("POOL_DATA_FROM_ESTORE");
        nameSeq.add("UNREAD_PREFETCH_PAGES");
        nameSeq.add("FILES_CLOSED");
        nameSeq.add("BP_NAME");
        nameSeq.add("DB_NAME");
        nameSeq.add("DB_PATH");
        nameSeq.add("INPUT_DB_ALIAS");
        erc = fillLV(pStart, SQLM_ELM_BUFFERPOOL, &keySeq, &nameSeq);
        break;

    case  SQLMA_DBASE:
        keySeq.add(SQLM_ELM_COMMIT_SQL_STMTS);
        keySeq.add(SQLM_ELM_ROLLBACK_SQL_STMTS);
        keySeq.add(SQLM_ELM_DYNAMIC_SQL_STMTS);
        keySeq.add(SQLM_ELM_STATIC_SQL_STMTS);
        keySeq.add(SQLM_ELM_FAILED_SQL_STMTS);

        keySeq.add(SQLM_ELM_ROWS_DELETED);
        keySeq.add(SQLM_ELM_ROWS_UPDATED);
        keySeq.add(SQLM_ELM_ROWS_INSERTED);
        keySeq.add(SQLM_ELM_ROWS_READ);
        keySeq.add(SQLM_ELM_ROWS_SELECTED);

        keySeq.add(SQLM_ELM_INT_COMMITS);
        keySeq.add(SQLM_ELM_INT_ROLLBACKS);

        keySeq.add(SQLM_ELM_TOTAL_SORTS);
        keySeq.add(SQLM_ELM_TOTAL_SORT_TIME);

        nameSeq.add("Statements COMMIT");
        nameSeq.add("Statements ROLLBACK");
        nameSeq.add("Statements DynSQL");
        nameSeq.add("Statements Stat.SQL");
        nameSeq.add("Failed SQL Stmts");
        nameSeq.add("Rows Deleted");
        nameSeq.add("Rows Updated");
        nameSeq.add("Rows Inserted");
        nameSeq.add("Rows Read");
        nameSeq.add("Rows Selected");
        nameSeq.add("Internal Commits");
        nameSeq.add("Internal Rollbacks");
        nameSeq.add("Sorts");
        nameSeq.add("Sort-time (total)");
        erc = fillLV(pStart, SQLM_ELM_DBASE, &keySeq, &nameSeq);
        break;

    case  SQLMA_DB2:
    case  SQLMA_DBASE_ALL:
        keySeq.add(SQLM_ELM_HADR_ROLE);
        keySeq.add(SQLM_ELM_HADR_STATE);
        keySeq.add(SQLM_ELM_HADR_SYNCMODE);
        keySeq.add(SQLM_ELM_HADR_CONNECT_STATUS);
        keySeq.add(SQLM_ELM_HADR_CONNECT_TIME);
        keySeq.add(SQLM_ELM_HADR_HEARTBEAT);
        keySeq.add(SQLM_ELM_HADR_LOCAL_HOST);
        keySeq.add(SQLM_ELM_HADR_LOCAL_SERVICE);
        keySeq.add(SQLM_ELM_HADR_REMOTE_HOST);
        keySeq.add(SQLM_ELM_HADR_REMOTE_SERVICE);
        keySeq.add(SQLM_ELM_HADR_TIMEOUT);
        keySeq.add(SQLM_ELM_HADR_PRIMARY_LOG_FILE);
        keySeq.add(SQLM_ELM_HADR_PRIMARY_LOG_PAGE);
        keySeq.add(SQLM_ELM_HADR_PRIMARY_LOG_LSN);
        keySeq.add(SQLM_ELM_HADR_STANDBY_LOG_FILE);
        keySeq.add(SQLM_ELM_HADR_STANDBY_LOG_PAGE);
        keySeq.add(SQLM_ELM_HADR_STANDBY_LOG_LSN);
        keySeq.add(SQLM_ELM_HADR_LOG_GAP);
        keySeq.add(SQLM_ELM_HADR_REMOTE_INSTANCE);
        keySeq.add(SQLM_ELM_HADR_PEER_WINDOW_END);
        keySeq.add(SQLM_ELM_HADR_PEER_WINDOW);
        keySeq.add(SQLM_ELM_BLOCKS_PENDING_CLEANUP);
        nameSeq.add("HADR_ROLE");
        nameSeq.add("HADR_STATE");
        nameSeq.add("HADR_SYNCMODE");
        nameSeq.add("HADR_CONNECT_STATUS");
        nameSeq.add("HADR_CONNECT_TIME");
        nameSeq.add("HADR_HEARTBEAT");
        nameSeq.add("HADR_LOCAL_HOST");
        nameSeq.add("HADR_LOCAL_SERVICE");
        nameSeq.add("HADR_REMOTE_HOST");
        nameSeq.add("HADR_REMOTE_SERVICE");
        nameSeq.add("HADR_TIMEOUT");
        nameSeq.add("HADR_PRIMARY_LOG_FILE");
        nameSeq.add("HADR_PRIMARY_LOG_PAGE");
        nameSeq.add("HADR_PRIMARY_LOG_LSN");
        nameSeq.add("HADR_STANDBY_LOG_FILE");
        nameSeq.add("HADR_STANDBY_LOG_PAGE");
        nameSeq.add("HADR_STANDBY_LOG_LSN");
        nameSeq.add("HADR_LOG_GAP");
        nameSeq.add("HADR_REMOTE_INSTANCE");
        nameSeq.add("HADR_PEER_WINDOW_END");
        nameSeq.add("HADR_PEER_WINDOW");
        nameSeq.add("BLOCKS_PENDING_CLEANUP");
        erc = fillLV(pStart, SQLM_ELM_HADR, &keySeq, &nameSeq);
    }


    return erc;
}
/****************************************************************
*
*   FILL LOCK
*
****************************************************************/
int db2dapi::fillLock(char * pStart)
{

    sqlm_header_info *pBlock, *pB1, *pB2;
    GString data;
    char * pData;
    tm("Call Jump...");



    headerSeq.add("Agent ID");
    headerSeq.add("Appl Name");
    headerSeq.add("Appl Status");
    headerSeq.add("Codepage");
    headerSeq.add("Locks Held");
    headerSeq.add("Locks Waiting");
    headerSeq.add("Lock Wait Time");
    headerSeq.add("Status Change Time");
    headerSeq.add("Appl ID");
    headerSeq.add("Sequence No");
    headerSeq.add("Auth ID");
    headerSeq.add("DB Alias");

    //Parse Datastream, find relevant position
    tm("fillLock, call Jump...");
    pBlock = jumpToKey(SQLM_ELM_DB_LOCK_LIST, pStart, data);
    if( !pBlock)
    {
        mb("No Monitor Info available, don't know why. Sorry.");
        return 0;
    }

    tm(" * * * * Size of pBlock1: "+GString(pBlock->size));

    char * pEnd = (char*) pBlock + pBlock->size + sizeof(sqlm_header_info);

    RowData * dataSeq;
    pData = (char*) pBlock;
    int i = 0;
    while( pData < pEnd )
    {
        tm("fillLock, in while");
        dataSeq = new RowData;

        pB1 = jumpToKey(SQLM_ELM_APPL_LOCK_LIST, pData, data);
        if( pB1 == NULL ) return 2;


        jumpToKey(SQLM_ELM_AGENT_ID, (char*)pB1, data);
        dataSeq->add(data);

        jumpToKey(SQLM_ELM_APPL_NAME, (char*)pB1, data);
        dataSeq->add(data);

        jumpToKey(SQLM_ELM_APPL_STATUS, (char*)pB1, data);
        dataSeq->add(data);

        jumpToKey(SQLM_ELM_CODEPAGE_ID, (char*)pB1, data);
        dataSeq->add(data);

        jumpToKey(SQLM_ELM_LOCKS_HELD, (char*)pB1, data);
        dataSeq->add(data);

        jumpToKey(SQLM_ELM_LOCKS_WAITING, (char*)pB1, data);
        dataSeq->add(data);

        jumpToKey(SQLM_ELM_LOCK_WAIT_TIME, (char*)pB1, data);
        dataSeq->add(data);


        //ExecTime is a nested Block
        GString t1, t2;
        pB2 = jumpToKey(SQLM_ELM_STATUS_CHANGE_TIME, (char*)pB1, data);
        tm("fillLock, in while2");
        jumpToKey(SQLM_ELM_SECONDS, (char*)pB2, t1);
        jumpToKey(SQLM_ELM_MICROSEC, (char*)pB2, t2);

        t2 = t1 + "." + t2.rightJustify(6, '0');
        dataSeq->add(t2);

        jumpToKey(SQLM_ELM_APPL_ID, (char*)pB1, data);
        dataSeq->add(data);

        jumpToKey(SQLM_ELM_SEQUENCE_NO, (char*)pB1, data);
        dataSeq->add(data);

        jumpToKey(SQLM_ELM_AUTH_ID, (char*)pB1, data);
        dataSeq->add(data);

        jumpToKey(SQLM_ELM_CLIENT_DB_ALIAS, (char*)pB1, data);
        dataSeq->add(data);
        tm("fillLock, in while3");
        //Get next block of type SQL_ELM_DYNSQL
        pData = (char*) pB1 + pB1->size + sizeof(sqlm_header_info);
        if( pData > pEnd ) break;

        rowSeq.add(dataSeq);
        //next row in QListWidget
        i++;
    }
    //for( int i = 0; i < pLV->rowCount(); ++i ) pLV->resizeRowToContents ( i ) ;
    return 0;
}

/****************************************************************
*
*   FILL DSQL
*
****************************************************************/
int db2dapi::fillDSQL(char * pStart)
{

    sqlm_header_info *pBlock, *pB1, *pB2;
    GString data;
    char * pData;
    int64_t executions;
    tm("Call Jump...");

    headerSeq.add("SQLStatement");
    headerSeq.add("Executions");
    headerSeq.add("Exec Time Total");
    headerSeq.add("Time/Stmt (MicroSec)");

    headerSeq.add("Rows Read");
    headerSeq.add("Rows Written");
    headerSeq.add("Rows CascadeDelete");
    headerSeq.add("Rows Inserted");
    headerSeq.add("Rows SetNullDeletes");

    headerSeq.add("Compilations");
    headerSeq.add("PrepTime Worst");
    headerSeq.add("PrepTime Best");
    headerSeq.add("Stmt Sorts");

    //Parse Datastream, find relevant position
    pBlock = jumpToKey(SQLM_ELM_DYNSQL_LIST, pStart, data);
    if( !pBlock)
    {
        mb("No Monitor Info available, don't know why. Sorry.");
        return 0;
    }

    tm(" * * * * Size of pBlock1: "+GString(pBlock->size));

    char * pEnd = (char*) pBlock + pBlock->size + sizeof(sqlm_header_info);

    RowData *dataSeq;
    pData = (char*) pBlock;
    int i = 0;
    while( pData < pEnd )
    {


        pB1 = jumpToKey(SQLM_ELM_DYNSQL, pData, data);
        if( pB1 == NULL ) return 2;

        jumpToKey(SQLM_ELM_NUM_EXECUTIONS, (char*)pB1, data);
        executions = data.asLongLong();
        if( executions == 0 )
        {
            pData = (char*) pB1 + pB1->size + sizeof(sqlm_header_info);
            if( pData > pEnd ) break;
            else continue;
        }

        dataSeq = new RowData;

        jumpToKey(SQLM_ELM_STMT_TEXT, (char*)pB1, data);
        dataSeq->add(data);

        jumpToKey(SQLM_ELM_NUM_EXECUTIONS, (char*)pB1, data);
        dataSeq->add(data);
        executions = data.asLongLong();

        //ExecTime is a nested Block
        GString t1, t2;
        pB2 = jumpToKey(SQLM_ELM_TOTAL_EXEC_TIME, (char*)pB1, data);
        jumpToKey(SQLM_ELM_SECONDS, (char*)pB2, t1);
        //      jumpToKey(SQLM_ELM_SS_EXEC_TIME, (char*)pB2, t1);
        jumpToKey(SQLM_ELM_MICROSEC, (char*)pB2, t2);
        dataSeq->add(t1 + "." + t2.rightJustify(6, '0'));
        if( executions > 0 )
        {
            int64_t timeInSec = (t1+t2).asLongLong();
            dataSeq->add(timeInSec / executions);
        }
        else dataSeq->add("N/A");



        jumpToKey(SQLM_ELM_ROWS_READ, (char*)pB1, data);
        dataSeq->add(data);

        jumpToKey(SQLM_ELM_ROWS_WRITTEN, (char*)pB1, data);
        dataSeq->add(data);


        jumpToKey(SQLM_ELM_INT_ROWS_DELETED, (char*)pB1, data);
        dataSeq->add(data);
        jumpToKey(SQLM_ELM_INT_ROWS_INSERTED, (char*)pB1, data);
        dataSeq->add(data);
        jumpToKey(SQLM_ELM_INT_ROWS_UPDATED, (char*)pB1, data);
        dataSeq->add(data);


        jumpToKey(SQLM_ELM_NUM_COMPILATIONS, (char*)pB1, data);
        dataSeq->add(data);
        jumpToKey(SQLM_ELM_PREP_TIME_WORST, (char*)pB1, data);
        dataSeq->add(data);
        jumpToKey(SQLM_ELM_PREP_TIME_BEST, (char*)pB1, data);
        dataSeq->add(data);
        jumpToKey(SQLM_ELM_STMT_SORTS, (char*)pB1, data);
        dataSeq->add(data);

        //Get next block of type SQL_ELM_DYNSQL
        pData = (char*) pB1 + pB1->size + sizeof(sqlm_header_info);
        if( pData > pEnd ) break;
        i++;
        rowSeq.add(dataSeq);
    }

    return 0;
}


/****************************************************************
*
*   FILL TAB LIST
*
****************************************************************/
int db2dapi::fillTabList(char * pStart)
{
    tm("Start fillTabList");
    sqlm_header_info *pBlock, *pB1;
    GString data;
    char * pData;


    tm("Adding cols...");
    headerSeq.add("TabSchema");
    headerSeq.add("Table Name");
    headerSeq.add("Rows Written");
    headerSeq.add("Rows Read");
//    headerSeq.add("Rows Inserted");
    headerSeq.add("Overflow Accesses");
    headerSeq.add("Table FileID");
    headerSeq.add("Table Type");
    headerSeq.add("Page Reorgs");
    if( pStart == NULL )
    {
        tm("DataStream is NULL");
        return 1;
    }
    else tm("Calling Jump...");
    pBlock = jumpToKey(SQLM_ELM_TABLE_LIST, pStart, data);
    if( !pBlock)
    {
        mb("No Monitor Info available, don't know why. Sorry.");
        return 0;
    }

    char * pEnd = (char*) pBlock + pBlock->size + sizeof(sqlm_header_info);
    pData = (char*) pBlock;

    RowData *dataSeq;
    while( pData < pEnd )
    {
        pB1 = jumpToKey(SQLM_ELM_TABLE, pData, data);
        if( pB1 == NULL )
        {
            tm("SQL_ELM_TABLE: No monitor-info available.");
            mb("No events since monitor was started.");
            return 2;
        }
        dataSeq = new RowData;

        jumpToKey(SQLM_ELM_TABLE_SCHEMA, (char*)pB1, data);
        dataSeq->add(data);

        jumpToKey(SQLM_ELM_TABLE_NAME, (char*)pB1, data);
        dataSeq->add(data);

        jumpToKey(SQLM_ELM_ROWS_WRITTEN, (char*)pB1, data);
        dataSeq->add(data);

        jumpToKey(SQLM_ELM_ROWS_READ, (char*)pB1, data);
        dataSeq->add(data);

        jumpToKey(SQLM_ELM_OVERFLOW_ACCESSES, (char*)pB1, data);
        dataSeq->add(data);
        jumpToKey(SQLM_ELM_TABLE_FILE_ID, (char*)pB1, data);
        dataSeq->add(data);
        jumpToKey(SQLM_ELM_TABLE_TYPE, (char*)pB1, data);
        dataSeq->add(data);

        jumpToKey(SQLM_ELM_PAGE_REORGS, (char*)pB1, data);
        dataSeq->add(data);

        pData = (char*) pB1 + pB1->size + sizeof(sqlm_header_info);
        if( pData > pEnd ) {tm("Done");break;}

        rowSeq.add(dataSeq);
    }
    //for( int i = 0; i < pLV->rowCount(); ++i ) pLV->resizeRowToContents ( i ) ;
    return 0;
}

/****************************************************************
*
*   FILL LV
*
****************************************************************/
int db2dapi::fillLV(char * pStart, int key, GSeq <int> * keySeq, GSeq <GString> * nameSeq)
{

    tm("Start fillLV...");
    sqlm_header_info *pBlock;
    GString data;
    tm("Call Jump...");


    headerSeq.add("Key");
    headerSeq.add("Data");

    char * pEnd;// = pStart + pHeader->size + sizeof(sqlm_header_info);
    tm("fillLV, pos1");
    pBlock = jumpToKey(key, pStart, data);
    if( !pBlock)
    {
        mb("No Monitor Info available, don't know why. Sorry.");
        return 0;
    }

    RowData * pRowData;
    tm(" * * * * Size of pBlock1: "+GString(pBlock->size));
    pEnd = (char*) pBlock + pBlock->size + sizeof(sqlm_header_info);

    tm("fillLV, pos2");
    char* pData = (char*) pBlock;
    for( unsigned int i = 1; i <= keySeq->numberOfElements(); ++i )
    {
        if( pData > pEnd ) break;
        pRowData = new RowData;

        jumpToKey(keySeq->elementAtPosition(i), (char*)pBlock, data);
        if( data.length() )
        {

            pRowData->add(nameSeq->elementAtPosition(i));
            pRowData->add(data);
        }
        rowSeq.add(pRowData);
    }
    tm("Done");
    return 0;
}


/****************************************************************
*
*   TABLESPACES, main
*
****************************************************************/
int db2dapi::initTabSpaceLV(GSeq <TAB_SPACE*> *dataSeq)
{
    tm("initTabSpaceLV 0");

    //struct sqlca sqlca;
    struct SQLB_TBSPQRY_DATA *dataP;
    sqluint32 numTS, maxTS;

    sqlbotsq (&sqlca, SQLB_OPEN_TBS_ALL, &numTS);
    maxTS = numTS;
    dataP = (struct SQLB_TBSPQRY_DATA *) malloc (numTS *sizeof (struct SQLB_TBSPQRY_DATA));

    memcpy(dataP->tbspqver,SQLB_TBSPQRY_DATA_ID, 8);

    sqlbftpq (&sqlca, maxTS, dataP, &numTS);
    fillTabSpaceLV (dataP, numTS, dataSeq);

    sqlbctsq (&sqlca);
    return 0;
}
/****************************************************************
*
*   TABLESPACES, fill
*
****************************************************************/
void db2dapi::fillTabSpaceLV (struct SQLB_TBSPQRY_DATA *dataP, sqluint32 num, GSeq <TAB_SPACE*> *dataSeq)
{
    tm("initTabSpaceLV in fillTabSpaceLV");
    //struct sqlca sqlca;
    struct SQLB_TBS_STATS tbs_stats;
    sqluint32 idx;

    GString val;
    tm("initTabSpaceLV in fillTabSpaceLV 1");

    TAB_SPACE * tabSpace;
    for (idx=0; idx < num; idx++, dataP++)
    {
        tm("initTabSpaceLV in fillTabSpaceLV 2");
        tabSpace = new TAB_SPACE;
        tabSpace->Id = GString(dataP->id);
        tabSpace->Name = GString(dataP->name);

        /* "Type" and "Content" are stored bitwise in the flag field */
        tm("initTabSpaceLV in fillTabSpaceLV3" );
        switch (dataP->flags & 0xF )
        {

        case SQLB_TBS_SMS:
            tabSpace->Type = "SMS";
            break;
        case SQLB_TBS_DMS:
            tabSpace->Type = "DMS";
            break;
        default:
            tabSpace->Type = "UNKNOWN";
            break;
        }
        tm("initTabSpaceLV in fillTabSpaceLV4");
        switch (dataP->flags & 0xF0)
        {
        case SQLB_TBS_ANY:
            tabSpace->Contents = "Regular contents";
            break;
        case SQLB_TBS_LONG:
            tabSpace->Contents = "Long field data";
            break;
        case SQLB_TBS_TMP:
            tabSpace->Contents = "Temp data";
            break;
        default:
            tabSpace->Contents = "UNKNOWN TYPE";
            break;
        } /* endswitch */
        tm("initTabSpaceLV in fillTabSpaceLV 5");
        switch (dataP->tbsState)
        {
        case SQLB_NORMAL:
            tabSpace->State = "Normal";
            break;
        case SQLB_QUIESCED_SHARE:
            tabSpace->State = "Quiesced: SHARE";
            break;
        case SQLB_QUIESCED_UPDATE:
            tabSpace->State = "Quiesced: UPDATE";
            break;
        case SQLB_QUIESCED_EXCLUSIVE:
            tabSpace->State = "Quiesced: EXCLUSIVE";
            break;
        case SQLB_LOAD_PENDING:
            tabSpace->State = "LOAD pending";
            break;
        case SQLB_DELETE_PENDING:
            tabSpace->State = "DELETE pending";
            break;
        case SQLB_BACKUP_PENDING:
            tabSpace->State = "BACKUP pending";
            break;
        case SQLB_ROLLFORWARD_IN_PROGRESS:
            tabSpace->State = "ROLLFORWARD in progress";
            break;
        case SQLB_ROLLFORWARD_PENDING:
            tabSpace->State = "ROLLFORWARD pending";
            break;
        case SQLB_RESTORE_PENDING:
            tabSpace->State = "RESTORE pending";
            break;
        case SQLB_DISABLE_PENDING:
            tabSpace->State = "DISABLE pending";
            break;
        case SQLB_REORG_IN_PROGRESS:
            tabSpace->State = "REORG in progress";
            break;
        case SQLB_BACKUP_IN_PROGRESS:
            tabSpace->State = "BACKUP in progress";
            break;
        case SQLB_STORDEF_PENDING:
            tabSpace->State = "Storage must be defined (pending)";
            break;
        case SQLB_RESTORE_IN_PROGRESS:
            tabSpace->State = "RESTORE in progress";
            break;
        case SQLB_STORDEF_ALLOWED:
            tabSpace->State = "STORAGE may be defined (allowed)";
            break;
        case SQLB_STORDEF_FINAL_VERSION:
            tabSpace->State = "STORDEF is in final";
            break;
        case SQLB_STORDEF_CHANGED:
            tabSpace->State = "STORDEF was changed prior to rollforward";
            break;
        case SQLB_REBAL_IN_PROGRESS:
            tabSpace->State = "DMS-Rebalancer is active";
            break;
        case SQLB_PSTAT_DELETION:
            tabSpace->State = "TBS deletion in progress";
            break;
        case SQLB_PSTAT_CREATION:
            tabSpace->State = "TBS creation in progress";
            break;
        default:
            tabSpace->State = "UNKNOWN";
            break;
        }
        tm("initTabSpaceLV in fillTabSpaceLV 6");
        sqlbgtss(&sqlca, dataP->id, &tbs_stats);

        tm("initTabSpaceLV in fillTabSpaceLV 7");

        tabSpace->TotalPages = GString(tbs_stats.totalPages);
        tabSpace->UsablePages = GString(tbs_stats.useablePages);
        tabSpace->UsedPages = GString(tbs_stats.usedPages);

        tabSpace->FreePages = GString(tbs_stats.freePages);

        tabSpace->HighWaterMark = GString(tbs_stats.highWaterMark);
        tm("initTabSpaceLV in fillTabSpaceLV10");

        dataSeq->add(tabSpace);
    }
}
#endif 


int db2dapi::importTable(GString dataFile, GSeq<GString> *pathSeq, GString format, GString statement, GString msgFile, GString modifierString)
{
    tm("Start Import...");
    if( pathSeq->numberOfElements() == 0 ) return -1;
     char temp[256];

    tm("Importing "+dataFile+", paths: "+GString(pathSeq->numberOfElements())+", path0: "+pathSeq->elementAtPosition(1)+", format: "+format+" stmt: "+statement+", msgFile: "+msgFile+", modifierString: "+modifierString);
    struct sqldcol       columnData;
    struct sqlchar       *columnStringPointer;

    struct sqluimpt_in   impInput;
    struct sqluimpt_out  impOutput;

    struct sqlu_media_list *pLobPathList;
    //struct sqlu_media_list *pLobFileList;
    struct sqlchar         *fileTypeMod;


    impInput.sizeOfStruct = SQLUIMPT_IN_SIZE;
    impOutput.sizeOfStruct = SQLUIMPT_OUT_SIZE;
    impInput.restartcnt = 0;
    impInput.commitcnt = 1000;

    columnStringPointer = (struct sqlchar *)malloc(strlen(statement)+sizeof (struct sqlchar));
    columnStringPointer->length = strlen(statement);
    strncpy (columnStringPointer->data, statement, strlen(statement));
    columnData.dcolmeth = 'D';
    tm("allocating lobFileStruct...");

    fileTypeMod = (struct sqlchar *)malloc(modifierString.length() + sizeof (short) + 1);
    fileTypeMod->length = modifierString.length();
    strncpy (fileTypeMod->data, modifierString, fileTypeMod->length);

    tm("Modifier: "+modifierString);



    /*
   if( modifier == 1 )
   {
      //fileTypeMod = (struct sqlchar *)malloc(strlen("lobsinfile compound=100") + sizeof (struct sqlchar));
      //fileTypeMod->length = strlen("lobsinfile compound=100");
      //strncpy (fileTypeMod->data, "lobsinfile compound=100", fileTypeMod->length);

      fileTypeMod = (struct sqlchar *)malloc(strlen("lobsinfile") + sizeof (struct sqlchar));
      fileTypeMod->length = strlen("lobsinfile");
      strncpy (fileTypeMod->data, "lobsinfile", fileTypeMod->length);
   }
   else if( modifier == 2 )
   {
      fileTypeMod = (struct sqlchar *)malloc(strlen("identityignore") + sizeof (struct sqlchar));
      fileTypeMod->length = strlen("identityignore");
      strncpy (fileTypeMod->data, "identityignore", fileTypeMod->length);
   }
   else
   {
      fileTypeMod = (struct sqlchar *)malloc(strlen("FORCECREATE") + sizeof (struct sqlchar));
      fileTypeMod->length = strlen("FORCECREATE");
      strncpy (fileTypeMod->data, "FORCECREATE", fileTypeMod->length);
   }
   */
    tm("Allocating paths...");
    pLobPathList = (sqlu_media_list *) malloc (sizeof(sqlu_media_list));
    pLobPathList->media_type = SQLU_LOCAL_MEDIA;
    pLobPathList->sessions = pathSeq->numberOfElements();
    pLobPathList->target.media = (sqlu_media_entry *) malloc(sizeof(sqlu_media_entry) * pLobPathList->sessions);
    for( int i = 1; i <= (int)pathSeq->numberOfElements(); ++i )
    {
        strcpy (pLobPathList->target.media[i-1].media_entry, pathSeq->elementAtPosition(i) );
    }

    ///remove((char*) msgFile); //delete old msgFile
    tm("Starting import, fileTypeMod: "+GString(fileTypeMod->data));

    sqluimpr (dataFile, pLobPathList, &columnData, columnStringPointer, format,
              fileTypeMod, msgFile, 0, &impInput, &impOutput, NULL, NULL, &sqlca);
    tm("SQLCODE from sqluimpr: "+GString(sqlca.sqlcode));


    tm("Freeing stuff...");
    free(fileTypeMod);
    free(pLobPathList->target.media);
    free(pLobPathList);
    free(columnStringPointer);

    tm("Import Done.");
    return sqlca.sqlcode;
}
#ifdef SQLM_DBMON_VERSION8
int db2dapi::importTableNew(GString dataFile, GSeq<GString> *pathSeq, GString format, GString statement, GString msgFile, GString modifierString)
{
    tm("Importing NEW API: "+dataFile+", paths: "+GString(pathSeq->numberOfElements())+", path0: "+pathSeq->elementAtPosition(1)+", format: "+format+" stmt: "+statement+", msgFile: "+msgFile+",  modifierString: "+modifierString);
    struct db2ImportStruct importParmStruct;
    struct sqlu_media_list *pLobPathList;
    struct sqldcol dataDescriptor;
    struct sqlchar* pActionString;
    struct sqlchar *pFileTypeMod;
    struct db2ImportIn inputInfo;
    struct db2ImportOut outputInfo;
    struct sqlu_media_list *pXmlFileList;
    int commitcount = 1000;

    memset(&importParmStruct, '\0', sizeof(db2ImportStruct));


    tm("Allocating XmlPaths...");
    pXmlFileList = (sqlu_media_list *) malloc (sizeof(sqlu_media_list));
    pXmlFileList->media_type = SQLU_LOCAL_MEDIA;
    //pLobPathList->media_type = SQLU_SERVER_LOCATION;
    pXmlFileList->sessions = pathSeq->numberOfElements();
    pXmlFileList->target.media = (sqlu_media_entry *) malloc(sizeof(sqlu_media_entry) * pXmlFileList->sessions);
    for( int i = 1; i <= (int)pathSeq->numberOfElements(); ++i )
    {
        strcpy (pXmlFileList->target.media[i-1].media_entry, pathSeq->elementAtPosition(i) );
    }

    tm("LobPaths...");
    pLobPathList = (sqlu_media_list *) malloc (sizeof(sqlu_media_list));
    pLobPathList->media_type = SQLU_LOCAL_MEDIA;
    pLobPathList->sessions = pathSeq->numberOfElements();
    pLobPathList->target.media = (sqlu_media_entry *) malloc(sizeof(sqlu_media_entry) * pLobPathList->sessions);
    for( int i = 1; i <= (int)pathSeq->numberOfElements(); ++i )
    {
        strcpy (pLobPathList->target.media[i-1].media_entry, pathSeq->elementAtPosition(i) );
    }
    dataDescriptor.dcolmeth = 'D';

    tm("ActionString...");
    pActionString = (struct sqlchar *)malloc(statement.length()+sizeof (short)+1);
    pActionString->length = statement.length();
    strncpy (pActionString->data, statement, statement.length());
    pActionString->data[statement.length()] = 0;
    tm("ActionString: "+GString(pActionString->data));

    tm("FileTypeMod for modifier "+modifierString);
    modifierString = modifierString.strip();
    pFileTypeMod = (struct sqlchar *)malloc(modifierString.length() + sizeof (short) + 1);
    pFileTypeMod->length = modifierString.length();
    strcpy (pFileTypeMod->data, modifierString);
    pFileTypeMod->data[modifierString.length()] = 0;
    tm("FileTypeMod: "+GString(pFileTypeMod->data)+", length: "+GString(pFileTypeMod->length));

    tm("InputInfo...");
    inputInfo.iRowcount = inputInfo.iRestartcount = 0;
    inputInfo.iSkipcount = inputInfo.iWarningcount = 0;
    inputInfo.iNoTimeout = 0;
    inputInfo.iAccessLevel = SQLU_ALLOW_NO_ACCESS;
    inputInfo.piCommitcount = (db2int32*)&commitcount;
    inputInfo.piXmlParse =  NULL;
    inputInfo.piXmlValidate = NULL;

    tm("Setting importParmStruct...");
    importParmStruct.piDataFileName    = (char*) dataFile;
    importParmStruct.piDataDescriptor  = &dataDescriptor;
    importParmStruct.piLobPathList     = pLobPathList;
    importParmStruct.piActionString    = pActionString;
    importParmStruct.piFileType        = (char*)format;
    importParmStruct.piMsgFileName     = (char*)msgFile;
    importParmStruct.iCallerAction     = SQLU_INITIAL;
    importParmStruct.piImportInfoIn    = &inputInfo;
    importParmStruct.poImportInfoOut   = &outputInfo;
    importParmStruct.piNullIndicators  = NULL;
    importParmStruct.piLongActionString = NULL; //pAction;
    importParmStruct.piXmlPathList     = pXmlFileList;
    importParmStruct.piFileTypeMod     = pFileTypeMod;

    tm("Calling import...");
    db2Import ( db2Version970, &importParmStruct, &sqlca );
    tm("Calling import...Done");

    free(pLobPathList);
    free(pXmlFileList);
    free(pActionString);
    free(pFileTypeMod);
    tm("Freeing done, erc: "+GString(sqlca.sqlcode));
    return sqlca.sqlcode;

//    tm("Importing NEW API: "+dataFile+", paths: "+GString(pathSeq->numberOfElements())+", path0: "+pathSeq->elementAtPosition(1)+", format: "+format+" stmt: "+statement+", msgFile: "+msgFile+", identityModifier: "+GString(identityModifier)+", modifierString: "+modifierString);
//    struct sqlca sqlca;
//    struct sqldcol dataDescriptor;
//    struct sqlchar *pAction;
//    char msgFileName[128];
//    struct db2ImportIn inputInfo;
//    struct db2ImportOut outputInfo;
//    struct db2ImportStruct importParmStruct;
//    int commitcount = 10;
//    int whiteSpace = 1;
//    unsigned short xmlParse = whiteSpace;
//    struct sqlchar *fileTypeMod = NULL;
//    struct sqlu_media_entry *pPathList;
//    union sqlu_media_list_targets listTargetsXmlPath;
//    struct sqlu_media_list mediaListXmlPath;
//    struct db2DMUXmlValidate xmlValidate;
//    struct db2DMUXmlValidateXds xdsArgs;
//    struct db2Char defaultSchema, ignoreSchemas;
//    struct db2DMUXmlMapSchema mapSchemas;
//    struct db2Char mapFromSchema;
//    struct db2Char mapToSchema;

//    /* import table */
//    dataDescriptor.dcolmeth = SQL_METH_D;
//    pAction = (struct sqlchar *)malloc(sizeof(short) + statement.length() + 1);
//    pAction->length = statement.length();
//    strcpy(pAction->data, (char*) statement);
//    strcpy(msgFileName, "tbimport.MSG");

//    /* Setup db2ImportIn structure */

//    /* XML Path setup */
//    pPathList=(struct sqlu_media_entry *)malloc(sizeof(struct sqlu_media_entry));
//#if(defined(DB2NT))
//    sprintf(pPathList->media_entry, "%s%sxmldatadir", getenv("DB2PATH"), PATH_SEP);
//#else /* UNIX */
//    sprintf(pPathList->media_entry, "%s%sxmldatadir", getenv("HOME"), PATH_SEP);
//#endif

//    listTargetsXmlPath.media = pPathList;
//    mediaListXmlPath.media_type = 'L';
//    mediaListXmlPath.sessions = 1;
//    mediaListXmlPath.target = listTargetsXmlPath;

//    /* File Type Modifier for Import Utility */
////    strcpy(temp,"XMLCHAR");
////    fileTypeMod = (struct sqlchar *) malloc(sizeof(short) + sizeof (temp) + 1);
////    fileTypeMod->length = strlen(temp);
////    strcpy(fileTypeMod->data,temp);

//    fileTypeMod = (struct sqlchar *)malloc(modifierString.length() + sizeof (struct sqlchar));
//    fileTypeMod->length = modifierString.length();
//    strncpy (fileTypeMod->data, modifierString, fileTypeMod->length);

//    struct sqlu_media_list *pLobPathList;
//    pLobPathList = (sqlu_media_list *) malloc (sizeof(sqlu_media_list));
//    pLobPathList->media_type = SQLU_LOCAL_MEDIA;
//    pLobPathList->sessions = pathSeq->numberOfElements();
//    pLobPathList->target.media = (sqlu_media_entry *) malloc(sizeof(sqlu_media_entry) * pLobPathList->sessions);
//    for( int i = 1; i <= (int)pathSeq->numberOfElements(); ++i )
//    {
//        strcpy (pLobPathList->target.media[i-1].media_entry, pathSeq->elementAtPosition(i) );
//    }


//    /* XML validate using XDS set up */
//    defaultSchema.iLength = 8;
//    defaultSchema.pioData=(char*)malloc(9);
//    strcpy(defaultSchema.pioData,"customer");
//    ignoreSchemas.iLength = 8;
//    ignoreSchemas.pioData=(char*)malloc(9);
//    strcpy(ignoreSchemas.pioData,"supplier");
//    mapFromSchema.iLength = 7;
//    mapFromSchema.pioData=(char*)malloc(8);
//    strcpy(mapFromSchema.pioData,"product");
//    mapToSchema.iLength = 8;
//    mapToSchema.pioData=(char*)malloc(9);
//    strcpy(mapToSchema.pioData,"customer");
//    mapSchemas.iMapFromSchema = mapFromSchema;
//    mapSchemas.iMapToSchema = mapToSchema;
//    xdsArgs.piDefaultSchema = &defaultSchema;
//    xdsArgs.iNumIgnoreSchemas = 1;
//    xdsArgs.piIgnoreSchemas =&ignoreSchemas;
//    xdsArgs.iNumMapSchemas = 1;
//    xdsArgs.piMapSchemas = &mapSchemas;
//    xmlValidate.iUsing = DB2DMU_XMLVAL_XDS;
//    xmlValidate.piXdsArgs =&xdsArgs;

//    inputInfo.iRowcount = inputInfo.iRestartcount = 0;
//    inputInfo.iSkipcount = inputInfo.iWarningcount = 0;
//    inputInfo.iNoTimeout = 0;
//    inputInfo.iAccessLevel = SQLU_ALLOW_NO_ACCESS;
//    inputInfo.piCommitcount = (db2int32*)&commitcount;
//    inputInfo.piXmlParse =  &xmlParse;
//    inputInfo.piXmlValidate = &xmlValidate;

//    printf("\n  Import table.\n");
//    printf("    client source file name: %s\n", (char*)dataFile);
//    printf("    client message file name: %s\n", (char*)msgFile);

//    importParmStruct.piFileType        = (char*)format;
//    importParmStruct.piFileTypeMod     = fileTypeMod;
//    importParmStruct.piDataFileName    = (char*)dataFile;
//    importParmStruct.piLobPathList     = pLobPathList;
//    importParmStruct.piDataDescriptor  = &dataDescriptor;
//    importParmStruct.piActionString    = pAction;
//    importParmStruct.piMsgFileName     = (char*)msgFile;
//    importParmStruct.piImportInfoIn    = &inputInfo;
//    importParmStruct.poImportInfoOut   = &outputInfo;
//    importParmStruct.piNullIndicators  = NULL;
//    importParmStruct.iCallerAction     = SQLU_INITIAL;
//    importParmStruct.piXmlPathList     = &mediaListXmlPath;

//    printf("\n-----------------------------------------------------------");
//    printf("\nUSE THE DB2 API:\n");
//    printf("  db2Import -- Import\n");
//    printf("TO IMPORT DATA TO A FILE.\n");

//    int rc = db2Import(db2Version970,
//              &importParmStruct,
//              &sqlca);
//    printf("db2Import rc: "+GString(rc)+", err: "+SQLError());

//    /* free memory allocated */
//    free(pAction);
//    free(pPathList);
//    free(fileTypeMod);
//    free(defaultSchema.pioData);
//    free(ignoreSchemas.pioData);
//    free(mapFromSchema.pioData);
//    free(mapToSchema.pioData);

//    /* display import info */
//    printf("\n  Import info.\n");
//    printf("    rows read     : %ld\n", (int)outputInfo.oRowsRead);
//    printf("    rows skipped  : %ld\n", (int)outputInfo.oRowsSkipped);
//    printf("    rows inserted : %ld\n", (int)outputInfo.oRowsInserted);
//    printf("    rows updated  : %ld\n", (int)outputInfo.oRowsUpdated);
//    printf("    rows rejected : %ld\n", (int)outputInfo.oRowsRejected);
//    printf("    rows committed: %ld\n", (int)outputInfo.oRowsCommitted);
//    return 0;
}
#else
int db2dapi::importTableNew(GString dataFile, GSeq<GString> *pathSeq, GString format, GString statement, GString msgFile, GString modifierString)
{
    tm("db2dapi::importTableNew: Falling back to db2dapi::importTable(...)");
    return importTable(dataFile, pathSeq, format, statement, msgFile, modifierString);
}
#endif
/*************************************************************/
/********************* OLD EXPORT ****************************/
/*************************************************************/

int db2dapi::exportTable(GString dataFile, GString format, GString statement, GString msgFile, GString modified)
{
    //efine  STMTLEN 120

    struct sqldcol       columnData;
    struct sqlchar       *columnStringPointer;
    struct sqluexpt_out  outputInfo;
    struct sqlchar         *fileTypeMod;

    fileTypeMod = (struct sqlchar *)malloc(modified.length() + sizeof (struct sqlchar));
    fileTypeMod->length = modified.length();
    strncpy (fileTypeMod->data, modified, fileTypeMod->length);

    /* need to preset the size of structure field and counts */
    outputInfo.sizeOfStruct = SQLUEXPT_OUT_SIZE;

    columnStringPointer = (struct sqlchar *)malloc(statement.length()
                                                   + sizeof (struct sqlchar));

    columnData.dcolmeth = 'D';

    columnStringPointer->length = strlen(statement);
    strncpy (columnStringPointer->data, statement, strlen(statement));

    remove((char*) msgFile); //delete old msgFile
    sqluexpr (dataFile, NULL, NULL, &columnData, columnStringPointer,
              format, fileTypeMod, msgFile, 0, &outputInfo, NULL, &sqlca);


    free(columnStringPointer);
    free(fileTypeMod);

    if (sqlca.sqlcode != 0) return sqlca.sqlcode;
    remove((char*) msgFile);
    return 0;

}
/*************************************************************/
/********************* NEW EXPORT ****************************/
/*************************************************************/

int db2dapi::exportTable(GString format, GString statement,
                         GString msgFile, GSeq<GString> *pathSeq, GString dataFile, int sessions, GString modified )
{
    tm("Start Export...");
    if( pathSeq->numberOfElements() == 0 ) return -1;
    tm("PathCount: "+GString(pathSeq->numberOfElements())+", firstPath: "+pathSeq->elementAtPosition(1)+", dataFile: "+dataFile+", sessions: "+GString(sessions)+", modfied: "+modified);

    sessions = sessions * 2 + 1;
    //sessions += 1;

    struct sqldcol       columnData;
    struct sqlchar       *columnStringPointer;
    struct sqluexpt_out  outputInfo;
    struct sqluimpt_in   impInput;
    struct sqlu_media_list *pLobPathList;
    struct sqlu_media_list *pLobFileList;
    struct sqlchar         *fileTypeMod;

    outputInfo.sizeOfStruct = SQLUEXPT_OUT_SIZE;
    impInput.sizeOfStruct = SQLUIMPT_IN_SIZE;
    impInput.restartcnt = impInput.commitcnt = 0;
    tm("Allocating...");
    columnStringPointer = (struct sqlchar *)malloc(statement.length() + sizeof (struct sqlchar));

    //   columnData.dcolmeth = 'D';
    columnData.dcolmeth = SQL_METH_D;
    if( sessions > 0 ) //EXPORT BLOBs
    {
        modified = "lobsinfile "+modified;
    }
    fileTypeMod = (struct sqlchar *)malloc(modified.length() + sizeof (struct sqlchar));
    fileTypeMod->length = modified.length();
    strncpy (fileTypeMod->data, modified, fileTypeMod->length);

    tm("Allocating paths...");
    pLobPathList = (sqlu_media_list *) malloc (sizeof(sqlu_media_list));
    pLobPathList->media_type = SQLU_LOCAL_MEDIA;
    //pLobPathList->media_type = SQLU_SERVER_LOCATION;
    pLobPathList->sessions = pathSeq->numberOfElements();
    pLobPathList->target.media = (sqlu_media_entry *) malloc(sizeof(sqlu_media_entry) * pLobPathList->sessions);
    for( int i = 1; i <= (int)pathSeq->numberOfElements(); ++i )
    {
        strcpy (pLobPathList->target.media[i-1].media_entry, pathSeq->elementAtPosition(i) );
        printf("Path %i: %s\n", i, pLobPathList->target.media[i-1].media_entry);
    }
    /*
   tm("Allocating paths...");
   pLobPathList = (sqlu_media_list *) malloc (sizeof(sqlu_media_list));
   pLobPathList->media_type = SQLU_LOCAL_MEDIA;
   pLobPathList->sessions = 1;
   pLobPathList->target.media = (sqlu_media_entry *) malloc(sizeof(sqlu_media_entry) * pLobPathList->sessions);
   strcpy (pLobPathList->target.media[0].media_entry, path );
   */
    tm("Allocating files...");
    /******** FILES *****************/
    pLobFileList = (sqlu_media_list *) malloc (sizeof(sqlu_media_list));
    pLobFileList->media_type = SQLU_CLIENT_LOCATION;
    pLobFileList->sessions = sessions;

    pLobFileList->target.location = (sqlu_location_entry *) malloc
            (sizeof(sqlu_location_entry) * pLobFileList->sessions);

    GString fName;
    short k;
    tm("Adding to list...");
    for( k = 0; k < sessions; ++ k )
    {
        fName = dataFile+GString(k);
        strcpy (pLobFileList->target.location[k].location_entry, fName );
    }
    tm("Preparing stmt...");
    columnStringPointer->length = strlen(statement);
    strncpy (columnStringPointer->data, statement, strlen(statement));
    //remove((char*) msgFile); //delete old MsgFile

    tm("Calling API, p1: "+pathSeq->elementAtPosition(1)+dataFile+", fileTypeMod: "+GString((char*)fileTypeMod));

    //if( pathSeq->numberOfElements() == 1 )
    {
        sqluexpr (pathSeq->elementAtPosition(1)+dataFile, pLobPathList, pLobFileList, &columnData, columnStringPointer,
                  format, fileTypeMod, msgFile, 0, &outputInfo, NULL, &sqlca);
    }

    tm("Start Free...");
    free(fileTypeMod);
    free(pLobPathList->target.media);
    free(pLobPathList);
    free(pLobFileList->target.location);
    free(pLobFileList);
    free(columnStringPointer);
    tm("Done Free.");
    return sqlca.sqlcode;


}

#ifdef SQLM_DBMON_VERSION8
//##########################################################################################
//# New export API
//##########################################################################################
int db2dapi::exportTableNew(GString format, GString statement,
                                GString msgFile, GSeq<GString> *pathSeq, GString dataFile, int sessions, GString modified )
{
    tm("PathCount: "+GString(pathSeq->numberOfElements())+", firstPath: "+pathSeq->elementAtPosition(1)+", dataFile: "+dataFile+", sessions: "+GString(sessions)+", modfied: "+modified);
    //sessions = sessions * 2 + 1;
    unsigned short saveschema = 1;
    //struct sqlca sqlca;
    struct sqldcol dataDescriptor;
    struct sqllob *pAction = {0};
    char msgFileName[128];
    struct db2ExportOut outputInfo = {0};
    struct db2ExportIn inputInfo = {0};
    struct db2ExportStruct exportParmStruct;
    struct sqlchar *fileTypeMod = NULL;
    struct sqlu_media_entry *pPathList = {0};
    struct sqlu_location_entry *psLocationEntry;
    union sqlu_media_list_targets listTargetsXmlPath = {0}, listTargetsXmlFile = {0};
    struct sqlu_media_list mediaListXmlPath, mediaListXmlFile;

     char temp[256];
    printf("pos1\n");

    /* export data */
    dataDescriptor.dcolmeth = SQL_METH_D;
    pAction = (struct sqllob *)malloc(sizeof(sqluint32) + statement.length() + 1);
    pAction->length = statement.length();
    strcpy(pAction->data, (char*)statement);
    strcpy(msgFileName, (char*) msgFile);
    printf("pos1.1\n");
    /* XML Path Specification */
    pPathList=(struct sqlu_media_entry *)malloc(sizeof(struct sqlu_media_entry));
    sprintf(pPathList->media_entry, "%s", (char*) pathSeq->elementAtPosition(1));
    printf("pos1.2\n");

    listTargetsXmlPath.media = pPathList;
    mediaListXmlPath.media_type = 'L';
    mediaListXmlPath.sessions = 1;
    mediaListXmlPath.target = listTargetsXmlPath;
    printf("pos1.3\n");

    /* XMLFILE base name specification */
    psLocationEntry=(struct sqlu_location_entry *)malloc(sizeof(struct sqlu_location_entry));
    strcpy(psLocationEntry->location_entry,"expxmlfile");
    listTargetsXmlFile.location = psLocationEntry;
    mediaListXmlFile.media_type = 'C';
    mediaListXmlFile.sessions = 1;
    mediaListXmlFile.target = listTargetsXmlFile;

    /* File Type Modifier for Export Utility */
    printf("pos2\n");
    if( sessions > 0 ) //EXPORT BLOBs
    {
        //modified = "MODIFIED BY "+modified;
    }
    tm("exportTableNew, modifier: "+modified);

    strcpy(temp,modified);
    fileTypeMod = (struct sqlchar *) malloc(sizeof(short) + sizeof (temp) + 1);
    fileTypeMod->length = strlen(temp);
    strcpy(fileTypeMod->data,temp);


//    fileTypeMod = (struct sqlchar *) malloc(sizeof(short) + modified.length() + 1);
//    fileTypeMod->length = modified.length();
//    strcpy(fileTypeMod->data, modified);




    printf("pos3\n");
//    strcpy(temp,"XMLINSEPFILES LOBSINFILE");
//    fileTypeMod = (struct sqlchar *) malloc(sizeof(short) + sizeof (temp) + 1);
//    fileTypeMod->length = strlen(temp);
//    strcpy(fileTypeMod->data,temp);

    /* XML Save Schema Option */
    inputInfo.piXmlSaveSchema = &saveschema;

    //LobPaths
    struct sqlu_media_list *pLobPathList;
    pLobPathList = (sqlu_media_list *) malloc (sizeof(sqlu_media_list));
    pLobPathList->media_type = SQLU_LOCAL_MEDIA;
    pLobPathList->sessions = pathSeq->numberOfElements();
    pLobPathList->target.media = (sqlu_media_entry *) malloc(sizeof(sqlu_media_entry) * pLobPathList->sessions);
    for( int i = 1; i <= (int)pathSeq->numberOfElements(); ++i )
    {
        strcpy (pLobPathList->target.media[i-1].media_entry, pathSeq->elementAtPosition(i) );
    }
    //LobFiles
    struct sqlu_media_list *pLobFileList;
    pLobFileList = (sqlu_media_list *) malloc (sizeof(sqlu_media_list));
    pLobFileList->media_type = SQLU_CLIENT_LOCATION;
    pLobFileList->sessions = sessions;

    pLobFileList->target.location = (sqlu_location_entry *) malloc(sizeof(sqlu_location_entry) * pLobFileList->sessions);
    printf("pos4\n");
    GString fName;
    short k;
    tm("Adding to list...");
    for( k = 0; k < sessions; ++ k )
    {
        fName = dataFile+GString(k);
        strcpy (pLobFileList->target.location[k].location_entry, fName );
    }

    /* Export Data */
    exportParmStruct.piDataFileName    = (char*)dataFile;
    exportParmStruct.piDataDescriptor  = &dataDescriptor;
    exportParmStruct.piLobFileList     = pLobFileList;
    exportParmStruct.piLobPathList     = pLobPathList;
    exportParmStruct.piActionString    = pAction;
    exportParmStruct.piFileType        = format;
    exportParmStruct.piFileTypeMod     = fileTypeMod;
    exportParmStruct.piMsgFileName     = msgFileName;
    exportParmStruct.iCallerAction     = SQLU_INITIAL;
    exportParmStruct.poExportInfoOut   = &outputInfo;
    exportParmStruct.piExportInfoIn    = &inputInfo;
    exportParmStruct.piXmlPathList     = &mediaListXmlPath;
    exportParmStruct.piXmlFileList     = &mediaListXmlFile;


    printf("\n  Export data.\n");
    printf("    client destination file name: %s\n", (char*)dataFile);
    printf("    action                      : %s\n", (char*) statement);
    printf("    client message file name    : %s\n", msgFileName);

    /*Performing Export data */
    int rc = db2Export(db2Version970,
              &exportParmStruct,
              &sqlca);
    tm("expRC: "+GString(rc)+", sqlcode: "+GString(SQLCODE)+", err: "+SQLError());
    //DB2_API_CHECK("data -- export");

    /* free memory allocated */
    free(pAction);
    free(pPathList);
    free(fileTypeMod);
    free(psLocationEntry);

    /* display exported data */
    //rc = ExportedDataDisplay(dataFileName);

    return 0;

    /*
	 if( pathSeq->numberOfElements() == 0 ) return -1;
    int rc = 0;
    unsigned short saveschema = 1;
    struct sqlca sqlca = {0};
    struct sqldcol dataDescriptor = {0};
    char actionString[256];
    struct sqllob *pAction = {0};
    struct db2ExportOut outputInfo = {0};
    struct db2ExportIn inputInfo = {0};
    struct db2ExportStruct exportParmStruct = {0};
    struct sqlchar *fileTypeMod = NULL;
    struct sqlu_media_entry *pPathList = {0};
    struct sqlu_location_entry *psLocationEntry = {0};
    union sqlu_media_list_targets listTargetsXmlPath = {0}, listTargetsXmlFile = {0};
    struct sqlu_media_list mediaListXmlPath = {0}, mediaListXmlFile = {0};
    struct sqlu_media_list *pLobPathList;
    char temp[256];

    printf("\n-----------------------------------------------------------");
    printf("\nUSE THE DB2 API:\n");
    printf("  db2Export -- Export\n");
    printf("TO EXPORT DATA TO A FILE.\n");

    printf("\n  Be sure to complete all table operations and release\n");
    printf("  all locks before starting an export operation. This\n");
    printf("  can be done by issuing a COMMIT after closing all\n");
    printf("  cursors opened WITH HOLD, or by issuing a ROLLBACK.\n");
    printf("  Please refer to the 'Administrative API Reference'\n");
    printf("  for the details.\n");
printf("pos1\n");
    // export data
    dataDescriptor.dcolmeth = SQL_METH_D;
    strcpy(actionString, "SELECT Cid,Info FROM customer_xml ORDER BY Cid");
    pAction = (struct sqllob *)malloc(sizeof(sqluint32) + statement.length() + 1);
    printf("pos1A\n");
    pAction->length = statement.length();
    printf("pos1B\n");
    strcpy(pAction->data, (char*)statement);
printf("pos1C\n");
    // XML Path Specification
    pPathList=(struct sqlu_media_entry *)malloc(sizeof(struct sqlu_media_entry));
#if(defined(DB2NT))
    sprintf(pPathList->media_entry, "%s%sxmldatadir", getenv("DB2PATH"), PATH_SEP);
#else // UNIX
    //sprintf(pPathList->media_entry, "%s%sxmldatadir", getenv("HOME"), PATH_SEP);
    sprintf(pPathList->media_entry, "%s", (char*)pathSeq->elementAtPosition(1));
#endif
printf("pos2\n");
    listTargetsXmlPath.media = pPathList;
    mediaListXmlPath.media_type = 'L';
    mediaListXmlPath.sessions = 1;
    mediaListXmlPath.target = listTargetsXmlPath;

    //XMLFILE base name specification
    psLocationEntry=(struct sqlu_location_entry *)malloc(sizeof(struct sqlu_location_entry));
    strcpy(psLocationEntry->location_entry,"expxmlfile");
    listTargetsXmlFile.location = psLocationEntry;
    mediaListXmlFile.media_type = 'C';
    mediaListXmlFile.sessions = 1;
    mediaListXmlFile.target = listTargetsXmlFile;

    // File Type Modifier for Export Utility
    strcpy(temp,"XMLINSEPFILES");
    fileTypeMod = (struct sqlchar *) malloc(sizeof(short) + sizeof (temp) + 1);
    fileTypeMod->length = strlen(temp);
    strcpy(fileTypeMod->data,temp);

    // XML Save Schema Option
    inputInfo.piXmlSaveSchema = &saveschema;

    // LOB paths
    printf("pos3\n");
    pLobPathList = (sqlu_media_list *) malloc (sizeof(sqlu_media_list));
    pLobPathList->media_type = SQLU_LOCAL_MEDIA;
    pLobPathList->sessions = pathSeq->numberOfElements();
    pLobPathList->target.media = (sqlu_media_entry *) malloc(sizeof(sqlu_media_entry) * pLobPathList->sessions);
    for( int i = 1; i <= pathSeq->numberOfElements(); ++i )
    {
        strcpy (pLobPathList->target.media[i-1].media_entry, pathSeq->elementAtPosition(i) );
    } 	
    printf("pos4\n");
    // Export Data
    exportParmStruct.piDataFileName    = dataFile;
    exportParmStruct.piLobPathList     = pLobPathList;
    exportParmStruct.piLobFileList     = NULL;
    exportParmStruct.piDataDescriptor  = &dataDescriptor;
    exportParmStruct.piActionString    = pAction;
    exportParmStruct.piFileType        = format;
    exportParmStruct.piFileTypeMod     = fileTypeMod;
    exportParmStruct.piMsgFileName     = msgFile;
    exportParmStruct.iCallerAction     = SQLU_INITIAL;
    exportParmStruct.poExportInfoOut   = &outputInfo;
    exportParmStruct.piExportInfoIn    = &inputInfo;
    exportParmStruct.piXmlPathList     = &mediaListXmlPath;
    exportParmStruct.piXmlFileList     = &mediaListXmlFile;
printf("pos5\n");
    printf("\n  Export data.\n");
    printf("    client destination file name: %s\n", (char*)dataFile);
    printf("    action                      : %s\n", actionString);
    printf("    client message file name    : %s\n", (char*)msgFile);

    //Performing Export data
    printf(" \n EXPORT TO expxmldata.del OF DEL XML TO xmldatadir XMLFILE expxmlfile\n");
    printf("    MODIFIED BY XMLCHAR XMLINSEPFILES XMLSAVESCHEMA \n");
    printf("    SELECT CID, INFO FROM customer_xml ORDER BY Cid\n ");
    db2Export(db2Version970,
              &exportParmStruct,
              &sqlca);

printf("pos6\n");
    // free memory allocated
    free(pAction);
    free(pPathList);
    free(pLobPathList);
    free(fileTypeMod);
    free(psLocationEntry);

    // display exported data
    //rc = ExportedDataDisplay(dataFileName);

    return 0;
    */
}
#else
int db2dapi::exportTableNew(GString format, GString statement, GString msgFile, GSeq<GString> *pathSeq, GString dataFile, int sessions, GString modified )
{
    tm("db2dapi::exportTableNew: Falling back to db2dapi::exportTable(...)");
    return exportTableNew(format, statement, msgFile, pathSeq, dataFile, sessions, modified );
}
#endif




GString db2dapi::reorgTable(GString table, GString indexName, GString tabSpace)
{
    int rc = 0;
    if ( indexName.length() && tabSpace.length() ) rc = sqlureot (table, indexName, tabSpace , &sqlca);
    if ( indexName.length() && !tabSpace.length() ) rc = sqlureot (table, indexName, NULL , &sqlca);
    if ( !indexName.length() && tabSpace.length() ) rc = sqlureot (table, 0, tabSpace , &sqlca);
    if ( !indexName.length() && !tabSpace.length() )
    {
        char *tarray = (char*)"";
        rc = sqlureot (table, NULL, NULL, &sqlca);
        if( !rc )
        {
            rc = sqlustat (table, 0, &tarray, SQL_STATS_ALL, SQL_STATS_CHG, &sqlca);
        }
    }
    if( SQLCODE || rc ) return SQLError();
    return "";
}

GString db2dapi::runStats(GString table, GSeq <GString> * indList,  unsigned char statsOpt, unsigned char shareLevel )
{
    char *indArr[32767];
    char *ptr;
    int size;
    unsigned long i;

    for( i = 1; i <= indList->numberOfElements(); ++i)
    {
        size = indList->elementAtPosition(i).length();
        ptr = new char[size+1];
        memcpy(ptr,(char*) indList->elementAtPosition(i), size);
        ptr[size] = 0;
        indArr[i-1] = ptr;
    }
    int rc = sqlustat (table, indList->numberOfElements(), indArr, statsOpt, shareLevel, &sqlca);
    if( SQLCODE || rc ) return SQLError();
    return "";
}

GString db2dapi::rebind(GString bindFile)
{
    int rc = sqlarbnd (bindFile, &sqlca, NULL);
    if( SQLCODE || rc ) return SQLError();
    return "";
}



/****************************************************************
*
*   GET DB VERSION
*
****************************************************************/
int db2dapi::getDBVersion(GString alias)
{
    tm("GetDBVers, alias: "+alias);
    int dbv = 1;
    //struct sqlca sqlca;
    short index;
    unsigned short dbHandle;
    unsigned short dbCount;
    struct sqledinfo *dbBuffer;

    GString text;
    sqledosd (NULL, &dbHandle, &dbCount, &sqlca);
    if (sqlca.sqlcode == SQLE_RC_NODBDIR)
    {
        tm("Err in getDBVers");
        return 2;
    }
    tm("Count: "+GString(dbCount));
    for (index = 0; index < dbCount; index++)
    {
        sqledgne (dbHandle, &dbBuffer, &sqlca);
        tm("GetDBVers, found: "+GString(dbBuffer->alias));
        tm("Stripped: "+GString(dbBuffer->alias).subString(1, 8).strip());

        if( GString(dbBuffer->alias).subString(1, 8).strip() == alias.strip() )
        {
            text = GString(dbBuffer->dbtype).subString(9, 20);
            tm("text1: "+text+"<-");
            text = text.subString(1, text.indexOf('.')-1).strip();
            tm("text2: "+text+"<-");
            if( text.asInt() > 0 ) dbv = text.asInt();
            //This is based on v8: Apparently they'll name the version in hex, but it's too soon to be sure...
            if( text.upperCase()  == "A" ) dbv = 10;
            if( text.upperCase()  == "B" ) dbv = 11;
            if( text.upperCase()  == "C" ) dbv = 12;
            if( text.upperCase()  == "D" ) dbv = 13;
            if( text.upperCase()  == "E" ) dbv = 14;
            if( text.upperCase()  == "F" ) dbv = 15;
        }
    }
    sqledcls (dbHandle, &sqlca);
    return dbv;
}

int db2dapi::stopReq()
{
    return sqleintr();
}




/****************************************************************
*
*   LOAD API
*
****************************************************************/
#ifdef SQLM_DBMON_VERSION8
int db2dapi::loadFromFileNew(GString dataFile, GSeq<GString> *pathSeq, GString format, GString statement, GString msgFile, GString modifierString, GString copyTarget)
{
    tm("db2dapi::loadFromFileNew, PathCount: "+GString(pathSeq->numberOfElements())+", firstPath: "+pathSeq->elementAtPosition(1)+", dataFile: "+dataFile+", modfied: "+modifierString+", copyTarget: "+copyTarget);

    struct db2LoadStruct loadParmStruct;
    struct sqlu_media_list *pLobPathList;
    struct sqlu_media_list *pSourceFileList;
    struct sqlu_media_list *pXmlFileList;
    struct sqlu_media_list *pCopyTargetList;
    struct sqldcol dataDescriptor;
    struct sqlchar* pActionString;
    struct sqlchar         *fileTypeMod;
    struct sqllob *pAction = {0};

    memset(&loadParmStruct, '\0', sizeof(db2LoadStruct));

    struct db2LoadIn * pLoadInfoIn;
    struct db2LoadOut * pLoadInfoOut;

    dataDescriptor.dcolmeth = 'D';

    tm("Allocating files...");
    /******** FILES *****************/
    pSourceFileList = (sqlu_media_list *) malloc (sizeof(sqlu_media_list));
    pSourceFileList->media_type = SQLU_CLIENT_LOCATION;
    pSourceFileList->sessions = 1;
    pSourceFileList->target.location = (sqlu_location_entry *) malloc(sizeof(sqlu_location_entry) * pSourceFileList->sessions);
    strcpy (pSourceFileList->target.media[0].media_entry, dataFile );

    tm("Allocating copyTarget...");
    pCopyTargetList = (sqlu_media_list *) malloc (sizeof(sqlu_media_list));
    pCopyTargetList->media_type = SQLU_LOCAL_MEDIA;
    pCopyTargetList->sessions = 1;
    pCopyTargetList->target.location = (sqlu_location_entry *) malloc(sizeof(sqlu_location_entry) * pCopyTargetList->sessions);
    strcpy (pCopyTargetList->target.media[0].media_entry, copyTarget );

    tm("Allocating LobPaths...");
    pLobPathList = (sqlu_media_list *) malloc (sizeof(sqlu_media_list));
    pLobPathList->media_type = SQLU_LOCAL_MEDIA;
    //pLobPathList->media_type = SQLU_SERVER_LOCATION;
    pLobPathList->sessions = pathSeq->numberOfElements();
    pLobPathList->target.media = (sqlu_media_entry *) malloc(sizeof(sqlu_media_entry) * pLobPathList->sessions);
    for( int i = 1; i <= (int)pathSeq->numberOfElements(); ++i )
    {
        strcpy (pLobPathList->target.media[i-1].media_entry, pathSeq->elementAtPosition(i) );
    }

    tm("Allocating XmlPaths...");
    pXmlFileList = (sqlu_media_list *) malloc (sizeof(sqlu_media_list));
    pXmlFileList->media_type = SQLU_LOCAL_MEDIA;
    //pLobPathList->media_type = SQLU_SERVER_LOCATION;
    pXmlFileList->sessions = pathSeq->numberOfElements();
    pXmlFileList->target.media = (sqlu_media_entry *) malloc(sizeof(sqlu_media_entry) * pXmlFileList->sessions);
    for( int i = 1; i <= (int)pathSeq->numberOfElements(); ++i )
    {
        strcpy (pXmlFileList->target.media[i-1].media_entry, pathSeq->elementAtPosition(i) );
    }

    tm("Action, stmt.length: "+GString(statement.length()));
    pActionString = (struct sqlchar *)malloc(statement.length()+sizeof (struct sqlchar)+1);
    pActionString->length = statement.length();
    strncpy (pActionString->data, statement, statement.length());
    pActionString->data[statement.length()] = 0;

    pAction = (struct sqllob *)malloc(sizeof(sqluint32) + statement.length() + 1);
    pAction->length = statement.length();
    strncpy(pAction->data, (char*)statement, statement.length());
    pAction->data[statement.length()] = 0;

    tm("Action: "+GString(pAction->data)+"<-");
    tm("ActionString: "+GString(pActionString->data)+"<-");

    tm("FileTypeMod...");
    fileTypeMod = (struct sqlchar *)malloc(modifierString.length() + sizeof (struct sqlchar) + 1);
    fileTypeMod->length = modifierString.length();
    strncpy (fileTypeMod->data, modifierString, fileTypeMod->length);
    fileTypeMod->data[modifierString.length()] = 0;
    tm("FileTypeMod: "+GString(fileTypeMod->data));


    pLoadInfoIn = NULL;
    pLoadInfoOut = NULL;
    loadParmStruct.piSourceList      = pSourceFileList;
    loadParmStruct.piLobPathList     = pLobPathList;
    loadParmStruct.piDataDescriptor  = &dataDescriptor;
    loadParmStruct.piActionString    = pActionString;
    loadParmStruct.piFileType        = (char*)format;
    loadParmStruct.piFileTypeMod     = fileTypeMod;
    loadParmStruct.piLocalMsgFileName = msgFile;
    loadParmStruct.piTempFilesPath   = NULL;
    loadParmStruct.piVendorSortWorkPaths = NULL;
    if( copyTarget.length() > 0 ) loadParmStruct.piCopyTargetList = pCopyTargetList;
    else loadParmStruct.piCopyTargetList = NULL;
    loadParmStruct.piNullIndicators = NULL;
    loadParmStruct.piLoadInfoIn     = pLoadInfoIn;
    loadParmStruct.poLoadInfoOut    = pLoadInfoOut;
    loadParmStruct.piPartLoadInfoIn = NULL;
    loadParmStruct.poPartLoadInfoOut = NULL;
    loadParmStruct.iCallerAction     = SQLU_INITIAL;
    loadParmStruct.piXmlPathList     = pXmlFileList;
    loadParmStruct.piLongActionString = NULL; //pAction;

    db2Load (db2Version970, &loadParmStruct, &sqlca);

    free(pAction);
    free(pActionString);
    free(pLobPathList);
    free(pXmlFileList);
    free(pSourceFileList);
    free(pCopyTargetList);
    free(fileTypeMod);
    tm("LoadNew finished, erc: "+GString(sqlca.sqlcode)+", err: "+SQLError());
    return sqlca.sqlcode;
}
#else
int db2dapi::loadFromFileNew(GString dataFile, GSeq<GString> *pathSeq, GString format, GString statement, GString msgFile, GString modifierString)
{
    tm("db2dapi::loadFromFileNew: Falling back to loadFromFile(...)");
    return loadFromFile(dataFile, pathSeq, format, statement, msgFile, modifierString);
}
#endif

int db2dapi::uncatalogNode(GString nodeName)
{
    int rc = sqleuncn (nodeName, &sqlca);
    //printf("RC uncNode: %i\n", rc);
    return sqlca.sqlcode;
}

int db2dapi::uncatalogDatabase(GString alias)
{
    int rc = sqleuncd (alias, &sqlca);
    //printf("RC uncDB: %i\n", rc);
    return sqlca.sqlcode;
}

int db2dapi::createNode(GString hostName, GString nodeName, GString port, GString comment)
{
    //struct sqlca sqlca;
    struct sqle_node_tcpip tcpStruct;
    struct sqle_node_struct newNode;

    strncpy(tcpStruct.hostname, hostName, SQL_HOSTNAME_SZ+1);
    strncpy(tcpStruct.service_name, port, SQL_SERVICE_NAME_SZ+1);
    strncpy(newNode.nodename, nodeName.subString(1, 8), SQL_NNAME_SZ + 1);
    strncpy(newNode.comment, comment, SQL_CMT_SZ + 1);
    newNode.struct_id = SQL_NODE_STR_ID;
    newNode.protocol = SQL_PROTOCOL_TCPIP;
    sqlectnd(&newNode, &tcpStruct, &sqlca);
    return sqlca.sqlcode;
}

GSeq<NODE_INFO*> db2dapi::getNodeInfo()
{
    unsigned short nodeDirHandle, i, nodeEntries = 0;
    struct sqleninfo *nodeEntry;
    GSeq<NODE_INFO*> nodeSeq;
    NODE_INFO* pInfo;

    sqlenops(&nodeDirHandle, &nodeEntries, &sqlca);
    if( SQLCODE ) return nodeSeq;


    for (i = 0; i < nodeEntries; i++)
    {
        sqlengne(nodeDirHandle, &nodeEntry, &sqlca);
        if( SQLCODE ) break;;

        pInfo = new NODE_INFO;
        pInfo->NodeName = GString(nodeEntry->nodename).subString(1, SQL_NNAME_SZ).strip();
        pInfo->Comment = GString(nodeEntry->comment).subString(1, SQL_CMT_SZ).strip();
        pInfo->HostName = GString(nodeEntry->hostname).subString(1, SQL_HOSTNAME_SZ).strip();
        pInfo->ServiceName = GString(nodeEntry->service_name).subString(1, SQL_SERVICE_NAME_SZ).strip();

        switch (nodeEntry->protocol)
        {
        case SQL_PROTOCOL_LOCAL:
            pInfo->ProtocolName = "Local";
            break;
        case SQL_PROTOCOL_NPIPE:
            pInfo->ProtocolName = "NPIPE";
            break;
        case SQL_PROTOCOL_SOCKS:
            pInfo->ProtocolName = "SOCKS";
            break;
        case SQL_PROTOCOL_SOCKS4:
            pInfo->ProtocolName = "SOCKS4";
            break;
        case SQL_PROTOCOL_TCPIP:
            pInfo->ProtocolName = "TCP/IP";
            break;
        case SQL_PROTOCOL_TCPIP4:
            pInfo->ProtocolName = "TCP/IPv4";
            break;
        case SQL_PROTOCOL_TCPIP6:
            pInfo->ProtocolName = "TCP/IPv6";
            break;
        default:
            pInfo->ProtocolName = "<None>";
            break;
        }
        nodeSeq.add(pInfo);
    }
    sqlencls(nodeDirHandle, &sqlca);
    return nodeSeq;
}


int db2dapi::catalogDatabase(GString name, GString alias, GString type, GString nodeName, GString path, GString comment, int authentication )
{
    //struct sqlca sqlca;
    //authentication = SQL_AUTHENTICATION_SERVER;
    int erc = sqlecadb(name.strip(), alias.strip(), type.strip(), nodeName.strip(), path.strip(), comment.strip(), authentication, NULL,  &sqlca);

    /* ignore warning SQL1100W = node not cataloged, */
    /* don't do the same in your code */
    if (sqlca.sqlcode != 1100)
    {
        return sqlca.sqlcode;
    }
    return erc;
}

int db2dapi::dbRestart(GString alias, GString user, GString pwd)
{
    struct db2RestartDbStruct dbRestartParam;

    dbRestartParam.piDatabaseName = alias;
    dbRestartParam.piUserId = user;
    dbRestartParam.piPassword = pwd;
    dbRestartParam.piTablespaceNames = NULL;
    db2DatabaseRestart(db2Version710, &dbRestartParam, &sqlca);
    return sqlca.sqlcode;
}

int db2dapi::loadFromFile(GString dataFile, GSeq<GString> *pathSeq, GString format, GString statement, GString msgFile, GString modifierString)
{
    GString dataPath;
    if( dataFile.occurrencesOf("/") ) dataPath = dataFile.subString(1, dataFile.lastIndexOf("/"));
    else if( dataFile.occurrencesOf("\\") ) dataPath = dataFile.subString(1, dataFile.lastIndexOf("\\"));

    //importTable(GString dataFile, GString path, GString format, GString statement, GString msgFile, int useLob)
    sqlu_media_list * pDataFileList;
    sqlu_media_list * pLobPathList;
    struct sqldcol DataDescriptor;
    struct sqlchar* pActionString;
    //char * pFileType; : "format" instead
    struct sqlchar* pFileTypeMod;
    //NOTUSED: char * pLocaleMsgFile;
    char * pRemoteMsgFile;
    short callAction;
    struct sqluload_in * pLoadInfoIn;
    struct sqluload_out * pLoadInfoOut;
    sqlu_media_list * pWorkDirectoryList;
    sqlu_media_list * pCopyTargetList;
    sqlint32 * pNullIndicators;


    /********************************************************
   * Set DataDescriptor: SQL_METH_D,
   * basically "1st column of file into 1st column of table"
   */
    DataDescriptor.dcolmeth = 'D';

    /********************************************************
   * Fill struct for a single file to load
   * CAUTION: sqluimpr takes PATHS, sqluload filenames (incl. paths)
   * So we need to set SQLU_CLIENT_LOCATION instead of SQLU_LOCAL_MEDIA
   */
    pDataFileList = (sqlu_media_list *) malloc (sizeof(sqlu_media_list));
    pDataFileList->media_type = SQLU_CLIENT_LOCATION;
    pDataFileList->sessions = 1;
    pDataFileList->target.media = (sqlu_media_entry *) malloc(sizeof(sqlu_media_entry) * pDataFileList->sessions);
    strcpy (pDataFileList->target.media[0].media_entry, dataFile );

    /********************************************************
   * Fill struct for LOBs
   */
    pLobPathList = (sqlu_media_list *) malloc (sizeof(sqlu_media_list));
    pLobPathList->media_type = SQLU_LOCAL_MEDIA;
    pLobPathList->sessions = pathSeq->numberOfElements();
    pLobPathList->target.media = (sqlu_media_entry *) malloc(sizeof(sqlu_media_entry) * pLobPathList->sessions);
    for( int i = 1; i <= (int)pathSeq->numberOfElements(); ++i )
    {
        strcpy (pLobPathList->target.media[i-1].media_entry, pathSeq->elementAtPosition(i) );
    }




    /********************************************************
   * Action
   */
    pActionString = (struct sqlchar *)malloc(strlen(statement)+sizeof (struct sqlchar));
    pActionString->length = strlen(statement);
    strncpy (pActionString->data, statement, strlen(statement));


    /********************************************************
   * Load LOBs...
   */
    GString strModifier;
//    if( identityModifier == 1 ) strModifier = "lobsinfile";
//    else if( identityModifier == 2 ) strModifier = "identityignore";
//    else if( identityModifier == 3 ) strModifier = "lobsinfile identityignore";
//    else if( identityModifier == 0 ) strModifier = "";
//    else if( identityModifier == 4 ) strModifier = "identityoverride";

    strModifier += " "+modifierString;
    pFileTypeMod = (struct sqlchar *)malloc(strModifier.length() + sizeof (struct sqlchar));
    pFileTypeMod->length = strModifier.length();
    strncpy (pFileTypeMod->data, strModifier, pFileTypeMod->length);

    /********************************************************
   * Need to set this, otherwise the tablespace of the target table will
   * be locked until backup is run.
   */
    pCopyTargetList = (sqlu_media_list *) malloc (sizeof(sqlu_media_list));
    pCopyTargetList->media_type = SQLU_LOCAL_MEDIA;
    pCopyTargetList->sessions = 1;
    pCopyTargetList->target.media = (sqlu_media_entry *) malloc(sizeof(sqlu_media_entry) * pCopyTargetList->sessions);
    strcpy (pCopyTargetList->target.media[0].media_entry, dataPath );

    pCopyTargetList = NULL;

    /********************************************************
   * CallerAction: Initial, Continue, etc.
   */
    callAction   = SQLU_INITIAL; /* Required for first load */


    /********************************************************
   * Stuff we don't need (yet)
   */
    pLoadInfoIn  = NULL;
    pLoadInfoOut = NULL;
    pWorkDirectoryList = NULL;
    pNullIndicators    = NULL; /* INT-Array of NULLABLE columns. */
    pRemoteMsgFile     = NULL;



    /********************************************************
   * Call load-api....
   */
    sqluload( pDataFileList, pLobPathList, &DataDescriptor, pActionString, format, pFileTypeMod,
                        msgFile, pRemoteMsgFile, callAction, pLoadInfoIn, pLoadInfoOut, pWorkDirectoryList,
                        pCopyTargetList, pNullIndicators, NULL, &sqlca);


    return sqlca.sqlcode;
}
GSeq <DB_INFO*> db2dapi::dbInfo()
{
    //struct sqlca sqlca;
    short index;
    unsigned short dbHandle;
    unsigned short dbCount;
    struct sqledinfo *dbBuffer;

    GSeq<DB_INFO*> pDataSeq;


    //sqledosd ((char*)delm, &dbHandle, &dbCount, &sqlca);
    sqledosd (NULL, &dbHandle, &dbCount, &sqlca);
    if (sqlca.sqlcode == SQLE_RC_NODBDIR)
    {
        //pDataSeq->add("Sorry, got error "+GString(sqlca.sqlcode));
        return pDataSeq;
    }
    DB_INFO *pDbInfo;
    for (index = 0; index < dbCount; index++)
    {
        sqledgne (dbHandle, &dbBuffer, &sqlca);
        pDbInfo = new DB_INFO;
        pDbInfo->init();

        pDbInfo->Alias = GString(dbBuffer->alias, 8).strip();
        pDbInfo->Database = GString(dbBuffer->dbname, 8).strip();
#if(defined(DB2NT))
        pDbInfo->Drive = GString(dbBuffer->drive, 12).strip();
#else
        pDbInfo->Drive = GString(dbBuffer->drive, 215).strip();
#endif
        pDbInfo->Directory = GString(dbBuffer->intname, 8).strip();
        pDbInfo->NodeName = GString(dbBuffer->nodename, 8).strip();
        pDbInfo->DbType   = GString(dbBuffer->dbtype, 20).strip();
        pDbInfo->Comment   = GString(dbBuffer->comment, 30).strip();
        pDataSeq.add(pDbInfo);
    }
    sqledcls (dbHandle, &sqlca);
    return pDataSeq;

}

GString db2dapi::getDbCfgForToken(GString nodeName, GString user, GString pwd, GString dbAlias, int token, int isNumToken)
{
    struct sqlca loc_sqlca;

    tm("Start getDbCfgForToken, node: "+nodeName+", user: "+user+", alias: "+dbAlias+", token: "+GString(token)+", isNum: "+GString(isNumToken));
    db2CfgParam cfgParameters[1];
    db2Cfg cfgStruct;


    int erc = sqleatin(nodeName, user, pwd, &loc_sqlca);
    if( erc ) return erc;
    cfgStruct.numItems = 1;
    cfgStruct.paramArray = cfgParameters;
    cfgStruct.flags = db2CfgDatabase | db2CfgDelayed;
    cfgStruct.dbname = (char*)dbAlias;

    cfgParameters[0].flags = 0;
    cfgParameters[0].token = token;

    if( isNumToken )
    {
        tm("getDbCfgForToken, alloc for int...");
        cfgParameters[0].ptrvalue = (char *)malloc(sizeof(sqluint16));
        *(sqluint16 *)(cfgParameters[0].ptrvalue) = 0;
    }
    else
    {
        tm("getDbCfgForToken, alloc for string...");
        cfgParameters[0].ptrvalue = (char *)malloc(sizeof(char) * 256);
        strcpy(cfgParameters[0].ptrvalue, "");
    }
/******************* TEST SET:
  if( !isNumToken )strcpy(cfgParameters[0].ptrvalue, "tsm_owner");
  else *(sqluint16 *)(cfgParameters[0].ptrvalue) = 50;
  //Setting:
  db2CfgSet(db2Version970, (void *)&cfgStruct, &sqlca);
   //Clear
  if( !isNumToken )strcpy(cfgParameters[0].ptrvalue, "");
  else *(sqluint16 *)(cfgParameters[0].ptrvalue) = 0;

***********************/
    tm("getDbCfgForToken, calling db2CfgGet...");
    erc = db2CfgGet(db2Version970, (void *)&cfgStruct, &loc_sqlca);
    tm("getDbCfgForToken, calling db2CfgGet done, rc: "+GString(erc));
    GString ret;
    if( !isNumToken)
    {
        ret = cfgParameters[0].ptrvalue;
    }
    else
    {
        ret = GString(*(sqluint16 *)(cfgParameters[0].ptrvalue));
    }
    tm("getDbCfgForToken, result: "+ret);
    tm("getDbCfgForToken, calling free()...");
    free(cfgParameters[0].ptrvalue);

    erc = sqledtin(&loc_sqlca);
    return ret;
}

/***********************************************************************/
/*            END DEFINE DB2 Version 7 and above                       */
/***********************************************************************/
int db2dapi::getHeaderData(int pos, GString * data)
{

    tm("getHeaderData for "+GString(pos));
    if( pos < 1 || pos > (int)headerSeq.numberOfElements()) return 1;
    *data = headerSeq.elementAtPosition(pos);
    tm("getHeaderData for "+GString(pos)+": "+(*data));
    return 0;
}

int db2dapi::getRowData(int row, int col, GString * data)
{
    if( row < 1 || row > (int)rowSeq.numberOfElements() ) return 1;


    if( col < 1 || col > (int)rowSeq.elementAtPosition(row)->numberOfElements() ) return 1;
    *data = rowSeq.elementAtPosition(row)->elementAt(col);
    return 0;
}

void db2dapi::mb(GString msg)
{   
    QMessageBox::information(0, "UDB API", msg);
}
void db2dapi::tm(GString msg)
{           
    //printf("db2dapi: %s\n", (char*) msg);
    if( m_pGDB ) m_pGDB->debugMsg("db2dapi", 1, msg);
}
db2dapi::RowData::~RowData()
{
    rowDataSeq.removeAll();
}
void db2dapi::RowData::add(GString data)
{
    rowDataSeq.add(data);
}
GString db2dapi::RowData::elementAt(int pos)
{
    if( pos < 1 || pos > (int)rowDataSeq.numberOfElements() ) return "";
    return rowDataSeq.elementAtPosition(pos);
}
unsigned long db2dapi::RowData::numberOfElements()
{
    return rowDataSeq.numberOfElements();
}
unsigned long db2dapi::getHeaderDataCount()
{
    return headerSeq.numberOfElements();
}
unsigned long db2dapi::getRowDataCount()
{
    return rowSeq.numberOfElements();
}

void db2dapi::clearSequences()
{
    RowData *pRowData;
    while( !rowSeq.isEmpty() )
    {
        pRowData = rowSeq.firstElement();
        delete pRowData;
        rowSeq.removeFirst();
    }
    headerSeq.removeAll();
}
