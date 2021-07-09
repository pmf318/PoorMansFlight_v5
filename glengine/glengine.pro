
################################
######## Use QT for some tasks.
######## Remove for pure C/C++
DEFINES += QT_GTHREAD
DEFINES += QT4_DSQL
DEFINES += MAP_GQSTRING
DEFINES += UDB_QT4
DEFINES += DEBUG_GUI

#Don't use QT3:
DEFINES += NO_QT  
###############################
CONFIG += THREAD


## Qt5 doc mentions this, but at least for now it's apparently not needed.
#CONFIG+=create_prl
#CONFIG += debug
#CONFIG+=DEBUG
unix{
    CONFIG+=debug
}

#CONFIG+=release



### from qt5 up
QT += widgets


TARGET= glengineQT

# This calls 'make -f Makfile staticlib'
# Do NOT set this uppercase, it will obviously not work.
# (I just spent hours figuring this out because in qt4 it used to work)
CONFIG+=staticlib


TEMPLATE=lib


win32-g++{
	DEFINES += __MINGW32__
	DEFINES -= UNICODE
}
win32-msvc{
        DEFINES += MAKE_VC
        DEFINES -= UNICODE
        CONFIG-=embed_manifest_dll
}

MAKEFILE=glengine.mak


INCLUDEPATH += ./inc



#CONFIG += qt

HEADERS += ./inc/gstring.hpp
HEADERS += ./inc/glist.hpp
HEADERS += ./inc/gstuff.hpp
HEADERS += ./inc/gfile.hpp
HEADERS += ./inc/gthread.hpp
HEADERS += ./inc/seqitem.hpp
HEADERS += ./inc/gseq.hpp
HEADERS += ./inc/growhdl.hpp
HEADERS += ./inc/dsqlplugin.hpp
HEADERS += ./inc/dbapiplugin.hpp
HEADERS += ./inc/gsocket.hpp
HEADERS += ./inc/gdebug.hpp
HEADERS += ./inc/showDeb.hpp
HEADERS += ./inc/gserver.hpp
HEADERS += ./inc/debugMessenger.hpp
HEADERS += ./inc/debugObserver.hpp
HEADERS += ./inc/messenger.hpp
HEADERS += ./inc/gstringlist.hpp
HEADERS += ./inc/gxml.hpp

##ODBC## 
##HEADERS += ./inc/odbcdsql.hpp


SOURCES += ./base/gstring/gstring.cpp
#SOURCES += ./base/glist/glist.cpp
SOURCES += ./base/gstuff/gstuff.cpp
SOURCES += ./base/gfile/gfile.cpp
SOURCES += ./base/gthread/gthread.cpp
SOURCES += ./base/seqitem/seqitem.cpp
SOURCES += ./base/gseq/gseq.cpp
SOURCES += ./base/gsocket/gsocket.cpp
SOURCES += ./base/growhdl/growhdl.cpp
SOURCES += ./base/gdebug/gdebug.cpp
SOURCES += ./base/gdebug/showDeb.cpp
SOURCES += ./base/dsqlplugin/dsqlplugin.cpp
SOURCES += ./base/dbapiplugin/dbapiplugin.cpp
SOURCES += ./base/gserver/gserver.cpp
SOURCES += ./base/debObserver/debugObserver.cpp
SOURCES += ./base/debObserver/messenger.cpp
SOURCES += ./base/gstringlist/gstringlist.cpp
SOURCES += ./base/gxml/gxml.cpp



OBJECTS_DIR=./obj
DESTDIR=./lib

############ link
unix{
}
win32{
}
   
