//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 
//
/*********************************************************************
*********************************************************************/

#ifndef _GSTRINGLIST_
#include <gstringlist.hpp>
#endif

#include <stdio.h>


GStringList::GStringList()
{

}

GStringList::GStringList(GString in, GString split)
{
    GString tmp;
    while( in.occurrencesOf(split) )
    {
        tmp = in.subString(1, in.indexOf(split)-1).strip();
        m_gstrSeq.add(tmp);
        in = in.remove(1, in.indexOf(split)+split.length());
    }
    if( split.length() && in.strip().length() ) m_gstrSeq.add(in);
}

GStringList::GStringList( const GStringList &aList )
{
    m_gstrSeq = aList.m_gstrSeq;
}

GStringList::~GStringList()
{
    m_gstrSeq.removeAll();
}

GStringList &GStringList::operator = (const GStringList aList)
{
    m_gstrSeq = aList.m_gstrSeq;
    return *this;
}

GString GStringList::at(int i)
{
    if( i < 1 || i > (int)m_gstrSeq.numberOfElements() ) return "";
    return m_gstrSeq.elementAtPosition(i);
}

int GStringList::count()
{
    return m_gstrSeq.numberOfElements();
}

void GStringList::add(GString in, GString split)
{
    GString tmp;
    while( in.occurrencesOf(split) )
    {
        tmp = in.subString(1, in.indexOf(split)-1).strip();
        m_gstrSeq.add(tmp);
        in = in.remove(1, in.indexOf(split)+split.length()-1);
        printf("After rem: %s\n", (char*) in);
    }
    if( split.length() && in.strip().length() ) m_gstrSeq.add(in);
}

GString GStringList::toString(GString sep)
{
    GString out;
    for( int i = 1; i <= (int)m_gstrSeq.numberOfElements() - 1; ++i )
    {
        out += m_gstrSeq.elementAtPosition(i) + sep;
    }
    out += m_gstrSeq.elementAtPosition(m_gstrSeq.numberOfElements());
    return out;
}

