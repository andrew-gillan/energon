/*
 * energon.c
 *
 *  Created on: Feb 28, 2013
 *      Author: haliax
 */

/*********************************************************************
Energon command line source code:

    engergon-alpha-one [-r REGISTER] [-w REGISTER] [--run] [--stop] [--help]
       [--version] <file>

**********************************************************************/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <argtable2.h>
#include "ADE7816.h"
#include "logger.h"

int mymain(int run, int stop, int log, int stopLog, int readRmsReg, int readEnergyReg, int readReg, int writeReg, int writeVal,
        const char *filename, int filecount) {

	if (run>0) {
    	if (ADE7816_runDSP() == 0) {printf("dsp running\n");}
    	else {printf("dsp run failed\n");}
    }

    if (stop>0) {
    	if (ADE7816_stopDSP() == 0) {printf("dsp stopped\n");}
    	else {printf("dsp stop failed\n");}
    }

    if (readReg>0) {
    	ADE7816_readRegister(readReg);
    }

    if (readRmsReg>0) {
    	ADE7816_readRmsRegisters();
    }

    if (readEnergyReg>0) {
    	ADE7816_readEnergyRegisters();
    }

    if (writeReg>0) {
    	ADE7816_writeRegister(writeReg, writeVal);
    	ADE7816_readRegister(writeReg);
    }

    if (filecount == 1) {
    	printf("%s\n",filename);
    	if (strcmp(filename, "test.txt") == 0) {
    		ADE7816_writeMultipleRegisters(filename);
    	}
    	else {
    		printf("File not recognized\n");
    	}
    }

    // this is the main logging routine entry point
    if (log>0) {
		pid_t childPid;

		switch (childPid = fork()) {
		case -1: // fork failed
			printf("Logging process failed to start!\n");
			return EXIT_FAILURE;
		case 0:  // Child of successful fork() comes here
			printf("\nstarting log module...\n");
			execl("energon-logger", "energon-logger", (char *)NULL);
			printf("log module not started!\n");
			_exit(EXIT_FAILURE);
		default: // Parent comes here after successful fork()
			printf("Log module PID=%d\n", childPid);
			break;
   		}
    }

    if (stopLog>0) {
    	printf("does nothing yet\n");
    }

    return EXIT_SUCCESS;
}


int main(int argc, char **argv)
    {
	// Actually I'm going to need the cal procedure and create bulk reads so I can capture the data I need all at once
	// Then the other action items will be handy validate the calibration results.
	struct arg_int  *readReg       = arg_int0("rR","read",NULL, "read register");
    struct arg_int  *writeReg      = arg_int0("wW","write",NULL,"write register");
    struct arg_int  *writeVal      = arg_int0("vV","value",NULL,"write register value");
	struct arg_lit  *run           = arg_lit0(NULL,"run",       "run dsp");
	struct arg_lit  *stop          = arg_lit0(NULL,"stop",      "stop dsp");
	struct arg_lit  *log           = arg_lit0(NULL,"log",       "start logging process");
	struct arg_lit  *stopLog       = arg_lit0(NULL,"stoplog",   "stop logging process");
	struct arg_lit  *readRmsReg    = arg_lit0(NULL,"rms",       "read rms registers");
	struct arg_lit  *readEnergyReg = arg_lit0(NULL,"energy",    "read energy registers");
	struct arg_file *infiles       = arg_file0(NULL,NULL,NULL,  "input file(s)");
    struct arg_lit  *help          = arg_lit0(NULL,"help",      "print this help and exit");
    struct arg_lit  *version       = arg_lit0(NULL,"version",   "print version information and exit");
    struct arg_end  *end           = arg_end(20);
    void* argtable[] = {readReg,writeReg,writeVal,run,stop,log,stopLog,readRmsReg,readEnergyReg,infiles,help,version,end};
    const char* progname = "energon-alpha-one";
    int nerrors;
    int exitcode=0;

    /* verify the argtable[] entries were allocated sucessfully */
    if (arg_nullcheck(argtable) != 0)
        {
        /* NULL entries were detected, some allocations must have failed */
        printf("%s: insufficient memory\n",progname);
        exitcode=1;
        goto exit;
        }

    /* set any command line default values prior to parsing */
    readReg->ival[0]=0;
    writeReg->ival[0]=0;
    writeVal->ival[0]=0;

    /* Parse the command line as defined by argtable[] */
    nerrors = arg_parse(argc,argv,argtable);

    /* special case: '--help' takes precedence over error reporting */
    if (help->count > 0)
        {
        printf("Usage: %s", progname);
        arg_print_syntax(stdout,argtable,"\n");
        arg_print_glossary(stdout,argtable,"  %-25s %s\n");
        exitcode=0;
        goto exit;
        }

    /* special case: '--write' without '--value' takes precedence error reporting */
    if (writeReg->count != writeVal->count)
        {
    	printf("Must provide register (-w) and value (-v) to be written\n");
        exitcode=0;
        goto exit;
        }

    /* special case: '--version' takes precedence error reporting */
    if (version->count > 0)
        {
    	printf("Energon A1\n");
        exitcode=0;
        goto exit;
        }

    /* If the parser returned any errors then display them and exit */
    if (nerrors > 0)
        {
        /* Display the error details contained in the arg_end struct.*/
        arg_print_errors(stdout,end,progname);
        printf("Try '%s --help' for more information.\n",progname);
        exitcode=1;
        goto exit;
        }

    /* special case: uname with no command line options induces brief help */
    if (argc==1)
        {
        printf("Try '%s --help' for more information.\n",progname);
        exitcode=0;
        goto exit;
        }

    /* normal case: take the command line options at face value */
    exitcode = mymain(run->count, stop->count, log->count, stopLog->count, readRmsReg->count, readEnergyReg->count, readReg->ival[0], writeReg->ival[0], writeVal->ival[0],
                      infiles->filename[0], infiles->count);

    exit:
    /* deallocate each non-null entry in argtable[] */
    arg_freetable(argtable,sizeof(argtable)/sizeof(argtable[0]));

    return exitcode;
    }
