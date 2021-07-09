//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 
//

#ifndef _GSTRING_
#define _GSTRING_

#ifdef  __IBMCPP__
#define MAKE_VA
#define NO_QT
#endif

#ifdef  _MSC_VER
  #ifndef MAKE_VC
    #define MAKE_VC
  #endif
#endif

#include <istream>
#include <ostream>

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
#ifdef MakeGString
  #define GStringExp_Imp _Export
#else
  #define GStringExp_Imp _Import
//  #pragma library( "GString.LIB" )
  #pragma library( "GLEngine.LIB" )
#endif
#endif
//************** Visual Age END *****************


#include <stdio.h>
#include <stdlib.h>
#if defined(__GNUC__) && __GNUC__ < 3
//#include <ostream.h>
#else
//#include <ostream>
#endif
#include <sstream>
//#include <iostream.h>
#include <string.h>
#include "gseq.hpp"

#ifdef MAP_GQSTRING
   #include <qstring.h>
#endif
#ifndef NO_QT
   #include <qstring.h>
#endif

#ifdef MAKE_VC
  #ifndef __AFX_H__
//  #include <afx.h>
    #include <stdint.h>
    #include <cstdint>
  #endif
#endif
#define GSTR_AS_HEX 1
#define MAX_GSTR_LNG 2100000000
static unsigned long _ulGStringInstanceCount = 0;
static int _iGStringDebug = 0;

// ******************************* CLASS **************************** //
#ifdef MakeGString
  class GStringExp_Imp GString
#else
  class GString
#endif
{


//class GStrList
//{
//public:
//      VCExport GStrList();
//      VCExport ~GStrList();
//      VCExport GStrList( const GStrList &aList );
//      VCExport GStrList &operator = (const GStrList aList);
//      VCExport GString at(int i);
//      VCExport int count();

//  private:
//      void add(GString in);

//};


private:
   char* ptr;
   unsigned long  size;
   //int8 size;
   void tm(const char * message);
   int fillBuf(char* in);
   int fillBuf(char* in, signed long lng);
   int catBuf(char* in);
   GString addBuf(char* in);
   unsigned long m_ulInstance;
   int hex2int(char c);
   void toHex( const char* chrs, signed long max);
   void hexToBin( const char* chrs, signed long max);
   void fromInt( const char* chrs, signed long max);
   void fromDBCS( const char* chrs, signed long max);
   GString upperOrLowerCase(int mode);
   int minIndexOfAny(GString tokenList);
    std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
    bool is_base64(unsigned char c);
    std::string base64_decode(std::string const& encoded_string);

   
public:
	enum GSTR_TYPE
	{
		NONE, 
        HEX,
        BIN,
        INT,
        DBCS
	}; 

   //include IString support....
  #ifdef MAKE_VA
//     VCExport operator IString() { return IString(*this);}
//     VCExport IPString( IString istr ): GString((char*) istr){};
  #endif
   //include CString support...
  #ifdef MAKE_VC
//     VCExport IPString( CString s ){ GString((const char*) s);}
//	     VCExport operator CString() { return (char*) *this;}
  #endif
  
  VCExport GString version();
  VCExport  void setDebugGString(int deb){_iGStringDebug = deb;}
  //Constructors:
  VCExport  GString();
  VCExport  GString( const GString &s );
  VCExport  GString( const GString &s, signed long max );
  VCExport  GString( const char* chrs, signed long max = -1, GSTR_TYPE type = NONE );
  VCExport  GString( const unsigned char* chrs, signed long max = -1 );
  VCExport  GString( const char aChar );

  VCExport  GString(const double & aDouble, const int prec = 6);
  VCExport  GString(const long double & aDouble, const int prec = 6);
  VCExport  GString(const unsigned long & aLong);
  VCExport  GString(const signed short& aShort);
  VCExport  GString(const unsigned short& aShort);
  VCExport  GString(const signed int & aInt);
  VCExport  GString(const unsigned int & aInt);
  VCExport  GString(const long & aLong);
  VCExport  GString(const float & aFloat);  
  VCExport  GString(std::string s);


#if defined(MAKE_VC) || defined(__MINGW32__)
  VCExport  GString(const int64_t & aBigInt);
  VCExport  GString(const uint64_t & aBigInt);
#endif
  VCExport  ~GString();
  VCExport unsigned char* AsBin();

//Operators
///////////////////////// Special For QT //////////////////////
/////////// This gets compiled inline    //////////////////////
#ifdef MAP_GQSTRING
	VCExport  operator QString() 
	{ 
        return QString::fromLocal8Bit(ptr);
	}
	VCExport  GString( QString s )
	{
		QByteArray bytes;
		m_ulInstance = _ulGStringInstanceCount++;
#if QT_VERSION >= 0x050000
        //QByteArray bytes = s.toLatin1();
    #ifndef MAKE_VC
        bytes = s.toUtf8();
    #else
        //QByteArray bytes = s.toLocal8Bit();
		bytes = s.toUtf8();
    #endif
#else
		bytes = s.toAscii();   
#endif
		
		bytes = s.toLocal8Bit();
		createFromQBytes(&bytes);
}
void createFromQBytes(QByteArray * bytes)
{
		ptr = 0;
        if( !bytes->length() )
		{
			ptr = new char[1]; ptr[0] = 0; size = 0; return;
		}
        if( bytes->length() > MAX_GSTR_LNG ) size = MAX_GSTR_LNG;
        else size = bytes->length();
		ptr = new char[size+1];
		memcpy(ptr, bytes->data(), size);
		ptr[size] = 0;
}
#endif
///////////////////////////////////////////////////////////////
  VCExport  GString &operator = (const char *);
  VCExport  GString &operator = (const char );
  VCExport  GString &operator = (const GString);

  //VCExport  operator char * () const;
  VCExport  operator char * (){ /* printf("op char*\n");printf("ret: %s\n", ptr);*/ return ptr;}
  //VCExport  operator const char * (){ printf("op const char*\n");return (const char*)ptr;};
  VCExport  operator unsigned char * () const {
	/*printf("op unsigned char* const\n");*/
        return (unsigned char*) ptr;
   }

  VCExport  operator char  (){
      //printf("op char const\n");
      return ptr[0];}

  VCExport  char&  operator [] (unsigned long pos);
  VCExport  char&  operator [] (int pos);


  VCExport  GString & operator+=(const GString x);
  VCExport  GString & operator+=(const char* x);
  VCExport  friend GString operator+(char* x, GString aString){
        return GString(x) + aString;}

  VCExport  friend GString operator+(const char* x, GString aString){
        return GString(x) + aString;}

  VCExport  GString operator+(const char* aString);
  VCExport  GString operator+(const GString x);

  VCExport  friend int operator==(const GString &x, const char * s)
           { return strcmp(x.ptr, s) == 0; }
  VCExport  friend int operator==(const GString &x, char * s)
           { return strcmp(x.ptr, s) == 0; }

  VCExport  friend int operator==(const GString &x, const GString &y)
           { return strcmp(x.ptr, y.ptr) == 0; }

  VCExport  friend int operator!=(const GString &x, const char* s)
           { return strcmp(x.ptr, s); }
  VCExport  friend int operator!=(const GString &x, const GString &y)
           { return strcmp(x.ptr, y.ptr); }

  VCExport friend int operator<(const GString &x, const GString &y)
           { return strcmp(x.ptr, y.ptr) < 0; }

  VCExport friend int operator>(const GString &x, const GString &y)
           { return strcmp(x.ptr, y.ptr) > 0; }

  //VCExport friend void * operator memcpy(const GString trg, const char* src, signed long n){}

  //VCExport friend std::istream& operator >> (std::istream& is, GString& a);
  VCExport GString& operator >> (GString& in);
  VCExport friend std::ostream& operator << (std::ostream& os, GString& s);


//  VCExport operator std::stringstream ( )
//        {
//            std::stringstream is;
//            &is  << ptr;
//            printf("operator std::istringstream\n");
//            return is;
//        }
//  std::stringstream getSs()
//  {
//      std::stringstream ss;
//      ss << "abc";
//      return ss;  // this is not a copy in C++11
//  }

//MemberFunctions
  VCExport  char* asChar();
  VCExport  void sayIt();
  VCExport  int asInt();
  VCExport  signed long asLong();
  VCExport  int64_t asLongLong();
  
  VCExport  unsigned long length() const;
  VCExport  signed long indexOf(const GString x, unsigned long startPos = 0);
  VCExport  signed long indexOf(const char aChar, unsigned long startPos = 0);
  VCExport  signed long indexOf(const char* aString, unsigned long startPos = 0);

  VCExport  GString leftJustify(unsigned long count, char padChar = ' ');
  VCExport  GString rightJustify(unsigned long count, char padChar = ' ');
  VCExport  GString translate(char in, char out);
  VCExport  GString translate(GString in, GString out);  

  VCExport  unsigned long lastIndexOf(const char aChar);
  VCExport  unsigned long lastIndexOf(const char * aStr);
  VCExport  unsigned long lastIndexOf(GString ipstr);

  VCExport  GString & remove(unsigned long startPos, signed long count);
  VCExport  GString & removeAll(char aChar);
  VCExport  GString & removeButOne(char aChar = ' ');
  VCExport  GString change(GString oldStr, GString newStr);
  VCExport  GString change(char oldChar, char newChar);
  GString changeProto(GString oldStr, GString newStr);
  //VCExport  GString & change(GString in, GString out);

  VCExport  GString stripLeading(char aChar = ' ');
  VCExport  GString stripLeading(GString str = " ");
  VCExport  GString stripTrailing(char aChar = ' ');
  VCExport  GString stripTrailing(GString str = " ");

  VCExport  GString strip(char aChar = ' ');
  VCExport  GString strip(const char * aStr);
  VCExport  GString subString(signed long startPos, signed long count, char padChar=' ');
  VCExport  int     isDigits();
  VCExport  int isUTF8();
  VCExport  bool isProbablyUtf8();
  VCExport  int isSomewhatPrintable(int chr2Oem = 0);
  VCExport  GString insert(char* str, signed long index = 0 );
  VCExport  unsigned long occurrencesOf(char aChar);
  VCExport  unsigned long occurrencesOf(const char* str);
  VCExport  unsigned long occurrencesOf(GString ipstr);
  VCExport  GString upperCase();
  VCExport  GString firstCharToUpper();

  VCExport  GString wideUpperCase();
  VCExport  GString lowerCase();
  VCExport  GString wideLowerCase();
  VCExport  GString reverse();
  VCExport  GString replaceAt(signed long pos, char in);
  VCExport  GString replaceAt(signed long pos, char* in);
  VCExport  GString replaceAt(signed long pos, GString in);
  VCExport  GSeq<GString> split(char token);
  VCExport  GSeq<GString> split(GString token);


  VCExport int asBinary(unsigned char** buf);  
  VCExport unsigned char* asBinary();
  VCExport GString toBase64();
  VCExport GString fromBase64();



};
#endif





