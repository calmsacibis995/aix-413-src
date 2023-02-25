/* @(#)66	1.8  src/tcpip/usr/bin/telnet/ibm3270.h, tcptelnet, tcpip411, GOLD410 6/22/94 11:56:36 */
/*
 *   COMPONENT_NAME: TCPTELNET
 *
 *   FUNCTIONS: sizeof
 *
 *   ORIGINS: 10,27,38
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *  The following definitions were taken from:
 *	"IBM 3270 Information Display System Component Description",
 *	GA27-2749-10	(11th edition, February 1980)
 */
 
/* Special Characters */
 
# define	EBCDIC_NUL	0x00	/* Null Character */
# define	EBCDIC_SOH	0x01	/* Start Of Header */
# define	EBCDIC_STX	0x02	/* Start Of Text */
# define	EBCDIC_GE	0x08	/* Graphics Escape */
# define	EBCDIC_DUP	0x1c	/* DUP code */
# define	EBCDIC_FM	0x1e	/* Field Mark code */
# define	EBCDIC_SP	0x40	/* Space */
 
/* SNA Channel Type Command Codes 3274 (A and C units) */
# define	CMD_SNA_EAU		0x6f	/* Erase All Unprotected */
# define	CMD_SNA_EW		0xf5	/* Erase/Write */
# define	CMD_SNA_EWA		0x7e	/* Erase/Write Alternate */
# define	CMD_SNA_RB		0xf2	/* Read Buffer */
# define	CMD_SNA_RM		0xf6	/* Read Modified */
# define	CMD_SNA_WRITE		0xf1	/* Write */
# define	CMD_SNA_WSF		0xf3	/* Write Structured Field */

/* Command Codes (3272 & 3274-1B/D version) */
 
# define	CMD_EAU		0x0f	/* Erase All Unprotected */
# define	CMD_EW		0x05	/* Erase/Write */
# define	CMD_EWA		0x0d	/* Erase/Write Alternate */
# define	CMD_RB		0x02	/* Read Buffer */
# define	CMD_RM		0x06	/* Read Modified */
# define	CMD_WRITE	0x01	/* Write */
# define	CMD_NOOP	0x03	/* No Operation */
# define	CMD_SEL		0x0b	/* Select */
# define	CMD_SENSE	0x04	/* Sense */
# define	CMD_SID		0xe4	/* Sense ID */
# define	CMD_WSF		0x11	/* Write Structured Field */
 
/* Orders in a Write stream */
 
# define	ORDER_SF	0x1d	/* Start Field */
# define	ORDER_SFE	0x29	/* Start Field Extended */
# define	ORDER_MF 	0x2c	/* Modify Field */
# define	ORDER_SBA	0x11	/* Set Buffer Address */
# define	ORDER_IC	0x13	/* Insert Cursor */
# define	ORDER_PT	0x05	/* Program Tab */
# define	ORDER_RA	0x3c	/* Repeat to Address */
# define	ORDER_EUA	0x12	/* Erase Unprotected to Address */
# define	ORDER_SA 	0x28	/* Set Attribute */
 
/* Fields of a Write Control Character (WCC) */
# define	WCC_RESET	0x40	/* Reset Bit */
# define	WCC_FORMAT	0x30	/* Printout Format */
# define	WCC_132		0x00	/* 132-column lines */
# define	WCC_40		0x10	/* 40-column lines */
# define	WCC_64		0x20	/* 64-column lines */
# define	WCC_80		0x30	/* 80-column lines */
# define	WCC_START	0x08	/* Start Printer */
# define	WCC_ALARM	0x04	/* Sound Alarm */
# define	WCC_RESTORE	0x02	/* Restore Keyboard */
# define	WCC_RMDT	0x01	/* Reset Modified Data Tags */
 
/* Fields of an Attribute Byte */
# define	ATTR_TOP	0xc0	/* top bits of the attribute */
# define	ATTR_PROT	0x20	/* protected field */
# define	ATTR_NUMERIC	0x10	/* numeric field */
# define	ATTR_DISPLAY	0x0c	/* display attributes */
# define	ATTR_LN		0x00	/* low intensity / no pen detect */
# define	ATTR_LP		0x04	/* low intensity / pen detect */
# define	ATTR_HP		0x08	/* high intensity / pen detect */
# define	ATTR_IN		0x0c	/* invisible / no pen detect */
# define	ATTR_MDT	0x01	/* Modified Data Tag */
# define	ATTR_LMDT	0x02	/* local hack to identify chars */
					/* written during this operation */
					
/* buffer address modes */
# define	FOURTEEN_BIT	0x00	/* 14-bit buffer address mode */
# define	TWELVE_BIT	0x40	/* 12-bit buffer address mode */
# define	ADDR_PART	0x3f	/* address part of the byte */
 
/* Attention Identifications */
 
# define	AID_NONE	0x60	/* No AID generated (display) */
# define	AID_ENTER	0x7d	/* ENTER key */
# define	AID_PF1		0xf1	/* PF1 key */
# define	AID_PF2		0xf2	/* PF2 key */
# define	AID_PF3		0xf3	/* PF3 key */
# define	AID_PF4		0xf4	/* PF4 key */
# define	AID_PF5		0xf5	/* PF5 key */
# define	AID_PF6		0xf6	/* PF6 key */
# define	AID_PF7		0xf7	/* PF7 key */
# define	AID_PF8		0xf8	/* PF8 key */
# define	AID_PF9		0xf9	/* PF9 key */
# define	AID_PF10	0x7a	/* PF10 key */
# define	AID_PF11	0x7b	/* PF11 key */
# define	AID_PF12	0x7c	/* PF12 key */
# define	AID_PF13	0xc1	/* PF13 key */
# define	AID_PF14	0xc2	/* PF14 key */
# define	AID_PF15	0xc3	/* PF15 key */
# define	AID_PF16	0xc4	/* PF16 key */
# define	AID_PF17	0xc5	/* PF17 key */
# define	AID_PF18	0xc6	/* PF18 key */
# define	AID_PF19	0xc7	/* PF19 key */
# define	AID_PF20	0xc8	/* PF20 key */
# define	AID_PF21	0xc9	/* PF21 key */
# define	AID_PF22	0x4a	/* PF22 key */
# define	AID_PF23	0x4b	/* PF23 key */
# define	AID_PF24	0x4c	/* PF24 key */
# define	AID_SELECT	0x7e	/* Selector light pen */
# define	AID_PA1		0x6c	/* PA1 key */
# define	AID_PA2		0x6e	/* PA2 (CNCL) key */
# define	AID_PA3		0x6b	/* PA3 key */
# define	AID_CLEAR	0x6d	/* CLEAR key */
# define	AID_SYSREQ	0xf0	/* SYS REQ key */
 
/*-----------------------------------------------------------------------------
* These defines are needed for the Read Partition (Query) reply; 3278 Model
* sizes
*----------------------------------------------------------------------------*/
#define M5_3278         0
#define M4_3278         1
#define M3_3278         2
#define M2_3278         3
#define M1_3278         4
#define M1_3277         5

/*-----------------------------------------------------------------------------
* Write Structured Field states
*----------------------------------------------------------------------------*/
#define WSF_ST_FID             0
#define WSF_ST_PID             1
#define WSF_ST_RP              2
#define WSF_ST_ERASE           3
#define WSF_ST_3270DS          4
#define WSF_ST_SRM             5
#define WSF_ST_NEXT_FIELD      6

/*-----------------------------------------------------------------------------
* Structured Field IDs
*----------------------------------------------------------------------------*/
#define SF_READ              0x01
#define SF_SET_REPLY         0x09
#define SF_OB3270DS          0x40
#define SF_ERASE             0x03

/*-----------------------------------------------------------------------------
* Read Partition Types
*----------------------------------------------------------------------------*/
#define RP_QUERY             0x02
#define RP_QUERY_LIST        0x03
#define RP_RM                0xf6
#define RP_RMA               0x6e
#define RP_RB                0xf2

/*-----------------------------------------------------------------------------
* Reply Modes
*----------------------------------------------------------------------------*/
#define RM_FIELD             0
#define RM_EXTFIELD          1
#define RM_CHAR              2

/*-----------------------------------------------------------------------------
* Outbound 3270 Data Stream Commands
*----------------------------------------------------------------------------*/
#define OBDS_WRITE           0xf1
#define OBDS_EW              0xf5
#define OBDS_EWA             0x7e
#define OBDS_EAU             0x6f
#define OBDS_BSC_COPY        0xf7

# define	SCREEN_SIZE		(27 * 132)
# define	COMMAND_BUFFER_SIZE	(3 * SCREEN_SIZE)
# ifdef BUF3270
extern unsigned char	command_buffer[];
extern unsigned char	*cbufp;
# endif
#define TN3270 "tn3270"


#ifdef BIDI /* start bidi support */


int  switched_already     = FALSE;
int  num_chars_reversed   = 0;
int  num_chars_pushed     = 0;
int  push_start_loc       = 0;
int  field_org_start      = 0;
int  field_end            = 0;
int  field_rev            = FALSE;
int  screen_reversed      = FALSE;
int  push_mode            = FALSE;
int  autopush_mode        = FALSE;
int  autofldrev_mode      = FALSE;
int  already_in_num_fld   = FALSE;
int  already_in_alphanum_fld   = FALSE;
int  before_push_insert_status = FALSE;
int  RTL_autopush_enabled   = FALSE;
int  LTR_autopush_enabled   = FALSE;
int  RTL_autofldrev_enabled = FALSE;
int  LTR_autofldrev_enabled = FALSE;
int  national_lang_mode     = FALSE;
int  latin_lang_mode        = TRUE;

#endif /* BIDI */


#ifdef ARAB /* start ARABIC support */
#define AUTO          00
#define BASE          16
#define INITIAL       17
#define MIDDLE        18
#define FINAL         19
#define ISOLATED      20
#define MAX           43

int ASD_ON = TRUE;           /* indicates if automatic shaping is on */
int SHAPE  = AUTO;           /* indicates type of shaping enabled */
int SYMSWAP_ON = FALSE;      /* indicates if symmetric swapping is on */
int NUMSWAP_ON = FALSE;      /* indicates if numeric swapping is on */
int ARABIC_ON = TRUE;        /* indicates whether Arabic language is ena
                                bled */
int BIDI_ON = TRUE;          /* indicates whether we are in a Bidi window
                                or not */
int HEBREW_ON = FALSE;
unsigned char ARAB_KYB_MAP[256];
                             /* array to store Arabic keyboard mapping */
#ifndef _POWER
struct hfintrox {        /* VTD stream header */
    char hf_esc;        /* esc */
    char hf_lbr;        /* [ */
    char hf_ex;         /* x */
    char hf_len[4];     /* sizeof structure the hf_intro appears in minus 3 */
    char hf_typehi;     /* major type: 1=gen, 2=ksr, 3=mom */
    char hf_typelo;     /* minor type: 00-3F=output, 40-7F=query,
                           C0-FF=query response */
};

#define HFINTROSZ       sizeof(struct hfintrox)
struct hfcursorx {                       /* cursor representation */
        char hf_intro[HFINTROSZ];       /* esc [ x sizeof(struct hfcursor)-3
                                                 HFCURSORH HFCURSORL */
                                        /* Next 2 fields always ignored */
        char hf_sublen;                 /* sub header length = 2 */
        char hf_subtype;                /* sub header type = 0 */
        char hf_rsvd;                   /* reserved */
        char hf_shape;                  /* cursor shape */
#define HFNONE          0               /* no cursor */
#define HFSNGLUS        1               /* single underscore */
#define HFDBLUS         2               /* double underscore */
#define HFHALFBLOB      3               /* half blob */
#define HFMIDLINE       4               /* mid character line */
#define HFFULLBLOB      5               /* full blob */
};

#define HFINTROESC      '\033'
#define HFINTROLBR      '['
#define HFINTROEX       'x'
#define HFINTROSZ       10 
#define HFCURSORH       0x02    /* high type */
#define HFCURSORL       0x08    /* low type */
#endif /* _POWER */
#endif /* ARAB */
