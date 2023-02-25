#! /bin/bsh
# @(#)03        1.13.3.1  src/bos/usr/sbin/lvm/highcmd/reorgvg.sh, cmdlvm, bos41J, 9510A_all 3/7/95 22:28:59
#
# COMPONENT_NAME: (cmdlvm) Logical Volume Commands
#
# FUNCTIONS: reorgvg.sh
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
      dspmsg -s 1 cmdlvm.cat 968 "`lvmmsg 968`\n" reorgvg >& 2
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

   if test $VGLOCKED -eq 1
   then
   	putlvodm -K $VGID   # Unlock the volume group
	VGLOCKED=0
   fi

   rm -f /tmp/pvmap$$ /tmp/stalepvmaps$$ /tmp/modifymap$$ /tmp/domainmap$$ \
         /tmp/lvmap$$    /tmp/migmap$$  /tmp/freemap$$ /tmp/migrate$$  \
         /tmp/lvmoves$$

   exit $EXIT_CODE
}


########################### get_stales ##################################
#
#   Usage: get_stales vgid lvidlist
#

get_stales()
{
this_vgid=$1

# get all the pvids for the volume group 
rc=`getlvodm -w $1`
test_return $?
rc=`echo "$rc" | awk '{print $1}'`

# generate the pvmaps for all the pv's in rc
gen_pv_maps $1 $rc > /tmp/pvmap$$

# print the stale pvmap entries for the lvids in the lvid list 

for i in $2
do
	awk "/${i}/{ if (\$3 !=\"0\" && \$3 !=\"1\") print}" /tmp/pvmap$$ >> /tmp/stalepvmap$$
done

set `wc /tmp/stalepvmap$$`
echo $1

}

########################### get_lvids ##################################
#
#   Usage: get_lvids vgid [lvname1 lvname2 ...]
#
#   if no lvnames were specified 
#	print to stdout all the lvids that exist for
#   	the vgid specifed that have a relocatable flag of true
#   if any lvnames were specified 
#	for every lvname specified
#	    if relocateable flag is true
#		print to stdout the lvid for this lvname
#	    else
#		print error and exit 1
#
#   NOTE: if there was an error obtaining an lvid or relocatable flag 
#   then get_lvids will exit 1
#
#   Globals
#	VGNAME - volume group name
#
get_lvids()
{
	VGID=$1
	shift
	COUNT=0
	# were no lvnames specified on the command line?
	if test $# -eq 0
	then
		# get all the lvids for the volume group 
		rc=`lqueryvg -g ${VGID} -L`
		test_return $?
		set $rc
		rc=$1
	else
		# get the lvids for all the lvnames entered
		rc=""
		LV_NAMES=$*
		for i in $LV_NAMES
		do
			# get the lvid for the current lvname
			LV_ID=`getlvodm -l ${i}`
			test_return $?
			# make sure the lvname belongs to the volume group
			test "`getlvodm -b ${LV_ID}`" = "$VGNAME"
			test_return $?
			rc="$rc $LV_ID"
		done
	fi

	# print all lvids that can be relocated
	for i in $rc 
	do
		# get the the static information on the logical volume
		real_lvname=`getlvodm -a $i`
		test_return $?

		rc=`getlvodm -c ${i}`
		test_return $?
		# is the logical volume relocatable
		if [ `echo "${rc}" | awk '{print $7}'` = "y" ]
		then
			COUNT=`expr $COUNT + 1`
			echo "$i"
		# was the logical volume expressly entered
		else
			# we must exit since the logical volume entered
			# can not be relocated
			dspmsg -s 1 cmdlvm.cat 972 "`lvmmsg 972`\n" reorgvg $real_lvname >& 2
			cleanup
		fi
	done

	# did we have a logical volume to reorganize
	if test $COUNT -ne 0
	then
		return 0
	else
		cleanup
	fi
}

########################### get_pvids ##################################
#
#  Usage: get_pvids vgid [-i]
#  get_pvids will print to stdout all the pvs that will be reorganized.
#
#  if the -i option is specified
#     for every pvname read get the pvid
#  else
#     get all the pvids for the volume group specified
#
#  if the -i option was specified and a pv is missing, free, or removed
#	exit 1
#  if the -i option was not specified and a pv is missing, free, or removed
#	give a warning and continue
#  print to stdout every pvid that has a status that is not one of
#      missing, free, removed.
#
#  if we did not print any pvids out then exit 1
#
get_pvids()
{
	# save or VGID
	VGID=$1
	COUNT=0
	if test $# -eq 2
	then
		# get all the pvnames from stdin
		rc=
		while read PVNAME
		do
			# turn the pvname into a pvid
			rc="$rc `getlvodm -p $PVNAME`"
			test_return $?
		done
		err_state="bomb"
	else
		# get all the pvids for the volume group
		rc=`getlvodm -w $VGID`
		test_return $?
		rc=`echo "$rc" | awk '{print $1}'`
		err_state="warn"
	fi

	# print to stdout all the pvids that are not 
	#    notallocated, missing, or removed
	#         missing = 2
	#	  notallocated = 8
	#	  removed = 4
	#
	for i in $rc
	do
		rc=`lquerypv -g $VGID -p $i -c`
		test_return $?
		test_state $rc 2 && test_state $rc 3 && test_state $rc 4 
		if test $? -ne 0
		then
			real_pvname=`getlvodm -g $i`
			test_return
			if [ "$err_state" = "bomb" ]
			then
				dspmsg -s 1 cmdlvm.cat 982 "`lvmmsg 982`\n" reorgvg $real_pvname >& 2
				cleanup
			else
				dspmsg -s 1 cmdlvm.cat 982 "`lvmmsg 982`\n" reorgvg $real_pvname >& 2
			fi
		else
			COUNT=`expr $COUNT + 1`
			echo $i
		fi
	done

	# we must have at least one pvid
	if test $COUNT -eq 0
	then
		dspmsg -s 1 cmdlvm.cat 984 "`lvmmsg 984`\n" reorgvg >& 2
		cleanup
	fi
	return 0

}

########################### get_pv_maps ##################################
#
#   Usage: gen_pv_maps vgid pvid1 pvid2 ....
#
#   for every pvid specified print its pvmap to stdout
#
gen_pv_maps()
{
	VGID=$1
	shift
	for i in $@
	do
		lquerypv -g $VGID -p ${i} -dt
		test_return $?
	done
	return 0
}

########################### get_sourc_map ##################################
#
#   Usage: gen_sourc_map lvid pvmap
#
#   print to stdout every line in pvmap containing lvid changing PVMAP to MODIFY
#
gen_source_map()
{
	awk "/${1} /{(\$1=\"MODIFY:\");print}" $2
	return 0
}

########################### gen_allocp_domain ##################################
#
#    Usage: gen_allocp_domain pvmap lvid1 lvid2 lvid3 ...
#
#    1. for every lvid specified, change the its pp status to free and
#       print its pvmap entries to stdout 
#    2. print every free pp to stdout
#
gen_allocp_domain()
{
	MAP=$1
	shift
	for i in $*
	do
		awk "/${i} /{(\$3=\"0\");print}" $MAP
	done
	awk '{if ($3 == "0") print}' $MAP
}

########################### get_free_pps ##################################
#
#   Usage: get_free_pps pvmap
#
#   print in sorted order the ppid's of all free pps in pvmap
#
get_free_pps()
{
	awk '{if ($3 == "0") print $2}' $1 | sort -t: +0 -1 
}

########################### paste_maps ##################################
#  Usage: paste_maps modifymap allocpmap
#
#  1. sort modifymap on logical partition number, print the ppids to
#     tmpfile1 
#  2. sort allocpmap on logical partition number, print the ppids to
#     tmpfile2
#  3. join (not natural) line n from tmpfile1 with line n from tmpfile2
#     creating a source, destination map
#  4. remove tmpfile1 tmpfile2
#
paste_maps()
{
	sort -t" " +5 -6 $1 | awk '{printf "%s\n",$2}' > /tmp/sortmap1$$
        
        cat $2 | sed 's/  */ /g' > /tmp/tempfile$$

	sort -t" " +2 /tmp/tempfile$$ | awk '{printf "%s:%4.4d\n",$1,$2}' > /tmp/sortmap2$$

	paste -d: /tmp/sortmap1$$ /tmp/sortmap2$$

	rm -f /tmp/sortmap1$$ /tmp/sortmap2$$

}

########################### reorg_migrate ##################################
#
#   Usage: reorg_migrate vgid migfixmap
#
#   for every line in migfixmap construct a lmigratepp call.
#   if any lmigratepp call fails then this routine will exit.
#
reorg_migrate()
{
	VGID=$1
        awk -F: '{print "-p "$1" -n "$2" -P "$3" -N "$4}' $2 |
	(while migrate=`line`
	do
		lmigratepp -g $VGID $migrate
		test_return $?
	done) 
}

########################### test_state #######################################
#
# NAME: test_state()
#
# DESCRIPTION: Tests physical volume state bit number. Returns the value
#	       of the bit.
#
# INPUT: 
#        $1 : Physical volume state 
#	 $2 : Bit number to test. (0-4)
#
# RETURN VALUES:
#         0 : Bit is not set.
#	  1 : Bit is set.
#
# OUTPUT: 
#         None.
#
#
test_state()
{
	STATE=$1		#set the state 
	BITNUM=$2		#set the bitnum

	#shift left until the bit is reached
	while [ "$BITNUM" -gt 0 ]
	do
		STATE=`expr $STATE / 2`
		BITNUM=`expr $BITNUM - 1`
	done
	     
	rc=`expr $STATE % 2`	#set the return code based on the bit
	return $rc  		#return bit value
}
shiftlvids()
{
	shift
	echo $*
}

########################### main #######################################
#
#   Usage: reorgvg [-i] vgname [lvname1 lvname2 ...]
#
#   Reorgvg will reorganize the physical partitions within the volume group
#   vgname. The reoganization will try and conform to the allocation
#   criteria set for each logical volume.
#
#   1. parse the command line options
#   2. determine the lvs to be reorganized
#   3. determine the pvs to be reorganized
#   4. for every logical volume to be reorganized
#       a. get the pvmaps for all logical volumes not yet reorganized
#	b. generate the modify map for the current logical volume to
#	   be passed to allocp
#	c. generate the allocp domain map which consists of all the pps that
#          make up the logical volume to be moved and all free pps
#	d. generate the lvmap for the current logical volume
#	e. generate the new lv location map (allocp)
#	f. determine the proper order of pp moves
#	g. move the pp in the current lv to there new location
#   
#   Notes:
#       If lvnames are specified then only those lvs will be reorganized.
#       If no lvnames are specified then all lvs in the volume group with
#       relocate flag set to yes will be reorganized.
#
#       If the -i option is specified then the pvs to be reorganized will
#       be read from stdin.
#


VGLOCKED=0
EXIT_CODE=1
trap 'cleanup' 0 1 2 15 

# get our option ?
set -- `getopt i $*`
if [ $? != 0 ]
then
	dspmsg -s 1 cmdlvm.cat 960 "`lvmmsg 960`\n" reorgvg >& 2
	cleanup
fi

IFLAG=

while [ $1 != -- ]
do
	case $1 in
		-i) IFLAG=$1; shift;;
	esac
done

shift    # -- from getopt

# we must have some command line input
if test $# -eq 0
then
	dspmsg -s 1 cmdlvm.cat 606 "`lvmmsg 606`\n" reorgvg >& 2
	dspmsg -s 1 cmdlvm.cat 960 "`lvmmsg 960`\n" reorgvg >& 2
	cleanup 
fi

VGNAME=$1
export VGNAME
shift
LVNAMES="$*"

# get vgid
VGID=`getlvodm -v $VGNAME`
test_return $?

putlvodm -k $VGID
test_return $?
VGLOCKED=1

# get the lvids to be reorganized
LVIDLIST=`get_lvids $VGID $LVNAMES`
test_return $?

STALE_COPIES=`get_stales $VGID $LVIDLIST`
if [ "$STALE_COPIES" -ne 0 ]
then
	 dspmsg -s 1 cmdlvm.cat 633 "`lvmmsg 633`\n" reorgvg  reorganizing >& 2
         cleanup
fi

# get the pvids to be reorganized
PVIDLIST=`get_pvids $VGID $IFLAG`
test_return $?

for i in $LVIDLIST
do
	real_lvname=`getlvodm -a $i`
	test $?

	# get the current allocation map of the volume group
	(gen_pv_maps $VGID $PVIDLIST) > /tmp/pvmap$$

	# get the modify map for allocp usage
	(gen_source_map $i /tmp/pvmap$$) > /tmp/modifymap$$

	# determine the domain of the reorganization
	(gen_allocp_domain /tmp/pvmap$$ $LVIDLIST) > /tmp/domainmap$$

	# get the logical volume map for the current lv
	lquerylv -L $i -ldt > /tmp/lvmap$$
	test_return $?

	# get the lv allocation characteristics
	rc=`getlvodm -c $i`
	test_return $?

	# determine the new lv pp allocation
	cat /tmp/domainmap$$ /tmp/lvmap$$ /tmp/modifymap$$ | \
           allocp -i $i `echo $rc | \
                 awk '{print " -c " $6 " -u " $4 " -e " $3 " -a " $2 " -t x "}'` > /tmp/migmap$$
	if [ $? != 0 ]
	then
		dspmsg -s 1 cmdlvm.cat 990 "`lvmmsg 990`\n" reorgvg $real_lvname >& 2
		cleanup
	fi

	# determine the order of pp moves
	(get_free_pps /tmp/pvmap$$) > /tmp/freemap$$
	(paste_maps /tmp/modifymap$$ /tmp/migmap$$) > /tmp/migrate$$
	(migfix /tmp/freemap$$ /tmp/migrate$$ /tmp/lvm_moves$$)
	case $? in
		0) (reorg_migrate $VGID) < /tmp/lvm_moves$$ &&
			dspmsg -s 1 cmdlvm.cat 962 "`lvmmsg 962`\n" reorgvg $real_lvname >& 2 ||
			dspmsg -s 1 cmdlvm.cat 964 "`lvmmsg 964`\n" reorgvg $real_lvname >& 2;; 
		5) dspmsg -s 1 cmdlvm.cat 962 "`lvmmsg 962`\n" reorgvg $real_lvname >& 2 ;;
		*) dspmsg -s 1 cmdlvm.cat 966 "`lvmmsg 966`\n" reorgvg
			cleanup;;
	esac

	# move the pps to there new locations
	LVIDLIST=`(shiftlvids $LVIDLIST)`
done

if [ "$EXIT_CODE" = 1 ]
then
   EXIT_CODE=0
fi
cleanup
~

