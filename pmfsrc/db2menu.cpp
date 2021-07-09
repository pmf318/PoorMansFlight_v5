#include "db2menu.h"
#include "pmf.h"


Db2Menu::Db2Menu(Pmf * pPmf, QMenuBar* menuBar)
{
    m_pPmf = pPmf;
    m_pMenuBar = menuBar;

    QMenu * tabMenu = new QMenu("DB2 Table");
    //m_qaCreateDDL = new QAction();

    tabMenu->addAction("Create DDL", pPmf, SLOT(createDDL()));
    menuBar->addMenu(tabMenu);

}
void Db2Menu::createDDL()
{
    TabEdit * pTE = m_pPmf->currentTabEdit();
    if( pTE ) pTE->runGetClp();
}

Db2Menu::~Db2Menu()
{
 //   m_pMenuBar->removeAction();
}
