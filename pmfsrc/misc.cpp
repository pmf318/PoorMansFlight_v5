#include "misc.h"

#include <gstring.hpp>

misc::misc() 
{
}

misc::~misc()
{
}

GString misc::asGString(QString s)
{
    QByteArray bytes = s.toAscii();
    const char* p = bytes.data();
    return GString(p);
}