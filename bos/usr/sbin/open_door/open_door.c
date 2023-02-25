static char sccsid[] = "@(#)97	1.1  src/bos/usr/sbin/open_door/open_door.c, cmdcfg, bos412, 9445C412b 11/2/94 03:25:01";
/*
 * COMPONENT_NAME: (CMDCFG) 
 *
 * FUNCTIONS: main, parse_options
 *
 * ORIGINS: 83
 *
 * LEVEL 1,5 Years Bull Confidential Information
 */


#include <stdio.h>
#include <sys/mdio.h>
#include <locale.h>
#include <nl_types.h>
#include <errno.h>
#include <fcntl.h>

#include "open_door_msg.h"
#define MSGSTR(Num,Str) catgets(catd,MS_RDS,Num,Str)
#define OPEN_DOOR 0x80
nl_catd catd;

static short cflag, error;
static int cabinet;

static void
parse_options(int argc, char **argv);

/* 
 * NAME: main
 *
 * FUNCTION: open the door
 *           uses the MRDSSET ioctl.
 *
 * RETURNS : exit code 0 or 1.
 */

main(int argc, char **argv)
{
    int     ret, nvram, status;
    MACH_DD_IO param;
    short   cnt;
    short   failed = FALSE;

    (void) setlocale(LC_ALL, "");
    catd = catopen(MF_OPEN_DOOR, NL_CAT_LOCALE);

    parse_options(argc, argv);

    status = MRDS_ON;

    param.md_size = 1;
    param.md_incr = MV_WORD;
    param.md_cbnum = cabinet | OPEN_DOOR;  /* to open door command */
    param.md_dknum = 0;    /* not required in this case */
    param.md_data = (char *) &status;

    /* open nvram to enable ioctls usage */
    nvram = open("/dev/nvram", O_RDWR);
    if (nvram == -1) {
	fprintf(stderr, MSGSTR(NV, "0114-005: Cannot open /dev/nvram\n"));
	exit(1);
    }

    /* open the door */
    ret = ioctl(nvram, MRDSSET, &param);
    if (ret == -1) {
	switch (errno) {
	  case EIO:
	    fprintf(stderr, MSGSTR(IO, "0114-001: I/O error\n"));
	    break;
	  case EFAULT:
	    fprintf(stderr, MSGSTR(FAULT,
				   "0114-002: Cannot transfer data between kernel space and user memory\n"));
	    break;
	  case EINVAL:
	    fprintf(stderr, MSGSTR(NOTSUP,
				   "0114-003: Command not supported on this model\n"));
	    break;
	  default:
	    fprintf(stderr, MSGSTR(FAIL,
				   "0114-004: Command failed\n"));
	}
	exit(1);
    }
    else {
	fprintf(stdout, MSGSTR(DISPLAY,
			    "Door of the cabinet %d is open\n"), cabinet);
    }


    exit(0);
}  /* main */

/* 
 * NAME: parse_options
 *
 * FUNCTION: reads the command line
 *
 * RETURNS : nothing.
 */

void
parse_options(int argc, char **argv)
{
    extern char *optarg;
    extern int optind;
    char   *getstr = "c:";
    char   *usage = "Usage: open_door -c CabinetNumber\n";
    int     c;

    cflag = FALSE;
    error = FALSE;
    while ((c = getopt(argc, argv, getstr)) != EOF) {
	switch (c) {
	  case 'c':
	    if (cflag)
		error = TRUE;
	    else {
		cflag = TRUE;
		if (isdigit((int) *optarg)) {
		    cabinet = atoi(optarg);
		}
		else
		    error = TRUE;
	    }
	    break;
	  case '?':
	    error = TRUE;
	}	/* case */
	if (error) {
	    fprintf(stderr, MSGSTR(USAGE, usage));
	    exit(1);
	}
    }	/* while */

    if (!cflag) {
	fprintf(stderr, MSGSTR(USAGE, usage));
	exit(1);
    }

    argc -= optind;
    if (argc != 0) {
	fprintf(stderr, MSGSTR(USAGE, usage));
	exit(1);
    }
}  /* parse_options */
