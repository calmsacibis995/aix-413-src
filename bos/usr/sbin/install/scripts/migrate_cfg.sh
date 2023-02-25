#!/usr/bin/ksh
# @(#)28        1.2  src/bos/usr/sbin/install/scripts/migrate_cfg.sh, cmdinstl, bos411, 9428A410j 6/14/94 17:13:11
# COMPONENT_NAME: (CMDINSTL) command install
#
# FUNCTIONS: migrate_cfg (default user-configurable file migration)
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
#
# NAME: migrate_cfg
#                                                                    
# FUNCTION: 
#       This script will perform default migration processing for
#       files in the <option>.cfgfiles file.
#
# ARGUMENTS:
#       $1 is the user-configuration filename, usually
#          /usr/lpp/<package>/<option>.cfgfiles.
#       $2 is the migration save directory.  If $2 is not specified,
#          then MIGSAVE must be set to the migration save directory.
#
#       The user-configurable file is a list of files with their
#       handling methods.  The file name should be a full pathname.
#	Handling methods are recognized by their first letter.
#	Recognized handling methods are: 
#	   p(reserve):   replace the installed file with the file from the
#		         save directory to the original location
#	   a(uto_merge): replace the installed file with the merged file from
#		         the save directory.
#	   s(afe_merge): leave the merged file in the save directory
#	   u(ser_merge): leave the saved file in the save directory
#	   h(old_new):   replace the installed file with the file from the
#		         save directory to the original location, but leave a
#		         copy of the installed file in the save directory
#	   <blank>:      same as preserve
#
#	   All other handling methods will result in the saved_file being
#	   left in the saved directory.
#
#	 If handling a file fails for some reason, then the original file is
#	 left in the save directory and an error message is displayed.  Since
#	 processing can still continue, a return code of 0 is still returned.
#
#
# RETURN VALUE DESCRIPTION:
#    0 if migration succeeds or can continue (some migrations may fail)
#   !0 if migration cannot continue
#
# NOTE:  Size requirements must be calculated prior to calling migrate_cfg.
#
#---------------------------------------------------------------------------
#
#

# Usage message if incorrect or missing parameters
usage() {
# echo "Usage: migrate_cfg filename save_directory"
# echo "   or: migrate_cfg filename # and MIGSAVE is set to the save_directory"
   /usr/sbin/inuumsg $INU_MIG_USAGE 
}

# Common migration failed message
migfail() {
#   echo "migrate_cfg: Unable to migrate $1, saved version remains in $MIGSAVE/$1."
    /usr/sbin/inuumsg $INU_MIG_FAIL $1 $1
}

#Main program

# Error message indices from inuumsg_msg.h
INU_MIG_USAGE=133
INU_MIG_FAIL=134
INU_MIG_NOCFG=135
INU_MIG_NONDIR=136

# Saved-off configuration files list
INU_STORED_CFGLIST=/var/adm/sw/cfgfiles.stored

# Check for reasonable input to script (fn and dir, or fn and $MIGSAVE)
if [ $# -eq 0 ] || [ $# -lt 2 ] && [ "x$MIGSAVE" = "x" ]
then
   usage
   exit 1
fi

# First parm is the name of the user-configurable file list
cfgfile=$1

# If second parameter is specified, use that as save directory
if [ "x$2" != "x" ]
then
   MIGSAVE=$2
fi

#  Make sure that we can access a non-empty config list file
if [ ! -r $cfgfile ] || [ ! -s $cfgfile ]
then
   #echo $0: Cannot read configuration file $cfgfile
   /usr/sbin/inuumsg $INU_MIG_NOCFG $cfgfile
   exit 1
fi

# Make sure that we can write to the save directory
if [ ! -d $MIGSAVE ]
then
   # Check to make sure that this is not a non-directory
   if [ -r $MIGSAVE ]
   then
      #echo $0: Target for save data must be a directory
      /usr/sbin/inuumsg $INU_MIG_NONDIR $MIGSAVE
      exit 1
   else
      # Create the directory, if possible
      mkdir -p $MIGSAVE || 
      (/usr/sbin/inuumsg $INU_MIG_NONDIR $MIGSAVE; exit 1)
   fi
fi

# Make sure that we can write to the save directory since we clean up in there

if [ ! -w $MIGSAVE ]
then
    #echo $0: Unable to write to directory $MIGSAVE
    /usr/sbin/inuumsg $INU_MIG_NONDIR $MIGSAVE
    exit 1
fi

# Process the file containing the user-configurable files and handling methods
# The format of the file is full_path/filename handling method
# The '#' is a comment character
# Any fields after the handling method on a line are ignored, so that space
# may also be used for comments.

cat $cfgfile |
    while read file method comments
    do
	  #Skip blank lines
	  if [ "x$file" = "x" ]
	  then
	     continue
	  fi

	  #Skip comments
	  if expr $file : "[#]" >/dev/null
	  then
	     continue
	  fi

	  # If the file wasn't saved off then continue, nothing to merge
	  if [ ! -r $MIGSAVE/$file ]
	  then
	     continue
	  fi

          #Process cfgfiles
	  case $method in 
	     # For preserve or automerge, just place back into filesystem
	     # If replacement fails, 
	     p*|a*|"")
		# No error condition if migrated file does not exist
		if [ -f $MIGSAVE/$file ]
		then
		   # Place over existing file - display msg if fails
		   mv $MIGSAVE/$file /$file || migfail $file
		fi
		;;
	     # For hold_new, place back into filesystem but save installed
	     # version.  If we fail saving off the new version, 
	     h*)
		if [ -f $MIGSAVE/$file ]
		then
		    # Copy installed version into tempfile in save directory
		    cp -p $file $MIGSAVE/tmpfile || (migfail $file; continue)
		    # Move saved or merged version into filesystem
		    mv $MIGSAVE/$file /$file || (migfail $file; continue)
		    # Move installed version into save location
		    mv $MIGSAVE/tmpfile $MIGSAVE/$file >/dev/null 2>&1
		fi
		;;
	     other)
	        ;;
	     # For anything else, just leave it in the save directory
	     # and append the file and the "part" to the file containing
	     # the list of stored, unmerged files
	     *)
		echo $file $INUTREE >> $INU_STORED_CFGLIST 2>/dev/null
		;;
	  esac
    done

    # At this point we should clean up the save directory if possible
    # However this is most appropriately done in rminstal so that 
    # multiple migrations can be cleaned up simultaneously
    # find $MIGSAVE/* -type d -print |
    #				sort -r | xargs -i rmdir {} >/dev/null 2>&1

exit 0
