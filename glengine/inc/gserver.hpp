
#ifndef _GServer_
#define _GServer_



#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include <gstring.hpp>
#include <gthread.hpp>
#include <gseq.hpp>
#include <time.h>

class ClientData
{
	
	class MyThreadFn : public GThread	
	{
		public:
            MyThreadFn(ClientData * pClientData, int aSocket, int id, unsigned long *pServerCast )
            {
                myClientData = pClientData;
                m_cltSocket = aSocket;
                m_id = id;
                m_pServerCast = pServerCast;
                printf("--MyThreadFn ctor. Sock: %i, cltSock: %i, id: %i, clt->id: %i\n", aSocket, pClientData->m_iSocket, id, pClientData->m_iID);
            }
			
			VCExport virtual void run();
            ///void setData( GSocket aSocket, int id ) ;
        public:
			ClientData * myClientData;
            unsigned long *m_pServerCast;
			int m_cltSocket;
			int m_id;
	};



	public:
		//ClientData &operator=(const ClientData cltData);
        ClientData( ){}
        ClientData( const ClientData &clt );
        ClientData &operator=(const ClientData);

		ClientData( unsigned long* pServer, int aSocket, int id );
		~ClientData(  );
		int id();
		GString ip();
		void startThread();
        void handleSocket(int aSocket, int id, unsigned long *pServerCast);
		int sendData(GString data);
        int sendData(GString data, int aSocket);

		
	private:
        MyThreadFn* pThread;

        int m_iSocket;
		void out(GString txt);
		int m_iReady;

        unsigned long *m_ServerCast;
		
        ///int sendIt(GSocket aSocket, int size = 0, char* pData = 0);
		char * m_bfSendBuffer;
		GString m_strDataString;
		GSeq <GString> m_fldIdSeq;
		GString m_strUserName;
		char * m_bufMain;
		int m_iMainBufSize;
		int m_iID;
		GString m_strIP;

};
class GServer
{

 
public:
	GServer();
	~GServer();
	int init(int port);
	void setDebug(int debug){m_iDebug = debug;}

	void createClient();
	void addToList(ClientData * pCLT);
	void addToConsolesList(ClientData * pCLT);
	GString showList();
	void killClient(int cltID);
	void removeFromList(ClientData *remCLT );
	void out(GString txt);
    time_t startTime(){return m_tStart;}
	void setStop(int stop){m_iStop = stop;}
	void killSrv();
    int httpGet(int argc, char **argv);
private:

	time_t m_tStart;
	GString in();
	int startListen(int port);
	
	int m_iDebug;
	int m_iNextClientID;	
	GSeq <ClientData*> cltDataSeq;

	int m_iSeqLock;
	int m_iStop;
	GString m_strMyIP;
	int m_iSocket;
	GSeq <ClientData*> cltSeq;
	GSeq <ClientData*> consolesSeq;
	int m_iPID;
	
};



#endif

