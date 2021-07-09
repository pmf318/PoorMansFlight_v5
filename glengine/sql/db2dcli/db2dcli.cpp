//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt
//

#ifndef _DB2DCLI_
#include <db2dcli.hpp>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <sqlcli.h>

/* For the Macintosh environment when generating 68K applications */
#ifdef DB268K
   /* Need to include ASLM for 68K applications */
   #include <LibraryManager.h>
#endif

#define  MAX_STMT_LEN 255
#define  MAXCOLS   100

#ifndef max
#define  max(a,b) (a > b ? a : b)
#endif

#define XML_MAX 5000

#include <gstuff.hpp>

///////////////////////////////////////////
//Global static instance counter.
//Each instance saves its private instance value in m_iMyInstance.
static int m_db2dcliCounter = 0;


/***********************************************************************
 * This class can either be instatiated or loaded via dlopen/loadlibrary
 ***********************************************************************/

//Define functions with C symbols (create/destroy instance).
#ifndef MAKE_VC
extern "C" db2dcli* create()
{
    return new db2dcli();
}
extern "C" void destroy(db2dcli* pdb2dcli)
{
   if( pdb2dcli ) delete pdb2dcli ;
}
#else
extern "C" __declspec( dllexport ) db2dcli* create()
{
    return new db2dcli();
}
extern "C" __declspec( dllexport ) void destroy(db2dcli* pdb2dcli)
{
    if( pdb2dcli ) delete pdb2dcli ;
}
#endif
/***********************************************************
 * CLASS
 **********************************************************/
db2dcli::db2dcli(db2dcli const & o)
{
    m_iLastSqlCode = 0;
    m_db2dcliCounter++;
    //m_IDSQCounter++;
    m_iTruncationLimit = 500;
    m_iMyInstance = m_db2dcliCounter;
    m_pGDB = o.m_pGDB;
    m_iGetResultAsHEX = o.m_iGetResultAsHEX;
    m_iCharForBit    = o.m_iCharForBit;
    deb("CopyCtor start");

    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_SQLEnv);
    deb("CopyCtor, ret from SQLAllocEnv: "+GString(ret));
    SQLSetEnvAttr(m_SQLEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
    ret = SQLAllocHandle(SQL_HANDLE_DBC, m_SQLEnv, &m_SQLHDBC);
    deb("CopyCtor, ret from SQLAllocDBC: "+GString(ret));

    //m_SQLEnv = o.m_SQLEnv;
    //m_SQLHDBC = o.m_SQLHDBC;
    m_SQLHSTMT = SQL_NULL_HSTMT;
    m_odbcDB  = DB2ODBC;
    m_iReadUncommitted = o.m_iReadUncommitted;
    m_pGDB = o.m_pGDB;
    m_iIsTransaction = o.m_iIsTransaction;    
    m_iReadCLOBs     = o.m_iReadCLOBs;

    m_strDB = o.m_strDB;
    m_strHost = o.m_strHost;
    m_strPWD = o.m_strPWD;
    m_strUID = o.m_strUID;
    m_iPort  = o.m_iPort;
    m_strLastSqlSelectCommand = o.m_strLastSqlSelectCommand;


    //ret = SQLAllocHandle(SQL_HANDLE_DBC, m_SQLEnv, &m_SQLHDBC);
    deb("Copy CTor done");

    this->connect(o.m_strDB, o.m_strUID, o.m_strPWD, o.m_strHost);
}

db2dcli::db2dcli()
{
    m_db2dcliCounter++;
    //m_IDSQCounter++;
    m_iTruncationLimit = 500;
    m_iMyInstance = m_db2dcliCounter;
    m_SQLEnv = SQL_NULL_HENV;
    m_SQLHDBC = SQL_NULL_HDBC;
    m_SQLHSTMT = SQL_NULL_HSTMT;
    m_iLastSqlCode = 0;
    m_odbcDB  = DB2ODBC;
    m_iGetResultAsHEX = 0;
    m_iReadCLOBs     = 0;
    m_iIsTransaction = 0;
    m_iCharForBit    = 4;
    m_strLastSqlSelectCommand = "";
    m_iReadUncommitted = 0;

    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_SQLEnv);
    if(!ret) ret = SQLSetEnvAttr(m_SQLEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);



    //if(!ret) ret = SQLAllocHandle(SQL_HANDLE_DBC, m_SQLEnv, &m_SQLHDBC);


    m_strDB ="";
    m_strUID ="";
    m_strPWD = "";
    m_strHost = "";
    m_pGDB = NULL;
}
db2dcli* db2dcli::clone() const
{
    return new db2dcli(*this);
}

db2dcli::~db2dcli()
{
    m_db2dcliCounter--;
    clearSequences();    
    SQLFreeStmt(m_SQLHSTMT, SQL_CLOSE);
    SQLFreeHandle(SQL_HANDLE_STMT, m_SQLHSTMT);
    SQLFreeConnect(m_SQLHDBC);
    SQLFreeEnv(m_SQLEnv);
    deb("Dtor done, current count: "+GString(m_db2dcliCounter));
}


/*
db2dcli::~db2dcli()
{
  short rc; 
  deb("Destroying db2dcli: closing RS...");

  closeRS();
  clearSequences();

//  rc = SQLFreeStmt(hstmt, SQL_DROP);
//  deb("rc FreeStmt: "+GString(rc));
  rc = SQLDisconnect(hdbc);
  deb("rc Disconnect: "+GString(rc));

  rc = SQLFreeConnect(hdbc);
  deb("rc FreeConnect: "+GString(rc));
  rc = SQLFreeEnv(henv);
  deb("rc FreeEnv: "+GString(rc));
  deb("Destroyed.");
}
*/
GString db2dcli::connect(GString db, GString user, GString passwd, GString host, GString port)
{
    PMF_UNUSED(port);

    deb("Connecting...");
    deb("Connecting: DB: "+db+", UID: "+user+", host: "+host);

    m_strDB = db;
    m_strHost = host;
    m_strPWD = passwd;
    m_strUID = user;
    m_iPort = port;

    SQLCHAR   server[SQL_MAX_DSN_LENGTH + 1];
    SQLCHAR   uid[19];
    SQLCHAR   pwd[31];


    strncpy((char *)server, (const char *)db, SQL_MAX_DSN_LENGTH );
    strncpy((char *)uid,  (const char *)user, 19);
    strncpy((char *)pwd, (const char *)passwd, 31);


    int erc;
    erc = SQLAllocEnv(&m_SQLEnv);    /* allocate an environment handle    */
    deb("ERC1: "+GString(erc));
    if( erc ) return readErrorState();
    erc = SQLAllocConnect(m_SQLEnv, &m_SQLHDBC);
    deb("ERC2: "+GString(erc));
    if( erc ) return readErrorState();
    erc = SQLSetConnectOption(m_SQLHDBC, SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_ON);
//          SQLSetConnectAttr(m_SQLHDBC, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER)true, 0);
    deb("ERC3: "+GString(erc));
    if( erc ) return readErrorState();

    /*
    if( isolation == "UR" )
    {
       //!! DB > v5 erc = SQLSetConnectOption(hdbc, SQL_ATTR_TXN_ISOLATION, SQL_TXN_READ_UNCOMMITTED);
       if( erc ) return 4;
    }
    */
    erc = SQLConnect(m_SQLHDBC, server, SQL_NTS, uid, SQL_NTS, pwd, SQL_NTS);
    deb("ERC4: "+GString(erc));
    if( erc ) return readErrorState();

    if( m_iReadUncommitted ) SQLSetConnectAttr (m_SQLEnv, SQL_TXN_ISOLATION, (void*)SQL_TXN_READ_UNCOMMITTED, 0);
    else SQLSetConnectAttr (m_SQLEnv, SQL_TXN_ISOLATION, (void*)SQL_TXN_SERIALIZABLE, 0);
    //SQLSetConnectAttr (m_SQLEnv, SQL_TXN_ISOLATION, (void*)SQL_TXN_READ_UNCOMMITTED, 0);


//    erc = SQLAllocStmt(hdbc, &hstmt); /* allocate a statement handle */
//    deb("Error Allocating StmtHandle: "+GString(erc));
    if( erc ) return readErrorState();
    iConnected = 1;
    m_strDB = db;
    m_strUID = user;
    m_strPWD = passwd;
    m_strHost = host;

    deb("Connect done.");
    return "";
}
int db2dcli::setConn()
{
    return 0;
    //return SQLSetConnection(m_SQLHDBC);
}
int db2dcli::disconnect()
{
  deb("Disconnecting...");
  short rc;

  rc = SQLTransact(m_SQLEnv, m_SQLHDBC, SQL_ROLLBACK);

  //closeRS();
  clearSequences();

  rc = SQLDisconnect(m_SQLHDBC);
  deb("rc Disconnect: "+GString(rc));

  rc = SQLFreeConnect(m_SQLHDBC);
  deb("rc FreeConnect: "+GString(rc));
  rc = SQLFreeEnv(m_SQLEnv);
  deb("rc FreeEnv: "+GString(rc));
  deb("Disconnect Done.");
  return 0;
}

GString db2dcli::initAll(GString message, unsigned long maxRows, int getLen)
{
    SQLRETURN ret = 0;
    deb("initAll: start. Msg: "+message);
    if( !iConnected )
    {
        sqlErrTxt = "@db2dcli: Not Connected.";
        return sqlErrTxt;
    }
    m_ulMaxRows = maxRows;
    m_iNumberOfColumns = m_iNumberOfRows = 0L;
    long erc;
    SQLRETURN       rc;

    clearSequences();

    rc = prepareSTMT(message);

    if( rc )
    {       
       deb("initAll, calling sqlError()");
       readErrorState();
       clearSequences();
       if( rc > 0 ) erc = rc;
       else erc = sqlCode();
       rc = SQLFreeStmt(m_SQLHSTMT, SQL_DROP);
       deb("initAll, Stmt freed due to error. rc: "+GString(erc));
       if( m_iLastSqlCode == 100) return "SQLCode 100: No row(s) found.";
       return m_strLastError;

    }

    deb("initAll: Starting while....");
    m_iNumberOfRows = 0;
    char *buf;
    GString data;
    long maxLen;
    GRowHdl * pRow;

    int getFullXML = 0;

    rc = getColInfo(&m_SQLHSTMT);
    if( rc )
    {
       deb("initAll, calling sqlError()");
       readErrorState();
       clearSequences();
       if( rc > 0 ) erc = rc;
       else erc = sqlCode();
       rc = SQLFreeStmt(m_SQLHSTMT, SQL_DROP);
       deb("initAll, Stmt freed due to error. rc: "+GString(erc));
       if( m_iLastSqlCode == 100) return "SQLCode 100: No row(s) found.";
       return m_strLastError;

    }

    if( getLen )
    {
        for(int i = 1; i <= m_iNumberOfColumns; ++i ) sqlLenSeq.add(hostVariable(i).length()+1);
    }

    deb("initAll: Starting while....");
    while ( SQLFetch(m_SQLHSTMT) == SQL_SUCCESS)
    {
        SQLINTEGER len = 0;
        if( maxRows > 0 && m_iNumberOfRows >= maxRows ) break;
        pRow = new GRowHdl;
        int bufLen;
        short isNull;

        for ( int i = 1; i <= m_iNumberOfColumns; i++ )
        {

            if( handleLobsAndXmls(i, &data, getFullXML, &isNull) )
            {
                deb("::readRowData, col "+GString(i)+" is LOB/XML, data: "+data);
                pRow->addElement(data, isNull);
            }
            else
            {
                deb("::readRowData, col: "+GString(i)+" no LOB/XML data. sqlVarLengthSeq has "+GString(sqlVarLengthSeq.numberOfElements())+" elements");
                bufLen = sqlVarLengthSeq.elementAtPosition(i)+1;


                //Double bufLen for CHAR FOR BIT DATA
                if( sqlForBitSeq.elementAtPosition(i)) bufLen = bufLen*2 - 1;
                if( sqlTypeSeq.elementAtPosition(i) == SQL_DECIMAL ) bufLen++;
                if( sqlTypeSeq.elementAtPosition(i) == SQL_DOUBLE ) bufLen += 6;

                buf = new char[bufLen];
                if( sqlTypeSeq.elementAtPosition(i) == SQL_LONGVARBINARY ) ret = SQLGetData(m_SQLHSTMT, i, SQL_C_BINARY, buf, bufLen, &len);
                else ret = SQLGetData(m_SQLHSTMT, i, SQL_C_CHAR, buf, bufLen, &len);
                deb("::readRowData, charForBit: "+GString(sqlForBitSeq.elementAtPosition(i))+", bufLen: "+GString(bufLen)+", len: "+GString(len));
                if( len > bufLen )
                {
                    //Todo: Either realloc or get correct size
                    bufLen = len+1;
                    delete [] buf;
                    buf = new char[bufLen];
                    if( sqlTypeSeq.elementAtPosition(i) == SQL_LONGVARBINARY ) ret = SQLGetData(m_SQLHSTMT, i, SQL_C_BINARY, buf, bufLen, &len);
                    else ret = SQLGetData(m_SQLHSTMT, i, SQL_C_CHAR, buf, bufLen, &len);
                    deb("::initAll, rc getData(2): "+GString(ret)+", buf: "+GString(buf)+", len in: "+GString(bufLen)+", lenRet: "+GString(len));
                }

                deb("::initAll, row: "+GString(m_iNumberOfRows)+", col: "+GString(i)+", ret: "+GString(ret)+", len: "+GString(len)+", bufLen: "+GString(bufLen)+", type: "+GString(sqlTypeSeq.elementAtPosition(i)));
                if (len == SQL_NULL_DATA)  pRow->addElement("NULL");
                else
                {
                    if( m_iGetResultAsHEX ) data = GString(buf, len, GString::HEX); //.stripTrailing('0') ;
                    else data = GString(buf, len) ;
                    deb("DATA: "+data);

                    if( bufLen > m_iTruncationLimit && m_iTruncationLimit > 0 && sqlForBitSeq.elementAtPosition(i) < 3) data = data.subString(1, m_iTruncationLimit);
                    if( !isNumType(i))pRow->addElement("'"+data+"'");
                    else pRow->addElement(data.translate(',', '.'));
                }
                delete[] buf;
            }
            if( getLen )
            {
                maxLen = max((signed) data.strip("'").strip().length()+1, sqlLenSeq.elementAtPosition(i));
                deb("initAll: Col "+GString(i)+", setting max: "+GString(maxLen));
                sqlLenSeq.replaceAt(i, maxLen);
            }
        }
        allRowsSeq.add( pRow );
        m_iNumberOfRows++;
    }
    if(ret) readErrorState();
    SQLCloseCursor(m_SQLHSTMT);
    m_strLastSqlSelectCommand = message;
    deb("::initAll, err fetch: "+GString(ret));

    SQLFreeStmt(m_SQLHSTMT, SQL_CLOSE);    
    if( m_SQLHSTMT) SQLFreeHandle(SQL_HANDLE_STMT, m_SQLHSTMT);
    //readErrorState();
    if( ret && ret != 100) return m_strLastError;
    return "";
}


void db2dcli::emptyLVSeq()
{
}
GString db2dcli::rowElement(unsigned long row, int col)
{
//Overload 1
    //deb("::rowElement for "+GString(line)+", col: "+GString(col));
    if( row < 1 || row > allRowsSeq.numberOfElements() ) return "@OutOfReach";
    GRowHdl *aRow;
    aRow = allRowsSeq.elementAtPosition(row);
    if( row < 1 || (unsigned long) col > aRow->elements() ) return "OutOfReach";
    return aRow->rowElementData(col);
}
GString db2dcli::rowElement(unsigned long row, const GString hostVar)
{
    //Overload 2
      unsigned long pos;
      if( row < 1 || row > allRowsSeq.numberOfElements() ) return "@OutOfReach";
      GRowHdl *aRow;
      aRow = allRowsSeq.elementAtPosition(row);
      pos = positionOfHostVar(hostVar);
      if( pos < 1 || pos > aRow->elements() ) return "OutOfReach";
      return aRow->rowElementData(pos);
}

/*******************************************************************
*******************************************************************/
int db2dcli::openRS(GString cmd)
{
    deb("openRS: START....");
    if( !iConnected )
    {
        sqlErrTxt = "@db2dcli: Not Connected.";
        return 1;
    }

    clearSequences();
    m_iNumberOfColumns = 0;
    SQLRETURN       rc;

    rc = prepareSTMT(cmd);
    rsIsOpen = 1;
    deb("openRS: ....Done");
    if( rc )
    {
        readErrorState();
        return sqlCode();
    }
    return 0;
}

/*******************************************************************
*******************************************************************/
int db2dcli::prepareSTMT(SQLCHAR* sqlstr)
{
    deb("analysing statement: "+GString(sqlstr));


    SQLINTEGER      rowcount;
    SQLRETURN       rc;

    rc = SQLAllocStmt(m_SQLHDBC, &m_SQLHSTMT); /* allocate a statement handle */
    deb("rc AllocStmt: "+GString(rc));
    if( rc ) return 1;

    deb("Preparing...");
    rc = SQLPrepare(m_SQLHSTMT, sqlstr, SQL_NTS);

    deb("RUNIT: PREP");
    deb("rc Prepare: "+GString(rc));
    if( rc ) return 1;


    rc = SQLExecute(m_SQLHSTMT);
    deb("rc Executing: "+GString(rc));
    if (rc != SQL_SUCCESS)
    {
        m_iLastSqlCode = rc;
        return rc;
    }

    rc = SQLNumResultCols(m_SQLHSTMT, &m_iNumberOfColumns);
    if( rc ) return rc;
    deb("Cols: "+GString(m_iNumberOfColumns)+" (0 if not select)");
    /* determine statement type */
    if (m_iNumberOfColumns == 0)
    {     /* statement is not a select statement */
        m_iIsTransaction = 1;
        rc = SQLRowCount(m_SQLHSTMT, &rowcount);
        deb("RowCount: "+GString(rowcount));

        if (rowcount > 0) {     /* assume statement is UPDATE, INSERT, DELETE */
            deb("Rows affected: "+GString(rowcount));
        }
        return 0;
    }

    deb("analysing...Done.");
    return (0);
}                               /* end process_stmt */

void db2dcli::testXML()
{
    SQLHSTMT hstmt;


    /* Variables for output XML data                   */
    SQLCHAR     HVBINARY[32768];
    /* Variables for output XML data lengths           */
    SQLINTEGER  LEN_HVBINARY;
    /* SQL statement buffer                            */
    SQLCHAR     sqlstmt[250];
    /* Return code for ODBC calls                      */
    SQLRETURN   rc = SQL_SUCCESS;
    rc = SQLAllocStmt(m_SQLHDBC, &hstmt);

    //SQLSetStmtAttr(hstmt,SQL_STREAM_OUTPUTLOB_ON_CALL,true,0);
    //SQLSetStmtAttr(hstmt,SQL_ATTR_STREAM_OUTPUTLOB_ON_CALL,true,0);




    deb("StmtAlloc: "+GString(rc));
    /* Prepare an SELECT statement for retrieving      */
    /* data from XML columns.                          */
    //strcpy((char *)sqlstmt, "SELECT DESCRIPTION  FROM GLE.PRODUCT  WHERE PID='100-100-01'" );
    strcpy((char *)sqlstmt, "SELECT BLOBCOL FROM GLE.BLC WHERE PARTNO=8" );

    deb("Stmt: "+GString(sqlstmt));
    /* Bind data for first XML column as SQL_C_BINARY. */
    /* This data will be retrieved as internally       */
    /* encoded, in the UTF-8 encoding scheme.          */
    rc = SQLBindCol(hstmt, 1, SQL_C_CHAR, HVBINARY, sizeof(HVBINARY), &LEN_HVBINARY);
    deb("rc1: "+GString(rc)+", lng: "+GString(LEN_HVBINARY));



    rc = SQLPrepare(hstmt, sqlstmt, SQL_NTS);
    deb("prep: "+GString(rc));
    rc = SQLExecute(hstmt);
    deb("exec: "+GString(rc));


    //rc = SQLExecDirect(hstmt, sqlstmt, SQL_NTS);
    deb("rc5: "+GString(rc));
    rc = SQLFetch(hstmt);
    deb("rc6: "+GString(rc)+", len: "+GString(LEN_HVBINARY));
    //deb("rc6: "+GString(rc)+", hvBin: "+GString(HVBINARY));

    return ;
    SQLINTEGER lng = 10;

    char * outBuffer = new char[lng];
    rc = SQLGetData(hstmt, 1, SQL_C_CHAR, NULL, 0, &lng);
    deb("rc: "+GString(rc)+", LNG: "+GString(lng)+", buf: "+GString(outBuffer)+"<-");

    rc = SQLGetData(hstmt, 1, SQL_C_CHAR, (SQLPOINTER) outBuffer, lng, &lng);
    deb("rc: "+GString(rc)+", LNG: "+GString(lng)+", buf: "+GString(outBuffer)+"<-");



}


void db2dcli::clearSequences()
{
    GRowHdl * aRow;
    hostVarSeq.removeAll();
    sqlTypeSeq.removeAll();
    sqlVarLengthSeq.removeAll();
    sqlIndVarSeq.removeAll();
    sqlForBitSeq.removeAll();
    sqlBitSeq.removeAll();
    xmlSeq.removeAll();
    lobSeq.removeAll();
    sqlLenSeq.removeAll();
    simpleColTypeSeq.removeAll();
    m_iNumberOfRows   = 0;
    m_iNumberOfColumns = 0;

    while( !allRowsSeq.isEmpty() )
    {
      aRow = allRowsSeq.firstElement();
      delete aRow;
      allRowsSeq.removeFirst();
    }
}
/*

void db2dcli::closeRS()
{
    if( !rsIsOpen ) return;
    short rc;
    rc = SQLFreeStmt(hstmt, SQL_DROP);
    deb("rc FreeStmt: "+GString(rc));
    if( !rc )
    {
       aLine.deleteLine();
       for (short i = 0; i < m_iNumberOfColumns; i++) {
          free(data[i]);
       }
    }
    rc = SQLTransact(henv, hdbc, SQL_COMMIT);
    deb("rc SQLTransaction COMMIT: "+GString(rc));
    if(rc) rc = SQLTransact(henv, hdbc, SQL_ROLLBACK);
    deb("rc SQLTransaction ROLLBACK: "+GString(rc));
    rsIsOpen = 0;
}
*/
void db2dcli::deb(GString fnName, GString txt)
{
    if( m_pGDB ) m_pGDB->debugMsg("db2dcli", m_iMyInstance, "::"+fnName+" "+txt);
}
void db2dcli::deb(GString txt)
{
    if( m_pGDB ) m_pGDB->debugMsg("db2dcli", m_iMyInstance, txt);
}



void db2dcli::setEmbrace(short embChars)
{
   iEmbChars = embChars;
}
GString db2dcli::version()
{
    return "db2dcli v 0.1";
}

int db2dcli::commit()
{
   deb("COMMIT called.");
   short rc = SQLTransact(m_SQLEnv, m_SQLHDBC, SQL_COMMIT);
   return rc;
}
int db2dcli::rollback()
{
   deb("ROLLBACK called.");
   short rc = SQLTransact(m_SQLEnv, m_SQLHDBC, SQL_ROLLBACK);
   return rc;
}

GString db2dcli::setFldData(GString fldData, short sqlType )
{
    deb("setFldData, type: "+GString(sqlType));
   switch( sqlType )
   {
	case SQL_CHAR:
	case SQL_VARCHAR:
	case SQL_DATE:
	case SQL_TIME:
	case SQL_TIMESTAMP:
           if( iEmbChars ) fldData = "'" + fldData + "'";
           break;

	case SQL_NUMERIC:
	case SQL_DECIMAL:
	case SQL_INTEGER:
	case SQL_SMALLINT:
	case SQL_FLOAT:
	case SQL_REAL:
	case SQL_DOUBLE:
	case SQL_BIGINT:
	case SQL_TINYINT:      
	case SQL_BIT:
	   break;

	case SQL_BINARY:
	case SQL_VARBINARY:
	case SQL_LONGVARBINARY:
	case SQL_GRAPHIC:
	case SQL_VARGRAPHIC:
	case SQL_LONGVARGRAPHIC:
           fldData = "@DSQL@ GraphicStuff";
           break;

	case SQL_CLOB:
           fldData = "@DSQL@ CLOB";
           break;
    case SQL_DBCLOB:
           fldData = "@DSQL@ DBCLOB";

           break;
	case SQL_BLOB:
           fldData = "@DSQL@ BLOB";
           break;

/******
      case 484:
      case 485:
           int p, s, ind, idx, top, bottom, point;
           char* ptr;
           p = ( (char *)&(sqldaptr->sqlvar[col].sqllen) )[0];
           s = ( (char *)&(sqldaptr->sqlvar[col].sqllen) )[1];
           ptr = sqldaptr->sqlvar[col].sqldata;
           if ((p %2) == 0) p += 1;
           idx = ( p + 2 ) / 2 ; //Number Of Bytes
           point = p - s ;       //Where the point is ...
           //Determine the sign 
           bottom = *(ptr + idx -1) & 0x000F ;
           if ( (bottom == 0x000D) || (bottom == 0x000B) ) fldData = "-";
           else fldData = "";
           for (ind=0; ind < idx; ind++)
           {
              top = *(ptr + ind) & 0x00F0 ;
              top = (top >> 4 ) ;
              bottom = *(ptr + ind) & 0x000F ;
              fldData += GString(top);
              if ( ind < idx - 1 ) fldData += GString(bottom);
           }
           fldData.insert(".", point+1);

           break;
*****/
      default:
           fldData = "Unknown SQLDataType: "+GString(sqlType);
   }
   return fldData;

}
GString db2dcli::realName(const short & sqlType)
{
   switch (sqlType)
   {
      case 384:
      case 385:
          return "DATE";
      case 388:
      case 389:
          return "TIME";
      case 392:
      case 393:
          return "TIMESTAMP";
      case 400:
      case 401:
          return "C-GraphString";
      case 404:
      case 405:
          return "BLOB";
      case 408:
      case 409:
          return "CLOB";
      case 412:
      case 413:
          return "DBCLOB";
      case 448:
      case 449:
          return "VARCHAR";
      case 452:
      case 453:
          return "CHAR";
      case 456:
      case 457:
          return "LONG";
      case 460:
      case 461:
          return "C-String";
      case 464:
      case 465:
          return "VarGraphicStr";
      case 468:
      case 469:
          return "GraphicStr";
      case 472:
      case 473:
          return "LongGraphStr";
      case 476:
      case 477:
          return "LStr";
      case 480:
      case 481:
          return "FLOAT";
      case 484:
      case 485:
          return "DECIMAL";
      case 488:
      case 489:
          return "ZonedDecimal";
      case 492:
      case 493:
          return "BigINT (64Bit)";
      case 496:
      case 497:
          return "INTEGER";
      case 500:
      case 501:
          return "SMALL";
      case 504:
      case 505:
          return "Numeric";
      case 804:
      case 805:
          return "BLOB";
      case 808:
      case 809:
          return "CLOB";
      case 812:
      case 813:
          return "DBCLOB";
      case 960:
      case 961:
          return "BLOB Locator";
      case 964:
      case 965:
          return "CLOB Locator";
      case 968:
      case 969:
          return "DBCLOB Locator";
      default:
          return "Unknown: "+GString(sqlType);
   }
   return "";

}
void db2dcli::convToSQL( GString& input )
{
//Strings containing ' have to be converted.
//Example: input = 'aString 'with' this'
//Output should be: 'aString ''with'' this'

   GString output = "";
   unsigned long i;
   if( input.occurrencesOf('\'') <= 2 ) return;
   input = input.strip();
   if( input[1] == '\'' && input[input.length()] == '\'')input = input.subString(2, input.length()-2);


   char * in = new char[input.length() + 1];
   strcpy( in, (char*) input );

   for(i=0;i < input.length();++i)
   {
     if( in[i] == '\'' ) output = output + GString(in[i]) + "'";
     else output = output + GString(in[i]);

//     if( GString(input[i]) == GString('\'') ) output = output + GString(input[i])+"'";
//     else output = output + GString(input[i]);
   }
   output = "'"+output+"'";
   input = output;
   delete [] in;
}
/*
GString db2dcli::getRS(short col)
{
   if( col < 1 || (unsigned) col > aLine.elements() ) return "@OutOfReach";
   return aLine.rowElement(col);
}
GString db2dcli::getRS(const GString & colName)
{
   short pos = positionOfHostVar(colName);
   if( pos < 1 || pos > (short) aLine.elements() ) return "@OutOfReach";
   return aLine.rowElement(pos);
}
*/
int db2dcli::positionOfHostVar(const GString& hostVar)
{
   for( short i = 1; i <= (short) hostVarSeq.numberOfElements(); ++i )
   {
      if( hostVarSeq.elementAtPosition(i) == hostVar ) return i;
   }
   return 0;
}
GString db2dcli::hostVariable(int pos)
{
   if( pos < 1 || pos > (short) hostVarSeq.numberOfElements() ) return "@hostVariable:OutOfReach";
   return hostVarSeq.elementAtPosition(pos);
}
short db2dcli::sqlType(const short & col)
{
   if( col < 1 || col > (short) sqlTypeSeq.numberOfElements() ) return -1;
   else return sqlTypeSeq.elementAtPosition(col);
}
short db2dcli::sqlType(const GString & name)
{
   for( short i = 1; i <= (short) hostVarSeq.numberOfElements(); ++ i )
   {
      if( hostVarSeq.elementAtPosition(i) == name ) return sqlTypeSeq.elementAtPosition(i);
   }
   return -1;
}
long db2dcli::sqlVarLength(const short & col)
{
   if( col < 1 || col+1 > m_iNumberOfColumns ) return -1;
   return collen[col+1];
}
/*
GString db2dcli::fullLine(const unsigned long & index, GString sep )
{
    if( index < 1 || index > allRowsSeq.numberOfElements() ) return "";
    GRowHdl *aLine;
    aLine = allRowsSeq.elementAtPosition(index);

    GString line = "";
    for( unsigned long i=1; i<=aLine->elements(); ++i )
    {
       line += hostVariable(i).strip() + ": "+(aLine->rowElement(i)).strip()+sep;
    }
    line.stripTrailing(sep);
    return line;
}
*/

GString db2dcli::readErrorState(SQLHSTMT stmt)
{
    deb("Starting readErrorState()...");

    if( stmt == 0 ) stmt = m_SQLHSTMT;
    SQLCHAR         sqlstate[SQL_SQLSTATE_SIZE + 1];
    SQLINTEGER sqlcode ;
    SQLSMALLINT length;
    SQLCHAR message[SQL_MAX_MESSAGE_LENGTH + 1];

    sqlcode = 0;
    sqlErrTxt = "";

    while (SQLError(m_SQLEnv, m_SQLHDBC, stmt, sqlstate, &sqlcode, message, SQL_MAX_MESSAGE_LENGTH + 1, &length) == SQL_SUCCESS)
    {
        sqlErrTxt = GString(message);
        deb("sqlCode: Message: "+GString(message)+", Length: "+GString(length)+", Code: "+GString(sqlcode));
        m_strLastError = GString(message, length);
        m_iLastSqlCode = sqlcode;
    };

    deb("readErrorState, to return: "+GString(sqlcode)+", message: "+m_strLastError);

    return message;
}

GString db2dcli::sqlError()
{
    return m_strLastError;

    SQLINTEGER sqlcode;
    SQLSMALLINT length;
    SQLCHAR message[400];

    SQLCHAR     sqlstate[SQL_SQLSTATE_SIZE + 1];
    SQLError(m_SQLEnv, m_SQLHDBC, m_SQLHSTMT, sqlstate, &sqlcode, message, 400, &length);
    m_strLastError = message;
    m_iLastSqlCode = sqlcode;

    deb("sqlError: Message: "+GString(message)+", Length: "+GString(length)+", Code: "+GString(sqlcode));
    return message;

}
int db2dcli::sqlCode()
{   
   deb("sqlCode called...");
   return m_iLastSqlCode;

   SQLCHAR         sqlstate[SQL_SQLSTATE_SIZE + 1];
   SQLINTEGER sqlcode ;
   SQLSMALLINT length;
   SQLCHAR message[SQL_MAX_MESSAGE_LENGTH + 1];

   sqlcode = 0;
   sqlErrTxt = "";
   while (SQLError(m_SQLEnv, m_SQLHDBC, m_SQLHSTMT, sqlstate, &sqlcode, message,
                    SQL_MAX_MESSAGE_LENGTH + 1, &length) == SQL_SUCCESS) {
       sqlErrTxt = GString(message);
       deb("sqlCode: Message: "+GString(message)+", Length: "+GString(length)+", Code: "+GString(sqlcode));
   };
   deb("sqlCode to return: "+GString(sqlcode));
   return sqlcode;
}

void db2dcli::sqlErr()  /* the error was reported from  */
{
    deb("Start sqlErr()... ");
    SQLCHAR         buffer[SQL_MAX_MESSAGE_LENGTH + 1];
    SQLCHAR         sqlstate[SQL_SQLSTATE_SIZE + 1];
    SQLINTEGER      sqlcode;
    SQLSMALLINT     length;


    while (SQLError(m_SQLEnv, m_SQLHDBC, m_SQLHSTMT, sqlstate, &sqlcode, buffer,
                    SQL_MAX_MESSAGE_LENGTH + 1, &length) == SQL_SUCCESS) {
deb("Start while....");

        deb("         SQLSTATE: "+GString(sqlstate));
        deb("Native Error Code: "+GString(sqlcode));
        deb("Buffer: "+GString(buffer));

//deb("sqlCode: Message: "+GString(buffer)+", Length: "+GString(length)+", Code: "+GString(sqlcode));
    };
    deb("Stop sqlErr().");
    //return (SQL_ERROR);

}
long db2dcli::uploadBlob(GString cmd, GSeq <GString> *fileSeq, GSeq <long> *lobType)
{
    deb("::uploadBlob, cmd: "+cmd);
    SQLRETURN       ret = 0;

    m_iLastSqlCode = 0;

    if(fileSeq->numberOfElements() == 0 ) return 0;

    unsigned char ** fileBuf = new unsigned char* [fileSeq->numberOfElements()];
    //char ** fileBuf = new char* [fileSeq->numberOfElements()];
    SQLINTEGER*         size = new SQLINTEGER[fileSeq->numberOfElements()];

    deb("::uploadBlob, SQLAllocHandle, rc: "+GString(ret));

    if( m_SQLHSTMT) SQLFreeHandle(SQL_HANDLE_STMT, m_SQLHSTMT);
    SQLAllocHandle(SQL_HANDLE_STMT, m_SQLHDBC, &m_SQLHSTMT);

    SQLPrepare(m_SQLHSTMT, (SQLCHAR*)cmd, SQL_NTS);

    for(unsigned long i = 1; i <= fileSeq->numberOfElements(); ++i )
    {
        deb("::uploadBlob, try open "+fileSeq->elementAtPosition(i));
        if( lobType->elementAtPosition(i) == DB2CLI_BLOB ) //Blob
        {
            deb("::uploadBlob: Is binary: "+GString(lobType->elementAtPosition(i)));
            if( !loadFileIntoBuf(fileSeq->elementAtPosition(i), &(fileBuf[i-1]), &(size[i-1]), 1) )
            {
                ret = SQLBindParameter(m_SQLHSTMT, i, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_LONGVARBINARY, size[i-1], 0, (SQLCHAR*)fileBuf[i-1], size[i-1], &size[i-1]) ;
            }
        }
        else
        {
            deb("::uploadBlob: Is char: "+GString(lobType->elementAtPosition(i)));
            if( !loadFileIntoBuf(fileSeq->elementAtPosition(i), &(fileBuf[i-1]), &(size[i-1]), 0) )
            {
                ret = SQLBindParameter(m_SQLHSTMT, i, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_LONGVARCHAR, size[i-1], 0, (SQLCHAR*)fileBuf[i-1], size[i-1], &size[i-1]) ;
            }
        }
    }
    if( !ret ) ret = SQLExecute(m_SQLHSTMT);


    delete [] fileBuf;
    delete [] size;
    readErrorState();
    deb("::uploadBlob, SQLExecute, rc: "+GString(ret)+", err: "+sqlError());

    SQLFreeStmt(m_SQLHSTMT, SQL_CLOSE);
    m_iLastSqlCode = ret;
    return ret;
}

long db2dcli::uploadBlob(GString cmd, char * buffer, long size)
{
    PMF_UNUSED(cmd);
    PMF_UNUSED(buffer);
    PMF_UNUSED(size);

    return -1;
}
int db2dcli::loadFileIntoBuf(GString fileName, unsigned char** fileBuf, SQLINTEGER *size, int type)
{
    FILE * f;
    //Apparently CR/LF is not counted in fread.
    //Even XML sources should be opened with "rb".
    if( type == 1 ) f = fopen(fileName, "rb");
    else f = fopen(fileName, "rb");
    if( f != NULL )
    {
        deb("::loadFileIntoBuf reading file....");
        fseek(f, 0, SEEK_END);
        *size = ftell(f);
        //rewind(f);
        fseek(f, 0, SEEK_SET);

        *fileBuf = new unsigned char[(*size)+1];
        fread(*fileBuf,sizeof(char),*size, f);
        fclose(f);
        deb("::loadFileIntoBuf reading file...OK, size: "+GString(*size));
        return 0;
    }
    return 1;
}

/*
long db2dcli::uploadBlob(GString sqlCmd, void* buffer, long size)
{

  signed long rc;
   SQLPOINTER valuePtr;

   SQLCHAR input_param[] = "NOPE";

   SQLINTEGER picLength = SQL_DATA_AT_EXEC;



   rc = SQLAllocStmt(m_SQLHDBC, &m_SQLHSTMT);
   deb("pos 4 "+GString(rc)+", code: "+GString(sqlCode())+", err: "+sqlError());

   rc = SQLPrepare(m_SQLHSTMT, sqlCmd, SQL_NTS);
   deb("pos 5 "+GString(rc)+", code: "+GString(sqlCode())+", err: "+sqlError());

   rc = SQLBindParameter(m_SQLHSTMT, 1, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_BLOB, 10, 0, (SQLPOINTER) input_param, size, &picLength);
   deb("pos 6 "+GString(rc)+", code: "+GString(sqlCode())+", err: "+sqlError());

   rc =  SQLExecute(m_SQLHSTMT);
   deb("pos 7 "+GString(rc)+", code: "+GString(sqlCode())+", err: "+sqlError());

   rc = SQLParamData(m_SQLHSTMT, (SQLPOINTER*) &valuePtr); // == SQL_NEED_DATA;
   deb("pos 8 "+GString(rc)+", code: "+GString(sqlCode())+", err: "+sqlError());

   rc = SQLPutData(m_SQLHSTMT, buffer, size);
   deb("pos 9 "+GString(rc)+", code: "+GString(sqlCode())+", err: "+sqlError());

   rc = SQLParamData(m_SQLHSTMT, (SQLPOINTER*) &valuePtr); // == SQL_NEED_DATA;
   deb("pos 10 "+GString(rc)+", code: "+GString(sqlCode())+", err: "+sqlError());

   commit();
   return 0;
}
*/
long db2dcli::writeToFile(char* ptr, long length, GString blobFile)
{
   FILE *fOut;
   if( (fOut = fopen(blobFile, "w+b")) == NULL)
   {
      deb("Could not open file "+blobFile);
      return 0;
   }
   fwrite(ptr, sizeof(char), length, fOut);
   fclose(fOut);
   return 0;
}

long db2dcli::writeToFile(GString fileName, char** buf, int len)
{
    FILE * f;
    f = fopen(fileName, "ab");
    int written = fwrite(*buf,1,len,f);
    fclose(f);
    return written;
}


int db2dcli::getSysTables()
{   
                             this->initAll("SELECT TABSCHEMA, TABNAME FROM SYSCAT.TABLES ORDER BY TABSCHEMA, TABNAME");
   if( m_iNumberOfRows == 0 ) this->initAll("SELECT CREATOR, NAME FROM SYSIBM.SYSTABLES ORDER BY CREATOR, NAME");
   if( m_iNumberOfRows == 0 ) this->initAll("SELECT TNAME, CREATOR FROM SYSTEM.SYSCATALOG ORDER BY TNAME, CREATOR");
   return 0;
}
int db2dcli::getTabSchema()
{
  this->initAll("SELECT DISTINCT(TABSCHEMA) FROM SYSCAT.TABLES ORDER BY TABSCHEMA");
  if( m_iNumberOfRows == 0 ) this->initAll("SELECT DISTINCT(CREATOR) FROM SYSIBM.SYSTABLES ORDER BY CREATOR");
  if( m_iNumberOfRows == 0 ) this->initAll("SELECT DISTINCT(CREATOR) FROM SYSTEM.SYSCATALOG ORDER BY CREATOR");
  return 0;
}
int db2dcli::getTable(GString schema)
{
  this->initAll("SELECT TABNAME FROM SYSCAT.TABLES WHERE TABSCHEMA='"+schema+"' ORDER BY TABNAME");
  if( m_iNumberOfRows == 0 ) this->initAll("SELECT NAME FROM SYSIBM.SYSTABLES WHERE CREATOR='"+schema+"' ORDER BY NAME");
  if( m_iNumberOfRows == 0 ) this->initAll("SELECT TNAME FROM SYSTEM.SYSCATALOG WHERE CREATOR='"+schema+"' ORDER BY TNAME");

  return 0;
}
int db2dcli::checkHistTable()
{
  this->initAll("SELECT COUNT(*) FROM SYSCAT.TABLES WHERE TABSCHEMA='PMF' AND TABNAME='HISTORY'");
  if( rowElement(1,1).asInt() > 0 ) return 1;

  this->initAll("SELECT NAME FROM SYSIBM.SYSTABLES WHERE CREATOR='PMF' AND NAME='HISTORY'");
  if( rowElement(1,1).asInt() > 0 ) return 1;

  this->initAll("SELECT TNAME FROM SYSTEM.SYSCATALOG WHERE CREATOR='PMF' AND TNAME='HISTORY'");
  if( rowElement(1,1).asInt() > 0 ) return 1;  

  return 0;
}


void db2dcli::setTruncationLimit(int limit)
{
    //Setting m_iTruncationLimit to 0 -> no truncation
    m_iTruncationLimit = limit;
}

int db2dcli::getTables(GString schema)
{
    deb("Gettables for schema "+schema);
    this->initAll("SELECT TABNAME FROM SYSCAT.TABLES WHERE TABSCHEMA='"+schema+"' ORDER BY TABNAME");
    if( this->numberOfRows() == 0 ) this->initAll("SELECT NAME FROM SYSIBM.SYSTABLES WHERE CREATOR='"+schema+"' ORDER BY NAME");
    if( this->numberOfRows() == 0 ) this->initAll("SELECT TNAME FROM SYSTEM.SYSCATALOG WHERE CREATOR='"+schema+"' ORDER BY TNAME");
    return 0;
}

int db2dcli::getAllTables(GSeq <GString > * tabSeq, GString filter)
{
    PMF_UNUSED(tabSeq);
    PMF_UNUSED(filter);
    return 0;
}

int db2dcli::initRowCrs()
{
    deb("::initRowCrs");
    if( allRowsSeq.numberOfElements() == 0 ) return 1;
    m_pRowAtCrs = allRowsSeq.initCrs();
    return 0;
}
int db2dcli::nextRowCrs()
{
    if( m_pRowAtCrs == NULL ) return 1;
    m_pRowAtCrs = allRowsSeq.setCrsToNext();
    return 0;
}

GString db2dcli::dataAtCrs(int col)
{
    if( m_pRowAtCrs == NULL ) return "@CrsNotOpen";
    if( col < 1 || (unsigned long)col > m_pRowAtCrs->elements() ) return "@OutOfReach";
    return m_pRowAtCrs->rowElementData(col);
}

long  db2dcli::dataLen(const short & pos)
{
    if( pos < 1 || pos > (int)sqlLenSeq.numberOfElements()) return 0;
    deb("Len at "+GString(pos)+": "+GString(sqlLenSeq.elementAtPosition(pos)));
    return sqlLenSeq.elementAtPosition(pos);
}
int db2dcli::isDateTime(int pos)
{
    if( pos < 1 || pos > (int)sqlTypeSeq.numberOfElements() ) return 0;
    if( sqlTypeSeq.elementAtPosition(pos) == SQL_TYPE_DATE ) return 1;
    else if( sqlTypeSeq.elementAtPosition(pos) == SQL_TYPE_TIME ) return 1;
    else if( sqlTypeSeq.elementAtPosition(pos) == SQL_TYPE_TIMESTAMP ) return 1;
    return 0;
}

int db2dcli::isNumType(int pos)
{
    if( pos < 1 || pos > (int)sqlTypeSeq.numberOfElements() ) return 0;
    if( sqlTypeSeq.elementAtPosition(pos) == SQL_DECIMAL ) return 1;
    else if( sqlTypeSeq.elementAtPosition(pos) == SQL_NUMERIC) return 1;
    else if( sqlTypeSeq.elementAtPosition(pos) == SQL_SMALLINT) return 1;
    else if( sqlTypeSeq.elementAtPosition(pos) == SQL_INTEGER) return 1;
    else if( sqlTypeSeq.elementAtPosition(pos) == SQL_REAL) return 1;
    else if( sqlTypeSeq.elementAtPosition(pos) == SQL_FLOAT) return 1;
    else if( sqlTypeSeq.elementAtPosition(pos) == SQL_DOUBLE) return 1;
    else if( sqlTypeSeq.elementAtPosition(pos) == SQL_BIGINT) return 1;
    else if( sqlTypeSeq.elementAtPosition(pos) == DB2CLI_BIT) return 1;
    return 0;
}
int db2dcli::hasForBitData()
{
    return 0;
}
int db2dcli::isForBitCol(int i)
{
    if( i < 1 || (unsigned long)i > sqlForBitSeq.numberOfElements() ) return 0;
    return sqlForBitSeq.elementAtPosition(i);
}
int db2dcli::isBitCol(int i)
{
    if( i < 1 || (unsigned long)i > sqlBitSeq.numberOfElements() ) return 0;
    return sqlBitSeq.elementAtPosition(i);
}


int db2dcli::isXMLCol(int i)
{
    if( i < 1 || (unsigned long)i > xmlSeq.numberOfElements() ) return 0;
    return xmlSeq.elementAtPosition(i);
}
int db2dcli::simpleColType(int i)
{
    //deb("get simpleColType for col "+GString(i));

    if(isXMLCol(i)) return CT_XML;
    //if(isLOBCol(i)) return CT_BLOB;
    if(isNumType(i)) return CT_INTEGER;
    if(isDateTime(i)) return CT_DATE;


    if( i < 1 || (unsigned long)i > simpleColTypeSeq.numberOfElements() ) return CT_UNKNOWN;
    return simpleColTypeSeq.elementAtPosition(i);
}

int db2dcli::isLongTypeCol(int i)
{
    if( i < 1 || (unsigned long)i > simpleColTypeSeq.numberOfElements() ) return 0;
    return simpleColTypeSeq.elementAtPosition(i) == CT_LONG;
}

int db2dcli::isLOBCol(int i)
{
    if( i < 1 || (unsigned long)i > lobSeq.numberOfElements() ) return 0;
    return lobSeq.elementAtPosition(i);
}

int db2dcli::isNullable(int i)
{
    if( i < 1 || (unsigned long)i > sqlIndVarSeq.numberOfElements() ) return 1;
    return sqlIndVarSeq.elementAtPosition(i);
}

int db2dcli::isFixedChar(int i)
{
    if( i < 1 || (unsigned long)i > sqlTypeSeq.numberOfElements() ) return 0;
    switch( sqlTypeSeq.elementAtPosition(i) )
    {
        case DB2CLI_FIXEDCHAR:
        case SQL_CHAR:
        case SQL_GRAPHIC:
            return 1;
    }
    return 0;
}
GString db2dcli::descriptorToFile( GString cmd, GString &blobFile, int * outSize )
{

    if( m_SQLHDBC == SQL_NULL_HDBC )
    {
        deb("::descriptorToFile: No handle, quitting.");
        return "";
    }

    SQLRETURN       ret;
    SQLSMALLINT     columns;
    SQLCHAR         colName[128];
    SQLSMALLINT     sqlType;
    SQLSMALLINT     colSize;
    SQLSMALLINT     nullable;
    SQLUINTEGER         size;
    SQLSMALLINT     scale;

    m_strLastError = "";
    m_iLastSqlCode = 0;

    SQLFreeHandle(SQL_HANDLE_DBC, m_SQLHDBC);
    ret = SQLAllocHandle(SQL_HANDLE_STMT, m_SQLHDBC, &m_SQLHSTMT);
    deb("::descriptorToFile, SQLAllocHandle, rc: "+GString(ret));
    if(ret) return readErrorState();

    ret = SQLExecDirect(m_SQLHSTMT, (SQLCHAR*)cmd, SQL_NTS);
    deb("::descriptorToFile, SQLExecDirect, rc: "+GString(ret));
    if(ret) return readErrorState();

    ret = SQLNumResultCols(m_SQLHSTMT, &columns);
    deb("::descriptorToFile, SQLNumResultCols, rc: "+GString(ret)+", cols: "+GString(columns));
    if(ret) return readErrorState();

    SQLINTEGER rowCount;
    SQLRowCount(m_SQLHSTMT, &rowCount);
    if( !rowCount )
    {
        m_iLastSqlCode = 100;
        return "The current row was changed (possibly by another application or a trigger).\nSimply refresh the view (hit F5 or click 'Open').";
    }


    char *buf;
    int bufLen = 10000000; //Read 10MB Blocks
    buf = new char[bufLen+1];


    while (SQL_SUCCEEDED(ret = SQLFetch(m_SQLHSTMT)))
    {
        SQLINTEGER len = 0;

        for ( int i = 1; i <= columns; i++ )
        {
            *outSize = 0;
            remove(blobFile);
            SQLDescribeCol (m_SQLHSTMT, i, colName, sizeof (colName),  &colSize, &sqlType, &size, &scale, &nullable);
            if(sqlType != DB2CLI_XML && sqlType != -370 && sqlType != DB2CLI_BLOB && sqlType != DB2CLI_CLOB && sqlType != DB2CLI_DBCLOB)
            {
                SQLFreeStmt(m_SQLHSTMT, SQL_CLOSE);
                return "Invalid column type";
            }
            while( 1 )
            {
                memset(buf, 0, sizeof(buf));
                //SQL_C_CHAR instead of SQL_C_BINARY?
                if( sqlType == DB2CLI_BLOB ) ret = SQLGetData( m_SQLHSTMT, i, SQL_C_BINARY, (SQLPOINTER) ((char *) (buf)),(SQLINTEGER) bufLen,(SQLINTEGER *) &len );
                else ret = SQLGetData( m_SQLHSTMT, i, SQL_C_CHAR, (SQLPOINTER) ((char *) (buf)),(SQLINTEGER) bufLen,(SQLINTEGER *) &len );
                deb("descriptorToFile, col: "+GString(i)+", type: "+GString(sqlType)+", bufLen: "+GString(bufLen)+", size: "+GString(len)+", ret: "+GString(ret));
                if( ret ) break;
                if( len < bufLen ) bufLen = len + (sqlType == DB2CLI_DBCLOB ? 2 : 0);
                *outSize += writeToFile(blobFile, &buf, bufLen);
                deb("written: "+GString(*outSize)+", bufLenNew: "+GString(bufLen));
                if( ret == 0  ) break;
            }
        }
    }
    delete [] buf;
    readErrorState();
    SQLFreeStmt(m_SQLHSTMT, SQL_CLOSE);
    deb("sqlCode: "+GString(sqlCode()));
    if( sqlCode() == 100 || sqlCode() == 0 ) return "";
    return m_strLastError;
}

void db2dcli::setReadUncommitted(short readUncommitted)
{
    m_iReadUncommitted = readUncommitted;
    if( m_iReadUncommitted ) SQLSetConnectAttr (m_SQLHDBC, SQL_TXN_ISOLATION, (void*)SQL_TXN_READ_UNCOMMITTED, 0);
    else SQLSetConnectAttr(m_SQLHDBC, SQL_TXN_ISOLATION, (void*)SQL_TXN_SERIALIZABLE, 0);
}

GString db2dcli::currentCursor(GString filter, GString command, long curPos, short commitIt, GSeq <GString> *fileList, GSeq <long> *lobType)
{
    deb(__FUNCTION__, "filter: "+filter+", cmd: "+command);
    int rc;
    if( curPos < 0 ) return "CurrentCursor(): POS < 0";
    SQLHSTMT     hstmtSelect;
    SQLHSTMT     hstmtUpdate;
    SQLRETURN    retcode = 0;


    filter += " FOR UPDATE";
    rc = SQLAllocStmt(m_SQLHDBC, &hstmtSelect); /* allocate a statement handle */
    deb(__FUNCTION__, "rc alloc SEL: "+GString(rc));
    rc = SQLSetCursorName(hstmtSelect, (SQLCHAR*)"PMF_UPD_CRS", SQL_NTS);
    deb(__FUNCTION__, "rc alloc setCRSName: "+GString(rc));
    rc = SQLAllocStmt(m_SQLHDBC, &hstmtUpdate); /* allocate a statement handle */
    deb(__FUNCTION__, "rc alloc UPD: "+GString(rc));

    rc = SQLExecDirect(hstmtSelect,(SQLCHAR*) filter, SQL_NTS);
    deb(__FUNCTION__, "rc execDirect: "+GString(rc));
    if(rc)
    {
        readErrorState(hstmtSelect);
        m_iLastSqlCode = rc;
        return rc;
    }

    long count = 0;
    command += " WHERE CURRENT OF PMF_UPD_CRS";
    do
    {
        rc = SQLFetch(hstmtSelect);
        deb(__FUNCTION__, "rc fetch: "+GString(rc));
        count++;
        if(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
        {
            if( count != curPos ) continue;

            if( fileList != NULL )
            {
                deb(__FUNCTION__, "FileList is valid, elements: "+GString(fileList->numberOfElements()));
                if( fileList->numberOfElements() ) rc = uploadBlob(command, fileList, lobType);
            }
            else rc = SQLExecDirect(hstmtUpdate, (SQLCHAR*)command, SQL_NTS);
            deb(__FUNCTION__, "rc Upd: "+GString(rc));
            if( rc ) readErrorState(hstmtUpdate);
            else m_iLastSqlCode = 0;
        }
        break;
    }
    while ((retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) );
    //SQLCloseCursor(hstmtSelect);
    if( commitIt )  SQLEndTran(SQL_HANDLE_DBC,m_SQLHDBC,SQL_COMMIT);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmtSelect);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmtUpdate);
    SQLFreeStmt(hstmtSelect, SQL_CLOSE);
    SQLFreeStmt(hstmtUpdate, SQL_CLOSE);
    deb(__FUNCTION__, "done, rc: "+GString(rc));
    return rc ? m_strLastError : "";
}

GString db2dcli::deleteByCursor(GString filter, GString command, long deleteCount, short commitIt)
{
    deb(__FUNCTION__, "filter: "+filter+", cmd: "+command);
    int rc;
    if( deleteCount < 0 ) return "CurrentCursor(): POS < 0";
    SQLHSTMT     hstmtSelect;
    SQLHSTMT     hstmtUpdate;
    SQLRETURN    retcode = 0;


    filter += " FOR UPDATE";
    rc = SQLAllocStmt(m_SQLHDBC, &hstmtSelect); /* allocate a statement handle */
    deb(__FUNCTION__, "rc alloc SEL: "+GString(rc));
    rc = SQLSetCursorName(hstmtSelect, (SQLCHAR*)"PMF_DEL_CRS", SQL_NTS);
    deb(__FUNCTION__, "rc alloc setCRSName: "+GString(rc));
    rc = SQLAllocStmt(m_SQLHDBC, &hstmtUpdate); /* allocate a statement handle */
    deb(__FUNCTION__, "rc alloc UPD: "+GString(rc));

    rc = SQLExecDirect(hstmtSelect,(SQLCHAR*) filter, SQL_NTS);
    deb(__FUNCTION__, "rc execDirect: "+GString(rc));
    if(rc)
    {
        readErrorState(hstmtSelect);
        m_iLastSqlCode = rc;
        return rc;
    }

    long count = 0;
    command += " WHERE CURRENT OF PMF_DEL_CRS";
    do
    {
        rc = SQLFetch(hstmtSelect);
        deb(__FUNCTION__, "rc fetch: "+GString(rc));
        count++;
        if(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO)
        {
            rc = SQLExecDirect(hstmtUpdate, (SQLCHAR*)command, SQL_NTS);
            deb(__FUNCTION__, "rc Upd: "+GString(rc));
            if( rc ) readErrorState(hstmtUpdate);
            else m_iLastSqlCode = 0;
        }
        break;
    }
    while ( count < deleteCount && (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) );
    //SQLCloseCursor(hstmtSelect);
    if( commitIt )  SQLEndTran(SQL_HANDLE_DBC,m_SQLHDBC,SQL_COMMIT);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmtSelect);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmtUpdate);
    SQLFreeStmt(hstmtSelect, SQL_CLOSE);
    SQLFreeStmt(hstmtUpdate, SQL_CLOSE);
    deb(__FUNCTION__, "done, rc: "+GString(rc));
    return rc ? m_strLastError : "";
}


int  db2dcli::getDataBases(GSeq <CON_SET*> *dbList)
{
    CON_SET * pCS;
    /*
    GString err = this->initAll("SELECT NAME FROM SYS.DATABASES ORDER BY NAME");
    deb("getDataBases: "+err);
    for(unsigned long i = 1; i <= this->numberOfRows(); ++i)
    {
        pCS = new CON_SET;
        pCS->DB = this->rowElement(i, 1);
        pCS->Host = _HOST_DEFAULT;
        dbList->add(pCS);
    }
    */
    deb("getDataBases start");
    SQLHENV env;
    char dsn[256];
    char desc[256];
    SQLSMALLINT dsn_ret;
    SQLSMALLINT desc_ret;
    SQLUSMALLINT direction;
    SQLRETURN ret;

    deb("getDataBases allocating handle...");
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
    if( ret )
    {
        deb("SQLAllocHandle returned "+GString(ret));
        m_iLastSqlCode = ret;
        return sqlCode();
    }
    deb("getDataBases setting envAttr...");
    ret = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0);
    if( ret )
    {
        deb("SQLAllocHandle returned "+GString(ret));
        m_iLastSqlCode = ret;
        return sqlCode();
    }
    deb("getDataBases getting DataSources...");
    ret = SQLDataSources(env, SQL_FETCH_FIRST, (SQLCHAR*)dsn, sizeof(dsn), &dsn_ret, (SQLCHAR*)desc,
                         sizeof(desc), &desc_ret);
    if( ret != SQL_SUCCESS &&  ret != SQL_SUCCESS_WITH_INFO )
    {
        deb("SQLDataSources returned "+GString(ret));
        m_iLastSqlCode = ret;
        return sqlCode();
    }
    deb("getDataBases start read...");
    while ((ret == SQL_SUCCESS) || (ret == SQL_SUCCESS_WITH_INFO))
    {
        deb("getDataBases, got DESC: "+GString(desc)+", DSN: "+GString(dsn));
        pCS = new CON_SET;
        pCS->DB = dsn;
        deb("getDataBases, adding "+GString(dsn));
        pCS->Host = _HOST_DEFAULT;
        pCS->Type = _DB2ODBC;
        dbList->add(pCS);

        ret = SQLDataSources(env, SQL_FETCH_NEXT,
                            (SQLCHAR*)dsn, sizeof(dsn), &dsn_ret,
                            (SQLCHAR*)desc, sizeof(desc), &desc_ret);

        m_iLastSqlCode = ret;
    }

//    direction = SQL_FETCH_FIRST;
//    deb("getDataBases start read...");
//    while(SQL_SUCCEEDED(ret = SQLDataSources(env, direction,
//                    (SQLCHAR*)dsn, sizeof(dsn), &dsn_ret,
//                    (SQLCHAR*)desc, sizeof(desc), &desc_ret)))
//    {
//        direction = SQL_FETCH_NEXT;
//        deb("getDataBases, got DESC: "+GString(desc)+", DSN: "+GString(dsn));
//        pCS = new CON_SET;
//        pCS->DB = dsn;
//        deb("getDataBases, adding "+GString(dsn));
//        pCS->Host = _HOST_DEFAULT;
//        pCS->Type = _DB2ODBC;
//        dbList->add(pCS);
//    }
    deb("getDataBases, ret: "+GString(ret));
    if( dbList->numberOfElements() > 0 ) ret = 0;
    m_iLastSqlCode = ret;

    if( ret == SQL_ERROR || ret == SQL_INVALID_HANDLE) return sqlCode();
    deb("getDataBases done, ret: "+GString(ret));
    return ret == SQL_NO_DATA ? 0 : ret;
}

int  db2dcli::deleteTable(GString tableName)
{
    this->initAll("drop table "+tableName);
    if( sqlCode() && this->getDdlForView(tableName).length() )
    {
        this->initAll("drop view "+tableName);
    }
    return sqlCode();
}

long db2dcli::retrieveBlob( GString sqlCmd, GString &blobFile, int writeFile  )
{
    /********************************************************
    * writeFile == 0: Read LOB into memory
    * used by "Generate DDL"
    */
    int outSize;
    if( writeFile ) return descriptorToFile(sqlCmd, blobFile, &outSize );

    long rc = 0;
    char  *outBuffer = new char[1]; //[24102];
    remove(blobFile);

    rc = SQLAllocStmt(m_SQLHDBC, &m_SQLHSTMT);
    deb("::retrieveBlob pos 1 "+GString(rc)+", err: "+readErrorState());

    rc = SQLExecDirect(m_SQLHSTMT, sqlCmd, SQL_NTS);
    deb("::retrieveBlob pos 2 "+GString(rc)+", err: "+readErrorState());

    rc = SQLFetch(m_SQLHSTMT);
    deb("::retrieveBlob pos 3 "+GString(rc)+", err: "+readErrorState());
    if( rc ) return rc;

    SQLINTEGER lng;
    rc = SQLGetData(m_SQLHSTMT, 1, SQL_C_BINARY, (SQLPOINTER) outBuffer, 0, &lng);
    deb("::retrieveBlob LNG: "+GString(lng));
    delete[] outBuffer;
    if( lng < 0 )
    {

        return 1;
    }
    outBuffer = new char[lng];
    SQLINTEGER lngOut = 0;
    deb("::retrieveBlob Calling again...");
    rc = SQLGetData(m_SQLHSTMT, 1, SQL_C_BINARY, (SQLPOINTER) outBuffer, lng+1, &lngOut);
    deb("::retrieveBlob LNG: "+GString(lngOut));

    //outBuffer[lng] = '\0';

    deb("::retrieveBlob pos 1 "+GString(rc));
 //   deb("Buffer: "+GString(outBuffer));
   deb("::retrieveBlob Writing file..., file: "+GString(blobFile));
   blobFile = GString(outBuffer, lngOut);
   delete [] outBuffer;
   return 0;
    //return writeToFile((char*) outBuffer, lng, blobFile);

}


#ifdef  QT4_DSQL
GString db2dcli::getIdenticals(GString table, QWidget* parent, QListWidget *pLB, short autoDel)
{
    GString message = "SELECT * FROM "+table;
    GString retString = "";
    deb("::getIdenticals, cmd: "+message);


    if( m_SQLHDBC == SQL_NULL_HDBC )
    {
        deb("::getIdenticals: No handle, quitting.");
        return "";
    }
    clearSequences();

    SQLRETURN ret = 0;
    SQLSMALLINT columns;

    SQLFreeHandle(SQL_HANDLE_DBC, m_SQLHDBC);
    ret = SQLAllocHandle(SQL_HANDLE_STMT, m_SQLHDBC, &m_SQLHSTMT);
    deb("::getIdenticals, SQLAllocHandle, rc: "+GString(ret));
    if(ret) return sqlError();

    ret = SQLExecDirect(m_SQLHSTMT, (SQLCHAR*)message, SQL_NTS);
    deb("::getIdenticals, SQLExecDirect, rc: "+GString(ret));
    if(ret) return sqlError();


    ret = SQLNumResultCols(m_SQLHSTMT, &columns);

    deb("::getIdenticals, SQLNumResultCols, rc: "+GString(ret)+", cols: "+GString(columns));
    if(ret) return sqlError();
    m_iNumberOfColumns = columns;

    getColInfo(&m_SQLHSTMT);

    char *buf;
    GString data, countCmd, out;

    db2dcli * tmpODBC = new db2dcli(*this);
    db2dcli * delODBC = new db2dcli(*this);

    int blockSize = 2000;
    unsigned long lns =0, perc = 0, pass = 1;
    QProgressDialog * apd = NULL;
    apd = new QProgressDialog(GString("Searching in "+table), "Cancel", 0, blockSize, parent);
    apd->setWindowModality(Qt::WindowModal);
    apd->setValue(1);


    while (SQL_SUCCEEDED(ret = SQLFetch(m_SQLHSTMT)))
    {
        SQLINTEGER len = 0;
        int bufLen;
        if( !(perc % 10) ) apd->setValue(perc);
        if( apd->wasCanceled() )
        {
           retString = "QUIT";
           break;
        }

        countCmd = "";
        for ( int i = 1; i <= m_iNumberOfColumns; i++ )
        {
            if(isXMLCol(i) || isLOBCol(i))continue;

            bufLen = sqlVarLengthSeq.elementAtPosition(i)+1;

            buf = new char[bufLen];
            ret = SQLGetData(m_SQLHSTMT, i, SQL_C_CHAR, buf, bufLen, &len);

            deb("::getIdenticals, row: "+GString(m_iNumberOfRows)+", col: "+GString(i)+", ret: "+GString(ret)+", len: "+GString(len)+", bufLen: "+GString(bufLen));
            if (len == SQL_NULL_DATA)  data = " IS NULL";
            else
            {
                data = buf;
                deb("Buf is valid, data: "+data);
                //if( sqlTypeSeq.elementAtPosition(i) == SQLSRV_DATETIME ) handleDateTimeString(data);
                if( !isNumType(i)) data = " = '"+data+"'";
                else data = " = "+data;
            }
            delete[] buf;
            countCmd += hostVariable(i)+data+ " AND ";
        }
        countCmd = countCmd .stripTrailing(" AND ");
        lns++;
        perc++;
        if( (int)perc > blockSize )
        {
            pass++;
            apd->setLabelText(table+", chunk #"+GString(pass)+" (2000 rows per chunk)");
            apd->setValue(pass);
            perc = 1;
        }
        deb("getIdenticals: CountCmd: "+countCmd);
        tmpODBC->initAll( "SELECT COUNT(*) FROM "+table+" WHERE "+countCmd);
        if(tmpODBC->rowElement(1,1).asInt() > 1)
        {
            out = table +": Found "+GString(tmpODBC->rowElement(1,1))+" results for "+countCmd;
            writeToLB(pLB, out);
        }
        for(int i=1; i<tmpODBC->rowElement(1,1).asInt() && autoDel; ++i)
        {
            delODBC->currentCursor("SELECT * FROM "+table+" WHERE "+countCmd, "DELETE FROM "+table, 1, 0);
        }


    }
    if( apd )
    {
        apd->setValue(blockSize);
        delete apd;
    }

    delete tmpODBC;
    delete delODBC;
    deb("::getIdenticals, err fetch: "+GString(ret));
    SQLFreeStmt(m_SQLHSTMT, SQL_CLOSE);
    if( ret && ret != 100) return m_strLastError;
    return retString;
}

void db2dcli::writeToLB(QListWidget * pLB, GString message)
{
    for( int i = 0; i < pLB->count(); ++i )
        if( GString(pLB->item(i)->text()) == message ) return;
    new QListWidgetItem(message, pLB);
}

#endif


void db2dcli::clearCommSequences()
{
    RowData *pRowData;
    while( !rowSeq.isEmpty() )
    {
        pRowData = rowSeq.firstElement();
        delete pRowData;
        rowSeq.removeFirst();
    }
}

GString db2dcli::fillChecksView(GString table, int showRaw)
{
    return "";
}


int db2dcli::forceApp(int appID)
{
    PMF_UNUSED(appID);
    return -1;
}

void db2dcli::setCLOBReader(short readCLOBData )
{
    m_iReadCLOBs = readCLOBData;
}

void db2dcli::setCurrentDatabase(GString db)
{
    PMF_UNUSED(db);
}


GString db2dcli::currentDatabase()
{
    return m_strDB;
}

unsigned long db2dcli::getResultCount(GString cmd)
{
    PMF_UNUSED(cmd);
    return -1;
}

int db2dcli::getColSpecs(GString table, GSeq<COL_SPEC*> *specSeq)
{
    GString tabSchema = this->tabSchema(table);
    GString tabName   = this->tabName(table);

   //DB2 v10: Column DEFAULT in SYSCAT.COLUMN is now CLOB (used to be VARCHAR). Sigh. We need to read the CLOB.
    this->setCLOBReader(1); //This will read any CLOB into fieldData


    GString cmd = "select colname, typename, length, scale, nulls, default, logged, compact, identity, generated, codepage "
        " from syscat.columns "
        " where tabschema = '"+tabSchema+"' and tabname ='"+tabName+"'  order by colno";

    deb("::getColSpecs: "+cmd);

    this->initAll(cmd);

    COL_SPEC *cSpec;


    for( unsigned i=1; i<=this->numberOfRows(); ++i )
    {
        cSpec = new COL_SPEC;
        cSpec->init();

        cSpec->ColName = this->rowElement(i,1).strip("'");

        /* colType CHARACTER (from syscat.tables) needs to be truncated to CHAR */
        cSpec->ColType = this->rowElement(i,2).strip("'");
        if( cSpec->ColType == "CHARACTER" ) cSpec->ColType = "CHAR";


        /* SMALLINT, INTEGER, BIGINT, DOUBLE */
        if( cSpec->ColType.occurrencesOf("INT") > 0 ||
                cSpec->ColType.occurrencesOf("DOUBLE") > 0 ||
                cSpec->ColType.occurrencesOf("DATE") > 0 ||
                cSpec->ColType.occurrencesOf("XML") > 0 ||
                cSpec->ColType.occurrencesOf("TIME"))
        {
            cSpec->Length = "N/A";
        }
        /* DECIMAL: (LENGTH, SCALE) */
        else if( cSpec->ColType.occurrencesOf("DECIMAL") )
        {
            cSpec->Length = this->rowElement(i,3)+", "+this->rowElement(i,4);
        }
        else if( cSpec->ColType.occurrencesOf("XML") )
        {
            cSpec->Length = "0";
        }
        else if( cSpec->ColType.occurrencesOf("LONG VARCHAR") || cSpec->ColType.occurrencesOf("LONG CHAR") )
        {
            cSpec->Length = "N/A";
        }
        /* xCHAR, xLOB etc */
        else
        {
            cSpec->Length = this->rowElement(i,3);
        }
        if( (cSpec->ColType.occurrencesOf("CHAR"))  && this->rowElement(i, 11) == "0" ) //codepage is 0 for CHAR FOR BIT
        {
            cSpec->Misc = "FOR BIT DATA";
        }

        /* check for NOT NULL, DEFAULT */
        if( this->rowElement(i,5) == "'N'" )
        {
            cSpec->Nullable = "NOT NULL";
        }
        //if( this->rowElement(i,6) != "NULL" )
        if( !this->isNull(i, 6) )
        {
            //cSpec->Default = cleanString(this->rowElement(i,6));
            cSpec->Default = this->rowElement(i,6);
        }
        if( this->rowElement(i,7) == "'N'" )
        {
            cSpec->Misc = "NOT LOGGED";
        }
        if( this->rowElement(i,7) == "'Y'" )
        {
            cSpec->Misc = "LOGGED";
        }
        if( this->rowElement(i,8) == "'Y'" )
        {
            cSpec->Misc += " COMPACT";
        }
        else if( this->rowElement(i,8) == "'N'" )
        {
            cSpec->Misc += " NOT COMPACT";
        }

        /* GENERATED ALWAYS IDENTITY */
        if( this->rowElement(i,9) == "'Y'" && this->rowElement(i,10) == "'A'")
        {
            int start, incr;
            getIdentityColParams(table, &start, &incr);
            cSpec->Misc = "GENERATED ALWAYS AS IDENTITY (START WITH "+GString(start)+", INCREMENT BY "+GString(incr)+")";
        }
        specSeq->add(cSpec);

    }


    return 0;
}

int db2dcli::getIdentityColParams(GString table, int *start, int * incr)
{
    db2dcli tmp = db2dcli(*this);
    tmp.initAll("SELECT start, increment FROM SYSCAT.COLIDENTATTRIBUTES WHERE TABSCHEMA='"+tabSchema(table)+"' AND TABNAME='"+tabName(table)+"'");
    *start = tmp.rowElement(1,1).asInt();
    *incr = tmp.rowElement(1,2).asInt();
    return 0;
}


int db2dcli::getTriggers(GString table, GString *text)
{
    *text = "";
    GSeq <GString> trgSeq = getTriggerSeq(table);
    for(int i = 1; i <= trgSeq.numberOfElements(); ++i)
    {
        *text += "\n-- Trigger #"+GString(i)+":\n"+trgSeq.elementAtPosition(i);
    }
    return 0;
}

int db2dcli::hasUniqueConstraint(GString tableName)
{
    GString cmd = "SELECT COUNT(UNIQUE_COLCOUNT) FROM SYSCAT.INDEXES WHERE TABSCHEMA='"+tabSchema(tableName)+
            "' AND TABNAME='"+tabName(tableName)+"' AND UNIQUE_COLCOUNT > 0 AND "+
            "INDEXTYPE IN ('BLOK','CLUS','DIM','REG')";

     this->initAll(cmd);
     return this->rowElement(1,1).asInt() > 0 ? 1 : 0;
}


int db2dcli::getUniqueCols(GString table, GSeq <GString> * colSeq)
{

    deb("::getUniqueCols start");
    GString cmd = "SELECT colnames FROM SYSCAT.INDEXES WHERE TABSCHEMA='"+tabSchema(table)+"' AND "
                   "TABNAME='"+tabName(table)+"' AND (UNIQUERULE  ='P' or UNIQUERULE ='U') and "
                   "INDEXTYPE IN ('BLOK','CLUS','DIM','REG')";

    this->initAll(cmd);
    deb("::getUniqueCols rows: "+GString(this->numberOfRows()));
    if( this->numberOfRows() == 0 ) return 1;
    GString cols = this->rowElement(1,1).strip("'").stripLeading('+'); //First row is enough
    deb("::getUniqueCols val for cols: "+cols);
    cols = cols.change('+', ',').change('-', ',').change('*', ',');
    cols = cols.stripLeading(',');
    *colSeq = cols.split(',');


//    GString cmd = "SELECT colnames FROM SYSCAT.INDEXES WHERE upper(TABSCHEMA)='"+tabSchema(table)+"' AND "
//        "upper(TABNAME)='"+tabName(table)+"' AND (UNIQUERULE  ='P' or UNIQUERULE ='U') and "
//        "COLNAMES not in (select '+' || COLNAME from SYSCAT.COLUMNS where TYPENAME ='XML' ) order by UNIQUERULE";
//    this->initAll(cmd);
//    if( this->numberOfRows() == 0 ) return 1;
//    GString cols = this->rowElement(1,1).strip("'").stripLeading('+'); //First row is enough
//    while(cols.occurrencesOf("+"))
//    {
//        colSeq->add(cols.subString(1, cols.indexOf("+")-1));
//        cols = cols.remove(1, cols.indexOf("+") );
//    }
//    colSeq->add(cols);
    return 0;
}
GSeq <IDX_INFO*> db2dcli::getIndexeInfo(GString table)
{
    GString tabSchema = this->tabSchema(table);
    GString tabName   = this->tabName(table);
    GString indSchema, indName;

    GString colNames, out = "";

    GString cmd = "SELECT IID, INDSCHEMA as Schema, INDNAME As Name, COLNAMES "
                  "as Columns, UNIQUERULE as TYPE,"
                  "t.TBSPACE as TABSPACE , i.CREATE_TIME, i.Definer, STATS_TIME, 'N/A' as Delete_Rule "
                  "FROM SYSCAT.INDEXES i, SYSCAT.TABLESPACES t WHERE "
                  "TABSCHEMA='"+tabSchema+"' AND "
                  "TABNAME='"+tabName+"' and t.TBSPACEID=i.TBSPACEID order by IID";
    this->initAll(cmd);


    GSeq <IDX_INFO*> indexSeq;
    IDX_INFO * pIdxInfo;
    for(unsigned long  i=1; i<=this->numberOfRows(); ++i )
    {
        pIdxInfo = new IDX_INFO;
        pIdxInfo->init();

        if( this->rowElement(i,5).strip("'").upperCase() == "P" ) pIdxInfo->Type = DEF_IDX_PRIM;
        else if( this->rowElement(i,5).strip("'").upperCase() == "U" ) pIdxInfo->Type = DEF_IDX_UNQ;
        else pIdxInfo->Type = DEF_IDX_DUPL;

        indSchema = this->rowElement(i,2).strip("'").strip();
        indName = this->rowElement(i,3).strip("'");
        //colNames = this->rowElement(i,4).strip("'");
        colNames = setIndexColUse(indSchema, indName, pIdxInfo);


        pIdxInfo->Iidx        = this->rowElement(i,1).strip("'").strip();
        pIdxInfo->Schema      = indSchema;
        pIdxInfo->Name        = indName;
        pIdxInfo->Columns     = colNames;
        pIdxInfo->TabSpace    = this->rowElement(i,6).strip("'");
        pIdxInfo->CreateTime  = this->rowElement(i,7).strip("'");
        pIdxInfo->Definer     = this->rowElement(i,8).strip("'");
        pIdxInfo->StatsTime   = this->rowElement(i,9).strip("'");
        pIdxInfo->DeleteRule   = this->rowElement(i,10).strip("'");
        out = "";
        if( pIdxInfo->Type == DEF_IDX_PRIM )
        {
            pIdxInfo->Stmt = "ALTER TABLE "+table+" ADD CONSTRAINT "+indName+" PRIMARY KEY ("+colNames+");";
        }
        else
        {
            out = "CREATE ";
            if( pIdxInfo->Type == DEF_IDX_UNQ ) out += "UNIQUE ";
            out += "INDEX "+indSchema+"."+indName+" ON ";
            out += table+" ("+colNames + ");";
            pIdxInfo->Stmt = out;
        }
        indexSeq.add(pIdxInfo);
    }


    //GString yep = "trim(FK_COLNAMES )  concat ' => ' concat trim(REFTABSCHEMA) concat '.' concat trim(REFTABNAME) concat  ' [' concat trim(PK_COLNAMES) concat  ']'";
    GString yep = "trim(FK_COLNAMES )"; //  concat ' => ' concat trim(REFTABSCHEMA) concat '.' concat trim(REFTABNAME) concat  ' [' concat trim(PK_COLNAMES) concat  ']'";
    cmd = "select 'N/A', tabschema, constname, "+yep+", 'Foreign Key', 'N/A', create_time, definer, 'N/A', case DELETERULE when 'C' then 'DELETE CASCADE' else '' end from syscat.references "
                                                      " where tabschema = '"+tabSchema+"' and tabname ='"+tabName+"'";
    GString err = this->initAll(cmd);
    if( err.length() )
    {
        //yep = "FK_COLNAMES concat ' => ' concat REFTABSCHEMA concat '.' concat REFTABNAME concat  ' [' concat PK_COLNAMES concat  ']'";
        yep = "FK_COLNAMES"; // concat ' => ' concat REFTABSCHEMA concat '.' concat REFTABNAME concat  ' [' concat PK_COLNAMES concat  ']'";
    }
    cmd = "select 'N/A', tabschema, constname, "+yep+", 'Foreign Key', 'N/A', create_time, definer, 'N/A', case DELETERULE when 'C' then 'DELETE CASCADE' else '' end from syscat.references "
                                                     " where tabschema = '"+tabSchema+"' and tabname ='"+tabName+"'";
    this->initAll(cmd);

    //printf("CMD: %s\n", (char*) cmd);

    for( int i = 1; i <= (int)this->numberOfRows(); ++i )
    {
        pIdxInfo = new IDX_INFO;
        pIdxInfo->init();

        indSchema = this->rowElement(i,2).strip("'").strip();
        indName = this->rowElement(i,3).strip("'");


        pIdxInfo->Type        = DEF_IDX_FORKEY;
        colNames = this->rowElement(i,4).strip("'");
        colNames = setIndexColUse(indSchema, indName, pIdxInfo);


        pIdxInfo->Iidx        = this->rowElement(i,1).strip("'").strip();
        pIdxInfo->Schema      = indSchema;
        pIdxInfo->Name        = indName;
        pIdxInfo->Columns     = this->rowElement(i,4).strip("'").removeButOne().change(" ", ", ");

        pIdxInfo->TabSpace    = "N/A";
        pIdxInfo->CreateTime  = this->rowElement(i,"CREATE_TIME").strip("'");
        pIdxInfo->Definer     = this->rowElement(i,"DEFINER").strip("'");
        pIdxInfo->StatsTime   = "N/A";
        pIdxInfo->DeleteRule   = this->rowElement(i,10).strip("'");
        indexSeq.add(pIdxInfo);
    }
    //Get foreign key statements: This re-initializes (*this)
    for( int i = 1; i <= indexSeq.numberOfElements(); ++i )
    {
        pIdxInfo = indexSeq.elementAtPosition(i);
        setForKeyDetails(table, pIdxInfo);
    }
    return indexSeq;
}

GString db2dcli::setIndexColUse(GString indSchema, GString indName, IDX_INFO* pIdx)
{
    indName = indName.strip("'").strip();
    indSchema = indSchema.strip("'").strip();

    deb("setIndexColUse, indSchema: "+indSchema+", indName: "+indName);
    GString colName, out, cmd, orderType;

    cmd = "select colname, case colorder when 'A' then 'ASC' when 'D' then 'DESC' when 'A' then 'RAND' else '' end as OrderNr, text ";
    cmd += "from SYSCAT.INDEXCOLUSE where indSchema='"+indSchema+"' and indname='"+indName+"' order by colseq";
    db2dcli tmpDSQL = db2dcli(*this);
    tmpDSQL.setCLOBReader(1);
    tmpDSQL.initAll(cmd);
    for( int i = 1; i <= tmpDSQL.numberOfRows(); ++i )
    {
        if( pIdx->Type != DEF_IDX_PRIM ) orderType = " " + tmpDSQL.rowElement(i,2).strip("'");
        colName = tmpDSQL.rowElement(i,3) == "NULL" ? tmpDSQL.rowElement(i,1) : tmpDSQL.rowElement(i,3);
        out += "\""+colName.strip("'")+"\"" + orderType +", ";
    }
    out = out.stripTrailing(", ");
    tmpDSQL.setCLOBReader(0);
    return out;
}

void db2dcli::setForKeyDetails(GString table, IDX_INFO* pIdx)
{

    if( pIdx->Type != DEF_IDX_FORKEY) return;

    GString tabSchema = this->tabSchema(table);
    GString tabName   = this->tabName(table);

    GString cmd = "select reftabschema, reftabname, deleterule, updaterule, fk_colnames, pk_colnames from syscat.references "
        " where tabschema = '"+tabSchema+"' and tabname ='"+tabName+"' and constname='"+pIdx->Name+"'";
    this->initAll(cmd);
    GString temp, fk, pk, fTable, reftabschema, reftabname, out;

    reftabschema = this->rowElement(1, 1).strip("'").strip();
    reftabname   = this->rowElement(1, 2).strip("'").strip();
    fTable = reftabschema+"."+reftabname.strip("'");
    temp = this->rowElement(1, 5).strip("'").strip().removeButOne();
    GSeq<GString> fkColSeq = temp.split(' ');
    for(int i = 1; i <= fkColSeq.numberOfElements(); ++i )
    {
        fk += "\""+fkColSeq.elementAtPosition(i)+"\", ";
    }
    fk = fk.strip().stripTrailing(",");

    temp = this->rowElement(1, 6).strip("'").strip().removeButOne();
    GSeq<GString> pkColSeq = temp.split(' ');
    for(int i = 1; i <= pkColSeq.numberOfElements(); ++i )
    {
        pk += "\""+pkColSeq.elementAtPosition(i)+"\", ";
    }
    pk = pk.strip().stripTrailing(",");

    out += "ALTER TABLE "+tabSchema+"."+tabName+" ADD CONSTRAINT "+pIdx->Name+" FOREIGN KEY ("+fk+") REFERENCES ";
    GString type = this->rowElement(1, 3).strip("'").strip();
    if( type == "C" )
    {
        out += " "+fTable+"("+pk+") ON DELETE CASCADE;";
        pIdx->DeleteRule = "ON DELETE CASCADE";
    }
    else if( type == "A" )
    {
        out += " "+fTable+"("+pk+") ON DELETE NO ACTION;";
        pIdx->DeleteRule = "ON DELETE NO ACTION";
    }
    else if( type == "N" )
    {
        out += " "+fTable+"("+pk+") ON DELETE SET NULL;";
        pIdx->DeleteRule = "ON DELETE SET NULL";
    }
    else if( type == "R" )
    {
        out += " "+fTable+"("+pk+") ON DELETE RESTRICT;";
        pIdx->DeleteRule = "ON DELETE RESTRICT";
    }
    else out += "    "+fTable+"("+pk+");";
    pIdx->Stmt = out.removeButOne();
    pIdx->RefCols = pk;
    pIdx->Columns = fk;
    pIdx->RefTab = fTable;
}

GString db2dcli::getChecks(GString table, GString filter)
{
    GString text;
    GSeq <GString> checksSeq = getChecksSeq(table);
    for(int i = 1; i <= checksSeq.numberOfElements(); ++i)
    {
        text += "\n-- Check #"+GString(i)+":\n"+checksSeq.elementAtPosition(i)+"\n";
    }
    return text;
}

GSeq <GString> db2dcli::getChecksSeq(GString table, GString filter)
{
    deb("getChecksSeq start");
    GString tabSchema = this->tabSchema(table);
    GString tabName   = this->tabName(table);
    //DB2 v10: Column DEFAULT in SYSCAT.COLUMN is now CLOB (used to be VARCHAR). Sigh. We need to read the CLOB.
    this->setCLOBReader(1); //This will read any CLOB into fieldData

    int erc;
    GString cmd = "select constname from syscat.checks "
        " where tabschema = '"+tabSchema+"' and tabname ='"+tabName+"'";
    this->initAll(cmd);

    GString blobText, out, temp;
    deb("getChecksSeq, Checks: "+GString(this->numberOfRows()));
    GSeq <GString> checksSeq;
    for( int i = 1; i <= (int)this->numberOfRows(); ++i )
    {
        out = "";
        temp = "Select text from syscat.checks where constname="+this->rowElement(i,1);
        temp += " and tabschema = '"+tabSchema+"' and tabname ='"+tabName+"'";

        erc = this->retrieveBlob( temp, blobText, 0 );
        if( erc )
        {
            out += temp+"\n";
            out += "-- !! ERROR "+GString(erc)+" retrieving check '"+this->rowElement(i,1)+"'\n";
            continue;
        }
        else
        {
            blobText = blobText.change("\t", " ").removeButOne().strip();
            out = "ALTER TABLE "+table+" ADD CONSTRAINT \""+this->rowElement(i,1)+"\" CHECK ("+blobText+");";
        }
        checksSeq.add(out);
    }
    return checksSeq;
}

GSeq <GString> db2dcli::getTriggerSeq(GString table)
{
    deb("getTriggerSeq start");
    GString tabSchema = this->tabSchema(table);
    GString tabName   = this->tabName(table);
    //DB2 v10: Column DEFAULT in SYSCAT.COLUMN is now CLOB (used to be VARCHAR). Sigh. We need to read the CLOB.
    this->setCLOBReader(1); //This will read any CLOB into fieldData

    int erc;
    GString cmd = "select trigschema, trigname, valid from syscat.triggers "
        " where tabschema = '"+tabSchema+"' and tabname ='"+tabName+"'";
    this->initAll(cmd);

    GString blobText, out, temp;
    deb("getTriggerSeq, triggers: "+GString(this->numberOfRows()));
    GSeq <GString> triggerSeq;
    for( int i = 1; i <= (int)this->numberOfRows(); ++i )
    {
        out = "";
        temp = "Select text from syscat.triggers where trigschema="+this->rowElement(i,1);
        temp += " and trigname = "+this->rowElement(i,2);

        erc = this->retrieveBlob( temp, blobText, 0 );
        if( erc )
        {
            out += temp+"\n";
            out += "-- !! ERROR "+GString(erc)+" retrieving trigger '"+this->rowElement(i,1)+"."+this->rowElement(i,2)+"'\n";
            continue;
        }
        blobText = blobText.change("\t", " ").strip();
        if( this->rowElement(i, 3).upperCase() != "'Y'" ) out += "-- !! This trigger is NOT ACTIVE !! -- ";
//        while ( blobText.occurrencesOf("\n") > 0 )
//        {
//            blobText = blobText.replaceAt(blobText.indexOf((char*)"\n"), (char*)" "); //  .remove(blobText.indexOf("\n"), 1);
//        }
        triggerSeq.add(out+blobText+";");
    }
    return triggerSeq;
}


int db2dcli::minColSeparator(GString col)
{
    for(int i = 1; i <= col.length(); ++i)
    {
        if( col[i] == '+') return i;
        if( col[i] == '-') return i;
        if( col[i] == '*') return i;
    }
    return 0;
}


void db2dcli::createXMLCastString(GString &xmlData)
{
    deb("::createXMLCastString called, in: "+xmlData);
    xmlData = " XMLPARSE (DOCUMENT('"+(xmlData)+"')) ";
    deb("::createXMLCastString called, out: "+xmlData);

}

int  db2dcli::isBinary(unsigned long row, int col)
{
    if( row < 1 || row > allRowsSeq.numberOfElements() ) return 0;
    GRowHdl *aRow;
    aRow = allRowsSeq.elementAtPosition(row);
    if( col < 1 || (unsigned long) col > aRow->elements() ) return 0;
    deb("::isBinary: "+GString(aRow->rowElement(col)->isBinary)+", row: "+GString(row)+", col: "+GString(col));
    return aRow->rowElement(col)->isBinary;
}

int  db2dcli::isTruncated(unsigned long row, int col)
{
    if( row < 1 || row > allRowsSeq.numberOfElements() ) return 0;
    GRowHdl *aRow;
    aRow = allRowsSeq.elementAtPosition(row);
    if( col < 1 || (unsigned long) col > aRow->elements() ) return 0;
    deb("::isTruncated: row: "+GString(row)+", col: "+GString(col)+", trunc: "+GString(aRow->rowElement(col)->isTruncated));
    return aRow->rowElement(col)->isTruncated;
}

int  db2dcli::isNull(unsigned long row, int col)
{
    if( row < 1 || row > allRowsSeq.numberOfElements() ) return 0;
    GRowHdl *aRow;
    aRow = allRowsSeq.elementAtPosition(row);
    if( col < 1 || (unsigned long) col > aRow->elements() ) return 0;
    deb("::isNull: "+GString(aRow->rowElement(col)->isNull)+", row: "+GString(row)+", col: "+GString(col));
    return aRow->rowElement(col)->isNull;
}

int db2dcli::uploadLongBuffer( GString cmd, GString data, int isBinary)
{
    deb("::uploadLongBuffer, cmd: "+cmd+", isBinary: "+GString(isBinary));
    int rc;
    SQLRETURN       ret = 0;

    m_iLastSqlCode = 0;


    //char ** fileBuf = new char* [fileSeq->numberOfElements()];
    SQLINTEGER size;

    deb("::uploadLongBuffer, SQLAllocHandle, rc: "+GString(ret));

    if( m_SQLHSTMT) SQLFreeHandle(SQL_HANDLE_STMT, m_SQLHSTMT);
    SQLAllocHandle(SQL_HANDLE_STMT, m_SQLHDBC, &m_SQLHSTMT);

    SQLPrepare(m_SQLHSTMT, (SQLCHAR*)cmd, SQL_NTS);

    if( isBinary )
    {
        rc = SQLBindParameter(m_SQLHSTMT, 1, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_LONGVARBINARY, size, 0, (SQLCHAR*)data, size, &size) ;
    }
    else
    {
        rc = SQLBindParameter(m_SQLHSTMT, 1, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_LONGVARCHAR, size, 0, (SQLCHAR*)data, size, &size) ;
    }
    deb("::uploadLongBuffer, SQLBindParameter, rc: "+GString(rc));
    ret = SQLExecute(m_SQLHSTMT);
    deb("::uploadLongBuffer, SQLExecute, rc: "+GString(ret));


    readErrorState();
    deb("::uploadLongBuffer, SQLExecute, rc: "+GString(ret)+", err: "+sqlError());

    SQLFreeStmt(m_SQLHSTMT, SQL_CLOSE);
    m_iLastSqlCode = ret;
    return ret;
}

GString db2dcli::cleanString(GString in)
{
    if( in.length() < 2 ) return in;
    if( in[1UL] == '\'' && in[in.length()] == '\'' ) return in.subString(2, in.length()-2);
    return in;
}

void db2dcli::getResultAsHEX(int asHex)
{
    m_iGetResultAsHEX = asHex;
}

void db2dcli::setGDebug(GDebug *pGDB)
{
    m_pGDB = pGDB;
}

void db2dcli::setCharForBit(int val)
{
    m_iCharForBit = val;
}

int db2dcli::exportAsTxt(int , GString , GString , GString , GSeq <GString>* , GSeq <GString>* , GString *)
{
    return -1;
}

db2dcli::RowData::~RowData()
{
    rowDataSeq.removeAll();
}
void db2dcli::RowData::add(GString data)
{
    rowDataSeq.add(data);
}
GString db2dcli::RowData::elementAtPosition(int pos)
{
    if( pos < 1 || pos > (int)rowDataSeq.numberOfElements() ) return "";
    return rowDataSeq.elementAtPosition(pos);
}
int db2dcli::getHeaderData(int pos, GString * data)
{
    if( pos < 1 || pos > (int)headerSeq.numberOfElements()) return 1;
    *data = headerSeq.elementAtPosition(pos);
    return 0;
}
unsigned long db2dcli::RowData::numberOfElements()
{
    return rowDataSeq.numberOfElements();
}

int db2dcli::getRowData(int row, int col, GString * data)
{
    if( row < 1 || row > (int)rowSeq.numberOfElements() ) return 1;
    if( col < 1 || col > (int)rowSeq.elementAtPosition(row)->numberOfElements() ) return 1;
    *data = rowSeq.elementAtPosition(row)->elementAtPosition(col);
    return 0;
}

unsigned long db2dcli::getRowDataCount()
{
    return rowSeq.numberOfElements();
}

unsigned long db2dcli::getHeaderDataCount()
{
    return headerSeq.numberOfElements();
}

int db2dcli::isTransaction()
{
    return m_iIsTransaction;
}

void db2dcli::setDatabaseContext(GString )
{

}


int db2dcli::getConnData(GString * db, GString *uid, GString *pwd) const
{
    *db = m_strDB;
    *uid = m_strUID;
    *pwd = m_strPWD;
    return 0;
}

int db2dcli::getColInfo(SQLHSTMT * stmt)
{
    SQLCHAR         colName[128];
    SQLSMALLINT     sqlType;
    SQLSMALLINT     colSize;
    SQLSMALLINT     nullable;
    SQLUINTEGER         size;
    SQLSMALLINT     scale;
    SQLINTEGER      displaysize;

    int rc = 0;

    deb("::getColInfo start");

    simpleColTypeSeq.removeAll();
    for (int i = 1; i <= m_iNumberOfColumns; i++)
    {

        SQLDescribeCol (*stmt, i, colName, sizeof (colName),  &colSize, &sqlType, &size, &scale, &nullable);
        deb("Describe for "+GString(colName)+": colSize "+GString(colSize)+", type: "+GString(sqlType)+", size: "+GString(size));
        hostVarSeq.add(GString(colName, colSize).strip());
        sqlTypeSeq.add(sqlType);
        setSimpleTypeSeq(sqlType);

        sqlIndVarSeq.add(nullable);
        if( sqlType == SQL_BINARY || sqlType == SQL_VARBINARY  )sqlForBitSeq.add(m_iCharForBit);
        else sqlForBitSeq.add(0);

        if( sqlType == DB2CLI_BIT  )sqlBitSeq.add(1);
        else sqlBitSeq.add(0);

        if(sqlType == DB2CLI_NVARCHAR ) sqlVarLengthSeq.add(32000);
        else sqlVarLengthSeq.add(size);

        deb("::getColInfo, sqlVarLengthSeq: "+GString(sqlVarLengthSeq.numberOfElements())+" elements");
        if(sqlType == DB2CLI_XML || sqlType == -370 ) xmlSeq.add(1);
        else xmlSeq.add(0);

        if( sqlType == DB2CLI_BLOB ) lobSeq.add(1); //BLOB
        else if(sqlType == DB2CLI_CLOB ) lobSeq.add(2); //CLOB
        else if(sqlType == DB2CLI_DBCLOB ) lobSeq.add(3); //DBCLOB
        else lobSeq.add(0);

        sqlLongTypeSeq.add(0);

        //BINDING
        SQLColAttributes(m_SQLHSTMT, i , SQL_COLUMN_DISPLAY_SIZE, NULL, 0, NULL, &displaysize);
        collen[i] = max(displaysize, (signed) strlen((char *) colName)) + 1;
        if( sqlType == DB2CLI_BLOB || sqlType == DB2CLI_DBCLOB ) collen[i] = colSize;
        if( sqlType == DB2CLI_XML )collen[i] = XML_MAX;

        if( sqlType == DB2CLI_BLOB )
        {
            rc = SQLBindCol(m_SQLHSTMT, i, SQL_C_BLOB_LOCATOR, &clobLoc, collen[i], &outlen[i]);
        }
        else if( sqlType == DB2CLI_CLOB && !m_iReadCLOBs )
        {
            rc = SQLBindCol(m_SQLHSTMT, i, SQL_C_CLOB_LOCATOR, &clobLoc, collen[i], &outlen[i]);
        }
        else if( sqlType == DB2CLI_DBCLOB && !m_iReadCLOBs )
        {
            rc = SQLBindCol(m_SQLHSTMT, i, SQL_DBCLOB_LOCATOR, &clobLoc, collen[i], &outlen[i]);
        }

        //else if( sqlType == SQL_DOUBLE ) rc = SQLBindCol(m_SQLHSTMT, i, SQL_C_DOUBLE, &clobLoc, collen[i], &outlen[i]);
        //else if( sqlType == SQL_BINARY ) rc = SQLBindCol(m_SQLHSTMT, i, SQL_BINARY, &clobLoc, collen[i], &outlen[i]);


        deb("Col "+GString(i)+": Name: "+GString(colName)+", type: "+GString(sqlType)+", colLen[i]: "+GString(collen[i])+", colSize: "+GString(colSize)+", varSize: "+GString(size));

    }
    deb("::getColInfo done");
    return rc;
}


void db2dcli::setSimpleTypeSeq(int colType)
{
    deb("setSimpleTypeSeq, in: "+GString(colType));
    switch(colType)
    {

        case SQL_GRAPHIC:
        case SQL_VARGRAPHIC:
        case SQL_LONGVARGRAPHIC:
            simpleColTypeSeq.add(CT_GRAPHIC);
            break;

        case DB2CLI_DBCLOB:
            simpleColTypeSeq.add(CT_DBCLOB);
            break;

        case DB2CLI_LONGVARCHAR:
            simpleColTypeSeq.add(CT_LONG);
            break;

        case SQL_BLOB:
            simpleColTypeSeq.add(CT_BLOB);
            break;

        case SQL_CLOB:
            simpleColTypeSeq.add(CT_CLOB);
            break;

        case DB2CLI_XML:
            simpleColTypeSeq.add(CT_XML);
            break;

        default:
            simpleColTypeSeq.add(CT_STRING);
    }
}

int db2dcli::handleLobsAndXmls(int col, GString * out, int getFullXML, short* isNull)
{

    PMF_UNUSED(getFullXML);
    *out = "";
    *isNull = 0;
    deb("::handleLobsAndXmls, col: "+GString(col)+", lob: "+GString(lobSeq.elementAtPosition(col))+", xml: "+GString(xmlSeq.elementAtPosition(col))+", m_iReadCLOBs: "+GString(m_iReadCLOBs));

    if( isLOBCol(col) == 0 && isXMLCol(col) == 0 ) return 0;

    SQLCHAR* buf = (SQLCHAR *) malloc(XML_MAX+1);
    SQLINTEGER ind = 0;
    int rc;
    SQLINTEGER strLng;
    SQLSMALLINT sqlColType;
    if( lobSeq.elementAtPosition(col) == 1 || lobSeq.elementAtPosition(col) == 2) sqlColType = SQL_C_BINARY;
    else if( lobSeq.elementAtPosition(col) == 3 ) sqlColType = SQL_C_CHAR;

    if( lobSeq.elementAtPosition(col) > 0 && !m_iReadCLOBs ||
        lobSeq.elementAtPosition(col) == 1 && m_iReadCLOBs  )
    {
        deb("Trying as LOCATOR");
        if( lobSeq.elementAtPosition(col) == 1 ) rc = SQLGetData(m_SQLHSTMT, col, SQL_C_BLOB_LOCATOR, buf, XML_MAX, &(ind));
        if( lobSeq.elementAtPosition(col) == 2 ) rc = SQLGetData(m_SQLHSTMT, col, SQL_C_CLOB_LOCATOR, buf, XML_MAX, &(ind));
        if( lobSeq.elementAtPosition(col) == 3 ) rc = SQLGetData(m_SQLHSTMT, col, SQL_C_DBCLOB_LOCATOR, buf, XML_MAX, &(ind));
    }
    else if (lobSeq.elementAtPosition(col) && m_iReadCLOBs)
    {
        SQLINTEGER lng = 0;
        char  *outBuffer = new char[1];
        rc = SQLGetData(m_SQLHSTMT, col, sqlColType, (SQLPOINTER) outBuffer, 0, &ind);
        delete[] outBuffer;
        deb("::retrieveBlob LNG: "+GString(ind));
        if( ind >= 0 )
        {
            ind += lobSeq.elementAtPosition(col) == 3 ? 2 : 0;
            outBuffer = new char[ind];
            deb("::retrieveBlob Calling again...");
            //rc = SQLGetData(m_SQLHSTMT, col, SQL_C_BINARY, outBuffer, ind, &lng);
            rc = SQLGetData(m_SQLHSTMT, col, sqlColType, outBuffer, ind, &lng);
            deb("::retrieveBlob LNG: "+GString(lng));
            if( lng > 0 )*out = GString(outBuffer, lng);
            delete [] outBuffer;
            free(buf);
            return 1;
        }
        else *out = "";
    }
    else
    {
        rc = SQLGetData(m_SQLHSTMT, col, SQL_C_CHAR, buf, XML_MAX, &(ind));
    }

    deb("::handleLobsAndXmls, have LOB/XML at col: "+GString(col)+", ind: "+GString(ind)+", len: "+GString(strLng)+", rc: "+GString(rc));
    if (ind == SQL_NULL_DATA )
    {
        *out = "NULL";
        *isNull = 1;
    }
    else if( lobSeq.elementAtPosition(col) == 1 ) *out = "@DSQL@BLOB";
    else if( lobSeq.elementAtPosition(col) == 2 ) *out = "@DSQL@CLOB";
    else if( lobSeq.elementAtPosition(col) == 3 ) *out = "@DSQL@DBCLOB";
    else
    {
        *out = "'"+GString(buf)+"'";
    }
    free(buf);
    deb("::handleLobsAndXmls, done.");

    return 1;
}

GString db2dcli::tabSchema(GString table)
{
    if( table.occurrencesOf(".") == 1 ) return table.subString(1, table.indexOf('.')-1).strip("\"");
    return "";
}
GString db2dcli::tabName(GString table)
{
    if( table.occurrencesOf(".") == 1 ) return table.subString(table.indexOf('.')+1, table.length()).strip().strip("\"");
    return "";
}
GString db2dcli::getDdlForView(GString tableName)
{
    GString cmd = "SELECT TEXT FROM SYSCAT.VIEWS WHERE VIEWSCHEMA='"+this->tabSchema(tableName)+"' AND ";
    cmd += "VIEWNAME='"+this->tabName(tableName)+"'";

    GString viewTxt;
    if( this->retrieveBlob( cmd, viewTxt, 0 ) ) return "";
    return viewTxt;
}

void db2dcli::setAutoCommmit(int commit)
{
    if( commit ) SQLSetConnectOption(m_SQLHDBC, SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_ON);
    else SQLSetConnectOption(m_SQLHDBC, SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF);
}

void db2dcli::currentConnectionValues(CON_SET * conSet)
{
    conSet->DB = m_strDB;
    conSet->Host = m_strHost;
    conSet->PWD = m_strPWD;
    conSet->UID = m_strUID;
    conSet->Port = m_iPort;
}

GString db2dcli::lastSqlSelectCommand()
{
    return m_strLastSqlSelectCommand;
}

TABLE_PROPS db2dcli::getTableProps(GString tableName)
{
    this->initAll("SELECT TYPE, BASE_TABSCHEMA, BASE_TABNAME FROM SYSCAT.TABLES WHERE TABSCHEMA='"+ tabSchema(tableName)+"' AND TABNAME='"+tabName(tableName)+"'");
    TABLE_PROPS tableProps;
    tableProps.init();
    if( rowElement(1,1).strip("'") == "A" ) tableProps.TableType = TYPE_ALIAS;
    else if( rowElement(1,1).strip("'") == "G" ) tableProps.TableType =  TYPE_CRT_TEMP_TABLE;
    else if( rowElement(1,1).strip("'") == "H" ) tableProps.TableType =  TYPE_HIR_TABLE;
    else if( rowElement(1,1).strip("'") == "L" ) tableProps.TableType =  TYPE_DET_TABLE;
    else if( rowElement(1,1).strip("'") == "N" ) tableProps.TableType =  TYPE_NICK;
    else if( rowElement(1,1).strip("'") == "S" ) tableProps.TableType =  TYPE_MAT_QUERY;
    else if( rowElement(1,1).strip("'") == "T" ) tableProps.TableType =  TYPE_UNTYPED_TABLE;
    else if( rowElement(1,1).strip("'") == "U" ) tableProps.TableType =  TYPE_TYPED_TABLE;
    else if( rowElement(1,1).strip("'") == "V" ) tableProps.TableType =  TYPE_UNTYPED_VIEW;
    else if( rowElement(1,1).strip("'") == "W" ) tableProps.TableType =  TYPE_TYPED_VIEW;

    if( tableProps.TableType == TYPE_ALIAS )
    {
        tableProps.BaseTabSchema = rowElement(1,2).strip("'").strip();
        tableProps.BaseTabName   = rowElement(1,3).strip("'").strip();
    }
    return tableProps;
}

int db2dcli::tableIsEmpty(GString tableName)
{
    GString err = this->initAll("SELECT * FROM "+tableName+" FETCH FIRST 1 ROWS ONLY");
    if( this->numberOfRows() == 0 || err.length() ) return 1;
    return 0;
}

int db2dcli::deleteViaFetch(GString tableName, GSeq<GString> * colSeq, int rowCount, GString whereClause)
{
    GString cmd, err;
    if( colSeq->numberOfElements() == 0 )
    {
        GString filter = "SELECT * FROM "+tableName;
        if( whereClause.length() ) filter += " WHERE "+whereClause;
        cmd = "DELETE FROM "+tableName;

        err = this->deleteByCursor(filter, cmd, rowCount, 0);
        this->commit();
        if( err.length() ) return sqlCode();
        return 0;
    }

    cmd = "DELETE FROM "+tableName+" WHERE (";
    for( int i = 1; i <= (int) colSeq->numberOfElements(); ++i )
    {
        cmd += colSeq->elementAtPosition(i) +",";
    }
    cmd.stripTrailing(",");
    cmd += ") IN ( SELECT ";
    for( int i = 1; i <= (int) colSeq->numberOfElements(); ++i )
    {
        cmd += colSeq->elementAtPosition(i) +",";
    }
    cmd.stripTrailing(",");    
    cmd += " FROM "+tableName+" ORDER BY "+colSeq->elementAtPosition(1)+" FETCH FIRST "+GString(rowCount)+" ROWS ONLY)";
    if( whereClause.length() ) cmd += " AND "+whereClause;

    err = this->initAll(cmd);    
    deb("deleteViaFetch: cmd: "+cmd+"\nErr: "+err);
    if( err.length() ) return sqlCode();    
    this->commit();
    return 0;
}


#ifndef NO_QT
GString db2dcli::initAll(GString message, QListView * pLV, unsigned long maxLines )
{
    deb("initAll: start. Msg: "+message);
    if( !iConnected )
    {
        sqlErrTxt = "@db2dcli: Not Connected.";
        return 1;
    }
    m_iNumberOfColumns = m_iNumberOfRows = 0;
    long erc;
    SQLRETURN       rc;

    QListViewItem * lvItem, *lastItem;

    clearSequences();


    rc = prepareSTMT(message); //ALLOCATES A STATEMENTHANDLE
    if( rc )
    {
       clearSequences();
       if( rc > 0 ) erc = rc;
       else erc = sqlCode();
       rc = SQLFreeStmt(m_SQLHSTMT, SQL_DROP);
       deb("initAll, Stmt freed due to error. rc: "+GString(erc));
       return erc;
    }
    GString         fldData;
    int             i;

    for (i = 1; i <= m_iNumberOfColumns; i++)
    {
       pLV->addColumn(hostVariable(i));	 
    }


    deb("initAll: Starting while....");
    m_iNumberOfRows = 0;

    while (SQLFetch(hstmt) == 0)
    {

       if( lastItem != NULL ) lvItem = new QListViewItem(pLV, lastItem);
       else lvItem = new QListViewItem(pLV);
       lastItem = lvItem;


       lvItemSeq.add(lvItem);

       for (i = 0; i < m_iNumberOfColumns; i++) {
          if (outlen[i] == SQL_NULL_DATA) fldData = "NULL";
          else
          { 
              if (outlen[i] >= (signed) collen[i]) {
                  fldData = GString(data[i]).subString(1, collen[i]);
              }
              else fldData = GString(data[i]);
              fldData = setFldData(fldData, sqlTypeSeq.elementAtPosition(i+1));
              lvItem->setText(i, fldData);
          }
          deb("FldData: "+fldData+", Type: "+GString(sqlTypeSeq.elementAtPosition(i+1)));
       }
       lvItemSeq.add(lvItem);
       m_iNumberOfRows++;
       deb("Lines stored: "+GString(m_iNumberOfRows));
    }
    rc = SQLFreeStmt(hstmt, SQL_DROP);
    deb("rc FreeStmt: "+GString(erc));
    deb("initAll Done.");

   return "";
}

GString db2dcli::getIdenticals(GString table, QListBox * aLB, QLabel * info, short autoDel )
{
    return "";
}


signed long db2dcli::countLines(GString message)
{

   return 0;
}

#endif



