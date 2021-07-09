#ifndef TABLESELECTOR_H
#define TABLESELECTOR_H

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


class TableSelector : public QWidget
{
    Q_OBJECT
public:
    TableSelector(DSQLPlugin* pDSQL, QWidget *parent=0, GString currentSchema = "", int hideSysTabs = 0, int hideListBox = 0);
    ~TableSelector();
    void fillLB(GString currentSchema);
    QListWidget * getTableHandle();
    GString tablePrefix();

private slots:
    void schemaSelected(int index);
    void contextSelected(int index);
    void tableSelected(int index);


signals:
    void tableSelection(QString tableName);
    void schemaSelection(QString schema);

private:
    QListWidget* tableLB;
    PmfSchemaCB * schemaCB;
    QComboBox * contextCB;
    QComboBox * tableCB;
    DSQLPlugin * m_pDSQL;
    int iHideSysTabs;
    GString strCurrentSchema;
    void fillSchemaCB(GString context );

};

#endif // TABLESELECTOR_H

