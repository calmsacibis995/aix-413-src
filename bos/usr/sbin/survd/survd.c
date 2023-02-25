static char sccsid[] = "@(#)91	1.6  src/bos/usr/sbin/survd/survd.c, cmdmp, bos411, 9436C411a 9/8/94 04:55:19";
/*
 * COMPONENT_NAME: (CMDMP) 
 *
 * FUNCTIONS: main, parse_options, sighand
 *
 * ORIGINS: 83
 *
 * LEVEL 1,5 Years Bull Confidential Information
 */


#include <stdio.h>
#include <errno.h>
#include <locale.h>
#include <procinfo.h>
#include <sys/ioctl.h>
#include <sys/mdio.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include "survd_msg.h"

#define MSGSTR(Num,Str) catgets(catd,MS_SURVD,Num,Str)
nl_catd catd;

#define HALF 1/2

static void parse_options();
static void sighand();

static short rflag, hflag, dflag, error;
static int delay = 60;
static char *pidfile = "/etc/survd.pid";
static int nvram;
static MACH_DD_IO param;

/* 
 * NAME: main
 *
 * FUNCTION: 
 *
 * RETURNS : nothing.
 */

main(int argc, char **argv)
{
	int ret;
	static FILE *fp, *console_p;
	short i;
	pid_t daemon_pid, search_pid;
	struct procsinfo Proc, *P;

    (void) setlocale (LC_ALL,"");
    catd = catopen(MF_SURVD, NL_CAT_LOCALE);	

	signal(SIGTERM, sighand);

	parse_options(argc, argv);
	
	if (!rflag) {
		param.md_delay = delay;
		if (hflag)
			param.md_mode = (ulong)MHARD_RESET;
		else
			param.md_mode = (ulong)MSOFT_RESET;

		/* open nvram to enable ioctls usage */
		nvram = open("/dev/nvram", O_RDWR);
		if (nvram == -1) {
			fprintf(stderr, MSGSTR(NV, "0043-001: Cannot open /dev/nvram\n"));
			exit(1);
		}

		/* check if surveillance is not already active */
		fp = fopen(pidfile, "r");
		if (fp != NULL) {
			ret = fscanf(fp, "%d", &daemon_pid);
			(void) fclose(fp);
			if (ret != EOF) {
				/* check if pid found corresponds to an existing process */
				P = &Proc;
				search_pid = daemon_pid;
				if ((ret = getprocs(P, sizeof(struct procsinfo), NULL, 0, &search_pid, 1)) != -1) {
					if ((P->pi_pid == daemon_pid) && (P->pi_state != SNONE)) {
						fprintf(stderr, MSGSTR(EXISTS, 
						"0043-010: Process ID found in %s corresponds to an existing process.\nCommand aborted.\n")
						,pidfile);
						exit(1);
					}
				}
			}
		} 

		/* set surveillance on */
		ret = ioctl(nvram, MSURVSET, &param);
		if (ret == -1) {
			switch (errno) {
			case EINVAL :
				fprintf(stderr, MSGSTR(NOTSUP,
				"0043-002: Command not supported\n"));
				exit(1);
			default :
				fprintf(stderr, MSGSTR(FAIL, "0043-003: Command failed\n"));
				exit(1);
			}
		}
		
		/* fork to exit the command and launch the son as a daemon */
		
		if (fork())
			exit(0);
		
		/* reassign stdin, stdout, stderr */
		for (i = 0; i < 3; i++)
			(void) close(i);
		(void) open("/", 0);
		(void) dup2(0, 1);
		(void) dup2(0, 2);

		/* disassociate from tty */
		i = open("/dev/tty", O_RDWR);
		if (i >= 0) {
			(void) ioctl(i, (int) TIOCNOTTY, (char *)0);
			(void) close(i);
		}

		/* tuck my process id away */
		fp = fopen(pidfile, "w");
		if (fp != NULL) {
			fprintf(fp, "%d\n", getpid());
			(void) fclose(fp);
		}
		else {
			fprintf(stderr, MSGSTR(NO_FILE, "0043-004: Cannot open %s"), pidfile);
			exit(1);
		}

		/* infinite loop */
		for (;;) {
			ret = ioctl(nvram, MSURVSET, &param);
			if (ret == -1) {
				console_p = fopen("/dev/console", "w");
				if (console_p != NULL) {
					fprintf(console_p, MSGSTR(CONSOLE1, 
					"0043-011: Surveillance daemon does not communicate with the bump anymore.\nThe machine is going to reboot\n"));
					(void)close(console_p);
					/* the system will reboot */
					exit(1);
				}
			}
			sleep(delay*HALF);
		}
	}
	else { /* reset flag */
		fp = fopen(pidfile, "r");
		if (fp == NULL) {
			fprintf(stderr, MSGSTR(NO_FILE, "0043-004: Cannot open %s\n"), pidfile);
			exit(1);
		}
		ret = fscanf(fp, "%d", &daemon_pid);
		if (ret == EOF) {
			fprintf(stderr, MSGSTR(READPID, "0043-009: Cannot read daemon pid in %s\n"), pidfile);
			exit(1);
		}
		(void) fclose(fp);
		ret = kill(daemon_pid, SIGTERM);
		if (ret == -1) {
			/* signal was not sent */
			switch (errno) {
			case ESRCH :
				fprintf(stderr, MSGSTR(PID, "0043-005: Pid found in %s is wrong\n"), pidfile);
				break;
			default :
				fprintf(stderr, MSGSTR(SIG, "0043-006: Signal was not sent to daemon\n"));
			}
			exit(1);
		}
		ret = unlink(pidfile);
		if (ret == -1) {
			fprintf(stderr, MSGSTR(UNLINK, "0043-007: Cannot remove %s\n"), pidfile);
			exit(1);
		}
		exit(0);
	}
}
						
void
sighand()
{
	int ret;
	int console_p;

	ret = ioctl(nvram, MSURVRESET, &param);
	if (ret == -1) { 
		/* cannot reset the surveillance */
		console_p = fopen("/dev/console", "w");
		if (console_p != NULL) {
			fprintf(console_p, MSGSTR(CONSOLE2, "0043-012: survd : cannot reset surveillance\n"));
			(void)close(console_p);
			/* the system will reboot */
			exit(1);
		}
	} 
	exit(0);
}
		

/* 
 * NAME: parse_options
 *
 * FUNCTION: read the command line.  
 *
 * RETURNS : nothing.
 */

void
parse_options(int argc, char **argv)
{
    extern char *optarg;
    extern int optind;
    char *getstr = "hrd:";
	char *usage = "Usage: survd [[-h] [-d Delay]] | [-r]\n";
	int c;

	hflag = FALSE;
	rflag = FALSE;
	dflag = FALSE;
	error = FALSE;
  	while ((c = getopt(argc, argv, getstr))!= EOF)
  	{
  		switch (c)
  		{
  			case 'h':
  				if ((hflag) || (rflag))
  					error = TRUE;
  				else
  					hflag = TRUE;
  				break;
  			case 'r':
  				if ((hflag) || (rflag) || (dflag))
  					error = TRUE;
  				else
  					rflag  = TRUE;
  				break;
  			case 'd':
  				if ((rflag) || (dflag))
  					error = TRUE;
  				else {
  					dflag = TRUE;
					if (isdigit(*optarg)) {
						delay = atoi(optarg);
						if (delay < 10) {
							fprintf(stderr, MSGSTR(DELAY, "0043-008: Delay must be greater than 10 sec\n"));
							exit(1);
						}
					}
					else
						error = TRUE;
				}
  				break;
  			case '?':
  				error = TRUE;
  		} /* case */

  		if (error) {
			fprintf(stderr, MSGSTR(USAGE, usage));
  			exit(1);
  		}
  	} /* while */

	argc -= optind;
	if (argc != 0) {
		fprintf(stderr, MSGSTR(USAGE, usage));
		exit(1);
	}
} /* parse_options */

