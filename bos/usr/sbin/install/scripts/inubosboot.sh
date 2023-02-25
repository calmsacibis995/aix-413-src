#!/usr/bin/ksh
# @(#)09        1.4  src/bos/usr/sbin/install/scripts/inubosboot.sh, cmdinstl, bos411, 9428A410j 6/23/94 19:06:34
# COMPONENT_NAME: cmdinstl
#
# FUNCTIONS: 	newbootimage
#		buildboot
#		verifyboot
#                                                                    
# ORIGINS: 27 
#
# (C) COPYRIGHT International Business Machines Corp. 1993
# All Rights Reserved
# Licensed Material - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#                                                                    

PROTODIR=/usr/lib/boot
DISKPROTO=$PROTODIR/disk.proto
REBOOT_MSG=92          	# inuumsg message that tells the user to reboot
INU_FATALI=102          # inuumsg message that tells the user about fatal errors
NEEDNEWBOOT=0
NONEWBOOT=1
ERROR=2

#
#  Determine if a new boot image needs to be created
#  This function takes the name of the proto file as $1
#  the possible proto files are:
#
# 	$DISKPROTO	used for stand-a-lone machines
#
#  This function return code will be one of these:
#
# 	$NEEDNEWBOOT  	a bosboot needs to be performed with the
# 			protofile that was specified
# 	$NONEWBOOT    	a bosboot does not need to be performed based on the
#			protofile that was specified
# 	$ERROR        	an error was encounter while trying to determine
#			if a bosboot was needed.
#
#  NOTE:
#
#  ASSUMPTIONS:
#	$INUCLIENTS will be zero length if instclient is not
#	one of our ancestors.
#	If $INUCLIENTS is non-zero length then chroot is also
#	(as well as instclient) an ancestor of this process.
#


newbootimage()
{
	[[ -z "$1" ]] && return $ERROR

	PROTOFILE=$1
	[[ $PROTOFILE != $DISKPROTO ]] && return $ERROR


	[[ -n "$BOSINSTENV" ]] && return  $NONEWBOOT

        # We don't have to support remote /usr clients
        # in 4.1

        # If in non-/usr SPOT environment 
        # bosboot is not needed
        #-------------------------------------------
        [[ -n "$INUCLIENTS" ]] && return  $NONEWBOOT

        return  $NEEDNEWBOOT
}

#  This routine builds the boot image
#  This function takes the name of the proto file as $1
#  the possible proto files are:
#
# 	$DISKPROTO
#
#  DO NOT REMOVE THIS NOTE WITH OUT TALKING TO THE OWNER OF BOSINST 
#  NOTE:  
#	The environment variable BOSINSTENV will be set by BOSINST
#  	if BOSINST is calling installp.  This is being added because
#  	BOSINST will call installp to apply all mandatory ptfs and some
#  	of these ptfs could cause bosboot to be run and since bosboot
#  	has not been run before the /dev/ipldevice is not set up so
#  	bosboot will fail (bosinst knows how to call bosboot so that 
#  	/dev/ipldevice is set up) and since bosinst will run bosboot
#  	after applying all of the mandatory ptfs why call bosboot
#  	in this situation. So if BOSINSTENV is set do not call bosboot.
#

buildboot()
{
	if [[ -n "$1" ]]; then
		PROTOFILE=$1
	else
		return $ERROR
	fi

	[[ $PROTOFILE != $DISKPROTO ]] && return $ERROR

        # if BOSINSTENV is set then just return 0 (see NOTE above)
	# DO NOT REMOVE THIS CODE WITH OUT TALKING TO THE OWNER
	# OF BOSINST 

	[[ -n "$BOSINSTENV" ]] && return 0

	ODMDIR=/etc/objrepos bosboot -a -d /dev/ipldevice
	return $?
}

#
# This function verifies via a quick check with bosboot that any subsequent
# bosboot will pass.  This is being used as a preliminary check by 
# installp before the PTF is applied.
# 
#  This function takes the name of the proto file as $1
#  the possible proto files are:
#
# 	$DISKPROTO
#

verifyboot()
{
	if [[ -n "$1" ]]; then
		PROTOFILE=$1
	else
		return $ERROR
	fi

	[[ $PROTOFILE != $DISKPROTO ]] && return $ERROR

        # if BOSINSTENV is set then just return 0 (see NOTE above)
	# DO NOT REMOVE THIS CODE WITH OUT TALKING TO THE OWNER
	# OF BOSINST 
	[[ -n "$BOSINSTENV" ]] &&  return 0

	ODMDIR=/etc/objrepos bosboot -v -d /dev/ipldevice
	return $?
}

#
# Main program
#

PATH=/usr/sbin:/usr/bin:/bin:/etc:$PATH
NIM_FILE="/etc/niminfo"

#
#  Options -v will invoke the bosboot verify capability which will ensure
#  than any subsequent bosboots will perform correctly.  If the verify
#  fails do not attempt the real bosboot.  Calling this script with no
#  options will perform normal bosboot processing.

case $1 in
  -v)	
       newbootimage $DISKPROTO 
       RC=$?
       if [[ $RC -eq $ERROR ]]; then
	   inuumsg $INU_FATALI
	   exit 1
       elif [[ $RC -eq $NEEDNEWBOOT ]]; then
            # Invoke bosboot verify to ensure capable
	    verifyboot $DISKPROTO 
	    RC=$?
            if [[ $RC -ne 0 ]]; then
               inuumsg $INU_FATALI
               exit 1
	    fi
	fi
        ;;

  *)	
       	newbootimage $DISKPROTO 
       	RC=$?
       	if [[ $RC -eq $ERROR ]]; then
           inuumsg $INU_FATALI
           exit 1
       	elif [[ $RC -eq $NEEDNEWBOOT ]]; then
             # Invoke bosboot to create a new boot.image
             buildboot $DISKPROTO 
             RC=$?
             if [[ $RC -ne 0 ]]; then
            	inuumsg $INU_FATALI
               	exit 1
             fi
             inuumsg $REBOOT_MSG
        fi
        ;;
esac

exit 0
