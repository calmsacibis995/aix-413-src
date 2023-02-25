#!/bin/ksh
# @(#)32	1.7  src/bos/usr/sbin/install/scripts/automsg.sh, cmdinstl, bos411, 9430C411a 8/1/94 19:31:09
#
#   COMPONENT_NAME: cmdinstl
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# automsg INSTALLP_L_OUTPUT_FILE [ OPTION_LIST_OUT_FILE ]

# or

# automsg -d $DEVICE [ OPTION_LIST_OUT_FILE ]

# This command will take as input an "installp -L" listing file name 
# and detects the .<msg>. and .<lang>. variables to use from the
# bos install data file /var/adm/ras/bosinst.data.
# It will also take a -d as input and call installp -L with the
# given device.
# It will also grep OUT any lines that have "ALL" because those
# are packages, and we should only return a list of message filesets
# to install.  This command also determines the primary language
# value based on the variables set in /etc/environment.
#
# The list of filesets is in the form:
#
# fileset  v.r.m.f
#
# one fileset per line
# example
# bos.msg.en_US.spam  4.1.0.0
#
# Return Codes:  0 success
#		 1 failure
# NOTE: This command does not print any failure messages.

# An input file or -d $device is required, but if an output file 
# (the second paramater) is not given, the filesets will be 
# printed to standard out.

BOS_DATA_FILE=/var/adm/ras/bosinst.data
BOS_DATA_NO_COMMENTS=/tmp/.bosinst.data.tmp
MSG_LIST=no_entry
LIST_DEVICE=no_device
INSTALLP_L_OUT=/tmp/.installp.L.out
RC=0
AUTOMSG=""
AUTOLOC=""

CAP_L_FILE=$1

# So grep will run faster
LANG=C

   if [ "$#" = 0 ]
   then
      exit 1
   fi

   if [ "$CAP_L_FILE" = "-d" ]
   then
      [ ! -r $2 ] && exit 1
         LIST_DEVICE=$2
	
      # Check for a third option - an output file

      if [ ! -z "$3" ]
      then
         MSG_LIST=$3
      fi

   else 

      # If the file does not have a nonzero length - error

      if [ ! -s "$CAP_L_FILE" ]
      then
         exit 1
      fi
	
      # If a second option is given, then it is the output file

      if [ ! -z "$2" ]
      then
         MSG_LIST=$2
      fi

   fi

   # If no output file was given, we print to stdout

   if [ "$MSG_LIST" = "no_entry" ]
   then
   cat_file="true"
   MSG_LIST=/tmp/.auto_msg_list
   fi

   # Get a value for .<msg>. from $BOS_DATA_FILE

   /usr/bin/grep -v "#" $BOS_DATA_FILE > $BOS_DATA_NO_COMMENTS
   /usr/bin/egrep -q "MESSAGES" $BOS_DATA_NO_COMMENTS
   RC=$?

   if [ "$RC" -eq 0 ]
   then
      AUTOMSG=$(/usr/bin/grep MESSAGES $BOS_DATA_NO_COMMENTS | \
                        /usr/bin/awk '{print $3}')
   else
      AUTOMSG=C
   fi

   # If the MESSAGES field is not set, default to C 
   if [ "$AUTOMSG" = "" ]
   then
      AUTOMSG=C
   fi

   # Get a value for .<locale>. from $BOS_DATA_FILE

   /usr/bin/egrep -q "CULTURAL_CONVENTION" $BOS_DATA_NO_COMMENTS
   RC=$?

   if [ "$RC" -eq 0 ]
   then
      AUTOLOC=$(/usr/bin/grep CULTURAL_CONVENTION $BOS_DATA_NO_COMMENTS | \
                        /usr/bin/awk '{print $3}')
   else
      AUTOLOC=C
   fi

   # If the CULTURAL_CONVENTION field is not set, default to C 
   if [ "$AUTOLOC" = "" ]
   then
      AUTOLOC=C
   fi

   if [ "$AUTOLOC" = "C" -a "$AUTOMSG" = "C" ]
   then
      /usr/bin/rm -f $BOS_DATA_NO_COMMENTS 
      exit 1
   fi

   /usr/bin/rm -f $MSG_LIST 

   # If -d was given, the we have to call installp -L

   if [ "$LIST_DEVICE" != "no_device" ]
   then
      /usr/sbin/installp -Lqd$LIST_DEVICE > $INSTALLP_L_OUT 2>/dev/null
      if [ "$?" != 0 ]
      then
         /usr/bin/rm -f $BOS_DATA_NO_COMMENTS $INSTALLP_L_OUT
         exit 1
      fi

      CAP_L_FILE=$INSTALLP_L_OUT
   fi

   # Grep the CAP_L_FILE (installp -L output, colon separated)
   # for the filesets that match the primary language and locale.
   # Don't print filesets that are already installed (Committed).
   
   if [ "$AUTOLOC" != "C" ]
   then
      /usr/bin/grep "\.loc\.$AUTOLOC" $CAP_L_FILE | /usr/bin/grep -v ALL | \
                  awk -F":" '$5 == "I" && $6 !~ /C/ { print $2 "  " $3 }' > $MSG_LIST 
   fi

   if [ "$AUTOMSG" != "C" ]
   then	
      /usr/bin/grep "\.msg\.$AUTOMSG" $CAP_L_FILE | /usr/bin/grep -v ALL | \
                  awk -F":" '$5 == "I" && $6 !~ /C/ { print $2 "  " $3 }' >> $MSG_LIST 
   fi
   
   if [ -s "$MSG_LIST" ]
   then
      sort -A -u -o $MSG_LIST $MSG_LIST
      if [ "$cat_file" = "true" ]
      then
         cat $MSG_LIST
         /usr/bin/rm -f $MSG_LIST 
      fi	
      /usr/bin/rm -f $BOS_DATA_NO_COMMENTS $INSTALLP_L_OUT 
      exit 0
   else
      /usr/bin/rm -f $BOS_DATA_NO_COMMENTS $INSTALLP_L_OUT 
      exit 1
   fi


exit 0
