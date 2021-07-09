###Set Version here.
PMF_VERSION=5000

################################
######## Use QT for some tasks.
######## Remove for pure C/C++
DEFINES += QT_GTHREAD
DEFINES += QT4_DSQL
DEFINES += MAP_GQSTRING
DEFINES += UDB_QT4
DEFINES += NO_QT
###############################
CONFIG += THREAD
CONFIG+=STATICLIB 

## Qt5 doc mentions this, but at least for now it's apparently not needed.
#CONFIG+=create_prl

CONFIG+=STATIC
#CONFIG+=DEBUG
#CONFIG-=embed_manifest_dll
DEFINES+=STATIC

### from qt5 up
QT += widgets


##QMAKE_CXXFLAGS_RELEASE -= -O2
##QMAKE_CLAGS_RELEASE -= -O2


##ODBC## 
#QT += sql
##ODBC## 
#QTPLUGIN += qsqlodbc
#CONFIG += plugin

TEMPLATE=lib

win32{
	DEFINES+=STATIC
	DEFINES += MAKE_VC
	DEFINES -= UNICODE
	CONFIG+=STATICLIB
	CONFIG+=STATIC
	CONFIG-=embed_manifest_dll
}

MAKEFILE=glengineQT.mak


INCLUDEPATH += ./inc



CONFIG += qt

#SOURCES += ./sql/glinehdl/glinehdl.cpp
#!HEADERS += ./inc/cliobj.hpp
#HEADERS += ./inc/dsqlinst.hpp
#HEADERS += ./inc/dsqlobj.hpp
#HEADERS += ./inc/glinehdl.hpp
#HEADERS += ./inc/gsnap.hpp
#HEADERS += ./inc/udbapi.hpp
#HEADERS += ./inc/dsqlapi.hpp
HEADERS += ./inc/gstring.hpp
HEADERS += ./inc/gstuff.hpp
HEADERS += ./inc/gfile.hpp
HEADERS += ./inc/gthread.hpp
HEADERS += ./inc/seqitem.hpp
HEADERS += ./inc/gseq.hpp
##ODBC## 
##HEADERS += ./inc/odbcdsql.hpp

#!SOURCES += ./sql/cliobj/cliobj.cpp
#SOURCES += ./sql/dsqlapi/dsqlapi.cpp
#SOURCES += ./sql/gsnap/gsnap.cpp
#SOURCES += ./sql/udbapi/udbapi.cpp
##ODBC##
##SOURCES += ./sql/odbcdsql/odbcdsql.cpp

SOURCES += ./base/gstring/gstring.cpp
SOURCES += ./base/gstuff/gstuff.cpp
SOURCES += ./base/gfile/gfile.cpp
SOURCES += ./base/gthread/gthread.cpp
SOURCES += ./base/seqitem/seqitem.cpp
SOURCES += ./base/gseq/gseq.cpp

TARGET= glengineQT
OBJECTS_DIR=./obj
DESTDIR=./lib

############ link
unix{
#LIBS+=-L$(HOME)/sqllib/lib -ldb2
##ODBC## 
###LIBS+=$(QTDIR)/plugins/sqldrivers/libqsqlodbc.so
}
win32{
#LIBS += $$(DB2PATH)/lib/db2api.lib
#LIBS += $$(DB2PATH)/lib/db2cli.lib
}
   
