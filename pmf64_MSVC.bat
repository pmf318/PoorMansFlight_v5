call c:"\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"



set DB2PATH=c:\Program Files\IBM\SQLLIB\
set LIBPATH=%LIBPATH%;%DB2PATH%\lib

set path=%PATH%;c:\Qt\5.12.0\msvc2017_64\
set path=%PATH%;c:\Qt\5.12.0\msvc2017_64\bin\
SET QTDIR=c:\Qt\5.12.0

rem Path to static build of QT
rem *** EDIT THIS IF NECESSARY ***
rem SET QTDIR=e:\qt\4.7.1.static
rem SET QTDIR=c:\Qt\Qt5.5.0\

rem QT needs to know which compiler is used
rem *** EDIT THIS IF NECESSARY***
SET QMAKESPEC=win32-msvc

rem *** No need to edit this ***
set path=%PATH%;%QTDIR%\;%QTDIR%\bin;
set path=%PATH%;%QTDIR%\qt\bin;
set include=%INCLUDE%;%QTDIR%\include\QtGui\;
set include=%INCLUDE%;%QTDIR%\include\QtCore\;
set include=%INCLUDE%;%QTDIR%\include\Qt\;
set include=%INCLUDE%;%QTDIR%\include\;
set include=%INCLUDE%;%QTDIR%\;
