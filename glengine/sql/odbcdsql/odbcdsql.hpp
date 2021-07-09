//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt
//

#ifndef _ODBCDSQL_
#define _ODBCDSQL_

#ifdef  __IBMCPP__
#define MAKE_VA
#define NO_QT
#endif

#ifdef  _MSC_VER
  #ifndef MAKE_VC
    #define MAKE_VC
  #endif
#endif


//#ifndef _IDSQL_
#include <idsql.hpp>
#include <growhdl.hpp>
//#endif


#include <sql.h>
#include <sqlext.h>

#ifndef MAKE_VC
//#include <odbcss.h>
#endif


//************** Visual C++ ********************
#ifdef MAKE_VC
   #define DllImport   __declspec( dllimport )
   #define DllExport   __declspec( dllexport )
   #define VCExport    DllExport
   #include <windows.h>
#else
   #define VCExport   
#endif
//(************** Visual C++ END *****************

//************** Visual Age *********************
#ifdef MAKE_VA
  #define NO_QT
#ifdef MakeodbcDSQL
  #define dynsqlExp_Imp _Export
#else
  #define dynsqlExp_Imp _Import
  #pragma library( "GLEngine.LIB" )
#endif
#endif

//************** Visual Age END *****************

/**************************************************
 * Apparently, TDS 8.0 actually is synonymous to TDS 7.1,
 * whereas TDS 9.0 was renamed to TDS 7.2
 * see http://www.freetds.org/userguide/tdshistory.htm
 * So, TDS 7.2 (aka 9.0) is better/newer than TDS 8.0 (aka 7.1)
 *
 * On Windows however we need to set USE_TDS71 to fork in a few places in odbcdsql.cpp
 * 
 * Note to self:
 * On Linux, the TDS version is set in odbc.ini (not in freetds.conf)
 *************************************************/


#ifdef MAKE_VC /////////// Windows:
#define SQLSRV_BLOB     -3
#define SQLSRV_CLOB     12
#define SQLSRV_XML      -152
#define SQLSRV_GUID     SQL_GUID
#define SQLSRV_DATETIME 93
#define SQLSRV_NVARCHAR -9
#define SQLSRV_BIT      -7
#define SQLSRV_FIXEDCHAR -8
#define USE_TDS71 1
#define SQLSRV_BIGINT   -5
#else  ////////////Linux:

#define SQLSRV_BLOB     SQL_LONGVARBINARY
#define SQLSRV_CLOB     SQL_LONGVARCHAR
#define SQLSRV_BIGINT   -5

#define USE_TDS72 1
#ifdef USE_TDS71
    #define SQLSRV_XML      -10
#else
    #ifdef SQL_SS_XML
        #define SQLSRV_XML SQL_SS_XML
    #else
        #define SQLSRV_XML      1
    #endif
#endif

#define SQLSRV_GUID     SQL_GUID
#define SQLSRV_DATETIME 93
#define SQLSRV_NVARCHAR -9

#ifdef SQL_BIT
    #define SQLSRV_BIT SQL_BIT
#else
    #define SQLSRV_BIT -7
#endif

#define SQLSRV_FIXEDCHAR -8

#endif

// ******************************* CLASS **************************** //

#ifdef odbcDSQL
  class odbcDSQLExp_Imp odbcDSQL
#else
  class odbcDSQL : public IDSQL
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
            GString elementAtPosition(int pos);
            unsigned long numberOfElements();
    };
	
    private:
	
      SQLHENV      m_SQLEnv;
      SQLHDBC      m_SQLHDBC;
      SQLHSTMT     m_SQLHSTMT;
      void deb(GString text);
      void deb(GString fnName, GString text);
      
      //QString uuidToString(const QVariant &v);
      int m_iMyInstance;

      ODBCDB m_odbcDB;
      GString m_strDB, m_strUID, m_strPWD, m_strNode, m_strHost;
      int m_iPort;
      signed int m_iNumberOfColumns;
      unsigned long m_iNumberOfRows;

      unsigned long m_ulMaxRows;
      unsigned long m_ulFetchedRows;
      GString m_strCurrentDatabase;
      int m_iTruncationLimit;
      int m_iReadUncommitted;
      int m_iIsTransaction;


      GSeq <GRowHdl* > allRowsSeq;
      GRowHdl *m_pRowAtCrs;


      GSeq <GString> hostVarSeq;
      GSeq <short>   sqlTypeSeq;
      GSeq <short>   sqlVarLengthSeq;
      GSeq <short>   sqlIndVarSeq;
      GSeq <short>   sqlForBitSeq;
	  GSeq <short>   sqlBitSeq;
      GSeq <short>   sqlLongTypeSeq;
      GSeq <short>   xmlSeq; //1: hostvar type is XML, 0: else
      GSeq <short>   lobSeq; //1: hostvar type is LOB, 0: else
      GSeq <short>   simpleColTypeSeq; //Simple type specifiers: LOB, XML, CHAR,...
      GSeq <long>    sqlLenSeq;

      GSeq <CON_SET*> m_seqConSet;
	  GSeq<GString> headerSeq;
      GSeq<RowData*> rowSeq;
      GString m_strContext;
      GString m_strLastSqlSelectCommand;

      void clearSequences();


      void resetAllSeq();
      int getColInfo(SQLHSTMT * stmt);
      GString sqlError(SQLHSTMT stmt);
      int loadFileIntoBuf(GString fileName, unsigned char** fileBuf, SQLLEN *size, int type = 1);
      //int loadFileIntoBuf(GString fileName,  char** fileBuf, SQLLEN *size, int type = 1);	  
      GString m_strLastError;
	  int     m_iLastSqlCode;
      void freeStmt(SQLHSTMT);
      int writeToFile(GString fileName, char** buf, int len);
      int handleLobsAndXmls(int col, GString * out, int getFullXML, short* isNull);
	  void handleDateTimeString(GString &in);
      GString tabSchema(GString table);
      GString tabName(GString table);
	  GString context(GString table);

	  void createIndexRow(QTableWidget *pWdgt, int row, 
	GString id, GString name, GString cols, GString unq, GString crt, GString mod, GString dis);
      int getIdentityColParams(GString table, int *seed, int * incr);
	  void errorState(SQLHSTMT hstmt);

       GString readRowData(GString message, unsigned long maxRows = 0, int getLen = 0, int getFullXML = 0);
       //GString fillIndexViewRawData(GString table);
	   
       //New: Keep
       GString getIndexStatement(int type, GString table, GString indexName);
       GString getForeignKeyStatement(GString table, GString keyName );

       GDebug * m_pGDB;
  public:                              // Public section
    //VCExport ODBCDSQL(ODBCDSQL & aODBC);
    //VCExport ODBCDSQL &operator=(ODBCDSQL aODBC);


    VCExport odbcDSQL();
    VCExport ~odbcDSQL();
    VCExport odbcDSQL(odbcDSQL const & o);
    VCExport odbcDSQL* clone() const;



    VCExport ODBCDB getDBType()
    {
        return SQLSERVER;
    }
    VCExport int getConnData(GString * db, GString *uid, GString *pwd) const;
    VCExport GString getDBTypeName(){ return _SQLSRV; }
	VCExport void setDBType(ODBCDB dbType){ m_odbcDB = dbType;}
    VCExport GString connect(GString db, GString uid, GString pwd, GString host = "", GString port = "");
	VCExport int disconnect();
	VCExport GString initAll(GString message, unsigned long maxRows = 0, int getLen = 0);
	VCExport int commit();
	VCExport int rollback();
    VCExport unsigned int  numberOfColumns();
	VCExport unsigned long numberOfRows();
	VCExport GString rowElement(unsigned long row, int col);
	VCExport GString rowElement(unsigned long row, GString hostVar);
	VCExport GString hostVariable(int col);
	

    VCExport GString currentCursor(GString filter, GString command, long curPos, short commitIt = 1,
                                    GSeq <GString> *fileList = NULL, GSeq <long> *lobType = NULL);

	
	VCExport int sqlCode();
    VCExport GString sqlError();
	


    VCExport int getTabSchema();
	VCExport int getTables(GString schema);
	VCExport void convToSQL(GString & input);
    VCExport short sqlType(const short & col);
    VCExport short sqlType(const GString & colName);
	VCExport GString realName(const short & sqlType);

	VCExport int initRowCrs();
	VCExport int nextRowCrs();
	VCExport GString dataAtCrs(int col);
	
	
	VCExport int getAllTables(GSeq <GString > * tabSeq, GString filter = "");

    VCExport GString descriptorToFile( GString cmd, GString &blobFile, int * outSize );

	VCExport signed long getCost();
    VCExport void setStopThread(int stop){PMF_UNUSED(stop);}
	//TODO: Operator=
    //TODO: Ctor(ODBCDSQL)

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
    VCExport int isLongTypeCol(int i);
    VCExport int isFixedChar(int i);
    VCExport int simpleColType(int i);
    VCExport void setCharForBit(int val){PMF_UNUSED(val);}
    VCExport void setReadUncommitted(short readUncommitted = 1);

    VCExport short bindIt(GString, GString, GString, GString, GString){ return 0;}


    VCExport long uploadBlob(GString cmd, GSeq <GString> *fileList, GSeq <long> *lobType);
    VCExport long uploadBlob(GString cmd, char * buffer, long size);
    VCExport int getDataBases(GSeq <CON_SET*> *dbList);



    long dataLen(const short &pos);

    VCExport int positionOfHostVar(const GString& hostVar);
    //Pack this into helper libs...
    VCExport int  deleteTable(GString tableName);

    VCExport void setCurrentDatabase(GString db);
    VCExport GString currentDatabase();

     long retrieveBlob( GString cmd, GString &blobFile, int writeFile = 1 );


    #ifdef  QT4_DSQL
    VCExport GString getIdenticals(GString table, QWidget* parent, QListWidget *pLB, short autoDel);
	VCExport GString  fillIndexView(GString table, QWidget* parent, QTableWidget *pWdgt);	
    VCExport void writeToLB(QListWidget * pLB, GString message);
    #endif


    VCExport int forceApp(int appID){ PMF_UNUSED(appID);return -1;}
    VCExport void setCLOBReader(short readCLOBData ){PMF_UNUSED(readCLOBData);}

    VCExport int getSysTables(){ return -1;}
    VCExport unsigned long getResultCount(GString cmd);
    VCExport  int getColSpecs(GString table, GSeq<COL_SPEC*> *specSeq);
    VCExport  int getTriggers(GString table, GString *text);
    VCExport GSeq <IDX_INFO*> getIndexeInfo(GString table);
    VCExport GSeq <GString> getTriggerSeq(GString table);

    VCExport GString getChecks(GString, GString = "");
    VCExport GSeq <GString> getChecksSeq(GString, GString);

    VCExport  int getUniqueCols(GString table, GSeq <GString> * colSeq);
    VCExport void createXMLCastString(GString &xmlData);
    VCExport int isBinary(unsigned long row, int col);
    VCExport int  isNull(unsigned long row, int col);
    VCExport int isTruncated(unsigned long row, int col);
    VCExport void setTruncationLimit(int limit);
    VCExport int uploadLongBuffer( GString cmd, GString data, int isBinary );
    VCExport GString cleanString(GString in);
    VCExport void getResultAsHEX(int asHex);
    VCExport void setGDebug(GDebug *pGDB);
    VCExport int exportAsTxt(int mode, GString sqlCmd, GString table, GString outFile, GSeq <GString>* startText, GSeq <GString>* endText, GString *err);
    VCExport GString fillIndexView(GString table, int showRaw = 0);
    VCExport GString fillChecksView(GString table, int showRaw = 0);
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







