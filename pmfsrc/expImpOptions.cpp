 //
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2018
//

#include "expImpOptions.h"

#include "helper.h"
#include "clickLabel.h"
#include "pmfdefines.h"
#include <qlayout.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QVBoxLayout>
#include <gfile.hpp>
#include <gstuff.hpp>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QLineEdit>


#define ROW_CHECKBOX 1
#define ROW_LINEEDIT 2
#define ROW_COMMENT 3





ExpImpOptions::ExpImpOptions(QWidget *parent, Mode mode, GString dbName) :QDialog(parent)
{
    this->resize(480,180);
    this->setWindowTitle("Additional options");
    m_Mode = mode;

    m_mainWdgt = new QTabWidget(this);
    QGridLayout * mainGrid = new QGridLayout(this);


    QPushButton * ok = new QPushButton("OK", this);
    QPushButton * cancel = new QPushButton("Cancel", this);
    QPushButton * saveSettings = new QPushButton("Save settings", this);
    QPushButton * clearSettings = new QPushButton("Restore default", this);
    connect(ok, SIGNAL(clicked()), this, SLOT(OKClicked()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(CancelClicked()));
    connect(saveSettings, SIGNAL(clicked()), this, SLOT(saveSettingsClicked()));
    connect(clearSettings, SIGNAL(clicked()), this, SLOT(clearSettingsClicked()));
    mainGrid->addWidget(m_mainWdgt, 0, 0, 1,5);
    mainGrid->addWidget(ok, 2, 0);
    mainGrid->addWidget(cancel, 2, 1);
    mainGrid->addWidget(saveSettings, 2, 2);
    mainGrid->addWidget(clearSettings, 2, 3);

    this->createEntries(mode);
    mainGrid->addWidget(new QLabel("Note that some options are mutually exclusive and may not work on all DB2 versions."),1, 0, 1, 4    );
    if( mode == MODE_LOAD || mode == MODE_LOAD_HADR )
    {
        createTab( "Load options", TYP_LOAD);
    }
    else
    {
        createTab( "All files", TYP_ALL);
        createTab( "DEL files", TYP_DEL);
        createTab( "IXF files", TYP_IXF);
        createTab( "LOB options", TYP_LOB);
    }
    if( mode == MODE_IMPORT ) m_strSettingsFileName = dbName+"_importSettings";
    else if( mode == MODE_LOAD || mode == MODE_LOAD_HADR )m_strSettingsFileName = dbName+"_loadSettings";
    else if( mode == MODE_EXPORT ) m_strSettingsFileName = dbName+"_exportSettings";
    loadPreviousSettings();     
    if( mode == MODE_LOAD_HADR ) setHadrPath();
}



ExpImpOptions::~ExpImpOptions()
{
    while(m_rowsSeq.numberOfElements())
    {
        OPTIONSROW *pRow;
        pRow = m_rowsSeq.firstElement();
        m_rowsSeq.removeFirst();
        delete pRow;
    }
}

void ExpImpOptions::saveSettingsClicked()
{
    saveSettingsForAllRows();
}

void ExpImpOptions::clearSettingsClicked()
{
    setRowsToDefault();
    if( m_Mode == MODE_LOAD_HADR ) setHadrPath(1);
}

void ExpImpOptions::createTab( GString tabTitle, FileType fileType)
{
    int i, count = 0;
    OPTIONSROW * pRow;
    for( i = 1; i <= (int)m_rowsSeq.numberOfElements(); ++i )
    {
        pRow = m_rowsSeq.elementAtPosition(i);
        if( fileType != pRow->FileType ) continue;
        count++;

    }
    if( !count ) return;

    QWidget * pWdgt = new QWidget(this);
    QGridLayout * grid = new QGridLayout(pWdgt);


    for( i = 1; i <= (int)m_rowsSeq.numberOfElements(); ++i )
    {
        pRow = m_rowsSeq.elementAtPosition(i);
        if( fileType != pRow->FileType ) continue;
        displayRow(m_rowsSeq.elementAtPosition(i), grid, i-1);
        grid->setRowStretch(i-1, 0);
        grid->setColumnStretch(i-1, 0);
    }
    grid->setColumnStretch(i-1, 1);
    grid->setRowStretch(i-1, 1);
    m_mainWdgt->addTab(pWdgt, tabTitle);
}

void ExpImpOptions::createEntries(Mode mode)
{

    if(mode == MODE_EXPORT)
    {
        createRow( TYP_DEL, "coldel", ROW_LINEEDIT, ",", "Column delimiter, default: comma (,)");
        createRow( TYP_DEL, "chardel", ROW_LINEEDIT, "\"", "String delimiter, default: double quotation mark (\")",2);
        createRow( TYP_DEL, "decpt", ROW_LINEEDIT, ".", "Decimal point, default: point (.)");
        createRow( TYP_DEL, "dldel", ROW_LINEEDIT, ";", "DATALINK delimiter, default: semicolon (;)");
        createRow( TYP_DEL, "timestampformat=", ROW_LINEEDIT, "", "Ex.: \"YYYY-MM-DD HH:MM:SS\" [in double-quotes(\")], default: Not set", 0);
        createRow( TYP_DEL, "nochardel ", ROW_CHECKBOX, "", "Suppresses column delimiters (dangerous), default: NOT SET");
        createRow( TYP_DEL, "nodoubledel ", ROW_CHECKBOX, "", "Suppresses recognition of double character delimiters, default: NOT SET");
        createRow( TYP_DEL, "datesiso", ROW_CHECKBOX, "", "Export date in ISO format YYYY-MM-DD, default: NOT SET");
        createRow( TYP_DEL, "decplusblank ", ROW_CHECKBOX, "", "Plus sign char, blank space instead of '+', default: NOT SET");
        createRow( TYP_DEL, "striplzeros ", ROW_CHECKBOX, "", "Strip leading zeroes from DECIMAL, default: NOT SET");
        createRow( TYP_DEL, "codepage=", ROW_LINEEDIT, "", "Codepage as string [in double-quotes(\")], default: Not set", 0);
        createRow( TYP_IXF, "codepage=", ROW_LINEEDIT, "", "Codepage as string [in double-quotes(\")], default: Not set", 0);
        createRow( TYP_LOB, "xmlinsepfiles", ROW_CHECKBOX, "", "Write each XML into seperate files, default: NOT SET");
        createRow( TYP_LOB, "lobsinsepfiles", ROW_CHECKBOX, "", "Write each LOB into seperate files, default: NOT SET");
        createRow( TYP_LOB, "xmlnodeclaration", ROW_CHECKBOX, "", "Suppress XML declaration, default: NOT SET");
        createRow( TYP_LOB, "xmlchar", ROW_CHECKBOX, "", "Use codepage from 'codepage' (see DEL/IXF tab), default: NOT SET (UNICODE)");
        createRow( TYP_LOB, "xmlgraphic", ROW_CHECKBOX, "", "Encode XML in UTF16, default: NOT SET (UNICODE)");
        createRow( TYP_LOB, "");
        createRow( TYP_LOB, "The following is not a DB2  param. It forces the creation of multiple files of same size. Useful when exporting over 4GB.");
        createRow( TYP_LOB, "Alternatively, check 'lobsinsepfiles' above.");

        createRow( TYP_LOB, "filecount", ROW_LINEEDIT, "1", "Split result into multiple files, default: 1 ", 5, 1);

    }
    else if( mode == MODE_IMPORT)
    {
        createRow( TYP_ALL, "compound=", ROW_LINEEDIT, "", "1-100, use nonatomic compound SQL to insert, x statements will be attempted each time, default: NOT SET", 0);
        createRow( TYP_ALL, "generatedignore", ROW_CHECKBOX, "", "Ignore data in GENERATED columns, default: NOT SET");
        createRow( TYP_ALL, "generatedmissing", ROW_CHECKBOX, "", "Create data for GENERATED columns, default: NOT SET");
        createRow( TYP_ALL, "identityignore", ROW_CHECKBOX, "", "Ignore data for IDENTITY columns, default: NOT SET");
        createRow( TYP_ALL, "identitymissing", ROW_CHECKBOX, "", "Create data for IDENTITY columns, default: NOT SET");
        createRow( TYP_ALL, "no_type_id", ROW_CHECKBOX, "", "Import into single sub table, default: NOT SET");
        createRow( TYP_ALL, "nodefaults", ROW_CHECKBOX, "", "Do not load default values, default: NOT SET");
        createRow( TYP_ALL, "norowwarnings", ROW_CHECKBOX, "", "Suppress warnings about rejected rows, default: NOT SET");
        createRow( TYP_ALL, "rowchangetimestampignore", ROW_CHECKBOX, "", "Ignore data for row change timestamp column, default: NOT SET");
        createRow( TYP_ALL, "rowchangetimestampmissing", ROW_CHECKBOX, "", "Create data for row change timestamp column, default: NOT SET");
        createRow( TYP_ALL, "seclabelchar", ROW_CHECKBOX, "", "Security labels are in string format (not encoded numeric format), default: NOT SET");
        createRow( TYP_ALL, "seclabelchar", ROW_CHECKBOX, "", "Security labels are indicated by name, default: NOT SET");



        createRow( TYP_DEL, "coldel", ROW_LINEEDIT, ",", "Column delimiter, default: comma (,) [Hint: Setting as HEX (0xNN) may work better]", 0);
        createRow( TYP_DEL, "chardel", ROW_LINEEDIT, "\"", "String delimiter, default: double quotation mark (\")", 0);
        createRow( TYP_DEL, "decpt", ROW_LINEEDIT, ".", "Decimal point, default: point (.)", 0);
        createRow( TYP_DEL, "nodoubledel ", ROW_CHECKBOX, "", "Suppresses recognition of double character delimiters, default: NOT SET");
        createRow( TYP_DEL, "keepblanks", ROW_CHECKBOX, "", "Preserve leading and trailing blanks in string types, default: NOT SET");
        createRow( TYP_DEL, "decplusblank", ROW_CHECKBOX, "", "Plus sign char, blank space instead of '+', default: NOT SET");
        createRow( TYP_DEL, "delprioritychar", ROW_CHECKBOX, "", "Revert delimiter priorities, default: NOT SET");
        createRow( TYP_DEL, "nochardel ", ROW_CHECKBOX, "", "Suppresses column delimiters (dangerous), default: NOT SET");
        createRow( TYP_DEL, "codepage=", ROW_LINEEDIT, "", "Codepage as string, default: Not set", 0);
        createRow( TYP_DEL, "dateformat=", ROW_LINEEDIT, "", "Ex.: \"YYYY-MM-DD\" [in double-quotes(\")], default: Not set", 0);
        createRow( TYP_DEL, "timeformat=", ROW_LINEEDIT, "", "Ex.: \"HH-MM-SS\" [in double-quotes(\")], default: Not set", 0);
        createRow( TYP_DEL, "implieddecimal ", ROW_CHECKBOX, "", "Location of decimal point is determined by column definition, default: NOT SET");
        createRow( TYP_DEL, "timestampformat=", ROW_LINEEDIT, "", "Ex.: \"YYYY-MM-DD HH:MM:SS\" [in double-quotes(\")], default: Not set", 0);
        createRow( TYP_DEL, "usedefaults", ROW_CHECKBOX, "", "Use default values when column data is missing, default: NOT SET");
        createRow( TYP_DEL, "usegraphiccodepage ", ROW_CHECKBOX, "", "Use graphic code page for DBCLOB, default: NOT SET");        


        createRow( TYP_LOB, "xmlchar", ROW_CHECKBOX, "", "Use codepage from 'codepage' (see DEL tab), default: NOT SET (UNICODE)");
        createRow( TYP_LOB, "xmlgraphic", ROW_CHECKBOX, "", "Encode XML in graphic codepage, default: NOT SET");
        createRow( TYP_LOB, "");
        createRow( TYP_LOB, "Some options, namely IDENTITYIGNORE and LOBSINFILE will be set by PMF automatically");
        createRow( TYP_LOB, "To import LOBs and XMLs, put them in the same directory as the data file.");

        createRow( TYP_IXF, "indexschema=", ROW_LINEEDIT, "", "Index name during creation. Set blank space to use connection ID, default: NOT SET", 0);
        createRow( TYP_IXF, "indexschema", ROW_CHECKBOX, "", "Use the users connection ID during index creation, default: NOT SET");
        createRow( TYP_IXF, "forcein", ROW_CHECKBOX, "", "Accept data despite code page mismatches, suppress translation between code pages, default: NOT SET");
        createRow( TYP_IXF, "nochecklengths", ROW_CHECKBOX, "", "Attempt import without length check, default: NOT SET");
        createRow( TYP_IXF, "indexixf", ROW_CHECKBOX, "", "Drop all indexes and recreate from IXF, default: NOT SET");
    }
    else if( mode == MODE_LOAD || mode == MODE_LOAD_HADR )
    {
        createRow( TYP_LOAD, "generatedoverride", ROW_CHECKBOX, "", "Override/Overwrite Generated Always columns, default: NOT SET");
        createRow( TYP_LOAD, "--- The following are mutually exclusive");
        createRow( TYP_LOAD, "identityoverride", ROW_CHECKBOX, "", "Override/Overwrite Identity columns, default: NOT SET");
        createRow( TYP_LOAD, "identitymissing", ROW_CHECKBOX, "", "Create missing values for Identity columns, default: NOT SET");
        createRow( TYP_LOAD, "identityignore", ROW_CHECKBOX, "", "Ignore values for Identity columns, default: NOT SET");
        createRow( TYP_LOAD, "--- The following are mutually exclusive");
        createRow( TYP_LOAD, "sortkeys", ROW_LINEEDIT, "", "Number between 0 and 562 949 953 421 311, default: NOT SET", 0);
        createRow( TYP_LOAD, "sortkeys no", ROW_CHECKBOX, "", "SORTKEYS default is to be turned off, default: NOT SET");
        createRow( TYP_LOAD, "--- The following are mutually exclusive");
        createRow( TYP_LOAD, "presorted yes", ROW_CHECKBOX, "", "Specifies that the input data set has already been sorted, default: NOT SET");
        createRow( TYP_LOAD, "presorted no", ROW_CHECKBOX, "", "Specifies that the input data set has not already been sorted, default: NOT SET");
        createRow( TYP_LOAD, "--- The following are mutually exclusive");
        createRow( TYP_LOAD, "rowformat brf", ROW_CHECKBOX, "", "Table will be converted to basic row format, default: NOT SET");
        createRow( TYP_LOAD, "rowformat rrf", ROW_CHECKBOX, "", "Table will be converted to basic reorder  format, default: NOT SET");
        createRow( TYP_LOAD, "--- The following are mutually exclusive");
        createRow( TYP_LOAD, "log yes", ROW_CHECKBOX, "", "Specifies normal logging during the load process, default: NOT SET");
        createRow( TYP_LOAD, "log no", ROW_CHECKBOX, "", "Loggin will be (mostly) turned off, default: NOT SET");
        createRow( TYP_LOAD, "--- On HADR and LOGRETAINed systems, select COPY YES and specify a path");

        createRow( TYP_LOAD, "copy no", ROW_CHECKBOX, "", "The table space will be placed in backup pending state if forward recovery is enabled, default: NOT SET");
        createRow( TYP_LOAD, "copy yes", ROW_CHECKBOX, "", "A copy of the loaded data will be saved. Set path below. Default: NOT SET");        
        createRow( TYP_LOAD, "to ", ROW_LINEEDIT, "", "Specifies the device or directory on which the copy image will be created, default: NOT SET", 0);
        createRow( TYP_LOAD, "--- HINT: You can create the environment variable 'global_db2_hadr_path' to specify a default path (click 'Restore default' for default path)");
    }
    else close();

}


GString ExpImpOptions::createModifiedString(FileType type)
{
    GString out;
    OPTIONSROW *pRow;
    for(int i = 1; i <= (int)m_rowsSeq.numberOfElements(); ++i)
    {
        pRow = m_rowsSeq.elementAtPosition(i);

        if( pRow->PmfInternal ) continue;
        if( pRow->FileType != type ) continue;
        if( pRow->Title == "copy yes") continue;
        if( pRow->Title == "to ") continue;

        if(pRow->WdgType == ROW_CHECKBOX )
        {
            QCheckBox *pCB = (QCheckBox*) pRow->TheWdgt;
            if( !pCB->isChecked()) continue;
            out += pRow->Title+" ";
        }
        else if (pRow->WdgType == ROW_LINEEDIT )
        {
            QLineEdit *pLE = (QLineEdit*) pRow->TheWdgt;
            if( GString(pLE->text()) == pRow->DefaultValue ) continue;
            out += pRow->Title+GString(pLE->text())+" ";
        }
    }
    m_gstrModified = out;
    return out;
}

GString ExpImpOptions::modifiedString()
{
    return m_gstrModified;
}

void ExpImpOptions::createRow(int mode, GString text)
{
    OPTIONSROW * pRow = new OPTIONSROW;
    pRow->WdgType = ROW_COMMENT;
    pRow->Title = text;
    pRow->FileType = mode;
    pRow->PmfInternal = 0;
    m_rowsSeq.add(pRow);
}

void ExpImpOptions::createRow(int mode, GString title, int type, GString defaultValue, GString defaultText, int maxLen, int pmfInt)
{
    OPTIONSROW * pRow = new OPTIONSROW;
    pRow->WdgType = type;
    pRow->FileType = mode;
    pRow->DefaultValue = defaultValue;
    pRow->DefaultText = defaultText;
    pRow->Title = title;
    pRow->PmfInternal = pmfInt;
    if( type == ROW_CHECKBOX )
    {
        QCheckBox * qCB = new QCheckBox();
        if( defaultValue.length() == 0 ) qCB->setChecked(false);
        else qCB->setChecked(true);
        pRow->OldValue = qCB->isChecked() ? "1" : "0";
        pRow->TheWdgt = qCB;
    }
    else if(type == ROW_LINEEDIT )
    {
        QLineEdit *pLE = new QLineEdit();

        if( maxLen > 0 )
        {
            pLE->setMaxLength(maxLen); //Usually single char
            pLE->setMaximumWidth(30);
        }
        else pLE->setMinimumWidth(200);
        pLE->setText(defaultValue);
        pRow->OldValue = defaultValue;
        pRow->TheWdgt = pLE;
    }
    m_rowsSeq.add(pRow);
}

void ExpImpOptions::displayAllRows()
{
    for(int i = 1; i <= (int)m_rowsSeq.numberOfElements(); ++i )
    {
        displayRow(m_rowsSeq.elementAtPosition(i), m_pMainGrid, i-1);
    }
}

void ExpImpOptions::displayRow(OPTIONSROW * pRow, QGridLayout *pGrid, int row )
{
    if( pRow->WdgType == ROW_COMMENT )
    {
        pGrid->addWidget(new QLabel(pRow->Title), row, 0, 1, 4);
        return;
    }

    pGrid->addWidget(new QLabel(pRow->Title), row, 0);
    pGrid->addWidget(pRow->TheWdgt, row, 1);
    if( pRow->MaxLen == 0 ) pGrid->addWidget(new QLabel(pRow->DefaultText), row, 2, 1, 4);
    else pGrid->addWidget(new QLabel(pRow->DefaultText), row, 2);
}

void ExpImpOptions::saveSettingsForAllRows()
{
    GSeq <GString> seqAll;
    OPTIONSROW * pRow;

    for(int i = 1; i <= (int)m_rowsSeq.numberOfElements(); ++i )
    {
        pRow = m_rowsSeq.elementAtPosition(i);
        if( pRow->WdgType == ROW_COMMENT ) continue;

//        if( pRow->WdgType == ROW_LINEEDIT ) data = getFieldValue((FileType) pRow->FileType, pRow->Title);
//        else if( pRow->WdgType == ROW_CHECKBOX ) checked = getCheckBoxValue((FileType) pRow->FileType, pRow->Title);
        seqAll.add(optionsrowToString(pRow));
    }
    GFile settingsFile( expImpSettingsDir()+m_strSettingsFileName, GF_OVERWRITE );

    settingsFile.overwrite(&seqAll);
}

int ExpImpOptions::settingsChanged()
{
    int checked;
    OPTIONSROW * pRow;
    for(int i = 1; i <= (int)m_rowsSeq.numberOfElements(); ++i )
    {
        pRow = m_rowsSeq.elementAtPosition(i);
        if( pRow->WdgType == ROW_COMMENT ) continue;

        if( pRow->WdgType == ROW_LINEEDIT )
        {
            if( pRow->OldValue != GString(((QLineEdit*) pRow->TheWdgt)->text()) )
            {
                printf("%s: Old: %s, New: %s\n", (char*) pRow->Title, (char*) pRow->OldValue, (char*) GString(((QLineEdit*) pRow->TheWdgt)->text()));
                return 1;
            }
        }
        else if( pRow->WdgType == ROW_CHECKBOX )
        {
            checked =((QCheckBox*) pRow->TheWdgt)->isChecked() ? 1 : 0;
            if( GString(checked) != pRow->OldValue )
            {
                printf("%s: Old: %i, New: %i\n", (char*) pRow->Title,  checked, (char*) pRow->OldValue);
                return 1;
            }
        }
    }
    return 0;
}


GString ExpImpOptions::optionsrowToString(OPTIONSROW * pRow)
{
    GString data;
    if( pRow->WdgType == ROW_LINEEDIT ) data = getFieldValue((FileType) pRow->FileType, pRow->Title);
    else if( pRow->WdgType == ROW_CHECKBOX ) data = getCheckBoxValue((FileType) pRow->FileType, pRow->Title) ? "1" : "0";
    return GString(pRow->FileType)+"@"+pRow->Title+"@"+GString(pRow->PmfInternal)+"@"+GString(pRow->WdgType)+"@"+data;
}

void ExpImpOptions::loadPreviousSettings()
{
    GFile settingsFile( expImpSettingsDir()+m_strSettingsFileName);
    GString input;
    OPTIONSROW * pRow;
    for(int i = 1; i <= settingsFile.lines(); ++i )
    {
        for(int j = 1; j <= (int)m_rowsSeq.numberOfElements(); ++j)
        {            
            input = settingsFile.getLine(i);
            if( input.occurrencesOf("@") < 4 ) continue;
            pRow = m_rowsSeq.elementAtPosition(j);
            int fileType = input.subString(1, input.indexOf("@")-1).asInt();
            input = input.remove(1, input.indexOf("@"));
            GString title = input.subString(1, input.indexOf("@")-1);
            input = input.remove(1, input.indexOf("@"));
            int pmfInternal = input.subString(1, input.indexOf("@")-1).asInt();
            input = input.remove(1, input.indexOf("@"));
            int wdgtType = input.subString(1, input.indexOf("@")-1).asInt();
            input = input.remove(1, input.indexOf("@"));
            GString data = input;
            if( pRow->Title != title ) continue;
            pRow->FileType = fileType;
            pRow->PmfInternal = pmfInternal;
            pRow->WdgType = wdgtType;
            if( wdgtType == ROW_LINEEDIT )
            {
                ((QLineEdit*) pRow->TheWdgt)->setText(data);
                pRow->OldValue = data;
            }
            else if( wdgtType == ROW_CHECKBOX )
            {
                if( data == "1" ) ((QCheckBox*) pRow->TheWdgt)->setChecked(true);
                else ((QCheckBox*) pRow->TheWdgt)->setChecked(false);
                pRow->OldValue = data;
            }
        }
    }
}

void ExpImpOptions::setHadrPath(int overwrite)
{
    OPTIONSROW* pRow;
    pRow = getOptionsRow(TYP_LOAD, "to ");
    if( !overwrite && ((QLineEdit*) pRow->TheWdgt)->text().length() > 0 ) return;
    if( GString(getenv("global_db2_hadr_path")).strip().length() > 0 )
    {
        ((QLineEdit*) pRow->TheWdgt)->setText(getenv("global_db2_hadr_path"));
        //pRow->OldValue = getenv("global_db2_hadr_path");
        pRow = getOptionsRow(TYP_LOAD, "copy yes");
        //pRow->OldValue = "1";
        ((QCheckBox*) pRow->TheWdgt)->setChecked( true );
    }
}


void ExpImpOptions::setRowsToDefault()
{
    OPTIONSROW * pRow;
    for(int j = 1; j <= (int)m_rowsSeq.numberOfElements(); ++j)
    {
        pRow = m_rowsSeq.elementAtPosition(j);
        if( pRow->WdgType == ROW_LINEEDIT )
        {
            ((QLineEdit*) pRow->TheWdgt)->setText(pRow->DefaultValue);
        }
        else if( pRow->WdgType == ROW_CHECKBOX )
        {
            ((QCheckBox*) pRow->TheWdgt)->setChecked(pRow->DefaultValue.length() > 0);
        }
    }
}

void ExpImpOptions::OKClicked()
{
    if(m_Mode == MODE_LOAD_HADR)
    {
        if( getFieldValue(TYP_LOAD, "to ").length() == 0 || getCheckBoxValue(TYP_LOAD, "copy yes") == 0 )
        {            
            mb("This appears to be a HADR system or LOGARCHMETH1 is enabled. You need to check 'COPY YES' and set a path.");
            return;
        }
    }    
//    if( settingsChanged() )
//    {
//        if( QMessageBox::question(this, "PMF", "Save changes?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes )
//        {
//            saveSettingsForAllRows();
//        }
//    }
    close();
}

void ExpImpOptions::CancelClicked()
{
	close();
}

void ExpImpOptions::mb(GString msg)
{
    Helper::msgBox(this, "pmf", msg);
}

OPTIONSROW* ExpImpOptions::getOptionsRow(FileType type, GString fieldName)
{
    OPTIONSROW * pRow;
    for( int i = 1; i <= m_rowsSeq.numberOfElements(); ++i )
    {
        pRow = m_rowsSeq.elementAtPosition(i);
        if( pRow->FileType != type ) continue;
        if( pRow->Title == fieldName )
        {
            return pRow;
        }
    }
    return NULL;
}

int ExpImpOptions::setFieldValue(FileType type, GString fieldName, GString data)
{
    OPTIONSROW * pRow = getOptionsRow(type, fieldName);
    if( pRow != NULL )
    {
        ((QLineEdit*) pRow->TheWdgt)->setText(data);
        return 0;
    }
    return 1;
}

GString ExpImpOptions::getFieldValue(FileType type, GString fieldName)
{
    OPTIONSROW * pRow = getOptionsRow(type, fieldName);
    if( pRow != NULL )
    {
        return ((QLineEdit*) pRow->TheWdgt)->text();
    }
    return "";

}

int ExpImpOptions::getCheckBoxValue(FileType type, GString fieldName)
{
    OPTIONSROW * pRow = getOptionsRow(type, fieldName);
    if( pRow != NULL )
    {
        return ((QCheckBox*) pRow->TheWdgt)->isChecked() ? 1 : 0;
    }
    return 0;
}

int ExpImpOptions::setCheckBoxValue(FileType type, GString fieldName, int checked)
{
    OPTIONSROW * pRow = getOptionsRow(type, fieldName);
    if( pRow != NULL )
    {
        ((QCheckBox*) pRow->TheWdgt)->setChecked( checked > 0 ? true : false );
    }
    return 0;
}
