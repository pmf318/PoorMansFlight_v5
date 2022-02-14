###########################################################
#
# pro file for generating Makefile(s) to build PMF5 and plugins
#
# Basic usage: 'qmake pmf5.pro', see readme.txt in this directory.
# ('qmake' is part of Qt's development suite)
#
###########################################################


trg = $$target
dbName = $$db

message(" ")
message("*** Always run ")
message("*** 'make distclean' or 'nmake distclean' ")
message("*** before re-running qmake")
message("*** See readme.txt in this directory")
message(" ")
DB2=1
TEMPLATE = subdirs
SUBDIRS =  ./glengine

!contains(trg, "db2odbc"){
!contains(trg, "db2"){
!contains(trg, "db2odbc"){
!contains(trg, "db2cae"){
!contains(trg, "oracle"){
!contains(trg, "mariadb"){
!contains(trg, "postgres"){
    !contains(trg, "sqlsrv"){
	!contains(trg, "all"){

	    message("Missing target.")
                message("To build PMF for DB2, SQL Server and MariaDB, run ")
		message("  qmake pmf5.pro \"target=all\"" "\"db=<databaseName>\" \\")
		message("       [\"uid=userName\"] [\"pwd=password\"] ")
		message("  ")
		message("To build PMF for SQL Server, run ")
		message("  qmake pmf5.pro \"target=sqlsrv\"") 	    
		message("  ")
		message("To build PMF for DB2, run ")
                message("  qmake pmf5.pro \"target=db2\"" "\"db=<databaseName>\" \\")
				message("       [\"uid=userName\"] [\"pwd=password\"] ")
		message("  ")				
		message("To build PMF for DB2 for ODBC, run ")
		message("  qmake pmf5.pro \"target=db2odbc\"" )
		message("  ")
                message("To build PMF for DB2 for the native client only, run ")
		message("  qmake pmf5.pro \"target=db2cae\"" "\"db=<databaseName>\" \\")
		message("       [\"uid=userName\"] [\"pwd=password\"] ")
		message("  ")
                message("To build PMF for DB2 for ODBC only, run ")
                message("  qmake pmf5.pro \"target=db2odbc\"" "\"db=<databaseName>\" \\")
		message("       [\"uid=userName\"] [\"pwd=password\"] ")
                message("  ")
                message("To build PMF for MariaDB, run ")
                message("  qmake pmf5.pro \"target=mariadb\"" )
		message("  ")
	    message("Again, see readme.txt in this directory")
            message(" ")
	}
    }
}
}
}
}
}
}
}
contains(trg, "all"){
    count(dbName, 1) {
        message("--------------------------")
        message("OK: target is DB2 and SqlServer")
        SUBDIRS += ./glengine/sql/odbcdsql
        SUBDIRS += ./glengine/sql/db2dsql
        SUBDIRS += ./glengine/sql/db2dapi
        SUBDIRS += ./glengine/sql/db2dcli
	##### Experimental: ##########
        # SUBDIRS += ./glengine/sql/mariadb
        # SUBDIRS += ./glengine/sql/postgres
        message("-> run 'make' or 'nmake' to start building pmf")
    }
    count(dbName, 0) {
        message("Usage:   qmake pmf5.pro \"target=all\"" "\"db=[DB2 database]\"")            
        message("  ")
    }    
}
contains(trg, "db2odbc"){

                message("Target is DB2ODBC")
                SUBDIRS += ./glengine/sql/db2dcli
        message("run 'make' or 'nmake' to start building pmf")

}
contains(trg, "db2all"){
    count(dbName, 1) {
        message("--------------------------")
        message("OK: target is DB2 for both ODBC and Native Client")
        SUBDIRS += ./glengine/sql/db2dsql
        SUBDIRS += ./glengine/sql/db2dapi
        SUBDIRS += ./glengine/sql/db2dcli
        message("-> run 'make' or 'nmake' to start building pmf")
    }
    count(dbName, 0) {
        message("Usage:   qmake pmf5.pro \"target=db2\"" "\"db=[DB2 database]\"")            
        message("  ")
    }    
}

contains(trg, "db2cae"){
    count(dbName, 1) {
        message("OK: target is DB2 Native Client")
        SUBDIRS += ./glengine/sql/db2dsql
        SUBDIRS += ./glengine/sql/db2dapi
        message("-> run 'make' or 'nmake' to start building pmf")
    }
    count(dbName, 0) {
        message("Usage:   qmake pmf5.pro \"target=db2cae\"" "\"db=[DB2 database]\"")            
        message("  ")
    }    
}

contains(trg, "db2odbc"){
    message("--------------------------")
    message("OK: Target is DB2/ODBC")
    SUBDIRS += ./glengine/sql/db2dcli
    message("-> run 'make' or 'nmake' to start building pmf")
}

contains(trg, "sqlsrv"){
    message("--------------------------")
    message("OK: Target is SqlServer")
    SUBDIRS += ./glengine/sql/odbcdsql 
    message("-> run 'make' or 'nmake' to start building pmf")
}

contains(trg, "mariadb"){
    message("--------------------------")
    message("OK: Target is MariaDB")
    SUBDIRS += ./glengine/sql/mariadb
    message("-> run 'make' or 'nmake' to start building pmf")
}
#contains(trg, "oracle"){
#    message("OK: Target is Oracle")
#    SUBDIRS += ./glengine/sql/oradsql
#    message("-> run 'make' or 'nmake' to start building pmf")
#}

message("  ")
SUBDIRS += ./pmfsrc
CONFIG += release
#CONFIG += debug

MAKEFILE=Makefile
