#ifndef PMFCOLUMN_H
#define PMFCOLUMN_H

#include <idsql.hpp>
#include <dsqlplugin.hpp>



class PmfColumn
{

public:
    PmfColumn(const COL_SPEC *colSpec);
    ~PmfColumn();

private:
     GString _colName;
     GString _colType;
     GString _length;
     GString _scale;
     GString _nullable;
     GString _default;
     GString _logged;
     GString _compact;
     GString _identity;
     GString _generated;
     GString _misc;

public:
    //This would have been much nicer, but does not compile w/ cl
    //const GString &ColName = _colName;


    const GString &colName(){return _colName;}

    const GString &colType() {return _colType;}
    const GString &colLength() {return _length;}
    const GString &scale() {return _scale;}
    const GString &nullable() {return _nullable;}
    const GString &defaultVal() {return _default;}
    const GString &logged() {return _logged;}
    const GString &compact() {return _compact;}
    const GString &identity() {return _identity;}
    const GString &generated() {return _generated;}
    const GString &misc() {return _misc;}


};

#endif
