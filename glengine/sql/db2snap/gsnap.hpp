//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 
//


#ifndef _GSNAP_
#define _GSNAP_

#ifdef  __IBMCPP__
#define MAKE_VA
#define NO_QT
#endif

#ifdef  _MSC_VER
  #ifndef MAKE_VC
    #define MAKE_VC
  #endif
#endif

#include <qlistview.h>

#include <sqlmon.h>

#include <db2ApiDf.h>

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
#ifdef Makegsnap
  #define gsnapExp_Imp _Export
#else
  #define gsnapExp_Imp _Import
//  #pragma library( "gsnap.LIB" )
  #pragma library( "GLEngine.LIB" )
#endif
#endif
//************** Visual Age END *****************

#include <gstring.hpp>
#include <string.h>
#include <stdio.h>
#include <gseq.hpp>

// ******************************* CLASS **************************** //
#ifdef MakeGString
  class gsnapExp_Imp gsnap
#else
  class gsnap
#endif
{
  private:
     void tm(GString msg);
     int fillLV();


  public:
    VCExport  gsnap();
    VCExport  ~gsnap();
    VCExport  GString version();

    VCExport  GString SQLError();
    VCExport      GString sqlError();

    /* STUFF FOR SNAPSHOT */
#ifdef SQLM_DBMON_VERSION7
  public:
    VCExport  int getSnapshotData(GString database, int type, QListView *aLV);
    VCExport  int startMonitors(GString database);

  private:
     int iMonVersion;
     sqlm_header_info* jumpToKey(int element, char* start, GString & data);
     int processData(char * buffer_ptr, QListView * pLV, int type);
     GString getData(sqlm_header_info * pHeader, char* data);
     int fillLV(char * pStart, QListView * pLV, int key, GSeq <int> * keySeq, GSeq <GString> * nameSeq);
     QListViewItem * lvItem;
     int iGetTime;
     int fillTabList(char * pStart, QListView * pLV);
     int fillDSQL(char * pStart, QListView * pLV);
     int fillLock(char * pStart, QListView * pLV);
     int initSnapshot(struct sqlma *pSqlMA, QListView * pLV, int type);
     int FreeMemory(struct sqlma *ma_ptr, char *buffer_ptr);
     void processBuffer(sqlm_header_info *datastream);
#endif
};

#endif





