static char sccsid[] = "@(#)49	1.2  src/bos/usr/sbin/nddstat/fddistat/fddistat.c, nddstat, bos411, 9428A410j 4/28/94 11:01:52";
/*
 * COMPONENT_NAME: NDDSTAT 
 *
 * FUNCTIONS: 
 *		dev_stats
 *		dev_title
 *		dev_display
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1994
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
#include <sys/cdli_fddiuser.h>

#include <locale.h>
#include <nl_types.h>
#include <fddistat_msg.h>

/*
 * Define the statistic table for mapping all fddi statistics.
 * The type name has to be stable_t so we can share the global 
 * statistics table stable
 */

typedef struct {
        fddi_ndd_stats_t fddi_ndd;
} stable_t;

typedef struct {
        u_int   bufferlen;
        caddr_t bufferptr;
} io_block_t;

io_block_t io_block;		/* table for socket ioctl */

/*
 * The following three variables are required for every individual command
 * in order to share the common code
 */ 
char cmdname[8] = "fddistat";	/* command name defined for message header */
extern stable_t stable;         /* global statistic table */
extern char *devname;		/* device name */

nl_catd fddi_catd;            	/* fd for fddistat cat message file */


#define MSGSTR(n,s) catgets(fddi_catd,MS_FDDISTAT,n,s)

#define NDDS stable.fddi_ndd.genstats
#define FDDIS stable.fddi_ndd.fddistats

/*****************************************************************************/
/*
 * NAME:     dev_stats
 *
 * FUNCTION: Get statistics from the device driver. The device driver is
 *	     opened by the caller, the fd is passed into this routine.
 *	     Perform the correct ioctl to the device driver according
 *	     to the need_ext_stats flag.
 *	     This "dev_stats" routine should be in every individual command
 *	     in order to share the common code.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      main
 *
 * INPUT:
 *      fd		- file descriptor used to access the device driver
 *	need_ext_stats 	- flag for generic statistics or device specific
 *			  statistics
 *
 * RETURNS:  
 *	0 - OK
 *	non-0 - errno returned from device interface
 */
/*****************************************************************************/
dev_stats(fd, need_ext_stats)
int fd;
int need_ext_stats;

{
  /* set up the message cat file */
  setlocale(LC_ALL, "");
  fddi_catd = catopen(MF_FDDISTAT, NL_CAT_LOCALE);

  io_block.bufferlen = sizeof(fddi_ndd_stats_t);
  io_block.bufferptr = (caddr_t)&stable;

  if (ioctl(fd, NDD_GET_STATS, (caddr_t)&io_block)) {
	return(errno);
  }

  return(0);
}

/*****************************************************************************/
/*
 * NAME:     dev_title
 *
 * FUNCTION: Format and write the statistics output title to the stdout.
 *	     This "dev_title" routine should be in every individual command
 *	     in order to share the common code.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      ndd_display
 *
 * INPUT:
 *	none
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
dev_title()

{

  int i;


  printf(MSGSTR(DEVNAME, "\nFDDI STATISTICS (%s) :\n"), devname);
}

/*****************************************************************************/
/*
 * NAME:     dev_display
 *
 * FUNCTION: Format and write the statistics of the fddi_ndd_stats_t to
 *	     the stdout. If the flag need_ext_stats is on, also include 
 *	     the device specific statistics in the output.
 *	     This "dev_display" routine should be in every individual command
 *	     in order to share the common code.
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      main
 *
 * INPUT:
 *	need_ext_stats - flag for generic statistics or device specific
 *			 statistics
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
dev_display(need_ext_stats) 
int need_ext_stats;

{
  int i,j;
  int days, hrs, minutes, snds;
  int tmp1, tmp2;


  printf("\n");

  printf(MSGSTR(BCAST, "Broadcast Packets: %-26d Broadcast Packets: %d\n"), 
	FDDIS.bcast_tx_ok, FDDIS.bcast_rx_ok);

  printf(MSGSTR(MCAST, "Multicast Packets: %-26d Multicast Packets: %d\n"),
	FDDIS.mcast_tx_ok, FDDIS.mcast_rx_ok);

  /* 
   * General statistics. This part of the output format should be the
   * same for every command.
   */

  printf(MSGSTR(GENSTATS, "\nGeneral Statistics:\n"));
  printf("-------------------\n");

  printf(MSGSTR(NOMBUF,
	"No mbuf Errors: %-29d\n"), 
	NDDS.ndd_nobufs);

  tmp1 = (FDDIS.smt_error_hi << 16) + FDDIS.smt_error_lo;
  tmp2 = (FDDIS.smt_event_hi << 16) + FDDIS.smt_event_lo;
  printf(MSGSTR(SMT_ERR_EVT,
	"SMT Error Word: %08x%22 SMT Event Word: %08x\n"),
	tmp1, tmp2);

  printf(MSGSTR(CPV_PORT,
	"Connection Policy Violation: %04x%13 Port Event: %04x\n"),
	FDDIS.cpv, FDDIS.port_event);

  printf(MSGSTR(SETCOUNT,
	"Set Count Hi: %04x%28 Set Count Lo: %04x\n"),
	FDDIS.setcount_hi, FDDIS.setcount_lo);

  printf(MSGSTR(ACI_PFRAME,
	"Adapter Check Code: %04x%22 Purged Frames: %d\n"),
	FDDIS.aci_code, FDDIS.pframe_cnt);

  printf("\n");

  printf(MSGSTR(ECM,
	"ECM State Machine:        "));

  switch(FDDIS.ecm_sm)
  {
	case CFDDI_ECM_OUT:
		printf("OUT\n");
		break;
	case CFDDI_ECM_IN:
		printf("IN\n");
		break;
	case CFDDI_ECM_TRC:
		printf("TRACE\n");
		break;
	case CFDDI_ECM_LEAVE:
		printf("LEAVE\n");
		break;
	case CFDDI_ECM_PATH_TEST:
		printf("PATH_TEST\n");
		break;
	case CFDDI_ECM_INSERT:
		printf("INSERT\n");
		break;
	case CFDDI_ECM_CHK:
		printf("CHK\n");
		break;
	case CFDDI_ECM_DEINSERT:
		printf("DEINSERT\n");
		break;
  };

  printf(MSGSTR(PCMA,
	"PCM State Machine Port A: "));

  switch(FDDIS.pcm_a_sm)
  {
	case CFDDI_PCM_OFF:
		printf("OFF\n");
		break;
	case CFDDI_PCM_BRK:
		printf("BREAK\n");
		break;
	case CFDDI_PCM_TRC:
		printf("TRACE\n");
		break;
	case CFDDI_PCM_CON:
		printf("CONNECT\n");
		break;
	case CFDDI_PCM_NXT:
		printf("NEXT\n");
		break;
	case CFDDI_PCM_SIG:
		printf("SIGNAL\n");
		break;
	case CFDDI_PCM_JOIN:
		printf("JOIN\n");
		break;
	case CFDDI_PCM_VRFY:
		printf("VERIFY\n");
		break;
	case CFDDI_PCM_ACT:
		printf("ACTIVE\n");
		break;
	case CFDDI_PCM_MAINT:
		printf("MAINT\n");
		break;
  };

  printf(MSGSTR(PCMB,
	"PCM State Machine Port B: "));

  switch(FDDIS.pcm_b_sm)
  {
	case CFDDI_PCM_OFF:
		printf("OFF\n");
		break;
	case CFDDI_PCM_BRK:
		printf("BREAK\n");
		break;
	case CFDDI_PCM_TRC:
		printf("TRACE\n");
		break;
	case CFDDI_PCM_CON:
		printf("CONNECT\n");
		break;
	case CFDDI_PCM_NXT:
		printf("NEXT\n");
		break;
	case CFDDI_PCM_SIG:
		printf("SIGNAL\n");
		break;
	case CFDDI_PCM_JOIN:
		printf("JOIN\n");
		break;
	case CFDDI_PCM_VRFY:
		printf("VERIFY\n");
		break;
	case CFDDI_PCM_ACT:
		printf("ACTIVE\n");
		break;
	case CFDDI_PCM_MAINT:
		printf("MAINT\n");
		break;
  };

  printf(MSGSTR(CFMA,
	"CFM State Machine Port A: "));

  switch(FDDIS.cfm_a_sm)
  {
	case CFDDI_CFM_ISOLATED:
		printf("ISOLATED\n");
		break;
	case CFDDI_CFM_LOCAL:
		printf("LOCAL\n");
		break;
	case CFDDI_CFM_SEC:
		printf("SECONDARY\n");
		break;
	case CFDDI_CFM_PRIM:
		printf("PRIMARY\n");
		break;
	case CFDDI_CFM_CONCAT:
		printf("CONCATENATED\n");
		break;
	case CFDDI_CFM_THRU:
		printf("THRU\n");
		break;
  };

  printf(MSGSTR(CFMB,
	"CFM State Machine Port B: "));

  switch(FDDIS.cfm_b_sm)
  {
	case CFDDI_CFM_ISOLATED:
		printf("ISOLATED\n");
		break;
	case CFDDI_CFM_LOCAL:
		printf("LOCAL\n");
		break;
	case CFDDI_CFM_SEC:
		printf("SECONDARY\n");
		break;
	case CFDDI_CFM_PRIM:
		printf("PRIMARY\n");
		break;
	case CFDDI_CFM_CONCAT:
		printf("CONCATENATED\n");
		break;
	case CFDDI_CFM_THRU:
		printf("THRU\n");
		break;
  };

  printf(MSGSTR(CF,
	"CF State Machine:         "));

  switch(FDDIS.cf_sm)
  {
	case CFDDI_CF_ISOLATED:
		printf("ISOLATED\n");
		break;
	case CFDDI_CF_LOCAL_A:
		printf("LOCAL_A\n");
		break;
	case CFDDI_CF_LOCAL_B:
		printf("LOCAL_B\n");
		break;
	case CFDDI_CF_LOCAL_AB:
		printf("LOCAL_AB\n");
		break;
	case CFDDI_CF_LOCAL_S:
		printf("LOCAL_S\n");
		break;
	case CFDDI_CF_WRAP_A:
		printf("WRAP_A\n");
		break;
	case CFDDI_CF_WRAP_B:
		printf("WRAP_B\n");
		break;
	case CFDDI_CF_WRAP_AB:
		printf("WRAP_AB\n");
		break;
	case CFDDI_CF_WRAP_S:
		printf("WRAP_S\n");
		break;
	case CFDDI_CF_C_WRAP_A:
		printf("C_WRAP_A\n");
		break;
	case CFDDI_CF_C_WRAP_B:
		printf("C_WRAP_B\n");
		break;
	case CFDDI_CF_C_WRAP_S:
		printf("C_WRAP_S\n");
		break;
	case CFDDI_CF_THRU:
		printf("THRU\n");
		break;
  };

  printf(MSGSTR(MCFM,
	"MAC CFM State Machine:    "));

  switch(FDDIS.mac_cfm_sm)
  {
	case CFDDI_MCFM_ISOLATED:
		printf("ISOLATED\n");
		break;
	case CFDDI_MCFM_LOCAL:
		printf("LOCAL\n");
		break;
	case CFDDI_MCFM_SEC:
		printf("SECONDARY\n");
		break;
	case CFDDI_MCFM_PRIM:
		printf("PRIMARY\n");
		break;
  };

  printf(MSGSTR(RMT,
	"RMT State Machine:        "));

  switch(FDDIS.rmt_sm)
  {
	case CFDDI_RMT_ISOLATED:
		printf("ISOLATED\n");
		break;
	case CFDDI_RMT_NON_OP:
		printf("NON_OP\n");
		break;
	case CFDDI_RMT_RING_OP:
		printf("RING_OP\n");
		break;
	case CFDDI_RMT_DETECT:
		printf("DETECT\n");
		break;
	case CFDDI_RMT_NON_OP_DUP:
		printf("NON_OP_DUP\n");
		break;
	case CFDDI_RMT_RING_OP_DUP:
		printf("RING_OP_DUP\n");
		break;
	case CFDDI_RMT_DIRECTED:
		printf("DIRECTED\n");
		break;
	case CFDDI_RMT_TRC:
		printf("TRACE\n");
		break;
  };

  printf("\n");

  printf(MSGSTR(FLAGS, "Driver Flags: "));

  i = 0;
  if (FDDIS.ndd_flags & NDD_UP) {
  	printf("Up ");
	i++;
  }
  if (FDDIS.ndd_flags & NDD_BROADCAST) {
  	printf("Broadcast ");
	i++;
  }
  if (FDDIS.ndd_flags & NDD_DEBUG) {
  	printf("Debug ");
	i++;
  }

  if (FDDIS.ndd_flags & NDD_RUNNING) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
	}
  	printf("Running ");
	i++;
  }

  if (FDDIS.ndd_flags & NDD_SIMPLEX) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
	}
  	printf("Simplex ");
	i++;
  }

  if (FDDIS.ndd_flags & NDD_DEAD) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
	}
  	printf("Dead ");
	i++;
  }

  if (FDDIS.ndd_flags & NDD_LIMBO) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
	}
  	printf("Limbo ");
	i++;
  }

  if (FDDIS.ndd_flags & NDD_PROMISC) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
	}
	printf("Promiscuous ");
	i++;
  }

  if (FDDIS.ndd_flags & NDD_ALTADDRS) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
  	}
  	printf("AlternateAddress ");
	i++;
  }

  if (FDDIS.ndd_flags & NDD_MULTICAST) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
  	}
  	printf("AllMulticast ");
	i++;
  }

  if (FDDIS.ndd_flags & NDD_MULTICAST) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
  	}
  	printf("AllMulticast ");
	i++;
  }

  if (FDDIS.ndd_flags & CFDDI_NDD_LLC_DOWN) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
  	}
  	printf("LLCDown ");
	i++;
  }

  if (FDDIS.ndd_flags & CFDDI_NDD_BEACON) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
  	}
  	printf("BeaconReception ");
	i++;
  }

  if (FDDIS.ndd_flags & CFDDI_NDD_SMT) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
  	}
  	printf("SMTReception ");
	i++;
  }

  if (FDDIS.ndd_flags & CFDDI_NDD_NSA) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
  	}
  	printf("NSAReception ");
	i++;
  }

  if (FDDIS.ndd_flags & CFDDI_NDD_BF) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
  	}
  	printf("BadFrameReception ");
	i++;
  }

  if (FDDIS.ndd_flags & CFDDI_NDD_DAC) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
  	}
  	printf("DualAttachStation ");
	i++;
  }

  printf("\n");

}

