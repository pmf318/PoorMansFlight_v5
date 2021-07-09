//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include "bookmark.h"
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QDir>
#include <qlayout.h>
#include "pmfdefines.h"
#include "gxml.hpp"
#include "helper.h"

#include <gstuff.hpp>
#include <gdebug.hpp>
#include <qfont.h>
#include <gfile.hpp>

Bookmark::Bookmark( GDebug * pGDeb, QWidget* parent, GString sql, GString table )
: QDialog(parent)
{
    m_pGDeb = pGDeb;
	deb("ctor");
	m_strSQL = sql;
	m_strTable = table;
	if( table.strip(".") == _selStringCB ) table= "";
	
	//Read bookmarks from file
	readAll();
	
	//No GUI, just provide r/w access to bookmarks
	if( !parent ) return;

	

	this->setWindowTitle("Add bookmark");
	QVBoxLayout *topLayout = new QVBoxLayout( );
	
	QGridLayout *grid = new QGridLayout(this);
	topLayout->addLayout( grid, 10 );
	
    QLabel* tmpQLabel0 = new QLabel( this );
    tmpQLabel0->setText( "Name:" );
    //tmpQLabel0->setFixedHeight( tmpQLabel0->sizeHint().height() );
    grid->addWidget(tmpQLabel0, 0, 0);
	nameLE = new QLineEdit( this );
    grid->addWidget(nameLE, 0, 1, 1, 2);

	
    QLabel* tmpQLabel1 = new QLabel( this);
    tmpQLabel1->setText( "Table:" );
    grid->addWidget(tmpQLabel1, 1, 0);
    tableLE = new QLineEdit( this );
    grid->addWidget(tableLE, 1, 1, 1, 2);
	
    QLabel* tmpQLabel2 = new QLabel( this );
    tmpQLabel2->setText( "SQL CMD:" );
    grid->addWidget(tmpQLabel2, 2, 0);
	sqlLE = new QLineEdit( this );
    grid->addWidget(sqlLE, 2, 1, 1, 2);
	
	grid->setColumnStretch(0, 0);
	grid->setColumnStretch(1, 100);
	grid->setColumnStretch(2, 100);
    tableLE->setText(table);
	sqlLE->setText(sql);
	
	
	okB = new QPushButton();
	connect( okB, SIGNAL(clicked()), SLOT(okClicked()) );
	okB->setText( "OK" );
    okB->setAutoRepeat( false );
    //okB->setAutoResize( false );
    okB->setDefault(true);
	okB->setFixedHeight( okB->sizeHint().height() );
    grid->addWidget(okB, 3, 1);
	cancelB = new QPushButton();
	connect( cancelB, SIGNAL(clicked()), SLOT(cancelClicked()) );
	cancelB->setText( "Cancel" );
    cancelB->setAutoRepeat( false );
    //cancelB->setAutoResize( false );
	cancelB->setFixedHeight( cancelB->sizeHint().height() );
    grid->addWidget(cancelB, 3, 2);
	
	setGeometry(parent->pos().rx()+parent->width()/2 - 185,parent->pos().ry()+30, 370, 160);
	//resize( 270, 160 );
}


Bookmark::~Bookmark()
{

}
int Bookmark::clearBookmarkSeq()
{
    m_seqBookmark.deleteAll();
//	m_seqName.removeAll();
//	m_seqSQL.removeAll();
//	m_seqTable.removeAll();
	return 0;
}
void Bookmark::okClicked()
{
	GString name = nameLE->text();
	if( !name.length() ) 
	{
		msg("Please set a name.");
		nameLE->setFocus();
		return;
	}
		
    this->addBookmark(name, m_strTable, m_strSQL);
	this->save();
	close();
}
void Bookmark::keyPressEvent(QKeyEvent *event)
{
	switch(event->key())
	{
		case Qt::Key_Escape:
			cancelClicked();
			break;
			
		case Qt::Key_Return:
			okClicked();
			break;
			
	}
	event->accept();
}
void Bookmark::closeEvent(QCloseEvent * event)
{
	event->accept();
}

void Bookmark::cancelClicked()
{
	close();
}


int Bookmark::readAll()
{
	deb("readAll start");
	GString name, sql, table, data;
	QString home = QDir::homePath ();
	if( !home.length() ) return 1; 
	deb("bmFile: "+bmFileName());

//    GString bn = bmFileName();
//    int res = access( bmFileName(), F_OK );
//    if( !Helper::fileExists(bmFileName())) return 0;
//    GXml gx;
//    gx.readFromFile(bmFileName());

//    GXml bmXml = gx.getBlocksFromXPath("BOOKMARKS");
//    int count = bmXml.countBlocks("bookmark");
//    for(int i=1; i <= count; ++i )
//    {
//        name = bmXml.getBlockAtPosition("bookmark", i).getAttribute("Name");
//        GXml sqlXml = bmXml.getBlockAtPosition("bookmark", i).getBlocksFromXPath("SQL");
//        sql = sqlXml.toString().stripLeading("<SQL>").stripTrailing("</SQL>");

//    }

	deb("Open file...");
	GFile f( bmFileName(), GF_APPENDCREATE);
    BOOKMARK * pBm;
	for( int i = 1; i <=f.lines(); ++i )
	{
		data = f.getLine(i);
        if( data.occurrencesOf(_PMF_PASTE_SEP) != 2 ) continue;
        pBm = new BOOKMARK;
        pBm->Name = data.subString(1, data.indexOf(_PMF_PASTE_SEP)-1).strip();
        data = data.remove(1, data.indexOf(_PMF_PASTE_SEP)+_PMF_PASTE_SEP.length()-1);
        pBm->Table = data.subString(1, data.indexOf(_PMF_PASTE_SEP)-1).strip();
        data = data.remove(1, data.indexOf(_PMF_PASTE_SEP)+_PMF_PASTE_SEP.length()-1);
        pBm->SqlCmd = data;
        m_seqBookmark.add(pBm);
	}
	deb("readAll done.");
	return 0;
}
int Bookmark::count()
{
    return m_seqBookmark.numberOfElements();
}
int Bookmark::addBookmark(GString name, GString table, GString sqlCmd)
{
    BOOKMARK *pBm = new BOOKMARK;
    pBm->Name = name;
    pBm->Table = table;
    pBm->SqlCmd = sqlCmd;

    m_seqBookmark.add(pBm);
	return 0;
}
int Bookmark::save()
{
    GFile *file;
	QString home = QDir::homePath ();
	if( !home.length() ) return 2; 
    if( !Helper::fileExists(bmFileName()) ) file = new GFile(bmFileName(), GF_APPENDCREATE);
    else file = new GFile( bmFileName() );

    BOOKMARK * pBm;
    GSeq<GString> seqAllBM;
    for( unsigned int i = 1; i <= m_seqBookmark.numberOfElements(); ++i )
	{
        pBm = m_seqBookmark.elementAtPosition(i);
        seqAllBM.add(pBm->Name + _PMF_PASTE_SEP + pBm->Table + _PMF_PASTE_SEP + (pBm->SqlCmd).change("\n", _CRLF_MARK));
	}
    file->overwrite(&seqAllBM);
    delete file;

    return 0;
}
GString Bookmark::getBookmarkName(int i)
{
    if( i < 1 || i > (signed) m_seqBookmark.numberOfElements() ) return "";
    return m_seqBookmark.elementAtPosition(i)->Name;
}

GString Bookmark::getBookmarkFlatSQL(int i)
{
    if( i < 1 || i > (signed) m_seqBookmark.numberOfElements() ) return "";
    return (m_seqBookmark.elementAtPosition(i)->SqlCmd).change(_CRLF_MARK, " ").removeButOne(' ');
}

GString Bookmark::getBookmarkFormattedSQL(int i)
{
    if( i < 1 || i > (signed) m_seqBookmark.numberOfElements() ) return "";
    return (m_seqBookmark.elementAtPosition(i)->SqlCmd).change(_CRLF_MARK, "\n");
}

GString Bookmark::getBookmarkTable(int i)
{
    if( i < 1 || i > (signed)m_seqBookmark.numberOfElements() ) return "";
    return m_seqBookmark.elementAtPosition(i)->Table;
}

void Bookmark::msg(GString txt)
{
	QMessageBox::information(this, "Bookmark", txt);
}
GString Bookmark::bmFileName()
{
	deb("bmFileName start");
	QString home = QDir::homePath ();
	if( !home.length() ) return "";
	#ifdef MAKE_VC
	return GString(home)+"\\"+_CFG_DIR+"\\"+_BM_FILE;
	#else
	return GString(home)+"/."+_CFG_DIR + "/"+_BM_FILE;
	#endif
}
void Bookmark::deb(GString msg)
{    
    m_pGDeb->debugMsg("bookmark", 1, msg);
}
