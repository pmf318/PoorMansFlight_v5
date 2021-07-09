//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include <qcheckbox.h>
//Added by qt3to4:
#include <QLabel>
#include <QGridLayout>
#include <gseq.hpp>
#include <gfile.hpp>
#include <qlayout.h>
#include <QHeaderView>

#include <gstuff.hpp>
#include <dbapiplugin.hpp>
#include "helper.h"
#include "simpleShow.h"
#include "createCheck.h"
#include "pmfTable.h"
#include "tabEdit.h"
#include "gstringlist.hpp"

#include <idsql.hpp>

#ifndef _REOARGALL_
#include "reorgAll.h"
#endif

#ifndef _newIndex_
#include "newIndex.h"
#endif

#ifndef _newForeignKey_
#include "newForeignKey.h"
#endif

#define MODE_RUNSTATS 0
#define MODE_REORG    1


/* These Parameters for RUNSTATS were copied from SQLUTIL.H (and prefixed with "MY_")
 * because we cannot include SQLUTIL.H here (because plugin)
 * This should be moved to the DB2API-Plugin sometime.
 */
#define MY_SQL_STATS_TABLE           'T'  /* TABLEOPT = Table w/o Indexes        */
#define MY_SQL_STATS_BOTH            'B'  /* TABLEOPT = Table and Indexes        */
#define MY_SQL_STATS_INDEX           'I'  /* TABLEOPT = Indexes w/o Table        */
#define MY_SQL_STATS_EXTTABLE_ONLY   'D'  /* TABLEOPT = Table and dist stats     */
#define MY_SQL_STATS_EXTTABLE_INDEX  'E'  /* TABLEOPT = Table and dist stats     */
                                       /* and basic indexes                   */
#define MY_SQL_STATS_EXTINDEX_ONLY   'X'  /* TABLEOPT = Ext stats for indexes    */
                                       /* only                                */
#define MY_SQL_STATS_EXTINDEX_TABLE  'Y'  /* TABLEOPT = Ext stats for indexes    */
                                       /* and basic table stats               */
#define MY_SQL_STATS_ALL             'A'  /* TABLEOPT = Ext stats for indexes    */
                                       /* and table with dist stats           */
#define MY_SQL_STATS_REF             'R'  /* SHAREOPT = Reference                */
#define MY_SQL_STATS_CHG             'C'  /* SHAREOPT = Change                   */


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


ReorgAll::ReorgAll( DSQLPlugin* pDSQL, QWidget *parent, GString tableName, int hideSystables )
    : QDialog( parent )
{

    m_pDSQL = new DSQLPlugin(*pDSQL);
    m_iHideSysTables = hideSystables;
	timer = NULL;
    m_pParent = parent;

	m_mainWdgt = new QTabWidget(this);

	iFullTabName = tableName;
    iTabName   = tableName.subString(tableName.indexOf('.')+1, tableName.length() ).strip().strip("\"");
    iTabSchema = tableName.subString(1, tableName.indexOf('.')-1 ).strip().strip("\"");

    this->resize(900, 480);
	///this->setOKButton("Exit");
	QGridLayout * mainGrid = new QGridLayout(this);
	QPushButton * exitBT = new QPushButton(this);
	exitBT->setText("Exit");
	mainGrid->addWidget(m_mainWdgt, 0, 0, 1,5);
	mainGrid->addWidget(exitBT,1, 0);
	connect(exitBT, SIGNAL(clicked()), SLOT(endIt()));
	theThread = NULL;
	tb = NULL;

	timer = new QTimer( this );
	connect( timer, SIGNAL(timeout()), this, SLOT(timerEvent()) );
	
	//Add stuff to mainWindow
    addColsWdgt();
    addTextLB();
    fillCreateTabLB();

	indexTab();
    if( m_pDSQL->getDBTypeName() == _DB2 || m_pDSQL->getDBTypeName() == _DB2ODBC ) checksTab();
    if( m_pDSQL->getDBTypeName() == _DB2 ) reorgTab();
    if( m_pDSQL->getDBTypeName() == _DB2 ) runstatsTab();
    if( m_pDSQL->getDBTypeName() == _DB2 ) rebindTab();
	alterTab();
    dropTab();
}
void ReorgAll::endIt()
{
	close();
}


/***************************************************************************************
 *
 * DDL
 *
 * ************************************************************************************/

void ReorgAll::addTextLB()
{
    QWidget * pWdgt = new QWidget(this);
    QGridLayout * grid = new QGridLayout(pWdgt);

    saveAsPlainTextBt = new QPushButton("Save as plain text", this);
    connect(saveAsPlainTextBt, SIGNAL(clicked()), SLOT(saveAsPlainTextClicked()));

    saveAsXmlBt = new QPushButton("Save as XML", this);
    connect(saveAsXmlBt, SIGNAL(clicked()), SLOT(saveAsXmlClicked()));

    optionsBt = new QPushButton("Options", this);
    connect(optionsBt, SIGNAL(clicked()), SLOT(optionsClicked()));

    mainEditor = new TxtEdit(NULL, m_pParent);
    mainEditor->setLineWrapMode(QTextBrowser::NoWrap);
    mainEditor->setGeometry( 20, 20, 520, 300);
    if( GString(m_pParent->metaObject()->className()) == GString("TabEdit") )
    {
        ((TabEdit*)m_pParent)->initTxtEditor(mainEditor);
    }
    grid->addWidget(mainEditor, 0, 0, 1, 4);

    grid->addWidget(saveAsXmlBt, 1, 1);
    grid->addWidget(saveAsPlainTextBt, 1, 0);
    grid->addWidget(optionsBt, 1, 3);
    m_mainWdgt->addTab(pWdgt, "DDL");
}

void ReorgAll::saveAsPlainTextClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save", Helper::getLastSelectedPath("getclp")+GString(iFullTabName).removeAll('\"')+".DDL");
    if( fileName.isNull() ) return;
    Helper::setLastSelectedPath("getclp", GStuff::pathFromFullPath(fileName));
    this->saveAsPlainText(fileName, 1);
}

int ReorgAll::saveAsPlainText(GString fileName, int overwrite)
{
    QFileInfo fileInfo(fileName);
    if( fileInfo.exists() && !overwrite ) return 1;

    GFile gf(fileName, 2);

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


    TABLE_PROPS tabProps = m_pDSQL->getTableProps(iFullTabName);
    if( tabProps.TableType == TYPE_TYPED_VIEW || tabProps.TableType == TYPE_UNTYPED_VIEW )
    {
        gf.writeToNewLine("-- Uncomment the next line to drop the view first:");
        gf.writeToNewLine("-- DROP VIEW  "+iFullTabName);
    }
    if( tabProps.TableType == TYPE_TYPED_TABLE || tabProps.TableType == TYPE_UNTYPED_TABLE )
    {
        gf.writeToNewLine("-- Uncomment the next line to drop the table first:");
        gf.writeToNewLine("-- DROP TABLE "+iFullTabName);
    }
    gf.writeToNewLine("");


    gf.writeToNewLine(mainEditor->document()->toPlainText());
    return 0;
}

void ReorgAll::saveAsXmlClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save", Helper::getLastSelectedPath("getclp")+iFullTabName.removeAll('\"')+".XML");
    if( fileName.isNull() ) return;
    Helper::setLastSelectedPath("getclp", GStuff::pathFromFullPath(fileName));
    this->saveAsXml(fileName, 1);
}

int ReorgAll::saveAsXml(GString fileName, int overwrite)
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

    TABLE_PROPS tabProps = m_pDSQL->getTableProps(iFullTabName);
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
}
void ReorgAll::writeAlias(QFile *outFile, OptionsTab* pOptTab, TABLE_PROPS *pProps)
{
    GString createTabStmt = "CREATE ALIAS "+iFullTabName+" FOR "+pProps->BaseTabSchema+"."+pProps->BaseTabName;

    if( pOptTab->getCheckBoxValue(XMLDDL_GROUPNAME, XMLDDL_WRITE_TABSTMT) )
    {
        createTabStmt = " stmt=\""+createTabStmt+";\"";
    }
    GString baseInfo = " base_schema=\""+pProps->BaseTabSchema+"\" base_name=\""+pProps->BaseTabName+"\"";
    writeToUtf8File(outFile, "\t<alias schema=\""+Helper::tableSchema(iFullTabName, m_pDSQL->getDBType())+"\" name=\""+
                    Helper::tableName(iFullTabName, m_pDSQL->getDBType())+"\""+baseInfo+createTabStmt+ " >");
    writeColumns(outFile);
    writeToUtf8File(outFile, "\t</alias>");
}

void ReorgAll::writeTable(QFile *outFile, OptionsTab* pOptTab)
{
    GString createTabStmt = m_strCreateTabStmt.removeAll('\n');
    GString tableSpaceAttribute;

    if( pOptTab->getCheckBoxValue(XMLDDL_GROUPNAME, XMLDDL_WRITE_TABLESPACE) )
    {
        GString tabSpace = tableSpace(iFullTabName);
        tableSpaceAttribute = " tablespace=\""+tabSpace+"\"";
        if( tabSpace.length() ) createTabStmt += " IN "+tabSpace;
    }
    else tableSpaceAttribute = "";

    if( pOptTab->getCheckBoxValue(XMLDDL_GROUPNAME, XMLDDL_WRITE_TABSTMT) )
    {
        createTabStmt = " stmt=\""+GStuff::formatForXml(createTabStmt).removeButOne()+";\"";
    }
    else createTabStmt = "";

    writeToUtf8File(outFile, "\t<table schema=\""+Helper::tableSchema(iFullTabName, m_pDSQL->getDBType())+
                    "\" name=\""+Helper::tableName(iFullTabName, m_pDSQL->getDBType())+"\""+tableSpaceAttribute+createTabStmt+ " >");

    writeColumns(outFile);

    GSeq <IDX_INFO*> idxSeq = m_pDSQL->getIndexeInfo(iFullTabName);
    writePrimKeyInfo(&idxSeq, outFile, pOptTab);
    writeForKeyInfo(&idxSeq, outFile, pOptTab);
    writeIdxInfo(&idxSeq, outFile, pOptTab);

    m_pDSQL->fillChecksView(iFullTabName, 0);
    writeChecks( m_pDSQL, outFile, pOptTab);

    GSeq <GString> trgSeq = m_pDSQL->getTriggerSeq(iFullTabName);
    writeSeqToFile(outFile, &trgSeq, "trigger");
    writeToUtf8File(outFile, "\t</table>");

}

void ReorgAll::writeColumns( QFile *outFile)
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

void ReorgAll::writeMQT(QFile *outFile, OptionsTab* pOptTab)
{
    GString createTabStmt = m_pDSQL->getDdlForView(iFullTabName).removeAll('\n').removeButOne();
    GString tableSpaceAttribute;

    if( pOptTab->getCheckBoxValue(XMLDDL_GROUPNAME, XMLDDL_WRITE_TABLESPACE) )
    {
        GString tabSpace = tableSpace(iFullTabName);
        tableSpaceAttribute = " tablespace=\""+tabSpace+"\"";
        if( tabSpace.length() ) createTabStmt += " IN "+tabSpace;
    }
    else tableSpaceAttribute = "";

    if( pOptTab->getCheckBoxValue(XMLDDL_GROUPNAME, XMLDDL_WRITE_TABSTMT) )
    {
        createTabStmt = " stmt=\""+GStuff::formatForXml(createTabStmt).removeButOne()+";\"";
    }
    else createTabStmt = "";

    writeToUtf8File(outFile, "\t<mqt schema=\""+Helper::tableSchema(iFullTabName, m_pDSQL->getDBType())+
                    "\" name=\""+Helper::tableName(iFullTabName, m_pDSQL->getDBType())+"\""+tableSpaceAttribute+createTabStmt+ " >");

    writeColumns(outFile);
    writeToUtf8File(outFile, "\t</mqt>");
}


void ReorgAll::writeView(QFile *outFile, OptionsTab*)
{
    GString createTabStmt = m_pDSQL->getDdlForView(iFullTabName).removeAll('\n').change('\t', ' ').removeButOne();
    //createTabStmt = linesToSingleLine(createTabStmt).removeButOne().strip();
    createTabStmt = createTabStmt.change(10, ' ').change(13, ' ').change('\t', ' ').removeButOne().strip();
    createTabStmt = " stmt=\""+ GStuff::formatForXml(createTabStmt)+";\"";
    writeToUtf8File(outFile, "\t<view schema=\""+Helper::tableSchema(iFullTabName, m_pDSQL->getDBType())+"\" name=\""+
                    Helper::tableName(iFullTabName, m_pDSQL->getDBType())+"\""+createTabStmt+ " />");
}

void ReorgAll::writeToUtf8File(QFile* fileOut, QString txt, int setBOM)
{
    QTextStream streamFileOut(fileOut);
    //This should be called only once, on the first line/entry
    streamFileOut.setGenerateByteOrderMark(setBOM == 1 ? true : false);
    streamFileOut.setCodec("UTF-8");
    streamFileOut << txt <<"\n";
    streamFileOut.flush();
}

void ReorgAll::optionsClicked()
{
    showOptionsDialog(this);
}

void ReorgAll::showOptionsDialog(QWidget* parent)
{
    OptionsTab* pOptTab = createOptionsTab(XMLDDL_GROUPNAME, parent);
    pOptTab->exec();
    delete pOptTab;
}

OptionsTab* ReorgAll::createOptionsTab(GString groupName, QWidget * parent)
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

void ReorgAll::writeChecks(DSQLPlugin *pDSQL, QFile * outFile, OptionsTab *pOptTab)
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
        stmt = GStuff::formatForXml(m_pDSQL->getChecks(iFullTabName).removeAll('\n'));
        if( writeCreateStmt) entry += " stmt=\""+stmt+"\"";
        entry += "/>";
        writeToUtf8File(outFile, entry);
    }

    writeToUtf8File(outFile, "\t\t</checks>");
}

void ReorgAll::writeIdxInfo(GSeq <IDX_INFO*> * idxSeq, QFile *outFile, OptionsTab *pOptTab)
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

void ReorgAll::writeForKeyInfo(GSeq <IDX_INFO*> * idxSeq, QFile * outFile, OptionsTab *pOptTab)
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


void ReorgAll::writePrimKeyInfo(GSeq <IDX_INFO*> * idxSeq, QFile *outFile, OptionsTab *pOptTab)
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

void ReorgAll::writeSeqToFile(QFile *outFile, GSeq <GString> *entry, GString tag)
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

GString ReorgAll::linesToSingleLine(GString in)
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
    return out.change('\t', ' ').removeButOne();
}

int ReorgAll::isSysIdx(GString idxName)
{
    if( idxName.subString(1, 3) == "SQL" && idxName.subString(4, 6).isDigits() )  return 1;
    return 0;
}

GString ReorgAll::formatColumnsInStmt(GString cols, OptionsTab *pOptTab)
{
    if( !pOptTab->getCheckBoxValue(XMLDDL_GROUPNAME, XMLDDL_COLS_IN_QUOTES) )
    {
        return cols.removeAll('\"');
    }
    return GStuff::formatForXml(cols);
}

GString ReorgAll::tableSpace(GString tableName)
{
    if( m_pDSQL->getDBType() != DB2ODBC && m_pDSQL->getDBType() != DB2 ) return "";
    GString schema = tableName.subString(1, tableName.indexOf(".")-1).strip("\"");
    GString table  = tableName.subString(tableName.indexOf(".")+1, tableName.length()).strip().strip("\"");
    GString err = m_pDSQL->initAll("Select TBSPACE from SYSCAT.TABLES where tabschema='"+schema+"' and tabname='"+table+"'");
    if( err.length() ) return "";
    return m_pDSQL->rowElement(1,1).strip("'");
}

short ReorgAll::fillCreateTabLB()
{

    PmfTable pmfTable(m_pDSQL, iFullTabName);

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
    tabWdgt->setSortingEnabled(true);
    Helper::setVHeader(tabWdgt);
    tabWdgt->verticalHeader()->hide();

    GString out, in;
    if( pmfTable.TableType() == TYPE_TYPED_VIEW || pmfTable.TableType() == TYPE_UNTYPED_VIEW )
    {
        out = "------------------------------ \n";
        out += "-- Note that this is a view -- \n";
        out += "------------------------------ \n";
        out += pmfTable.ddlForView().change('\t', ' ').removeButOne()+";\n\n";
        mainEditor->setText(out);
        return 0;
    }
    else if( pmfTable.TableType() == TYPE_ALIAS )
    {
        out = "------------------------------ \n"+out;
        out = "-- Note that this is an alias -- \n"+out;
        out = "------------------------------ \n"+out;
        m_strCreateTabStmt = "CREATE ALIAS "+iFullTabName+ " FOR "+pmfTable.BaseTabSchema()+"."+pmfTable.BaseTabName();
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
        if( tableSpace(iFullTabName).length() ) out = m_strCreateTabStmt + " IN "+tableSpace(iFullTabName)+";\n\n";
        else out = m_strCreateTabStmt+";\n\n";
    }

    GSeq <IDX_INFO*> idxSeq = m_pDSQL->getIndexeInfo(iFullTabName);
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

    m_pDSQL->getTriggers(iFullTabName, &in);
    if( in.length() ) out += in+"\n\n";

    out += "-- Checks --\n";
    in = m_pDSQL->getChecks(iFullTabName);
    if( in.length() ) out += in+"\n\n";

    mainEditor->setText(out);
    return 0;
}


/***************************************************************************************
 *
 * Columns
 *
 * ************************************************************************************/

void ReorgAll::addColsWdgt()
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

    connect((QWidget*)tabWdgt->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(sortColumnsClicked(int)));
    //fillColumnsTab();
}

void ReorgAll::sortColumnsClicked(int)
{
    Helper::setVHeader(tabWdgt);
}

void ReorgAll::fillColumnsTab()
{
    PmfTable pmfTable(m_pDSQL, iFullTabName);
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
}

void ReorgAll::addTableItem(int col, GString txt)
{

    QTableWidgetItem * pItem = new QTableWidgetItem();

    pItem->setFlags(pItem->flags() ^ Qt::ItemIsEditable);
    if( txt.isDigits() && txt.length() ) pItem->setData(Qt::DisplayRole, qlonglong(txt.asLong()));
    else pItem->setData(Qt::DisplayRole, QString::fromLocal8Bit(txt));

    if( col == 0 ) tabWdgt->insertRow(tabWdgt->rowCount());//Create new row. Crude, I know.
    tabWdgt->setItem(tabWdgt->rowCount()-1, col, pItem);
}


/***************************************************************************************
*
*            ALTER TABLE
*
***************************************************************************************/
void ReorgAll::alterTab()
{
	QWidget * pWdgt = new QWidget(this);
	
	QGridLayout * grid = new QGridLayout(pWdgt);

    GString msg = "Alter table "+formTabName();
    QLabel *s1 = new QLabel( msg, pWdgt );
    s1->setStyleSheet("font-weight: bold;");

	QLabel * txt = new QLabel(pWdgt);
	txt->setText("Column name:");
	colNameLE = new QLineEdit(pWdgt);

    QLabel * hint = new QLabel(pWdgt);
    hint->setStyleSheet("QLabel { color : red; }");
    hint->setText("CAUTION: Column names are case-sensitive!");
	
    grid->addWidget(s1, 0, 0);
    grid->addWidget(txt, 1, 0);
    grid->addWidget(colNameLE, 1,1, 1, 2);
    grid->addWidget(hint, 1,3, 1, 4);
    int offs = 2;
	
    addToGrid( createRow("Integer",      1, pWdgt), grid, ++offs );
    addToGrid( createRow("SmallInt",     1, pWdgt), grid, ++offs );
    addToGrid( createRow("Date",         1, pWdgt), grid, ++offs );
    addToGrid( createRow("Time",         1, pWdgt), grid, ++offs );
    addToGrid( createRow("TimeStamp",    1, pWdgt), grid, ++offs );
    addToGrid( createRow("Long VarChar", 1, pWdgt), grid, ++offs );
    addToGrid( createRow("Long VarChar FOR BIT DATA",   1, pWdgt), grid, ++offs );
    addToGrid( createRow("Decimal",      4, pWdgt), grid, ++offs );
    addToGrid( createRow("Char",         2, pWdgt), grid, ++offs );
    addToGrid( createRow("VarChar",      2, pWdgt), grid, ++offs );
    if( m_pDSQL->getDBTypeName() == _DB2 || m_pDSQL->getDBTypeName() == _DB2ODBC )
    {
        addToGrid( createRow("Graphic",        2, pWdgt), grid, ++offs );
        addToGrid( createRow("VarGraphic",     2, pWdgt), grid, ++offs );
    }
    else
    {
        addToGrid( createRow("NChar",        2, pWdgt), grid, ++offs );
        addToGrid( createRow("NVarChar",     2, pWdgt), grid, ++offs );
    }
    addToGrid( createRow("Char FOR BIT DATA",     5, pWdgt), grid, ++offs );
    addToGrid( createRow("VarChar FOR BIT DATA",   5, pWdgt), grid, ++offs );
    addToGrid( createRow("CLOB",         3, pWdgt), grid, ++offs );
    addToGrid( createRow("BLOB",         3, pWdgt), grid, ++offs );
    if( m_pDSQL->getDBTypeName() == _DB2 || m_pDSQL->getDBTypeName() == _DB2ODBC )
    {
        addToGrid( createRow("DBCLOB",       3, pWdgt), grid, ++offs );
    }
    addToGrid( createRow("XML",          0, pWdgt), grid, ++offs );
	
	altTab = new QPushButton(pWdgt);
    altTab->setText("Add column");
	grid->addWidget(altTab,++offs,0);
	connect(altTab, SIGNAL(clicked()), SLOT(alterTableClicked()));
	m_mainWdgt->addTab(pWdgt, "Alter table");
	
}

void ReorgAll::alterTableClicked()
{
   NEWROW * aRow;
   for( unsigned int i = 1; i <= rowSeq.numberOfElements(); ++ i )
   {
       aRow = rowSeq.elementAtPosition(i);
       if( aRow->typeRB->isChecked() ) 
       {
          alterTable( aRow );
          return;
       }
   }
   tm("Select a column type.");
}


void ReorgAll::alterTable(NEWROW * aRow)
{
   GString colName = GString(colNameLE->text());
   if( !colName.length() ) 
   {
      tm("Specify a column name");
      return;
   }
   colName = "\""+colName+"\"";

   GString cmd = "ALTER TABLE "+formTabName()+" ADD ";
   GString nl, temp;
   
   
   if( aRow->nnCB != NULL )
   {
       if( aRow->nnCB->isChecked() && aRow->rowType > 0 ) nl = " NOT NULL";
       if( GString(aRow->defaultLE->text()).strip().length() ) nl += " DEFAULT "+GString(aRow->defaultLE->text()).strip();
   }
   ///else if( aRow->nnCB->isChecked() )nl = " NOT NULL ";
   else nl = "";
   switch( aRow->rowType )
   {
       case 0:
           cmd += colName+" "+aRow->name;
           break;

       case 1:
           cmd += colName+" "+aRow->name+nl;
           break;
       case 2:
       case 4:
           cmd += colName+" "+aRow->name+" ("+GString(aRow->sizeLE->text())+")"+nl;
           break;
       case 3:
           cmd += colName+" "+aRow->name+" ("+GString(aRow->sizeLE->text());
           if( aRow->mbCB->currentText() == "Byte" ) cmd += ")"+nl;
           if( aRow->mbCB->currentText() == "KB" ) cmd += "K)"+nl;
           if( aRow->mbCB->currentText() == "MB" ) cmd += "M)"+nl;
           if( aRow->logCB->isChecked() ) cmd += " LOGGED ";
           else cmd += " NOT LOGGED ";
           if( aRow->compCB->isChecked() ) cmd += " COMPACT";
           break;
       case 5:
           temp = aRow->name;           
           temp.insert(" ("+GString(aRow->sizeLE->text())+")", temp.length()-12);
           cmd += colName+" "+temp+nl;
           break;
   }
   GString msg = "Executing\n"+cmd+"\nContinue?";
   if( aRow->rowType == 5 && aRow->nnCB->isChecked() )
   {
       msg = "FOR BIT DATA: Make sure that the default string starts with x (for example x'0000') and that it's twice the column size.";
       msg += "\n\n\nExecuting\n"+cmd+"\nContinue?";
   }
   if( QMessageBox::information(this, "PMF", msg, "Yes", "No", 0, 0, 1) ) return;
   GString err = m_pDSQL->initAll(cmd);

/*
   if( aRow->nnCB != NULL )
   {
       if( aRow->nnCB->isChecked() && aRow->defaultLE->text().strip().stripLeading("x").stripLeading("X").strip("'").length() != 2 * GString(aRow->sizeLE->text()).asInt() )
       {
           tm("FOR BIT DATA: Length of default string should be twice the column size. Default must start with x, for example: x'0000' or x'1212' ");
           return;
       }
   }
*/
   if( err.length() )tm(err);
   else if( m_pDSQL->getDBType() == DB2 || m_pDSQL->getDBType() == DB2ODBC)
   {
       tm("Table altered.\n\nHint: You may need to reorganize the table.");
       m_mainWdgt->setCurrentWidget(pWdgtReorg);
   }

}

NEWROW* ReorgAll::createRow(GString buttonName, int type, QWidget* pWdgt)
{

	NEWROW * aRow = new NEWROW;
	rowSeq.add(aRow);
	aRow->rowType = type;
	QRadioButton* aRB = new QRadioButton(pWdgt);
	aRB->setText(buttonName);
	aRow->name = buttonName;
	aRow->typeRB = aRB;
    aRow->nnCB = NULL;
    aRow->defaultLE = NULL;

    if( type == 0 ) //i.e. XML))
    {
        //Apparently, ALTER...ADD XML is not possible w/ NOT NULL
        //aRow->nnCB = new QCheckBox("Not Null");
        return aRow;
    }
    aRow->nnCB = new QCheckBox("NotNull     Default value:");
	aRow->defaultLE = new QLineEdit(pWdgt);
    if( type == 5 )aRow->defaultLE->setPlaceholderText("x'0000...'");
    if( type <= 1 ) return aRow;

	if( type == 4 ) aRow->sizeTxt = new QLabel("Prec, Scale:", pWdgt);
	else aRow->sizeTxt = new QLabel("Size:", pWdgt);
	aRow->sizeLE = new QLineEdit(pWdgt);
    if( type == 2 || type == 4 || type == 5) return aRow;

	aRow->mbCB = new QComboBox(pWdgt);
	aRow->logCB = new QCheckBox("Logged", pWdgt);
	aRow->compCB = new QCheckBox("Compact", pWdgt);

	return aRow;
}

void ReorgAll::addToGrid(NEWROW * aRow, QGridLayout * aGrid, int row)
{

   //Allways these:
   aGrid->addWidget(aRow->typeRB, row, 0);   
   if( aRow->rowType == 0 )return;

   aGrid->addWidget(aRow->nnCB, row, 1);

   aGrid->addWidget(aRow->defaultLE, row, 2);
   if( aRow->rowType == 1 ) return;

   //Cols with "SIZE: "
   aGrid->addWidget(aRow->sizeTxt, row, 3);
   aGrid->addWidget(aRow->sizeLE, row, 4);
   if( aRow->rowType == 2 || aRow->rowType == 4 || aRow->rowType == 5) return;

   aGrid->addWidget(aRow->mbCB, row, 5);
 
   aRow->mbCB->addItem("Byte");
   aRow->mbCB->addItem("KB");
   aRow->mbCB->addItem("MB");

   aGrid->addWidget(aRow->logCB, row, 6);
   aGrid->addWidget(aRow->compCB, row, 7);

}
/***************************************************************************************
*
*            CHECKS
*
***************************************************************************************/
void ReorgAll::checksTab()
{
    QWidget * pWdgt = new QWidget(this);

    QGridLayout * grid = new QGridLayout(pWdgt);
    GString msg = "Checks for table "+formTabName();
    QLabel *s = new QLabel( msg, pWdgt);
    s->setStyleSheet("font-weight: bold;");
    grid->addWidget(s, 0,0, 1, 5);

    checksLV = new QTableWidget(pWdgt);
    connect((QWidget*)checksLV->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(sortClicked(int)));

    grid->addWidget(checksLV, 3,0, 1, 5);
    checksLV->setSelectionBehavior(QAbstractItemView::SelectRows);

    grid->addWidget(new QLabel("Double-click row to see CREATE statement"), 1,0, 1, 5);

    newCheck = new QPushButton(pWdgt);
    newCheck ->setText("New Check");
    connect(newCheck, SIGNAL(clicked()), SLOT(newChecksClicked()));
    grid->addWidget(newCheck, 4,0);

    connect(checksLV, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(checksClicked(QTableWidgetItem*)));

    delCheck = new QPushButton(pWdgt);
    delCheck ->setText("Drop Check");
    connect(delCheck, SIGNAL(clicked()), SLOT(delChecksClicked()));
    grid->addWidget(delCheck, 4,1);

    refreshChecksB = new QPushButton(pWdgt);
    refreshChecksB ->setText("Refresh View");
    connect(refreshChecksB, SIGNAL(clicked()), SLOT(fillChecksLV()));
    grid->addWidget(refreshChecksB, 4,2);

    m_mainWdgt->addTab(pWdgt, "Checks");

    m_pDSQL->fillChecksView(iFullTabName, 0);

    fillListViewFromAPI(m_pDSQL, checksLV);
    Helper::setVHeader(checksLV);
}


void ReorgAll::newChecksClicked()
{
    CreateCheck foo(m_pDSQL, iFullTabName, this);
    foo.exec();
    fillChecksLV();
}

void ReorgAll::delChecksClicked()
{
    QItemSelectionModel* selectionModel = checksLV->selectionModel();
    QModelIndexList selected = Helper::getSelectedRows(selectionModel);

    if( selected.count() == 0 )
    {
        tm("Nothing selected.");
        return;
    }
    for(int i= 0; i< selected.count();i++)
    {
        QModelIndex index = selected.at(i);
        int pos = index.row();

        GString constName = GString(checksLV->item(pos,0)->text());

        if( QMessageBox::question(this, "PMF", "Drop Check "+constName+"?", "Yes", "No", 0, 1) != 0 ) continue;
        GString cmd = "alter table "+formTabName()+" drop constraint \""+constName+"\"";
        GString err = m_pDSQL->initAll(cmd);
        if( err.length() ) tm(err);
    }
    fillChecksLV();

}

void ReorgAll::checksClicked(QTableWidgetItem * pItem)
{
    GString checkName   = checksLV->item(pItem->row(), 0)->text();
    GString checkSchema = checksLV->item(pItem->row(), 1)->text();

    GString checkStmt = m_pDSQL->getChecks(iFullTabName, checkName);

    SimpleShow *foo = new SimpleShow("Check", this);
    foo->setText(checkStmt);
    foo->exec();
}

/***************************************************************************************
*
*            INDEXES
*
***************************************************************************************/
void ReorgAll::indexTab()
{
	QWidget * pWdgt = new QWidget(this); 

	QGridLayout * grid = new QGridLayout(pWdgt);
    GString msg = "Indexes for table "+formTabName();
	QLabel *s = new QLabel( msg, pWdgt);
    s->setStyleSheet("font-weight: bold;");
	grid->addWidget(s, 0,0, 1, 5);
    grid->addWidget(new QLabel("Double-click row to see CREATE statement"), 1,0, 1, 5);
	
	rawIndexCB = new QCheckBox(pWdgt);
	rawIndexCB->setText("Show raw data");
	rawIndexCB->setChecked(false);
    grid->addWidget(rawIndexCB, 2,0, 1, 5);
	connect(rawIndexCB, SIGNAL(stateChanged(int)), SLOT(fillIndLV()));
    if( m_pDSQL->getDBType() != SQLSERVER )
    {
        rawIndexCB->hide();
    }
    indexLV = new QTableWidget(pWdgt);
    connect((QWidget*)indexLV->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(sortClicked(int)));
#if QT_VERSION >= 0x050000
    indexLV->horizontalHeader()->setSectionsMovable(true);
#else
    indexLV->horizontalHeader()->setMovable(true);
#endif

    connect(indexLV, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(indexClicked(QTableWidgetItem*)));

    grid->addWidget(indexLV, 3,0, 1, 5);
    indexLV->setSelectionBehavior(QAbstractItemView::SelectRows);

    newInd = new QPushButton(pWdgt);
    newInd ->setText("New Index");
    connect(newInd, SIGNAL(clicked()), SLOT(newIndClicked()));
    grid->addWidget(newInd, 4,0);

    newFKey = new QPushButton(pWdgt);
    newFKey ->setText("New Foreign Key");
    connect(newFKey, SIGNAL(clicked()), SLOT(newForeignKeyClicked()));
    grid->addWidget(newFKey, 4,1);

	delInd = new QPushButton(pWdgt);
	delInd ->setText("Drop Index");
	connect(delInd, SIGNAL(clicked()), SLOT(delClicked()));
    grid->addWidget(delInd, 4,2);

    refreshIdxB = new QPushButton(pWdgt);
    refreshIdxB ->setText("Refresh View");
    connect(refreshIdxB, SIGNAL(clicked()), SLOT(fillIndLV()));
    grid->addWidget(refreshIdxB, 4,3);

	m_mainWdgt->addTab(pWdgt, "Index");
	
	fillIndLV( );

}

void ReorgAll::indexClicked(QTableWidgetItem * pItem)
{
    SimpleShow *foo = new SimpleShow("Index", this, true);
    //foo->setLineWrapping(QTextEdit::WidgetWidth);
    GString txt = indexLV->item(pItem->row(), 10)->text();
    foo->setText(txt);
    foo->exec();
    return;
}

void ReorgAll::newIndClicked()
{
   NewIndex * ni = new NewIndex(m_pDSQL, this);
   ni->setTableName(iTabSchema, iTabName);
   ni->exec();   
   fillIndLV();
}

void ReorgAll::newForeignKeyClicked()
{
   NewForeignKey * nfk = new NewForeignKey(m_pDSQL, this, iFullTabName, m_iHideSysTables);
   nfk->setTableName(iTabSchema, iTabName);
   nfk->exec();

   fillIndLV();
}

short ReorgAll::fillIndLV()
{  

    indexLV->setSortingEnabled(false);
    indexLV->clear();
    indexLV->setColumnCount(11);

    indexLV->setHorizontalHeaderItem(0, new QTableWidgetItem("IndexId"));
    indexLV->setHorizontalHeaderItem(1, new QTableWidgetItem("Schema"));
    indexLV->setHorizontalHeaderItem(2, new QTableWidgetItem("Name"));
    indexLV->setHorizontalHeaderItem(3, new QTableWidgetItem("Columns"));
    indexLV->setHorizontalHeaderItem(4, new QTableWidgetItem("Type"));
    indexLV->setHorizontalHeaderItem(5, new QTableWidgetItem("Tablespace"));
    indexLV->setHorizontalHeaderItem(6, new QTableWidgetItem("Create Time"));
    indexLV->setHorizontalHeaderItem(7, new QTableWidgetItem("Definer"));
    if( m_pDSQL->getDBType() == DB2 || m_pDSQL->getDBType() == DB2ODBC )
    {
        indexLV->setHorizontalHeaderItem(8, new QTableWidgetItem("Stats Time"));
    }
    else indexLV->setHorizontalHeaderItem(8, new QTableWidgetItem("Modified"));
    indexLV->setHorizontalHeaderItem(9, new QTableWidgetItem("Delete Rule"));
    indexLV->setHorizontalHeaderItem(10, new QTableWidgetItem("Create Stmt"));


    if( m_pDSQL->getDBType() != DB2 && m_pDSQL->getDBType() != DB2ODBC )
    {
        indexLV->hideColumn(5);
        indexLV->hideColumn(7);
    }


    GSeq<IDX_INFO*> idxSeq = m_pDSQL->getIndexeInfo(iFullTabName);

    indexLV->setRowCount(idxSeq.numberOfElements());
    IDX_INFO* pIdx;
    for(int i = 1; i <= idxSeq.numberOfElements(); ++i )
    {
        pIdx = idxSeq.elementAtPosition(i);

        indexLV->setItem(i-1, 0, createReadOnlyItem(pIdx->Iidx));
        indexLV->setItem(i-1, 1, createReadOnlyItem(pIdx->Schema));
        indexLV->setItem(i-1, 2, createReadOnlyItem(pIdx->Name));
        if( pIdx->Type == DEF_IDX_FORKEY )
        {
            GString tmp = pIdx->Columns + " => "+pIdx->RefTab+" ["+pIdx->RefCols+"]";
            indexLV->setItem(i-1, 3, createReadOnlyItem(tmp));
        }
        else indexLV->setItem(i-1, 3, createReadOnlyItem(pIdx->Columns));
        indexLV->setItem(i-1, 4, createReadOnlyItem(pIdx->Type));
        indexLV->setItem(i-1, 5, createReadOnlyItem(pIdx->TabSpace));
        indexLV->setItem(i-1, 6, createReadOnlyItem(pIdx->CreateTime));
        indexLV->setItem(i-1, 7, createReadOnlyItem(pIdx->Definer));
        indexLV->setItem(i-1, 8, createReadOnlyItem(pIdx->StatsTime));
        indexLV->setItem(i-1, 9, createReadOnlyItem(pIdx->DeleteRule));
        indexLV->setItem(i-1, 10, createReadOnlyItem(pIdx->Stmt));

    }
    idxSeq.deleteAll();
    Helper::setVHeader(indexLV);

    return 0;
}

QTableWidgetItem * ReorgAll::createReadOnlyItem(GString txt)
{
    QTableWidgetItem * pItem = new QTableWidgetItem((char*)txt);
    pItem->setFlags(pItem->flags() ^ Qt::ItemIsEditable);
    return pItem;
}

short ReorgAll::fillChecksLV()
{
    m_pDSQL->fillChecksView(iFullTabName, 0);
    fillListViewFromAPI(m_pDSQL, checksLV);
    Helper::setVHeader(checksLV);
    return 0;
}

void ReorgAll::fillListViewFromAPI(DSQLPlugin *pDSQL, QTableWidget *pLV)
{
    pLV->setSortingEnabled(false);
    pLV->clear();

    GString data;
    QTableWidgetItem * pItem;

    pLV->setColumnCount(pDSQL->getHeaderDataCount());

    for( int i = 1; i <= (int)pDSQL->getHeaderDataCount(); ++i)
    {
        pDSQL->getHeaderData(i, &data);        
        pItem = new QTableWidgetItem((char*)data);
        pLV->setHorizontalHeaderItem(i-1, pItem);
    }
    pLV->setRowCount(pDSQL->getRowDataCount());
    //printf("fillListViewFromAPI, start. Cols: %i, rows: %i\n", pDSQL->getHeaderDataCount(), pDSQL->getRowDataCount());
    for(int i = 1; i <= (int)pDSQL->getRowDataCount(); ++i)
    {
        for(int j = 1; j <= (int)pDSQL->getHeaderDataCount(); ++j)
        {
            pDSQL->getRowData(i, j, &data);
            if( pLV->horizontalHeaderItem(j-1)->text() == "COLUMNS" )pItem = new QTableWidgetItem((char*)Helper::createIndexSortString(data));
            else pItem = new QTableWidgetItem((char*)data);
            pItem->setFlags(pItem->flags() ^ Qt::ItemIsEditable);
            pLV->setItem(i-1, j-1, pItem);
            //printf("fillListViewFromAPI, setRow: %i, col: %i, data: %s\n", i-1, j-1, (char*) data);
        }
    }

    for(int j = 0; j < pLV->columnCount(); ++j)
    {
        pLV->resizeColumnToContents ( j );
        if( pLV->columnWidth(j) > 200 ) pLV->setColumnWidth(j, 200);
    }
    pLV->setSortingEnabled(true);
}

void ReorgAll:: delClicked()
{
    GString cmd, indexName, indexSchema;

	QItemSelectionModel* selectionModel = indexLV->selectionModel();
    QModelIndexList selected = Helper::getSelectedRows(selectionModel);

	if( selected.count() == 0 )
	{
		tm("Nothing selected.");
		return;
	}
	for(int i= 0; i< selected.count();i++)
	{
	 	QModelIndex index = selected.at(i);
		int pos = index.row();
		QTableWidgetItem *pItem;

        if( m_pDSQL->getDBTypeName() == _DB2 || m_pDSQL->getDBTypeName() == _DB2ODBC )
		{
            //indexName = GString(indexLV->item(pos, 4)->text())+"."+GString(indexLV->item(pos,0)->text());
            indexName = GString(indexLV->item(pos,2)->text());
            indexSchema = GString(indexLV->item(pos,1)->text());
            pItem = indexLV->item(pos, 3);
            if( pItem == NULL ) return;
            if( GString(pItem->text()).occurrencesOf("Primary") > 0 ) cmd = "alter table "+formTabName()+" drop primary key "+indexName; //Primary Key
            else if( GString(pItem->text()).occurrencesOf("Foreign") > 0 ) cmd = "alter table "+formTabName()+" drop foreign key "+indexName; //Foreign Key
            else cmd = "drop index "+GStuff::wrap(indexSchema)+"."+GStuff::wrap(indexName); //"Ordinary" Index
		}
        else if( m_pDSQL->getDBTypeName() == _MARIADB  )
        {
            indexName = GString(indexLV->item(pos,2)->text());
            if( indexName == "PRIMARY") cmd = "ALTER TABLE "+formTabName()+" DROP PRIMARY KEY";
            else cmd = "drop index "+indexName+" on "+formTabName();
        }
		else
		{
            indexName = "["+GString(indexLV->item(pos, 1)->text()).strip("'")+"]";
			pItem = indexLV->item(pos, 3);
            if( pItem == NULL ) return;
            if( GString(pItem->text()).occurrencesOf("Primary") > 0 ) cmd = "alter table "+formTabName()+" drop constraint "+indexName; //Primary Key
            else cmd = "drop index "+indexName+" on "+iFullTabName; //"Ordinary" Index
		}
		if( QMessageBox::question(this, "PMF", "Drop index "+indexName+"?", "Yes", "No", 0, 1) != 0 ) continue;
        GString err = m_pDSQL->initAll(cmd);
		if( err.length() ) tm(err);
		
	}
    fillIndLV();	
}


/***************************************************************************************
*
*            REORG
*
***************************************************************************************/

void ReorgAll::reorgTab()
{

    pWdgtReorg = new QWidget(this);
    QGridLayout * grid = new QGridLayout(pWdgtReorg);
    reorgLV = new QTableWidget(pWdgtReorg);
    connect((QWidget*)reorgLV->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(sortClicked(int)));
	reorgLV->setSelectionBehavior(QAbstractItemView::SelectRows);  

    GString msg = "Reorganize table "+formTabName();
    QLabel * s1 = new QLabel(msg, pWdgtReorg);
    s1->setStyleSheet("font-weight: bold;");
    QLabel* s2 = new QLabel( "You may select one and only one index for reorganization", pWdgtReorg );

	msg = "You may select a temporary tablespace for reorganization";
    QLabel * s3 = new QLabel( msg, pWdgtReorg );

    tabspcLB = new QListWidget(pWdgtReorg);
	tabspcLB->setSelectionMode(QListWidget::SingleSelection);

    refreshB = new QPushButton(pWdgtReorg);
	refreshB->setText("Refresh View");
    connect(refreshB, SIGNAL(clicked()), SLOT(refreshReorgListView()));

    runB = new QPushButton(pWdgtReorg);
	runB->setText("Reorganize now");
	connect(runB, SIGNAL(clicked()), SLOT(callReorg()));

	grid->addWidget( s1, 0, 0, 1, 4);
	grid->addWidget( s2, 2, 0, 1, 4);
	grid->addWidget( reorgLV, 3, 0, 1, 4);
	grid->addWidget( s3, 4, 0, 1, 4);	
	grid->addWidget( tabspcLB, 5, 0, 1, 4);
	grid->addWidget( refreshB, 6, 0);
	grid->addWidget( runB, 6, 1);

    refreshReorgListView();


    m_pDSQL->initAll("Select tbspace from syscat.tablespaces");
	tabspcLB->addItem("<None>");
    for( unsigned int i = 1; i <= m_pDSQL->numberOfRows(); ++i )
    {
        tabspcLB->addItem(m_pDSQL->rowElement(i,1).strip().strip("'"));
	}
		
    m_mainWdgt->addTab(pWdgtReorg, "Reorganize");
}


void ReorgAll::refreshReorgListView()
{
    GString cmd = "SELECT Creator, NAME As Name, COLNAMES as Columns, "
                  "CASE UNIQUERULE when 'D' then 'Dupl. Allowed' when 'P' then 'Primary' "
                  "when 'U' then 'Unique' else UNIQUERULE end as TYPE, "
                  "CREATE_TIME, STATS_TIME FROM"
                  " SYSIBM.SYSINDEXES WHERE TBCREATOR='"+iTabSchema+"' AND TBNAME='"+iTabName+"'";
	if( fillLV( reorgLV, cmd ) )
	{
		cmd = "SELECT ICREATOR, INAME, COLNAMES FROM SYSTEM.SYSINDEXES WHERE CREATOR='"+iTabSchema+"' AND TNAME='"+iTabName+"'";	
		fillLV( reorgLV, cmd );
	}

}
int ReorgAll::fillLV( QTableWidget * pLV, GString cmd )
{
	//tm("Unify this.");
    pLV->setSortingEnabled(false);
	QTableWidgetItem * pItem;
	pLV->clear();
    GString err = m_pDSQL->initAll(cmd);
	if( err.length()) return 1;


    pLV->setRowCount(m_pDSQL->numberOfRows());
    pLV->setColumnCount(m_pDSQL->numberOfColumns());

    for( unsigned int j = 1; j <= m_pDSQL->numberOfColumns(); ++j)
	{
        pItem = new QTableWidgetItem((char*)m_pDSQL->hostVariable(j));
		pLV->setHorizontalHeaderItem(j-1, pItem);
	}	
    for( unsigned int i = 1; i <= m_pDSQL->numberOfRows(); ++i )
	{
        for( unsigned int j = 1; j <= m_pDSQL->numberOfColumns(); ++ j )
		{
			pItem = new QTableWidgetItem();
            if( m_pDSQL->hostVariable(j) == "COLUMNS" )pItem->setText(Helper::createIndexSortString( m_pDSQL->rowElement(i, j).strip("'").strip()) );
            else pItem->setText(m_pDSQL->rowElement(i, j).strip("'").strip() );
            pItem->setFlags(pItem->flags() ^ Qt::ItemIsEditable);
            pLV->setItem(i-1, j-1, pItem);
		}
	}
    pLV->setSortingEnabled(true);
    Helper::setVHeader(pLV);
	return 0;
}

void ReorgAll::callReorg()
{

	theThread = new TheThread;
	theThread->setMode(MODE_REORG);
    tb = new ThreadBox( this, "Please be patient", "Reorganizing "+formTabName(), m_pDSQL->getDBTypeName() );
	timer->start(200);
	theThread->setOwner( this );
    theThread->setBox(tb);
	theThread->start();
    GStuff::dormez(300);
	tb->exec();
    while( theThread->isAlive() ) GStuff::dormez(100);
    theThread->killBox();
    if( reorgRunstatsErr.length() )
    {
        GStuff::dormez(200);
        tm(reorgRunstatsErr);
    }
}


void ReorgAll::TheThread::run()
{
	if( myMode == MODE_REORG ) myReorg->startReorg();
	else if(myMode == MODE_RUNSTATS) myReorg->startRunstats();
}


void ReorgAll::startReorg()
{
	GString indexName, tabSpace;
	
	QItemSelectionModel* selectionModel = reorgLV->selectionModel();
    QModelIndexList selected = Helper::getSelectedRows(selectionModel);
	if( selected.count() )
	{
		QModelIndex index = selected.at(0);
		int pos = index.row();
        QTableWidgetItem *pItem = indexLV->item(pos, 2);
        if( NULL != pItem )
        {
            indexName = indexLV->item(pos, 1)->text()+"."+ pItem->text();
        }
	}


	for( short i = 1; i < tabspcLB->count(); ++ i ) //i=1: Ignore first entry <None>
	{
		if( tabspcLB->item(i)->isSelected() ) tabSpace = GString(tabspcLB->item(i)->text());
	}
    DBAPIPlugin * pApi = new DBAPIPlugin(m_pDSQL->getDBTypeName());
    reorgRunstatsErr = pApi->reorgTable(formTabName(), indexName, tabSpace);
    m_pDSQL->commit(); //Commit is recommended here
    delete pApi;
}



/***************************************************************************************
*
*            RUNSTATS
*
***************************************************************************************/
void ReorgAll::runstatsTab()
{
    
	QWidget * pWdgt = new QWidget(this); 
	QGridLayout * grid = new QGridLayout(pWdgt);
	runstatsLV = new QTableWidget(pWdgt);
    connect((QWidget*)runstatsLV->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(sortClicked(int)));
	runstatsLV->setSelectionBehavior(QAbstractItemView::SelectRows);  
//    QHeaderView* headerView = new QHeaderView(Qt::Horizontal);
//    runstatsLV->setHorizontalHeader(headerView);


    GString msg = "Runstats on table "+formTabName();
	QLabel *s1 = new QLabel( msg, pWdgt );
    s1->setStyleSheet("font-weight: bold;");
	QLabel *s2 = new QLabel( "You may select multiple indexes for runstats", pWdgt );


	//RadioButtons:

	tableRB       = new QRadioButton("Table only", pWdgt);
	extTabOnlyRB  = new QRadioButton("Extended (distribution) statistics", pWdgt);
	bothRB        = new QRadioButton("Table and Indexes", pWdgt);
	extTabIndexRB = new QRadioButton("Table (with distribution stats) and basic Indexes", pWdgt);
	indexRB       = new QRadioButton("Only Indexes", pWdgt);
	extIndexRB    = new QRadioButton("Extended stats for Indexes only", pWdgt);
	extIndexTabRB = new QRadioButton("Extendended stats for Indexes and basic Table stats", pWdgt);
	statsAllRB    = new QRadioButton("Ext. stats for indexes, table stats with distribution stats (basically all)", pWdgt);
    statsAllRB->setChecked(true);

	shareCB = new QCheckBox("Allow other users read/write access during runstats", pWdgt);

	QPushButton * runstatsB = new QPushButton("Runstats", pWdgt);
	runstatsB->setText("Start Runstats");

    QPushButton * reorgRefreshB = new QPushButton(pWdgt);
    reorgRefreshB->setText("Refresh View");


	grid->addWidget(s1, 0, 0, 1, 4);
	grid->addWidget(s2, 1, 0, 1, 4);
	grid->addWidget(tableRB, 2, 0, 1, 4);
	grid->addWidget(extTabOnlyRB, 3, 0, 1, 4);
	grid->addWidget(bothRB, 4, 0, 1, 4);
	grid->addWidget(extTabIndexRB, 5, 0, 1, 4);
	grid->addWidget(indexRB, 6, 0, 1, 4);
	grid->addWidget(extIndexRB, 7, 0, 1, 4);
	grid->addWidget(extIndexTabRB, 8, 0, 1, 4);
	grid->addWidget(statsAllRB, 9, 0, 1, 4);
	grid->addWidget(shareCB, 10, 0, 1, 4);
	grid->addWidget(runstatsLV, 11, 0, 1, 4);
    grid->addWidget(reorgRefreshB, 12, 0);
    grid->addWidget(runstatsB, 12, 1);
    connect(reorgRefreshB, SIGNAL(clicked()), SLOT(refreshRunstatsListView()));
	connect(runstatsB, SIGNAL(clicked()), SLOT(callRunstats()));

    refreshRunstatsListView();
	m_mainWdgt->addTab(pWdgt, "Runstats");
}


void ReorgAll::refreshRunstatsListView()
{
    GString cmd = "SELECT Creator, NAME As Name, COLNAMES as Columns, "
            "CASE UNIQUERULE when 'D' then 'Dupl. Allowed' when 'P' then 'Primary' "
            "when 'U' then 'Unique' else UNIQUERULE end as Type, "
            "CREATE_TIME, STATS_TIME FROM "
            "SYSIBM.SYSINDEXES WHERE TBCREATOR='"+iTabSchema+"' AND TBNAME='"+iTabName+"'";

    if( fillLV( runstatsLV, cmd ) )
    {
        cmd = "SELECT ICREATOR, INAME, COLNAMES FROM SYSTEM.SYSINDEXES WHERE CREATOR='"+iTabSchema+"' AND TNAME='"+iTabName+"'";
        fillLV( runstatsLV, cmd );
    }
}

void ReorgAll::callRunstats()
{

    theThread = new TheThread;
    theThread->setMode(MODE_RUNSTATS);
    tb = new ThreadBox( this, "Please wait...", "Runstats on "+formTabName(), m_pDSQL->getDBTypeName() );
    timer->start(100);
    theThread->setOwner( this );
    theThread->setBox(tb);

    theThread->start();
    GStuff::dormez(300);
    tb->exec();

    //Wait for thread to finish:
    while(theThread->isAlive()) GStuff::dormez(200);
    theThread->killBox();
    if( reorgRunstatsErr.length() )
    {
        GStuff::dormez(200);
        tm(reorgRunstatsErr);
    }
    printf("REFR\n");
    refreshRunstatsListView();
    refreshReorgListView();
}
void ReorgAll::startRunstats()
{

    unsigned char opt, shr;
    if( tableRB->isChecked() )      opt = MY_SQL_STATS_TABLE;
    if(extTabOnlyRB->isChecked() )  opt = MY_SQL_STATS_EXTTABLE_ONLY;
    if(bothRB->isChecked() )        opt = MY_SQL_STATS_BOTH;
    if(extTabIndexRB->isChecked() ) opt = MY_SQL_STATS_EXTTABLE_INDEX;
    if(indexRB->isChecked() )       opt = MY_SQL_STATS_INDEX;
    if(extIndexRB->isChecked() )    opt = MY_SQL_STATS_EXTINDEX_ONLY;
    if(extIndexTabRB->isChecked() ) opt = MY_SQL_STATS_EXTINDEX_TABLE;
    if(statsAllRB->isChecked() )    opt = MY_SQL_STATS_ALL;

    if( shareCB->isChecked() ) shr = MY_SQL_STATS_REF;
    else shr = MY_SQL_STATS_CHG;

	GSeq <GString> indSeq;
	QTableWidgetItem * pItem;

	
	QItemSelectionModel* selectionModel = runstatsLV->selectionModel();
    QModelIndexList selected = Helper::getSelectedRows(selectionModel);

	for(int i= 0; i< selected.count();i++)
	{
	 	QModelIndex index = selected.at(i);
		pItem = runstatsLV->item(index.row(), 0);
        if( NULL != pItem ) indSeq.add( GString(pItem->text())+"."+GString(runstatsLV->item(index.row(), 1)->text()) );
	}
    DBAPIPlugin *pAPI = new DBAPIPlugin(m_pDSQL->getDBTypeName());
    if( !pAPI )
    {
        reorgRunstatsErr = "Cannot load DBApi-Plugin.";
        return;
    }
    reorgRunstatsErr = pAPI->runStats(formTabName(), &indSeq, opt, shr);
    m_pDSQL->commit(); //Commit is recommended here
    indSeq.removeAll();
    delete pAPI;

}

void ReorgAll::timerEvent()
{

	if( !theThread ) return;
    if( !theThread->isAlive() && tb != NULL  )
	{
        if( tb->isVisible() ) tb->close();
        timer->stop();
	}    
}


/***************************************************************************************
*
*   REBIND
*
***************************************************************************************/

void ReorgAll::rebindTab()
{

	QWidget * pWdgt = new QWidget(this); 
	QGridLayout * grid = new QGridLayout(pWdgt);


	GString msg = "Select the packages for rebind.";
	QLabel * s1 = new QLabel( msg, pWdgt);
	msg = "Please note that packages MUST NOT be in use by other users or rebind will be blocked.";
	QLabel * s2 = new QLabel( msg, pWdgt );
	msg = "You may remove packages from the list and save/load the list.";
	QLabel * s3 = new QLabel( msg, pWdgt );

	bindLB = new QListWidget( pWdgt);
	bindLB->setSelectionMode(QListWidget::ExtendedSelection);


	QPushButton * selFilesB = new QPushButton(pWdgt);
	selFilesB->setText("Show all packages");

	QPushButton * takeItemB = new QPushButton(pWdgt);
	takeItemB->setText("Remove from List");

	QPushButton * saveB = new QPushButton(pWdgt);
	saveB->setText("Save List");

	QPushButton * loadB = new QPushButton(pWdgt);
	loadB->setText("Load List");

	QPushButton * runRebindB = new QPushButton(pWdgt);
	runRebindB->setText("Bind");

	connect(selFilesB, SIGNAL(clicked()), SLOT(getPackages()));
	connect(saveB, SIGNAL(clicked()), SLOT(saveList()));
	connect(loadB, SIGNAL(clicked()), SLOT(loadList()));
	connect(takeItemB, SIGNAL(clicked()), SLOT(takeIt()));
	connect(runRebindB, SIGNAL(clicked()), SLOT(callRebind()));
	grid->addWidget(s1, 0, 0, 1, 6);
	grid->addWidget(s2, 1, 0, 1, 6);
	grid->addWidget(s3, 2, 0, 1, 6);
	grid->addWidget(bindLB, 3, 0, 1, 6);
	grid->addWidget(selFilesB, 4, 0);
	grid->addWidget(takeItemB, 4, 1);
	grid->addWidget(saveB, 4, 2);
	grid->addWidget(loadB, 4, 3);
	grid->addWidget(runRebindB, 4, 5);
	m_mainWdgt->addTab(pWdgt, "Rebind");
	getPackages();
}

void ReorgAll::takeIt()
{
	int count = bindLB->count();
	for (long i =0; i<count; ++i )
	{
		if( bindLB->item(i)->isSelected() ) 
		{
			bindLB->takeItem(i--); 
			count--;
		}
	}
}

void ReorgAll::saveList()
{
	QString name = QFileDialog::getSaveFileName(this, "*.*", "");
	if(GString(name).length() == 0) return;
	GFile f((char*) GString(name), GF_OVERWRITE);
	if( !f.initOK() )
	{
		tm("Could not open File "+GString(name));
		return;
	}
	for (long i =0; i < bindLB->count(); ++i )
	{
		if( GString(bindLB->item(i)->text()).length() ) f.writeToNewLine(GString(bindLB->item(i)->text()));
	}

}


void ReorgAll::getPackages()
{
    if( m_pDSQL->getDBTypeName() != "DB2"  )
    {
        bindLB->addItem("<DB2 only>");
        return;
    }
	GString package;
	bindLB->clear();
    GString err= m_pDSQL->initAll("SELECT PKGSCHEMA, PKGNAME from syscat.packages order by PKGSCHEMA, PKGNAME");
	if(err.length() )tm(err);
    for(unsigned i = 1; i <= m_pDSQL->numberOfRows(); ++ i)
	{
        package = m_pDSQL->rowElement(i,1).strip("'").strip()+"."+m_pDSQL->rowElement(i,2).strip("'").strip();
		bindLB->addItem(package);
	}
	
}


void ReorgAll::loadList()
{

	bindLB->clear();
	QString name = QFileDialog::getOpenFileName(this, "*.*", "");
	if(GString(name).length() == 0) return;
	QFile f(name);
	QString line;
	if( !f.open(QIODevice::ReadOnly ))
	{
		tm("Could not open file "+GString(name)+", permission denied.");
		return;
	}
	f.close();
	QTextStream in(&name);
	while (!in.atEnd()) {
		QString line = in.readLine();
		bindLB->addItem(line);
	}
	
}

void ReorgAll::getBindFiles()
{
	QStringList files = QFileDialog::getOpenFileNames ( this,
					    "Select file(s) to bind", 
					    ".", 
					    ".bnd"); 
	QStringList list = files;
	QStringList::Iterator it = list.begin();
	while(it != list.end()) {
		bindLB->addItem(*it);
		++it;
	}					    
}

void ReorgAll::callRebind()
{


    DBAPIPlugin * pApi = new DBAPIPlugin(m_pDSQL->getDBTypeName());
    GString err;
    GString fileName;
    unsigned long i, count = 0;
    for( int x = 0; x < bindLB->count(); ++x )
    {
          if( bindLB->item(x)->isSelected() ) count++;    
    }
    QProgressDialog apd(GString("Binding..."), "Cancel", 0, count, this); 
    apd.setWindowModality(Qt::WindowModal);
    apd.setValue(0);
     
    //apd.setCaption("DSQLOBJ");
    for( i = 0; i < count; ++i )
    {
        apd.setValue(i);

        if( apd.wasCanceled() ) break;
        fileName = bindLB->item(i)->text();
        //Better not try rebinding our bnd.file
        if( fileName.occurrencesOf("PMF") > 0 )
        {
            bindLB->item(i)->setSelected(false);
            continue;
        }
        if( !bindLB->item(i)->isSelected() ) continue;
        bindLB->setCurrentRow(i);
        apd.setLabelText(fileName);
        err = pApi->rebind(fileName);
        if( err.length() )
        {
            if( QMessageBox::information(this, "PMF", "Error "+err+" binding "+fileName+"\n\nContinue?", "Yes", "No", 0, 0, 1) ) return;
        }
        else bindLB->item(i)->setSelected(false);
        m_pDSQL->commit();
    }
    apd.setValue(count);
    tm("Rebind finished.");
    delete pApi;
}
/***************************************************************************************
*
*   DROP or RENAME COLUMN
*
***************************************************************************************/

void ReorgAll::dropTab()
{

    QWidget * pWdgt = new QWidget(this);
    QGridLayout * grid = new QGridLayout(pWdgt);

    QLabel *s1 = new QLabel( "Rename or remove columns from table "+formTabName(), pWdgt );
    s1->setStyleSheet("font-weight: bold;");

    GString msg = "Edit column names or select columns to be dropped.";
    QLabel * s2 = new QLabel( msg, pWdgt);
    s2->setStyleSheet("font-weight: bold;");

    dropLV = new QTableWidget( pWdgt);
    connect((QWidget*)dropLV->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(sortClicked(int)));
    dropLV->setSortingEnabled(true);
    dropLV->setSelectionBehavior(QAbstractItemView::SelectRows);
    //dropLW->setSelectionMode(QListWidget::ExtendedSelection);

    QPushButton * selSaveB = new QPushButton(pWdgt);
    selSaveB->setText("Save");

    QPushButton * selDropB = new QPushButton(pWdgt);
    selDropB->setText("Drop column(s)");

    QPushButton* refreshDropB = new QPushButton(pWdgt);
    refreshDropB ->setText("Refresh View");

    connect(selSaveB, SIGNAL(clicked()), SLOT(saveChanges()));
    connect(selDropB, SIGNAL(clicked()), SLOT(dropCols()));
    connect(refreshDropB, SIGNAL(clicked()), SLOT(fillColLV()));

    grid->addWidget(s1, 0, 0, 1, 6);
    grid->addWidget(s2, 1, 0, 1, 6);
    grid->addWidget(dropLV, 3, 0, 1, 6);
    grid->addWidget(selSaveB, 4, 0);
    grid->addWidget(refreshDropB, 4, 1);
    grid->addWidget(selDropB, 4, 2);

    fillColLV();    
    Helper::setVHeader(dropLV);
    m_mainWdgt->addTab(pWdgt, "Rename/drop columns");
}

void ReorgAll::itemChg(QTableWidgetItem * pItem)
{
    //dropLW->setSortingEnabled(false);
    QFont font = this->font();
    font.setBold(true);
}

void ReorgAll::fillColLV()
{
    disconnect(dropLV, SIGNAL(itemChanged(QTableWidgetItem*) ), this, SLOT(itemChg(QTableWidgetItem*)));
    dropLV->clear();

    PmfTable pmfTable(m_pDSQL, iFullTabName);

    m_pDSQL->initAll("Select * from "+formTabName(), 1);

    dropLV->setColumnCount(4);
    dropLV->setRowCount(m_pDSQL->numberOfColumns());
    dropLV->setHorizontalHeaderItem(0, new QTableWidgetItem("Pos"));
    dropLV->setHorizontalHeaderItem(1, new QTableWidgetItem("Column"));
    dropLV->setHorizontalHeaderItem(2, new QTableWidgetItem("Type"));
    dropLV->setHorizontalHeaderItem(3, new QTableWidgetItem("Size"));

    QTableWidgetItem * pItem;
    for( unsigned int i = 1; i <= pmfTable.columnCount(); ++ i )
    {
        pItem = new QTableWidgetItem();
        pItem->setData(Qt::DisplayRole, qlonglong(i));
        pItem->setFlags(pItem->flags() ^ Qt::ItemIsEditable);
        pItem->setTextAlignment( Qt::AlignRight | Qt::AlignVCenter);
        dropLV->setItem(i-1, 0, pItem);

        pItem = new QTableWidgetItem(QString::fromLocal8Bit(GString(pmfTable.column(i)->colName())));
        dropLV->setItem(i-1, 1, pItem);
        dropLV->setRowHeight(i-1, QFontMetrics( dropLV->font()).height()+5);

        pItem = new QTableWidgetItem(QString::fromLocal8Bit(GString(pmfTable.column(i)->colType())));
        dropLV->setItem(i-1, 2, pItem);
        dropLV->setRowHeight(i-1, QFontMetrics( dropLV->font()).height()+5);

        pItem = new QTableWidgetItem(QString::fromLocal8Bit(GString(pmfTable.column(i)->colLength())));
        dropLV->setItem(i-1, 3, pItem);
        dropLV->setRowHeight(i-1, QFontMetrics( dropLV->font()).height()+5);

    }
    connect(dropLV, SIGNAL(itemChanged(QTableWidgetItem*) ), this, SLOT(itemChg(QTableWidgetItem*)));
}

void ReorgAll::saveChanges()
{
    GString orgVal, newVal, cmd, err, tabName;
    int index;
    DSQLPlugin *pDSQL = new DSQLPlugin(*m_pDSQL);

    PmfTable pmfTable(m_pDSQL, iFullTabName);

    pDSQL->setDatabaseContext(iFullTabName.subString(1, iFullTabName.indexOf(".")-1).strip("\"").strip("\'"));
    for( int i = 0; i < dropLV->rowCount(); ++i )
    {
        index = GString(dropLV->item(i, 0)->text()).asInt();
        orgVal = pmfTable.column(i+1)->colName();
        newVal = GString(dropLV->item(i, 1)->text());
        if( orgVal != newVal)
        {
            if( pDSQL->getDBType() == DB2 || pDSQL->getDBType() == DB2ODBC )
            {
                cmd = "ALTER TABLE "+formTabName()+" RENAME COLUMN "+ GStuff::setDbQuotes(orgVal)+ " TO "+GStuff::setDbQuotes(newVal);
            }
            else if( pDSQL->getDBType() == SQLSERVER )
            {
                tabName = iTabName;
                cmd = "sp_RENAME '"+tabName.removeAll('\"')+".["+orgVal+"]', '"+newVal+"', 'COLUMN'";
            }
            else if( pDSQL->getDBType() == MARIADB )
            {
                tabName = iTabName;
                cmd = "ALTER TABLE "+tabName.removeAll('\"')+" CHANGE "+orgVal+"  "+newVal + " "+GString(dropLV->item(i, 2)->text());
                if( GString(dropLV->item(i, 3)->text()) != "N/A" ) cmd += " ("+GString(dropLV->item(i, 3)->text())+")";
            }
            err = pDSQL->initAll(cmd);
            if( err.length() ) tm(err);
        }
    }
    delete pDSQL;
    fillColLV();
}

void ReorgAll::dropCols()
{
    GString err;
    QItemSelectionModel* selectionModel = dropLV->selectionModel();
    QModelIndexList selected = Helper::getSelectedRows(selectionModel);

    if( selected.count() == 0 )
    {
        QMessageBox::information(this, "PMF", "Select column(s) to drop");
        return;
    }
    if( QMessageBox::information(this, "PMF", "Dropping "+GString(selected.count())+" column(s), continue?", "Yes", "No", 0, 0, 1) ) return;

    for (long i = 0; i < selected.count(); ++i )
    {
        QModelIndex index = selected.at(i);
        err = m_pDSQL->initAll("ALTER TABLE "+formTabName()+" DROP COLUMN \""+(GString(dropLV->item(index.row(), 1)->text()) )+"\"");
        if( err.length() ) break;
    }
    fillColLV();
    if( err.length() )tm(err);
    else if( m_pDSQL->getDBType() == DB2 || m_pDSQL->getDBType() == DB2ODBC )
    {
        tm("Hint: You should reorganize the table.");
        m_mainWdgt->setCurrentWidget(pWdgtReorg);
    }    
}

void ReorgAll::setTabIndex(int pos)
{
    m_mainWdgt->setCurrentIndex(pos);
}


ReorgAll::~ReorgAll()
{
    delete m_pDSQL;
	if( timer ) delete timer;
}

GString ReorgAll::formTabName()
{
    if( m_pDSQL->getDBType() == MARIADB ) return iTabSchema+"."+iTabName;
    return GStuff::setDbQuotes(iTabSchema)+"."+GStuff::setDbQuotes(iTabName);
}

void ReorgAll::sortClicked(int)
{
    //nice:
    QObject* obj = sender();
    if( obj->metaObject()->className() == reorgLV->horizontalHeader()->metaObject()->className() )
    {
        Helper::setVHeader((QTableWidget*)obj->parent());
        return;
    }
    //fallback:
    Helper::setVHeader(reorgLV);
    Helper::setVHeader(runstatsLV);
    Helper::setVHeader(indexLV);
    Helper::setVHeader(checksLV);
    Helper::setVHeader(dropLV);
}
