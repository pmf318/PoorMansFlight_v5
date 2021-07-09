#ifndef GXml_HPP
#define GXml_HPP


#include "gstring.hpp"
#include "gseq.hpp"

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
#ifdef MakeGXml
#define GXmlExp_Imp _Export
#else
#define GXmlExp_Imp _Import
#pragma library( "GLEngine.LIB" )
#endif
#endif
//************** Visual Age END *****************

typedef struct {
    GString Name;
    GString Value;
} GXML_ATTR;



#ifdef MakeGXml
class GXmlExp_Imp GXml
        #else
class GXml
        #endif
{

public:
    GXml();
    GXml(GString xmlAsString);
    GXml( const GXml &x );
    ~GXml();
    int readFromFile(GString fileName);
    int lastErrCode();
    GString toString();
    GXml getBlocks(GString tag);
    GXml getBlocksFromXPath(GString tag);
    GXml getBlockAtPosition(GString tag, int position);
    int countBlocks(GString tag);
    GSeq <GXML_ATTR> attributeSeq();
    int attributeCount();
    GString getAttribute(GString name);



private:
    int endTagPos(GString data, GString tag);
    GString getBlock(GString &data, GString tag);
    void cleanInput();
    GString _xmlAsString;
    int _lastErrCode;
    GSeq <GXml> _blockSeq;
};
#endif
