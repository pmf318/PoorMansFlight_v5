<a href="https://github.com/pmf318/PoorMansFlight_v5/blob/main/pmf5setup_64bit.exe">pmf5setup_64bit.exe.exe</a>

THIS IS PRETTY MUCH OUTDATED.
CURRENT VERSION ON  leipelt.de

License:
PMF is released under the GPL. 
A copy of the GPL can be found in gpl.txt in this directory.

-------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------

These are very short instructions for building PMF.
A more detailed version is available at www.leipelt.net

PMF can access DB2 and SQL Server. You can build PMF with the 
plugins for DB2 or the plugin for SQL Server or all plugins.

Depending on what you decide to build, the requirements differ.


Part 1: Building PMF on Linux
Part 2: Building PMF on Windows

-------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------


1. Building PMF on Linux

Requirements:
- Qt4 or Qt5 development packages
- make (usually in automake)
- g++ compiler
- for DB2: "Application development tools" from db2setup
- a working database connection
- for SQL Server: FreeTDS, unixODBC (or unixODBC2) and UnixODBC[2]-dev

Unpack pmf.tar.gz into a suitable directory:
$ tar -xzf pmf.tar.gz
-------------------------------------------------------------------------------------
1.1 Buildung PMF with both DB2 plugins
- Install DB2 "Application development tools": in db2Setup, 
    choose "Custom install" and select "Application development tools"   
- You need at least one valid database connection, 
    for example the 'sample' database. Set this in "db=..."
- If the database is not local, you need to supply a username (uid) 
    and password (pwd)

In a shell:
$ qmake pmf5.pro "target=db2" "db=[databaseName]" 
- or -
$ qmake pmf5.pro "target=db2" "db=[databaseName]" "uid=[userName]" 
- or -
$ qmake pmf5.pro "target=db2" "db=[databaseName]" "uid=[userName]" "pwd=[password]"

This should generate a Makefile. Next, run
$ make

If you want to build only the ODBC plugin, run
$ qmake pmf5.pro "target=db2odbc" 
$ make

If you want to build only the plugin for the native client (CAE), run
$ qmake pmf5.pro "target=db2cae" "db==databaseName" "uid=userName" "pwd=password"
$ make

** Always run 'make distclean' or 'nmake distclean' before re-running qmake

*** Notes: 
If you supply only a uid without pwd, you will be asked for the pwd
during the build process. The pwd will *not* be stored.

If you supply uid and pwd, they will be stored in the Makefile
To work around this, you could:
- Use a local database (for example DB2's 'sample' database) to connect 
  without uid/pwd
- Set uid but not pwd: You will be asked for the pwd
  during the build process. In this case, the pwd will *not* be stored.
- Remove the Makefile after the build. To re-generate the Makfile, re-run qmake.


-------------------------------------------------------------------------------------
1.2 Buildung PMF with the SQL Server plugin
In a shell:
$ qmake pmf5.pro "target=sqlsrv"
$ make

*** Always run 'make distclean' or 'nmake distclean' before re-running qmake

-------------------------------------------------------------------------------------
1.3 Buildung PMF with both plugins (SQL Server and DB2)
The same procedure as in 1.1, simply replace 
"target=db2" with "target=all".

Run one of these commands:
$ qmake pmf5.pro "target=all" "db=databaseName" 
- or -
$ qmake pmf5.pro "target=all" "db=databaseName" "uid=userName" 
- or -
$ qmake pmf5.pro "target=all" "db=databaseName" "uid=userName" "pwd=password"

This should generate a Makefile. Next, run
$ make


For more info, see Notes in 1.1

*** Always run 'make distclean' or 'nmake distlean' before re-running qmake

-------------------------------------------------------------------------------------
You should now have the excutable (./pmf) built.
If you get stuck, drop me a mail and I'll try to help.

See www.leipelt.net for instructions on how to set up SQL Server 
to accept connections from Linux.



-------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------
2. Building PMF on Windows

Requirements:
- Compiler: MS VS2010 or higher or MinGW
- Qt5: Choose the version for your compiler
- for DB2: "Application development tools" from db2setup
- for SQL Server: "SQL Server development tools". Probably "Microsoft Data Access SDK" is sufficient, I haven't tried that.

I'm currently using MS Visual Studio 2010 Express Edition. It should be possible to use later versions, or MinGW, but I haven't tried it.
Note on  MinGW: Do *not* use MinGW when linking against DB2 v7 libs.

-------------------------------------------------------------------------------------
2.1 Buildung PMF with the DB2 plugin 
- You need at least one valid DB2 database connection
- Install DB2 "Application development tools": in db2Setup, choose "Custom install" and select "Application development tools"
- Probably a batch file to set environment variables for Qt, compiler and DB2 (see below)

Unpack pmf.zip into a suitable directory.
Open a DB2 CLP (Command Line Processor) window (via Start->run->"db2cmd") and navigate to the place where you unpacked PMF.
You probably need to set environment variables now:
Contained in pmf.zip is 'pmf_MSVC.bat', edit it to fit your environment.


When the environment variables are set, run one of these commands in the DB2 CLP:
  qmake pmf5.pro "target=all" "db=databaseName" 
- or -
  qmake pmf5.pro "target=all" "db=databaseName" "uid=userName" 
- or -
  qmake pmf5.pro "target=all" "db=databaseName" "uid=userName" "pwd=password"

where "userName" and "password" are valid for DB2.

For an explanation of "db", "uid", "pwd" see Notes in 1.1

Next, run 
  nmake or make for MinGW
  

 
-------------------------------------------------------------------------------------

2.2 Building PMF for both DB2 and SQL Server:

Same as above, simply replace "target=db2" with "target=all"

-------------------------------------------------------------------------------------

2.3 Building PMF with the SQL Server plugin

You may need a batch file to set environment variables for Qt, compiler and DB2 (see above)

Unpack pmf.zip into a suitable directory.
Open command window and navigate to the place where you unpacked PMF.
You probably need to set environment variables now. Contained in pmf.zip is 'pmf_MSVC.bat', edit it to fit your environment.

When the environment variables are set, run these commands in the command window:
  qmake pmf5.pro "target=dsqlsrv"
  nmake
(or 'make' for MinGW)
-------------------------------------------------------------------------------------  

  
