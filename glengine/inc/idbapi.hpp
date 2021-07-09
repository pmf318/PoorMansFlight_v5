/*****************************************************
 * Interface for DB2 API Access: Export/Import/Monitors
******************************************************/


#include <gstring.hpp>
#include <gseq.hpp>
#include <gdebug.hpp>



#ifndef IDBAPI_HPP
#define IDBAPI_HPP
//static int m_IDBAPICounter = 0;


typedef struct{
    GString Database;
    GString Alias;
    GString Drive;
    GString Directory;
    GString NodeName;
    //GString ReleaseLevel;
    GString Comment;
    GString DbType;
    GString Type;
    GString AuthType;
    void init(){Database = Alias = Drive = Directory = NodeName = Comment = DbType = AuthType = "";}
}  DB_INFO;


typedef struct{
    GString Id;
    GString Name;
    GString Type;
    GString Contents;
    GString State;
    GString TotalPages;
    GString UsablePages;
    GString UsedPages;
    GString FreePages;
    GString HighWaterMark;
}  TAB_SPACE;

typedef struct{
    GString NodeName;
    GString Comment;
    GString HostName;
    GString ServiceName;
    GString ProtocolName;
} NODE_INFO;

class IDBAPI
{
public:


    IDBAPI(){}
    //IDBAPI( IDBAPI const & ){}
    virtual ~IDBAPI(){}
    virtual  GString SQLError() = 0;
    virtual IDBAPI* clone() const = 0;


	
    virtual  int initMonitor(GString database, GString node, GString user, GString pwd) = 0;
    virtual  int getSnapshotData(int type) = 0;
    virtual  int startMonitor() = 0;
    virtual  int resetMonitor() = 0;
    virtual  int stopMonitor() = 0;
    virtual  int initTabSpaceLV(GSeq <TAB_SPACE*> *dataSeq) = 0;

    virtual  int importTable(GString dataFile, GSeq<GString> *pathSeq, GString format, GString statement, GString msgFile, GString modifierString = "") = 0;
    virtual  int importTableNew(GString dataFile, GSeq<GString> *pathSeq, GString format, GString statement, GString msgFile, GString modifierString = "") = 0;

    virtual  GString reorgTable(GString table, GString indexName="", GString tabSpace = "") = 0;
    virtual  GString runStats(GString table, GSeq <GString> * indList, unsigned char statsOpt, unsigned char shareLevel ) = 0;

    virtual  GString rebind(GString bindFile) = 0;

    virtual  int exportTable(GString dataFile, GString format, GString statement, GString msgFile, GString modified) = 0;
    virtual  int exportTable(GString format, GString statement, GString msgFile, GSeq<GString>* pathSeq, GString dataFile, int sessions, GString modified ) = 0;
    virtual  int exportTableNew(GString format, GString statement, GString msgFile, GSeq<GString>* pathSeq, GString dataFile, int sessions, GString modified ) = 0;
    virtual  GString getDbCfgForToken(GString nodeName, GString user, GString pwd, GString dbAlias, int token, int isNumToken) = 0;

    virtual  int getDBVersion(GString alias) = 0;
    virtual  int stopReq() = 0;
    virtual  int getDynSQLSnapshotData() = 0;

    virtual  int loadFromFile(GString dataFile, GSeq<GString> *pathSeq, GString format, GString statement, GString msgFile, GString modifierString) = 0;
    virtual  int loadFromFileNew(GString dataFile, GSeq<GString> *pathSeq, GString format, GString statement, GString msgFile, GString modifierString, GString copyTarget = "") = 0;
    virtual  GSeq <DB_INFO*>  dbInfo() = 0;
    virtual  int getRowData(int row, int col, GString * data) = 0;
    virtual  int getHeaderData(int pos, GString * data) = 0;
    virtual  unsigned long getHeaderDataCount() = 0;
    virtual  unsigned long getRowDataCount() = 0;
    virtual void setGDebug(GDebug *pGDB) = 0;
    virtual int createNode(GString hostName, GString nodeName, GString port, GString comment = "") = 0;
    virtual GSeq<NODE_INFO*> getNodeInfo() = 0;
    virtual int catalogDatabase(GString name, GString alias, GString type, GString nodeName, GString path, GString comment, int authentication ) = 0;
    virtual int uncatalogNode(GString nodeName) = 0;
    virtual int uncatalogDatabase(GString alias) = 0;
    virtual int dbRestart(GString alias, GString user, GString pwd) = 0;


};

#endif // IDBAPI_HPP
