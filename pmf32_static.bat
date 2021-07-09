call c:"\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars32.bat"

set DB2PATH=c:\Program Files\IBM\SQLLIB\
set LIBPATH=%LIBPATH%;%DB2PATH%\lib


rem set path=%PATH%;c:\Qt\5.12.0\msvc2017\
rem set path=%PATH%;c:\Qt\5.12.0\msvc2017\bin\

rem SET QTDIR=d:\qt-static\
rem SET QTDIR=d:\qt5.12.5-full_static\
SET QTDIR=d:\qt5.12.5-vc19-full_static\

SET MSVC_STATIC=1



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
set include=%INCLUDE%;%QTDIR%\include\QtWidgets;
set include=%INCLUDE%;%QTDIR%\;
