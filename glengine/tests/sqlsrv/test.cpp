#include <stdio.h>

#include <gstring.hpp>
#include <dsqlplugin.hpp>
#include <gdebug.hpp>
#include <idsql.hpp>
#include "gseq.hpp"

#define BUFLEN 256

DSQLPlugin * _plg;
GDebug* _pGDeb;


void deb(GString msg);
void in();

void runTests();
void updateDefaultCol();
void readGuid();
void readDefaultFromClob();

int main(int argc, char** ppArgv)
{
    deb("Testing SqlSrv plugin...");
    GDebug* _pGDeb = GDebug::getGDebug(0);

    _plg = new DSQLPlugin(_SQLSRV, "../../../plugins/odbcdsql.dll");
    if( _plg->loadError() )
    {
        deb("LoadErr: "+GString(_plg->loadError()));
        return 0;
    }
    _plg->setGDebug(_pGDeb);
    _plg->initAll("USE \"IMAGECENTER\"");
    GString err = _plg->connect("w7_loc", "sa", "cheffe"); //Yup, passwd in source.
    deb("Conn: "+err);
    runTests();
    _plg->disconnect();
    return 0;
}


void runTests()
{
    updateDefaultCol();
    readGuid();
    readDefaultFromClob();
}

void updateDefaultCol()
{
    deb("--- Running "+GString(__FUNCTION__));
    GString cmd = "delete from imagecenter.test.defaulttest";
    _plg->initAll(cmd);
    cmd = "INSERT INTO \"IMAGECENTER\".\"TEST\".\"DEFAULTTEST\" (\"TEXT\") VALUES ('BLA')";
    GString err = _plg->initAll(cmd);
    if( err.length()) deb("---> Test updateDefaultCol() failed: "+err);
}

void readGuid()
{
    deb("--- Running "+GString(__FUNCTION__));
    GString cmd = "delete from \"IMAGECENTER\".\"TEST\".\"ID_XML\"";
    _plg->initAll(cmd);
    cmd = "INSERT INTO \"IMAGECENTER\".\"TEST\".\"ID_XML\" (\"ID\", \"NAME\") VALUES ('01994974-495F-4AFB-9EFF-973268BC62D8', 'BLA')";
    GString err = _plg->initAll(cmd);
    if( err.length()) deb("---> Test insertGuid failed: "+err);

    cmd = "select count(*) from \"IMAGECENTER\".\"TEST\".\"ID_XML\" where id='01994974-495F-4AFB-9EFF-973268BC62D8'";
    err = _plg->initAll(cmd);
    if( err.length()) deb("---> Test selectGuid failed: "+err);

    if( _plg->rowElement(1,1).asInt() != 1 )deb("---> Test countGuid failed. count: "+GString(_plg->rowElement(1,1).asInt()));

    cmd = "select id from \"IMAGECENTER\".\"TEST\".\"ID_XML\" where name='BLA'";
    err = _plg->initAll(cmd);
    if( err.length()) deb("---> Test selectGuidByName failed: "+err);

    if( _plg->rowElement(1,1) != "'01994974-495F-4AFB-9EFF-973268BC62D8'" )deb("---> Test readGuidByName failed: "+_plg->rowElement(1,1));

}

void readDefaultFromClob()
{
    deb("--- Running "+GString(__FUNCTION__));
    GSeq<COL_SPEC*> specSeq;
    _plg->getColSpecs("\"IMAGECENTER\".\"TEST\".\"ID_XML\"", &specSeq);
    if( specSeq.elementAtPosition(2)->Default != "''") deb("---> Test readDefaultFromClob failed: Not ''");
    if( specSeq.elementAtPosition(4)->Default != "getdate()") deb("---> Test readDefaultFromClob failed: Not getdate() but "+specSeq.elementAtPosition(4)->Default);
}

void deb(GString  msg)
{
    printf("Test> %s\n", (char*)msg);
#ifdef MAKE_VC
    flushall();
#endif
}
void in()
{
    char buffer[100];   /* the string is stored through pointer to this buffer */
    printf("Enter string:");
    fflush(stdout);
    fgets(buffer, BUFLEN, stdin); /* buffer is sent as a pointer to fgets */

}
