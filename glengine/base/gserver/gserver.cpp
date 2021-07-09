
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

#ifndef _GServer_
#include "gserver.hpp"
#endif

#include <gstring.hpp>
#include <gstuff.hpp>
#include <idsql.hpp>

#ifdef MAKE_VC
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <string>
#include <io.h>
#include <errno.h>
#elif __MINGW32__
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <string>
#include <io.h>
#include <errno.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include <signal.h>
#include <sys/types.h>

static int counter;

char consInput[1000];
const int SRVMAXCONNECTIONS = 100;
const int RECBUFSIZE = 12;



#ifdef MAKE_VC
#include <string>
#include <stdio.h>
#define BUFFER_LEN (4096)
using std::string;
int GServer::httpGet(int argc, char **argv)
{
    char buf[1024];
    FillMemory(buf,1024,0x0);
    HANDLE fhand;
    string request;
    int sendret;
    int iRecv;
    int iResponseLength=0;
    int offset;
    DWORD dw;
    string res2;
    char recvBuffer[BUFFER_LEN]={0};
    string response;
    const char lb[]="\r\n\r\n";
    const char snsn[]="%s\n";
    bool error1=false;
    bool error2=false;
    bool error3=false;
    if(argc!=4)
    {
        printf(snsn,"Correct usage: httpget server.com /folder/file.zip c:\\savehere.zip");
        goto cleanup;
    }
    WSADATA wsaData;
    if(WSAStartup(MAKEWORD(2,2),&wsaData)!=0)
    {
        printf(snsn,"Error initializing Winsock 2.2");
        goto cleanup;
    }
    error1=true;
    if(LOBYTE(wsaData.wVersion)!=2||HIBYTE(wsaData.wVersion)!=2)
    {
        printf(snsn,"Winsock 2.2 not available");
        goto cleanup;
    }
    struct hostent *h;
    struct sockaddr_in sa;
    SOCKET server1;
    h=gethostbyname(argv[1]);
    if(h==0)
    {
        printf(snsn,"gethostbyname() failed");
        goto cleanup;
    }
    memcpy((char *)&sa.sin_addr,(char *)h->h_addr,sizeof(sa.sin_addr));
    sa.sin_family=h->h_addrtype;
    sa.sin_port=htons(80);
    server1=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(server1==INVALID_SOCKET)
    {
        printf(snsn,"socket() failed");
        goto cleanup;
    }
    error1=false;
    error2=true;
    if(connect(server1,(struct sockaddr *)&sa,sizeof(sa))<0)
    {
        printf(snsn,"connect() failed");
        goto cleanup;
    }
    request+="GET http://";
    request+=argv[1];
    request+=argv[2];
    request+=" HTTP/1.0";
    request+=&lb[2];
    request+="Host: ";
    request+=argv[1];
    request+=lb;
    printf("%s",request.c_str());
    sendret=send(server1,request.c_str(),request.length(),0);
    if(sendret==-1)
    {
        printf(snsn,"send() failed");
        goto cleanup;
    }
	#ifdef MAKE_VC
    LPCSTR arg = (LPCSTR)argv[3];
    fhand=CreateFile(arg,GENERIC_WRITE,FILE_SHARE_READ,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
	#else
	fhand=CreateFile(argv[3],GENERIC_WRITE,FILE_SHARE_READ,0,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
	#endif
    if(fhand==INVALID_HANDLE_VALUE)
    {
        printf(snsn,"CreateFile() failed");
        goto cleanup;
    }
    error2=false;
    error3=true;
    while((iRecv=recv(server1,recvBuffer,BUFFER_LEN-1,0))>0)
    {
        response.append(recvBuffer,iRecv);
        iResponseLength+=iRecv;
        ZeroMemory(recvBuffer,BUFFER_LEN);
    }
    if(iRecv==SOCKET_ERROR)
    {
        printf(snsn,"recv() failed");
    }
    offset=response.find(lb)+4;
    if(offset!=string::npos)
    {
        res2.assign(response,offset,response.size());
        if(WriteFile(fhand,res2.data(),res2.size(),&dw,0)==0)
        {
            printf(snsn,"WriteFile() failed");
            goto cleanup;
        }
    }


    cleanup:
    if(error1)
    {
        WSACleanup();
    }
    if(error2)
    {
        WSACleanup();
        closesocket(server1);
    }
    if(error3)
    {
        WSACleanup();
        closesocket(server1);
        CloseHandle(fhand);
    }
    return 0;
}

#endif





GServer::GServer()
{
	m_iDebug = 1;
	m_iStop = 0;
	counter = 0;
}

GServer::~GServer()
{
	out("Removing clients...\n");
	ClientData * pCLT;
    for( int i = 1; i <= (int)cltSeq.numberOfElements(); ++i )
	{
		pCLT = cltSeq.elementAtPosition(i);
		if( !pCLT ) continue;
		delete pCLT;
		pCLT = NULL;
	}
	out("...Server down.");
}


int GServer::init(int port)
{

	out("Starting Server on port "+GString(port));
	int erc = startListen(port);
	if( erc )
	{
		out("Failed to start server. ERC: "+GString(erc));
		return 1;
	}
	out("Server started.");
	
	return 0;
}
void GServer::addToList(ClientData * pCLT)
{
    out("Adding to list: "+GString(pCLT->id()));
	cltSeq.add(pCLT);    
    out("::addToList, elmts: "+GString(cltSeq.numberOfElements()));
}
void GServer::addToConsolesList(ClientData * pCLT)
{
	consolesSeq.add(pCLT);
}

GString GServer::showList()
{
	GString data;
	ClientData * pCLT;
    for( int i = 1; i <= (int)cltSeq.numberOfElements(); ++i )
	{
		pCLT = cltSeq.elementAtPosition(i);
		if( pCLT == NULL ) continue;
		data += "Client #"+GString(pCLT->id())+", IP: "+pCLT->ip()+"\n";

	}
	return data;
}
void GServer::killClient(int cltId)
{
    out("::killClient, to kill: "+GString(cltId));
	ClientData * pCLT;
    for( int i = 1; i <= (int)cltSeq.numberOfElements(); ++i )
	{
		pCLT = cltSeq.elementAtPosition(i);
		if( !pCLT ) continue;
		if( pCLT->id() == cltId )
		{
			out("calling removingFromList, found "+GString(pCLT->id())); 
			removeFromList(pCLT);
			delete pCLT;
			pCLT = NULL;
		}
	}
}

void GServer::removeFromList(ClientData *remCLT )
{
    out("Removing from list, current count: "+GString(cltSeq.numberOfElements()));
	ClientData * pCLT;
    for( int i = 1; i <= (int)cltSeq.numberOfElements(); ++i )
	{
		pCLT = cltSeq.elementAtPosition(i);
		if( !pCLT ) continue;		
		if( remCLT->id() == pCLT->id() ) 
		{
			cltSeq.replaceAt(i, NULL);
			break;
		}
	}
}
void GServer::out(GString txt)
{
	//if( !m_iDebug ) return;
/***
long i;
for( i=1; i<=40; ++i)
{
	if( i <= txt.length() ) putchar((char) txt[i]);
	else putchar(' ');
}
fflush(stdout);
printf("\r");
return;
****/
	printf("CFCServ> %s\n", (char*) txt);
	#ifdef MAKE_VC
	_flushall();
	#endif
	ClientData * pCLT;
    for( int i = 1; i <= (int)consolesSeq.numberOfElements(); ++ i )
	{
		pCLT = consolesSeq.elementAtPosition(i);
		if( !pCLT ) continue;
        printf(">>>>Sending to console:  %s\n", (char*) txt);
        pCLT->sendData(txt);
	}
}
GString GServer::in()
{
	printf("=> ");
	fflush( stdout );
	//gets(consInput);
	return GString(consInput).strip();
}

int GServer::startListen(int port)
{	
	#ifdef MAKE_VC
	WSADATA wsaData;
	int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
	if ( iResult != NO_ERROR ) 
	{
		out("Erc "+GString(iResult)+" on WSAStartup, quitting");
		return 1;
	}	
	#endif
	//Create Socket
	m_iSocket = socket ( AF_INET, SOCK_STREAM, 0 );
	int on = 1;
	int erc = setsockopt ( m_iSocket, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on, sizeof ( on ) );
	if( erc ) 
	{
		out("Erc "+GString(erc)+" on setSockOpt, quitting");
		return 1;
	}
 	//Bind Socket
	sockaddr_in Addr;
	Addr.sin_family = AF_INET;
	Addr.sin_addr.s_addr = htonl(INADDR_ANY);
	Addr.sin_port = htons ( port );
	erc = bind( m_iSocket, ( struct sockaddr * ) &Addr, sizeof ( Addr ) );
	if( erc ) 
	{
		out("Erc "+GString(erc)+" on bindPort, quitting");
		return 1;
	}

	//Listen
	erc = listen ( m_iSocket, SRVMAXCONNECTIONS );
	if( erc ) 
	{
		out("Erc "+GString(erc)+" on Listen, quitting");
		return 1;
	}
	
	m_iNextClientID = 1;

#ifndef MAKE_VC
	m_iPID = getpid();	
	out("PID: "+GString(m_iPID));
#endif
	while ( !m_iStop )
	{
		if( m_iStop ) break;
		out("Waiting for a client to connect. CURRENT CONNECTIONS: "+GString(counter));
		try{
			out("Creating client...");
			createClient();
			m_iNextClientID++;
			out("Client created");
			counter++;
		} 
		catch(...) {
			out("Got EXCEPTION, stopping server. Sorry.");
			break;
		}
	}
	return 0;

}

void GServer::killSrv()
{
#if defined (MAKE_VC) || defined (__MINGW32__)
    out("Kill: Linux only.");
#else
    kill(m_iPID, SIGINT);

#endif
}


/************************************************************
************************************************************/
void GServer::createClient()
{
	out("createClient called: Socket: "+GString(m_iSocket)+", id: "+GString(m_iNextClientID));
    ClientData  *cltData = new ClientData((unsigned long*)this, m_iSocket, m_iNextClientID);
    addToList(cltData);
    showList();

    //cltData->handleSocket( cltSocket, m_iID );
    cltData->startThread();

	out("client created");
}


/*********************************************************************************
*
*  CLASS CLIENTDATA
*  -Start Thread for each new connection
*
*********************************************************************************/ 
void ClientData::startThread()
{
    printf("+++++++Start thread\n");
    pThread = new MyThreadFn( this, m_iSocket, m_iID, m_ServerCast );
    printf("+++++++Start thread: START: sock: %i, cltSock: %i, id: %i, cltId: %i\n", m_iSocket, pThread->m_cltSocket , m_iID, pThread->m_id);
	pThread->start();
}

void ClientData::MyThreadFn::run()
{      
    printf("+++++++Start thread: RUN1: sock: %i, cltSock: %i, id: %i, cltId: %i\n", m_cltSocket, myClientData->m_iSocket, m_id, myClientData->m_iID);
    //printf("+++++++Start thread: START: sock: %i, cltSock: %i, id: %i, cltId: %i\n", m_iSocket, m_cltSocket , m_iID, m_id);
    myClientData->handleSocket( m_cltSocket, m_id, m_pServerCast );
}

/******************************************************
*
*  CTor f. ClientData
*
******************************************************/
ClientData::ClientData( const ClientData &clt )
{
    m_iSocket = clt.m_iSocket;
    m_iReady = clt.m_iReady;
    m_ServerCast = clt.m_ServerCast;
    m_strUserName = clt.m_strUserName;
    m_iID = clt.m_iID;
    m_strIP = clt.m_strIP;
}

ClientData & ClientData::operator=(const ClientData clt)
{
    m_iSocket = clt.m_iSocket;
    m_iReady = clt.m_iReady;
    m_ServerCast = clt.m_ServerCast;
    m_strUserName = clt.m_strUserName;
    m_iID = clt.m_iID;
    m_strIP = clt.m_strIP;
    return *this;
}

ClientData::ClientData( unsigned long*  pServer, int aSocket, int id )
{
    printf("ClientData ctor start\n");
  		
    sockaddr_in Addr;
	int length = sizeof(Addr);
	m_iID = id;
	while( 1 )
	{
        m_iSocket = accept ( aSocket, ( sockaddr * ) &Addr, ( socklen_t * ) &length );
        if( m_iSocket ) break;
	}		
	m_ServerCast = pServer;	
    //m_iSocket = m_iSocket;
    printf("ClientData ctor, sockIn: %i, iSock: %i\n", aSocket, m_iSocket);
	m_iID = id;
	out( "Client created for "+GString(inet_ntoa(Addr.sin_addr)));
	m_strIP = GString(inet_ntoa(Addr.sin_addr));
}


/************************************************************
*
*  This is threaded
* 
************************************************************/

void ClientData::handleSocket(int aSocket, int id, unsigned long* serverCast)
{
  
    PMF_UNUSED(id);
	int lng, erc;
	int payloadSize = 0;
	char buf [ RECBUFSIZE + 1 ];
	char * dataBuf = NULL;	
	char * tempBuf = NULL;
    int cnt = 0;
	GString msg, cmd;
	
	while( 1 )
	{
		msg = "";
		try{
		memset ( buf, 0, RECBUFSIZE + 1 );
		lng = recv ( aSocket, buf, RECBUFSIZE, 0 );
		if( lng <= 0 )
		{
            out("Client "+GString(id)+" quit in an impolite fashion. (lng: "+GString(lng)+")");
			break;
		}
		
		cmd = GString(buf);
		payloadSize = cmd.asInt() - int(RECBUFSIZE);
		if( payloadSize < 0 ) 
		{
            out("Got negative payloadSize\n");
			break;
		}
		//this->out("payloadSize: "+GString(payloadSize));
		dataBuf = new char[payloadSize+1];
		tempBuf = new char[payloadSize+1];
		cnt = 0;
		while( cnt < payloadSize )
		{
            if( !aSocket )
            {
                delete [] dataBuf;
                delete [] tempBuf;
                return;
            }
			lng = recv ( aSocket, tempBuf, payloadSize, 0 );
			if( lng < 0 )
			{
                out("Connection to client "+GString(id)+" was lost\n");
				delete [] dataBuf;
				delete [] tempBuf;
				return;
			}
;
			if( cnt > payloadSize ) break;
			memcpy(dataBuf+cnt, tempBuf, lng);
			cnt += lng;
		}
		dataBuf[payloadSize] = '\0';
		cmd = GString(dataBuf, payloadSize);
		delete [] dataBuf;
		delete [] tempBuf;
        //out("Got cmd: "+cmd+"<-");

		if( cmd == "CON" ) 
		{
            msg = "[ID "+GString(id)+"]";
		}
		else if( cmd == "INFO" )
		{
			msg = "Connections: "+GString(counter)+"\n";
            msg += ((GServer*) serverCast)->showList();
		}
		else if( cmd == "?" )
		{
            msg = "CFCSrv 0.2 worlds dumbest server\n";
			msg += "***Commands***:\nINFO - show connections\nKILL <n> - kill client <n>\n";
			msg += "QUIT - Exit session\nCFCSTOP - Stop server and exit";
		}
        else if( cmd == "CONSOLE" ) (((GServer*) serverCast)->addToConsolesList(this));
		else if( cmd == "QUIT" ) break;
        /*
        else if( cmd == "KILLSRV" ) //use CFCSTOP
        {
            delete (GServer*) serverCast;
        }
        */
		else if( cmd.occurrencesOf("KILL") )
		{
            int idToKill = cmd.subString(5, cmd.length()).strip().asInt();
            if( idToKill == id ) msg = "Use QUIT instead of trying to kill self";
			else
			{
                out("Killing client "+GString(idToKill)+"...");
                ((GServer*) serverCast)->killClient(idToKill);
			}
		}
		else if( cmd == "CFCSTOP" )
		{
            ((GServer*) serverCast)->killSrv();
			break;
        }
        else
        {
            out("Unknown: "+cmd);
            //msg = "CFCSrv: Unknown command.";
        }
        //out("calling send: "+msg);
        erc = sendData(msg, aSocket);

		if( erc < 0 ) break;

		}
        catch(...) {((GServer*) serverCast)->out("-->Got EXCEPTION in this client...");}

	} //END WHILE
	counter--;
    ((GServer*) serverCast)->out("Shutting down client "+GString(id)+", remaining connections: "+GString(counter));
    ((GServer*) serverCast)->removeFromList(this);
}
int ClientData::sendData(GString data)
{
	GString sndData = GString(data.length()+12).rightJustify(12, '0')+ data;
    int erc = send ( m_iSocket, sndData, sndData.length(), 0 );
    if(erc) printf("::sendData to clt %i failed: %i\n", m_iSocket, erc);
    return erc;
}

int ClientData::sendData(GString data, int aSocket)
{
    GString sndData = GString(data.length()+12).rightJustify(12, '0')+ data;
    //printf("SENDING: %s\n", (char*) data);
    return send ( aSocket, sndData, sndData.length(), 0 );
}

ClientData::~ClientData(  )
{

	out("Killing client "+GString(m_iID));
#if defined (MAKE_VC) || defined (__MINGW32__)
    shutdown(m_iSocket, SD_SEND);
#else
    shutdown(m_iSocket, SHUT_WR);
#endif
    ((GServer*) m_ServerCast)->removeFromList(this);
	counter--;
}


void ClientData::out(GString txt)
{
//	if( !m_iDebug ) return;

	txt = GString(" ["+GString(m_iID)+"] ") + txt;
	//txt.sayIt();
	((GServer*) m_ServerCast)->out(txt);
}

int ClientData::id()
{
	return m_iID;
}
GString ClientData::ip()
{
	return m_strIP;
}
