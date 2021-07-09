//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 
//



#ifndef _GSTUFF_
#include <gstuff.hpp>
#endif

#ifndef MAKE_VC
#include <limits.h>
#include <unistd.h>
#include <sys/time.h>
#endif

#include "iobserver.hpp"
#include "debugObserver.hpp"

#include <time.h>


//GString encKey = "2pBLW3pzh$3_336PfFY=Jmym-RGrNsfn";
GString encKey = "Abcdabcdabciabcdabcd";

static time_t someTime;

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


static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}


GString GStuff::breakLongLine(GString txt, int maxLen)
{
    if( txt.length() == 0 ) return txt;

    if( (int)txt.length() < maxLen )
    {
        return txt +"\n";
    }
    int pos, count;
    pos = count = 0;
    for(int  i = 1; i <= (int)txt.length(); ++i )
    {
        count++;
        if( txt[i] == ' ' || txt[i] == ',' || txt[i] == '>') pos = i;
        if( count >= maxLen && pos > 0 )
        {
            txt = txt.insert((char*)"\n", pos+1);
            count = 0;
        }
    }
    return txt+"\n";
}

GString GStuff::format(GString in, unsigned long maxLng, char cut)
{
    unsigned long pos;
    GString out, tmp, sep;
    if( in.length() <= maxLng ) return in;
    while (in.length() > maxLng )
    {
        pos = maxLng;
        sep = "-";
        if( in.occurrencesOf(cut) > 0 )
        {
            pos = in.subString(1, maxLng).lastIndexOf(cut);
            //pos can be 0, we're checking the subString, which might not contain blank
            if( pos > maxLng || pos == 0) pos = maxLng;
            else sep = "";
        }
        tmp = in.subString(1, pos)+sep+"\n";
        in.remove(1, pos);
        out += tmp;
    }
    out += in;
    return out;
}

GString GStuff::formatPath(GString path)
{
#if defined(MAKE_VC) || defined (__MINGW32__)
    path = path.change("/", "\\");
#else
    path = path.change("\\", "/");
#endif
    return path;
}
int GStuff::shred(GString in, unsigned long maxLng, char cut1, char cut2)
{
    lineSeq.removeAll();

    unsigned long pos;
    GString tmp, sep;
    if( in.length() <= maxLng ) return in;
    while (in.length() > maxLng )
    {
        pos = maxLng;
        sep = "-";
        if( in.occurrencesOf(cut1) > 0 || in.occurrencesOf(cut2) > 0)
        {
            pos = in.subString(1, maxLng).lastIndexOf(cut1);
            //pos can be 0, we're checking the subString, which might not contain cut1
            if( pos > maxLng || pos == 0) //No luck.
            {
                pos = in.subString(1, maxLng).lastIndexOf(cut2);
                if( pos > maxLng || pos == 0) pos = maxLng;
                else sep = " ";
            }
            else sep = " ";
        }
        tmp = in.subString(1, pos)+sep;
        in.remove(1, pos);
        lineSeq.add(tmp);
    }
    lineSeq.add(in);
    return 0;

}

void GStuff::splitStringDelFormat(GString in, GSeq<GString> *outSeq)
{
    int inApo = 0;
    char apo= '\"';
    GString part;
    for( int i = 1; i<= (int)in.length(); ++i )
    {
        part += in[i];
        if( in[i] == apo ) inApo = inApo ? 0 : 1;
        //printf("in[%i]: %s, inApo: %i\n", i,(char*) GString(in[i]), inApo);
        if( inApo ) continue;
        if( in[i] == ',' || i == (int)in.length() )
        {
            outSeq->add(part.stripTrailing(','));
            //printf("Adding part: %s\n", (char*) part);
            part = "";
        }
    }

}

void GStuff::splitString(GString in, char del, GSeq<GString> *outSeq)
{
    char *ptr;
    char *delimiter = new char[1];
    delimiter[0] = del;
    ptr = strtok(in, delimiter);

    while(ptr != NULL)
    {
        outSeq->add(ptr);
        ptr = strtok(NULL, delimiter);
    }
    delete[] delimiter;
}

int GStuff::lineCount()
{
    return lineSeq.numberOfElements();
}

GString GStuff::getLine(unsigned long i)
{
    if( i < 1 ) i = 1;
    if( i > lineSeq.numberOfElements() ) return "";
    return lineSeq.elementAtPosition(i);
}
void GStuff::dormez(int seconds)
{
#ifdef MAKE_VC
    Sleep(seconds);
#else
    usleep(seconds*1000);
    /*
   struct timespec timeOut,remains;

   timeOut.tv_sec = 0;//seconds;
   timeOut.tv_nsec = seconds*1000000000;
   nanosleep(&timeOut, &remains);
   */
#endif

}
void GStuff::sendMsg(GString msg)
{
    DebugMessenger* msngr = new DebugMessenger();
    IObserver* debObs = new DebugObserver(msngr,"A");
    msngr->attach(debObs);
    msngr->setData(msg);
    msngr->notify();

    msngr->detach(debObs);
    delete debObs;
    delete msngr;
}
void GStuff::startStopwatch()
{
    someTime = time(0);
}
int GStuff::getStopwatch()
{
    struct tm  start, stop;
    time_t now = time(0);
    start = *localtime(&someTime);
    stop  = *localtime(&now);
    //Ret as string: http://en.cppreference.com/w/cpp/chrono/c/strftime
    //strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
    int seconds = difftime(now, someTime);
    return seconds;
}

GString GStuff::fileFromPath(GString fullPath)
{
    if( fullPath.occurrencesOf("\\")) return fullPath.subString(fullPath.lastIndexOf("\\")+1, fullPath.length()).strip();
    else if( fullPath.occurrencesOf("/")) return fullPath.subString(fullPath.lastIndexOf("/")+1, fullPath.length()).strip();
    return fullPath;
}

GString GStuff::pathFromFullPath(GString fullPath)
{
    if( fullPath.occurrencesOf("\\")) return fullPath.subString(1, fullPath.lastIndexOf("\\")).strip();
    else if( fullPath.occurrencesOf("/")) return fullPath.subString(1, fullPath.lastIndexOf("/")).strip();
    return fullPath;
}

int GStuff::hexToInt(char s[]) {
    int hexdigit, i, inhex, n;
    i=0;

    if(s[i] == '0') {
        ++i;
        if(s[i] == 'x' || s[i] == 'X'){
            ++i;
        }
    }

    n = 0;
    inhex = 1;
    for(; inhex == 1; ++i) {
        if(s[i] >= '0' && s[i] <= '9') hexdigit = s[i] - '0';
        else if(s[i] >= 'a' && s[i] <= 'f') hexdigit = s[i] - 'a' + 10;
        else if(s[i] >= 'A' && s[i] <= 'F') hexdigit = s[i] - 'A' + 10;
        else inhex = 0;

        if(inhex == 1) n = 16 * n + hexdigit;
    }
    return n;
}


/********************************************
* The follwing code (base64_decode) was taken from
* https://en.wikibooks.org/wiki/Algorithm_Implementation/Miscellaneous/Base64#C_2
* and on this page is declared public domain
********************************************/
#define WHITESPACE 64
#define EQUALS     65
#define INVALID    66

static const unsigned char charList[] = {
    66,66,66,66,66,66,66,66,66,66,64,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,62,66,66,66,63,52,53,
    54,55,56,57,58,59,60,61,66,66,66,65,66,66,66, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,66,66,66,66,66,66,26,27,28,
    29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
    66,66,66,66,66,66
};

int GStuff::base64_decode(char *in, size_t inLen, unsigned char *out, size_t *outLen)
{
    char *end = in + inLen;
    char iter = 0;
    unsigned int buf = 0;
    size_t len = 0;

    int pos = 0;

    while (in < end)
    {
        pos++;
        unsigned char c = charList[*in++];
        switch (c)
        {
        case WHITESPACE: continue;   /* skip whitespace */
        case INVALID:    return 11;   /* invalid input, return error */
        case EQUALS:                 /* pad character, end of data */
            in = end;
            continue;
        default:
            buf = buf << 6 | c;
            iter++; // increment the number of iteration
            /* If the buffer is full, split it into bytes */
            if (iter == 4) {
                if ((len += 3) > *outLen) return 21; /* buffer overflow */
                *(out++) = (buf >> 16) & 255;
                *(out++) = (buf >> 8) & 255;
                *(out++) = buf & 255;
                buf = 0; iter = 0;
            }
        }
    }

    if (iter == 3) {
        if ((len += 2) > *outLen) return 2; /* buffer overflow */
        *(out++) = (buf >> 10) & 255;
        *(out++) = (buf >> 2) & 255;
    }
    else if (iter == 2) {
        if (++len > *outLen) return 3; /* buffer overflow */
        *(out++) = (buf >> 4) & 255;
    }
    *outLen = len; /* modify to reflect the actual output size */
    return 0;
}
//UNTESTED!
int GStuff::base64_encode(char *in, size_t inLen, unsigned char *out, size_t *outLen)
{
    char *end = in + inLen;
    char iter = 0;
    unsigned int buf = 0;
    size_t len = 0;

    int pos = 0;

    unsigned char invCharList[sizeof(charList)];
    for (int i = 0; i < sizeof(charList); i++)
    {
        invCharList[charList[i]] = i;
    }
    while (in < end)
    {
        pos++;
        unsigned char c = invCharList[*in++];
        switch (c)
        {
        case WHITESPACE: continue;   /* skip whitespace */
        case INVALID:    return 11;   /* invalid input, return error */
        case EQUALS:                 /* pad character, end of data */
            in = end;
            continue;
        default:
            buf = buf << 6 | c;
            iter++; // increment the number of iteration
            /* If the buffer is full, split it into bytes */
            if (iter == 4) {
                if ((len += 3) > *outLen) return 21; /* buffer overflow */
                *(out++) = (buf >> 16) & 255;
                *(out++) = (buf >> 8) & 255;
                *(out++) = buf & 255;
                buf = 0; iter = 0;
            }
        }
    }

    if (iter == 3) {
        if ((len += 2) > *outLen) return 2; /* buffer overflow */
        *(out++) = (buf >> 10) & 255;
        *(out++) = (buf >> 2) & 255;
    }
    else if (iter == 2) {
        if (++len > *outLen) return 3; /* buffer overflow */
        *(out++) = (buf >> 4) & 255;
    }
    *outLen = len; /* modify to reflect the actual output size */
    return 0;
}


GString GStuff::setDbQuotes(GString in)
{
    if(in[1UL] == '\"' && in[in.length()] == '\"') return in;
    return "\""+in+"\"";
}

GString GStuff::wrap(GString in)
{
    return "\""+in+"\"";
}
GString GStuff::formatForXml(GString in)
{
    in = in.change("&", "&amp;");
    in = in.change("\"", "&quot;").change("<", "&lt;").change(">", "&gt;").change("'", "&apos;").change("/", "&#47;").change("(", "&#40;").change(")", "&#41;");
    return in;
}
GString GStuff::decorateTabName(GString in)
{
    if( !in.length() || in.occurrencesOf(".") == 0 ) return in;
    in = in.removeAll('\"');
    in = in.change(".", "\".\"");
    return "\"" + in + "\"";
}

void GStuff::xorEncrypt(GString key, GString& txt, int txtLen)
{
    int i;
    int keyLength = key.length();
    for( i = 1 ; i <= txtLen ; i++ )
    {
        txt[i] = txt[i]^key[i%keyLength];
    }
}

GString GStuff::encryptDecrypt(GString toEncrypt) {
    char key[10] = {'p', 'm', 'f', 'w', 'e', 'a', 'k', 'e', 'n', 'c'};
    GString out = toEncrypt;
    for (int i = 1; i <= toEncrypt.length(); i++)
    {
        out[i] = toEncrypt[i] ^ key[(i-1) % (sizeof(key) / sizeof(char))];
    }
    return out;
}

void GStuff::xorEncrypt(char *data, int data_len)
{
    int keyLen = 10;
    char key[10] = {'p', 'm', 'f', 'w', 'e', 'a', 'k', 'e', 'n', 'c'};
    for (int i = 0; i < data_len; i++)
    {
        data[i] ^= key[ i % keyLen ];
    }
}

//unsigned char* GStuff::encryptDecrypt(unsigned char* toEncrypt) {
//    char key[10] = {'p', 'm', 'f', 'w', 'e', 'a', 'k', 'e', 'n', 'c'};
//    GString out = toEncrypt;
//    for (int i = 1; i <= toEncrypt.length(); i++)
//    {
//        out[i] = toEncrypt[i] ^ key[(i-1) % (sizeof(key) / sizeof(char))];
//    }
//    return out;
//}


GString GStuff::encryptToBase64(GString in)
{
    if( !in.length() ) return in;
    char* data = new char[in.length()+1];
    strncpy(data, (char*) in, in.length());
    GStuff::xorEncrypt(data, in.length());
    GString out = GStuff::base64_encode((unsigned char*) data, in.length());
    delete [] data;
    return out;
}

GString GStuff::decryptFromBase64(GString in)
{
    unsigned char* data = new unsigned char[in.length()+1];
    size_t outLen = in.length();
    GStuff::base64_decode((char*)in, (size_t)in.length(), data, &outLen);
    data[outLen] = 0;
    GStuff::xorEncrypt((char*)data, outLen);
    return data;
}



void GStuff::xor_encrypt(char *key, char *input, int n)
{
    int i;
    int keyLength = strlen(key);
    for( i = 0 ; i < n ; i++ )
    {
        input[i]=input[i]^key[i%keyLength];
    }
}

GString GStuff::encryptPwd(GString txt)
{
    int len = txt.length();
    char * buf = new char[len+1];
    strncpy( buf, txt, len );
    xor_encrypt(encKey, buf, len);
    return GString(len) + ":"+GString(buf, len);





    xorEncrypt(encKey, txt, txt.length());
    GString test = GString(txt, txt.length());
    test.sayIt();
    int x = test.length();
    printf("len: %i\n", x);
    return GString(len) + ":"+test;
}

GString GStuff::decryptPwd(GString txt)
{
    int len = txt.subString(1, txt.indexOf(":")-1).asInt();
    GString pwd = txt.subString(txt.indexOf(":")+1, txt.length()).strip();
    xorEncrypt(encKey, pwd, len);
    return pwd;

}

GString GStuff::TableNameFromStmt(GString stmt)
{

    if( GString(stmt).upperCase().occurrencesOf(" FROM ") == 0 ) return "";
    stmt = stmt.removeButOne(' ');
    GSeq <GString> words = stmt.split(' ');
    int pos = 0;
    for( pos = 1; pos <= (int) words.numberOfElements(); ++pos )
    {
        if( words.elementAtPosition(pos).upperCase() == "FROM" ) break;
    }
    if( pos == 0 ) return "";
    if( words.numberOfElements() >= pos+1 && words.elementAtPosition(pos+1).occurrencesOf("."))
    {
        return words.elementAtPosition(pos+1);
    }
    else if( words.numberOfElements() >= pos+3 && words.elementAtPosition(pos+2) == ".")
    {
        return words.elementAtPosition(pos+1) + words.elementAtPosition(pos+2) + words.elementAtPosition(pos+3);
    }
    else if( words.numberOfElements() >= pos+1 ) return words.elementAtPosition(pos+1);

    return "";
}

GString GStuff::WhereClauseFromStmt(GString stmt)
{
    GString whereClause = stmt.upperCase();
    if( whereClause.occurrencesOf(" WHERE ") )
    {
        whereClause = stmt.subString(whereClause.indexOf(" WHERE ")+7, whereClause.length()).strip();
        if( whereClause.occurrencesOf(" ORDER ") )whereClause = whereClause.subString(1, whereClause.indexOf(" ORDER ")).strip();
    }
    else whereClause = "";
    return whereClause;
}

GString GStuff::currentWorkDir()
{
#ifdef MAKE_VC
    char buf[256];
    GetCurrentDirectoryA(256, buf);
    return buf;
#else
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) return cwd;
#endif
    return "";
}

GString GStuff::GetTime()
{

    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);    
    strftime(buf, sizeof(buf), "%X", &tstruct);
    int milliSec;
#ifndef MAKE_VC
    struct timeval  tv;
    gettimeofday(&tv, NULL);
    milliSec = tv.tv_usec / 1000;
#else
    SYSTEMTIME st;
    GetSystemTime(&st);
    milliSec = st.wMilliseconds;
#endif
    return GString(buf)+"."+GString(milliSec);
}


std::string GStuff::base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
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

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';

  }

  return ret;

}

std::string GStuff::base64_decode(std::string const& encoded_string) {
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

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if (i) {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}

/***
GString stdStuff::getTxt(GString definer)
{
    for(long i = 1; i <= defSeq.numberOfElements(); ++i )
    {
       if( definer == defSeq.elementAtPosition(i) ) return txtSeq.elementAtPosition(i);
    }
    return "<errTxtSeq>";
}

int stdStuff::fillTxtSeq(int lang)
{
  defSeq.add("BT_OK");
  defSeq.add("BT_EXIT");

   case LANG_EN:
      txtSeq.add("OK");
      txtSeq.add("Exit");
      break;

   return 0;
}
***/
