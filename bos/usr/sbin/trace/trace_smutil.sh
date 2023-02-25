#!/bin/bsh
# @(#)99 1.12 src/bos/usr/sbin/trace/trace_smutil.sh, cmdtrace, bos41J, 9513A_all 3/22/95 13:57:31
#
# COMPONENT_NAME: CMDTRACE SMIT shell interface program
# 
# FUNCTIONS: interfaces to trclist, trace, trcgroup, cat, catpr
# 
# ORIGINS: 27 83 
# 
# (C) COPYRIGHT International Business Machines Corp. 1988, 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# LEVEL 1, 5 Years Bull Confidential Information
#

# this file
TRACE_SMUTIL=/usr/lib/ras/trace_smutil

[ $# -gt 0 ] || { exit 0 ; }

trap "exit 5" 2

UTILNAME=$1
shift

case $UTILNAME in

#
# trclist.
# Used to list current trace groupings
#
trclist)

TRCLIST=/usr/lib/ras/trclist
[ -f $TRCLIST ] || { echo "all" ; exit 0 ; }
cat $TRCLIST | grep -v '^#' | grep -v '^[ 	]*$'

;;

trace)
ARGS=`echo $* | sed s'/-./& /g'`
OPTIONS="`getopt fladshcq1234567j:k:m:o:T:L:J: $ARGS`"
set -- $OPTIONS

CMDLINE=/usr/sbin/trace
while [ -n "$1" ]; do
	ARG=`echo $1 | sed s'/^-//'`
	case $ARG in
	f|l|a|d|s|h|c|1|2|3|4|5|6|7)
		CMDLINE="$CMDLINE -$ARG"
		shift
		;;
	# special case: -n is interpreted by the echo command
	# use -q in the SMIT dialog and shell script interface
	# and translate it into -n flag to pass to the trace command
	q)
		CMDLINE="$CMDLINE -n"
		shift
		;;
	
	m|o|T|L)
		CMDLINE="$CMDLINE -$ARG $2"
		shift 2
		;;
	j|k)
		CMDLINE="$CMDLINE -$ARG \""
		shift
		while [ -n "$1" ]; do
			case $1 in
			--)
				shift
				;;
			
			-*)
				break
				;;

                        *)
				CMDLINE="$CMDLINE $1"
				shift
				;;
			esac
		done
		CMDLINE="$CMDLINE \""
		;;
	J)
		shift
		Jflag=0
		while [ -n "$1" ]; do
			ARG=`echo $1 | sed s'/^-//'`
			case $ARG in
			0*|1*|2*|3*|4*|5*|6*|7*|8*|9*)
				L=`$TRACE_SMUTIL trcgroup $ARG`
				if [ $Jflag -eq 0 ]; then
				   [ "$L" != "" ] && CMDLINE="$CMDLINE -j $L"
				   Jflag=1
				else
				   [ "$L" != "" ] && CMDLINE="$CMDLINE,$L"
				fi
				shift
				;;
			-)
				shift
				;;
			*)
				break
				;;
			esac
		done
		;;
	-)
		shift
		break
		;;
	esac
done

eval $CMDLINE

;;

trcgroup)
[ $# -gt 0 ] || { exit 0 ; }

LIST=
case $1 in
99) # ALL
	LIST="\
00,01,02,03,04,05,06,07,08,09,0a,0b,0c,0d,0e,0f,\
10,11,12,13,14,15,16,17,18,19,1a,1b,1c,1d,1e,1f,\
20,21,22,23,24,25,26,27,28,29,2a,2b,2c,2d,2e,2f,\
30,31,32,33,34,35,36,37,38,39,3a,3b,3c,3d,3e,3f,\
40,41,42,43,44,45,46,47,48,49,4a,4b,4c,4d,4e,4f,\
50,51,52,53,54,55,56,57,58,59,5a,5b,5c,5d,5e,5f,\
60,61,62,63,64,65,66,67,68,69,6a,6b,6c,6d,6e,6f,\
70,71,72,73,74,75,76,77,78,79,7a,7b,7c,7d,7e,7f,\
80,81,82,83,84,85,86,87,88,89,8a,8b,8c,8d,8e,8f,\
90,91,92,93,94,95,96,97,98,99,9a,9b,9c,9d,9e,9f,\
a0,a1,a2,a3,a4,a5,a6,a7,a8,a9,aa,ab,ac,ad,ae,af,\
b0,b1,b2,b3,b4,b5,b6,b7,b8,b9,ba,bb,bc,bd,be,bf,\
c0,c1,c2,c3,c4,c5,c6,c7,c8,c9,ca,cb,cc,cd,ce,cf,\
d0,d1,d2,d3,d4,d5,d6,d7,d8,d9,da,db,dc,dd,de,df,\
e0,e1,e2,e3,e4,e5,e6,e7,e8,e9,ea,eb,ec,ed,ee,ef,\
f0,f1,f2,f3,f4,f5,f6,f7,f8,f9,fa,fb,fc,fd,fe,ff"
	;;
1)  # GENERAL KERNEL ACTIVITY
	LIST="106,134,139,107,135,15b,12e"
	;;
2)  # GENERAL KERNEL ACTIVITY + SYSTEM CALLS
	LIST="106,134,139,107,135,15b,12e,101,104"
	;;
3)  # FILE OPENS
	LIST="15b,107"
	;;
4)  # FILE ACTIVITY
	LIST="106,134,139,107,135,15b,12e,19c,163"
	;;
5)  # EXECS, FORKS, EXITS
	LIST="134,139,135"
	;;
6)  # EXECS, FORKS, EXITS, DISPATCHES
	LIST="134,139,135,106"
	;;
7)  # FILE ACTIVITY (with physical file system)
	LIST="106,134,139,107,135,15b,12e,19c,163,10a"
	;;
8)  # FILE ACTIVITY (with physical file system and VMM)
	LIST="106,134,139,107,135,15b,12e,19c,163,10a,1ba"
	;;
9)  # FILE ACTIVITY (with physical file system, VMM, and LVM)
	LIST="106,134,139,107,135,15b,12e,19c,163,10a,1ba,10b"
	;;
10) # FILE ACTIVITY (with physical file system, VMM, LVM, disk device drivers)
	LIST="106,134,139,107,135,15b,12e,19c,163,10a,1ba,10b,223,221,222,45A,45B,2a0,2a1,2a2"
	;;
11) # SYSTEM CALLS
	LIST="101,104"
	;;
12) # FLIHS and SLIHS
	LIST="100,200,102,103"
	;;
13) # LOGICAL FILE SYSTEM
	LIST="108"
	;;
14) # PHYSICAL FILE SYSTEM
	LIST="10a"
	;;
15) # VIRTUAL MEMORY MANAGER
	LIST="1b"
	;;
16) # LOGICAL VOLUME MANAGER
	LIST="10b,105"
	;;
17) # LOGICAL VOLUME MANAGER BADBLOCK EVENTS
	LIST="105"
	;;
18) # IPC: SHARED MEMORY
	LIST="174,175,176,177"
	;;
19) # IPC: MESSAGES
	LIST="178,179,17a,17b,17c"
	;;
20) # IPC: SEMAPHORES
	LIST="17d,17e,17f"
	;;
21) # ERROR LOGGING
	LIST="1d1"
	;;
22) # DEVICE DRIVER: HFT
	LIST="201,202"
	;;
23) # DEVICE DRIVER: PARALLEL PRINTER
	LIST="1c8"
	;;
24) # DEVICE DRIVER: TAPE
	LIST="1ca,326"
	;;
25) # DEVICE DRIVER: ETHERNET - HIGH PERFORMANCE LAN ADAPTER (8ef5)
	LIST="351,352,353,354"
	;;
26) # DEVICE DRIVER: TOKEN RING - HIGH PERFORMANCE ADAPTER (8fc8)
	LIST="26a,26b,26c"
	;;
27) # DEVICE DRIVER: C3270
	LIST="1cf"
	;;
28) # DEVICE DRIVER: FLOPPY DISK
	LIST="220"
	;;
29) # DEVICE DRIVER: SCSI
	LIST="223,22a,22b"
	;;
30) # DEVICE DRIVER: DISK
	LIST="222,221,2a1,2a2"
	;;
31) # DEVICE DRIVER: MULTI-PROTOCAL ADAPTERS
	LIST="224,22C"
	;;
32) # DEVICE DRIVER: GRAPHICS
	LIST="1d0"
	;;
33) # DEVICE DRIVER: pty
	LIST="402"
	;;
34) # DEVICE DRIVER: rs232
	LIST="40"
	;;
35) # DEVICE DRIVER: 64 Port Async Controller
	LIST="404"
	;;
36) # DEVICE DRIVER: x25
	LIST="225"
	;;
37) # DEVICE DRIVER: harrier2
	LIST="228"
	;;
38) # DEVICE DRIVER: SCSI target mode
	LIST="229"
	;;
39) # DEVICE DRIVER: Dials/LPFKeys
	LIST="226"
	;;
40) # LOCKS ACTIVITY
	LIST="112,113,114,115,20e,20f"
	;;
41) # LIBPTHREADS LOCKS ACTIVITY
	LIST="230,231,232,233"
	;;
42) # UMSMMCS
	LIST="302,303,304"
	;;
43) # UMSMMFS
	LIST="307,308,309"
	;;
44) # UMSCORE
	LIST="301,305"
	;;
45) # X.25 STREAMS BASED LPP
	LIST="25b,25c,25d,25e,329,32a,32b,32c,33b,33c"
	;;
46) # DEVICE DRIVER: ATM
	LIST="339"
	;;
47) # DEVICE DRIVER: ETHERNET - INTEGRATED ETHERNET ADAPTER
	LIST="320,321,322,323"
	;;
48) # DEVICE DRIVER: ETHERNET - HIGH PERFORMANCE ADAPTER (8f95) 
	LIST="327,328,27D,27E"
	;;
49) # DEVICE DRIVER: ETHERNET - IBM ISA ADAPTER
	LIST="330,331,332,333"
	;;
50) # DEVICE DRIVER: TOKEN RING - IBM ISA ADAPTER
	LIST="334,335,336,337"
	;;
51) # DEVICE DRIVER: TOKEN RING - HIGH PERFORMANCE ADAPTER (8fa2) 
	LIST="32D,32E,32F,338"
	;;
52) # DEVICE DRIVER: 128 Port Async Controller
	LIST="410"
	;;
53) # DEVICE DRIVER: ETHERNET - IBM PCI ADAPTER
	LIST="2A4,2A5,2A6"
	;;
54) # DEVICE DRIVER: TOKENRING - IBM PCI ADAPTER
	LIST="2A7,2A8,2A9,2AA"
	;;
55) # DEVICE DRIVER: SSA
	LIST="45A,45B"
	;;
56) # DEVICE DRIVER: IDE ADAPTER
	LIST="2a0"
	;;
57) # DEVICE DRIVER: IDE SUBSYSTEM
	LIST="2a0,2a1,2a2"
	;;
esac
echo $LIST
;;

cat)
	FILE=$1
	if [ "$FILE" = "" ] ; then
		cat
		RC=$?
	else
		FILE=`echo $FILE | sed 's/^Z//' `
		cat > $FILE
		RC=$?
	fi
	exit $RC
;;

catpr)
	PRINTER=$1
	if [ "$PRINTER" = "" ] ; then
		cat
		RC=$?
	else
		PRINTER=`echo $PRINTER | sed 's/^Z//' `
		if [ -n "$PRINTER" ] ; then
		PRINTER="-P $PRINTER"
		fi
		enq $PRINTER
		RC=$?
	fi
	exit $RC
;;

esac


