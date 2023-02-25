#!/usr/bin/ksh
# @(#)28        1.4  src/packages/X11/Dt/rte/root/X11.Dt.rte.post_i.sh, pkgxcde, pkg41J, 9517A_all 4/25/95 12:20:18
#
#   COMPONENT_NAME: pkgxcde
#
#   FUNCTIONS: post_i.sh
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.


# function to check for valid save directory
check_migsave()
{
   # This is the start of code copied and modified
   # from the migrate_cfg script.
   if [ ! -d $LOCAL_MIGSAVE ]
   then
      # Check to make sure that this is not a non-directory
      if [ -r $LOCAL_MIGSAVE ]
      then
         /usr/sbin/inuumsg $INU_MIG_NONDIR $LOCAL_MIGSAVE
         exit 1
      else
         # Create the directory, if possible
         /bin/mkdir -p $LOCAL_MIGSAVE ||
         (/usr/sbin/inuumsg $INU_MIG_NONDIR $LOCAL_MIGSAVE; exit 1)
      fi
   fi

   # Make sure that we can write to the save directory since we clean up in there

   if [ ! -w $LOCAL_MIGSAVE ]
   then
       /usr/sbin/inuumsg $INU_MIG_NONDIR $LOCAL_MIGSAVE
       exit 1
   fi
   # This is the end of code copied and modified from the migrate_cfg script.
}

# This is the start of code copied from the migrate_cfg script.
# Error message indices from inuumsg_msg.h
INU_MIG_USAGE=133
INU_MIG_FAIL=134
INU_MIG_NOCFG=135
INU_MIG_NONDIR=136
# This is the end of this part of code copied from migrate_cfg script.

# Since some of the processing is done when MIGSAVE is not
# set. A local value for the save directory needs to be set.
# If the system location for save files during install changes
# in the future, then this value also needs to change.
LOCAL_MIGSAVE="/lpp/save.config"

# If not migrating from an old version of the desktop to a new
# version, then move potentially incompatible
# scripts in /etc/config and all its subdirectories into the
# save directory.
DIRECTORY="/etc/dt/config"

# Design note: This script finds individual files to move rather than
# copying entire directories with "cp -p -r" in order to avoid
# deleting any previously saved files in /lpp/save.config/etc/dt/config
# and to retain the directory structure in /etc/dt.
if [ -z "$MIGSAVE" ] && [ -d $DIRECTORY ]
then
   check_migsave
   for DIR_TO_BE_SAVED in `find $DIRECTORY -type d  -print`
   do
      SAVE_DIR=$LOCAL_MIGSAVE$DIR_TO_BE_SAVED
      if [ ! -d $SAVE_DIR ]
      then
         mkdir -p $SAVE_DIR
         if [ $? != 0 ]
         then
            /usr/sbin/inuumsg $INU_MIG_NONDIR $LOCAL_MIGSAVE
            exit 1
          fi
      fi
      for FILE_TO_BE_SAVED in `find $DIR_TO_BE_SAVED -type f  -print`
      do
           DIR_FILE=`dirname $FILE_TO_BE_SAVED`
           BASE_FILE=`basename $FILE_TO_BE_SAVED`
           SAVE_NAME=$SAVE_DIR"/"$BASE_FILE
           if [ "$DIR_FILE" = "$DIR_TO_BE_SAVED" ]
           then
              mv  $FILE_TO_BE_SAVED $SAVE_NAME
              # if the move fails it is not necessary to restore
              # any files since this would be done later
              # by X11.Dt.rte.root.unpost_i.sh which would be called
              # due to the failure of the installation.
              if [ $? != 0 ]
              then
                 /usr/sbin/inuumsg $INU_MIG_NONDIR $LOCAL_MIGSAVE
                 exit 1
              fi
          fi
      done
   done
fi

#
#
# Regardless of whether the user is migrating
# from an old version of the desktop to a new version or not,
# convert all action definition files in the subdirectories
# of /etc/dt/appconfig/types to a format compatible with the new desktop.
#
DIRECTORY="/etc/dt/appconfig/types"
for ACTION_FOLDER in `find $DIRECTORY -type d  -print`
do
        check_migsave
        BACKUP_FOLDER=$LOCAL_MIGSAVE$ACTION_FOLDER
        /usr/dt/migrate/bin/dtmigrate $ACTION_FOLDER $BACKUP_FOLDER
done
# ignore unsuccessful return codes from dtmigrate since
# the unconverted files will remain in /etc/dt/appconfig/types/<LANG>
# and the user can fix the problem and rerun dtmigrate later.
exit 0
