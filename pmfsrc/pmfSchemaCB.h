//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#ifndef _PMFSCHEMACB_
#define _PMFSCHEMACB_

#include <QComboBox>

#include <gstring.hpp>
#include <gseq.hpp>
#include <dsqlplugin.hpp>

class  PmfSchemaCB: public QComboBox
{
    Q_OBJECT
public:

    PmfSchemaCB(QWidget* parent, GString currentSchema = "" );
    virtual ~PmfSchemaCB(){}
    int fill(DSQLPlugin* pDSQL, GString schema = "", int hideSysTabs = 0, int caseSensitive = 1 );
private:
	GString m_gstrCurrentSchema;    
};

#endif // _included
