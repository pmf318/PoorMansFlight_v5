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



#include <qtablewidget.h>

#include <sqlmon.h>

static  struct sqlca    sqlca;
static  char            SqlMsg[1024];


/***********************************************************************
 * This class can either be instatiated or loaded via dlopen/loadlibrary
 ***********************************************************************/
//Define functions with C symbols (create/destroy instance).
extern "C" udbapi* create(GString database, GString ndName, GString usr, GString passwd, int type, QTableWidget * pLV)
{
    printf("udbapi: extC create called\n");
    return new udbapi(database, ndName, usr, passwd, type, pLV);
}
extern "C" void destroy(udbapi* pUDBAPI)
{
   if( pUDBAPI ) delete pUDBAPI ;
}



#ifdef MAKE_VC1
   extern "C" int  __declspec( dllexport ) getSnapshot( char* dbName, char* nodeName, char* user, char* pwd, int type, QTableWidget* mainLV )
#else
   extern "C" int getSnapshot( char* dbName, char* nodeName, char* user, char* pwd, int type, long lv )
#endif
{
   int erc = 0;

#ifdef MAKE_VC1
#else
  QTableWidget * mainLV = (QTableWidget*) lv;
#endif
   udbapi aSnap(dbName, nodeName, user, pwd, type, mainLV);
   aSnap.startMonitors();


   //erc = aSnap.startMainThread();   
   erc = aSnap.getSnapshotData();
   //aSnap.resetMonitor();
   return erc;

}
/*
#ifdef MAKE_VC1
   extern "C" unsigned long  __declspec( dllexport ) getTabSpace( QTableWidget* mainLV )
#else
   extern "C" unsigned long getTabSpace( long mainLV )
#endif
{
   udbapi aSnap;
#ifdef MAKE_VC1
   aSnap.initTabSpaceLV(mainLV);
#else

#endif

   return 0;

}
*/

GString udbapi::SQLError()
{
    if( !sqlca.sqlcode ) return "";
    sqlaintp( SqlMsg, sizeof(SqlMsg), 1023, &sqlca );
    return( GString(&SqlMsg[0]) );
}


udbapi::udbapi(GString database, GString ndName, GString usr, GString passwd, int type, QTableWidget * pLV)
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
   QMessageBox::information(mainLV, "PMF", "Working...");

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
int udbapi::initSnapshot(struct sqlma *pSqlMA, QTableWidget * pLV, int type)
{
tm("Start initSN, node: "+nodeName);

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
int udbapi::startMonitors()
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
int udbapi::processData(char * pStart, QTableWidget * pLV, int type)
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
int udbapi::fillLock(char * pStart, QTableWidget * pLV)
{

	sqlm_header_info *pBlock, *pB1, *pB2;
	GString data;
	char * pData;
	tm("Call Jump...");

	
	pLV->clear();
	QTableWidgetItem * pItem;

	pLV->setColumnCount(12);
	int j = 0;
	pItem = new QTableWidgetItem("Agent ID");
	pLV->setHorizontalHeaderItem(j++, pItem);	
	pItem = new QTableWidgetItem("Appl Name");
	pLV->setHorizontalHeaderItem(j++, pItem);		
	pItem = new QTableWidgetItem("Appl Status");
	pLV->setHorizontalHeaderItem(j++, pItem);		
	pItem = new QTableWidgetItem("Codepage");
	pLV->setHorizontalHeaderItem(j++, pItem);		
	pItem = new QTableWidgetItem("Locks Held");
	pLV->setHorizontalHeaderItem(j++, pItem);		
	pItem = new QTableWidgetItem("Locks Waiting");
	pLV->setHorizontalHeaderItem(j++, pItem);		
	pItem = new QTableWidgetItem("Lock Wait Time");
	pLV->setHorizontalHeaderItem(j++, pItem);		
	pItem = new QTableWidgetItem("Status Change Time");
	pLV->setHorizontalHeaderItem(j++, pItem);	
	pItem = new QTableWidgetItem("Appl ID");
	pLV->setHorizontalHeaderItem(j++, pItem);		
	pItem = new QTableWidgetItem("Sequence No");
	pLV->setHorizontalHeaderItem(j++, pItem);		
	pItem = new QTableWidgetItem("Auth ID");
	pLV->setHorizontalHeaderItem(j++, pItem);		
	pItem = new QTableWidgetItem("DB Alias");
	pLV->setHorizontalHeaderItem(j++, pItem);	
	
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
	int i = 0;
	while( pData < pEnd )
	{
		tm("fillLock, in while");
		
		pItem = new QTableWidgetItem();
		pB1 = jumpToKey(SQLM_ELM_APPL_LOCK_LIST, pData, data);
		if( pB1 == NULL ) return 2;

		pLV->setRowCount(i+1);
		jumpToKey(SQLM_ELM_AGENT_ID, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 0, pItem);

		jumpToKey(SQLM_ELM_APPL_NAME, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 1, pItem);

		jumpToKey(SQLM_ELM_APPL_STATUS, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 2, pItem);

		jumpToKey(SQLM_ELM_CODEPAGE_ID, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 3, pItem);

		jumpToKey(SQLM_ELM_LOCKS_HELD, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 4, pItem);

		jumpToKey(SQLM_ELM_LOCKS_WAITING, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 5, pItem);

		jumpToKey(SQLM_ELM_LOCK_WAIT_TIME, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 6, pItem);

		//ExecTime is a nested Block
		GString t1, t2;
		pB2 = jumpToKey(SQLM_ELM_STATUS_CHANGE_TIME, (char*)pB1, data);
		tm("fillLock, in while2");
		jumpToKey(SQLM_ELM_SECONDS, (char*)pB2, t1);
		jumpToKey(SQLM_ELM_MICROSEC, (char*)pB2, t2);

		t2 = t1 + "." + t2.rightJustify(6, '0');
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 7, pItem);

		jumpToKey(SQLM_ELM_APPL_ID, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 8, pItem);

		jumpToKey(SQLM_ELM_SEQUENCE_NO, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 9, pItem);

		jumpToKey(SQLM_ELM_AUTH_ID, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 10, pItem);

		jumpToKey(SQLM_ELM_CLIENT_DB_ALIAS, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 11, pItem);
		tm("fillLock, in while3");
		//Get next block of type SQL_ELM_DYNSQL
		pData = (char*) pB1 + pB1->size + sizeof(sqlm_header_info);
		if( pData > pEnd ) break;
		

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
int udbapi::fillDSQL(char * pStart, QTableWidget * pLV)
{

	sqlm_header_info *pBlock, *pB1, *pB2;
	GString data;
	char * pData;
	tm("Call Jump...");
	QTableWidgetItem * pItem;
	pLV->clear();

	int j = 0;
	pLV->setColumnCount(12);
	pItem = new QTableWidgetItem("SQLStatement");
	pLV->setHorizontalHeaderItem(j++, pItem);
	pItem = new QTableWidgetItem("Rows Read");
	pLV->setHorizontalHeaderItem(j++, pItem);
	pItem = new QTableWidgetItem("Rows Written");
	pLV->setHorizontalHeaderItem(j++, pItem);
	pItem = new QTableWidgetItem("Rows Deleted");
	pLV->setHorizontalHeaderItem(j++, pItem);
	pItem = new QTableWidgetItem("Rows Inserted");
	pLV->setHorizontalHeaderItem(j++, pItem);
	pItem = new QTableWidgetItem("Rows Updated");
	pLV->setHorizontalHeaderItem(j++, pItem);
	pItem = new QTableWidgetItem("Executions");
	pLV->setHorizontalHeaderItem(j++, pItem);
	pItem = new QTableWidgetItem("Compilations");
	pLV->setHorizontalHeaderItem(j++, pItem);
	pItem = new QTableWidgetItem("PrepTime Worst");
	pLV->setHorizontalHeaderItem(j++, pItem);
	pItem = new QTableWidgetItem("PrepTime Best");
	pLV->setHorizontalHeaderItem(j++, pItem);
	pItem = new QTableWidgetItem("Stmt Sorts");
	pLV->setHorizontalHeaderItem(j++, pItem);
	pItem = new QTableWidgetItem("Exec Time (sec.microsec)");
	pLV->setHorizontalHeaderItem(j++, pItem);

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
	int i = 0;
	while( pData < pEnd )
	{
		pItem = new QTableWidgetItem();
		pB1 = jumpToKey(SQLM_ELM_DYNSQL, pData, data);
		if( pB1 == NULL ) return 2;
		pLV->setRowCount(i+1);
		
		jumpToKey(SQLM_ELM_STMT_TEXT, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 0, pItem);
		

		jumpToKey(SQLM_ELM_ROWS_READ, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 1, pItem);

		jumpToKey(SQLM_ELM_ROWS_WRITTEN, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 2, pItem);
		jumpToKey(SQLM_ELM_INT_ROWS_DELETED, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 3, pItem);
		jumpToKey(SQLM_ELM_INT_ROWS_INSERTED, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 4, pItem);
		jumpToKey(SQLM_ELM_INT_ROWS_UPDATED, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 5, pItem);

		jumpToKey(SQLM_ELM_NUM_EXECUTIONS, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 6, pItem);
		jumpToKey(SQLM_ELM_NUM_COMPILATIONS, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 7, pItem);

		jumpToKey(SQLM_ELM_PREP_TIME_WORST, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 8, pItem);
		jumpToKey(SQLM_ELM_PREP_TIME_BEST, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 9, pItem);
		jumpToKey(SQLM_ELM_STMT_SORTS, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 10, pItem);


		//ExecTime is a nested Block
		GString t1, t2;
		tm("Getting seconds...");
		pB2 = jumpToKey(SQLM_ELM_TOTAL_EXEC_TIME, (char*)pB1, data);
		jumpToKey(SQLM_ELM_SECONDS, (char*)pB2, t1);
		//      jumpToKey(SQLM_ELM_SS_EXEC_TIME, (char*)pB2, t1);
		jumpToKey(SQLM_ELM_MICROSEC, (char*)pB2, t2);
		tm("Getting seconds...DONE");

		t2 = t1 + "." + t2.rightJustify(6, '0');
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 11, pItem);

		//Get next block of type SQL_ELM_DYNSQL
		pData = (char*) pB1 + pB1->size + sizeof(sqlm_header_info);
		if( pData > pEnd ) break;
		i++;
	}
	//for( int i = 0; i < pLV->rowCount(); ++i ) pLV->resizeRowToContents ( i ) ;
	return 0;
}


/****************************************************************
*
*   FILL TAB LIST
*
****************************************************************/
int udbapi::fillTabList(char * pStart, QTableWidget * pLV)
{
	tm("Start fillTabList");
	sqlm_header_info *pBlock, *pB1;
	GString data;
	char * pData;

	pLV->clear();
	pLV->setRowCount(0);
	QTableWidgetItem * pItem;
	tm("Adding cols...");
	int j = 0;
	pLV->setColumnCount(9);	
	pItem = new QTableWidgetItem("TabSchema");
	pLV->setHorizontalHeaderItem(j++, pItem);
	pItem = new QTableWidgetItem("Table Name");
	pLV->setHorizontalHeaderItem(j++, pItem);
	pItem = new QTableWidgetItem("Rows Written");
	pLV->setHorizontalHeaderItem(j++, pItem);
	pItem = new QTableWidgetItem("Rows Read");
	pLV->setHorizontalHeaderItem(j++, pItem);
	pItem = new QTableWidgetItem("Rows Inserted");
	pLV->setHorizontalHeaderItem(j++, pItem);
	pItem = new QTableWidgetItem("Overflow Accesses");
	pLV->setHorizontalHeaderItem(j++, pItem);
	pItem = new QTableWidgetItem("Table FileID");
	pLV->setHorizontalHeaderItem(j++, pItem);
	pItem = new QTableWidgetItem("Table Type");
	pLV->setHorizontalHeaderItem(j++, pItem);
	pItem = new QTableWidgetItem("Page Reorgs");
	pLV->setHorizontalHeaderItem(j++, pItem);
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
	int i = 0;
	while( pData < pEnd )
	{
		pItem = new QTableWidgetItem();
		pB1 = jumpToKey(SQLM_ELM_TABLE, pData, data);
		if( pB1 == NULL )
		{
			tm("SQL_ELM_TABLE: No monitor-info available.");
            mb("No events since monitor was started.");
			return 2;
		}
		pLV->setRowCount(i+1);

		jumpToKey(SQLM_ELM_TABLE_SCHEMA, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 0, pItem);

		jumpToKey(SQLM_ELM_TABLE_NAME, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 1, pItem);

		jumpToKey(SQLM_ELM_ROWS_WRITTEN, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 2, pItem);

		jumpToKey(SQLM_ELM_ROWS_READ, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 3, pItem);

		jumpToKey(SQLM_ELM_OVERFLOW_ACCESSES, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 4, pItem);
		jumpToKey(SQLM_ELM_TABLE_FILE_ID, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 5, pItem);
		jumpToKey(SQLM_ELM_TABLE_TYPE, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 6, pItem);

		jumpToKey(SQLM_ELM_PAGE_REORGS, (char*)pB1, data);
		pItem = new QTableWidgetItem((char*)data);
		pLV->setItem(i, 7, pItem);

		pData = (char*) pB1 + pB1->size + sizeof(sqlm_header_info);
		if( pData > pEnd ) {tm("Done");break;}
		i++;
	}
	//for( int i = 0; i < pLV->rowCount(); ++i ) pLV->resizeRowToContents ( i ) ;
	return 0;
}

/****************************************************************
*
*   FILL LV
*
****************************************************************/
int udbapi::fillLV(char * pStart, QTableWidget * pLV, int key, GSeq <int> * keySeq, GSeq <GString> * nameSeq)
{

	tm("Start fillLV...");
    sqlm_header_info *pBlock;
	GString data;
	tm("Call Jump...");

	pLV->clear();
	
	pLV->setColumnCount(2);
	QTableWidgetItem * pItem;
	pItem = new QTableWidgetItem("Key");
	pLV->setHorizontalHeaderItem(0, pItem);
	pItem = new QTableWidgetItem("Data");
	pLV->setHorizontalHeaderItem(1, pItem);


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
	for( unsigned int i = 0; i < keySeq->numberOfElements(); ++i )
	{
		if( pData > pEnd ) break;
		
		jumpToKey(keySeq->elementAtPosition(i), (char*)pBlock, data);
		if( data.length() )
		{
			pLV->setRowCount(i+1);
			pItem = new QTableWidgetItem((char*) nameSeq->elementAtPosition(i+1));
			pLV->setItem(i, 0, pItem);
			pItem = new QTableWidgetItem((char*) data);
			pLV->setItem(i, 1, pItem);
			
		}
	}
	//for( int i = 0; i < pLV->rowCount(); ++i ) pLV->resizeRowToContents ( i ) ;
	tm("Done");
    return 0;
}


/****************************************************************
*
*   TABLESPACES, main
*
****************************************************************/
int udbapi::initTabSpaceLV(QTableWidget * pLV)
{
    //mb("initTabSpaceLV 0");
	QTableWidgetItem * pItem;	
	pLV->setColumnCount(10);
	pItem = new QTableWidgetItem("ID");
	pLV->setHorizontalHeaderItem(0, pItem);
	pItem = new QTableWidgetItem("Name");
	pLV->setHorizontalHeaderItem(1, pItem);
	pItem = new QTableWidgetItem("Type");
	pLV->setHorizontalHeaderItem(2, pItem);
	pItem = new QTableWidgetItem("Contents");
	pLV->setHorizontalHeaderItem(3, pItem);
	pItem = new QTableWidgetItem("State");
	pLV->setHorizontalHeaderItem(4, pItem);
	pItem = new QTableWidgetItem("Total Pages");
	pLV->setHorizontalHeaderItem(5, pItem);
	pItem = new QTableWidgetItem("Usable Pages");
	pLV->setHorizontalHeaderItem(6, pItem);
	pItem = new QTableWidgetItem("Used Pages");
	pLV->setHorizontalHeaderItem(7, pItem);
	pItem = new QTableWidgetItem("Free Pages");
	pLV->setHorizontalHeaderItem(8, pItem);
	pItem = new QTableWidgetItem("High Water Mark ");
	pLV->setHorizontalHeaderItem(9, pItem);

	struct sqlca sqlca;
	struct SQLB_TBSPQRY_DATA *dataP;
	sqluint32 numTS, maxTS;

	sqlbotsq (&sqlca, SQLB_OPEN_TBS_ALL, &numTS);
	maxTS = numTS;
    dataP = (struct SQLB_TBSPQRY_DATA *) malloc (numTS *sizeof (struct SQLB_TBSPQRY_DATA));

    memcpy(dataP->tbspqver,SQLB_TBSPQRY_DATA_ID, 8);

	sqlbftpq (&sqlca, maxTS, dataP, &numTS);
	fillTabSpaceLV (dataP, numTS, pLV);

	sqlbctsq (&sqlca);
	return 0;
}
/****************************************************************
*
*   TABLESPACES, fill
*
****************************************************************/
void udbapi::fillTabSpaceLV (struct SQLB_TBSPQRY_DATA *dataP, sqluint32 num, QTableWidget * mainLV)
{
	struct sqlca sqlca;
	struct SQLB_TBS_STATS tbs_stats;
	sqluint32 idx;
	QTableWidgetItem * lvItem;
	GString val;
	mainLV->setRowCount(num);
	for (idx=0; idx < num; idx++, dataP++) 
	{
		lvItem = new QTableWidgetItem((char*)GString(dataP->id));
		mainLV->setItem(idx, 0, lvItem);
		lvItem = new QTableWidgetItem((char*)GString(dataP->name));
		mainLV->setItem(idx, 1, lvItem);
		
		/* "Type" and "Content" are stored bitwise in the flag field */
		switch (dataP->flags & 0xF ) 
		{
			case SQLB_TBS_SMS:
				lvItem = new QTableWidgetItem("SMS");
				mainLV->setItem(idx, 2, lvItem);				
				break;
			case SQLB_TBS_DMS:
				lvItem = new QTableWidgetItem("DMS");
				mainLV->setItem(idx, 2, lvItem);				
				break;
			default:
				lvItem = new QTableWidgetItem("UNKNOWN");
				mainLV->setItem(idx, 2, lvItem);				
				break;
		}

		switch (dataP->flags & 0xF0) 
		{
			case SQLB_TBS_ANY:
				lvItem = new QTableWidgetItem("Regular contents");
				mainLV->setItem(idx, 3, lvItem);				
				break;
			case SQLB_TBS_LONG:
				lvItem = new QTableWidgetItem("Long field data");
				mainLV->setItem(idx, 3, lvItem);				
				break;
			case SQLB_TBS_TMP:
				lvItem = new QTableWidgetItem("Temp data");
				mainLV->setItem(idx, 3, lvItem);				
				break;
			default:
				lvItem = new QTableWidgetItem("UNKNOWN TYPE");
				mainLV->setItem(idx, 3, lvItem);				
				break;
		} /* endswitch */

		switch (dataP->tbsState) 
		{
			case SQLB_NORMAL:
				lvItem = new QTableWidgetItem("Normal");
				mainLV->setItem(idx, 4, lvItem);				
				break;
			case SQLB_QUIESCED_SHARE:
				lvItem = new QTableWidgetItem("Quiesced: SHARE");
				mainLV->setItem(idx, 4, lvItem);				
				break;
			case SQLB_QUIESCED_UPDATE:
				lvItem = new QTableWidgetItem("Quiesced: UPDATE");
				mainLV->setItem(idx, 4, lvItem);				
				break;
			case SQLB_QUIESCED_EXCLUSIVE:
				lvItem = new QTableWidgetItem("Quiesced: EXCLUSIVE");
				mainLV->setItem(idx, 4, lvItem);				
				break;
			case SQLB_LOAD_PENDING:
				lvItem = new QTableWidgetItem("LOAD pending");
				mainLV->setItem(idx, 4, lvItem);				
				break;
			case SQLB_DELETE_PENDING:
				lvItem = new QTableWidgetItem("DELETE pending");
				mainLV->setItem(idx, 4, lvItem);				
				break;
			case SQLB_BACKUP_PENDING:
				lvItem = new QTableWidgetItem("BACKUP pending");
				mainLV->setItem(idx, 4, lvItem);				
				break;
			case SQLB_ROLLFORWARD_IN_PROGRESS:
				lvItem = new QTableWidgetItem("ROLLFORWARD in progress");
				mainLV->setItem(idx, 4, lvItem);				
				break;
			case SQLB_ROLLFORWARD_PENDING:
				lvItem = new QTableWidgetItem("ROLLFORWARD pending");
				mainLV->setItem(idx, 4, lvItem);				
				break;
			case SQLB_RESTORE_PENDING:
				lvItem = new QTableWidgetItem("RESTORE pending");
				mainLV->setItem(idx, 4, lvItem);				
				break;
			case SQLB_DISABLE_PENDING:
				lvItem = new QTableWidgetItem("DISABLE pending");
				mainLV->setItem(idx, 4, lvItem);				
				break;
			case SQLB_REORG_IN_PROGRESS:
				lvItem = new QTableWidgetItem("REORG in progress");
				mainLV->setItem(idx, 4, lvItem);				
				break;
			case SQLB_BACKUP_IN_PROGRESS:
				lvItem = new QTableWidgetItem("BACKUP in progress");
				mainLV->setItem(idx, 4, lvItem);				
				break;
			case SQLB_STORDEF_PENDING:
				lvItem = new QTableWidgetItem("Storage must be defined (pending)");
				mainLV->setItem(idx, 4, lvItem);				
				break;
			case SQLB_RESTORE_IN_PROGRESS:
				lvItem = new QTableWidgetItem("RESTORE in progress");
				mainLV->setItem(idx, 4, lvItem);				
				break;
			case SQLB_STORDEF_ALLOWED:
				lvItem = new QTableWidgetItem("STORAGE may be defined (allowed)");
				mainLV->setItem(idx, 4, lvItem);				
				break;
			case SQLB_STORDEF_FINAL_VERSION:
				lvItem = new QTableWidgetItem("STORDEF is in final");
				mainLV->setItem(idx, 4, lvItem);				
				break;
			case SQLB_STORDEF_CHANGED:
				lvItem = new QTableWidgetItem("STORDEF was changed prior to rollforward");
				mainLV->setItem(idx, 4, lvItem);				
				
				break;
			case SQLB_REBAL_IN_PROGRESS:
				lvItem = new QTableWidgetItem("DMS-Rebalancer is active");
				mainLV->setItem(idx, 4, lvItem);								
				break;
			case SQLB_PSTAT_DELETION:
				lvItem = new QTableWidgetItem("TBS deletion in progress");
				mainLV->setItem(idx, 4, lvItem);				
				
				break;
			case SQLB_PSTAT_CREATION:
				lvItem = new QTableWidgetItem("TBS creation in progress");
				mainLV->setItem(idx, 4, lvItem);								
				break;
			default:
				lvItem = new QTableWidgetItem("UNKNOWN");
				mainLV->setItem(idx, 4, lvItem);				
				break;
		}

		sqlbgtss(&sqlca, dataP->id, &tbs_stats);


		lvItem = new QTableWidgetItem((char*)GString(tbs_stats.totalPages));
		mainLV->setItem(idx, 5, lvItem);				
		
		lvItem = new QTableWidgetItem((char*)GString(tbs_stats.useablePages));
		mainLV->setItem(idx, 6, lvItem);				
		
		lvItem = new QTableWidgetItem((char*)GString(tbs_stats.usedPages));
		mainLV->setItem(idx, 7, lvItem);				
		
		lvItem = new QTableWidgetItem((char*)GString(tbs_stats.freePages));
		mainLV->setItem(idx, 8, lvItem);				
		
		lvItem = new QTableWidgetItem((char*)GString(tbs_stats.highWaterMark));
		mainLV->setItem(idx, 9, lvItem);				

	}
}
#endif

long  udbapi::importTable(GString dataFile, GString path, GString format, GString statement, GString msgFile, int useLob)
{
   tm("Importing "+dataFile+", path: "+path+", format: "+format+" stmt: "+statement+", msgFile: "+msgFile);
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
   if( useLob )
   {
      //fileTypeMod = (struct sqlchar *)malloc(strlen("lobsinfile compound=100") + sizeof (struct sqlchar));
      //fileTypeMod->length = strlen("lobsinfile compound=100");
      //strncpy (fileTypeMod->data, "lobsinfile compound=100", fileTypeMod->length);

      fileTypeMod = (struct sqlchar *)malloc(strlen("lobsinfile") + sizeof (struct sqlchar));
      fileTypeMod->length = strlen("lobsinfile");
      strncpy (fileTypeMod->data, "lobsinfile", fileTypeMod->length);
   }
   else
   {
      fileTypeMod = (struct sqlchar *)malloc(strlen("FORCECREATE") + sizeof (struct sqlchar));
      fileTypeMod->length = strlen("FORCECREATE");
      strncpy (fileTypeMod->data, "FORCECREATE", fileTypeMod->length);
   }

   pLobPathList = (sqlu_media_list *) malloc (sizeof(sqlu_media_list));
   pLobPathList->media_type = SQLU_LOCAL_MEDIA;
   pLobPathList->sessions = 1;
   pLobPathList->target.media = (sqlu_media_entry *) malloc(sizeof(sqlu_media_entry) * pLobPathList->sessions);
   strcpy (pLobPathList->target.media[0].media_entry, path );

   remove((char*) msgFile); //delete old msgFile
   tm("Starting import...");

   sqluimpr (dataFile, pLobPathList, &columnData, columnStringPointer, format,
      fileTypeMod, msgFile, 0, &impInput, &impOutput, NULL, NULL, &sqlca);
   tm("SQLCODE from sqluimpr: "+GString(sqlca.sqlcode));
   if (sqlca.sqlcode != 0) return sqlca.sqlcode;

   remove((char*) msgFile);
   free(fileTypeMod);
   free(pLobPathList->target.media);
   free(pLobPathList);

   free(columnStringPointer);
   tm("Import Done.");
   return 0;
}
/*************************************************************/
/********************* OLD EXPORT ****************************/
/*************************************************************/

long  udbapi::exportTable(GString dataFile, GString format, GString statement, GString msgFile)
{
//efine  STMTLEN 120

   struct sqldcol       columnData;
   struct sqlchar       *columnStringPointer;
   struct sqluexpt_out  outputInfo;

   /* need to preset the size of structure field and counts */
   outputInfo.sizeOfStruct = SQLUEXPT_OUT_SIZE;

   columnStringPointer = (struct sqlchar *)malloc(statement.length()
      + sizeof (struct sqlchar));

   columnData.dcolmeth = 'D';

   columnStringPointer->length = strlen(statement);
   strncpy (columnStringPointer->data, statement, strlen(statement));

   remove((char*) msgFile); //delete old msgFile
   sqluexpr (dataFile, NULL, NULL, &columnData, columnStringPointer,
      format, NULL, msgFile, 0, &outputInfo, NULL, &sqlca);


   free(columnStringPointer);

   if (sqlca.sqlcode != 0) return sqlca.sqlcode;
   remove((char*) msgFile);
   return 0;

}
/*************************************************************/
/********************* NEW EXPORT ****************************/
/*************************************************************/

long  udbapi::exportTable(GString format, GString statement,
                  GString msgFile, GString path, GString dataFile, int sessions )
{
   tm("Start BlobExp...");
   tm("Path: "+path+", dataFile: "+dataFile+", sessions: "+GString(sessions));

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
      fileTypeMod = (struct sqlchar *)malloc(strlen("lobsinfile") + sizeof (struct sqlchar));
      fileTypeMod->length = strlen("lobsinfile");
      strncpy (fileTypeMod->data, "lobsinfile", fileTypeMod->length);
   }
   else fileTypeMod = NULL;

   tm("Allocating paths...");
   pLobPathList = (sqlu_media_list *) malloc (sizeof(sqlu_media_list));
   pLobPathList->media_type = SQLU_LOCAL_MEDIA;
   pLobPathList->sessions = 1;
   pLobPathList->target.media = (sqlu_media_entry *) malloc(sizeof(sqlu_media_entry) * pLobPathList->sessions);
   strcpy (pLobPathList->target.media[0].media_entry, path );

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
   remove((char*) msgFile); //delete old MsgFile

   tm("Calling API, p1: "+path+dataFile);

   sqluexpr (path+dataFile, pLobPathList, pLobFileList, &columnData, columnStringPointer,
      format, fileTypeMod, msgFile, 0, &outputInfo, NULL, &sqlca);

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

GString udbapi::reorgTable(GString table, GString indexName, GString tabSpace)
{
   if ( indexName.length() && tabSpace.length() ) sqlureot (table, indexName, tabSpace , &sqlca);
   if ( indexName.length() && !tabSpace.length() ) sqlureot (table, indexName, NULL , &sqlca);
   if ( !indexName.length() && tabSpace.length() ) sqlureot (table, 0, tabSpace , &sqlca);
   if ( !indexName.length() && !tabSpace.length() )
   {
       char *tarray = "";
       sqlureot (table, NULL, NULL, &sqlca);
       sqlustat (table, 0, &tarray, SQL_STATS_ALL, SQL_STATS_CHG, &sqlca);
   }
   if( SQLCODE ) return SQLError();
   return "";
}


GString udbapi::runStats(GString table, GSeq <GString> * indList,  unsigned char statsOpt, unsigned char shareLevel )
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
   sqlustat (table, indList->numberOfElements(), indArr, statsOpt, shareLevel, &sqlca);
   for( i = 1; i <= indList->numberOfElements(); ++i) delete indArr[i-1];
   if( SQLCODE ) return SQLError();
   return "";
}

GString udbapi::rebind(GString bindFile)
{
    sqlarbnd (bindFile, &sqlca, NULL);
    if( SQLCODE ) return SQLError();
    return "";
}



/****************************************************************
*
*   GET DB VERSION
*
****************************************************************/
signed int udbapi::getDBVersion(GString alias)
{
   tm("GetDBVers, alias: "+alias);
   int dbv = 1;
   struct sqlca sqlca;
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

int udbapi::stopReq()
{
   return sqleintr();
}




/****************************************************************
*
*   LOAD API
*
****************************************************************/
signed int udbapi::loadFromFile()
{
   GString dataPath = "c:\\";
   GString dataFile = "c:\\BABAG.IXF";
   GString lobPath;
   GString statement = "INSERT INTO PB.BA1";
   GString format = "IXF";
   GString msgFile = "C:\\loadMsg.txt";
   int useLob = 0;


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
   pLobPathList->sessions = 1;
   pLobPathList->target.media = (sqlu_media_entry *) malloc(sizeof(sqlu_media_entry) * pLobPathList->sessions);
   strcpy (pLobPathList->target.media[0].media_entry, dataPath );



   /********************************************************
   * Action
   */
   pActionString = (struct sqlchar *)malloc(strlen(statement)+sizeof (struct sqlchar));
   pActionString->length = strlen(statement);
   strncpy (pActionString->data, statement, strlen(statement));


   /********************************************************
   * Load LOBs...
   */
   if( useLob )
   {
      pFileTypeMod = (struct sqlchar *)malloc(strlen("lobsinfile") + sizeof (struct sqlchar));
      pFileTypeMod->length = strlen("lobsinfile");
      strncpy (pFileTypeMod->data, "lobsinfile", pFileTypeMod->length);
   }
   else
   {
      pFileTypeMod = (struct sqlchar *)malloc(strlen("") + sizeof (struct sqlchar));
      pFileTypeMod->length = strlen("");
      strncpy (pFileTypeMod->data, "", pFileTypeMod->length);
   }

   /********************************************************
   * Need to set this, otherwise the tablespace of the target table will
   * be locked until backup is run.
   */
   pCopyTargetList = (sqlu_media_list *) malloc (sizeof(sqlu_media_list));
   pCopyTargetList->media_type = SQLU_LOCAL_MEDIA;
   pCopyTargetList->sessions = 1;
   pCopyTargetList->target.media = (sqlu_media_entry *) malloc(sizeof(sqlu_media_entry) * pCopyTargetList->sessions);
   strcpy (pCopyTargetList->target.media[0].media_entry, dataPath );



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
   int erc = sqluload( pDataFileList, pLobPathList, &DataDescriptor, pActionString, format, pFileTypeMod,
             msgFile, pRemoteMsgFile, callAction, pLoadInfoIn, pLoadInfoOut, pWorkDirectoryList,
             pCopyTargetList, pNullIndicators, NULL, &sqlca);


   return erc;
}

/***********************************************************************/
/*            END DEFINE DB2 Version 7 and above                       */
/***********************************************************************/

void udbapi::mb(GString msg)
{   
   QMessageBox::information(0, "UDB API", msg);   
}
void udbapi::tm(GString msg)
{   
    return;
    printf(" UDBAPI> %s\n",  (char*) msg);
#ifdef MAKE_VC1
    flushall();
#endif
}

