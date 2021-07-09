

//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 
//
//  This is a very poor wrapper for threads. Whatever.
//




#ifndef _GTHREAD_
#include <gthread.hpp>
#endif

#include <gstuff.hpp>

#ifdef QT_GTHREAD
	#include <qlabel.h>
#endif

#ifdef MAKE_VC
//   #define _WIN32_WINNT 0x0500
   #include <windows.h>
   #include <process.h>
#endif

//void threadFn(void *arg);
 
int _isAlive;
int GThread::stop()
{
    _isAlive = 0;
    return 0;
}
/********************************************************************
* LINUX LINUX LINUX LINUX LINUX LINUX LINUX LINUX LINUX LINUX LINUX 
********************************************************************/
#ifndef MAKE_VC //Linux
extern "C" void* threadFn (void* arg) 
{
	GThread *myThread = (GThread *)arg;
 	myThread->run();
    return NULL;
} 
int GThread::start_Thread()
{    
	int erc;
    //printf("GThread::start_Thread() called\n");
    //erc = pthread_create(&thread, NULL,(void*(*) (void *))&GThread::threadFn, this);
    erc = pthread_create(&thread, NULL,(void*(*) (void *))&threadFn, this);
    //erc = pthread_create(&thread, NULL,threadFn, this);    
    //printf("GThread, thread created. erc: %i\n", erc);
    pthread_join(thread, NULL);
	if( erc == 0 ) _isAlive = 1;
	else _isAlive = 0;
	return erc;
}


GThread::GThread()
{
    //printf("GThread ctor\n");
	_isAlive = 0;    
	#ifdef QT_GTHREAD
	myWdgt = NULL; 
	#endif
	boxOK = 0;
    //printf("GThread ctor done\n");
}

#ifdef QT_GTHREAD
void GThread::setBox(QWidget * aWdgt)
{
	myWdgt = aWdgt;
	boxOK = 1;
	aWdgt->setWindowTitle("RUNNING");
}
#endif

GThread::~GThread()
{
    //printf("GThread dtor\n");
	killBox();
}

void GThread::killBox()
{
    try
    {
        if( boxOK )
        {
            //printf("BoxOK...\n");
            //printf("Closing\n");
        }
    }
    catch(...){}

}
int GThread::start()
{
    //printf("GThread::start.\n");
	_isAlive = 1;
    pthread_create(&thread, NULL, StartThread, this);
    //printf("GThread::started, erc: %i\n", erc);
	//pthread_join( thread, NULL);
    return 0;
}


#ifdef QT_GTHREAD
int GThread::wdgtIsUp()
{

//printf("Gthread, in wdgtIsUp()\n");
      if( myWdgt )
      {
//printf("Gthread, in wdgtIsUp(), box NOT null\n");
          if( !myWdgt->isVisible() ) return 0;
      }
      return 1;
      
}
int GThread::checkState()
{
   return 0;
}
#endif

int GThread::isAlive()
{
	return _isAlive;
}
void GThread::setDone()
{
    boxOK = 0;
}

void *StartThread(void * aThreadClass)
{
    //printf("GThread::startThread\n");
	int count = 0;
	GThread *myThread = (GThread *)aThreadClass;
	myThread->run();
    ////printf("Check up...\n");
	#ifdef QT_GTHREAD
	while (!myThread->wdgtIsUp() && count++ < 100 )
	{
		GStuff::dormez(100);
        //printf("!up, dormCnt: %i\n", count);
	}
	#endif

	myThread->killBox();

	_isAlive = 0;
	myThread->setAlive( 0 );
    return NULL;
}
int GThread::doIt()
{
   return 0;
}

//******************************************************************************
//***************** Windows ****************************************************
//******************************************************************************
#else  //Windows
extern "C"
{
    #include <process.h>
    #include <windows.h>
}


void GThread::threadFn(void *arg)
{
	GThread *myThread = (GThread *)arg;
 	myThread->run();

}
int GThread::start_Thread()
{    
    int erc;
    //printf("GThread::start_Thread() called\n");
    erc = _beginthread(threadFn, 0, this);     
   //printf("GThread, thread created. erc: %i\n", erc);
   return erc;
}


GThread::GThread()
{
  _isAlive = 0;    
  #ifdef QT_GTHREAD
     myWdgt = NULL; 
  #endif
  boxOK = 0;
}

#ifdef QT_GTHREAD
void GThread::setBox(QWidget * aWdgt)
{
//printf("GThread, in ::setBox, boxOK should be 1\n");
  myWdgt = aWdgt;
  boxOK = 1;
  aWdgt->setWindowTitle("RUNNING");
}
#endif

GThread::~GThread()
{
   killBox();
}

void GThread::killBox()
{
    //printf("GThread, in killBox..\n");
    //printf("GThread, in killBox, boxOK: %i\n", boxOK);
#ifdef QT_GTHREAD
    if( boxOK )myWdgt->setWindowTitle("FINISHED");
#endif
    CloseHandle(_threadID);
    try {
        if( boxOK )
        {
            //printf("BoxOK...\n");
            //GStuff::dormez(10000);
            //printf("Closing\n");
#ifdef QT_GTHREAD
            //printf("GThread, in killBox, boxOK: %i, trying to close wdgt\n", boxOK);
            myWdgt->close();
#endif
        }
    }
    catch(...){//printf("DELEXC\n");}
    }
}


HANDLE GThread::start()
{
   _isAlive = 1;
  _threadID = (HANDLE) _beginthreadex(0,0,StartThread,this,0,0);
   return 0;
}



#ifdef QT_GTHREAD
int GThread::wdgtIsUp()
{
//printf("Gthread, in wdgtIsUp()...\n");
      if( myWdgt )
      {
//printf("Gthread, in wdgtIsUp(), wdgt NOT null\n");
          GStuff::dormez(300);
          if( !myWdgt->isVisible() ) return 0;
      }
      return 1;
      
}
int GThread::checkState()
{
   return 0;
}
#endif

int GThread::isAlive()
{
	return _isAlive;
}
void GThread::setDone()
{
//printf("GThread, setDone (i.e. set boxOK = 0)\n");
    //boxOK = 0;
}

//unsigned long __stdcall StartThread(void * aThreadClass)
unsigned __stdcall StartThread(void * aThreadClass)
{
//printf("GThread::startThread\n");
  GThread *myThread = (GThread *)aThreadClass;
  myThread->run();

/*****
  #ifdef QT_GTHREAD
     while (!myThread->wdgtIsUp() && count++ < 100 )
     {
        GStuff::dormez(100);
        //printf("!up, dormCnt: %i\n", count);
     }
   #endif
//printf("GThread, killBox..\n");
  myThread->killBox();
//printf("GThread, killBox..DONE\n");
****/
   _isAlive = 0;
  #ifdef MAKE_VC
  return 0;
  #endif
  myThread->setAlive( 0 );
}




unsigned long __stdcall SomeThread(void * aThreadClass)
{
  GThread *myThread = (GThread *)aThreadClass;
  myThread->run();
  return 0;
}
int GThread::doIt()
{
   unsigned long tID;
   CreateThread(NULL, 4096,SomeThread,this,NULL,&tID);
   //_threadID = (HANDLE) _beginthreadex(0,0,SomeThread,this,0,0);
   return 0;
}


#endif ////////WIN

