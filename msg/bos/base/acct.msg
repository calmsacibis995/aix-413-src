$ ENGLISH_SOURCE_VERSION= 1.7.2.7
$ @(#)67        1.1  src/msg/bos/base/acct.msg, bosmsg, msg411, GOLD410 2/9/94 08:45:43
$
$ COMPONENT_NAME: (CMDACCT) Command Accounting
$
$ FUNCTIONS: none
$
$ ORIGINS: 9,27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "
$ $ Message numbers 001 - 100 reserved for this file.
$set 1
1 "chargefee: 0850-001 cannot find login name "
2 "chargefee: 0850-002 charge is not valid: "
3 "ckpacct: 0850-003 /var/adm still low on space (%s blks); acctg still off\n"
4 "ckpacct: 0850-004 /var/adm too low on space (%s blks); turning acctg off\n"
8 "ckpacct: 0850-005 /var/adm free space restored; turning acctg on\n"
9  "Usage: dodisk [ -o ] [ filesystem ... ]\n"
10 "dodisk: 0850-006 is not a directory -- ignored"
11 "dodisk: No data"
12 "MAJ/MIN			CONNECT SECONDS	START TIME	SESSION START\n"
13 "DEVICE	UID	LOGIN	PRIME	NPRIME	(NUMERIC)	DATE	TIME\n"
14 "SESSIONS, SORTED BY ENDING TIME\n"
$  	The above are headings for the prctmp report; keep columns the same
15 "Usage: prdaily [-c] [-l] [mmdd]\n"
16 "prdaily: 0850-007 Cannot print command exception reports except for %s.\n" 
17 "DAILY REPORT FOR %s"
18 "DAILY USAGE REPORT FOR %s"
19 "DAILY COMMAND SUMMARY\n"
20 "MONTHLY TOTAL COMMAND SUMMARY\n"
21 "LAST LOGIN\n"
22 "Usage: prtacct [-f fields ] [-v] file [heading]\n"
23 "\r\n************ ACCT ERRORS : see %s%s********\r\n"
24 "\r\n*********** 2 CRONS or ACCT PROBLEMS***********\r\n"
25 "runacct: 0850-008 locks found, run aborted\n" 
26 "runacct: 0850-009 Insufficient space on /dev/%1$s (%2$s blks); Terminating procedure\n"
27 "runacct: 0850-010 acctg already run for %1$s : check %2$s \n"
29 "restarting acctg for %1$s at %2$s\n"
31 "previous state was %s\n"
32 "runacct: 0850-011 runacct called with arguments that are not valid\n"
33 "runacct: 0850-012 turnacct switch returned rc=\c"
34 "runacct: 0850-013 S%s.%s already exists\n" 
35 "file setups probably already run\n"
36 "runacct: S%s.%s already exists: run setup manually\n"
37 "files setups complete\n"
38 "runacct: 0850-014 wtmpfix errors see %s/wtmperror%s\n" 
39 "wtmp processing complete\n"
40 "connect acctg complete\n"
41 "WARNING: accounting already run for %s\n" 
42 "WARNING: remove %s/%s to rerun\n" 
43 "process acctg complete for %s\n" 
44 "all process actg complete for %s\n" 
45 "tacct merge to create daytacct complete\n"
46 "merged fees\n"
47 "no fees\n"
48 "merged disk records\n"
49 "no disk records\n"
50 "merged queueing system records\n"
51 "no queueing system records\n"
52 "WARNING: recreating %s/sum/tacct \n"
53 "updated sum/tacct\n"
54 "WARNING: recreating %s/sum/cms \n" 
55 "command summaries complete\n"
56 "system accounting completed at %s\n" 
58 "runacct: 0850-015 state is not valid, check "
63 "acctg on"
64 "Usage: turnacct on|off|switch\n"
65 "Usage: chargefee name number\n"
66 "TOTAL COMMAND SUMMARY FOR FISCAL"
67 "LAST LOGIN"
68 "accting off"
69 "runacct: 0850-016 An error occurred while executing the acctmerg program. \
The pacct files will be restored in order to prevent the loss of accounting \
data and all intermediate lock files will be removed.\n"
70 "%1$s: 0850-017 %2$s not found\n"
71 "Time Exception Command Usage Summary\n"
72 "\t\t\t\tCommand Exception Usage Summary\n"
73 "\t\t\t\tTotal CPU > %1$s or Total KCORE > %2$s\n"
74 "\t\t\t\tLogins with exceptional Prime/Non-prime Time Usage\n"
75 "\t\t\t\tCPU > %1$s or KCORE > %2$s or CONNECT > %3$s\n\n\n"
76 "\tLogin\t\tCPU (mins)\tKCORE-mins\tCONNECT-mins\tdisk\t# of\t# of\t# Disk\tfee\n"
77 "UID\tName\t\tPrime\tNprime\tPrime\tNprime\tPrime\tNprime\tBlocks\tProcs\tSess\tSamples\n\n"
$ The above are headings for the prdaily -l report; keep columns the same
$ This is a new message.
$ Do not translate %s, it will be the name of the command.
$ Do not translate "acctcon1" or "acctcon2", they are command names.
78 "%1$s: You must execute \"acctcon1\" and \"acctcon2\" to create the \n\
connection accounting file prior to executing \"%1$s\"\n"
$ NOTE TO TRANSLATOR:
$ Do not translate the words acctmerg or pacct.
$
$quote "
$set MS_ACCT
USAGE		"Usage: %s [-a [-op] | -t] [-cjns] [file ...]\n"
NOMEM		"%s: 0850-018 Cannot allocate memory\n"
CANTOPEN 	"%1$s: 0850-019 cannot open %2$s\n"
THEADER1        "COMMAND      NUMBER    TOTAL     TOTAL    TOTAL       MEAN     MEAN       HOG       CHARS    BLOCKS\n"
THEADER2        " NAME         CMDS   KCOREMIN   CPU-MIN  REAL-MIN    SIZE-K   CPU-MIN    FACTOR    TRNSFD     READ\n"
TOTALS		"TOTALS"
HDP		"                                PRIME/NON-PRIME TIME COMMAND SUMMARY\n"
HDNONPRIME	"                                  NON-PRIME TIME COMMAND SUMMARY\n"
HDTOT		"                                     TOTAL COMMAND SUMMARY\n"
HDPRIME		"                                   PRIME TIME COMMAND SUMMARY\n"
HD1             "COMMAND      NUMBER    TOTAL     TOTAL    TOTAL       MEAN     MEAN       HOG       CHARS    BLOCKS\n"
HD2             " NAME         CMDS   KCOREMIN   CPU-MIN  REAL-MIN    SIZE-K   CPU-MIN    FACTOR    TRNSFD     READ\n"
HD3             "COMMAND          NUMBER-CMDS      TOTAL          CPU-MIN            REAL-MIN         MEAN     MEAN       HOG       CHARS    BLOCKS\n"
HD4             " NAME          (P)       (NP)   KCOREMIN      (P)      (NP)       (P)      (NP)     SIZE-K   CPU-MIN    FACTOR    TRNSFD     READ\n"
GUNKNOWN	"0850-020 Unknown group"
UUNKNOWN	"%1$s: 0850-021 Unknown user %2$s\n"
OPENERR		"0850-022 open error on output file"
ARFHD		"\nACCOUNTING RECORDS FROM:  %s"
STAFT		"START AFT: %s"
STBEF		"START BEF: %s"
ENDAFT		"END AFTER: %s"
ENDBEF		"END BEFOR: %s"
CMSHED1		"COMMAND                      START    END          REAL"
CHARSTR		"CHARS"
BLOCKSTR	"BLOCKS"
CPUSTR		"CPU"
HOGSTR		"HOG"
MEANSTR		"MEAN"
KDCORESTR	"KCORE"
CMSHED2		"NAME       USER     TTYNAME  TIME     TIME       (SECS)"
SYSSTR		"SYS"
USERSTR		"USER"
SECSTR		"(SECS)"
TRNSFDSTR	"TRNSFD"
READSTR		"READ"
FACTORSTR	"FACTOR"
SIZEKSTR	"SIZE(K)"
MINSTR		"MIN"
FSTATSTR	"  F STAT"
BADTIME		"0850-023 bad time:"
PATRNERR	"pattern syntax"
NOCMDS		"\nNo commands matched\n"
NOSTATIN	"acctcom: 0850-024 cannot stat stdin\n"
NOSTATNULL	"acctcom: 0850-025 cannot stat /dev/null\n"
COMUSAGE1	"Usage: acctcom [options] [files]\n"
COMUSAGE2	"\nWhere options can be:\n"
COMUSAGE3	"-b	read backwards through file"
COMUSAGE4	"-f	print the fork/exec flag and exit status"
COMUSAGE5	"-h	print hog factor (total-CPU-time/elapsed-time)"
COMUSAGE6	"-i	print I/O counts"
COMUSAGE7	"-k	show total Kcore minutes instead of memory size"
COMUSAGE8	"-m	show mean memory size"
COMUSAGE9	"-r	show CPU factor (user-time/(sys-time + user-time))"
COMUSAGE10	"-t	show separate system and user CPU times"
COMUSAGE11	"-v	don't print column headings"
COMUSAGE12	"-a	print average statistics of selected commands"
COMUSAGE13	"-q	print average statistics only"
COMUSAGE14	"-l line	\tshow processes belonging to terminal /dev/line"
COMUSAGE15	"-u user	\tshow processes belonging to user name or user ID"
COMUSAGE16	"-u #	\tshow processes executed by super-user"
COMUSAGE17	"-u ?	\tshow processes executed by unknown UID's"
COMUSAGE18	"-g group	show processes belonging to group name of group ID"
COMUSAGE19	"-s time	\tshow processes ending after time (hh[:mm[:ss]])"
COMUSAGE20	"-e time	\tshow processes starting before time"
COMUSAGE21	"-S time	\tshow processes starting after time"
COMUSAGE22	"-E time	\tshow processes ending before time"
COMUSAGE23	"-n regex	select commands matching the ed(1) regular expression"
COMUSAGE24	"-o file	\tdo not print, put selected pacct records into file"
COMUSAGE25	"-H factor	show processes that exceed hog factor"
COMUSAGE26	"-O sec	\tshow processes that exceed CPU system time sec"
COMUSAGE27	"-C sec	\tshow processes that exceed total CPU time sec"
COMUSAGE28	"-I chars	show processes that transfer more than char chars"
BADWTMP		"bad wtmp: offset %lu.\n"
WTMPREC		"bad record is:  %1$.12s\t%2$.8s\t%3$lu"
CON1BADTYPE	"acctcon1: type %1$d for %2$s %3$s %4$s is not valid"
CON1TOOSMALL	"acctcon1: recompile with larger NSYS\n"
BADTIMEOLD	"%1$s: bad times: old: %2$s"
BADTIMENEW	"new: %s"
CON1FROM	"from %s"
CON1TO		"to   %s"
DATECHGS	"%d\tdate changes\n"
DATECHG		"%d\tdate change\n"
CON1TOT1	"TOTAL DURATION: %.0f MINUTES\n\n"
CON1TOT2	"LINE\tMINUTES\tPERCENT\t# SESS\t# ON\t# OFF\n"
CON1TOT3	"TOTALS\t%1$.0f\t--\t%2$d\t%3$d\t%4$d\n"
NOCORE		"out of core\n"
BADSPEC		"%1$s: bad fieldspec `%2$s'\n"
ONESPEC		"%s: 0850-026 only one input fieldspec allowed\n"
TOOMANYFILES	"%1$s: >%2$d files\n"
ACCTUID		"UID"
ACCTPRINT	"PRINT"
ACCTFEES	"FEES"
ACCTPROCESSES	"PROCESSES"
ACCTSESS	"SESS"
ACCTCANTON	"0850-027 cannot turn accounting ON\n"
ACCTNOOFF	"0850-028 cannot turn accounting OFF\n"
PNPHOLTBLERR	"pnpsplit: 0850-029 holidays table setup failed\n"
UPDTHOL		"***UPDATE %s WITH NEW HOLIDAYS***\n"
BADYEARCONV	"%s: 0850-030 bad {yr ptime nptime} conversion\n"
BADYEAR		"pnpsplit: 0850-031 not valid year: %d\n"
PNPINVAL	"pnpsplit: 0850-032 not valid p/np hours\n"
PNPHOL1		"pnpsplit: 0850-033 too many holidays, "
PNPHOL2		"recompile with larger NHOLIDAYS\n"
PNPBADAY	"pnpsplit: 0850-034 not valid day of year %d\n"
CANTREAD	"%1$s: 0850-035 Can't read %2$s\n"
OUTOFMEM	"%1$s: 0850-036 Out of memory -- %2$s not used\n"
USAGEWTMP	"Usage: %1$s reason [ >> %2$s ]\n"
CANTMKTMP	"0850-037 cannot make temporary: %s\n"
LOGCHG		"wtmpfix: logname \"%8.8s\" changed to \"INVALID\"\n"
INTRUC		"Input truncated at offset %ld\n"
NEWDTEXPTED	"New date expected at offset %ld\n"
CATREADTMP	"0850-038 Cannot read from temp: %s\n"
BADFILOFF	"Bad file at offset %ld\n"
CMDSSTR		"cmds"
REALSTR		"Real"
CHRSTR		"CHAR"
BLKSTR		"BLK"
UTSTR		"USR/TOT"
$
$ NOTE TO TRANSLATOR
$ THE FOLLOWING WORDS ARE A PART OF TWO ROW HEADING IN THE FOLLOWING FORMAT:
$
$ LOGIN    CPU       CPU       KCORE     KCORE     RD/WR     RD/WR     BLKIO
$ NAME     PRIME     NPRIME    PRIME     NPRIME    PRIME     NPRIME    PRIME
$
$ BLKIO     CONNECT   CONNECT   DISK
$ NPRIME    PRIME     NPRIME    BLOCKS
$
$ # OF        # OF   # DISK
$ PROCS       SESS   SAMPLES
$
ACCTLOGNAME2    "LOGIN"
$ LOGIN is the first of the two row heading, please do not use more than 8 char
$ LOGIN here refers to the user login name.
ACCTLOGNAME3    "NAME"
$ NAME is the second of the two rwo heading, please do not use more than 8 char
ACCT_CPU        "CPU"
$ CPU is the first of the two row heading, please do not use more than 9 char
$ CPU here implies the CPU time.
ACCT_KCORE      "KCORE"
$ KCORE is the first of the two row heading, please do not use more than 9 char
$ KCORE is refering here to memory.
ACCT_RD_WR      "RD/WR"
$ RD/WR is the first of the two row heading, please do not use more than 9 char
$ RD/WR here means the total blocks read/written
ACCT_BLKIO      "BLKIO"
$ BLKIO is the first of the two row heading, please do not use more than 9 char
ACCT_CONNECT    "CONNECT"
$ CONNECT implies the connect time, please do not use more than 9 char
ACCTDSK_BLOCKS2 "DISK"
$ part 1 of the two row heading, please do not use more than 9 char
ACCTDSK_BLOCKS3 "BLOCKS"
$ part 2 of the row row heading, please do not use more than 9 char
ACCTPROCESSES2  "PROCS"
$ PROCS part 2,i.e total number of processes, please do not use more than 9 char
ACCTDSAMPS2     "# DISK"
$ part 1, num of disk samples, please do not use more than 7 char
ACCTDSAMPS3     "SAMPLES"
$ part 2, please don't use more than 7 char
ACCT_PRIME      "PRIME"
$ part 2 of many of the above headings, please do not use more than 9 char
$ prime refers to prime time or normal working hours
ACCT_NPRIME     "NPRIME"
$ part 2 of many of the above headings, please do not use more than 9 char
$ nprime refers to non prime time.
ACCT_NUM        "# OF"
$ part 1 of two headings, please do not use more than 9 char
CON1USAGE	"Usage: acctcon1 [-l file] [-o file] [-pt]\n"
DUSGUSAGE       "Usage: acctdusg [-p file] [-u file]\n"
FWTMPUSAGE	"Usage: fwtmp [-ci]\n"
BADCTMP         "%s: The input file is corrupted and cannot be converted to ctmp format.\n"
$ This is a new message.
$ Do not translate %s, it will be the name of the command.
$ Do not translate "ctmp", it is the name of the input file format.
BADRECORD       "%s: An Invalid record is detected at line %lu in the input file. Quitting.\n"
