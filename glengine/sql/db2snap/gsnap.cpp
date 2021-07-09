//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt
//


#include "../../inc/gstring.hpp"
#include <string.h>
#include <stdio.h>

#ifndef _GSNAP_
//#include "gsnap.hpp"
#endif


#include "sqlca.h"
#include "sqlda.h"
#include <sqlenv.h>
#include "sqlutil.h"

//#include <iostream>

#include <sqlmon.h>

#include <db2ApiDf.h>

#define SNAPSHOT_BUFFER_UNIT_SZ 1024
#define NUMELEMENTS 779




static  struct sqlca    sqlca;
//static  struct sqlda    *sqldaPointer;
static  char            SqlMsg[257];

//extern "C"
//{
#ifdef MAKE_VC
   unsigned long __declspec( dllexport ) ModulInit()
   {
printf("IN MOD INIT\n");
flushall();

       return 0L;
   }

   unsigned long  __declspec( dllexport ) ModulClose()
   {
printf("IN MOD CLOSE\n");
flushall();

      return 0L;
   }
   unsigned long  __declspec( dllexport ) moduleToRun ( char* dbName, int type, QListView* mainLV );
#else

/*
   unsigned long extern ModulInit()
   {
printf("IN MOD INIT\n");


       return 0L;
   }

   unsigned long  extern ModulClose()
   {
printf("IN MOD CLOSE\n");


      return 0L;
   }
   unsigned long  extern moduleToRun ( char* dbName, int type, long lv );
*/
#endif   

//}

#ifdef MAKE_VC
extern "C" unsigned long  __declspec( dllexport ) moduleToRun( char* dbName, int type, QListView* mainLV )
#else
extern "C" unsigned long moduleToRun( char* dbName, int type, long lv )
//unsigned long moduleToRun( char* dbName, int type, long lv )
#endif
{
printf("Start ModToRun\n");
GString s;
//gsnap asnap;
//asnap.startMonitors(dbName);
/**
#ifdef MAKE_VC
#else
   QListView * mainLV = (QListView*) lv;
#endif
printf("IN MODTORUN\n");
   gsnap aSnap;
printf("IN MODTORUN 1\n");


   aSnap.startMonitors(dbName);

printf("IN MODTORUN 2.0\n");

printf("IN MODTORUN cols: %i\n", mainLV->columns());


printf("IN MODTORUN 2a\n");


   int erc;

   if( mainLV->columns() == 0 )
   {
      mainLV->addColumn("Statement", 150);
      mainLV->addColumn("Rows Read");
      mainLV->addColumn("Rows Written");
      mainLV->addColumn("Rows Deleted");
      mainLV->addColumn("Rows Inserted");
      mainLV->addColumn("Rows Updated");
      mainLV->addColumn("Executions");
      mainLV->addColumn("Compilations");
      mainLV->addColumn("PrepTime Worst");
      mainLV->addColumn("PrepTime Best");
      mainLV->addColumn("Stmt Sorts");
      mainLV->addColumn("Exec Time (sec.microsec)");
   }
printf("IN MODTORUN 3\n");


   mainLV->clear();
printf("IN MODTORUN 4\n");

   erc = aSnap.getSnapshotData(dbName, SQLMA_DYNAMIC_SQL, mainLV);
printf("IN MODTORUN 5\n");

printf("IN MODTORUN END\n");

   return erc;
**/
}

/*****
gsnap::gsnap()
{
tm("In CTor");
}

gsnap::~gsnap()
{
}

GString gsnap::version()
{
  return "GSNAP v 1.0 (C) 2004 Gregor Leipelt";
}
*****/

