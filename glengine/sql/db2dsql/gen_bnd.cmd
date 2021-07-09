

if "%3" == "" goto :local
db2 connect to %1 user %3 using %4
goto :precompile

:local
db2 connect to %1 

:precompile
rem Do not edit this:
del PMF*.cpp
del PMF*.bnd
copy dsqlobj.sqc PMF%2.sqc
db2 prep PMF%2.sqc bindfile using PMF%2.bnd
copy PMF%2.c PMF%2.cpp
del PMF%2.c
copy PMF%2.bnd ..\..\..\plugins
