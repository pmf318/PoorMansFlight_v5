//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include <qapplication.h>
#include <qpushbutton.h>
#include <QLineEdit>
#include <qdialog.h>
#include <qlabel.h>
#include <QRadioButton>
#include <QComboBox>
#include <qbuttongroup.h>
#include <qlistwidget.h>
#include <QTextEdit>
#include <QTextBrowser>
#include <QTabWidget>
#include <QTableWidget>
#include <gstring.hpp>
#include <qfiledialog.h>
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qpainter.h>
#include <qfont.h>
#include <dsqlplugin.hpp>

#ifndef _OPTIONSTAB_
#include "optionsTab.h"
#endif
#include "txtEdit.h"
#include "dbapiplugin.hpp"
#include <gfile.hpp>
#ifndef _CatalogDB_
#define _CatalogDB_


typedef struct{
   GString Database;
   GString Alias;
   GString NodeName;
   GString Host;
   int Port;
   } CATALOG_DB;


class CatalogDB : public QDialog
{
    Q_OBJECT

public:
    CatalogDB(DSQLPlugin* pDSQL, QWidget * parent = 0);
    ~CatalogDB();

    //   void tm(GString message){QMessageBox::information(this, "PMF", message);}
    void saveGeometry();
    void restoreGeometry();
    void closeEvent(QCloseEvent *event);
    int catalogChanged();
    static int catalogDbAndNode(CATALOG_DB * pCatalogDb);
    static int uncatalogDb(CATALOG_DB * pCatalogDb);


    
private slots:
    void OKClicked();
    void closeClicked();
    void selectionToggled();

protected:
    void keyPressEvent(QKeyEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);

private:
    void fillAllFields();
    GSeq<NODE_INFO*> fillNodesCB(DBAPIPlugin* pApi);
    static int dbAliasExistsOnNode(CATALOG_DB * pCatalogDb, GSeq <DB_INFO*> *dbSeq, GSeq<NODE_INFO*> *nodeSeq );
    static int nodeExists(CATALOG_DB * pCatalogDb, GSeq<NODE_INFO*>* nodeSeq);
    QRadioButton *useExistingRB, *createNewRB;
    QComboBox *nodesCB, *protocolCB;
    QLineEdit *nameLE, *hostLE, *commentLE, *portLE, *databaseLE, *aliasLE;
    QPushButton * okB;
    QPushButton * closeB;
    int m_iCatalogChanged;

    QRect m_qrGeometry;
    void msg(GString txt);

    OptionsTab * m_pTab;
    GString _dbTypeName;
};

#endif
