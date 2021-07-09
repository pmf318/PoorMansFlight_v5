#include <stdio.h>
#include<iostream>
#include<dlfcn.h>
#include "../../inc/idsql.hpp"
#include "../../inc/gstring.hpp"

using namespace std;

int main()
{
    cout << "IDSQL:: LoadInst called" <<endl;
    void *handle;
    handle = dlopen("./libodbcdsql.so", RTLD_NOW);
    if (!handle)
    {
        printf("IDSQL: Err on dlopen(): %s\n", dlerror());
        return NULL;
    }

    typedef IDSQL* create_t();
    typedef void destroy_t(IDSQL*);

    create_t* creat=(create_t*)dlsym(handle,"create");
    destroy_t* destroy=(destroy_t*)dlsym(handle,"destroy");
    if (!creat)
    {
        cout<<"creat: Error %s"<<dlerror();
        return NULL;
    }
    if (!destroy)
    {
        cout<<"destroy: Error %s"<<dlerror();
        return NULL;
    }
    cout << "IDSQL:: LoadInst, calling creat" <<endl;
    IDSQL* tst = creat();

    destroy(tst);
    return 0 ;

}
