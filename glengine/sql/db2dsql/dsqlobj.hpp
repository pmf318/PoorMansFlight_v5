
//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt
//


#ifndef _DSQLOBJ_
#define _DSQLOBJ_

#ifdef  __IBMCPP__
#define MAKE_VA
#define NO_QT
#endif

#ifdef  _MSC_VER
#ifndef MAKE_VC
#define MAKE_VC
#endif
#endif

#ifndef _IDSQL_
#include <idsql.hpp>
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
#ifdef Makedsqlobj
#define dsqlobjExp_Imp _Export
#else
#define dsqlobjExp_Imp _Import
//  #pragma library( "dsqlobj.LIB" )
#pragma library( "GLEngine.LIB" )
#endif
#endif
//************** Visual Age END *****************

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

//End

struct SomeLOBFile {
     sqluint32 name_length;
     sqluint32 data_length;
     sqluint32 file_options;
     char      name[2048];
};

// ******************************* CLASS **************************** //
#ifdef Makedsqlobj
class dsqlobjExp_Imp dsqlobj
        #else
class dsqlobj : public IDSQL
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
    void clearCommSequences();

    GSeq <GString> hostVarSeq;
    GSeq <short>   sqlTypeSeq;
    GSeq <short>   sqlVarLengthSeq;
    GSeq <short>   sqlIndVarSeq;
    GSeq <long>    sqlLenSeq;

    GSeq <short>   sqlForBitSeq;
    GSeq <short>   sqlLongTypeSeq;

    GSeq <short>   simpleColTypeSeq;

    GSeq <CON_SET*> m_seqConSet;

    GRowHdl *m_pRowAtCrs;

    GString m_strDB;
    GString m_strUID;
    GString m_strPWD;
    GString m_strHost;
    int m_iPort;


    int m_iMyInstance;
    unsigned long iNumberOfColumns;
    unsigned long iNumberOfRows;
    short connectOK;
    short iStop;
    short rsOK;
    short iEmbrace;
    short m_iCharForBit;
    int iCommit;
    signed long iErrCode;
    signed long iCost;
    signed long iAffectedRows;
    short m_iReadUncommitted;

    short iCommandOK;
    short m_iReadCLOBs;
    int m_iTruncationLimit;
    int m_iGetResultAsHEX;
    GString m_strLastSqlSelectCommand;

    struct sqlda * _crsSqldaptr;
    /* Pretending to be precompiler....*/
    typedef struct {
        unsigned long    name_length;
        unsigned long    data_length;
        unsigned long    file_options;
        char    name[255];
    } myFile;


    struct lob {
        long length;
        char *data;
    } *lobPointer;

    struct loc_varchar {
        short length;
        char *data;
    } *locVarcharPointer;

    myFile mf;
    short someDummy;


#ifndef NO_QT
    QMessageBox *threadBox;
#endif

    class SqlThread : public GThread
    {
    public:
        virtual void run();
        void setMain( dsqlobj * aDSQL ) { myDSQL = aDSQL; }
    private:
        dsqlobj * myDSQL;
    };
    int _threadActive;
    GDebug * m_pGDB;

private:
    int prepSQLDAForLOBs(struct SomeLOBFile *pLobFile, struct sqlda **sqlDA, GString fileName, int lobType, int col);
    void clearAllSequences();
    long setSqldaToLOB(struct sqlda* sqldaptr, short idx, int lobType);
    int getIdentityColParams(GString table, int *start, int * incr);
    GString tabSchema(GString table);
    GString tabName(GString table);
    GString readGraphicData(struct sqlda* sqldaptr, short col, int offset = 0);
    GString readGraphicDataExp(struct sqlda* sqldaptr, short col, int offset);
    GString convertGraphicString(const char* src, int lng);
    unsigned int hexToInt(char c);
    GString setIndexColUse(GString indSchema, GString indName, IDX_INFO* pIdx);
    void setForKeyDetails(GString table, IDX_INFO* pIdx);
    void cleanLobFileSeq(GSeq<SomeLOBFile*> *lobFileSeq);
    int minColSeparator(GString col);
    GString getForeignKeyStatement(GString table, GString keyFilter );
    GString deleteByCursor(GString filter, GString command, long deleteCount, short commitIt = 1);

public:
    VCExport      dsqlobj(short commit = 1, short setEmbrace = 1, short charForBit = 4);
    VCExport      ~dsqlobj();
    VCExport      dsqlobj( dsqlobj const& o);
    VCExport      dsqlobj* clone() const;



    VCExport      ODBCDB getDBType(){ return DB2; }
    VCExport      GString getDBTypeName(){ return "DB2"; }

    //VCExport     dsqlobj &operator=(dsqlobj aDSQL);
    /******************************************************
        * Declaring allRowsSeq as public:
        * A more object-oriented way would be to use a copy constructor
        * for GSeq. On the other hand, allRowsSeq can get LARGE,
        * and I fear repercussions on performance.
        */
    /***************** NOT ANYMORE, see initRowCrs(), nextRowCrs() *****/
    GSeq <GRowHdl* > allRowsSeq;

    /******** Stuff for QT **********************************/
#ifdef  QT4_DSQL
    VCExport GString  getIdenticals(GString table, QWidget * parent, QListWidget * pLB, short autoDel);
    VCExport GString    countOccurrences(GString table, struct sqlda * sqldaptr, QListWidget *pLB, short autoDel);
    VCExport void     writeToLB(QListWidget * aLB, GString message);
#endif
    VCExport GString fillChecksView(GString table, int showRaw = 0);

#ifndef NO_QT
    GSeq <QListViewItem*> lvItemSeq;
    VCExport GString       initAll(GString message, QListView * pLV, unsigned long maxLines = 0, short showRowNr = 0 );
    VCExport void          writeToLB(QListBox * aLB, GString message);
    VCExport GString       getIdenticals(GString table, QListBox * aLB, QLabel * info, short autoDel);
    VCExport short         countOccurrences(GString table, struct sqlda * sqldaptr, QListBox * aLB, short autoDel);

#endif
    /******** Stuff for QT END ******************************/
    VCExport      void setCLOBReader(short readCLOBData = 0);
    VCExport signed long   countLines(GString message);
    VCExport      void setEmbrace(short embChars = 1);
    VCExport      void setReadUncommitted(short readUncommitted = 1);
    VCExport      void setCommit(short commit = 1);
    VCExport      GString version();
    VCExport      int commit();
    VCExport      int rollback();
    VCExport      short bindIt(GString bndFile, GString database,
                               GString user, GString pwd, GString msgFile);

    VCExport      GString connect(GString dataBase, GString uid, GString pwd, GString nodeName, GString port = "");
    VCExport      int disconnect();



    VCExport      GString initAll(GString message, unsigned long maxLines = 0, int getLen = 0);
    VCExport      void emptyLVSeq();
    VCExport      GString sqlError();
    VCExport      int sqlCode();
    VCExport      short init_da(struct sqlda **sqldaptr, short cols, short lobs = 0);
    VCExport      int alloc_host_vars (struct sqlda * sqldaptr);
    VCExport      void free_da(struct sqlda * sqldaptr);
    VCExport      GString setFldData(struct sqlda *sqldaptr, short col );
    VCExport      void setCellData(DATA_CELL*pCell, struct sqlda* sqldaptr, short col );
    VCExport      void tm(GString message);
    VCExport      void mb(GString message);

    VCExport      unsigned int  numberOfColumns(); //{ return iNumberOfColumns;};
    VCExport      unsigned long  numberOfRows();
    VCExport      GString        realName(const short & sqlType);
    VCExport      void           convToSQL( GString& input );

    VCExport      GString rowElement(unsigned long row, int col);
    VCExport      GString rowElement(unsigned long row, GString hostVar);
    VCExport      virtual int    positionOfHostVar(const GString& hostVar);
    VCExport      GString        hostVariable(int pos);
    VCExport      short          sqlType(const short & col);
    VCExport      short          sqlType(const GString & name);
    VCExport      long           dataLen(const short & pos);

    VCExport      short          sqlVarLength(const short & col);
    VCExport      GString        fullLine(const unsigned long & index, GString sep = " | ");

    VCExport      GString        currentCursor(GString filter, GString command,
                                               long curPos, short commitIt = 1,
                                               GSeq <GString> *fileList = NULL, GSeq <long> *lobType = NULL);


    VCExport      int         deleteTable(GString tableName);
    VCExport      void          setStopAll(short stop=1){ iStop = stop;}


    VCExport      int           getAllTables(GSeq <GString > * tabSeq, GString filter = ""){ PMF_UNUSED(tabSeq); PMF_UNUSED(filter); return 1;}

    VCExport      signed long   openRS(GString message); //FOR READ ONLY?????
    VCExport      signed long   getNextRS(short col, GString &value);
    VCExport      signed long   nextRS();
    VCExport      signed long   closeRS();
    VCExport      GString       getRS( const GString& hostVar);
    VCExport      GString       getRS( short col );
    VCExport      signed long   getCost(){ return iCost;}
    VCExport      signed long   affectedRows(){return iAffectedRows;}
    VCExport      long uploadBlob(GString cmd, GSeq <GString> *fileList, GSeq <long> *lobType);
    VCExport      long uploadBlob(GString cmd, char * buffer, long size);
    VCExport      long retrieveBlob( GString cmd, GString &blobFile, int writeFile = 1 );
    VCExport      GString descriptorToFile( GString cmd, GString &blobFile, int * outSize );
    VCExport      GString descriptorToFileOld( GString cmd, GString &blobFile, int * outSize );
    VCExport      GString descriptorToFileNew( GString cmd, GString &blobFile, int * outSize );
    VCExport      short writeToFile(char* ptr, long length, GString blobFile);

    VCExport      int getSysTables();
    VCExport      int getTabSchema();
    VCExport      int getTables(GString schema);
    VCExport      int checkHistTable();
    VCExport      int commandExecutedOK(){ return iCommandOK; }
    VCExport      int forceApp(int appID);
    VCExport      int openCrs();
    VCExport      void setStopThread(int stop);
    VCExport      void setDBType(ODBCDB ){}

    VCExport int initRowCrs();
    VCExport int nextRowCrs();
    VCExport GString dataAtCrs(int col);
    VCExport int execByDescriptor( GString cmd, GSeq <GString> *dataSeq, GSeq <int> *typeSeq,
                                   GSeq <short>* sqlVarLengthSeq, GSeq <int> *forBitSeq );

    VCExport      int execByDescriptor( GString cmd, GSeq <GString> *dataSeq);
    VCExport      int isNumType(int pos);
    VCExport      int isDateTime(int pos);
    VCExport      int hasForBitData();
    VCExport      int isForBitCol(int i);
	VCExport      int isBitCol(int i);
    VCExport      int isXMLCol(int i);
    VCExport      int isLOBCol(int i);
    VCExport      int isNullable(int i);
    VCExport      int isLongTypeCol(int i);
    VCExport      int isFixedChar(int i);
    VCExport      int simpleColType(int i);
    VCExport      void setCharForBit(int val);
    VCExport      int getDataBases(GSeq <CON_SET*> *dbList);

    VCExport   void setCurrentDatabase(GString db){ PMF_UNUSED(db); }
    VCExport   GString currentDatabase() { return m_strDB; }


    VCExport      void testme();
    VCExport  unsigned long getResultCount(GString cmd);
    VCExport  int getColSpecs(GString table, GSeq<COL_SPEC*> *specSeq);
    VCExport  int getTriggers(GString table, GString *text);
    VCExport GSeq <IDX_INFO*> getIndexeInfo(GString table);


    VCExport GString getChecks(GString table, GString filter = "");
    VCExport GSeq <GString> getChecksSeq(GString table, GString filter = "");
    VCExport GSeq <GString> getTriggerSeq(GString table);

    VCExport  int getUniqueCols(GString table, GSeq <GString> * colSeq);
    VCExport void createXMLCastString(GString &xmlData);
    VCExport int isBinary(unsigned long row, int col);
    VCExport int  isNull(unsigned long row, int col);
    VCExport int isTruncated(unsigned long row, int col);
    VCExport void setTruncationLimit(int limit);
    VCExport int uploadLongBuffer( GString cmd, GString data, int isBinary);
    VCExport void getResultAsHEX(int asHex);
    VCExport GString cleanString(GString in);
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

    ///
    /////////////////////////// TESTBED ////////////////////////////
    ///
private:

    GString initAll_CLI(GString message, unsigned long maxRows, int getLen);
    int prepareSTMT(unsigned char* sqlstr);

};

#endif


