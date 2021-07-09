#include "messenger.hpp"
#include "iobserver.hpp"
 
void Messenger::attach(IObserver* observer)
{
    observers.push_back(observer);
}
 
 
void Messenger::detach(IObserver *observer)
{
    observers.remove(observer);
}
 
 
void Messenger::notify()
{
    list<IObserver*>::iterator iter = observers.begin();
    for ( ; iter != observers.end(); iter++ )
    {
        (*iter)->update();
       
    }      
}