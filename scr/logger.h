/*
 * logger.h
 *
 *  Created on: Dec 22, 2013
 *      Author: haliax
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#define LOGGER_DB_PATH "energon.sl3"

int LOGGER_init (sqlite3 **db);
int LOGGER_start (sqlite3 *db);
int LOGGER_close (sqlite3 *db);

#endif /* LOGGER_H_ */
