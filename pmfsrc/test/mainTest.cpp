#include <tabEdit.h>

#include <stdio.h>
#include <gstring.hpp>
#include <QString>


void deb(GString msg);
void prepareTables();

DSQLPlugin * _plg;

int main(int argv, char **args)
{
    QApplication app(argv, args);

    GDebug* _pGDeb = GDebug::getGDebug(3);
    _plg = new DSQLPlugin(_DB2, "../../plugins/libdb2dsql.so");
    _plg->connect("SAMPLE", "", "", "");
    _plg->setGDebug(_pGDeb);

    prepareTables();

    GSeq <GString> unqColSeq;
    _plg->getUniqueCols("test.id_xml", &unqColSeq);


    QTableWidgetItem *pItem;


    QTableWidget *pWdgt = new QTableWidget(1, 5);
    pWdgt->setItem(0, 0, new QTableWidgetItem("1"));
    pWdgt->setItem(0, 1, new QTableWidgetItem(""));
    pWdgt->setItem(0, 2, new QTableWidgetItem("36"));
    pWdgt->setItem(0, 3, new QTableWidgetItem("XYZ"));
    pWdgt->setItem(0, 4, new QTableWidgetItem(""));

    GDebug * pGDB = new GDebug(3);
    tabEdit * pTE = new tabEdit(_plg, pWdgt, pGDB);
    pItem = pWdgt->itemAt(0,0);
    if( !pItem ) return 0;
    DSQLPlugin * pDSQL = new DSQLPlugin(*_plg);
    _plg->initAll("select * from test.defaulttest");
    GString err = pTE->updateRowViaUniqueCols(pItem, pDSQL, &unqColSeq, "", -1, "");
    deb(err);

    return 0;
}


void prepareTables()
{
    deb("--- Preparing tables");
    GString cmd = "delete from test.id_xml";
    _plg->initAll(cmd);
    cmd = "INSERT INTO \"TEST\".\"ID_XML\" (\"ID\", \"NAME\") VALUES (x'1B04B0B7C4557B478A29AC2E621A5F95', 'BLA')";
    GString err = _plg->initAll(cmd);
    if( err.length()) deb("---> Test insertGuid failed: "+err);
}

void deb(GString  msg)
{
    printf("Test> %s\n", (char*)msg);
#ifdef MAKE_VC
    flushall();
#endif
}
