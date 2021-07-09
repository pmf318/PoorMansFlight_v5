#ifndef PMF_MISC_H
#define PMF_MISC_H

#include <QString>
#include <gstring.hpp>

class misc 
{
//    Q_OBJECT
public:
    misc();
     ~misc();
    static GString asGString(QString s);  
private:

};

#endif 
