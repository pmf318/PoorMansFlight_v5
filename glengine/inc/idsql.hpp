
//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt
//
/******************************************************************
 *
 * Interface for dsqlobj (DB2) and odbcdsql (ODBC, SqlServer)
 *
 *****************************************************************/
#ifndef _IDSQL_
#define _IDSQL_

#ifdef  __IBMCPP__
#define MAKE_VA
#define NO_QT
#endif

#ifdef  _MSC_VER
  #ifndef MAKE_VC
    #define MAKE_VC
  #endif
#endif

#ifdef MAKE_VC
#elif 	__MINGW32__
#else
    #if defined(__GNUC__) && __GNUC__ < 3
        #include ostream.h>
    #else
        #include <ostream>
        #include<iostream>
        #include<dlfcn.h>
    #endif
#endif

//************** Visual C++ ********************
#ifdef MAKE_VC
   #define DllImport   __declspec( dllimport )
   #define DllExport   __declspec( dllexport )
   #define VCExport    DllExport
#else
   #define VCExport
#endif
//(************** Visual C++ END *****************

//************** Visual Age *********************
#ifdef MAKE_VA
  #define NO_QT
#ifdef MakeIDSQL
  #define dynsqlExp_Imp _Export
#else
  #define dynsqlExp_Imp _Import
  #pragma library( "GLEngine.LIB" )
#endif
#endif
//************** Visual Age END *****************

//Includes from QT4 Lib
#ifdef QT4_DSQL
   #include <qlistwidget.h>
   #include <qtablewidget.h>
   #include <qprogressdialog.h>
#endif

//Includes from QT(3) Lib
#ifndef NO_QT
   #include <qlistview.h>
   #include <qlabel.h>
   #include <qlistbox.h>
   #include <qmessagebox.h>
   #include <qprogressdialog.h>
#endif


#include <gstring.hpp>
#include <gseq.hpp>
#include <gdebug.hpp>



//static int m_IDSQCounter = 0;

#define PMF_UNUSED(expr) (void)(expr)

#define DEF_IDX_DUPL "DUPL. ALLOWED"
#define DEF_IDX_PRIM "PRIMARY KEY"
#define DEF_IDX_FORKEY "FOREIGN KEY"
#define DEF_IDX_UNQ "UNIQUE IDX"

enum ODBCDB
{
        NOTSET = 0,
        DB2,
        DB2ODBC,
        SQLSERVER,
        ORACLE,
        ODBC,
        MARIADB,
        POSTGRES,
        DBTEMPLATE
};
enum COLTYPE
{
    CT_UNKNOWN = 1,
    CT_STRING,
    CT_INTEGER,
    CT_LONG,
    CT_CLOB,
    CT_BLOB,
    CT_DBCLOB,
    CT_XML,
    CT_DATE,
    CT_DECIMAL,
    CT_FLOAT,
    CT_GRAPHIC,
    CT_PMF_RAW
};


enum DISP_DATA
{
    DATA_HIDE = 1,
    DATA_BIN,
    DATA_HEX,
    DATA_AUTO
};

enum TABLE_TYPE
{
    TYPE_UNKNOWN = 1,
    TYPE_ALIAS,
    TYPE_CRT_TEMP_TABLE,
    TYPE_HIR_TABLE,
    TYPE_DEL_TABLE,
    TYPE_DET_TABLE,
    TYPE_NICK,
    TYPE_MAT_QUERY,
    TYPE_UNTYPED_TABLE,
    TYPE_TYPED_TABLE,
    TYPE_UNTYPED_VIEW,
    TYPE_TYPED_VIEW
};

typedef struct {
    TABLE_TYPE TableType;
    GString BaseTabSchema;
    GString BaseTabName;
    void init(){TableType = TYPE_UNKNOWN; BaseTabSchema = ""; BaseTabName = "";}
}  TABLE_PROPS;


typedef struct {
    GString Type;
    GString DB;
    GString UID;
    GString PWD;
    GString Host;
    GString Port;
    int DefDB;
    int CSVer;
}  CON_SET;

typedef struct {
    GString ColName;
    GString ColType;
    GString Length;
    GString Scale;
    GString Nullable;
    GString Default;
    GString Logged;
    GString Compact;
    GString Identity;
    GString Generated;
    GString Misc;
    void init(){ColName = ""; ColType = ""; Length = ""; Scale = ""; Nullable = ""; Default = "";
          Logged = ""; Compact = ""; Identity = ""; Generated = ""; Misc = ""; }
} COL_SPEC;


typedef struct {
    GString Iidx;
    GString Schema;
    GString Name;
    GString Columns;
    GString Type;
    GString TabSpace;
    GString CreateTime;
    GString Definer;
    GString StatsTime;
    GString DeleteRule;
    GString IsDisabled;
    GString Stmt;
    GString RefCols;
    GString RefTab;
    void init(){Iidx = ""; Schema = ""; Name = ""; Columns = ""; Type = ""; TabSpace = "";
          CreateTime = ""; Definer = ""; StatsTime = ""; DeleteRule = ""; Stmt = "";
          RefCols = ""; RefTab = ""; }
} IDX_INFO;




static GString _HOST_DEFAULT  = "[local]";
static GString _PORT_DEFAULT  = "[default]";
static GString _SQLSRV        = "SQLServer";
static GString _DB2           = "DB2";
static GString _DB2ODBC       = "DB2(ODBC)";
static GString _ORAODBC       = "Oracle(ODBC)";
static GString _MARIADB       = "MariaDB";
static GString _POSTGRES      = "Postgres";
static GString _DBTEMPLATE    = "DbTemplate";


#define PMF_UNUSED(expr) (void)(expr)

// ******************************* CLASS **************************** //
#ifdef MakeIDSQL
  class IDSQLExp_ImpIDSQL 
#else
  class IDSQL
#endif
{

  public:              
 
    IDSQL(){} //short commit = 1, short setEmbrace = 1, short charForBit = 3
    //IDSQL( IDSQL const & )  {  }
    virtual ~IDSQL()
    {
        //printf("IDSQL Dtor\n");
    }

    virtual IDSQL* clone() const = 0;

    virtual ODBCDB getDBType() = 0;
    virtual GString getDBTypeName() = 0;


    //virtual const IDSQL& operator=( const IDSQL& f ) = 0;
    virtual GString connect(GString db, GString uid, GString pwd, GString host = "", GString port = "") = 0;
    virtual int disconnect() = 0;
    virtual void currentConnectionValues(CON_SET * conSet) = 0;
    virtual GString initAll(GString message, unsigned long maxRows = 0, int getLen = 0) = 0;
    virtual void setTruncationLimit(int limit) = 0;
    virtual int commit() = 0;
    virtual int rollback() = 0;
    virtual unsigned int numberOfColumns() = 0;
    virtual unsigned long numberOfRows() = 0;
    virtual GString rowElement(unsigned long row, int col) = 0;
    virtual GString rowElement(unsigned long row, GString hostVar) = 0;
    virtual GString hostVariable(int col) = 0;
    virtual int positionOfHostVar(const GString& hostVar) = 0;
		
    virtual	int sqlCode() = 0;
    virtual GString sqlError() = 0;
	
	
    virtual	int getTabSchema() = 0;
    virtual	int getTables(GString schema) = 0;
    virtual	void convToSQL(GString & input) = 0;
	
    virtual int getAllTables(GSeq <GString > * tabSeq, GString filter = "")  = 0;
    virtual short sqlType(const short & col) = 0;
    virtual short sqlType(const GString & colName) = 0;
    virtual GString realName(const short & sqlType) = 0;

    virtual  int initRowCrs() = 0;
    virtual  int nextRowCrs() = 0;
    virtual  GString dataAtCrs(int col) = 0;

    virtual  signed long getCost() = 0;

    virtual void setStopThread(int stop) = 0;
    virtual void setDBType(ODBCDB dbType) = 0;

    virtual int execByDescriptor( GString cmd, GSeq <GString> *dataSeq) = 0;
    virtual int execByDescriptor( GString cmd, GSeq <GString> *dataSeq, GSeq <int> *typeSeq,
                                    GSeq <short>* sqlVarLengthSeq, GSeq <int> *forBitSeq ) = 0;

    virtual int isNumType(int pos) = 0;
    virtual int isDateTime(int pos) = 0;
    virtual int hasForBitData() = 0;
    virtual int isForBitCol(int i) = 0;
    virtual int isBitCol(int i) = 0;
    virtual int isXMLCol(int i) = 0;
    virtual int isLOBCol(int i) = 0;
    virtual int isLongTypeCol(int i) = 0;
    virtual int simpleColType(int i) = 0;
    virtual int isFixedChar(int i) = 0;
    virtual int isNullable(int i) = 0;


    virtual void setCharForBit(int val) = 0;
    virtual GString descriptorToFile( GString cmd, GString &blobFile, int * outSize ) = 0;
    virtual void setReadUncommitted(short readUncommitted = 1) = 0;
    virtual short bindIt(GString bndFile, GString database, GString user, GString pwd, GString msgFile) = 0;

    virtual GString  currentCursor(GString filter, GString command, long curPos, short commitIt = 1,
                                   GSeq <GString> *fileList = NULL, GSeq <long> *lobType = NULL) = 0;

    virtual long uploadBlob(GString cmd, GSeq <GString> *fileList, GSeq <long> *lobType) = 0;
    virtual long uploadBlob(GString cmd, char * buffer, long size) = 0;
    virtual long dataLen(const short & pos) = 0;
    virtual int  getDataBases(GSeq <CON_SET*> *dbList) = 0;

    //Pack this into helper libs...
    virtual int  deleteTable(GString tableName) = 0;
    virtual long retrieveBlob( GString cmd, GString &blobFile, int writeFile = 1 ) = 0;

    #ifdef  QT4_DSQL
    virtual GString getIdenticals(GString table, QWidget* parent, QListWidget *pLB, short autoDel) = 0;
    #endif
    virtual GString fillChecksView(GString table, int showRaw = 0) = 0;
    virtual int forceApp(int appID) = 0;
    virtual void setCLOBReader(short readCLOBData ) = 0;

    virtual int getSysTables() = 0;
    virtual void setCurrentDatabase(GString db) = 0;
    virtual GString currentDatabase() = 0;
    virtual  unsigned long getResultCount(GString cmd) = 0;
    virtual int getColSpecs(GString table, GSeq<COL_SPEC*> *specSeq) = 0;
    virtual int getTriggers(GString table, GString *text) = 0;

    virtual GSeq <GString> getTriggerSeq(GString table) = 0;
    virtual GSeq <IDX_INFO*> getIndexeInfo(GString table) = 0;


    virtual GString getChecks(GString table, GString filter = "") = 0;
    virtual GSeq <GString> getChecksSeq(GString table, GString filter = "") = 0;

    virtual int getUniqueCols(GString table, GSeq <GString> * colSeq) = 0;
    virtual void createXMLCastString(GString &xmlData) = 0;
    virtual int  isBinary(unsigned long row, int col) = 0;
    virtual int  isTruncated(unsigned long row, int col) = 0;
    virtual int uploadLongBuffer( GString cmd, GString data, int isBinary) = 0;
    virtual GString cleanString(GString in) = 0;
    virtual void getResultAsHEX(int asHex) = 0;
    virtual void setGDebug(GDebug *pGDB) = 0;
    virtual int exportAsTxt(int mode, GString sqlCmd, GString table, GString outFile, GSeq <GString>* startText, GSeq <GString>* endText, GString *err) = 0;
    virtual  int getHeaderData(int pos, GString * data) = 0;
    virtual  int getRowData(int row, int col, GString * data) = 0;
    virtual unsigned long getRowDataCount() = 0;
    virtual unsigned long getHeaderDataCount() = 0;
    virtual int isTransaction() = 0;
    virtual void setDatabaseContext(GString context) = 0;
    virtual int hasUniqueConstraint(GString tableName) = 0;
    virtual GString getDdlForView(GString tableName) = 0;
    virtual void setAutoCommmit(int commit) = 0;
    virtual GString lastSqlSelectCommand() = 0;
    virtual TABLE_PROPS getTableProps(GString tableName) = 0;
    virtual int tableIsEmpty(GString tableName) = 0;
    virtual int deleteViaFetch(GString tableName, GSeq<GString> * colSeq, int rowCount, GString whereClause) = 0;
};

#endif





