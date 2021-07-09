
set DB2PATH=c:\Program Files\IBM\SQLLIB\
set LIBPATH=%LIBPATH%;%DB2PATH%\lib


SET MINGW=d:\qtMinGw\Tools\mingw730_32\
SET QTDIR=d:\QtMinGW_static\


set path=%PATH%;%MINGW%\;%MINGW%\bin;


rem *** No need to edit this ***
set path=%PATH%;%QTDIR%\;%QTDIR%\bin;
set path=%PATH%;%QTDIR%\qt\bin;
set include=%INCLUDE%;%QTDIR%\include\QtGui\;
set include=%INCLUDE%;%QTDIR%\include\QtCore\;
set include=%INCLUDE%;%QTDIR%\include\Qt\;
set include=%INCLUDE%;%QTDIR%\include\;
set include=%INCLUDE%;%QTDIR%\include\QtWidgets;
set include=%INCLUDE%;%QTDIR%\;
