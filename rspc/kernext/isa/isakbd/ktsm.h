/* @(#)75   1.9  src/rspc/kernext/isa/isakbd/ktsm.h, isakbd, rspc41J, 9521A_all 5/23/95 09:06:40  */
/*
 *   COMPONENT_NAME: isakbd
 *
 *   FUNCTIONS: none
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */


#include  "ktsmtrace.h"                 /* must be first                     */
#include  <sys/types.h>
#include  <sys/sleep.h>                 /* wait structure                    */
#include  <sys/timer.h>
#include  <sys/malloc.h>                /* memory allocation definitions     */
#include  <sys/sysmacros.h>             /* defines the minor() macro         */
#include  <sys/errno.h>
#include  <sys/pin.h>
#include  <sys/ioacc.h>                 /* bus access macros                 */
#include  <sys/iocc.h>
#include  <sys/ioctl.h>
#include  <sys/adspace.h>
#include  <sys/intr.h>
#include  <sys/device.h>
#include  <sys/except.h>
#include  <sys/uio.h>
#include  <sys/xmem.h>
#include  <sys/cfgdb.h>
#include  <sys/signal.h>
#include  <sys/devinfo.h>
#include  <sys/dump.h>
#include  <sys/inputdd.h>
#include  <sys/ktsmdds.h>
#include  <stdarg.h>
#include  <sys/errids.h>
#include  <string.h>


/*****************************************************************************/
/* Structures for NIO port addresses                                         */
/*****************************************************************************/

#define	IOMEM_ATT_SIZE		0x1000	/* 4k is minimum you can attach to */
#define KM_DATA_PORT 		0x60
#define	KM_CMD_STATUS_PORT	0x64
#define SPEAKER_ENABLE_PORT	0x61
#define SPEAKER_TONE_PORT	0x42
#define CONTROL_WORD_PORT	0x43

#define  WRITE_KBD_CMD        0x0001    /* Write to kybd cmd                 */


#define SIO_SOUND_IO_BASE		0x80000040	/*  io start of sio timer registers */
#define	MODE3			0xB6	/* MODE 3 for square wave */
#define SPEAKER_OFF             0x00    /* bits 0&1: 00 = off                */
#define SPEAKER_ON              0x03    /* bits 0&1: 03 = on                 */
#define COUNTER_CLOCK_FREQ	1193000	/* 1.193 Mhz */
/*---------------------------------------------------------------------------*/
/* keyboard commands                                                         */
/*---------------------------------------------------------------------------*/


/* keyboard commands                                                         */
#define  K_RESET_CMD            0xFF    /* Reset command                     */
#define  K_RESEND_CMD           0xFE    /* Resend command                    */
#define  MAKE_CMD               0xFD    /* Set key to make only              */
#define  MAKE_BREAK_CMD         0xFC    /* Set key to make/break only        */
#define  TYPAMATIC_CMD          0xFB    /* Set key to typamatic only         */
#define  ALL_TYPA_MK_BRK_CMD    0xFA    /* Set all keys typa/make/brk        */
#define  DBL_RATE_TMATIC_CMD    0xFA    /* Set dbl rate (106 only)           */
#define  ALL_MAKE_CMD           0xF9    /* Set all keys make only            */
#define  ALL_MAKE_BRK_CMD       0xF8    /* Set all keys make/brk only        */
#define  ALL_TYPAMATIC_CMD      0xF7    /* Set all keys typamatic only       */
#define  DEFAULT_CMD            0xF6    /* Set default conditions            */
#define  DEF_DISABLE_CMD        0xF5    /* Disable default conditions        */
#define  K_ENABLE_CMD           0xF4    /* Enable keyboard output            */
#define  RATE_DELAY_CMD         0xF3    /* Set typamtic rates & delay        */
#define  READID_CMD             0xF2    /* Read ID command                   */
#define  NON_CMD                0xF1    /* NO GOOD ON R2 KEYBOARD            */
#define  SELECT_SCANCODE        0xF0    /* Select other scan code set        */
#define  LAYOUTID_CMD           0xEF    /* Read Layout ID command            */
#define  K_ECHO_CMD             0xEE    /* Echo command                      */
#define  LED_CMD                0xED    /* Set LED command                   */


#define  KBD_RESET_CMD            0xFF01      /* Reset command               */
#define  KBD_RESEND_CMD           0xFE01      /* Resend command              */
#define  SET_MAKE_CMD             0xFD01      /* Set key to make only        */
#define  SET_MAKE_BREAK_CMD       0xFC01      /* Set key to make/break only  */
#define  SET_TYPAMATIC_CMD        0xFB01      /* Set key to typamatic only   */
#define  SET_ALL_TYPA_MK_BRK_CMD  0xFA01      /* Set all keys typa/make/brk  */
#define  SET_DBL_RATE_TMATIC_CMD  0xFA01      /* Set dbl rate (106 only)     */
#define  SET_ALL_MAKE_CMD         0xF901      /* Set all keys make only      */
#define  SET_ALL_MAKE_BRK_CMD     0xF801      /* Set all keys make/brk only  */
#define  SET_ALL_TYPAMATIC_CMD    0xF701      /* Set all keys typamatic only */
#define  SET_DEFAULT_CMD          0xF601      /* Set default conditions      */
#define  DEFAULT_DISABLE_CMD      0xF501      /* Disable default conditions  */
#define  KBD_ENABLE_CMD           0xF401      /* Enable keyboard output      */
#define  SET_RATE_DELAY_CMD       0xF301      /* Set typamtic rates & delay  */
#define  READ_ID_CMD              0xF201      /* Read keyboard id            */
#define  UNSUPORTED_CMD           0xF101      /* NO GOOD ON R2 KEYBOARD      */
#define  SELECT_SCAN_CODE         0xF001      /* Select other scan code set  */

#define  LAYOUT_ID_CMD            0xEF01      /* Read layout id              */
#define  ECHO_CMD                 0xEE01      /* Echo command                */
#define  SET_LED_CMD              0xED01      /* Set LED command             */

#define  SET_ALL_LEDS_OFF         0x0001      /* data to set all leds off    */
#define  SET_500MS_DELAY          0x2B01      /* data to set delay at 500 ms */
#define  SET_300MS_DELAY          0x0B01      /* data to set delay at 300 ms */
#define  SET_400MS_DELAY          0x2B01      /* data to set delay at 400 ms */
#define  SELECT_SCAN_CODE_3       0x0301      /* data to select sc-code set 3*/

#define  SET_ACTION_SCAN_CODE     0x5801      /* data to set action key      */
#define  SET_R_ALT_SCAN_CODE      0x3901      /* data to set right alt key   */
#define  SET_L_ALT_SCAN_CODE      0x1901      /* data to set left alt key    */
#define  SET_L_SHIFT_SCAN_CODE    0x1201      /* data to set left shift key  */
#define  SET_R_SHIFT_SCAN_CODE    0x5901      /* data to set right shift     */
#define  SET_CONTROL_SCAN_CODE    0x1101      /* data to set control key     */
#define  SET_CAPS_LOCK_SCAN_CODE  0x1401      /* data to set caps lock key   */
#define  SET_NUM_LOCK_SCAN_CODE   0x7601      /* data to set num lock key    */
#define  SET_SCRLLOCK_SCAN_CODE   0x5F01      /* data to set scrl lock key   */
#define  SET_106K_1_SCAN_CODE     0x0E01      /* data to set keypos 1 key    */
#define  SET_106K_131_SCAN_CODE   0x8501      /* data to set alpha key       */
#define  SET_106K_132_SCAN_CODE   0x8601      /* data to set keypos 132 key  */
#define  SET_KATA_KEY_SCAN_CODE   0x8601      /* data to set katakana key    */
#define  SET_HIRA_KEY_SCAN_CODE   0x8701      /* data to set hiragana key    */
#define  SET_L_ARROW_SCAN_CODE    0x6101      /* data to set left arrow key  */
#define  SET_R_ARROW_SCAN_CODE    0x6A01      /* data to set right arrow key */
#define  SET_U_ARROW_SCAN_CODE    0x6301      /* data to set up arrow key    */
#define  SET_D_ARROW_SCAN_CODE    0x6001      /* data to set down arrow key  */


#define  ENABLE_KBD_CMD      0xAE
#define  DISABLE_KBD_CMD     0xAD
#define  DISABLE_AUX_CMD     0xA7
#define  ENABLE_AUX_CMD      0xA8
#define  READ_CCB_CMD        0x20
#define  WRITE_CCB_CMD       0x60

#define  KBD_ENABLE_INT      0x01             /* kbd interrupt bit on KMC    */

/*****************************************************************************/
/* Keyboard Defines - RSC Platform                                           */
/*****************************************************************************/

/* status/command port (0x51)                                                */
#define  KEYERROR               0x80    /* 1 = parity or framing error       */

#define  RX_FULL                0x01    /* 1 = recieve buffer Sandalfoot     */
#define  TX_FULL		0x02	/* new for sandalfoot */
#define  TX_EMPTY               0x20    /* 1 = transmit buffer empty         */
#define  KYBD_DATA              0x10    /* status of device data             */
#define  KYBD_CLOCK             0x08    /* status of device clock            */
#define  LOOP_MODE              0x04    /* 1 = set loop mode on (diagnostic) */
#define  KYBD_RESET             0x02    /* 1 = reset keyboard controller     */
#define  ENABLE_KYBD            0x01    /* 0 = enable, 1 = disable keyboard  */

/*****************************************************************************/
/* Mouse Defines                                                             */
/*****************************************************************************/

/* adapter command register (port 0x64)                                      */
#define  ADPTWRAP      0x0001 /* diagnostic wrap DD=test data                */

#define  M_ENABLE_INT  0x02   /* interrupt enabled after status reg is read  */
#define  M_DISABLE_INT ~(0x02) /* interrupt disabled                          */
#define  M_ENABLE_AUX     0xA8
#define  M_DISABLE_AUX    0xA7

/* Mouse receive registers (3 bytes)                                      */
/* ...byte 0 - receive data register (port 0x60)                           */
#define             M_DATA_0          0xFF000000

#define             REG_3_FULL        0x20000000  /* __1_____ register 3 full*/
#define             REG_2_FULL        0x10000000  /* ___1____ register 2 full*/
#define             REG_1_FULL        0x01 /* _______1 register 1 full*/
#define             M_DATA_ERR        0x08 /* 1_______ data error     */
#define             M_INTERRUPT       0x20 /* __1_____ auxillary interrupt   */
#define             M_FACE_ERR        0x04000000  /* _____1__ interface err  */

/* ...byte 1 - receive data register 1 (port 0x60)                           */
#define             M_DATA_1          0x00FF0000  /* mouse report frame 0    */

/* ...byte 2 - receive data register 2 (port 0x60)                           */
#define             M_DATA_2          0x0000FF00  /* mouse report frame 1    */

/* ...byte 3 - receive data register 3 (port 0x60)                           */
#define             M_DATA_3          0x000000FF  /* mouse report frame 2    */

/*---------------------------------------------------------------------------*/
/* mouse commands                                                            */
/*---------------------------------------------------------------------------*/

#define  MOUSE_CMD	 0xD4		/* Tell next command to go to mouse      */
#define  M_RESET         0xFF		/* Reset mouse                           */
#define  M_INV	         0xFC		/* Invalid command     */
#define  M_RESEND        0xFE		/* Resend previous byte(s) of packet     */
#define  M_SET_DEFAULT   0xF6		/* Set conditions to default state       */
#define  M_DISABLE       0xF5		/* Disable mouse                         */
#define  M_ENABLE        0xF4		/* Enable mouse                          */
#define  M_READ_CMD_BYTE 0x20

#define  M_SET_SAM_RATE  0xF3		/* Set sampling rate (stream mode only)  */
#define	 M_RATE_10	 0x0A     /* Sampling rate of 10                   */
#define  M_RATE_20	 0x14     /* Sampling rate of 20                   */
#define  M_RATE_40	 0x28     /* Sampling rate of 40                   */
#define  M_RATE_60	 0x3C     /* Sampling rate of 60                   */
#define  M_RATE_80	 0x50     /* Sampling rate of 80                   */
#define  M_RATE_100	 0x64     /* Sampling rate of 100                  */
#define  M_RATE_200	 0xC8     /* Sampling rate of 200                  */

#define  M_READ_DEV_TYP  0xF2     /* Read device type                      */
#define  M_SET_REM_MODE  0xF0     /* Set remote mode                       */
#define  M_SET_WRP_MODE  0xEE     /* Set wrap mode                         */
#define  M_RES_WRP_MODE  0xEC     /* Reset wrap mode                       */
#define  M_READ_DATA     0xEB     /* Read data                             */
#define  M_SET_STM_MODE  0xEA     /* Set stream mode                       */
#define  M_STATUS_REQ    0xE9     /* Status request                        */

#define  M_SET_RES       0xE8     /* Set resolution                        */
#define     M_RES_1      0x00     /* 1 count  per mm                       */
#define     M_RES_3      0x01     /* 3 counts per mm                       */
#define     M_RES_6      0x02     /* 6 counts per mm                       */
#define     M_RES_12     0x03     /* 12 counts per mm                      */

#define  M_SET_SCL_2_1   0xE7     /* Set scaling to 2:1 (stream mode only) */
#define  M_RESET_SCALE   0xE6     /* Reset scaling to 1:1                  */

#define  M_NULL          0x00     /* null                                  */
#define  M_DATA_WAITING  0x21

/*---------------------------------------------------------------------------*/
/* mouse response (4 byte read)                                              */
/*---------------------------------------------------------------------------*/

#define  M_ACK           0xFA /* response from mouse after valid input */

#define  M_INV_CMD_RC    0x00FE0000 /* response for invalid command          */
#define  M_INV_CMDS_RC   0x00FC0000 /* response for invalid commands         */

#define  M_BAT_CC        0xAA       /* response for succesful BAT test       */
#define  M_IBM_2_BUTTON  0x00       /* id for ibm 2 button mouse             */
#define  M_LOG_2_BUTTON  0x02       /* id for logitech 2 button mouse        */
#define  M_LOG_3_BUTTON  0x03       /* id for logitech 3 button mouse        */
#define  M_DEFAULT_ID    0x00       /* default id send after reset command   */

/* mouse data report                                                         */
#define             M_Y_DATA_OVERFLOW 0x00800000  /* 1= overflow             */
#define             M_X_DATA_OVERFLOW 0x00400000  /* 1= overflow             */
#define             M_Y_DATA_SIGN     0x00200000  /* 1= negative             */
#define             M_X_DATA_SIGN     0x00100000  /* 1= negative             */
#define             M_C_BUTTON_STATUS 0x00040000  /* 1= down; center button  */
#define             M_R_BUTTON_STATUS 0x00020000  /* 1= down; right button   */
#define             M_L_BUTTON_STATUS 0x00010000  /* 1= down; left  button   */

/* mouse status request                                                      */
#define             MSR_STR_REM_MODE  0x00400000  /* stream =0; remote=1     */
#define             MSR_DIS_ENABLE    0x00200000  /* disable=0; enable=1     */
#define             MSR_SCALING       0x00100000  /* 1:1    =0; 2:1   =1     */
#define             MSR_LEFT_BUTTON   0x00040000  /* up     =0; down  =1     */
#define             MSR_CENTER_BUTTON 0x00020000  /* up     =0; down  =1     */

/*****************************************************************************/
/* delay timings                                                             */
/*****************************************************************************/

/* watchdog timers values                                                    */
#define WDOGFULL            200000000 /* 200 ms input buffer full timeout    */
                                      /* max 8042 retry cycle takes 46ms so  */
                                      /* need to wait just a little longer   */
#define WDOGTIME            50000000  /* so set no response timeout to 50ms  */
#define RSTWD               500000000 /* 500ms reset timeout                 */

/* ktsm_sleep timer values                                                   */
#define POR_WAIT            2500000   /* 2500ms sleep time for waiting POR   */
#define FRAME_WAIT          200000    /* 200ms I/O wait loop sleep time      */
#define ADPT_RST_WAIT       120000    /* 120ms adapter reset sleep time      */
#define RESETTO             700000    /* 700ms RESET timeout                 */
#define RCVTO               30000     /* 30ms timeout waiting for RCV frames */
#define DMBSY               1500      /* 1.5 ms timeout waiting for mouse    */
                                      /*   adapter to drop busy              */

#define KPOLL_TO            30        /* 30sec keep alive poll ACK timeout   */

/*****************************************************************************/
/* error log ID's                                                            */
/*****************************************************************************/

                                      /* error log resource names            */
#define RES_NAME_KBD        "KBDDD"
#define RES_NAME_MSE        "MOUSEDD"

#define PIO_ERROR           1         /* permanent PIO error                 */
#define RCV_ERROR           2         /* receive errors have exceeded        */
                                      /*   threshold                         */
#define KBDBAT_ERROR        3         /* invalid keyboard BAT                */
#define KBDID_ERROR         4         /* invalid keyboard ID                 */
#define XMEMCPY_ERROR       5         /* cross memory copy failure           */
#define KIO_ERROR           6         /* keyboard  I/O error                 */

#define MDDBAT_ERROR        7         /* invalid mouse BAT                   */
#define MDDID_ERROR         8         /* invalid mouse ID                    */
#define MDDTYPE_ERROR       9         /* invalid type of mouse               */
#define MIO_ERROR           10        /* mouse I/O error                     */

#define ADPT_ERROR          11        /* invalid 8051 adapter status         */
#define ABAT_ERROR          12        /* 8051 adapter BAT failure            */

#define TAB_CFG_ERROR       13        /* Tablet Read Configuration Failure   */
#define TABTYPE_ERROR       14        /* Tablet Read Configuration Failure   */
#define TIO_ERROR           15        /* tablet I/O error                    */

#define PM_REG_ERROR        16        /* pm handle registratin failure       */

/*****************************************************************************/
/* Non scan code responses from keyboard                                     */
/*****************************************************************************/

#define  OVERRUN                  0x00  /* overrun condition                 */
#define  OVERRUN_2                0xFF  /* overrun condition                 */
#define  KBD_ACK                  0xFA  /* keyboard acknowledge              */
#define  BREAK_CODE               0xF0  /* break code                        */
#define  KBD_ECHO                 0xEE  /* keyboard echo                     */
#define  KBD_BAT_CC               0xAA  /* keyboard BAT completion code      */
#define  KBD_RESEND               0xFE  /* keyboard resend                   */

#define  KBD_POR_ID_1             0x83  /* keyboard POR id byt */
#define  KBD_POR_ID_1B            0x84  /* keyboard POR id byt */
#define  KBD_POR_ID_1C            0xBF  /* keyboard POR id byt */
#define  KBD_POR_ID_2A            0xB0  /* keyboard POR id byte #2 (101 kbd) */
#define  KBD_POR_ID_2B            0xB1  /* keyboard POR id byte #2 (102 kbd) */
#define  KBD_POR_ID_2C            0xB2  /* keyboard POR id byte #2 (106 kbd) */
#define  KBD_POR_ID_2D            0xAB  /* keyboard POR id byte #2           */

/*****************************************************************************/
/* Defines for misc key scan codes                                           */
/*****************************************************************************/

#define  ACTION_SCAN_CODE         0x58        /* data to set action key      */
#define  R_ALT_SCAN_CODE          0x39        /* data to set right alt key   */
#define  L_ALT_SCAN_CODE          0x19        /* data to set left alt key    */
#define  L_SHIFT_SCAN_CODE        0x12        /* data to set left shift key  */
#define  R_SHIFT_SCAN_CODE        0x59        /* data to set right shift     */
#define  CONTROL_SCAN_CODE        0x11        /* data to set control key     */
#define  CAPS_LOCK_SCAN_CODE      0x14        /* data to set caps lock key   */
#define  NUM_LOCK_SCAN_CODE       0x76        /* data to set num lock key    */
#define  K106_1_SCAN_CODE         0x0E        /* data to set keypos 1 key    */
#define  K106_131_SCAN_CODE       0x85        /* data to set alpha key       */
#define  K106_132_SCAN_CODE       0x86        /* data to set keypos 132 key  */
#define  KATA_KEY_SCAN_CODE       0x86        /* data to set katakana key    */
#define  HIRA_KEY_SCAN_CODE       0x87        /* data to set hiragana key    */
#define  L_ARROW_SCAN_CODE        0x61        /* data to set left arrow key  */
#define  R_ARROW_SCAN_CODE        0x6A        /* data to set right arrow key */
#define  U_ARROW_SCAN_CODE        0x63        /* data to set up arrow key    */
#define  D_ARROW_SCAN_CODE        0x60        /* data to set down arrow key  */

#define  LARGEST_SCAN_CODE        0x84        /* largest possible scan code  */

/*****************************************************************************/
/* Defines for misc position codes                                           */
/*****************************************************************************/

#define  L_SHIFT_POS_CODE         44
#define  R_SHIFT_POS_CODE         57

#define  L_ALT_POS_CODE           60
#define  R_ALT_POS_CODE           62

#define  CNTL_POS_CODE            58

#define  ACTION_POS_CODE          64

#define  CAPS_LOCK_POS_CODE       30
#define  NUM_LOCK_POS_CODE        90

#define  ALPHA_POS_CODE           60
#define  KATAKANA_POS_CODE        30
#define  HIRAGANA_POS_CODE        133

#define  R_CURSOR_POS_CODE        89
#define  L_CURSOR_POS_CODE        79
#define  U_CURSOR_POS_CODE        83
#define  D_CURSOR_POS_CODE        84

#define  NUMPAD0                  99
#define  NUMPAD1                  93
#define  NUMPAD2                  98
#define  NUMPAD3                 103
#define  NUMPAD4                  92
#define  NUMPAD5                  97
#define  NUMPAD6                 102
#define  NUMPAD7                  91
#define  NUMPAD8                  96
#define  NUMPAD9                 101

#define  BACKSPACE                15

#define  SAK_KEY_0                58    /* ctrl key pos code for SAK         */
#define  SAK_KEY_0_SC             0x11  /* ctrl key scan code for SAK        */
#define  SAK_KEY_1                47    /* "x" key pos  code for SAK         */
#define  SAK_KEY_1_SC             0x22  /* "x" key scan code for SAK         */
#define  SAK_KEY_2                20    /* "r" key pos  code for SAK         */

/*****************************************************************************/
/* Miscellaneous definitions                                                 */
/*****************************************************************************/

                                       /* device type for devinfo sturct     */
#define INPUTDEV               DD_INPUT

#define M_RETRY                7       /* max mouse operation retries        */
#define BUF_RETRY              4000    /* max retries for input buffer full  */
#define K_RETRY                7       /* max keyboard operation retries     */
#define T_RETRY                3       /* max tablet operation retries       */
#define R_RETRY                3       /* max receive (get_iq()) retries     */

#define RCV_ERR_THRESHOLD      50      /* receive error threshold            */

#define  SCAN_CODE_3           0x03    /* data to select sc-code set 3       */

#define  MIN_TYPA_RATE         2       /* minimum typamatic rate             */
#define  MAX_TYPA_RATE         30      /* maximum typamatic rate             */
#define  DEF_TYPA_RATE         20      /* default typamatic rate             */
#define  DEF_DELAY             500     /* default typamatic delay            */

                                       /* keyboard led cmd parms             */
#define  CAPSLOCK_LED          4       /*   Capslock led                     */
#define  NUMLOCK_LED           2       /*   Numlock led                      */
#define  SCRLLOCK_LED          1       /*   Scroll lock led                  */
#define  CAPSLOCK_LED_106      2       /*   Capslock led - 106 key keyboard  */
#define  NUMLOCK_LED_106       4       /*   Numlock led - 106 key keyboard   */
#define  ALL_LEDS_OFF          0       /*   all leds off                     */

                                       /* keyboard rate/delay cmd parms      */
#define  DELAY_1000MS          0x60    /* data to set delay at 1000 ms       */
#define  DELAY_750MS           0x40    /* data to set delay at 750 ms        */
#define  DELAY_500MS           0x20    /* data to set delay at 500 ms        */
#define  DELAY_250MS           0x00    /* data to set delay at 250 ms        */

#define KOREAN_MAP             1       /* dds map parameter for Korean       */
#define JAPANESE_MAP           2       /* dds map parameter for Japanese     */
#define CHINESE_MAP            3       /* dds map parameter for Chinese      */

#define NO_NUM_PAD             1       /* dds type parameter for non 10key kb*/

#define SOUND_Q_SIZE           64      /* size of sound queue (num elements) */
#define MAX_ALARM_DURATION     32767
#define MAX_ALARM_FREQ         12000
#define MIN_ALARM_FREQ         32

#define NOVOLUME               0xff    /* adapter volume not set             */

#define  MIN_MOUSE_THRESHOLD       0
#define  MAX_MOUSE_THRESHOLD       32767

#define  DEFAULT_THRESHOLD         22
#define  DEFAULT_RESOLUTION        MRES3
#define  DEFAULT_SAMPLE_RATE       MSR100
#define  DEFAULT_SCALING           MSCALE11

#define NO_PARM   0xffff               /* no command parameter               */

#define  INPUT_DEVICE_TYPE     0x18     /* input device type mask            */
#define  NO_INPUT_DEVICE       0x00
#define  STYLUS_INPUT_DEVICE   0x08
#define  PUCK_INPUT_DEVICE     0x10

/*****************************************************************************/
/* input ring control block structure                                        */
/*****************************************************************************/
struct rcb {
   pid_t   owner_pid;                  /* process ID of ring's owner         */
#define KERNEL_PID   -1                /*   kernel process owns channel      */

   caddr_t ring_ptr;                   /* pointer to input ring              */
                                       /*   (NULL if no ring registered)     */
   int     ring_size;                  /* ring size                          */
   struct  xmem  xmdb;                 /* cross memory descriptor block      */
   uchar   rpt_id;                     /* report source identifier           */
};

/*****************************************************************************/
/*  I/O locking structure                                                    */
/*****************************************************************************/

struct _km_io_lock {

  uchar other;                        /* none zero if other device configured*/
#define NO_OTHER_DEVICE_USING 0
#define OTHER_DEVICE_USING    1
  uchar cmd_save;                     /* 8042 command byte save area         */
#define DIS_AUX_INTERFACE 0x20
#define DIS_KBD_INTERFACE 0x10
#define EN_AUX_INTR 0x02
#define EN_KBD_INTR 0x01
  uchar lock;                         /* adapter locked state                */
#define NOT_LOCKED 0
#define LOCKED_BY_KBD  1
#define LOCKED_BY_MSE  2

};


