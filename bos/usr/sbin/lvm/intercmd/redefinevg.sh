#! /bin/bsh
# @(#)42	1.14  src/bos/usr/sbin/lvm/intercmd/redefinevg.sh, cmdlvm, bos411, 9428A410j 4/9/91 11:53:45
#
# COMPONENT_NAME: (cmdlvm) Logical Volume Commands
#
# FUNCTIONS: redefinevg.sh
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
## [End of PROLOG]

#
# FILE NAME: redefinevg.sh
#
# FILE DESCRIPTION: This shell script imports the definition of a volume
# group from a set of physical volumes.  See the AIX Commands reference
# for a full description of the importvg.
#
# RETURN VALUE DESCRIPTION:   0           Successful
#                             non-zero    Unsuccessful
#
# EXTERNAL PROCEDURES CALLED: odmget, lqueryvg, lvgenmajor, lvgenminor,
#                             getlvodm, putlvodm, mknod
#

############################# cleanup ######################################
#
# FUNCTION NAME: cleanup()
#
# DESCRIPTION: Called from main and trap command to remove /tmp files
#
cleanup()
{
   trap '' 0 1 2 15
   rm -f /tmp/pvlist$$ > /dev/null 2>&1
   if test $EXIT_CODE -eq 1  &&  test "$VGEXISTED" -eq 1
   then
	 # call mknod to recreate the node that lvrelminor deleted 
         mknod /dev/$VGNAME c $MAJOR 0 >/dev/null 2>&1
    fi

    umask $OLD_UMASK	# restore umask 

    exit $EXIT_CODE
}


################################# main #####################################
#
#  Main Function
#
#  Input:
#       Command line options and arguments: redefinevg { -d device | -i vgid } vgname
#
#  Output:
#       Error Messages (stderr)
#

VGEXISTED=

EXIT_CODE=1

OLD_UMASK=`umask`	# save old umask value
umask 117		# set umask for rw-rw---- 

ODMDIR=/etc/objrepos
export ODMDIR

# Trap on exit/interrupt/break to clean up

trap 'cleanup' 0 1 2 15

PROG=`basename $0`      # just get the basename of the command

ODMDELETE=/usr/bin/odmdelete

# Parse command line options
set -- `getopt d:i:V: $*`
if [ $? != 0 ]                # if there is a syntax error.
then
   dspmsg -s 1 cmdlvm.cat 560 "`lvmmsg 560`\n" $PROG >& 2
   exit
fi

dFLAG= ; dVAL= ; iFLAG= ; iVAL= ; vFLAG= ; vVAL=

while  [ $1  !=  -- ]         # While there is a command line option
do
   case $1 in
      -d) dFLAG='-d';  dVAL=$2;  shift;  shift;;   # key device to define VG
      -i) iFLAG='-i';  iVAL=$2;  shift;  shift;;   # id of VG to define
      -V) vFLAG='-V';  vVAL=$2;  shift;  shift;;   # major number of VG
   esac
done


# Parse command line arguments
shift       # skip past "--" from getopt
if [ $# != 1 ]
then
   dspmsg -s 1 cmdlvm.cat 560 "`lvmmsg 560`\n" $PROG >& 2
   exit
fi
VGNAME=$1


#  Read the VGID from the key devices descriptor area.
if [ -n "$dFLAG" ]
then
    KEY_VGID=`lqueryvg -p $dVAL -v`  #key device defines the VG
    if [ $? != 0 ]
    then   
       dspmsg -s 1 cmdlvm.cat 562 "`lvmmsg 562`\n" $PROG $dVAL >& 2
       exit
    fi
elif [ -n "$iFLAG" ]
then
	KEY_VGID=$iVAL
else
   dspmsg -s 1 cmdlvm.cat 560 "`lvmmsg 560`\n" $PROG >& 2
   exit
fi


# Get the list of all configured physical volumes in the database
PVS=`getlvodm -C`

if test -z "$PVS"   # couldn't find any configured PVs
then
   dspmsg -s 1 cmdlvm.cat 566 "`lvmmsg 566`\n" $PROG >& 2
   exit
fi


# From list of all configured PVs, build list containing PVNAMEs and PVIDs for
# all PVs which appear to belong to the volume group. If a PV's descriptor area
# has the key VGID, then the PV is considered in the volume group.
for name in $PVS
do
    VGID=`lqueryvg -p $name -v 2>/dev/null`
    if [ $? -eq 0 -a "$VGID" = "$KEY_VGID" ]
    then
	 PVID=`getlvodm -p $name`
	 if [ $? -eq 0   -a  -n "$PVID" ]
	 then
	    echo $PVID $name >> /tmp/pvlist$$
	 fi
    fi
done

if [ ! -s /tmp/pvlist$$ ]       # zero length file
then
     dspmsg -s 1 cmdlvm.cat 567 "`lvmmsg 567`\n" $PROG >& 2
     exit 
fi


# NOTE: In importvg we already checked to insure that VGNAME is not already
#       in use -- if used then print error message and exit, redefinevg would
#       not be called at all.
#       We decided to have the checking here also because redefinevg is
#       a separated command and in the future it might be used in other
#       places or used by itself.  When we know for sure that we do not
#       need the ckecking here anymore then we will delete it.

# if vgname is already in database but used for another VG then
#   fatal error
# else
#   if vgname is in database, save the VG's auto_on value, else set default
#   remove any existing VG entry from database in order to build fresh one
#   store all VG info in database (including all the associated PV info)
EXISTING_VGID=`getlvodm -v $VGNAME 2>/dev/null`
rc=$?
if [ $rc -eq 3 ]           # VGNAME not found in database
then
   EXISTING_VGNAME=`getlvodm -t $KEY_VGID 2>/dev/null` # is VGID in database?
   if [ $? -eq 0 ]                                     # with a different VGNAME
   then
      dspmsg -s 1 cmdlvm.cat 572 "`lvmmsg 572`\n" $PROG $EXISTING_VGNAME >& 2
      exit
   fi
elif [ $rc -eq 0  -a  "$EXISTING_VGID" != "$KEY_VGID" ]
then
   dspmsg -s 1 cmdlvm.cat 574 "`lvmmsg 574`\n" $PROG $VGNAME >& 2
   if [ -z "$EXISTING_VGID" ]
   then
      #need a new msg saying that odm is inconsistent, exportvg then importvg
      dspmsg -s 1 cmdlvm.cat 621 "`lvmmsg 621`\n" $PROG $VGNAME >& 2
   fi
   exit
fi


# check to see whether /dev entry for the volume group already existed
if [ -r /dev/"$VGNAME" ]
then
    VGEXISTED=1         # node was there
else
    VGEXISTED=0         # node was not there
fi

# use user-supplied major number or system-supplied major number.
if [ -z "$vFLAG" ]
then
   MAJOR=`lvgenmajor $VGNAME`
else
   MAJOR=`lvchkmajor $vVAL $VGNAME`
fi

if [ $? -eq 1 ]
then
   dspmsg -s 1 cmdlvm.cat 568 "`lvmmsg 568`\n" $PROG >& 2
   exit
fi

# release the minor number before trying to get it back
lvrelminor $VGNAME >/dev/null 2>&1

MINOR=`lvgenminor -p 0 $MAJOR  $VGNAME`
if [ $? != 0 ]
then
   dspmsg -s 1 cmdlvm.cat 570 "`lvmmsg 570`\n" $PROG >& 2
   exit
fi


# VG in database or not
AUTO_ON=`getlvodm -u $VGNAME 2> /dev/null`  # try to save any old value
if [ $? != 0 ]
then
   AUTO_ON="y"    # default value for new entry
fi
putlvodm -V $KEY_VGID 2> /dev/null         # delete old entry
if [ -s $ODMDELETE ]
then
	odmdelete -q name="$VGNAME" -o CuAt 2>&1 1>/dev/null
	odmdelete -q name="$VGNAME" -o CuDv 2>&1 1>/dev/null
fi
putlvodm -v $VGNAME -o $AUTO_ON $KEY_VGID  # add the new VG
if [ $? != 0 ]
then
   dspmsg -s 1 cmdlvm.cat 576 "`lvmmsg 576`\n" $PROG >& 2
   exit
fi

if test -s /tmp/pvlist$$
then
   while read PVID PVNAME
   do
       putlvodm -P $PVID 2>/dev/null      # delete old entry
       putlvodm -p $KEY_VGID $PVID        # add new entry
       if [ $? != 0 ]
       then
	  dspmsg -s 1 cmdlvm.cat 576 "`lvmmsg 576`\n" $PROG >& 2
       fi
   done < /tmp/pvlist$$
else
   dspmsg -s 1 cmdlvm.cat 567 "`lvmmsg 567`\n" $PROG >& 2
   exit 
fi


#Remove any existing node for the VG, and recreate fresh one to insure correctness
rm -f /dev/$VGNAME 2>/dev/null          # Note: lvrelminor is being bypassed
mknod /dev/$VGNAME c $MAJOR 0

EXIT_CODE=0

exit $EXIT_CODE

