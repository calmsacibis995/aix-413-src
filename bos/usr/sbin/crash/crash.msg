$ @(#)79	1.8.5.5  src/bos/usr/sbin/crash/crash.msg, cmdcrash, bos411, 9435A411a 8/26/94 09:14:37

$ /*
$  * COMPONENT_NAME: (CMDCRASH) 
$  *
$  * FUNCTIONS: 
$  *
$  * ORIGINS: 27 83
$  *
$  * This module contains IBM CONFIDENTIAL code. -- (IBM
$  * Confidential Restricted when combined with the aggregated
$  * modules for this product)
$  *                  SOURCE MATERIALS
$  * (C) COPYRIGHT International Business Machines Corp. 1988, 1994
$  * All Rights Reserved
$  *
$  * US Government Users Restricted Rights - Use, duplication or
$  * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$  *
$  * LEVEL 1,5 Years Bull Confidential Information
$  *
$  * (Copyright statements and/or associated legends of other
$  * companies whose code appears in any part of this module must
$  * be copied here.)
$  */

$ DUE TO THE NATURE OF CRASH COMMAND MESSAGES IN THIS FILE ARE 
$ DEPENDANT ON OPERATING SYSTEM TERMINOLOGY.
$ A MEANINGFUL TRANSLATION OF THIS COMMAND WOULD REQUIRE AN AIX
$ EXPERT OF THAT LANGUAGE .

$ NOTE: ***** ONLY A FEW COMMENTS HAVE BEEN ADDED FOR THIS DROP *******

$quote "         define quote character.

$len 
$set MS_crsh        Define initial set#

$ Note: Do not translate %d %s 0x%8d %u .
$ OS Dependant keywords : Buffer, Process, Table, Stack, Mbuf, Inode, Stat,
$ Stack, heap, address, header, t_maint, directory, structure, trb, map,
$ Process slot,extension segment, zombie, Component dump table, uarea, dump,
$ vnode, 

BUF_MSG_1            "Buffer %3d above maximum limit.Specify a value that is less than %d.\n"
BUF_MSG_2            "0452-002: Error: Cannot retrieve address of buffer table.\n"
BUF_MSG_3            "0452-003: Error: Cannot read buffer header %u.\n"
BUF_MSG_4            " write"
$ This is a state of buffer flags.Do not remove the leading space.
BUF_MSG_5            " read"
$ This is a state of buffer flags.Do not remove the leading space.
BUF_MSG_6            " done"
$ This is a state of buffer flags.Do not remove the leading space.
BUF_MSG_7            " error"
$ This is a state of buffer flags.Do not remove the leading space.
BUF_MSG_8            " busy"
$ This is a state of buffer flags.Do not remove the leading space.
BUF_MSG_9            " inflight"
$ This is a state of buffer flags.Do not remove the leading space.
BUF_MSG_10           " age"
$ This is a state of buffer flags.Do not remove the leading space.
BUF_MSG_11           " async"
$ This is a state of buffer flags.Do not remove the leading space.
BUF_MSG_12           " delwri"
$ This is a state of buffer flags.Do not remove the leading space.
BUF_MSG_13           " stale"
$ This is a state of buffer flags.Do not remove the leading space.
BUF_MSG_14           "\n\
BUFFER FOR BUF_HDR %d:"
$ Heading for printing out the buffer contents.BUF_HDR refers to buffer header.
BUF_MSG_15           "Buffer %d above maximum limit.Specify a value less than %d.\n"
BUF_MSG_16           "0452-016: Cannot read pointer to buffer table from address 0x%8x\n"
BUF_MSG_17           "0452-017: Cannot read buffer header %d from address 0x%8x\n"
BUF_MSG_18           "0452-018: Cannot read buffer %d from address 0x%8x\n"
BUF_MSG_19           "0452-019: Wrong display mode specified.\n"
BUF_MSG_21           "0452-020: Cannot create file %s.\n"
BUF_MSG_22           "0452-021: Cannot write to  file %s.\n"
BUF_MSG_23           "file:  %s\n"
BUF_MSG_24           "0452-023: Buffer %2d does not contain directory entries\n"
BUF_MSG_25           "0452-024: Buffer %2d does not contain directory entries\n"

DUMP_MSG_1           "what?\n"
DUMP_MSG_2           "Component dump table entry %d not found.\n"
DUMP_MSG_3           "0452-050: No dump routines available in directory %s.\n"
DUMP_MSG_4           "0452-051: Cannot stat %s.\n"
DUMP_MSG_5           "Component Names:\n"
DUMP_MSG_6           "0452-053:  Component dump table not found.\n"
DUMP_MSG_7           "Enter component number (or q to quit)\n"
DUMP_MSG_8           "0452-055: Cannot read dump file.\n"
DUMP_MSG_9           "0452-056: Premature end of data in dump file.\n"
DUMP_MSG_10          "Specified component dump table index is not valid.\n"
DUMP_MSG_13          "0452-058: Cannot execute %s, errno = %d.\n"
DUMP_MSG_14          "Cannot fork %s, errno = %d.\n"
FILE_MSG_1           "File table entry %4d above maximum limit.\n\
\tSpecify a value less than %d.\n"
FILE_MSG_2           "0452-061: Cannot read file table entry %4d.\n"
FILE_MSG_3           "0452-062: Cannot read vnode associated with file table entry %4d.\n"
FILE_MSG_4           " read"
$ This is a state of file flags.Do not remove the leading space.
FILE_MSG_5           " write"
$ This is a state of file flags.Do not remove the leading space.
FILE_MSG_6           " append"
$ This is a state of file flags.Do not remove the leading space.
FILE_MSG_7           " sync"
$ This is a state of file flags.Do not remove the leading space.
FILE_MSG_8           " mpx"
$ This is a state of file flags.Do not remove the leading space.
FILE_MSG_9           " ndelay"
$ This is a state of file flags.Do not remove the leading space.
FILE_MSG_11           " nonblock"
$ This is a state of file flags.Do not remove the leading space.
FILE_MSG_10           "0452-063: Cannot read gnode associated with file table entry %4d.\n"

INODE_MSG_2          "0452-087: Cannot read inode hash table entry %d.\n"
INODE_MSG_3          "0452-088: Cannot read inode at address 0x%8x.\n"
INODE_MSG_5          " lck"
INODE_MSG_6          " wnt"
INODE_MSG_7          " glck"
INODE_MSG_8          " acc"
INODE_MSG_9          " chg"
INODE_MSG_10         " upd"
INODE_MSG_11         " fsyn"
INODE_MSG_12         " map"
INODE_MSG_13         " def"
INODE_MSG_14         " nshr"
INODE_MSG_15         " wait"
INODE_MSG_16         " zomb"

F_MAIN_MSG_3         "USAGE: crash [-a] [systemimagefile [kernelfile]]\n"
$ Messages F_MAIN_MSG_3 MSG_4 and MSG_5 are no longer used see MAIN_MSG_99
MAIN_MSG_7           "Using /unix as the default namelist file.\n"
P_MAIN_MSG_8         "eh?\n"
MAIN_MSG_10          "Dump Analyzer\n"
MAIN_MSG_12          "Available commands:\n\n"
MAIN_MSG_13          " ADDRESS   MAJ  MIN INUMB REF LINK  UID  GID   SIZE   MODE  SMAJ SMIN FLAGS\n"
MAIN_MSG_14          "USAGE: inode [-] [<maj> <min> <inode>]\n"
MAIN_MSG_17          "VFS ADDRESS   TYPE OBJECT    STUB NUMBER FLAGS\tPATHS\n"
MAIN_MSG_18          "ADDRESS VFS MVFS VNTYPE FSTYPE COUNT ISLOT  DATAPTR FLAGS\n"
MAIN_MSG_19          "  ADDRESS   SIZE    TYPE     LINK       DATAPTR\n"
MAIN_MSG_20          "USAGE: mbuf <hexaddr> ...\n"
MAIN_MSG_22          "\tno match\n"
MAIN_MSG_23          "no match\n"
MAIN_MSG_24          "\tno match\n"
MAIN_MSG_25          "BUF MAJ  MIN    BLOCK   FLAGS\n"
MAIN_MSG_26          "\nBUFFER HEADER %d:\n"
MAIN_MSG_27          "Mode %s not valid.\n"
MAIN_MSG_31          "0452-131: cm command is only allowed on a running system.\n"
MAIN_MSG_32          "USAGE: cm [<SlotNum> <SegNum>]\n"
MAIN_MSG_33          "USAGE: cm [<SlotNum> <SegNum>]\n"
MAIN_MSG_34          "Missing parameter.\n\tSpecify an address or a symbol.\n"
MAIN_MSG_35          "Symbol not found.\n"
MAIN_MSG_36          "SYSTEM STAT:\n\n"
MAIN_MSG_37          "\nSYSTEM VARIABLES:\n\n"
MAIN_MSG_38          "\nSYSTEM TRB STRUCTURES:\n\n"
MAIN_MSG_39          "\nBUFFER HEADERS :\n\n"
MAIN_MSG_40          "BUF MAJ  MIN    BLOCK   FLAGS\n"
MAIN_MSG_41          "\nHASHED INODES\n\n"
MAIN_MSG_42          "MAJ  MIN INUMB REF LINK  UID  "
MAIN_MSG_43          "GID   SIZE   MODE  SMAJ SMIN FLAGS\n"
MAIN_MSG_44          "\nTTY STRUCTURES:\n\n"
MAIN_MSG_45          "\nVFS STRUCTURES:\n\n"
MAIN_MSG_46          "VFS ADDRESS   TYPE OBJECT    STUB NUMBER FLAGS\tPATHS\n"
MAIN_MSG_47          "\nFILE TABLES:\n\n"
MAIN_MSG_49          "\nPROCESS TABLE:\n\n"
MAIN_MSG_50          "0452-149: Cannot read process table entry %3d.\n"
MAIN_MSG_51          "0452-150: Cannot read extension segment value from address 0x%8x.\n"
MAIN_MSG_52          "0452-151: Cannot read process table entry %3d.\n"
MAIN_MSG_53          "\nDATA FOR PROCESS TABLE ENTRY %d:"
P_MAIN_MSG_56        "\n\eh?\n"
P_MAIN_MSG_57        "> "
MAIN_MSG_58          "\t%c is not a valid hexadecimal digit.\n"
MAIN_MSG_59          " SLOT    FILE        REF     TYPE            ADDRESS      FLAGS\n"
MAIN_MSG_60		"0452-700: Parameter %s not valid.Specify a thread slot number.\n"
MAIN_MSG_61		"SLT ST    PID   PPID   PGRP   UID  EUID  TCNT  NAME\n"
MAIN_MSG_62		"0452-702: Parameter %s not valid.Specify a thread slot number.\n"
MAIN_MSG_63		"0452-703: Parameter %s not valid.Specify a thread slot number.\n"
MAIN_MSG_64		"0452-704: Parameter %s not valid.Specify a thread slot number.\n"
MAIN_MSG_65		"SLT ST    TID      PID    CPUID  POLICY PRI CPU    EVENT  PROCNAME     FLAGS\n"
MAIN_MSG_66		"0452-706: Specify a process slot\n"
MAIN_MSG_67		"0452-707: Specify a thread address\n"
MAIN_MSG_68		"0452-708: No such option\n"
MAIN_MSG_69		"0452-709: Specify a logical processor number or *.\n"
MAIN_MSG_70		"0452-710: Specify a logical processor number. \n"
MAIN_MSG_71		"Selected cpu number : %2d\n"
MAIN_MSG_72		"0452-712: Specify a logical processor number.\n"
MAIN_MSG_73		"0452-713: This machine has got %2d cpus.\n"
MAIN_MSG_74		"0452-714: Cannot read thread table entry $d."
MAIN_MSG_75		"\nDATA FOR THREAD TABLE ENTRY %d:"
MAIN_MSG_76		"\nPER PROCESSOR DATA AREAS\n"
MAIN_MSG_77		"\nSTATUS OF PROCESSORS \n"
MAIN_MSG_78		"0452-715: Cannot read process entry %d\n"
MAIN_MSG_79		"\nTHREAD TABLE:\n\n"
MAIN_MSG_80		"CPU     TID  TSLOT     PID  PSLOT  PROC_NAME\n"
F_MAIN_MSG_99	     "USAGE: crash [-a] [-i includefile] [systemimagefile [kernelfile]]\n"
MAIN_MSG_100	     "Building structure debug info..."

MBUF_MSG_1           "Not a valid mbuf pointer:%8x, must be on %d byte boundary!\n"
MBUF_MSG_2           "0452-159: Cannot read mbuf structure at address %8x.\n"
MBUF_MSG_3           "free"
MBUF_MSG_4           "data"
MBUF_MSG_5           "header"
MBUF_MSG_6           "socket"
MBUF_MSG_7           "pcb"
MBUF_MSG_8           "rtable"
MBUF_MSG_9           "htable"
MBUF_MSG_10          "atable"
MBUF_MSG_11          "soname"
MBUF_MSG_13          "soopts"
MBUF_MSG_14          "ftable"
MBUF_MSG_15          "rights"
MBUF_MSG_16          "ifaddr"
MBUF_MSG_17          "weird"
MBUF_MSG_18          "0452-171: Cannot read mbuf data at address %8x.\n"
MBUF_MSG_19          "0452-172: Cannot read mbuf data at address %8x.\n"
MBUF_MSG_20          "0452-173: Cannot retrieve address of mbclusters.\n"
MBUF_MSG_21          "0452-174: Cannot retrieve address of mbufs.\n"

PRVFS_MSG_1          "0452-258: Cannot read rootvfs structure at address 0x%8x.\n"
PRVFS_MSG_2          "0452-259: Cannot read vfs list from address 0x%8x.\n"
PRVFS_MSG_3          "0452-260: Cannot read vfs vmount data at address 0x%8x.\n"
PRVFS_MSG_4          "0452-261: Cannot read vfs gfs data at address 0x%8x.\n"
PRVFS_MSG_5          "ADDRESS VFS MVFS VNTYPE FSTYPE COUNT ISLOT    INODE FLAGS\n"
PRVFS_MSG_6          "0452-263: VFS %4d not currently on vfs list.\n"
PRVFS_MSG_7          "\t%s mounted over %s\n"
PRVFS_MSG_8          "vdir"
PRVFS_MSG_9          "vchr"
PRVFS_MSG_10         "vblk"
PRVFS_MSG_11         "vreg"
PRVFS_MSG_12         "vmpc"
PRVFS_MSG_13         "vfifo"
PRVFS_MSG_14         "vbad"
PRVFS_MSG_15         "vnon"
PRVFS_MSG_16         "vlnk"
PRVFS_MSG_17         "vsock"
PRVFS_MSG_18         "weird"
PRVFS_MSG_19         "dir"
PRVFS_MSG_20         "mpc"
PRVFS_MSG_21         "chr"
PRVFS_MSG_22         "blk"
PRVFS_MSG_23         "reg"
PRVFS_MSG_24         "fifo"
PRVFS_MSG_25         "weird"
PRVFS_MSG_26         "aix"
PRVFS_MSG_27         "0452-284: Cannot read vnode structure at address 0x%8x.\n"
PRVFS_MSG_28         "\tflags:\tC=disconnected D=device   I=remote P=removable\n"
PRVFS_MSG_29         "\t\tR=readonly     S=shutdown U=unmounted Y=dummy\n"

PRVNODE_MSG_1        "0452-287: Cannot read vnode at address 0x%8x.\n"
PRVNODE_MSG_2        " Total VNODES printed %d\n"
PRVNODE_MSG_3        "vreg"
PRVNODE_MSG_4        "vdir"
PRVNODE_MSG_5        "vblk"
PRVNODE_MSG_6        "vchr"
PRVNODE_MSG_7        "vlnk"
PRVNODE_MSG_8        "vsock"
PRVNODE_MSG_9        "vfifo"
PRVNODE_MSG_10       "vmpc"
PRVNODE_MSG_11       "vbad"
PRVNODE_MSG_12       "vundef"
PRVNODE_MSG_13       "weird"
PRVNODE_MSG_14       "vfs_root "
PRVNODE_MSG_15       "vfs_unmounted "
PRVNODE_MSG_16       "remote "
PRVNODE_MSG_17       "dma "
PRVNODE_MSG_18       "text "
PRVNODE_MSG_19       "rmdc "
PRVNODE_MSG_20       "rename"
$ See additional PRVNODE message at bottom of file

SOCKET_MSG_1         "0452-329: Cannot read file table entry %3d.\n"
SOCKET_MSG_2         "0452-330: Cannot read socket at address 0x%8x.\n"
SK_SOCKET_MSG_3      "rcv"
SK_SOCKET_MSG_4      "snd"
SOCKET_MSG_5         " lock"
SOCKET_MSG_6         " want"
SOCKET_MSG_7         " wait"
SOCKET_MSG_8         " sel"
SOCKET_MSG_9         " kiodone"

STAT_MSG_1           "0452-338: Cannot read uname structure from address 0x%8x\n"
STAT_MSG_2           "\tsysname: %s\n"
STAT_MSG_3           "\tnodename: %s\n"
STAT_MSG_4           "\trelease: %s\n"
STAT_MSG_5           "\tversion: %s\n"
STAT_MSG_6           "\tmachine: %s\n"
STAT_MSG_7           "\ttime of crash: %s"
STAT_MSG_8           "\tage of system: "
STAT_MSG_9           "%lu day, "
STAT_MSG_10          "%lu hr., "
STAT_MSG_11          "%lu min."
STAT_MSG_12          "0452-349: Cannot read v structure from address 0x%8x\n"
STAT_MSG_13	     "\tabend code: %x\n"
STAT_MSG_14	     "\tcsa: 0x%x\n"

SYMTAB_MSG_1         "Reading in Symbols "
SYMTAB_MSG_2         "0452-351: Cannot read symbol %s\n"
SYMTAB_MSG_3         "Symbol %s has null value.\n"
SYMTAB_MSG_4         "0452-353: No namelist file opened.\n"
SYMTAB_MSG_5         "read error in namelist file"
SYMTAB_MSG_6         "0452-355: No symbol table found in %s.\n"
SYMTAB_MSG_7         "Read error on symbol:%d, errno:%d.\n"
SYMTAB_MSG_16        "Segment %d has become invalid for currently mapped process\n"
SYMTAB_MSG_17        "Invalid mode"
SYMTAB_MSG_18        "warning: word alignment performed\n"
SYMTAB_MSG_19        "  read error"
SYMTAB_MSG_20        "warning: word alignment performed\n"
SYMTAB_MSG_21        "  read error"
SYMTAB_MSG_22        "  read error"
SYMTAB_MSG_23        "warning: word alignment performed\n"
SYMTAB_MSG_24        "  read error"
SYMTAB_MSG_25        "cannot open namelist file\n"
SYMTAB_MSG_26        "cannot dup namelist file"
SYMTAB_MSG_27        "WARNING: dumpfile does not appear to match namelist\n"
SYMTAB_MSG_30	     "Segment %d has become invalid for currently mapped thread\n"

SYSVAD_MSG_1         "Kernel not included in this dump.\n"
SYSVAD_MSG_2         "Not a valid dump data area:0x%08x\n"
SYSVAD_MSG_3         "Address 0x%08x: page not in dump. Padding with 0x%08x.\n"
SYSVAD_MSG_4         "Dump data incomplete.Only %d bytes found out of %d.\n"
SYSVAD_MSG_5         "0452-374: Parameters to fill not valid.\n"
SYSVAD_MSG_6         "0452-375: Parameters to fill not valid.\n"

TTY_MSG_3            "0452-376: Cannot execute %s, errno = %d.\n"
TTY_MSG_4            "0452-377: Cannot fork %s, errno = %d.\n"

XDUMPX_MSG_2         "   %d lines same as above\n"
XDUMPX_MSG_3         "   %d line same as above\n"
XMALLOC_MSG_1        "Kernel heap usage \n"
XMALLOC_MSG_2        "Pinned heap usage \n"
XMALLOC_MSG_3        "Kernel and pinned heap usage \n"
XMALLOC_MSG_4        "0452-412: Cannot read pointer to heap at address 0x%8x\n"
XMALLOC_MSG_5        "0452-413: Cannot read heap from address 0x%8x\n"
XMALLOC_MSG_6        "heap size = %8d amount used = %8d  \n"
XMALLOC_MSG_7        "0452-415: Cannot read hash table at address 0x%8x\n"
XMALLOC_MSG_8        "0452-416: Cannot read record at address 0x%8x.\n"
XMALLOC_MSG_9        "from = %8x bytes = %8d number = %8d \n"
XMALLOC_MSG_10       "Not enough mstat structures.\n"

LE_MSG_1	     "Cannot read kernel loader anchor address from address 0x%08x\n"
LE_MSG_2	     "No loader entry found for module address 0x%08x\n"
LE_MSG_3	     "Cannot read loader entry from address 0x%08x\n"
LE_MSG_4	     "Cannot read file name from address 0x%08x\n"

PRINT_MSG_1	     "Usage: print <type> <address>\n"
ERRPT_MSG_2	     "\nLAST %d ERRORS READ BY ERRDEMON (MOST RECENT FIRST):\n"

STAB_MSG_1	     "%s: out of memory!\n"
STAB_MSG_2	     "WARNING: pointer in field definition\n"
STAB_MSG_3	     "type definition not found\n"
STAB_MSG_4	     "%s: no debug/include file(s) specified\n"
STAB_MSG_5	     "no debug info found (bad magic)\n"

NDB_MSG_1            "Cannot find tcb\n"
NDB_MSG_2            "Cannot find udb\n"
NDB_MSG_3            "It looks like ifnet isn't loaded, goodbye\n"
NDB_MSG_4            "type ? for help\n"
NDB_MSG_5             "ndb: error reading input, goodbye.\n"
NDB_MSG_6             "that wasn't a unique command\n"
NDB_MSG_7             "unknown command\n"
NDB_MSG_8             "Invalid argument: %s\n"
NDB_MSG_9             "Can't read file structure\n"
NDB_MSG_10            "File[%d] is not a socket\n"
NDB_MSG_11            "An address is required\n"
NDB_MSG_12            "the send buffer has no mbufs\n"
NDB_MSG_13            "the receive buffer has no mbufs\n"
NDB_MSG_14            "\nEnd of NDB\n"
NDB_MSG_15      	"Welcome to ndb, a network debugger for network kernel structures\n"
NDB_MSG_16            "\n\nHelp stuff: \n\n"
NDB_MSG_17            "invalid mbuf ptr:%8x, must be on %d byte boundary\n"
NDB_MSG_18            "at end of ifnet chain\n"
NDB_MSG_19            " %s%s----------- INPCB  INFO -------------------\n"
NDB_MSG_20            "----------------- SOCKET INFO -------------------\n"
NDB_MSG_21            "---------------------- TCPCB ---------------------\n"
NDB_MSG_22    "    ---------------------- IFNET INFO -----(@0x%08x)----------\n"
NDB_MSG_23    "Not a valid hexadecimal number for an address\n"
NDB_MSG_24    "At any prompt type a '?' for context sensitive help.\n"
NDB_MSG_25    "There are two main functions currently supported by ndb:\n"
NDB_MSG_26    "\t* show sockets and related control blocks and buffers,\n"
NDB_MSG_27    "allowing you to move back and forth along the chain of control blocks\n"
NDB_MSG_28    "\t* show interface structures\n"
NDB_MSG_29    "following the chain from the head to the interface list\n"
NDB_MSG_30    "THINGS TO REMEMBER :\n"
NDB_MSG_31    " - When you hit ENTER, the option menu will be displayed.\n\n"
NDB_MSG_32    "at end of act chain\n"
NDB_MSG_33    "at end of mbuf list\n"

CHNGMAP_MSG_1        	"0452-500: Thread slot %d above maximum limit.Specify a value less than %d.\n"
CHNGMAP_MSG_2        	"0452-501: Thread Table Slot %d not in use or zombie.\n"
CHNGMAP_MSG_3        	"0452-502: Cannot read uarea for this thread."
CHNGMAP_MSG_4       	"0452-503: Segment %d above maximum limit.\n\
\tSpecify a value that is greater than 0 and less than 16\n"
CHNGMAP_MSG_5        	"0452-504: Segment %d not in use by thread %d\n"
CHNGMAP_MSG_6		"0452-505: Cannot read thread table entry %3d.\n"
CHNGMAP_MSG_7		"0452-506: Cannot read uarea for thread table entry %3d.\n"

CALLOUT_MSG_1		"TRB's On The Active List Of Processor %d.\n"
CALLOUT_MSG_2		"0452-552: Cannot read ppda structure at address 0x%08x\n"
CALLOUT_MSG_3		"0452-553: This processor is disabled\n"
CALLOUT_MSG_4		"0452-554: There are no TRB's on the active list of this processor.\n"
CALLOUT_MSG_5		"0452-555: Cannot read trb structure at address 0x%8x\n"
CALLOUT_MSG_6		"TRB #%d on Active List\n\n"
CALLOUT_MSG_7		" Timer address......................0x%x\n"
CALLOUT_MSG_8		" Thread id (-1 for dev drv).........0x%x\n"
CALLOUT_MSG_9		" Timer flags........................0x%x\n"
CALLOUT_MSG_10		" Next scheduled timeout (secs)......0x%x\n"
CALLOUT_MSG_11		" Next scheduled timeout (nanosecs)..0x%x\n"
CALLOUT_MSG_12		" Timeout function...................0x%x\n"
CALLOUT_MSG_13		" Timeout function data..............0x%x\n"

FSTACK_MSG_4         	"STACK TRACE:\n"
FSTACK_MSG_5         	"\n\t**** no match ****\n"
FSTACK_MSG_6      	"0452-074: Frame pointer 0x%8x not valid.\n"
FSTACK_MSG_7       	"0452-075: Cannot read frame pointer at address 0x%8x.\n"
FSTACK_MSG_8       	"0452-076: Cannot read frame pointer at address 0x%8x.\n"
FSTACK_MSG_9       	"0452-077: Frame pointer 0x%8x not valid.\n"
FSTACK_MSG_10      	"0452-078: Cannot read frame pointer at address 0x%8x.\n"
FSTACK_MSG_11      	"0452-079: Cannot read frame pointer at address 0x%8x.\n"
FSTACK_MSG_12      	"0452-080: Cannot read return address at address 0x%8x.\n"
FSTACK_MSG_13      	"0452-081: Cannot read return address at address 0x%8x.\n"
FSTACK_MSG_14      	"IAR not in kernel segment.\n"
FSTACK_MSG_15       	"\n\t**** no match ****\n"
FSTACK_MSG_18       	"0452-084: Cannot read frame pointer at address 0x%8x.\n"
FSTACK_MSG_19       	"0452-085: Cannot read frame pointer at address 0x%8x.\n"
FSTACK_MSG_20		"0452-600: Thread slot %d above maximum limit.Specify a value less than %d.\n"
FSTACK_MSG_21		"0452-601: Cannot read uarea for this thread.\n"
FSTACK_MSG_22		"0452-602: Thread is currently swapped out.\n"
FSTACK_MSG_23		"0452-603: Cannot read thread structure.\n"
FSTACK_MSG_24 		"0452-604: Cannot find associated process.\n"

MISC_MSG_1              "Parameter supplied is not numeric.\n"
MISC_MSG_2           	"0452-178: Cannot open dump file %s.\n"
MISC_MSG_3           	"0452-179: Cannot read v structure from address 0x%8x.\n"
MISC_MSG_4		"0452-180: Cannot read the number of cpus at address 0x%08x\n"
MISC_MSG_5           	"0452-181: Cannot read process table entry %d.\n"
MISC_MSG_8           	"0452-184: Cannot read uarea for this process.\n"
MISC_MSG_9           	"0452-185: Cannot read uarea for this process.\n"
MISC_MSG_10		"0452-800: Cannot read the machine architecture at address 0x%08x\n"
MISC_MSG_11		"0452-801: Cannot find a default cpu. Use the cpu command to select one\n"
MISC_MSG_15		"0452-805: Cannot read current thread slot\n"
MISC_MSG_16		"0452-806: Cannot read current process slot\n"
MISC_MSG_17		"0452-807: Cannot read uthread area for this thread.\n"
MISC_MSG_18		"0452-808: Cannot read uthread area for this thread.\n"

MP_MSG_1		"Per Processor Data Area for processor %2d\n"
MP_MSG_2		"0452-900: Processor number not valid. This machine has %2d processors\n"
MP_MSG_3		"0452-901: Cannot read ppda structure at address 0x%08x\n"
MP_MSG_4		"This processor is disabled\n"
MP_MSG_6		"0452-905: Processor number is not valid. This machine has %2d processors\n"
MP_MSG_7		"0452-902: Cannot read ppda structure at address 0x%08x\n"
MP_MSG_8		"Processor %2d is disabled\n"
MP_MSG_9		"0452-903: Cannot read thread structure.\n"
MP_MSG_10		"0452-904: Cannot read process structure.\n"
MP_MSG_11		"ZOMBIE"
MP_MSG_12		"swapper"
MP_MSG_13		"0452-908: Cannot read user structure.\n"

MST_MSG_1	     	"Cannot read csa at 0x%08x \n"
MST_MSG_2	     	"Mst at 0x%08x paged out \n"
MST_MSG_3	     	"Cannot read mst at 0x%08x \n"
MST_MSG_4		"    Exception Struct\n"
MST_MSG_5 		"    Segment Regs\n    "
MST_MSG_6		"General Purpose Regs\n    "
MST_MSG_7		"    Floating Point Regs\n    "
MST_MSG_8		"0452-850: Cannot read csa at 0x%08x \n"
MST_MSG_9		"                MSTSAVE AREA AT ADDRESS 0x%08x\n\n"


PRINTU_MSG_0		"\tUTHREAD AREA FOR SLOT %3d\n\n"
PRINTU_MSG_1		"SAVED MACHINE STATE \n"
PRINTU_MSG_2		"    Exception Struct\n"
PRINTU_MSG_3		"    Segment Regs\n    "
PRINTU_MSG_4		"General Purpose Regs\n    "
PRINTU_MSG_5		"Floating Point Regs\n    "
PRINTU_MSG_6		"\n\nKernel stack address: 0x%08x\n"
PRINTU_MSG_7		"\nSYSTEM CALL STATE \n"
PRINTU_MSG_8		"\nPER-THREAD TIMER MANAGEMENT \n"
PRINTU_MSG_9		"\nSIGNAL MANAGEMENT \n"
PRINTU_MSG_10		"\nUSER INFORMATION \n"
PRINTU_MSG_11		"    CRED Structure Not Found\n"
PRINTU_MSG_12		"\nMISCELLANOUS FIELDS:\n"
PRINTU_MSG_13		"\tUSER AREA OF ASSOCIATED PROCESS %s (SLOT %3d, PROCTAB 0x%08x)\n"
PRINTU_MSG_14		"\nSIGNAL MANAGEMENT \n"
PRINTU_MSG_15		"    Signals to be blocked (sig#:hi/lo mask,flags,&func)\n    "
PRINTU_MSG_17		"\nACCOUNTING DATA \n"
PRINTU_MSG_18		"\nCONTROLLING TTY \n"
PRINTU_MSG_19		"PINNED PROFILING BUFFER\n"
PRINTU_MSG_20		"\nRESOURCE LIMITS AND COUNTERS\n"
PRINTU_MSG_21		"\nAUDITING INFORMATION \n"
PRINTU_MSG_22		"\nSEGMENT REGISTER INFORMATION \n"
PRINTU_MSG_23		"    Reg        Flag      Fileno     Pointer  \n"
PRINTU_MSG_24		"FILE SYSTEM STATE \n"
PRINTU_MSG_25		"FILE DESCRIPTOR TABLE\n"
PRINTU_MSG_26		"Rest of user area paged out. \n"
PRINTU_MSG_27           "0452-204: Cannot read cred structure at address 0x%8x.\n"
PRINTU_MSG_28           " still_used"
PRINTU_MSG_29           " sc_to_sigslih"
PRINTU_MSG_30           " no_ulimit"
PRINTU_MSG_31           " sig_on_stack"

PROC_MSG_1        	"Process slot %d above maximum limit.Specify a value less than %d.\n"
PROC_MSG_5		"a"
PROC_MSG_6		"w"
PROC_MSG_8      	"i"
PROC_MSG_9      	"z"
PROC_MSG_10      	"t"
PROC_MSG_11          	"?"
PROC_MSG_13          	"ZOMBIE"
PROC_MSG_14          	"swapper"
PROC_MSG_15          	" Cannot read uarea "
PROC_MSG_16          	"Misc:  adspace:0x%08x  *ttyl:0x%08x\n"
PROC_MSG_17          	"Statistics:  size:0x%08x(pages)  audit:0x%08x\n"
PROC_MSG_18          	"\tFLAGS:%s\n"
PROC_MSG_20_OLD		"Dispatch Fields:  pevent:0x%08x  wevent:0x%08x\n"
$ This message no longer used.  It is being replaced by PROC_MSG_20.
PROC_MSG_21		"Thread Fields:  *threadlist:0x%08x  threadcount:%5d \n"
PROC_MSG_22_OLD		"   active:%5d  suspended:%5d  local:%5d  localsleep:%5d \n"
$ This message no longer used.  It is being replaced by PROC_MSG_22.
PROC_MSG_23		"Scheduler Fields:  "
PROC_MSG_24		" fixed pri:%3u"
PROC_MSG_25		"Signal Information: \n"
PROC_MSG_27		" swapped_in"
PROC_MSG_28		" no_swap"
PROC_MSG_29		" special_fork_stack"
PROC_MSG_30		" trace"
PROC_MSG_31		" fixed_pri"
PROC_MSG_32		" kproc"
PROC_MSG_33		" terminating"
PROC_MSG_34		" execing"
PROC_MSG_35		" locks"
PROC_MSG_36		" debug"
PROC_MSG_37		" exiting"
PROC_MSG_38		" suspended"
PROC_MSG_39		" get_out"
PROC_MSG_40		" restarted"
PROC_MSG_41		" psm_kill"
PROC_MSG_42		" suspended_umode"
PROC_MSG_43		" sig_slih"
PROC_MSG_44		" stopped_wh_traced"
PROC_MSG_45		" stopped_af_fork_wh_traced"
PROC_MSG_46		" stopped_af_exec_wh_traced"
PROC_MSG_47		" stopped_af_load_wh_traced"

PSTAT_MSG_1		"USAGE: pstat [-a] [-A] [-f] [-i] [-p] [-P] [-s] [-t] [-u procslot] [-U threadslot] [-S] [-T] [[System] CoreFile]\n"
PSTAT_MSG_2		"SYSTEM VARS:\n\n"
PSTAT_MSG_3		"PROC TABLE:\n\n"
PSTAT_MSG_4		"THREAD TABLE:\n\n"
PSTAT_MSG_5		"FILE TABLE:\n\n"
PSTAT_MSG_6		"SLOT  REF     INODE    FLAGS\n"
PSTAT_MSG_7		"INODE TABLE:\n\n"
PSTAT_MSG_8		"PAGE SPACE:\n\n"
PSTAT_MSG_9		"\t  USED PAGES   FREE PAGES\n"
PSTAT_MSG_10		"0452-326: Cannot read vmkerdata structure from address 0x%8x\n"
PSTAT_MSG_11		"TTY STRUCTS:\n\n"
PSTAT_MSG_12		"USER AREA:\n\n"
PSTAT_MSG_13		"UTHREAD AREA:\n\n"
PSTAT_MSG_14		"STATUS OF PROCESSORS:\n\n"


THREAD_MSG_1		"0452-950: Thread slot %d above maximum limit.Specify a value less than %d.\n"
THREAD_MSG_2		"SLT ST    TID      PID    CPUID  POLICY PRI CPU    EVENT  PROCNAME     FLAGS\n"
THREAD_MSG_3		"i"
THREAD_MSG_4		"r"
THREAD_MSG_5		"w"
THREAD_MSG_6		"s"
THREAD_MSG_7		"t"
THREAD_MSG_8		"z"
THREAD_MSG_10		"?"
THREAD_MSG_11 		"0452-951: Cannot read uarea for the process associated to this thread.\n"
THREAD_MSG_12		" local"
THREAD_MSG_13		" sigslih"
THREAD_MSG_14		" trcsig"
THREAD_MSG_15		" omask"
THREAD_MSG_16		" sigdelivery"
THREAD_MSG_17		" term"
THREAD_MSG_18		" sel"
THREAD_MSG_19		" prioboost"
THREAD_MSG_20		" susp"
THREAD_MSG_21		" sig_avail"
THREAD_MSG_22		" assert_sig"
THREAD_MSG_23		" funnel"
THREAD_MSG_24		" kthread"
THREAD_MSG_25		" setstate"

THRDUTIL_MSG_1		"0452-1000: Cannot read thread table entry %3d.\n"
THRDUTIL_MSG_2		"0452-1001: Thread entry %d is not in the dump.\n"
THRDUTIL_MSG_3		"0452-1002: Cannot read extension segment value from address 0x%8x\n"
THRDUTIL_MSG_4		"0452-1003: Cannot read thread table entry %3d.\n"
THRDUTIL_MSG_5		"0452-1004: Cannot read process table entry %d.\n"
THRDUTIL_MSG_6		"0452-1005: Cannot read extension segment value from address 0x%8x.\n"
THRDUTIL_MSG_7		"0452-1006: Cannot read proc table entry %d.\n"
THRDUTIL_MSG_8		"0452-1007: Cannot read ppda structure from address 0x%8x\n"

U_MSG_1			"0452-750: Process slot %d above maximum limit.Specify a value less than %d.\n"
U_MSG_2			"0452-751: \nSlot of process associated to the thread was not found.\n"
U_MSG_3			"0452-752: Cannot read uarea for this process.\n"
U_MSG_4			"0452-753: Process is currently swapped out.\n"
U_MSG_5			"0452-754: Thread slot %d above maximum limit.Specify a value less than %d.\n"
U_MSG_6			"0452-755: Cannot read uthread area  for this thread.\n"
U_MSG_7			"Thread is currently swapped out.\n"
U_MSG_8			"0452-757: Process slot %d above maximum limit.Specify a value less than %d.\n"
U_MSG_9			"0452-758: Slot of process associated to the thread was not found.\n"
U_MSG_10		"Struct user of associated process (slot %3d):\n"
U_MSG_11		"0452-759: Cannot read uarea for this process.\n"
U_MSG_12              	"Process is currently swapped out.\n"
U_MSG_13		"Uthread structure of thread slot %3d :\n"
U_MSG_14		"0452-760: Thread slot %d not in right range. Specify a value less than %d and positive.\n"
U_MSG_15		"0452-761: Cannot read uarea for this thread.\n"
U_MSG_16		"0452-762: Thread is currently swapped out.\n"
U_MSG_17		"0452-763: Thread slot %d above maximum limit. Specify a value less than %d.\n"
U_MSG_18		"0452-764: Cannot read uarea for this thread.\n"
U_MSG_19		"Thread is currently swapped out.\n"
U_MSG_20                "KERNEL STACK:\n"
U_MSG_21		"0452-766: Cannot read thread structure.\n"
U_MSG_22		"0452-767: Cannot find associated process.\n"
U_MSG_23		"0452-765: Frame pointer not valid.\n"
U_MSG_24		"0452-768: Cannot read kernel stack at address 0x%8x.\n"
U_MSG_25		"0452-769: Cannot read kernel stack at address 0x%8x.\n"
U_MSG_26		"\tno match\n"
U_MSG_27		"0452-770: Frame pointer not valid.\n"
U_MSG_28		"0452-771: Cannot read return address at address 0x%8x.\n"
U_MSG_29		"0452-772: Cannot read frame pointer at address 0x%8x.\n"
U_MSG_30		"0452-773: Cannot read frame pointer at address 0x%8x.\n"
U_MSG_31                "0452-774: Cannot read return address at address 0x%8x.\n"
U_MSG_32                "IAR not in kernel segment.\n"
U_MSG_33		"MST STACK TRACE:\n"
U_MSG_34		"0452-775: Cannot read ppda structure from address 0x%08x\n"
U_MSG_35		"0452-776: Cannot read csa at 0x%08x \n"
U_MSG_36		"0452-777: Cannot read uarea for this thread.\n"
U_MSG_37		"0452-778: Thread is currently swapped out.\n"
U_MSG_38		"0452-779: Cannot read thread structure.\n"
U_MSG_39		"0452-780: Cannot find associated process.\n"
U_MSG_40		"0452-781: Mst at 0x%08x paged out \n"
U_MSG_41		"0452-782: Cannot read mst at 0x%08x \n"
U_MSG_42		"KFP STACK TRACE:\n"
U_MSG_43		"STACK TRACE:\n"
U_MSG_44		"0452-783: Thread slot %d above maximum limit. Specify a value less than %d.\n"
U_MSG_45		"0452-784: Cannot read uarea for this thread.\n"
U_MSG_46		"Thread is currently swapped out.\n"
U_MSG_47		"0452-785: Cannot read thread structure.\n"
U_MSG_48		"0452-786: Cannot find associated process.\n"


DLA_MSG_1		"0452-1051: Out of memory\n"
DLA_MSG_2		"0452-1052: Out of lock descriptors\n"
DLA_MSG_3		"No deadlock found\n"
DLA_MSG_4		"No waited lock on cpu %d\n"
DLA_MSG_5		"0452-1055: This is not a running cpu\n"
DLA_MSG_6		"No waited lock for thread %x\n"
DLA_MSG_7		"0452-1057: Bad argument for deadlock analyzer\n"
DLA_MSG_8 		"Deadlock from cpu %d. This cpu waits for the first line lock,\n"
DLA_MSG_9 		"Deadlock from tid %05x. This tid waits for the first line lock,\n"
DLA_MSG_10 		"No deadlock, but chain from cpu %d, that waits for the first line lock,\n"
DLA_MSG_11 		"No deadlock, but chain from tid %05x, that waits for the first line lock,\n"
DLA_MSG_12 		"owned by Owner-Id that waits for the next line lock, and so on ...\n"
DLA_MSG_13		"     LOCK NAME    |   ADDRESS  |  OWNER-ID | LOCK STATUS | WAITING FUNCTION \n"
DLA_MSG_14 		"0452-1064: Cannot access this thread\n"
DLA_MSG_15 		"0452-1065: Cannot set curslot, no curthread on cpu %d\n"

STREAM_MSG_1		"No open streams are available on the system.\n"
$ Do not translate streams.
STREAM_MSG_2		"0452-1100: Cannot read the stream head table.\n"
$ Do not translate "stream head".
STREAM_MSG_3 		" "
$ STREAM_MSG_3 is no longer used
STREAM_MSG_4		"0452-1102: Cannot read sthq_s structure.\n"
$ Do not translate sthq_s.
QUEUE_MSG_1		"0452-1110: The specified address is not a valid queue address.\n"
$ Do not translate queue.
QUEUE_MSG_2		"No queues are found in the system.\n"
$ Do not translate queues.
QUEUE_MSG_3		"0452-1111: Cannot read the queue structure.\n"
$ Do not translate queue.
MBLOCK_MSG_1		"0452-1120: This subcommand requires an address.\n"
MBLOCK_MSG_2		"0452-1121: The specified address is not a valid msgb address.\n"
$ Do not translate msgb.
MBLOCK_MSG_3		"0452-1122: Cannot locate the msgb structure.\n"
$ Do not translate msgb.
DBLOCK_MSG_1		"0452-1130: The specified address is not a valid datab address.\n"
$ Do not translate datab.
DBLOCK_MSG_2		"0452-1131: Cannot locate the datab structure.\n"
$ Do not translate datab.
QRUN_MSG_1		"No queues are scheduled for service.\n"
$ Do not translate queues.
QRUN_MSG_2		" "
$ QRUN_MSG_2 is no longer used.
QRUN_MSG_3		"0452-1141: Cannot read streams_runq.\n"
$ Do not translate streams_runq.
LINKBLK_MSG_1		"No linkblk structure can be found on the system.\n"
$ Do not translate linkblk.
DMODSW_MSG_1		"No STREAMS drivers are loaded on the system.\n"
$ Do not translate STREAMS.
DMODSW_MSG_2		"0452-1150: Cannot locate the modsw structure.\n"
$ Do not translate modsw.
FMODSW_MSG_1		"No STREAMS modules are loaded on the system.\n"
$ Do not translate STREAMS.

PROC_MSG_20		"Dispatch Fields:  pevent:0x%08x \n"
PROC_MSG_22		"   active:%5d  suspended:%5d  local:%5d \n"
$ Following message added for PRVNODE
PRVNODE_MSG_21		"0452-288: Cannot read vnode list anchor at address 0x%08x.\n"

MISC_MSG_19		"0452-810: Cannot read %1$s variable from address 0x%2$8x.\n"
$ Substitution 1 is a variable name that will never be translated

QRUN_MSG_4		"0452-1142: Cannot read the sqe_s structure.\n"
$ Do not translate sqe_s.
PROC_MSG_26       " orphanpgrp"

$ Do not translate the first 15 characters (spaces included) 
$ in the following NDB_MSG_xx message (NDB_MSG_34 thru NDB_MSG_57).
$ For examples, "?" in NDB_MSG_34, "help" in NDB_MSG_35, "tcb [addr]" in NDB_MSG_36.
NDB_MSG_34        "?              help - you're reading it\n"
NDB_MSG_35        "help           a more lengthy help text\n"
NDB_MSG_36        "tcb [addr]     show TCBs, default is HEAD TCB\n"
NDB_MSG_37        "udb [addr]     show UDBs, default is HEAD UDB\n"
NDB_MSG_38        "sockets addr   show sockets at given addr\n"
NDB_MSG_39        "mbuf [addr]    show mbuf at address, default is enter mbuf menu\n"
NDB_MSG_40        "ifnet [addr]   show ifnet structures, default is \"ifnet\"\n"
NDB_MSG_41        "quit           exit ndb\n"
NDB_MSG_42        "xit            exit ndb\n"
NDB_MSG_43        "next           go to next tcb/socket\n"
NDB_MSG_44        "previous       go to previous tcb/socket\n"
NDB_MSG_45        "tcb [addr]     reread tcb/socket, default is current\n"
NDB_MSG_46        "ctrlblk [addr] read the control block\n"
NDB_MSG_47        "sendmbufs      show the send mbufs\n"
NDB_MSG_48        "recvmbufs      show the receive mbufs\n"
NDB_MSG_49        "sock addr      show socket at given address\n"
NDB_MSG_50        "head           show the head tcb in the chain\n"
NDB_MSG_51        "quit           return to top_level\n"
NDB_MSG_52        "next           go to next udb\n"
NDB_MSG_53        "previous       go to previous udb\n"
NDB_MSG_54        "head           go to head of ifnet list\n"
NDB_MSG_55        "next           go to next ifnet struct\n"
NDB_MSG_56        "socket addr    show socket at addr\n"
NDB_MSG_57        "addr           show mbuf at address\n"

$ Symptom generation messages
SYMP_MSG_1	"Unable to get the dump code or the csa value.\n"
$ Don't translate csa
SYMP_MSG_2	"Symptom strings can only be generated against a dump.\n"
SYMP_MSG_3	"Usage:  symptom [-e] (-e puts the symptom in the error log.)\n"
SYMP_MSG_4	"You must be root to log a symptom.\n"
SYMP_MSG_5	"Symptom criteria was not met.\n"
$ This means that some piece of data for generating a symptom
$ string was invalid or missing.
SYMP_MSG_6	"This dump was caused by a paging device I/O error.\n"
SYMP_MSG_7	"Could not retrieve the data to produce a symptom string.\n"

