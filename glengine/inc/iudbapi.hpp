/*****************************************************
 * Interface for DB2 API Access: Export/Import/Monitors
******************************************************/


#include <gstring.hpp>
#include <gseq.hpp>

#include <QTableWidget>

#ifndef IUDBAPI_HPP
#define IUDBAPI_HPP


class IUDBAPI
{
public:
    IUDBAPI(){}
    IUDBAPI(GString database, GString ndName, GString user, GString pwd, int type, QTableWidget * pLV);
    //virtual  ~udbapi(){}
    virtual  GString SQLError() = 0;
    virtual  void startMainThread() = 0;

    
    virtual  int getSnapshotData() = 0;
    virtual  int startMonitors() = 0;
    virtual  int resetMonitor() = 0;
    virtual  int initTabSpaceLV(QTableWidget * mainLV) = 0;

    virtual  long    importTable(GString dataFile, GString path, GString format, GString statement, GString msgFile, int useLob = 1) = 0;
    virtual  long    exportTable(GString dataFile, GString format, GString statement, GString msgFile) = 0;
    virtual  GString reorgTable(GString table, GString indexName="", GString tabSpace = "") = 0;
    virtual  GString runStats(GString table, GSeq <GString> * indList, unsigned char statsOpt, unsigned char shareLevel ) = 0;
    virtual  GString rebind(GString bindFile) = 0;

    virtual  long  exportTable(GString format, GString statement, GString msgFile, GString path, GString dataFile, int sessions ) = 0;
    virtual  signed int getDBVersion(GString alias) = 0;
    virtual  int stopReq() = 0;

    virtual  signed int loadFromFile() = 0;
};

#endif // IUDBAPI_HPP
