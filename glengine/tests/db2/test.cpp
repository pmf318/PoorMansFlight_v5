#include <stdio.h>

#include <gstring.hpp>
#include <dsqlplugin.hpp>
#include <dbapiplugin.hpp>
#include <gdebug.hpp>
#include <idsql.hpp>
#include "gseq.hpp"

#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define BUFLEN 256

DSQLPlugin * _plg;
GDebug* _pGDeb;


void deb(GString msg);
void in();

void runTests();
void updateDefaultCol();
void readGuid();
void readDefaultFromClob();
void getNullableVal();
void writeLobs();
void readWriteDoubleDec();
void readWriteMultiLob();
void newDB2Export();
void newDB2Import();
int fileSize(GString fileName);


int main(int argc, char** ppArgv)
{
    deb("Testing DB2 plugin...");
    GDebug* _pGDeb = GDebug::getGDebug(0);
#ifdef MAKE_VC
    _plg = new DSQLPlugin(_DB2, "../../../plugins/db2dsql.dll");
#else
    _plg = new DSQLPlugin(_DB2, "../../../plugins/libdb2dsql.so");
#endif
    if( _plg->loadError() )
    {
        deb("LoadErr: "+GString(_plg->loadError()));
        return 0;
    }
    _plg->setGDebug(_pGDeb);
    _plg->connect("sample", "", "");
    runTests();
    _plg->disconnect();
    delete _plg;

    deb("Testing DB2(ODBC) plugin...");
#ifdef MAKE_VC
    _plg = new DSQLPlugin(_DB2ODBC, "../../../plugins/db2dcli.dll");
#else
    _plg = new DSQLPlugin(_DB2ODBC, "../../../plugins/libdb2dcli.so");
#endif

    if( _plg->loadError() )
    {
        deb("LoadErr: "+GString(_plg->loadError()));
        return 0;
    }
    _plg->setGDebug(_pGDeb);
    _plg->connect("sample", "", "");
    runTests();
    _plg->disconnect();
    delete _plg;

    return 0;
}


void runTests()
{
    newDB2Export();
    newDB2Import();

    updateDefaultCol();
    readGuid();
    readDefaultFromClob();
    getNullableVal();
    writeLobs();
    readWriteDoubleDec();
    readWriteMultiLob();
}

void newDB2Export()
{
    deb("--- Running "+GString(__FUNCTION__));
    DBAPIPlugin *pApiPlg = NULL;
    pApiPlg = new DBAPIPlugin(_DB2, "../../../plugins/libdb2dapi.so");
    if( !pApiPlg || !pApiPlg->isOK() )
    {
         deb("Could not load the DB-API plugin, sorry.");
        return;
    }
    GSeq<GString> pathSeq;
    pathSeq.add("/home/moi/develop/c/pmf5.1/glengine/tests/db2/exported/");
    int erc = pApiPlg->exportTableNew("DEL", "SELECT id, rule, xmlcol FROM TEST.MULTILOB", "msg.txt", &pathSeq, "Test.del", 1, "LOBSINFILE XMLINSEPFILES") ;
    if( erc) deb("---> Test failed: "+GString(erc));
    delete pApiPlg;
}

void newDB2Import()
{
    deb("--- Running "+GString(__FUNCTION__));
    DBAPIPlugin *pApiPlg = NULL;
    pApiPlg = new DBAPIPlugin(_DB2, "../../../plugins/libdb2dapi.so");
    if( !pApiPlg || !pApiPlg->isOK() )
    {
         deb("Could not load the DB-API plugin, sorry.");
        return;
    }
    GSeq<GString> pathSeq;
    pathSeq.add("/home/moi/develop/c/pmf5.1/glengine/tests/db2/exported");
    int erc = pApiPlg->importTableNew("/home/moi/develop/c/pmf5.1/glengine/tests/db2/Test.del", &pathSeq, "DEL", "INSERT INTO TEST.MULTILOB2 ", "impMsg.txt", "XMLCHAR");
    if( erc) deb("---> Test failed: "+GString(erc));
    _plg->commit();
    delete pApiPlg;
}


void updateDefaultCol()
{
    deb("--- Running "+GString(__FUNCTION__));
    GString cmd = "delete from test.defaulttest";
    _plg->initAll(cmd);
    cmd = "INSERT INTO \"TEST\".\"DEFAULTTEST\" (\"TEXT\") VALUES ('BLA')";
    GString err = _plg->initAll(cmd);
    if( err.length()) deb("---> Test updateDefaultCol() failed: "+err);
}

void readGuid()
{
    deb("--- Running "+GString(__FUNCTION__));
    GString cmd = "delete from test.id_xml";
    _plg->initAll(cmd);
    cmd = "INSERT INTO \"TEST\".\"ID_XML\" (\"ID\", \"NAME\") VALUES (x'1B04B0B7C4557B478A29AC2E621A5F95', 'BLA')";
    GString err = _plg->initAll(cmd);
    if( err.length()) deb("---> Test insertGuid failed: "+err);

    cmd = "select count(*) from \"TEST\".\"ID_XML\" where id=x'1B04B0B7C4557B478A29AC2E621A5F95'";
    err = _plg->initAll(cmd);
    if( err.length()) deb("---> Test selectGuid failed: "+err);

    if( _plg->rowElement(1,1).asInt() != 1 )deb("---> Test countGuid failed: "+err);

    cmd = "select id from \"TEST\".\"ID_XML\" where name='BLA'";
    err = _plg->initAll(cmd);
    if( err.length()) deb("---> Test selectGuidByName failed: "+err);

    if( _plg->rowElement(1,1) != "'1B04B0B7C4557B478A29AC2E621A5F95'" )deb("---> Test readGuidByName failed: "+_plg->rowElement(1,1));

}

void getNullableVal()
{
    deb("--- Running "+GString(__FUNCTION__));
    _plg->initAll("select * from test.id_xml");

    if( _plg->isNullable(1) != 0 ) deb("---> Test "+GString(__FUNCTION__)+" failed.");
    if( _plg->isNullable(2) != 0 ) deb("---> Test "+GString(__FUNCTION__)+" failed.");
    if( _plg->isNullable(3) != 1 ) deb("---> Test "+GString(__FUNCTION__)+" failed.");
    if( _plg->isNullable(4) != 0 ) deb("---> Test "+GString(__FUNCTION__)+" failed.");

    _plg->initAll("select * from test.defaulttest");
    if( _plg->isNullable(1) != 0 ) deb("---> Test "+GString(__FUNCTION__)+" failed.");
    if( _plg->isNullable(2) != 1 ) deb("---> Test "+GString(__FUNCTION__)+" failed.");
    if( _plg->isNullable(3) != 0 ) deb("---> Test "+GString(__FUNCTION__)+" failed.");

}

void readDefaultFromClob()
{
    deb("--- Running "+GString(__FUNCTION__));
    GSeq<COL_SPEC*> specSeq;
    COL_SPEC* pSpec;
    _plg->getColSpecs("\"TEST\".\"ID_XML\"", &specSeq);
    if( specSeq.elementAtPosition(2)->Default != "''") deb("---> Test readDefaultFromClob failed: Not ''");
    if( specSeq.elementAtPosition(4)->Default != "CURRENT TIMESTAMP") deb("---> Test readDefaultFromClob failed: Not CURRENT TIMESTAMP");
}

void writeLobs()
{
    deb("--- Running "+GString(__FUNCTION__));
    _plg->initAll("delete from test.fields");

    _plg->initAll("select * from test.fields");
    GSeq <GString> fileSeq;
    GSeq <long> lobTypeSeq;

    fileSeq.add("test.cpp");
    lobTypeSeq.add(_plg->sqlType(3));
    deb("sqlType for col 3: "+GString(_plg->sqlType(3)));
    GString cmd = "INSERT INTO TEST.FIELDS (NAME, FIELD) VALUES('clob', ?) ";
    int erc = _plg->uploadBlob(cmd, &fileSeq, &lobTypeSeq);
    if( erc ) deb("---> Test "+GString(__FUNCTION__)+" failed, erc (1): "+GString(erc)+": "+_plg->sqlError());

    fileSeq.removeAll();
    lobTypeSeq.removeAll();
    fileSeq.add("test.cpp");
    lobTypeSeq.add(_plg->sqlType(4));
    deb("sqlType for col 4: "+GString(_plg->sqlType(4)));
    cmd = "INSERT INTO TEST.FIELDS (NAME, RULE) VALUES('blob', ?) ";
    erc = _plg->uploadBlob(cmd, &fileSeq, &lobTypeSeq);
    if( erc ) deb("---> Test "+GString(__FUNCTION__)+" failed, erc (2): "+GString(erc)+": "+_plg->sqlError());

}

void readWriteMultiLob()
{
    GString fileName1 = "Fld1.txt";
    GString fileName2 = "Fld2.txt";

    remove(fileName1);
    remove(fileName2);
    deb("--- Running "+GString(__FUNCTION__));
    _plg->initAll("delete from test.multilob");

    _plg->initAll("select * from test.fields");
    GSeq <GString> fileSeq;
    GSeq <long> lobTypeSeq;

    fileSeq.add("./test.cpp");
    lobTypeSeq.add(_plg->sqlType(3));
    fileSeq.add("./test.pro");
    lobTypeSeq.add(_plg->sqlType(3));

    deb("sqlType for col 3: "+GString(_plg->sqlType(3)));
    GString cmd = "INSERT INTO TEST.MULTILOB (NAME, FIELD1, FIELD2) VALUES('clob', ?, ?) ";
    int erc = _plg->uploadBlob(cmd, &fileSeq, &lobTypeSeq);
    if( erc ) deb("---> Test "+GString(__FUNCTION__)+" failed, erc (1): "+GString(erc)+": "+_plg->sqlError());

    int outSize1, outSize2;
    _plg->descriptorToFile("Select FIELD1 from TEST.MULTILOB", fileName1,&outSize1);
    _plg->descriptorToFile("Select FIELD2 from TEST.MULTILOB", fileName2,&outSize2);

    if( outSize1 != fileSize("test.cpp"))deb("---> Test "+GString(__FUNCTION__)+" failed, sizeIn: "+GString(fileSize("test.cpp"))+", out: "+GString(outSize1));
    if( outSize2 != fileSize("test.pro"))deb("---> Test "+GString(__FUNCTION__)+" failed, sizeIn: "+GString(fileSize("test.pro"))+", out: "+GString(outSize2));
    remove(fileName1);
    remove(fileName2);
}

void readWriteDoubleDec()
{
    deb("--- Running "+GString(__FUNCTION__));
    _plg->initAll("delete from test.fields");
   if( _plg->getDBType() == DB2ODBC )

    _plg->initAll("delete from test.double");
    GString err = _plg->initAll("Insert into test.double (DBL,DECM) Values (123, 456)");
    if( err.length() )  deb("---> writeDoubleDec failed: "+err);
    _plg->initAll("Select dbl, decm from test.double");
    if( _plg->getDBType() == DB2ODBC )
    {
        if( _plg->rowElement(1,1) != "1.23000000000000E+002" ||_plg->rowElement(1,2) != "456.000") deb("---> Test (1)"+GString(__FUNCTION__)+" failed");
    }
    else if( _plg->rowElement(1,1) != "1.230000E+02" ||_plg->rowElement(1,2) != "456.000") deb("---> Test (2)"+GString(__FUNCTION__)+" failed");

}

int fileSize(GString fileName)
{
    struct stat st;
    if (stat(fileName, &st) == 0) return st.st_size;
    return -1;
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
