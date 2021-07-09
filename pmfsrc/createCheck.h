//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2000
//

#include <qapplication.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qradiobutton.h>
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

#include <gfile.hpp>
#ifndef _createCheck_
#define _createCheck_


class CreateCheck : public QDialog
{
    Q_OBJECT
public:
    CreateCheck(DSQLPlugin* pDSQL, GString tableName, QWidget * parent = 0);
    ~CreateCheck();

    //   void tm(GString message){QMessageBox::information(this, "PMF", message);}
    void saveGeometry();
    void restoreGeometry();
    void closeEvent(QCloseEvent *event);

private slots:
    void OKClicked();
    void closeClicked();

private:
    void keyPressEvent(QKeyEvent *event);
    QPushButton * okB;
    QPushButton * closeB;

    QRect m_qrGeometry;
    void msg(GString txt);
    QWidget* createFields(GString groupName);

    DSQLPlugin * m_pDSQL;
    GString m_strTableName;
    OptionsTab * m_pTab;
};

#endif
