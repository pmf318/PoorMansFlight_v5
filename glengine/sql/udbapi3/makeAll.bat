echo Removing environment for DB2 v2 and setting for DB2 v7


set include=
set lib=

call c:\vcvars32.bat
call .\set7.bat

nmake -f makefile.qt clean
nmake -f makefile.qt
nmake -f makefile.qt install
