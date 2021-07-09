#include <string>
#include "debugMessenger.hpp"
#include "iobserver.hpp"
 
using namespace std;
class DebugObserver : public IObserver
{
 
private:
    GString name;
    GString observerState;
    DebugMessenger* subject; // Dieses Objekt hält die Daten (=notifier)
 
public:
    void update();
    void setSubject(DebugMessenger* subj);
    DebugMessenger* getSubject();
    DebugObserver(DebugMessenger* subj, GString name);
 
};
