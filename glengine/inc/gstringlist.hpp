//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 
//

#ifndef _GSTRINGLIST_
#define _GSTRINGLIST_

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
#ifdef MakeGStringList
#define GStringListExp_Imp _Export
#else
#define GStringListExp_Imp _Import
#pragma library( "GLEngine.LIB" )
#endif
#endif
//************** Visual Age END *****************


#include <stdio.h>
#include <stdlib.h>
#include "gseq.hpp"
#include "gstring.hpp"


static unsigned long _ulGStringListInstanceCount = 0;
static int _iGStringListDebug = 0;


// ******************************* CLASS **************************** //
#ifdef MakeGStringList
class GStringListExp_Imp GStringList
        #else
class GStringList
        #endif
{
public:
    VCExport GStringList();
    VCExport GStringList(GString in, GString split=" ");
    VCExport ~GStringList();
    VCExport GStringList( const GStringList &aList );
    VCExport GStringList &operator = (const GStringList aList);
    VCExport GString at(int i);
    VCExport int count();
    VCExport void add(GString in, GString split = " ");
    VCExport GString toString(GString sep);

private:
    GSeq <GString> m_gstrSeq;

};

#endif





