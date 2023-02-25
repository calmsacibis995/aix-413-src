#!/bin/ksh
#@(#)23       1.3  src/bos/usr/sbin/install/migrate/merge_swvpds.sh, cmdinstl, bos41J, 9519B_all 5/11/95 10:50:06 
#
#   COMPONENT_NAME: CMDINSTL
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# USAGE:
#   merge_swvpds 
#
# ARGUMENTS:  None
#
# RETURN VALUE DESCRIPTION:
#   0 -- if everything succeeded
#
#   1 -- if there was a failure but we were able to copy
#        the original 41 vpds back 
#  
#   2 -- If a fatal error occurs 
#
# Restrictions:
#
# Known limitations:
#


#------------------------------------------------------------------------
# Function:     recover 
# Remarks:      Call restodms, and exit with soft_err 
# Arguments:    None
# Return code:  exits is soft_err 
#
#------------------------------------------------------------------------
function recover 
{
   restodms $VPD_BASE/$SAVEDIR_41
   soft_err
}
 
#------------------------------------------------------------------------
# Function:     soft_trap 
# Remarks:      simple exit 
# Arguments:    None
# Return code:  exits with PASS 
#
#------------------------------------------------------------------------
function soft_trap 
{
   sync
   sync
   exit ${PASS}
}


#------------------------------------------------------------------------
# Function:     soft_err 
# Remarks:      Original vpd is still intact
#               Can do preservation install 
# Arguments:    None 
# Return code:  1 
#
#------------------------------------------------------------------------
function soft_err
{
   sync
   sync
   print "\n${CMD}: Error in conversion routines" 1>&2
   print "${CMD}: Original vpd is still intact" 1>&2
   # Don't need the saved vpd
   rm -rf ${VPD_BASE}/${SAVEDIR_41}  
   exit $SOFTERR 
}

#------------------------------------------------------------------------
# Function:     fatal_err
# Remarks:      Original vpd is not intact
#               Cannot continue with migration
# Arguments:    None
# Return code:  1 
#
#------------------------------------------------------------------------
function fatal_err
{
   print "\n${CMD}: Error in conversion routines" 1>&2
   print "Can not continue with migration" 1>&2
   exit $FATAL
}

#-----------------------------------------------------------------------------
#
#     Program:  saveodms
#     Purpose:  save the history, product, and lpp databases for future use.
#               The saved vpd can be restored by restodms
#
#     Usage:    saveodms savedir
#                 where savedir is the directory under which the vpd tree
#                 is created, and the vpd is saved. For e.g. saveodms myvpd
#                 causes odms to be saved under /u/myvpd.odm/....
#                 To restore this use restodms myvpd
#                 If no savedir is specified, the default id orig is used.
#
#      Note:    1. It makes sure that we have enough space in /u to succeed.
#               2. Time of save operation is left in the save directory in the
#                  file timestamp.
#               3. If the savedir exists, it is moved to savedir.old
#
#
#-----------------------------------------------------------------------------
#
function saveodms
{
   BASEDIR=$1

   #
   #  Expand /tmp if necessary
   #

SAVEFS=/tmp             # File system where VPD will be saved
integer tot_space=0     # Total space in KB used by VPD

ODMDIR_LIST="/etc/objrepos  /usr/lib/objrepos /usr/share/lib/objrepos"
FILE_LIST="history history.vc inventory inventory.vc lpp lpp.vc product 
           product.vc"

# Ensure that the target exists
#
for i in $ODMDIR_LIST
do
  for j in $FILE_LIST
  do
     [ ! -d $i/$j ]  || soft_err 
  done
done

# figure out space requirements
#
OLD_PWD=`pwd`
for dir in $ODMDIR_LIST
do
     cd $dir  || soft_err
     let tot_space=` du -a $FILE_LIST | \
                     awk ' BEGIN {tot=0}; {tot=tot+$1}; END{print tot}' `
done

cd $OLD_PWD || soft_err

let free=`df $SAVEFS 2>/dev/null | awk 'NR==2 {print $3}'`
if [ $tot_space -gt $free ]; then
   let need=$tot_space-$free
   let need=$need*2
   chfs -a size=+$need $SAVEFS
   if [ $? -ne 0 ]; then    # destroy the .old directory
       print "${CMD}: Not enough space in $SAVEFS to succeed" 1>&2
       soft_err 
   fi
fi

# Create the tree under savedir
#
rm -rf $BASEDIR
mkdir -p $BASEDIR/etc/objrepos $BASEDIR/usr/lib/objrepos \
         $BASEDIR/usr/share/lib/objrepos >/dev/null 2>&1 || soft_err 

# Copy the vpd
#
for i in $ODMDIR_LIST
do
  for j in $FILE_LIST
  do
     cp $i/$j  $BASEDIR/$i/$j || soft_err 
  done
done

   return ${PASS} 

}

#
#-----------------------------------------------------------------------------
#
#     Program:  restodms
#     Purpose:  restores the history, product, and lpp databases that were
#               saved by saveodms
#     Usage:    restodms savedir
#                 where savedir is the directory under which the vpd tree
#                 is saved. For e.g. saveodms /tmp/bos/myvpd
#                 causes odms to be saved under /tmp/bos/myvpd
#                 To restore this use restodms  /tmp/bos/myvpd
#
#      Note:    1. It does not make sure that we have enough space in the
#                  relevant filesystems to succeed because we are just
#                  copying back the original files.
#
#-----------------------------------------------------------------------------
#

function restodms
{

  BASEDIR=$1        # User specified directory
  if [ ! -d $BASEDIR ]; then
    print "\n${CMD}: $BASEDIR does not exist" 1>&2
    fatal_err 
  fi

# Copy the vpd
#
for i in $ODMDIR_LIST
do
  for j in $FILE_LIST
  do
     cp -p $BASEDIR/$i/$j  $i/$j || fatal_err 
  done
done

   return ${PASS} 
}


# --------------------------- main () --------------------------------


cmdname=`basename ${0}`           # argv[0]
VPD_BASE="/tmp/bos"
VPD_OFFSET="/usr/lib/objrepos /etc/objrepos /usr/share/lib/objrepos"
VPDS="product lpp history inventory"
ODMADD="/usr/bin/odmadd"
SAVEDIR_41="41.orig"
PATH=/usr/sbin:/usr/bin:/usr/lpp/bos/migrate:/usr/lib/instl:/bin:/etc:$PATH
src_vpd=
target_vpd=
typeset -i FATAL=2 \
           SOFTERR=1 \
           PASS=0

   CMD=`basename $0`
#------------------------------------------------------------------------
# Messages.
# initialize message_stuff
#------------------------------------------------------------------------

#------------------------------------------------------------------------
# Default Messages.
#------------------------------------------------------------------------


   
   if [ ! -d ${VPD_BASE} ] ;  then
      print "\n${CMD}: ${VPD_BASE} does not exist"  1>&2
      soft_err
   fi

   print "\n${CMD}: Merging vital product databases. Please wait." 1>&2

   saveodms $VPD_BASE/$SAVEDIR_41   || soft_err

   trap recover 1 2 3 4 6 7 8 10 11 12 15 29


   for db in ${VPD_OFFSET}
   do

      if [ ! -d ${VPD_BASE}${db} ] ;  then
	 if [ "${db}" != "/usr/share/lib/objrepos" ]
	 then
             print "\n${CMD}: ${VPD_BASE}${db} does not exist" 1>&2 
             recover
	 else
	     # A missing share part is better than losing the merged
	     # usr and root parts, so continue with the VPD merge
	     continue
	 fi
      fi

      convert_swvpd ${VPD_BASE}${db}  ${db} || {\
	     if [ "${db}" != "/usr/share/lib/objrepos" ]
	     then
                 recover
	     fi
      }

   done

   trap soft_trap 1 2 3 4 6 7 8 10 11 12 15 29
   
   # Don't need the saved vpd
   rm -rf ${VPD_BASE}/${SAVEDIR_41}  

   # get rid of source vpds too
   #
   for db in ${VPD_OFFSET}
   do
      if [ -d ${VPD_BASE}${db} ]
      then
          cd ${VPD_BASE}${db} 
          rm -rf product product.vc lpp lpp.vc history history.vc \
      	         inventory inventory.vc fix fix.vc
      fi
   done

   exit ${PASS}
