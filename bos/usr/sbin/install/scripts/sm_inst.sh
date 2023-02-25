#!/bin/ksh
# @(#)12        1.60  src/bos/usr/sbin/install/scripts/sm_inst.sh, cmdinstl, bos41J, 9521B_all 5/25/95 16:19:30
#
#   COMPONENT_NAME: cmdinstl
#
#   FUNCTIONS: 	installp_format()
#		installp_format_signal_handler()
#		installp_format_cleanup()
#		installp_cmd()
#		installp_query()
#		installp_cmd_signal_handler()
#		bffcreate_format()
#		bffcreate_format_signal_handler()
#		bffcreate_cmd()
#		bffcreate_cmd_signal_handler()
#		list_devices()
#		list_device()
#		list_cdrom()
#		list_filesystem_images()
#		check_device()
#		list_bundles()
#		make_bundle()
#		make_filter()
#		get_cd_mount_point()
#		unmount_cd()
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#=============================================================================
# NOTE - This program is not intended to be used by users directly.
#	 It is used by the SMIT install screens for various activities that
#        are performed within the SMIT install environment.
#=============================================================================

#=============================================================================
# Function:  is_level_greater
#
# Parameters:  2 V.R.M.F. Levels
#
# Returns:  0 if the first level is greater than the second level
#           1 if they are equal
#           2 if the second is greater than the first
#
#=============================================================================
function is_level_greater {
   level1=$1
   level2=$2

   [[ $level1 = $level2 ]] && return 1

   greater=`echo "${level1}\n${level2}" | 
            sort -t. -k1,1n -k2,2n -k3,3n -k4,4n |
            tail -1`

   if [[ $level1 = $greater ]]; then
      return 0
   else
      return 2
   fi
}

#=============================================================================
# installp_format
#
# This is the code for formatting the output from the installp -L command
# for presentation in SMIT.  The code outputs the formatted output to stdout
# and if the list flag (-l) is not specified, it also creates a file that
# allows mapping the "nice" listing to the items that need to go on the
# installp command line.
#
# Once the user selects the packages they want to install from the "nice"
# software listing this command returns, and executes the command from the
# dialog, the command they execute uses the database this command creates
# to map the nice names the user selected back into the name that goes onto
# the installp command line.
#
# Flags:
#   -b bundle		# bundle parsing required for -b BUNDLE
#   -R filter		# filter parsing required for -R FILTER
#   -d device		# device for installp -l -d DEVICE
#   -e			# list enhancements
#   -i			# list install packages
#   -l			# Does a latest listing
#   -m			# list maintenance levels
#   -n			# do not get netls information
#   -o file		# Output file
#   -O			# Only run to generate list (F4 pop-up list)
#   -P file		# Input file with pre-run installp -L output
#   -s			# list filesets
#
# Bundle = predefined set of options tied together for ease of installation
#	   default selection can be overridden from custom install.
# Filter = undefined set of options which are tied together for ease of
#          installation and maintenance.  Note: Filters may override -iems
#          flags on this command.  The filter is only specified in the
#          "SOFTWARE to install" field of Custom Install panels.  Thus it
#          will only ever be used by installp_cmd below
#
# Bundle and filter can be used simultaneously on the same medium
# eg. BUNDLE=Server ; FILTER=ALL_LICENSED
#=============================================================================
installp_format()
{
   # Set a trap so if this process is interupted, temporary files are
   # removed and the cd rom drive is unmounted if necesary

   trap 'installp_format_signal_handler' INT QUIT TERM

   # Easy debug code
   [ -n "$SM_INST_DEBUG" ] && set -x

   # This command creates temporary files for a short period of time
   # using the following location

   TMPFORMAT=/tmp/.installp_format.$$
   GREPFORMAT=/tmp/.installp_format2.$$
   TEMPFILE=/tmp/.filesets.$$
   TEMP2FILE=/tmp/.filesets2.$$
   NETLS_FILE=/tmp/.netls.$$
   STATUS_FILE=/tmp/.status.$$
   TMPBUNDLEFILE=/tmp/.bundle.$$
   TMPFILTERFILE=/tmp/.filter.$$
   TMPAUTOMSG=/tmp/.install_auto_msg

   # These variables indicate whether the particular type of package
   # indicated is listed if it is found on the install media.
   # The following represent the different types of install packages.
   # These variables are modified by command line flags to this cmd.

   INST=false                 # install packages
   ENH=false                  # enhancements
   ML=false                   # maintenance levels
   SS=false                   # filesets
   LATEST=false               # latest level listing only
   LIST_ONLY=false            # do not print headers by default
   NO_DELIMITER=false         # do not print delimiter followed by fileset
   UNMOUNT=FALSE              # flag indicating if CD-ROM fs needs unmounted
   DEVICE=                    # device specification for installp -ld DEVICE
   OUTFILE=                   # if output file is necessary
   VALUE=                     # return string from get_cd_mount_point
   RC=                        # used to hold return code
   DO_NETLS=true           
   BUNDLE=                    # bundle specification if required
   FILTER=                    # option filter specification if required
   SYS_BUNDLE_DIR=            # system bundles directory
   USER_BUNDLE_DIR=           # user defined bundles directory
   PREV_RUN=                  # giving installp -L output in a file
   PREV_RUN_FILE=             # file containing installp -L output 
   MESSAGES_ONLY=false        # flag to just do an installp -L then return
	
   # command line parsing
   while getopts b:R:d:ielMmo:OP:nNs FLAG; do
      case $FLAG in
         b)  BUNDLE=$OPTARG;;         # bundle name
         R)  FILTER=$OPTARG;;         # filter name
         d)  DEVICE=$OPTARG;;         # device
         i)  INST=true;;              # install package
         e)  ENH=true;;               # enhancement
         l)  LATEST=true;;            # latest level
         o)  OUTFILE=$OPTARG;;        # output file
         O)  LIST_ONLY=true;;         # headings required
         m)  ML=true;;                # maintenance level
         M)  MESSAGES_ONLY=true;;     # just create the $TMPAUTOMSG file
         N)  NO_DELIMITER=true;;      # for view bundle
         n)  DO_NETLS=false;;         # set to false if installp_format is 
                                      # invoked from installp_cmd.
         P)  PREV_RUN=true            # use previous output
             PREV_RUN_FILE=$OPTARG;;  # filename of previous run output
         s)  SS=true;;                # fileset
      esac
   done

   # Ensure device specification is valid
   if [ ! "$PREV_RUN" ]; then
      check_device $DEVICE 
   fi

   # If -l (latest) listing required, override -iems flags
   # since they are ignored.  This improves performance.
   if [ $LATEST = "true" ]; then
      INST=true ; ENH=false ; ML=false ; SS=false
   fi

   # If the user specified a bundle (with the -b flag),
   # check existence of the bundle up front since we don't
   # want to waste time doing anything else if it has been
   # wrongly specified.  Check that file exists and is non-zero
   # size. sys_bundles take precedence over user_bundles with same
   # name.

   if [ "$BUNDLE" ]; then
      SYS_BUNDLE_DIR=/usr/sys/inst.data/sys_bundles
      USER_BUNDLE_DIR=/usr/sys/inst.data/user_bundles

      if [ -s $SYS_BUNDLE_DIR/$BUNDLE.bnd ]; then 
         make_bundle $BUNDLE $SYS_BUNDLE_DIR/$BUNDLE.bnd $TMPBUNDLEFILE \
            || return 1
      elif [ -s $USER_BUNDLE_DIR/$BUNDLE.bnd ]; then
         make_bundle $BUNDLE $USER_BUNDLE_DIR/$BUNDLE.bnd $TMPBUNDLEFILE \
            || return 1
      else
         # Not a valid bundle
         inuumsg 150 $BUNDLE
         return 1
      fi
   fi

   # Prepare any filter file based on specification
   if [ "$FILTER" ]; then
      make_filter $FILTER $TMPFILTERFILE || return 1
   fi

   # If the user specified a CD-ROM device (w/the -d flag):
   # If it is *not* already mounted:
   #    mount it
   #    set a flag indicating that it needs to be unmounted later
   # In either case (needs to be mounted or not):
   #    get its mount point
   if [[ $DEVICE = /dev/cd* ]]; then
      VALUE=$(get_cd_mount_point $DEVICE)
      if [ $? != 0 ]; then
         inuumsg 155
         return 1
      fi
      DEVICE=$(print $VALUE | cut -f1 -d' ')
      UNMOUNT=$(print $VALUE | cut -f2 -d' ')
   fi

   # The installp -L command is used to get a listing of everything on
   # the install media, and that output is what is formatted and filtered
   # based on the flags passed into this cmd. The program 
   # get_license_info is used to get the netls information which is 
   # compared with the output of installp -L.

   if [ "$DO_NETLS" = "true" ]; then
      get_license_info -f$STATUS_FILE >$NETLS_FILE 2>/dev/null &
      NETLS_PID=$!
   fi

   if [ ! "$PREV_RUN" ]; then
      installp -qLd$DEVICE >$TMPFORMAT
      RC=$?
      if [ $RC != 0 ]; then 
         installp_format_cleanup
         # return 3 as a generic installp -L failure
         RC=3
         return $RC
      fi

      # Are we doing licencse stuff
      if [ "$DO_NETLS" = "true" ]; then
         # Does anything have license stuff?
         cut -f 13,14,15 -d : $TMPFORMAT | fgrep -v :: | \
            fgrep -qs : > /dev/null 2>&1
         # well?
         if [ $? != 0 ]; then
            # Nope, no licence stuff
            unset NETLS_PID
            DO_NETLS=false
         fi
      fi

      # Save off message, locale and man page options now 
      grep -F '.msg.' $TMPFORMAT >$GREPFORMAT
      grep -F '.loc.' $TMPFORMAT >>$GREPFORMAT

      if [ -s "$GREPFORMAT" ]; then
         sort -A -u -o $TMPAUTOMSG $GREPFORMAT
         rm -f $GREPFORMAT
      fi
   else
      # Copy the previous run output into TMPFORMAT
      # The PREV_RUN_FILE will not be removed at the end
      cp $PREV_RUN_FILE $TMPFORMAT
   fi

   # If the only thing we wanted wanted was the message file, exit now
   if [ "$MESSAGES_ONLY" = "true" ]; then
      return 0
   fi

   # If we have a bundle, save off all options which are matched.
   # Do not worry if some options are unmatched as these are handled
   # by the awk component
   if [ "$BUNDLE" ]; then
      for bundle_entry in $(sed -e 's/\./\\\./g' <$TMPBUNDLEFILE); do
         grep ":$bundle_entry[\.:]" $TMPFORMAT >>$GREPFORMAT 2>/dev/null
      done
      sort -A -u -o $TMPFORMAT $GREPFORMAT
      rm -f $GREPFORMAT
   fi

   # If we mounted a CD-ROM device real-time, we are now done with it
   [ "$UNMOUNT" = "TRUE" ] && unmount_cd $DEVICE
   UNMOUNT=FALSE

   # Only do this if the outfile is to be created:
   # If the database file that we are going to create real-time already
   # exists, attempt to remove it.
   if [ "$OUTFILE" -a -f "$INSTALLP_DB_FILE" ]; then
      rm -f $INSTALLP_DB_FILE
      RC=$?
      if [ $RC != 0 ]; then
         installp_format_cleanup
         return $RC
      fi
   fi

   if [ "$FILTER" = "_update_all" ]; then

      # Create a list of what's currently installed (at the latest levels)
      lslpp -qLc | cut -d: -f2,3 | sed 's/:/ /' > $GREPFORMAT

      # Create a list of "name level" pairs of what's in the .toc
      cat $TMPFORMAT | cut -d: -f2,3 | sed 's/:/ /' > $TEMPFILE

      num_installed_filesets=`cat $GREPFORMAT | wc -l`
      num_toc_filesets=`cat $TEMPFILE | wc -l`

      # If the number of filesets in the .toc is less than the number of 
      # filesets installed on the system then we'll want to loop through
      # the filesets in the .toc, otherwise we'll want to loop though the 
      # list of installed filesets.  Either way we'll be creating a list
      # of updates (from the .toc) to filesets which are installed at a
      # lower V.R.M.F level on the system.

      if [ $num_toc_filesets -lt $num_installed_filesets ]; then

         while read toc_fileset toc_level; do

            # Do a quick check to see if $toc_fileset is in the list of 
            # installed filesets - if not then continue (big time saver)

            grep "$toc_fileset " $GREPFORMAT >/dev/null 2>&1 || continue

            installed_level=`grep "$toc_fileset " $GREPFORMAT | cut -f2 -d' '`

            if is_level_greater $toc_level $installed_level; then
               grep ":${toc_fileset}:${toc_level}:" $TMPFORMAT >> $TEMP2FILE
            fi
         done < $TEMPFILE

      else

         while read installed_fileset installed_level; do

            # Do a quick check to see if $installed_fileset is in the .toc 
            # if not then continue (big time saver)

            grep "$installed_fileset " $TEMPFILE >/dev/null 2>&1 || continue

            # There could be multiple levels of $installed_fileset in .toc
            for toc_level in $( grep "$installed_fileset" $TEMPFILE | 
                                cut -f2 -d' ' )
            do
               if is_level_greater $toc_level $installed_level; then
                  grep ":${installed_fileset}:${toc_level}:" $TMPFORMAT \
                     >> $TEMP2FILE
               fi
            done

         done < $GREPFORMAT 

      fi

      if [ -s $TEMP2FILE ]; then
         sort -u $TEMP2FILE -o $TMPFORMAT
      else
         # if TEMP2FILE is empty that means there were no updates available
         # on the media at a level greater than those installed 

         installp_format_cleanup
         RC=2
         return $RC
      fi

      rm -f $TEMPFILE $TEMP2FILE
   fi

   # cat the listing, removing whitespace lines and headers, and pipe
   # it into awk so it can do the processing and print out the nicely
   # formatted version.
   #
   # The sed was added to remove single quotes from the installp -l
   # listing.  This is necessary to remove the ' character from the
   # description field because the shell removes the quote and causes
   # problems.

   cat $TMPFORMAT | sed "s/[',]//g" | \
   awk -v OUTFILE=$OUTFILE \
       -v NETLS_FILE=$NETLS_FILE \
       -v STATUS_FILE=$STATUS_FILE \
       -v BUNDLE=$BUNDLE \
       -v BUNDLE_FILE=$TMPBUNDLEFILE \
       -v FILTER=$FILTER \
       -v FILTER_FILE=$TMPFILTERFILE \
       -v LATEST=$LATEST \
       -v INST=$INST \
       -v ENH=$ENH \
       -v ML=$ML \
       -v SS=$SS \
       -v DO_NETLS=$DO_NETLS \
       -v NETLS_PID=$NETLS_PID \
       -v LIST_ONLY=$LIST_ONLY \
       -v NO_DELIMITER=$NO_DELIMITER \
       -f /usr/lib/instl/sm_inst.awk
   RC=$?

   # Get rid of the temporary files
   rm -f $TMPFORMAT $GREPFORMAT $NETLS_FILE $TMPBUNDLEFILE \
         $TMPFILTERFILE $TMPFILTERFILE $STATUS_FILE 
	

   [ $RC != 0 ] && return $RC

   return 0
}


#-----------------------------------------------------------------------------
# installp_format_signal_handler
#
# Signal handler for the installp_format program
#-----------------------------------------------------------------------------
installp_format_signal_handler()
{
   # Ignore signals in signal handler
   trap '' INT QUIT TERM

   # Clean up the executing environment
   installp_format_cleanup

   exit 1
}

#-----------------------------------------------------------------------------
# installp_format_cleanup
#
# Cleanup routine for the installp_format program
#-----------------------------------------------------------------------------
installp_format_cleanup()
{
   # Remove the temporary file if it exists
   rm -f $TMPFORMAT $GREPFORMAT $STATUS_FILE $NETLS_FILE $TMPBUNDLEFILE \
         $STATUS_FILE

        [ -f "$TMPFILTERFILE" ] && rm -f $TMPFILTERFILE

   # Unmount the cdrom drive it it needs to be unmounted
   [ "$UNMOUNT" = "TRUE" ] && unmount_cd $DEVICE
   UNMOUNT=FALSE
}


#=============================================================================
# installp_cmd
#
# This is the code for the installp_cmd script that is called from within
# SMIT.  The primary purpose is to rearrange the command line arguments and to
# take the selected name SMIT gives and map it into the images that need to
# go on the installp command line.
#=============================================================================
installp_cmd()
{
	# Easy debug code
	[ -n "$SM_INST_DEBUG" ] && set -x

	# getopts/getopt was not used here because I needed more control
	# over the arguments because the -S (software) flag had more than
	# one argument which getopts can not handle.  When -S is encountered,
	# everything up until the next flag is placed in the SC variable
	# by the default action from the case statement.  Also, set can not
	# be used here because it would put a -- after the first value from
	# the -S flag, thus none of the following flags following -S would
	# be parsed.
	#
	# There is no error checking since the SMIT input is "known", other
	# wise, this method would not work, but if the input was not coming
	# from SMIT, the multi-argument value for -S could easily have
	# quotes around it making it a single argument, that is just not
	# possible (given a clean method) to do from within SMIT since the
	# quotes immediately get stripped, prior to getting to this function.

	# Set a trap so that the cd rom is always unmounted if necessary
	trap 'installp_cmd_signal_handler' INT QUIT TERM

	TMPFILE=/tmp/.installp_cmd.$$
	INSTALLP_DB_FILE=/tmp/.installp_db.$$
	TMPAUTOMSG=/tmp/.install_auto_msg
	LASTAUTOMSG=/tmp/.install_auto_last

	# Initialize variables
	AC=
	A=
	AUTOMSG=
	BC=
	BUNDLE= 
	CC=
	C=
	DC=
	D=
	DEVICE=
	F=
        FILTER=
	G=
	I=
	IC=
	L=
	LATEST=false
        LOG=
	NC=
        P=
	R=
	S=
	T=
	TYPE=
	Q=
	V=
        VERB=
	XC=
	SC=
	VALUE=
	UNMOUNT=
	INSTALLP_CMD=
	INSTALLP_FORMAT_RUN=
	SW=
	RC=
	CQ="-Q"


	# Parse the command line
	while :; do
		case $1 in
		-A)	AC="A";;
		-a)	A="a";;
		-B)	BC="B";;
		-b)	BUNDLE="-b $2"	# bundle specified - get bundle name
			shift;;
		-C)	CC="C";;
		-c)	C="c";;
		-D)	DC="D";;
		-d)	D="d$2"
			DEVICE=$2
			shift;;
                -e)     LOG=" -e $2"
                        LOGFILE=$2
                        shift;;
                -E)	MSG=E;;
		-F)	F="F";;
		-g)	G="g";;
		-G)	AUTOMSG="true";;
		-i)	I="i";;
		-I)	IC="I";;
                -J)	INFO=J;;
		-l)	L="l";;
		-L)	LATEST="true";;
		-N)	NC="N";;
		-o)	SC="$SC "$(/usr/bin/tr "\012" "," <$2)
					# temp input file for S/W from SMIT
			shift;;
                -p)     P="p";;
		-r)	R="r";;
		-s)	S="s";;
		-S)	;;		# '*' will catch the arguments for -S
		-t*)	T="$1 ";;	# -tDirectory
		-T)	shift		# Types may be overridden by
			TYPE=$1;;	# specification of a software keyword.
		-Q)	CQ="";;		# Turn -Q off to report instreqs
		-q)	Q="q";;
		-v)	V="v";;
                -V2)    VERB=" -V2";;
		-X)	XC="X";;
		"")	break;;
		*)	SC="$SC $1";;	# must be a -S argument
		esac
		shift			# shift arguments to look at next one
	done

	# Ensure device specification is valid
	check_device $DEVICE 

	# If we have a CD-ROM device:  If it is not already mounted, mount
	# it, in either case, get its mount point.  If we mount it, we set
	# a flag so it can be unmounted at the end of our set of operations
	#
	# Note: The d in front of of the device is intentional. It is the flag
	# that is used for the installp command.
	if [[ $DEVICE = /dev/cd* ]]; then
		VALUE=$(get_cd_mount_point $DEVICE)
                if [ $? != 0 ]; then
                   inuumsg 155
                   return 1
                fi
		DEVICE=$(print $VALUE | cut -f1 -d' ')
		D=d$DEVICE
		UNMOUNT=$(print $VALUE | cut -f2 -d' ')
	fi

	SW=$(installp_query $TYPE $LATEST $D $SC)
        RC=$?

        # Return code 2 from installp_query means no matches found
	[ "$RC" = "2" ] && {
		if [ -n "$BUNDLE" ]
		then
			inuumsg 161
		else
			case "$SC" in
			   " ALL_LICENSED"|" all_licensed")
				inuumsg 139
			   ;;
			   " ALL_ENHANCEMENTS"|" all_enhancements"|\
			   " ALL_MAINTENANCE"|" all_maintenance"|\
			   " ALL_FILESET_UPDATES"|" all_fileset_updates"|\
			   " ALL_UPDATES"|" all_updates")
				inuumsg 162
			   ;;
			   " _UPDATE_ALL"|" _update_all")
				inuumsg 177
			   ;;
			   *)
				if [ $LATEST = "true" ]
				then
		 		    inuumsg 139
				else
				    for i in 1 2 3 4
				    do
					T=$(print $TYPE | cut -c$i)
					case $T in
					    i)  inuumsg 139 ;;
					    e)  : ;;
					    m)  inuumsg 141 ;;
					    s)  inuumsg 142 ;;
					    *)  break;;
					esac
				    done
				fi
			;;
		    esac
		fi
		[ "$UNMOUNT" = "TRUE" ] && unmount_cd $DEVICE
		UNMOUNT=FALSE
		return 0
        }

        # Return code 3 from installp_query means installp -L failed
	# eg. media error, bad toc.
	[ "$RC" = "3" ] && exit 3

	# Here we print out the installp command for the user in a nice
	# formatted method since it is typically going to be far greater
	# than 80 characters.  The installp command is on the first line
	# and every other line after that has a OPP/option on it followed
	# optionally by the version/ptf if it was provided.
	# The variable SW contains the single list that is formatted nicely,
	# the tr separates the SW list into multiple lines.

	print $SW | \
	/usr/ucb/tr -s " \t" "\012\012" | \
	awk '
	BEGIN {
		expecting_release = "false"		# release is the 2nd
							# field of the normal
							# combination
		count = 0
	}
	$0 ~ /^[ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz]/ {
		# If the next value was not a release (which is possible),then
		# add a newline after the OPP that does not have the release
		# specified with it, PRIOR to printing out the current OPP.
		if (expecting_release == "true") {
			printf("\n")
		}
		printf("    %s", $0)
		expecting_release = "true"	# now we expect the release
		next				# like 01.02.0000.0000.SSxyz
	}
	{
		printf("  %s\n", $0)
		expecting_release = "false"
		next
	}
	END {
		printf("\n")
	}
	' | grep -v "^[ \t]*$" | sort -A -u | sed -e "s/\.$//g"> $TMPFILE

	if [ "$AUTOMSG" = "true" ]; then
		# If the $TMPAUTOMSG file doesn't exist call automsg with -d
		# -d $DEVICE so it will call installp -L
		if [ "$INSTALLP_FORMAT_RUN" = "true" ]; then
			# Call automsg to get <lang> and format -L listing
			automsg $TMPAUTOMSG $LASTAUTOMSG
		else
			# Call automsg with device to get .msg.'s and .loc.'s
			automsg -d $DEVICE $LASTAUTOMSG
		fi

	fi  


	# Echo to the screen the installp command that will be executed
	# and cat the file containing the software that will be executed
	# to the screen
	print "installp $T-$Q$A$C$AC$CC$I$IC$F$R$BC$S$DC$G$NC$P$V$XC$VERB$LOG -d $DEVICE -f File 2>&1"
	print "\nFile:\n"
	cat $TMPFILE
	print

	if [ -s "$LASTAUTOMSG" ]
	then
		cat $LASTAUTOMSG >> $TMPFILE
	fi

        # -Q flag here tells installp to supress instreq failures
	# if -Q is not passed to installp_command from smit
        # Hard coded -J flag here tells installp we're called from SMIT
	installp $T-$Q$A$C$AC$CC$I$IC$L$R$BC$S$DC$G$F$NC$P$V$XC$D \
            $CQ -J $VERB $LOG -f $TMPFILE 2>&1 
	RC=$?

	rm -f $TMPFILE $LASTAUTOMSG $TMPAUTOMSG > /dev/null 2>&1

	[ "$UNMOUNT" = "TRUE" ] && unmount_cd $DEVICE
	UNMOUNT=FALSE

	return $RC
}

#-----------------------------------------------------------------------------
# installp_query
#
# The query routine takes as input the 'nice' software names that the user has
# selected from the cmd_to_list (installp_format) menu, and converts those
# 'nice' names into their counterparts that the installp command understands,
# and returns those names (and versions) back to the installp_command routine.
#-----------------------------------------------------------------------------
installp_query()
{
	# Easy debug code
	[ -n "$SM_INST_DEBUG" ] && set -x

	# Initialize variables
	LATEST_FLAG=
	USER_VALUE=
	FIELD=
	CHECK_NETLS=

	TYPE=$1
	LATEST=$2
	DEVICE=$3
	shift 3
	SOFTWARE=$*
	[ "$LATEST" = "true" ] && LATEST_FLAG="-l"

	# Handle cases where software to install is a keyword and/or bundle, 
	# rather than a user selected list.  Ensure list generated from 
	# keyword and/or bundle is not null.

	case "$SOFTWARE" in
		ALL|all)
			FILTER="-n" # No NetLS information for ALL
			CHECK_NETLS=false
		;;
		ALL_LICENSED|all_licensed)
			FILTER="-R $SOFTWARE" # Must have NetLS info.
			CHECK_NETLS=true
		;;
		_UPDATE_ALL|_update_all|\
		ALL_UPDATES|all_updates|\
		ALL_MAINTENANCE|all_maintenance|\
		ALL_ENHANCEMENTS|all_enhancements|\
		ALL_FILESET_UPDATES|all_fileset_updates)
			FILTER="-n -R $SOFTWARE" # No NetLS for updates
			CHECK_NETLS=false
		;;
		*)
			# cancel bundle if S/W selection made (custom install
			# of bundle could be by keyword or manual selection,
			# easy install of bundle always has null 
			# S/W selection)
			[ -n "$SOFTWARE" ] && BUNDLE=
			FILTER=
			CHECK_NETLS=
		;;
	esac

	if [ -n "$BUNDLE$FILTER" ] # have selective criteria been applied?
	then
		installp_format -$TYPE $LATEST_FLAG $FILTER $BUNDLE \
				-o$INSTALLP_DB_FILE -$DEVICE || return $RC
		
		INSTALLP_FORMAT_RUN=true
		awk -v check_netls=$CHECK_NETLS '
	       	   BEGIN { FS="|" }
		   $0 !~ /.*ALL|/ {
		      if ( check_netls == "false" )
		         printf("%s %s ", $(NF - 1), $NF)
		      else
		         # Install latest
                         if ( LATEST == "true") {
                            NETLS_CODE = $(NF -2)
                            if ( NETLS_CODE != "!")
                               printf("%s %s ", $(NF - 1), $NF)
                         }
                         # Install from all available software
                         else {
                            NETLS_CODE = $(NF -4)
                            if ( NETLS_CODE != "!")
                               printf("%s %s ", $(NF - 1), $NF)
                         }
                  } ' $INSTALLP_DB_FILE
		  [ -f $INSTALLP_DB_FILE ] || return 1
		  rm -f $INSTALLP_DB_FILE
		  return 0
	fi

	# make sure the ARG list has at least one comma (for algorithm), and
	# if it does not, add one at the end so that it does.  Also strip
	# out leading commas or duplicate commas arising from the selection
	# of blank lines in smit F4 listing.

	SOFTWARE=$(print $SOFTWARE | awk '
		BEGIN {RS=","} 
		$0=="" {continue}
		{printf("%s,",$0)}')

	# Get the first field from the users SOFTWARE listing
	typeset -i FIELD=1
	USER_VALUE=$(print $SOFTWARE | cut -f$FIELD -d',' | 
		/usr/ucb/tr -d !*)

        FIRST_TIME=true
	while [ -n "$USER_VALUE" ]; do

           # Check if the product/option name is present separated by the @@ 
           # delimiter after the nice description. If it is then simply print
           # that value.

             VALUE=$(print $USER_VALUE | awk -F "@@" '{ print $2 }')

           # If the product/option name is not present at the end of the nice
           # description, then invoke installp_format and redirect the output
           # to a file. This should be done only if we have not already 
           # executed installp_format.

             [ -z "$VALUE" -a "$FIRST_TIME" = "true" ] && {
                FIRST_TIME=false
                installp_format -$TYPE $LATEST_FLAG \
			-o$INSTALLP_DB_FILE -$DEVICE -n || return $RC
		INSTALLP_FORMAT_RUN=true
             }

             if [ -n "$VALUE" ]; then
                 print $VALUE
             else 

       	        # The USER_VALUE variable contains the "nice" name 
		# that we are going to look in the file for.  Strip
		# out extraneous spaces.

		USER_VALUE=$(print $USER_VALUE | \
			awk '{n=0 ; while (++n <= NF) printf("%s ",$n)}')

	        # Move through the database removing all ws, and provide
	        # that as input to the next awk which compares the comma
	        # delimited input to the first field of the database

	        cat $INSTALLP_DB_FILE | \
	        sed "s/[ 	]//g" | \
	        awk -v USER_VALUE="$USER_VALUE" -v LATEST=$LATEST '
	       	  BEGIN {
			FS="|"
			print_record = "false"		# used for printing
			match_found = "false"		# find a db match?
			all = "false"			# input contain the
			if (USER_VALUE ~ /.*ALL$/) {	#   ALL keyword?
				all = "true"
			}
			if (USER_VALUE ~ /^\.$/ || USER_VALUE ~ /#.*$/) {
				match_found = "true"
				exit 0
			}
		  }

		# If the comma delimited input value is the same as the current
		# line, and the input is not an ALL record, simply print out
		# the line and exit.  If the input is an ALL record, set the
		# print_record flag, set the type of PTF found, and set the
		# criteria that differentiates that PTF from the PTFs below it
		# in the list (ie opp/option/fileset name), then continue
		# reading (and printing) the next set of records that the ALL
		# in the database represents.
		#
		# If we are doing a query based off a cmd_to_list database
		# that is of the latest menu type, if we have a match, we
		# simply print the image and the version from that field, and
		# we are done.
		$1 == USER_VALUE {

			if (LATEST == "true") {
				match_found = "true"
				printf("%s %s ", $(NF - 1), $NF)
				exit 0
			}

			if (print_record != "true") {
				if (all == "false") {
					match_found = "true"
					printf("%s %s ", $(NF - 1), $NF)
					exit 0
				}

				pkg_type = $2

				# We know we have a ICEM ptf type that is is
				# an ALL header, thus print_record flag is set
				# so each line that follows that matches the
				# version and string criteria will be printed
				if (pkg_type ~ /[ICEM]/) {
					print_record = "true"
					version = $3
					string = $4
					next
				}

				# The matching PTF is an S/F/G PTF with ALL
				# Set the print_record flag so following line
				# that matches the version/string combination
				# will be printed
				print_record = "true"
				ALL_grouping_type = $4
				version = $3
				string = $5
				next
			}
		}

		{	#default action
			if (print_record == "true") {
				if (pkg_type ~ /[ICEM]/) {
					if ($4 == string && $3 == version) {
						match_found = "true"
						printf("%s %s ",
							$(NF - 1), $NF)
						next
					}
					exit 0
				}

				# If the record is an ALL record, it should
				# just be skipped because there can be ALL
				# headings within other ALL headings, and an
				# ALL heading by itself does not represent the
				# actual PTF, the PTFs it represents will be
				# below it.
				if ($1 ~ /.*ALL$/) {
					next
				}

				if (ALL_grouping_type == "OPP") {
					if ($5 == string && $3 == version) {
						match_found = "true"
						printf("%s %s ",
							$(NF - 1), $NF)
						next
					}
					exit 0
				}
				if (ALL_grouping_type == "OPTION") {
					if ($6 == string && $3 == version) {
						match_found = "true"
						printf("%s %s ",
							$(NF - 1), $NF)
						next
					}
					exit 0
				}
				if (ALL_grouping_type == "SS") {
					if ($7 == string && $3 == version) {
						match_found = "true"
						printf("%s %s ",
							$(NF - 1), $NF)
						next
					}
					exit 0
				}
			}
		}
		END {
			if (match_found == "true") {
				exit 0	# indicates a database value
			}
			else {
				exit 9	# indicates user typed in value
			}
		}
		' 

		# If the code above exited with a return code of 9, that
		# signifies that the field that was being searched for in
		# the database does not exist, thus, it must not have come
		# from the smit f4 selection list, thus the assumption is
		# that the user typed the value in (ie something like bos.obj)
		# so that value is passed onto smit by the following lines of
		# code.
		[ $? -eq 9 ] && {
			print $USER_VALUE
		}

             fi
		# Get the next comma delimited field from the software listing
		# smit field.
		FIELD=$FIELD+1
		USER_VALUE=$(print $SOFTWARE | cut -f$FIELD -d',' | 
			/usr/ucb/tr -d !*)
	done

	[ -r $INSTALLP_DB_FILE ] || return 1
	rm -f $INSTALLP_DB_FILE 

	return 0
}

#-----------------------------------------------------------------------------
# installp_cmd_signal_handler
#
# Signal handler for the installp_cmd routine.
#-----------------------------------------------------------------------------
installp_cmd_signal_handler()
{
	# Ignore signals in signal handler
	trap '' INT QUIT TERM

	[ -n "$TMPFILE" ] && rm -f $TMPFILE
	[ -r "$INSTALLP_DB_FILE" ] && rm -r $INSTALLP_DB_FILE

	# Unmount the cdrom drive it it needs to be unmounted
	[ "$UNMOUNT" = "TRUE" ] && unmount_cd $DEVICE
	UNMOUNT=FALSE

	exit 1
}

#=============================================================================
# bffcreate_format
#
# This function is used to format the cmd_to_list output for the bffcreate
# command to allow the user to select what packages they want to use the
# bffcreate command on.
#=============================================================================
bffcreate_format()
{
	# Set a signal handler to do cleanup activities
	trap 'bffcreate_format_signal_handler' INT TERM QUIT

	# Initialize variables
	DEVICE=
	UNMOUNT=
	VALUE=
	RC=0
	TMPFILE=

	# The first argument to this command is the device name
	DEVICE=$1

	# Ensure device specification is valid
	check_device $DEVICE 

	# This command creates a temporary file for a short period of time
	# in the following location
	TMPFILE=/tmp/.bffcreate_format.$$

	UNMOUNT=FALSE		# flag indicating if CD-ROM fs needs unmounted

	# If the user specified a CD-ROM device:
	# If it is *not* already mounted:
	#    mount it
	#    set a flag indicating that it needs to be unmounted later
	# In either case (needs to be mounted or not):
	#    get its mount point
	if [[ $DEVICE = /dev/cd* ]]; then
		VALUE=$(get_cd_mount_point $DEVICE)
                if [ $? != 0 ]; then
                   inuumsg 155
                   return 1
                fi
		DEVICE=$(print $VALUE | cut -f1 -d' ')
		UNMOUNT=$(print $VALUE | cut -f2 -d' ')
        fi

	# Get the output from bffcreate and store it in a temporary file
	bffcreate -lJqd $DEVICE | grep -v "#" >$TMPFILE
	RC=$?
	[ $RC != 0 ] && {
		[ -f $TMPFILE ] && rm -f $TMPFILE
		[ "$UNMOUNT" = "TRUE" ] && unmount_cd $DEVICE
		return $RC
	}

	# If we mounted a CD-ROM device real-time, we are now done with it
	# so unmount it.
	[ "$UNMOUNT" = "TRUE" ] && unmount_cd $DEVICE

	# Format the output from the installp command for the cmd_to_list
	# output within SMIT
        inuumsg 171
	cat $TMPFILE | awk '{ printf ("   %-32s %-25s\n", $1, $2) }' | sort -u -

	# Get rid of the temporary file
	rm -f $TMPFILE
	
	[ $RC != 0 ] && return $RC

	return 0
}

#-----------------------------------------------------------------------------
# bffcreate_format_signal_handler
#
# Signal handler for the bffcreate_format command.
#-----------------------------------------------------------------------------
bffcreate_format_signal_handler()
{
	# Ignore signals in signal handler
	trap '' INT TERM QUIT

	# Remove the temporary files if they exist
	[ -n "$TMPFILE" ] && rm -f $TMPFILE

	# Unmount the cd rom if necessary
	[ "$UNMOUNT" = "TRUE" ] && unmount_cd $DEVICE
	UNMOUNT=FALSE

	exit 1
}

#=============================================================================
# bffcreate_cmd
#=============================================================================
bffcreate_cmd()
{
	trap 'bffcreate_cmd_signal_handler' INT TERM QUIT

	DEVICE=
	T=
	W=
	X=
	SW=
	VALUE=
	UNMOUNT=

	while getopts d:t:w:X FLAG; do
		case $FLAG in
			d)	DEVICE="$OPTARG";;	# device
			t)	T="-t$OPTARG";;		# sw store directory
			w)	W="-w$OPTARG";;		# temporary storage
			X)	X="X";;			# expand fs
		esac
	done

	shift OPTIND-1			# shift off the cmdline flags

	SW=$*				# remainder is software

	# Ensure device specification is valid
	check_device $DEVICE 

	# If we have a CD-ROM device:  If it is not already mounted, mount
	# it, in either case, get its mount point.  If we mount it, we set
	# a flag so it can be unmounted at the end of our set of operations
	#
	# Note: The d in front of of the device is intential.  It is the flag
	# that is used for the bffcreate command.
        if [[ $DEVICE = /dev/cd* ]]; then
		VALUE=$(get_cd_mount_point $DEVICE)
                if [ $? != 0 ]; then
                   inuumsg 155
                   return 1
                fi
		DEVICE=$(print $VALUE | cut -f1 -d' ')
		UNMOUNT=$(print $VALUE | cut -f2 -d' ')
	fi

	print "bffcreate -qv$X $T $W -d $DEVICE \\"
	print "    $SW \\"
	print "    2>&1"

	# Execute the actual bffcreate command with its arguments
	bffcreate -qv$X $T $W -d $DEVICE $SW 2>&1
	RC=$?

	# Unmount the cd rom if necessary
	[ "$UNMOUNT" = "TRUE" ] && unmount_cd $DEVICE
	UNMOUNT=FALSE

	return $RC
}

#-----------------------------------------------------------------------------
# bffcreate_cmd_signal_handler
#
# Signal handler for the bffcreate_cmd routine
#-----------------------------------------------------------------------------
bffcreate_cmd_signal_handler()
{
	# Ignore signals in signal handler
	trap '' INT TERM QUIT

	# Unmount the cd rom if necessary
	[ "$UNMOUNT" = "TRUE" ] && unmount_cd $DEVICE
	UNMOUNT=FALSE

	exit 1
}

#=============================================================================
# list_devices
#
# Lists the currently available devices to use for install to stdout for the
# user
#=============================================================================

#-----------------------------------------------------------------------------
# main for the list_devices program
#-----------------------------------------------------------------------------
list_devices()
{
	IMAGES_DIR=/usr/sys/inst.images

	# This order represents the order we currently think is most
	# often used by customers
	list_device tape .1
	list_device diskette
	list_cdrom
	[ "$1" != "bffcreate" ] && list_filesystem_images

	return 0
}

#-----------------------------------------------------------------------------
# Take a device as an argument and check to see if it available, if so, print
# that device out
#-----------------------------------------------------------------------------
list_device()
{
	CLASS=$1
	EXTENSION=$2

	lsdev -C -c $CLASS -S a -F 'name description' 2>/dev/null |\
		while read NAME DESCRIPTION; do
			print "/dev/$NAME$EXTENSION ($DESCRIPTION)"
		done

	return 0
}


#-----------------------------------------------------------------------------
# List cdrom if it exists
#-----------------------------------------------------------------------------
list_cdrom()
{
   lsdev -C -c cdrom -S a -F 'name description' 2>/dev/null | \
   while read NAME DESCRIPTION; do
      MOUNT_POINT=$(mount 2>/dev/null | awk '
                  $1 == NAME { MOUNT_POINT=$2 }
                  END { print MOUNT_POINT }
                  ' NAME=/dev/$NAME)

      if [ "$MOUNT_POINT" ]; then
         print "/dev/$NAME ($MOUNT_POINT)"
      else
         print "/dev/$NAME ($DESCRIPTION)"
      fi
   done

   return 0
}


#-----------------------------------------------------------------------------
# If there are images in the IMAGES_DIR directory, list the directory as well
# as the images in the directory
#-----------------------------------------------------------------------------
list_filesystem_images()
{
   if [ -d $IMAGES_DIR ]; then
      ls $IMAGES_DIR/ | grep -v '^\.' >/dev/null 2>&1 \
         && print "$IMAGES_DIR (Install Directory)" #prt dir
   fi

   return 0
}


#=============================================================================
# check_device
#
# Checks the specified device to ensure that it is valid
#=============================================================================
check_device()
{
   # Check that device file or directory exists and is readable
   if [ -r $1 ]; then
      case $1 in
         /dev/rmt*)
             [ -c $1 ] && return 0      # Tape drive
             ;;
         /dev/fd*|/dev/cd*)
             [ -b $1 ] && return 0      # Diskette or CD-ROM drive
             ;;
         /dev/*)
             inuumsg 153
             exit 1
             ;;
         *)
             if [ -d $1  -a  ! -s $1/.toc ]; then
                inutoc $1 && return 0
             else
                return 0
             fi
             ;;
      esac
   else
      inuumsg 154
      exit 1
   fi
   inuumsg 155
   exit 1
}


#=============================================================================
# list_bundles
#
# Lists the currently available bundles to use for install to stdout for the
# user.  Strips out bundle files with zero size.
#=============================================================================
list_bundles()
{
   # Easy debug code
   [ -n "$SM_INST_DEBUG" ] && set -x

   SYS_BUNDLE_DIR=/usr/sys/inst.data/sys_bundles
   USER_BUNDLE_DIR=/usr/sys/inst.data/user_bundles

   if [ ! -d $SYS_BUNDLE_DIR ]; then
      inuumsg 156 $SYS_BUNDLE_DIR
      exit 1
   fi

   for BUNDLE in $(ls $SYS_BUNDLE_DIR/*.bnd $USER_BUNDLE_DIR/*.bnd 2>/dev/null)
   do
      [ -s $BUNDLE ] && print $BUNDLE
   done | sed -e s,.bnd$,, -e s,$SYS_BUNDLE_DIR/,, -e s,$USER_BUNDLE_DIR/,, | 
          sort -A -u
}


#=============================================================================
# make_bundle
#
# Parses the user specified bundle file, eliminating comments, duplicates
# and invalid specifications within the bundle (including any specific VRMF)
#=============================================================================
make_bundle()
{
   # Easy debug code
   [ -n "$SM_INST_DEBUG" ] && set -x

   BUNDLE=$1
   IN_BUNDLE_FILE=$2
   OUT_BUNDLE_FILE=$3

   egrep -v "^#|^$" <$IN_BUNDLE_FILE | \
   cut -f1 -d" " | sort -A -u >$OUT_BUNDLE_FILE
   if [ ! -s $OUT_BUNDLE_FILE ]; then
      # No output therefore invalid bundle contents
      inuumsg 157 $BUNDLE
      return 1
   fi
}

#=============================================================================
# make_filter
#
# Applys the system defined criteria for filters and creates
# a temporary filter file containing the selective results.
# This is not possible for filters based on licenses.
# The filter may override the -iems flags specified on the command line.
#=============================================================================
make_filter()
{
   # Easy debug code
   [ -n "$SM_INST_DEBUG" ] && set -x

   FILTER=$1
   TMPFILTERFILE=$2

   case $FILTER in
      # Not documented for use by customers.  Only used
      # for updating all installed filesets to latest level.
      _UPDATE_ALL|_update_all)
         lslpp -qLc | cut -f2 -d: | \
         sort -A -u >$TMPFILTERFILE
         INST=true  # list install packages
         ENH=true   # list enhancements
         ML=true    # list maintenance levels
         SS=true    # list filesets
      ;;
      ALL_UPDATES|all_updates)
         lslpp -qLc | cut -f2 -d: | \
         sort -A -u >$TMPFILTERFILE
         INST=false # do not list install packages
         ENH=true   # list enhancements
         ML=true    # list maintenance levels
         SS=true    # list filesets
      ;;
      ALL_MAINTENANCE|all_maintenance)
         lslpp -qLc | cut -f2 -d: | \
         sort -A -u >$TMPFILTERFILE
         INST=false # do not list install packages
         ENH=false  # do not list enhancements
         ML=true    # list maintenance levels
         SS=false   # do not list filesets
      ;;
      ALL_ENHANCEMENTS|all_enhancements)
         lslpp -qLc | cut -f2 -d: | \
         sort -A -u >$TMPFILTERFILE
         INST=false # do not list install packages
         ENH=true   # list enhancements
         ML=false   # do not list maintenance levels
         SS=false   # do not list filesets
      ;;
      ALL_FILESET_UPDATES|all_fileset_updates)
         lslpp -qLc | cut -f2 -d: | \
         sort -A -u >$TMPFILTERFILE
         INST=false # do not list install packages
         ENH=false  # do not list enhancements
         ML=false   # do not list maintenance levels
         SS=true    # list filesets
      ;;
      ALL_LICENSED|all_licensed)
         # Handle this one later as is a additive filter
         # Use default category selection criteria
      ;;
      *)
         # Invalid filter name
         return 1
      ;;
   esac

}

#=============================================================================
# Shared code used by multiple programs above
#=============================================================================

#=============================================================================
# get_cd_mount_point
#
# Arguments:  Expects one argument, the CD-ROM device (eg. "/dev/cd0")
#
# Ouputs:  Prints the directory where the .toc was found followed by
#          the string "TRUE" or "FALSE".  This string indicates whether
#          or not the CD-ROM should be unmounted when we're finished with
#          it.  "TRUE" indicates that it wasn't previously mounted (we had
#          to create a CR-ROM file system and mount it); therefore we 
#          should unmount it when we're finished.  "FALSE" says that it 
#          was already mounted, therefore we don't want to unmount it when 
#          we're finished.
#
# Returns:  0 if .toc was found (successful)
#           1 if unable to located the .toc (failed)
#
#=============================================================================
get_cd_mount_point()
{
   DEVICE=$1

   #------------------------------------------------------------------------
   # If CD-ROM is already mounted, get the mount point.  It could be mounted
   # multiple times, so return the "last" mount point as the one to be used.
   #------------------------------------------------------------------------
   MOUNT_POINT=`mount 2>/dev/null | grep $DEVICE 2>/dev/null | tail -1 |
                awk '{ print $2 }'`

   if [ "$MOUNT_POINT" ]; then
      #------------------------------------------------------------------
      # The CD-ROM is already mounted so we don't we want to leave it the 
      # way it is when we're finished with it (leave it mounted).
      #------------------------------------------------------------------
      UNMOUNT=FALSE
   else
      UNMOUNT=TRUE
      #-----------------------------------------------------------------
      # The CD-ROM was not mounted so we create a CD-ROM file system and 
      # mount the CD-ROM over that file system.
      #-----------------------------------------------------------------
      BASE_DEV=${DEVICE##*/}
      MOUNT_POINT=/mnt$$
      crfs -v cdrfs -p ro -d $BASE_DEV -m $MOUNT_POINT no || return 1
      mount $MOUNT_POINT
      if [ $? != 0 ]; then
         rmfs $MOUNT_POINT
         rmdir $MOUNT_POINT
         return 1
      fi
   fi

   #------------------------------------------------------------------
   # Determine the directory which contains the .toc and return that 
   # directory along with information about whether or not to unmount
   # the CDROM when finished.
   #------------------------------------------------------------------
   if [ -s $MOUNT_POINT/.toc ]; then
      print $MOUNT_POINT $UNMOUNT
   elif [ -s $MOUNT_POINT/usr/sys/inst.images/.toc ]; then
      print $MOUNT_POINT/usr/sys/inst.images $UNMOUNT
   else
      #-----------------------------------------------------------
      # We were unable to locate the .toc - must be invalid media.
      #-----------------------------------------------------------
      if [[ $UNMOUNT = TRUE ]]; then
         unmount_cd $MOUNT_POINT
      fi
      return 1
   fi

   return 0
}


unmount_cd()
{
   DEVICE=$1

   MOUNT_POINT=$(print $DEVICE | cut -d"/" -f2)
   unmount /$MOUNT_POINT || return 1
   rmfs /$MOUNT_POINT || return 1
   rmdir /$MOUNT_POINT || return 1
   
   return 0
}


instfix_sig_handler()
{
   # Ignore signals in signal handler
   trap '' INT TERM QUIT

   # Remove the temporary files 
   rm -f $TMPFILE

   # Unmount the cd rom if necessary
   [ "$UNMOUNT" = "TRUE" ] && unmount_cd $DEVICE
   UNMOUNT=FALSE

   exit 1
}


instfix_cmd()
{
   # Easy debug code
   [ -n "$SM_INST_DEBUG" ] && set -x

   # Trap.. catch signals to remove temporary files, etc.
   trap 'instfix_sig_handler' INT TERM QUIT

   c=
   p=
   v=
   N=
   X=
   TDIR=
   VERB=
   DEVICE=
   FILESETS=/tmp/.instfix_filesets.$$
   SELECTIONS=/tmp/.instfix_selections.$$

   # Parse the command line
   while :; do
      case $1 in
         -c)     c="c";;                       # Commit
         -d)     DEVICE=$2                     # Device
                 shift;;
         -f)     tr ' ' '\n' <$2 >$SELECTIONS  # File containing selections
                 shift;; 
         -p)     p="p";;                       # Preview
         -t*)    TDIR=" $1 ";;                 # -tAlt_Save_Dir
         -v)     v="v";;                       # Verify installation
         -N)     N="N";;                       # No Save
         -X)     X="X";;                       # Extend file system
         -V2)    VERB=" -V2 ";;                # Set verbosity to -V2
         "")     break;;
         *)      break;;
      esac
      shift                   # shift arguments to look at next one
   done

   # Ensure device specification is valid
   check_device $DEVICE

   # If we have a CD-ROM device:  If it is not already mounted, mount
   # it, in either case, get its mount point.  If we mount it, we set
   # a flag so it can be unmounted at the end of our set of operations.

   if [[ $DEVICE = /dev/cd* ]]; then
      VALUE=$(get_cd_mount_point $DEVICE)
      [ $? != 0 ] && return 1
      DEVICE=$(print $VALUE | cut -f1 -d' ')
      UNMOUNT=$(print $VALUE | cut -f2 -d' ')
   fi

   # Print the instfix command
   print "instfix -d $DEVICE -p -f $SELECTIONS > File\n"

   # Get option list with instfix command
   instfix -d $DEVICE -p -f $SELECTIONS >$FILESETS || return 1

   # Print the installp command
   print "\ninstallp -${p}a${c}g${N}B${X}${v}${VERB}${TDIR} -f File -d $DEVICE"

   # Show the contents of $FILESETS indented a few spaces
   print "\nFile:\n"
   cat $FILESETS | sed 's/^/    /'
   print

   # Call installp
   installp -${p}a${c}g${N}B${X}${v}${VERB}${TDIR} -f $FILESETS -d $DEVICE
   RC=$?

   # Clean up
   rm -f $SELECTIONS $FILESETS >/dev/null 2>&1

   [ "$UNMOUNT" = "TRUE" ] && unmount_cd $DEVICE
   UNMOUNT=FALSE

   return $RC
}


instfix_list()
{
   # Easy debug code
   [ -n "$SM_INST_DEBUG" ] && set -x

   #Trap.. catch signals to remove temporary files,etc.
   trap 'instfix_sig_handler' INT TERM QUIT

   DEVICE=$1

   check_device $DEVICE

   if [[ $DEVICE = /dev/cd* ]]; then
      VALUE=$(get_cd_mount_point $DEVICE)
      if [ $? != 0 ]; then
         inuumsg 155
         return 1
      fi
      DEVICE=$(print $VALUE | cut -f1 -d' ')
      UNMOUNT=$(print $VALUE | cut -f2 -d' ')
   fi

   RC=0
   instfix -T -d $DEVICE || RC=1

   [ "$UNMOUNT" = "TRUE" ] && unmount_cd $DEVICE
   UNMOUNT=FALSE

   return $RC
}


#=============================================================================
# MAIN
#=============================================================================

export PATH=/usr/sbin:/usr/bin:/etc:/bin:/usr/lib/instl:$PATH

# Easy debug code
[ -n "$SM_INST_DEBUG" ] && set -x

# Exit if no command line parameters
[ -z "$1" ] && exit 1

FUNCTION=$1
shift

if [ $FUNCTION = list_bundles ]; then
   $FUNCTION
   exit $?
else
   $FUNCTION $*
   exit $?
fi

exit 1
