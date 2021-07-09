//
//
//   Include file for Sequence class
//   Author: Gregor Leipelt (gregor@leipelt.de)
//


#ifndef _GSEQ_
#define _GSEQ_

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
#ifdef MakeGSeq
#define GSeqExp_Imp _Export
#else
#define GSeqExp_Imp _Import
//  #pragma library( "GSeq.LIB" )
#pragma library( "GLEngine.LIB" )
#endif
#endif
//************** Visual Age END *****************


#include <seqitem.hpp>


static int m_GSeqCounter = 0;

template <class Type>

class GSeq
{

private:
    SeqItem <Type> *first, *last, *tmp, *crs;
    unsigned long count, i;

public:
    VCExport   GSeq(){
        first = last = crs = NULL; count = 0; m_GSeqCounter++;
    }

    VCExport   GSeq( const GSeq &aSeq )
    {
        first = last = crs = NULL; count = 0; m_GSeqCounter++;
        unsigned long i = aSeq.count;
        crs = aSeq.first;
        while( i )
        {
            add(crs->item);
            crs = crs->next;
            i--;
        }
    }
    VCExport   ~GSeq(){
        this->removeAll();m_GSeqCounter--;
    }

    VCExport  void removeAll(){
        if( first == NULL ) return;
        tmp = first;

        while( tmp->next != NULL )
        {
            count--;
            tmp = first->next;
            delete first;
            first = tmp;
        };
        if( count >= 1 ){ delete first; count--;}
        else if( count == 0 && first != NULL ) delete first;
        //delete tmp;
        first = last = NULL;
        count = 0;
    }

    VCExport  Type firstElement() {
        if( first == NULL){ return NULL;}
        return first->item;
    }
    VCExport  Type lastElement() {
        return last->item;
    }

    VCExport  int  isEmpty(){
        if( first == NULL ) return 1; else return 0;
    }

    VCExport  void removeFirst() {
        if( first == NULL) return;
        tmp = first->next; delete first; first = tmp; count--;
    }

    VCExport  void removeLast()
    {
        if( count == 0 || first == NULL || last == NULL) return;
        if( count == 1 )
        {
            //if( first ) delete first;
            if( last  )
            {
                first = last;
                delete last;
            }
            first = last = NULL;
            count = 0;
            return;
        }
        i = 1; tmp = first;
        while( ++i < count && tmp->next != NULL ){tmp = tmp->next; }
        delete last;
        last = NULL;
        tmp->next = NULL;
        last = tmp;
        count--;
    }

    VCExport  Type elementAtPosition(unsigned long pos)
    {
        tmp = first; i= 1;
        while( tmp->next != NULL && i<pos ){
            ++i; tmp = tmp->next;
        }
        return tmp->item;
    }

    VCExport  void add(const Type &newVal) {
        SeqItem<Type> *pItem = new SeqItem<Type> (newVal);
        //if( isEmpty() ){ first = last = pItem; count++;}
        if( count == 0 ){ first = last = pItem; count++;}
        else { last->next = pItem; last = pItem; ++count;}

    }
    VCExport  unsigned long numberOfElements() {
        if (first == NULL) return 0;
        return count;
    }

    VCExport  unsigned long itemCount(){
        return count;
    }

    VCExport  short replaceAt(unsigned long pos, const Type &newVal)
    {
        if( pos < 1 ) return 1;
        if( first == NULL )return 4;
        tmp = first; i=1;
        while( i < pos ){++i; tmp = tmp->next; if( tmp == NULL ) return 2;}
        tmp->item = newVal;
        return 0;
    }
    VCExport  Type initCrs(){
        crs = first; return crs->item;
    }

    VCExport  Type setCrsToNext() {
        if( first == NULL ) return NULL;
        if( crs == NULL || crs->next == NULL ) crs = first;
        else crs = crs->next;
        return crs->item;
    }

    VCExport GSeq <Type> &operator = (const GSeq <Type> aSeq)  {
        removeAll();
        unsigned long i = aSeq.count;
        crs = aSeq.first;
        while( i )
        {
            add(crs->item);
            crs = crs->next;
            i--;
        }
        return *this;
    }

    VCExport GSeq <Type> &operator += (const GSeq <Type> aSeq)  {
        unsigned long i = aSeq.count;
        crs = aSeq.first;
        while( i )
        {
            add(crs->item);
            crs = crs->next;
            i--;
        }
        return *this;
    }

    VCExport SeqItem <Type> *firstSeqItem()  {
        crs = first;
        return first;
    }

    VCExport const SeqItem <Type> *nextSeqItem()  {
        if( first == NULL ) return NULL;
        if( crs == NULL || crs->next == NULL ) return NULL;
        else crs = crs->next;
        return crs;
    }

    VCExport short insertAt(unsigned long pos, const Type &newVal)   {
        if( pos > count || count == 0 )
        {
            add(newVal);
            return 0;
        }
        if( pos < 1 ) pos = 1;

        SeqItem<Type> *pItem = new SeqItem<Type> (newVal);

        tmp = first; i = 1;
        pItem->item = newVal;

        while( tmp->next != NULL && i+1 < pos ){ ++i; tmp = tmp->next;}
        if( pos == 1 )
        {
            pItem->next = first;
            first = pItem;
        }
        else
        {
            pItem->next = tmp->next;
            tmp->next = pItem;
        }

        count++;
        return 0;
    }
    VCExport short removeAt(unsigned long pos)
    {
        if( pos > count || count == 0 || pos < 1 ) return 1;
        if( pos == count )
        {
            removeLast();
            return 0;
        }
        if( pos == 1 )
        {
            removeFirst();
            return 0;
        }
        i = 1; tmp = first;
        while( tmp->next != NULL && i+1 < pos ){ ++i; tmp = tmp->next;}
        SeqItem<Type> *remItem = tmp->next;
        tmp->next = remItem->next;
        delete remItem;
        count--;
        return 0;
    }
    //This should only be used on Types that were created with "new" and have a working destructor
    VCExport void deleteAll()
    {
        while( !this->isEmpty() )
        {
            first->item = this->firstElement();
            delete first->item;
            this->removeFirst();
        }
        first = NULL;
        count = 0;
    }
    unsigned long getSmallest(GSeq * pSeq)
    {
        SeqItem <Type> *tmp = pSeq->first;
        SeqItem <Type> *smallest = tmp;
        unsigned long pos = 1, k = 1;
        while( tmp && k <= pSeq->count )
        {
            if( tmp->item < smallest->item )
            {
                smallest = tmp;
                pos = k;
            }
            k++;
            if( tmp->next ) tmp = tmp->next;
            else break;
        }
        return pos;
    }

    VCExport int contains(Type elmt)
    {
        if( first == NULL ) return 0;
        tmp = first;
        while( tmp->next != NULL )
        {
            if( tmp->item == elmt ) return 1;
            tmp = tmp->next;
        }
        if( tmp->item == elmt ) return 1;
        return 0;
    }

    VCExport void sort()
    {
        //Compromising between size and speed:
        //1. Move all elements into tmpSeq.
        GSeq <Type> tmpSeq;
        while( count )
        {
            tmpSeq.add(first->item);
            this->removeFirst();
        }
        //"this" is now empty (removeFirst above)
        //Now find the smallest item and add it to "this"
        unsigned long pos;

        while (tmpSeq.numberOfElements() )
        {
            pos = getSmallest(&tmpSeq);
            this->add(tmpSeq.elementAtPosition(pos));
            tmpSeq.removeAt(pos); //clearing tmpSeq
        }
    }
};
#endif







