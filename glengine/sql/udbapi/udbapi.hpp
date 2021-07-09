//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 
//


#ifndef _UDBAPI_
#define _UDBAPI_

#ifdef  __IBMCPP__
#define MAKE_VA
#define NO_QT
#endif

#ifdef  _MSC_VER
  #ifndef MAKE_VC
    #define MAKE_VC
  #endif
#endif

#include <qtablewidget.h>
#include <iudbapi.hpp>

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
#ifdef Makeudbapi
  #define udbapiExp_Imp _Export
#else
  #define udbapiExp_Imp _Import
//  #pragma library( "udbapi.LIB" )
  #pragma library( "GLEngine.LIB" )
#endif
#endif
//************** Visual Age END *****************

#include <gstring.hpp>
#include <string.h>
#include <stdio.h>
#include <gseq.hpp>
#include <gthread.hpp>


// ******************************* CLASS **************************** //
#ifdef MakeGString
  class udbapiExp_Imp udbapi
#else
  class udbapi : public IUDBAPI
#endif
{


   class MainThread : public GThread
   {
      public:
         virtual void run();
         void setOwner( udbapi * aUDB ) { myUDBApi = aUDB; }
      private:
         udbapi * myUDBApi;
   };

  private:
     MainThread * mainThread;

     void tm(GString msg);
     void mb(GString msg);
     int fillLV();
     GString nodeName, dataBase, user, pwd;
     int snType;

     QMessageBox * runInfo;


  public:
    VCExport  udbapi(){}
    VCExport  udbapi(GString database, GString ndName, GString user, GString pwd, int type, QTableWidget * pLV);
    VCExport  ~udbapi(){}
    VCExport  GString SQLError();
    VCExport  void startMainThread();

    VCExport  long    importTable(GString dataFile, GString path, GString format, GString statement, GString msgFile, int useLob = 1);
    VCExport  long    exportTable(GString dataFile, GString format, GString statement, GString msgFile);
    VCExport  GString reorgTable(GString table, GString indexName="", GString tabSpace = "");
    VCExport  GString runStats(GString table, GSeq <GString> * indList, unsigned char statsOpt, unsigned char shareLevel );
    VCExport  GString rebind(GString bindFile);

    VCExport  long  exportTable(GString format, GString statement, GString msgFile, GString path, GString dataFile, int sessions );
    VCExport  signed int getDBVersion(GString alias);
    VCExport  int stopReq();

    VCExport  signed int loadFromFile();

    /* STUFF FOR SNAPSHOT */
#ifdef SQLM_DBMON_VERSION7
  public:
    VCExport  int getSnapshotData();
    VCExport  int startMonitors();
    VCExport  int resetMonitor();
    VCExport int initTabSpaceLV(QTableWidget * mainLV);
    
  private:
     int iMonVersion;
     sqlm_header_info* jumpToKey(int element, char* start, GString & data);
     int processData(char * buffer_ptr, QTableWidget * pLV, int type);
     GString getData(sqlm_header_info * pHeader, char* data);


     int iGetTime;
     int fillTabList(char * pStart, QTableWidget * pLV);
     int fillDSQL(char * pStart, QTableWidget * pLV);
     int fillLock(char * pStart, QTableWidget * pLV);
     int initSnapshot(struct sqlma *pSqlMA, QTableWidget * pLV, int type);
     int FreeMemory(struct sqlma *ma_ptr, char *buffer_ptr);
     int initELMArray();
     void processBuffer(sqlm_header_info *datastream);
    void  fillTabSpaceLV (struct SQLB_TBSPQRY_DATA *dataP, sqluint32 num, QTableWidget * mainLV);
     QTableWidgetItem * lvItem;
     int fillLV(char * pStart, QTableWidget * pLV, int key, GSeq <int> * keySeq, GSeq <GString> * nameSeq);     
     QTableWidget * mainLV;
#endif
};

#endif





