/* @(#)73  1.10  src/bos/usr/sbin/sa/sa.h, cmdstat, bos411, 9428A410j 6/3/94 12:57:08 */
/*
*/
/*
 * COMPONENT_NAME: (CMDSTAT) status
 *
 * FUNCTIONS:
 *
 * ORIGINS: 3, 27 83
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1994
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * LEVEL 1, 5 Years Bull Confidential Information
 */
/*
 *
 * Copyright 1976, Bell Telephone Laboratories, Inc.
 */

/*
	sa.h contains struct sa and defines variable used 
		in sadc.c and sar.c.
	The nlist setup table in sadc.c is arranged as follows:
	For VAX and PDP11 machines,
		- device status symbol names for controllers with 8 drives ,
			headed by _hpstat. 
			(not include the _gdstat)
		- device status symbol names for controllers with 4 drives,
			headed by _rlstat. 
		- device status symbol names for controllers with 1 drive,
			headed by _tsstat. 
		- general disk driver status system name.
		- symbol name for _sysinfo.
		- symbol names for system tables: inode, file,
			text and process.
		- symbol name for _var.
		- symbol name for _rl_cnt.
		- symbol name for _gd_cnt.
		- symbol name for system error table:

	For 3b20S system,
		- symbol name of dskinfo table
		- symbol name of MTC tape drive.
		- symbol name for _sysinfo.
		- symbol names for system tables: inode, file,
			text and process.
		- symbol name for _var.
		- symbol name for dsk_cnt.
		- symbol name for mtc_cnt.
		- symbol name for system error table:
	For IBM 370 system,
		- symbol name for sysinfo.
		- symbol names for system tables: inode, file,
			text and process.
		- symbol name for var.
		- symbol name for system error table:
			Note that this is always the last entry in setup
			table, since the number of entries of setup table
			is used in sadc.c.
*/
 
#include <a.out.h>
#include <stdio.h>

/*
	The following variables define the positions of symbol
	table names in setup table:
*/
 
#define  DISKINFO         0
#define  TAPEINFO         1
#define  SINFO            2
#define  INO              3
#define  FLE              4
#define  TXT              5
#define  PRO              6
#define  V                7
#define  DISKCNT          8
#define  TAPECNT          9
#define	 VMKER           10
#define	 VMINFO          11
#define  THREAD		 12
#define  CPUINFO	 13
#define  NLIST_ELEMENTS  14

struct nlist  nl[] = {
	{"fsv"},
	{NULL},
};

struct nlist setup[] = {
	{"dskinfo"},
	{"tapinfo"},
	{"sysinfo"},
	{"inode"},
	{"file"},
	{"text"},
	{"proc"},
	{"v"},
	{"diskcnt"},
	{"tapecnt"},
	{"vmker"},
	{"vmminfo"},
	{"thread"},
	{"cpuinfo"},
	{0},
};

char devnm[SINFO][6] = {
	"disk-",
	"tape-"
};

#define NDEVS 20    /* Until a better estimate comes along */
 
/*
	structure sa defines the data structure of system activity data file
*/
 
struct paginginfo {
	unsigned int    freeslots;
	unsigned int    cycles;
	unsigned int    faults;
	unsigned int    otherdiskios;
};

#define N_PROCESSOR 8 
#define GLOBAL N_PROCESSOR  /* index of stat global */

/*
 * This is borrowed from <sys/sysinfo.h> and must be kept in synch.
 */

struct flt_sysinfo {
        double  cpu[CPU_NTIMES];
        double  bread;
        double  bwrite;
        double  lread;
        double  lwrite;
        double  phread;
        double  phwrite;
        double  pswitch;
        double  syscall;
        double  sysread;
        double  syswrite;
        double  sysfork;
        double  sysexec;
        double  runque;
        double  runocc;
        double  swpque;
        double  swpocc;
        double  iget;
        double  namei;
        double  dirblk;
        double  readch;
        double  writech;
        double  rcvint;
        double  xmtint;
        double  mdmint;
        struct  ttystat ttystat;
        double  msg;
	double  sema;
        double  ksched;
        double  koverf;
        double  kexit;
        double  rbread;
        double  rcread;
        double  rbwrt;
        double  rcwrt;
        double  devintrs;
        double  softintrs;
        double  traps;
};

struct flt_paging {
        double    freeslots;
        double    cycles;
        double    faults;
        double    otherdiskios;
	};



struct sa {
	struct sysinfo     si[N_PROCESSOR + 1];
			   /* defined in /usr/include/sys/sysinfo.h  */
	struct paginginfo  pi;   /* defined above */
	int     szinode;         /* current size of inode table  */
	int     szfile;          /* current size of file table  */
	int     sztext;          /* current size of text table  */
	int     szproc;          /* current size of proc table  */
	int     szthread;        /* current size of thread table  */
	int     mszinode;        /* maximum size of inode table  */
	int     mszfile;         /* maximum size of file table  */
	int     msztext;         /* maximum size of text table  */
	int     mszproc;         /* maximum size of proc table  */
	int	mszthread;	 /* maximum size of thread table */
	time_t  ts;              /* time stamp  */

	long    devio[NDEVS][4]; /* device unit information  */

#define	 IO_OPS   0  /* number of I /O requests since boot */
#define	 IO_BCNT  1  /* number of blocks transferred since boot */
#define	 IO_ACT   2  /* cumulative time in ticks when drive is active */
#define	 IO_RESP  3  /* cumulative I/O response time in ticks since boot */
};

struct sa_float {
        struct  flt_sysinfo si[N_PROCESSOR + 1];  /* must match (struct sysinfo) by name */
        struct  flt_paging pi;   /* defined above */
        int     szinode;         /* current size of inode table  */
        int     szfile;          /* current size of file table  */
        int     sztext;          /* current size of text table  */
        int     szproc;          /* current size of proc table  */
	int     szthread;        /* current size of thread table  */
        int     mszinode;        /* maximum size of inode table  */
        int     mszfile;         /* maximum size of file table  */
        int     msztext;         /* maximum size of text table  */
        int     mszproc;         /* maximum size of proc table  */
	int	mszthread;	 /* maximum size of thread table */
        time_t  ts;              /* time stamp  */

        long    devio[NDEVS][4]; /* device unit information  */

#define  IO_OPS   0  /* number of I /O requests since boot */
#define  IO_BCNT  1  /* number of blocks transferred since boot */
#define  IO_ACT   2  /* cumulative time in ticks when drive is active */
#define  IO_RESP  3  /* cumulative I/O response time in ticks since boot */
};


extern struct sa  sa;

struct sa_old {
	struct sysinfo     sa_old_si; /* defined in /usr/include/sys/sysinfo.h  */
	struct paginginfo  sa_old_pi;   /* defined above */
	int     sa_old_szinode;         /* current size of inode table  */
	int     sa_old_szfile;          /* current size of file table  */
	int     sa_old_sztext;          /* current size of text table  */
	int     sa_old_szproc;          /* current size of proc table  */
	int     sa_old_mszinode;        /* maximum size of inode table  */
	int     sa_old_mszfile;         /* maximum size of file table  */
	int     sa_old_msztext;         /* maximum size of text table  */
	int     sa_old_mszproc;         /* maximum size of proc table  */
	long    sa_old_inodeovf;        /* cumulative overflows of inode table
					    since boot  */
	long    sa_old_fileovf;         /* cumulative overflows of file table
					    since boot  */
	long    sa_old_textovf;         /* cumulative overflows of text table
					    since boot  */
	long   sa_old_procovf;         /* cumulative overflows of proc table
					    since boot  */
	time_t  sa_old_ts;              /* time stamp  */

	long    sa_old_devio[NDEVS][4]; /* device unit information  */

#define	 IO_OPS   0  /* number of I /O requests since boot */
#define	 IO_BCNT  1  /* number of blocks transferred since boot */
#define	 IO_ACT   2  /* cumulative time in ticks when drive is active */
#define	 IO_RESP  3  /* cumulative I/O response time in ticks since boot */
};


#define SAR_MAGIC  0x12F00124
#define CPU_STOPPED -1 /* this value is used to initialize the cpu which are
				not running */

struct sa_head {
		int magic;
		int nbcpu;
	    };

