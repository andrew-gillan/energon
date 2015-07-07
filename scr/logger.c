/*
 * logger.c
 *
 *  Created on: Dec 22, 2013
 *      Author: haliax
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include "ADE7816.h"
#include "logger.h"
#include "sqlite3.h"

static int insertEnergyMeasurementRecord(sqlite3_stmt *sql_stmt_handle, int logTime, const char *measurementName, const float measurementValue) {

	// bind measurements and values
	sqlite3_bind_int(sql_stmt_handle, 1, logTime);
	sqlite3_bind_text(sql_stmt_handle, 2, measurementName, -1, SQLITE_STATIC);
	sqlite3_bind_double(sql_stmt_handle, 3, measurementValue);

	// insert measurements
	if (sqlite3_step(sql_stmt_handle) != SQLITE_DONE) {
		printf ("Energy interval not logged!\n");
		return -1;
	}

    // reset the sql statement
	if (sqlite3_reset(sql_stmt_handle) != SQLITE_OK) {
		printf ("SQL Statement not reset!\n");
		return -2;
	}

	return 0;
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
  int i;
  for(i=0; i<argc; i++){
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

static int init_sqlite_db (sqlite3 **db) {
    sqlite3 *db_open;
    int rc;
    char *sql;
    char *zErrMsg = 0;

     printf("SQLite version : %s\n", sqlite3_libversion());

	// open db
    rc = sqlite3_open(LOGGER_DB_PATH, &db_open);
    if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db_open));
      sqlite3_close(db_open);
      return(-1);
    }

    // create table
    /* Create SQL statement */
       sql = "CREATE TABLE IF NOT EXISTS ENERGYLOG(" \
             "TIMESTAMP      INT                 NOT NULL," \
             "MEASUREMENT    TEXT                NOT NULL," \
             "VALUE          REAL );";

       /* Execute SQL statement */
       rc = sqlite3_exec(db_open, sql, callback, 0, &zErrMsg);
       if( rc != SQLITE_OK ) {
    	   fprintf(stderr, "SQL error: %s\n", zErrMsg);
    	   sqlite3_free(zErrMsg);
    	   return (-2);
       }

    *db = db_open;
	return 0;
}

static SQLITE_API int close_sqlite_db (sqlite3 *db) {
	return sqlite3_close(db);
}

static int logStart (sqlite3 *db) {
	//char *zErrMsg = 0;
    int rc;
    const char zSql[] = "INSERT INTO ENERGYLOG VALUES (?,?,?); ";

    int nByte = sizeof(zSql);
    sqlite3_stmt *sql_stmt_handle;
    const char *pzTail;

    int log = 1; // enable logging loop

    // prepare sql statement with for repeated use with bindings.
    rc = sqlite3_prepare_v2(db, zSql, nByte, &sql_stmt_handle, &pzTail);
    //sqlite3_prepare_v2()
    if( rc ){
      fprintf(stderr, "Can't prepare statement: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return(-2);
    }

    while (log) {
    	time_t t;
    	int sleepTime, logTime;
    	ADE7816_energyRegisterValues_t energyValue;

        // setup loop and periodic timer os call (need a way to exit the loop as well to stop the logger!)
    	t = time(NULL);
    	// printf("ctime() of time() value is: %s\n", ctime(&t));
    	sleepTime = 900 - (t % 900); // sleep until 15 minute boundary
    	logTime = t + sleepTime;
    	sleep(sleepTime);  // TODO: check return value?

        // read the energy registers
    	ADE7816_getEnergyRegisters(&energyValue);
//    	printf("awatthr : %f avarhr : %f\n", energyValue.awatthr, energyValue.avarhr);
//    	printf("bwatthr : %f bvarhr : %f\n", energyValue.bwatthr, energyValue.bvarhr);
//    	printf("cwatthr : %f cvarhr : %f\n", energyValue.cwatthr, energyValue.cvarhr);
//    	printf("dwatthr : %f dvarhr : %f\n", energyValue.dwatthr, energyValue.dvarhr);
//    	printf("ewatthr : %f evarhr : %f\n", energyValue.ewatthr, energyValue.evarhr);
//    	printf("fwatthr : %f fvarhr : %f\n", energyValue.fwatthr, energyValue.fvarhr);

    	insertEnergyMeasurementRecord(sql_stmt_handle, logTime, "AWATTHR", energyValue.awatthr);
    	insertEnergyMeasurementRecord(sql_stmt_handle, logTime, "AVARHR", energyValue.avarhr);
    	insertEnergyMeasurementRecord(sql_stmt_handle, logTime, "BWATTHR", energyValue.bwatthr);
    	insertEnergyMeasurementRecord(sql_stmt_handle, logTime, "BVARHR", energyValue.bvarhr);
    	insertEnergyMeasurementRecord(sql_stmt_handle, logTime, "CWATTHR", energyValue.cwatthr);
    	insertEnergyMeasurementRecord(sql_stmt_handle, logTime, "CVARHR", energyValue.cvarhr);
    	insertEnergyMeasurementRecord(sql_stmt_handle, logTime, "DWATTHR", energyValue.dwatthr);
    	insertEnergyMeasurementRecord(sql_stmt_handle, logTime, "DVARHR", energyValue.dvarhr);
    	insertEnergyMeasurementRecord(sql_stmt_handle, logTime, "EWATTHR", energyValue.ewatthr);
    	insertEnergyMeasurementRecord(sql_stmt_handle, logTime, "EVARHR", energyValue.evarhr);
    	insertEnergyMeasurementRecord(sql_stmt_handle, logTime, "FWATTHR", energyValue.fwatthr);
    	insertEnergyMeasurementRecord(sql_stmt_handle, logTime, "FVARHR", energyValue.fvarhr);

    	// TODO: command line to stop the loop/process
    }

	return 0;
}

int main(int argc, char **argv) {

	sqlite3 *db;

	if (ADE7816_init() == 0) {
		// TODO: add return value checking
		init_sqlite_db(&db);
		logStart(db);
		close_sqlite_db(db);

		// TODO: log an exit message
		printf("Exit Success\n");
		return EXIT_SUCCESS;
	}
	else {
		// TODO: log an exit message
		printf("Exit Failure\n");
		return EXIT_FAILURE;
	}
}
