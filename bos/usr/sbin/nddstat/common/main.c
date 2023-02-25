static char sccsid[] = "@(#)53  1.2  src/bos/usr/sbin/nddstat/common/main.c, nddstat, bos411, 9428A410j 2/23/94 18:10:09";
/*
 * COMPONENT_NAME: NDDSTAT 
 *
 * FUNCTIONS: 
 *		main
 *		ndd_display
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <values.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ndd_var.h>
#include <sys/ndd.h>

#include <locale.h>
#include <nl_types.h>
#include "../common/nddstat_msg.h"




#define INT_IN_DEV_STATS	128	/* 128 int is reserved for mapping 
					   various device statistics table */
#define NDDS 	stable.ndd_stats
#define MSGSTR(n,s) catgets(catd,MS_NDDSTAT,n,s)

/*
 * Global table for containing stats from devices. The dev_stats table will 
 * be mapped by individual command to different media generic/specific stats.
 * Notice that the total table size is ndd_genstats + 128 words now.
 */
typedef struct {
	ndd_genstats_t	ndd_stats;
	int dev_stats[INT_IN_DEV_STATS];
} stable_t;

stable_t stable;		/* statistic table for all statistics */
char *devname = NULL;		/* device name */
nl_catd catd;			/* fd for cat message file */

extern char cmdname[];		/* command name defined by individual command
				   for message header   */


/*****************************************************************************/
/*
 * NAME:     main
 *
 * FUNCTION: The main user interface part of the <media>stat utility
 *	
 *	syntax:  <media>stat [-d -r] <device_name> 
 *
 *	default - Displays the generic driver statistics.
 *		  (i.e. ndd_genstats + media_genstats)
 *
 *      -d 	- displays all of the driver statistics, including the 
 *		  device specific statistics.  
 *		  (i.e. ndd_genstats + media_genstats + device_specstats)
 *
 *      -r	- Resets all of the statistics back to their 
 *		  initial values. Privileged users only.
 *		  (All statistics in ndd_genstats + media_genstats + 
 *		  device_specstats)
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      Shell
 *
 * INPUT:
 *      argc		- number of parameters
 *	argv		- array of parameters
 *
 * RETURNS:  
 *	0 - OK
 *	non-0 - error exit
 */
/*****************************************************************************/
main(argc, argv)
        int argc;
        char *argv[];
{
  extern char *optarg;
  extern int optind;
  int ch;
  int fd;
  struct sockaddr_ndd sa;
  int need_ext_stats = FALSE;
  int need_clear = FALSE;
  int rc;



  /* set up the message cat file */
  setlocale(LC_ALL, "");
  catd = catopen(MF_NDDSTAT, NL_CAT_LOCALE);

  while ((ch = getopt(argc, argv, "dr")) != EOF) {
	switch((char)ch) {
		case 'd':
			/* the device extended statistics is needed */
			need_ext_stats = TRUE;
                        break;

                case 'r': 
			/* clear the statistics after reading */
			/* only super user is allowed for this option */
			if (getuid()) {
				printf(MSGSTR(NO_PERMISSION,
				  "\n%s: 0909-001 Permission denied.\n"), 
				  cmdname);
				exit(1);
			}
			need_clear = TRUE;
                        break;

		case '?':
		default:
			printf(MSGSTR(USAGE, 
			  "\nusage: %s [-dr] <device name>\n"), cmdname);
			exit(1);
			break;
	}
  }
	
  argc -= optind;
  argv += optind;

  if (argc <= 0 || argc > 1) {
	printf(MSGSTR(USAGE, "\nusage: %s [-dr] <device name>\n"), cmdname);
	exit(1);
  }

  /* get the device name */
  devname = *argv;

  if ((fd = socket(AF_NDD, SOCK_DGRAM, 0)) < 0) {
    	printf(MSGSTR(SOCKET_FAILED, 
	  "\n%s: 0909-002 Unable to open device %s, errno = %d\n"), cmdname, 
	  devname, errno);
    	exit(1);
  }

  sa.sndd_family = AF_NDD;
  sa.sndd_len = sizeof(struct sockaddr_ndd);
  sa.sndd_filterlen = 0;
  bcopy(devname, sa.sndd_nddname, sizeof(sa.sndd_nddname));

  if (connect(fd, &sa, sizeof(sa))) {
    	printf(MSGSTR(CONNECT_FAILED, 
	  "\n%s: 0909-003 Unable to connect to device %s, errno = %d\n"), 
	  cmdname, devname, errno);
	close(fd);
    	exit(1);
  }

  /*
   * Call routines provided by individual command to get the statistics
   * and display them to the stdout.
   * If clear option is specified, issue the clear command to the device
   * driver after the statistics are displayed.
   */
  if (!(rc = dev_stats(fd, need_ext_stats))) {
	ndd_display(devname);
	dev_display(need_ext_stats);
	if (need_clear) {
  	  if (ioctl(fd, NDD_CLEAR_STATS, NULL)) {
    	    printf(MSGSTR(CLEAR_FAILED,
	    "\n%s: 0909-005 Unable to clear statistics on device %s, errno = %d\n"),
	      cmdname, devname, errno);
	  }
	}
  }
  else {
        printf(MSGSTR(GET_FAILED,
          "\n%s: 0909-004 Unable to get statistics on device %s, errno = %d\n"),
          cmdname, devname, errno);
	close(fd);
	exit(1);
  }

  close(fd);

}


/*****************************************************************************/
/*
 * NAME:     ndd_display
 *
 * FUNCTION: Format and write the statistics of the ndd_genstats_t to
 *	     the stdout.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      main
 *
 * INPUT:
 *      none
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
ndd_display() 

{
  int days, hrs, minutes, snds;
  double ttmp, rtmp;



  printf("-------------------------------------------------------------");
  /* 
   * Call routine provided by individual command to display its title
   */
  dev_title();
  
  printf(MSGSTR(ELAPSED_TIME, "Elapsed Time: "));
  snds = NDDS.ndd_elapsed_time % 60;
  minutes = NDDS.ndd_elapsed_time / 60;
  hrs = minutes / 60;
  minutes = minutes % 60;
  days = hrs / 24;
  hrs = hrs % 24;
  printf(MSGSTR(TIME, "%d days %d hours %d minutes %d seconds\n"), 
	days, hrs, minutes, snds);

  printf(MSGSTR(TX_RV_STATS, 
	"\nTransmit Statistics:                          Receive Statistics:\n"));
  printf("--------------------                          -------------------\n");

  ttmp = (double)MAXLONG * (double)NDDS.ndd_opackets_msw;
  ttmp += (double)NDDS.ndd_opackets_lsw;
  rtmp = (double)MAXLONG * (double)NDDS.ndd_ipackets_msw;
  rtmp += (double)NDDS.ndd_ipackets_lsw;
  printf(MSGSTR(PKTS, "Packets: %-36.0f Packets: %.0f\n"), ttmp, rtmp);

  ttmp = (double)MAXLONG * (double)NDDS.ndd_obytes_msw;
  ttmp += (double)NDDS.ndd_obytes_lsw;
  rtmp = (double)MAXLONG * (double)NDDS.ndd_ibytes_msw;
  rtmp += (double)NDDS.ndd_ibytes_lsw;
  printf(MSGSTR(BYTES, "Bytes: %-38.0f Bytes: %.0f\n"), ttmp, rtmp);

  ttmp = (double)MAXLONG * (double)NDDS.ndd_xmitintr_msw;
  ttmp += (double)NDDS.ndd_xmitintr_lsw;
  rtmp = (double)MAXLONG * (double)NDDS.ndd_recvintr_msw;
  rtmp += (double)NDDS.ndd_recvintr_lsw;
  printf(MSGSTR(INTRS, "Interrupts: %-33.0f Interrupts: %.0f\n"), 
	ttmp, rtmp);

  printf(MSGSTR(ERRORS, "Transmit Errors: %-28d Receive Errors: %d\n"), 
	NDDS.ndd_oerrors, NDDS.ndd_ierrors);

  printf(MSGSTR(DROPS, "Packets Dropped: %-28d Packets Dropped: %d\n"), 
	NDDS.ndd_opackets_drop, NDDS.ndd_ipackets_drop);
  printf(MSGSTR(MAX_TX_Q, 
    "Max Packets on S/W Transmit Queue: %-10d Bad Packets: %d\n"), 
    NDDS.ndd_xmitque_max, NDDS.ndd_ibadpackets);
  printf(MSGSTR(OFLOW, "S/W Transmit Queue Overflow: %d\n"), 
    NDDS.ndd_xmitque_ovf);
  printf(MSGSTR(TXQ_LEN, "Current S/W+H/W Transmit Queue Length: %d\n"), 
    NDDS.ndd_xmitque_cur);


}
