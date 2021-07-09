#include <stdio.h>
#include <gstring.hpp>
#include <gstringlist.hpp>
#include <gstuff.hpp>
#include <gsocket.hpp>


#include <QString>

#include <sstream>


//#include "utf8.h"

#include <iostream>
#include <locale>
#include <clocale>
#include <cwchar>

#include <fcntl.h>
#ifndef _O_U16TEXT
  #define _O_U16TEXT 0x20000
#endif
#include <QApplication>
#include <QTextEdit>
#include <QTableWidget>
#include <QFile>
#include <QTextStream>

char consInput[1000];



char EncKey[] = "Abcdabcdabciabcdabcd";
void deb(char * msg);
GString in(GString msg);
int conv();
int hexTostr();
int h2s();
void printIt();
void testFile(GString filename);
int tbWdgt(int argv, char **args);
//char *get_utf8_input();
void opExtr1();
void opExtr2();
void remButOne();
void replaceChars();
void handlePwd();
void Xor_encrypt(char *key, char *input, int n);
void GXor_encrypt(GString key, GString &input, int n);
void GEncryptPwd(GString txt);
void GDecryptPwd(GString txt);
void EncryptPwd(char* txt);
void DecryptPwd(char* txt);
void TestGStringList();
void SplitIt();
void testBase64();

void printString(GString in);
int pwdTest(void) ;

void testGetTableName();

void testSocket();

#ifdef __WINDOWS__

void ReadActiveDirectory();
void ReadActiveDirectory2();
HRESULT ListMembersWithWinNtProvider();
HRESULT CheckUserGroups(IADsUser *pUser);
HRESULT PrintAllUsersInGlobalCatalog();
#endif

int main(int argv, char **args)
{
    //ListMembersWithWinNtProvider();
#ifdef __WINDOWS__
    ReadActiveDirectory();
    ReadActiveDirectory2();
#endif
    testBase64();
    return 0;

    testGetTableName();
    SplitIt();
    return 0;
    testSocket();
    return 0;
    SplitIt();

    remButOne();
    TestGStringList();
    return 0;
    //ReadActiveDirectory();
    //pwdTest();
    TestGStringList();
    handlePwd();
    return 0;
    remButOne();
    replaceChars();
    handlePwd();


    return 0;

    QApplication app(argv, args);
    opExtr1();
    opExtr2();
    testFile("systemsetting_utf8.txt");
	testFile("systemsetting_ansi.txt");
    tbWdgt(argv, args);
    return 0;

    QTextEdit textEdit;
    QString s;
    for(  short i = 192; i < 198; ++i)
    {
        printf("i: %i, c: %c\n", i, i);
        s += QChar((char)i);
    }
    QByteArray ba = s.toLocal8Bit();
    const char *c_str2 = ba.data();

    s += QChar('\xc4');
    s += QChar((int) 196);
    GString g = s;
    textEdit.setText(g);
    textEdit.show();


    //return app.exec();
    return 0;
}

void testGetTableName()
{
    GString cmd = "bla from MyTab";
    deb("testGetTableName: "+GStuff::TableNameFromStmt(cmd));
    cmd = "bla from MyTabSpace .  MyTab";
    deb("testGetTableName: "+GStuff::TableNameFromStmt(cmd));
    cmd = "bla from MyTabSpace.MyTab";
    deb("testGetTableName: "+GStuff::TableNameFromStmt(cmd));
    cmd = "bla from MyTabSpace . MyTab";
    deb("testGetTableName: "+GStuff::TableNameFromStmt(cmd));

    cmd = "bla from MyTab and some";
    deb("testGetTableName: "+GStuff::TableNameFromStmt(cmd));
    cmd = "bla from MyTabSpace . MyTab  and some";
    deb("testGetTableName: "+GStuff::TableNameFromStmt(cmd));
    cmd = "bla from MyTabSpace.MyTab and some";
    deb("testGetTableName: "+GStuff::TableNameFromStmt(cmd));
    cmd = "bla from MyTabSpace . MyTab and some";
    deb("testGetTableName: "+GStuff::TableNameFromStmt(cmd));

    cmd = "bla from \"MyTab1\" and some";
    deb("testGetTableName: "+GStuff::TableNameFromStmt(cmd));
    cmd = "bla from \"MyTabSpace2\" .   \"MyTab\"  and some";
    deb("testGetTableName: "+GStuff::TableNameFromStmt(cmd));
    cmd = "bla from \"MyTabSpace3.MyTab\" and some";
    deb("testGetTableName: "+GStuff::TableNameFromStmt(cmd));
    cmd = "bla from \"MyTabSpace4\" . MyTab and some";
    deb("testGetTableName: "+GStuff::TableNameFromStmt(cmd));

    cmd = "select * from TEST5 .  sometab";
    deb("testGetTableName: "+GStuff::TableNameFromStmt(cmd));


}

void testSocket()
{
    GString server = in("Server: ");
    int port = 80 ;
    GString tmp = in("Port: [80] ");
    if( tmp.length() > 0 ) port = tmp.asInt();
    GString verSite = "/";
    //GString server = "google.de";
    //GString server = "192.168.121.21";
    //int port = 80;


    GSocket sock;
    GString data, choice;
    sock.set_non_blocking(true);
    int rc;

    while(1)
    {
        deb("\nq: quit\nf: get file\ns: site\n");
        choice = in("q f s [s] ");
        if( choice == "q" ) break;
        if( choice == "f" )
        {
            tmp = in("FileLoc: [out will be ./sock.txt]");
            rc = sock.connect(server, port);
            deb("Error from connect: "+GString(rc));
            rc = sock.httpRecvRawToFile(server, tmp, "sock.txt");
            deb("Received "+GString(rc)+" bytes");
        }
        else
        {
            tmp = in("Site: [/] ");
            if( tmp == "q") break;
            if( tmp.length() ) verSite = tmp;
            else verSite = "/";
            verSite = verSite.stripLeading('/') + "/";
            rc = sock.connect(server, port);
            deb("Error from connect: "+GString(rc));
            rc = sock.sendRaw("GET "+verSite+" HTTP/1.0\r\n");
            deb("Rc send1: "+GString(rc));
            rc = sock.sendRaw("Host: "+server+":"+GString(port)+"\r\n");
            deb("Rc send2: "+GString(rc));
            rc = sock.sendRaw("\r\n"); //<- always close with \r\n, this signals end of http request
            deb("Rc send3: "+GString(rc));
            rc = sock.recvRawText(&data);
            deb("Rc recev: "+GString(rc));
            deb("Error from recRaw: "+GString(rc));
            deb("Data: "+data);
            deb("Size: "+GString(data.length())+" bytes [q to quit]");
        }

    }

}

void SplitIt()
{
    GString s= ";This,Is.To-Be;Splitted;Now;";
    GSeq<GString> res = s.split(".;,-");
    for(int i = 1; i <= res.numberOfElements(); ++i)
    {
        printf("r: %i, txt: %s\n", i, (char*) res.elementAtPosition(i));
    }

    s = "1@2@3@@@A@|A|B||";

    res = s.split("@|");
    for(int i = 1; i <= res.numberOfElements(); ++i)
    {
        printf("r: %i, txt: %s\n", i, (char*) res.elementAtPosition(i));
    }

    s = "1@2@3@@@A@";
    res = s.split('@');
    for(int i = 1; i <= res.numberOfElements(); ++i)
    {
        printf("r: %i, txt: %s\n", i, (char*) res.elementAtPosition(i));
    }

}

int pwdTest(void)
{
  char plain[] = "This is plain text";
  char key[] = "Abcdabcdabciabcdabcd";
  int n = strlen(plain);
  // encrypt:
  Xor_encrypt(key, plain, n);
  printf("encrypted string: \n");
  for(int ii = 0; ii < n; ii++) {
      if(plain[ii] > 0x32 && plain[ii] < 0x7F ) printf("PT: %i: %c\n", ii, plain[ii]);
      else printf("PT: %i: 0x%02x\n", ii, plain[ii]);
  }
  printf("\n");
  // **** if you include this next line, things go wrong!
 // n = strlen(plain);
  Xor_encrypt(key, plain, n);
  printf("after round trip, plain string is '%s'\n", plain);
  return 0;
}
void printString(GString in)
{
    for(int i = 1; i <= in.length(); ++i)
    {
        printf("->%c\n", in[i]);
    }
}

void Xor_encrypt(char *key, char *input, int n)
{
    int i;
    int keyLength = strlen(key);
    for( i = 0 ; i < n ; i++ )
    {
        input[i]=input[i]^key[i%keyLength];
    }
    for(int ii = 0; ii < n; ii++)
    {
        if(input[ii] > 0x32 && input[ii] < 0x7F ) printf("XOE: %i: %c\n", ii, input[ii]);
        else printf("XOE: %i: 0x%02x\n", ii, input[ii]);
    }
}

void GXor_encrypt(GString key, GString &input, int n)
{
    int i;
    int keyLength = strlen(key);
    for( i = 0 ; i < n ; i++ )
    {
        input[i]=input[i]^key[i%keyLength];
    }
    for(int ii = 0; ii < n; ii++)
    {
        if(input[ii] > 0x32 && input[ii] < 0x7F ) printf("XOE: %i: %c\n", ii, input[ii]);
        else printf("XOE: %i: 0x%02x\n", ii, input[ii]);
    }
}

void GEncryptPwd(GString txt)
{
    Xor_encrypt(EncKey, txt, txt.length());

    for(int ii = 0; ii < txt.length(); ii++)
    {
        if(txt[ii] > 0x32 && txt[ii] < 0x7F ) printf("GEN: %i: %c\n", ii, txt[ii]);
        else printf("GEN: %i: 0x%02x\n", ii, txt[ii]);
    }
    txt = GString(txt.length())+":"+txt;

}
void GDecryptPwd(GString txt)
{
}

void EncryptPwd(char* txt)
{

    int n = strlen(txt);
    printf("n: %i\n", n);
    Xor_encrypt(EncKey, txt, n);
    char* buf = new char[n+4];
    txt = (char *) realloc(txt, 18);
    printf( "%03d:%s", n, txt);
    sprintf(txt, "%03d:%s", n, txt);

}

void DecryptPwd(char* txt)
{
    //int len = GString(txt).subString(1, 3).asInt();
    int len = 18;
    Xor_encrypt(EncKey, txt, len);
}

void TestGStringList()
{
    GStringList gl;
    gl.add("Aber dies ist ein String");
    gl.count();
    GString s = gl.toString(", ");
    s.sayIt();

    GStringList gl2;
    gl2.add("Aber: dies: ist: ein: String", ": ");
    gl2.count();
    s = gl2.toString("-");
    s.sayIt();

    GStringList gl3;
    gl2.add("CRLFSTart\nAber\ndies\nist\nein\String\nmitCRLF", "\n");
    gl2.count();
    s = gl2.toString(" ");
    s.sayIt();

}

void handlePwd()
{
    GString pwd = "This is plain text";
    GEncryptPwd(pwd);
    GDecryptPwd(pwd);
    printf("out: %s\n", (char*)pwd);
}

void replaceChars()
{
    GString txt = "1234567890";
    txt.sayIt();
    txt[1] = 'A';
    txt[3] = 'B';
    txt[5] = 'C';
    txt[7] = 'D';
    txt[9] = 'E';
    txt.sayIt();

}

void remButOne()
{
    GString x = "   123   456  ";
    x = x.removeButOne();
    deb("x1: "+x+"<-");

    x = "123   456  ";
    x = x.removeButOne();
    deb("x2: "+x+"<-");

    x = "123   456";
    x = x.removeButOne();
    deb("x3: "+x+"<-");

    x = "OneTab         NoThree         andAgain:                               end";
    x = x.change("\t", " ");
    x = x.removeButOne();

    deb("x3: "+x+"<-");

}

void opExtr1()
{
    deb("opExtr1 start");
    GString x = "Hello my World ABCXYZA12345678901234687878421 ";
    std::stringstream is; //, x1, x2, x3, x4;
    is << x;
    GString x1, x2, x3, x4;
    x >> x1 >> x2 >> x3 >> x4;
    deb("x1: "+x1+", x2: "+x2+", x3: "+x3+", x4: "+x4);
}

void opExtr2()
{
    GString x = "HALLO 1 2 3";
    std::cout << x;
}

void testFile(GString filename)
{	
	printf("Testing %s\n", (char*) filename);
    GString path = GString(filename);
    QFile f(path);
    if (!f.open(QFile::ReadOnly | QFile::Text)) return;
    QTextStream in(&f);
    while(!in.atEnd())
    {   
        GString line = in.readAll();
        if(line.isProbablyUtf8())
        {
            printf("line %s isUtf8\n", (char*) line);
        }
        else printf("line %s is not Utf8\n", (char*) line);
    }
    f.close();    
}
int tbWdgt(int argv, char **args)
{

    QApplication app(argv, args);
    QTableWidget wdgt;
    wdgt.setColumnCount(4);
    wdgt.setRowCount(1);
    wdgt.resize(500, 100);
    QTableWidgetItem qitem, gitem, gitem2, gitem3;
    GString s = "Ä-";
    QString q = "Ä-";


    for(  short i = 192; i < 198; ++i)
    {
        ///printf("i: %i, c: %c\n", i, i);
        q += QChar((char)i);
        s += GString((char(i)));
    }
    q += QChar('\xc4');
    q += QChar((int) 196);


    qitem.setData(Qt::DisplayRole, q);
    gitem.setData(Qt::DisplayRole, QString::fromLatin1(s));
    gitem2.setData(Qt::DisplayRole, QString::fromLocal8Bit(s));
    gitem3.setData(Qt::DisplayRole, QString("ÄÖÜ"));

    //gitem2.setData(Qt::DisplayRole, QStringLiteral(s));

    wdgt.setItem(0,0, &qitem);
    wdgt.setItem(0,1, &gitem);
    wdgt.setItem(0,2, &gitem2);
    wdgt.setItem(0,3, &gitem3);
    wdgt.show();
    app.exec();
    GString data = wdgt.item(0,3)->text();

    data = data.wideUpperCase();

return 0;

    setlocale(LC_ALL, "");
    int i = 0;
    wchar_t c;

    wchar_t str[] = L"Test Stringäöü.\n";

    while (str[i])
    {
      c = str[i];
      putwchar (towupper(c));
      str[i] = (towupper(c));
      i++;
    }
    fputws(str, stdout);
//    i = 0;
//    size_t cSize = mbstowcs(NULL, ptr, 0) + 1;
//    printf("size: %i\n", cSize);
//    //std::wstring wc( cSize, L'#' );
//    wchar_t *wc = new wchar_t[cSize];
//    mbstowcs( wc, ptr, cSize );
//    printf("start while\n");
//    while (wc[i])
//    {
//        printf("in while: %i\n", i);
//      c = wc[i];
//      putwchar (towupper(c));
//      wc[i] = (towupper(c));
//      i++;
//    }
//    fputws(wc, stdout);
//    delete [] wc;
//    return "";



    return 0;
}









void printIt()
{
    QString s;
    for(  short i = 190; i <= 196; ++i)
    {
        printf("i: %i, c: %c\n", i, i);
        s += QChar((char)i);
    }
    char c = 'Ä';
    printf("Ä as number: %i\n", (unsigned char)c);
      QByteArray ba = s.toLatin1();
      const char *c_str2 = ba.data();
      printf("str2: %s\n", c_str2);

}

void deb(char * msg)
{

    printf("Test> %s\n", msg);
    #ifdef MAKE_VC
    flushall();
    #endif
}
int h2s()
{
/*
    wchar_t dest[6];
    mbstowcs (dest, u8"\xC4", 6);
    printf("Dest: %x\n", dest);



    //char narrow_str[] = "z\u00df\u6c34\U0001f34c";
    //char narrow_str[] = "\x53\x70\xc3\x9f\xe6\xb0\xb4\xf0\x9f\x8d\x8c"; //"\x53\xC4";
                    // or "\x7a\xc3\x9f\xe6\xb0\xb4\xf0\x9f\x8d\x8c";
    char narrow_str[] = "\x53\x70\xc4";
    wchar_t warr[29]; // the expected string is 28 characters plus 1 null terminator
    std::setlocale(LC_ALL, "de_DE.utf8");

    std::swprintf(warr, sizeof warr/sizeof *warr,
                  L"Converted from UTF-8: '%s'", narrow_str);

    std::wcout.imbue(std::locale("en_US.utf8"));
    std::wcout << warr << '\n';
*/
    return 0;

    return 0;
    const char *src = "530061006E006400650072007300C400";
    int number = (int)strtol("C4", NULL, 16);
    GString out = "530061006E006400650072007300C400";
    for(int i = 1; i <= out.length(); ++i)
    {
        //number = out
    }


    printf("Number: %i\n", number);

    char c = 'Ä';
    printf("Ä as number: %i\n", (unsigned char)c);

       char buffer[18];
       char *dst = buffer;
       char *end = buffer + sizeof(buffer);
       unsigned int u;

       while (dst < end && sscanf(src, "%2x", &u) == 1)
       {
           *dst++ = u;
           src += 2;
       }

       for (dst = buffer; dst < end; dst+=2)
           printf("%d: %c (%d, 0x%02x)\n", dst - buffer,(isprint(*dst) ? *dst : '.'), *dst, *dst);
     //if( iEmbrace ) out = "'" + out + "'";

}

int hexTostr()
{
    const char *src = "530061006E006400650072007300C400";
       char buffer[18];
       char *dst = buffer;
       char *end = buffer + sizeof(buffer);
       unsigned int u;

       while (dst < end && sscanf(src, "%2x", &u) == 1)
       {
           *dst++ = u;
           src += 2;
       }

       for (dst = buffer; dst < end; dst+=2)
           printf("%d: %c (%d, 0x%02x)\n", dst - buffer,(isprint(*dst) ? *dst : '.'), *dst, *dst);
       return 0;
}

GString in(GString msg)
{
    char buffer[100];   /* the string is stored through pointer to this buffer */
    if( msg.length() ) printf("%s", msg);
    else printf("=> ");
    fflush(stdout);
    fgets(buffer, 100, stdin); /* buffer is sent as a pointer to fgets */
    return buffer;
}

//GString in(GString msg)
//{
//    if( msg.length() ) printf("%s", msg);
//    else printf("=> ");
//    gets(consInput);
//    return GString(consInput).strip();
//}

int conv()
{
    char src[] = "C4";
    char dst[100];
    size_t srclen = 6;
    size_t dstlen = 12;
    int rc;

    fprintf(stderr,"in: %s\n",src);

    char * pIn = src;
    char * pOut = ( char*)dst;
/*
    iconv_t conv = iconv_open("UTF-8","UTF-16");
    rc = iconv(conv, &pIn, &srclen, &pOut, &dstlen);
    fprintf(stderr,"rc: %i\n",rc);
    iconv_close(conv);
*/
    fprintf(stderr,"out: %s\n",dst);
    return 0;
}

void testBase64()
{
    GString in ="TGljZW5zZToKUE1GIGlzIHJlbGVhc2VkIHVuZGVyIHRoZSBHUEwuIApBIGNvcHkgb2YgdGhlIEdQTCBjYW4gYmUgZm91bmQgaW4gZ3BsLnR4dCBpbiB0aGlzIGRpcmVjdG9yeS4KCi0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0KLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLQotLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tCgpUaGVzZSBhcmUgdmVyeSBzaG9ydCBpbnN0cnVjdGlvbnMgZm9yIGJ1aWxkaW5nIFBNRi4KQSBtb3JlIGRldGFpbGVkIHZlcnNpb24gaXMgYXZhaWxhYmxlIGF0IHd3dy5sZWlwZWx0Lm5ldAoKUE1GIGNhbiBhY2Nlc3MgREIyIGFuZCBTUUwgU2VydmVyLiBZb3UgY2FuIGJ1aWxkIFBNRiB3aXRoIHRoZSAKcGx1Z2lucyBmb3IgREIyIG9yIHRoZSBwbHVnaW4gZm9yIFNRTCBTZXJ2ZXIgb3IgYWxsIHBsdWdpbnMuCgpEZXBlbmRpbmcgb24gd2hhdCB5b3UgZGVjaWRlIHRvIGJ1aWxkLCB0aGUgcmVxdWlyZW1lbnRzIGRpZmZlci4KCgpQYXJ0IDE6IEJ1aWxkaW5nIFBNRiBvbiBMaW51eApQYXJ0IDI6IEJ1aWxkaW5nIFBNRiBvbiBXaW5kb3dzCgotLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tCi0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0KLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLQoKCjEuIEJ1aWxkaW5nIFBNRiBvbiBMaW51eAoKUmVxdWlyZW1lbnRzOgotIFF0NCBvciBRdDUgZGV2ZWxvcG1lbnQgcGFja2FnZXMKLSBtYWtlICh1c3VhbGx5IGluIGF1dG9tYWtlKQotIGcrKyBjb21waWxlcgotIGZvciBEQjI6ICJBcHBsaWNhdGlvbiBkZXZlbG9wbWVudCB0b29scyIgZnJvbSBkYjJzZXR1cAotIGEgd29ya2luZyBkYXRhYmFzZSBjb25uZWN0aW9uCi0gZm9yIFNRTCBTZXJ2ZXI6IEZyZWVURFMsIHVuaXhPREJDIChvciB1bml4T0RCQzIpIGFuZCBVbml4T0RCQ1syXS1kZXYKClVucGFjayBwbWYudGFyLmd6IGludG8gYSBzdWl0YWJsZSBkaXJlY3Rvcnk6CiQgdGFyIC14emYgcG1mLnRhci5negotLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tCjEuMSBCdWlsZHVuZyBQTUYgd2l0aCBib3RoIERCMiBwbHVnaW5zCi0gSW5zdGFsbCBEQjIgIkFwcGxpY2F0aW9uIGRldmVsb3BtZW50IHRvb2xzIjogaW4gZGIyU2V0dXAsIAogICAgY2hvb3NlICJDdXN0b20gaW5zdGFsbCIgYW5kIHNlbGVjdCAiQXBwbGljYXRpb24gZGV2ZWxvcG1lbnQgdG9vbHMiICAgCi0gWW91IG5lZWQgYXQgbGVhc3Qgb25lIHZhbGlkIGRhdGFiYXNlIGNvbm5lY3Rpb24sIAogICAgZm9yIGV4YW1wbGUgdGhlICdzYW1wbGUnIGRhdGFiYXNlLiBTZXQgdGhpcyBpbiAiZGI9Li4uIgotIElmIHRoZSBkYXRhYmFzZSBpcyBub3QgbG9jYWwsIHlvdSBuZWVkIHRvIHN1cHBseSBhIHVzZXJuYW1lICh1aWQpIAogICAgYW5kIHBhc3N3b3JkIChwd2QpCgpJbiBhIHNoZWxsOgokIHFtYWtlIHBtZjUucHJvICJ0YXJnZXQ9ZGIyIiAiZGI9ZGF0YWJhc2VOYW1lIiAKLSBvciAtCiQgcW1ha2UgcG1mNS5wcm8gInRhcmdldD1kYjIiICJkYj1kYXRhYmFzZU5hbWUiICJ1aWQ9dXNlck5hbWUiIAotIG9yIC0KJCBxbWFrZSBwbWY1LnBybyAidGFyZ2V0PWRiMiIgImRiPWRhdGFiYXNlTmFtZSIgInVpZD11c2VyTmFtZSIgInB3ZD1wYXNzd29yZCIKClRoaXMgc2hvdWxkIGdlbmVyYXRlIGEgTWFrZWZpbGUuIE5leHQsIHJ1bgokIG1ha2UKCklmIHlvdSB3YW50IHRvIGJ1aWxkIG9ubHkgdGhlIE9EQkMgcGx1Z2luLCBydW4KJCBxbWFrZSBwbWY1LnBybyAidGFyZ2V0PWRiMm9kYmMiIAokIG1ha2UKCklmIHlvdSB3YW50IHRvIGJ1aWxkIG9ubHkgdGhlIHBsdWdpbiBmb3IgdGhlIG5hdGl2ZSBjbGllbnQgKENBRSksIHJ1bgokIHFtYWtlIHBtZjUucHJvICJ0YXJnZXQ9ZGIyY2FlIiAiZGI9PWRhdGFiYXNlTmFtZSIgInVpZD11c2VyTmFtZSIgInB3ZD1wYXNzd29yZCIKJCBtYWtlCgoqKiBBbHdheXMgcnVuICdtYWtlIGRpc3RjbGVhbicgb3IgJ25tYWtlIGRpc3RjbGVhbicgYmVmb3JlIHJlLXJ1bm5pbmcgcW1ha2UKCioqKiBOb3RlczogCklmIHlvdSBzdXBwbHkgb25seSBhIHVpZCB3aXRob3V0IHB3ZCwgeW91IHdpbGwgYmUgYXNrZWQgZm9yIHRoZSBwd2QKZHVyaW5nIHRoZSBidWlsZCBwcm9jZXNzLiBUaGUgcHdkIHdpbGwgKm5vdCogYmUgc3RvcmVkLgoKSWYgeW91IHN1cHBseSB1aWQgYW5kIHB3ZCwgdGhleSB3aWxsIGJlIHN0b3JlZCBpbiB0aGUgTWFrZWZpbGUKVG8gd29yayBhcm91bmQgdGhpcywgeW91IGNvdWxkOgotIFVzZSBhIGxvY2FsIGRhdGFiYXNlIChmb3IgZXhhbXBsZSBEQjIncyAnc2FtcGxlJyBkYXRhYmFzZSkgdG8gY29ubmVjdCAKICB3aXRob3V0IHVpZC9wd2QKLSBTZXQgdWlkIGJ1dCBub3QgcHdkOiBZb3Ugd2lsbCBiZSBhc2tlZCBmb3IgdGhlIHB3ZAogIGR1cmluZyB0aGUgYnVpbGQgcHJvY2Vzcy4gSW4gdGhpcyBjYXNlLCB0aGUgcHdkIHdpbGwgKm5vdCogYmUgc3RvcmVkLgotIFJlbW92ZSB0aGUgTWFrZWZpbGUgYWZ0ZXIgdGhlIGJ1aWxkLiBUbyByZS1nZW5lcmF0ZSB0aGUgTWFrZmlsZSwgcmUtcnVuIHFtYWtlLgoKCi0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0KMS4yIEJ1aWxkdW5nIFBNRiB3aXRoIHRoZSBTUUwgU2VydmVyIHBsdWdpbgpJbiBhIHNoZWxsOgokIHFtYWtlIHBtZjUucHJvICJ0YXJnZXQ9c3Fsc3J2IgokIG1ha2UKCioqKiBBbHdheXMgcnVuICdtYWtlIGRpc3RjbGVhbicgb3IgJ25tYWtlIGRpc3RjbGVhbicgYmVmb3JlIHJlLXJ1bm5pbmcgcW1ha2UKCi0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0KMS4zIEJ1aWxkdW5nIFBNRiB3aXRoIGJvdGggcGx1Z2lucyAoU1FMIFNlcnZlciBhbmQgREIyKQpUaGUgc2FtZSBwcm9jZWR1cmUgYXMgaW4gMS4xLCBzaW1wbHkgcmVwbGFjZSAKInRhcmdldD1kYjIiIHdpdGggInRhcmdldD1hbGwiLgoKUnVuIG9uZSBvZiB0aGVzZSBjb21tYW5kczoKJCBxbWFrZSBwbWY1LnBybyAidGFyZ2V0PWFsbCIgImRiPWRhdGFiYXNlTmFtZSIgCi0gb3IgLQokIHFtYWtlIHBtZjUucHJvICJ0YXJnZXQ9YWxsIiAiZGI9ZGF0YWJhc2VOYW1lIiAidWlkPXVzZXJOYW1lIiAKLSBvciAtCiQgcW1ha2UgcG1mNS5wcm8gInRhcmdldD1hbGwiICJkYj1kYXRhYmFzZU5hbWUiICJ1aWQ9dXNlck5hbWUiICJwd2Q9cGFzc3dvcmQiCgpUaGlzIHNob3VsZCBnZW5lcmF0ZSBhIE1ha2VmaWxlLiBOZXh0LCBydW4KJCBtYWtlCgoKRm9yIG1vcmUgaW5mbywgc2VlIE5vdGVzIGluIDEuMQoKKioqIEFsd2F5cyBydW4gJ21ha2UgZGlzdGNsZWFuJyBvciAnbm1ha2UgZGlzdGxlYW4nIGJlZm9yZSByZS1ydW5uaW5nIHFtYWtlCgotLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tCllvdSBzaG91bGQgbm93IGhhdmUgdGhlIGV4Y3V0YWJsZSAoLi9wbWYpIGJ1aWx0LgpJZiB5b3UgZ2V0IHN0dWNrLCBkcm9wIG1lIGEgbWFpbCBhbmQgSSdsbCB0cnkgdG8gaGVscC4KClNlZSB3d3cubGVpcGVsdC5uZXQgZm9yIGluc3RydWN0aW9ucyBvbiBob3cgdG8gc2V0IHVwIFNRTCBTZXJ2ZXIgCnRvIGFjY2VwdCBjb25uZWN0aW9ucyBmcm9tIExpbnV4LgoKCgotLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tCi0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0KLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLQoyLiBCdWlsZGluZyBQTUYgb24gV2luZG93cwoKUmVxdWlyZW1lbnRzOgotIENvbXBpbGVyOiBNUyBWUzIwMTAgb3IgaGlnaGVyIG9yIE1pbkdXCi0gUXQ1OiBDaG9vc2UgdGhlIHZlcnNpb24gZm9yIHlvdXIgY29tcGlsZXIKLSBmb3IgREIyOiAiQXBwbGljYXRpb24gZGV2ZWxvcG1lbnQgdG9vbHMiIGZyb20gZGIyc2V0dXAKLSBmb3IgU1FMIFNlcnZlcjogIlNRTCBTZXJ2ZXIgZGV2ZWxvcG1lbnQgdG9vbHMiLiBQcm9iYWJseSAiTWljcm9zb2Z0IERhdGEgQWNjZXNzIFNESyIgaXMgc3VmZmljaWVudCwgSSBoYXZlbid0IHRyaWVkIHRoYXQuCgpJJ20gY3VycmVudGx5IHVzaW5nIE1TIFZpc3VhbCBTdHVkaW8gMjAxMCBFeHByZXNzIEVkaXRpb24uIEl0IHNob3VsZCBiZSBwb3NzaWJsZSB0byB1c2UgbGF0ZXIgdmVyc2lvbnMsIG9yIE1pbkdXLCBidXQgSSBoYXZlbid0IHRyaWVkIGl0LgpOb3RlIG9uICBNaW5HVzogRG8gKm5vdCogdXNlIE1pbkdXIHdoZW4gbGlua2luZyBhZ2FpbnN0IERCMiB2NyBsaWJzLgoKLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLQoyLjEgQnVpbGR1bmcgUE1GIHdpdGggdGhlIERCMiBwbHVnaW4gCi0gWW91IG5lZWQgYXQgbGVhc3Qgb25lIHZhbGlkIERCMiBkYXRhYmFzZSBjb25uZWN0aW9uCi0gSW5zdGFsbCBEQjIgIkFwcGxpY2F0aW9uIGRldmVsb3BtZW50IHRvb2xzIjogaW4gZGIyU2V0dXAsIGNob29zZSAiQ3VzdG9tIGluc3RhbGwiIGFuZCBzZWxlY3QgIkFwcGxpY2F0aW9uIGRldmVsb3BtZW50IHRvb2xzIgotIFByb2JhYmx5IGEgYmF0Y2ggZmlsZSB0byBzZXQgZW52aXJvbm1lbnQgdmFyaWFibGVzIGZvciBRdCwgY29tcGlsZXIgYW5kIERCMiAoc2VlIGJlbG93KQoKVW5wYWNrIHBtZi56aXAgaW50byBhIHN1aXRhYmxlIGRpcmVjdG9yeS4KT3BlbiBhIERCMiBDTFAgKENvbW1hbmQgTGluZSBQcm9jZXNzb3IpIHdpbmRvdyAodmlhIFN0YXJ0LT5ydW4tPiJkYjJjbWQiKSBhbmQgbmF2aWdhdGUgdG8gdGhlIHBsYWNlIHdoZXJlIHlvdSB1bnBhY2tlZCBQTUYuCllvdSBwcm9iYWJseSBuZWVkIHRvIHNldCBlbnZpcm9ubWVudCB2YXJpYWJsZXMgbm93OgpDb250YWluZWQgaW4gcG1mLnppcCBpcyAncG1mX01TVkMuYmF0JywgZWRpdCBpdCB0byBmaXQgeW91ciBlbnZpcm9ubWVudC4KCgpXaGVuIHRoZSBlbnZpcm9ubWVudCB2YXJpYWJsZXMgYXJlIHNldCwgcnVuIG9uZSBvZiB0aGVzZSBjb21tYW5kcyBpbiB0aGUgREIyIENMUDoKICBxbWFrZSBwbWY1LnBybyAidGFyZ2V0PWFsbCIgImRiPWRhdGFiYXNlTmFtZSIgCi0gb3IgLQogIHFtYWtlIHBtZjUucHJvICJ0YXJnZXQ9YWxsIiAiZGI9ZGF0YWJhc2VOYW1lIiAidWlkPXVzZXJOYW1lIiAKLSBvciAtCiAgcW1ha2UgcG1mNS5wcm8gInRhcmdldD1hbGwiICJkYj1kYXRhYmFzZU5hbWUiICJ1aWQ9dXNlck5hbWUiICJwd2Q9cGFzc3dvcmQiCgp3aGVyZSAidXNlck5hbWUiIGFuZCAicGFzc3dvcmQiIGFyZSB2YWxpZCBmb3IgREIyLgoKRm9yIGFuIGV4cGxhbmF0aW9uIG9mICJkYiIsICJ1aWQiLCAicHdkIiBzZWUgTm90ZXMgaW4gMS4xCgpOZXh0LCBydW4gCiAgbm1ha2Ugb3IgbWFrZSBmb3IgTWluR1cKICAKCiAKLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLQoKMi4yIEJ1aWxkaW5nIFBNRiBmb3IgYm90aCBEQjIgYW5kIFNRTCBTZXJ2ZXI6CgpTYW1lIGFzIGFib3ZlLCBzaW1wbHkgcmVwbGFjZSAidGFyZ2V0PWRiMiIgd2l0aCAidGFyZ2V0PWFsbCIKCi0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0KCjIuMyBCdWlsZGluZyBQTUYgd2l0aCB0aGUgU1FMIFNlcnZlciBwbHVnaW4KCllvdSBtYXkgbmVlZCBhIGJhdGNoIGZpbGUgdG8gc2V0IGVudmlyb25tZW50IHZhcmlhYmxlcyBmb3IgUXQsIGNvbXBpbGVyIGFuZCBEQjIgKHNlZSBhYm92ZSkKClVucGFjayBwbWYuemlwIGludG8gYSBzdWl0YWJsZSBkaXJlY3RvcnkuCk9wZW4gY29tbWFuZCB3aW5kb3cgYW5kIG5hdmlnYXRlIHRvIHRoZSBwbGFjZSB3aGVyZSB5b3UgdW5wYWNrZWQgUE1GLgpZb3UgcHJvYmFibHkgbmVlZCB0byBzZXQgZW52aXJvbm1lbnQgdmFyaWFibGVzIG5vdy4gQ29udGFpbmVkIGluIHBtZi56aXAgaXMgJ3BtZl9NU1ZDLmJhdCcsIGVkaXQgaXQgdG8gZml0IHlvdXIgZW52aXJvbm1lbnQuCgpXaGVuIHRoZSBlbnZpcm9ubWVudCB2YXJpYWJsZXMgYXJlIHNldCwgcnVuIHRoZXNlIGNvbW1hbmRzIGluIHRoZSBjb21tYW5kIHdpbmRvdzoKICBxbWFrZSBwbWY1LnBybyAidGFyZ2V0PWRzcWxzcnYiCiAgbm1ha2UKKG9yICdtYWtlJyBmb3IgTWluR1cpCi0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0tLS0gIAogIE";
    GString out = in.fromBase64();
    printf("OUT: %s\ņ", (char*) out);
}


void Read3()
{

}
