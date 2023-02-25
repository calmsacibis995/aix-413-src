#!/usr/bin/ksh
# @(#)48        1.6  src/packages/X11/Dt/rte/root/X11.Dt.rte.pre_rm.sh, pkgxcde, pkg41J, 9517B_all 4/28/95 08:45:19
#
#   COMPONENT_NAME: pkgxcde
#
#   FUNCTIONS: pre_rm
#
#   ORIGINS: 27
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# add_one_dir: adds a directory entry to the array
# Before adding the new entry, entries from the existing size
# file are read into the array.
# $1 = directory
# $2 = amount of permanent space
# $3 = amount of temporary space
function add_one_dir
{
  integer NO_ADD
  if [ "$2" != "0" ] || [ "$3" != "0" ]
  then

      # If entries from existing size file have not been read, then read
      # the file entries into the array.
      if [ NO_SAVE_DIRS -eq 0 ]
      then
         cat $SIZE_FILE | while read -r SAVE_DIRS[NO_SAVE_DIRS] DIR_SIZES[NO_SAVE_DIRS] DIR_TMP_SIZES[NO_SAVE_DIRS]
         do
             if [ -z "${DIR_TMP_SIZES[NO_SAVE_DIRS]}" ]
             then
                DIR_TMP_SIZES[NO_SAVE_DIRS]=0
             fi
             NO_SAVE_DIRS=NO_SAVE_DIRS+1
         done
      fi
      TMP_NO_SAVE_DIRS=0
      NO_ADD=0
      while true
      do
         # if the directory is already in the array, then
         # update its size values if necessary.
         if [ "$1" = "${SAVE_DIRS[TMP_NO_SAVE_DIRS]}" ]
         then
             NO_ADD=1
             if [ ${DIR_SIZES[TMP_NO_SAVE_DIRS]} -lt $2 ]
             then
                DIR_SIZES[NO_SAVE_DIRS]=$2
                SIZE_FILE_CHANGED=1
             fi
             if [ ${DIR_TMP_SIZES[TMP_NO_SAVE_DIRS]} -lt $3 ]
             then
                DIR_TMP_SIZES[NO_SAVE_DIRS]=$3
                SIZE_FILE_CHANGED=1
             fi
             break

         fi
         TMP_NO_SAVE_DIRS=TMP_NO_SAVE_DIRS+1
         # exit from loop if at end of array.
         if [ TMP_NO_SAVE_DIRS -ge NO_SAVE_DIRS ]
         then
             break
         fi
      done
      if [ NO_ADD -eq 0 ]
      then
         # if an existing entry was not updated, then add a new
         # entry to the array.
         SAVE_DIRS[NO_SAVE_DIRS]=$1
         DIR_SIZES[NO_SAVE_DIRS]=$2
         DIR_TMP_SIZES[NO_SAVE_DIRS]=$3
         NO_SAVE_DIRS=NO_SAVE_DIRS+1
         SIZE_FILE_CHANGED=1
      fi
  fi
}
# add_dirs: adds directory entries above specified directory to the array
# $1 = directory
function add_dirs
{
  TMPDIR=$1
  while true
  do
     TMPDIR=`dirname $TMPDIR`
     if [ "$TMPDIR" = "/" ]
     then
         break
     else
         add_one_dir $TMPDIR 8 0
     fi
  done
}
# Since this script is called when MIGSAVE is not
# set, a local value for the save directory needs to be set.
# If the system location for save files during install changes
# in the future, then this value also needs to change.
LOCAL_MIGSAVE="/lpp/save.config"
SIZE_FILE="/usr/lpp/X11.Dt/X11.Dt.rte.size"
TMP_SIZE_FILE="/tmp/X11.Dt.rte.size_PRE_RM.TEMP"

integer DIR_SIZES NO_SAVE_DIRS TMP_NO_SAVE_DIRS TMP_BLOCKS_READ BLOCKS_READ
integer SIZE_FILE_CHANGED
SIZE_FILE_CHANGED=0
NO_SAVE_DIRS=0

# Calculate the amount of space that will be required to
# save copies of all files in /etc/config and all its subdirectories into the
# save directory
DIRECTORY="/etc/dt/config"
if [ -d "$DIRECTORY" ]
then
   integer NUMBER_OF_BLOCKS TOTAL_BLOCKS
   TOTAL_BLOCKS=0
   du $DIRECTORY | while read -r NUMBER_OF_BLOCKS SPACE_DIR
   do
       # Since du returns total space in all subdirectories as part
       # of space for highest directory, need
       # to keep track of how much space is really in the subdirectories
       # and then subtract that out from the space given for the
       # highest directory.
       # du always returns space in highest directory last so these
       # checks will work properly.
       if [ "$SPACE_DIR" != "$DIRECTORY" ]
       then
          TOTAL_BLOCKS=TOTAL_BLOCKS+NUMBER_OF_BLOCKS
       else
          NUMBER_OF_BLOCKS=NUMBER_OF_BLOCKS-TOTAL_BLOCKS
       fi
       SAVE_DIR=$LOCAL_MIGSAVE$SPACE_DIR
       # If additional save space is needed, then
       # add directories and sizes to array.
       if [ NUMBER_OF_BLOCKS -ne 0 ]
       then
          # determine subdirectories above save directory for which
          # mimimum space (i.e. 8 512-bytes blocks) is required to
          # create the directory.
          add_dirs "$LOCAL_MIGSAVE$DIRECTORY"

          # add the save directory to the list of directories needing space
          add_one_dir $SAVE_DIR $NUMBER_OF_BLOCKS 0
       fi
   done
fi
#
# Action definition files will be migrated
# regardless of whether the user is migrating
# from an old version of the desktop to a new version or not.
# So, calculate the amount of space that will be required in
# /tmp to convert the largest file, the amount of space in
# /etc/dt/appconfig/types and its subdirectories for converted files,
# and the amount of space in the save directory to backup copies of the
# converted files.
#

integer LARGEST_FILE ORIG_4K ADD_BYTES TMP_BYTES
integer CONVERTED_4K OLD_FILE_512
integer OLD_FILE_BYTES NEW_FILE_BYTES
integer OLD_FILE_4K NEW_FILE_4K BLOCKS_512
LARGEST_FILE=0
DIRECTORY="/etc/dt/appconfig/types"
if [ -d "$DIRECTORY" ]
then
     for ACTION_FOLDER in `find $DIRECTORY -type d  -print`
     do
           ORIG_4K=0
           CONVERTED_4K=0
           if [ "$ACTION_FOLDER" != "$DIRECTORY" ]
           then
              for ACTION_FILE in `find $ACTION_FOLDER -name '*.dt' -print`
              do
                 ADD_BYTES=`/usr/bin/grep -c -E "DesktopTools" \
                     $ACTION_FILE 2>/dev/null`
                 /usr/bin/grep -E "DT_ARG0_VALUE" \
                 $ACTION_FILE 2>/dev/null | while read -r GREPLINE
                 do
                     if [ ! -z "$GREPLINE" ]
                     then
                         TMP_BYTES=${#GREPLINE}
                         ADD_BYTES=ADD_BYTES+(TMP_BYTES*9)+751
                     fi
                 done
                 /usr/bin/grep -E "LOCK_DISPLAY|\
EXIT_SESSION|RELOAD_TYPES_DB|SHOW_DIRECTORY|SHOW_TOOLS|\
REMOVE_TRASH|SHOW_TRASH|OPEN_FILE" $ACTION_FILE 2>/dev/null | \
                 while read -r GREPLINE
                 do
                     if [ ! -z "$GREPLINE" ]
                     then
                         TMP_BYTES=${#GREPLINE}
                         ADD_BYTES=ADD_BYTES+6*(TMP_BYTES+73)
                     fi
                 done
                 /usr/bin/grep -E "TRASH_DIR|\
PARENT_DIR|CURRENT_DIRECTORY|DOT_DIR|DIRECTORY_LOCK|\
DIRECTORY" $ACTION_FILE 2>/dev/null | \
                 while read -r GREPLINE
                 do
                     if [ ! -z "$GREPLINE" ]
                     then
                         TMP_BYTES=${#GREPLINE}
                         # this space is to account for adding
                         # ",Print" (add 6 chars) to ACTIONS field line,
                         # changing name (add max 3 for DIR->FOLDER)
                         # adding MEDIA statement (add 14 chars),
                         # and changing ICON field line (add 1 char)
                         # plus possible additional white space
                         # on four changed lines
                         ADD_BYTES=ADD_BYTES+24+4*(TMP_BYTES)
                     fi
                 done
                 /usr/bin/grep -E "TRASH_DIR|DIRECTORY_LOCK1" \
$ACTION_FILE 2>/dev/null | \
                 while read -r GREPLINE
                 do
                     if [ ! -z "$GREPLINE" ]
                     then
                         TMP_BYTES=${#GREPLINE}
                         # this space is to account for
                         # updating PATH_PATTERN or
                         # or MODE field lines.
                         # This may cause additional white space
                         # on changed line.
                         ADD_BYTES=ADD_BYTES+TMP_BYTES
                     fi
                 done
                 /usr/bin/grep -E "TRASH_DIR1|\
PARENT_DIR1|CURRENT_DIRECTORY1|DOT_DIR1|DIRECTORY_LOCK1|\
DIRECTORY1" $ACTION_FILE 2>/dev/null | \
                 while read -r GREPLINE
                 do
                     if [ ! -z "$GREPLINE" ]
                     then
                         TMP_BYTES=${#GREPLINE}
                         # this space is to account for changing
                         # name (add maximum of 3 for DIR->FOLDER)
                         # plus possible additional white space
                         # on changed line
                         ADD_BYTES=ADD_BYTES+TMP_BYTES+3
                     fi
                 done
                 if [ ADD_BYTES -ne 0 ]
                 then
                     TMP=`du -a $ACTION_FILE`
                     OLD_FILE_512=${TMP%%?/*}
                     OLD_FILE_BYTES=OLD_FILE_512*512
                     NEW_FILE_BYTES=OLD_FILE_BYTES+ADD_BYTES
                     # calculate number of 4K blocks in old file
                     OLD_FILE_4K=OLD_FILE_BYTES/4096
                     TMP_BYTES=OLD_FILE_4K*4096
                     if [ OLD_FILE_BYTES -ne TMP_BYTES ]
                     then
                          OLD_FILE_4K=OLD_FILE_4K+1
                     fi
                     # calculate number of additional 4K blocks in new file
                     NEW_FILE_4K=NEW_FILE_BYTES/4096
                     TMP_BYTES=NEW_FILE_4K*4096
                     if [ NEW_FILE_BYTES -ne TMP_BYTES ]
                     then
                          NEW_FILE_4K=NEW_FILE_4K+1
                     fi
                     CONVERTED_4K=CONVERTED_4K+(NEW_FILE_4K-OLD_FILE_4K)
                     ORIG_4K=ORIG_4K+OLD_FILE_4K
                     if [ NEW_FILE_4K -gt LARGEST_FILE ]
                     then
                         LARGEST_FILE=NEW_FILE_4K
                     fi
                 fi
              done
              if [ CONVERTED_4K -ne 0 ]
              then
                 SAVE_DIR=$LOCAL_MIGSAVE$ACTION_FOLDER
                 # determine subdirectories above save directory for which
                 # mimimum space (i.e. 8 512-bytes blocks) is required to
                 # create the directory.
                 add_dirs "$SAVE_DIR"

                 # add the number of blocks required to save all files in
                 # this directory in the size file for this fileset.
                 BLOCKS_512=$ORIG_4K*8
                 add_one_dir $SAVE_DIR $BLOCKS_512 0

                 # add the original directory to the list of directories
                 # needing additional space
                 BLOCKS_512=CONVERTED_4K*8
                 add_one_dir $ACTION_FOLDER $BLOCKS_512 0
              fi
           fi
     done
fi
# if size file needs updating. then ...
if [ SIZE_FILE_CHANGED -eq 1 ]
then
   # compute number of 512 blocks in largest file to be
   # saved in /tmp
   LARGEST_FILE=LARGEST_FILE*8
   add_one_dir "/tmp" $LARGEST_FILE 0
   NO_SAVE_DIRS=NO_SAVE_DIRS-1

   # remove current copy of temporary size file
   rm $TMP_SIZE_FILE >/dev/null 2>/dev/null

   # Write out new version of size file
   TMP_NO_SAVE_DIRS=0
   while true
   do
      if [ ${DIR_SIZES[TMP_NO_SAVE_DIRS]} -ne 0 ]
      then

          if [ ${DIR_TMP_SIZES[TMP_NO_SAVE_DIRS]} -eq 0 ]
          then
             echo ${SAVE_DIRS[TMP_NO_SAVE_DIRS]} ${DIR_SIZES[TMP_NO_SAVE_DIRS]} >> $TMP_SIZE_FILE
          else
             echo ${SAVE_DIRS[TMP_NO_SAVE_DIRS]} ${DIR_SIZES[TMP_NO_SAVE_DIRS]} ${DIR_TMP_SIZES[TMP_NO_SAVE_DIRS]} >> $TMP_SIZE_FILE
          fi
      fi
      # exit from loop if at end of array.
      TMP_NO_SAVE_DIRS=TMP_NO_SAVE_DIRS+1
      if [ TMP_NO_SAVE_DIRS -gt NO_SAVE_DIRS ]
      then
          break
      fi
   done

   # move temporary size file back to the permanent size file
   mv $TMP_SIZE_FILE $SIZE_FILE

fi
exit 0

