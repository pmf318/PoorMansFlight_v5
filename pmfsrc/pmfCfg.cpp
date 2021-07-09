//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include "pmfCfg.h"
#include "pmfdefines.h"
#include <gstuff.hpp>
#include <gfile.hpp>

#include <QGridLayout>
#include <QDir>
#include "pmf.h"

PmfCfg::PmfCfg(GDebug *pGDeb, QWidget *parent)
  :QDialog(parent) //, f("Charter", 48, QFont::Bold)
{
    m_pGDeb = pGDeb;

	deb("Ctor (1)");
	QGridLayout * grid = new QGridLayout(this);
	
	//this->resize(260, 200);
	ok = new QPushButton(this);
	esc = new QPushButton(this);

	ok->setText("Save");
	esc->setText("Cancel");
	QLabel * hint1 = new QLabel("Set configuration here.", this);
	QLabel * hint2 = new QLabel("", this);	
	
	GSeq <GString > m_keySeq;
	connect(ok, SIGNAL(clicked()), SLOT(save()));
	connect(esc, SIGNAL(clicked()), SLOT(cancel()));

	int j = 0;
	deb("Ctor (2)");	
	// This adds items to the grid
	/*
	addToSeq("Database", "Default database to connect to"); j++;
	addToSeq("User","Username"); j++;
	addToSeq("Password","CAUTION: Will be saved as plain text!"); j++;	
	*/
    //addToSeq("ESC", "Use ESC to close PMF (value 1 or 0)");j++;
	addToSeq("MaxRows","Number of rows to fetch, default: 2000"); j++;
	///addToSeq("CharForBit","1: Don't show, 2: as binary "); j++;
	deb("Ctor (3)");	
	GString cfgFile = loadCfgFile(grid);
	if( !cfgFile.length() ) 
	{
		deb("Ctor (4)");
		createCfgFile();
		cfgFile = loadCfgFile(grid);
	}
	deb("Ctor (5)");		
    hint2->setText("[ConfigFile is "+cfgFileName()+"]");
	grid->addWidget(hint1, 0, 0, 1, 2);
	grid->addWidget(hint2, j+1, 0, 1, 2);
	deb("Ctor (6)");			
//      Setting row=m_cfgSeq.numberOfElements() does not work. Increment the lineNr for ok/esc button manually.
	grid->addWidget(ok, j+2, 0);
	grid->addWidget(esc, j+2, 1);	
	deb("Ctor done");
}
PmfCfg::~PmfCfg()
{
}
void PmfCfg::addToSeq(GString key, GString help)
{
	CFG_ROW * pRow = new CFG_ROW;
	pRow->key  = key;
	pRow->help = help;
	m_cfgSeq.add(pRow);
	//pRow->pLE = new QLineEdit(this);
}
int PmfCfg::save()
{

	GFile cfgFile(cfgFileName());
	if( !cfgFile.initOK() ) return 2;
	GString val, hlp;
	CFG_ROW * pRow;
	for( unsigned int i = 1; i <= m_cfgSeq.numberOfElements();  ++i )
	{
		pRow = m_cfgSeq.elementAtPosition(i);
        cfgFile.addOrReplaceLine(pRow->key, pRow->key+"="+GString(pRow->pLE->text()));
	}
	close();
	return 0;
}
void PmfCfg::cancel()
{
	close();
}
GString PmfCfg::createCfgFile()
{
	QString home = QDir::homePath ();
	if( !home.length() ) return "";
	
	QDir aDir(home);
    #if defined(MAKE_VC) || defined (__MINGW32__)
	aDir.mkdir(_CFG_DIR);

	#else
	aDir.mkdir("."+_CFG_DIR);
	#endif
	QFile cfgFile( cfgFileName() );	
	if (!cfgFile.open(QIODevice::WriteOnly | QIODevice::Text)) return "";
	CFG_ROW * pRow;
	QTextStream out(&cfgFile);
	out << "#\n# This is PMFs config file. Do not edit manually.\n";
	out << "# (Unless you know what you're doing. If you know what you're doing, that's cool.)\n#\n";	
	for(unsigned int i = 1; i <= m_cfgSeq.numberOfElements(); ++ i)
	{
		pRow = m_cfgSeq.elementAtPosition(i);
		out << "HELP_" << (char*)pRow->key << "=" << (char*) pRow->help << "\n";		
		out << (char*) pRow->key << "=\n";
	}
	cfgFile.close();
	return home;
}
GString PmfCfg::loadCfgFile(QGridLayout * grid)
{
	deb("::loadCfgFile (1)");
	QString home = QDir::homePath ();
	deb("::loadCfgFile (2)");	
	if( !home.length() ) return "";	
	QLabel *pKeyLB, *pHlpLB;
	GFile cfgFile( cfgFileName() );
	deb("::loadCfgFile (3)");	
	if( !cfgFile.initOK() ) return "";
	GString val, hlp;
	CFG_ROW * pRow;
	deb("::loadCfgFile (3.1), elmts: "+GString(m_cfgSeq.numberOfElements()));
	for( unsigned int i = 1; i <= m_cfgSeq.numberOfElements();  ++i )
	{
		deb("::loadCfgFile (4), i: "+GString(i));
		pRow = m_cfgSeq.elementAtPosition(i);
		deb("::loadCfgFile (4.1), i: "+GString(i));		
		pRow->pLE = new QLineEdit(this);	
		deb("::loadCfgFile (4.2), i: "+GString(i));		
		pKeyLB = new QLabel(pRow->key, this);		
		deb("::loadCfgFile (4.3), i: "+GString(i)+", key: "+pRow->key);		
		val = cfgFile.getKey(pRow->key);
		deb("::loadCfgFile (4.4), i: "+GString(i));
		hlp = cfgFile.getKey("HELP_"+pRow->key);
		pHlpLB = new QLabel(hlp, this);		
		deb("::loadCfgFile (4.5), i: "+GString(i));		
		pRow->pLE->setText(val);
		deb("::loadCfgFile (4.6), i: "+GString(i));		
		grid->addWidget(pKeyLB, i, 0);
		grid->addWidget(pRow->pLE, i, 1, 1, 2);
		grid->addWidget(pHlpLB, i, 3);
		deb("::loadCfgFile (4.7), i: "+GString(i));		
	}
	deb("::loadCfgFile (5)");	
	return GString(home) + "/" + _CFGFILE;
}
GString PmfCfg::getValue(GString key)
{
	deb("::getVal (1)");
	QString home = QDir::homePath ();
	deb("::getVal (2)");	
	if( !home.length() ) return "";
	GFile cfgFile( cfgFileName() );
	deb("::getVal (3)");	
	if( !cfgFile.initOK() ) return "";
	deb("::getVal (4)");	
	return cfgFile.getKey(key);
}
GString PmfCfg::cfgFileName()
{
	QString home = QDir::homePath ();
	if( !home.length() ) return "";
    #if defined(MAKE_VC) || defined (__MINGW32__)
	return GString(home)+"\\"+_CFG_DIR+"\\"+_CFGFILE;
	#else
	return GString(home)+"/."+_CFG_DIR + "/"+_CFGFILE;
	#endif
}
void PmfCfg::deb(GString txt)
{
    m_pGDeb->debugMsg("pmfCfg", 1, txt);
}

