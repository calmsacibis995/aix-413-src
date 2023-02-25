/* @(#)56	1.5  src/rspc/kernext/isa/itok/itok_hw.h, isatok, rspc41J, 9524A_all 6/1/95 12:42:05 */
/*
 * COMPONENT_NAME: ISATOK - IBM 16/4 PowerPC Token-ring driver.
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_ITOK_HW
#define _H_ITOK_HW

/*

  This section contains constants and structures for the Adapter
  Identification PROM (AIP). It's only used when the driver boots and
  it is read-only. Located in the BIOS area.
*/

#define ENCODED_HW_ADDR_OFFSET		0x00  /* Tokenring address location. */
#define CHANNEL_ID_OFFSET		0x30  /* MC or ISA indication.       */
#define ADAPTER_TYPE_OFFSET		0xA0  /* Several types supported.    */
#define DATA_RATE_OFFSET		0xA2  /* 4, 16 Mbps or both.         */
#define EARLY_TOKEN_RELEASE_OFFSET	0xA4  /* Early token-release supp. ? */
#define RAM_SIZE_OFFSET			0xA6  /* How much shared RAM.        */
#define RAM_PAGING_OFFSET		0xA8  /* What page size are supported*/
#define DHB_4MB_SIZE_OFFSET		0xAA  /* Max DHB size at 4 Mbps.     */
#define DHB_16MB_SIZE_OFFSET		0xAC  /* Max DHB size at 16 Mbps.    */

#define AIP_CHECKSUM_RANGE		0x60  /* For initial checksum        */
#define AIP_CHECKSUM2_RANGE		0xF0  /* calculations.               */

#define RAM_64K_AVAILABLE		0x0B  /* Check bit.                  */
#define RRR_RAM_SIZE			0x0F  /* Check bit.                  */

#define TOK_MAX_CORRELATOR		0x255

#define ADAPTER_IS_8BIT_WIDE		0x0e  /* 8 Bit adapter.              */
#define ADAPTER_IS_16BIT_WIDE		0x0d  /* 16 Bit adapter.             */

/* This structure is filled in when the driver boots (read-only info). */
typedef struct {
  char  encoded_hw_addr[CTOK_NADR_LENGTH];/* Token-ring addr.             */
  uchar adapter_type;                     /* F = 1st,E = 2nd,..,O = 16th. */
  uchar data_rate;                        /* F = 4Mbps,E = 16Mbps,D = both*/
  uchar early_token_release;              /* F = no,E = 4,D = 16,C = Both */
  uchar avaliable_RAM;                    /* Total available RAM.         */
  uchar RAM_paging;                       /* Paging & size support.       */
  uchar initialize_RAM_area;              /* Reset upper 512 bytes ?      */
  uchar DHB_size_4_mbps;                  /* Max. DHB size at 4 Mbps.     */
  uchar DHB_size_16_mbps;                 /* Max. DHB size at 16 Mbps.    */
} aip_t;

/* 
  The following constants and structures are for the Adapter Control Area (ACA)
  which is part of the MMIO (shared memory) region and contains 9 double-
  registers.
*/

typedef struct {
  ushort RRR_register;      /* RAM Relocation register.             */
  ushort WRBR_register;     /* Write Region Base Register.          */
  ushort WWOR_register;     /* Write Window Open Register.          */
  ushort WWCR_register;     /* Write Window Close Register.         */
  ushort ISRP_register;     /* Interrupt Status Register PC.        */
  ushort ISRA_register;     /* Interrupt Status Register Adapter.   */
  ushort TCR_register;      /* Timer Control Register.              */
  ushort TVR_register;      /* Timer Value Register.                */
  ushort SRPR_register;     /* Shared Ram Page Register.            */
} aca_t;

/*
   ACA area offsets for (read,write,set,reset) operations of certain bits
   in the (RRR,WRBR,WWOR,WWCR,ISRP,ISRA,TCR,TVR,SRPR) registers.
   These registers are located in the ACA area and are all double-registers
   named (xxxx_even, xxxx_odd).
*/

#define RRR_EVEN_OFFSET		0 /* Ram Relocation Register (double register)*/
#define RRR_ODD_OFFSET		1 /* (Where to position the shared RAM).      */
#define WRBR_EVEN_OFFSET	2 /* Write Region Base Register (double)      */
#define WRBR_ODD_OFFSET		3 /* (Start of the writeable area in RAM).    */
#define WWOR_EVEN_OFFSET	4 /* Write Window Open Register (double)      */
#define WWOR_ODD_OFFSET		5 /* (Start of a write window)                */
#define WWCR_EVEN_OFFSET	6 /* Write Window Close Register              */
#define WWCR_ODD_OFFSET		7 /* (End of the write window).               */
#define ISRP_EVEN_OFFSET	8 /* Interrupt Status Register PC (double)    */
#define ISRP_ODD_OFFSET		9 /* (Used by the adapter for the PC).        */
#define ISRA_EVEN_OFFSET     0x0A /* Interrupt Status Register Adap. (double) */
#define ISRA_ODD_OFFSET	     0x0B /* (Used by the driver towards the adapter. */
#define TCR_EVEN_OFFSET	     0x0C /* Timer Control Register (double)          */
#define TCR_ODD_OFFSET	     0x0D /* (Second half in ODD).                    */
#define TVR_EVEN_OFFSET	     0x0E /* Timer Value Register (double)            */
#define TVR_ODD_OFFSET	     0x0F /* (Initial countdown,Current count).       */
#define SRPR_EVEN_OFFSET     0x18 /* Shared RAM Page Register (double)        */
#define SRPR_ODD_OFFSET	     0x19 /* (which of the 4 pages is in use).        */
#define ISRP_EVEN_RESET	     0x28 /* ISRP even byte reset.                    */
#define ISRP_ODD_RESET	     0x29 /* ISRP odd byte reset.                     */
#define ISRA_ODD_RESET	     0x2B /* ISRP odd byte reset.                     */
#define TCR_EVEN_RESET	     0x2C /* ISRP even byte reset.                    */
#define ISRP_EVEN_SET	     0x48 /* ISRP even byte reset.                    */
#define ISRP_ODD_SET	     0x49 /* ISRP odd byte reset.                     */
#define ISRA_ODD_SET	     0x4B /* ISRP odd byte reset.                     */
#define TCR_EVEN_SET	     0x4C /* ISRP even byte reset.                    */

/* Offsets within the init response block (in bytes). */
#define INIT_STATUS_OFFSET	0x01
#define BRINGUP_CODE_OFFSET	0x06
#define ADAPTER_ADDR_OFFSET	0x0C
#define ADAPTER_PARAM_OFFSET	0x0E

/*

  The following 4 structures are used for command handshaking with the IBM 16/4
  8 bit ISA adapter.

             ------------                       -------------
             !          ! ------ SRB ---------> !           !
             !  Driver  !                       !  Adapter  !
             !          ! <----- SSB ---------- !           !
             ------------                       -------------

  SRB : Used by the driver when passing a command to the adapter.

  SSB : Used by the adapter to pass a response to the driver.


             ------------                       -------------
             !          ! <----- ARB ---------- !           !
             !  Driver  !                       !  Adapter  !
             !          ! ------ ASB ---------> !           !
             ------------                       -------------

  ARB : Used by the adapter when passing a command/status to the driver.

  ASB : Used by the driver to pass a response to the adapter.
*/

typedef struct {
  uint busy;
  uint offset;
} srb_t;

typedef struct {
  uint offset;
} ssb_t;

typedef struct {
  uint offset;
} arb_t;

typedef struct {
  uint offset;
} asb_t;

#define SRB_RESPONSE	(intr_val & 0x20)
#define SSB_RESPONSE	(intr_val & 0x04)
#define ARB_COMMAND	(intr_val & 0x08)
#define ASB_COMMAND	(intr_val & 0x10)

/* Adapter commands. */
#define SRB_INTERRUPT		0x00
#define SRB_MODIFY		0x01
#define SRB_RESTORE		0x02
#define SRB_OPEN		0x03
#define SRB_CLOSE		0x04
#define SRB_SET_GROUP_ADDR	0x06
#define SRB_SET_FUNC_ADDR	0x07
#define SRB_READ_LOG		0x08
#define SRB_TX_DIR_FRAME	0x0A
#define SRB_TX_I_FRAME		0x0B
#define SRB_TX_UI_FRAME		0x0D
#define SRB_TX_XID_CMD		0x0E
#define SRB_TX_XID_RESP_FINAL	0x0F
#define SRB_TX_XID_RESP		0x10
#define SRB_TX_TEST_CMD		0x11
#define SRB_RESET		0x14
#define SRB_OPEN_SAP		0x15
#define SRB_CLOSE_SAP		0x16
#define SRB_REALLOCATE		0x17
#define SRB_OPEN_STATION	0x19
#define SRB_CLOSE_STATION	0x1A
#define SRB_CONNECT_STATION	0x1B
#define SRB_DLC_MODIFY		0x1C
#define SRB_FLOW_CONTROL	0x1D
#define SRB_STATISTICS		0x1E

/* Adapter to PC system command opcodes. */
#define ARB_RECEIVED_DATA	0x81
#define ARB_TRANSMIT_DATA	0x82
#define ARB_STATUS		0x83
#define ARB_RING_CHANGE		0x84

/* Received data offsets. */
#define RCV_STATION_OFFSET	0x4
#define RCV_BUFFER_OFFSET	0x6
#define RCV_LAN_HDR_LEN_OFFSET	0x8
#define RCV_DLC_HDR_LEN_OFFSET	0x9
#define RCV_FRAME_LEN_OFFSET	0xA

/* Receive buffer offsets. */
#define NEXT_BUFFER_OFFSET	0x2
#define BUFFER_LEN_OFFSET	0x6
#define RCV_BUFF_HDR_LENGTH	0x8
/*
  Relative memory mapped area locations and offsets (BIOS, ACA, blocks)
*/

#define BIOS_LENGTH		2       /* Length in bytes of the BIOS area. */
#define BIOS_AREA_OFFSET	0x0000  /* Offset within the BIOS area.      */
#define ACA_AREA_OFFSET		0x1E00  /* Attachment Ctrl Area BIOS offset. */
#define AIP_AREA_OFFSET		0x1F00  /* Adap. Identification Area BIOS off*/
#define SUPP_INTR_OFFSET        0x1FBA  /* Supported Interrupts Id location  */
#define ACA_AREA_LENGTH		128     /* Size of the ACA area (bytes).     */
#define AIP_AREA_LENGTH		255     /* Size of the AIP area (bytes).     */
#define ASB_ADDRESS_OFFSET	0x08    /* Adapter Status Block offset.      */
#define SRB_ADDRESS_OFFSET	0x0A    /* System Request Block offset.      */
#define ARB_ADDRESS_OFFSET	0x0C    /* Adapter Request Block offset.     */
#define SSB_ADDRESS_OFFSET	0x0E    /* System Status Block offset.       */
#define ASB_AREA_LENGTH		0x12    /* Adap. Status Block length (bytes) */
#define SRB_AREA_LENGTH		0x1C    /* System Request Block length       */
#define ARB_AREA_LENGTH		0x1C    /* Adapter Request Block length      */
#define SSB_AREA_LENGTH		0x14    /* System Status Block length        */

/*
  Constants for resetting the adapter.
*/

#define RESET_LATCH_OFFSET	0x01
#define RESET_RELEASE_OFFSET	0x02
#define RESET_VALUE		0x01
#define RELEASE_VALUE		0x01

#define TOK_RESET_WAIT_TIME	0x03     /* Seconds. */
#define TOK_RETRY_OPEN_TIME	0x10	 /* Seconds. */

#define TOK_NBR_OPEN_ATTEMPTS	0x03	 /* Number of attempts. */
/*
  Constants for initialization
*/

#define IBM_16_4_ADAPTER_ID	"PICO6110990 "
#define IBM_ADAPTER_ID_LENGTH	12		/* In bytes. */

#define SW_INTR_ENABLING	0x40

#define INITIAL_SRB_SIZE	0x40

/*
   Constants used when calculating the BIOS addr.
*/

#define LOWEST_BIOS_BASE        0xC0000     /* Lowest possible setting.      */
#define GET_BIOS_BITS		0x3C        /* Use these bits to calculate.  */
#define BIOS_ADDR_ADJUST        11          /* Use only these 4 bits.        */

/*
  Constants used when polling for the adapter location.
*/
#define TOK_PRIMARY_ADDR	0x0A20      /* Primary adapter's IO-base.     */
#define TOK_SECONDARY_ADDR	0x0A24      /* Secondary adapters's ....      */

/*
  Constants for page control.
*/

#define PAGE_0	0x00
#define PAGE_1	0x40
#define PAGE_2	0x80
#define PAGE_3	0xC0

/* Constants used when opening a SAP (netid). */
#define TOK_GROUP_MEMBER	0x01
#define TOK_GROUP_SAP		0x02
#define TOK_INDIVIDUAL_SAP	0x04
#define TOK_PASS_XID_2_APP	0x08
#define TOK_MAX_SAPS		0xFF

/*

  This structure is used to supply the open parameters to the adapter via
  the SRB. It is the only command greater than the 20 bytes allocated to
  the final SRB, i.e. when this command is being setup a preliminary SRB
  is used.
  The location of the final SRB is returned in this preliminary SRB.
*/

typedef struct {
  uchar cmd;                               /* Should be DIR.OPEN.ADAPTER.     */
  uchar reserved[7];                       /* Not used (zero out).            */
  uchar option1;                           /* Config bits. See note below.    */
  uchar option2;                           /* Config bits. See note below.    */
  uchar cur_tok_addr[CTOK_NADR_LENGTH];    /* Our addr. if 0 use builtin one. */
  uchar grp_addr[4];                       /* Our grp addr. if 0 use none.    */
  uchar func_addr[4];                      /* Our func. addr. if 0 use none.  */
  uchar num_rcv_buf[2];                    /* Less than 2 => default 8 used.  */
  uchar rcv_buf_len[2];                    /* Multiple of 8,min. 96,max. 2048 */
  uchar dhb_length[2];                     /* Multiple of 8, 0 => default 600 */
  uchar num_dhb;                           /* Num. tx. buffers. 1 or 2.       */
  uchar reserved2;                         /* Not used (zero out).            */
  uchar dlc_max_sap;                       /* Max open at one time (def. 126) */
  uchar dlc_max_sta;                       /* Max nbr stations opened at once */
  uchar dlc_max_gsap;                      /* Same with group SAP's           */
  uchar dlc_max_gmem;                      /* Max nbr SAP's per any given grp */
  uchar dlc_t1_tick_1;                     /* In 40-ms. intervals. Def. = 5.  */
  uchar dlc_t2_tick_1;                     /* In 40-ms. intervals. Def. = 1.  */
  uchar dlc_ti_tick_1;                     /* In 40-ms. intervals. Def. = 25. */
  uchar dlc_t1_tick_2;                     /* In 40-ms. intervals. Def. = 25. */
  uchar dlc_t2_tick_2;                     /* In 40-ms. intervals. Def. = 10. */
  uchar dlc_ti_tick_2;                     /* In 40-ms. intervals. Def. = 125.*/
#define PRODUCT_ID_LEN		18
  uchar product_id[PRODUCT_ID_LEN];        /* Host prod.-id (pc/workstation)  */
} open_param_t;

/*
  Option bits as used in the open structure above.
*/

/* Option 1 (8 bits). */
#define OP_CONTENDER		0x01  /* Content to become a monitor.      */
#define OP_PASS_ATTN_MAC_FRAMES	0x08  /* Passes MAC attention frames.      */
#define OP_PASS_ADAP_MAC_FRAMES	0x10  /* Passes all MAC frames to hw addr. */
#define OP_DISABLE_SOFT_ERRORS	0x20  /* Enables net status changes intr's.*/
#define OP_DISABLE_HARD_ERRORS	0x40  /* Enables net staus changes intr's. */
#define WRAP_INTERFACE		0x80  /* Loop data back to user.           */
/* Option 2 (8 bits). */
#define OP_EARLY_RELEASE	0x10   /* Only used for 16 Mbits/s rates.  */
#define OP_REMOTE_PGM_LOAD	0x20  /* Prevents adap. fm being a monitor.*/
#define OP_PASS_BEACON_FRAMES	0x80  /* Passes MAC Beacon frames.         */

/* Other open constants for open structure above.  */

#define ITOK_ISA_DEFAULT_OPEN_OPTION1	(OP_DISABLE_SOFT_ERRORS + \
                                         OP_DISABLE_HARD_ERRORS) 
#define ITOK_ISA_DEFAULT_OPEN_OPTION2	0
/* 
   Adjust the following constants for possible performance gains. 
   Never have more than 2 tx-buffers, the adapter will not guarantee
   proper operation if more than 2 is specified.
*/

/* Start of performance tunable parameters. */
#define OPEN_RCV_BUF_COUNT	8     /* Number of receive buffers.           */
#define OPEN_RCV_BUF_LENGTH   1024    /* Size of each receive buffer (bytes). */
#define OPEN_XMIT_BUF_COUNT	2     /* Number of transmit buffers( max. 2)  */
#define OPEN_XMIT_BUF_LENGTH 2048     /* Size of each transmit buffer (bytes) */
/* End of performance tunable parameters. */

#define OPEN_MAX_NBR_SAPS      126    /* Max nbr. simultanously opened.       */
#define OPEN_CONFIG_NBR_SAPS    20    /* What we allow at config time.        */
#define OPEN_CONFIG_NBR_STATIONS 0    /* No stations allowed.                 */
#define OPEN_CONFIG_NBR_GRP_SAPS 0    /* No group SAP's supported.            */
#define OPEN_CONFIG_MAX_GMEM     0    /* Max. nbr. SAP's per group.           */

#define OPEN_T1_40MS_TICK_1     5     /* DLC T1 (1..5) timer tick.            */
#define OPEN_T2_40MS_TICK_1     1     /* DLC T2 (1..5) time between ticks.    */
#define OPEN_Ti_40MS_TICK_1    25     /* DLC Ti (1..5) time between ticks.    */
#define OPEN_T1_40MS_TICK_2    25     /* DLC T1 (6..10) time between ticks.   */
#define OPEN_T2_40MS_TICK_2    10     /* DLC T2 (6..10) time between ticks.   */
#define OPEN_Ti_40MS_TICK_2   125     /* DLC Ti (6..10) time between ticks.   */

/* _________________________________________________________________________  */#define SRB_RETCODE_OFFSET	2     /* In bytes.                            */
#define SRB_STATION_ID_OFFSET	4     /* In bytes.                            */

#define SRB_INITIATED		0xFE  /* Goes into the retcode field.         */
#define SRB_SUCCESS		0x00  /*       ......                         */
#define SRB_IN_PROGRESS		0xFF
#define ASB_AVAILABLE		0xFF

/* Constants for use by the error-logger routine (AIX error log).
   Each major grouping has up to 0x10 error-codes.                            */

#define TOK_MEM_ERROR		0x30
#define TOK_TIMER_ALLOC_ERR	(TOK_MEM_ERROR + 0x01)

#define TOK_RESTART_ADAPTER	0x40

#define TOK_ERROR_LOG		0x50

#define TOK_ASB_ERROR		0x60
#define TOK_INVALID_STATION_ID	(TOK_ASB_ERROR + 0x00)
#define TOK_UNRECOGNIZED_CMD	(TOK_ASB_ERROR + 0x01)
#define TOK_UNRECOGNIZED_CORR	(TOK_ASB_ERROR + 0x02)

#define TOK_OPEN_FAILED		0x70
#define TOK_BADPARAM		(TOK_OPEN_FAILED + 0x5)

#define TOK_RING_CHANGE		0x80
#define TOK_NOT_SEEN0		(TOK_RING_CHANGE + 0x0)
#define TOK_NOT_SEEN1		(TOK_RING_CHANGE + 0x1)
#define TOK_NOT_SEEN2		(TOK_RING_CHANGE + 0x2)
#define TOK_NOT_SEEN3		(TOK_RING_CHANGE + 0x3)
#define TOK_NOT_SEEN4		(TOK_RING_CHANGE + 0x4)
#define TOK_RING_RECOVERY	(TOK_RING_CHANGE + 0x5)
#define TOK_SINGLE_STATION	(TOK_RING_CHANGE + 0x6)
#define TOK_ERR_CNT_OVERFLOW	(TOK_RING_CHANGE + 0x7)
#define TOK_REMOVE_RECEIVED	(TOK_RING_CHANGE + 0x8)
#define TOK_NOT_SEEN5		(TOK_RING_CHANGE + 0x9)
#define TOK_AUTO_REMOVAL	(TOK_RING_CHANGE + 0xa)
#define TOK_LOBE_WIRE_FAULT	(TOK_RING_CHANGE + 0xb)
#define TOK_TX_BEACON		(TOK_RING_CHANGE + 0xc)
#define TOK_SOFT_ERROR		(TOK_RING_CHANGE + 0xd)
#define TOK_HARD_ERROR		(TOK_RING_CHANGE + 0xe)
#define TOK_SIGNAL_LOSS		(TOK_RING_CHANGE + 0xf)

#define TOK_XMIT_TIMEOUT_ERROR	0x90

/* DIR.READ.LOG command structure, also passed to the AIX error-log. */
typedef struct {
  uchar line_errors;
  uchar internal_errors;
  uchar burst_errors;
  uchar AC_errors;
  uchar abort_delimiters;
  uchar reserved1;
  uchar lost_frames;
  uchar receive_congestion_count;
  uchar frame_copied_errors;
  uchar frequency_errors;
  uchar token_errors;
  uchar reserved2;
  uchar reserved3;
  uchar reserved4;
} tok_error_log;

typedef struct {
   ushort  adap_phys_addr[2];         /* Adapter Physical Address */
   uchar   upstream_node_addr[6];     /* Upstream Node Address */
   ushort  upstream_phys_addr[2];     /* Upstream Physical Addr */
   ushort  last_poll_addr[3];         /* Last Poll Address */
   ushort  reserved1;                 /* Authorized Environment */
   ushort  tx_access_prior;           /* Transmit Access Priority */
   ushort  src_class_author;          /* Source Class Authorization */
   ushort  last_atten_code;           /* Last Attention Code */
   ushort  last_src_addr[3];          /* Last Source Address */
   ushort  last_bcon_type;            /* Last Beacon Type */
   ushort  last_maj_vector;           /* Last Major Vector */
   ushort  ring_status;               /* Ring Status */
   ushort  sft_err_time_val;          /* Soft Error Timer Value */
   ushort  front_end_time_val;        /* Front End Error Counter */
   ushort  local_ring;                /* Ring number */
   ushort  monitor_err_code;          /* Monitor Error Code */
   ushort  bcon_tx_type;              /* Beacon Transmit Type */
   ushort  bcon_rcv_type;             /* Beacon Receive Type */
   ushort  frame_corr_save;           /* Frame Correlator Save */
   ushort  bcon_station_naun[3];      /* Beaconing Station NAUN */
   ushort  reserved2[2];              /* Reserved */
   ushort  bcon_station_phys_addr[2]; /* Beaconing Station Physical Address */
} tok_ring_info_t;

#define TOK_SRB_ERR_LOG_OFFSET		0x06
#endif /* If ! _H_ITOK_HW */
