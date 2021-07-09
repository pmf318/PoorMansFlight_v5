#include "pmfTable.h"


PmfTable::PmfTable(DSQLPlugin *pDSQL, GString tabName)
{
    m_pDSQL = new DSQLPlugin(*pDSQL);
    m_tableName = tabName;
    this->init();
}

PmfTable::~PmfTable()
{
    delete m_pDSQL;
    m_colSeq.removeAll();
    m_unqColNameSeq.removeAll();
}

int PmfTable::reInit()
{
    return this->init();
}

int PmfTable::init()
{
    m_viewStatement = "";
    m_createStatement = "";

    TABLE_PROPS props = m_pDSQL->getTableProps(m_tableName);
    _baseTabName = props.BaseTabName;
    _baseTabSchema = props.BaseTabSchema;
    _tableType = props.TableType;

    GSeq<COL_SPEC*> m_colDescSeq;
    if( _tableType == TYPE_ALIAS )
    {
        m_pDSQL->getColSpecs(_baseTabSchema+"."+_baseTabName, &m_colDescSeq);
    }
    else m_pDSQL->getColSpecs(m_tableName, &m_colDescSeq);

    for(int i = 1; i <= m_colDescSeq.numberOfElements(); ++i)
    {
        m_colSeq.add(new PmfColumn(m_colDescSeq.elementAtPosition(i)));
    }
    m_colDescSeq.removeAll();

    m_pDSQL->getUniqueCols(m_tableName, &m_unqColNameSeq);
    return 0;
}

GString PmfTable::quotedName()
{
    return m_tableName;
}

GString PmfTable::unQuotedName()
{
    return GString(m_tableName).removeAll('\"');
}

GString PmfTable::tabName()
{
    GString table = m_tableName;
    table.removeAll('\"');
    if( m_pDSQL->getDBType() == SQLSERVER )
    {
        if( table.occurrencesOf(".") != 2 && table.occurrencesOf(".") != 1) return "@ErrTabString";
        if( table.occurrencesOf(".") == 2 ) table = table.remove(1, table.indexOf("."));
        return table.subString(table.indexOf(".")+1, table.length()).strip();
    }
    else if( m_pDSQL->getDBType() == DB2 || m_pDSQL->getDBType() == DB2ODBC || m_pDSQL->getDBType() == MARIADB )
    {
        if( table.occurrencesOf(".") == 1 ) return table.subString(table.indexOf('.')+1, table.length()).strip().strip("\"");
    }
    return "";
}

GString PmfTable::tabSchema()
{
    GString table = m_tableName;
    table.removeAll('\"');
    if( m_pDSQL->getDBType() == SQLSERVER )
    {
        if( table.occurrencesOf(".") != 2 && table.occurrencesOf(".") != 1) return "@ErrTabString";
        if( table.occurrencesOf(".") == 2 ) table.remove(1, table.indexOf("."));
        return table.subString(1, table.indexOf(".")-1);
    }
    else if( m_pDSQL->getDBType() == DB2 || m_pDSQL->getDBType() == DB2ODBC || m_pDSQL->getDBType() == MARIADB )
    {
        if( table.occurrencesOf(".") == 1 ) return table.subString(1, table.indexOf('.')-1).strip("\"");
    }
    return "";
}

GString PmfTable::context()
{
    GString table = m_tableName;
    table.removeAll('\"');
    if( m_pDSQL->getDBType() == SQLSERVER )
    {
        if( table.occurrencesOf(".") != 2 ) return "";
        return table.subString(1, table.indexOf(".")-1);
    }
    return "";
}

int PmfTable::columnCount()
{
    return m_colSeq.numberOfElements();
}

PmfColumn* PmfTable::column(int i)
{
    if( i <= 0 || i > m_colSeq.numberOfElements() ) return NULL;
    return m_colSeq.elementAtPosition(i);
}

PmfColumn* PmfTable::column(GString colName)
{
    for(int i = 1; i <= m_colSeq.numberOfElements(); ++i )
    {
        if( m_colSeq.elementAtPosition(i)->colName() == colName ) return m_colSeq.elementAtPosition(i);
    }
    return NULL;
}

GString PmfTable::createTabStmt()
{
    if( m_createStatement.length() ) return m_createStatement;

    GString out, defVal, colLen, colName;
    out = ("CREATE TABLE "+m_tableName+" (\n" );
    for(int i = 1; i <= (int)m_colSeq.numberOfElements(); ++i )
    {
        PmfColumn *pmfCol = m_colSeq.elementAtPosition(i);

        if( m_pDSQL->getDBType() == MARIADB )colName = "    " +pmfCol->colName()+ "    ";
        else colName = "    \"" +pmfCol->colName()+ "\"    ";


        if( pmfCol->defaultVal().length() ) defVal = " DEFAULT "+pmfCol->defaultVal();
        else defVal = " ";

        if( pmfCol->colLength() == "N/A" ) colLen = " ";
        else colLen = "("+pmfCol->colLength() +") ";

        out += colName + pmfCol->colType() +" " + colLen + pmfCol->nullable()+" "+pmfCol->misc()+" "+defVal;

        if( i < (int)m_colSeq.numberOfElements() ) out = out.strip()+",\n";
        else out+="\n";
    }
    if( m_pDSQL->getDBType() == DB2ODBC || m_pDSQL->getDBType() == DB2 ) out = out.stripTrailing(",")+")";
    else out = out.stripTrailing(",")+");\n\n";
    m_createStatement = out;
    return m_createStatement;
}

GString PmfTable::ddlForView()
{
    if( m_viewStatement.length() ) return m_viewStatement;

    if( _tableType == TYPE_TYPED_VIEW || _tableType == TYPE_UNTYPED_VIEW )
    {
        m_viewStatement = m_pDSQL->getDdlForView(m_tableName);
    }
    else m_viewStatement = "";
    return m_viewStatement;
}

int PmfTable::uniqueColCount()
{
    return m_unqColNameSeq.numberOfElements();
}

GString PmfTable::uniqueColName(int i)
{
    if( i < 1 || i > m_unqColNameSeq.numberOfElements() ) return "";
    return m_unqColNameSeq.elementAtPosition(i);
}

void PmfTable::deb(GString msg)
{
    printf("PmfTable> %s\n", (char*)msg);
}
