static char sccsid[] = "@(#)82	1.7  src/bos/usr/sbin/bindprocessor/bindprocessor.c, cmdmp, bos411, 9436C411a 9/8/94 04:54:42";
/*
 * COMPONENT_NAME: (CMDMP) 
 *
 * FUNCTIONS: main, parse_options
 *
 * ORIGINS: 83
 *
 * LEVEL 1,5 Years Bull Confidential Information
 */


#include <stdio.h>
#include <errno.h>
#include <locale.h>
#include <nl_types.h>
#include <sys/systemcfg.h>
#include <sys/processor.h>
#include <nlist.h>
#include <sys/time.h>
#include "bindprocessor_msg.h"

#define MSGSTR(Num,Str) catgets(catd,MS_BINDPROC,Num,Str)
nl_catd catd;

static pid_t processid;
static cpu_t cpu;
static short qflag, uflag;

static void parse_options(int argc, char **argv);


/* 
 * NAME: main
 *
 * FUNCTION: list the processors available for binding
 *           or bind/unbind a process to/from a processor
 *           using the bindprocessor system call.
 *
 * RETURNS : exit code 0 or 1.
 */

main(int argc, char **argv)
{
	int ret;

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_BINDPROCESSOR, NL_CAT_LOCALE);

	parse_options(argc, argv);

	/* query option : list the available processors */
	if (qflag) {
		int i;
		int run_cpu;

		fprintf(stdout, MSGSTR(AVAIL, "The available processors are: "));
		run_cpu = _system_configuration.ncpus;
		for (i=0; i< run_cpu; i++){
			fprintf(stdout, " %1d", i);
		}
		fprintf(stdout,"\n");
	}
	else {
		if (uflag)   /* unbind */
			cpu = PROCESSOR_CLASS_ANY;

		if ((ret = bindprocessor(BINDPROCESS, processid, cpu)) == -1) {
			switch (errno) {
			case EINVAL : 
				fprintf(stderr, MSGSTR(NOTAVAIL, 
				"1730-001: Processor %d is not available\n"), cpu);
				break;
			case ESRCH :
				fprintf(stderr, MSGSTR(NOPROC, 
				"1730-002: Process %d does not match an existing process\n"), processid);
				break;
			case EPERM :
				fprintf(stderr, MSGSTR(NOPERM, 
				"1730-003: Operation not permitted\n"));
				break;
			default :
				fprintf(stderr, MSGSTR(FAIL, "1730-004: Command failed\n"));
				break;
			}
			exit(1);
		}
	}
	exit(0);
} /* main */

/* 
 * NAME: parse_options
 *
 * FUNCTION: read the command line
 *
 * RETURNS : nothing.
 */

void parse_options(int argc, char **argv)
{
	extern char *optarg;
	extern int   optind;
	char        *getstr = "u:q";
	int c;
	short errflag;
	char *usage = "Usage: bindprocessor { -q | -u ProcessID | ProcessID [ProcessorNum] }\n";

	qflag = FALSE;
	uflag = FALSE;
	errflag = FALSE;
  	while ((c = getopt(argc, argv, getstr))!= EOF)
  	{
  		switch (c)
  		{
		case 'q':
			if (qflag || uflag)
				errflag = TRUE;
			else
				qflag = TRUE;
			break;
		case 'u':
			if (qflag || uflag)
				errflag = TRUE;
			else 
				uflag = TRUE;
			break;
  		case '?':
  			errflag = TRUE;
  		} /* case */
  		if (errflag) {
			fprintf(stderr, MSGSTR(USAGE, usage));
  			exit(1);
  		}
  	} /* while */

	argc -= optind;
	argv += optind;
	
	if ((qflag || uflag) && (argc != 0)) {
		fprintf(stderr, MSGSTR(USAGE, usage));
  		exit(1);
	}
	if (!qflag){
		if (uflag) {
			if (isdigit((int)*optarg))
				processid = (pid_t)atol(optarg);
			else 
				errflag++;
		}
		else { /* no flags */
			if (isdigit((int)*argv[0]))
				processid = (pid_t)atol(argv[0]);
			else 
				errflag++;
			if (argc == 1) {
				/* no processor was specified, choose one */
				struct timeval Tp;
				int ret;

				ret = gettimeofday(&Tp, NULL);
				if (ret == -1) {
					/* call failed - choose 0 as default cpu */
					cpu = 0;
				}
				else {
					cpu = (cpu_t)(Tp.tv_usec%(_system_configuration.ncpus));
				}
			}
			else {
				if (argc == 2) {
					/* a processor number was specified */
					if (isdigit((int)*argv[1]))
						cpu = (cpu_t)atoi(argv[1]);
					else {
						fprintf(stderr, MSGSTR(USAGE, usage));
						exit(1);
					}
				}
				else errflag++; /* too many parameters passed */
			}
		}
	}
	if (errflag) {
		fprintf(stderr, MSGSTR(USAGE, usage));
		exit(1);
	}
} /* parse_options */

