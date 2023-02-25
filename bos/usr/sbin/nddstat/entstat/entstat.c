static char sccsid[] = "@(#)56  1.6  src/bos/usr/sbin/nddstat/entstat/entstat.c, nddstat, bos41J, 9507B 2/3/95 14:05:24";
/*
 * COMPONENT_NAME: NDDSTAT 
 *
 * FUNCTIONS: 
 *		dev_stats
 *		dev_title
 *		dev_display
 *		en3com_display
 *		ient_display
 *		ien_isa_display
 *		lce_display
 *		ken_pci_display
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
#include <sys/cdli_entuser.h>

#include <locale.h>
#include <nl_types.h>
#include <entstat_msg.h>



/*
 * Define the statistic table for mapping all ethernet statistics.
 * The type name has to be stable_t so we can share the global 
 * statistics table stable
 */

typedef struct {
        ent_ndd_stats_t ent_ndd;
        union {
                en3com_stats_t en3com;
                ient_stats_t ient;
		ien_isa_stats_t ien_isa;
		enlce_stats_t lce;
		kent_stats_t kent;
        } ent_dev;
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
char cmdname[8] = "entstat";	/* command name defined for message header */
extern stable_t stable;         /* global statistic table */
extern char *devname;		/* device name */

nl_catd ent_catd;            	/* fd for entstat cat message file */

#define MSGSTR(n,s) catgets(ent_catd,MS_ENTSTAT,n,s)

#define NDDS stable.ent_ndd.ent_ndd_genstats
#define ENTS stable.ent_ndd.ent_ent_genstats
#define EN3COM stable.ent_dev.en3com
#define IENT stable.ent_dev.ient
#define IEN_ISA stable.ent_dev.ien_isa
#define LCE stable.ent_dev.lce
#define KENT stable.ent_dev.kent


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
  ent_catd = catopen(MF_ENTSTAT, NL_CAT_LOCALE);

  io_block.bufferlen = sizeof(ent_ndd_stats_t);
  io_block.bufferptr = (caddr_t)&stable;

  if (ioctl(fd, NDD_GET_STATS, (caddr_t)&io_block)) {
	return(errno);
  }

  if (need_ext_stats) {
    switch (ENTS.device_type) {
        case ENT_3COM:
                io_block.bufferlen = sizeof(en3com_all_stats_t);
                break;

        case ENT_IENT:
                io_block.bufferlen = sizeof(ient_all_stats_t);
                break;

        case ENT_IEN_ISA:
                io_block.bufferlen = sizeof(ien_isa_all_stats_t);
                break;

        case ENT_LCE:
                io_block.bufferlen = sizeof(enlce_all_stats_t);
                break;

	case ENT_KEN_PCI:
                io_block.bufferlen = sizeof(kent_all_stats_t);
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


  printf(MSGSTR(DEVNAME, "\nETHERNET STATISTICS (%s) :\n"), devname);

  printf(MSGSTR(DEVTYPE, "Device Type: "));
  switch (ENTS.device_type) {
        case ENT_3COM:
                printf(MSGSTR(HPLAN, 
		  "Ethernet High Performance LAN Adapter\n"));
                break;

        case ENT_IENT:
                printf(MSGSTR(ITDENT, "Integrated Ethernet Adapter\n"));
                break;

        case ENT_IEN_ISA:
                printf(MSGSTR(ISAIEN, "IBM ISA Ethernet adapter\n"));
                break;

        case ENT_LCE:
                printf(MSGSTR(ENLCE, 
			"Low Cost High Performance Ethernet adapter\n"));
                break;

	case ENT_KEN_PCI:
		printf(MSGSTR(PCIKEN, 
			"IBM PCI Ethernet Adapter (22100020)\n"));

        default:
                printf("\n");
                break;
  }

  printf(MSGSTR(HWADDR, "Hardware Address: "));
  for (i = 0; i < ENT_NADR_LENGTH - 1; i++) {
        printf("%02x:", ENTS.ent_nadr[i]);
  }
  printf("%02x\n", ENTS.ent_nadr[i]);






}
/*****************************************************************************/
/*
 * NAME:     dev_display
 *
 * FUNCTION: Format and write the statistics of the ent_genstats_t to
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
  if (ENTS.dev_elapsed_time != NDDS.ndd_elapsed_time) {
  	printf(MSGSTR(ELAPSED_TIME, "Elapsed Time: "));
  	snds = ENTS.dev_elapsed_time % 60;
  	minutes = ENTS.dev_elapsed_time / 60;
  	hrs = minutes / 60;
  	minutes = minutes % 60;
  	days = hrs / 24;
  	hrs = hrs % 24;
  	printf(MSGSTR(TIME, "%d days %d hours %d minutes %d seconds\n"), 
		days, hrs, minutes, snds);
  }

  printf(MSGSTR(BCAST, "Broadcast Packets: %-26d Broadcast Packets: %d\n"), 
	ENTS.bcast_tx_ok, ENTS.bcast_rx_ok);

  printf(MSGSTR(MCAST, "Multicast Packets: %-26d Multicast Packets: %d\n"),
	ENTS.mcast_tx_ok, ENTS.mcast_rx_ok);

  printf(MSGSTR(CARRIER, "No Carrier Sense: %-27d CRC Errors: %d\n"), 
	ENTS.carrier_sense, ENTS.fcs_errs);

  printf(MSGSTR(DMA, "DMA Underrun: %-31d DMA Overrun: %d\n"), 
	ENTS.underrun, ENTS.overrun);

  printf(MSGSTR(CTSERR, "Lost CTS Errors: %-28d Alignment Errors: %d\n"), 
	ENTS.cts_lost, ENTS.align_errs);

  printf(MSGSTR(MAXCOLL, 
	"Max Collision Errors: %-23d No Resource Errors: %d\n"), 
	ENTS.excess_collisions, ENTS.no_resources);

  printf(MSGSTR(LATECOLL, 
	"Late Collision Errors: %-22d Receive Collision Errors: %d\n"), 
	ENTS.late_collisions, ENTS.rx_collisions);

  printf(MSGSTR(DEFERED, "Deferred: %-35d Packet Too Short Errors: %d\n"), 
	ENTS.defer_tx, ENTS.short_frames);

  printf(MSGSTR(SQE, "SQE Test: %-35d Packet Too Long Errors: %d\n"), 
	ENTS.sqetest, ENTS.long_frames);

  printf(MSGSTR(TMOUT, 
	"Timeout Errors: %-29d Packets Discarded by Adapter: %d\n"), 
	ENTS.tx_timeouts, ENTS.rx_drop);

  printf(MSGSTR(SCOLL, 
	"Single Collision Count: %-21d Receiver Start Count: %d\n"), 
	ENTS.s_coll_frames, ENTS.start_rx);

  printf(MSGSTR(MCOLL, "Multiple Collision Count: %d\n"), ENTS.m_coll_frames);

  printf(MSGSTR(HW_TXQ_LEN, "Current HW Transmit Queue Length: %d\n"), 
	ENTS.hw_txq_len);


  /* 
   * General statistics. This part of the output format should be the
   * same for every command.
   */

  printf(MSGSTR(GENSTATS, "\nGeneral Statistics:\n"));
  printf("-------------------\n");
  printf(MSGSTR(NOMBUF, "No mbuf Errors: %d\n"), NDDS.ndd_nobufs);
  printf(MSGSTR(RESET, "Adapter Reset Count: %d\n"), ENTS.restart_count);
  printf(MSGSTR(FLAGS, "Driver Flags: "));

  i = 0;
  if (ENTS.ndd_flags & NDD_UP) {
  	printf("Up ");
	i++;
  }
  if (ENTS.ndd_flags & NDD_BROADCAST) {
  	printf("Broadcast ");
	i++;
  }
  if (ENTS.ndd_flags & NDD_DEBUG) {
  	printf("Debug ");
	i++;
  }
  if (ENTS.ndd_flags & NDD_RUNNING) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
	}
	printf("Running ");
	i++;
  }
  if (ENTS.ndd_flags & NDD_SIMPLEX) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
	}
  	printf("Simplex ");
	i++;
  }
  if (ENTS.ndd_flags & NDD_DEAD) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
	}
  	printf("Dead ");
	i++;
  }
  if (ENTS.ndd_flags & NDD_LIMBO) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
	}
  	printf("Limbo ");
	i++;
  }
  if (ENTS.ndd_flags & NDD_PROMISC) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
	}
	printf("Promiscuous ");
	i++;
  }
  if (ENTS.ndd_flags & NDD_ALTADDRS) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
  	}
  	printf("AlternateAddress ");
	i++;
  }
  if (ENTS.ndd_flags & NDD_MULTICAST) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
  	}
  	printf("AllMulticast ");
	i++;
  }
  if (ENTS.ndd_flags & NDD_DETACHED) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
  	}
  	printf("Detached ");
	i++;
  }

  if (ENTS.ndd_flags & ENT_RCV_BAD_FRAME) {
  	if (i >= 3) {
		printf("\n\t");
		i = 0;
  	}
  	printf("ReceiveBadPacket");
  }
  printf("\n");

  /*
   * Display device specific statistics if the -d option is specified
   * by user.
   */
  if (need_ext_stats) {
	switch (ENTS.device_type) {
		case ENT_3COM:
			en3com_display();
			break;

		case ENT_IENT:
			ient_display();
			break;

		case ENT_IEN_ISA:
			ien_isa_display();
			break;

		case ENT_LCE:
			lce_display();
			break;

		case ENT_KEN_PCI:
			ken_pci_display();

		default:
			break;
	}
  }

}


/*****************************************************************************/
/*
 * NAME:     en3com_display
 *
 * FUNCTION: Format and write the statistics of the en3com_stats_t to
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
en3com_display() 

{

  printf(MSGSTR(HPLAN_STATS, 
	"\nEthernet High Performance LAN Adapter Specific Statistics:\n"));
  printf("----------------------------------------------------------\n");
  printf(MSGSTR(RV_POOL, "Receive Buffer Pool Size: %d\n"), 
	EN3COM.rv_pool_size);
  printf(MSGSTR(TX_POOL, "Transmit Buffer Pool Size: %d\n"), 
	EN3COM.tx_pool_size);
  printf(MSGSTR(IP_PROMIS, "In Promiscuous Mode for IP Multicast: "));
  if (EN3COM.multi_promis_mode)
	printf("Yes\n");
  else
	printf("No\n");
  printf(MSGSTR(UPLOAD, "Packets Uploaded from Adapter: %d\n"),
	EN3COM.adpt_rcv_pack);
  printf(MSGSTR(HOST_EOL, "Host End-of-List Encountered: %d\n"), 
	EN3COM.host_rcv_eol);
  printf(MSGSTR(EOL_586, "82586 End-of-List Encountered: %d\n"), 
	EN3COM.adpt_rcv_eol);
  printf(MSGSTR(DMATMOUT, "Receive DMA Timeouts: %d\n"), EN3COM.rcv_dma_to);
  printf(MSGSTR(RESERVED, "Adapter Internal Data: 0x%x 0x%x 0x%x 0x%x 0x%x\n"), 
	EN3COM.reserved[0], EN3COM.reserved[1], EN3COM.reserved[2], 
	EN3COM.reserved[3], EN3COM.reserved[4]);



}


/*****************************************************************************/
/*
 * NAME:     ient_display
 *
 * FUNCTION: Format and write the statistics of the ient_stats_t to
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
ient_display() 

{
  int i;


  printf(MSGSTR(ITDENT_STATS, 
  	"\nIntegrated Ethernet Adapter Specific Statistics:\n"));
  printf("------------------------------------------------\n");

  printf(MSGSTR(COLL_TABLE, "Packets with Transmit collisions:\n"));
  for (i = 0; i < 5; i++) {
	printf(MSGSTR(COLL_FREQ,
	  "%2d collisions: %-10d %2d collisions: %-10d %2d collisions: %d\n"),
	  i+1, IENT.coll_freq[i], i+6, IENT.coll_freq[i+5], i+11,
	  IENT.coll_freq[i+10]);
  }




}
/*****************************************************************************/
/*
 * NAME:     ien_isa_display
 *
 * FUNCTION: Format and write the statistics of the ien_isa_stats_t to
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
ien_isa_display() 

{

  int i;


  printf(MSGSTR(IENISA_STATS, 
	"\nIBM ISA Ethernet Adapter Specific Statistics:\n"));
  printf("---------------------------------------------\n");
  printf(MSGSTR(IP_PROMIS, "In Promiscuous Mode for IP Multicast: "));
  if (IEN_ISA.multi_promis_mode)
	printf("Yes\n");
  else
	printf("No\n");

  printf(MSGSTR(COLL_TABLE, "Packets with Transmit collisions:\n"));
  for (i = 0; i < 5; i++) {
	printf(MSGSTR(COLL_FREQ,
	  "%2d collisions: %-10d %2d collisions: %-10d %2d collisions: %d\n"),
	  i+1, IEN_ISA.coll_freq[i], i+6, IEN_ISA.coll_freq[i+5], i+11,
	  IEN_ISA.coll_freq[i+10]);
  }


}

/*****************************************************************************/
/*
 * NAME:     lce_display
 *
 * FUNCTION: Format and write the statistics of the enlce_stats_t to
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
lce_display() 

{


  printf(MSGSTR(ENLCE_STATS, 
	"\nLow Cost High Performance Ethernet Adapter Specific Statistics:\n"));
  printf("---------------------------------------------------------------\n");
  printf(MSGSTR(ENLCE_TYPE, "Adapter Type: "));
  switch (LCE.adapter_type) {
	case AUI_BASET:
		printf("AUI_10BASET\n");
		break;
	case BASE2_ONLY:
		printf("10BASE2\n");
		break;
	default:
		printf("\n");
		break;
  }

  printf(MSGSTR(ENLCE_MEDIA, "Media Selection: "));
  switch (LCE.media_selection) {
	case AUI:
		printf("AUI\n");
		break;
	case BASET:
		printf("10BASET\n");
		break;
	case BASE2:
		printf("10BASE2\n");
		break;
	case AUTO:
		printf("AUTOSENSE\n");
		break;
	default:
		printf("\n");
		break;
  }
		
  printf(MSGSTR(ENLCE_MACE, "Ethernet Controller Version: 0x%x\n"), 
	LCE.mace_version);
  printf(MSGSTR(ENLCE_MCNET, "Micro Channel Controller Version: 0x%x\n"),
	LCE.mcnet_version);


}

/*****************************************************************************/
/*
 * NAME:     ken_pci_display
 *
 * FUNCTION: Format and write the statistics of the kent_stats_t to
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
ken_pci_display() 

{
  int i;


  printf(MSGSTR(PCIKEN_STATS, 
  	"\nIBM PCI Ethernet Adapter Specific Statistics:\n"));
  printf("------------------------------------------------\n");

  printf(MSGSTR(COLL_TABLE, "Packets with Transmit collisions:\n"));
  for (i = 0; i < 5; i++) {
	printf(MSGSTR(COLL_FREQ,
	  "%2d collisions: %-10d %2d collisions: %-10d %2d collisions: %d\n"),
	  i+1, KENT.coll_freq[i], i+6, KENT.coll_freq[i+5], i+11,
	  KENT.coll_freq[i+10]);
  }

}
