//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//
#include "pmfSchemaCB.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h> 
//#include <windows.h> 
#include <gstring.hpp>
#include <QComboBox>
#include <qmessagebox.h>


#include "pmfdefines.h"

PmfSchemaCB::PmfSchemaCB(QWidget *parent, GString currentSchema )
 :QComboBox(parent)
{    
	m_gstrCurrentSchema = currentSchema;
}

int PmfSchemaCB::fill(DSQLPlugin * pDSQL, GString schema, int hideSysTabs, int caseSensitive)
{
    this->clear();
	GString s;
    pDSQL->getTabSchema();

	int cur = -1;
	this->addItem( _selStringCB); //First item is "<select>"
    for( unsigned int i = 1; i <= pDSQL->numberOfRows(); ++i)
	{        
        s= pDSQL->rowElement(i,1).strip().strip("'").strip();
        if( hideSysTabs && ( s == "SYSCAT" || s == "SYSIBM" ||
                             s == "SYSIBMADM" || s == "SYSPUBLIC" ||
                             s == "SYSSTAT" || s == "SYSTOOLS") ) continue;
        if( !caseSensitive )
        {
            if( GString(schema).upperCase() == GString(s).upperCase() ) cur = i;
        }
        else if( schema == s ) cur = i;
		this->addItem(s);
	}
	if( cur >= 0 ) 
	{
		this->setCurrentIndex(cur);
	}
    return cur;
}




