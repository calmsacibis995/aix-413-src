#!/bin/ksh
# @(#)97	1.7  src/bos/usr/sbin/tapechk/tapechk.sh, cmdarch, bos411, 9428A410j 3/28/91 11:48:37
#
# COMPONENT_NAME: (CMDARCH) archive files
#
# FUNCTIONS: tapechk
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# tapechk makes a (weak) attempt to check validity of
# a backup/restore tape.
if [ -z "$TAPE" ]
  then
    tape=/dev/rmt0
  else
    tape=$TAPE
fi

# If first parm is a question mark, explain tapechk
if [ "$1" = -? ]
  then
    echo The tapechk command has the following syntax:
    echo
    echo "    tapechk [$1] [x | [x y]]"
    echo
    echo --  \"x\" and \"y\" must be decimal numbers.
    echo --  tapechk checks the next \"x\" files from the beginning
    echo "    of the tape."
    echo --  tapechk skips the next \"y\" files from the beginning
    echo "    of the tape."
    echo
    echo You can request an \"x\" value by itself, or an \"x\" value
    echo with a \"y\" value.  If you do not request any value, tapechk
    echo checks the first physical block on the tape.
    exit 0
fi

# If parms are not numbers, complain
ch=`expr ${1:-0} + 0 2>/dev/null`
sk=`expr ${2:-0} + 0 2>/dev/null`
if [ ! "$ch"  ]
  then
    echo The tapechk command does not recognize the \"$1\" flag you
    echo entered.  Try again, or enter \"tapechk -?\" for information
    echo about valid flags.
    exit 1
  else
    if [ ! "$sk" ]
      then
	echo The tapechk command does not recognize the \"$2\" flag you
	echo entered.  Try again, or enter \"tapechk -?\" for information
	echo about valid flags.
	exit 2
      fi
fi

# Must rewind tape before we begin.
echo 118-008 The tapechk command is rewinding the tape.  Please wait.
tctl -f "$tape" rewind
if [ $? != 0 ]
  then
    echo 118-009 The tapechk command could not rewind the tape.  Please refer
    echo "        to the messages reference book."
    exit 3
fi

# If no parms given, just check first block.
if [ ! "$1" ]
  then
    echo 118-010 The tapechk command is checking the first block
    echo "        on the tape.  Please wait."
    tctl -f "$tape" fsr 1
    if [ $? != 0 ]
      then
	echo 118-011 The tapechk command could not check the files
	echo "        you requested.  One or more files may be damaged, or the"
	echo "        tape drive may not be working properly."
	tctl -f "$tape" rewind
	exit 4
      else
	echo 118-012 The first block on tape \""$tape"\" appears to be OK.
	tctl -f "$tape" rewind
	exit 0
    fi
fi

# If second parm = n, skip first n FILES on tape
if [ "$2" ]
  then
    echo 118-013 The tapechk command is skipping the first $2 file\(s\)
    echo "        from the beginning of the tape.  Please wait."
    tctl -f "$tape" fsf $sk
    if [ $? != 0 ]
      then
	echo 118-014 The tapechk command could not skip over the files you
	echo "        requested.  Please refer to the messages reference book."
	tctl -f "$tape" rewind
	exit 5
    fi
fi

# tctl skipping DOES do reads of tape it skips
# so we "read" the tape with skip
echo 118-015 The tapechk command is checking the next $1 file\(s\).
echo "        Please wait."
tctl -f "$tape" fsf $ch
if [ $? != 0 ]
  then
    echo 118-011 The tapechk command could not check the files you
    echo "        requested.  One or more files may be damaged, or the tape"
    echo "        drive may not be working properly."
    exit 6
fi
echo 118-016 The files you requested on tape \""$tape"\" appear
echo "        to be OK."

# Rewind tape before letting go.  Note; we used to
# do this in backround but delay was too long.
tctl -f "$tape" rewind
exit 0
