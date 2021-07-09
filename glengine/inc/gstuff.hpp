//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt
//

#ifndef _GSTUFF_
#define _GSTUFF_


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
#ifdef MakeGStuff
#define GStuffExp_Imp _Export
#else
#define GStuffExp_Imp _Import
#pragma library( "GLEngine.LIB" )
#endif
#endif
//************** Visual Age END *****************


#if defined(__GNUC__) && __GNUC__ < 3
#include ostream.h>
#else
#include <ostream>
#endif

#include <gstring.hpp>
#include <gseq.hpp>


#ifdef MakeGStuff
class GStuffExp_Imp GStuff
        #else
class GStuff
        #endif
{
public:
    GStuff(){}
    //static GString getTxt(GString definer);
    VCExport static GString format(GString in, unsigned long maxLng = 100, char cut = ' ');
    VCExport static void dormez(int seconds);
    VCExport static GString formatPath(GString path);
    VCExport int shred(GString in, unsigned long maxLng = 100, char cut1 = ' ', char cut2 = ',');
    VCExport int lineCount();
    VCExport GString getLine(unsigned long i);
    VCExport static void sendMsg(GString msg);
    VCExport static void setBit(int &store, int val){ store |= 1 << val;}
    VCExport static void resetBit(int &store, int val){ store &= ~(1 << val); }
    VCExport static int hasBit(int store, int val){ return (store & (1 << val)) > 0 ? 1 : 0; }
    VCExport static void toggleBit(int &store, int val){ store ^= 1 << val; }
    VCExport static void startStopwatch();
    VCExport static int  getStopwatch();
    VCExport static int hexToInt(char s[]);
    VCExport static GString fileFromPath(GString fullPath);
    VCExport static GString pathFromFullPath(GString fullPath);
    VCExport static void splitString(GString in, char del, GSeq<GString> *outSeq);
    VCExport static void splitStringDelFormat(GString in, GSeq<GString> *outSeq);
    VCExport static GString setDbQuotes(GString in);
    VCExport static GString breakLongLine(GString txt, int maxLen);
    VCExport static GString wrap(GString in);
    VCExport static GString formatForXml(GString in);
    VCExport static GString decorateTabName(GString in);
    VCExport static void xorEncrypt(GString key, GString& txt, int txtLen);
    VCExport static void xor_encrypt(char *key, char *input, int n);
    VCExport static GString encryptPwd(GString txt);
    VCExport static GString decryptPwd(GString txt);
    VCExport static GString encryptToBase64(GString in);
    VCExport static GString decryptFromBase64(GString in);
    VCExport static GString encryptDecrypt(GString toEncrypt);
    //VCExport static unsigned char* encryptDecrypt(unsigned char* toEncrypt);
    //VCExport static std::string GStuff::base64_decode(std::string const& encoded_string);
    VCExport static int base64_decode(char *in, size_t inLen, unsigned char *out, size_t *outLen);
    VCExport static int base64_encode(char *in, size_t inLen, unsigned char *out, size_t *outLen);
    VCExport static GString GetTime();
    VCExport static GString TableNameFromStmt(GString stmt);
    VCExport static GString WhereClauseFromStmt(GString stmt);
    VCExport static GString currentWorkDir();
    VCExport static std::string base64_decode(std::string const& encoded_string);
    VCExport static std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);

    VCExport static void xorEncrypt(char *data, int data_len);


private:
    GSeq <GString> lineSeq;
};

#endif
