


#ifndef DSQLINST_HPP
#define DSQLINST_HPP


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
#undef VCExport
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
  #include <istring.hpp>
#ifdef MakeGStuff
  #define GStuffExp_Imp _Export
#else
  #define GStuffExp_Imp _Import
  #pragma library( "GLEngine.LIB" )
#endif
#endif
//************** Visual Age END *****************


#ifdef MAKE_VC
#else 
	#if defined(__GNUC__) && __GNUC__ < 3
		#include ostream.h>
	#else
		#include <ostream>
		#include<iostream>
		#include<dlfcn.h>
	#endif
#endif


#ifndef _IDBAPI_
#include <idbapi.hpp>
#endif

#include <gstring.hpp>
#include <gseq.hpp>


#ifndef _IDSQL_
#include <idsql.hpp>
#endif

#ifndef _ODBCDSQL_
//#include <odbcdsql.hpp>
#endif

static IDSQL::ODBCDB m_dbType; // = IDSQL::NOTSET;
/*
static GString m_uid;
static GString m_pwd;
static GString m_db;
static GString m_host;
*/
using namespace std;



static GSeq <GString> typeSeq;
static GSeq <GString> dsqlPluginSeq;
static GSeq <GString> dbapiPluginSeq;


#ifdef MAKE_VC
    static HINSTANCE dsqlHandle = 0;
    static HINSTANCE dbapiHandle = 0;
#else
    static void *dsqlHandle = 0;
    static void *dbapiHandle = 0;
#endif



#ifdef Makedsqlinst
  class dsqlinstExp_Imp dsqlInst
#else
  class dsqlInst
#endif
{

    private:

      static void init()
      {
          typeSeq.removeAll();
          dsqlPluginSeq.removeAll();
          dbapiPluginSeq.removeAll();

          GString path = "./plugins/";
          addToSeq("DB2", path+"libdb2dsql.so", path+"db2dsql.lib", path+"libdb2api.so", path+"db2api.lib");
          addToSeq("SqlServer", path+"libodbcdsql.so",  path+"odbcdsql.dll", "y", "x");
      }
      static void addToSeq(GString db, GString dsqlLin, GString dsqlWin, GString dbapiLin, GString dbapiWin)
      {
          typeSeq.add(db);
#ifdef MAKE_VC
          dsqlPluginSeq.add(dsqlWin);
          dbapiPluginSeq.add(dbapiWin);
#else
          deb("type: "+db+", adding dsql: "+dsqlLin+", dbapi: "+dbapiLin);
          dsqlPluginSeq.add(dsqlLin);
          dbapiPluginSeq.add(dbapiLin);
#endif
      }
      static GString test(GString dbType)
      {
          deb("test: in: "+dbType+", elemnts in seq: "+GString(typeSeq.numberOfElements()));
          for(int i = 1; i <= typeSeq.numberOfElements(); ++i )
          {
              if(dbType == typeSeq.elementAtPosition(i)) return dsqlPluginSeq.elementAtPosition(i);
          }
          return "";
      }

      static GString dsqlPluginName(GString dbType)
      {
          deb("dsqlPluginName: in: "+dbType+", elemnts in seq: "+GString(typeSeq.numberOfElements()));
          for(int i = 1; i <= typeSeq.numberOfElements(); ++i )
          {
              if(dbType == typeSeq.elementAtPosition(i)) return dsqlPluginSeq.elementAtPosition(i);
          }
          return "";
      }

      static GString dbapiPluginName(GString dbType)
      {
          deb("dbapiPluginName calling init()");
          init();
          deb("dbapiPluginName: in: "+dbType+", elemnts in seq: "+GString(typeSeq.numberOfElements()));
          for(int i = 1; i <= typeSeq.numberOfElements(); ++i )
          {
              deb("dbapiPluginName: in: "+dbType+", checking "+typeSeq.elementAtPosition(i));
              if(dbType == typeSeq.elementAtPosition(i))
              {
                  deb("dbapiPluginName: found "+dbapiPluginSeq.elementAtPosition(i));
                  return dbapiPluginSeq.elementAtPosition(i);
              }
          }
          return "";
      }

    public:
      VCExport static GSeq <GString> *DBTypes()
      {
          init();
          return &typeSeq;
      }

      VCExport static IDSQL::ODBCDB getType()
      {
          return m_dbType;
      }
      VCExport static void setDBType(IDSQL::ODBCDB type)
      {
          m_dbType = type;
      }

      VCExport static void setConnData(GString db, GString uid, GString pwd, GString host)
      {
          /*
          m_db = db;
          printf("DB: %s\n", (char*)m_db);
          m_uid = uid;
          m_pwd = pwd;
          m_host = host;
          */

      }
      VCExport static IDSQL* getInstance(IDSQL* in)
      {
          init();
          if( in == NULL ) return NULL;
          if( in->getDBType() == IDSQL::NOTSET ) return NULL;
          return loadInstance(in->getDBType());
      }

#ifndef MAKE_VC
      static void getDSQLHandle(GString libPath)
      {
          if( !dsqlHandle ) dsqlHandle = dlopen(libPath, RTLD_NOW);
      }
      static void closeDSQLHandle()
      {
          if(dsqlHandle) dlclose(dsqlHandle);
          dsqlHandle = 0;
      }
#else
      static void getDSQLHandle(GString libPath)
      {
          if( !dsqlHandle ) dsqlHandle = LoadLibrary( libPath );
      }
      static void closeDSQLHandle()
      {
          if( dsqlHandle ) FreeLibrary(dsqlHandle);
          dsqlHandle = 0;
      }
#endif

#ifndef MAKE_VC
      static void getDBAPIHandle(GString libPath)
      {
          if( !dbapiHandle ) dbapiHandle = dlopen(libPath, RTLD_NOW);
      }
      static void closeDBAPIHandle()
      {
          if(dbapiHandle) dlclose(dbapiHandle);
          dbapiHandle = 0;
      }
#else
      static void getDBAPIHandle(GString libPath)
      {
          if( !dbapiHandle ) dbapiHandle = LoadLibrary( libPath );
      }
      static void closeDBAPIHandle()
      {
          if( dbapiHandle ) FreeLibrary(dbapiHandle);
          dbapiHandle = 0;
      }
#endif


      /******************************************************************
       * Load plugin for dynamic SQL and create instance of IDSQL
       *****************************************************************/
      VCExport static IDSQL* loadInstance(GString dbType)
      {
          init();

          GString libPath = dsqlPluginName(dbType);
          getDSQLHandle(libPath);
          if(!dsqlHandle)
          {
              deb("Could not load dsqlPlugin for "+dbType);
              return NULL;
          }

      #ifndef MAKE_VC //Linux
          typedef IDSQL* create_t();
          typedef void destroy_t(IDSQL*);
          create_t* crt=(create_t*)dlsym(dsqlHandle,"create");
          destroy_t* destr =(destroy_t*)dlsym(dsqlHandle,"destroy");
		#else
		  typedef IDSQL* (*CREATE) ();
          typedef void (*DESTROY)(IDSQL*);          
          CREATE crt = (CREATE) GetProcAddress( dsqlHandle, "create" );;
          DESTROY destr = (DESTROY) GetProcAddress( dsqlHandle, "destroy" );
		#endif
          if( !crt || !destr )
          {
             closeDSQLHandle();
             return NULL;
          }
          IDSQL* tst = crt();
          return tst;
      }
      VCExport static void closeHandle()
      {
          #ifdef MAKE_VC
          if(dsqlHandle) FreeLibrary(dsqlHandle);
          dsqlHandle = 0;
          #else
          if( dsqlHandle ) dlclose(dsqlHandle);
          dsqlHandle = 0;
          #endif
      }

      VCExport static IDSQL* loadInstance(IDSQL::ODBCDB type)
      {
          init();
          deb("loadInstance(IDSQL::ODBCDB type) called");

          if( type == IDSQL::DB2 ) return loadInstance("DB2");
          else if( type == IDSQL::SQLSERVER ) return loadInstance("SqlServer");
          return NULL;
      }
      /******************************************************************
       * Load plugin for database API (export/import, monitoring etc) and create instance of IDBAPI
       *****************************************************************/
      VCExport static IDBAPI* loadDBAPI(IDSQL::ODBCDB type)
      {
          init();
          deb("loadDBAPI(IDSQL::ODBCDB type) called");

          if( type == IDSQL::DB2 ) return loadDBAPI("DB2");
          else if( type == IDSQL::SQLSERVER ) return loadDBAPI("SqlServer");
          return NULL;
      }

      VCExport static IDBAPI* loadDBAPI(GString dbType)
      {
          deb("Load dbapi, start");
          init();

          test(dbType);
          dsqlPluginName(dbType);
          GString libPath = dbapiPluginName(dbType);
          getDBAPIHandle(libPath);
          deb("plugin for '"+dbType+"': "+libPath);

          if(!dbapiHandle)
          {
              deb("could not load dbapiPlugin for "+dbType+": "+libPath);
              return NULL;
          }

      #ifndef MAKE_VC //Linux
          typedef IDBAPI* create_t();
          typedef void destroy_t(IDBAPI*);
          create_t* crt=(create_t*)dlsym(dsqlHandle,"create");
          destroy_t* destr =(destroy_t*)dlsym(dsqlHandle,"destroy");
        #else
          typedef IDBAPI* (*CREATE) ();
          typedef void (*DESTROY)(IDBAPI*);
          CREATE crt = (CREATE) GetProcAddress( dsqlHandle, "create" );;
          DESTROY destr = (DESTROY) GetProcAddress( dsqlHandle, "destroy" );
        #endif
          if( !crt || !destr )
          {
             closeDSQLHandle();
             return NULL;
          }
          IDBAPI* tst = crt();
          return tst;
      }

      static void deb(GString msg)
      {
          msg = "dsqlInst: "+msg;
          msg.sayIt();
      }

};



#endif // DSQLINST_HPP


