#ifndef GSOCKET_HPP
#define GSOCKET_HPP


//************** Visual C++ ********************
#undef VCExport
#ifdef MAKE_VC
#define DllImport   __declspec( dllimport )
#define DllExport   __declspec( dllexport )
#define VCExport DllExport
#else
#define VCExport
#endif
//(************** Visual C++ END *****************

//************** Visual Age *********************
#ifdef MAKE_VA
#include <istring.hpp>
#ifdef MakeGSocket
#define GSocketExp_Imp _Export
#else
#define GSocketExp_Imp _Import
#pragma library( "GLEngine.LIB" )
#endif
#endif
//************** Visual Age END *****************

//Right. On Linux everything is fine, on Windows it's not.
//Windows.h must not be included before winsock2.h
//So we cannot include GString here.
//Do not read up on winsock and winsock2, it's ugly.
//class GString;
//SOLVED via ugly ifdefs in .cpp

class GString;

const int MAXHOSTNAME = 200;
const int MAXCONNECTIONS = 5;
const int MAXRECV = 12;

/*
typedef struct{
    GString Server;
    GString SrcFileUrl;
    GString OutFileName;
    int Port;
    int SetUserAgent;
    void init(){Server = SrcFileUrl = OutFileName = ""; Port = -1; SetUserAgent = 1;}
} HTTP_GET_STRUCT;
*/


#ifdef MakeGSocket
class GSocketExp_Imp GSocket
        #else
class GSocket
        #endif
{
public:
    GSocket();
    ~GSocket();

    int create();
    int bind ( int port );
    int listen();
    int accept ( GSocket& aSocket	);
    int id();
    void setID(int id);

    int connect ( GString host, int port );
    int oldConnect ( GString host, int port );

    int send ( GString msg );
    int sendRaw ( GString msg );
    int recv ( GString * msg );
    int recvRawText ( GString * msg );
    int httpRecvRawToFile ( GString srv, GString srcFile, GString fileName);
    //int httpRecvRawToFile ( HTTP_GET_STRUCT *pStruct);
    int handle();
    int downloadSize();
    void setStopFlag(int stop);
    GString ipFromHostName(GString hostname);

    void set_non_blocking ( bool val );

    int is_valid() { return m_Socket != -1; }
    void deb(GString msg);
    /*
  GSocket &operator = (const GSocket someSock)
                      {
                     m_Socket = someSock.m_Socket;
                  m_Addr = someSock.m_Addr;
                  m_iID = someSock.m_iID;
                  return *this;
            }
  */
private:
    int m_iID;
    int m_Socket;
    int m_iDownloadSize;
    int m_iStopFlag;

};


#endif

