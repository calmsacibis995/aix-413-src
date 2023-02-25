#! /bin/bsh
# @(#)38	1.17.1.7  src/bos/usr/sbin/lvm/highcmd/exportvg.sh, cmdlvm, bos411, 9438C411a 9/23/94 17:43:32
#
# COMPONENT_NAME: (cmdlvm) Logical Volume Commands
#
# FUNCTIONS: exportvg
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1993
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


#
# FILE NAME: exportvg
#
# FILE DESCRIPTION: High-level logical volume shell command for exporting
#                   the definition of a volume group.
#
#
# RETURN VALUE DESCRIPTION:
#                             0     Successful
#                             1     Unsuccessful
#
#
# EXTERNAL PROCEDURES CALLED:  getlvodm, lvaryonvg, lqueryvgs,
#                              lvaryoffvg, putlvodm,
#                              lvrelminor, lvrelmajor
#
#
# GLOBAL VARIABLES: none
#

# hash functions into system table
hash getlvodm lqueryvgs putlvodm lvrelminor \
     lvrelmajor dspmsg


########################### test_return #######################################
#
# NAME: test_return()
#
# DESCRIPTION: Tests function return code. Will exit and output error message
#              if bad.
#
# INPUT:
#        $1 : Function return code
#
# OUTPUT:
#        Error messages (Standard Error)
#
# RETURN VALUE DESCRIPTION:
#                           0   Successful
#
# NOTE: This function will not return if input value is invalid. (ie. it will
#       exit with exit status of 1. )
#
test_return()
{
   if [ "$1" != 0 ]
   then     # Unsuccessful - give general error message and exit ...
        dspmsg -s 1 cmdlvm.cat 772 "`lvmmsg 772`\n" $PROGNAME $VGNAME >&2
        exit
   fi
}

############################ cleanup ###########################################
#
# NAME: cleanup()
#
# DESCRIPTION: Called from trap command to clean up environment and exit.
#
# INPUT: none
#
# OUTPUT:
#        Error messages  (Standard Error)
#
# RETURN VALUE DESCRIPTION:
#                           0   Successful
#
# NOTE: This function will not return.
#
#
cleanup()
{

   trap '' 0 1 2 15


   if [ -n "$LOCKED" ]
   then
          putlvodm -K $VGID 2>/dev/null
   fi

   #save the state of the configuration database on the ipl disk
   savebase

   #  delete temporary files
   rm -f /tmp/pvid_names$$
   rm -f /tmp/rootdevice$$
   rm -f /tmp/lvinfo$$
   rm -f /tmp/pvinfo$$
   rm -f /tmp/vginfo$$
   exit $EXIT_CODE
}

############################## main ############################################
#  Export volume group
#  Input:
#       Command line options and arguments:
#       exportvg vgname
#  Output:
#       Error Messages (Standard error)
#
EXIT_CODE=1           #Initialize exit code. This will be reset to 0 before
                      #exiting only if exportvg completes successfully.

#
# Trap on exit/interrupt/break to clean up
#

trap 'cleanup' 0 1 2 15
PROGNAME=`basename $0`

ODMDIR=/etc/objrepos
export ODMDIR

#
# Parse command line arguments
#

set -- `getopt - $*`

if [ $? != 0 ]                      # Determine if there is a syntax error.
then
   dspmsg -s 1 cmdlvm.cat 606 "`lvmmsg 606`\n" exportvg >& 2
   dspmsg -s 1 cmdlvm.cat 760 "`lvmmsg 760`\n" $PROGNAME >&2
   exit
fi

shift

if [ -n "$1" ]     #if vgname argument on command line
then
        VGNAME=$1

        # Determine the major number of the root device file.
        ls -l /dev/IPL_rootvg |sed 's/,/ /g' > /tmp/rootdevice$$

        read skipfld skipfld skipfld skipfld ROOTMAJOR skipfld  < /tmp/rootdevice$$

        VGID=`getlvodm -v $VGNAME`
   	test_return $?           # check for error return

        # Determine the major number of the volume group entered
        VGMAJOR=`getlvodm -d $VGNAME` 2>/dev/null

        if [ "$VGMAJOR" -eq "$ROOTMAJOR" ]
        then
           dspmsg -s 1 cmdlvm.cat 762 "`lvmmsg 762`\n" $PROGNAME $VGNAME >&2
           exit
        fi

else
        dspmsg -s 1 cmdlvm.cat 606 "`lvmmsg 606`\n" $PROGNAME >&2
        dspmsg -s 1 cmdlvm.cat 760 "`lvmmsg 760`\n" $PROGNAME >&2
        exit
fi


#
# Get volume group information from object data manager
#



if [ -n "$VGID" ]
then
   putlvodm -k $VGID  2>/dev/null  # lock the volume group so only we change it now
   test_return $?           # check for error return
   LOCKED=y

# Determine if the volume group is varied on.
# If the volume group is varied on then output an error message
# since a volume group that is varied off can only be exported.

   lqueryvgs > /tmp/vginfo$$  2> /dev/null
   if [ $? = 0 ]
   then
       while read VGID_ON MAJOR_NUM
       do
	   if [ $VGID_ON = $VGID ]
           then 
	       exit 99
           fi
       done < /tmp/vginfo$$
   fi
   if [ $? = 99 ]
   then
       dspmsg -s 1 cmdlvm.cat 764 "`lvmmsg 764`\n" $PROGNAME >&2
       exit
   fi
fi


# Get all the of the logical volume id numbers
# Remove all evidence of their existence from object data manager.

getlvodm -L $VGNAME > /tmp/lvinfo$$  2>/dev/null
if [ ! -s "/tmp/lvinfo$$" ]
then
	odmget -q name=$VGNAME CuDep | grep dependency | cut -d"\"" -f2 >/tmp/lvinfo$$
	LVID=
fi

while read LVNAME LVID

do
        putlvodm -L $LVID 2>/dev/null            #remove LV device entries

        odmdelete -q name="$LVNAME" -o CuAt 2>&1 1>/dev/null
        odmdelete -q name="$LVNAME" -o CuDv 2>&1 1>/dev/null
        odmdelete -q dependency="$LVNAME" -o CuDep 2>&1 1>/dev/null

        #release LVMINOR number in the object data manager
        #and associated special files in /dev directory.
        lvrelminor $LVNAME

done < /tmp/lvinfo$$


#remove physical volume information from object data manager

if [ -n "$VGID" ]
then
	getlvodm -w $VGID > /tmp/pvinfo$$  2>/dev/null
	PVLIST="`cut -d' ' -f2 < /tmp/pvinfo$$`"
	while read PVID PVNAME
	do
	  putlvodm -P $PVID 2>/dev/null #remove physical volume from volume group
	  #If putlvodm failed, output warning and continue
    	  if [ $? != 0 ]
    	  then
		dspmsg -s 1 cmdlvm.cat 768 "`lvmmsg 768`\n" $PROGNAME $PVNAME >&2
		continue
    	  fi

	#
	# is this an R/W optical omd disk?
	#
	  if [ ! -z "`odmget -q \"name=${PVNAME} and \
		PdDvLn LIKE rwoptical*\" CuDv 2>/dev/null`" ] # device found
	  then
		#
		# clear the enclosure PVID, to prevent dups on importvg
		#
		PVID=`lquerypv -i`	# get a uid (unique ID)
		echo "
CuAt:
	name = \"${PVNAME}\"
	value = \"${PVID}\"
" |
		odmchange -o CuAt \
			-q "name='$PVNAME' and attribute='pvid'" \
			>/dev/null 2>&1

		if [ $? -ne 0 ]		# something happened, can't change
		then
			dspmsg -s 1 cmdlvm.cat 768 "`lvmmsg 768`\n" \
				$PROGNAME $PVNAME >&2
			continue
		fi
	  fi
	done  < /tmp/pvinfo$$
fi

putlvodm -V $VGID 2>/dev/null   #remove VGMAJOR number from object data manager
#If putlvodm failed, output warning and continue.
if [ $? != 0 ]
then
        odmdelete -q name="$VGNAME" -o CuAt 2>&1 1>/dev/null
        odmdelete -q name="$VGNAME" -o CuDv 2>&1 1>/dev/null
        odmdelete -q name="$VGNAME" -o CuDep 2>&1 1>/dev/null
fi

LOCKED=
#release minor number associated with the volume group
lvrelminor $VGNAME

#release major number associated with the volume group
lvrelmajor $VGNAME

#
# call imfs -x to export all lv's from /etc/filesystems
#
if [ -n "$PVLIST" ]
then
	imfs -x $PVLIST
fi
 
# remove mapped file
/bin/rm -f /etc/vg/vg`echo $VGID | tr "[a-z]" "[A-Z]"` > /dev/null 2>&1

EXIT_CODE=0             #branch to cleanup with success
