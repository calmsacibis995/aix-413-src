/* @(#)28	1.1  src/rspc/usr/lib/boot/softros/roslib/diskette.h, rspc_softros, rspc411, 9432A411a 8/5/94 16:39:10  */
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#ifdef CC_CARD
/*------------------------*/
/* Definition for CC card */
/*------------------------*/
#define ARB_CTRL_POINT     0x90
#define SYS_BD_SET_UP      0x94
#define NIO_POS_REG0       0x100
#else
#  define SIO_PAC_REG        *(unsigned char *)(0x80800841)
#endif


#define READ                 0
#define WRITE                1
#define DSKT_INTERRUPT_LEVEL 6    


/*------------------------------*/
/* FDC Register Addr Definition */
/*------------------------------*/
#define SRA   0x3f0               /* Status Register A              */
#define SRB   0x3f1               /* Status Register B              */
#define DOR   0x3f2               /* Digital Output Register        */
#define TDR   0x3f3               /* Tape Drive Register            */
#define MSR   0x3f4               /* Main Status Register           */
#define DSR   0x3f4               /* Data Rate Select Register      */
#define FIFO  0x3f5               /* Data Register (FIFO)           */
#define _NA   0x3f6               /* Not Assined                    */
#define DIR   0x3f7               /* Digital Input Register         */
#define CCR   0x3f7               /* Configuration Control Register */

#define DISK_CHANGED_BIT  0x80


/*------------------------*/
/* FDC Command Definition */
/*------------------------*/
#define CONFIGURE_CMD       (unsigned char)0x13
#define SPECIFY_CMD         (unsigned char)0x03
#define RECAL_CMD           (unsigned char)0x07
#define SENSE_INTR_CMD      (unsigned char)0x08
#define SEEK_CMD            (unsigned char)0x0f
#define READ_ID_CMD         (unsigned char)0x4a
#define READ_DATA_CMD       (unsigned char)0xc6 /* MT On, MFM On, SK Off */
#define WRITE_DATA_CMD      (unsigned char)0xc5 /* MT On, MFM On         */
#define READ_A_TRACK_CMD    (unsigned char)0x42 /* MFM Bit On            */
#define PERPENDICULAR_CMD   (unsigned char)0x12
#define LOCK_CMD            (unsigned char)0x14 /* LOCK Bit Off          */
#define VERSION_CMD         (unsigned char)0x10
#define VERIFY_CMD          (unsigned char)0xd6

#define BYTES_PER_SECT      512
#define DMA                 0x00 /* DMA Mode ON: Used in SPECYFY CMD */
#define READ_DATA_CMD_BYTES_PER_SECT (unsigned char)2   /* 512 bytes/sect: Used in READ     */
                                         /* DATA CMD                         */

/*----------------------*/
/* DMA Setup Definition */
/*----------------------*/
#define DMA_CHAN              2

#ifdef CC_CARD
/*------*/
/* 8237 */
/*------*/
#define DMA_MODE              0x0d

#define FUNC_PORT             0x18
#define EXEC_PORT             0x1a

#define IO_ADDR_CMD           0x00
#define BASE_MEM_ADDR_CMD     0x20
#define MEM_ADDR_CMD          0x30
#define BASE_XFER_COUNT_CMD   0x40
#define XFER_COUNT_CMD        0x50
#define RD_DMA_STATUS_CMD     0x60
#define DMA_MODE_CMD          0x70
#define DMA_ARB_LEVEL_CMD     0x80
#define SET_DMA_MASK_CMD      0x90
#define RESET_DMA_MASK_CMD    0xa0
#define SET_SW_REQ_CMD        0xb0
#define RESET_SW_REQ_CMD      0xc0
#define RESET_DMA_CTLR_CMD    0xd0
#else
/*---------*/
/* 82378IB */
/*---------*/
#define DCOM_REG              0x0008
#define DCM_REG               0x000b
#define DCEM_REG              0x040b
#define DR_REG                0x0009
#define MASK_REG              0x000a
#define MASK_REG_2            0x00d4            
#define MASK_ALL_REG          0x000f
#define DS_REG                0x0008
#define BASE_ADDR_REG         (0x0000 + DMA_CHAN * 2)
#define BYTE_COUNT_REG        (0x0001 + DMA_CHAN * 2)
#define LOW_PAGE_REG_2        0x0081
#define HIGH_PAGE_REG_2       0x0481
#define CLR_BYTE_POINTER_REG  0x000c
#define DMC_REG               0x000d
#define CLR_MASK_REG          0x000e

#define SET_DMA_MASK_CMD      (unsigned char)(0x04 + DMA_CHAN)
#define CLR_DMA_MASK_CMD      (unsigned char)(0x00 + DMA_CHAN)
#define SET_MODE_CMD_R        (unsigned char)(0x54 + DMA_CHAN)
#define SET_MODE_CMD_W        (unsigned char)(0x58 + DMA_CHAN)
#define SET_EXT_MODE_CMD      (unsigned char)(0x20 + DMA_CHAN)
#endif


/*--------------*/
/* Result Bytes */
/*--------------*/
#define FDC_RESULT_0           (cb_ptr->g_fdc_result[0])
#define FDC_RESULT_1           (cb_ptr->g_fdc_result[1])
#define FDC_RESULT_2           (cb_ptr->g_fdc_result[2])
#define FDC_RESULT_3           (cb_ptr->g_fdc_result[3])
#define FDC_RESULT_4           (cb_ptr->g_fdc_result[4])
#define FDC_RESULT_5           (cb_ptr->g_fdc_result[5])
#define FDC_RESULT_6           (cb_ptr->g_fdc_result[6])


/*--------------------*/
/* Used by detmedia() */
/*--------------------*/
#define NUM_MED_DR_COMB        4        /* A360_IN_1p2 Not Supported */
#define A_2p8_IN_2p8           1
#define A_1p4_IN_2p8           2
#define A_720_IN_2p8           3
#define A_1p2_IN_1p2           4
#define A_360_IN_1p2           5
#define UNKNOWN               -1


/*--------------*/
/* Return Codes */
/*--------------*/
#define OK                     0
#define NO                    -1
#define TRUE                   1        /* TRUE = 1        */
#define NO_INT                 2
#define GET_RESULTS            3
#define RDY_TO_WRITE           4
#define WRONG_INTERRUPT        5
#define READ_ID_FAILED         6
#define DONT_KNOW_MEDIA        7
#define SPECIFY_FAILED         8
#define READ_CMD_FAILED        9
#define SEEK_FAILED            10
#define RECAL_FAILED           11
#define SET_MODE_FAILED        12
#define INT_OCCURRED           13
#define INT_TIME_OUT           14
#define WRITE_CMD_FAILED       15

#define YES                    OK
#define BAD                    NO
#define NO_DRIVE               RECAL_FAILED

/*-------------------*/
/* Timer Definitions */
/*-------------------*/
#define RTC_1_u_SEC    1000
#define RTC_1_m_SEC    RTC_1_u_SEC * 1000
#define RTC_1_SEC      RTC_1_m_SEC * 1000


/*------------------------*/
/* Diskette Control Block */
/*------------------------*/
#define NUM_OF_DRIVES 2

typedef struct dskt_cb {
    
    struct fdc_drive_parms {
	int g_step_rate;
	int g_head_unload_time;
	int g_head_load_time;
	int g_gap_length;
	int g_max_cyl;
	int g_max_sect;
	int g_max_head;
	int g_data_rate;
	int g_media_drive_flag;
    } fdc[NUM_OF_DRIVES];
    
    int g_index;
    int g_msr_data;
    int g_int_flag;
    int g_int_expected;
    int g_cyl[NUM_OF_DRIVES];
    int g_motor_on[NUM_OF_DRIVES];
    
    unsigned char g_fdc_result[8];
    
} DSKT_CB, *CB_PTR;
