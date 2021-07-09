#include <stdio.h>

#include <gstring.hpp>
#include <dsqlobj.hpp>
#include <dsqlplugin.hpp>
#include <QtWidgets/QApplication>


void deb(GString function, GString txt);
void readXML();

DSQLPlugin * m_pIDSQL;

int main(int argc, char**argv)
{
    QApplication app(argc, argv);
    m_pIDSQL = new DSQLPlugin(_DB2);
    m_pIDSQL->connect("sample", "", "");


    if( rc ) return 1;
    m_pIDSQL->initAll("SELECT name from moi.staffg where id=110");
    //pDSQL->initAll("SELECT * FROM MOI.STAFFG ");
    for(int i = 1; i <= m_pIDSQL->numberOfRows(); ++i)
    {
        printf("Row: %i\n ", i);
        for( int j = 1; j <= m_pIDSQL->numberOfColumns(); ++j)
        {
            printf("  col: %i, data: %s\n ", j, (char*)pDSQL->rowElement(i,j));
        }
    }

    pDSQL->disconnect();
}

void readXML()
{
    GString cmd = "SELECT xml_col from moi.typetest where id=x'DA755D91A296B145A72D2317BB008DEB'";
    GString err = m_pIDSQL->initAll(cmd);
    deb(__FUNCTION__, "initErr: "+err);
    int outSize;
    err = m_pIDSQL->descriptorToFile(cmd, "SomeXML.xml", &outSize);
    deb(__FUNCTION__, "descToFile: "+err);
    deb(__FUNCTION__, "outSize: "+GString(outSize));

}

void deb(GString function, GString txt)
{
    txt.sayIt();
}
