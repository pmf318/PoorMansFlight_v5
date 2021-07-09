//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 
//



#ifndef _DB2DCLI_
#define _DB2DCLI_

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

//************** Visual Age *********************
#ifdef MAKE_VA
#ifdef Makedb2dcli
  #define db2dcliExp_Imp _Export
#else
  #define db2dcliExp_Imp _Import
  #pragma library( "GLEngine.LIB" )
#endif
#endif
//************** Visual Age END *****************

#include <idsql.hpp>


#include <gstring.hpp>
#include <gseq.hpp>


//#include <sal.h>
#ifdef __MINGW32__
#include "MySpecStrings.h"
#endif

#include <sqlcli1.h>
//#include <sqlcli.h>



#include <gstring.hpp>
#include <gthread.hpp>
#include <string.h>
#include <gseq.hpp>
#include <growhdl.hpp>
#include <stdio.h>

//Includes from QT4 Lib
#ifdef QT4_DSQL
#include <qlistwidget.h>
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


#ifdef MAKE_VC /////////// Windows:
#define DB2CLI_BLOB     -98
#define DB2CLI_CLOB     -99
#define DB2CLI_DBCLOB     SQL_DBCLOB
#define DB2CLI_XML      -152
#define DB2CLI_GUID     SQL_GUID
#define DB2CLI_DATETIME 93
#define DB2CLI_NVARCHAR -9
#define DB2CLI_BIT      -7
#define DB2CLI_FIXEDCHAR -8
#define USE_TDS72 1
#define DB2CLI_LONGVARCHAR SQL_LONGVARBINARY //defined in sqlext.h

#else  ////////////Linux:

#define DB2CLI_BLOB     -98
#define DB2CLI_CLOB     -99

#define DB2CLI_XML      -370
#define DB2CLI_DBCLOB	-350

#define DB2CLI_GUID     SQL_GUID
#define DB2CLI_DATETIME 93
#define DB2CLI_NVARCHAR -9
#define DB2CLI_BIT      -7
#define DB2CLI_FIXEDCHAR -8
#define DB2CLI_LONGVARCHAR -4

#endif



#ifdef Makedb2dcli
  class db2dcliExp_Imp db2dcli
#else
  class db2dcli : public IDSQL
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
      GSeq<GString> headerSeq;
      GSeq<RowData*> rowSeq;

private:
   //struct sqlca    sqlca;
   short        m_iNumberOfColumns;
   unsigned long m_iNumberOfRows;
  // QListView  * myLV;
   unsigned long iMaxLines;
   short        iCols;

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

   SQLUINTEGER     collen[256];
   SQLINTEGER      outlen[256];
   SQLCHAR        *colData[256];
   short          iEmbChars;

   short          iStop;
   signed long    sqlErrCode;
   GString        sqlErrTxt;
   short rsIsOpen;
   long writeToFile(GString fileName, char** buf, int len);
   long writeToFile(char* ptr, long length, GString blobFile);
   short iConnected;
   short iCommandOK;
   signed long iCost;
   short iCommit;
   short iDebug;
   unsigned long m_ulMaxRows;

   int m_iMyInstance;
   int m_iTruncationLimit;
   SQLHENV m_SQLEnv;
   SQLHDBC m_SQLHDBC;
   SQLHSTMT m_SQLHSTMT;
   int m_iLastSqlCode;
   GString m_strLastError;
   int m_iIsTransaction;
   GString m_strLastSqlSelectCommand;

   GString m_strDB;
   GString m_strUID;
   GString m_strPWD;
   GString m_strHost;
   int m_iPort;
   GDebug * m_pGDB;
   short m_iCharForBit;
   short m_iReadCLOBs;

   int m_iGetResultAsHEX;
   ODBCDB m_odbcDB;
   int m_iReadUncommitted;

   SQLINTEGER  clobLoc ;   /* TEST ONLY */

   int getConnData(GString * db, GString *uid, GString *pwd) const;
   int handleLobsAndXmls(int col, GString * out, int getFullXML, short* isNull);
   int getColInfo(SQLHSTMT * stmt);
   void testXML();

   int loadFileIntoBuf(GString fileName, unsigned char** fileBuf, SQLINTEGER *size, int type);
   void setSimpleTypeSeq(int colType);
   GString tabSchema(GString table);
   GString tabName(GString table);
   int getIdentityColParams(GString table, int *start, int * incr);
   void clearCommSequences();
   GString readErrorState(SQLHSTMT stmt = 0);
   GString setIndexColUse(GString indSchema, GString indName, IDX_INFO* pIdx);
   int minColSeparator(GString col);
   void setForKeyDetails(GString table, IDX_INFO* pIdx);
   GString deleteByCursor(GString filter, GString command, long deleteCount, short commitIt);

  public:

   /******** Stuff for QT **********************************/
    #ifndef NO_QT
    GSeq <QListViewItem*> lvItemSeq;
    VCExport GString       initAll(GString message, QListView * pLV, unsigned long maxLines );
	VCExport GString       getIdenticals(GString table, QListBox * aLB, QLabel * info, short autoDel);
	VCExport signed long   countLines(GString message);
    #endif
    /******** Stuff for QT END ******************************/
    VCExport    db2dcli();
    VCExport    ~db2dcli();
    VCExport    db2dcli(db2dcli const& o);
    VCExport    db2dcli* clone() const;
    VCExport ODBCDB getDBType()
    {
        return DB2ODBC;
    }



    VCExport    GString initAll(GString message, unsigned long maxRows = 0, int getLen = 0);
    VCExport      void emptyLVSeq();

    VCExport      void setCommit(short commit){iCommit = commit;}



    VCExport int disconnect();
    VCExport int openRS(GString cmd);

    VCExport int prepareSTMT(SQLCHAR* sqlstr);

    VCExport void display_results(SQLSMALLINT nresultcols);
    VCExport void deb(GString text);
    VCExport void deb(GString fnName, GString text);
    VCExport void sqlErr();

    //VCExport int nextRS();
    //VCExport void closeRS();
    VCExport void clearSequences();
    //VCExport void clearDataArray();

    VCExport      void setEmbrace(short embChars = 1);
    VCExport      GString version();
    VCExport      int commit();
    VCExport      int rollback();
    VCExport      GString setFldData(GString fldData, short sqlType );

    VCExport      unsigned int   numberOfColumns(){ return m_iNumberOfColumns;}
    VCExport      unsigned long  numberOfRows(){ return m_iNumberOfRows;}
    VCExport      GString        realName(const short & sqlType);
    VCExport      void           convToSQL( GString& input );

    VCExport      int          positionOfHostVar(const GString& hostVar);
    VCExport      short          sqlType(const short & col);
    VCExport      short          sqlType(const GString & name);

    VCExport      long           sqlVarLength(const short & col);
    VCExport      GString        fullLine(const unsigned long & index, GString sep = " | ");
    VCExport      GString        getRS(short col);
    VCExport      GString        getRS(const GString & colName);
    VCExport      GString        sqlError();
    VCExport      int           sqlCode();



    VCExport      signed long   getCost(){ return -1;}
    VCExport      int commandExecutedOK(){ return iCommandOK; }

    VCExport      int getSysTables();
    VCExport      int getTabSchema();
    VCExport      int getTable(GString schema);
    VCExport      int checkHistTable();
    VCExport      int setConn();

    VCExport GString getDBTypeName(){ return _DB2ODBC; }
    VCExport GString connect(GString db, GString uid, GString pwd, GString host = "", GString port = "");
    VCExport void setTruncationLimit(int limit);
    VCExport GString rowElement(unsigned long row, int col);
    VCExport GString rowElement(unsigned long row, GString hostVar);
    VCExport GString hostVariable(int col);
    VCExport int getTables(GString schema);
    VCExport int getAllTables(GSeq <GString > * tabSeq, GString filter = "");
    VCExport  int initRowCrs();
    VCExport  int nextRowCrs();
    VCExport  GString dataAtCrs(int col);
    VCExport void setStopThread(int ){}

    VCExport void setDBType(ODBCDB dbType){ m_odbcDB = dbType;}
    VCExport int execByDescriptor( GString, GSeq <GString> *){return -1;}
    VCExport int execByDescriptor( GString, GSeq <GString> *, GSeq <int> *,
                                    GSeq <short>* , GSeq <int> * ){return -1;}

    VCExport int isNumType(int pos);
    VCExport int isDateTime(int pos);
    VCExport int hasForBitData();
    VCExport int isForBitCol(int i);
    VCExport int isBitCol(int i);
    VCExport int isXMLCol(int i);
    VCExport int isLOBCol(int i);
    VCExport int isNullable(int i);
    VCExport int isLongTypeCol(int i);
    VCExport int simpleColType(int i);
    VCExport int isFixedChar(int i);

    VCExport void setCharForBit(int val);

    VCExport GString descriptorToFile( GString cmd, GString &blobFile, int * outSize );
    VCExport void setReadUncommitted(short readUncommitted = 1);
    VCExport short bindIt(GString , GString , GString , GString , GString ){ return 0;}

    VCExport GString  currentCursor(GString filter, GString command, long curPos, short commitIt = 1,
                                   GSeq <GString> *fileList = NULL, GSeq <long> *lobType = NULL);

    VCExport long uploadBlob(GString cmd, GSeq <GString> *fileList, GSeq <long> *lobType);
    VCExport long uploadBlob(GString cmd, char * buffer, long size);
    VCExport long dataLen(const short & pos);
    VCExport int  getDataBases(GSeq <CON_SET*> *dbList);
    VCExport int  deleteTable(GString tableName);
    VCExport long retrieveBlob( GString cmd, GString &blobFile, int writeFile = 1 );
#ifdef  QT4_DSQL
VCExport GString getIdenticals(GString table, QWidget* parent, QListWidget *pLB, short autoDel);
VCExport void writeToLB(QListWidget * pLB, GString message);
#endif
    VCExport GString fillChecksView(GString table, int showRaw = 0);
    VCExport int forceApp(int appID);
    VCExport void setCLOBReader(short readCLOBData );
    VCExport void setCurrentDatabase(GString db);
    VCExport GString currentDatabase();
    VCExport  unsigned long getResultCount(GString cmd);
    VCExport int getColSpecs(GString table, GSeq<COL_SPEC*> *specSeq);
    VCExport int getTriggers(GString table, GString *text);
    VCExport GSeq <IDX_INFO*> getIndexeInfo(GString table);

    VCExport GString getChecks(GString table, GString filter = "");
    VCExport GSeq <GString> getChecksSeq(GString table, GString filter = "");
    VCExport GSeq <GString> getTriggerSeq(GString table);

    VCExport int getUniqueCols(GString table, GSeq <GString> * colSeq);
    VCExport void createXMLCastString(GString &xmlData);
    VCExport int  isBinary(unsigned long row, int col);
    VCExport int  isNull(unsigned long row, int col);
    VCExport int  isTruncated(unsigned long row, int col);
    VCExport int uploadLongBuffer( GString cmd, GString colData, int isBinary);
    VCExport GString cleanString(GString in);
    VCExport void getResultAsHEX(int asHex);
    VCExport void setGDebug(GDebug *pGDB);
    VCExport int exportAsTxt(int mode, GString sqlCmd, GString table, GString outFile, GSeq <GString>* startText, GSeq <GString>* endText, GString *err);
    VCExport  int getHeaderData(int pos, GString * colData);
    VCExport  int getRowData(int row, int col, GString * colData);
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



