static char sccsid[] = "@(#)84  1.5  src/bos/usr/sbin/nddstat/tokstat/tokstat.c, nddstat, bos41J, 9520B_all 5/18/95 11:44:00";
/*
 * COMPONENT_NAME: NDDSTAT 
 *
 * FUNCTIONS: 
 *		dev_stats
 *		dev_title
 *		dev_display
 *		isa_display
 *		mon_display
 *		mps_display
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
#include <sys/cdli_tokuser.h>

#include <locale.h>
#include <nl_types.h>
#include <tokstat_msg.h>

/*
 * Define the statistic table for mapping all token-ring statistics.
 * The type name has to be stable_t so we can share the global 
 * statistics table stable
 */

typedef struct {
        tok_ndd_stats_t tok_ndd;
        union {
                tr_mon_stats_t mon;
                tr_mps_stats_t mps;
                tr_ibm_isa_stats_t isa;
                tr_sky_stats_t sky;
        } tok_dev;
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
char cmdname[8] = "tokstat";	/* command name defined for message header */
extern stable_t stable;         /* global statistic table */
extern char *devname;		/* device name */

nl_catd tok_catd;            	/* fd for tokstat cat message file */


#define MSGSTR(n,s) catgets(tok_catd,MS_TOKSTAT,n,s)

#define NDDS stable.tok_ndd.tok_ndd_stats
#define TOKS stable.tok_ndd.tok_gen_stats
#define ISA stable.tok_dev.isa
#define MON stable.tok_dev.mon
#define MPS stable.tok_dev.mps
#define SKY stable.tok_dev.sky


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
  tok_catd = catopen(MF_TOKSTAT, NL_CAT_LOCALE);

  io_block.bufferlen = sizeof(tok_ndd_stats_t);
  io_block.bufferptr = (caddr_t)&stable;

  if (ioctl(fd, NDD_GET_STATS, (caddr_t)&io_block)) {
	return(errno);
  }

  if (need_ext_stats) {
    switch (TOKS.device_type) {
        case TOK_IBM_ISA:
                io_block.bufferlen = sizeof(tr_ibm_isa_all_stats_t);
                break;

        case TOK_MON:
                io_block.bufferlen = sizeof(mon_all_stats_t);
                break;

        case TOK_MPS:
                io_block.bufferlen = sizeof(mps_all_stats_t);
                break;

        case TOK_SKY:
                io_block.bufferlen = sizeof(sky_all_stats_t);
                break;

        default:
                return(0);
    }
    io_block.bufferptr = (caddr_t)&stable;

    if (ioctl(fd, NDD_GET_ALL_STATS, (caddr_t)&io_block)) {
    	return(errno);
    }

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


  printf(MSGSTR(DEVNAME, "\nTOKEN-RING STATISTICS (%s) :\n"), devname);

  printf(MSGSTR(DEVTYPE, "Device Type: "));
  switch (TOKS.device_type) {
        case TOK_IBM_ISA:
                printf(MSGSTR(ISANAME, 
		  "Token-Ring IBM ISA Adapter\n"));
                break;

        case TOK_MON:
                printf(MSGSTR(MONNAME, 
		  "Token-Ring High-Performance Adapter (8fc8)\n"));
                break;

        case TOK_MPS:
                printf(MSGSTR(MPSNAME,
		  "Token-Ring High-Performance Adapter (8fa2)\n"));
                break;

        case TOK_SKY:
                printf(MSGSTR(SKYNAME,
		  "IBM PCI Token-Ring High-Performance Adapter (14101800)\n"));
                break;

        default:
                printf("\n");
                break;
  }

  printf(MSGSTR(HWADDR, "Hardware Address: "));
  for (i = 0; i < CTOK_NADR_LENGTH - 1; i++) {
        printf("%02x:", TOKS.tok_nadr[i]);
  }
  printf("%02x\n", TOKS.tok_nadr[i]);
}

/*****************************************************************************/
/*
 * NAME:     dev_display
 *
 * FUNCTION: Format and write the statistics of the tok_genstats_t to
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
  int i;
  int days, hrs, minutes, snds;


  printf("\n");
  if (TOKS.dev_elapsed_time != NDDS.ndd_elapsed_time) {
  	printf(MSGSTR(ELAPSED_TIME, "Elapsed Time: "));
  	snds = TOKS.dev_elapsed_time % 60;
  	minutes = TOKS.dev_elapsed_time / 60;
  	hrs = minutes / 60;
  	minutes = minutes % 60;
  	days = hrs / 24;
  	hrs = hrs % 24;
  	printf(MSGSTR(TIME, "%d days %d hours %d minutes %d seconds\n"), 
		days, hrs, minutes, snds);
  }

  printf(MSGSTR(BCAST, "Broadcast Packets: %-26d Broadcast Packets: %d\n"), 
	TOKS.bcast_xmit, TOKS.bcast_recv);

  printf(MSGSTR(MCAST, "Multicast Packets: %-26d Multicast Packets: %d\n"),
	TOKS.mcast_xmit, TOKS.mcast_recv);

  printf(MSGSTR(TIMEOUT_CONG,
	"Timeout Errors: %-29d Receive Congestion Errors: %d\n"), 
	TOKS.tx_timeouts, TOKS.rx_congestion);

  printf(MSGSTR(SW_TXQ_LEN, "Current SW Transmit Queue Length: %d\n"), 
	TOKS.sw_txq_len);

  printf(MSGSTR(HW_TXQ_LEN, "Current HW Transmit Queue Length: %d\n"), 
	TOKS.hw_txq_len);


  /* 
   * General statistics. This part of the output format should be the
   * same for every command.
   */

  printf(MSGSTR(GENSTATS, "\nGeneral Statistics:\n"));
  printf("-------------------\n");

  printf(MSGSTR(NOMBUF_LOBE,
	"No mbuf Errors: %-29d Lobe Wire Faults: %d\n"), 
	NDDS.ndd_nobufs, TOKS.lobewires);

  printf(MSGSTR(ABORT_AC,
	"Abort Errors: %-31d AC Errors: %d\n"), 
	TOKS.abort_errs, TOKS.ac_errs);

  printf(MSGSTR(BURST_COPY,
	"Burst Errors: %-31d Frame Copy Errors: %d\n"), 
	TOKS.burst_errs, TOKS.framecopies);

  printf(MSGSTR(FREQ_HARD,
	"Frequency Errors: %-27d Hard Errors: %d\n"), 
	TOKS.freq_errs, TOKS.hard_errs);

  printf(MSGSTR(INT_LINE,
	"Internal Errors: %-28d Line Errors: %d\n"), 
	TOKS.int_errs, TOKS.line_errs);

  printf(MSGSTR(LOST_ONLY,
	"Lost Frame Errors: %-26d Only Station: %d\n"), 
	TOKS.lostframes, TOKS.singles);

  printf(MSGSTR(TOKEN_REMOVE,
	"Token Errors: %-31d Remove Received: %d\n"), 
	TOKS.token_errs, TOKS.removes);

  printf(MSGSTR(RING_SIGNAL,
	"Ring Recovered: %-29d Signal Loss Errors: %d\n"), 
	TOKS.recoverys, TOKS.signal_loss);

  printf(MSGSTR(SOFT_BEACON,
	"Soft Errors: %-32d Transmit Beacon Errors: %d\n"), 
	TOKS.soft_errs, TOKS.tx_beacons);

  printf(MSGSTR(FLAGS, "Driver Flags: "));

  i = 0;
  if (TOKS.ndd_flags & NDD_UP) {
  	printf("Up ");
	i++;
  }
  if (TOKS.ndd_flags & NDD_BROADCAST) {
  	printf("Broadcast ");
	i++;
  }
  if (TOKS.ndd_flags & NDD_DEBUG) {
  	printf("Debug ");
	i++;
  }

  if (TOKS.ndd_flags & NDD_RUNNING) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
	}
  	printf("Running ");
	i++;
  }

  if (TOKS.ndd_flags & NDD_SIMPLEX) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
	}
  	printf("Simplex ");
	i++;
  }

  if (TOKS.ndd_flags & NDD_DEAD) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
	}
  	printf("Dead ");
	i++;
  }

  if (TOKS.ndd_flags & NDD_LIMBO) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
	}
  	printf("Limbo ");
	i++;
  }

  if (TOKS.ndd_flags & NDD_PROMISC) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
	}
	printf("Promiscuous ");
	i++;
  }

  if (TOKS.ndd_flags & NDD_ALTADDRS) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
  	}
  	printf("AlternateAddress ");
	i++;
  }

  if (TOKS.ndd_flags & NDD_MULTICAST) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
  	}
  	printf("AllMulticast ");
	i++;
  }

  if (TOKS.ndd_flags & TOK_ATTENTION_MAC) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
  	}
  	printf("ReceiveAttentionMAC ");
	i++;
  }

  if (TOKS.ndd_flags & TOK_BEACON_MAC) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
  	}
  	printf("ReceiveBeaconMAC ");
	i++;
  }

  if (TOKS.ndd_flags & TOK_RECEIVE_FUNC) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
  	}
  	printf("ReceiveFunctionalAddr ");
	i++;
  }

  if (TOKS.ndd_flags & TOK_RECEIVE_GROUP) {
  	if (i >= 3) {
		printf("\n\t");
  	}
  	printf("ReceiveGroupAddr ");
	i++;
  }

  if (TOKS.ndd_flags & TOK_RING_SPEED_4) {
  	if (i >= 3) {
		printf("\n\t");
  	}
  	printf("4 Mbps ");
	i++;
  }

  if (TOKS.ndd_flags & TOK_RING_SPEED_16) {
  	if (i >= 3) {
		printf("\n\t");
  	}
  	printf("16 Mbps ");
  }

  printf("\n");

  /*
   * Display device specific statistics if the -d option is specified
   * by user.
   */
  if (need_ext_stats) {
	printf("\n");

	switch (TOKS.device_type) {
		case TOK_IBM_ISA:
			isa_display();
			break;

		case TOK_MON:
			mon_display();
			break;

		case TOK_MPS:
			mps_display();
			break;

		case TOK_SKY:
			sky_display();
			break;

		default:
			break;
	}
  }

}

/*****************************************************************************/
/*
 * NAME:     isa_display
 *
 * FUNCTION: Format and write the statistics of the tr_ibm_isa_stats_t to
 *	     the stdout. 
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      dev_display
 *
 * INPUT:
 *	none
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
isa_display() 

{

  printf(MSGSTR(ISASTATS, 
	"\nToken-Ring IBM ISA Adapter Specific Statistics:\n"));
  printf("-----------------------------------------------\n");

  printf(MSGSTR(READLOGS, "Number of read log commands issued: %d\n"),
	ISA.read_logs);
}

/*****************************************************************************/
/*
 * NAME:     mon_display
 *
 * FUNCTION: Format and write the statistics of the tr_mon_stats_t to
 *	     the stdout. 
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      dev_display
 *
 * INPUT:
 *	none
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
mon_display() 

{

  printf(MSGSTR(MONSTATS, 
	"\nToken-Ring High-Performance Adapter (8fc8) Specific Statistics:\n"));
  printf("--------------------------------------------------------------\n");

  printf(MSGSTR(BUS_PARITY,
	"DMA Bus Errors: %-29d DMA Parity Errors: %d\n"), 
	MON.DMA_bus_errors, MON.DMA_parity_errors);
  printf(MSGSTR(ARI, "ARI/FCI Errors: %d\n"), MON.ARI_FCI_errors);
}

/*****************************************************************************/
/*
 * NAME:     mps_display
 *
 * FUNCTION: Format and write the statistics of the tr_mps_stats_t to
 *	     the stdout. 
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      dev_display
 *
 * INPUT:
 *	none
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
mps_display() 

{

  printf(MSGSTR(MPSSTATS, 
	"\nToken-Ring High-Performance Adapter (8fa2) Specific Statistics:\n"));
  printf("--------------------------------------------------------------\n");

  printf(MSGSTR(OVER_UNDER,
	"Receive Overruns: %-27d Transmit Underruns: %d\n"), 
	MPS.recv_overrun, MPS.xmit_underrun);

  printf(MSGSTR(RX_F_ERROR,
	"Receive Frame Errors: %-23d\n"), 
	 MPS.rx_frame_err);
}

/*****************************************************************************/
/*
 * NAME:     sky_display
 *
 * FUNCTION: Format and write the statistics of the tr_mps_stats_t to
 *	     the stdout. 
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * NOTES:
 *
 * CALLED FROM:
 *      dev_display
 *
 * INPUT:
 *	none
 *
 * RETURNS:  
 *	none
 */
/*****************************************************************************/
sky_display() 

{

  printf(MSGSTR(SKYSTATS, 
	"IBM PCI Token-Ring High-Performance Adapter (14101800) Specific Statistics:\n"));
  printf("--------------------------------------------------------------\n");

  printf(MSGSTR(OVER_UNDER,
	"Receive Overruns: %-27d Transmit Underruns: %d\n"), 
	SKY.recv_overrun, SKY.xmit_underrun);

  printf(MSGSTR(R_ERR_N_TX_R,
	"Receive Frame Errors: %-23d No_Txq_Resource: %d\n"), 
	SKY.rx_frame_err, SKY.no_txq_resource);
}
