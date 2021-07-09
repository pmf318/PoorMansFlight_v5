

#ifndef _SUBJECT_
#define _SUBJECT_

#include <list>
#include "iobserver.hpp"
 
using namespace std;
 
class Messenger
{
 
public:
    void attach(IObserver* observer);
    void detach(IObserver* observer);
    void notify();
 
private:
    list<IObserver*> observers;
 
 
protected:
    // Durch protected-Konstruktor wird diese Klasse abstrakt
    Messenger() {};
 
};

#endif