#! /bin/bsh
# @(#)80	1.7.2.13  src/bos/usr/sbin/lvm/highcmd/migratepv.sh, cmdlvm, bos41J, 9511A_all 3/8/95 12:55:49
#
# COMPONENT_NAME: (cmdlvm) Logical Volume Commands
#
# FUNCTIONS: migratepv
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
# FILE NAME: migratepv
#
# FILE DESCRIPTION: High-level logical volume shell command for moving
#                   allocated physical partitions contained on one
#                   physical volume to one or more other physical volumes.
#                   See the AIX Commands Reference manual for additional
#                   information.
#
#
# RETURN VALUE DESCRIPTION:
#	0	Successful
#	1	Unsuccessful
#
# EXTERNAL PROCEDURES CALLED:
#	lmigratepp
#	getlvodm
#	dspmsg
#	lquerylv,lquerypv,lqueryvg
#	sort, grep, awk
#

########################### create_dpvlist ##############################
#
#
# NAME: create_dpvlist()
#
# DESCRIPTION: Creates a default list of destination physical volumes.  Will
#              be called if destination physical volumes were not specified
#              on the command line. The default list will contain all physical
#              volumes - except for the source physical volume - that are in
#              the volume group.
#
#
# INPUT: none
#
# OUTPUT:
#        Error messages (Standard Error)
#
# RETURN VALUE DESCRIPTION:
#                           0   Successful
#
# NOTE: This function will not return if an error is encountered.
#
create_dpvlist()
{

   lqueryvg -g $VGID -P > /tmp/tempa$$  # Call lqueryvg with -P to get list
                                        # of pvids and their associated states
   test_return $?                       # Fatal error if lqueryvg fails.

   # Acceptable physical volume states are ACTIVE or ACTIVE/STALE.
   # Possible pv states returned from lqueryvg are as follows:
   # 1 (ACTIVE), 2 (MISSING), 4 (REMOVED), 8 (NOALLOC), and 16 (STALE)
   # Therefore, the ACTIVE bit MUST be set and the STALE bit MAY or MAY NOT
   # be set.  All other state bits MUST NOT be set.  The following creates
   # the default destination pv list with only those pvs that have an
   # acceptable state. It also excludes the source pv from the list.

   DPVLIST=`awk "{if ((\\$3 == 1 || \\$3 == 17) && \
${SPVID} != \\$1) print \\$1}" /tmp/tempa$$`

}

########################### check_dpvlist ################################
#
# NAME: check_dpvlist()
#
# DESCRIPTION: Checks the specified list of destination physical volumes
#              to make sure the pv states are set to only ACTIVE or ACTIVE and
#              STALE -- and that the source pv is not included in this list
#              of destination pvs -- and that each pv belongs to the source
#              pv's volume group.
#
# INPUT: none
#
# OUTPUT:
#        Error messages (Standard Error)
#
# RETURN VALUE DESCRIPTION:
#                           0   Successful
#
# NOTE: This function will not return if an error is encountered.
#
check_dpvlist()
{
   for PVNAME in $DESTPVS
   do

      DPVID=`getlvodm -p $PVNAME`     # Call getlvodm passing pvname for pvid.
      test_return $?                  # Treat as fatal if unable to obtain

      DPVLIST="$DPVLIST $DPVID"       # Collect dpvids in $DPVLIST

      if [ "$DPVID" = "$SPVID" ]      # Check to make sure source pv
      then                            # is not included in list of dest pvs.
	 dspmsg -s 1 cmdlvm.cat 804 "`lvmmsg 804`\n" migratepv $PVNAME >& 2
         cleanup
      fi

      DVGID=`getlvodm -j $PVNAME`     # Must check to make sure this pv is in
      test_return $?                  # same volume group as the source pv.
                                      # If the id can't be obtained, fatal 
      if [ "$DVGID" != "$VGID" ]
      then
	 dspmsg -s 1 cmdlvm.cat 806 "`lvmmsg 806`\n" migratepv $PVNAME >& 2
         cleanup
      fi

      DPVSTATE=`lquerypv -g $VGID -p $DPVID -c`  # Get this pv's state.
      test_return $?

      if [ $DPVSTATE -ne $ACTIVE  -a  $DPVSTATE -ne $ACTIVE_STALE ]
      then
	 dspmsg -s 1 cmdlvm.cat 808 "`lvmmsg 808`\n" migratepv $PVNAME >& 2
         cleanup
      fi

   done

}

########################### check_spv ################################
#
# NAME: check_spv()
#
# DESCRIPTION: Checks source physical volume to make sure the pv
#	       states are set to only ACTIVE or ACTIVE and STALE
#
# INPUT: none
#
# OUTPUT:
#        Error messages (Standard Error)
#
# RETURN VALUE DESCRIPTION:
#                           0   Successful
#
# NOTE: This function will not return if an error is encountered.
#
check_spv()
{
      SPVSTATE=`lquerypv -g $VGID -p $SPVID -c`  # Get this pv's state.
      test_return $?

      if [ $SPVSTATE -ne $ACTIVE  -a  $SPVSTATE -ne $ACTIVE_STALE \
				  -a  $SPVSTATE -ne $ACTIVE_NOALLOC ]
      then
	 dspmsg -s 1 cmdlvm.cat 817 "`lvmmsg 817`\n" migratepv $SOURCEPV >& 2
         cleanup
      fi
}

########################## count_spvptns ####################################
#
# NAME: count_spvptns()
#
# DESCRIPTION: Counts number of partitions in the source physical volume to
#              be migrated.  If the command line logical volume option
#              wasn't specified, it will include all partitions that are
#              allocated.  Otherwise, it will include all partitions that
#              have been allocated specifically to the specified lv.
#
# INPUT: none
#
# OUTPUT:
#        Error messages (Standard Error)
#
# RETURN VALUE DESCRIPTION:
#                           0   Successful
#
# NOTE: This function will not return if an error is encountered.
#
count_spvptns()
{
   if [ -z "$LFLAG" ]         #if -l option was not specified on command line,
   then                       #count all partitions on the pv that have been
                              #allocated.
      SPTN_COUNT=`lquerypv -g $VGID -p $SPVID -n` # Call lquerypv with the -n
      test_return $?                     # get the # of allocated partitions
                                         # on this pv. 
                                         #  Treat lquerypv -g $VGID failure
                                         # as fatal error.
   else

      # Get count of partitions in the lv
      SPTN_COUNT=`lquerylv -L $LVID -r | grep $SPVID | wc -l`
      test_return $?

   fi
}

########################### make_dpvmap ######################################
#
#
# NAME: make_dpvmap()
#
# DESCRIPTION: Prepares one destination physical volume map by concatenating
#              all of the destination pv maps. Sets $DPTN_COUNT variable
#              to the number of unallocated physical partitions in this
#              new destination physical volume map.
#
# INPUT: none
#
# OUTPUT:
#        Error messages (Standard Error)
#
# RETURN VALUE DESCRIPTION:
#                           0   Successful
#
# NOTE: This function will not return if an error is encountered.
#
make_dpvmap()
{

   > /tmp/dpvmap$$
   for DEST_PVID in $DPVLIST
   do
          lquerypv -g $VGID -p $DEST_PVID -dt >> /tmp/dpvmap$$ 
          test_return $?                           # get the dpv map.
   done

   # if /tmp/dpvmap$$ is empty, it is no space available
   L_OF_MAP=`wc -l < /tmp/dpvmap$$`
   if [ $L_OF_MAP = 0 ]
   then
   	dspmsg -s 1 cmdlvm.cat 810 "`lvmmsg 810`\n" migratepv >& 2
	test_return 1
   fi
}

count_free()
{
    # Get the number of unallocated partitions in the pvs by subtracting the
    # number of allocated partitions from the number of partitions in the pv.
        DPTN_COUNT=0   # Initialize $DPTN_COUNT to zero
	for DEST_PVID in $DPVLIST
	do
		TOT_PTNS=`lquerypv -g $VGID -p $DEST_PVID -P`     # total pp's
		test_return $? 
		USED_PTNS=`lquerypv -g $VGID -p $DEST_PVID -n` 
				# Gives the number of pp's already allocated
		test_return $? 
		FREE_PTNS=`expr $TOT_PTNS - $USED_PTNS`
				# Get the number of unallocated pp's
		DPTN_COUNT=`expr $DPTN_COUNT + $FREE_PTNS` 
				# Keep a running count of the
                                # number of free pp's in dpvmap$$
	done
}


########################### get_slvlist ######################################
#
#
# NAME: get_slvlist()
#
# DESCRIPTION: Gets a sorted list of lvids on the source  volume.
#
# INPUT: none
#
# OUTPUT:
#        Error messages (Standard Error)
#
# RETURN VALUE DESCRIPTION:
#                           0   Successful
#
# NOTE: This function will not return if an error is encountered.
#
get_slvlist()
{
   lquerypv -g $VGID -p $SPVID -d > /tmp/spvmap$$
			# Get the map of the source physical
			# volume.  Treat as a fatal error if
   test_return $?       # this is not possible.

   SLVLIST=`awk '{print $4}' /tmp/spvmap$$ | grep -v 0000000000000000.0 \
   | sort -u`
}


########################## make_allocpin ####################################
#
#
# NAME: make_allocpin()
#
# DESCRIPTION: Constructs the input map for allocp by concatenating the
#              destination pvmap with the current lvmap.  Changes
#              "LVMAP:" tokens to "MODIFY:" for all lvmap entries that
#              reside on the source physical volume.
#
# INPUT: none
#
# OUTPUT:
#        Error messages (Standard Error)
#
# RETURN VALUE DESCRIPTION:
#                           0   Successful
#
# NOTE: This function will not return if an error is encountered.
#
make_allocpin()
{

   # Get this logical volume's map.  Change the "LVMAP:" token to "MODIFY:"
   # for each entry that resides on the source physical volume.

   lquerylv -L $LVID -dlt > /tmp/tempa$$
   test_return $?

   LV_LP_CNT=`lquerylv -L $LVID -c`
   TRUE_LV_CNT=`cat /tmp/tempa$$ | wc -l`
   MIRR_CNT=`expr $TRUE_LV_CNT / $LV_LP_CNT`

   # This must be calculated for cases where all copies of the lv
   # reside on the same disk.  If this happens, allocp needs to know
   # that it must move than one set of lvs.

   grep $SPVID /tmp/tempa$$ | sed 's/LVMAP/MODIFY/'  > /tmp/tempb$$

   # create the input file for allocp
   #   1. destination pv maps
   #   2. all pp for the lv not on the source pv
   #   3. modify map

   cat /tmp/dpvmap$$ /tmp/tempa$$ /tmp/tempb$$ > /tmp/allocpin$$

}


########################## default_lvchars ##################################
#
#
# NAME: default_lvchars()
#
# DESCRIPTION: Sets up pertinent default logical volume characteristics
#              required for call to allocp.  Will be used only if the
#              current lv's characteristics cannot be obtained from odm.
#
# INPUT: none
#
# OUTPUT:
#        Error messages (Standard Error)
#
# RETURN VALUE DESCRIPTION:
#                           0   Successful
#
#
default_lvchars()
{
   AVAL=m    # Intra-policy defaults to m for middle

   EVAL=x    # Inter-policy defaults to x for maximum

   UVAL=32   # Upperbound defaults to max number of pvs

   SFLAG=""  # Strict policy defaults to null for stict
}


########################### make_migmap ####################################
#
#
# NAME: make_migmap()
#
# DESCRIPTION: Constructs the input map for migrate command in the following
#              format:    spvid slpnum sppnum  dpvid dlpnum dppnum
#
# INPUT: none
#
# OUTPUT:
#        Error messages (Standard Error)
#
# RETURN VALUE DESCRIPTION:
#                           0   Successful
#
# NOTE: This function will not return if an error is encountered.
#
make_migmap()
{
   # Call lquerylv with the -r option to get the logical volume
   # map for the current lv in the following format:  pvid ppnum lpnum

   lquerylv -L $LVID -r > /tmp/tempa$$
   test_return $?

   # Extract and sort (by lpn) those lp entries that reside on the source pv

   grep $SPVID /tmp/tempa$$ | sort +2 -n > /tmp/from_file$$

   # Sort allocp output by lpn

   cat /tmp/alloc_out$$ | sort +2 -n > /tmp/to_file$$

   # Paste together corresponding entries from from_file and to_file to
   # create the required input tuple for migrate.

   paste -d" " /tmp/from_file$$ /tmp/to_file$$ > /tmp/mig_map$$

   rm -f /tmp/from_file$$ /tmp/to_file$$

}

########################### test_return ##################################
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
   then             # Unsuccessful - give general error message and exit ...
      dspmsg -s 1 cmdlvm.cat 812 "`lvmmsg 812`\n" migratepv >& 2
      cleanup
   fi
}

############################ cleanup #####################################
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

   if test $LOCKEDVG -eq 1
   then
   	putlvodm -K $VGID   # Unlock the volume group
	LOCKEDVG=0
   fi

   rm -f /tmp/tempa$$     /tmp/tempb$$   /tmp/tempc$$    /tmp/tempd$$ \
         /tmp/dpvmap$$    /tmp/spvmap$$  /tmp/allocpin$$ /tmp/alloc_out$$ \
         /tmp/from_file$$ /tmp/to_file$$ /tmp/mig_map$$ /tmp/dpvmap$$
   rm -f /tmp/pvmap$$ /tmp/freemap$$ /tmp/ppmap$$

   exit $EXIT_CODE
}

#
#   Usage: migrate vgid migfixmap
#
#   for every line in map construct a lmigratepp call.
#   if any lmigratepp call fails then this routine will exit.
#
migrate_it()
{
        awk  '{print "-p "$1" -n "$2" -P "$4" -N "$5}' |
	(while migrate=`line`
	do
		lmigratepp -g $VGID $migrate
		test_return $?
	done)
}
############################ is_a_boot_lv ###################################
#
# NAME: is_a_boot_lv()
#
# DESCRIPTION:
#
# INPUT: none
#
# OUTPUT:
#        Error messages  (Standard Error)
#
# RETURN VALUE DESCRIPTION:	none
#
#
is_a_boot_lv()
{
	test "`getlvodm -c $1 | awk '{ print $1 }'`" = "boot"
}

######################## check_for_boot_lvs ##################################
#
# NAME: check_for_boot_lvs()
#
# DESCRIPTION:
#
# INPUT: none
#
# OUTPUT:
#        Error messages  (Standard Error)
#
# RETURN VALUE DESCRIPTION:	none
#
#
check_for_boot_lvs()
{
	#
	# walk thru the lvid list, and check the lv types.
	# make a list (BOOTLIST) of the boot lv's
	#
	BOOTLIST=
	for LVID in $SLVLIST
	do
		if is_a_boot_lv $LVID
		then
			BOOTLIST="$BOOTLIST $LVID"
		fi
	done

	if [ -n "$BOOTLIST" ]
	then
		for lv in $BOOTLIST
		do
	 		dspmsg -s 1 cmdlvm.cat 1011 "`lvmmsg 1011`\n" migratepv `getlvodm -e $lv` >& 2
		done
		test_return 1
	fi
}

######################## make_contig_lv_map ##################################
#
# NAME: make_contig_lv_map()
#
# DESCRIPTION:
#	$1 is lvid we're migrating from
#
# INPUT: none
#
# OUTPUT:
#	Error messages  (Standard Error)
#	name of pv we migrated to (stdout)
#
# RETURN VALUE DESCRIPTION:	none
#
#
make_contig_lv_map()
{
	# get size of current lv
	PPS=`lquerylv -L $LVID -c`
	test_return $?

	# loop through DESTPVS looking for a pv that has enough
	# contiguous pp's to support this lv
	for PVNAME in $DESTPVS
	do
		PVID=`getlvodm -p $PVNAME`	# get pvid
		test_return $?

		# get the physical volumes allocation map
		lquerypv -p $PVID -g $VGID -d -t > /tmp/pvmap$$
		test_return $?
	
		# get all the free partitions in the PV map
		grep "0 ODMtype" /tmp/pvmap$$ > /tmp/freemap$$

		# loop through /tmp/freemap$$, looking for contiguous pps

		> /tmp/ppmap$$
		LASTPP=0
		COUNT=0
		cat /tmp/freemap$$ |
		while read LINE 
		do
			# cut the PP number out of a map line	
			PV=`echo $LINE | cut -d" " -f2`
			PP=`echo $PV | cut -d":" -f2`

			TEMP=`expr $PP - 1`
			
			if [ "$TEMP" = "$LASTPP" ] 
			then 
				# if this another sequential PP then add to list
				echo $PV >> /tmp/ppmap$$
				COUNT=`expr $COUNT + 1`
			else
				# if not sequential PP then start new list
				echo $PV > /tmp/ppmap$$
				COUNT=1
			fi
			LASTPP=$PP
		
			# if the requested PPs have been found
			if [ "$COUNT" = "$PPS" ] 
			then
				break
			fi
		done

		if [ `wc -l < /tmp/ppmap$$` = $PPS ]
		then
			echo $PVNAME
			sed -e 's/:/ /' < /tmp/ppmap$$ > /tmp/alloc_out$$
			return 0
		fi
	done

	dspmsg -s 1 cmdlvm.cat 1013 "`lvmmsg 1013`\n" migratepv `getlvodm -e $LVID` >& 2
	return 1
}

######################### check_for_dump_device ###############################
#
# NAME: check_for_dump_device()
#
# DESCRIPTION: function determines if lv is defined as primary or secondary
#              dump device.
#
# INPUT: LVID
#
# OUTPUT: LVNAME
#
# RETURN VALUE DESCRIPTION:     0 if not a dump device
#                               1 if primary dump device
#                               2 if secondary dump device
#
#
check_for_dump_device()
{
LVNAME=`getlvodm -a $1`

	if [ "`sysdumpdev -l | grep primary | awk '{ print $2 }'`" = "/dev/$LVNAME" ]
	then
		# this is the primary dump device
		return 1
	fi

	if [ "`sysdumpdev -l | grep secondary | awk '{ print $2 }'`" = "/dev/$LVNAME" ]
	then
		# this is the secondary dump device
		return 2
	fi

	# else this is not a dump device
	return 0
}

############################## main #######################################
#  Migrate Physical Volume
#  Input:
#       Command line options and arguments:
#       migratepv [-i] [-l lvname] sourcepv [destinationpv...]
#  Output:
#       Error Messages (Standard error)
#
########### hash

LOCKEDVG=0

EXIT_CODE=1           #Initialize exit code. This will be reset to 0 before
                      #exiting only if mklv completes successfully.

#
# Trap on exit/interrupt/break to clean up
#

trap 'cleanup' 0 1 2 15 

# Define some constant values

ACTIVE=1

ACTIVE_STALE=17

ACTIVE_NOALLOC=9

#
# Parse command line options
#

set -- `getopt il: $*`

if [ $? != 0 ]                      # Determine if there is a syntax error.
then
   dspmsg -s 1 cmdlvm.cat 800 "`lvmmsg 800`\n" migratepv >& 2
   cleanup
fi

IFLAG= ; LFLAG= ; LVAL= 

while  [ $1  !=  -- ]               # While there is a command line option
do
   case $1 in
           -i) IFLAG='-i';  shift;;      # destinationpv's read from stdin
           -l) LFLAG='-l'; LVAL=$2; shift; shift;;     # lvname(only move ptns
                                                       # allocated to lvname )
   esac
done   #end - while there is a command line option

#
# Parse command line arguments
#

shift     # skip past "--" from getopt

if test $# -ne 0        #if sourcepv is on the command line
then
   SOURCEPV=$1
else
   dspmsg -s 1 cmdlvm.cat 612 "`lvmmsg 612`\n" migratepv >& 2
   dspmsg -s 1 cmdlvm.cat 800 "`lvmmsg 800`\n" migratepv >& 2
   cleanup
fi

shift    # skip to next command line argument

if [ -z "$IFLAG" ]
then
	DESTPVS="$*"

else   # i option specified -- read destpv's from standard in

   while read LINE
   do
      DESTPVS="$DESTPVS $LINE"      # save destination pvs in $DESTPVS
   done

fi     # end if -i option specified

# First, check to see if the disk named even holds a copy of
# the lv that they want to remove.
if [ -n "$LVAL" ]
then
	PVID=`getlvodm -p $SOURCEPV`
        LV_BY_ID=`getlvodm -l $LVAL`
        DISK_CHECK=`lquerylv -L $LV_BY_ID -r | grep "^$PVID "`
	if [ -z "$DISK_CHECK" ]
	then
   		dspmsg -s 1 cmdlvm.cat 681 "`lvmmsg 681`\n" migratepv >& 2
		exit
	fi
fi



VGID=`getlvodm -j $SOURCEPV`  # VGID will be necessary. Must treat as a fatal
test_return $?                # error if VGID cannot be obtained.

SPVID=`getlvodm -p $SOURCEPV`  # get the source pvid, passing source pvname.
test_return $?                 # Treat as a fatal error if cannot obtain.

check_spv		       # check if source pv is active

if [ -n "$LFLAG" ]             # if -l was specified on the command line
then                           # get the lvid of the given lvname
   LVID=`getlvodm -l $LVAL`
   test_return $?
fi

#
# Lock the volume group.  ( All pv's, source and destination, must belong to
# the same volume group. )
#

putlvodm -k $VGID             # Locks the vg
test_return $?
LOCKEDVG=1

if [ -z "$DESTPVS" ]   # If destinationpv's were not specified on the command
then                   # line, create list of default destination pv's (id's).

   create_dpvlist
   if test -z "$DESTPVS"
   then
	dspmsg -s 1 cmdlvm.cat 814 "`lvmmsg 814`\n" migratepv >& 2
	test_return 1
   fi

else                    # destinationpv name(s) were specified, call routine
                        # to convert this list of names to a list of id's and
                        # to check for any problems with these pv's - are they
   check_dpvlist        # active and set to allocate? Do they all belong to
                        # same vg (they should) ? Is source pv in dpvlist
fi                      # (it shouldn't be) ?

# Call routine to count the number of partitions in the source physical volume
# to be migrated.  If the logical volume was not specified, this will include
# all partitions that are allocated.  Otherwise it will include all partitions
# that are allocated specifically to the specified logical volume. This number
# will be stored in $SPTN_COUNT.

count_spvptns

# calculate the number of unallocated physical partitions in the new
# destination physical volume map, storing it in $DPTN_COUNT.
count_free


# Check to make sure there are enough free partitions in the destination pvs
# to continue with this migrate.

if [ $DPTN_COUNT -lt $SPTN_COUNT ]   #If the number of free partitions is less
then                                 #than the # of partitions to be moved
                                     # issue error message and terminate.
   dspmsg -s 1 cmdlvm.cat 810 "`lvmmsg 810`\n" migratepv >& 2
   cleanup
fi

if [ -n "$LFLAG" ]    # If lvname option was specified on cmd line,
then                  # assign the corresponding lvid to $SLVLIST.
   SLVLIST=$LVID
else


   # The following routine will get a sorted list of lvids that are on the
   # source physical volume, storing it in $SLVLIST.

   get_slvlist

   # look for lv's of type "boot" to keep people from accidentally
   # migrating their blv

   check_for_boot_lvs

fi

#
#	if the -l flag was given for a boot lv,
#	do special code here to ensure contiguous
#	boot logical volumes...
#
test -n "$LFLAG" && is_a_boot_lv $LVID
if [ $? = 0 ]
then
	# allocate contiguous pp's on the destination pv
	PVNAME=`make_contig_lv_map $LVID`
	test_return $?

	# Call routine to create input map (mig_map$$) for migrate.  This will
	# consist of migration tuples in the following format:
	#              spvid slpnum sppnum  dpvid dlpnum dppnum
	make_migmap

	# perform the migration
	cat /tmp/mig_map$$ | migrate_it

	dspmsg -s 1 cmdlvm.cat 1012 "`lvmmsg 1012`\n" migratepv `getlvodm -e $LVID` /dev/$PVNAME >& 2

	cleanup
fi

# The following loop through the list of lv's on the source physical volume
# will perform the migrate one logical volume at a time, making an updated
# destination pvmap after each migrate.

for LVID in $SLVLIST
do
   # set stripe flag if it a striped LV
   STRIPE_SIZE=`lquerylv -L $LVID -b`
   test_return $?

   if [ $STRIPE_SIZE != 0 ]
   then
	LVNAME=`getlvodm -a $LVID`
	dspmsg -s 1 cmdlvm.cat 818 "`lvmmsg 818`\n" migratepv $LVNAME >& 2
   	continue
   fi

   make_dpvmap

   # Call routine to construct the input map for allocp.  This map will
   # include the destination pvmap and the lvmap for this lvid (the
   # "LVMAP:" token will have been changed to "MODIFY:" for each entry
   # in this lvmap that resides on the source physical volume).

   make_allocpin

   # Call getlvodm to get this lv's characteristics.  They will be
   # returned in positional parameters: $1=type $2=intra-policy value
   # $3=inter-policy value $4=upperbound value $5=strict value
   # $6=copies value $7=reloc-value

   rc=`getlvodm -c $LVID`
   test_return $?

   if test `echo "$rc" | cut -d" " -f5` = "n"
   then
	SFLAG="-k"
   else
	SFLAG=
   fi

   # Call allocp to generate a new logical volume allocation for those
   # partitions marked with the MODIFY: token.  Use the lv characteristics
   # obtained above with the exception of type - which must be x for explicit.
   # (If getlvodm failed, the positional parameters will be null and the
   # defaults will be taken.)

   # Allocp is called in a special way to handle moving of logical
   # volumes with the migratepv cmd.  Specifically, this arose out
   # of the case where the lv and it's mirror reside on the same disk.

   if [ -n "$LFLAG" ]             # if -l was specified on the command line
   then
   	cat /tmp/allocpin$$ | allocp -k -c $MIRR_CNT -i $LVID -t x $SFLAG \
   	`echo $rc | awk '{print " -u " $4 " -e " $3 " -a " $2}'` > /tmp/alloc_out$$
   else	
   	cat /tmp/allocpin$$ | allocp -i $LVID -t x $SFLAG \
   	`echo $rc | awk '{print " -u " $4 " -e " $3 " -a " $2}'` > /tmp/alloc_out$$
   fi


   # Call routine to create input map (mig_map$$) for migrate.  This will
   # consist of migration tuples in the following format:
   #              spvid slpnum sppnum  dpvid dlpnum dppnum

   make_migmap

   # Call migrate to perform the migration.  If a failure occurs, set
   # the failure flag so that a warning can be issued, but keep on
   # going - as we have no recovery action at this point.
   # Call migrate to perform the migration.  If a failure occurs, set
   # the failure flag so that a warning can be issued, but keep on
   # going - as we have no recovery action at this point.

	check_for_dump_device $LVID
	case $? in

	1)	sysdumpdev -p /dev/sysdumpnull > /dev/null 2>&1
		cat /tmp/mig_map$$ | migrate_it
		sysdumpdev -p /dev/$LVNAME > /dev/null 2>&1
		;;

	2)	sysdumpdev -s /dev/sysdumpnull > /dev/null 2>&1
		cat /tmp/mig_map$$ | migrate_it
		sysdumpdev -s /dev/$LVNAME > /dev/null 2>&1
		;;

	*)	cat /tmp/mig_map$$ | migrate_it
		;;

	esac

done   # for lvid in slvlist

if [ "$EXIT_CODE" = 1 ]
then
   EXIT_CODE=0
fi
cleanup
