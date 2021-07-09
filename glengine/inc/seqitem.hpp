//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 
//
#ifndef _SEQITEM_
#define _SEQITEM_

#ifdef  __IBMCPP__
#define MAKE_VA
#define NO_QT
#endif

#ifdef  _MSC_VER
  #ifndef MAKE_VC
    #define MAKE_VC
  #endif
#endif

//************** Visual C++ ********************
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
#ifdef MakeSeqItem
  #define SeqItemExp_Imp _Export
#else
  #define SeqItemExp_Imp _Import
//  #pragma library( "SeqItem.LIB" )
  #pragma library( "GLEngine.LIB" )
#endif
#endif
//************** Visual Age END *****************

#include <stdlib.h>


template <class Type>
#ifdef MakeSeqItem
  class SeqItemExp_Imp SeqItem
#else
  class SeqItem
#endif
{
  public:
    Type item;
    SeqItem <Type> *next;

    VCExport    SeqItem (const Type &neu){ next = NULL; item = neu; }
    VCExport    SeqItem (){ next = NULL;  }    
    VCExport    SeqItem<Type>&operator =(const SeqItem <Type> *s){next = s->next; item = s->neu;}

};
#endif
