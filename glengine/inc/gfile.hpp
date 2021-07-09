//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 
//


#ifndef _GFILE_
#define _GFILE_

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
#ifdef MakeGFile
  #define GFileExp_Imp _Export
#else
  #define GFileExp_Imp _Import
//  #pragma library( "GFile.LIB" )
  #pragma library( "GLEngine.LIB" )
#endif
#endif
//************** Visual Age END *****************



#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <gstring.hpp>
#include <gseq.hpp>

#define GF_NOCREATE 1
#define GF_OVERWRITE 2
#define GF_APPENDCREATE 3
#define GF_READONLY 4

#ifdef MakeGFile
  class GFileExp_Imp GFile
#else
  class GFile
#endif
{
private:
   GString fileName;
   int init;
   GSeq <GString> m_seqLines;
   void tm(GString  message);
   int m_iDebug;
   int seqToFile(GSeq <GString> * aSeq, GString mode);

public:
   VCExport   GString version();
   VCExport   GFile(int debugMode = 0);
   VCExport   GFile(GString fileName, int action = GF_NOCREATE, int debugMode = 0);
   VCExport   GString getKey(GString key, GString sep = "=", GString rem = "#");
   VCExport   int writeToNewLine(GString line);
   VCExport   int writeLine(GString line);
   VCExport   int replaceLine(GString key, GString line, GString sep="=", GString rem = "#", int add=0);
   VCExport   int addOrReplaceLine(GString key, GString line, GString sep="=", GString rem = "#");
   VCExport   int setFileName(GString fName, int action = GF_NOCREATE);
   VCExport   int initOK();
   VCExport   int overwrite(GSeq <GString> * aSeq);
   VCExport   int append(GSeq <GString> * aSeq);
   VCExport   GString getLine(int index);
   VCExport   int lines();
   VCExport   int addLine(GString line);
};
#endif


