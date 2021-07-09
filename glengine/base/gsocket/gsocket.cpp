// Implementation of the Socket class.

/*
#ifdef MAKE_VC
    #ifndef _WINSOCK_WRAPPER_H_
    #define _WINSOCK_WRAPPER_H_
     #if _MSC_VER > 1000
    #pragma once
    #endif
    #ifndef _WINDOWS_
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #undef WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
    #endif
#endif
*/


#ifdef _MSC_VER
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <string>
#include <io.h>
#include <errno.h>
#elif 	__MINGW32__
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <string>
#include <io.h>
#include <errno.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif

 
#include <gstring.hpp>
#include <gstuff.hpp>
#include <gsocket.hpp>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>

#define bufSize  1434


#include "gstring.hpp"
#include "gdebug.hpp"

//Moved this here  from .hpp
sockaddr_in m_Addr;

GSocket::GSocket() :  m_Socket ( -1 )
{
	deb("GSocket ctor");
    m_iDownloadSize = -1;
    m_iStopFlag = 0;
	memset ( &m_Addr, 0, sizeof ( m_Addr ) );
	deb("GSocket ctor ok");
}

GSocket::~GSocket()
{
  deb("Closing socket...");
  #ifdef MAKE_VC
  if ( is_valid() )closesocket(m_Socket ); 
  WSACleanup();
  #else
  if ( is_valid() ) close ( m_Socket );
  #endif
  deb("Closing socket OK.");
}
void GSocket::setID(int id)
{
   m_iID = id;
}

int GSocket::id()
{
   return m_iID;
}
int GSocket::create()
{
  deb("::create");
  m_iStopFlag = 0;
  m_Socket = socket ( AF_INET, SOCK_STREAM, 0 );


  if ( ! is_valid() ) return 1;


  int on = 1;
  if ( setsockopt ( m_Socket, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on, sizeof ( on ) ) == -1 )
    return 1;

  struct timeval timeout;
  timeout.tv_sec = 3;
  timeout.tv_usec = 0;

  if (setsockopt (m_Socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) return 2;
  if (setsockopt (m_Socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,  sizeof(timeout)) < 0) return 3;
  return 0;

}



int GSocket::bind ( int port )
{

  if ( ! is_valid() ) return 1;

  m_Addr.sin_family = AF_INET;
  m_Addr.sin_addr.s_addr = INADDR_ANY;
  m_Addr.sin_port = htons ( port );

  int erc = ::bind( m_Socket, ( struct sockaddr * ) &m_Addr, sizeof ( m_Addr ) );
  if ( erc == -1 ) return 2;
  return 0;
}


int GSocket::listen()
{
  if ( ! is_valid() ) return 1;

  int erc = ::listen ( m_Socket, MAXCONNECTIONS );

  if (erc == -1 ) return 2;
  return m_Socket;
}


int GSocket::accept ( GSocket& aSocket ) 
{
  int addr_length = sizeof ( m_Addr );
  aSocket.m_Socket = ::accept ( m_Socket, ( sockaddr * ) &m_Addr, ( socklen_t * ) &addr_length );

  if ( aSocket.m_Socket <= 0 ) return 1;
  return 0;
}


int GSocket::send ( GString msg )
{
    m_iStopFlag = 0;
    GString out = GString(msg.length()+12).rightJustify(12, '0') + msg;
#ifdef MAKE_VC
    int status = ::send ( m_Socket, out, out.length(), 0 );
    deb("send err: "+GString(WSAGetLastError()));
#elif 	__MINGW32__
    int status = ::send ( m_Socket, out, out.length(), 0 );
    deb("send err: "+GString(WSAGetLastError()));
#else
    int status = ::send ( m_Socket, out, out.length(), MSG_NOSIGNAL );
#endif
    deb("Send, status: "+GString(status));
    if ( status == -1 ) return 1;

    return 0;
}
int GSocket::sendRaw ( GString msg )
{
    GString x;
    m_iStopFlag = 0;
#ifdef MAKE_VC
    int status = ::send ( m_Socket, msg, msg.length(), 0 );

#else
    int status = ::send ( m_Socket, msg, msg.length(), 0 );//MSG_NOSIGNAL
#endif
    deb("SendRaw, status: "+GString(status));
    if ( status == -1 ) return 1;

    return 0;
}

void GSocket::setStopFlag(int stop)
{
    m_iStopFlag = stop;
}

/*
int GSocket::httpRecvRawToFile( HTTP_GET_STRUCT *pStruct )
{
    m_iStopFlag = 0;
    m_iDownloadSize = 0;

    GString srcFileUrl = pStruct->SrcFileUrl;
    if( srcFileUrl.strip().length() < 1) return -99;

    GString outFile;
    if( !pStruct->OutFileName.length() )
    {
        if( srcFileUrl.occurrencesOf("/") ) outFile = srcFileUrl.subString(srcFileUrl.lastIndexOf("/")+1, srcFileUrl.length()).strip();
        else outFile = srcFileUrl;
    }
    srcFileUrl = "/"+srcFileUrl.stripLeading('/');
    if( pStruct->Port < 0 )
    {
        if( pStruct->Server.occurrencesOf("https") ) pStruct->Port = 443;
        else pStruct->Port = 80;
    }


    m_iDownloadSize = -1;
    char buf [ bufSize ];


    deb("RecRawToFile start");
    FILE * pFile;
    pFile = fopen (outFile, "wb");
    if( pFile == NULL )
    {
        deb("RecRawToFile cannot open trgFile");
        return -1;
    }


    set_non_blocking(true);
    int rc = connect(pStruct->Server, pStruct->Port);
    deb("RecRawToFile rc connect: "+GString(rc));
    GString httpRequest;
    if( !rc )
    {
        httpRequest = "GET "+srcFileUrl+" HTTP/1.0\n";
        httpRequest += "Host: "+pStruct->Server+":"+GString(pStruct->Port)+"\n";
        if( pStruct->SetUserAgent ) httpRequest += "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/53.0.2785.143 Safari/537.36\n";
        rc = sendRaw(httpRequest+"\r\n"); //<- always close with \r\n, this signals end of http request
    }
    if( rc ) return -1;

    int lng, loops = 0, fileSize = 0;
    int headerSize = 252; //Usually. Is overwritten below.
    deb("RECV LNG  Start LOOP ");
    while( !m_iStopFlag )
    {
       memset ( buf, 0, bufSize );
       deb("RECV LNG(loop), calling recv");
       lng = ::recv ( m_Socket, buf, bufSize, 0 );
       //printf("GSocket::httpRecvRawToFile recSize: %i, ", lng);
       deb("RECV LNG(loop): "+GString(lng));
       loops++;
       if( lng <= 0 ) break;
       if( loops == 1 )
       {
           headerSize = GString(buf).indexOf("\r\n\r\n")+3;
           fwrite (buf+headerSize , sizeof(char), lng-headerSize, pFile);
           lng -= headerSize;
       }
       else fwrite (buf , sizeof(char), lng, pFile);
       if( !(loops % 10) )GStuff::dormez(20);

       fileSize += lng;
       m_iDownloadSize = fileSize;
       printf("setSize: %i\n", m_iDownloadSize);
       //Sleep(1);
    }
    fclose(pFile);
    deb("RecRawToFile full lng: "+GString(fileSize)+", loops: "+GString(loops));
    return fileSize;
}
*/
int GSocket::httpRecvRawToFile ( GString server, GString srcFile, GString fileName )
{
    m_iStopFlag = 0;
    m_iDownloadSize = 0;

    if( srcFile.strip().length() < 1) return -99;
    srcFile = "/"+srcFile.stripLeading('/');

    m_iDownloadSize = -1;
    char buf [ bufSize ];
    int port = 80;

    deb("RecRawToFile start");
    FILE * pFile;
    pFile = fopen (fileName, "wb");
    if( pFile == NULL )
    {
        deb("RecRawToFile cannot open trgFile");
        return -1;
    }


    set_non_blocking(true);
    int rc = connect(server, port);
    deb("RecRawToFile rc connect: "+GString(rc));
    if( !rc )
    {

        rc = sendRaw("GET "+srcFile+" HTTP/1.0\r\n");
        deb("RecRawToFile rc sendHttpHeader: "+GString(rc));
        rc = sendRaw("Host: "+server+":"+GString(port)+"\r\n");
        deb("RecRawToFile rc sendHttpHost: "+GString(rc));
        rc = sendRaw("\r\n"); //<- always close with \r\n, this signals end of http request
        deb("RecRawToFile rc sendHttpFin: "+GString(rc));
        //usleep(100*1000);

        //rc = sendRaw("GET /blacklists.tgz HTTP/1.1\r\nHost: squidguard.mesd.k12.or.us\r\n\r\n Connection: keep-alive\r\n\r\n Keep-Alive: 300\r\n");
    }
    if( rc ) return -1;


    int lng, loops = 0, fileSize = 0;
    int headerSize = 252; //Usually. Is overwritten below.


    deb("RECV LNG  Start LOOP ");
    while( !m_iStopFlag )
    {
       memset ( buf, 0, bufSize );
       deb("RECV LNG(loop), calling recv");
       lng = ::recv ( m_Socket, buf, bufSize, 0 );
       //printf("GSocket::httpRecvRawToFile recSize: %i, ", lng);
       deb("RECV LNG(loop): "+GString(lng));
       loops++;
       if( lng <= 0 ) break;
       if( loops == 1 )
       {
           headerSize = GString(buf).indexOf("\r\n\r\n")+3;
           fwrite (buf+headerSize , sizeof(char), lng-headerSize, pFile);
           lng -= headerSize;
       }
       else fwrite (buf , sizeof(char), lng, pFile);
       if( !(loops % 10) )GStuff::dormez(20);

       fileSize += lng;
       m_iDownloadSize = fileSize;
       printf("setSize: %i\n", m_iDownloadSize);
       //Sleep(1);
    }
    fclose(pFile);
    deb("RecRawToFile full lng: "+GString(fileSize)+", loops: "+GString(loops));
    return fileSize;
}

int GSocket::downloadSize()
{
    printf("GSocket::downloadSize: %i\n", m_iDownloadSize);
    return m_iDownloadSize;
}

int GSocket::recvRawText ( GString * data )
{
    char buf [ bufSize ];
    m_iStopFlag = 0;
    deb("RecRaw start");
    /*
   struct sockaddr addr;
   socklen_t fromlen;
   fromlen = sizeof addr;
   int byte_count = recvfrom(m_Socket, buf, bufSize, 0, &addr, &fromlen);
   *data += GString(buf, byte_count);
   deb("data: "+(*data));
   return byte_count;
*/

    *data = "";
    memset ( buf, 0, bufSize );

    int lng = ::recv ( m_Socket, buf, bufSize, 0 );
    deb("RECV(0) LNG: "+GString(lng));
    if( lng <= 0 ) return 0;
    *data += GString(buf, lng);

    while( lng )
    {
        memset ( buf, 0, bufSize );
        lng = ::recv ( m_Socket, buf, bufSize, 0 );
        deb("RECV LNG: "+GString(lng));
        if( lng <= 0 ) break;
        *data += GString(buf, lng);
        if( lng < bufSize ) break;
    }

    return lng;
}   


int GSocket::recv ( GString * data )
{
    m_iStopFlag = 0;
    *data = "";
    char buf [ MAXRECV + 1 ];
    char * dataBuf = NULL;
    char * tempBuf = NULL;

    memset ( buf, 0, MAXRECV + 1 );
    int lng = ::recv ( m_Socket, buf, MAXRECV, 0 );
    deb("Rec lng: "+GString(lng));
    if( lng <= 0 ) return 0;


    GString msg = GString(buf);
    int payloadSize = msg.asInt() - int(MAXRECV);
    if( payloadSize < 0 ) return 1;
    dataBuf = new char[payloadSize+1];
    tempBuf = new char[payloadSize+1];
    int cnt = 0;
    while( cnt < payloadSize )
    {
        lng = ::recv ( m_Socket, tempBuf, payloadSize, 0 );
        if( lng < 0 || cnt > payloadSize ) break;
        memcpy(dataBuf+cnt, tempBuf, lng);
        cnt += lng;
    }
    dataBuf[payloadSize] = '\0';
    *data = GString(dataBuf, payloadSize);
    delete [] dataBuf;
    delete [] tempBuf;
    return lng;
}

int GSocket::connect ( GString host, int port )
{
	deb("Connect start");
   	#ifdef MAKE_VC
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if(iResult != NO_ERROR)
	if ( iResult != NO_ERROR ) 
	{
		deb("WSAStartup Err: "+GString(iResult));
		return 1;
	}
	#endif
	
	struct addrinfo aiHints, *rp;
	struct addrinfo *aiList = NULL;
    int retVal, rc;

	memset(&aiHints, 0, sizeof(aiHints));
	/* OK
	aiHints.ai_family = AF_INET;
	aiHints.ai_socktype = SOCK_STREAM;
	aiHints.ai_protocol = IPPROTO_TCP;
	*/
	aiHints.ai_family = AF_UNSPEC;
    aiHints.ai_socktype = SOCK_STREAM; //SOCK_DGRAM for UDP
    aiHints.ai_protocol = IPPROTO_TCP;
	aiHints.ai_flags = 0;
	if ((retVal = getaddrinfo(host, GString(port), &aiHints, &aiList)) != 0) 
    {
        deb("getAddrInfo failed, erc: "+GString(retVal));
        return 1;
    }
    for (rp = aiList; rp != NULL; rp = rp->ai_next)
    {
		
        m_Socket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		deb("looping aiList, m_Socket: "+GString(m_Socket));
        if (m_Socket == -1) continue;
		deb("looping aiList, connecting socket...");
#ifdef MAKE_VC
        //rc = ::connect(m_Socket, (SOCKADDR *) & rp->ai_addr, rp->ai_addrlen);
		rc = ::connect(m_Socket, rp->ai_addr, rp->ai_addrlen);
#else
        rc = ::connect(m_Socket, rp->ai_addr, rp->ai_addrlen);
#endif
		deb("looping aiList, connecting socket, rc: "+GString(rc));		
        if ( rc != -1 )
        {
             //perror("socket error(2)");
             //Not sure about bind...
            //::bind(m_Socket, rp->ai_addr, rp->ai_addrlen);
            //perror("socket error(3)");
            int optval = 1;
			deb("looping aiList, setting sockopt");
            setsockopt ( m_Socket, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &optval, sizeof ( optval ) );
            setsockopt ( m_Socket, IPPROTO_TCP, TCP_NODELAY, ( const char* ) &optval, sizeof(optval));
            deb("got connection");
            break;
		
		}
        else
        {
#ifdef MAKE_VC
            //_close(m_Socket);
			closesocket(m_Socket);
#else
            close(m_Socket);
#endif
        }
	}
	freeaddrinfo(aiList);
	if ( !rp ) return 2;
	return 0;
/*******	
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int rc;
	#ifdef MAKE_VC
	WSADATA wsaData;
	int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
	if ( iResult != NO_ERROR ) 
	{
		deb("WSAStartup Err: "+GString(iResult));
		return 1;
	}
	#endif

	deb("Connect (1)");
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    // Allow IPv4 or IPv6 
    hints.ai_socktype = SOCK_STREAM; //Datagram socket,  SOCK_STREAM: TCP Only. For UDP, use  SOCK_DGRAM
    hints.ai_flags = 0;
    hints.ai_protocol = 0; //IPPROTO_TCP; //=0         //Any protocol 

    hints.ai_protocol = IPPROTO_TCP;

	deb("Connect (2)");
    rc = getaddrinfo(host, GString(port), &hints, &result);
	deb("Connect (2.1)");
    if (rc != 0)
    {
        deb("getAddrInfo failed, erc: "+GString(rc));
        return 1;
    }


    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        m_Socket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (m_Socket == -1) continue;
        perror("socket error(1)");

        if (::connect(m_Socket, rp->ai_addr, rp->ai_addrlen) != -1)
        {
             perror("socket error(2)");
             //Not sure about bind...
            //::bind(m_Socket, rp->ai_addr, rp->ai_addrlen);
            perror("socket error(3)");
            //
            int on = 1;
            setsockopt ( m_Socket, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on, sizeof ( on ) );
            deb("got connection");
            break;
        }
        else
        {
#ifdef MAKE_VC
            _close(m_Socket);
#else
            close(m_Socket);
#endif
        }
		
    }

    if ( !rp )
    {
        deb("Connection failed.");
        freeaddrinfo(result);
        return 1;
    }


    deb("Connection OK");
    //m_Socket =  socket(rp->ai_family, result->ai_socktype, result->ai_protocol);
    //inet_ntop(rp->ai_family, addr, ipstr, sizeof ipstr);
    //deb(ipstr);
    #ifdef MAKE_VC
    WSACleanup();
    #endif

    freeaddrinfo(result);
***/	
    return 0;
}

int GSocket::oldConnect ( GString host, int port )
{
  int rc;
  struct sockaddr_in localAddr, servAddr;
  struct hostent *he;
  
  #ifdef MAKE_VC
  WSADATA wsaData;
  int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
  if ( iResult != NO_ERROR ) 
  {
      deb("WSAStartup Err: "+GString(iResult));
	  return 1;
  }
  #endif


  he = gethostbyname( host );
  if(he==NULL) {
    deb("unknown host");
    return 1;
  }

  servAddr.sin_family = he->h_addrtype;
  memcpy((char *) &servAddr.sin_addr.s_addr, he->h_addr_list[0], he->h_length);
  servAddr.sin_port = htons( port );

  /* create socket */
  m_Socket = socket(AF_INET, SOCK_STREAM, 0);
  if(m_Socket<0)
  {
    deb("cannot open socket ");
    return 2;
  }

  /* bind port number */
  localAddr.sin_family = AF_INET;
  localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  localAddr.sin_port = htons(0);
  
  rc = ::bind(m_Socket, (struct sockaddr *) &localAddr, sizeof(localAddr));
  if(rc<0)
 {
    deb("cannot bind "+host+", port "+GString(port));
    return 3;
  }
				
  /* connect to server */
  rc = ::connect(m_Socket, (struct sockaddr *) &servAddr, sizeof(servAddr));
  if(rc<0) 
  {
    deb("cannot connect, rc: "+GString(rc));
    return 4;
  }
  return 0;
}

void GSocket::set_non_blocking ( bool val )
{
#ifdef MAKE_VC
    unsigned long mode = val ? 1 : 0;
    ioctlsocket(m_Socket, FIONBIO, &mode);
#elif 	__MINGW32__
    unsigned long mode = val ? 1 : 0;
    ioctlsocket(m_Socket, FIONBIO, &mode);
#else
  int opts;

  opts = fcntl ( m_Socket, F_GETFL );

  if ( opts < 0 ) return;

  if ( val ) opts = ( opts | O_NONBLOCK );
  else opts = ( opts & ~O_NONBLOCK );

  fcntl ( m_Socket, F_SETFL,opts );
#endif
}

int GSocket::handle()
{
   return m_Socket;
}

GString GSocket::ipFromHostName(GString hostname)
{
    struct hostent *he;
    struct in_addr **addr_list;
    int i;

    if ( (he = gethostbyname( hostname ) ) == NULL)
    {        
        //herror("gethostbyname");
        return "";
    }

    addr_list = (struct in_addr **) he->h_addr_list;

    for(i = 0; addr_list[i] != NULL; i++)
    {
        return inet_ntoa(*addr_list[i]);
    }
    return "";
}

void GSocket::deb(GString msg)
{
    printf("GSocket: %s\n", (char*) msg);
	#ifdef MAKE_VC
    //_flushall();
	#endif
    //GDebug::debMsg("GSocket", 1, msg);
}
