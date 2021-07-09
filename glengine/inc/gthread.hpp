//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt
//

#ifndef _GTHREAD_
#define _GTHREAD_


#ifdef  __IBMCPP__
#define MAKE_VA
#define NO_QT
#endif

#ifdef  _MSC_VER
  #ifndef MAKE_VC
    #define MAKE_VC
  #endif
#endif



#ifdef MAKE_VC
#else
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include <iostream>
#include <string.h>
#endif
#include <gstring.hpp>


#ifdef QT_GTHREAD
#if QT_VERSION >= 0x050000
    #include <QtWidgets/QMessageBox>
#else
    #include <QMessageBox>
#endif
	#include <qwidget.h>
	#include <qtimer.h>
	#include <qapplication.h>
#endif




// ******************************* CLASS **************************** //
#ifdef MakeGThread
  class GStringExp_Imp GThread
#else
  class GThread
#endif
{

	//  ************** QT RELATED THINGS ************
	#ifdef QT_GTHREAD
		public: 
      		VCExport    void setBox(QWidget * aWdgt);
            VCExport    int wdgtIsUp();

  		public slots:
      			int checkState();

  		private: 
      			QWidget *myWdgt;
	#endif



private:
  void tm(char* msg){ printf("GThread > %s\n", msg); }

#ifdef MAKE_VC
  HANDLE tHD, hdMutex;
#else
#endif

public:
	//friend void *threadFn(void* fd);
	VCExport    GThread();
	VCExport    void killBox();

	VCExport    ~GThread();
	VCExport    void setDone();
	#ifdef MAKE_VC
	VCExport    HANDLE start();
	VCExport    int doIt();
	#else
	VCExport    int start();
	VCExport    int doIt();	
	#endif
    VCExport    int stop();
	VCExport    int isAlive();
	VCExport    void setAlive(int alive){iAlive = alive;}
	//protected:
	VCExport    virtual void run() = 0;

	#ifdef MAKE_VC
	static void threadFn(void *arg);
	#else
	//static void* threadFn(void *arg);
	//void* threadFn(void *arg);
	#endif
	VCExport int start_Thread();
	

   
private:
	int iAlive;
	int boxOK;

	#ifdef MAKE_VC
	HANDLE _threadID;
	#else
	pthread_t  thread;
	#endif
};
#endif

/****************************************************************
      StartThread is calling the overwritten "run" function
****************************************************************/

#ifdef MAKE_VC
unsigned __stdcall StartThread(void *);
unsigned long __stdcall SomeThread(void *);
#else
extern "C" void *StartThread(void *);
#endif







