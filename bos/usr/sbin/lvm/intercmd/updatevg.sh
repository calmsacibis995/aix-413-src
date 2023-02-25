#! /bin/bsh
# @(#)45	1.9  src/bos/usr/sbin/lvm/intercmd/updatevg.sh, cmdlvm, bos411, 9428A410j 6/15/90 21:59:37
#
# COMPONENT_NAME: (cmdlvm) Logical Volume Commands
#
# FUNCTIONS: updatevg.sh
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
# FILE NAME: updatevg.sh
#
# FILE DESCRIPTION: This shell script resyncs the definition of a volume
# group in the database.  The LVM information and the logical volume
# control block are both used to do the resync.  The volume group must
# be varied on when this command is executed.
#
# RETURN VALUE DESCRIPTION:   0           Successful
#                             non-zero    Unsuccessful
#
#
# EXTERNAL PROCEDURES CALLED:
#

############################ cleanup ########################################
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
#       EXIT_CODE
#
# NOTE: This function will not return.
#
#
cleanup()
{
    trap '' 0 1 2 15
    rm -f /tmp/DBLIST$$
    rm -f /tmp/dblist$$ /tmp/dbpvlist$$ /tmp/pvlist$$
    exit $EXIT_CODE
}


############################ update_odm #####################################
#
# NAME: update_odm()
#
# DESCRIPTION: get the physical volume name and add physical volume to the
#              data base
#
# INPUT: none
#
# OUTPUT:
#        Error messages  (Standard Error)
#
# RETURN VALUE DESCRIPTION:
#        0      Successful
#
#
#
update_odm()
{
    read pvid PVNAME dummy < /tmp/dblist$$
    putlvodm -p $KEY_VGID $lvpvid     # add pv to data base
    if [ $? -ne 0 ]       # print warning message if cannot access odm
    then
	dspmsg -s 1 cmdlvm.cat 508 "`lvmmsg 508`\n" $PROG $PVNAME >& 2
	EXIT_CODE=2
    else
    	PVCOUNT=1	 # at least one PV is updated in database
    fi
}


################################ main #####################################
#  Update Database for Volume Group
#
#  Input:
#       Command line options and arguments: updatevg vgdescript
#
#  Output:
#       Error Messages (Standard error)
#

ODMDIR=/etc/objrepos
export ODMDIR

hash getlvodm getlvcb lqueryvg putlvcb putlvodm
EXIT_CODE=1           #Initialize exit code. This will be reset to 0 before
		      #exiting only if updatevg completes successfully.
#
# Trap on exit/interrupt/break to clean up
#

trap 'cleanup' 0 1 2 15

PROG=`basename $0`      # just get the basename of the command

# Confirm command line argument was passed as input.
if [ $# != 1 ]
then
    dspmsg -s 1 cmdlvm.cat 500 "`lvmmsg 500`\n" $PROG >& 2
    exit
fi
VGDESCRIPT=$1        # vgdescript from the command line

# Get the VGID from the data base.
KEY_VGID=`getlvodm -v $VGDESCRIPT`
if [ $? != 0 ]
then
   dspmsg -s 1 cmdlvm.cat 502 "`lvmmsg 502`\n" $PROG $VGDESCRIPT >& 2
   exit
fi



# Get from LVM map file the list of PV ids in the VG.
lqueryvg -g $KEY_VGID -P > /tmp/pvlist$$
if [ $? != 0 ]
then
   dspmsg -s 1 cmdlvm.cat 502 "`lvmmsg 502`\n" $PROG $VGDESCRIPT >& 2
   exit
fi
LVMPVS=`cut -d" " -f1 /tmp/pvlist$$`
if test -z "$LVMPVS"  # couldn't find any PVs in the VG
then
   dspmsg -s 1 cmdlvm.cat 506 "`lvmmsg 506`\n" $PROG >& 2
   exit
fi



# Get the list of all configured physical volumes in the database
PVS=`getlvodm -C`
if test -z "$PVS"  # couldn't find any configured PVs
then
   dspmsg -s 1 cmdlvm.cat 506 "`lvmmsg 506`\n" $PROG >& 2
   exit
fi

# From list of all configured PVs, build list containing PVNAMEs
# and PVIDs for all PVs which appear to belong to the volume group.
for name in $PVS
do
    VGID=`lqueryvg -p $name -v 2>/dev/null`
    if [ $? -eq 0 -a "$VGID" = "$KEY_VGID" ]
    then
       PVID=`getlvodm -p $name`
       if [ $? -eq 0  -a  -n "$PVID" ]
       then
           echo $PVID $name >> /tmp/DBLIST$$
       fi
    fi
done



# Remove from the database all PVs allocated to the volume group KEY_VGID.
# This insures no extras seen, the valid PV entries will be rebuilt.

getlvodm -w $KEY_VGID > /tmp/dbpvlist$$  # save both pvids and names in file
if [ $? -eq 0 ]
then
   DBPVIDS=`cut -d" " -f1 /tmp/dbpvlist$$`        # get pvids only
   for dbpvid in $DBPVIDS
   do
       putlvodm -P $dbpvid     # remove the pv from the database
       if [ $? -ne 0 ]
       then
	  dspmsg -s 1 cmdlvm.cat 508 "`lvmmsg 508`\n" $PROG $dbpvid >& 2
	  EXIT_CODE=2
       fi
   done
fi



# For each PVID in the LVM list, check if it matches one of the
# configured PVs which appear to belong to the volume group.  If
# match, then insure the PV id and name are put in the database.

PVCOUNT=0	 # reset the count to 0
for lvpvid in $LVMPVS
do
    if test -s /tmp/DBLIST$$  &&  grep -i $lvpvid /tmp/DBLIST$$ >/tmp/dblist$$
    then         # lvpvid matches one of the configured PVs in the DBLIST
	  update_odm

    # none in DBLIST matches lvpvid, search in database dbpvlist file if
    # file is nonzero
    elif test -s /tmp/dbpvlist$$  &&  grep -i $lvpvid /tmp/dbpvlist$$ > /tmp/dblist$$
    then           # if there is at least 1 entry
	  update_odm

    else     # not found in DBLIST and dbpvlist has zero length
	  dspmsg -s 1 cmdlvm.cat 510 "`lvmmsg 510`\n" $PROG $lvpvid >& 2
	  EXIT_CODE=2
    fi
done

if [ "$EXIT_CODE" -ne 2 ]
then
   EXIT_CODE=0          # reset exit code to indicate successful completion
elif [ "$EXIT_CODE" -eq 2 -a "$PVCOUNT" -eq 0 ]
then
   EXIT_CODE=1		# could not update any PV in the volume group exit 1   
fi

exit                    # trap will handle clean up
