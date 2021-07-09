#ifndef DB2MENU_H
#define DB2MENU_H

#include <qglobal.h>

#if QT_VERSION >= 0x050000
#include <QtWidgets/QApplication>
#else
#include <QtGui/QApplication>
#endif

#include <QMenuBar>
#include <QTableWidget>
#include <QAction>

class Pmf;

class Db2Menu
{
private:
    Pmf* m_pPmf;
    QMenuBar * m_pMenuBar;
    QTableWidget * m_pWdgt;
    QAction * m_qaCreateDDL;


public:
    Db2Menu(Pmf * pPmf, QMenuBar* menuBar);
    ~Db2Menu();

private slots:
    void createDDL();

};

#endif // DB2MENU_H
