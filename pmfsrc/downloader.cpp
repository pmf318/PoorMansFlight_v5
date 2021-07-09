#include "downloader.h"
#include "gstring.hpp"
#include "helper.h"
#include "pmfdefines.h"

#ifdef MAKE_VC
#include <string>
#include <stdio.h>
#define BUFFER_LEN (4096)
using std::string;
#endif


Downloader::Downloader(GDebug *pGDeb, QWidget* parent) :QWidget(parent)
{
    m_pGDeb = pGDeb;
    m_pParent = parent;
}

int Downloader::getPmfSetup()
{
    deb("getPmfSetup start");
    pThread = new MyThread;
    pThread->setOwner( this );
    pThread->start();
    deb("getPmfSetup start done");
    return 0;
}

Downloader::~Downloader()
{

}
void Downloader::cancelDownload()
{
    if(m_pSocket) m_pSocket->setStopFlag(1);
}

int Downloader::downloadedSize()
{
    printf("downloader::downloadedSize: %i\n", m_pSocket->downloadSize());
    if( m_pSocket ) return m_pSocket->downloadSize();    
    return -1;
}

int Downloader::httpGetFile(GString srv, GString srcFile, GString trgFile)
{
    deb("downloader::httpGetFile start");
    m_pSocket = new GSocket();
    int size = m_pSocket->httpRecvRawToFile(srv, srcFile, trgFile);
    emit downloadCompleted();
    deb("downloader::httpGetFile end, size: "+GString(size));
    delete m_pSocket;
    return size;
}

void Downloader::MyThread::run()
{

    myDownLd->deb("downloader::MyThread start");
    myDownLd->httpGetFile(_PMF_HTTP_SRV, _PMF_HTTP_FILE_LOCATION, newVersionFileLocation());
    myDownLd->deb("downloader::MyThread done");    
}

void Downloader::deb(GString msg)
{
    printf("downloader: %s\n", (char*) msg);
    m_pGDeb->debugMsg("downloader", 1, msg);
}


