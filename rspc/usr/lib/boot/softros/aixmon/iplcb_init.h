/* @(#)99	1.7  src/rspc/usr/lib/boot/softros/aixmon/iplcb_init.h, rspc_softros, rspc41J, 9521A_all 5/23/95 09:48:13  */
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: BID_VAL
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*  iplcb_init.h - Header File for IPL control block initialiation routine */

#include <sys/iplcb.h>
#include <sys/rosinfo.h>


/************************************************************************/
/****************** General Defines *************************************/
/************************************************************************/

#define BITMAP_SIZE	8192		/* Number of bytes in RAM bitmap*/
					/* With BYTES_PER_BIT = 16384,
					 * this allows 1Gb of real mem. */
#define BYTES_PER_BIT	16384		/* Bytes/bit in bitmap		*/

#define NOT_MP			(0)
#define NOT_USED		(0)
#define NOT_APPLICABLE         (-1)

/************************************************************************/
/****************** Platform Defines ************************************/
/************************************************************************/

#define PPC_MODEL	     0x0800	/* Code for PPC Model		*/
#define MAX_BUC_SLOTS	     16		/* Number of Buc structures	*/
#define MAX_NUM_OF_SIMMS     6		/* Number of SIMM slots		*/
#define PROCESSOR_RUN_AIX    1		/* Does processor run aix	*/
#define A_PROCESSOR         (2)
#define A_IO_DEVICE         (3)
#define BUC_HW_CONFIGURED   (2)
#define P601_SLOT           (0)
#define P601_RESET          (0x80000000)
#define ENABLE_CONFIG       (0x80000000)
#define NVRAM_BASE_ADDR     (0xFF600000)
#define NVRAM_SIZE          (0x00002000) /* Size of NVRAM cache		*/

#define BUS_PCMCIA              4       /* Used by Masters machines to  */
                                        /* cover PCMCIA bus interrupts  */
#define BUS_ISA                 5       /* Used by Masters machines to  */
                                        /* cover edge triggered ints    */
#define BUS_PCI                 6       /* Used by Masters machines to  */
                                        /* cover level triggered ints   */

					/* BUC defines for SF busses	*/
#define A_BUS_BRIDGE	5 		/* Device Characteristics	*/

#define PCI_BUS_ID	0x2010		/* Device ID Types		*/
#define ISA_BUS_ID	0x2020
#define PCMCIA_BUS_ID	0x2030
#define DMA_BUC_ID	0x2040
#define HIBERNATION_BUC_ID	0x2050

					/* Size of DMA Buffer 		*/
					/* Must be a multiple of memroy
					   bitmap bits			*/
#define DMA_BUF_SIZE		(2 * BYTES_PER_BIT)

#define	RSPC_UP_PCI	3
#define RSPC		2

/*** Temporary until ioacc.h gets dropped	***/

#define	IO_PCI		0x0003
#define IO_ISA		0x0004

#define PCI_IOMEM	0
#define PCI_BUSMEM	1
#define PCI_CFGMEM	2

#define ISA_IOMEM	0
#define ISA_BUSMEM	1

#define BID_VAL(type, region, num) \
	(((type) << 6) | ((region) << 16) | (num))

/* Defines from RS/6000 601 ROS		*/

#define INVALID               0
#define UP                    0
#define DOWN                  1
#define NO_CONFLICT           0
#define CONFLICT              1
#define PBS_fixed_address        0x10000
#define MAX_EXTENTS              16

#define TWOBYTES                      2
#define FOURBYTES                     4
#define HEXZERO                       0x00
#define CRCBYTES_LEN                  4
#define CHKBYTES_LEN                  2
#define LISTSIZE                      84  /* length of each NVRAM device list */
#define VOLIDSIZE                     16  /* length of the scsi volume id     */
#define SEARCH_PATH                   1
#define NOSEARCH_PATH                 2
#define YES                           0
#define NO                            1
#define NVRAM_EXPAN                   'R'
#define NATIVE_IO                     'N'
#define ROS_FLOPPY                    'F'
#define SJL                           'K'
#define GENERAL                       'G'
#define INTERNAL_SCSI                 'I'
#define EXTERNAL_SCSI                 'E'
#define TAPE_SCSI                     'T'
#define CDROM_SCSI                    'C'
#define VOLID_SCSI                    'V'
#define SPECIFIC_SCSI_ADDR            'S'
#define FEATURE_ROM                   'M'
#define ETHERNET_BOOT                 'D'
#define TOKEN_BOOT                    'O'
#define FDDI_BOOT                     'P'
#define SERVER_IP_ADDR                'B'
#define GATEWAY_IP_ADDR               'W'
#define CLIENT_IP_ADDR                'L'
#define HARDWARE_ADDR                 'H'
#define NETWORK_ADDR_UNKNOWN          0
#define NETWORK_ADDR_KNOWN            1

#define FAIL               FALSE
#define SEARCH_PATH        1
#define NOSEARCH_PATH      2
#define SECURE             1
#define SERVICE            2
#define NORMAL             3
#define UNDEFINED_KEY_POS  0
#define KEY_DELAY          500000                /* delay in micro seconds   */
#define LED_DELAY          500000                /* delay in micro seconds   */

#define UI_COUNT                     1   /* the number of ipl tries between  */
                                         /* calls to the user interface      */
#define IPL_UNDEFINED                0   /* step controls for iplc           */
#define IPL_NORMAL_MODE_DEVICE_LIST  1
#define IPL_PREVIOUS_DEVICE          2
#define IPL_ROM_SCAN                 3
#define IPL_SCSI_HARDFILE            4
#define IPL_SCSI_CDROM_TAPE          5
#define IPL_NIO                      6
#define IPL_TOKEN                    7
#define IPL_ETHERNET                 8
#define IPL_SERVICE_MODE_DEVICE_LIST 9
#define IPL_UPDATE_COUNT             10
#define IPL_CALL_USER_INTERFACE      11
#define IPL_FDDI                     12
