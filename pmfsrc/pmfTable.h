#ifndef PMFTABLE_H
#define PMFTABLE_H

#include <idsql.hpp>
#include <dsqlplugin.hpp>
#include "pmfColumn.h"

class PmfTable
{
private:
    int init();

    GString m_tableName;
    DSQLPlugin* m_pDSQL;
    GSeq<PmfColumn*> m_colSeq;
    GSeq<GString> m_unqColNameSeq;

    void deb(GString msg);

    GString _baseTabName;
    GString _baseTabSchema;
    TABLE_TYPE _tableType;

    GString m_viewStatement;
    GString m_createStatement;

public:
    PmfTable(DSQLPlugin* pDSQL, GString tabName);
    ~PmfTable();
    int reInit();

    GString quotedName();
    GString unQuotedName();
    GString tabSchema();
    GString context();
    GString tabName();

    int columnCount();
    PmfColumn* column(int i);
    PmfColumn* column(GString colName);

    int uniqueColCount();
    GString uniqueColName(int i);

    GString ddlForView();
    GString createTabStmt();

    const GString &BaseTabName()  {return _baseTabName;}
    const GString &BaseTabSchema()  {return _baseTabSchema;}
    const TABLE_TYPE &TableType()  {return _tableType;}
    const GSeq<GString> uniqueColNames(){ return m_unqColNameSeq; }
};

#endif
