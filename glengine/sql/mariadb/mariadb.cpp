    //
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt
//

#ifndef _MARIADB_
#include <mariadb.hpp>
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
static int m_mariaDBobjCounter = 0;

#define XML_MAX 250



/***********************************************************************
 * This class can either be instatiated or loaded via dlopen/loadlibrary
 ***********************************************************************/

//Define functions with C symbols (create/destroy instance).
#ifndef MAKE_VC
extern "C" mariaDB* create()
{
    //printf("Calling mariaDB creat()\n");
    return new mariaDB();
}
extern "C" void destroy(mariaDB* pODBCSQL)
{
   if( pODBCSQL ) delete pODBCSQL ;
}
#else
extern "C" __declspec( dllexport ) mariaDB* create()
{
    //printf("Calling mariaDB creat()\n");
    _flushall();
    return new mariaDB();
}	
extern "C" __declspec( dllexport ) void destroy(mariaDB* pODBCSQL)
{
    if( pODBCSQL ) delete pODBCSQL ;
}	
#endif
/***********************************************************
 * CLASS
 **********************************************************/

mariaDB::mariaDB(mariaDB  const &o)
{
	m_iLastSqlCode = 0;
    m_mariaDBobjCounter++;
    m_iTruncationLimit = 500;
    m_iMyInstance = m_mariaDBobjCounter;
    m_pGDB = o.m_pGDB;
    deb("CopyCtor start");

    m_odbcDB  = MARIADB;
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

    m_mariaSql = mysql_init(NULL);
    //m_mariaSql = o.m_mariaSql;   
    deb("CopyCtor: calling connect, connection data: "+m_strDB+", uid: "+m_strUID+", host: "+m_strHost+", port: "+GString(m_iPort));
    this->connect(m_strDB, m_strUID, m_strPWD, m_strHost, GString(m_iPort));
    if( m_strCurrentDatabase.length() ) this->initAll("USE "+m_strCurrentDatabase);
    this->initAll("SET NAMES 'utf8'");
    //!!TODO
//    if( m_iReadUncommitted ) readRowData("set transaction isolation level read uncommitted");
//    else readRowData("set transaction isolation level REPEATABLE READ");
    deb("Copy CTor done");
}

mariaDB::mariaDB()
{
    m_mariaDBobjCounter++;
    //m_IDSQCounter++;
    m_iTruncationLimit = 500;
    m_iMyInstance = m_mariaDBobjCounter;

	m_iLastSqlCode = 0;
    m_iIsTransaction = 0;	
    m_odbcDB  = MARIADB;
    m_mariaSql = mysql_init(NULL);
    if (!m_mariaSql)
    {
        //printf("Init faild, out of memory?\n");
        return;
    }
    //else printf("INIT OK\n");

    m_strDB ="";
    m_strUID ="";
    m_strPWD = "";
    m_strHost = "localhost";
    m_iPort = 3306;
    m_pGDB = NULL;
    m_strLastSqlSelectCommand = "";
    m_iReadUncommitted = 0;
    m_iReadClobData = 0;
    m_strCurrentDatabase = "";
    printf("mariaDB[%i]> DefaultCtor done. Port: %i\n", m_iMyInstance, m_iPort );
}
mariaDB* mariaDB::clone() const
{
    return new mariaDB(*this);
}

mariaDB::~mariaDB()
{
    deb("Dtor start, current count: "+GString(m_mariaDBobjCounter));
    disconnect();

    m_mariaDBobjCounter--;
    //m_IDSQCounter--;
    deb("Dtor, clearing RowData...");
    clearSequences();
    deb("Dtor done, current count: "+GString(m_mariaDBobjCounter));
}

int mariaDB::getConnData(GString * db, GString *uid, GString *pwd) const
{
    *db = m_strDB;
    *uid = m_strUID;
    *pwd = m_strPWD;
	return 0;
}


GString mariaDB::connect(GString db, GString uid, GString pwd, GString host, GString port)
{    
    deb("::connect, connect to DB: "+db+", uid: "+uid+", host: "+host+", port: "+port);
    if( host.strip().length() == 0 ) host = "localhost";
    if( host == _HOST_DEFAULT ) host = "localhost";
    if( port.asInt() <= 0 ) port = "3306";

    m_strDB = db;
    m_strUID = uid;
    m_strPWD = pwd;
    m_strNode = host;
    m_strHost = host;
    m_iPort = port.asInt();
    deb("::connect, connection data: "+m_strDB+", uid: "+m_strUID+", host: "+host+", port: "+GString(port));

    MYSQL *ret = mysql_real_connect(m_mariaSql, host, uid, pwd, db, (unsigned int) port.asInt(), NULL, 0);
    if( ret == NULL || ret != m_mariaSql )
    {
        return sqlError();
    }
    this->initAll("SET NAMES 'utf8'");
    return "";
}

int mariaDB::disconnect()
{	

    deb("Disconnecting and closing.");       
    if( !m_mariaSql ) return 0;
    mysql_close (m_mariaSql);
    m_mariaSql = mysql_init(NULL);
	return 0;
}
GString mariaDB::initAll(GString message, unsigned long maxRows,  int getLen)
{    

    deb("::initAll, start. msg: "+message);
    MYSQL_RES *res;
    MYSQL_ROW row;
    MYSQL_FIELD *fld;

    if( mysql_query(m_mariaSql, (const char*) message) )
    {
        return sqlError();
    }
    res = mysql_store_result(m_mariaSql);
    resetAllSeq();

    GRowHdl * pRow;
    m_iNumberOfColumns = mysql_field_count(m_mariaSql);

    if( m_iNumberOfColumns == 0 ) return sqlError();

    m_strLastSqlSelectCommand = message;
    m_iNumberOfRows = 0;
    for(unsigned int i = 0; (fld = mysql_fetch_field(res)); i++)
    {
        hostVarSeq.add(fld->name);
        sqlTypeSeq.add(fld->type);
        setSimpleColType(fld->type);
    }

    while ((row = mysql_fetch_row(res)))
    {
        // Print all columns
        pRow = new GRowHdl;
        for(int i = 0; i < m_iNumberOfColumns; i++)
        {
            // Make sure row[i] is valid!
            if(row[i] == NULL) pRow->addElement("NULL");
            else if( isNumType(i+1)) pRow->addElement(row[i]);
            else if( isLOBCol(i+1) && !m_iReadClobData )  pRow->addElement("@DSQL@BLOB");
            else pRow->addElement("'"+GString(row[i])+"'");
        }
        m_iNumberOfRows++;
        if( maxRows > 0 && m_iNumberOfRows > maxRows ) break;
        allRowsSeq.add( pRow );
    }
    if(res != NULL) mysql_free_result(res);
    return sqlError();
}

void mariaDB::setSimpleColType(enum_field_types type)
{
    switch(type)
    {
    case MYSQL_TYPE_SHORT:
    case MYSQL_TYPE_INT24:
        simpleColTypeSeq.add(CT_INTEGER);
        break;

    case MYSQL_TYPE_LONG:
    case MYSQL_TYPE_DOUBLE:
    case MYSQL_TYPE_LONGLONG:
        simpleColTypeSeq.add(CT_LONG);
        break;

    case MYSQL_TYPE_NEWDECIMAL:
    case MYSQL_TYPE_DECIMAL:
        simpleColTypeSeq.add(CT_DECIMAL);
        break;


    case MYSQL_TYPE_FLOAT:
        simpleColTypeSeq.add(CT_FLOAT);
        break;

    case MYSQL_TYPE_TIMESTAMP:
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_TIME:
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_YEAR:
    case MYSQL_TYPE_NEWDATE:
    case MYSQL_TYPE_TIMESTAMP2:
    case MYSQL_TYPE_DATETIME2:
    case MYSQL_TYPE_TIME2:
        simpleColTypeSeq.add(CT_DATE);
        break;

    case MYSQL_TYPE_NULL:
    case MYSQL_TYPE_VARCHAR:
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_BIT:
    case MYSQL_TYPE_ENUM:
    case MYSQL_TYPE_SET:
        simpleColTypeSeq.add(CT_STRING);
        break;


    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_GEOMETRY:
        simpleColTypeSeq.add(CT_CLOB);
        break;

    default:
        simpleColTypeSeq.add(CT_STRING);
        break;
    }
}

/*****************************************************
 * 1. getFullXML is currently unused and anyway will only be useful w/ TDS 7.1/8.0
 *
 * 2. TDS7.2: len will be returned as -4
 *
 * So at the moment (using TDS 7.2) we return the buf although len is -4
 *
*****************************************************/

int mariaDB::handleLobsAndXmls(int col, GString * out, int getFullXML, short* isNull)
{
    PMF_UNUSED(getFullXML);

    deb("::handleLobsAndXmls, col: "+GString(col)+", lob: "+GString(lobSeq.elementAtPosition(col))+", xml: "+GString(xmlSeq.elementAtPosition(col)));

    *out = "";
    *isNull = 0;

    if( isLOBCol(col) == 0 && isXMLCol(col) == 0 ) return 0;
    return 1;
}

int mariaDB::commit()
{
    //m_db.commit();
	return 0;
}

int mariaDB::rollback()
{
    //m_db.rollback();
	return 0;
}


int mariaDB::initRowCrs()
{
	deb("::initRowCrs");
    if( allRowsSeq.numberOfElements() == 0 ) return 1;
    m_pRowAtCrs = allRowsSeq.initCrs();
    return 0;
}
int mariaDB::nextRowCrs()
{	
    if( m_pRowAtCrs == NULL ) return 1;
    m_pRowAtCrs = allRowsSeq.setCrsToNext();
    return 0;
}
long  mariaDB::dataLen(const short & pos)
{
    if( pos < 1 || pos > (short) sqlLenSeq.numberOfElements()) return 0;
    deb("Len at "+GString(pos)+": "+GString(sqlLenSeq.elementAtPosition(pos)));    
    return sqlLenSeq.elementAtPosition(pos);
}

int mariaDB::isDateTime(int pos)
{
    if( pos < 1 || pos > (int)sqlTypeSeq.numberOfElements() ) return 0;
    switch( sqlTypeSeq.elementAtPosition(pos) )
    {
        case MYSQL_TYPE_TIMESTAMP:
        case MYSQL_TYPE_DATE:
        case MYSQL_TYPE_TIME:
        case MYSQL_TYPE_DATETIME:
        case MYSQL_TYPE_TIMESTAMP2:
        case MYSQL_TYPE_DATETIME2:
        case MYSQL_TYPE_TIME2:
        case MYSQL_TYPE_YEAR:
        case MYSQL_TYPE_NEWDATE:
            return 1;
    }
    return 0;
}

int mariaDB::isNumType(int pos)
{
    if( pos < 1 || pos > (int) sqlTypeSeq.numberOfElements() ) return 0;
    switch( sqlTypeSeq.elementAtPosition(pos) )
    {
        case MYSQL_TYPE_DECIMAL:
        case MYSQL_TYPE_NEWDECIMAL:
        case MYSQL_TYPE_TINY:
        case MYSQL_TYPE_SHORT:
        case MYSQL_TYPE_LONG:
        case MYSQL_TYPE_FLOAT:
        case MYSQL_TYPE_DOUBLE:
            return 1;
    }
    return 0;
}
int mariaDB::hasForBitData()
{
    return 0;
}
int mariaDB::isForBitCol(int i)
{
    if( i < 1 || (unsigned long)i > sqlForBitSeq.numberOfElements() ) return 0;
    return sqlForBitSeq.elementAtPosition(i);
}
int mariaDB::isBitCol(int i)
{
    if( i < 1 || (unsigned long)i > sqlBitSeq.numberOfElements() ) return 0;
    return sqlBitSeq.elementAtPosition(i);
}

unsigned int mariaDB::numberOfColumns()
{
    //deb("::numberOfColumns called");
    return m_iNumberOfColumns;
}

unsigned long mariaDB::numberOfRows()
{
    return m_iNumberOfRows;
}
GString mariaDB::dataAtCrs(int col)
{
    if( m_pRowAtCrs == NULL ) return "@CrsNotOpen";
    if( col < 1 || (unsigned long)col > m_pRowAtCrs->elements() ) return "@OutOfReach";
    return m_pRowAtCrs->rowElementData(col);
}
GString mariaDB::rowElement( unsigned long row, int col)
{
    //Overload 1
    //tm("::rowElement for "+GString(line)+", col: "+GString(col));
    if( row < 1 || row > allRowsSeq.numberOfElements() ) return "@OutOfReach";
    GRowHdl *aRow;
    aRow = allRowsSeq.elementAtPosition(row);
    if( row < 1 || (unsigned long) col > aRow->elements() ) return "OutOfReach";
    return aRow->rowElementData(col);
}
int mariaDB::positionOfHostVar(const GString& hostVar)
{
    unsigned long i;
    for( i = 1; i <= hostVarSeq.numberOfElements(); ++i )
    {
        if( hostVarSeq.elementAtPosition(i) == hostVar ) return i;
    }
    return 0;
}
GString mariaDB::rowElement( unsigned long row, GString hostVar)
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


GString mariaDB::hostVariable(int col)
{
    //deb("::hostVariable called, col: "+GString(col));
    if( col < 1 || col > (short) hostVarSeq.numberOfElements() ) return "@hostVariable:OutOfReach";
    return hostVarSeq.elementAtPosition(col);
}



GString mariaDB::currentCursor(GString filter, GString command, long curPos, short commitIt, GSeq <GString> *fileList, GSeq <long> *lobType)
{
    deb(__FUNCTION__, "filter: "+filter+", cmd: "+command);
    int rc;


    if( curPos < 0 ) return "CurrentCursor(): POS < 0";
    deb(__FUNCTION__, "done, rc: "+GString(rc));
    return rc ? m_strLastError : "";
}



int mariaDB::sqlCode()
{
    //deb("::sqlCode: m_iLastSqlCode = "+GString(m_iLastSqlCode));
    m_iLastSqlCode = mysql_errno(m_mariaSql);
	return m_iLastSqlCode;
}
GString mariaDB::sqlError()
{    
    deb("::sqlError: m_iLastSqlCode = "+GString(m_iLastSqlCode));
    m_strLastError = mysql_error(m_mariaSql);
    //printf("--------------------------------\nError: %s\nLastCmd: %s\n------------------\n", (char*) m_strLastError, (char*) m_strLastSqlSelectCommand);
    return m_strLastError;
}		

int mariaDB::getTabSchema()
{
    GString cmd = "select distinct(table_schema) from information_schema.tables order by table_schema";
    this->initAll(cmd);
    return 0;
}
int mariaDB::getTables(GString schema)
{
    m_strCurrentDatabase = schema;
    this->initAll("USE "+schema);
    GString cmd = "select table_name from information_schema.tables where table_schema = '"+schema+"' order by table_name";
    this->initAll(cmd);
    return 0;
}
void mariaDB::convToSQL( GString& input )
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

int mariaDB::getAllTables(GSeq <GString > * tabSeq, GString filter)
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
short mariaDB::sqlType(const short & col)
{
   if( col < 1 || col > (short) sqlTypeSeq.numberOfElements() ) return -1;
   else return sqlTypeSeq.elementAtPosition(col);
}
short mariaDB::sqlType(const GString & colName)
{
   for( short i = 1; i <= (short) hostVarSeq.numberOfElements(); ++ i )
   {
      if( hostVarSeq.elementAtPosition(i) == colName ) return sqlTypeSeq.elementAtPosition(i);
   }
   return -1;
}
GString mariaDB::realName(const short & sqlType)
{
	switch(sqlType)
	{
    case MYSQL_TYPE_DECIMAL:
    case MYSQL_TYPE_TINY:
    case MYSQL_TYPE_SHORT:
    case MYSQL_TYPE_LONG:
    case MYSQL_TYPE_FLOAT:
    case MYSQL_TYPE_DOUBLE:
            return "Numeric";
			
    case MYSQL_TYPE_TIMESTAMP:
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_TIME:
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_TIMESTAMP2:
    case MYSQL_TYPE_DATETIME2:
    case MYSQL_TYPE_TIME2:
    case MYSQL_TYPE_YEAR:
    case MYSQL_TYPE_NEWDATE:
            return "Time";
			
		default:
			return "'string'";
	}
	return "";
}

/********************************************************************
 * lobTypeSequence is -4 for BLOBs, -1 for CLOBs, and 0 else
 */


int mariaDB::loadFileIntoBuf(GString fileName, unsigned char** fileBuf, unsigned long *size)
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



long mariaDB::uploadBlob(GString cmd, GSeq <GString> *fileSeq, GSeq <long> *lobType)
{
    deb("::uploadBlob, cmd: "+cmd);

    MYSQL_STMT    *stmt;
    MYSQL_BIND  *bind = new MYSQL_BIND[fileSeq->numberOfElements()];

    stmt = mysql_stmt_init(m_mariaSql);

    if (!stmt)  return sqlError();


    char* aQuery = (char*) cmd;
    if (mysql_stmt_prepare(stmt, aQuery, strlen(aQuery)))
    {
        return sqlCode();
    }
    memset(bind, 0, sizeof(bind));


    unsigned char   **fileBuf = new unsigned char* [fileSeq->numberOfElements()];
    unsigned long   *fileSize = new unsigned long[fileSeq->numberOfElements()];

    for( int i = 0; i < fileSeq->numberOfElements(); ++i )
    {
        loadFileIntoBuf(fileSeq->elementAtPosition(i+1), &(fileBuf[i]), &(fileSize[i]));
        bind[i].buffer_type= static_cast<enum_field_types>(lobType->elementAtPosition(i+1));
        bind[i].buffer= fileBuf[i];
        bind[i].buffer_length= fileSize[i];
        bind[i].is_null= 0;
        bind[i].length= &fileSize[i];
    }

    if (mysql_stmt_bind_param(stmt, bind))
    {
        delete [] fileBuf;
        delete [] fileSize;
        return sqlCode();
    }

    MYSQL_RES* aRes = mysql_stmt_result_metadata(stmt);

    my_bool aBool = 1;
    mysql_stmt_attr_set(stmt, STMT_ATTR_UPDATE_MAX_LENGTH, &aBool);

    mysql_stmt_execute(stmt);
    GString errTxt = mysql_stmt_error(stmt);
    //printf("ERRR: %s\n", (char*) errTxt);
    if (mysql_stmt_execute(stmt))
    {
        delete [] fileBuf;
        delete [] fileSize;
        //printf("##########################################\n");
        sqlError();
        //printf("##########################################\n");
        return sqlCode();
    }
    delete [] fileBuf;
    delete [] fileSize;
    delete [] bind;
    return 0;
}

GString mariaDB::descriptorToFile( GString cmd, GString &blobFile, int * outSize )
{
    deb("::descriptorToFile, cmd: "+cmd);

    MYSQL_STMT    *stmt;
    MYSQL_BIND    bind[1];
    MYSQL_BIND    bind_result[1];

    *outSize = 0;

    // _con your mysql connection
    stmt = mysql_stmt_init(m_mariaSql);
    if (!stmt)  return sqlError();


    char* aQuery = (char*) cmd;
    if (mysql_stmt_prepare(stmt, aQuery, strlen(aQuery)))
    {
        deb("::descriptorToFile, err(1):"+sqlError());
        return sqlError();
    }

        // Here fill binded parameters (here a string)
    memset(bind, 0, sizeof(bind));

    GString aStr;
    long unsigned int aSize;

    bind[0].buffer_type= MYSQL_TYPE_BLOB;
    bind[0].buffer= (char *) aStr;
    bind[0].buffer_length= 2048;
    bind[0].is_null= 0;
    bind[0].length= &aSize;

    if (mysql_stmt_bind_param(stmt, bind))
    {
        deb("::descriptorToFile, err bind:"+sqlError());
        return sqlError();
    }

    MYSQL_RES* aRes = mysql_stmt_result_metadata(stmt);

        // Set STMT_ATTR_UPDATE_MAX_LENGTH attribute
    my_bool aBool = 1;
    mysql_stmt_attr_set(stmt, STMT_ATTR_UPDATE_MAX_LENGTH, &aBool);

    /* Execute the select statement - 1*/
    if (mysql_stmt_execute(stmt))
    {
        deb("::descriptorToFile, err exec:"+sqlError());
        return sqlError();
    }

    if (mysql_stmt_store_result(stmt))
    {
        deb("::descriptorToFile, err store:"+sqlError());
        return sqlError();
    }

    // Retrieving meta data information
    MYSQL_FIELD* aField = &aRes->fields[0];

    fprintf(stdout, " field %s \n",aField->name);
    fprintf(stdout, " field length %d \n",(int) aField->length);
    fprintf(stdout, " field max length %d \n", (int) aField->max_length);


    int totalrows = mysql_stmt_num_rows(stmt);
    fprintf(stdout, " fetched %d description\n",totalrows);
    fprintf(stdout, " field count %d \n",(int) aRes->field_count);

    long unsigned int aMaxSize;
    char* aBuffer = (char*) malloc(aField->max_length);

    memset (bind_result, 0, sizeof (bind_result));
    bind_result[0].buffer_type= MYSQL_TYPE_BLOB;
    bind_result[0].is_null= 0;
    bind_result[0].buffer= (char *) aBuffer;
    bind_result[0].buffer_length= aField->max_length;
    bind_result[0].length= &aMaxSize;

    mysql_stmt_bind_result(stmt, bind_result);

    std::string aStrData;
    while(!mysql_stmt_fetch(stmt))
    {
        fprintf(stdout, " size %d\n", (int) aMaxSize);
        aStrData = std::string(aBuffer,aMaxSize);
        fprintf(stdout, " data %s\n", aStrData.c_str());
        *outSize += writeToFile(blobFile, &aBuffer, aMaxSize);
    }


    free(aBuffer);
    mysql_free_result(aRes);

    return sqlError();
}

int mariaDB::writeToFile(GString fileName, char** buf, int len)
{
    FILE * f;
    f = fopen(fileName, "ab");
    int written = fwrite(*buf,1,len,f);
    fclose(f);
    return written;
}

signed long mariaDB::getCost()
{
    return -1;
}

int mariaDB::isXMLCol(int i)
{
    if( i < 1 || (unsigned long)i > xmlSeq.numberOfElements() ) return 0;
    return xmlSeq.elementAtPosition(i);
}

int mariaDB::simpleColType(int i)
{
    if( i < 1 || (unsigned long)i > simpleColTypeSeq.numberOfElements() ) return CT_UNKNOWN;

    if(isXMLCol(i)) return CT_XML;
    if(isLOBCol(i)) return CT_DBCLOB;
    if(isNumType(i)) return CT_INTEGER;
    if(isDateTime(i)) return CT_DATE;
    return CT_STRING;
}

int mariaDB::isLOBCol(int i)
{
    if( i < 1 || (unsigned long)i > sqlTypeSeq.numberOfElements() ) return 0;
    switch(sqlTypeSeq.elementAtPosition(i))
    {
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_BLOB:
        return 1;

    }
    return 0;
}

int mariaDB::isNullable(int i)
{
    if( i < 1 || (unsigned long)i > sqlIndVarSeq.numberOfElements() ) return 1;
    return sqlIndVarSeq.elementAtPosition(i);
}


int mariaDB::isFixedChar(int i)
{
    if( i < 1 || (unsigned long)i > sqlTypeSeq.numberOfElements() ) return 0;
    return 0;
}

int mariaDB::getDataBases(GSeq <CON_SET*> *dbList)
{
    return 0;
    CON_SET * pCS;
    this->initAll("select table_schema as database_name, table_name from information_schema.tables where table_type = 'BASE TABLE' order by database_name, table_name;");
	deb("getDataBases done");
    return 0;
}
void mariaDB::setCurrentDatabase(GString db)
{
    m_strCurrentDatabase = db;
    this->initAll("use "+m_strCurrentDatabase);
    m_strCurrentDatabase = db;
}
GString mariaDB::currentDatabase()
{
    return m_strCurrentDatabase;
}
unsigned long mariaDB::getResultCount(GString cmd)
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
void mariaDB::resetAllSeq()
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
int mariaDB::getColSpecs(GString table, GSeq<COL_SPEC*> *specSeq)
{

    GString tabSchema = this->tabSchema(table);
    GString tabName   = this->tabName(table);

    this->setCLOBReader(1); //This will read any CLOB into fieldData

    GString cmd = "SELECT column_name, data_type, character_maximum_length, numeric_precision, is_nullable, column_default, numeric_scale FROM information_schema.COLUMNS WHERE TABLE_SCHEMA='"+tabSchema+"' AND table_name='"+tabName+"'";

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
        if( cSpec->ColType.occurrencesOf("int") > 0 ||
                cSpec->ColType.occurrencesOf("double") > 0 ||
                cSpec->ColType.occurrencesOf("float") > 0 ||
                cSpec->ColType.occurrencesOf("date") > 0 ||
                cSpec->ColType.occurrencesOf("time"))
        {
            cSpec->Length = "N/A";
        }
        /* DECIMAL: (LENGTH, SCALE) */
        else if( cSpec->ColType.occurrencesOf("decimal") )
        {
            cSpec->Length = this->rowElement(i,4).strip("'").strip()+". "+this->rowElement(i,7).strip("'").strip();
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

int mariaDB::getIdentityColParams(GString table, int *seed, int * incr)
{
    mariaDB * tmp = new mariaDB(*this);
    tmp->initAll("select IDENT_SEED ( '"+table+"' )");
    *seed = tmp->rowElement(1,1).asInt();

    tmp->initAll("select IDENT_INCR ( '"+table+"' )");
    *incr = tmp->rowElement(1,1).asInt();

    delete tmp  ;
	return 0;
}


int mariaDB::getTriggers(GString table, GString *text)
{
    *text = "";
    GSeq <GString> trgSeq = getTriggerSeq(table);
    for(int i = 1; i <= trgSeq.numberOfElements(); ++i)
    {
        *text += "\n-- Trigger #"+GString(i)+":\n"+trgSeq.elementAtPosition(i);
    }
    return 0;
}

GSeq <GString> mariaDB::getTriggerSeq(GString table)
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


GSeq <IDX_INFO*> mariaDB::getIndexeInfo(GString table)
{                

//    this->initAll("use "+m_strCurrentDatabase);

    GString cmd = "select index_schema, index_name, group_concat(column_name order by seq_in_index) as index_columns, index_type,"
            "case non_unique "
                 "when 1 then 'Not Unique' "
                 "else 'Unique' "
                 "end as is_unique, "
             "table_name "
     "from information_schema.statistics "
     "where table_schema ='"+tabSchema(table)+"' and  table_name ='"+tabName(table)+"' "
     " group by index_schema, index_name,index_type,non_unique,table_name order by index_schema, index_name;";

    m_iReadClobData = 1;
    this->initAll(cmd);
    deb("::getIndexeInfo, cmd: "+cmd);
    deb("::getIndexeInfo, found: "+GString(this->numberOfRows()));

    GSeq <IDX_INFO*> indexSeq;

    IDX_INFO * pIdx;
    GString out;
    for(int  i=1; i<=(int)this->numberOfRows(); ++i )
    {
        pIdx = new IDX_INFO;        
        deb("::getIndexeInfo, i: "+GString(i));

        if( this->rowElement(i, 2).strip("'").upperCase() == "PRIMARY" )pIdx->Type = DEF_IDX_PRIM;
        else if( this->rowElement(i, 5).strip("'").upperCase() == "UNIQUE" ) pIdx->Type = DEF_IDX_UNQ;
        else pIdx->Type = DEF_IDX_DUPL;

        pIdx->Iidx = GString(i);
        pIdx->Schema = this->rowElement(i, 1).strip("'");
        pIdx->Name = this->rowElement(i, 2).strip("'");
        pIdx->Columns = this->rowElement(i, 3).strip("'");
        pIdx->CreateTime = "N/A";
        pIdx->StatsTime = "N/A";
        pIdx->IsDisabled = "N/A";
        pIdx->DeleteRule = "N/A";
        pIdx->StatsTime = "N/A";
        pIdx->Stmt = "N/A";
        out = "";
        if( pIdx->Type == DEF_IDX_PRIM )
        {
            pIdx->Stmt = "ALTER TABLE "+table+" ADD CONSTRAINT PRIMARY KEY ("+pIdx->Columns+");";
        }
        else
        {
            out = "CREATE ";
            if( pIdx->Type == DEF_IDX_UNQ ) out += "UNIQUE ";
            out += "INDEX "+pIdx->Name +" ON ";
            out += table+" ("+pIdx->Columns + ");";
            pIdx->Stmt = out;
        }

        indexSeq.add(pIdx);
        deb("::fillIndexView, i: "+GString(i)+", currIDx: "+GString(i));
    }
    m_iReadClobData = 0;

    return indexSeq;
}


GString mariaDB::getChecks(GString, GString )
{
    return "";
}

GSeq <GString> mariaDB::getChecksSeq(GString, GString)
{
    GSeq <GString> aSeq;
    return aSeq;
}



GString mariaDB::getIndexStatement(int type, GString table, GString indexName)
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

GString mariaDB::getForeignKeyStatement(GString table, GString foreignKeyName )
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





int mariaDB::hasUniqueConstraint(GString tableName)
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


int mariaDB::getUniqueCols(GString table, GSeq <GString> * colSeq)
{
    GSeq <IDX_INFO*> idxSeq = getIndexeInfo(table);

    IDX_INFO * pIdx;
    for( int i = 1; i <= idxSeq.numberOfElements(); ++i )
    {
        pIdx = idxSeq.elementAtPosition(i);
        if( pIdx->Type == DEF_IDX_PRIM || pIdx->Type == DEF_IDX_UNQ )
        {
            *colSeq = pIdx->Columns.split(',');
            return 0;
        }
    }
    return 1;
}

GString mariaDB::tabName(GString table)
{
    table = table.removeAll('\"');
    deb("mariaDB::tabName: "+table);
    if( table.occurrencesOf(".") != 2 && table.occurrencesOf(".") != 1) return "@ErrTabString";
    if( table.occurrencesOf(".") == 2 ) table = table.remove(1, table.indexOf("."));
    return table.subString(table.indexOf(".")+1, table.length()).strip();
}
GString mariaDB::context(GString table)
{
    table.removeAll('\"');
    deb("mariaDB::context for "+table);
    if( table.occurrencesOf(".") != 2 ) return "";
    return table.subString(1, table.indexOf(".")-1);
}

GString mariaDB::tabSchema(GString table)
{
    table.removeAll('\"');
    deb("mariaDB::tabSchema: "+table);
    if( table.occurrencesOf(".") != 2 && table.occurrencesOf(".") != 1) return "@ErrTabString";
    if( table.occurrencesOf(".") == 2 ) table.remove(1, table.indexOf("."));
    return table.subString(1, table.indexOf(".")-1);
}

void mariaDB::createXMLCastString(GString &xmlData)
{
	deb("::createXMLCastString called");
	xmlData = " cast('"+(xmlData)+"' as xml)";
}

int mariaDB::isBinary(unsigned long row, int col)
{
    if( row < 1 || row > allRowsSeq.numberOfElements() ) return 0;
    GRowHdl *aRow;
    aRow = allRowsSeq.elementAtPosition(row);
    if( row < 1 || (unsigned long) col > aRow->elements() ) return 0;
    return aRow->rowElement(col)->isBinary;
}

int  mariaDB::isNull(unsigned long row, int col)
{
    if( row < 1 || row > allRowsSeq.numberOfElements() ) return 0;
    GRowHdl *aRow;
    aRow = allRowsSeq.elementAtPosition(row);
    if( col < 1 || (unsigned long) col > aRow->elements() ) return 0;
    deb("::isNull: "+GString(aRow->rowElement(col)->isNull)+", row: "+GString(row)+", col: "+GString(col));
    return aRow->rowElement(col)->isNull;
}

int mariaDB::isTruncated(unsigned long row, int col)
{
    if( row < 1 || row > allRowsSeq.numberOfElements() ) return 0;
    GRowHdl *aRow;
    aRow = allRowsSeq.elementAtPosition(row);
    if( col < 1 || (unsigned long) col > aRow->elements() ) return 0;
    return aRow->rowElement(col)->isTruncated;
}
void mariaDB::deb(GString fnName, GString txt)
{
    if( m_pGDB ) m_pGDB->debugMsg("mariaDB", m_iMyInstance, "::"+fnName+" "+txt);
}

void mariaDB::deb(GString text)
{
    if( m_pGDB ) m_pGDB->debugMsg("mariaDB", m_iMyInstance, text);
}
void mariaDB::setTruncationLimit(int limit)
{
    //Setting m_iTruncationLimit to 0 -> no truncation
    m_iTruncationLimit = limit;
}
int mariaDB::uploadLongBuffer( GString cmd, GString data, int isBinary )
{
    PMF_UNUSED(cmd);
    PMF_UNUSED(data);
    PMF_UNUSED(isBinary);
    return -1;
}
GString mariaDB::cleanString(GString in)
{
    if( in.length() < 2 ) return in;
    if( in[1UL] == '\'' && in[in.length()] == '\'' ) return in.subString(2, in.length()-2);
    return in;
}
int mariaDB::isLongTypeCol(int i)
{
    if( i < 1 || (unsigned long)i > sqlLongTypeSeq.numberOfElements() ) return 0;
    return sqlLongTypeSeq.elementAtPosition(i);
}
void mariaDB::getResultAsHEX(int asHex)
{
    PMF_UNUSED(asHex);
//TODO
}
void mariaDB::setReadUncommitted(short readUncommitted)
{
    m_iReadUncommitted = readUncommitted;
}

void mariaDB::setGDebug(GDebug *pGDB)
{
    m_pGDB = pGDB;
}
void mariaDB::setDatabaseContext(GString context)
{
    deb("setting context: "+context);
    m_strCurrentDatabase = context;
    this->initAll("use "+m_strCurrentDatabase);
}

int mariaDB::exportAsTxt(int mode, GString sqlCmd, GString table, GString outFile, GSeq <GString>* startText, GSeq <GString>* endText, GString *err)
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
int mariaDB::deleteTable(GString tableName)
{
    tableName.removeAll('\"');
	this->initAll("drop table "+tableName);
    if( sqlCode() && this->getDdlForView(tableName).length())
    {
        this->initAll("drop view "+tabSchema(tableName)+"."+tabName(tableName));
    }
	return sqlCode();
}


GString mariaDB::fillChecksView(GString, int)
{
    return "";
}

mariaDB::RowData::~RowData()
{
    rowDataSeq.removeAll();
}
void mariaDB::RowData::add(GString data)
{
    rowDataSeq.add(data);
}
GString mariaDB::RowData::elementAtPosition(int pos)
{
    if( pos < 1 || pos > (int)rowDataSeq.numberOfElements() ) return "";
    return rowDataSeq.elementAtPosition(pos);
}
int mariaDB::getHeaderData(int pos, GString * data)
{
    if( pos < 1 || pos > (int)headerSeq.numberOfElements()) return 1;
    *data = headerSeq.elementAtPosition(pos);
    return 0;
}

int mariaDB::getRowData(int row, int col, GString * data)
{
    if( row < 1 || row > (int)rowSeq.numberOfElements() ) return 1;
    if( col < 1 || col > (int)rowSeq.elementAtPosition(row)->numberOfElements() ) return 1;
    *data = rowSeq.elementAtPosition(row)->elementAtPosition(col);
    return 0;
}

unsigned long mariaDB::RowData::numberOfElements()
{
    return rowDataSeq.numberOfElements();
}
unsigned long mariaDB::getHeaderDataCount()
{
    return headerSeq.numberOfElements();
}
unsigned long mariaDB::getRowDataCount()
{
    return rowSeq.numberOfElements();
}

int mariaDB::isTransaction()
{
    return m_iIsTransaction;
}

void mariaDB::clearSequences()
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

GString mariaDB::getDdlForView(GString tableName)
{       
    this->setCLOBReader(1);
    this->initAll("SELECT VIEW_DEFINITION FROM INFORMATION_SCHEMA.VIEWS WHERE TABLE_SCHEMA='"+ tabSchema(tableName)+"' AND TABLE_NAME='"+tabName(tableName)+"'");
    this->setCLOBReader(1);
    if( this->numberOfRows() == 0 ) return "";
    return "CREATE VIEW "+tabName(tableName)+" AS "+  cleanString(this->rowElement(1, 1).strip());
}

void mariaDB::setAutoCommmit(int commit)
{
}

int mariaDB::execByDescriptor( GString cmd, GSeq <GString> *dataSeq)
{
    PMF_UNUSED(cmd);
    PMF_UNUSED(dataSeq);
    return -1;
}
int mariaDB::execByDescriptor( GString cmd, GSeq <GString> *dataSeq, GSeq <int> *typeSeq,
                                GSeq <short>* sqlVarLengthSeq, GSeq <int> *forBitSeq )
{
    PMF_UNUSED(cmd);
    PMF_UNUSED(dataSeq);
    PMF_UNUSED(typeSeq);
    PMF_UNUSED(sqlVarLengthSeq);
    PMF_UNUSED(forBitSeq);
    return -1;
}
long mariaDB::uploadBlob(GString cmd, char * buffer, long size)
{
    PMF_UNUSED(cmd);
    PMF_UNUSED(buffer);
    PMF_UNUSED(size);
    return -1;
}

long mariaDB::retrieveBlob( GString cmd, GString &blobFile, int writeFile )
{
    PMF_UNUSED(cmd);
    PMF_UNUSED(blobFile);
    PMF_UNUSED(writeFile);
    return -1;
}

void mariaDB::currentConnectionValues(CON_SET * conSet)
{
    conSet->DB = m_strDB;
    conSet->Host = m_strHost;
    conSet->PWD = m_strPWD;
    conSet->UID = m_strUID;
    conSet->Port = m_iPort;
}

GString mariaDB::lastSqlSelectCommand()
{
    return m_strLastSqlSelectCommand;;
}

TABLE_PROPS mariaDB::getTableProps(GString tableName)
{
    TABLE_PROPS tableProps;
    tableProps.init();

    this->initAll("SELECT TABLE_TYPE FROM information_schema.TABLES where TABLE_SCHEMA='"+ tabSchema(tableName)+"' AND TABLE_NAME='"+tabName(tableName)+"'");
    if( rowElement(1,1).strip("'") == "VIEW" ) tableProps.TableType = TYPE_TYPED_VIEW;
    else if( rowElement(1,1).strip("'") == "BASE TABLE" ) tableProps.TableType = TYPE_TYPED_TABLE;
    else if( rowElement(1,1).strip("'") == "SYSTEM VIEW" ) tableProps.TableType = TYPE_TYPED_VIEW;

    return tableProps;
}

int mariaDB::tableIsEmpty(GString tableName)
{
    GString err = this->initAll("select top 1 * from " + tableName);
    if( this->numberOfRows() == 0 || err.length() ) return 1;
    return 0;
}

int mariaDB::deleteViaFetch(GString tableName, GSeq<GString> * colSeq, int rowCount, GString whereClause)
{
    GString cmd = "DELETE TOP ("+GString(rowCount)+") FROM "+tableName;
    if( whereClause.length() ) cmd += " WHERE "+whereClause;
    GString err = this->initAll(cmd);    
    deb("deleteViaFetch: cmd: "+cmd+"\nErr: "+err);
    if( err.length() ) return sqlCode();
    this->commit();
    return 0;
}


void mariaDB::setCLOBReader(short readCLOBData )
{
    m_iReadClobData = readCLOBData;
}

/******************************************************************
 *
 *  ifdef: Use QT for some tasks
 *
 *****************************************************************/
#ifdef  QT4_DSQL
GString mariaDB::getIdenticals(GString table, QWidget* parent, QListWidget *pLB, short autoDel)
{

	GString message = "SELECT * FROM "+table;
	GString retString = "";

    GString countCmd, out, data;
    deb("::getIdenticals, cmd: "+message);

    MYSQL_RES *res;
    MYSQL_ROW row;
    MYSQL_FIELD *fld;

    mariaDB * tmpODBC = new mariaDB(*this);
    mariaDB * delODBC = new mariaDB(*this);


    if( mysql_query(m_mariaSql, (const char*) message) )
    {
        return sqlError();
    }
    res = mysql_store_result(m_mariaSql);
    resetAllSeq();

    m_iNumberOfColumns = mysql_field_count(m_mariaSql);

    if( m_iNumberOfColumns == 0 ) return sqlError();

    m_strLastSqlSelectCommand = message;
    m_iNumberOfRows = 0;
    for(unsigned int i = 0; (fld = mysql_fetch_field(res)); i++)
    {
        hostVarSeq.add(fld->name);
        sqlTypeSeq.add(fld->type);
        setSimpleColType(fld->type);
    }
    int blockSize = 2000;
    unsigned long lns =0, perc = 0, pass = 1;
    QProgressDialog * apd = NULL;
    apd = new QProgressDialog(GString("Searching in "+table), "Cancel", 0, blockSize, parent);
    apd->setWindowModality(Qt::WindowModal);
    apd->setValue(1);

    while ((row = mysql_fetch_row(res)))
    {
        if( !(perc % 10) ) apd->setValue(perc);
        if( apd->wasCanceled() )
        {
           retString = "QUIT";
           break;
        }
        countCmd = "";
        for ( int i = 0; i < m_iNumberOfColumns; i++ )
        {
            if(isXMLCol(i+1) || isLOBCol(i+1))continue;

            if(row[i] == NULL) data = " IS NULL";
            else if( isNumType(i+1)) data = "="+GString(row[i]);
            else data = "='"+GString(row[i])+"'";
            countCmd += hostVariable(i+1)+data+ " AND ";
        }
        countCmd = countCmd .stripTrailing(" AND ");
        m_iNumberOfRows++;
        deb("getIdenticals: CountCmd: "+countCmd);
        tmpODBC->initAll( "SELECT COUNT(*) FROM "+table+" WHERE "+countCmd);
        if(tmpODBC->rowElement(1,1).strip("'").asInt() > 1)
        {
            out = table +": Found "+GString(tmpODBC->rowElement(1,1))+" results for "+countCmd;
            writeToLB(pLB, out);
        }
        for(int i=1; i<tmpODBC->rowElement(1,1).asInt() && autoDel; ++i)
        {
            delODBC->currentCursor("SELECT * FROM "+table+" WHERE "+countCmd, "DELETE FROM "+table, 1, 0);
        }

    }
    writeToLB(pLB, "-- Checked: "+table);
    if( apd )
    {
        apd->setValue(blockSize);
        delete apd;
    }

    delete tmpODBC;
    delete delODBC;
    deb("::getIdenticals, err fetch: "+sqlError());
    return sqlError();
}


void mariaDB::writeToLB(QListWidget * pLB, GString message)
{
    for( int i = 0; i < pLB->count(); ++i )
        if( GString(pLB->item(i)->text()) == message ) return;
    new QListWidgetItem(message, pLB);
}


GString  mariaDB::fillIndexView(GString table, QWidget* parent, QTableWidget *pWdgt)
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
void mariaDB::createIndexRow(QTableWidget *pWdgt, int row,
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


