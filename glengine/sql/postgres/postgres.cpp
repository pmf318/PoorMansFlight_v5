    //
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt
//

#ifndef _POSTGRES_
#include <postgres.hpp>
#endif

#include <QVariant>
#include <QStringList>
#include <QDebug>
#include <QUuid>
#include <QMessageBox>

#include <idsql.hpp>
#include <gdebug.hpp>
#include <gfile.hpp>
#include <gstuff.hpp>


#ifdef MAKE_VC
/////#include <windows.h>
#endif

#ifndef max
  #define max(A,B) ((A) >(B) ? (A):(B))
#endif

///////////////////////////////////////////
//Global static instance counter.
//Each instance saves its private instance value in m_iMyInstance.
static int m_postgresobjCounter = 0;

#define XML_MAX 250



/***********************************************************************
 * This class can either be instatiated or loaded via dlopen/loadlibrary
 ***********************************************************************/

//Define functions with C symbols (create/destroy instance).
#ifndef MAKE_VC
extern "C" postgres* create()
{
    //printf("Calling postgres creat()\n");
    return new postgres();
}
extern "C" void destroy(postgres* pODBCSQL)
{
   if( pODBCSQL ) delete pODBCSQL ;
}
#else
extern "C" __declspec( dllexport ) postgres* create()
{
    //printf("Calling postgres creat()\n");
    _flushall();
    return new postgres();
}	
extern "C" __declspec( dllexport ) void destroy(postgres* pODBCSQL)
{
    if( pODBCSQL ) delete pODBCSQL ;
}	
#endif
/***********************************************************
 * CLASS
 **********************************************************/

postgres::postgres(postgres  const &o)
{
	m_iLastSqlCode = 0;
    m_postgresobjCounter++;
    m_iTruncationLimit = 500;
    m_iMyInstance = m_postgresobjCounter;
    m_pGDB = o.m_pGDB;
    deb("CopyCtor start");

    m_odbcDB  = POSTGRES;
    m_iReadUncommitted = o.m_iReadUncommitted;
    m_strCurrentDatabase = o.m_strCurrentDatabase;
    m_iIsTransaction = o.m_iIsTransaction;
    m_strDB = o.m_strDB;
    m_strHost = o.m_strHost;
    m_strPWD = o.m_strPWD;
    m_strUID = o.m_strUID;
    m_iPort  = o.m_iPort;
    deb("CopyCtor, orgPort: "+GString(o.m_iPort)+", new: "+GString(m_iPort));
    m_iReadClobData = o.m_iReadClobData;
    m_strCurrentDatabase = o.m_strCurrentDatabase;
    m_strLastSqlSelectCommand = o.m_strLastSqlSelectCommand;

    GString db, uid, pwd;
    o.getConnData(&db, &uid, &pwd);

    //m_pgConn = o.m_pgConn;
    deb("CopyCtor: calling connect, connection data: "+m_strDB+", uid: "+m_strUID+", host: "+m_strHost+", port: "+GString(m_iPort));
    this->connect(m_strDB, m_strUID, m_strPWD, m_strHost, GString(m_iPort));
    if( m_strCurrentDatabase.length() ) this->initAll("USE "+m_strCurrentDatabase);
    //!!TODO
//    if( m_iReadUncommitted ) readRowData("set transaction isolation level read uncommitted");
//    else readRowData("set transaction isolation level REPEATABLE READ");
    deb("Copy CTor done");
}

postgres::postgres()
{
    m_postgresobjCounter++;
    //m_IDSQCounter++;
    m_iTruncationLimit = 500;
    m_iMyInstance = m_postgresobjCounter;

	m_iLastSqlCode = 0;
    m_iIsTransaction = 0;	
    m_odbcDB  = POSTGRES;

    m_strDB ="";
    m_strUID ="";
    m_strPWD = "";
    m_strHost = "localhost";
    m_iPort = 5432;
    m_pGDB = NULL;
    m_strLastSqlSelectCommand = "";
    m_iReadUncommitted = 0;
    m_iReadClobData = 0;
    m_strCurrentDatabase = "";
    m_pgConn = NULL;    
    printf("postgres[%i]> DefaultCtor done. Port: %i\n", m_iMyInstance, m_iPort );
}
postgres* postgres::clone() const
{
    return new postgres(*this);
}

postgres::~postgres()
{
    deb("Dtor start, current count: "+GString(m_postgresobjCounter));
    disconnect();

    m_postgresobjCounter--;
    //m_IDSQCounter--;
    deb("Dtor, clearing RowData...");
    clearSequences();
    deb("Dtor done, current count: "+GString(m_postgresobjCounter));
}

int postgres::getConnData(GString * db, GString *uid, GString *pwd) const
{
    *db = m_strDB;
    *uid = m_strUID;
    *pwd = m_strPWD;
	return 0;
}


GString postgres::connect(GString db, GString uid, GString pwd, GString host, GString port)
{    
    deb("::connect, connect to DB: "+db+", uid: "+uid+", host: "+host+", port: "+port);

    if( !port.length() || port == "0") port = "5432";
    m_strDB = db;
    m_strUID = uid;
    m_strPWD = pwd;
    m_strNode = host;
    m_strHost = host;
    m_iPort = port.asInt();
    m_strCurrentDatabase = db;
    deb("::connect, connection data: "+m_strDB+", uid: "+m_strUID+", host: "+host+", port: "+GString(port));

    m_pgConn = PQsetdbLogin(host, port, "", "", db, uid, pwd);
    if (PQstatus(m_pgConn) != CONNECTION_OK)
    {
        return sqlError();
    }
    return "";
}

int postgres::disconnect()
{	

    deb("Disconnecting and closing...");
    if( !m_pgConn ) return 0;
    deb("Disconnecting and closing...calling PQfinish...");
    PQfinish(m_pgConn);
    m_pgConn = NULL;
    deb("Disconnecting and closing...done.");
	return 0;
}
GString postgres::initAll(GString message, unsigned long maxRows,  int getLen)
{    

    deb("::initAll, start. msg: "+message);

    GRowHdl * pRow;
    PGresult   *res;

    resetAllSeq();

    m_strLastSqlSelectCommand = message;
    m_iNumberOfRows = 0;
    m_iLastSqlCode = 0;


    res = PQexec(m_pgConn, message);
    if (PQresultStatus(res) == PGRES_COMMAND_OK)
    {
        return "";
    }
    else
    {
        sqlError();
        if(m_strLastError.length()) return m_strLastError;
    }



    res = PQexec(m_pgConn, "BEGIN");
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        return sqlError();
    }

    PQclear(res);

    message = "DECLARE myportal CURSOR FOR "+message;
    res = PQexec(m_pgConn, message);
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        sqlError();
        res = PQexec(m_pgConn, "END");
        PQclear(res);
        return m_strLastError;
    }
    PQclear(res);

    res = PQexec(m_pgConn, "FETCH ALL in myportal");
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        return sqlError();
    }
    deb("cols: "+GString(m_iNumberOfColumns));
    m_iNumberOfColumns = PQnfields(res);
    for (int i = 0; i < m_iNumberOfColumns; i++)
    {
        hostVarSeq.add(PQfname(res, i));
        sqlTypeSeq.add(PQftype(res, i));
        if( PQftype(res, i) == 142 ) xmlSeq.add(1);
        else xmlSeq.add(0);
        deb("HostVars: col "+GString(i)+", name: "+GString(PQfname(res, i))+", type: "+GString(PQftype(res, i)));
    }
    printf("\n\n");

    for (int i = 0; i < PQntuples(res); i++)
    {
        pRow = new GRowHdl;
        for (int j = 0; j < m_iNumberOfColumns; j++)
        {
            if( PQgetisnull(res, i, j) )pRow->addElement("NULL");
            else if( isLOBCol(j+1) ) pRow->addElement("@DSQL@BLOB");
            else if( isNumType(j+1) ) pRow->addElement(PQgetvalue(res, i, j));
            else pRow->addElement("'"+GString(PQgetvalue(res, i, j))+"'");
        }
        m_iNumberOfRows++;
        if( maxRows > 0 && m_iNumberOfRows > maxRows ) break;
        allRowsSeq.add( pRow );
    }

    PQclear(res);
    res = PQexec(m_pgConn, "CLOSE myportal");
    PQclear(res);

    res = PQexec(m_pgConn, "END");
    PQclear(res);

    return sqlError();
}

//void postgres::setSimpleColType(enum_field_types type)
//{
//    switch(type)
//    {
//    case MYSQL_TYPE_SHORT:
//    case MYSQL_TYPE_INT24:
//        simpleColTypeSeq.add(CT_INTEGER);
//        break;

//    case MYSQL_TYPE_LONG:
//    case MYSQL_TYPE_DOUBLE:
//    case MYSQL_TYPE_LONGLONG:
//        simpleColTypeSeq.add(CT_LONG);
//        break;

//    case MYSQL_TYPE_NEWDECIMAL:
//        simpleColTypeSeq.add(CT_DECIMAL);
//        break;


//    case MYSQL_TYPE_FLOAT:
//        simpleColTypeSeq.add(CT_FLOAT);
//        break;

//    case MYSQL_TYPE_TIMESTAMP:
//    case MYSQL_TYPE_DATE:
//    case MYSQL_TYPE_TIME:
//    case MYSQL_TYPE_DATETIME:
//    case MYSQL_TYPE_YEAR:
//    case MYSQL_TYPE_NEWDATE:
//    case MYSQL_TYPE_TIMESTAMP2:
//    case MYSQL_TYPE_DATETIME2:
//    case MYSQL_TYPE_TIME2:
//        simpleColTypeSeq.add(CT_DATE);
//        break;

//    case MYSQL_TYPE_NULL:
//    case MYSQL_TYPE_VARCHAR:
//    case MYSQL_TYPE_VAR_STRING:
//    case MYSQL_TYPE_STRING:
//    case MYSQL_TYPE_BIT:
//        simpleColTypeSeq.add(CT_STRING);
//        break;

//    case MYSQL_TYPE_ENUM:
//    case MYSQL_TYPE_SET:

//    case MYSQL_TYPE_TINY_BLOB:
//    case MYSQL_TYPE_MEDIUM_BLOB:
//    case MYSQL_TYPE_LONG_BLOB:
//    case MYSQL_TYPE_BLOB:
//    case MYSQL_TYPE_GEOMETRY:
//        simpleColTypeSeq.add(CT_CLOB);
//        break;

//    default:
//        simpleColTypeSeq.add(CT_STRING);
//        break;
//    }
//}

/*****************************************************
 * 1. getFullXML is currently unused and anyway will only be useful w/ TDS 7.1/8.0
 *
 * 2. TDS7.2: len will be returned as -4
 *
 * So at the moment (using TDS 7.2) we return the buf although len is -4
 *
*****************************************************/

int postgres::handleLobsAndXmls(int col, GString * out, int getFullXML, short* isNull)
{
    PMF_UNUSED(getFullXML);

    deb("::handleLobsAndXmls, col: "+GString(col)+", lob: "+GString(lobSeq.elementAtPosition(col))+", xml: "+GString(xmlSeq.elementAtPosition(col)));

    *out = "";
    *isNull = 0;

    if( isLOBCol(col) == 0 && isXMLCol(col) == 0 ) return 0;
    return 1;
}

int postgres::commit()
{
    //m_db.commit();
	return 0;
}

int postgres::rollback()
{
    //m_db.rollback();
	return 0;
}


int postgres::initRowCrs()
{
	deb("::initRowCrs");
    if( allRowsSeq.numberOfElements() == 0 ) return 1;
    m_pRowAtCrs = allRowsSeq.initCrs();
    return 0;
}
int postgres::nextRowCrs()
{	
    if( m_pRowAtCrs == NULL ) return 1;
    m_pRowAtCrs = allRowsSeq.setCrsToNext();
    return 0;
}
long  postgres::dataLen(const short & pos)
{
    if( pos < 1 || pos > (short) sqlLenSeq.numberOfElements()) return 0;
    deb("Len at "+GString(pos)+": "+GString(sqlLenSeq.elementAtPosition(pos)));    
    return sqlLenSeq.elementAtPosition(pos);
}

int postgres::isDateTime(int pos)
{
    if( pos < 1 || pos > (int)sqlTypeSeq.numberOfElements() ) return 0;
    switch( sqlTypeSeq.elementAtPosition(pos) )
    {
    case 1114:
    case 1184:
            return 1;
    }
    return 0;
}

int postgres::isNumType(int pos)
{
    if( pos < 1 || pos > (int) sqlTypeSeq.numberOfElements() ) return 0;
    switch( sqlTypeSeq.elementAtPosition(pos) )
    {
    case 21:
    case 23:

            return 1;
    }
    return 0;
}

int postgres::isXMLCol(int i)
{
    if( i < 1 || (unsigned long)i > xmlSeq.numberOfElements() ) return 0;
    return xmlSeq.elementAtPosition(i);
}


int postgres::hasForBitData()
{
    return 0;
}
int postgres::isForBitCol(int i)
{
    if( i < 1 || (unsigned long)i > sqlForBitSeq.numberOfElements() ) return 0;
    return sqlForBitSeq.elementAtPosition(i);
}
int postgres::isBitCol(int i)
{
    if( i < 1 || (unsigned long)i > sqlBitSeq.numberOfElements() ) return 0;
    return sqlBitSeq.elementAtPosition(i);
}

unsigned int postgres::numberOfColumns()
{
    //deb("::numberOfColumns called");
    return m_iNumberOfColumns;
}

unsigned long postgres::numberOfRows()
{
    return m_iNumberOfRows;
}
GString postgres::dataAtCrs(int col)
{
    if( m_pRowAtCrs == NULL ) return "@CrsNotOpen";
    if( col < 1 || (unsigned long)col > m_pRowAtCrs->elements() ) return "@OutOfReach";
    return m_pRowAtCrs->rowElementData(col);
}
GString postgres::rowElement( unsigned long row, int col)
{
    //Overload 1
    //tm("::rowElement for "+GString(line)+", col: "+GString(col));
    if( row < 1 || row > allRowsSeq.numberOfElements() ) return "@OutOfReach";
    GRowHdl *aRow;
    aRow = allRowsSeq.elementAtPosition(row);
    if( row < 1 || (unsigned long) col > aRow->elements() ) return "OutOfReach";
    return aRow->rowElementData(col);
}
int postgres::positionOfHostVar(const GString& hostVar)
{
    unsigned long i;
    for( i = 1; i <= hostVarSeq.numberOfElements(); ++i )
    {
        if( hostVarSeq.elementAtPosition(i) == hostVar ) return i;
    }
    return 0;
}
GString postgres::rowElement( unsigned long row, GString hostVar)
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


GString postgres::hostVariable(int col)
{
    //deb("::hostVariable called, col: "+GString(col));
    if( col < 1 || col > (short) hostVarSeq.numberOfElements() ) return "@hostVariable:OutOfReach";
    return hostVarSeq.elementAtPosition(col);
}



GString postgres::currentCursor(GString filter, GString command, long curPos, short commitIt, GSeq <GString> *fileList, GSeq <long> *lobType)
{
    deb(__FUNCTION__, "filter: "+filter+", cmd: "+command);
    int rc;


    if( curPos < 0 ) return "CurrentCursor(): POS < 0";
    deb(__FUNCTION__, "done, rc: "+GString(rc));
    return rc ? m_strLastError : "";
}



int postgres::sqlCode()
{
	return m_iLastSqlCode;
}
GString postgres::sqlError()
{    
    m_iLastSqlCode = 0;
    deb("::sqlError: m_iLastSqlCode = "+GString(m_iLastSqlCode));
    m_strLastError = PQerrorMessage(m_pgConn);
    if( m_strLastError.length() )
    {
        m_iLastSqlCode = -1;
//        if( m_pgConn) PQfinish(m_pgConn);
//        m_pgConn = NULL;
    }
    printf("--------------------------------\nError: %s\nLastCmd: %s\n------------------\n", (char*) m_strLastError, (char*) m_strLastSqlSelectCommand);
    return m_strLastError;
}		

int postgres::getTabSchema()
{
    GString cmd = "SELECT distinct table_schema from information_schema.tables where table_catalog='"+m_strCurrentDatabase+"'";
    this->initAll(cmd);
    return 0;
}
int postgres::getTables(GString schema)
{
    GString cmd = "SELECT table_name FROM information_schema.tables where table_schema='"+schema+"' and table_catalog='"+m_strCurrentDatabase+"' order by table_name";
    this->initAll(cmd);
    return 0;
}
void postgres::convToSQL( GString& input )
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
   }
   output = "'"+output+"'";
   input = output;
   delete [] in;

}

int postgres::getAllTables(GSeq <GString > * tabSeq, GString filter)
{
    PMF_UNUSED(tabSeq);
    PMF_UNUSED(filter);
    /*
	QStringList sl = m_db.tables(QSql::AllTables);
	tabSeq->removeAll();
	for (int i = 0; i < sl.size(); ++i) tabSeq->add(sl.at(i));
    */
	return 0;
}
short postgres::sqlType(const short & col)
{
   if( col < 1 || col > (short) sqlTypeSeq.numberOfElements() ) return -1;
   else return sqlTypeSeq.elementAtPosition(col);
}
short postgres::sqlType(const GString & colName)
{
   for( short i = 1; i <= (short) hostVarSeq.numberOfElements(); ++ i )
   {
      if( hostVarSeq.elementAtPosition(i) == colName ) return sqlTypeSeq.elementAtPosition(i);
   }
   return -1;
}
GString postgres::realName(const short & sqlType)
{
	switch(sqlType)
	{
            return "Numeric";
			
            return "Time";
			
		default:
			return "'string'";
	}
	return "";
}

int postgres::loadFileIntoBuf(GString fileName, char** fileBuf, int *size)
{
    FILE * f;
    //Apparently CR/LF is not counted in fread.
    //Even XML sources should be opened with "rb".
    f = fopen(fileName, "rb");
    if( f != NULL )
    {
        deb("::loadFileIntoBuf reading file....");
        fseek(f, 0, SEEK_END);
        *size = ftell(f);
        fseek(f, 0, SEEK_SET);        
        *fileBuf = new char[(*size)+1];
        int res = fread(*fileBuf,sizeof(char),*size, f);
        fclose(f);
        std::string enc = GStuff::base64_encode((unsigned char*) *fileBuf, *size);

        delete [] *fileBuf;
        *fileBuf = new char[enc.size()+1];
        memset(*fileBuf, '0', enc.size());
        memcpy(*fileBuf, GString(enc), enc.size());
        *size = enc.size();

        deb("::loadFileIntoBuf reading file...OK, size: "+GString(*size)+", bytesRead: "+GString(res));

        return 0;
    }
    return 1;
}


void postgres::impExpLob()
{
    Oid         lobjOid;
     PGresult *res;

    res = PQexec(m_pgConn, "begin");
    PQclear(res);

    lobjOid = lo_import(m_pgConn, "/home/moi/tok.txt");

    lo_export(m_pgConn, lobjOid, "/home/moi/tok3.txt");

    res = PQexec(m_pgConn, "end");
    PQclear(res);
}


long postgres::uploadBlob(GString cmd, GSeq <GString> *fileSeq, GSeq <long> *lobType)
{
    deb("::uploadBlob, cmd: "+cmd);

    cmd = cmd.change("?", "$1::bytea");
    const char   *const *fileBuf = new char* [fileSeq->numberOfElements()];
    int  *const fileSize = new int[fileSeq->numberOfElements()];

    int rc = 0;
    for( int i = 0; i < fileSeq->numberOfElements(); ++i )
    {
        rc += loadFileIntoBuf(fileSeq->elementAtPosition(i+1),  (char**) &(fileBuf[i]), &(fileSize[i]));
    }
    if( rc ) return 1;

    PGresult *res = PQexecParams(m_pgConn, cmd, fileSeq->numberOfElements(), NULL, (fileBuf), fileSize, NULL, 0);
    delete [] fileBuf;
    delete [] fileSize;

    if (PQresultStatus(res) != PGRES_COMMAND_OK) return -1;
    return 0;
}

GString postgres::descriptorToFile( GString cmd, GString &blobFile, int * outSize )
{
    deb("::descriptorToFile, cmd: "+cmd);
    *outSize = 0;

    postgres ps(*this);
    GSeq<GString> cmdSeq = cmd.split(' ');
    if( cmdSeq.numberOfElements() < 2) return "Invalid sqlCmd: "+cmd;

    GString colName = cmdSeq.elementAtPosition(2);
    cmd = cmd.change(colName, "encode("+colName+",'escape')");
    GString err = ps.initAll(cmd);
    if( err.length() ) return err;

    GString raw = ps.rowElement(1,1).strip("'");
    std::string ret = GStuff::base64_decode((char*) raw);

    FILE * f;
    f = fopen(blobFile, "ab");
    *outSize = fwrite((char*) ret.c_str(),1,ret.size(),f);
    fclose(f);
    return sqlError();

}

int postgres::writeToFile(GString fileName, GString data, int len)
{
    FILE * f;
    f = fopen(fileName, "ab");
    int written = fwrite((char*) data,1,len,f);
    fclose(f);
    return written;
}

signed long postgres::getCost()
{
    return -1;
}

int postgres::simpleColType(int i)
{
    if( i < 1 || (unsigned long)i > simpleColTypeSeq.numberOfElements() ) return CT_UNKNOWN;

    if(isXMLCol(i)) return CT_XML;
    if(isLOBCol(i)) return CT_DBCLOB;
    if(isNumType(i)) return CT_INTEGER;
    if(isDateTime(i)) return CT_DATE;
    return CT_STRING;
}

int postgres::isLOBCol(int i)
{
    if( i < 1 || (unsigned long)i > sqlTypeSeq.numberOfElements() ) return 0;
    switch(sqlTypeSeq.elementAtPosition(i))
    {
    case 17:
        return 1;

    }
    return 0;
}

int postgres::isNullable(int i)
{
    if( i < 1 || (unsigned long)i > sqlIndVarSeq.numberOfElements() ) return 1;
    return sqlIndVarSeq.elementAtPosition(i);
}


int postgres::isFixedChar(int i)
{
    if( i < 1 || (unsigned long)i > sqlTypeSeq.numberOfElements() ) return 0;
    return 0;
}

int postgres::getDataBases(GSeq <CON_SET*> *dbList)
{
    return 0;
    CON_SET * pCS;
    this->initAll("select table_schema as database_name, table_name from information_schema.tables where table_type = 'BASE TABLE' order by database_name, table_name;");
	deb("getDataBases done");
    return 0;
}
void postgres::setCurrentDatabase(GString db)
{
    m_strCurrentDatabase = db;
    this->initAll("use "+m_strCurrentDatabase);
    m_strCurrentDatabase = db;
}
GString postgres::currentDatabase()
{
    return m_strCurrentDatabase;
}
unsigned long postgres::getResultCount(GString cmd)
{
   cmd = cmd.upperCase();
   if( cmd.occurrencesOf("SELECT") == 0 ) return 0;
   if( cmd.occurrencesOf("FROM") == 0 ) return 0;
   GString tmp = cmd.subString(1, cmd.indexOf("SELECT")+6);
   tmp += " COUNT (*) ";
   tmp += cmd.subString(cmd.indexOf("FROM"), cmd.length()).strip();

   this->initAll(tmp);
   return this->rowElement(1,1).asLong();
}

/*****************************************************************************
*
* PRIVATE METHOD
*
*****************************************************************************/
void postgres::resetAllSeq()
{
    GRowHdl * aLine;
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
      aLine = allRowsSeq.firstElement();
      //aLine->deleteLine();
      delete aLine;
      allRowsSeq.removeFirst();
    }
}
int postgres::getColSpecs(GString table, GSeq<COL_SPEC*> *specSeq)
{

    GString tabSchema = this->tabSchema(table);
    GString tabName   = this->tabName(table);
    COL_SPEC *cSpec;

    this->setCLOBReader(1); //This will read any CLOB into fieldData

    GString cmd = "SELECT column_name, data_type, character_maximum_length, numeric_precision, is_nullable, column_default FROM information_schema.COLUMNS WHERE TABLE_SCHEMA='"+tabSchema+"' AND table_name='"+tabName+"'";

    deb("::getColSpecs: "+cmd);

    this->initAll(cmd);




    for( unsigned i=1; i<=this->numberOfRows(); ++i )
    {
        cSpec = new COL_SPEC;
        cSpec->init();

        cSpec->ColName = this->rowElement(i,1).strip("'");

        /* colType CHARACTER (from syscat.tables) needs to be truncated to CHAR */
        cSpec->ColType = this->rowElement(i,2).strip("'");
        if( cSpec->ColType == "CHARACTER" ) cSpec->ColType = "CHAR";


        /* SMALLINT, INTEGER, BIGINT, DOUBLE */
        if( cSpec->ColType.occurrencesOf("int") > 0 ||
                cSpec->ColType.occurrencesOf("double") > 0 ||
                cSpec->ColType.occurrencesOf("float") > 0 ||
                cSpec->ColType.occurrencesOf("date") > 0 ||
                cSpec->ColType.occurrencesOf("time"))
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
            cSpec->Length = this->rowElement(i,3).strip("'");
        }
        if( (cSpec->ColType.occurrencesOf("CHAR")) && this->rowElement(i, 11) == "0" ) //codepage is 0 for CHAR FOR BIT
        {
            cSpec->Misc = "FOR BIT DATA";
        }


        /* check for NOT NULL, DEFAULT */
        if( this->rowElement(i,5) == "'NO'" )
        {
            cSpec->Nullable = "NOT NULL";
        }
        if( !this->isNull(i, 6) )
        {
            //cSpec->Default = cleanString(this->rowElement(i,6));
            cSpec->Default = this->rowElement(i,6);
        }
        if( this->rowElement(i,12).asInt() > 0 )
        {
            //cSpec->Misc = " INLINE LENGTH "+this->rowElement(i,12)+" ";
        }
        if( this->rowElement(i,7) == "'N'" )
        {
            cSpec->Misc = "NOT LOGGED";
        }
        if( this->rowElement(i,7) == "'Y'" )
        {
            cSpec->Misc += "LOGGED";
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

    this->setCLOBReader(0);
    return 0;
}

int postgres::getIdentityColParams(GString table, int *seed, int * incr)
{
    postgres * tmp = new postgres(*this);
    tmp->initAll("select IDENT_SEED ( '"+table+"' )");
    *seed = tmp->rowElement(1,1).asInt();

    tmp->initAll("select IDENT_INCR ( '"+table+"' )");
    *incr = tmp->rowElement(1,1).asInt();

    delete tmp  ;
	return 0;
}


int postgres::getTriggers(GString table, GString *text)
{
    *text = "";
    GSeq <GString> trgSeq = getTriggerSeq(table);
    for(int i = 1; i <= trgSeq.numberOfElements(); ++i)
    {
        *text += "\n-- Trigger #"+GString(i)+":\n"+trgSeq.elementAtPosition(i);
    }
    return 0;
}

GSeq <GString> postgres::getTriggerSeq(GString table)
{
    deb("getTriggerSeq start");
    GString tabSchema = this->tabSchema(table);
    GString tabName   = this->tabName(table);
    GString trig;

    GSeq <GString> triggerSeq;

    this->setCLOBReader(1); //This will read any CLOB into fieldData

    GString cmd = "select trigger_schema, trigger_name, action_timing, event_manipulation, action_statement "
            "from information_schema.triggers where event_object_schema='"+tabSchema+"' and event_object_table='"+tabName+"' order by action_order";

    this->initAll(cmd);

    deb("getTriggers, cmd: "+cmd);
    for( unsigned long i = 1; i <= this->numberOfRows(); ++i )
    {
        trig = "CREATE TRIGGER "+this->rowElement(i, 1).strip("'")+"."+this->rowElement(i, 1).strip("'")+" "+this->rowElement(i, 3).strip("'")+" ";
        trig += this->rowElement(i, 4).strip("'")+" ON "+table+" "+this->rowElement(i, 5).strip("'");
        triggerSeq.add(trig);
    }
    this->setCLOBReader(0);
    return triggerSeq;
}


GSeq <IDX_INFO*> postgres::getIndexeInfo(GString table)
{                

//    this->initAll("use "+m_strCurrentDatabase);

    GString cmd = "select schemaname, indexname, indexdef from pg_catalog.pg_indexes "
     "where schemaname ='"+tabSchema(table)+"' and  tablename ='"+tabName(table)+"' ";

    m_iReadClobData = 1;
    this->initAll(cmd);
    deb("::getIndexeInfo, cmd: "+cmd);
    deb("::getIndexeInfo, found: "+GString(this->numberOfRows()));

    GSeq <IDX_INFO*> indexSeq;

    IDX_INFO * pIdx;
    for(int  i=1; i<=(int)this->numberOfRows(); ++i )
    {
        pIdx = new IDX_INFO;        
        deb("::getIndexeInfo, i: "+GString(i));
        GString cols = this->rowElement(i, 3).strip("'");
        cols = cols.subString(cols.indexOf("(")+1, cols.length()).strip();
        cols = cols.subString(1, cols.indexOf(")")-1);


        pIdx->Iidx = GString(i);
        pIdx->Schema = this->rowElement(i, 1).strip("'");
        pIdx->Name = this->rowElement(i, 2).strip("'");
        pIdx->Columns = cols;
        pIdx->CreateTime = "N/A";
        pIdx->StatsTime = "N/A";
        pIdx->IsDisabled = "N/A";
        pIdx->DeleteRule = "N/A";
        pIdx->StatsTime = "N/A";
        pIdx->Stmt = this->rowElement(i, 3).strip("'");

        indexSeq.add(pIdx);
        deb("::fillIndexView, i: "+GString(i)+", currIDx: "+GString(i));
    }
    m_iReadClobData = 0;

    return indexSeq;
}


GString postgres::getChecks(GString, GString )
{
    return "";
}

GSeq <GString> postgres::getChecksSeq(GString, GString)
{
    GSeq <GString> aSeq;
    return aSeq;
}



GString postgres::getIndexStatement(int type, GString table, GString indexName)
{
    GString indCmd, unq, incStatement, order;
    int indexID = -1;
    indexName = indexName.strip("'");
    GString cmd = "select i.name, c.name, i.index_id, i.is_primary_key, i.is_unique,"
            "i.is_disabled , i.is_hypothetical, ic.key_ordinal, ic.is_included_column, ic.is_descending_key "
            "from sys.tables t  inner join sys.schemas s on t.schema_id = s.schema_id "
            "inner join sys.indexes i on i.object_id = t.object_id "
            "inner join sys.index_columns ic on ic.object_id = t.object_id and i.index_id = ic.index_id "
            "inner join sys.columns c on c.object_id = t.object_id and ic.column_id = c.column_id "
            "where s.name='"+tabSchema(table)+"' and t.name='"+tabName(table)+"' "
            "and i.is_primary_key = '"+GString(type)+"' and i.name='"+indexName+"' order by i.index_id";

    deb("::getIndexStatement, cmd: "+cmd);

    this->initAll(cmd);
    deb("::getIndexes, found: "+GString(this->numberOfRows()));
    if( incStatement.length() ) incStatement= " INCLUDE ( "+incStatement.stripTrailing(",")+")";
    if( indexID >= 0 ) indCmd = indCmd.stripTrailing(",")+")"+incStatement+";";

    incStatement = "";
    indCmd = "";
    if( this->rowElement(1, 5).strip("'").asInt() == 1 ) unq = " UNIQUE ";
    else unq = "";

    indexID = this->rowElement(1, 3).strip("'").asInt();
    deb("::getIndexStatement, newInd: "+GString(indexID));

    //Get PrimaryKeys only
    if( type == 1 && this->rowElement(1, 4).strip("'").asInt() == 1 )
    {
        indCmd += "ALTER TABLE "+table+" ADD CONSTRAINT "+this->rowElement(1, 1)+" PRIMARY KEY(";
    }
    else if(type == 0 && this->rowElement(1, 4).strip("'").asInt() == 0) //Ordinary index
    {
        indCmd += "CREATE "+unq+" INDEX "+this->rowElement(1, 1)+" ON "+table + "(";
    }
    if( this->rowElement(1, 10).asInt() == 1 ) order = " DESC";
    else order = " ASC";
    if(this->rowElement(1, 9).asInt() == 1 ) incStatement +=  this->rowElement(1, 2).strip("'")+",";
    else indCmd += this->rowElement(1, 2).strip("'")+order+",";
    deb("::getIndexStatement, indCmd(1): "+indCmd+", inc: "+incStatement+", col9: "+this->rowElement(1, 9));

    if( incStatement.length() ) incStatement= " INCLUDE ( "+incStatement.stripTrailing(",")+")";
    if( indexID >= 0 ) indCmd = indCmd.stripTrailing(",")+")"+incStatement;


    deb("::getIndexStatement, indCmd(Fin): "+indCmd);

    return indCmd + ";";

}

GString postgres::getForeignKeyStatement(GString table, GString foreignKeyName )
{
    //this->initAll("use "+m_strCurrentDatabase);
    foreignKeyName = foreignKeyName.strip("'");

    GString out;
    GString cmd =
    "SELECT OBJECT_NAME(f.parent_object_id) t1, "               //TableName,
    " COL_NAME(fc.parent_object_id,fc.parent_column_id) t2, "   //ColName
    " OBJECT_NAME(f.referenced_object_id) t3, "                 //referenced table
    " OBJECT_NAME(f.object_id) t4, "                            //FKey name
    " COL_NAME(fc.referenced_object_id,fc.referenced_column_id) t5," //referenced col
    " (f.delete_referential_action) t6,"                          //on delete
    " (f.delete_referential_action_desc) t7,"                    //"CASCADE"
    " (f.update_referential_action) t8,"
    " (f.update_referential_action_desc) t9	"
    " FROM sys.foreign_keys AS f INNER JOIN sys.foreign_key_columns AS fc "
          " ON f.OBJECT_ID = fc.constraint_object_id INNER JOIN "
          " sys.tables t  ON t.OBJECT_ID = fc.referenced_object_id "
          " WHERE    OBJECT_NAME (f.parent_object_id) = '"+tabName(table)+"' AND OBJECT_NAME(f.object_id)='"+foreignKeyName+"'";

    GString keyName, cols, refCols;
    deb("::getForeignKeys, cmd: "+cmd);
    this->initAll(cmd);
    out = "";
    if( keyName == this->rowElement(1, 4).strip("'") || keyName == "")
    {
        cols += this->rowElement(1, 2).strip("'") +", ";
        refCols += this->rowElement(1, 5).strip("'") +", ";
        keyName = this->rowElement(1, 4).strip("'");
    }
    if( keyName != this->rowElement(1, 4).strip("'") || 1 == this->numberOfRows() )
    {
        out += "ALTER TABLE "+table+" ADD CONSTRAINT "+this->rowElement(1, 4).strip("'")+" FOREIGN KEY ("+cols.stripTrailing(", ")+") REFERENCES "+
                this->rowElement(1, 3).strip("'")+"("+refCols.stripTrailing(", ")+")";
        if( this->rowElement(1, "t6").strip("'").asInt() == 1 ) out += " ON DELETE "+this->rowElement(1, "t7").strip("'");
        if( this->rowElement(1, "t8").strip("'").asInt() == 1 ) out += " ON UPDATE "+this->rowElement(1, "t9").strip("'");
        keyName = "";
    }
    return out.removeButOne() + ";";
}





int postgres::hasUniqueConstraint(GString tableName)
{

    GSeq <GString>  colSeq;
    if( this->getUniqueCols(tableName, &colSeq)) return 0;
    return colSeq.numberOfElements();

    /////////////////////////////////////////////////////////////////////
    // DEAD CODE AHEAD
    /////////////////////////////////////////////////////////////////////
    GString cmd = "select count(*) from INFORMATION_SCHEMA.CONSTRAINT_COLUMN_USAGE ";
            cmd += "where table_catalog='"+context(tableName)+"' and TABLE_SCHEMA='"+tabSchema(tableName)+"' and ";
            cmd += "TABLE_NAME='"+tabName(tableName)+"'";

    this->initAll(cmd);
    deb("hasUniqueConstraint, first check: "+this->rowElement(1,1));
    if(this->rowElement(1,1).asInt() > 0 ) return 1;

    cmd = "SELECT count(*) FROM sys.[tables] AS BaseT  INNER JOIN sys.[indexes] I ON BaseT.[object_id] = I.[object_id]  ";
    cmd += "INNER JOIN sys.[index_columns] IC ON I.[object_id] = IC.[object_id] ";
    cmd += "INNER JOIN sys.[all_columns] AC ON BaseT.[object_id] = AC.[object_id] AND IC.[column_id] = AC.[column_id] ";
    cmd += "WHERE BaseT.[is_ms_shipped] = 0 AND I.[type_desc] <> 'HEAP' ";
    cmd += "and OBJECT_SCHEMA_NAME(BaseT.[object_id],DB_ID()) = '"+tabSchema(tableName)+"' and ";
    cmd += "BaseT.[name] ='"+tabName(tableName)+"' and I.[is_unique] >0";

    this->initAll(cmd);
    deb("hasUniqueConstraint, second check: "+this->rowElement(1,1));
    if(this->rowElement(1,1).asInt() > 0 ) return 1;

    return 0;
}


int postgres::getUniqueCols(GString table, GSeq <GString> * colSeq)
{
    //this->initAll("use "+m_strCurrentDatabase);
    int indexID = -1;
    GString cmd = "select c.name, i.index_id "
            "from sys.tables t  inner join sys.schemas s on t.schema_id = s.schema_id "
            "inner join sys.indexes i on i.object_id = t.object_id "
            "inner join sys.index_columns ic on ic.object_id = t.object_id and i.index_id = ic.index_id "
            "inner join sys.columns c on c.object_id = t.object_id and ic.column_id = c.column_id "
            "where s.name='"+tabSchema(table)+"' and t.name='"+tabName(table)+"' "
            "and i.is_unique >0 "
            "order by i.is_primary_key desc, index_id" ;
    deb("getUniqueCols, cmd: "+cmd);
    this->initAll(cmd);
    deb("getUniqueCols, cols: "+GString(this->numberOfRows()));
    if( this->numberOfRows() == 0 ) return 1;

    for(unsigned long  i=1; i<=this->numberOfRows(); ++i )
    {
        if( this->rowElement(i, 3).strip("'").asInt() != indexID )
        {
            if( indexID >= 0 && this->rowElement(i, 2).strip("'").asInt() != indexID) break;
            indexID = this->rowElement(i, 2).strip("'").asInt();
            colSeq->add(this->rowElement(i, 1).strip("'"));
        }
    }
    return 0;
}

GString postgres::tabName(GString table)
{
    table = table.removeAll('\"');
    deb("postgres::tabName: "+table);
    if( table.occurrencesOf(".") != 2 && table.occurrencesOf(".") != 1) return "@ErrTabString";
    if( table.occurrencesOf(".") == 2 ) table = table.remove(1, table.indexOf("."));
    return table.subString(table.indexOf(".")+1, table.length()).strip();
}
GString postgres::context(GString table)
{
    table.removeAll('\"');
    deb("postgres::context for "+table);
    if( table.occurrencesOf(".") != 2 ) return "";
    return table.subString(1, table.indexOf(".")-1);
}

GString postgres::tabSchema(GString table)
{
    table.removeAll('\"');
    deb("postgres::tabSchema: "+table);
    if( table.occurrencesOf(".") != 2 && table.occurrencesOf(".") != 1) return "@ErrTabString";
    if( table.occurrencesOf(".") == 2 ) table.remove(1, table.indexOf("."));
    return table.subString(1, table.indexOf(".")-1);
}

void postgres::createXMLCastString(GString &xmlData)
{
	deb("::createXMLCastString called");
	xmlData = " cast('"+(xmlData)+"' as xml)";
}

int postgres::isBinary(unsigned long row, int col)
{
    if( row < 1 || row > allRowsSeq.numberOfElements() ) return 0;
    GRowHdl *aRow;
    aRow = allRowsSeq.elementAtPosition(row);
    if( row < 1 || (unsigned long) col > aRow->elements() ) return 0;
    return aRow->rowElement(col)->isBinary;
}

int  postgres::isNull(unsigned long row, int col)
{
    if( row < 1 || row > allRowsSeq.numberOfElements() ) return 0;
    GRowHdl *aRow;
    aRow = allRowsSeq.elementAtPosition(row);
    if( col < 1 || (unsigned long) col > aRow->elements() ) return 0;
    deb("::isNull: "+GString(aRow->rowElement(col)->isNull)+", row: "+GString(row)+", col: "+GString(col));
    return aRow->rowElement(col)->isNull;
}

int postgres::isTruncated(unsigned long row, int col)
{
    if( row < 1 || row > allRowsSeq.numberOfElements() ) return 0;
    GRowHdl *aRow;
    aRow = allRowsSeq.elementAtPosition(row);
    if( row < 1 || (unsigned long) col > aRow->elements() ) return 0;
    return aRow->rowElement(col)->isTruncated;
}
void postgres::deb(GString fnName, GString txt)
{
    if( m_pGDB ) m_pGDB->debugMsg("postgres", m_iMyInstance, "::"+fnName+" "+txt);
}

void postgres::deb(GString text)
{
    if( m_pGDB ) m_pGDB->debugMsg("postgres", m_iMyInstance, text);
}
void postgres::setTruncationLimit(int limit)
{
    //Setting m_iTruncationLimit to 0 -> no truncation
    m_iTruncationLimit = limit;
}
int postgres::uploadLongBuffer( GString cmd, GString data, int isBinary )
{
    PMF_UNUSED(cmd);
    PMF_UNUSED(data);
    PMF_UNUSED(isBinary);
    return -1;
}
GString postgres::cleanString(GString in)
{
    if( in.length() < 2 ) return in;
    if( in[1UL] == '\'' && in[in.length()] == '\'' ) return in.subString(2, in.length()-2);
    return in;
}
int postgres::isLongTypeCol(int i)
{
    if( i < 1 || (unsigned long)i > sqlLongTypeSeq.numberOfElements() ) return 0;
    return sqlLongTypeSeq.elementAtPosition(i);
}
void postgres::getResultAsHEX(int asHex)
{
    PMF_UNUSED(asHex);
//TODO
}
void postgres::setReadUncommitted(short readUncommitted)
{
    m_iReadUncommitted = readUncommitted;
}

void postgres::setGDebug(GDebug *pGDB)
{
    m_pGDB = pGDB;
}
void postgres::setDatabaseContext(GString context)
{
    deb("setting context: "+context);
    m_strCurrentDatabase = context;
    this->initAll("use "+m_strCurrentDatabase);
}

int postgres::exportAsTxt(int mode, GString sqlCmd, GString table, GString outFile, GSeq <GString>* startText, GSeq <GString>* endText, GString *err)
{
    PMF_UNUSED(mode);
    deb("exportAsTxt start. cmd: "+sqlCmd+", table: "+table+", out: "+outFile);
    GFile f(outFile, GF_APPENDCREATE);
    if( !f.initOK() )
    {
        *err = "Could not open file "+outFile+" for writing.";
        return 1;
    }
    GSeq <GString> aSeq;
    aSeq.add("--------------------------------------- ");
    aSeq.add("-- File created by Poor Man's Flight (PMF) www.leipelt.de");
    aSeq.add("-- Statement: "+sqlCmd);
    aSeq.add("-- ");
    aSeq.add("-- NOTE: LOBs will be given as NULL. ");
    aSeq.add("--------------------------------------- ");
    aSeq.add("");
    aSeq.add("-- Use the following line to set the target database:");
    aSeq.add("-- USE [DataBase] ");
    aSeq.add("");

	aSeq.add("-- SET IDENTITY_INSERT "+table+" ON");

    for(unsigned long i = 1; i <= startText->numberOfElements(); ++i) aSeq.add(startText->elementAtPosition(i));

//    *err = this->readRowData(sqlCmd, 0, 0, 1);
    if(err->length()) return this->sqlCode();

    remove(outFile);


    GString columns = "INSERT INTO "+table+"(";
    for(unsigned long i = 1; i <= this->numberOfColumns(); ++i)
    {
        columns += "["+this->hostVariable(i)+"], ";
    }
    columns = columns.stripTrailing(", ")+") VALUES (";

    deb("exportAsTxt cols is: "+columns);

    GString out, data;
    for(unsigned long i = 1; i <= this->numberOfRows(); ++i)
    {
        out = "";
        for(unsigned long j = 1; j <= this->numberOfColumns(); ++j)
        {
            data = this->rowElement(i, j);
            if( this->isNumType(j))  out += cleanString(data)+", ";
			else if( data == "NULL" )out += data+", ";
            else if( data.occurrencesOf("@DSQL@") ) out +="NULL, ";
            else out += data+", ";
        }
        out = out.stripTrailing(", ") + ")";
        aSeq.add(columns+out);
        deb("exportAsTxt adding  "+out);
        if( aSeq.numberOfElements() > 1000)
        {
            f.append(&aSeq);
            aSeq.removeAll();
            deb("exportAsTxt appending to "+outFile);
        }
    }
    for(unsigned long i = 1; i <= endText->numberOfElements(); ++i) aSeq.add(endText->elementAtPosition(i));
	aSeq.add("-- SET IDENTITY_INSERT "+table+" OFF");
    f.append(&aSeq);

    return 0;
}
int postgres::deleteTable(GString tableName)
{
    tableName.removeAll('\"');
	this->initAll("drop table "+tableName);
    if( sqlCode() && this->getDdlForView(tableName).length())
    {
        this->initAll("drop view "+tabSchema(tableName)+"."+tabName(tableName));
    }
	return sqlCode();
}


GString postgres::fillChecksView(GString, int)
{
    return "";
}

postgres::RowData::~RowData()
{
    rowDataSeq.removeAll();
}
void postgres::RowData::add(GString data)
{
    rowDataSeq.add(data);
}
GString postgres::RowData::elementAtPosition(int pos)
{
    if( pos < 1 || pos > (int)rowDataSeq.numberOfElements() ) return "";
    return rowDataSeq.elementAtPosition(pos);
}
int postgres::getHeaderData(int pos, GString * data)
{
    if( pos < 1 || pos > (int)headerSeq.numberOfElements()) return 1;
    *data = headerSeq.elementAtPosition(pos);
    return 0;
}

int postgres::getRowData(int row, int col, GString * data)
{
    if( row < 1 || row > (int)rowSeq.numberOfElements() ) return 1;
    if( col < 1 || col > (int)rowSeq.elementAtPosition(row)->numberOfElements() ) return 1;
    *data = rowSeq.elementAtPosition(row)->elementAtPosition(col);
    return 0;
}

unsigned long postgres::RowData::numberOfElements()
{
    return rowDataSeq.numberOfElements();
}
unsigned long postgres::getHeaderDataCount()
{
    return headerSeq.numberOfElements();
}
unsigned long postgres::getRowDataCount()
{
    return rowSeq.numberOfElements();
}

int postgres::isTransaction()
{
    return m_iIsTransaction;
}

void postgres::clearSequences()
{
    RowData *pRowData;
    while( !rowSeq.isEmpty() )
    {
        pRowData = rowSeq.firstElement();
        delete pRowData;
        rowSeq.removeFirst();
    }
	headerSeq.removeAll();
}

GString postgres::getDdlForView(GString tableName)
{       
    this->setCLOBReader(1);
    this->initAll("SELECT VIEW_DEFINITION FROM INFORMATION_SCHEMA.VIEWS WHERE TABLE_SCHEMA='"+ tabSchema(tableName)+"' AND TABLE_NAME='"+tabName(tableName)+"'");
    this->setCLOBReader(1);
    if( this->numberOfRows() == 0 ) return "";
    return "CREATE VIEW "+tabName(tableName)+" AS "+  cleanString(this->rowElement(1, 1).strip());
}

void postgres::setAutoCommmit(int commit)
{
}

int postgres::execByDescriptor( GString cmd, GSeq <GString> *dataSeq)
{
    PMF_UNUSED(cmd);
    PMF_UNUSED(dataSeq);
    return -1;
}
int postgres::execByDescriptor( GString cmd, GSeq <GString> *dataSeq, GSeq <int> *typeSeq,
                                GSeq <short>* sqlVarLengthSeq, GSeq <int> *forBitSeq )
{
    PMF_UNUSED(cmd);
    PMF_UNUSED(dataSeq);
    PMF_UNUSED(typeSeq);
    PMF_UNUSED(sqlVarLengthSeq);
    PMF_UNUSED(forBitSeq);
    return -1;
}
long postgres::uploadBlob(GString cmd, char * buffer, long size)
{
    PMF_UNUSED(cmd);
    PMF_UNUSED(buffer);
    PMF_UNUSED(size);
    return -1;
}

long postgres::retrieveBlob( GString cmd, GString &blobFile, int writeFile )
{
    PMF_UNUSED(cmd);
    PMF_UNUSED(blobFile);
    PMF_UNUSED(writeFile);
    return -1;
}

void postgres::currentConnectionValues(CON_SET * conSet)
{
    conSet->DB = m_strDB;
    conSet->Host = m_strHost;
    conSet->PWD = m_strPWD;
    conSet->UID = m_strUID;
    conSet->Port = m_iPort;
}

GString postgres::lastSqlSelectCommand()
{
    return m_strLastSqlSelectCommand;;
}

TABLE_PROPS postgres::getTableProps(GString tableName)
{
    TABLE_PROPS tableProps;
    tableProps.init();

    this->initAll("SELECT TABLE_TYPE FROM information_schema.TABLES where TABLE_SCHEMA='"+ tabSchema(tableName)+"' AND TABLE_NAME='"+tabName(tableName)+"'");
    if( rowElement(1,1).strip("'") == "VIEW" ) tableProps.TableType = TYPE_TYPED_VIEW;
    else if( rowElement(1,1).strip("'") == "BASE TABLE" ) tableProps.TableType = TYPE_TYPED_TABLE;
    else if( rowElement(1,1).strip("'") == "SYSTEM VIEW" ) tableProps.TableType = TYPE_TYPED_VIEW;

    return tableProps;
}

int postgres::tableIsEmpty(GString tableName)
{
    GString err = this->initAll("select top 1 * from " + tableName);
    if( this->numberOfRows() == 0 || err.length() ) return 1;
    return 0;
}

int postgres::deleteViaFetch(GString tableName, GSeq<GString> * colSeq, int rowCount, GString whereClause)
{
    GString cmd = "DELETE TOP ("+GString(rowCount)+") FROM "+tableName;
    if( whereClause.length() ) cmd += " WHERE "+whereClause;
    GString err = this->initAll(cmd);    
    deb("deleteViaFetch: cmd: "+cmd+"\nErr: "+err);
    if( err.length() ) return sqlCode();
    this->commit();
    return 0;
}


void postgres::setCLOBReader(short readCLOBData )
{
    m_iReadClobData = readCLOBData;
}

/******************************************************************
 *
 *  ifdef: Use QT for some tasks
 *
 *****************************************************************/
#ifdef  QT4_DSQL
GString postgres::getIdenticals(GString table, QWidget* parent, QListWidget *pLB, short autoDel)
{

	GString message = "SELECT * FROM "+table;
	GString retString = "";
	deb("::getIdenticals, cmd: "+message);


    return retString;
}

void postgres::writeToLB(QListWidget * pLB, GString message)
{
    for( int i = 0; i < pLB->count(); ++i )
        if( GString(pLB->item(i)->text()) == message ) return;
    new QListWidgetItem(message, pLB);
}


GString  postgres::fillIndexView(GString table, QWidget* parent, QTableWidget *pWdgt)
{
    PMF_UNUSED(parent);
	//Clear data:
	while( pWdgt->rowCount() ) pWdgt->removeRow(0);
	
	GString id, name, cols, unq, crt, mod, dis;
	int indexID = -1;
    //this->initAll("use "+m_strCurrentDatabase);
	
    GString cmd = "select i.index_id, i.name, c.name,  i.is_primary_key, i.is_unique_constraint,"
			"create_date, modify_date, i.is_disabled "
            "from sys.tables t  inner join sys.schemas s on t.schema_id = s.schema_id "
            "inner join sys.indexes i on i.object_id = t.object_id "
            "inner join sys.index_columns ic on ic.object_id = t.object_id "
            "inner join sys.columns c on c.object_id = t.object_id and ic.column_id = c.column_id "
			"where s.name='"+tabSchema(table)+"' and t.name='"+tabName(table)+"'";
    this->initAll(cmd);
	deb("::fillIndexView, cmd: "+cmd);
    deb("::fillIndexView, found: "+GString(this->numberOfRows()));

    pWdgt->setColumnCount(7);
	
	QTableWidgetItem * pItem;
	int i = 0;
	pItem = new QTableWidgetItem("IndexID"); pWdgt->setHorizontalHeaderItem(i++, pItem);
    pItem = new QTableWidgetItem("Name"); pWdgt->setHorizontalHeaderItem(i++, pItem);
	pItem = new QTableWidgetItem("Columns"); pWdgt->setHorizontalHeaderItem(i++, pItem);
    pItem = new QTableWidgetItem("Type"); pWdgt->setHorizontalHeaderItem(i++, pItem);
	pItem = new QTableWidgetItem("Created"); pWdgt->setHorizontalHeaderItem(i++, pItem);
	pItem = new QTableWidgetItem("Modified"); pWdgt->setHorizontalHeaderItem(i++, pItem);
	pItem = new QTableWidgetItem("IsDisabled"); pWdgt->setHorizontalHeaderItem(i++, pItem);
	
	int row = 0;
    for(int  i=1; i<= (int)this->numberOfRows(); ++i )
    {
		deb("::fillIndexView, i: "+GString(i));
        if( this->rowElement(i, 1).strip("'").asInt() != indexID )
        {
			deb("::fillIndexView, i: "+GString(i)+", indexChg, new: "+GString(indexID));
			if( cols.length() )
			{
				cols = cols.stripTrailing(", ");
				createIndexRow(pWdgt, row++, id, name, cols, unq, crt, mod, dis);
			}
			id   = this->rowElement(i, 1);
			name = this->rowElement(i, 2);
			cols = ""; 
			if( this->rowElement(i, 4) == "'1'" ) unq = "Primary Key";
			else if( this->rowElement(i, 5) == "'1'" ) unq = "Unique Key";
			else unq = "";
			crt = this->rowElement(i, 6);
			mod = this->rowElement(i, 7);
			dis = this->rowElement(i, 8);
            indexID = this->rowElement(i, 1).strip("'").asInt();
			deb("::fillIndexView, i: "+GString(i)+", currIDx: "+GString(id)+", cols: "+cols);
        }
		cols += this->rowElement(i, 3).strip("'")+", ";
		deb("::fillIndexView, i: "+GString(i)+", currIDx: "+GString(id)+", cols: "+cols);
    }
	if( cols.length() )
	{
		cols = cols.stripTrailing(", ");
		createIndexRow(pWdgt, row++, id, name, cols, unq, crt, mod, dis);
		deb("::fillIndexView, final: "+GString(id)+", cols: "+cols);
	}
	return "";
}
void postgres::createIndexRow(QTableWidget *pWdgt, int row,
		GString id, GString name, GString cols, GString unq, GString crt, GString mod, GString dis)
{
	if( !id.length() ) return;
	deb("::fillIndexView, createRow, id: "+id);
	int j = 0;
	QTableWidgetItem * pItem;
	pWdgt->insertRow(pWdgt->rowCount());
	pItem = new QTableWidgetItem(); pItem->setText(id);   pWdgt->setItem(row, j++, pItem);
	pItem = new QTableWidgetItem(); pItem->setText(name); pWdgt->setItem(row, j++, pItem);
	pItem = new QTableWidgetItem(); pItem->setText(cols); pWdgt->setItem(row, j++, pItem);
	pItem = new QTableWidgetItem(); pItem->setText(unq);  pWdgt->setItem(row, j++, pItem);
	pItem = new QTableWidgetItem(); pItem->setText(crt);  pWdgt->setItem(row, j++, pItem);
	pItem = new QTableWidgetItem(); pItem->setText(mod);  pWdgt->setItem(row, j++, pItem);
	pItem = new QTableWidgetItem(); pItem->setText(dis);  pWdgt->setItem(row, j++, pItem);

}
#endif
/******************************************************************
 *
 *  END ifdef: Use QT for some tasks
 *
 *****************************************************************/


