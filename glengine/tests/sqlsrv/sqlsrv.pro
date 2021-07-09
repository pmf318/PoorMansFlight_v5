##################################################################
#
# Usage: qmake pmf.pro
#
##################################################################
# Windows only: Set path to DB2
# The envornment variable DB2PATH should point to something like
# c:\program files\sqllib
# or
# c:\program files\ibm\sqllib
# depending on DB2 version
# If DB2PATH is not set you need to set it manually
#
##################################################################

TEMPLATE=app
################
## NO_QT EXCLUDES QT2/3.
DEFINES += NO_QT
CONFIG+=release
CONFIG+=console
### from qt5 up
QT += widgets

################
## THESE DEFINES ARE NEEDED TO BUILD AGAINST QT4
DEFINES += MAP_GQSTRING
DEFINES += QT_GTHREAD
DEFINES += QT4_DSQL
DEFINES += UDB_QT4
DEFINES -= UNICODE
###############

###CONFIG+=link_prl
CONFIG += THREAD
#CONFIG -= DEBUG

#QT += sql
##ODBC## QTPLUGIN += qsqlodbc


#CONFIG += STATIC
CONFIG += embed_manifest_exe

#QMAKE_LFLAGS += /MANIFEST

win32{
DEFINES += MAKE_VC
INCLUDEPATH += ../../inc
DESTDIR=./
}
unix:!macx{
INCLUDEPATH += ../../inc
CONFIG += DEBUG
}
OBJECTS_DIR=./obj
MOC_DIR=./obj

MAKEFILE = Makefile

OBJECTS_DIR=./obj
MOC_DIR=./obj


CONFIG += qt
SOURCES += test.cpp

#HEADERS += ../../inc/gstring.hpp
#HEADERS += ../../inc/gstuff.hpp
#HEADERS += ../../inc/gfile.hpp
#HEADERS += ../../inc/gthread.hpp
#HEADERS += ../../inc/seqitem.hpp
#HEADERS += ../../inc/gseq.hpp
#HEADERS += ../../inc/glinehdl.hpp

#SOURCES += ../../base/gstring/gstring.cpp
#SOURCES += ../../base/gstuff/gstuff.cpp
#SOURCES += ../../base/gfile/gfile.cpp
#SOURCES += ../../base/gthread/gthread.cpp
#SOURCES += ../../base/seqitem/seqitem.cpp
#SOURCES += ../../base/gseq/gseq.cpp
#SOURCES += ../../base/glinehdl/glinehdl.cpp


TARGET= test

win32{
#LIBS += ./glengine/lib/libglengine.a
LIBS += ../../lib/glengineQT.lib
LIBS += WS2_32.lib user32.lib
}
unix:!macx{
LIBS += ../../lib/libglengineQT.a
#LIBS += -L$(HOME)/sqllib/lib -ldb2
LIBS += -ldl
##ODBC## LIBS += $(QTDIR)/plugins/sqldrivers/libqsqlodbc.so
}


