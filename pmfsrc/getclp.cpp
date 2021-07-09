//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include "getclp.h"
#include "helper.h"
#include <qlayout.h>
#include <QGridLayout>
#include <gstuff.hpp>
#include <QCloseEvent>
#include <QHeaderView>
#include "tabEdit.h"
#include <QTime>
#include <QDate>


#include "helper.h"
#include "gstringlist.hpp"
#include "pmfTable.h"


#define XMLDDL_SET_TS "Timestamp"
#define XMLDDL_WRITE_XSD "XSD-entry"
#define XMLDDL_WRITE_SYSIDX "SYS Indexes"
#define XMLDDL_WRITE_IDXSTMT "Index create stmt"
#define XMLDDL_WRITE_FKEYSTMT "Foreign Key create stmt"
#define XMLDDL_WRITE_PKEYSTMT "Primary Key create stmt"
#define XMLDDL_WRITE_PKEYSCHEMA "Prim. Key name+schema"
#define XMLDDL_WRITE_CHECKSTMT "Check stmt"
#define XMLDDL_WRITE_TABSTMT "Table create stmt"
#define XMLDDL_WRITE_TABLESPACE "Include TABLESPACE"
#define XMLDDL_WRITE_BOM "Write BOM"
#define XMLDDL_COLS_IN_QUOTES "Quotes in create stmts"
#define XMLDDL_GROUPNAME "XmlDdlOptions"


Getclp::Getclp(DSQLPlugin* pDSQL,  TabEdit *parent, GString tableName)
  :QDialog(parent)
{

    m_pDSQL = new DSQLPlugin(*pDSQL);
    iTableName = tableName;
    m_pParent = parent;

    this->resize(760, 400);

	this->setWindowTitle("Table Info");

    m_mainWdgt = new QTabWidget(this);


	ok = new QPushButton(this);
    ok->setDefault(true);
	ok->setText("Exit");
	connect(ok, SIGNAL(clicked()), SLOT(OKClicked()));


    saveAsPlainTextBt = new QPushButton(this);
    saveAsPlainTextBt->setText("Save as plain text");
    connect(saveAsPlainTextBt, SIGNAL(clicked()), SLOT(saveAsPlainTextClicked()));

    saveAsXmlBt = new QPushButton(this);
    saveAsXmlBt->setText("Save as XML");
    connect(saveAsXmlBt, SIGNAL(clicked()), SLOT(saveAsXmlClicked()));

    optionsBt = new QPushButton("Options", this);
    connect(optionsBt, SIGNAL(clicked()), SLOT(optionsClicked()));

    QGridLayout * mainGrid = new QGridLayout(this);
    mainGrid->addWidget(m_mainWdgt, 0, 0, 1,5);

    mainGrid->addWidget(ok, 1, 2);
    mainGrid->addWidget(saveAsXmlBt, 1, 1);
    mainGrid->addWidget(saveAsPlainTextBt, 1, 0);
    mainGrid->addWidget(optionsBt, 1, 4);

    addColsWdgt();
    addTextLB();
    fillLB();
    //Enable SORT after filling
    tabWdgt->setSortingEnabled(true);

    Helper::setVHeader(tabWdgt);
    tabWdgt->verticalHeader()->hide();
//    for( int i = 0; i < tabWdgt->rowCount(); ++ i)
//    {
//#ifdef MAKE_VC
//        tabWdgt->setRowHeight(i, QFontMetrics( tabWdgt->font()).height()+3);
//#else
//        tabWdgt->setRowHeight(i, QFontMetrics( tabWdgt->font()).height()+5);
//#endif
//    }

}

void Getclp::addTextLB()
{
    QWidget * pWdgt = new QWidget(this);
    QGridLayout * grid = new QGridLayout(pWdgt);
    //infoLB = new QTextEdit(this);
    //mainEditor = new QTextBrowser(this);


    mainEditor = new TxtEdit(NULL, m_pParent);
    mainEditor->setLineWrapMode(QTextBrowser::NoWrap);
    mainEditor->setGeometry( 20, 20, 520, 300);
    if( m_pParent ) m_pParent->initTxtEditor(mainEditor);

    grid->addWidget(mainEditor, 0, 0, 1, 4);

    m_mainWdgt->addTab(pWdgt, "DDL");
}
void Getclp::addColsWdgt()
{
    QWidget * pWdgt = new QWidget(this);
    QGridLayout * grid = new QGridLayout(pWdgt);
    tabWdgt = new QTableWidget(this);
    tabWdgt->setGeometry( 20, 20, 520, 300);


    grid->addWidget(tabWdgt, 0, 0, 1, 4);

    tabWdgt->setColumnCount(7);
    QTableWidgetItem * pItem;

    pItem = new QTableWidgetItem("Pos");
    tabWdgt->setHorizontalHeaderItem(0, pItem);

    pItem = new QTableWidgetItem("Name");
    tabWdgt->setHorizontalHeaderItem(1, pItem);

    pItem = new QTableWidgetItem("Type");
    tabWdgt->setHorizontalHeaderItem(2, pItem);

    pItem = new QTableWidgetItem("Size");
    tabWdgt->setHorizontalHeaderItem(3, pItem);

    pItem = new QTableWidgetItem("Nullable");
    tabWdgt->setHorizontalHeaderItem(4, pItem);

    pItem = new QTableWidgetItem("Default");
    tabWdgt->setHorizontalHeaderItem(5, pItem);

    pItem = new QTableWidgetItem("Misc");
    tabWdgt->setHorizontalHeaderItem(6, pItem);

    m_mainWdgt->addTab(pWdgt, "Columns");

    connect((QWidget*)tabWdgt->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(sortClicked(int)));
}

Getclp::~Getclp()
{
    if( m_pDSQL ) delete m_pDSQL;
}

void Getclp::sortClicked(int)
{
    Helper::setVHeader(tabWdgt);
}

void Getclp::OKClicked()
{
	close();
}

void Getclp::saveAsPlainTextClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save", Helper::getLastSelectedPath("getclp")+GString(iTableName).removeAll('\"')+".DDL");
    if( fileName.isNull() ) return;
    Helper::setLastSelectedPath("getclp", GStuff::pathFromFullPath(fileName));
    this->saveAsPlainText(fileName, 1);
}

int Getclp::saveAsPlainText(GString fileName, int overwrite)
{
    QFileInfo fileInfo(fileName);
    if( fileInfo.exists() && !overwrite ) return 1;

    GFile gf(GString(fileName), 2);

    gf.writeToNewLine("-- File created by Poor Man's Flight");
    gf.writeToNewLine("--");

    if( m_pDSQL->getDBType() == DB2 || m_pDSQL->getDBType() == DB2ODBC )
    {
        gf.writeToNewLine("-- To create the table below, open a DB2 CLP (CommandLineProcessor),");
        gf.writeToNewLine("-- connect to the database, and execute:");
        gf.writeToNewLine("-- db2 -tvf <name of this file> (and don't forget to commit)");
        gf.writeToNewLine("-- ");
    }
    gf.writeToNewLine("-- WARNING: BEFORE CREATING THE TABLE, VERIFY THE SETTINGS BELOW.");
    gf.writeToNewLine("-- For example, if you use IDENTTY columns, verify that START and INCREMENT are OK");
    gf.writeToNewLine("-- (PMF will not set these for you).");
    gf.writeToNewLine("-- Also, make sure that the table gets created in the correct tablespace");
    gf.writeToNewLine("-- by adding the appropriate CREATE ... IN <tablespace> statement.");
    gf.writeToNewLine("-- The same goes for INDEX IN <tablespace>.");
    gf.writeToNewLine("-- You might also want to add something like GRANT SELECT ON <table> TO...");
    gf.writeToNewLine("-- ");


    TABLE_PROPS tabProps = m_pDSQL->getTableProps(iTableName);
    if( tabProps.TableType == TYPE_TYPED_VIEW || tabProps.TableType == TYPE_UNTYPED_VIEW )
    {
        gf.writeToNewLine("-- Uncomment the next line to drop the view first:");
        gf.writeToNewLine("-- DROP VIEW  "+iTableName);
    }
    if( tabProps.TableType == TYPE_TYPED_TABLE || tabProps.TableType == TYPE_UNTYPED_TABLE )
    {
        gf.writeToNewLine("-- Uncomment the next line to drop the table first:");
        gf.writeToNewLine("-- DROP TABLE "+iTableName);
    }
    gf.writeToNewLine("");


    gf.writeToNewLine(mainEditor->document()->toPlainText());
    return 0;
}

void Getclp::saveAsXmlClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save", Helper::getLastSelectedPath("getclp")+iTableName.removeAll('\"')+".XML");
    if( fileName.isNull() ) return;
    Helper::setLastSelectedPath("getclp", GStuff::pathFromFullPath(fileName));
    this->saveAsXml(fileName, 1);
}

void Getclp::writeChecks(DSQLPlugin *pDSQL, QFile * outFile, OptionsTab *pOptTab)
{    
    if( pDSQL->getRowDataCount() == 0 ) return;

    GString value, tag, entry, stmt;
    writeToUtf8File(outFile, "\t\t<checks>");

    int writeCreateStmt = pOptTab->getCheckBoxValue(XMLDDL_GROUPNAME, XMLDDL_WRITE_CHECKSTMT);
    for(int i = 1; i <= (int)pDSQL->getRowDataCount(); ++i)
    {
        entry = "\t\t\t<check ";
        for(int j = 1; j <= (int)m_pDSQL->getHeaderDataCount(); ++j)
        {
            pDSQL->getHeaderData(j, &tag);
            pDSQL->getRowData(i, j, &value);
            entry += tag.lowerCase()+"=\""+GStuff::formatForXml(value)+"\" ";
        }
        stmt = GStuff::formatForXml(m_pDSQL->getChecks(iTableName).removeAll('\n'));
        if( writeCreateStmt) entry += " stmt=\""+stmt+"\"";
        entry += "/>";
        writeToUtf8File(outFile, entry);
    }

    writeToUtf8File(outFile, "\t\t</checks>");
}

GString Getclp::formatColumnsInStmt(GString cols, OptionsTab *pOptTab)
{
    if( !pOptTab->getCheckBoxValue(XMLDDL_GROUPNAME, XMLDDL_COLS_IN_QUOTES) )
    {
        return cols.removeAll('\"');
    }
    return GStuff::formatForXml(cols);
}

void Getclp::writeForKeyInfo(GSeq <IDX_INFO*> * idxSeq, QFile * outFile, OptionsTab *pOptTab)
{
    int count = 0;
    IDX_INFO* pIdx;
    for( int i = 1; i <= idxSeq->numberOfElements(); ++i )
    {
        pIdx = idxSeq->elementAtPosition(i);
        if( pIdx->Type != DEF_IDX_FORKEY  ) continue;
        count++;
    }
    if (!count) return;
    GString entry;
    int writeCreateStmt = pOptTab->getCheckBoxValue(XMLDDL_GROUPNAME, XMLDDL_WRITE_FKEYSTMT);

    writeToUtf8File(outFile, "\t\t<foreign_keys>");
    for( int i = 1; i <= idxSeq->numberOfElements(); ++i )
    {
        pIdx = idxSeq->elementAtPosition(i);
        if( pIdx->Type != DEF_IDX_FORKEY ) continue;
        entry = "\t\t\t<foreign_key ";
        //entry += "iid=\""+pIdx->Iidx+"\" ";
        entry += "schema=\""+pIdx->Schema+"\" ";
        entry += "name=\""+pIdx->Name+"\" ";
        entry += "columns=\""+pIdx->Columns.removeAll('\"')+"\" ";
        entry += "type=\""+pIdx->Type+"\" ";
        entry += "reference_table=\""+pIdx->RefTab+"\" ";
        entry += "reference_columns=\""+pIdx->RefCols.removeAll('\"')+"\" ";
        entry += "delete_rule=\""+pIdx->DeleteRule+"\" ";
        if( writeCreateStmt ) entry += "stmt=\""+formatColumnsInStmt(pIdx->Stmt, pOptTab)+"\" ";
        entry += "/>";
        writeToUtf8File(outFile, entry);
    }
    writeToUtf8File(outFile, "\t\t</foreign_keys>");
}


void Getclp::writePrimKeyInfo(GSeq <IDX_INFO*> * idxSeq, QFile *outFile, OptionsTab *pOptTab)
{
    IDX_INFO* pIdx;
    int count = 0;
    for( int i = 1; i <= idxSeq->numberOfElements(); ++i )
    {
        pIdx = idxSeq->elementAtPosition(i);
        if( pIdx->Type != DEF_IDX_PRIM  ) continue;
        count++;
    }
    if (!count) return;
    GString entry;
    writeToUtf8File(outFile, "\t\t<primary_keys>");

    int writeSysIdx = pOptTab->getCheckBoxValue(XMLDDL_GROUPNAME, XMLDDL_WRITE_SYSIDX);
    int writeCreateStmt = pOptTab->getCheckBoxValue(XMLDDL_GROUPNAME, XMLDDL_WRITE_PKEYSTMT);
    int writeSchema = pOptTab->getCheckBoxValue(XMLDDL_GROUPNAME, XMLDDL_WRITE_PKEYSCHEMA);
    for( int i = 1; i <= idxSeq->numberOfElements(); ++i )
    {
        pIdx = idxSeq->elementAtPosition(i);
        if( pIdx->Type != DEF_IDX_PRIM ) continue;
        entry = "\t\t\t<primary_key ";
        entry += "iid=\""+pIdx->Iidx+"\" ";
        if( (writeSysIdx || !isSysIdx(pIdx->Name)) && writeSchema )
        {
            entry += "schema=\""+pIdx->Schema+"\" ";
            entry += "name=\""+pIdx->Name+"\" ";
        }
        entry += "columns=\""+pIdx->Columns.removeAll('\"')+"\" ";
        entry += "type=\""+pIdx->Type+"\" ";
        if( writeCreateStmt ) entry += "stmt=\""+formatColumnsInStmt(pIdx->Stmt, pOptTab)+"\" ";
        entry += "/>";
        writeToUtf8File(outFile, entry);
    }
    writeToUtf8File(outFile, "\t\t</primary_keys>");
}

int Getclp::isSysIdx(GString idxName)
{
    if( idxName.subString(1, 3) == "SQL" && idxName.subString(4, 6).isDigits() )  return 1;
    return 0;
}

void Getclp::writeIdxInfo(GSeq <IDX_INFO*> * idxSeq, QFile *outFile, OptionsTab *pOptTab)
{
    IDX_INFO* pIdx;
    int count = 0;
    int writeSysIdx = pOptTab->getCheckBoxValue(XMLDDL_GROUPNAME, XMLDDL_WRITE_SYSIDX);
    int writeCreateStmt = pOptTab->getCheckBoxValue(XMLDDL_GROUPNAME, XMLDDL_WRITE_IDXSTMT);

    for( int i = 1; i <= idxSeq->numberOfElements(); ++i )
    {
        pIdx = idxSeq->elementAtPosition(i);
        if( pIdx->Type == DEF_IDX_FORKEY || pIdx->Type == DEF_IDX_PRIM ) continue;
        if( writeSysIdx == 0 && pIdx->Name.subString(1, 3) == "SQL" ) continue;
        count++;
    }
    if (!count) return;
    GString entry;
    writeToUtf8File(outFile, "\t\t<indexes>");

    for( int i = 1; i <= idxSeq->numberOfElements(); ++i )
    {
        pIdx = idxSeq->elementAtPosition(i);
        if( pIdx->Type == DEF_IDX_FORKEY || pIdx->Type == DEF_IDX_PRIM ) continue;        
        if( writeSysIdx == 0 && pIdx->Name.subString(1, 3) == "SQL" ) continue;
        entry = "\t\t\t<index ";
        entry += "iid=\""+pIdx->Iidx+"\" ";
        entry += "schema=\""+pIdx->Schema+"\" ";
        entry += "name=\""+pIdx->Name+"\" ";
        entry += "columns=\""+pIdx->Columns.removeAll('\"')+"\" ";
        entry += "type=\""+pIdx->Type+"\" ";
        if( writeCreateStmt ) entry += "stmt=\""+formatColumnsInStmt(pIdx->Stmt, pOptTab)+"\" ";
        entry += "/>";
        writeToUtf8File(outFile, entry);
    }
    writeToUtf8File(outFile, "\t\t</indexes>");
}

OptionsTab* Getclp::createOptionsTab(GString groupName, QWidget * parent)
{
    OptionsTab* pOptTab = new OptionsTab(parent, "XmlDdl");
    pOptTab->addRow( groupName, "--- Options for XML file output");
    pOptTab->addRow( groupName, XMLDDL_SET_TS, ROW_CHECKBOX, "1", "Create XML-tag containing Date and Timestamp (as comment)");
    pOptTab->addRow( groupName, XMLDDL_WRITE_BOM, ROW_CHECKBOX, "1", "Write ByteOrderMark (BOM)");
    pOptTab->addRow( groupName, XMLDDL_WRITE_SYSIDX, ROW_CHECKBOX, "1", "Include Indexnames starting with 'SQL....' (usually system generated indexes)");
    pOptTab->addRow( groupName, XMLDDL_WRITE_IDXSTMT, ROW_CHECKBOX, "1", "Include CREATE statements for Indexes");
    pOptTab->addRow( groupName, XMLDDL_WRITE_PKEYSTMT, ROW_CHECKBOX, "1", "Include CREATE statements for Primary Keys");
    pOptTab->addRow( groupName, XMLDDL_WRITE_PKEYSCHEMA, ROW_CHECKBOX, "1", "Include name and schema for Primary Keys");
    pOptTab->addRow( groupName, XMLDDL_WRITE_FKEYSTMT, ROW_CHECKBOX, "1", "Include CREATE statements for Foreign Keys");
    pOptTab->addRow( groupName, XMLDDL_WRITE_CHECKSTMT , ROW_CHECKBOX, "1", "Include CREATE statements for Checks");
    pOptTab->addRow( groupName, XMLDDL_WRITE_TABSTMT, ROW_CHECKBOX, "1", "Include CREATE statement for the table");
    pOptTab->addRow( groupName, XMLDDL_WRITE_TABLESPACE, ROW_CHECKBOX, "0", "Include TABLESPACE in table statements", 0);
    pOptTab->addRow( groupName, XMLDDL_COLS_IN_QUOTES, ROW_CHECKBOX, "1", "Create stmt for IDX/ForKey/PrimKey: Put column names in double-quotes", 0);

    pOptTab->addRow( groupName, XMLDDL_WRITE_XSD, ROW_LINEEDIT, "", "Expand header with XSD-entry (enter xsd-filename here)", 0);

    pOptTab->displayRowsInTab(groupName, "XML Options");
    return pOptTab;
}

//This gets also called from allTabDDL, hence the parent
void Getclp::showOptionsDialog(QWidget* parent)
{
    OptionsTab* pOptTab = createOptionsTab(XMLDDL_GROUPNAME, parent);
    pOptTab->exec();
    delete pOptTab;
}

void Getclp::optionsClicked()
{
    showOptionsDialog(this);
}

void Getclp::writeColumns( QFile *outFile)
{
    GString column, data, entry;
    writeToUtf8File(outFile, "\t\t<columns>");
    for( int i = 0; i < tabWdgt->rowCount(); ++i )
    {
        entry = "\t\t\t<column ";
        for( int j = 0; j < tabWdgt->columnCount(); ++j )
        {
            data = tabWdgt->item(i,j)->text();
            column = tabWdgt->horizontalHeaderItem(j)->text().toLower();
            if( GString(column).upperCase() == "NULLABLE")
            {
                if( data == "NOT NULL") data = "false";
                else data = "true";
            }
            entry += column +"=\""+data+"\" ";
        }
        entry += "/>";
        writeToUtf8File(outFile, entry);
    }
    writeToUtf8File(outFile, "\t\t</columns>");
}

int Getclp::saveAsXml(GString fileName, int overwrite)
{
    QFileInfo fileInfo(fileName);
    if( fileInfo.exists() && !overwrite ) return 1;
    OptionsTab* pOptTab = createOptionsTab(XMLDDL_GROUPNAME, this);
    int writeTimestamp = pOptTab->getCheckBoxValue(XMLDDL_GROUPNAME, XMLDDL_SET_TS);
    GString  xsdFile = pOptTab->getFieldValue(XMLDDL_GROUPNAME, XMLDDL_WRITE_XSD);

    int writeBOM = pOptTab->getCheckBoxValue(XMLDDL_GROUPNAME, XMLDDL_WRITE_BOM);

    remove(fileName);

    QFile *outFile = new QFile(fileName);
    outFile->open(QFile::ReadWrite | QIODevice::Text);

    GString timeStamp = QTime::currentTime().toString();
    GString dateStamp = QDate::currentDate().toString("yyyy.MM.dd");

    //Write BOM on first file-entry
    writeToUtf8File(outFile, "<?xml version=\"1.0\" encoding=\"utf-8\" ?>", writeBOM);
    if( writeTimestamp) writeToUtf8File(outFile, "<!-- File created by Poor Man's Flight "+Helper::pmfVersion()+", "+dateStamp+" - "+timeStamp+" /> -->");

    if( xsdFile.length() ) writeToUtf8File(outFile, "<config xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\""+xsdFile+"\" >");
    else writeToUtf8File(outFile, "<config >");

    TABLE_PROPS tabProps = m_pDSQL->getTableProps(iTableName);
    if( tabProps.TableType == TYPE_TYPED_VIEW || tabProps.TableType == TYPE_UNTYPED_VIEW )
    {
        writeView(outFile, pOptTab);
    }
    else if( tabProps.TableType == TYPE_ALIAS  )
    {
        writeAlias(outFile, pOptTab, &tabProps);
    }
    else if( tabProps.TableType == TYPE_MAT_QUERY  )
    {
        writeMQT(outFile, pOptTab);
    }
    else writeTable(outFile, pOptTab);

    writeToUtf8File(outFile, "</config>");

    outFile->close();
    delete pOptTab;
    return 0;




//    int isView = 0;

//    TABLE_PROPS tabProps = m_pDSQL->getTableProps(iTableName);
//    if( tabProps.TableType == TYPE_TYPED_VIEW || tabProps.TableType == TYPE_UNTYPED_VIEW )
//    {
//        isView = 1;
//        createTabStmt = m_pDSQL->getDdlForView(iTableName).removeAll('\n');
//    }
//    else createTabStmt = m_strCreateTabStmt.removeAll('\n');

//    if( pOptTab->getCheckBoxValue(XMLDDL_GROUPNAME, XMLDDL_WRITE_TABLESPACE) && !isView )
//    {
//        tableSpaceAttribute = " tablespace=\""+tableSpace(iTableName)+"\"";
//        createTabStmt += " IN "+tableSpace(iTableName)+";";
//    }
//    else tableSpaceAttribute = "";

//    if( pOptTab->getCheckBoxValue(XMLDDL_GROUPNAME, XMLDDL_WRITE_TABSTMT) )
//    {
//        if( !isView ) createTabStmt = " stmt=\""+GStuff::formatForXml(createTabStmt).removeButOne()+"\"";
//        else createTabStmt = " stmt=\""+createTabStmt+tableSpaceAttribute+"\"";
//    }
//    else createTabStmt = "";

//    if( xsdFile.length() ) writeToUtf8File(outFile, "<config xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\""+xsdFile+"\" >");
//    else writeToUtf8File(outFile, "<config >");



//    GString tableTypeName = isView ? "view" : "table";

//    writeToUtf8File(outFile, "\t<"+tableTypeName+" schema=\""+Helper::tableSchema(iTableName, m_pDSQL->getDBType())+"\" name=\""+Helper::tableName(iTableName, m_pDSQL->getDBType())+"\""+tableSpaceAttribute+createTabStmt+ " >");

//    writeColumns(outFile);

//    if( isView )
//    {
//        delete pOptTab;
//        writeToUtf8File(outFile, "\t</view>");
//        writeToUtf8File(outFile, "</config>");
//        outFile->close();
//        return 0;
//    }

//    GSeq <IDX_INFO*> idxSeq = m_pDSQL->getIndexeInfo(iTableName);
//    writePrimKeyInfo(&idxSeq, outFile, pOptTab);
//    writeForKeyInfo(&idxSeq, outFile, pOptTab);
//    writeIdxInfo(&idxSeq, outFile, pOptTab);

//    m_pDSQL->fillChecksView(iTableName, 0);
//    writeChecks( m_pDSQL, outFile, pOptTab);

//    GSeq <GString> trgSeq = m_pDSQL->getTriggerSeq(iTableName);
//    writeSeqToFile(outFile, &trgSeq, "trigger");
//    writeToUtf8File(outFile, "\t</table>");
//    writeToUtf8File(outFile, "</config>");

//    outFile->close();
//    delete pOptTab;
//    return 0;
}

void Getclp::writeAlias(QFile *outFile, OptionsTab* pOptTab, TABLE_PROPS *pProps)
{
    GString createTabStmt = "CREATE ALIAS "+iTableName+" FOR "+pProps->BaseTabSchema+"."+pProps->BaseTabName;

    if( pOptTab->getCheckBoxValue(XMLDDL_GROUPNAME, XMLDDL_WRITE_TABSTMT) )
    {
        createTabStmt = " stmt=\""+createTabStmt+";\"";
    }
    GString baseInfo = " base_schema=\""+pProps->BaseTabSchema+"\" base_name=\""+pProps->BaseTabName+"\"";
    writeToUtf8File(outFile, "\t<alias schema=\""+Helper::tableSchema(iTableName, m_pDSQL->getDBType())+"\" name=\""+
                    Helper::tableName(iTableName, m_pDSQL->getDBType())+"\""+baseInfo+createTabStmt+ " >");
    writeColumns(outFile);
    writeToUtf8File(outFile, "\t</alias>");
}

void Getclp::writeMQT(QFile *outFile, OptionsTab* pOptTab)
{
    GString createTabStmt = m_pDSQL->getDdlForView(iTableName).removeAll('\n').change('\t', ' ').removeButOne();
    GString tableSpaceAttribute;

    if( pOptTab->getCheckBoxValue(XMLDDL_GROUPNAME, XMLDDL_WRITE_TABLESPACE) )
    {
        GString tabSpace = tableSpace(iTableName);
        tableSpaceAttribute = " tablespace=\""+tabSpace+"\"";
        if( tabSpace.length() ) createTabStmt += " IN "+tabSpace;
    }
    else tableSpaceAttribute = "";

    if( pOptTab->getCheckBoxValue(XMLDDL_GROUPNAME, XMLDDL_WRITE_TABSTMT) )
    {
        createTabStmt = " stmt=\""+GStuff::formatForXml(createTabStmt).removeButOne()+";\"";
    }
    else createTabStmt = "";

    writeToUtf8File(outFile, "\t<mqt schema=\""+Helper::tableSchema(iTableName, m_pDSQL->getDBType())+
                    "\" name=\""+Helper::tableName(iTableName, m_pDSQL->getDBType())+"\""+tableSpaceAttribute+createTabStmt+ " >");

    writeColumns(outFile);
    writeToUtf8File(outFile, "\t</mqt>");
}


void Getclp::writeView(QFile *outFile, OptionsTab*)
{
    GString createTabStmt = m_pDSQL->getDdlForView(iTableName).removeAll('\n').removeButOne();
    //createTabStmt = linesToSingleLine(createTabStmt).removeButOne().strip();
    createTabStmt = createTabStmt.change(10, ' ').change(13, ' ').change('\t', ' ').removeButOne().strip();
    createTabStmt = " stmt=\""+ GStuff::formatForXml(createTabStmt)+";\"";
    writeToUtf8File(outFile, "\t<view schema=\""+Helper::tableSchema(iTableName, m_pDSQL->getDBType())+"\" name=\""+
                    Helper::tableName(iTableName, m_pDSQL->getDBType())+"\""+createTabStmt+ " />");
}

void Getclp::writeTable(QFile *outFile, OptionsTab* pOptTab)
{
    GString createTabStmt = m_strCreateTabStmt.removeAll('\n');
    GString tableSpaceAttribute;

    if( pOptTab->getCheckBoxValue(XMLDDL_GROUPNAME, XMLDDL_WRITE_TABLESPACE) )
    {
        GString tabSpace = tableSpace(iTableName);
        tableSpaceAttribute = " tablespace=\""+tabSpace+"\"";
        if( tabSpace.length() ) createTabStmt += " IN "+tabSpace;
    }
    else tableSpaceAttribute = "";

    if( pOptTab->getCheckBoxValue(XMLDDL_GROUPNAME, XMLDDL_WRITE_TABSTMT) )
    {
        createTabStmt = " stmt=\""+GStuff::formatForXml(createTabStmt).removeButOne()+";\"";
    }
    else createTabStmt = "";

    writeToUtf8File(outFile, "\t<table schema=\""+Helper::tableSchema(iTableName, m_pDSQL->getDBType())+
                    "\" name=\""+Helper::tableName(iTableName, m_pDSQL->getDBType())+"\""+tableSpaceAttribute+createTabStmt+ " >");

    writeColumns(outFile);

    GSeq <IDX_INFO*> idxSeq = m_pDSQL->getIndexeInfo(iTableName);
    writePrimKeyInfo(&idxSeq, outFile, pOptTab);
    writeForKeyInfo(&idxSeq, outFile, pOptTab);
    writeIdxInfo(&idxSeq, outFile, pOptTab);

    m_pDSQL->fillChecksView(iTableName, 0);
    writeChecks( m_pDSQL, outFile, pOptTab);

    GSeq <GString> trgSeq = m_pDSQL->getTriggerSeq(iTableName);
    writeSeqToFile(outFile, &trgSeq, "trigger");
    writeToUtf8File(outFile, "\t</table>");

}

void Getclp::writeToUtf8File(QFile* fileOut, QString txt, int setBOM)
{
    QTextStream streamFileOut(fileOut);
    //This should be called only once, on the first line/entry
    streamFileOut.setGenerateByteOrderMark(setBOM == 1 ? true : false);
    streamFileOut.setCodec("UTF-8");
    streamFileOut << txt <<"\n";
    streamFileOut.flush();
}

void Getclp::writeSeqToFile(QFile *outFile, GSeq <GString> *entry, GString tag)
{
    GString key;
    if( entry->numberOfElements() > 0 )
    {
        writeToUtf8File(outFile, "\t\t<"+tag+"s>");
        for( int i = 1; i <= entry->numberOfElements(); ++i )
        {
            //key = entry->elementAtPosition(i).removeAll('\n');
            key = linesToSingleLine(entry->elementAtPosition(i));
            writeToUtf8File(outFile, "\t\t\t<"+tag+" stmt=\""+GStuff::formatForXml(key).strip()+"\" />");
        }
        writeToUtf8File(outFile, "\t\t</"+tag+"s>");
    }
}

GString Getclp::linesToSingleLine(GString in)
{
    GStringList list(in, "\n");
    GString out;
    for(int i = 1; i <= list.count(); ++i )
    {
        if( list.at(i).occurrencesOf("--") )
        {
            out += list.at(i).subString(1, list.at(i).indexOf("--")-1)+" ";
        }
        else
        {
            out += list.at(i)+" ";
        }
    }
    return out.removeButOne();
}

short Getclp::fillLB()
{

    PmfTable pmfTable(m_pDSQL, iTableName);

    for(int i = 1; i <= (int)pmfTable.columnCount(); ++i )
    {
        addTableItem(0, GString(i));
        addTableItem(1, pmfTable.column(i)->colName());
        addTableItem(2, pmfTable.column(i)->colType());
        addTableItem(3, pmfTable.column(i)->colLength());
        addTableItem(4, pmfTable.column(i)->nullable());
        addTableItem(5, pmfTable.column(i)->defaultVal());
        addTableItem(6, pmfTable.column(i)->misc());
    }
    tabWdgt->sortByColumn(0, Qt::AscendingOrder);

    GString out, in;
    if( pmfTable.TableType() == TYPE_TYPED_VIEW || pmfTable.TableType() == TYPE_UNTYPED_VIEW )
    {        
        out = "------------------------------ \n";
        out += "-- Note that this is a view -- \n";
        out += "------------------------------ \n";
        printf("step 1: %s\n", (char*) pmfTable.ddlForView());
        printf("step 2: %s\n", (char*) pmfTable.ddlForView().change('\t', ' '));
        printf("step 3: %s\n", (char*) pmfTable.ddlForView().change('\t', ' ').removeButOne());
        out += pmfTable.ddlForView().change('\t', ' ').removeButOne()+";\n\n";
        mainEditor->setText(out);
        return 0;
    }
    else if( pmfTable.TableType() == TYPE_ALIAS )
    {
        out = "------------------------------ \n"+out;
        out = "-- Note that this is an alias -- \n"+out;
        out = "------------------------------ \n"+out;
        m_strCreateTabStmt = "CREATE ALIAS "+iTableName+ " FOR "+pmfTable.BaseTabSchema()+"."+pmfTable.BaseTabName();
        out += m_strCreateTabStmt+";\n\n";
    }
    else if( pmfTable.TableType() == TYPE_MAT_QUERY )
    {
        out = "------------------------------ \n"+out;
        out = "-- Note that this is a Materialized Query Table -- \n"+out;
        out = "------------------------------ \n"+out;
        out += pmfTable.ddlForView().change('\t', ' ').removeButOne()+";\n\n";
        mainEditor->setText(out);
        return 0;
    }
    else
    {
        m_strCreateTabStmt = pmfTable.createTabStmt();
        if( tableSpace(iTableName).length() ) out = m_strCreateTabStmt + " IN "+tableSpace(iTableName)+";\n\n";
        else out = m_strCreateTabStmt+";\n\n";
    }

    GSeq <IDX_INFO*> idxSeq = m_pDSQL->getIndexeInfo(iTableName);
    out += "-- Primary keys --\n";
    in = "";
    for( int i = 1; i <= idxSeq.numberOfElements(); ++i )
    {
        if( idxSeq.elementAtPosition(i)->Type == DEF_IDX_PRIM ) in += idxSeq.elementAtPosition(i)->Stmt+"\n";
    }
    if( in.length() ) out += in+"\n\n";

    in = "";
    out += "-- Foreign keys --\n";
    for( int i = 1; i <= idxSeq.numberOfElements(); ++i )
    {
        if( idxSeq.elementAtPosition(i)->Type == DEF_IDX_FORKEY ) in += idxSeq.elementAtPosition(i)->Stmt+"\n";
    }
    if( in.length() ) out += in+"\n\n";

    out += "-- Indexes --\n";
    in = "";
    for( int i = 1; i <= idxSeq.numberOfElements(); ++i )
    {
        if( idxSeq.elementAtPosition(i)->Type == DEF_IDX_DUPL || idxSeq.elementAtPosition(i)->Type == DEF_IDX_UNQ ) in += idxSeq.elementAtPosition(i)->Stmt+"\n";
    }
    if( in.length() ) out += in+"\n\n";

    idxSeq.deleteAll();

    m_pDSQL->getTriggers(iTableName, &in);
    if( in.length() ) out += in+"\n\n";

    out += "-- Checks --\n";
    in = m_pDSQL->getChecks(iTableName);
    if( in.length() ) out += in+"\n\n";

    mainEditor->setText(out);
    return 0;
}

//GString Getclp::createTableStmt(GString tableName, GSeq<COL_SPEC*> *specSeq)
//{
//    GString out, defVal, colLen;
//    out = ("CREATE TABLE "+tableName+" (\n" );

//    for(int i = 1; i <= (int)specSeq->numberOfElements(); ++i )
//    {
//        defVal = specSeq->elementAtPosition(i)->Default;
//        colLen = specSeq->elementAtPosition(i)->Length;

//        if( defVal.length() ) defVal = " DEFAULT "+defVal+" ";
//        else defVal = " ";

//        if( colLen == "N/A" ) colLen = " ";
//        else colLen = "("+colLen +") ";


//        out += "    \"" +specSeq->elementAtPosition(i)->ColName+ "\"    "+
//                specSeq->elementAtPosition(i)->ColType +" "+
//                colLen +
//                specSeq->elementAtPosition(i)->Nullable+defVal+
//                specSeq->elementAtPosition(i)->Misc+" ";

//        if( i < (int)specSeq->numberOfElements() ) out = out.strip()+",\n";
//        else out+="\n";
//    }
//    if( m_pDSQL->getDBType() == DB2ODBC || m_pDSQL->getDBType() == DB2 ) out = out.stripTrailing(",")+")";
//    else out = out.stripTrailing(",")+");\n\n";
//    return out;
//}

void Getclp::addTableItem(int col, GString txt)
{

    QTableWidgetItem * pItem = new QTableWidgetItem();

    pItem->setFlags(pItem->flags() ^ Qt::ItemIsEditable);
    if( txt.isDigits() && txt.length() ) pItem->setData(Qt::DisplayRole, qlonglong(txt.asLong()));
    else pItem->setData(Qt::DisplayRole, QString::fromLocal8Bit(txt));

    if( col == 0 ) tabWdgt->insertRow(tabWdgt->rowCount());//Create new row. Crude, I know.
    tabWdgt->setItem(tabWdgt->rowCount()-1, col, pItem);
}

void Getclp::msg(GString txt)
{
    txt.sayIt();
}
void Getclp::saveGeometry()
{
    m_qrGeometry = this->geometry();
}
void Getclp::restoreGeometry()
{
    this->setGeometry(m_qrGeometry);
}
void Getclp::closeEvent(QCloseEvent * event)
{
    if( m_pParent ) m_pParent->getClpClosed();
    event->accept();
}
void Getclp::keyPressEvent(QKeyEvent *event)
{
    if( event->key() == Qt::Key_Escape )
    {
        if( m_pParent ) m_pParent->getClpClosed();
    }

}
GString Getclp::tableSpace(GString tableName)
{
    if( m_pDSQL->getDBType() != DB2ODBC && m_pDSQL->getDBType() != DB2 ) return "";
    GString schema = tableName.subString(1, tableName.indexOf(".")-1).strip("\"");
    GString table  = tableName.subString(tableName.indexOf(".")+1, tableName.length()).strip().strip("\"");
    GString err = m_pDSQL->initAll("Select TBSPACE from SYSCAT.TABLES where tabschema='"+schema+"' and tabname='"+table+"'");
    if( err.length() ) return "";
    return m_pDSQL->rowElement(1,1).strip("'");
}

