/*
 * logger.c
 *
 *  Created on: Dec 22, 2013
 *      Author: haliax
 */

#include <stdio.h>
#include <string.h>
#include "ADE7816.h"
#include "logger.h"
#include "sqlite3.h"

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
  int i;
  for(i=0; i<argc; i++){
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

int LOGGER_init (sqlite3 **db) {
    sqlite3 *db_open;
    int rc;
    char *sql;
    char *zErrMsg = 0;

	// open db
    rc = sqlite3_open(LOGGER_DB_PATH, &db_open);
    if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return(-1);
    }

    // create table
    /* Create SQL statement */
       sql = "CREATE TABLE IF NOT EXISTS log(" \
             "ID             INT PRIMARY KEY     NOT NULL," \
             "timestamp      TEXT                NOT NULL," \
             "measurement    TEXT                NOT NULL," \
             "value          REAL );";

       /* Execute SQL statement */
       rc = sqlite3_exec(db_open, sql, callback, 0, &zErrMsg);
       if( rc != SQLITE_OK ) {
    	   fprintf(stderr, "SQL error: %s\n", zErrMsg);
    	   sqlite3_free(zErrMsg);
    	   return (-2);
       } else {
          fprintf(stdout, "Table created successfully\n");
       }

    *db = db_open;
	return 0;
}

int LOGGER_start (sqlite3 *db) {
    char *zErrMsg = 0;
    int rc;
    char zSql = "";  // need to create the sql statement with binding capability
    int nByte = sizeof(zSql);
    sqlite3_stmt *sql_stmt_handle;
    char *pzTail = NULL;

    // prepare sql statement with for repeated use with bindings.
    rc = sqlite3_prepare_v2(db, zSql, nByte, &sql_stmt_handle, &pzTail);
    if( rc ){
      fprintf(stderr, "Can't prepare statement: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return(-2);
    }

    // setup loop and periodic timer os call (need a way to exit the loop as well to stop the logger!)

    // read the energy registers
    ADE7816_readEnergyRegisters(); // need to adjust this function to return the register readings instead of printing them

    // bind measurements and values

    // insert measurements

    // reset the sql statement

    // end loop

	return 0;
}

SQLITE_API LOGGER_close (sqlite3 *db) {
	return sqlite3_close(db);
}


