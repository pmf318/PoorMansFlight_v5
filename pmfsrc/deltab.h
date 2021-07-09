//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include <qapplication.h>
#include <qpushbutton.h>
#include <qmainwindow.h>
#include <qlineedit.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <QListWidget>
#include <QComboBox>
#include <gstring.hpp>
#include <qfiledialog.h>
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>

#include <gseq.hpp>


#include <dsqlplugin.hpp>
#include "pmfSchemaCB.h"
#include "tableSelector.h"

#ifndef _DELTAB_
#define _DELTAB_

class Deltab : public QDialog
{
    Q_OBJECT
public:
    Deltab(DSQLPlugin* pDSQL, QWidget *parent=0, GString currentSchema = "", int hideSysTabs = 0);
    ~Deltab();


private slots:
    void OKClicked();
    void CancelClicked();
    void keyPressEvent(QKeyEvent *event);

private:
    QPushButton * ok, * cancel;
    DSQLPlugin * m_pDSQL;
    TableSelector * tbSel;
    void msg(GString message);
};

#endif
