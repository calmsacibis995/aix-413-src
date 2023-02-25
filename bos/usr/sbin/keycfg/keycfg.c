static char sccsid[] = "@(#)69 1.3 src/bos/usr/sbin/keycfg/keycfg.c, dsaukey, bos41B, 412_41B_sync 12/21/94 06:49:59";
/*
 * COMPONENT_NAME: DSAUKEY
 *
 * FUNCTIONS: keycfg command
 *
 * ORIGINS: 83
 *
 * LEVEL 1,5 Years Bull Confidential Information
 */


#include <stdio.h>
#include <errno.h>
#include <locale.h>
#include <fcntl.h>
#include <sys/mdio.h>
#include <sys/iocc.h>
#include <sys/cfgodm.h>
#include <sys/systemcfg.h>

#include "keycfg_msg.h"

#define MSGSTR(Num,Str) catgets(catd,MS_KEYCFG,Num,Str)

#define MODE_NORMAL		"normal"
#define MODE_SECURE		"secure"
#define MODE_SERVICE            "service"


/* defines for the electronic key's position */
#define EKEY_POS_NORMAL 	0xff
#define EKEY_POS_SERVICE 	0xee
#define EKEY_POS_SECURE 	0xdd

/* nvram addresses */
#define PKEY_ADDR	0x20080
#define EKEY_ADDR	0x20084

#define TIMEOUT		1

/* field width for printing out */
#define FIELDLEN	25

/* define for  Electronic Mode Switch from Service Line retrieval */
#define DIAG_PASSOFFSET 12
#define DIAG_PASSLEN    16
#define DIAG_FLAGS_SIZE (DIAG_PASSOFFSET + DIAG_PASSLEN)
#define DIAG_FLAGS		2 /* BUMP info type */
#define EMSfromServiceLine	6
#define EMS_ENABLED  1
#define EMS_DISABLED 0

static MACH_DD_IO param;
static int dflag, cflag;
static int expectedMode;
static char *Curtty; 
static char tty[50];
static struct CuDv stCuDv;

nl_catd catd;

static void parse_options();
static void errusage();
static void handle_error();
static char *status();
static void getBumpFlag ();
static char *getTTYons2();

main(int argc, char **argv)
{
	int ret, nvram;
	int ModeSwitch = 0;
	int EModeSwitch = 0, KeyModeSwitch = 0;
	int EMSflag;

	/* initialisations */

	(void) setlocale (LC_ALL,"");
	catd = catopen(MF_KEYCFG, NL_CAT_LOCALE);

	/* test if we are on a Pegasus machine */

#ifdef __rs6k_smp_mca
	if (!__rs6k_smp_mca()) {
#else
	if (!__power_pc()) {
#endif
	fprintf(stderr, MSGSTR(NOTSUP,
		"1104-001: Command not supported\n"));
	exit(1);
        }

	parse_options(argc, argv);

	/* open nvram to enable ioctls usage */
	nvram = open("/dev/nvram", O_RDWR);
	if (nvram == -1) {
		 fprintf(stderr, MSGSTR(NV, "1104-002: Cannot open /dev/nvram\n"));
	 exit(1);
	}

	/* init machine device driver structure */

	param.md_size = 1;
	param.md_incr = MV_WORD;
	param.md_length = 0;

	if (dflag) {
		fprintf(stdout, "%-*s%-*s%-*s\n\n", FIELDLEN, MSGSTR(HEAD_MS, "Mode Switch"), FIELDLEN,  MSGSTR(HEAD_KMS, "Key Mode Switch"), FIELDLEN, MSGSTR(HEAD_EMS,"Electronic Mode Switch"));
	
		/* get the mode switch and display it */
		param.md_data = (char *)&ModeSwitch;
		param.md_addr = 0;
		ret = ioctl(nvram, MIOGETKEY, &param);
		if (ret == -1)
			handle_error(nvram);
		else
		{
			/* display mode switch */
			fprintf(stdout, "%-*s",FIELDLEN, status(ModeSwitch));
		}

		/* get the Key Mode Switch and display it */
		param.md_data = (char *)&KeyModeSwitch;
		param.md_addr = PKEY_ADDR;

		ret = ioctl(nvram, MIONVGET, &param);
		if (ret == -1)
			handle_error(nvram);
		else
			/* display key mode switch */
			fprintf(stdout, "%-*s", FIELDLEN, status(KeyModeSwitch));
		/* get the Electronic Mode Switch and display it */
		/* if Key Mode Switch is not normal, display ignored message */

		param.md_data = (char *)&EModeSwitch;
		param.md_addr = EKEY_ADDR;
		ret = ioctl(nvram, MIONVGET, &param);
		if (ret == -1)
			handle_error(nvram);

		if ((KeyModeSwitch & KEY_POS_MASK) == KEY_POS_NORMAL)

			/* display electronic mode switch */
			fprintf(stdout, "%s\n", status(EModeSwitch));
		else 
			/* display electronic mode switch and ignored message */
			fprintf(stdout, "%s (%s)\n", status(EModeSwitch), MSGSTR(NONE, "ignored"));

	}

	if (cflag) {
		/* check if electronic mode switch from service line flag 
		   is enabled or disabled */
		getBumpFlag(nvram, &EMSflag);
		if (EMSflag == EMS_DISABLED) {
		   /* check if keycfg command is executed from remote service line */
		   Curtty = (char *)getuinfo("TTY");
		   strcpy(tty,"/dev/");
		   strcat(tty,getTTYons2());
		   if (strcmp(tty, Curtty) == 0) {
		      fprintf(stderr, MSGSTR(AUTHORISED, 
				"1104-005: To modify Electronic Mode Switch,\nElectronic Mode Switch from Service Line must be enabled\n"));
		      (void)close(nvram);
		      exit(1);
	              }
		}
		/* check Key Mode Switch */
		/* electronic mode switch can only be set
		   if key mode switch is normal */
		param.md_data = (char *)&KeyModeSwitch;
		param.md_addr = PKEY_ADDR;

		ret = ioctl(nvram, MIONVGET, &param);
		if (ret == -1)
			handle_error(nvram);
		else
			if ((KeyModeSwitch & KEY_POS_MASK) != KEY_POS_NORMAL) {
				fprintf(stderr, MSGSTR(EXPECTED, 
				"1104-004: To modify Electronic Mode Switch, Key Mode Switch must be normal\n"));
				(void)close(nvram);
				exit(1);
			}
			else {
				/* set the electronic mode switch to the specified value */
				EModeSwitch = expectedMode;
				param.md_data = (char *)&EModeSwitch;
				param.md_addr = 0;
				ret = ioctl(nvram, MIOSETKEY, &param);
				if (ret == -1)
					handle_error(nvram);

				/* timeout for updating key register */
				sleep(TIMEOUT);

				/* check if specified electronic mode switch is really set */
				param.md_data = (char *)&EModeSwitch;
				param.md_addr = EKEY_ADDR;
				ret = ioctl(nvram, MIONVGET, &param);
				if (ret == -1)
					handle_error(nvram);
				else
					if ((EModeSwitch & KEY_POS_MASK) == (expectedMode & KEY_POS_MASK)) {
						fprintf(stdout,
						 catgets(catd,MS_KEYCFG, SUCCESS, "Electronic Mode Switch set to %s\n"),
						 status(EModeSwitch));	
					}
					else {
						fprintf(stderr, MSGSTR(FAIL, 
						"1104-003: Command failed\n"));
						(void)close(nvram);
						exit(1);
					}
			}
	} /* end if cflag */
	
	(void)close(nvram);

}

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
	char	    *getstr = "dc:";
	int c;

	dflag = 0;
	cflag = 0;

	while ((c = getopt(argc, argv, getstr))!= EOF)
	{
		switch (c)
		{
                case 'd':
                        if (dflag + cflag)
                                errusage();
                        else
                                dflag = 1;
                        break;
                case 'c':
                        if (dflag + cflag)
                                errusage();
                        else
			{
                        	cflag = 1;
				if (!strcoll( optarg, MODE_NORMAL ))
					expectedMode =  EKEY_POS_NORMAL;
				else if (!strcoll( optarg, MODE_SERVICE))
					expectedMode = EKEY_POS_SERVICE;
				else if (!strcoll( optarg, MODE_SECURE))
					expectedMode = EKEY_POS_SECURE;
				else
					errusage();
			}
                        break;
	        case '?':
                        errusage();
			break;
                } /* case */
        } /* while */

        /* check the flags consistency */
        if ((dflag + cflag) != 1)
                errusage();

	argc -= optind;
	argv += optind;

	/* check the arguments */
	if (dflag && (argc != 0))
		 errusage();

	if (cflag && (argc != 0))
		 errusage();
}

/*
 * NAME: errusage
 *
 * FUNCTION: print usage error and exit
 *
 * RETURNS : exit 1
 */

void errusage()
{
        char *usage =
"Usage: keycfg {-d | -c EModeSwitch}\nwith EModeSwitch = normal or service or secure\n";
        fprintf(stderr, MSGSTR(USAGE, usage));
        exit(1);
}

/*
 * NAME: handle_error
 *
 * FUNCTION: display error message and exits
 *
 * RETURNS : exits 1
 *
 */

void handle_error(int nvram)
{
	(void)close(nvram);
	switch (errno) {
	case EINVAL :
		fprintf(stderr, MSGSTR(NOTSUP,
			"1104-001: Command not supported\n"));
		break;
	default :
		fprintf(stderr, MSGSTR(FAIL, "1104-003: Command failed\n"));
		break;
	}
	exit(1);
}

/*
 * NAME: status
 *
 * FUNCTION: decode the status of the Mode Switch.
 *
 * RETURNS : status of Mode Switch.
 */

char *status(int Mode)
{
	switch (Mode & KEY_POS_MASK)
	{
	case KEY_POS_NORMAL:
		return(MSGSTR(NORMAL, "normal"));
		break;
	case KEY_POS_SERVICE:
		return(MSGSTR(SERVICE, "service"));
		break;
	case KEY_POS_SECURE:
		return(MSGSTR(SECURE, "secure"));
		break;
	default:
		return(MSGSTR(NONE, "ignored"));
		break;

	}
}

void getBumpFlag (int nvram, int *EMS)
{
/* use mpcfg code to get diagnostic flags */
	int ret;
	MACH_DD_IO param_mdd;
	ulong data_length;
	unsigned char buf_diag[DIAG_FLAGS_SIZE];

	/* get diagnostic flags */
	param_mdd.md_incr = MV_BYTE; /* user buffer size in bytes */
	param_mdd.md_length = &data_length;
       	param_mdd.md_data = (char *)&buf_diag[0];
       	param_mdd.md_type = DIAG_FLAGS;
       	param_mdd.md_size = DIAG_FLAGS_SIZE;
       	ret = ioctl(nvram, MDINFOGET, &param_mdd);
        if ((ret == -1) || (data_length != DIAG_FLAGS_SIZE))
		handle_error(nvram);
	else {
		*EMS = buf_diag[EMSfromServiceLine-1] ?
			EMS_ENABLED : EMS_DISABLED;
	}
}

char *getTTYons2()
{
	int rc;

	odm_initialize();
	rc = (int) odm_get_first(CuDv_CLASS, "connwhere=s2", &stCuDv);
	odm_terminate ();
	if (rc == 1) {
		return("");
	}
	return(stCuDv.name);
}
