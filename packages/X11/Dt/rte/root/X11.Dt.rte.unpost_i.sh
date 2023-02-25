#!/usr/bin/ksh
# @(#)29        1.5  src/packages/X11/Dt/rte/root/X11.Dt.rte.unpost_i.sh, pkgxcde, pkg41J, 9517A_all 4/25/95 12:20:24
#
#   COMPONENT_NAME: pkgxcde
#
#   FUNCTIONS: unpost_i.sh
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.


# function to restore the files from a directory and its
# subdirectories

restore_dir()
{
    RESTORE_DIR=$LOCAL_MIGSAVE$DEST_DIR$DIRECTORY
    if [ -d $RESTORE_DIR ]
    then
       rc=0
       if [ ! -d $DEST_DIR ]
       then
          mkdir -p $DEST_DIR
          rc=$?
       fi
       if [ $rc = 0 ]
       then
          cp -p -r $RESTORE_DIR $DEST_DIR
          if [ $? != 0 ]
          then
             /usr/sbin/inuumsg $INU_MIG_NONDIR $LOCAL_MIGSAVE
          fi
       else
             /usr/sbin/inuumsg $INU_MIG_NONDIR $LOCAL_MIGSAVE
       fi
    fi
}

# This is the start of code copied from the migrate_cfg script.
# Error message indices from inuumsg_msg.h
INU_MIG_NONDIR=136
# This is the end of this part of code copied from migrate_cfg script.

# Since some of the processing is done when MIGSAVE is not
# set. A local value for the save directory needs to be set.
# If the system location for save files during install changes
# in the future, then this value also needs to change.
LOCAL_MIGSAVE="/lpp/save.config"
DIRECTORY="/config"
DEST_DIR="/etc/dt"
restore_dir
DIRECTORY="/types"
DEST_DIR="/etc/dt/appconfig"
restore_dir
exit 0
