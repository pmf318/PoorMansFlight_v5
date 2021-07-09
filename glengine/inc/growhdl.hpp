//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt
//

#ifndef _GROWHDL
#define _GROWHDL_

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
#ifdef MakeGRowHdl
  #define GRowHdlExp_Imp _Export
#else
  #define GRowHdlExp_Imp _Import
//  #pragma library( "GLineHdl.LIB" )
  #pragma library( "GLEngine.LIB" )
#endif
#endif
//************** Visual Age END *****************


#include <gstring.hpp>
#include <gseq.hpp>

typedef struct {
    GString data;
    short isNull;
    short isBinary;
    short isTruncated;
    void init(){isBinary = 0; isTruncated = 0; isNull = 0;}
} DATA_CELL;

// ******************************* CLASS **************************** //
#ifdef MakeGRowHdl
  class GLineHdlExp_Imp GLineHdl
#else
  class GRowHdl
#endif
{
   private:
   GSeq <DATA_CELL*> rowElementSeq; //Max Columns

   short count;
   void clearSeq();

   public:

   VCExport     GRowHdl(){count = 0;}
   VCExport     GRowHdl(const GRowHdl & aGLHD);
   VCExport     ~GRowHdl();
   VCExport     GString version();
   VCExport     void    addElement(GString element, short isNull = 0, short bin = 0, short trunc = 0);
   VCExport     void    addElement(DATA_CELL *pCell);
   VCExport     DATA_CELL* rowElement(short index);

   VCExport     GString row();
   VCExport     GString rowElementData(short index);
   VCExport     GString justifyRowData(GSeq <short> * justSeq);
   VCExport     unsigned long elements();
   VCExport     GRowHdl & operator=(GRowHdl lh);
   VCExport     void tm(GString msg);
};
#endif
