/* @(#)63       1.1.1.2  src/rspc/kernext/isa/ient/ient_hw.h, isaent, rspc41J, 9517A_all 4/21/95 13:05:36 */
/*
 * COMPONENT_NAME: isaent - IBM ISA Ethernet Device Driver
 *                                                                          
 * FUNCTIONS: none
 *                                                                        
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */

/* IO-section layout :

             IO-base X'00'   Control Reg. 1   (Write only)
             IO-base X'01'   AT Detect Reg.   (Read only)
             IO-base X'05'   Control Reg. 2   (Write only)
             IO-base X'08'   
             IO-base X'09'  
             IO-base X'0A'   Config Reg. A    (Read/Write)
             IO-base X'0B'   Config Reg. B    (Read/Write)
             IO-base X'0C'   Config Reg. C    (Read only)

             IO-base X'10'   Command Reg.     (Read/Write)    Page 0,1 or 2.

*/

#ifndef _H_IENT_HW
#define _H_IENT_HW

#define NUM_XMIT_BUFS		2
#define XMIT_BUF1_OFFSET	0x0	 /* we have 2 transmit buffers of 2k each */
#define XMIT_BUF2_OFFSET	0x800   /* 2K, start at 2048 */

#define IEN_ISA_INIT_BASE     0x278     /* Must write here once to setup I/O. */

/* Constants used when configuring the config registers. */

#define IEN_ISA_IO_ADDR1      0x240     /* Possible IO-base addresses,        */
#define IEN_ISA_IO_ADDR2      0x280     /* these 7 possible IO-addresses gets */
#define IEN_ISA_IO_ADDR3      0x2C0     /* translated into the below listed   */
#define IEN_ISA_IO_ADDR4      0x300     /* bit values.                        */
#define IEN_ISA_IO_ADDR5      0x320    
#define IEN_ISA_IO_ADDR6      0x340
#define IEN_ISA_IO_ADDR7      0x360

#define IEN_ISA_IO_BITS1      0x2       /* These 'bit-patterns' are the ones  */
#define IEN_ISA_IO_BITS2      0x3       /* that the adapter needs to see when */
#define IEN_ISA_IO_BITS3      0x4       /* configuring config register A.     */
#define IEN_ISA_IO_BITS4      0x0
#define IEN_ISA_IO_BITS5      0x5
#define IEN_ISA_IO_BITS6      0x6
#define IEN_ISA_IO_BITS7      0x7

#define IEN_ISA_INTR_LEVEL_1  0x03      /* Possible interrupt levels, these   */
#define IEN_ISA_INTR_LEVEL_2  0x04      /* 7 possible levels gets translated  */
#define IEN_ISA_INTR_LEVEL_3  0x05      /* into the below listed bit values.  */
#define IEN_ISA_INTR_LEVEL_4  0x07
#define IEN_ISA_INTR_LEVEL_5  0x09
#define IEN_ISA_INTR_LEVEL_6  0x0A
#define IEN_ISA_INTR_LEVEL_7  0x0B

#define IEN_ISA_INTR_BITS1    0x10      /* These 'bit-patterns' are the ones  */
#define IEN_ISA_INTR_BITS2    0x18      /* that the adapter needs to see when */
#define IEN_ISA_INTR_BITS3    0x20      /* configuring register A (bits 3..5).*/
#define IEN_ISA_INTR_BITS4    0x28
#define IEN_ISA_INTR_BITS5    0x08
#define IEN_ISA_INTR_BITS6    0x30
#define IEN_ISA_INTR_BITS7    0x38

#define IEN_ISA_MEM_MODE      0x80      /* Used when configuring register A.  */
#define IEN_ISA_16BIT_WIDE    0xC0      /* Configure for 16Kbytes & 16 bit.   */

#define IEN_ISA_PAGE0         0x00      /* Used when setting the page #.      */
#define IEN_ISA_PAGE1         0x40
#define IEN_ISA_PAGE2         0x80

#define IEN_ISA_MEM_ENABLE    0x40      /* Used at restart time (mem_ctrl_1). */
#define IEN_ISA_RESET_NIC     0x80      /* Used at restart time (mem_ctrl_1). */
#define IEN_ISA_LOW_MEM_MASK  0x3F      /* Used when setting up shared mem.   */
#define IEN_ISA_HIGH_MEM_MASK 0x1F      /* Used when setting up shared mem.   */

/* ROM offsets. */
#define IEN_ISA_ROM_OFFSET    0x08      /* In bytes, relative to io-base.     */
#define IEN_ISA_TYPE_OFFSET   (IEN_ISA_ROM_OFFSET + 6) /* Rel. to io-base.  */
#define IEN_ISA_CHKSUM_OFFSET (IEN_ISA_ROM_OFFSET + 7) /* Rel. to io-base.  */

/* NIC Register offsets in page 0. */
#define PAGE0_COMMAND         0x10      /* Command register                RW */
#define PAGE0_CLDA0           0x11      /* Current local DMA address 0     RO */
#define PAGE0_PSTART      PAGE0_CLDA0   /* Page Start register             WO */
#define PAGE0_CLDA1           0x12      /* Current local DMA address 1     RO */
#define PAGE0_PSTOP       PAGE0_CLDA1   /* Page Stop register              WO */
#define PAGE0_BNRY            0x13      /* Boundary Pointer                RW */
#define PAGE0_TSR             0x14      /* Transmit Status register        RO */
#define PAGE0_TPSR        PAGE0_TSR     /* Transmit Page Start Address     WO */
#define PAGE0_NCR             0x15      /* Number of collisions register   RO */
#define PAGE0_TBCR0       PAGE0_NCR     /* Transmit Byte count register 0  WO */
#define PAGE0_FIFO            0x16      /* FIFO buffer register            RO */
#define PAGE0_TBCR1       PAGE0_FIFO    /* Transmit Byte count register 1  WO */
#define PAGE0_ISR             0x17      /* Interrupt status register       RW */
#define PAGE0_CRDA0           0x18      /* Current Remote DMA address 0    RO */
#define PAGE0_RSAR0       PAGE0_CRDA0   /* Remote Start Address register 0 WO */
#define PAGE0_CRDA1           0x19      /* Current Remote DMA address 1    WO */
#define PAGE0_RSAR1       PAGE0_CRDA1   /* Remote Start Address register 1 WO */
#define PAGE0_RBCR0           0x1A      /* Remote byte count register 0    WO */
#define PAGE0_RBCR1           0x1B      /* Remote byte count register 1    WO */
#define PAGE0_RSR             0x1C      /* Receive status register         RO */
#define PAGE0_RCR         PAGE0_RSR     /* Receive configuration register  WO */
#define PAGE0_CNTR0           0x1D      /* Tally Counter 0 (Align. errors) RO */
#define PAGE0_TCR         PAGE0_CNTR0   /* Transmit configuration register WO */
#define PAGE0_CNTR1           0x1E      /* Tally Counter 1 (CRC errors)    RO */
#define PAGE0_DCR         PAGE0_CNTR1   /* Data configuration register     WO */
#define PAGE0_CNTR2           0x1F      /* Tally Counter 2 (Missed packets)RO */
#define PAGE0_IMR         PAGE0_CNTR2   /* Interrupt masks                 WO */

/* NIC Register offsets in page 1. */
#define PAGE1_COMMAND         0x10      /* Command register                RW */
#define PAGE1_PAR0            0x11      /* Physical address register 0     RW */
#define PAGE1_PAR1            0x12      /* Physical address register 1     RW */
#define PAGE1_PAR2            0x13      /* Physical address register 2     RW */
#define PAGE1_PAR3            0x14      /* Physical address register 3     RW */
#define PAGE1_PAR4            0x15      /* Physical address register 4     RW */
#define PAGE1_PAR5            0x16      /* Physical address register 5     RW */
#define PAGE1_CURR            0x17      /* Current page (receive unit)     RW */
#define PAGE1_MAR0            0x18      /* Multicast address register 0    RW */
#define PAGE1_MAR1            0x19      /* Multicast address register 1    RW */
#define PAGE1_MAR2            0x1A      /* Multicast address register 2    RW */
#define PAGE1_MAR3            0x1B      /* Multicast address register 3    RW */
#define PAGE1_MAR4            0x1C      /* Multicast address register 4    RW */
#define PAGE1_MAR5            0x1D      /* Multicast address register 5    RW */
#define PAGE1_MAR6            0x1E      /* Multicast address register 6    RW */
#define PAGE1_MAR7            0x1F      /* Multicast address register 7    RW */

/* NIC Register offsets in page 2. */
/* Page 2 registers are ONLY used for diagnostic purposes. */
#define PAGE2_COMMAND         0x10      /* Command register                RW */
#define PAGE2_PSTART          0x11      /* Page Start Register             RO */
#define PAGE2_CLDA0     PAGE2_PSTART    /* Current Local DMA Address 0     WO */
#define PAGE2_PSTOP           0x12      /* Page Stop Register              RW */
#define PAGE2_CLDA1     PAGE2_PSTOP     /* Current Local DMA Address 1     WO */
#define PAGE2_RNPP            0x13      /* Remote Next Packet Pointer      RW */
#define PAGE2_TPSR            0x14      /* Transmit Page Start Address     RO */
#define PAGE2_LNPP            0x15      /* Local Next Packet Pointer       RW */
#define PAGE2_ACUP            0x16      /* Address counter (upper)         RW */
#define PAGE2_ACLOW           0x17      /* Address counter (lower)         RW */
#define PAGE2_RCR             0x1C      /* Receive Configuration register  RO */
#define PAGE2_TCR             0x1D      /* Transmit Configuration register RO */
#define PAGE2_DCR             0x1E      /* Data Configuration register     RO */
#define PAGE2_IMR             0x1F      /* Interrupt Mask register         RO */

/* Regular register offsets. */
#define IEN_ISA_CTRL_REG_1_OFFSET       0x00
#define IEN_ISA_AT_DETECT_REG_OFFSET    0x01
#define IEN_ISA_CTRL_REG_2_OFFSET       0x05
#define IEN_ISA_ETHERNET_ADDR_OFFSET    0x08
#define IEN_ISA_INITIAL_REG_A_OFFSET    0x0A
#define IEN_ISA_INITIAL_REG_B_OFFSET    0x0B
#define IEN_ISA_CFG_REG_A_OFFSET        0x1A
#define IEN_ISA_CFG_REG_B_OFFSET        0x1B
#define IEN_ISA_CFG_REG_C_OFFSET        0x1C

/* Shared Memory Control Register 1. (write only) */

typedef struct
{
  uint  NIC_reset    : 1,    /* Resets NIC core of adapter.             */
        mem_enable   : 1,    /* Enables ext. mem. acceses.              */
        mem_addr_low : 6;    /* Lower address bits of shared mem. addr. */
} mem_ctl_reg1;

/* Shared Memory Control Register 2. (write only) */

typedef struct
{
  uint  mem_width_host : 1,  /* Enables host 16 bit wide accesses.       */
        mem_width_adap : 1,  /* Enables adapter 16 bit wide accesses.    */
        unused         : 1,  /* Unused.                                  */
        mem_addr_high  : 5;  /* Higher address bits of shared mem. addr. */
} mem_ctl_reg2;

/* Shared Memory AT Detect Register (read only). */

typedef struct
{
  uint   unused    : 7,      /* Not much here.           */
         at_detect : 1;      /* 16 bit slot when high.   */
} at_detect_reg;

/* Mode Configuration Register A (read/write) */

typedef struct
{
  uint  mem_model : 1,       /* I/O or shared memory mode.                  */
        fast_read : 1,       /* Only for IO-mode (we don't use IO mode)     */
        irq       : 3,       /* Interrupt level (3,4,5,7,9,10 or 11)        */
        io_base   : 3;       /* The location of the IO base (0x300, ...)    */
} cfg_reg_A;

/* Mode Configuration Register B (read/write) */

typedef struct
{
   uint phys_con   :2,       /* Phys. Layer: 0=10BASE-T,1=10BASE2,2=10BASE5  */
        good_link  :1,       /* 1 = the link test pulse generation disabled  */
        io16_ctrl  :1,       /* 1 = generates IO 16 after IORD/WR goes active*/
        chrdy      :1,       /* 0 = generates CHRDY after the cmd strobe.    */
        bus_err    :1,       /* 1 = adap. inserts wait states and host aborts*/
        prom_write :1,       /* 0 = no PROM write cycles are generated.      */
        ee_load    :1;       /* 1 = enables EEPROM load algorithm.           */
} cfg_reg_B;

/* Mode Configuration Register C (read only) */

typedef struct
{
  uint  soft_enable : 1,      /* 0 = allowed to program config. regs. A & B. */
        clock_sel   : 1,      /* 0 = NIC core clocked at 20MHZ (bit ignored) */
        intr_mode   : 1,      /* 1 = coded output interrupt mode is used.    */
        compatible  : 1,      /* 0 = compatible with Ethercard+ & NE2000.    */
        prom_addr   : 4;      /* Boot prom select. 0x0C000,0x0C400,..,0x0DC00*/
} cfg_reg_C;

/* Data Configuration register (write only) */

typedef union
{
    struct
    {
        uint  not_used       : 1,    /* Nothing here.                */
              fifo_thres     : 2,    /* FIFO threshold select.       */
              auto_init_dma  : 1,    /* Auto init remote DMA.        */
              loopback_sel   : 1,    /* Loopback select.             */
              long_addr_sel  : 1,    /* Long address select.         */
              byte_order_sel : 1,    /* Byte order select.           */
              word_wide_xfer : 1;    /* Allow word wide DMA xfers.   */
  } cfg_bits;
    
  unsigned char value;               /* Using when setting all bits. */

} data_cfg_reg;

/* data_cfg_reg.value */

#define IEN_ISA_DCR_WTS     0x01     /* Word transfer select.        */
#define IEN_ISA_DCR_BOS     0x02     /* Byte order select.           */
#define IEN_ISA_DCR_LAS     0x04     /* Long address select.         */
#define IEN_ISA_DCR_BMS     0x08     /* Burst mode select.           */
#define IEN_ISA_DCR_AR      0x10     /* Autoinitialize Remote.       */
#define IEN_ISA_DCR_FT0     0x20     /* Fifo threshold select.       */
#define IEN_ISA_DCR_FT1     0x40     /* Fifo threshold select.       */
#define IEN_ISA_DCR_RES     0x80     /* Reserved.                    */

/* Transmit Configuration register (write only) */

typedef union
{
    struct
    {
        uint   reserved        : 3,  /* Nothing here.                        */
               coll_algo       : 1,  /* Modify the coll. algo. to node prio. */
               auto_tx_disable : 1,  /* Allow other station to kill xmitter. */
               loopback_ctl    : 2,  /* Encoded Loopback control.            */
               inhibit_crc     : 1;  /* Inhibit CRC in loopback mode.        */
    } cfg_bits;

    unsigned char value;             /* Used when setting all bits.          */

} tx_conf_reg;

#define IEN_ISA_INHIBIT_CRC        0x01
#define IEN_ISA_INTERNAL_LOOPBACK  0x02
#define IEN_ISA_ENDEC_LOOPBACK     0x04
#define IEN_ISA_EXTERNAL_LOOPBACK  0x06
#define IEN_ISA_AUTO_XMIT          0x08
#define IEN_ISA_COLL_OFFSET_ENABLE 0x10


/* Receive Configuration register (write only) */

typedef union
{
    struct
    {
        uint   reserved          : 3,  /* Nothing here.                   */
               promiscuous_mode  : 1,  /* Receive all physical addresses. */
               accept_mcasts     : 1,  /* Accept Multicast packets.       */
               accept_bcasts     : 1,  /* Accept broadcasts packets.      */
               rcv_small_packets : 1,  /* Accept packets < 64 bytes.      */
               rcv_err_packets   : 1;  /* Accept data with errors         */
    } cfg_bits;
    
    unsigned char value;               /* Used when setting all bits.     */

} rcv_conf_reg;

#define IEN_ISA_RCV_ERRORS           0x01
#define IEN_ISA_RCV_WRONG_LEN        0x02
#define IEN_ISA_ACCEPT_BCASTS        0x04
#define IEN_ISA_ACCEPT_MCASTS        0x08
#define IEN_ISA_PROMIS_MODE          0x10
#define CRC_POLYNOMIAL         0x04C11DB6  /* CRC used in Multicast Addresses */

/* Interrupt Mask Register (write only) */

union intr_mask_reg
{
    struct
    {
        uint   not_used            : 1, /* Nothing here.                   */
               enable_DMA_complete : 1, /* Allow DMA completion interrupts.*/
               enable_cnt_overflow : 1, /* Allow tally cnt overflow intr's.*/
               enable_rx_overrun   : 1, /* Allow rx overrun interrupts.    */
               enable_tx_err       : 1, /* Allow transmit error interrupts.*/
               enable_rx_err       : 1, /* Allow receive error interrupts. */
               enable_tx           : 1, /* Allow transmission interrupts.  */
               enable_rx           : 1; /* Allow receive interrupts.       */
  } mask_bits;

  unsigned char value;                  /* Used when setting all bits.     */
};

#define IEN_ISA_ENB_RX_INTR        0x01
#define IEN_ISA_ENB_TX_INTR        0x02
#define IEN_ISA_ENB_RX_ERR_INTR    0x04
#define IEN_ISA_ENB_TX_ERR_INTR    0x08
#define IEN_ISA_ENB_RX_OVR_ERR     0x10
#define IEN_ISA_ENB_CNT_OVFL       0x20
#define IEN_ISA_ENB_DMA_COMP       0x40
#define IEN_ISA_ENB_ALL_INTR       0x7F

/* Command Register (read/write) */

typedef union
{
    struct
    {
        uint  set_page : 2,     /* Page select. Page 0 = 00, 1 = 01, 2 = 10. */
              do_dma   : 3,     /* Remote DMA cmd. Not used here.            */
              do_tx    : 1,     /* Transmit a packet (after TBCR1,0&TPSR set)*/
              start    : 1,     /* Start from a reset or cold boot.          */
              stop     : 1;     /* Software reset. Enters reset state.       */
  } cmd_bits;

    unsigned char value;        /* For setting all bits at once.             */

} cmd_reg;

#define IEN_ISA_PAGE_0            0
#define IEN_ISA_PAGE_1            1
#define IEN_ISA_REMOTE_DMA_RD     1
#define IEN_ISA_REMOTE_DMA_WR     2
#define IEN_ISA_STOP_REMOTE_DMA   4
#define IEN_ISA_STOP_NIC       0x01     /* Stop the NIC.            */
#define IEN_ISA_START_NIC      0x02     /* Start the NIC.           */
#define IEN_ISA_START_TX       0x04     /* Transmit packet.         */
#define IEN_ISA_DMA_READ       0x08     /* Remote DMA read.         */
#define IEN_ISA_DMA_WRITE      0x10     /* Remote DMA write.        */
#define IEN_ISA_SEND           0x18     /* Send packet command.     */
#define IEN_ISA_NO_DMA         0x20     /* Abort/stop DMA.          */
#define IEN_ISA_SET_PAGE_0     0x00     /* Set page 0.              */
#define IEN_ISA_SET_PAGE_1     0x40     /* Set page 1.              */
#define IEN_ISA_SET_PAGE_2     0x80     /* Set page 2.              */

/* Interrupt Status Register (read/write) */

typedef union
{
    struct
    {
        uint  reset_stat   : 1,   /* Set when in reset or receive overflow.*/
              DMA_complete : 1,   /* Remote DMA complete.                  */
              cnt_overflow : 1,   /* Counter overflow from network tally.  */
              rx_overrun   : 1,   /* Receive buffers exhausted.            */
              tx_err       : 1,   /* Transmit error                        */
              rx_err       : 1,   /* Receive error                         */
              data_tx      : 1,   /* Packet transmitted with no errors.    */
              data_rx      : 1;   /* Packet received with no errors.       */
  } stat_bits;
    
  unsigned char  value;

} intr_stat_reg;

#define IEN_ISA_RECEIVE_MASK    0x15  /* Did we get a receive interrupt.   */
#define IEN_ISA_XMIT_MASK       0x0A  /* Did we get a transmit interrupt.  */
#define IEN_ISA_ERROR_MASK      0x70  /* Did we get a misc. interrupt.     */
#define IEN_ISA_RESET_PENDING   0x80  /* Is the NIC being reset.           */

/* Transmit Status register (read only) */

typedef union
{
    struct
    {
        uint   out_window_coll : 1,   /* Collision after a slot time.        */
               cd_heartbeat    : 1,   /* Failure to tx coll signal.          */
               fifo_underrun   : 1,   /* Tx aborted:no timely bus accesess.  */
               cr_sense_lost   : 1,   /* Carrier sense lost, tx not aborted. */
               tx_aborted      : 1,   /* Tx failed due to max. collisions.   */
               tx_coll         : 1,   /* At least 1 collision when xmitting. */
               reserved        : 1,   /* Nothing here.                       */
               tx_ok           : 1;   /* Packet xmitted without errors.      */
  } stat_bits;
    
  unsigned char  value;

} tx_stat_reg;

/* Receive Status register (read only) */

typedef union
{
    struct
    {
        uint  deferring         : 1,   /* Internal Carrier Sense/Collision on */
              receiver_disabled : 1,   /* In monitor mode (no receiving).     */
              multicast_addr    : 1,   /* Multicast/broadcast addr. received. */
              missed_packet     : 1,   /* No receive buffers or in monitor md.*/
              fifo_overrun      : 1,   /* Reception of the packet aborted.    */
              frame_align_err   : 1,   /* Packet not on a byte boundary.      */
              crc_error         : 1,   /* Packet received with a CRC error.   */
              packet_ok         : 1;   /* Packet received without errors.     */
  } stat_bits;
    
  unsigned char  value;

} rcv_stat_reg;

#define IEN_ISA_RCV_START	0x10
#define IEN_ISA_PAGE_SIZE       256    /* Nbr of bytes. */

/* IBM ISA Ethernet Adapter Statistics Counters offsets */

#define RV_CRC             0    /* receive CRC error                 */
#define RV_ALIGN         0x4    /* receive alignment error           */
#define RV_OVERRUN       0x8    /* receive overrun error             */
#define RV_SHORT         0xC    /* receive short frame error         */
#define RV_LONG         0x10    /* receive long frame error          */
#define RV_RSC          0x14    /* receive resource error            */
#define RV_DISCARD      0x18    /* receive - packets discarded       */
#define TX_MAX_COLL     0x1C    /* transmit maximum collision error  */
#define TX_NO_CS        0x20    /* transmit no carrier sense error   */
#define TX_UNDERRUN     0x24    /* transmit underrun error           */
#define TX_CLS          0x28    /* transmit no clear-to-send error   */
#define TX_TMOUT        0x2C    /* transmit timeout error            */
#define ADPT_RV_EL      0x30    /* times EL bit seen in receive list */        
#define ADPT_RSC_586    0x34    /* no resource error from 82586      */
#define ADPT_RVPKTS_OK  0x38    /* good packets received             */
#define ADPT_RVPKTS_UP  0x3C    /* received packets uploaded         */
#define ADPT_RV_START   0x40    /* times of "start reception" command */
#define ADPT_DMA_TMOUT  0x44    /* bus master trasnfer timeout       */
#define ADPT_3COM_STAT  0x48    /* five 16 bit 3com stat variable    */
#define ADPT_TXPKTS_DN  0x56    /* packets downloaded                */
#define ADPT_TX_ERR     0x5E    /* transmit errors                   */
#define ADPT_TX_RETRY   0x6E    /* number of transmit retries        */
#define ADPT_TX_FAILED  0x72    /* number of transmit failed         */
#define TX_ONE_COLL     0x7A    /* number of packets with single collision   */
#define TX_MULTI_COLL   0x7E    /* number of packets with multiple collision */
#define LAST_STATS      0x84    /* the last stats counter on the adapter     */

/* Receive buffer header, all buffers are 256 bytes long. */
typedef struct
{
  rcv_stat_reg rcv_stat;    /* 1 Byte which is equal to the rcv stat register.*/
  uchar        next_buffer; /* If packet spans more than one buffer.          */
  ushort       bytes;       /* Length of the entire packet.                   */
} rcv_hdr_t;

#endif /* _H_IENT_HW */
