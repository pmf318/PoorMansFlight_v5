//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 
//
/*********************************************************************
*********************************************************************/

#ifndef _GSTRING_
#include <gstring.hpp>
#endif

#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#include <ctype.h>
#include <wctype.h>
//#define MAKE_SEC


std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const char* B64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const int B64index[256] =
{
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  62, 63, 62, 62, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 0,  0,  0,  0,  0,  0,
    0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0,  0,  0,  0,  63,
    0,  26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51
};


/*********************************************************************
*
* TODO: insert(..) still insecure
*
*********************************************************************/



/*********************************************************************
*********************************************************************/
GString GString::version()
{
    return "GString v 1.10 (C) 1999-2011 Gregor Leipelt";
}
/*********************************************************************
*********************************************************************/

void GString::tm(const char* message)
{
    if( !_iGStringDebug ) return;
    printf(" >GString[%lu]: %s\n", m_ulInstance, message);
#ifdef MAKE_VC
    _flushall();
#endif
}

//Destructor
GString::~GString(){
    tm("DTor");
    _ulGStringInstanceCount--;

    if(ptr != 0) delete [] ptr;
}

//Constructors....
/*************
#ifdef MAKE_VA
  GString::GString(IString istr){
     tm("Constructor IString...");
     ptr = 0;
     size = istr.length();
     ptr = new char[size+1];
     strcpy(ptr, istr);
     tm("Constructor IString ...Done");
  }
#endif
****************/
GString::GString(){
    m_ulInstance = _ulGStringInstanceCount++;
    tm("Default Constructor...");
    ptr = new char[1];
    ptr[0] = 0;
    //   ptr = '\0';
    size = 0;

    tm("Default Constructor Done" );
}
GString::GString( const GString &s )
{
    m_ulInstance = _ulGStringInstanceCount++;
    tm("Constructor GString...");
    ptr = 0;
    size = s.size;
    fillBuf(s.ptr, size);

    tm("Constructor GString...Done.");
}
GString::GString( const GString &s, signed long max )
{
    m_ulInstance = _ulGStringInstanceCount++;
    tm("Constructor GString...");
    ptr = 0;
    size = s.size;
    fillBuf(s.ptr, max);

    tm("Constructor GString...Done.");
}
GString::GString(const char* chrs, signed long max, GSTR_TYPE type)
{
    m_ulInstance = _ulGStringInstanceCount++;
    tm("Constructor char*...");
    ptr = 0;
    if( chrs == NULL )
    {
        ptr = new char[1];
        ptr[0] = 0;
        size = 0;
        return;
    }

    if( type == HEX )
    {
        this->toHex(chrs, max);
        return;
    }
    else if( type == BIN )
    {
        this->hexToBin(chrs, max);
        return;
    }
    else if( type == INT )
    {
        this->fromInt(chrs, max);
        return;
    }
    else if( type == DBCS )
    {
        this->fromDBCS(chrs, max);
        return;
    }

    if( max < 0 ) size = strlen(chrs);
    else size = max;
    ptr = new char[size+1];

    //strcpy(ptr, chrs);
    memcpy(ptr, chrs, size);
    ptr[size] = 0;
    tm("Constructor char*...Done.");
}

GString::GString( const unsigned char* chrs, signed long max )
{

    m_ulInstance = _ulGStringInstanceCount++;
    tm("Constructor unsigned char*...");
    ptr = 0;
    if( chrs == NULL )
    {
        ptr = new char[1];
        ptr[0] = 0;
        size = 0;
        return;
    }
    if( max < 0 ) size = strlen((char*)chrs);
    else size = max;
    fillBuf((char*) chrs, size);

    tm("Constructor unsigned char*...Done.");
}

GString::GString(const char aChar)
{
    m_ulInstance = _ulGStringInstanceCount++;
    tm("Constructor unsigned char*...");
    tm("Constructor char...");
    ptr = 0;
    size = 1;
    //Does this work?
    ptr = new char[2];
    //   memset(ptr, aChar, 1);
    ptr[0] = aChar;
    ptr[1] = 0;
    tm("Constructor char...Done.");
}
/*********************************************************************
*********************************************************************/
GString::GString(const double & aDouble, const int prec){
    m_ulInstance = _ulGStringInstanceCount++;
    tm("Constructor double...");
    ptr = 0;
    char buffer[50];
#if defined(MAKE_VC) || defined (__MINGW32__)
    int res = sprintf_s(buffer, 50, "%*E", prec, aDouble);
#else
    int res = snprintf(buffer, 50, "%*E", prec, aDouble);
#endif
    if( res < 0 ) printf("  --> ERC In Double\n");
    fillBuf(buffer, res);
    tm("Constructor double...Done.");
}

GString::GString(const long double & aDouble, int prec){
    m_ulInstance = _ulGStringInstanceCount++;
    tm("Constructor double...");
    ptr = 0;
    char buffer[50];
#if defined(MAKE_VC) || defined (__MINGW32__)
    int res = sprintf_s(buffer, 50, "%.*LE", prec, aDouble);
#else
    int res = snprintf(buffer, 50, "%.*LE", prec, aDouble);
#endif
    if( res < 0 ) printf("  --> ERC In LongDouble\n");
    fillBuf(buffer, res);
    tm("Constructor double...Done.");
}

GString::GString(const signed short & aShort){
    m_ulInstance = _ulGStringInstanceCount++;
    tm("Constructor short...");
    ptr = 0;
    char buffer [50];
#if defined(MAKE_VC) || defined (__MINGW32__)
    int res = sprintf_s(buffer, 50, "%d", aShort);
#else
    int res = snprintf(buffer, 50, "%d", aShort);
#endif
    if( res < 0 ) printf("  --> ERC In short\n");
    fillBuf(buffer, res);

    tm("Constructor short...Done.");
}
GString::GString(const unsigned short & aShort){
    m_ulInstance = _ulGStringInstanceCount++;
    tm("Constructor unsigned short...");
    ptr = 0;
    char buffer [50];
#if defined(MAKE_VC) || defined (__MINGW32__)
    int res = sprintf_s(buffer, 50, "%u", aShort);
#else
    int res = snprintf(buffer, 50, "%u", aShort);
#endif
    if( res < 0 ) printf("  --> ERC In unsigned short\n");
    fillBuf(buffer, res);

    tm("Constructor unsigned short...Done.");
}

GString::GString(const signed int & aInt){
    m_ulInstance = _ulGStringInstanceCount++;
    tm("Constructor int...");
    ptr = 0;
    char buffer [50];
#if defined(MAKE_VC) || defined (__MINGW32__)
    int res = sprintf_s(buffer, 50, "%d", aInt);
#else
    int res = snprintf(buffer, 50, "%d", aInt);
#endif
    if( res < 0 ) printf("  --> ERC In int\n");
    fillBuf(buffer, res);
    tm("Constructor int...Done.");
}

GString::GString(const unsigned int & aInt){
    m_ulInstance = _ulGStringInstanceCount++;
    tm("Constructor unsigned int...");
    ptr = 0;
    char buffer [50];
#if defined(MAKE_VC) || defined(__MINGW32__)
    int res = sprintf_s(buffer, 50, "%u", aInt);
#else
    int res = snprintf(buffer, 50, "%u", aInt);
#endif
    if( res < 0 ) printf("  --> ERC In unsigned int\n");
    fillBuf(buffer, res);
    tm("Constructor unsigned int...Done.");
}
GString::GString(const unsigned long & aLong){
    m_ulInstance = _ulGStringInstanceCount++;
    tm("Constructor long...");
    ptr = 0;
    char buffer [200];
#if defined(MAKE_VC) || defined(__MINGW32__)
    int res = sprintf_s(buffer, 200, "%u", aLong);
#else
    int res = snprintf(buffer, 200, "%lu", aLong);
#endif
    if( res < 0 ) printf("  --> ERC In unsigned long\n");
    //printf("GString ctor signed long: size %i, as int: %i, as lu: %lu\n", size, aLong, aLong);
    fillBuf(buffer, res);

    tm("Constructor long...Done.");
}

GString::GString(const signed long & aLong){
    m_ulInstance = _ulGStringInstanceCount++;
    tm("Constructor long...");
    ptr = 0;
    char buffer [50];
    //size = (signed long) sprintf(buffer, "%ld", aLong);
#if defined(MAKE_VC) || defined(__MINGW32__)
    int res  = (signed long) sprintf_s(buffer, 50, "%ld", aLong);
#else
    int res  = (signed long) snprintf(buffer, 50, "%ld", aLong);
#endif
    if( res < 0 ) printf("  --> ERC In long\n");
    fillBuf(buffer, res);
    tm("Constructor long...Done.");
}

GString::GString(const  float & aFloat){
    m_ulInstance = _ulGStringInstanceCount++;
    tm("Constructor float...");
    ptr = 0;
    char buffer [50];
#if defined(MAKE_VC) || defined(__MINGW32__)
    int res = sprintf_s(buffer, 50, "%f", aFloat);
#else
    int res = snprintf(buffer, 50, "%f", aFloat);
#endif
    if( res < 0 ) printf("  --> ERC In float\n");
    fillBuf(buffer, res);
    tm("Constructor float...Done.");
}

GString::GString(std::string s)
{
    if( s.size() == 0 )
    {
        ptr = new char[1];
        ptr[0] = 0;
        size = 0;
        return;
    }
    else ptr = new char[s.size() + 1];
    size = s.size();
    ptr[size] = 0;
    strcpy(ptr, s.c_str());
}

#if defined(MAKE_VC) || defined(__MINGW32__)
GString::GString(const int64_t & aBigInt)
{
    m_ulInstance = _ulGStringInstanceCount++;
    tm("Constructor bigInt...");
    ptr = 0;
    char buffer [21];
    int res = sprintf_s(buffer, 21, "%I64d", aBigInt);
    if( res < 0 ) printf("  --> ERC In bigInt");
    fillBuf(buffer, res);
    tm("Constructor bigInt...Done.");
}
GString::GString(const uint64_t & aBigInt)
{
    m_ulInstance = _ulGStringInstanceCount++;
    tm("Constructor bigInt...");
    ptr = 0;
    char buffer [21];
    int res = sprintf_s(buffer, 21, "%I64u", aBigInt);
    if( res < 0 ) printf("  --> ERC In bigInt");
    fillBuf(buffer, res);
    tm("Constructor bigInt...Done.");
}

#endif

/*****************************************************************************************************************************/
/******************************************** OPERATORS *****************************************************************/
/*****************************************************************************************************************************/
/*
void * GString::memcpy(const GString trg, const char* src, signed long n)
{
}
*/
GString &GString::operator=(const char * chrs)
{
    //m_ulInstance = _ulGStringInstanceCount++;
    tm("Operator = char*...");
    if( !chrs ) printf("++++++++ BAD PTR\n");
    if( !chrs ) chrs = "";
    if( ptr != chrs )
    {
        if( ptr != 0 ) delete [] ptr;
        size = strlen(chrs);
        //!!ptr = new char[size+1];
        fillBuf( (char*) chrs, size );
        ptr[size] = 0;
    }
    tm("Operator = char*...Done.");
    return *this;
}

GString &GString::operator=(const char chr)
{
    //!!MAKE_SEC
    m_ulInstance = _ulGStringInstanceCount++;
    tm("Operator = char...");
    if (ptr != 0 ) delete [] ptr;
    ptr = new char[2];
    ptr[0] = chr;
    ptr[1] = 0;
    size = 1;
    tm("Operator = char...Done.");
    return *this;
}

GString &GString::operator=(const GString gStr)
{
    m_ulInstance = _ulGStringInstanceCount++;
    tm("Operator = GString...");
    if( ptr != gStr.ptr )
    {
        if( ptr != 0 ) delete [] ptr;
        /*****
     size = strlen(gStr.ptr);
     ptr = new char[size+1];
     strcpy(ptr, gStr.ptr);
      *****/
        fillBuf(gStr.ptr);
    }
    tm("Operator = GString...Done.");
    return *this;
}

GString & GString::operator+=(const GString x){
    tm("Operator += GString...");
    //How about this = this+x ????
    catBuf( x.ptr );
    tm("Operator += GString...Done.");
    return *this;
}
GString & GString::operator+=(const char* aString){
    tm("Operator += char*...");

    catBuf((char*)aString);
    tm("Operator += char*...Done.");
    return *this;
}


GString GString::operator+(const char* x){
    tm("Operator + char*...");
    return addBuf( (char*) x );
}
GString GString::operator+(const GString x){
    tm("Operator + GString ...");
    return addBuf( x.ptr );
}

char& GString::operator [] (unsigned long pos){
    tm("Operator [signed long] ...");
    if( pos < 1 ) pos = 1;
    if( pos > strlen(ptr) ) pos = strlen(ptr);
    tm("Operator [] ...Done.");
    return ptr[pos -1];
}
char& GString::operator [] (int pos){
    tm("Operator [int] ...");
    if( pos < 1 ) pos = 1;
    if( pos > (signed)strlen(ptr) ) pos = strlen(ptr);
    tm("Operator [] ...Done.");
    return ptr[pos -1];
}

//std::istream& operator>>(std::istream& in, GString& s)
//{
//    std::string temp;
//    in >> temp;
//    s.size = temp.length() + 1;
//    s.ptr = new char[s.size];
//    s.ptr[temp.length()] = '\0';
//    strncpy(s.ptr, &temp[0], s.size);
//    return in;
//}

GString& GString::operator>>(GString& in)
{
    if( this->occurrencesOf(' ') == 0 )
    {
        in = *this;
        return *this;
    }
    in = this->subString(1, this->indexOf(' ')-1);
    *this = this->subString(this->indexOf(' ')+1, this->length()).strip();
    return *this;

//    std::string temp;
//    std::istringstream is(this->ptr);
//    is.getline(a.s, 250);
//    is >> temp;
//    std::cout << temp;
//    in.size = temp.length() + 1;
//    in.ptr = new char[in.size];
//    in.ptr[temp.length()] = '\0';
//    strncpy(in.ptr, &temp[0], in.size);
//    return in;
}

std::ostream& operator<< (std::ostream& os, GString& s)
{
    os << s.ptr << "\n";
    return os;
}

//******************* Members *****************************
int GString::asInt(){
    tm("Member asInt()");
    if( ptr == 0 ) return 0;
    return atoi(ptr);
}

signed long GString::asLong(){
    tm("Member asLong()");
    if( ptr == 0 ) return 0;
    return atol(ptr);
}

int64_t GString::asLongLong(){
    tm("Member asLong()");
    if( ptr == 0 ) return 0;
#if defined(MAKE_VC) || defined (__MINGW32__)
    return _atoi64(ptr);
#else
    return atoll(ptr);
#endif
}

char* GString::asChar(){
    tm("Member asChar()");
    return ptr;
}

unsigned long GString::length() const{
    return strlen(ptr);
}
void GString::toHex(const char* chrs, signed long max)
{

    tm("::toHex start");
    if( ptr != 0 ) delete [] ptr; //Check this!
    ptr = new char[2*max+1];
    size = 2*max;
    ptr[size] = 0;
    //memset(ptr, '0', size);

    int c;
    for(int i=0; i<max; i++)
    {
        c = chrs[i];
        //instead of "(unsigned char)" we could use "%hhX": sprintf(tmp+i*2, "%hhX", (long)ptr[i]);
        if( c == 12 ) sprintf(ptr+i*2, "%s", "0C");
        else if( c == 10 ) sprintf(ptr+i*2,"%s", "0A");
        else if( c == 13 ) sprintf(ptr+i*2,"%s", "0D");
        else if( c == 0 ) sprintf(ptr+i*2, "%s", "00");
        else if( !c ) sprintf(ptr+i*2, "%s", "00");
        else sprintf(ptr+i*2, "%02lX", (long)(unsigned char)chrs[i]);
    }

    tm("::toHex end");
}
void GString::fromDBCS(const char *chrs, signed long max)
{
    if( ptr != 0 ) delete [] ptr;
    size = max;
    ptr = new char[size+1];
    ptr[size] = 0;
    int ic = 0;

    //printf("String::fromDBCS, lng: %i\n", max);
    for(int i = 0; i < max; ++i)
    {
        ic = chrs[i];
        /*
        if( ic == 0 )
        {
            i++;
            ptr[j] = chrs[i];
            j++;
        }
        */

        //printf("String::fromDBCS, i: %i, ic: %i\n", i, ic);
        if( ic < 127*265 ) ptr[i] = (signed char) ic / 256;
        else sprintf( ptr+i, "%x", chrs[i]) ;
    }

}

void GString::fromInt(const char *chrs, signed long max)
{
    if( ptr != 0 ) delete [] ptr;
    size = max;
    ptr = new char[size+1];
    ptr[size] = 0;

    for(int i = 0; i < max; ++i)
    {
        //if( i >= strlen(chrs)) break;
        ptr[i] = (char) chrs[i];
    }

}

/***************************************
 * Convert HEX data (i.e. "491FC7D8CC8CE74192243C89FCBBC194") to
 * binary data.
 * Code is not beautiful, but works. For now.
 * ToDo: Check LEN vs strlen(chrs), either err or fill ptr to LEN
 *
 **************************************/
void GString::hexToBin(const char *chrs, signed long len)
{
    tm("::hexToBin start");
    if( !chrs ) return;
    if( !chrs[0] ) return;
    if( len % 2 ) return;

    if( ptr != 0 ) delete [] ptr;
    size = len/2;
    ptr = new char[size+1];
    ptr[size] = 0;

    short unsigned int ch;
    for(int i = 0 ; i < len/2; ++i )
    {
        sscanf((char*)chrs, "%2hx", &ch );
        chrs += 2;
        ptr[i] = ch;

    }
    tm("::hexToBin end ");
    return;
}
// Private helper function:

int GString::hex2int(char c)
{
    c = toupper(c);

    if (c >= '0' && c <= '9') return      c - '0';
    if (c >= 'A' && c <= 'F') return 10 + c - 'A';
    if (c >= 'a' && c <= 'f') return 10 + c - 'a';
    return c;
}
void GString::sayIt(){
    printf("%s\n", ptr);
}

signed long GString::indexOf(const GString x, unsigned long startPos){
    tm("Member indexOf( GString )");
    if( ptr == 0 ) return 0;
    if( startPos > strlen(ptr)) return 0;
    char *dest;
    unsigned long r;
    dest = strstr(ptr+startPos, x.ptr);
    if( dest == NULL ){ tm("indexOf: Ret 0"); return 0;}
    else {
        r = dest-ptr+1;
        //tm("indexOf: Ret "+GString(r));
        return r;
    }
}
signed long GString::indexOf(const char aChar, unsigned long startPos){
    tm("Member indexOf( char )");
    if( ptr == 0 ) return 0;
    if( startPos > strlen(ptr)) return 0;
    char * dest;
    dest = strchr(ptr+startPos, aChar);
    if( dest == NULL ) return 0;
    else return (dest - ptr + 1);
}

signed long GString::indexOf(const char* x, unsigned long startPos){
    if( ptr == 0 ) return 0;
    tm("Member indexOf( char* )");
    char *dest;
    if( startPos > strlen(ptr)) return 0;
    dest = strstr(ptr+startPos, x);
    if( dest == NULL ) return 0;
    else return( dest - ptr + 1);
}

GString GString::leftJustify(unsigned long count, char padChar){
    tm("Member leftJustify...");
    if( count == strlen(ptr) ) return *this;

    size = strlen(ptr);

    char* tmp = new char[count+1];
    if( count < size ){
        strncpy(tmp, ptr, count);
    }
    else{
        memset(tmp, padChar, count);
        memcpy(tmp, ptr, size);
    }
    tmp[count]=0;
    if( ptr != 0 ) delete [] ptr;
    size = strlen(tmp);
    //!!ptr = new char[size+1];
    //!!strcpy(ptr, tmp);
    fillBuf(tmp);
    delete [] tmp;
    tm("Member leftJustify...Done.");
    return *this;
}
GString GString::rightJustify(unsigned long count, char padChar){
    tm("Member rightJustify...");
    if( count == strlen(ptr) ) return *this;
    char* tmp = new char[count+1];
    size = strlen(ptr);
    if( count < size ){
        memcpy(tmp, ptr+size-count, count);
    }
    else {
        memset(tmp, padChar, count);
        memcpy(tmp+count-size, ptr, size);
    }
    tmp[count]=0;
    if( ptr != 0 ) delete [] ptr;
    size = strlen(tmp);
    //!!ptr = new char[size+1];
    //!!strcpy(ptr, tmp);
    fillBuf(tmp);
    delete [] tmp;
    tm("Member rightJustify...Done.");
    return *this;
}



unsigned long GString::lastIndexOf(const char aChar)
{
    tm("Member lastIndexOf(char)");
    if( ptr == 0 ) return 0;
    char* p;
    p = strrchr(ptr, aChar);
    if( p != NULL ) return p - ptr +1;
    else return 0;
}

unsigned long GString::lastIndexOf(const char *aStr)
{
    tm("Member lastIndexOf(char*)...");
    if( ptr == 0 ) return 0;
    char* tmp = new char[strlen(ptr)+1];
    strcpy(tmp, ptr);
    size = strlen(ptr);
    unsigned long i, pos = 0;
    for( i = 0; i < size; ++ i ){
        if( !memcmp( tmp+i, aStr, strlen(aStr)) ) pos = i + 1;
    }
    delete [] tmp;
    tm("Member lastIndexOf(char*)...Done.");
    return pos;
}
unsigned long GString::lastIndexOf(GString ipstr)
{
    tm("Member lastIndexOf(GString)");
    if( ptr == 0 ) return 0;
    return lastIndexOf((char*)ipstr);
}

GString & GString::remove(unsigned long startPos, signed long count){
    tm("Member remove...");
    if( startPos == 0 ) startPos = 1;
    size = strlen(ptr);
    if( startPos > size ) return *this;
    if( count + startPos - 1 > size ) count = size - startPos + 1;
    char* tmp = new char[size-count+1];
    memcpy( tmp, ptr, startPos - 1);
    memcpy( tmp+startPos-1, ptr+count+startPos-1, size-startPos-count+1);

    tmp[size-count] = 0;
    delete [] ptr;
    size = strlen(tmp);
    ptr = new char[size+1];
    strcpy(ptr, tmp);
    ptr[size] = 0;
    delete [] tmp;
    tm("Member remove...Done.");
    return *this;
}
GString & GString::removeAll(char aChar)
{
    char* tmp = new char[size+1];
    unsigned long i, j = 0;
    for( i = 0; i < size; ++ i )
    {
        if( ptr[i] != aChar )
        {
            tmp[j] = ptr[i];
            j++;
        }
    }
    size = j;
    tmp[size] = 0;
    delete [] ptr;
    ptr = new char[size+1];
    strcpy(ptr, tmp);
    ptr[size] = 0;
    delete [] tmp;
    return *this;
}

GString & GString::removeButOne(char aChar)
{
    char* tmp = new char[size+1];
    unsigned long i, j = 0;
    for( i = 0; i < size; ++ i )
    {
        if( i+1 < size && ptr[i] == aChar && ptr[i+1] == aChar ) continue;
        else
        {
            tmp[j] = ptr[i];
            j++;
        }
    }
    size = j;
    tmp[size] = 0;
    delete [] ptr;
    ptr = new char[size+1];
    strcpy(ptr, tmp);
    ptr[size] = 0;
    delete [] tmp;
    return *this;
}

GString GString::change(GString oldStr, GString newStr)
{
    GString ret, org;
    org = GString(ptr);
    while( org.occurrencesOf( oldStr ) > 0 )
    {
        ret += org.subString(1, org.indexOf(oldStr) - 1 );
        ret += newStr;
        org.remove( 1, org.indexOf(oldStr)+oldStr.length()-1 );
    }
    ret += org;    
    return ret;
}

GString GString::change(char oldChar, char newChar)
{
    char* tmp = new char[size+1];
    unsigned long i, j = 0;
    for( i = 0; i < size; ++ i )
    {
        if( ptr[i] == oldChar ) tmp[j] = newChar;
        else tmp[j] = ptr[i];
        j++;
    }
    size = j;
    tmp[size] = 0;
    delete [] ptr;
    ptr = new char[size+1];
    strcpy(ptr, tmp);
    ptr[size] = 0;
    delete [] tmp;
    return *this;
}

GString GString::changeProto(GString oldStr, GString newStr)
{
    GString ret, org, tmpOrg, tmpNew, tmpOld;
    org = GString(ptr);
    tmpOrg = org.upperCase();
    tmpNew = newStr.upperCase();
    tmpOld = oldStr.upperCase();
    int idx;
    while( tmpOrg.occurrencesOf( tmpOld ) > 0 )
    {
        ret += org.subString(1, tmpOrg.indexOf(tmpOld) - 1 );
        ret += newStr;
        idx = tmpOrg.indexOf(tmpOld)+tmpOld.length()-1;
        tmpOrg.remove( 1, idx );
        org.remove( 1, idx );
    }
    ret += org;
    return ret;
}

GString GString::stripLeading(char aChar)
{
    /* Easier:
     while ptr[0] == aChar) ptr++;
     BUT: freeing ptr requires the original
     pointer, free[] will crash if the pointer
     has been modified.
    */
    tm("Member stripLeading(char)...");
    char* tmp = new char[strlen(ptr)+1];
    unsigned long count = 0;
    while( ptr[count] == aChar ){ ++count; };
    memcpy( tmp, ptr+count, size-count );
    tmp[ size - count ] = 0;

    delete [] ptr;
    size = strlen(tmp);
    ptr = new char[size+1];
    strcpy(ptr, tmp);
    delete [] tmp;
    tm("Member stripLeading(char)...Done.");
    return *this;
}

GString GString::stripLeading(GString str)
{
    if( str.length() > strlen(ptr) ) return *this;
    GString tmp = GString(ptr);
    while( tmp.indexOf(str) == 1 )
    {
        tmp = tmp.subString(str.length()+1, tmp.length()-str.length());
    }
    delete [] ptr;
    size = tmp.length();
    ptr = new char[size+1];
    strcpy(ptr, tmp.ptr);
    return *this;
}

GString GString::stripTrailing(char aChar){
    tm("Member stripTrailing(char)...");
    char* tmp = new char[strlen(ptr)+1];
    signed long count = strlen(ptr)-1;
    size = strlen(ptr);
    while( ptr[count] == aChar )
    {
        count--;
        if( count < 0 ) break;
    };
    memcpy( tmp, ptr, count+1 );
    tmp[ count+1 ] = 0;

    delete [] ptr;
    size = strlen(tmp);
    ptr = new char[size+1];
    strcpy(ptr, tmp);
    delete [] tmp;
    tm("Member stripTrailing(char)...Done.");
    return *this;
}
GString GString::stripTrailing(GString str){
    tm("Member stripTrailing(char *)...");
    signed long pos = strlen(ptr)-str.length();
    if( pos < 0 ) return *this;
    if( str.length() == 0 ) return *this;
    if( str.length() == 1 ) return stripTrailing(str[0]);
//    printf("Strip %s from %s, read: %i\n", (char*)str, (char*) ptr, memcmp(ptr+pos, str, str.length()));
//    printf("pos: %i, ptr+pos: %s, str: %s, str.length(): %i\n", pos, ptr+pos, (char*)str, str.length());
    while( !memcmp(ptr+pos, str, str.length())  )
    {
        ptr[pos] = '\0';
        pos = strlen(ptr)-str.length();
    }
    size = strlen(ptr);
    tm("Member stripTrailing(char *)...Done.");

    //In case of emergency: Something like:   ptr = p - size;
    return *this;
}

GString GString::strip(const char* aStr)
{
    tm("Member strip(char*)...");
    unsigned long lng, tmpLng;
    lng = strlen(aStr);

    if( lng > strlen(ptr) )return *this;

    GString str = GString(aStr);
    if( str == *this ) return "";


    GString tmp = GString(ptr);
    while( tmp.subString(1, lng) == str ){
        tmp.remove(1, lng);
    }
    tmpLng = tmp.length();
    //!!
    //************************
    while (tmp.subString(tmpLng-lng+1, lng) == aStr){
        tmp.remove(tmpLng-lng+1, lng);
        tmpLng = tmp.length();
    }
    tm("Member strip(char*)...Done.");
    //******************
    return tmp;
}
GString GString::strip(char aChar){
    tm("Member strip(char)...");
    //cout << "**PTR: "<<ptr<<endl;
    signed long upper, lower;
    if( strlen(ptr) == 0 || ptr == 0 ) return *this;
    upper = strlen(ptr)-1;
    lower = 0;
    while( ptr[upper] == aChar )
    {
        upper--;
        if( upper < 0 ) break;
    };
    while( ptr[lower] == aChar ){ lower++;};

    if( lower == (signed long) strlen(ptr) )
    {
        delete [] ptr;
        ptr = new char[1];
        ptr[0] = 0;
        size = 0;
        return *this;
    }

    if( upper-lower+1 < 0 ) return *this;

    char* tmp = new char[strlen(ptr)+1];
    char* src = new char[strlen(ptr)+1];
    strcpy(src, ptr);

    memcpy( tmp, ptr+lower, upper-lower +1 );
    tmp[ upper - lower + 1 ] = 0;

    delete [] ptr;
    size = strlen(tmp);
    ptr = new char[size+1];
    strcpy(ptr, tmp);
    delete [] tmp;
    delete [] src;
    tm("Member strip(char)...Done.");
    return *this;
}
GString  GString::subString(signed long startPos, signed long count, char padChar){
    tm("Member subString...Start: "+GString(startPos)+", count: "+GString(count));
    GString ret;
    if( startPos < 1 ) startPos = 1;
    if( startPos > (signed)strlen(ptr) ) return (ret = "");
    if( count <= 0 ) return GString("");
    //   if( count < 0 ) return GString(ptr);
    char * tmp = new char[count+1];
    memset(tmp, padChar, count);
    if( (startPos + count ) > (signed)strlen(ptr) ) count = strlen(ptr) - startPos + 1;
    memcpy(tmp, ptr+startPos-1, count);
    tmp[count] = 0;
    ret = tmp;
    delete [] tmp;
    tm("Member subString...Done, return ->"+ret+"<-");
    return ret;
}

unsigned long GString::occurrencesOf(char chr)
{
    tm("Member occurrencesOf(char)...");

    char * org = ptr;
    unsigned long i, cnt;
    for (i=0; ptr[i]; ptr[i]==chr ? i++ : *ptr++);
    ptr = org;
    return i;

    cnt = 0;
    for(i=0; i<strlen(ptr); ++i)
    {
        if( ptr[i] == chr ) cnt++;
    }
    tm("Member occurrencesOf(char)...Done.");
    return cnt;
}
unsigned long GString::occurrencesOf(const char* str)
{
    tm("Member occurrencesOf(char*)...");
    size = strlen(ptr);
    if( 0 == size ) return 0;
    if( strlen(str) > size ) return 0;

    char* tmp = new char[strlen(ptr)+1];
    strcpy(tmp, ptr);
    tmp[size] = '\0';
    unsigned long i, count;
    count = 0;


    //Seriously not good: memcmp on ''
    //for(i=0; i<strlen(ptr); ++i)
    //This is better:
    for( i=0; i <= size - strlen(str); ++i )
    {
        if( !memcmp( tmp+i, str, strlen(str)) ) ++count;
    }
    delete [] tmp;
    tm("Member occurrencesOf(char*)...Done.");
    return count;
}

unsigned long GString::occurrencesOf(GString ipstr)
{
    tm("Member occurrencesOf(GString)");
    return occurrencesOf((char*) ipstr);
}

GString GString::insert(char* str, signed long index)
{
    tm("Member insert...");
    if( index < 1 ) index = 1;
    index -= 1;
    if( index > (signed)strlen(ptr) ) return *this;
    char * tmp = new char[strlen(ptr)+strlen(str)+1];
    memcpy(tmp, ptr, index);
    memcpy(tmp+index, str, strlen(str));
    memcpy(tmp+index+strlen(str), ptr+index, strlen(ptr)-index);
    tmp[strlen(ptr)+strlen(str)] = 0;
    if( ptr ) delete[] ptr;
    ptr = new char[strlen(tmp)+1];
    strcpy( ptr, tmp );
    size = strlen(ptr);
    ptr[size] = 0;
    delete[] tmp;
    tm("Member insert...Done.");
    return *this;
}

GString GString::translate(char in, char out)
{
    tm("Member translate(char, char)...");
    size = strlen(ptr);
    char * p;
    for ( p = ptr; *p != '\0'; p++ ) if ( *p == in )  *p = out;
    ptr = p - size;
    tm("Member translate()...Done.");
    return *this;
}
GString GString::translate(GString in, GString out)
{
    GString ret, temp;
    temp = *this;
    signed long count = temp.occurrencesOf(in);
    if( count <= 0 ) return *this;

    for(long i = 1; i <= count; ++i )
    {
        ret += temp.subString(1, temp.indexOf(in)-1) + out;
        temp = temp.remove(1, temp.indexOf(in)+in.length()-1);
    }
    return ret;
}
int GString::isDigits()
{
    tm("Member isDigits()");
    char * end;
    strtoul(*this, &end, 10);
    if( *end == 0 ) return 1;
    else return 0;
}



int GString::isSomewhatPrintable(int chr2Oem)
{
    unsigned long i = 0;
    const unsigned char * bytes= (unsigned char *)ptr;
    int rc = 0;


    if( chr2Oem )
    {
#ifdef MAKE_VC
        CharToOem(ptr, ptr);
#endif
    }
    /*
    if( chr2Oem )
    {
        CharToOem(ptr, ptr);
        size = strlen(ptr);
        bytes = new char[size+1];
        memcpy(bytes, ptr, size);
        bytes[size] = 0;
        CharToOem(bytes, bytes);
    }
    else bytes = (unsigned char *)ptr;

    GString printable = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]\]^`abcdefghijklmnopqrstuvwxyz{|}~ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõöøùúûüýþÿ";

    while (i < size)
    {
        if( !printable.occurrencesOf(ptr[i])) return 0;
        i++;
    }
    return 1;

    i = 0;
    */
    while (i < size)
    {
        //printf("pos %i, bytes: %i\n", i, bytes[i]);
        if (bytes[i] <= 0x1F)  rc = 1;
        else if (bytes[i] == 0x7F)  rc = 1;
        else if (bytes[i] >= 0xA6 &&  bytes[i] <= 0xB4 )  rc = 1;
        else if (bytes[i] >= 0xB9 &&  bytes[i] <= 0xBC )  rc = 1;
        else if (bytes[i] >= 0xBF &&  bytes[i] <= 0xC5 )  rc = 1;
        else if (bytes[i] >= 0xC8 &&  bytes[i] <= 0xCF )  rc = 1;
        else if (bytes[i] >= 0xD9 &&  bytes[i] <= 0xDF )  rc = 1;
        else if (bytes[i] >= 0xEE)  rc = 1;
        if( rc ) break;
        i++;
    }
    if( chr2Oem )
    {
#ifdef MAKE_VC
        OemToChar(ptr, ptr);
#endif
    }
    return rc == 1 ? 0 : 1;
}

bool GString::isProbablyUtf8()
{
    {
        if (!ptr) return true;

        const unsigned char * bytes = (const unsigned char *)ptr;
        unsigned int cp;
        int num;

        while (*bytes != 0x00)
        {
            if ((*bytes & 0x80) == 0x00)
            {
                // U+0000 to U+007F
                cp = (*bytes & 0x7F);
                num = 1;
            }
            else if ((*bytes & 0xE0) == 0xC0)
            {
                // U+0080 to U+07FF
                cp = (*bytes & 0x1F);
                num = 2;
            }
            else if ((*bytes & 0xF0) == 0xE0)
            {
                // U+0800 to U+FFFF
                cp = (*bytes & 0x0F);
                num = 3;
            }
            else if ((*bytes & 0xF8) == 0xF0)
            {
                // U+10000 to U+10FFFF
                cp = (*bytes & 0x07);
                num = 4;
            }
            else
                return false;

            bytes += 1;
            for (int i = 1; i < num; ++i)
            {
                if ((*bytes & 0xC0) != 0x80)
                    return false;
                cp = (cp << 6) | (*bytes & 0x3F);
                bytes += 1;
            }

            if ((cp > 0x10FFFF) ||
                    ((cp >= 0xD800) && (cp <= 0xDFFF)) ||
                    ((cp <= 0x007F) && (num != 1)) ||
                    ((cp >= 0x0080) && (cp <= 0x07FF) && (num != 2)) ||
                    ((cp >= 0x0800) && (cp <= 0xFFFF) && (num != 3)) ||
                    ((cp >= 0x10000) && (cp <= 0x1FFFFF) && (num != 4)))
                return false;
        }

        return true;
    }
}

int GString::isUTF8()
{
    unsigned long i = 0;
    int continBytes = 0;
    const unsigned char * bytes = (unsigned char *)ptr;
    //    printf("::isUTF8, ptr: %s\n", ptr);

    while (i < size)
    {

        if (bytes[i] == 0x0D)  return 0;
        if (bytes[i] == 0x0A)  return 0;
        if (bytes[i] <= 0x7F) continBytes = 0;

        else if (bytes[i] >= 0xC0 && bytes[i] <= 0xDF ) continBytes = 1;
        else if (bytes[i] >= 0xE0 && bytes[i] <= 0xEF ) continBytes = 2;
        else if (bytes[i] >= 0xF0 && bytes[i] <= 0xF4 ) continBytes = 3;
        else return 0;
        i += 1;
        while (i < size && continBytes > 0  && bytes[i] >= 0x80 && bytes[i] <= 0xBF)
        {
            i += 1;
            continBytes -= 1;
        }
        if (continBytes != 0)
        {
            return 0;
        }
    }
    return 1;
}

GString GString::upperOrLowerCase(int mode)
{
    setlocale(LC_ALL, "");
    int i = 0;
    wchar_t c;
    size_t cSize = mbstowcs(NULL, ptr, 0)+1;

    wchar_t* wc = new wchar_t[cSize];
    mbstowcs( wc, ptr, cSize );
    while (wc[i])
    {
        c = wc[i];
        if(mode) wc[i] = towupper(c);
        else wc[i] = towlower(c);
        i++;
    }


    size = strlen(ptr);
    delete [] ptr;

    ptr = new char[size+1];
    ptr[size] = 0;

    memset( ptr, 0, size + 1);

    wcstombs(ptr, wc, size);
    delete[] wc;
    return *this;
}

GString GString::wideUpperCase()
{
    return upperOrLowerCase(1);
}

GString GString::wideLowerCase()
{
    return upperOrLowerCase(0);
}

GString GString::firstCharToUpper()
{
    tm("Member firstCharToUpper()...");
    size = strlen(ptr);
    if( 0 == size ) return *this;
    char * p, *s;
    s = ptr;
    unsigned long i;
    for ( p = ptr, i = 0; *p != '\0' && i < size; p++, i++ )
    {
         i == 0 ? *p = toupper( *p ) : *p = tolower( *p );
    }
    ptr = s;
    tm("Member firstCharToUpper()...Done.");
    return *this;
}

GString GString::upperCase()
{
    tm("Member upperCase()...");

    size = strlen(ptr);
    if( 0 == size ) return *this;
    char * p, *s;
    s = ptr;
    unsigned long i;
    for ( p = ptr, i = 0; *p != '\0' && i < size; p++, i++ ) *p = toupper( *p );
    ptr = s;
    tm("Member upperCase()...Done.");
    return *this;
}
GString GString::lowerCase()
{
    tm("Member lowerCase()...");
    size = strlen(ptr);
    if( 0 == size ) return *this;
    char * p;
    for ( p = ptr; *p != '\0'; p++ ) *p = tolower( *p );
    ptr = p - size;
    tm("Member lowerCase()...Done.");
    return *this;
}
GString GString::reverse()
{
    tm("Member reverse()...");
    //   strrev(ptr);
    unsigned long i, size;
    size = strlen(ptr);
    if( 0 == size ) return *this;

    char * tmp = new char[size+1];
    for( i = 0; i < size; ++ i )
    {
        tmp [i] = ptr[size - i - 1];
    }
    memcpy(ptr, tmp, size);
    ptr[size] = 0;
    delete []tmp;
    tm("Member reverse()...Done.");
    return *this;
}
GString GString::replaceAt(signed long pos, char in)
{
    if( pos < 0 || pos > (signed)strlen(ptr) ) return *this;
    if( pos == 0 ) pos = 1;
    size = strlen(ptr);
    memset( ptr+pos-1, in, 1 );
    ptr[size] = 0;
    return *this;
}
GString GString::replaceAt(signed long pos, char* in)
{
    if( pos < 0 || pos > (signed)strlen(ptr) ) return *this;
    if( pos == 0 ) pos = 1;
    signed long size;
    size = strlen(ptr);
    memcpy( ptr+pos-1, in, strlen(in) );
    ptr[size] = 0;
    return *this;
}
GString GString::replaceAt(signed long pos, GString in)
{
    return replaceAt(pos, (char*)in);
}
int GString::asBinary(unsigned char** buf)
{
    tm("::asBinary start");
    size = strlen(ptr);
    char * start = ptr;
    if( size % 2 ) return 0;

    *buf = new unsigned char[size/2+1];
    //(*buf)[size/2+1] = 0;
    short unsigned int ch;
    for(unsigned long i = 0 ; i < size; ++i )
    {
        //printf("asBinary buf loopStart: %i\n", i);
        sscanf(ptr, "%2hx", &ch );
        //printf("asBinary buf loop: %i (1)\n", i);
        ptr += 2;
        //printf("asBinary buf loop: %i (2)\n", i);
        (*buf)[i] = ch;
        //printf("asBinary buf loop: %i (3)\n", i);
    }
    printf("asBinary buf done\n");
    ptr = start;
    tm("::asBinary end ");
    return size/2;
}


unsigned char* GString::asBinary()
{
    tm("::asBinary start");
    size = strlen(ptr);
    char * start = ptr;

    unsigned char *buf = new unsigned char[size/2+1];
    short unsigned int ch;
    for(unsigned i = 0 ; i < size; ++i )
    {
        sscanf(ptr, "%2hx", &ch );
        ptr += 2;
        buf[i] = ch;
    }
    ptr = start;
    tm("::asBinary end ");
    return buf;
}


/***************************************************************************************
*
*   Private methods
*
***************************************************************************************/
int GString::catBuf(char* in)
{
    //printf("CatBuf start, ptr: %s, in: %s\n", ptr, in);
    size = strlen(ptr);
#ifdef MAKE_SEC
    if( size != 0 && size + 1 <= 0 ) {printf("ALERT\n"); size = MAX_GSTR_LNG;}
#endif

    char * org = new char[size+1];
#ifdef MAKE_SEC
    strncpy(org, ptr, size);
    org[size] = 0;
    //printf("Org: %s\n", org);
    if( ptr != 0 ) delete [] ptr;
    size += strlen(in);
    if( size != 0 && size + 1 <= 0 ) {printf("ALERT\n"); size = MAX_GSTR_LNG;}
    ptr = new char[size+1];
    strncpy(ptr, org, strlen(org));
    ptr[strlen(org)] = 0;
    //printf("ptr1: %s\n", ptr);
    strncat( ptr, in, size - strlen(ptr));
    //printf("ptr2: %s\n", ptr);
#else
    strcpy(org, ptr);
    if( ptr != 0 ) delete [] ptr;
    size += strlen(in)+1;
    ptr = new char[size];
    strcpy(ptr, org);
    strcat( ptr, in);
#endif
    delete [] org;
    //printf("CatBuf End\n");
    return 0;
}


int GString::fillBuf(char* in)
{
    size = strlen(in);
    //printf("fillBuf, in: %s, lng: %i\n", in, size);

#ifdef MAKE_SEC
    if( size != 0 && size + 1 <= 0 ) {printf("ALERT\n"); size = MAX_GSTR_LNG;}
#endif

    ptr = new char[size+1];
#ifdef MAKE_SEC
    strncpy(ptr, in, size);
    ptr[size] = 0;
#else
    strcpy(ptr, in);
    ptr[size] = 0;
#endif
    return 0;
}

int GString::fillBuf(char* in, signed long lng)
{
    size = lng;
//    printf("fillBuf2, in: %s, lng: %i\n", in, size);

#ifdef MAKE_SEC
    if( size != 0 && size + 1 <= 0 ) {printf("ALERT\n"); size = MAX_GSTR_LNG;}
#endif

    ptr = new char[size+1];
#ifdef MAKE_SEC
    strncpy(ptr, in, size);
    ptr[size] = 0;
#else
    strcpy(ptr, in);
    ptr[size] = 0;
#endif
    return 0;
    //printf("fillBuf2, out: %s, lng: %i\n", ptr, size);
}
GString GString::addBuf(char* in)
{
    size = strlen(in) + strlen(ptr);
#ifdef MAKE_SEC
    if( size != 0 && size + 1 <= 0 ) {printf("ALERT\n"); size = MAX_GSTR_LNG;}
    char* out = new char[size + 1];
    //memset(out, 0, size);
    strcpy( out, ptr );
    //out[strlen(ptr)] = 0;
    strncat( out, in, size - strlen(ptr) );
    GString ret = out;
    delete [] out;
#else
    char* out = new char[strlen(in) + strlen(ptr) + 1];
    strcpy(out, ptr);
    strcat(out, in);
    GString ret = out;
    delete [] out;
#endif
    //printf("AddBufEnd\n");
    return ret;
}
GSeq <GString> GString::split(char token)
{
    GSeq<GString> aSeq;

    GString tmp = *this;
    //tmp = tmp.removeButOne(token);
    while(tmp.occurrencesOf(token))
    {
        aSeq.add(tmp.subString(1, tmp.indexOf(token)-1));
        tmp = tmp.remove(1, tmp.indexOf(token));
    }
    //if( tmp.length() )
    {
        aSeq.add(tmp);
    }
    return aSeq;
}

int GString::minIndexOfAny(GString tokenList)
{
    int min = 0;
    int pos;
    for(int i = 1; i <= (int) tokenList.length(); ++i )
    {
        pos = this->indexOf(tokenList[i]);
        if( min == 0 ) min = pos;
        if( pos > 0 && pos < min ) min = pos;
    }
    return min;
}

GSeq <GString> GString::split(GString token)
{
    GSeq<GString> aSeq;
    int pos;
    GString tmp = *this;
    while(tmp.occurrencesOf(token))
    {
        pos = tmp.indexOf(token);
        if( pos == 0 ) break;
        aSeq.add(tmp.subString(1, pos-1));
        tmp = tmp.remove(1, pos+token.length()-1);
    }
    aSeq.add(tmp);

    return aSeq;
}


//GSeq <GString> GString::split(GString tokenList)
//{
//    GSeq<GString> aSeq;
//    int pos;
//    GString tmp = *this;
//    while(true)
//    {
//        pos = tmp.minIndexOfAny(tokenList);
//        if( pos == 0 ) break;
//        aSeq.add(tmp.subString(1, pos-1));
//        tmp = tmp.remove(1, pos);
//    }
//    aSeq.add(tmp);

//    return aSeq;
//}

GString GString::toBase64()
{
    return base64_encode((unsigned char*) ptr, size);
    GString result;
    int len = this->length();
    result.rightJustify((len + 2) / 3 * 4, '=');

    int j    = 1;
    int pad  = this->length() % 3;
    int last = this->length() - pad;

    for (int i = 0; i < last; i += 3)
    {
        int n = int(ptr[i]) << 16 | int(ptr[i + 1]) << 8 | ptr[i + 2];
        result[j++] = B64chars[n >> 18];
        result[j++] = B64chars[n >> 12 & 0x3F];
        result[j++] = B64chars[n >> 6 & 0x3F];
        result[j++] = B64chars[n & 0x3F];
    }
    if (pad)  /// set padding
    {
        int n = --pad ? int(ptr[last]) << 8 | ptr[last + 1] : ptr[last];
        result[j++] = B64chars[pad ? n >> 10 & 0x3F : n >> 2];
        result[j++] = B64chars[pad ? n >> 4 & 0x03F : n << 4 & 0x3F];
        result[j++] = pad ? B64chars[n << 2 & 0x3F] : '=';
    }
    return result;
}

GString GString::fromBase64()
{
    return base64_decode(ptr);

    int len = this->length();
    if (len == 0) return "";

    int j = 1;
    int pad1 = len % 4 || ptr[len - 1] == '=';
    int pad2 = pad1 && (len % 4 > 2 || ptr[len - 2] != '=');

    const int last = (len - pad1) / 4 << 2;
    GString result;
    result = result.rightJustify(last / 4 * 3 + pad1 + pad2);


    for (int i = 0; i < last; i += 4)
    {
        int n = B64index[ptr[i]] << 18 | B64index[ptr[i + 1]] << 12 | B64index[ptr[i + 2]] << 6 | B64index[ptr[i + 3]];
        result[j++] = n >> 16;
        result[j++] = n >> 8 & 0xFF;
        result[j++] = n & 0xFF;
    }
    if (pad1)
    {
        int n = B64index[ptr[last]] << 18 | B64index[ptr[last + 1]] << 12;
        result[j++] = n >> 16;
        if (pad2)
        {
            n |= B64index[ptr[last + 2]] << 6;
            result[j++] = n >> 8 & 0xFF;
        }
    }
    return result;
}


/*
GString::GStrList GString::split(char token)
{
    GStrList aList;
    return aList;
}


GString::GStrList::GStrList()
{

}


GString::GStrList::~GStrList()
{
    m_seqGString.removeAll();
}
GString::GStrList::GStrList( const GStrList &aList )
{

}

GString::GStrList &GString::GStrList::operator = (const GStrList aList)
{

}

GString GString::GStrList::at(int i){
    if( i < 1 || i > m_seqGString.numberOfElements() ) return NULL;
    return m_seqGString.elementAtPosition(i);
}
int GString::GStrList::count(){
    return m_seqGString.numberOfElements();
}
void GString::GStrList::add(GString in){
    return m_seqGString.add(in);
}
*/
/****************** OLD STUFF ********************************
GString GString::translate(char in, char out)
{
   tm("Member translate(char, char)...");
   size = strlen(ptr);
   char * tmp = new char[size+1];

   long i;
   for( i=0; i < size; ++i )
   {
      if ( ptr[i] != in ) tmp[i] = ptr[i];
      else tmp[i] = out;
   }
   delete [] ptr;
   tmp[size] = 0;
   ptr = new char[size+1];
   strcpy( ptr, tmp );
   ptr[size] = 0;

   delete [] tmp;
   tm("Member translate()...Done.");
   return *this;
}
GString GString::upperCase()
{
   tm("Member upperCase()...");
// Not on Linux....
//   strupr(ptr);

// very nice....
   char * p;
   for( p = ptr; p < ptr + strlen(ptr); ++p )
   {result
      tmp[p] = toupper( *p );
   }
//END
   unsigned long i;
   for( i = 0; i < strlen(ptr); ++ i )
   {
      ptr[i] = toupper( ptr[i] );
   }
   tm("Member upperCase()...Done.");
   return *this;
}
GString GString::lowerCase()
{
   tm("Member lowerCase()...");
//   strlwr(ptr);
   unsigned long i;
   for( i = 0; i < strlen(ptr); ++ i )
   {
      ptr[i] = tolower( ptr[i] );
   }
   tm("Member lowerCase()...Done.");
   return *this;
}

************************************************************/


bool GString::is_base64(unsigned char c)
{
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string GString::base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len)
{
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; (i <4) ; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i)
    {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = ( char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while((i++ < 3))
            ret += '=';

    }

    return ret;

}

std::string GString::base64_decode(std::string const& encoded_string)
{
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i ==4) {
            for (i = 0; i <4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = ( char_array_4[0] << 2       ) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) +   char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = 0; j < i; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}
