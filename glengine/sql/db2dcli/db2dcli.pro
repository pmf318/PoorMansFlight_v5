


TARGET= db2dcli
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
unix{
    INCLUDEPATH *= $(HOME)/sqllib/include
}
win32{
    INCLUDEPATH *= $$(DB2PATH)/include
}


CONFIG += qt

SOURCES += db2dcli.cpp
HEADERS += db2dcli.hpp \
    MySpecStrings.h

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
LIBS += -L$(HOME)/sqllib/lib -ldb2
#LIBS+= -l odbc
LIBS+= ../../lib/libglengineQT.a
}

win32-g++{
    LIBS+=-L$(HOME)/sqllib/lib
    LIBS+=../../lib/libglengineQT.a
    LIBS += $$(DB2PATH)/lib/win32/db2api.lib
    LIBS += $$(DB2PATH)/lib/win32/db2cli.lib

}

win32-msvc{
	contains(QMAKE_TARGET.arch, x86_64) {
		
			LIBS += user32.lib
			LIBS += $$(DB2PATH)/lib/db2cli.lib
			LIBS += WS2_32.lib user32.lib
			LIBS += ../../lib/glengineQT.lib	
		}
		else
		{
			LIBS += user32.lib
			LIBS += $$(DB2PATH)/lib/win32/db2cli.lib
			LIBS += WS2_32.lib user32.lib
			LIBS += ../../lib/glengineQT.lib
	}
}


win32{
        QMAKE_POST_LINK+=del "..\..\..\plugins\db2dcli.exp"
        QMAKE_POST_LINK+=del "..\..\..\plugins\db2dcli.lib"
        QMAKE_POST_LINK+=del "..\..\..\plugins\db2dcli.a"
        QMAKE_POST_LINK+=del "..\..\..\plugins\db2dcli.dll.manifest"
}
unix:!macx{
}
