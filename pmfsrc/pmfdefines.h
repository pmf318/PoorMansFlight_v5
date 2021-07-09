//
//  This file is part of PMF (Poor Man's Flight), see README.PMF for details
//  Author: Gregor Leipelt (gregor@leipelt.de)  (C) G. Leipelt 2011
//
#include <QDir>
#include <iostream>

#ifdef MAKE_VC
#include <direct.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif


#ifndef _PMFDEFINES_
#define _PMFDEFINES_
static GString _selStringCB   = "<Select>";
static GString _newStringCB   = "<New...>";
static GString _helpStringCB  = "<Help>";
static GString _newTabString  = "...";
static GString _SQLCMD        = "SQL-CMD";
static GString _UPDRow        = "UPD";
static GString _UPDMark       = "U";
static GString _INSRow        = "INS";
static GString _INSMark       = "I";
static GString _NEWRow        = "NEW";
static GString _NEWMark       = "N";
static GString _CFGFILE       = "pmfcfg";
static GString _NO_DB         = "<None>";
static GString _BM_FILE       = "bookmarks";
static GString _HST_FILE      = "sqlhist";
static GString _CONNSET_FILE  = "connset";
static GString _INIT_FILE     = "init";
static GString _CFG_DIR       = "pmf5";
static GString _RESTORE_FILE  = "restore_";
static GString _PMF_CMD       = "@PMF_CMD@";
static GString _PMF_PASTE_SEP = "@PMF_SEP@";
static GString _PMF_IS_LOB    = "@PMF_LOB@";
static GString _TMP_DIR       = "PMF_tmp";
static GString _IDENTITY      = "[IDENTITY]";
static GString _EXTSQL_DIR    = "extSql";
static GString _USERACT_DIR   = "userActions";
static GString _PMF_HTTP_SRV  = "www.leipelt.de";
static GString _PMF_HTTP_FILE_LOCATION = "/download/PMF5/pmf5setup.exe";
static GString _DARK_THEME    = " (dark)";
static GString _EXPIMPSET_DIR   = "expImpSettings";
static GString _XMLDDLSET_DIR   = "xmlDdlSettings";
static GString _OPTIONSTABSET_DIR   = "optionsTabSettings";
static GString _CRLF_MARK     = "@@CRLF@@";


static GString _ENTERTOSAVE = "enterToSave";
static GString _TEXTCOMPLETER = "textCompleter";
static GString _COUNTROWS = "countRows";
static GString _CONVERTGUID = "convertGuid";
static GString _READUNCOMMITTED = "readUncommitted";
static GString _HIDESYSTABS = "hideSysTabs";
static GString _REFRESHONFOCUS = "refreshOnFocus";
static GString _USEESCTOCLOSE = "useEscToClose";
static GString _CHECKSENABLED = "checksEnabled";
static GString _SHOWCLOSEONTABS = "showCloseOnTabs";
static GString _LAST_SQL_TABLE  = " [TABLE: ";

static GString _LOCAL_CFG_FLAG  = "local.cfg";

#define MaxUserActions 100

static GString newVersionFileLocation()
{
    return QDir::homePath()+"//pmf5setup.exe";
}

static GString basePath(GString subDir, int useLocalCfg = 0)
{
    GString pathSep;
#ifdef MAKE_VC
    pathSep = "\\";
#else
    pathSep = "/";
#endif
    GString flagFile = GString(QDir::currentPath())+pathSep+_LOCAL_CFG_FLAG;
    QFileInfo fileInfo(flagFile);

    GString cfgRoot;
    if( fileInfo.exists() && useLocalCfg ) cfgRoot = QDir::currentPath();
    else cfgRoot = QDir::homePath();
    GString path = cfgRoot+pathSep+_CFG_DIR+pathSep;
    if( subDir.length() ) path += subDir+pathSep;
    QDir().mkpath(path);
    return path;
}

static GString extSqlSaveDir(){
    return basePath(_EXTSQL_DIR);
}

static GString userActionsDir(){
    return basePath((_USERACT_DIR));
}

static GString expImpSettingsDir(){
    return basePath(_EXPIMPSET_DIR, 1);
}

//static GString xmlDdlSettingsDir(){
//    return basePath(_XMLDDLSET_DIR);
//}


static GString optionsTabSettingsDir(){
    return basePath(_OPTIONSTABSET_DIR);
}

static GString autoCatalogFilePath(){
    return basePath("", 1);
}


#endif
