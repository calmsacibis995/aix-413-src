/* @(#)71	1.2  src/rspc/usr/lib/boot/softros/aixmon/nvram.h, rspc_softros, rspc411, GOLD410 6/13/94 17:20:05  */
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: NT
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

// f389  -  5/23/94  -  Dakoarch nvram.h dated 5/20/94. syy

/* Structure map for NVRAM on PowerPC Reference Platform */

/* All fields are either character/byte strings which are valid either
endian or they are big-endian numbers.  The RESTART structure is in
whatever endian stored it and it is marked as to which it is.  It is
expected that little endian OSs will map the whole NVRAM into a form
suitable for it by flipping the bytes as necessary for the numbers.

There are a number of Date and Time fields which are in RTC format,
big-endian. These are stored in UT (GMT).

For enum's: if given in hex then they are bit significant, i.e. only
one bit is on for each enum.
*/

#ifndef _NVRAM_
#define _NVRAM_

#define NVSIZE 4096       /* size of NVRAM */
#define OSAREASIZE 512    /* size of OSArea space */
#define CONFSIZE 1024     /* guess at size of Configuration space */

/* The SECURITY fields are maintained by the firmware which should
be the only code that writes the fields. Each operating system may
supply a program which displays their values for the user. These fields
really belong in EEPROM so they are not lost on battery problems
and are secure. */

typedef struct _SECURITY {
  unsigned long BootErrCnt;         /* Count of boot password errors */
  unsigned long ConfigErrCnt;       /* Count of config password errors */
  unsigned long BootErrorDT[2];     /* Date&Time from RTC of last error in pw */
  unsigned long ConfigErrorDT[2];   /* Date&Time from RTC of last error in pw */
  unsigned long BootCorrectDT[2];   /* Date&Time from RTC of last correct pw */
  unsigned long ConfigCorrectDT[2]; /* Date&Time from RTC of last correct pw */
  unsigned long BootSetDT[2];       /* Date&Time from RTC of last set of pw */
  unsigned long ConfigSetDT[2];     /* Date&Time from RTC of last set of pw */
  unsigned char Serial[16];         /* Box serial number */
  } SECURITY;

/* There are 2 slots for recording errors before information is lost.
If all the slots are full and another error happens then the most
recent one should be overwritten to preserve the oldest ones which may
have triggered the flurry.  The overrun bit should be set on this one.
The detector of the error should find a free slot and fill in the
log information and set the Pending status.  If it then succeeds in
logging it, it should set the logged status.  If boot sees any bits on
in H type log entries it should ignore the FastBoot request and do a
more thorough POST.  In particular it should attempt to run
diagnostics on any failing devices.  It should then set the
DiagnosedOK or DiagnosedFail as appropriate.  When OS's start up they
should look at the log entries and try to record them in their
permanent logs.  If they succeed they should clear the status.  */

typedef enum _ERROR_STATUS {
  Clear = 0,                         /* empty entry - already logged */
  Pending = 1,
  DiagnosedOK = 2,
  DiagnosedFail = 3,
  Overrun = 4,
  Logged = 5
  } ERROR_STATUS;

typedef enum _OS_ID {
  Unknown = 0,
  Firmware = 1,
  AIX = 2,
  NT = 3,
  WPOS2 = 4,
  WPX = 5,
  Taligent = 6,
  Solaris = 7,
  Netware = 8,
  USL = 9,
  Low_End_Client = 10,
  SCO = 11
  } OS_ID;

typedef struct _ERROR_LOG {
  unsigned char Status;    /* ERROR_STATUS */
  unsigned char Os;        /* OS_ID */
  unsigned char Type;      /* H for Hardware, S for Software */
  unsigned char Severity;  /* S - severe, E - error */
    /* The severity should be set S if the OS cannot proceed unless
    fixed.  It should be set to E otherwise.  If the boot process sees
    an S severity it should not attempt to boot the operating system
    that caused it but remain in firmware state.  If it runs the
    diagnostics and gets DiagnosedOK, it may proceed.  */

  unsigned long ErrDT[2];  /* Date&Time from RTC */
  unsigned char code[8];   /* detailed classification of error */
  union {
    unsigned char detail[20];
    } data;
  } ERROR_LOG;

typedef enum _BOOT_STATUS {
    BootStarted = 0x01,
    BootFinished = 0x02,
    RestartStarted = 0x04,
    RestartFinished = 0x08,
    PowerFailStarted = 0x10,
    PowerFailFinished = 0x20,
    ProcessorReady = 0x40,
    ProcessorRunning = 0x80,
    ProcessorStart = 0x0100
    } BOOT_STATUS;

typedef struct _RESTART_BLOCK {
    unsigned short Version;
    unsigned short Revision;
    unsigned long BootMasterId;
    unsigned long ProcessorId;
    volatile unsigned long BootStatus;
    unsigned long CheckSum; /* Checksum of RESTART_BLOCK */
    void * RestartAddress;
    void * SaveAreaAddr;
    unsigned long SaveAreaLength;
  } RESTART_BLOCK;

typedef enum _OSAREA_USAGE {
  Empty = 0,
  Used = 1
  } OSAREA_USAGE;

typedef enum _PM_MODE {
  Suspend = 0x80, /* part of state is in memory */
  Hibernate = 0   /* Nothing in memory */
  } PMMode;

typedef struct _HEADER {
  unsigned short Size;    /* NVRAM size in K(1024) */
  unsigned char Version;  /* Structure map different */
  unsigned char Revision; /* Structure map the same -
                             may be new values in old fields
                             in other words old code still works */
  unsigned short Crc1;    /* check sum from beginning of nvram to OSArea */
  unsigned short Crc2;    /* check sum of config */
  unsigned char LastOS;   /* OS_ID */
  unsigned char Endian;   /* B if big endian, L if little endian */
  unsigned char OSAreaUsage; /* OSAREA_USAGE */
  unsigned char PMMode;   /* Shutdown mode */
  RESTART_BLOCK RestartBlock;
  SECURITY Security;
  ERROR_LOG ErrorLog[2];

/* Global Environment information */
  void * GEAddress;
  unsigned long GELength;
  /* Date&Time from RTC of last change to Global Environment */
  unsigned long GELastWriteDT[2];

/* Configuration information */
  void * ConfigAddress;
  unsigned long ConfigLength;
  /* Date&Time from RTC of last change to Configuration */
  unsigned long ConfigLastWriteDT[2];
  unsigned long ConfigCount; /* Count of entries in Configuration */

/* OS dependent temp area */
  void * OSAreaAddress;
  unsigned long OSAreaLength;
  /* Date&Time from RTC of last change to OSAreaArea */
  unsigned long OSAreaLastWriteDT[2];
  } HEADER;

/*
NVRAM has three major large areas - one for storing Global environment
strings, one for storing Plug and Play compressed configuration
packets for devices that cannot be figured out dynamically, and one
for transient OS dependent information.  The space is shared and
dynamically adjustable.  There are address-length pairs that describe
them.  This allows larger NVRAM parts to be used without bumping the
version number.  Functions which add information and do not have room
can attempt to readjust the space balance and move the data as
necessary.

The global environment variable should be settable from the firmware
and some OSs may require that some of them are set as part of the
install process.  OSs should make efforts to use the same variable
names for similar things to conserve space.  These variables should be
things that are either needed or used by the firmware or things that
are common to a machine no matter what OS the user is running.

See the Plug and Play Spec for a complete definition of the format of
the compressed configuration packets.  Microsoft is in the process of
extending it to cover PCI and PCMCIA type devices.  This information
should be settable by the user from the firmware state when devices are
installed or removed.  OSs may also set this when device drivers are
installed.  They should attempt to respect values already set for a
particular device.  The compressed packets are expanded into the
configuration structure passed in the residual data at boot time along
with other dynamically discovered devices or devices defined in the
FLASH that are part of the planar.

Both the environment variables and the configuration information are
compatible with and required by OpenBoot.

The OSArea area saves information that is only valid until the next boot
of the machine.  It is valid if and only if the LastOS matches the
current OS */

/* Here is the whole map of the NVRAM */
typedef struct _NVRAM_MAP {
  HEADER Header;
  unsigned char GEArea[NVSIZE-CONFSIZE-OSAREASIZE-sizeof(HEADER)];
  unsigned char OSArea[OSAREASIZE];
  unsigned char ConfigArea[CONFSIZE];
  } NVRAM_MAP;


#if 0
See ARC Spec - or summary of it in BOOTUP.PS for more information
  GlobalEnvironment required by NT (first 4 would also do for AIX IPLIST):
    LoadIdentifier=<id>;<id>; ...
    SystemPartition=<path expression>;<path expression>; ...
    OSLoadPartition=<path expression>;<path expression>; ...
    OSLoadOptions=<options>;<options>; ...
    OSLoader=<path expression>;<path expression>; ...
    OSLoadFilename=<file name>;<file name>; ...
    AutoLoad=<yes|no>
    TimeZone=<POSIX spec>
    FWSearchPath=<path expression>;<path expression>; ...
    ConsoleIn=<path expression>
    ConsoleOut=<path expression>
  Open Firmware variables
    Script=
  Network variables
    ClientIPAddr=
    ServerIPAddr=
    GatewayIPAddr=
    NetMask=
    (other variables if RIPL is supported)
  Other useful variables
    Lang=
    ErrorLogPartition=<path expression>
    PagingPartition=<path expression>
    DumpPartition=<path expression>
    CheckpointPartition=<path expression>
    LastIPL=
    /* Delay time before automatic continuation with default action */
    DelayTime=
    /* FAST = Minimal POST testing unless hardware error on last boot */
    Post=FAST|SLOW;
    Operatorless=1; /* Make default assumptions automatically */
    Secure=1; /* Enforce security */
    Quiet=1; /* Make as little noise as possible */
    (Default: Post=FAST; Operatorless=0; Secure=0; Quiet=0;
#endif       /* comments */

#endif  /* ndef _NVRAM_ */
