


TARGET= postgres
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

#MAKEFILE=mariadb.mak

DEFINES+=QT4_DSQL


INCLUDEPATH += ../../inc
INCLUDEPATH += .

CONFIG += qt

SOURCES += postgres.cpp
HEADERS += postgres.hpp

unix{
    INCLUDEPATH += /usr/include/postgresql
    LIBS+= ../../lib/libglengineQT.a
    LIBS+= -L/usr/local/pgsql/lib -lpq
}

win32-g++{
}

win32-msvc{
	contains(QMAKE_TARGET.arch, x86_64) {
		INCLUDEPATH += $$(POSTGRES_INC_64)
        LIBS += $$(POSTGRES_LIB_64)/libpq.lib
	}
	else{
		INCLUDEPATH += $$(POSTGRES_INC_32)
        LIBS += $$(POSTGRES_LIB_32)/libpq.lib
	}
    LIBS += user32.lib
    
    LIBS += WS2_32.lib
    LIBS += ../../lib/glengineQT.lib
    QMAKE_POST_LINK+=del "..\..\..\plugins\postgres.exp"
	QMAKE_POST_LINK+=del "..\..\..\plugins\postgres.lib"		
}
   
