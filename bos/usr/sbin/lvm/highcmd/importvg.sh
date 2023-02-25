#! /bin/bsh
# @(#)47	1.21.1.8  src/bos/usr/sbin/lvm/highcmd/importvg.sh, cmdlvm, bos411, 9438C411a 9/23/94 17:43:36
#
# COMPONENT_NAME: (cmdlvm) Logical Volume Commands
#
# FUNCTIONS: importvg.sh
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
## [End of PROLOG]


#
# FILE NAME: importvg
#
# FILE DESCRIPTION: This shell script imports the definition of a volume
# group from a set of physical volumes.
#
# RETURN VALUE DESCRIPTION:
#                             0           Successful
#                             non-zero    Unsuccessful
#
# EXTERNAL PROCEDURES CALLED: getlvodm, getvgname, redefinevg, varyonvg, synclvodm, varyoffvg
#

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
# NOTE: This function will not return if input value is invalid. (ie. it will
#       exit with exit status of 1. )
#
#
cleanup()
{
   trap '' 0 1 2 15

   if [ "$DEFINEDVG" -eq 1  -a  "$EXIT_CODE" -eq 1 ]
   then
	getlvodm -L $VGID > /tmp/lvinfo$$ 2>/dev/null
	if test -s /tmp/lvinfo$$
	then
	     while read LVNAME LVID
	     do
		 putlvodm -L $LVID 2>/dev/null  #remove LV device entries
                 if [ $? -ne 0 -a -s $ODMDELETE ]
                 then
                    odmdelete -q name="$LVNAME" -o CuAt 2>&1 1>/dev/null
                    odmdelete -q name="$LVNAME" -o CuDv 2>&1 1>/dev/null
                    odmdelete -q dependency="$LVNAME" -o CuDep 2>&1 1>/dev/null
                 fi

		 #release LVMINOR number in the object data manager
		 #and associated special files in /dev directory.
		 lvrelminor $LVNAME 2>/dev/null
	     done < /tmp/lvinfo$$
	fi
	rm -f /tmp/lvinfo$$

	#remove physical volume information from object data manager
	getlvodm -w $VGID > /tmp/pvinfo$$ 2>/dev/null
	if test -s /tmp/pvinfo$$
	then
	    while read PVID PVNAME
	    do
		putlvodm -P $PVID 2>/dev/null   #remove PV from VG
	    done  < /tmp/pvinfo$$
	    rm -f /tmp/pvinfo$$   #clean up the temp file right away

	    putlvodm -V $VGID 2>/dev/null     #remove VGMAJOR number from odm
            if [ $? -ne 0 -a -s $ODMDELETE ]
            then
               odmdelete -q name="$VGNAME" -o CuAt 2>&1 1>/dev/null
               odmdelete -q name="$VGNAME" -o CuDv 2>&1 1>/dev/null
               odmdelete -q name="$VGNAME" -o CuDep 2>&1 1>/dev/null
            fi

	    #release minor number associated with the volume group
	    lvrelminor $VGNAME 2>/dev/null

	    #release major number associated with the volume group
	    lvrelmajor $VGNAME 2>/dev/null
	fi
   fi

   if [ "$LOCKEDVG" -eq 1 ]
   then
       putlvodm -K $VGID >/dev/null 2>&1     # unlock volume group
   fi


   getlvodm -R 
   if [ $? -eq 0 ]
   then
	   savebase	#save the database to the ipl_device
   fi

   exit $EXIT_CODE
}



################################ main #####################################
#  Import logical volume
#  Input:
#       Command line options and arguments:
#       importvg [-y vgname] PhysicalVolume
#
#  Output:
#       Error Messages (Standard error)
#

EXIT_CODE=1
DEFINEDVG=0
LOCKEDVG=0

#
# Trap on exit/interrupt/break to clean up
#

trap 'cleanup' 0 1 2 15

ODMDIR=/etc/objrepos
export ODMDIR

PROG=`basename $0`      # just get the basename of the command

ODMDELETE=/usr/bin/odmdelete

# Parse command line options
set -- `getopt y:V:f $*`
if [ $? != 0 ]                # if there is a syntax error.
then
   dspmsg -s 1 cmdlvm.cat 774 "`lvmmsg 774`\n" $PROG >& 2
   exit
fi

yFLAG= ; yVAL= ; vFLAG= ; vVAL= ; fFLAG= ;

while  [ $1  !=  -- ]         # While there is a command line option
do
   case $1 in
	   -y) yFLAG='-n';  yVAL=$2;  shift;  shift;;   #vgname
	   -V) vFLAG='-V';  vVAL=$2;  shift;  shift;;   #major number
	   -f) fFLAG='-f';	shift;						#force varyon
   esac
done

# Parse command line arguments
shift     # skip past "--" from getopt
if [ $# -eq 1 ]       #if key_device name argument on command line
then
   KEY_DEVICE=$1
else
   dspmsg -s 1 cmdlvm.cat 604 "`lvmmsg 604`\n" $PROG >& 2
   dspmsg -s 1 cmdlvm.cat 774 "`lvmmsg 774`\n" $PROG >& 2
   exit
fi

# Before performing any checks, check yVAL (vgname) for any
# non-alphanumeric characters. Do not allow the vg to get
# imported with a bad vgname.

# the case statement checks all the invalid characters:
# 	<space> through ,   
# 	/
# 	: through @
# 	[ through ^
# 	`
# 	{ through ~
#
# this leaves only the valid cases [A-Z][a-z][0-9][.-_]
case $yVAL in 
	*[\ -,/:-@[-\^\`\{-~]*)
        	#Invalid character in vgname
        	dspmsg -s 1 cmdlvm.cat 874 "`lvmmsg 874`\n" importvg $yVAL >& 2
        	exit;;
esac

TVGID=`getlvodm -v $yVAL 2>/dev/null` # is VGNAME defined in ODM?
rc1=$?					 # without a VGID
if [ $rc1 -eq 0 -a  -z "$TVGID" -a -s $ODMDELETE ]
then
    odmdelete -q name="$yVAL" -o CuAt >/dev/null
    odmdelete -q name="$yVAL" -o CuDv >/dev/null
fi

# Get the name for the new volume group definition.
VGNAME=`getvgname $yFLAG $yVAL`
if [ $? -ne 0 ]
then
   dspmsg -s 1 cmdlvm.cat 776 "`lvmmsg 776`\n" $PROG $KEY_DEVICE $yVAL >& 2
   exit
fi


# Confirm the VG name is not already used -- if in ODM then used.
getlvodm -v $VGNAME > /dev/null 2>&1
if [ $? -eq 0 ]
then
   dspmsg -s 1 cmdlvm.cat 778 "`lvmmsg 778`\n" $PROG $VGNAME >& 2
   exit
fi

# Make sure that the vgname is not the same as one of the lvnames of the same
# volume group.
lqueryvg -p $KEY_DEVICE -L | grep " $VGNAME " > /dev/null
if [ $? -eq 0 ]
then
   dspmsg -s 1 cmdlvm.cat 776 "`lvmmsg 776`\n" $PROG $KEY_DEVICE $yVAL >& 2
   exit
fi

# If it is an optical device "omd", 
# check if this phycial device belongs to a volume group.
# if so, command reject
if [ -x /usr/bin/odmget ]
then
    if [ ! -z "`odmget -q \"name=${KEY_DEVICE} and \
	PdDvLn LIKE rwoptical*\" CuDv 2>/dev/null`" ] # omd device found
    then
        set `getlvodm -P | grep $KEY_DEVICE 2>/dev/null`
        ENCLPVID=$2			# enclosure PVID in ODM
        OLDVGNAME=$3
        expr $OLDVGNAME : "\([Nn][Oo][Nn][Ee]\)" >/dev/null 2>/dev/null
        if [ $? -ne 0 ]
        then
            dspmsg -s 1 cmdlvm.cat 1031 "`lvmmsg 1031`\n" $PROG $VGNAME $KEY_DEVICE
	    exit
        fi

        MEDIAPVID=`lqueryvg -p $KEY_DEVICE -P | cut -c1-16 2>/dev/null`
        if  [ $? -eq 0 ]	# volume group previous defined
        then
	    set $MEDIAPVID	# how many items in this list
	    if [ $# -ne 1 ]	# something's wrong, there's more PVs than expected
	    then
   		dspmsg -s 1 cmdlvm.cat 780 "`lvmmsg 780`\n" \
			$PROG $KEY_DEVICE >& 2
   		exit
	    fi

	    if [ "$ENCLPVID" != "$MEDIAPVID" ]	# they don't match
	    then
		# update the stanza with the MEDIAPVID from the optical disk
		
		echo "
CuAt:
	name = \"${KEY_DEVICE}\"
	value = \"${MEDIAPVID}0000000000000000\"
" |
		odmchange -o CuAt \
			-q "name='$KEY_DEVICE' and attribute='pvid'" \
			>/dev/null 2>&1

		if [ $? -ne 0 ]		# something happened, can't change
		then
   			dspmsg -s 1 cmdlvm.cat 780 "`lvmmsg 780`\n" \
				$PROG $KEY_DEVICE >& 2
   			exit
		fi
	    fi
        fi
    fi
fi

# Define the volume group in ODM -- at least enough info to varyon.
if [ -z "$vFLAG" ]
then
   redefinevg -d $KEY_DEVICE $VGNAME
else
   redefinevg -d $KEY_DEVICE -V $vVAL $VGNAME
fi

if [ $? -ne 0 ]
then
   dspmsg -s 1 cmdlvm.cat 780 "`lvmmsg 780`\n" $PROG $KEY_DEVICE >& 2
   exit
fi
DEFINEDVG=1
VGID=`getlvodm -v $VGNAME`   # get the VGID from odm


# Varyon the volume group so accurate VG information can be obtained.
if [ -z "$fFLAG" ]
then
    varyonvg -n $VGNAME
else
    varyonvg -n -f $VGNAME       # force varyon
fi

if [ $? -ne 0 ]
then
   dspmsg -s 1 cmdlvm.cat 780 "`lvmmsg 780`\n" $PROG $KEY_DEVICE >& 2
   exit
fi

putlvodm -k $VGID      # lock the volume group
if [ $? -ne 0 ]
then
   dspmsg -s 1 cmdlvm.cat 780 "`lvmmsg 780`\n" $PROG $KEY_DEVICE >& 2
   exit
fi
LOCKEDVG=1

# synchronize the lvcb, the database and the volume group information in LVM
synclvodm $VGNAME
rc=$?
if [ $rc -ne 0 ]
then
   if [ $rc -eq 1 ]
   then
       dspmsg -s 1 cmdlvm.cat 780 "`lvmmsg 780`\n" $PROG $KEY_DEVICE >& 2
       exit              # exit if fatal error from synclvodm
   else
       dspmsg -s 1 cmdlvm.cat 782 "`lvmmsg 782`\n" $PROG $KEY_DEVICE >& 2
   fi
fi


# Display the name of VG imported.
echo $VGNAME

putlvodm -K $VGID
LOCKEDVG=0

# If imfs command exists, call it to update file systems
if [ -x /usr/sbin/imfs ]
then
    /usr/sbin/imfs $VGNAME
fi

if [ "$EXIT_CODE" -ne 2 ]
then
   EXIT_CODE=0      #Reset exit code to indicate successful completion of varyon
fi

exit
