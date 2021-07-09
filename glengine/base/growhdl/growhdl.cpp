//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt
//

#include <idsql.hpp>
#ifndef _GRowHdl_
#include "growhdl.hpp"
#endif


//( Constructor in .hpp )

GString GRowHdl::version()
{
    return "GLineHdl v 1.0 (C) 1999-2001 Gregor Leipelt";
}
GRowHdl::GRowHdl(const GRowHdl & aGLHD)
{
    rowElementSeq = aGLHD.rowElementSeq;
}
GRowHdl::~GRowHdl(){
    clearSeq();
}
void GRowHdl::addElement(DATA_CELL * pCell)
{
    rowElementSeq.add(pCell);
    count ++;
}

void GRowHdl::addElement(GString element, short isNull, short bin, short trunc)
{
    DATA_CELL * pCell = new DATA_CELL;
    pCell->data = element;
    pCell->isNull = isNull;
    pCell->isBinary = bin;
    pCell->isTruncated = trunc;
    rowElementSeq.add(pCell);
    count ++;
}

GString GRowHdl::row()
{
    unsigned int i;
    GString out = "";
    for(i=1; i<=rowElementSeq.numberOfElements();++i)
    {
        out += GString( rowElementSeq.elementAtPosition(i)->data );
    }

    return out;
}

unsigned long GRowHdl::elements()
{
    return rowElementSeq.numberOfElements();
}

GString GRowHdl::rowElementData(short index)
{
    if( index < 1 || index > (short) rowElementSeq.numberOfElements() ) return "@OutOfReach";
    return rowElementSeq.elementAtPosition(index)->data;
}

DATA_CELL *GRowHdl::rowElement(short index)
{
    if( index < 1 || index > (short) rowElementSeq.numberOfElements() ) return NULL;
    return rowElementSeq.elementAtPosition(index);
}

GString GRowHdl::justifyRowData(GSeq <short> * justSeq)
{
    PMF_UNUSED(justSeq);
    unsigned int i;
    GString line = "";

    for(i=1; i<=rowElementSeq.numberOfElements();++i)
    {
        //line += GString( rowElementSeq.elementAtPosition(i)->data ).leftJustify(justSeq->elementAtPosition(i));
    }
    return line;
}

GRowHdl & GRowHdl::operator=(GRowHdl lh)
{
    tm("  LH: Start OP =...Elements to copy (ELS): "+GString(lh.elements())+", count: "+GString(count) );
    clearSeq();
    rowElementSeq = lh.rowElementSeq;
    return *this;
}
void GRowHdl::clearSeq()
{
    DATA_CELL * pCell;
    while( !rowElementSeq.isEmpty() )
    {
      pCell = rowElementSeq.firstElement();
      delete pCell;
      rowElementSeq.removeFirst();
    }
}

void GRowHdl::tm(GString msg)
{
    return;
    msg.sayIt();
}






