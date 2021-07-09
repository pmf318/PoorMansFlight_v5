//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 
//
/*********************************************************************
*********************************************************************/

#ifndef _GDEBUG_
#include <gdebug.hpp>
#endif

#include <QMessageBox>




#include "gsocket.hpp"
#   include "gstuff.hpp"
#include "debugObserver.hpp"


#ifdef DEBUG_GUI
//static GDebug *m_debugGui;
#endif

static int _iGDebugInstanceCount = 0;

int debugSettings = 0;
GString globalLogFile = "";
FILE *fOut = NULL;

#ifdef DEBUG_GUI
#include <QBoxLayout>
#include <QPushButton>
#endif

GDebug *GDebug::m_GDebug = NULL;

//int _haveSocket;


GDebug* GDebug::getGDebug(int level, GString logFile)
{
    if( !m_GDebug )
    {
        m_GDebug = new GDebug(level, logFile);
        if( logFile.length() ) remove(logFile);
    }
    return m_GDebug;
}

void GDebug::debugMsg(GString className, int instance, GString msg)
{
    if( m_iDebugLevel == 0 ) return;

    GString out = "--"+className+"["+GString(instance)+"]> "+msg;
    //printf("INST: %i>class: %s, Msg: %s, haveSock: %i\n", _ulGDebugInstanceCount, (char*) className, (char*) msg, _haveSocket);
    if( GStuff::hasBit(m_iDebugLevel, GDEB_TO_SRV) && _haveSocket )sendToServer(out, 0);

    if( GStuff::hasBit(m_iDebugLevel, minLevelForClass(className)) )GDebug::staticWriteDebug(m_strLogFile, out);
    if(pSD) pSD->addToBox(out);
}

//static method
void GDebug::debMsg(GString className, int instance, GString msg)
{
    if( debugSettings == 0 ) return;
    if( !GStuff::hasBit(debugSettings, minLevelForClass(className))) return;
    GString out = "--"+className+"["+GString(instance)+"]> "+msg;
    GDebug::staticWriteDebug(globalLogFile, out);
}

int GDebug::minLevelForClass(GString className)
{
    if(className == "odbcDSQL") return GDEB_BASE;
    else if(className == "db2api") return GDEB_BASE;
    else if(className == "DSQLOBJ") return GDEB_BASE;
    else if(className == "UDBAPI") return GDEB_BASE;
    else return GDEB_GUI;
}

void GDebug::staticWriteDebug(GString fileName, GString msg)
{

    if( fileName.length() )
    {
        if( fOut == NULL ) fOut = fopen(fileName, "a");
        if( fOut != NULL)
        {
            fputs( msg+"\n", fOut );
            //fclose(fOut);
            return;
        }
    }
    printf("%s\n", (char*)msg);
    #ifdef MAKE_VC
    _flushall();
    #endif

}

void GDebug::sendToServer(GString msg, int async)
{
    GString data;
    int erc;
    if( !m_pGSock )
    {
        if( createSocket() )
        {
            printf("setSock to 0, INST: %i> \n", _iGDebugInstanceCount);
            //_haveSocket = 0;
            return; //no server found
        }
    }
    erc = m_pGSock->send(msg);
    if( erc ) //Try again
    {
        createSocket();
        erc = m_pGSock->send(msg);
    }
    if( !async ) m_pGSock->recv(&data);

}
int GDebug::createSocket()
{
    if( m_pGSock ) delete m_pGSock;
    GString data;
    m_pGSock = new GSocket();
    int erc = m_pGSock->connect("localhost", 44000);
    if(erc) return erc;
    erc = m_pGSock->send("CON");
    if(erc) return erc;
    m_pGSock->recv(&data);
    return 0;
}

void GDebug::release()
{
	if( m_GDebug != NULL ) delete m_GDebug;    
	m_GDebug = NULL;
}
GDebug::~GDebug()
{
	if( pSD ) delete pSD;
    pSD = NULL;
    _iGDebugInstanceCount--;
    if( fOut != NULL ) fclose(fOut);
}

GDebug::GDebug(int level, GString logFile)
{
    m_pGSock = NULL;
    m_iDebugLevel = debugSettings =level;
    _haveSocket = 1;


    pSD = NULL;
    globalLogFile = m_strLogFile = logFile;
    _iGDebugInstanceCount++;
}
#ifdef DEBUG_GUI
void GDebug::setParent(QWidget *parent)
{
    GStuff::setBit(m_iDebugLevel, GDEB_TO_BOX);
    debugSettings = m_iDebugLevel;
    pSD = new ShowDeb(parent, this);
    pSD->resize(600, 400);
    pSD->addToBox("-- Caution: Leaving this open will slow down PMF considerably.");
    pSD->addToBox("-- It's really only for debugging and possibly not of much use to you.");
    pSD->addToBox("-- Usage:");
    pSD->addToBox("-- Move this window out of the way (it's non-modal)");
    pSD->addToBox("-- and use PMF as usual. Debug-messages will be shown here.");

	pSD->show();
}
#endif

void GDebug::setDebLevel(int l)
{
    printf("Set DEBLVL: %i\n", l);
    debugSettings = l;
}
void GDebug::showDebClosed()
{
    if( pSD ) delete pSD;
    pSD = NULL;
    GStuff::resetBit(m_iDebugLevel, GDEB_TO_BOX);
    debugSettings = m_iDebugLevel;
}

void GDebug::deb(GString message)
{
    printf("GDebug: %s\n", (char*)message);
#ifdef MAKE_VC
    _flushall();
#endif
}


