//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 
//



#ifndef _GDEBUG_
#define _GDEBUG_

#ifdef  __IBMCPP__
#define MAKE_VA
#define NO_QT
#endif

#ifdef  _MSC_VER
  #ifndef MAKE_VC
    #define MAKE_VC
  #endif
#endif


#define GDEB_BASE 0
#define GDEB_GUI 1
#define GDEB_TO_FILE 2
#define GDEB_TO_BOX 3
#define GDEB_TO_SRV 4

#ifdef MAKE_VC
#include <windows.h>
#endif

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
    #ifdef MakeGDebug
      #define GDebugExp_Imp _Export
    #else
      #define GDebugExp_Imp _Import
      #pragma library( "GLEngine.LIB" )
    #endif
#endif

//************** Visual Age END *****************

#ifdef DEBUG_GUI
#include <qwidget.h>
#include <QTextEdit>
#include <QListWidget>
#include <QPushButton>
#include <QDialog>
#include <QCloseEvent>
#endif

#include <gstring.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <gsocket.hpp>
#include "showDeb.hpp"

extern int debugSettings;
extern GString globalLogFile;


// ******************************* CLASS **************************** //
#ifdef MakeGDebug
  class GDebugExp_Imp GDebug
#else
  class GDebug
#endif
{            
	  
public:
    VCExport static GDebug* getGDebug(int level, GString log = "");
	VCExport static void release();
    VCExport static void setDebLevel(int l);
    VCExport static void debMsg(GString className, int instance, GString msg);

    #ifdef DEBUG_GUI
    void setParent(QWidget * parent);
    #endif



    void debugMsg(GString className, int instance, GString msg);
    void showDebClosed();

private:
    GDebug(int level, GString log = "");
    ~GDebug();
    static GDebug * m_GDebug;
    #ifdef DEBUG_GUI
    ShowDeb * pSD;
    #else
    void *pSD;
    #endif
    int _haveSocket;

    void deb(GString message);
    int m_iDebugLevel;
    GSocket *m_pGSock;
    GString m_strLogFile;
    int createSocket();
    void sendToServer(GString msg, int async);

    static void staticWriteDebug(GString fileName, GString msg);
    static int minLevelForClass(GString className);
	
};
#endif //class





