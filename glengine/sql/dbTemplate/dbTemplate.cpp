    //
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt
//

#ifndef _DBTEMPLATE_
#include <dbTemplate.hpp>
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
static int m_dbTemplateobjCounter = 0;

#define XML_MAX 250



/***********************************************************************
 * This class can either be instatiated or loaded via dlopen/loadlibrary
 ***********************************************************************/

//Define functions with C symbols (create/destroy instance).
#ifndef MAKE_VC
extern "C" dbTemplate* create()
{
    //printf("Calling dbTemplate creat()\n");
    return new dbTemplate();
}
extern "C" void destroy(dbTemplate* pODBCSQL)
{
   if( pODBCSQL ) delete pODBCSQL ;
}
#else
extern "C" __declspec( dllexport ) dbTemplate* create()
{
    //printf("Calling dbTemplate creat()\n");
    _flushall();
    return new dbTemplate();
}	
extern "C" __declspec( dllexport ) void destroy(dbTemplate* pODBCSQL)
{
    if( pODBCSQL ) delete pODBCSQL ;
}	
#endif
/***********************************************************
 * CLASS
 **********************************************************/

dbTemplate::dbTemplate(dbTemplate  const &o)
{
	m_iLastSqlCode = 0;
    m_dbTemplateobjCounter++;
    m_iTruncationLimit = 500;
    m_iMyInstance = m_dbTemplateobjCounter;
    m_pGDB = o.m_pGDB;
    deb("CopyCtor start");

    m_odbcDB  = DBTEMPLATE;
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

    deb("CopyCtor: calling connect, connection data: "+m_strDB+", uid: "+m_strUID+", host: "+m_strHost+", port: "+GString(m_iPort));
    this->connect(m_strDB, m_strUID, m_strPWD, m_strHost, GString(m_iPort));
    deb("Copy CTor done");
}

dbTemplate::dbTemplate()
{
    m_dbTemplateobjCounter++;
    m_iTruncationLimit = 500;
    m_iMyInstance = m_dbTemplateobjCounter;

	m_iLastSqlCode = 0;
    m_iIsTransaction = 0;	
    m_odbcDB  = DBTEMPLATE;

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
    printf("dbTemplate[%i]> DefaultCtor done. Port: %i\n", m_iMyInstance, m_iPort );
}
dbTemplate* dbTemplate::clone() const
{
    return new dbTemplate(*this);
}

dbTemplate::~dbTemplate()
{
    deb("Dtor start, current count: "+GString(m_dbTemplateobjCounter));
    disconnect();

    m_dbTemplateobjCounter--;
    deb("Dtor, clearing RowData...");
    clearSequences();
    deb("Dtor done, current count: "+GString(m_dbTemplateobjCounter));
}

int dbTemplate::getConnData(GString * db, GString *uid, GString *pwd) const
{
    *db = m_strDB;
    *uid = m_strUID;
    *pwd = m_strPWD;
	return 0;
}


GString dbTemplate::connect(GString db, GString uid, GString pwd, GString host, GString port)
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

    return "CONNECT not implemented";
}

int dbTemplate::disconnect()
{	

    deb("Disconnecting and closing...");
    deb("Disconnecting and closing...calling PQfinish...");
    deb("Disconnecting and closing...done.");
	return 0;
}
GString dbTemplate::initAll(GString message, unsigned long maxRows,  int getLen)
{    

    deb("::initAll, start. msg: "+message);

    GRowHdl * pRow;

    resetAllSeq();

    m_strLastSqlSelectCommand = message;
    m_iNumberOfRows = 0;
    m_iLastSqlCode = 0;


    return sqlError();
}

int dbTemplate::commit()
{    
	return 0;
}

int dbTemplate::rollback()
{
    //m_db.rollback();
	return 0;
}


int dbTemplate::initRowCrs()
{
	deb("::initRowCrs");
    if( allRowsSeq.numberOfElements() == 0 ) return 1;
    m_pRowAtCrs = allRowsSeq.initCrs();
    return 0;
}
int dbTemplate::nextRowCrs()
{	
    if( m_pRowAtCrs == NULL ) return 1;
    m_pRowAtCrs = allRowsSeq.setCrsToNext();
    return 0;
}
long  dbTemplate::dataLen(const short & pos)
{
    if( pos < 1 || pos > (short) sqlLenSeq.numberOfElements()) return 0;
    deb("Len at "+GString(pos)+": "+GString(sqlLenSeq.elementAtPosition(pos)));    
    return sqlLenSeq.elementAtPosition(pos);
}

int dbTemplate::isDateTime(int pos)
{
    if( pos < 1 || pos > (int)sqlTypeSeq.numberOfElements() ) return 0;
//    switch( sqlTypeSeq.elementAtPosition(pos) )
//    {
//            return 1;
//    }
    return 0;
}

int dbTemplate::isNumType(int pos)
{
    if( pos < 1 || pos > (int) sqlTypeSeq.numberOfElements() ) return 0;
//    switch( sqlTypeSeq.elementAtPosition(pos) )
//    {
//            return 1;
//    }
    return 0;
}

int dbTemplate::isXMLCol(int i)
{
    if( i < 1 || (unsigned long)i > xmlSeq.numberOfElements() ) return 0;
    return xmlSeq.elementAtPosition(i);
}


int dbTemplate::hasForBitData()
{
    return 0;
}
int dbTemplate::isForBitCol(int i)
{
    if( i < 1 || (unsigned long)i > sqlForBitSeq.numberOfElements() ) return 0;
    return sqlForBitSeq.elementAtPosition(i);
}
int dbTemplate::isBitCol(int i)
{
    if( i < 1 || (unsigned long)i > sqlBitSeq.numberOfElements() ) return 0;
    return sqlBitSeq.elementAtPosition(i);
}

unsigned int dbTemplate::numberOfColumns()
{
    //deb("::numberOfColumns called");
    return m_iNumberOfColumns;
}

unsigned long dbTemplate::numberOfRows()
{
    return m_iNumberOfRows;
}
GString dbTemplate::dataAtCrs(int col)
{
    if( m_pRowAtCrs == NULL ) return "@CrsNotOpen";
    if( col < 1 || (unsigned long)col > m_pRowAtCrs->elements() ) return "@OutOfReach";
    return m_pRowAtCrs->rowElementData(col);
}
GString dbTemplate::rowElement( unsigned long row, int col)
{
    //Overload 1
    //tm("::rowElement for "+GString(line)+", col: "+GString(col));
    if( row < 1 || row > allRowsSeq.numberOfElements() ) return "@OutOfReach";
    GRowHdl *aRow;
    aRow = allRowsSeq.elementAtPosition(row);
    if( row < 1 || (unsigned long) col > aRow->elements() ) return "OutOfReach";
    return aRow->rowElementData(col);
}
int dbTemplate::positionOfHostVar(const GString& hostVar)
{
    unsigned long i;
    for( i = 1; i <= hostVarSeq.numberOfElements(); ++i )
    {
        if( hostVarSeq.elementAtPosition(i) == hostVar ) return i;
    }
    return 0;
}
GString dbTemplate::rowElement( unsigned long row, GString hostVar)
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

GString dbTemplate::hostVariable(int col)
{
    //deb("::hostVariable called, col: "+GString(col));
    if( col < 1 || col > (short) hostVarSeq.numberOfElements() ) return "@hostVariable:OutOfReach";
    return hostVarSeq.elementAtPosition(col);
}

GString dbTemplate::currentCursor(GString filter, GString command, long curPos, short commitIt, GSeq <GString> *fileList, GSeq <long> *lobType)
{
    deb(__FUNCTION__, "filter: "+filter+", cmd: "+command);
    int rc;

    if( curPos < 0 ) return "CurrentCursor(): POS < 0";
    deb(__FUNCTION__, "done, rc: "+GString(rc));
    return rc ? m_strLastError : "";
}



int dbTemplate::sqlCode()
{
	return m_iLastSqlCode;
}
GString dbTemplate::sqlError()
{    
    m_iLastSqlCode = 0;
    deb("::sqlError: m_iLastSqlCode = "+GString(m_iLastSqlCode));
    return m_strLastError;
}		

int dbTemplate::getTabSchema()
{
    return 0;
}
int dbTemplate::getTables(GString schema)
{
    return 0;
}
void dbTemplate::convToSQL( GString& input )
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

int dbTemplate::getAllTables(GSeq <GString > * tabSeq, GString filter)
{
	return 0;
}
short dbTemplate::sqlType(const short & col)
{
   if( col < 1 || col > (short) sqlTypeSeq.numberOfElements() ) return -1;
   else return sqlTypeSeq.elementAtPosition(col);
}
short dbTemplate::sqlType(const GString & colName)
{
   for( short i = 1; i <= (short) hostVarSeq.numberOfElements(); ++ i )
   {
      if( hostVarSeq.elementAtPosition(i) == colName ) return sqlTypeSeq.elementAtPosition(i);
   }
   return -1;
}
GString dbTemplate::realName(const short & sqlType)
{
//	switch(sqlType)
//	{
//            return "Numeric";
			
//            return "Time";
			
//		default:
//			return "'string'";
//	}
	return "";
}

int dbTemplate::loadFileIntoBuf(GString fileName, char** fileBuf, int *size)
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


void dbTemplate::impExpLob()
{
}


long dbTemplate::uploadBlob(GString cmd, GSeq <GString> *fileSeq, GSeq <long> *lobType)
{
    deb("::uploadBlob, cmd: "+cmd);
    return 0;
}

GString dbTemplate::descriptorToFile( GString cmd, GString &blobFile, int * outSize )
{
    deb("::descriptorToFile, cmd: "+cmd);
    *outSize = 0;

    dbTemplate ps(*this);
    GSeq<GString> cmdSeq = cmd.split(' ');
    if( cmdSeq.numberOfElements() < 2) return "Invalid sqlCmd: "+cmd;

    return sqlError();
}

int dbTemplate::writeToFile(GString fileName, GString data, int len)
{
    FILE * f;
    f = fopen(fileName, "ab");
    int written = fwrite((char*) data,1,len,f);
    fclose(f);
    return written;
}

signed long dbTemplate::getCost()
{
    return -1;
}

int dbTemplate::simpleColType(int i)
{
    if( i < 1 || (unsigned long)i > simpleColTypeSeq.numberOfElements() ) return CT_UNKNOWN;
    if(isXMLCol(i)) return CT_XML;
    if(isLOBCol(i)) return CT_DBCLOB;
    if(isNumType(i)) return CT_INTEGER;
    if(isDateTime(i)) return CT_DATE;
    return CT_STRING;
}

int dbTemplate::isLOBCol(int i)
{
    if( i < 1 || (unsigned long)i > sqlTypeSeq.numberOfElements() ) return 0;
//    switch(sqlTypeSeq.elementAtPosition(i))
//    {
//    }
    return 0;
}

int dbTemplate::isNullable(int i)
{
    if( i < 1 || (unsigned long)i > sqlIndVarSeq.numberOfElements() ) return 1;
    return sqlIndVarSeq.elementAtPosition(i);
}


int dbTemplate::isFixedChar(int i)
{
    if( i < 1 || (unsigned long)i > sqlTypeSeq.numberOfElements() ) return 0;
    return 0;
}

int dbTemplate::getDataBases(GSeq <CON_SET*> *dbList)
{
    return 0;
    CON_SET * pCS;
    return 0;
}
void dbTemplate::setCurrentDatabase(GString db)
{
    m_strCurrentDatabase = db;
}
GString dbTemplate::currentDatabase()
{
    return m_strCurrentDatabase;
}
unsigned long dbTemplate::getResultCount(GString cmd)
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
void dbTemplate::resetAllSeq()
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
int dbTemplate::getColSpecs(GString table, GSeq<COL_SPEC*> *specSeq)
{

    GString tabSchema = this->tabSchema(table);
    GString tabName   = this->tabName(table);
    COL_SPEC *cSpec;

    this->setCLOBReader(1); //This will read any CLOB into fieldData

    this->setCLOBReader(0);
    return 0;
}

int dbTemplate::getIdentityColParams(GString table, int *seed, int * incr)
{
    dbTemplate * tmp = new dbTemplate(*this);
    delete tmp  ;
	return 0;
}


int dbTemplate::getTriggers(GString table, GString *text)
{
    *text = "";
    GSeq <GString> trgSeq = getTriggerSeq(table);
    for(int i = 1; i <= trgSeq.numberOfElements(); ++i)
    {
        *text += "\n-- Trigger #"+GString(i)+":\n"+trgSeq.elementAtPosition(i);
    }
    return 0;
}

GSeq <GString> dbTemplate::getTriggerSeq(GString table)
{
    deb("getTriggerSeq start");
    GString tabSchema = this->tabSchema(table);
    GString tabName   = this->tabName(table);
    GString trig;

    GSeq <GString> triggerSeq;

    this->setCLOBReader(1); //This will read any CLOB into fieldData

    this->setCLOBReader(0);
    return triggerSeq;
}


GSeq <IDX_INFO*> dbTemplate::getIndexeInfo(GString table)
{
    GSeq <IDX_INFO*> indexSeq;
    return indexSeq;
}


GString dbTemplate::getChecks(GString, GString )
{
    return "";
}

GSeq <GString> dbTemplate::getChecksSeq(GString, GString)
{
    GSeq <GString> aSeq;
    return aSeq;
}




int dbTemplate::hasUniqueConstraint(GString tableName)
{

    GSeq <GString>  colSeq;
    if( this->getUniqueCols(tableName, &colSeq)) return 0;
    return colSeq.numberOfElements();
}


int dbTemplate::getUniqueCols(GString table, GSeq <GString> * colSeq)
{
    return 0;
}

GString dbTemplate::tabName(GString table)
{
    return "";
}
GString dbTemplate::context(GString table)
{
    return "";
}

GString dbTemplate::tabSchema(GString table)
{
    return "";
}

void dbTemplate::createXMLCastString(GString &xmlData)
{
}

int dbTemplate::isBinary(unsigned long row, int col)
{
    if( row < 1 || row > allRowsSeq.numberOfElements() ) return 0;
    GRowHdl *aRow;
    aRow = allRowsSeq.elementAtPosition(row);
    if( row < 1 || (unsigned long) col > aRow->elements() ) return 0;
    return aRow->rowElement(col)->isBinary;
}

int  dbTemplate::isNull(unsigned long row, int col)
{
    if( row < 1 || row > allRowsSeq.numberOfElements() ) return 0;
    GRowHdl *aRow;
    aRow = allRowsSeq.elementAtPosition(row);
    if( col < 1 || (unsigned long) col > aRow->elements() ) return 0;
    deb("::isNull: "+GString(aRow->rowElement(col)->isNull)+", row: "+GString(row)+", col: "+GString(col));
    return aRow->rowElement(col)->isNull;
}

int dbTemplate::isTruncated(unsigned long row, int col)
{
    if( row < 1 || row > allRowsSeq.numberOfElements() ) return 0;
    GRowHdl *aRow;
    aRow = allRowsSeq.elementAtPosition(row);
    if( row < 1 || (unsigned long) col > aRow->elements() ) return 0;
    return aRow->rowElement(col)->isTruncated;
}
void dbTemplate::deb(GString fnName, GString txt)
{
    if( m_pGDB ) m_pGDB->debugMsg("dbTemplate", m_iMyInstance, "::"+fnName+" "+txt);
}

void dbTemplate::deb(GString text)
{
    if( m_pGDB ) m_pGDB->debugMsg("dbTemplate", m_iMyInstance, text);
}
void dbTemplate::setTruncationLimit(int limit)
{
    //Setting m_iTruncationLimit to 0 -> no truncation
    m_iTruncationLimit = limit;
}
int dbTemplate::uploadLongBuffer( GString cmd, GString data, int isBinary )
{
    PMF_UNUSED(cmd);
    PMF_UNUSED(data);
    PMF_UNUSED(isBinary);
    return -1;
}
GString dbTemplate::cleanString(GString in)
{
    if( in.length() < 2 ) return in;
    if( in[1UL] == '\'' && in[in.length()] == '\'' ) return in.subString(2, in.length()-2);
    return in;
}
int dbTemplate::isLongTypeCol(int i)
{
    if( i < 1 || (unsigned long)i > sqlLongTypeSeq.numberOfElements() ) return 0;
    return sqlLongTypeSeq.elementAtPosition(i);
}
void dbTemplate::getResultAsHEX(int asHex)
{
    PMF_UNUSED(asHex);
}
void dbTemplate::setReadUncommitted(short readUncommitted)
{
    m_iReadUncommitted = readUncommitted;
}

void dbTemplate::setGDebug(GDebug *pGDB)
{
    m_pGDB = pGDB;
}
void dbTemplate::setDatabaseContext(GString context)
{
    deb("setting context: "+context);
}

int dbTemplate::exportAsTxt(int mode, GString sqlCmd, GString table, GString outFile, GSeq <GString>* startText, GSeq <GString>* endText, GString *err)
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


    for(unsigned long i = 1; i <= endText->numberOfElements(); ++i) aSeq.add(endText->elementAtPosition(i));
	aSeq.add("-- SET IDENTITY_INSERT "+table+" OFF");
    f.append(&aSeq);

    return 0;
}
int dbTemplate::deleteTable(GString tableName)
{
    tableName.removeAll('\"');
	this->initAll("drop table "+tableName);
    if( sqlCode() && this->getDdlForView(tableName).length())
    {
        this->initAll("drop view "+tabSchema(tableName)+"."+tabName(tableName));
    }
	return sqlCode();
}


GString dbTemplate::fillChecksView(GString, int)
{
    return "";
}

dbTemplate::RowData::~RowData()
{
    rowDataSeq.removeAll();
}
void dbTemplate::RowData::add(GString data)
{
    rowDataSeq.add(data);
}
GString dbTemplate::RowData::elementAtPosition(int pos)
{
    if( pos < 1 || pos > (int)rowDataSeq.numberOfElements() ) return "";
    return rowDataSeq.elementAtPosition(pos);
}
int dbTemplate::getHeaderData(int pos, GString * data)
{
    if( pos < 1 || pos > (int)headerSeq.numberOfElements()) return 1;
    *data = headerSeq.elementAtPosition(pos);
    return 0;
}

int dbTemplate::getRowData(int row, int col, GString * data)
{
    if( row < 1 || row > (int)rowSeq.numberOfElements() ) return 1;
    if( col < 1 || col > (int)rowSeq.elementAtPosition(row)->numberOfElements() ) return 1;
    *data = rowSeq.elementAtPosition(row)->elementAtPosition(col);
    return 0;
}

unsigned long dbTemplate::RowData::numberOfElements()
{
    return rowDataSeq.numberOfElements();
}
unsigned long dbTemplate::getHeaderDataCount()
{
    return headerSeq.numberOfElements();
}
unsigned long dbTemplate::getRowDataCount()
{
    return rowSeq.numberOfElements();
}

int dbTemplate::isTransaction()
{
    return m_iIsTransaction;
}

void dbTemplate::clearSequences()
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

GString dbTemplate::getDdlForView(GString tableName)
{       
    return "";
}

void dbTemplate::setAutoCommmit(int commit)
{
}

int dbTemplate::execByDescriptor( GString cmd, GSeq <GString> *dataSeq)
{
    PMF_UNUSED(cmd);
    PMF_UNUSED(dataSeq);
    return -1;
}
int dbTemplate::execByDescriptor( GString cmd, GSeq <GString> *dataSeq, GSeq <int> *typeSeq,
                                GSeq <short>* sqlVarLengthSeq, GSeq <int> *forBitSeq )
{
    PMF_UNUSED(cmd);
    PMF_UNUSED(dataSeq);
    PMF_UNUSED(typeSeq);
    PMF_UNUSED(sqlVarLengthSeq);
    PMF_UNUSED(forBitSeq);
    return -1;
}
long dbTemplate::uploadBlob(GString cmd, char * buffer, long size)
{
    PMF_UNUSED(cmd);
    PMF_UNUSED(buffer);
    PMF_UNUSED(size);
    return -1;
}

long dbTemplate::retrieveBlob( GString cmd, GString &blobFile, int writeFile )
{
    PMF_UNUSED(cmd);
    PMF_UNUSED(blobFile);
    PMF_UNUSED(writeFile);
    return -1;
}

void dbTemplate::currentConnectionValues(CON_SET * conSet)
{
    conSet->DB = m_strDB;
    conSet->Host = m_strHost;
    conSet->PWD = m_strPWD;
    conSet->UID = m_strUID;
    conSet->Port = m_iPort;
}

GString dbTemplate::lastSqlSelectCommand()
{
    return m_strLastSqlSelectCommand;;
}

TABLE_PROPS dbTemplate::getTableProps(GString tableName)
{
    TABLE_PROPS tableProps;
    tableProps.init();
    return tableProps;
}

int dbTemplate::tableIsEmpty(GString tableName)
{
    GString err = this->initAll("select top 1 * from " + tableName);
    if( this->numberOfRows() == 0 || err.length() ) return 1;
    return 0;
}

int dbTemplate::deleteViaFetch(GString tableName, GSeq<GString> * colSeq, int rowCount, GString whereClause)
{
    GString cmd = "DELETE TOP ("+GString(rowCount)+") FROM "+tableName;
    if( whereClause.length() ) cmd += " WHERE "+whereClause;
    GString err = this->initAll(cmd);    
    deb("deleteViaFetch: cmd: "+cmd+"\nErr: "+err);
    if( err.length() ) return sqlCode();
    this->commit();
    return 0;
}


void dbTemplate::setCLOBReader(short readCLOBData )
{
    m_iReadClobData = readCLOBData;
}

/******************************************************************
 *
 *  ifdef: Use QT for some tasks
 *
 *****************************************************************/
#ifdef  QT4_DSQL
GString dbTemplate::getIdenticals(GString table, QWidget* parent, QListWidget *pLB, short autoDel)
{

	GString message = "SELECT * FROM "+table;
	GString retString = "";
	deb("::getIdenticals, cmd: "+message);


    return retString;
}

void dbTemplate::writeToLB(QListWidget * pLB, GString message)
{
    for( int i = 0; i < pLB->count(); ++i )
        if( GString(pLB->item(i)->text()) == message ) return;
    new QListWidgetItem(message, pLB);
}


GString  dbTemplate::fillIndexView(GString table, QWidget* parent, QTableWidget *pWdgt)
{
    PMF_UNUSED(parent);
	//Clear data:
	while( pWdgt->rowCount() ) pWdgt->removeRow(0);
	
	GString id, name, cols, unq, crt, mod, dis;
	int indexID = -1;
    //this->initAll("use "+m_strCurrentDatabase);
	
    GString cmd = "";
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
void dbTemplate::createIndexRow(QTableWidget *pWdgt, int row,
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


