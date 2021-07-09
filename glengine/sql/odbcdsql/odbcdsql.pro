


TARGET= odbcdsql
OBJECTS_DIR=./obj
DESTDIR=../../../plugins


################################
######## Use QT for some tasks.
######## Remove for pure C/C++
DEFINES += QT_GTHREAD
DEFINES += QT4_DSQL
DEFINES += MAP_GQSTRING
DEFINES += UDB_QT4
DEFINES += DEBUG_GUI
###############################
CONFIG += THREAD
CONFIG+=release
unix{
    CONFIG+=debug
}

#CONFIG-=debug

## Qt5 doc mentions this, but at least for now it's apparently not needed.
#CONFIG+=create_prl


### from qt5 up
QT += widgets


CONFIG+=plugin

##QMAKE_CXXFLAGS_RELEASE -= -O2
##QMAKE_CLAGS_RELEASE -= -O2


##ODBC## 
#QT += sql
##ODBC## 
#QTPLUGIN += qsqlodbc


TEMPLATE=lib
DEFINES += NO_QT
win32{
	DEFINES+=STATIC
	DEFINES += MAKE_VC
	DEFINES -= UNICODE
	CONFIG+=dll
	CONFIG-=embed_manifest_dll
}

#MAKEFILE=odbcdsql.mak

DEFINES+=QT4_DSQL


INCLUDEPATH += ../../inc
INCLUDEPATH += .

CONFIG += qt

SOURCES += odbcdsql.cpp
HEADERS += odbcdsql.hpp

unix{
#    HEADERS += ../../inc/gstring.hpp
#    HEADERS += ../../inc/gstuff.hpp
#    HEADERS += ../../inc/gdebug.hpp
#    HEADERS += ../../inc/gfile.hpp
#    HEADERS += ../../inc/gthread.hpp
#    HEADERS += ../../inc/seqitem.hpp
#    HEADERS += ../../inc/gseq.hpp
#    HEADERS += ../../inc/growhdl.hpp

#    SOURCES += ../../base/gstring/gstring.cpp
#    SOURCES += ../../base/gstuff/gstuff.cpp
#    SOURCES += ../../base/gdebug/gdebug.cpp
#    SOURCES += ../../base/gfile/gfile.cpp
#    SOURCES += ../../base/gthread/gthread.cpp
#    SOURCES += ../../base/seqitem/seqitem.cpp
#    SOURCES += ../../base/gseq/gseq.cpp
#    SOURCES += ../../base/growhdl/growhdl.cpp
}

unix{
    LIBS+= -l odbc
    LIBS+= ../../lib/libglengineQT.a
}

win32-g++{
    LIBS+= -l odbc32
    LIBS+= ../../lib/libglengineQT.a
}

win32-msvc{
    LIBS += user32.lib
    LIBS += odbc32.lib
    LIBS += WS2_32.lib
    LIBS += ../../lib/glengineQT.lib
}
   
win32{
	QMAKE_POST_LINK+=del "..\..\..\plugins\odbcdsql.exp"	
	QMAKE_POST_LINK+=del "..\..\..\plugins\odbcdsql.lib"	
        QMAKE_POST_LINK+=del "..\..\..\plugins\odbcdsql.a"
	QMAKE_POST_LINK+=del "..\..\..\plugins\odbcdsql.dll.manifest"		
}
unix:!macx{
}
