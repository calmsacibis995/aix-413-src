/* @(#)18	1.12  src/bos/usr/sbin/lvm/include/cmdlvm.h, cmdlvm, bos411, 9428A410j 3/9/94 12:55:16 */

/*
 * COMPONENT_NAME: (cmdlvm) Logical Volume Commands
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * FILENAME: cmdlvm.h
 *
 * FILE DESCRIPTION: Include file for the logical volume commands.
 */

/****** LOGICAL VOLUME COMMAND CONSTANTS ******/
#define STRSIZE 50               /* string size of common strings/tokens */
#define MAXPPS 1016              /* max physical partitions allowed on a physical volume */
#define MAXLV 32512              /* max logical partitions allowed in a logical volume - from max number of pps on a disk times max number of pps in vg*/
#define MAXMOD 10000             /* max modify map entries allowed within the input map */
#define NUMBER_OF_PVS_FLAG 9999  /* signal the upper bound value is the number of pv's in vg */
#define MEGABYTE 1048576         /* number of bytes in one megabyte */
#define DEVICE_DIRECTORY "/dev/"
#define SREGION 5

#define TRUE 1
#define FALSE 0
#define SUCCESS 0
#define FAILURE 1
#define ILLEGALSYNTAX 1
#define ODMACCESSERROR 2
#define OBJNOTFOUND 3
#define MALLOC_FAILURE 5
#define LVMTIMEOUT 5
#define WAITTIME 3
#define WAITLOOP 5

#define FILESYSTEM_TYPE "aixv3fs"
#define FILESYSTEM_PREFIX "fslv"
#define FILESYSTEM_LOG_TYPE "aixv3log"
#define FILESYSTEM_LOG_PREFIX "loglv"
#define PAGING_TYPE "paging"
#define PAGING_PREFIX "pagelv"
#define BOOT_TYPE "boot"
#define BOOT_PREFIX "bootlv"
#define LVDEFAULT_TYPE "lvdefault"
#define LVDEFAULT_PREFIX "lv"
#define VGDEFAULT_PREFIX "vg"
#define LVUNQPREFIX "logical_volume/prefix"
#define VGDEFAULT_TYPE "vgdefault"

/****** LOGICAL VOLUME COMMAND MACROS ******/
#define SAMEPVS(pv1,pv2) (pv1.word1==pv2.word1 && pv1.word2==pv2.word2)
#define SAMELVID(lv1,lv2) (SAMEPVS((lv1).vg_id, (lv2).vg_id) && ((lv1).minor_num == (lv2).minor_num))
#define DISTANCE(d1, d2) (d1 - d2 >= 0 ? d1 - d2 : d2 - d1)

/****** LOGICAL VOLUME COMMAND TYPES ******/
typedef enum { min, map, max } INTER;  /* inter-pv allocation policies */
typedef enum { center, middle, edge } INTRA;  /* intra-pv allocation policies */
typedef enum { explicit, uncopy, copy, extend } TYPE;  /* allocation types */

typedef char STRING;
typedef int BOOLEAN;
typedef int RETURN_CODE;
typedef unsigned char BYTE;

struct ID { int w1; int w2; };
struct LVID { struct ID vg; int minor_num; };
struct REGION { int low; int high; };

/****** LOGICAL VOLUME COMMAND FUNCTIONS ******/
extern char *get_zeroed_mem();
extern char *getmem();
extern char *lvm_msg();
extern char *lvm_msgstr();

/****** LOGICAL VOLUME CONTROL BLOCK STRUCTURE ******/
#define LVCB_TAG_SIZE          10
#define LVCB_TYPE_SIZE         32
#define LVCB_NAME_SIZE         32
#define LVCB_TIME_SIZE         30
#define LVCB_LVID_SIZE         25
#define LVCB_VGID_SIZE         20
#define LVCB_MACHINE_ID_SIZE   10
#define LVCB_TAG               "AIX LVCB"
#define LVCB_LABEL_SIZE     128
#define LVCB_FS_SIZE        128        

struct LVCB {
        STRING validity_tag[LVCB_TAG_SIZE];
        STRING type[LVCB_TYPE_SIZE];
        STRING lvid[LVCB_LVID_SIZE];
        STRING lvname[LVCB_NAME_SIZE];
        STRING vgname[LVCB_NAME_SIZE];
        STRING created_time[LVCB_TIME_SIZE];
        STRING modified_time[LVCB_TIME_SIZE];
        STRING mod_machine_id[LVCB_MACHINE_ID_SIZE];
        char relocatable;
        char interpolicy;
        char intrapolicy;
        char auto_on;
        char strict;
        short int upperbound;
        short int num_lps;
        short int copies;
        STRING label[LVCB_LABEL_SIZE];
	STRING fs[LVCB_FS_SIZE];
	short int striping_width;
	short int stripe_exp;
};

/****** LOGICAL VOLUME DEFAULT VALUES ******/
#define LVDF_TYPE       "jfs"
#define LVDF_RELOCAT    'y'
#define LVDF_INTER      'm'
#define LVDF_INTRA      'm'
#define LVDF_STRICT     'y'
#define LVDF_UPPERBOUND 32
#define LVDF_COPIES     1
#define LVDF_STRIPE_WIDTH     0
#define LVDF_STRIPE_BLK_EXP     0

/******* GETLVODM AND PUTLVODM - COMMAND SPECIFIC DEFINITIONS ******/
/* putlvodm and getlvodm defines */


#define DBNAMESIZE 16
struct pvlist {
        char pvname[DBNAMESIZE];
        struct unique_id pvid;
        struct unique_id vgid;
        int boot_device;
};

#define LVSERIAL_ID "lvserial_id"
#define VGSERIAL_ID "vgserial_id"
#define PVSERIAL_ID "pvserial_id"
#define INTRA "intra"
#define INTER "inter"
#define LABEL "label"
#define STRICTNESS "strictness"
#define STRIPE_WIDTH "stripe_width"
#define STRIPE_SIZE "stripe_size"
#define COPIES "copies"
#define RELOCATABLE "relocatable"
#define TYPEA "type"
#define SIZE "size"
#define COPYFLAG "copyflag"
#define UPPERBOUND "upperbound"
#define PREFIX "prefix"
#define SEQ "sequence"
#define STATE "state"
#define LOCK "lock"
#define AUTO_ON "auto_on"
#define LVUNIQUETYPE "logical_volume/lvsubclass/lvtype"
#define VGUNIQUETYPE "logical_volume/vgsubclass/vgtype"
#define PVUNIQUETYPE "logical_volume/pvsubclass/pvtype"
#define STATUS 1
#define CHGSTATUS 1
#define PREV_STATUS 1
#define DECIMAL "."
#define OTHER 1
#define VGIDLENGTH 16
#define VGPREFIX "vg"
#define DECIMAL "."
#define MAXNUMVGS 32
#define MAXNUMPVS 32
#define STATUS 1
#define CHGSTATUS 1
#define PREV_STATUS 1
#define LVPREFIXTYPE "logical_volume/prefix"
#define CLASS_LV "logical_volume"
#define LVSUBCLASS "lvsubclass"
#define LVTYPE "lvtype"
#define LVPREFIX "lv"
#define VGSUBCLASS "vgsubclass"
#define VGTYPE "vgtype"
#define DEVNAMESIZE 15
#define CRITSIZE 256
#define LVCHARSIZE 256
#define PREFIXSZ 13
#define CUPVPVID 33
#define LVMPVID 17
#define TYPELENGTH 32
#define LABELLENGTH 128
#define LVMDD "hd_pin"
#define CFGPATH "/etc/objrepos"


/* IPL_varyon ERROR DEFINITIONS */

#define VARYON_FAILURE    1
#define ODM_FAILURE       2
#define ODM_OBJNOTFOUND   3
#define LQUERYPV_FAILURE  4
#define MALLOC_FAILURE    5
#define MKNOD_FAILURE     6
#define READERROR         7
#define OPEN_ERROR        8
#define LQUERYVG_FAILURE  9
#define NOKMID_FOUND     10
#define INVALIDIPLREC    11
#define GENMAJOR_FAILURE 12
#define NO_ERROR          0
