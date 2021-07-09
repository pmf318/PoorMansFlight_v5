//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt
//

#define SNAPSHOT_BUFFER_UNIT_SZ 1024
#define NUMELEMENTS 779

//#include <stdio.h>

#include <udbapi.hpp>

#include "sqlca.h"
#include "sqlda.h"
#include <sqlenv.h>
#include "sqlutil.h"



#include <qlistview.h>

#include <sqlmon.h>

static  struct sqlca    sqlca;
static  char            SqlMsg[1024];


#ifdef MAKE_VC
   extern "C" int  __declspec( dllexport ) getSnapshot( char* dbName, char* nodeName, char* user, char* pwd, int type, QListView* mainLV )
#else
   extern "C" int getSnapshot( char* dbName, char* nodeName, char* user, char* pwd, int type, long lv )
#endif
{
   int erc = 0;

#ifdef MAKE_VC
#else
  QListView * mainLV = (QListView*) lv;
#endif
   udbapi aSnap(dbName, nodeName, user, pwd, type, mainLV);
   aSnap.startMonitors(dbName);

   aSnap.startMonitors(dbName);

   //erc = aSnap.startMainThread();   
   erc = aSnap.getSnapshotData();
   //aSnap.resetMonitor();
   return erc;

}

#ifdef MAKE_VC
   extern "C" unsigned long  __declspec( dllexport ) getTabSpace( QListView* mainLV )
#else
   extern "C" unsigned long getTabSpace( long lv )
#endif
{
   udbapi aSnap;
#ifdef MAKE_VC
   aSnap.initTabSpaceLV(mainLV);
#else

#endif

   return 0;

}


GString udbapi::SQLError()
{
    sqlaintp( SqlMsg, sizeof(SqlMsg), 1023, &sqlca );
    return( GString(&SqlMsg[0]) );
}


udbapi::udbapi(GString database, GString ndName, GString usr, GString passwd, int type, QListView * pLV)
{
    dataBase = database;
    nodeName = ndName;
    user     = usr;
    pwd      = passwd;
    snType   = type;
    mainLV   = pLV;
}
/****************************************************************
*
*
*  Stuff for snapshot
*  THIS ONLY GETS COMPILED ON DB2 v 7 and above
*
*****************************************************************/
#ifdef SQLM_DBMON_VERSION7
void udbapi::startMainThread()
{
   mainThread = new MainThread;
   mainThread->setOwner( this );
   mainThread->start();
   runInfo = new  QMessageBox(mainLV, "Working...");
   runInfo->setText("THREAD IS RUNNING");
   runInfo->show(); 

}

void udbapi::MainThread::run()
{
   myUDBApi->getSnapshotData();
}


int udbapi::getSnapshotData()
{

tm("Start getSN, node: "+nodeName);

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
  strncpy((char *)pSqlMA->obj_var[0].object, (char*)dataBase.strip(), SQLM_OBJECT_SZ);


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
  
  return initSnapshot(pSqlMA, mainLV, snType);

//  runInfo->close();

}


/****************************************************************
*
*   INIT SNAPSHOT
*
****************************************************************/
int udbapi::initSnapshot(struct sqlma *pSqlMA, QListView * pLV, int type)
{
tm("Start initSN, node: "+nodeName);

/********** attach is now done in dsqlobj::connect(...)
  if( !nodeName.length() ) 
  {
      nodeName = getenv("DB2INSTANCE");
      if( !nodeName.length() )  nodeName = "DB2"; //We're guessing here...
  }
  //This apparently fucks up the monitor results for dynSQL on remote instances...
  if( nodeName != "DB2" ) sqleatin(nodeName, user, pwd, &sqlca);
  
  if( sqlca.sqlcode  )
  {
     mb("Tried to attach to instance "+nodeName+". It failed with error-code "+GString(sqlca.sqlcode));
     return 1;  
  }
  //Done attach
*******/
  int erc;
  struct sqlm_collected collected;
  struct sqlca sqlca;
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
  #ifdef SQLM_DBMON_VERSION8
    getSnapshotSizeParam.iSnapshotClass = SQLM_CLASS_DEFAULT;
  #endif

//!!
#ifdef SQLM_DBMON_VERSION8
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
#ifdef SQLM_DBMON_VERSION8  
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
#ifdef SQLM_DBMON_VERSION8
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
  processData(buffer_ptr, pLV, type);
  FreeMemory(pSqlMA, buffer_ptr);
tm("initSN5");
//  if( nodeName != "DB2" ) sqledtin(&sqlca);
  return erc;
}


/****************************************************************
*
*   START MONITORS
*
****************************************************************/
int udbapi::startMonitors(GString database)
{
  tm("Start Mon...");
  int rc = 0;
  struct sqlca sqlca;
  db2MonitorSwitchesData switchesData;
  //db2gMonitorSwitchesData getSwitchesData;

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

#ifdef SQLM_DBMON_VERSION8
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

#ifdef SQLM_DBMON_VERSION8  
     db2MonitorSwitches(db2Version810, &switchesData, &sqlca);
#else     
     db2MonitorSwitches(db2Version710, &switchesData, &sqlca);
#endif     


  if (sqlca.sqlcode != 0L)
  {
    if (sqlca.sqlcode < 0L)
    {
      return((int)sqlca.sqlcode);
    }
  }
  tm("Mon started");
  return rc;
}

/****************************************************************
*
*   RESET MONITORS
*
****************************************************************/
int udbapi::resetMonitor()
{
  tm("Start ResetMon...");
  int rc = 0;
  struct sqlca sqlca;
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



  tm("Resetting...DONE");
#ifdef SQLM_DBMON_VERSION8  
//     db2MonitorSwitches(db2Version810, &switchesData, &sqlca);
     db2ResetMonitor(db2Version810, &switchesData, &sqlca);
#else
//     db2MonitorSwitches(db2Version710, &switchesData, &sqlca);
     db2ResetMonitor(db2Version710, &switchesData, &sqlca);
#endif     
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
int udbapi::FreeMemory(struct sqlma *pSqlMA, char *buffer_ptr)
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
GString udbapi::getData(sqlm_header_info * pHeader, char* pData)
{
   char buff[20];
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
sqlm_header_info* udbapi::jumpToKey(int element, char* pStart, GString& data)
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
int udbapi::processData(char * pStart, QListView * pLV, int type)
{

tm("Start ProcData");
   GSeq <int> keySeq;
   GSeq <GString> nameSeq;
   int erc;
tm("+++++++++++++++++++++++ BUFPTR +++++++++++++++++++++++++++");
tm(pStart);
tm("+++++++++++++++++++++++ BUFPTR DONE+++++++++++++++++++++++++++");

   switch( type )
   {
      case  SQLMA_DYNAMIC_SQL:
         erc = fillDSQL(pStart, pLV);
         break;

      case  SQLMA_DBASE_TABLES:
         erc = fillTabList(pStart, pLV);
         break;

      case  SQLMA_DBASE_LOCKS:
         erc = fillLock(pStart, pLV);
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
         erc = fillLV(pStart, pLV, SQLM_ELM_BUFFERPOOL, &keySeq, &nameSeq);
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

         erc = fillLV(pStart, pLV, SQLM_ELM_DBASE, &keySeq, &nameSeq);

         break;

   }
   tm("RET: "+GString(erc));

   return erc;
}
/****************************************************************
*
*   FILL LOCK
*
****************************************************************/
int udbapi::fillLock(char * pStart, QListView * pLV)
{

   sqlm_header_info *pBlock, *pB1, *pB2;
   GString data;
   char * pData;
   tm("Call Jump...");

   pLV->clear();
   while( pLV->columns() ) pLV->removeColumn(0);

   pLV->addColumn("Agent ID");
   pLV->addColumn("Appl Name");
   pLV->addColumn("Appl Status");
   pLV->addColumn("Codepage");
   pLV->addColumn("Locks Held");
   pLV->addColumn("Locks Waiting");
   pLV->addColumn("Lock Wait Time");
   pLV->addColumn("Status Change Time");
   pLV->addColumn("Appl ID");
   pLV->addColumn("Sequence No");
   pLV->addColumn("Auth ID");
   pLV->addColumn("DB Alias");

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


   pData = (char*) pBlock;
   while( pData < pEnd )
   {
tm("fillLock, in while");
      lvItem = new QListViewItem(pLV);
      pB1 = jumpToKey(SQLM_ELM_APPL_LOCK_LIST, pData, data);
      if( pB1 == NULL ) return 2;

      jumpToKey(SQLM_ELM_AGENT_ID, (char*)pB1, data);
      lvItem->setText(0, data);

      jumpToKey(SQLM_ELM_APPL_NAME, (char*)pB1, data);
      lvItem->setText(1, data);

      jumpToKey(SQLM_ELM_APPL_STATUS, (char*)pB1, data);
      lvItem->setText(2, data);

      jumpToKey(SQLM_ELM_CODEPAGE_ID, (char*)pB1, data);
      lvItem->setText(3, data);

      jumpToKey(SQLM_ELM_LOCKS_HELD, (char*)pB1, data);
      lvItem->setText(4, data);
      jumpToKey(SQLM_ELM_LOCKS_WAITING, (char*)pB1, data);
      lvItem->setText(5, data);

      jumpToKey(SQLM_ELM_LOCK_WAIT_TIME, (char*)pB1, data);
      lvItem->setText(6, data);

      //ExecTime is a nested Block
      GString t1, t2;
      pB2 = jumpToKey(SQLM_ELM_STATUS_CHANGE_TIME, (char*)pB1, data);
tm("fillLock, in while2");
      jumpToKey(SQLM_ELM_SECONDS, (char*)pB2, t1);
      jumpToKey(SQLM_ELM_MICROSEC, (char*)pB2, t2);

      t2 = t1 + "." + t2.rightJustify(6, '0');
      lvItem->setText(7, t2);

      jumpToKey(SQLM_ELM_APPL_ID, (char*)pB1, data);
      lvItem->setText(8, data);
      jumpToKey(SQLM_ELM_SEQUENCE_NO, (char*)pB1, data);
      lvItem->setText(9, data);

      jumpToKey(SQLM_ELM_AUTH_ID, (char*)pB1, data);
      lvItem->setText(10, data);
      jumpToKey(SQLM_ELM_CLIENT_DB_ALIAS, (char*)pB1, data);
      lvItem->setText(11, data);
tm("fillLock, in while3");
       //Get next block of type SQL_ELM_DYNSQL
      pData = (char*) pB1 + pB1->size + sizeof(sqlm_header_info);
      if( pData > pEnd ) break;
   }
   return 0;
}

/****************************************************************
*
*   FILL DSQL
*
****************************************************************/
int udbapi::fillDSQL(char * pStart, QListView * pLV)
{

   sqlm_header_info *pBlock, *pB1, *pB2;
   GString data;
   char * pData;
   tm("Call Jump...");

   pLV->clear();
   while( pLV->columns() ) pLV->removeColumn(0);

   pLV->addColumn("SQLStatement", 150);
   pLV->addColumn("Rows Read");
   pLV->addColumn("Rows Written");
   pLV->addColumn("Rows Deleted");
   pLV->addColumn("Rows Inserted");
   pLV->addColumn("Rows Updated");
   pLV->addColumn("Executions");
   pLV->addColumn("Compilations");
   pLV->addColumn("PrepTime Worst");
   pLV->addColumn("PrepTime Best");
   pLV->addColumn("Stmt Sorts");
   pLV->addColumn("Exec Time (sec.microsec)");

   //Parse Datastream, find relevant position
   pBlock = jumpToKey(SQLM_ELM_DYNSQL_LIST, pStart, data);
   if( !pBlock)
   {
       mb("No Monitor Info available, don't know why. Sorry.");
       return 0;
   }

   tm(" * * * * Size of pBlock1: "+GString(pBlock->size));

   char * pEnd = (char*) pBlock + pBlock->size + sizeof(sqlm_header_info);


   pData = (char*) pBlock;

   while( pData < pEnd )
   {
      lvItem = new QListViewItem(pLV);
      pB1 = jumpToKey(SQLM_ELM_DYNSQL, pData, data);
      if( pB1 == NULL ) return 2;

      jumpToKey(SQLM_ELM_STMT_TEXT, (char*)pB1, data);
      lvItem->setText(0, data);

      jumpToKey(SQLM_ELM_ROWS_READ, (char*)pB1, data);
      lvItem->setText(1, data);

      jumpToKey(SQLM_ELM_ROWS_WRITTEN, (char*)pB1, data);
      lvItem->setText(2, data);
      jumpToKey(SQLM_ELM_INT_ROWS_DELETED, (char*)pB1, data);
      lvItem->setText(3, data);
      jumpToKey(SQLM_ELM_INT_ROWS_INSERTED, (char*)pB1, data);
      lvItem->setText(4, data);
      jumpToKey(SQLM_ELM_INT_ROWS_UPDATED, (char*)pB1, data);
      lvItem->setText(5, data);

      jumpToKey(SQLM_ELM_NUM_EXECUTIONS, (char*)pB1, data);
      lvItem->setText(6, data);
      jumpToKey(SQLM_ELM_NUM_COMPILATIONS, (char*)pB1, data);
      lvItem->setText(7, data);

      jumpToKey(SQLM_ELM_PREP_TIME_WORST, (char*)pB1, data);
      lvItem->setText(8, data);
      jumpToKey(SQLM_ELM_PREP_TIME_BEST, (char*)pB1, data);
      lvItem->setText(9, data);
      jumpToKey(SQLM_ELM_STMT_SORTS, (char*)pB1, data);
      lvItem->setText(10, data);


      //ExecTime is a nested Block
      GString t1, t2;
tm("Getting seconds...");
      pB2 = jumpToKey(SQLM_ELM_TOTAL_EXEC_TIME, (char*)pB1, data);
      jumpToKey(SQLM_ELM_SECONDS, (char*)pB2, t1);
//      jumpToKey(SQLM_ELM_SS_EXEC_TIME, (char*)pB2, t1);
      jumpToKey(SQLM_ELM_MICROSEC, (char*)pB2, t2);
tm("Getting seconds...DONE");

      t2 = t1 + "." + t2.rightJustify(6, '0');
      lvItem->setText(11, t2);

      //Get next block of type SQL_ELM_DYNSQL
      pData = (char*) pB1 + pB1->size + sizeof(sqlm_header_info);
      if( pData > pEnd ) break;
   }
   return 0;
}


/****************************************************************
*
*   FILL TAB LIST
*
****************************************************************/
int udbapi::fillTabList(char * pStart, QListView * pLV)
{
   tm("Start fillTabList");
   sqlm_header_info *pBlock, *pB1;
   GString data;
   char * pData;

   pLV->clear();
   while( pLV->columns() ) pLV->removeColumn(0);
   tm("Adding cols...");
   pLV->addColumn("TabSchema", 150);
   pLV->addColumn("Table Name");
   pLV->addColumn("Rows Written");
   pLV->addColumn("Rows Read");
   pLV->addColumn("Rows Inserted");
   pLV->addColumn("Overflow Accesses");
   pLV->addColumn("Table FileID");
   pLV->addColumn("Table Type");
   pLV->addColumn("Page Reorgs");
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
   while( pData < pEnd )
   {
      lvItem = new QListViewItem(pLV);
      pB1 = jumpToKey(SQLM_ELM_TABLE, pData, data);
      if( pB1 == NULL )
      {
         tm("SQL_ELM_TABLE: No monitor-info available.");
	 return 2;
      }


      jumpToKey(SQLM_ELM_TABLE_SCHEMA, (char*)pB1, data);
      lvItem->setText(0, data);

      jumpToKey(SQLM_ELM_TABLE_NAME, (char*)pB1, data);
      lvItem->setText(1, data);

      jumpToKey(SQLM_ELM_ROWS_WRITTEN, (char*)pB1, data);
      lvItem->setText(2, data);

      jumpToKey(SQLM_ELM_ROWS_READ, (char*)pB1, data);
      lvItem->setText(3, data);

      jumpToKey(SQLM_ELM_OVERFLOW_ACCESSES, (char*)pB1, data);
      lvItem->setText(4, data);
      jumpToKey(SQLM_ELM_TABLE_FILE_ID, (char*)pB1, data);
      lvItem->setText(5, data);
      jumpToKey(SQLM_ELM_TABLE_TYPE, (char*)pB1, data);
      lvItem->setText(6, data);

      jumpToKey(SQLM_ELM_PAGE_REORGS, (char*)pB1, data);
      lvItem->setText(7, data);

      pData = (char*) pB1 + pB1->size + sizeof(sqlm_header_info);
      if( pData > pEnd ) {tm("Done");break;}
   }
   return 0;
}

/****************************************************************
*
*   FILL LV
*
****************************************************************/
int udbapi::fillLV(char * pStart, QListView * pLV, int key, GSeq <int> * keySeq, GSeq <GString> * nameSeq)
{

tm("Start fillLV...");
   sqlm_header_info * pHeader, *pBlock;
   GString data;
   tm("Call Jump...");

   pLV->clear();
   while( pLV->columns() ) pLV->removeColumn(0);

   pLV->addColumn("Key");
   pLV->addColumn("Data");
   QListViewItem * lvItem;

   char * pEnd;// = pStart + pHeader->size + sizeof(sqlm_header_info);
tm("fillLV, pos1");
   pBlock = jumpToKey(key, pStart, data);
   if( !pBlock)
   {
       mb("No Monitor Info available, don't know why. Sorry.");
       return 0;
   }

   tm(" * * * * Size of pBlock1: "+GString(pBlock->size));
   pEnd = (char*) pBlock + pBlock->size + sizeof(sqlm_header_info);

tm("fillLV, pos2");
   char* pData = (char*) pBlock;
   for( unsigned int i = 1; i <= keySeq->numberOfElements(); ++i )
   {
      if( pData > pEnd ) break;
      lvItem = new QListViewItem(pLV);
      jumpToKey(keySeq->elementAtPosition(i), (char*)pBlock, data);
      if( data.length() )
      {
         lvItem->setText(0, nameSeq->elementAtPosition(i));
         lvItem->setText(1, data);
      }
   }
tm("Done");
   return NULL;
}


/****************************************************************
*
*   TABLESPACES, main
*
****************************************************************/
short udbapi::initTabSpaceLV(QListView * mainLV)
{
   mainLV->addColumn("ID");
   mainLV->addColumn("Name");
   mainLV->addColumn("Type");
   mainLV->addColumn("Contents");
   mainLV->addColumn("State");
   mainLV->addColumn("Total Pages");
   mainLV->addColumn("Usable Pages");
   mainLV->addColumn("Used Pages");
   mainLV->addColumn("Free Pages");
   mainLV->addColumn("High Water Mark ");

   struct sqlca sqlca;
   struct SQLB_TBSPQRY_DATA *dataP;
   sqluint32 numTS, maxTS;

   sqlbotsq (&sqlca, SQLB_OPEN_TBS_ALL, &numTS);
   maxTS = numTS;
   dataP = (struct SQLB_TBSPQRY_DATA *) malloc (numTS *
      sizeof (struct SQLB_TBSPQRY_DATA));
   strcpy(dataP->tbspqver,SQLB_TBSPQRY_DATA_ID);

   sqlbftpq (&sqlca, maxTS, dataP, &numTS);
   fillTabSpaceLV (dataP, numTS, mainLV);

   sqlbctsq (&sqlca);
   return 0;
}
/****************************************************************
*
*   TABLESPACES, fill
*
****************************************************************/
void udbapi::fillTabSpaceLV (struct SQLB_TBSPQRY_DATA *dataP, sqluint32 num, QListView * mainLV)
{
   struct sqlca sqlca;
   struct SQLB_TBS_STATS tbs_stats;
   sqluint32 idx;
   QListViewItem * lvItem;
   GString val;
   for (idx=0; idx < num; idx++, dataP++) {
      lvItem = new QListViewItem(mainLV);
      lvItem->setText(0, GString(dataP->id));
      lvItem->setText(1, GString(dataP->name));
      /* "Type" and "Content" are stored bitwise in the flag field */
      switch (dataP->flags & 0xF ) {
         case SQLB_TBS_SMS:
            lvItem->setText(2, "SMS");
            break;
         case SQLB_TBS_DMS:
            lvItem->setText(2, "DMS");
            break;
         default:
            lvItem->setText(2, "UNKNOWN");
            break;
      }

      switch (dataP->flags & 0xF0) {
         case SQLB_TBS_ANY:
            lvItem->setText(3, "regular contents");
            break;
         case SQLB_TBS_LONG:
            lvItem->setText (3, "long field data");
            break;
         case SQLB_TBS_TMP:
            lvItem->setText(3, "temp data");
            break;
         default:
            lvItem->setText(3, "UNKNOWN TYPE");
            break;
      } /* endswitch */

      switch (dataP->tbsState) {
         case SQLB_NORMAL:
            lvItem->setText (4, "Normal");
            break;
         case SQLB_QUIESCED_SHARE:
            lvItem->setText (4, "Quiesced: SHARE");
            break;
         case SQLB_QUIESCED_UPDATE:
            lvItem->setText (4, "Quiesced: UPDATE");
            break;
         case SQLB_QUIESCED_EXCLUSIVE:
            lvItem->setText (4, "Quiesced: EXCLUSIVE");
            break;
         case SQLB_LOAD_PENDING:
            lvItem->setText (4, "Load pending");
            break;
         case SQLB_DELETE_PENDING:
            lvItem->setText (4, "Delete pending");
            break;
         case SQLB_BACKUP_PENDING:
            lvItem->setText (4, "Backup pending");
            break;
         case SQLB_ROLLFORWARD_IN_PROGRESS:
            lvItem->setText (4, "Roll forward in progress");
            break;
         case SQLB_ROLLFORWARD_PENDING:
            lvItem->setText (4, "Roll forward pending");
            break;
         case SQLB_RESTORE_PENDING:
            lvItem->setText (4, "Restore pending");
            break;
         case SQLB_DISABLE_PENDING:
            lvItem->setText (4, "Disable pending");
            break;
         case SQLB_REORG_IN_PROGRESS:
            lvItem->setText (4, "Reorg in progress");
            break;
         case SQLB_BACKUP_IN_PROGRESS:
            lvItem->setText (4, "Backup in progress");
            break;
         case SQLB_STORDEF_PENDING:
            lvItem->setText (4, "storage must be defined");
            break;
         case SQLB_RESTORE_IN_PROGRESS:
            lvItem->setText (4, "Restore in progress");
            break;
         case SQLB_STORDEF_ALLOWED:
            lvItem->setText (4, "storage may be defined");
            break;
         case SQLB_STORDEF_FINAL_VERSION:
            lvItem->setText (4, "storDef is in 'final' state");
            break;
         case SQLB_STORDEF_CHANGED:
            lvItem->setText (4, "storDef was changed prior to rollforward");
            break;
         case SQLB_REBAL_IN_PROGRESS:
            lvItem->setText (4, "dms rebalancer is active");
            break;
         case SQLB_PSTAT_DELETION:
            lvItem->setText (4, "TBS deletion in progress");
            break;
         case SQLB_PSTAT_CREATION:
            lvItem->setText (4, "TBS creation in progress");
            break;
         default:
            lvItem->setText (4, "UNKNOWN");
            break;
      }

      sqlbgtss(&sqlca, dataP->id, &tbs_stats);


      lvItem->setText (5, GString(tbs_stats.totalPages));
      lvItem->setText (6, GString(tbs_stats.useablePages));
      lvItem->setText (7, GString(tbs_stats.usedPages));
      lvItem->setText (8, GString(tbs_stats.freePages));
      lvItem->setText (9, GString(tbs_stats.highWaterMark));

   }
}
#endif
/***********************************************************************/
/*            END DEFINE DB2 Version 7 and above                       */
/***********************************************************************/

void udbapi::mb(GString msg)
{   
   QMessageBox::information(0, "UDB API", msg);   
}
void udbapi::tm(GString msg)
{   
    //return;
    printf(" UDBAPI> %s\n",  (char*) msg);
#ifdef MAKE_VC
    flushall();
#endif
}

