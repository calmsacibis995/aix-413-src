static char sccsid[] = "@(#)98  1.8  src/bos/usr/sbin/trace/trc_load.c, cmdtrace, bos41B, 412_41B_sync 12/6/94 02:13:43";
/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 * 
 * FUNCTIONS: trc_load trc_unload
 * 
 * ORIGINS: 83
 * 
 *
 *  LEVEL 1, 5 Years Bull Confidential Information
 */

/*  The programs trc_load and trc_unload are added for the sake of the load 
 *  dynamic of the driver trace. 
 *  trc_load() loads and configures the device driver
 *  trc_unload() unconfigures and unloads the device driver
 */



#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/sysconfig.h>
#include <sys/cfgodm.h>
#include <sys/sysmacros.h>
#include <sys/device.h>
#include <sys/trcctl.h>
#include <cf.h>
#include <sys/mode.h>
#include <stdio.h>
#include "trace.h"

#define driver "trcdd"
#define FICTRACE "systrace"
#define FICTRCTL "systrctl"


/* Fonction main() : call trc_load or trc_unload
 * Input of main   : chan = number of the channel
 *		     load_flag = 0  we call trc_unload to unload the driver
 *		                 1  we call trc_load with lock
 *		                 2  we call trc_load without taking lock
 * Output of main  : none
 *	
 */

extern odmerrno;
extern mid_t loadext();

static trc_load();
static trc_unload();

main(argc,argv)
int argc;
char *argv[];
{
int arg;
int load_flag;

	arg = atoi(argv[1]);
	load_flag = atoi(argv[2]);
	if (load_flag)
		trc_load(arg,load_flag);
	else 
		trc_unload(arg);
}

/*
 * Function trc_load() : load driver and handle signals
 * Inputs	       : chan = channel in use
 *        		 load_flag = 2 if this routine is called when trace 
 *				       runs with -g
 *                   		     1 else
 * Outputs	       : exit 0 or error value
 */

static trc_load (chan,load_flag)
int	chan;
int 	load_flag;
{
	long	majorno;
	struct cfg_dd cfg;           /* structure for command sysconfig  */
	char	*odm_msg;

	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);


	if (odm_initialize() < 0) {
		odm_err_msg(odmerrno, &odm_msg);
		fprintf(stderr,"trc_load : %s", odm_msg);
		exit (EXIT_ODMINIT);
	}
	if (odm_lock ("/etc/objrepos/config_lock", ODM_WAIT) == -1) {
		odm_err_msg(odmerrno, &odm_msg);
		fprintf(stderr,"trc_load : %s", odm_msg);
		odm_terminate();
		exit (EXIT_ODMLOCK);
	}


	if ((majorno = trc_ctlfile(chan)) == -1) {
		perror("trc_load");
		odm_terminate();
		exit (EXIT_MKSPECIAL);
	}

	odm_terminate();

	if (load_flag == 1) {
		lock_sema();
	}

	/* We load the driver in the kernel*/
	if ((cfg.kmid = loadext(driver, TRUE, FALSE)) == NULL) {
		perror ("trc_load : LOADEXT");
		if (load_flag == 1) {
                	unlock_sema();
        	}
		exit (EXIT_LOADEXT);
	}

	cfg.ddsptr = NULL;
	cfg.ddslen = 0;
	cfg.devno = makedev(majorno, (long)MDEV_SYSTRACE);
	cfg.cmd = CFG_INIT;

	/* Call trcconfig() driver routine  */
	if (sysconfig (SYS_CFGDD, &cfg, sizeof(struct cfg_dd )) == -1) {
		/* We unload the driver */
		loadext(driver, FALSE, FALSE);
		perror ("trc_load : SYSCONFIG");
		if (load_flag == 1) {
                	unlock_sema();
                }
		exit (EXIT_CFGINIT);
	}

	if (load_flag == 1) {
		unlock_sema();
	}
	exit (0);
}





/*
 * Function: this routine checks if the special files, associated with the
 * channel, exists, if not it creates them.
 * This routine also checks the existence of the files /dev/systrace and
 * /dev/systrctl.
 * Inputs: channel
 * Outputs:  majorno or  -1 if error
 */
trc_ctlfile(chan)
int	chan;
{
	long	majorno;
	char	devname[52];
	char	ctlname[52];
	long	minortrace[] = 
	{
		MDEV_SYSTRACE, MDEV_SYSTRACE1, MDEV_SYSTRACE2, MDEV_SYSTRACE3,
		MDEV_SYSTRACE4, MDEV_SYSTRACE5, MDEV_SYSTRACE6, MDEV_SYSTRACE7	};
	long	minortrctl[] = 
	{
		MDEV_SYSTRCTL, MDEV_SYSTRCTL1, MDEV_SYSTRCTL2, MDEV_SYSTRCTL3,
		MDEV_SYSTRCTL4, MDEV_SYSTRCTL5, MDEV_SYSTRCTL6, MDEV_SYSTRCTL7	};

	majorno = 5;

	if (trc_creat (FICTRACE, MDEV_SYSTRACE, majorno) == -1)
		return (-1);

	if (trc_creat (FICTRCTL, MDEV_SYSTRCTL, majorno) == -1)
		return (-1);

	if (chan == 0)
		return(majorno);

	sprintf(devname, "%s%d", FICTRACE, chan);
	sprintf(ctlname, "%s%d", FICTRCTL, chan);

	if (trc_creat(devname, minortrace[chan], majorno) == -1)
		return(-1);
	if (trc_creat(ctlname, minortrctl[chan], majorno) == -1)
		return(-1);

	return (majorno);
}



/*
 * Function: create special file with the given filename and minor and
 * major numbers
 * Inputs: filename, minor and major numbers
 * Outputs: 0 or -1 if error
 */
trc_creat(filename, minorno, majorno)
char	*filename;
long	minorno;
long	majorno;
{
	dev_t devno;
	int	filetype;
	char buf[20];

	filetype = S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

     /* If minor number exists, genminor() returns NULL, otherwise
        return NULL  */
	genminor(filename, majorno, minorno, 1, 1, 1);
	devno = makedev(majorno, minorno);
	if (mk_sp_file(devno, filename, filetype) == -1)
		return (-1);
	sprintf(buf,"/dev/%s",filename);
	if (chown(buf,0,0) == -1 )
	{
		perror("trc_creat");
		return (-1);
	}
	return(0);
}


/*
 * Function trc_unload() : unload the driver
 * Inputs		 : status of the caller
 * Outputs 		 : exit with the status
 */

static trc_unload (status)
int	status;
{
	struct cfg_dd cfg;           /* structure for command sysconfig  */
	long	majorno;              /* major of /dev/systrace  */
	char	*odm_msg;

	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);


	if (odm_initialize() < 0) {
		odm_err_msg(odmerrno, &odm_msg);
		fprintf(stderr,"trc_unload : %s", odm_msg);
		exit (EXIT_ODMINIT);
	}
	if (odm_lock ("/etc/objrepos/config_lock", ODM_WAIT) == -1) {
		odm_err_msg(odmerrno, &odm_msg);
		fprintf(stderr,"trc_unload : %s", odm_msg);
		odm_terminate();
		exit (EXIT_ODMLOCK);
	}


	majorno = 5;
	odm_terminate();

	lock_sema();
	/* Initialize the structue cfg */
	cfg.kmid = NULL;
	cfg.cmd = CFG_TERM;
	cfg.ddsptr = NULL;
	cfg.ddslen = 0;
	cfg.devno = makedev(majorno, (long)MDEV_SYSTRACE);

	errno = 0;
	/* Call trcconfig() driver routine */
	if (sysconfig (SYS_CFGDD, &cfg, sizeof(struct cfg_dd )) == -1) {
		/* While busy, go on trying again */
		while (errno == EBUSY) {
			sleep(1);
			errno = 0;
			sysconfig (SYS_CFGDD, &cfg, sizeof(struct cfg_dd ));
		}
		/* Otherwise, exit() */
		if (errno) {
			perror ("trc_unload : SYSCONFIG");
		unlock_sema();
			exit(EXIT_CFGINIT);
		}
	}

	/* We unload the driver */
	if (loadext(driver, FALSE, FALSE) == NULL) {
		perror ("trc_unload : LOADEXT");
		unlock_sema();
		exit(EXIT_LOADEXT);
	}

	unlock_sema();
	exit(status);
}

