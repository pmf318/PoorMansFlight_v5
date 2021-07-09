
#ifndef _GXML
#include "gxml.hpp"
#endif

#include "gstuff.hpp"
#include "gdebug.hpp"

#include <iostream>
#include <ctype.h>

#include <gseq.hpp>




GXml::GXml()
{
    _blockSeq.removeAll();
    _lastErrCode = 0;
}

GXml::GXml( const GXml &x )
{
    _lastErrCode = 0;
    _xmlAsString = x._xmlAsString;
}

GXml::GXml(GString xmlAsString)
{
    _blockSeq.removeAll();
    _xmlAsString = xmlAsString.strip();
    _lastErrCode = 0;
}

GXml::~GXml()
{
    _blockSeq.removeAll();
}

int GXml::readFromFile(GString fileName)
{
    _blockSeq.removeAll();
    _lastErrCode = 0;
    FILE *fp;
    fp = fopen(fileName, "r");
    if( !fp )
    {
        _lastErrCode = 1001;
        return _lastErrCode;
    }
    char ch;
    char cr = 10;
    char lf = 13;
    while((ch = fgetc(fp)) != EOF)
    {
        if( ch == lf || ch == cr ) continue;
        _xmlAsString += ch;
    }
    fclose(fp);
    cleanInput();
//    printf("----\n");
//    printf("IN: %s\n", (char*) _xmlAsString);
//    printf("----\n");


    return 0;
}

int GXml::countBlocks(GString tag)
{
    int c1 = _xmlAsString.occurrencesOf("<"+tag+">");
    int c2 = _xmlAsString.occurrencesOf("<"+tag+" ");
    int c3 = _xmlAsString.occurrencesOf("<"+tag+"/>");
    return c1 + c2 + c3;
     //return _xmlAsString.occurrencesOf("<"+tag+">") + _xmlAsString.occurrencesOf("<"+tag+" ") + _xmlAsString.occurrencesOf("<"+tag+"/>");
}

GXml GXml::getBlocks(GString tag)
{
    GString blockXml;
    GString data = _xmlAsString;

    int count = countBlocks(tag);
    for(int i = 1; i <= count; ++i )
    {
        blockXml += getBlock(data, tag);
    }
    return GXml(blockXml);
}

GXml GXml::getBlocksFromXPath(GString tag)
{
    GSeq <GString> xPathElmtSeq;
    GString data = _xmlAsString;
    xPathElmtSeq = tag.stripLeading('/').split('/');
    GXml tempXml(data);
    for( int i = 1; i <= xPathElmtSeq.numberOfElements(); ++i )
    {
//        printf("--------\n");
//        printf("tag: %s\n", (char*) xPathElmtSeq.elementAtPosition(i));
//        printf("--------\n");
        tempXml = tempXml.getBlocks(xPathElmtSeq.elementAtPosition(i));
//        printf("tempXml for tag %s: %s\n", (char*)xPathElmtSeq.elementAtPosition(i), (char*) tempXml.toString());
//        printf("--------\n");
    }
    return tempXml;
}


int GXml::attributeCount()
{
    GString data = _xmlAsString;
    GSeq <GString> attrSeq = data.split(' ');
    return attrSeq.numberOfElements()-1;
}

GString GXml::getAttribute(GString name)
{
    GSeq <GXML_ATTR> attrSeq = attributeSeq();
    for( int i = 1; i <= attrSeq.numberOfElements(); ++i )
    {
        if( attrSeq.elementAtPosition(i).Name == name ) return attrSeq.elementAtPosition(i).Value;
    }
    return "";
}

GSeq<GXML_ATTR> GXml::attributeSeq()
{
    GString data = _xmlAsString.subString(1, _xmlAsString.indexOf(">")-1).strip("<").stripTrailing("/").strip();
    GSeq <GXML_ATTR> xmlAttrSeq;

    int pos = data.indexOf(" ");
    if( pos > 0 ) data = data.remove(1, pos);

    while(data.length())
    {

        if( data.occurrencesOf("=") == 0 ) break;
        GXML_ATTR * pAttr = new GXML_ATTR;
        pos = data.indexOf("=");
        pAttr->Name = data.subString(1, pos-1).strip();

        //Value is enclosed in '"'
        if( data.occurrencesOf("\"") < 2 ) break;

        data = data.remove(1, data.indexOf("\""));        
        pAttr->Value = data.subString(1, data.indexOf("\"")-1);
        data = data.remove(1, data.indexOf("\""));
        xmlAttrSeq.add(*pAttr);
    }
//    GSeq <GString> attrSeq = data.split(' ');
//    GString attr;
//    GSeq <GXML_ATTR> xmlAttrSeq;
//    for( int i = 2; i <= attrSeq.numberOfElements(); ++i )
//    {
//        attr = attrSeq.elementAtPosition(i);
//        GXML_ATTR * pAttr = new GXML_ATTR;
//        pAttr->Name = attr.subString(1, attr.indexOf("=")-1);
//        pAttr->Value = attr.subString(attr.indexOf("=")+1, attr.length()).strip("\"").strip();
//        xmlAttrSeq.add(*pAttr);
//    }
    return xmlAttrSeq;
}


GXml GXml::getBlockAtPosition(GString tag, int position)
{
    GString blockXml;
    GString data = _xmlAsString;

    int count = countBlocks(tag);
    for(int i = 1; i <= count; ++i )
    {
        blockXml = getBlock(data, tag);
        if( i == position ) return GXml(blockXml);
    }
    return GXml("");
}

GString GXml::getBlock(GString &data, GString tag)
{
    int startPos, endPos;
    GString out;
    GString endTag = "</"+tag+">";

//    printf("---getBlock--------\n");
//    printf("IN: %s, tagToFind: %s\n", (char*) data, (char*) tag);
//    printf("---getBlock--------\n");


    startPos = data.indexOf("<"+tag+">");
    endPos   = data.indexOf(endTag);
    if( startPos > 0 && endPos > startPos )
    {
        out+= data.subString(startPos, endPos+endTag.length()-startPos);
        data = data.remove(1, endPos+endTag.length());
    }

    startPos = data.indexOf("<"+tag+"/>");
    if( startPos > 0 )
    {
        endPos = GString("<"+tag+"/>").length();
        out += data.subString(startPos, endPos-startPos);
        data = data.remove(1, endPos+endTag.length());
    }

    startPos = data.indexOf("<"+tag+" ");
    if( startPos > 0 )
    {
        data = data.subString(startPos, data.length()).strip();
        endPos = data.indexOf(">");
        if( data[endPos-1] == '/')
        {
            out += data.subString(1, endPos);
            data = data.remove(1, endPos);
        }
        else
        {
            endPos = data.indexOf(endTag);
            out += data.subString(1, endPos+endTag.length()-1);
            data = data.remove(1, endPos+endTag.length()-1);
        }
    }
//    printf("-----------\n");
//    printf("OUT: %s\n", (char*) out);
//    printf("-----------\n");
    return out;
}


GString GXml::toString()
{
    return _xmlAsString;
}

void GXml::cleanInput()
{
    while( _xmlAsString.occurrencesOf(" />"))
    {
        _xmlAsString = _xmlAsString.change(" />", "/>");
    }
    while( _xmlAsString.occurrencesOf(" >"))
    {
        _xmlAsString = _xmlAsString.change(" >", ">");
    }
    while( _xmlAsString.occurrencesOf("> "))
    {
        _xmlAsString = _xmlAsString.change("> ", ">");
    }
}

int GXml::endTagPos(GString data, GString tag)
{
    int pos = data.indexOf("</"+tag+">");
    if( pos > 0 ) return pos;
    return -1;
}

int GXml::lastErrCode()
{
    return _lastErrCode;
}


