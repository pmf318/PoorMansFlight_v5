


TARGET= mariadb
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

SOURCES += mariadb.cpp
HEADERS += mariadb.hpp

unix{
    INCLUDEPATH += /usr/include/mysql
    LIBS+= ../../lib/libglengineQT.a
    # "echo $(mysql_config --libs)" yields "mariadb" on Mageia7 and "mariadbclient" on Debian
    #LIBS+=-L/usr/lib/x86_64-linux-gnu $(mysql_config --libs) -lpthread -lz -lm -ldl
    LIBS+=-L/usr/lib/x86_64-linux-gnu $$system(mysql_config --libs)
    #LIBS+=-L/usr/lib/x86_64-linux-gnu -lmariadbclient -lpthread -lz -lm -ldl

}

win32-g++{
    LIBS+= -l odbc32
    LIBS+= ../../lib/libglengineQT.a
}

win32-msvc{
	contains(QMAKE_TARGET.arch, x86_64) {
		INCLUDEPATH += $$(MARIADB_INC_64)
		LIBS += $$(MARIADB_LIB_64)/libmariadb.lib
	}
	else{
		INCLUDEPATH += $$(MARIADB_INC_32)
		LIBS += $$(MARIADB_LIB_32)/libmariadb.lib
	}
    LIBS += user32.lib
    
    LIBS += WS2_32.lib
    LIBS += ../../lib/glengineQT.lib
}
   
win32{
        QMAKE_POST_LINK+=del "..\..\..\plugins\mariadb.exp"
        QMAKE_POST_LINK+=del "..\..\..\plugins\mariadb.lib"
        QMAKE_POST_LINK+=del "..\..\..\plugins\mariadb.a"
        QMAKE_POST_LINK+=del "..\..\..\plugins\mariadb.dll.manifest"
}
unix:!macx{
}
