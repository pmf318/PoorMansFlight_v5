//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include "exportBox.h"

#include "helper.h"
#include "pmfdefines.h"
#include <qlayout.h>
//Added by qt3to4:
#include <QGridLayout>
#include <QVBoxLayout>
#include <gfile.hpp>
#include <gstuff.hpp>
#include <QGroupBox>
#include <QSettings>

#include <dbapiplugin.hpp>

#include <QPropertyAnimation>

ExportBox::ExportBox(DSQLPlugin* pDSQL, QWidget *parent, GString *prevDir)
:QDialog(parent) //, f("Charter", 48, QFont::Bold)
{
    m_pIDSQL = pDSQL;
    this->resize(480,180);
	this->setWindowTitle("Export To File");

	
	m_gstrPrevDir = prevDir;


	QBoxLayout *topLayout = new QVBoxLayout();

    m_pMainGrid = new QGridLayout(this);
	
    topLayout->addLayout(m_pMainGrid);

	
	ok = new QPushButton(this);
	ok->setGeometry(30, 210, 50, 30);
	ok->setFixedHeight( ok->sizeHint().height() );
    ok->setDefault(true);
	cancel = new QPushButton(this);
	ok ->setText("Go!");
	cancel->setText("Cancel");
    cancel->setGeometry(100, 210, 70, 30);
	cancel->setFixedHeight( cancel->sizeHint().height() );
	connect(ok, SIGNAL(clicked()), SLOT(OKClicked()));
	connect(cancel, SIGNAL(clicked()), SLOT(CancelClicked()));
	
	
	//RadioButtons:
	QButtonGroup * formGroup = new QButtonGroup(this);
	QButtonGroup * contGroup = new QButtonGroup(this);
	
	allRB = new QRadioButton();
	selRB = new QRadioButton();
	contGroup->addButton(allRB);
	contGroup->addButton(selRB);

    connect(allRB, SIGNAL(clicked()), SLOT(modeClicked()));
    connect(selRB, SIGNAL(clicked()), SLOT(modeClicked()));
	


	ixfRB = new QRadioButton(this);
	delRB = new QRadioButton(this);
	wsfRB = new QRadioButton(this);
    ddlRB = new QRadioButton(this);
	txtRB = new QRadioButton(this);
	formGroup->addButton(ixfRB);
	formGroup->addButton(delRB);
	formGroup->addButton(wsfRB);
    formGroup->addButton(ddlRB);
	formGroup->addButton(txtRB);

    optionsB = new QPushButton("Options");
    connect(optionsB, SIGNAL(clicked()), SLOT(optionsClicked()));
    if( m_pIDSQL->getDBType() == DB2 )  formGroup->addButton(optionsB);
    else optionsB->hide();

	fileNameLE = new QLineEdit();
	
	getFileB = new QPushButton();
	connect(getFileB, SIGNAL(clicked()), SLOT(getFileClicked()));
	
	//formGroup->setGeometry(20, 20, 400, 100);
	//contGroup->setGeometry(20, 120, 400, 40);
	
	//Things in ContGroup
	//allRB->setGeometry(230, 10, 170, 20);
	//selRB->setGeometry(20, 10, 200, 20);
	allRB->setText("Export whole table");
	selRB->setText("Export SQL-Result");
    selRB->setChecked(true);
	
	//Things in FormatGroup
	/*
	ixfRB->setGeometry(20, 60, 100, 20);
	delRB->setGeometry(140, 60, 100, 20);
	wsfRB->setGeometry(260, 60, 100, 20);
	txtRB->setGeometry(380, 60, 80, 20);
	*/
	ixfRB->setText("IXF Format");
	delRB->setText("DEL Format");
	wsfRB->setText("WSF Format");
    ddlRB->setText("As SQL");
	txtRB->setText("Plain Text");
	getFileB->setGeometry(300, 20, 120, 20);
	getFileB->setFixedHeight( getFileB->sizeHint().height() );
	getFileB->setText("Select File");
	fileNameLE->setGeometry(20, 20, 280, 30);
	
	//Export LOBs
	exportLobCB = new QCheckBox(this);
	//exportLobCB->setGeometry(20, 180, 430, 20);
    exportLobCB->setText("Export LOBs to file(s), click 'Options' for more.");
	

    QGroupBox * btGroupBox = new QGroupBox();
    //btGroupBox->setTitle("Select format");
    QHBoxLayout *frmLayout = new QHBoxLayout;
    frmLayout->addWidget(ixfRB);
    frmLayout->addWidget(delRB);
    frmLayout->addWidget(wsfRB);
    frmLayout->addWidget(ddlRB);
    frmLayout->addWidget(txtRB);
    frmLayout->addWidget(optionsB);
    btGroupBox->setLayout(frmLayout);
    m_pMainGrid->addWidget(btGroupBox, 0, 0, 1, 4);
    /*
	grid->addWidget(ixfRB, 0, 0);
	grid->addWidget(delRB, 0, 1);
	grid->addWidget(wsfRB, 0, 2);
	grid->addWidget(txtRB, 0, 3); 
    */
	
    m_pMainGrid->addWidget(fileNameLE, 1,0,1,3);
	
    m_pMainGrid->addWidget(allRB, 2, 1);
    m_pMainGrid->addWidget(selRB, 2, 0);
	
    m_pMainGrid->addWidget(getFileB, 1, 3);
    m_pMainGrid->addWidget(exportLobCB, 4, 0, 1, 4);
	
    m_pMainGrid->addWidget(ok, 5, 0);
    m_pMainGrid->addWidget(cancel, 5, 3);

    m_iFullSelectLobCount = 0;
    m_iCurrentSelectLobCount = 0;


	/*************** ugly.
	QGroupBox *groupBox = new QGroupBox(tr("Exclusive Radio Buttons"));
	
	QRadioButton *radio1 = new QRadioButton(tr("&Radio button 1"));
	QRadioButton *radio2 = new QRadioButton(tr("R&adio button 2"));
	QRadioButton *radio3 = new QRadioButton(tr("Ra&dio button 3"));
	
    radio1->setChecked(true);
	
	QHBoxLayout *vbox = new QHBoxLayout;
	vbox->addWidget(radio1);
	vbox->addWidget(radio2);
	vbox->addWidget(radio3);
	vbox->addStretch(1);
	groupBox->setLayout(vbox);
	grid->addWidget(groupBox, 5, 0, 1, 4);
	********************/     
    ixfRB->setEnabled(false);
    wsfRB->setEnabled(false);
    delRB->setEnabled(false);
    ddlRB->setEnabled(false);
    txtRB->setChecked(true);

    if( m_pIDSQL->getDBType() == SQLSERVER )
    {
        ddlRB->setEnabled(true);
        ddlRB->setChecked(true);
    }
    if( m_pIDSQL->getDBType() == DB2 )
    {
        ixfRB->setEnabled(true);
        wsfRB->setEnabled(true);
        delRB->setEnabled(true);
        ixfRB->setChecked(true);
    }
    if( m_pIDSQL->getDBType() == MARIADB )
    {
        txtRB->setText("Standard");
    }

	aThread = NULL;
	tb = NULL;
	timer = new QTimer( this );
    CON_SET conSet;
    m_pIDSQL->currentConnectionValues(&conSet);
    m_pExpImpOptions = new ExpImpOptions(this, ExpImpOptions::MODE_EXPORT, conSet.DB);
	connect( timer, SIGNAL(timeout()), this, SLOT(timerEvent()) );	

}
ExportBox::~ExportBox()
{
    timer->stop();
	delete timer;
	/*
	delete ok;
	delete cancel;
	delete ixfRB;
	delete delRB;
	
	delete fileNameLE;
	delete getFileB;
	*/
    delete m_pExpImpOptions;
}

void ExportBox::setSelect(GString sel, GString table)
{
	iAction = sel;
	iTableName = table;
	
	unsigned int i;

    DSQLPlugin* pDSQL  = new DSQLPlugin(*m_pIDSQL);
    pDSQL->initAll("SELECT * FROM "+iTableName, 1);

    m_iFullSelectLobCount = 0;
    m_iCurrentSelectLobCount = 0;

    //Number of LOB files to remain below 4GB per file
    m_iFullSelectLobFiles = 1;
    m_iCurrentSelectLobFiles = 1;


	//Check for either fullSelect or specific select if LOBs are present.
    for(i=1; i<=pDSQL->numberOfColumns(); ++i)
	{
        if( (pDSQL->sqlType(i) >= 404 && pDSQL->sqlType(i) <= 413) ||
            (pDSQL->sqlType(i) >= 916 && pDSQL->sqlType(i) <= 970 ) )
        {
            m_iFullSelectLobCount++;
        }
	}
    if( m_iFullSelectLobCount > 0 ) m_iFullSelectLobFiles = getNumberOfLobFiles(pDSQL, table);


    if( sel != "SELECT * FROM "+iTableName )
    {
        GString constraint;
        int index = GString(sel).upperCase().indexOf(" WHERE ");
        if( index > 0 )
        {
            constraint = sel.subString(index, sel.length() ).strip();
        }
        else constraint = "";

        pDSQL->initAll(sel, 1);
        for( i=1; i<=pDSQL->numberOfColumns(); ++i)
        {
            if( (pDSQL->sqlType(i) >= 404 && pDSQL->sqlType(i) <= 413) ||
                    (pDSQL->sqlType(i) >= 916 && pDSQL->sqlType(i) <= 970 ) )
            {
                m_iCurrentSelectLobCount++;
            }
        }
        if( m_iCurrentSelectLobCount > 0 ) m_iCurrentSelectLobFiles = getNumberOfLobFiles(pDSQL, table, constraint);
    }
    else
    {
        m_iCurrentSelectLobCount = m_iFullSelectLobCount;
        m_iCurrentSelectLobFiles = m_iFullSelectLobFiles;
    }
    delete pDSQL;
    modeClicked();
}

int ExportBox::getNumberOfLobFiles(DSQLPlugin * pDSQL, GString table, GString constraint)
{
    GString getSumOfCols;
    for(int i=1; i<= (int)pDSQL->numberOfColumns(); ++i)
    {
        if( (pDSQL->sqlType(i) >= 404 && pDSQL->sqlType(i) <= 413) ||
                (pDSQL->sqlType(i) >= 916 && pDSQL->sqlType(i) <= 970 ) )
        {
            getSumOfCols += "sum(BIGINT(length("+pDSQL->hostVariable(i)+")))+";
        }
    }
    getSumOfCols = "select ("+getSumOfCols.stripTrailing("+") + ")/1024/1024/4096+1 from "+table+constraint;
    pDSQL->initAll(getSumOfCols);
    return pDSQL->rowElement(1,1).asInt();
}

void ExportBox::modeClicked()
{
    if( m_iCurrentSelectLobCount > 0 && selRB->isChecked() ) exportLobCB->setEnabled(true);
    else if( m_iFullSelectLobCount  > 0 && allRB->isChecked() ) exportLobCB->setEnabled(true);
    else
    {
        exportLobCB->setChecked(false);
        exportLobCB->setEnabled(false);
    }
}

void ExportBox::optionsClicked()
{
    m_pExpImpOptions->exec();
}

void ExportBox::callExport()
{

	GString path, file;
	path = GString(fileNameLE->text());
	
	GFile aFile(path, GF_APPENDCREATE);
	if( !aFile.initOK() )
	{
        mb("Invalid path / filename.");
		fileNameLE->setFocus();
		return;
	}
    else remove(path);
	//   remove( path) ;
	path = path.translate('\\', '/');
	if(  !path.length() || path.occurrencesOf("/") == 0 ) 
	{
        mb("Specify path and filename.");
		fileNameLE->setFocus();
		return;
	}
	
	
	file = path.subString(path.lastIndexOf('/')+1, path.length()).strip();
	if( !file.strip().length() ) 
	{
        mb("Filename missing or invalid. ");
		return;
	}
    /*
    int w = this->width();
    int h = this->height();
    QPropertyAnimation * animation = new QPropertyAnimation(this, "size");
    animation->setDuration(150);
    animation->setStartValue(QSize(w, h));
    animation->setEndValue(QSize(w, h+200));
    animation->start();
    expImpOptions * foo = new expImpOptions(m_pIDSQL, this, "DEL_EXPORT");
    foo->exec();
    m_gstrModified = foo->modifiedString();
    */
	
    timer->start( 200 );
	aThread = new ExportThread;
    tb = new ThreadBox( this, "Please wait...", "Exporting", m_pIDSQL->getDBTypeName() );
   	tb->setThread(aThread);
	aThread->setOwner( this );
    aThread->setBox(tb);
	aThread->start();
	
	tb->exec();

    if( m_gstrExpErr.length() )mb(m_gstrExpErr);
	/* tb is closed already.
	aThread->setDone();
	tb->enableClose();
	
    mb("Done.");
	*/
}

void ExportBox::timerEvent()
{
	if( !aThread ) return;
    if( !aThread->isAlive() )
    {
        if( tb) tb->close();
        timer->stop();
    }
}

void ExportBox::ExportThread::run()
{
	myExport->startExport();
}

void ExportBox::askLobExport()
{

    if( allRB->isChecked() && m_iFullSelectLobCount == 0 ) return;
    if( selRB->isChecked() && m_iCurrentSelectLobCount == 0 ) return;

    GString msg = "There are LOBs present. Do you want to export them too?";

    if( !exportLobCB->isChecked() )
    {
        if( QMessageBox::question(this, "PMF", msg, QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes )
        {
            exportLobCB->setChecked(true);
        }
    }
    if( exportLobCB->isChecked()  )
    {
        if( m_pExpImpOptions->getCheckBoxValue(ExpImpOptions::TYP_LOB, "lobsinsepfiles") == 0 )
        {
            int pathCount = m_pExpImpOptions->getFieldValue(ExpImpOptions::TYP_LOB, "filecount").asInt();
            int recommendedCount = 1;
            if( allRB->isChecked() && pathCount < m_iFullSelectLobFiles )
            {
                recommendedCount = m_iFullSelectLobFiles;
            }
            if( selRB->isChecked() && pathCount < m_iCurrentSelectLobFiles )
            {
                recommendedCount = m_iCurrentSelectLobFiles;
            }
            msg = "To keep the exported LOB files below 4GB a PathCount of "+GString(recommendedCount)+" is recommended.";
            msg +="\nWould you like to use this value?";
            if( recommendedCount > 1 && QMessageBox::question(this, "PMF", msg, QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes )
            {
                m_pExpImpOptions->setFieldValue(ExpImpOptions::TYP_LOB, "filecount", GString(recommendedCount));
            }
        }
    }
}

void ExportBox::OKClicked()
{
	if( Helper::askFileOverwrite(this, fileNameLE->text()) ) return;

    askLobExport();

	callExport();
    QSettings settings(_CFG_DIR, "pmf5");
    QString path = GStuff::pathFromFullPath(fileNameLE->text());
    settings.setValue("exportPath", path);
}

void ExportBox::startExport()
{
	GString format, selCmd;
    short erc = 0;
    m_gstrExpErr = "";
    GString modififier;

    if( delRB->isChecked() )
    {
        modififier = m_pExpImpOptions->createModifiedString(ExpImpOptions::TYP_DEL);
        format = "DEL" ;
    }
    else if( ixfRB->isChecked() )
    {
        modififier = m_pExpImpOptions->createModifiedString(ExpImpOptions::TYP_IXF);
        format = "IXF";
    }
    else if( wsfRB->isChecked() )
    {
        format = "WSF";
    }
    else if( ddlRB->isChecked() )
    {
        format = "DDL";
    }


	if( selRB->isChecked() ) selCmd = iAction;
	else selCmd = "SELECT * FROM "+iTableName;
	
    DSQLPlugin *pDSQL = new DSQLPlugin(*m_pIDSQL);
    pDSQL->initAll(selCmd, 1);
	
	GString fullPath = GString(fileNameLE->text());
	
	GString path, file;
	fullPath = fullPath.translate('\\', '/');
	if( fullPath.occurrencesOf('/') > 0 )
	{
		path = fullPath.subString(1, fullPath.lastIndexOf('/'));
		file = fullPath.subString(fullPath.lastIndexOf('/')+1, fullPath.length()).strip();
	}
	else
	{
		path = "";
		file = fullPath;
	}
	
	#ifdef MAKE_VC
	path = path.translate('/', '\\');
	#endif
	if( !file.length() ) return;
    DBAPIPlugin *pPlg = NULL;
    if( m_pIDSQL->getDBType() == DB2 )
    {
        pPlg = new DBAPIPlugin(m_pIDSQL->getDBTypeName());
        if( !pPlg || !pPlg->isOK() )
        {
            mb("Could not load the DB-API plugin, sorry.");
            return;
        }
    }
    if( txtRB->isChecked() && pDSQL->getDBType() == MARIADB )
    {
        GString expCmd = selCmd+" INTO OUTFILE '"+path+file+"'";
        m_gstrExpErr = pDSQL->initAll(expCmd);
        printf("EXPORT: %s\n", (char*) expCmd);
        delete pDSQL;
        return;
    }
    else if( txtRB->isChecked() )
	{
		unsigned int i, j;
		GString line;
		GFile gf(path+file, 3);
        pDSQL->initAll(selCmd, 0, 1);
        remove(path+file);
		
		line = "   File created by Poor Man's Flight (PMF) www.leipelt.de ";
		gf.writeToNewLine(line);
		line = "   Select: "+selCmd;
		gf.writeToNewLine(line);
		line = " ";
		gf.writeToNewLine(line);
		line = "";
        for( i=1; i<=pDSQL->numberOfColumns(); ++i)
        {
            line += pDSQL->hostVariable(i).leftJustify( pDSQL->dataLen(i) );
        }
        gf.writeToNewLine(line);
        for( i=1; i<=pDSQL->numberOfRows(); ++i)
		{
			line = "";
            for( j = 1; j <= pDSQL->numberOfColumns(); ++ j )
			{
                line += pDSQL->rowElement(i, j).strip("'").strip().leftJustify(pDSQL->dataLen(j));
			}
			gf.writeToNewLine(line);
		}        
        delete pDSQL;
		return;
	}
    else if( ddlRB->isChecked() )
    {
        GSeq <GString> startText;
        GSeq <GString> endText;
        GString err;
        erc = pDSQL->exportAsTxt(1, selCmd, iTableName, path+file, &startText, &endText, &err);
        if( erc) m_gstrExpErr ="Export failed. ErrCode: "+GString(erc)+", ErrMsg: "+err;
        delete pDSQL;
        return;
    }
    else if( exportLobCB->isChecked()  )
	{
        long count = pDSQL->getResultCount(selCmd);
		count = count / 1000 + 1;

        if( allRB->isChecked() )count *= m_iFullSelectLobCount;
        else if( selRB->isChecked() )count *= m_iCurrentSelectLobCount;

        modififier = m_pExpImpOptions->createModifiedString(ExpImpOptions::TYP_LOB) + " "+ modififier;
        int pathCount = m_pExpImpOptions->getFieldValue(ExpImpOptions::TYP_LOB, "filecount").asInt();
        GSeq<GString> pathSeq;
        for( int i = 1; i <= pathCount; ++i)
        {
            pathSeq.add(path.strip());
        }
        pPlg->exportTable(format, selCmd, path+"PMF_EXP.LOG", &pathSeq, file, count, modififier);
	}
    else if( m_pIDSQL->getDBType() == DB2 )
	{        
        erc = pPlg->exportTable(fileNameLE->text(), format, selCmd, path+"PMF_EXP.LOG", modififier);
		if( erc == 3107 ) 
		{
            m_gstrExpErr = "There is at least one warning in \n"+path+"PMF_EXP.LOG";
		   erc = 0;
		}
	}
    if( erc ) m_gstrExpErr = "Export failed, Error "+GString(erc)+", "+pPlg->SQLError();
}

unsigned long ExportBox::getResultCount(GString cmd)
{
   cmd = cmd.upperCase();
   if( cmd.occurrencesOf("SELECT") == 0 ) return 0;
   if( cmd.occurrencesOf("FROM") == 0 ) return 0;
   GString tmp = cmd.subString(1, cmd.indexOf("SELECT")+6);
   tmp += " COUNT (*) ";
   tmp += cmd.subString(cmd.indexOf("FROM"), cmd.length()).strip();
   return 0;
}

void ExportBox::CancelClicked()
{
	close();
}

int ExportBox::getFileClicked()
{    
    GString fileName = iTableName;

    fileName = fileName.removeAll('\"');
    if( delRB->isChecked() ) fileName += ".DEL";
    else if( ixfRB->isChecked() ) fileName += ".IXF";
    else if( wsfRB->isChecked() ) fileName += ".WSF";
    else if( ddlRB->isChecked() ) fileName += ".TXT";


    QString name = QFileDialog::getSaveFileName ( this, "Select file",  *m_gstrPrevDir+fileName, "*.*",  0, QFileDialog::DontConfirmOverwrite );
    if( name.isNull() || !name.length() ) return 1;
	fileNameLE->setText(name);

    *m_gstrPrevDir = GString(name).stripTrailing(fileName);
	return 0;
}
void ExportBox::mb(GString msg)
{
    Helper::msgBox(this, "pmf", msg);
}



