//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)
//

#include "importBox.h"
#include "getCodepage.h"
#include "helper.h"
#include "pmfdefines.h"
#include <qlayout.h>
#include <QGridLayout>
#include <QGroupBox>
#include <gfile.hpp>
#include <gstuff.hpp>
#include <dbapiplugin.hpp>
#include <QMimeData>
#include <QSettings>
#include <QList>
#include <QScrollBar>

#include "multiImport.h"
#include "pmfTable.h"

#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
//#   include <QUrlQuery>
#include <QUrl>
#endif


ImportBox::ImportBox(DSQLPlugin* pDSQL, QWidget *parent, GString currentSchema, GString currentTable, GString *prevDir, int hideSysTabs)
  :QDialog(parent) //, f("Charter", 48, QFont::Bold)
{

    if( !pDSQL ) return;
    m_pGDeb = NULL;

    m_iHideSysTables = hideSysTabs;
    m_sqlErc = 0;
    m_sqlErrTxt = "";

    m_pDSQL = new DSQLPlugin(*pDSQL);
    m_gstrPrevDir = prevDir;
    QGridLayout * grid = new QGridLayout( this );

    QDir qd;
    QString aPath = qd.homePath();

    #ifdef MAKE_VC
    m_strLogFile = GString(aPath)+"\\PMF_IMP.LOG";
    m_strLogFile = m_strLogFile.change("/", "\\");
    #else
    m_strLogFile = GString(aPath)+"/.pmf5/PMF_IMP.LOG";
    #endif


    ok = new QPushButton(this);
    ok->setDefault(true);
    cancel = new QPushButton(this);
    ok ->setText("Go!");
    cancel->setText("Exit");
    delLog = new QPushButton(this);
    delLog->setText("Remove ErrorLog");
    connect(ok, SIGNAL(clicked()), SLOT(OKClicked()));
    connect(cancel, SIGNAL(clicked()), SLOT(CancelClicked()));
    connect(delLog, SIGNAL(clicked()), SLOT(deleteErrLog()));

    //RadioButtons:
    typeGroup   = new QButtonGroup(this);
    importRB = new QRadioButton("Import", this);
    loadRB = new QRadioButton("Load", this);
    connect(importRB, SIGNAL(clicked()), this, SLOT(typeSelected()));
    connect(loadRB, SIGNAL(clicked()), this, SLOT(typeSelected()));
    importRB->setChecked(true);
    typeGroup->addButton(importRB);
    typeGroup->addButton(loadRB);
    optionsB = new QPushButton("Options");
    connect(optionsB, SIGNAL(clicked()), SLOT(optionsClicked()));
    if( pDSQL->getDBType() == DB2 )  typeGroup->addButton(optionsB);
    else optionsB->hide();


    //RadioButtons:
    formGroup   = new QButtonGroup(this);
    ixfRB = new QRadioButton(this);
    delRB = new QRadioButton(this);
    wsfRB = new QRadioButton(this);
    formGroup->addButton(ixfRB);
    formGroup->addButton(delRB);
    formGroup->addButton(wsfRB);




    actionGroup = new QButtonGroup(this);
    insRB = new QRadioButton(this);
    crtRB = new QRadioButton(this);
    updRB = new QRadioButton(this);
    replRB = new QRadioButton(this);
    repCrRB = new QRadioButton(this);
    connect(insRB, SIGNAL(clicked()), this, SLOT(optionSelected()));
    connect(crtRB, SIGNAL(clicked()), this, SLOT(optionSelected()));
    connect(updRB, SIGNAL(clicked()), this, SLOT(optionSelected()));
    connect(replRB, SIGNAL(clicked()), this, SLOT(optionSelected()));
    connect(repCrRB, SIGNAL(clicked()), this, SLOT(optionSelected()));
    actionGroup->addButton(insRB);
    actionGroup->addButton(crtRB);
    actionGroup->addButton(updRB);
    actionGroup->addButton(replRB);
    actionGroup->addButton(repCrRB);



    tableNameLE = new QLineEdit(this);
    fileNameDropZone =  new PmfDropZone(this);
    tableLB = new QListWidget(this);
    errorLB = new QTextBrowser(this);
    getFileB = new QPushButton(this);
    connect(getFileB, SIGNAL(clicked()), SLOT(getFileClicked()));
    connect(fileNameDropZone, SIGNAL(fileWasDropped()), SLOT(filesDropped()));

    schemaCB = new PmfSchemaCB(this);
    //Things in FormatGroup
    ixfRB->setText("IXF Format");
    delRB->setText("DEL Format");
    wsfRB->setText("WSF Format");
    getFileB->setText("Select File(s)");

    //Things in actionGroup
    insRB->setText("Insert into table");
    updRB->setText("Insert-Update table");
    replRB->setText("Replace data");
    repCrRB->setText("Replace-Create (IXF only)");
    crtRB->setText("Create table (IXF only)");

    //defaults
    insRB->setChecked(true);
    ixfRB->setChecked(true);
    //View ErrorLog

    QLabel * info = new QLabel(this);
    QFont fontBold  = info->font();
    fontBold.setBold(true);
    info->setText("Drag&drop file(s) into field below or click 'Select File'");
    grid->addWidget(info, 0, 0, 1, 4);
    fileNameDropZone->setPlaceholderText("Drag&drop file(s) here");

    QLabel * hint = new QLabel("Hint: Importing/Loading from LOCAL drives works best.");
    hint->setStyleSheet("font-weight: bold;");
    grid->addWidget(hint, 1, 0, 1, 4);

    //Row 1
    grid->addWidget(fileNameDropZone, 2, 0, 1, 2);
    grid->addWidget(getFileB, 2, 2);

    //Row 2
    QGroupBox * typeGroupBox = new QGroupBox();
    QHBoxLayout *typeLayout = new QHBoxLayout;
    typeGroupBox->setTitle("Import or Load");
    typeLayout ->addWidget(importRB);
    typeLayout ->addWidget(loadRB);
    typeLayout ->addWidget(optionsB);
    typeGroupBox->setLayout(typeLayout);
    grid->addWidget(typeGroupBox, 3, 0, 1, 4);

    //Row 3
    QGroupBox * formatGroupBox = new QGroupBox();
    QHBoxLayout *frmLayout = new QHBoxLayout;
    formatGroupBox->setTitle("File format");
    frmLayout->addWidget(ixfRB);
    frmLayout->addWidget(delRB);
    frmLayout->addWidget(wsfRB);
    formatGroupBox->setLayout(frmLayout);
    grid->addWidget(formatGroupBox, 4, 0, 1, 4);

    //left
    grid->addWidget(new QLabel("(Right-click buttons for help)"), 5, 0);
    grid->addWidget(insRB, 6, 0);
    grid->addWidget(replRB, 7, 0);
    grid->addWidget(updRB, 8, 0);
    grid->addWidget(crtRB, 9, 0);
    grid->addWidget(repCrRB, 10, 0);
    grid->addWidget(tableNameLE, 11, 0);
    QLabel *tbInfo = new QLabel("<-Tablename is case-sensitive!");
    grid->addWidget(tbInfo, 11, 1);


    //right
    grid->addWidget(schemaCB, 5, 1, 1, 2);
    //grid->addWidget()
    grid->addWidget(tableLB, 6, 1, 5, 2);

    //bottom
    grid->addWidget(errorLB, 13, 0, 1, 4);

    QGroupBox * btGroupBox = new QGroupBox();
    QHBoxLayout *btLayout = new QHBoxLayout;
    btLayout->addWidget(ok);
    btLayout->addWidget(cancel);
    btLayout->addWidget(delLog);
    btGroupBox->setLayout(btLayout);
    grid->addWidget(btGroupBox, 14, 0, 1, 4);

    insRB->setChecked(true);
    tableNameLE->setEnabled(false);
    tableNameLE->setPlaceholderText("table to create (or select from list)");
    setWhatsThisText();

    grid->setRowStretch(7, 90);
    connect(schemaCB, SIGNAL(activated(int)), SLOT(schemaSelected(int)));
    this->currentSchema = currentSchema;
    this->currentTable = currentTable;

    if( m_pDSQL->getDBType() == MARIADB )
    {
        ixfRB->setText("Standard");
        importRB->setDisabled(true);
        loadRB->setDisabled(true);
        ixfRB->setDisabled(true);
        wsfRB->setDisabled(true);
        delRB->setDisabled(true);
        insRB->setDisabled(true);
        updRB->setDisabled(true);
        replRB->setDisabled(true);
        repCrRB->setDisabled(true);
        crtRB->setDisabled(true);
    }


    schemaCB->fill(pDSQL, currentSchema, hideSysTabs);
    aThread = NULL;
    tb = NULL;
    timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), this, SLOT(timerEvent()) );


    if( parent->height() > 780 ) this->resize(610, 780);
    else this->resize(610, 580);
    this->move(parent->x() + (parent->width() - width()) / 2,  parent->y() + (parent->height() - height()) / 2);

    fillLB(currentSchema, currentTable);

    m_pExpImpOptions = NULL;
//  fillLB(tableNameSeq);
}
ImportBox::~ImportBox()
{
    //delete tb;
    delete ok;
    delete cancel;
    delete ixfRB;
    delete delRB;
    delete wsfRB;
    delete insRB;
    delete updRB;
    delete repCrRB;
    delete replRB;
    delete crtRB;
    delete tableLB;
    delete fileNameDropZone;
    delete tableNameLE;
    delete getFileB;
    delete formGroup;
    delete actionGroup;
    delete errorLB;
    delete delLog;
    delete schemaCB;
    delete timer;
    delete m_pDSQL;
    if( m_pExpImpOptions != NULL)delete m_pExpImpOptions;    
}

void ImportBox::setWhatsThisText()
{
    insRB->setWhatsThis("Insert data into existing table");
    crtRB->setWhatsThis("Create new table from IXF file. Enter table name below.");
    updRB->setWhatsThis("Existing rows will be updated, new rows inserted. A primary key is mandatory.");
    replRB->setWhatsThis("Replaces ALL data, EXISTING DATA WILL BE LOST.");
    repCrRB->setWhatsThis("(Re-)creates the table, EXISTING DATA WILL BE LOST. Enter table name below or select table from the right.");
}

void ImportBox::optionSelected()
{
    if( crtRB->isChecked() || repCrRB->isChecked() )
    {
        tableNameLE->setEnabled(true);
        tableLB->setEnabled(false);
    }
    else
    {
        tableNameLE->setEnabled(false);
        tableLB->setEnabled(true);
    }
}

void ImportBox::typeSelected()
{
    if( loadRB->isChecked())        
    {
        GString txt = "CAUTION!\n\nUsing LOAD can seriously damage the target table (and all its dependent tables).";
        txt += "\nUse this only if you really, really know what you are doing. ";
        txt += "\n\nAlso, when using LOAD, LOBs and XML data can only be read from the server side (!)";
        msg(txt);
        insRB->setEnabled(true);
        replRB->setEnabled(true);
        updRB->setEnabled(false);
        repCrRB->setEnabled(false);
        crtRB->setEnabled(false);
        if( updRB->isChecked() || repCrRB->isChecked() || crtRB->isChecked() )
        {
            msg("'Load' does not support UPDATE/CREATE/REPL.CREATE ");
            insRB->setChecked(true);
        }
    }
    else
    {
        insRB->setEnabled(true);
        replRB->setEnabled(true);
        updRB->setEnabled(true);
        repCrRB->setEnabled(true);
        crtRB->setEnabled(true);
    }
}

bool ImportBox::isHADRorLogArchMeth()
{
    m_pDSQL->initAll("select * from SYSIBMADM.SNAPHADR");
    if( m_pDSQL->numberOfRows() ) return 1;
    GString res;
    DBAPIPlugin * pAPI = new DBAPIPlugin(m_pDSQL->getDBTypeName());
    pAPI->setGDebug(m_pGDeb);
    CON_SET conSet;
    m_pDSQL->currentConnectionValues(&conSet);

    //822 is SQLF_DBTN_LOGARCHMETH1 in sqlutil.h
    res = pAPI->getDbCfgForToken(conSet.Host, conSet.UID, conSet.PWD, conSet.DB, 822, 0);
    delete pAPI;
    if( res.upperCase() != "OFF" && res.length() ) return 1;
    return 0;
}

void ImportBox::optionsClicked()
{
    createExpImpOptions();
    m_pExpImpOptions->exec();
}

GString ImportBox::newCodePage(GString path)
{
#ifdef MAKE_VC
        if( Helper::fileIsUtf8(path) ) return "1208";
#else
        if( !Helper::fileIsUtf8(path) )return "1252";
#endif
    return "";
}

void ImportBox::callImport(GString fullPath)
{
    GString file;
    /*
    GString codePage = newCodePage(path);
    if( delRB->isChecked() && codePage.length())
    {
        getCodepage * foo = new getCodepage(this);
        foo->setValue(codePage);
        foo->exec();
        m_pExpImpOptions->setFieldValue(expImpOptions::TYP_DEL, "codepage=", foo->getValue());
    }
    */
    if( repCrRB->isChecked() )
    {
        if( QMessageBox::question(this, "PMF Import", "WARNING:\nThis will (Re-)create the table, existing data will be lost. Continue?", QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes ) return;
    }
    if( replRB->isChecked() )
    {
        if( QMessageBox::question(this, "PMF Import", "WARNING:\nExisting data will be lost. Continue?", QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes ) return;
    }
    GFile aFile(fullPath, GF_READONLY);
    if( !aFile.initOK() )
    {
        msg("Invalid path / filename.");
        return;
    }
    fullPath = fullPath.translate('\\', '/');
    if(  !fullPath.length() || fullPath.occurrencesOf("/") == 0 )
    {
        msg("Specify path and filename.");
        return;
    }

    file = fullPath.subString(fullPath.lastIndexOf('/')+1, fullPath.length()).strip();
    if( !file.strip().length() )
    {
        msg("Filename missing or invalid. ");
        return;
    }
    int i, found = -1;
    if( !crtRB->isChecked() )
    {
        for( i=0; i<tableLB->count(); ++i)
        {
            if( tableLB->item(i)->isSelected() )
            {
                found = i;
                break;
            }
        }
    }
    if( crtRB->isChecked() && GString(tableNameLE->text()).length() == 0 )
    {
        msg("Specify table to create or select table");
        tableNameLE->setFocus();
        return;
    }

    GString tableName;
    if( found >= 0 ) tableName = currentSchema+"."+GString(tableLB->item(found)->text());        

    //Create table: tableName from LineEdit
    if( crtRB->isChecked() ) tableName = GString(tableNameLE->text());
    else if ( repCrRB->isChecked() )
    {
        if( tableNameLE->text().length() ) tableName = GString(tableNameLE->text());
    }

    if( !tableName.length() )
    {
        msg("No table name selected or specified.\nTo import, select existing table from the right; to create, enter table name below.");
        return;
    }

    if( QMessageBox::question(this, "PMF", "Importing into\n\n"+tableName+"\n\nContinue?", QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes ) return;

    timer->start( 200 );
    aThread = new ImportThread;
    tb = new ThreadBox( this, "Please wait...", "Importing", m_pDSQL->getDBTypeName() );
    tb->setThread(aThread);
    aThread->setOwner( this );
    aThread->setBox(tb);
    aThread->start();
    GStuff::dormez(300);

    tb->exec();

    /******
while( aThread->isAlive() )
{
GStuff::dormez(100);
printf("alive\n");
}
   //tb = NULL;
   //aThread->setDone();
*****/
    //tm("Done.");

}
void ImportBox::displayLog()
{

    if( loadRB->isChecked() && !m_sqlErc )
    {
        runLoadChecks();
    }
    errorLB->update();

    GString errTxt;
    //if( m_sqlErc )

    QFile f(m_strLogFile);
    if( f.size() > 1000000)
    {
        GString msg;
        if( m_sqlErc ) msg = "There were errors.\n";
        else msg = "Import was successful.\n";
        msg += "Logfile is in "+m_strLogFile+" and it is too big to show ("+GString((unsigned long)f.size()/1000000)+"MB), please check it manually.";
        return;
    }
    errTxt += "Please wait, reading LogFile...\n";
    errTxt += "   ****  LogFile: "+m_strLogFile+"  ****\n";
    errTxt += "ErrorCode: "+GString(m_sqlErc)+"\n";
    errTxt += m_sqlErrTxt+"\n";

    errorLB->blockSignals( true );
    if( f.open(QIODevice::ReadOnly) )
    {
        QTextStream t( &f );
        GString s;
        while( !t.atEnd() )
        {
            s = t.readLine();
            errTxt += s+"\n";
        }
        f.close();
    }
    else errTxt += "   No LogFile was created.\n";
    errorLB->blockSignals( false );
    errTxt += "\n####### Hint:\n";
    errTxt += "If Import/Load failed, retry from a LOCAL drive.\n";
    errTxt += "#######\n";
    errorLB->setText(m_importMsg+"\n"+ errTxt);
    errorLB->verticalScrollBar()->setValue(errorLB->verticalScrollBar()->maximum());
    //else errorLB->addItem("   No error was returned.");
}
void ImportBox::timerEvent()
{
    if( !aThread ) return;
    if( !aThread->isAlive() )
    {
        if( tb )
        {
            tb->close();
        }
        displayLog();
        timer->stop();
    }
}

void ImportBox::ImportThread::run()
{
   myImport->startImport();
}

void ImportBox::OKClicked()
{
    if( m_impFileSeq.numberOfElements() == 0  )
    {
        msg("Select file(s) to import");
        return;
    }
    if( m_pExpImpOptions == NULL)
    {
        createExpImpOptions();
    }

    if( isHADRorLogArchMeth() && loadRB->isChecked() )
    {
        int err = 0;
        if( m_pExpImpOptions != NULL )
        {
            if( m_pExpImpOptions->getFieldValue(ExpImpOptions::TYP_LOAD, "to ").length() == 0 || m_pExpImpOptions->getCheckBoxValue(ExpImpOptions::TYP_LOAD, "copy yes") == 0 )
            {
                err = 1;
            }
        }
        else err = 1;
        if( err )
        {
            Helper::msgBox(this, "pmf", "This appears to be a HADR system or LOGARCHMETH1 is enabled.\nOpen 'Options', check 'COPY YES' and set a path.");
            return;
        }
    }
    remove(m_strLogFile);
    //Do not call setComboBoxesFromFile(), this would override user's choice of target table
    if( m_impFileSeq.numberOfElements() == 1 )
    {
        callImport(m_impFileSeq.elementAtPosition(1));
    }
    else
    {
        for(int i = 1; i <= m_impFileSeq.numberOfElements(); ++i)
        {
            GString file = m_impFileSeq.elementAtPosition(i);
            setComboBoxesFromFile(file);
            fileNameDropZone->setText(file);
            callImport(file);
        }
        fileNameDropZone->clearFileList();
        m_impFileSeq.removeAll();
    }
    QSettings settings(_CFG_DIR, "pmf5");
    QString path = GStuff::pathFromFullPath(fileNameDropZone->text());
    settings.setValue("importPath", path);
    setImportOptionsEnabled(true);
}

void ImportBox::createExpImpOptions()
{    
    CON_SET conSet;
    m_pDSQL->currentConnectionValues(&conSet);
    if( m_pExpImpOptions ) delete m_pExpImpOptions;
    if( importRB->isChecked() )
    {
        m_pExpImpOptions = new ExpImpOptions(this, ExpImpOptions::MODE_IMPORT, conSet.DB);
    }
    else if( loadRB->isChecked() )
    {
        if( isHADRorLogArchMeth() )m_pExpImpOptions = new ExpImpOptions(this, ExpImpOptions::MODE_LOAD_HADR, conSet.DB);
        else m_pExpImpOptions = new ExpImpOptions(this, ExpImpOptions::MODE_LOAD, conSet.DB);
    }
}

GString ImportBox::getTargetTable()
{
    signed int found = -1;
    if( !crtRB->isChecked() )
    {
        for( int i = 0; i < tableLB->count(); ++i )
        {
            if( tableLB->item(i)->isSelected() )
            {
                found = i;
                break;
            }
        }
    }
    //tableName from ListBox selection
    GString tableName;
    if( found >= 0 && m_pDSQL->getDBType() == MARIADB ) tableName = currentSchema+"."+GString(tableLB->item(found)->text());
    else if( found >= 0 ) tableName = "\""+currentSchema+"\".\""+GString(tableLB->item(found)->text())+"\"";
    return tableName;
}

void ImportBox::startImport()
{

    GString format;
    GString action, modifier, copyTarget;

    if( m_pExpImpOptions != NULL)
    {
        if( importRB->isChecked() )  modifier = m_pExpImpOptions->createModifiedString(ExpImpOptions::TYP_ALL);
        else if( loadRB->isChecked() )  modifier = m_pExpImpOptions->createModifiedString(ExpImpOptions::TYP_LOAD);
        if( m_pExpImpOptions->getCheckBoxValue(ExpImpOptions::TYP_LOAD, "copy yes") )
        {
            copyTarget = m_pExpImpOptions->getFieldValue(ExpImpOptions::TYP_LOAD, "to ");
        }
    }

    if( delRB->isChecked() )
    {
        if( m_pExpImpOptions != NULL) modifier += " "+ m_pExpImpOptions->createModifiedString(ExpImpOptions::TYP_DEL);
        format = "DEL" ;
    }
    else if( ixfRB->isChecked() )
    {
        if( m_pExpImpOptions != NULL) modifier += " "+ m_pExpImpOptions->createModifiedString(ExpImpOptions::TYP_IXF);
        format = "IXF";
    }
    if(wsfRB->isChecked() ) format = "WSF";

    if( insRB->isChecked() ) action = "INSERT INTO ";
    if( updRB->isChecked() ) action = "INSERT_UPDATE INTO ";
    if( crtRB->isChecked() ) action = "CREATE INTO ";
    if( replRB->isChecked() ) action = "REPLACE INTO ";
    if( repCrRB->isChecked() ) action = "REPLACE_CREATE INTO ";

    //Some of these checks were done above, but this runs in a thread.
    GString tableName = getTargetTable();

    //Create table: tableName from LineEdit
    if( crtRB->isChecked() || repCrRB->isChecked() )
    {
        tableName = GStuff::decorateTabName(tableNameLE->text());
    }

    if( !tableName.length() ) return;
    action += tableName;

    GString path, file, fullPath;
    fullPath = fileNameDropZone->text();
    file = fullPath;

    #ifdef MAKE_VC
    fullPath = fullPath.translate('/', '\\');
    path = path.translate('/', '\\');
    file = file.translate('/', '\\');
    if( fullPath.occurrencesOf('\\') > 0 ) path = fullPath.subString(1,fullPath.lastIndexOf('\\'));
    else path = "";
    #else
    if( fullPath.occurrencesOf('/') > 0 ) path = fullPath.subString(1,fullPath.lastIndexOf('/'));
    else path = "";
    #endif

    *m_gstrPrevDir = path;
    //On error: Try again w/out "lobsinfile"


    if( m_pDSQL->getDBType() == MARIADB )
    {
        GString cmd = "LOAD DATA INFILE '"+file+"' INTO TABLE "+tableName;
        m_sqlErrTxt = m_pDSQL->initAll(cmd);
        return;
    }



    DBAPIPlugin * pAPI = new DBAPIPlugin(m_pDSQL->getDBTypeName());
    m_importMsg = "";
    if( !pAPI )
    {
        m_importMsg += "Could not load DBAPiPlugin (should be in the 'plugin' subdirectory)\n";
        return;
    }
    pAPI->setGDebug(m_pGDeb);
    m_sqlErc = 0;
    m_sqlErrTxt = "";

    //Last param is modifier
    //0: None
    //1: MODIFIED BY LOBSINFILE
    //2: MODIFIED BY IDENTITYIGNORE
    //3: MODIFIED BY IDENTITYIGNORE AND LOBSINFILE
    //4: MODIFIED BY IDENTITYOVERRIDE
    GSeq<GString> pathSeq;

    pathSeq.add(path.strip());
    if( loadRB->isChecked() )
    {
        m_importMsg += "----------\n";
        m_importMsg += " Modifier: "+modifier+"\n";
        m_importMsg += " CopyTarget: "+copyTarget+"\n";
        m_importMsg += "----------\n";
        m_sqlErc = pAPI->loadFromFileNew(file, &pathSeq, format, action, m_strLogFile, modifier, copyTarget);
        if( m_sqlErc) m_sqlErrTxt = pAPI->SQLError();

        if( m_sqlErc && m_sqlErc != 3107) m_sqlErc = pAPI->loadFromFileNew(file, &pathSeq, format, action, m_strLogFile, "lobsinfile identityignore", copyTarget);
        if( m_sqlErc && m_sqlErc != 3107) m_sqlErc = pAPI->loadFromFileNew(file, &pathSeq, format, action, m_strLogFile, "identityignore", copyTarget);
        if( m_sqlErc && m_sqlErc != 3107) m_sqlErc = pAPI->loadFromFileNew(file, &pathSeq, format, action, m_strLogFile, "lobsinfile", copyTarget);
        if( m_sqlErc && m_sqlErc != 3107) m_sqlErc = pAPI->loadFromFileNew(file, &pathSeq, format, action, m_strLogFile, "FORCECREATE", copyTarget);
        if( !m_sqlErc )
        {
            m_importMsg += "--- LOAD RETURNED NO ERROR.\n";
        }

    }
    else
    {
//        m_sqlErc = pAPI->importTable(file, &pathSeq, format, action, m_strLogFile, modifier);
//        if( m_sqlErc && m_sqlErc != 3107 ) m_sqlErc = pAPI->importTable(file, &pathSeq, format, action, m_strLogFile, modifier);
//        if( m_sqlErc && m_sqlErc != 3107 ) m_sqlErc = pAPI->importTable(file, &pathSeq, format, action, m_strLogFile, modifier);
//        if( m_sqlErc && m_sqlErc != 3107 )
//        {
//            m_sqlErrTxt = pAPI->SQLError();
//            errorLB->addItem("Trying again without LOBs and FORCECREATE...");
//            m_sqlErc = pAPI->importTable(file, &pathSeq, format, action, m_strLogFile, modifier);
//        }

        m_sqlErc = pAPI->importTableNew(file, &pathSeq, format, action, m_strLogFile, "lobsinfile identityignore "+modifier);
        if( m_sqlErc && m_sqlErc != 3107 && m_sqlErc != -3005) m_sqlErc = pAPI->importTableNew(file, &pathSeq, format, action, m_strLogFile, "identityignore "+modifier);
        if( m_sqlErc && m_sqlErc != 3107 && m_sqlErc != -3005) m_sqlErc = pAPI->importTableNew(file, &pathSeq, format, action, m_strLogFile, "lobsinfile "+modifier);
        if( m_sqlErc && m_sqlErc != 3107 && m_sqlErc != -3005)
        {
            m_sqlErrTxt = pAPI->SQLError();
            m_importMsg += "Trying again without LOBs and FORCECREATE...\n";
            m_sqlErc = pAPI->importTableNew(file, &pathSeq, format, action, m_strLogFile, "FORCECREATE "+modifier);
        }
    }
    delete pAPI;

}

void ImportBox::runLoadChecks()
{
    PmfTable pmfTable(m_pDSQL, currentTable);
    GString property, outMsg, cmd;
    GString schema = pmfTable.tabSchema();
    GString table  = pmfTable.tabName();
    m_pDSQL->initAll("select STATUS, ACCESS_MODE, PROPERTY from SYSCAT.TABLES WHERE TabSchema='"+schema+"' and TabName='"+table+"'");
    if( m_pDSQL->numberOfRows() == 1 )
    {
        if( m_pDSQL->rowElement(1, "STATUS").strip().upperCase() != "'N'" ||  m_pDSQL->rowElement(1, "ACCESS_MODE").strip().upperCase() != "'F'")
        {
            outMsg = "Table is in integrity pending state.\n";
            outMsg += "Would you like to run\nSET INTEGRITY FOR <table> IMMEDIATE CHECKED?";
            cmd = "SET INTEGRITY FOR "+currentTable+" IMMEDIATE CHECKED";
            property = m_pDSQL->rowElement(1, "PROPERTY").strip().strip("'").upperCase();
            if(property.length() > 0)
            {
                if( property[1] == 'Y')
                {
                    outMsg = "This appears to be a user maintained materialized query table.\n";
                    outMsg += "Would you like to run\nSET INTEGRITY FOR <table> ALL IMMEDIATE UNCHECKED?";
                    cmd = "SET INTEGRITY FOR "+currentTable+" ALL IMMEDIATE UNCHECKED";
                }
            }
            if( QMessageBox::question(this, "PMF", outMsg, QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes ) return;
            GString err = m_pDSQL->initAll(cmd);
            if( err.length() )m_importMsg += "!!! INTEGRITY CHECK failed: "+err+"\n";
            else m_importMsg += "--- INTEGRITY CHECK returned no error.\n";
        }
    }
    for( int i = 1; i <= pmfTable.columnCount(); ++i )
    {
        int max = 0;
        if( GString(pmfTable.column(i)->misc()).occurrencesOf("GENERATED ALWAYS") )
        {
            cmd = "select max(" +pmfTable.column(i)->colName()+") from "+currentTable;
            GString err = m_pDSQL->initAll(cmd);
            if( !err.length() )
            {
                max = m_pDSQL->rowElement(1, 1).strip("'").asInt()+1;
                outMsg = "Column "+pmfTable.column(i)->colName()+" is GENERATED ALWAYS. Do you want to restart the counter with current MAX?";
                if( QMessageBox::question(this, "PMF", outMsg, QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes )
                {
                    cmd = "ALTER TABLE "+currentTable+" ALTER COLUMN " +pmfTable.column(i)->colName()+" RESTART WITH "+GString(max);
                    err = m_pDSQL->initAll(cmd);
                    if(err.length())msg(err);
                }
            }
        }
    }

    // ALTER TABLE "DATA"."ARCHIVDATEINAMEZUTASK" ALTER COLUMN ID RESTART WITH 10

}


void ImportBox::CancelClicked()
{
    fileNameDropZone->clearFileList();
    close();
}

void ImportBox::getFileClicked()
{
    QStringList filenames = QFileDialog::getOpenFileNames(this, "Select file to import", *m_gstrPrevDir);

    if( filenames.isEmpty() ) return;
    else if (filenames.count() == 1 ) fileNameDropZone->setText(filenames.at(0));
    else fileNameDropZone->setText("["+GString(filenames.count()+" files]"));
    m_impFileSeq.removeAll();
    for( int i = 0; i < filenames.count(); ++i )
    {
        m_impFileSeq.add(GString(filenames.at(i)));
    }
    GString ret = setFilesToImport(m_impFileSeq);
    if( ret != "OK" )
    {
        fileNameDropZone->clearFileList();
        if(ret.length()) msg(ret);
    }
}

short ImportBox::fillLB(GSeq <GString> * tableNameSeq)
{

    for( unsigned i=1; i<=tableNameSeq->numberOfElements(); ++i)
    {
        tableLB->addItem(tableNameSeq->elementAtPosition(i));
    }
    return 0;
}

void ImportBox::deleteErrLog()
{
    errorLB->clear();
    QFile f(m_strLogFile);

    ///if( !f.remove() ) errorLB->addItem("<Could Not Remove File "+m_strLogFile+". Sorry.>");

}
void ImportBox::setSchema(GString schema)
{
    iSchema = schema;
}
void ImportBox::fillLB(GString schema, GString table)
{
    GString name;
    m_pDSQL->getTables(schema);
    tableLB->clear();
    QListWidgetItem *pItem = NULL;
    int pos = -1;
    for( unsigned int i=1; i<=m_pDSQL->numberOfRows(); ++i)
    {
        name   = m_pDSQL->rowElement(i,1).strip().strip("'").strip();
        new QListWidgetItem(name, tableLB);
        if( "\""+schema+"\".\""+name+"\"" == table )
        {
            pos = i-1;
            pItem = tableLB->item(pos);
            pItem->setSelected(true);
            tableLB->setCurrentRow(pos);
        }
    }
}
void ImportBox::schemaSelected(int index)
{
    fillLB(schemaCB->itemText(index));
    currentSchema = schemaCB->itemText(index);
}

void ImportBox::setButtons()
{
    QFont font  = this->font();
    font.setBold(false);
    delRB->setFont(font);
    wsfRB->setFont(font);
    ixfRB->setFont(font);

    delRB->setText("DEL Format");
    wsfRB->setText("WSF Format");
    ixfRB->setText("IXF Format");
}


IMPORT_TYPE ImportBox::importTypeFromFile(GString file)
{
    setButtons();
    FILE *f;
    char *buffer;
    int n;
    unsigned long size;
    IMPORT_TYPE type = IMP_FILE_UNKNOWN;

    f = fopen(file, "rb");
    if( !f ) return type;

    //Get file size
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    //Read max 1000 bytes from file
    if( size > 1000 ) size = 1000;
    buffer=(char *)malloc(size+1);
    n = fread(buffer, size, sizeof(char), f);


    if( GString(buffer, 20).occurrencesOf("IXF")) type = IMP_FILE_IXF;
    else if( GString(buffer, 8, GString::HEX) == GString("0000020004040700") ) type = IMP_FILE_WSF;
    else if ( memchr(buffer, '\0', size) == NULL ) type = IMP_FILE_DEL;    
    fclose(f);
    free(buffer);
    return type;
}

void ImportBox::setImportTypeFromFileType(GString file)
{
    setButtons();
    QFont font = this->font();
    font.setBold(true);
    if( importTypeFromFile(file) == IMP_FILE_IXF )
    {
        ixfRB->setChecked(true);
        ixfRB->setText("IXF Format (guessed)");
        ixfRB->setFont(font);
    }
    else if( importTypeFromFile(file) == IMP_FILE_WSF )
    {
        wsfRB->setChecked(true);
        wsfRB->setText("WSF Format (guessed)");
        wsfRB->setFont(font);
    }
    else if ( importTypeFromFile(file) == IMP_FILE_DEL )
    {
        delRB->setChecked(true);
        delRB->setText("DEL Format (guessed)");
        delRB->setFont(font);
    }
}



int ImportBox::checkForSameImportType(GSeq<GString> *fileSeq)
{
    if( fileSeq->numberOfElements() > 1 )
    {
        IMPORT_TYPE type = importTypeFromFile(fileSeq->elementAtPosition(1));
        for( int i = 2; i <= fileSeq->numberOfElements(); ++i )
        {
            if( importTypeFromFile(fileSeq->elementAtPosition(1)) != type )
            {
                return 1;
            }
        }
    }
    return 0;
}

int ImportBox::showMultiImport(GSeq<GString> *fileSeq)
{
    if( fileSeq->numberOfElements() <= 1 ) return QDialog::Accepted;
    MultiImport * foo = new MultiImport(m_pGDeb, m_pDSQL, this);
    GString fileName;
    for( int i = 1; i <= fileSeq->numberOfElements(); ++i )
    {
        fileName = Helper::fileNameFromPath(fileSeq->elementAtPosition(i));
        foo->createRow(fileName, schemaFromFile(fileName), tableFromFile(fileName));
    }    
    return foo->exec();
}

GString ImportBox::setFilesToImport(GSeq<GString> fileSeq)
{
    m_impFileSeq.removeAll();
    if( fileSeq.numberOfElements() == 0 )
    {
        handleImportFiles();
        return "OK";
    }
    if( checkForSameImportType(&fileSeq) )
    {
        fileNameDropZone->clearFileList();
        return "Files are not of same type (IXF, DEL, WSF)";
    }
    for( int i = 2; i <= fileSeq.numberOfElements(); ++i )
    {
        if( !canMapToTable(fileSeq.elementAtPosition(i)) )
        {
            fileNameDropZone->clearFileList();
            GString m = "Not all files can be mapped to tables. The filenames must be in the form of\n\n";
            m += "[Schema].[Table] or\n[Schema].[Table].[Extension]\n\n";
            m += "PMF will then import the files into the corresponding tables.";
            m += "\n\nHint: When importing LOBs, do *not* select the files containing the actual LOBs.";
            return m;
        }
    }

    m_impFileSeq = fileSeq;
    handleImportFiles();
    int rc = showMultiImport(&fileSeq);
    if( rc != QDialog::Accepted )
    {
        m_impFileSeq.removeAll();
        setImportOptionsEnabled(true);
        return "";
    }
    return "OK";

    //Todo:
    //Check types
    //guessSchemaAndTableFromFile
    //Create struct
//    fileNameLE->setText(file);
//    setImportTypeFromFileType();
//    guessSchemaAndTableFromFile();
}

void ImportBox::setImportOptionsEnabled(bool enabled)
{
    crtRB->setEnabled(enabled);
    repCrRB->setEnabled(enabled);
    schemaCB->setEnabled(enabled);
    tableLB->setEnabled(enabled);
    if( !enabled ) insRB->setChecked(true);
}

void ImportBox::handleImportFiles()
{
    setImportOptionsEnabled(true);
    if( m_impFileSeq.numberOfElements() == 0 ) return;    
    else if (m_impFileSeq.numberOfElements() == 1 ) fileNameDropZone->setText(m_impFileSeq.elementAtPosition(1));
    else
    {
        fileNameDropZone->setText("["+GString(m_impFileSeq.numberOfElements())+" files to import]");
        setImportOptionsEnabled(false);
    }
    setImportTypeFromFileType(m_impFileSeq.elementAtPosition(1));
    setComboBoxesFromFile(m_impFileSeq.elementAtPosition(1));
}


int ImportBox::canMapToTable(GString input)
{
    GString schema = schemaFromFile(input);
    GString table = tableFromFile(input);
    GString cmd = "Select count(*) from syscat.tables where translate(tabschema)='"+schema.upperCase()+"' and translate(tabname)='"+table.upperCase()+"'";    
    m_pDSQL->initAll(cmd);

    if( m_pDSQL->rowElement(1,1).asInt() != 1 ) return 0;
    return 1;
}

GString ImportBox::tableFromFile(GString file)
{
    if( file.occurrencesOf(".") == 0 ) return  "";
    file = Helper::fileNameFromPath(file);
    GSeq<GString> seq = file.split('.');
    if( seq.numberOfElements() >= 2 ) return seq.elementAtPosition(2);
    return "";
}

GString ImportBox::schemaFromFile(GString file)
{
    if( file.occurrencesOf(".") == 0 ) return  "";
    file = Helper::fileNameFromPath(file);
    return file.subString(1, file.indexOf(".")-1);
}


void ImportBox::filesDropped()
{    
    GString ret = setFilesToImport(fileNameDropZone->fileList());
    if( ret != "OK" )
    {
        if( ret.length() ) msg(ret);
    }
}

void ImportBox::setComboBoxesFromFile(GString input)
{
    if( !input.length() ) return;
    GString schema = schemaFromFile(input);
    GString table = tableFromFile(input);
    GString cmd = "Select count(*) from syscat.tables where translate(tabschema)='"+schema.upperCase()+"' and translate(tabname)='"+table.upperCase()+"'";
    m_pDSQL->initAll(cmd);
    if( m_pDSQL->rowElement(1,1).asInt() != 1 ) return;

    //if( QMessageBox::question(this, "PMF Import", "Filename suggests target table "+schema+"."+table+"\nImport?", QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes ) return;


    int index = schemaCB->fill(m_pDSQL, schema, 0);
    if( index < 0 )
    {
        schemaCB->fill(m_pDSQL, currentSchema, m_iHideSysTables);
        index = schemaCB->fill(m_pDSQL, schema, 0);
    }
    if( index < 0 ) return;

    fillLB(schemaCB->itemText(index));
    currentSchema = schemaCB->itemText(index);
    QListWidgetItem *pItem;
    QList<QListWidgetItem*> items = tableLB->findItems(table, Qt::MatchCaseSensitive);
    if( items.count() ==  0 ) items = tableLB->findItems(table, Qt::MatchFixedString);
    if( items.count() == 1 )
    {
        pItem = items.at(0);
        tableLB->setCurrentItem(pItem);
        tableNameLE->setText(schema+"."+table);
    }
}
