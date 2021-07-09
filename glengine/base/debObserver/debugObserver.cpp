#include <iostream>
#include "iobserver.hpp"
#include "debugObserver.hpp"
 
using namespace std;
 
 
// Daten anzeigen
void DebugObserver::update()
{
    observerState = subject->getData();
    printf("Observer: Got Data: %s\n", (char*)observerState);
}
 
void DebugObserver::setSubject(DebugMessenger* obj)
{
    subject = obj;
}
 
DebugMessenger* DebugObserver::getSubject()
{
    return subject;
}
 
DebugObserver::DebugObserver(DebugMessenger* subj, GString n)
{
    name = n;
    subject = subj;
}
