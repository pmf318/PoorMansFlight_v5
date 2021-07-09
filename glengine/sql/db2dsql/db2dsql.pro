
############################
# SET VERSION HERE 
############################
PMF_VERSION=5126


TARGET= db2dsql
OBJECTS_DIR=./obj
DESTDIR=../../../plugins
CONFIG+=release
QMAKE_CLEAN += *.c 
QMAKE_CLEAN +=PMF*.cpp 
QMAKE_CLEAN +=PMF*.sqc

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
#CONFIG += debug
#CONFIG+=DEBUG
unix{
    CONFIG+=debug
}

unix{
QMAKE_CXXFLAGS += -Wno-narrowing
}

## Qt5 doc mentions this, but at least for now it's apparently not needed.
#CONFIG+=create_prl


### from qt5 up
QT += widgets


CONFIG+=plugin
TEMPLATE=lib
DEFINES += NO_QT
win32-msvc{
	CONFIG  += dll
	DEFINES+=STATIC
	DEFINES += MAKE_VC
	DEFINES -= UNICODE
	CONFIG-=embed_manifest_dll
}
win32-g++{
        CONFIG  += dll
        DEFINES+=STATIC
        DEFINES += __MINGW32__
        DEFINES -= UNICODE
        CONFIG-=embed_manifest_dll
        QMAKE_CXXFLAGS_WARN_ON += -Wno-narrowing
}

### call precomp or gen_bnd.cmd to precompile dsqlobj.sqc 
### and generate the BIND file. 

QMAKE_EXTRA_TARGETS+=bindtarget
win32{

	bindtarget.target=PMF$${PMF_VERSION}.cpp
        bindtarget.commands = gen_bnd.cmd $$db $${PMF_VERSION}W $$uid $$pwd
	bindtarget.depends = FORCE
	bindtarget.CONFIG += no_link
	bindtarget.variable_out = SOURCES
}
unix:!macx{
        bindtarget.target=PMF$${PMF_VERSION}L.cpp
        bindtarget.commands = ./precomp $$db PMF$${PMF_VERSION}L ../../../plugins $$uid $$pwd
#	bindtarget.depends = FORCE

}

win32{
	PRE_TARGETDEPS+=PMF$${PMF_VERSION}.cpp
#        QMAKE_POST_LINK=del "PMF$${PMF_VERSION}W.*"
	QMAKE_POST_LINK+=del "..\..\..\plugins\db2dsql.exp"	
        QMAKE_POST_LINK+=del "..\..\..\plugins\db2dsql.a"
	QMAKE_POST_LINK+=del "..\..\..\plugins\db2dsql.lib"	
}
unix:!macx{
        PRE_TARGETDEPS+=PMF$${PMF_VERSION}L.cpp
##	QMAKE_POST_LINK=rm PMF$${PMF_VERSION}L.*
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

win32{
        SOURCES += PMF$${PMF_VERSION}W.cpp
}
unix:!macx{
        SOURCES += PMF$${PMF_VERSION}L.cpp
}
HEADERS += dsqlobj.hpp
unix{
#    HEADERS += ../../inc/growhdl.hpp
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
#   SOURCES += ../../base/gseq/gseq.cpp
#    SOURCES += ../../base/growhdl/growhdl.cpp
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
   
win32-msvc{
	QMAKE_POST_LINK+=del "..\..\..\plugins\db2dsql.exp"
	QMAKE_POST_LINK+=del "..\..\..\plugins\db2dsql.lib"
	QMAKE_POST_LINK+=del "..\..\..\plugins\db2dsql.dll.manifest"	
}
