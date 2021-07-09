##################################################################
#
# Usage: "qmake <this file>"
#
##################################################################
# Windows only: Set path to DB2
# The environment variable DB2PATH should point to something like
# c:\program files\sqllib
# or 
# c:\program files\ibm\sqllib
# depending on DB2 version
# If DB2PATH is not set you need to set it manually
#
##################################################################


IS_MSVC_STATIC = $$(MSVC_STATIC)
!isEmpty(IS_MSVC_STATIC){
	DEFINES += MSVC_STATIC_BUILD
}


CONFIG+=release
unix{
    CONFIG+=debug
}
#CONFIG += console
#CONFIG+=STATIC
#CONFIG+=staticlib
#CONFIG+=STATICLIB

!isEmpty(IS_MSVC_STATIC){
	win32-msvc{
		CONFIG+=STATIC
		#CONFIG += staticlib
		DEFINES += STATIC
		#QT      += core gui
		DEFINES += QT_NODLL
		DEFINES += QT_STATIC_BUILD
	
		QMAKE_CFLAGS_RELEASE += /MT
		QMAKE_CXXFLAGS_RELEASE += /MT
		QMAKE_CFLAGS_RELEASE -= -MD
		QMAKE_CXXFLAGS_RELEASE -= -MD
		QMAKE_LFLAGS += /MANIFEST
		CONFIG += embed_manifest_exe		
	}
}

################
# Set version here:
# ALSO: SET THE SAME STRING IN ./glengine/sql/db2dsql.pro !
PMF_VERSION=5126

################
################
DEFINES += PMF_VER=\"$${PMF_VERSION}\"
TEMPLATE=app
################
## NO_QT EXCLUDES QT2/3. 
DEFINES += NO_QT

### from qt5 up
QT += widgets
QT += xml

###QT += network

################
## THESE DEFINES ARE NEEDED TO BUILD AGAINST QT4
DEFINES += MAP_GQSTRING
DEFINES += QT_GTHREAD
DEFINES += QT4_DSQL
DEFINES += UDB_QT4
DEFINES -= UNICODE
DEFINES += DEBUG_GUI

DEFINES += QT_STATICPLUGIN 

win32{
	QTPLUGIN += qwindows
}

DEFINES += NOMINMAX
###############
#CONFIG -= import_plugins

CONFIG+=create_prl
CONFIG+=link_prl
CONFIG += THREAD
##QT += sql
##ODBC## QTPLUGIN += qsqlodbc


win32{
QMAKE_CXXFLAGS_RELEASE -= -Zc:strictStrings
QMAKE_CFLAGS_RELEASE -= -Zc:strictStrings
QMAKE_CFLAGS -= -Zc:strictStrings
QMAKE_CXXFLAGS -= -Zc:strictStrings

win32-msvc{
	DEFINES += MAKE_VC
}
win32-g++{
	DEFINES -= MAKE_VC
	DEFINES += __MINGW32__
}

INCLUDEPATH += ../glengine/inc
INCLUDEPATH += ../zlib
DESTDIR=./
#Apparently needed to get "qplatformnativeinterface", i.e. the main windows HWND
QT += gui-private
}
unix:!macx{
INCLUDEPATH += ../glengine/inc
}
OBJECTS_DIR=./obj
MOC_DIR=./obj

MAKEFILE = pmf.mak

OBJECTS_DIR=./obj
MOC_DIR=./obj


CONFIG += qt
SOURCES += ./main.cpp \
    sqlHighlighter.cpp \
    userActions.cpp \
    userInput.cpp \
    pmfTableWdgt.cpp \
    downloader.cpp
SOURCES += ./helper.cpp
SOURCES += ./tableSelector.cpp
SOURCES += ./db2menu.cpp
SOURCES += ./pmf.cpp
SOURCES += ./loginbox.cpp
SOURCES += ./tabEdit.cpp
SOURCES += ./exportBox.cpp
SOURCES += ./threadBox.cpp
SOURCES += ./deltab.cpp
SOURCES += ./extSQL.cpp
SOURCES += ./finddbl.cpp
#SOURCES += ./mtIndex.h
#SOURCES += ./reorgTbl.cpp
SOURCES += ./reorgAll.cpp
SOURCES += ./simpleShow.cpp
SOURCES += ./newIndex.cpp
SOURCES += ./tabSpace.cpp
SOURCES += ./tbSize.cpp
SOURCES += ./getSnap.cpp
SOURCES += ./pmfCfg.cpp
SOURCES += ./importBox.cpp
SOURCES += ./getclp.cpp
SOURCES += ./pmfSchemaCB.cpp
SOURCES += ./querydb.cpp
SOURCES += ./bookmark.cpp
SOURCES += ./connSet.cpp
SOURCES += ./editBm.cpp
SOURCES += ./showXML.cpp
SOURCES += ./editDoubleByte.cpp
SOURCES += ./txtEdit.cpp
SOURCES += ./xmlEdit.cpp
SOURCES += ./showXPath.cpp
SOURCES += ./expImpOptions.cpp
SOURCES += ./getCodepage.cpp
SOURCES += ./clickLabel.cpp
SOURCES += ./newForeignKey.cpp
SOURCES += ./pmfDropZone.cpp
SOURCES += ./pmfPushButton.cpp
SOURCES += ./odbcMdf.cpp
SOURCES += ./multiImport.cpp
SOURCES += ./allTabDDL.cpp
SOURCES += ./optionsTab.cpp
SOURCES += ./catalogInfo.cpp
SOURCES += ./pmfTable.cpp
SOURCES += ./pmfColumn.cpp
SOURCES += ./createCheck.cpp
SOURCES += ./catalogDB.cpp
SOURCES += ./monExport.cpp
SOURCES += ./pmfTableItem.cpp


HEADERS += ./pmf.h \
    sqlHighlighter.h \
    userActions.h \
    userInput.h \
    pmfTableWdgt.h \
    downloader.h
HEADERS += ./helper.h
HEADERS += ./tableSelector.h
HEADERS += ./db2menu.h
HEADERS += ./loginbox.h
HEADERS += ./tabEdit.h
HEADERS += ./exportBox.h
HEADERS += ./threadBox.h
HEADERS += ./deltab.h
HEADERS += ./extSQL.h
HEADERS += ./finddbl.h
#HEADERS += ./mtIndex.h
#HEADERS += ./reorgTbl.h
HEADERS += ./reorgAll.h
HEADERS += ./simpleShow.h
HEADERS += ./newIndex.h
HEADERS += ./tabSpace.h
HEADERS += ./tbSize.h
HEADERS += ./getSnap.h
HEADERS += ./pmfCfg.h
HEADERS += ./importBox.h
HEADERS += ./getclp.h
HEADERS += ./pmfSchemaCB.h
HEADERS += ./querydb.h
HEADERS += ./bookmark.h
HEADERS += ./connSet.h
HEADERS += ./editBm.h
HEADERS += ./showXML.h
HEADERS += ./editDoubleByte.h
HEADERS += ./txtEdit.h
HEADERS += ./xmlEdit.h
HEADERS += ./showXPath.h
HEADERS += ./expImpOptions.h
HEADERS += ./getCodepage.h
HEADERS += ./clickLabel.h
HEADERS += ./newForeignKey.h
HEADERS += ./pmfDropZone.h
HEADERS += ./pmfPushButton.h
HEADERS += ./odbcMdf.h
HEADERS += ./multiImport.h
HEADERS += ./allTabDDL.h
HEADERS += ./optionsTab.h
HEADERS += ./cataloInfo.h
HEADERS += ./pmfTable.h
HEADERS += ./pmfColumn.h
HEADERS += ./createCheck.h
HEADERS += ./catalogDB.h
HEADERS += ./catalogInfo.h
HEADERS += ./monExport.h
HEADERS += ./pmfTableItem.h



TARGET= pmf

RC_FILE = ./pmfico.rc
RESOURCES = ./icons/pmf.qrc
DESTDIR=..



win32-msvc{
	LIBS += ../glengine/lib/glengineQT.lib 
	LIBS += WS2_32.lib Shell32.lib msvcrt.lib kernel32.lib user32.lib odbc32.lib odbccp32.lib
	LIBS += legacy_stdio_definitions.lib
	!isEmpty(IS_MSVC_STATIC){
		LIBS += legacy_stdio_definitions.lib
		LIBS += -L/qt-static/lib 
		LIBS += libcmt.lib
		LIBS += msvcrt.lib
	}
	contains(QMAKE_TARGET.arch, x86_64) {
		LIBS += ../zlib/zdll.lib
	}
	else{
		LIBS += ../zlib/zlib.lib		
	}
}
win32{
    #LIBS += ../glengine/lib/libglengineQT.a
	#LIBS += ../zlib/zlib.lib
}

unix:!macx{
	LIBS += ../glengine/lib/libglengineQT.a
	LIBS += -ldl -lz
}


