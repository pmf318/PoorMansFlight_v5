//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 
//


#ifndef _DSQLPLUGIN_
#define _DSQLPLUGIN_

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
#include <gdebug.hpp>

#include <idsql.hpp>

//This is global
extern GString m_strDSQLPluginPath;
#define PluginLoaded 0
#define PluginMissing 1
#define PluginNotLoadable 2

#ifdef MakePlugin
  class DSQLPluginExp_Imp DSQLPlugin
#else
  class DSQLPlugin  : IDSQL
#endif
{
private:
    #ifdef MAKE_VC
        HINSTANCE dsqlHandle;
		typedef IDSQL* (*CREATE) ();
        typedef void   (*DESTROY)(IDSQL*);
		CREATE crt;
		DESTROY destr;
    #elif __MINGW32__
      void* dsqlHandle;
      typedef IDSQL* (*CREATE) ();
      typedef void   (*DESTROY)(IDSQL*);
      CREATE crt;
      DESTROY destr;
    #else
        void *dsqlHandle;
		typedef IDSQL* create_t();
		typedef void destroy_t(IDSQL*);
		create_t* crt;
		destroy_t* destr;
    #endif
	

	IDSQL* m_pIDSQL;

	void init();
    int loadPlugin(GString libPath);
    int callLoader(GString dbName, GString file = "");
    void deb(GString msg);
    int setPathEnvironment();
    GString m_strDBTypeName;
    int m_iPluginLoaded;
    int m_iMyInstanceCounter;
    //GString m_strDB, m_strUID, m_strPWD, m_strNode, m_strHost;
    int m_iLoadError;
    GDebug * m_pGDeb;



public:    
   VCExport   DSQLPlugin(GString dbTypeName, GString file = "");
   VCExport   DSQLPlugin(const DSQLPlugin& plg);
   VCExport   DSQLPlugin& operator= (DSQLPlugin const& in);
   VCExport   ~DSQLPlugin();

   VCExport DSQLPlugin* clone() const;

   VCExport static void setPluginPath(GString path);
   VCExport static GString pluginPath();



   VCExport   int isOK();
   VCExport   int loadError();

   //List of Databasetypes: DB2, SQlServer,...
   VCExport static void DBTypeNames(GSeq <GString>* list);


   /************************************************
    * Interface implementation
    ***********************************************/
   VCExport ODBCDB getDBType();
   VCExport GString getDBTypeName();
   //VCExport IDSQL( const IDSQL & );

   //VCExport const IDSQL& operator=( const IDSQL& f );
   VCExport GString connect(GString db, GString uid, GString pwd, GString host = "", GString port = "") ;
   VCExport int disconnect();
   VCExport GString initAll(GString message, unsigned long maxRows = 0, int getLen = 0);
   VCExport int commit();
   VCExport int rollback();
   VCExport unsigned int numberOfColumns();
   VCExport unsigned long numberOfRows();
   VCExport GString rowElement(unsigned long row, int col);
   VCExport GString rowElement(unsigned long row, GString hostVar);
   VCExport GString hostVariable(int col);
   VCExport int positionOfHostVar(const GString& hostVar);

   ///VCExport	GString currentCursor(GString filter, GString command, long curPos, short commitIt);

   VCExport	int sqlCode();
   VCExport GString sqlError();


   ////VCExport	int uploadBlob(GString cmd, GSeq <GString> *fileSeq, GSeq <GString> * lobTypeSeq);
   VCExport	int getTabSchema();
   VCExport	int getTables(GString schema);
   VCExport	void convToSQL(GString & input);

   VCExport int getAllTables(GSeq <GString > * tabSeq, GString filter = "") ;
   VCExport short sqlType(const short & col);
   VCExport short sqlType(const GString & colName);
   VCExport GString realName(const short & sqlType);

   VCExport  int initRowCrs();
   VCExport  int nextRowCrs();
   VCExport  GString dataAtCrs(int col);

   VCExport  signed long getCost();
   ////VCExport  GString descriptorToFile( GString cmd, GString &blobFile ) ;
   VCExport void setStopThread(int stop);
   VCExport void setDBType(ODBCDB dbType);

   VCExport int execByDescriptor( GString cmd, GSeq <GString> *dataSeq);
   VCExport int execByDescriptor( GString cmd, GSeq <GString> *dataSeq, GSeq <int> *typeSeq,
                                   GSeq <short>* sqlVarLengthSeq, GSeq <int> *forBitSeq );

   VCExport int isNumType(int pos);
   VCExport int isDateTime(int pos);
   VCExport int hasForBitData();
   VCExport int isForBitCol(int i);
   VCExport int isBitCol(int i);
   VCExport int isXMLCol(int i);
   VCExport int isLOBCol(int i);
   VCExport int isNullable(int i);


   VCExport int simpleColType(int i);
   VCExport void setCharForBit(int val);
   VCExport GString descriptorToFile( GString cmd, GString &blobFile, int * outSize );
   VCExport void setReadUncommitted(short readUncommitted = 1);
   VCExport short bindIt(GString bndFile, GString database, GString user, GString pwd, GString msgFile);

   VCExport GString  currentCursor(GString filter, GString command, long curPos, short commitIt = 1,
                                  GSeq <GString> *fileList = NULL, GSeq <long> *lobType = NULL);

   VCExport long uploadBlob(GString cmd, GSeq <GString> *fileList, GSeq <long> *lobType);
   VCExport long uploadBlob(GString cmd, char * buffer, long size);
   VCExport long dataLen(const short & pos);
   VCExport int  getDataBases(GSeq <CON_SET*> *dbList);

   //Pack this into helper libs...
   VCExport int  deleteTable(GString tableName);
   VCExport long retrieveBlob( GString cmd, GString &blobFile, int writeFile = 1 );

   #ifdef  QT4_DSQL
   VCExport GString getIdenticals(GString table, QWidget* parent, QListWidget *pLB, short autoDel);
   #endif
   VCExport GString fillChecksView(GString table, int showRaw = 0);
   VCExport int forceApp(int appID);
   VCExport void setCLOBReader(short readCLOBData );

   VCExport int getSysTables();
   VCExport void setCurrentDatabase(GString db);
   VCExport GString currentDatabase();
   VCExport unsigned long getResultCount(GString cmd);
   VCExport int getColSpecs(GString table, GSeq<COL_SPEC*> *specSeq);

   VCExport GSeq <IDX_INFO*> getIndexeInfo(GString table);

   VCExport int getTriggers(GString table, GString *text);
   VCExport GSeq <GString> getTriggerSeq(GString table);


   GString getChecks(GString table, GString filter = "");
   VCExport GSeq <GString> getChecksSeq(GString table, GString filter = "");

   VCExport int getUniqueCols(GString table, GSeq <GString> * colSeq);
   VCExport void createXMLCastString(GString &xmlData);
   VCExport int isBinary(unsigned long row, int col);
   VCExport int isTruncated(unsigned long row, int col);
   VCExport int isLongTypeCol(int i);
   VCExport int isFixedChar(int i);
   VCExport void setTruncationLimit(int limit);
   VCExport int uploadLongBuffer( GString cmd, GString data, int isBinary);
   VCExport GString cleanString(GString in);
   VCExport void getResultAsHEX(int asHex);
   VCExport void setGDebug(GDebug *pGDB);
   VCExport int exportAsTxt(int mode, GString sqlCmd, GString table, GString outFile, GSeq <GString>* startText, GSeq <GString>* endText, GString *err);

    VCExport  int getHeaderData(int pos, GString * data);
    VCExport  int getRowData(int row, int col, GString * data);
    VCExport unsigned long getRowDataCount();
    VCExport unsigned long getHeaderDataCount();
    VCExport int isTransaction();
    VCExport void setDatabaseContext(GString context);
    VCExport int hasUniqueConstraint(GString tableName);
    VCExport GString getDdlForView(GString tableName);
    VCExport void setAutoCommmit(int commit);
    VCExport void currentConnectionValues(CON_SET * conSet);
    VCExport GString lastSqlSelectCommand();
    VCExport TABLE_PROPS getTableProps(GString tableName);
    VCExport int tableIsEmpty(GString tableName);
    VCExport int deleteViaFetch(GString tableName, GSeq<GString> * colSeq, int rowCount, GString whereClause);


};
#endif

