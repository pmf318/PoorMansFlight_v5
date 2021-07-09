
#ifndef _DEBUGMESSENGER_
#define _DEBUGMESSENGER_


#include "gstring.hpp"
#include "messenger.hpp"


 
using namespace std;
 
class DebugMessenger : public Messenger
{
 
private:
    GString data;
 
public:
    void setData(GString data) { this->data = data; }
    GString getData() { return data; }
    DebugMessenger() : Messenger() {}
};

#endif
