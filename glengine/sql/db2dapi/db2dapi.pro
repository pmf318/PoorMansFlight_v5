

TARGET= db2dapi
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

TEMPLATE=lib
DEFINES += NO_QT
win32{
	CONFIG  += dll
	DEFINES+=STATIC
	DEFINES += MAKE_VC
	DEFINES -= UNICODE
	CONFIG-=embed_manifest_dll
}


INCLUDEPATH += ../../glengine/inc
INCLUDEPATH += .
unix{
	INCLUDEPATH += $(HOME)/sqllib/include
}
win32{
	INCLUDEPATH += $$(DB2PATH)/include
}


INCLUDEPATH += ../../inc

CONFIG += qt
SOURCES += db2dapi.cpp
HEADERS += db2dapi.hpp

unix{
#    HEADERS += ../../inc/gstring.hpp
#    HEADERS += ../../inc/gstuff.hpp
#    HEADERS += ../../inc/gdebug.hpp
#    HEADERS += ../../inc/gfile.hpp
#    HEADERS += ../../inc/gthread.hpp
#    HEADERS += ../../inc/seqitem.hpp
#    HEADERS += ../../inc/gseq.hpp

#    SOURCES += ../../base/gstring/gstring.cpp
#    SOURCES += ../../base/gstuff/gstuff.cpp
#    SOURCES += ../../base/gdebug/gdebug.cpp
#    SOURCES += ../../base/gfile/gfile.cpp
#    SOURCES += ../../base/gthread/gthread.cpp
#    SOURCES += ../../base/seqitem/seqitem.cpp
#    SOURCES += ../../base/gseq/gseq.cpp
}

############ link
unix{
LIBS+=-L$(HOME)/sqllib/lib -ldb2
LIBS+=../../lib/libglengineQT.a
}
win32-g++{
    LIBS+=-L$(HOME)/sqllib/lib
    LIBS+=../../lib/libglengineQT.a
    LIBS += $$(DB2PATH)/lib/win32/db2api.lib
    LIBS += $$(DB2PATH)/lib/win32/db2cli.lib

}

win32-msvc{
	contains(QMAKE_TARGET.arch, x86_64) {
		LIBS += $$(DB2PATH)/lib/db2api.lib
		LIBS += $$(DB2PATH)/lib/db2cli.lib
		LIBS += WS2_32.lib user32.lib
		LIBS += ../../lib/glengineQT.lib	
	}
	else
	{
		LIBS += $$(DB2PATH)/lib/win32/db2api.lib
		LIBS += $$(DB2PATH)/lib/win32/db2cli.lib
		LIBS += WS2_32.lib user32.lib
		LIBS += ../../lib/glengineQT.lib
	}
}
   
win32{
        QMAKE_POST_LINK+=del "..\..\..\plugins\db2dapi.exp"
        QMAKE_POST_LINK+=del "..\..\..\plugins\db2dapi.lib"
        QMAKE_POST_LINK+=del "..\..\..\plugins\db2dapi.a"
        QMAKE_POST_LINK+=del "..\..\..\plugins\db2dapi.dll.manifest"
}
unix:!macx{
}
