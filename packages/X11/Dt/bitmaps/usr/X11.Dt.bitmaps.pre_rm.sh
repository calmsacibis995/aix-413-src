#!/usr/bin/ksh
# @(#)47        1.5  src/packages/X11/Dt/bitmaps/usr/X11.Dt.bitmaps.pre_rm.sh, pkgxcde, pkg41J, 9519A_all 5/8/95 08:56:34
#
#   COMPONENT_NAME: pkgxcde
#
#   FUNCTIONS: pre_rm
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
function process_line
{
   # parse out the name of the icon file to be saved
   # from the output line from lppchk.
   # Depending on whether there are multiple files with Size or
   # Checksum errors, the output may appear with or without
   # a message number.
   # So, all varieties of output need to be checked.
   BITMAP_FILENAME=${OLDLINE##lppchk:  The checksum for file }
   if [ "$BITMAP_FILENAME" = "$OLDLINE" ]
   then
       BITMAP_FILENAME=${OLDLINE##lppchk:  Size of }
       if [ "$BITMAP_FILENAME" = "$OLDLINE" ]
       then
           BITMAP_FILENAME=${OLDLINE##lppchk: 0504-208  Size of }
           if [ "$BITMAP_FILENAME" = "$OLDLINE" ]
           then
               BITMAP_FILENAME=${OLDLINE##lppchk: 0504-212  The checksum for file }
           fi
       fi
   fi

   BITMAP_FILENAME=${BITMAP_FILENAME%% *}
   #
   # determine the directory for the file from lppchk
   #
   DIR_BITMAP_FILENAME=`dirname $BITMAP_FILENAME`
   #
   # This function is only intended to save icon files if they changed.
   # If other files in the fileset are changed, they will not be saved.
   # If other files need to be saved in the future, then the
   # number of blocks for these different directories need to
   # calculated on a per directory basis.
   if [ "$DIR_BITMAP_FILENAME" = "$ICON_DIR" ]
   then
      # save the name of the icon file to be saved in X11.Dt.bitmaps.cfgfiles
      echo "$BITMAP_FILENAME user_merge" >>$CFGFILES_FILE
      BLOCKS=`du -a $BITMAP_FILENAME`
      BLOCKS=${BLOCKS%%?/*}
      let NUMBER_OF_BLOCKS=NUMBER_OF_BLOCKS+BLOCKS
   fi
}
#
# Since this script is called when MIGSAVE is not
# set, a local value for the save directory needs to be set.
# If the system location for save files during install changes
# in the future, then this value also needs to change.
LOCAL_MIGSAVE="/usr/lpp/save.config"
# Here is sample of output from lppchk.
# lppchk:  The checksum for file /usr/dt/appconfig/icons/C/Dthvol.m.bm is 44640,
#            expected value is 34400.
#
#
SIZE_FILE="/usr/lpp/X11.Dt/X11.Dt.bitmaps.size"
TMP_SIZE_FILE="/tmp/X11.Dt.bitmaps.size_PRE_RM.TEMP"
CFGFILES_FILE="/usr/lpp/X11.Dt/X11.Dt.bitmaps.cfgfiles"
ICON_DIR="/usr/dt/appconfig/icons/C"
ICON_SAVE_DIR=$LOCAL_MIGSAVE$ICON_DIR
integer NUMBER_OF_BLOCKS
NUMBER_OF_BLOCKS=0
# read lines from the output from lppchk
# One logical line may consist of more than one physical line.
NEWLINE=""
LANG=C lppchk -c X11.Dt.bitmaps  2>&1 | while read -r LPPCHKOUT
do
    # check if this the start of an lppchk output line
    PART_OF_LINE=${LPPCHKOUT#lppchk:}
    if [ ! -z "$PART_OF_LINE" ]
    then
        # if "lppchk:" was not found on the line, then
        # the line just read is a continuation line
        if [ "$PART_OF_LINE" = "$LPPCHKOUT" ]
        then
           # continuation of a line
           # concatenate part of line just read to previous part of line
           NEWLINE=$NEWLINE$LPPCHKOUT
        else
           # start of an output line
           OLDLINE=$NEWLINE
           NEWLINE=$LPPCHKOUT
           # If there is a previous line to process, then process it.
           if [ ! -z "$OLDLINE" ]
           then
              process_line
           fi
        fi
    fi
done
OLDLINE=$NEWLINE
if [ ! -z "$OLDLINE" ]
then
   process_line
fi

# update the entries in the size file for this fileset for
# the directories required to save the icons.
# If entries do not exist, then add new entries for these directories
# to the size file for this fileset.
integer DIR_SIZES NO_SAVE_DIRS TMP_NO_SAVE_DIRS TMP_BLOCKS_READ BLOCKS_READ
if [ $NUMBER_OF_BLOCKS != 0 ]
then
    # determine subdirectories above save directory for which
    # mimimum space (i.e. 8 512-bytes blocks) is required to
    # create the directory.
    TMPDIR=$ICON_SAVE_DIR
    NO_SAVE_DIRS=0
    while true
    do
       TMPDIR=`dirname $TMPDIR`
       if [ "$TMPDIR" = "/" ]
       then
           break
       else
           SAVE_DIRS[$NO_SAVE_DIRS]=$TMPDIR
           DIR_SIZES[$NO_SAVE_DIRS]=8
           NO_SAVE_DIRS=$NO_SAVE_DIRS+1
           continue
       fi
    done
    # add icon directory to the list of directories needing space
    SAVE_DIRS[$NO_SAVE_DIRS]=$ICON_SAVE_DIR
    DIR_SIZES[$NO_SAVE_DIRS]=$NUMBER_OF_BLOCKS

    # remove file to be used to generate temporary copy of size file
    rm $TMP_SIZE_FILE >/dev/null 2>/dev/null

    # read entries from current size file to determine if any
    # entries match those needing to be added for the icon directories.
    # If they match, update with larger values as necessary.
    # Write out possibly updated entries to temporary copy of size file.
    cat $SIZE_FILE | while read -r DIRNAME BLOCKS_READ TMP_BLOCKS_READ
    do
         TMP_NO_SAVE_DIRS=0
         while true
         do
            if [ "$DIRNAME" = "${SAVE_DIRS[$TMP_NO_SAVE_DIRS]}" ]
            then
                # add size needed in directory to
                # current size + nuumber of new blocks needed.
                BLOCKS_READ=$BLOCKS_READ+${DIR_SIZES[$TMP_NO_SAVE_DIRS]}

                # set size to 0 to indicate that existing entry
                # was updated so that a duplicate entry will not
                # be written.
                DIR_SIZES[$TMP_NO_SAVE_DIRS]=0
            fi
            TMP_NO_SAVE_DIRS=$TMP_NO_SAVE_DIRS+1
            # exit from loop if at end of array.
            if [ $TMP_NO_SAVE_DIRS -gt $NO_SAVE_DIRS ]
            then
                break
            fi
         done
         if [ $TMP_BLOCKS_READ -ne  0 ]
         then
           echo $DIRNAME $BLOCKS_READ $TMP_BLOCKS_READ >>$TMP_SIZE_FILE
         else
           echo $DIRNAME $BLOCKS_READ >>$TMP_SIZE_FILE
         fi

    done
    # For those directories needing to be added that were not found
    # as existing entries in the current size file, write new entries to
    # the temporary size file.
    TMP_NO_SAVE_DIRS=0
    while true
    do
       if [ ${DIR_SIZES[$TMP_NO_SAVE_DIRS]} -ne 0 ]
       then
           echo ${SAVE_DIRS[$TMP_NO_SAVE_DIRS]} ${DIR_SIZES[$TMP_NO_SAVE_DIRS]} >> $TMP_SIZE_FILE
       fi
       # exit from loop if at end of array.
       TMP_NO_SAVE_DIRS=TMP_NO_SAVE_DIRS+1
       if [ $TMP_NO_SAVE_DIRS -gt  $NO_SAVE_DIRS ]
       then
           break
       fi
    done

    # move temporary size file back to the permanent file
    mv $TMP_SIZE_FILE $SIZE_FILE
fi
exit 0
