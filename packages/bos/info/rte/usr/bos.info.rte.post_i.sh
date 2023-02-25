#!/usr/bin/ksh
# @(#)85	1.5  src/packages/bos/info/rte/usr/bos.info.rte.post_i.sh, pkginfo, pkg411, 9437A411a  9/12/94  11:07:41
#
#   COMPONENT_NAME: pkginfo
#
#   FUNCTIONS: 
#
#   ORIGINS: 27
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# If this is not a migration, exit.
if [ -z "$MIGSAVE" ]
then
  exit 0
fi

# If it is a 4.1 to 4.1 migration, copy ispaths from $MIGSAVE and exit.
# The other migration code here is for 3.2 to 4.1
if [ -n "$INSTALLED_LIST" ]
then
  # bos.info.rte is a 4.1 option.  If it's not in the INSTALLED_LIST, 
  # this is a 4.1->4.x install.
  grep -q "bos\.info\.rte" $INSTALLED_LIST
  if [ $? -eq 0 ]
  then
    # bos.info.rte found; must be 4.1.
    if [ -s $MIGSAVE/usr/lpp/info/data/ispaths -a ! -s /usr/lpp/info/data/ispaths ]
    then
      mv $MIGSAVE/usr/lpp/info/data/ispaths /usr/lpp/info/data
    fi
    exit 0
  fi
fi

# First we do migration that occurs whether databases are installed or not.
DEFAULT_INFO_DATA_DIR=/usr/lpp/info/data
MIG_INFO_DATA_DIR=/usr/lpp/info/data/32
SHARE_DATA_DIR=/usr/share/info/data

#
# Take care of things in /usr/share/info/data.
#
if [ -d $SHARE_DATA_DIR ]
then
   # Remove old things we know about that may have been in /usr/share/info/data
   # which are no longer in use.
   for old_file in comprops.def krsfonts.def krstfmac.def krstfnam.def
   do
      rm $SHARE_DATA_DIR/$old_file 2>/dev/null
      if [ $? -eq 0 ]
      then
         ODMDIR=/usr/share/lib/objrepos odmdelete -o inventory -q"loc0=$SHARE_DATA_DIR/$old_file" >/dev/null 2>&1
      fi
   done

   # Remove things in ODM from /usr/share/info/data which we've moved in
   # migration.
   for old_file in ispaths ispaths.full ispaths.min
   do
     ODMDIR=/usr/share/lib/objrepos odmdelete -o inventory -q"loc0=$SHARE_DATA_DIR/$old_file" >/dev/null 2>&1
   done

   # Copy old things that might still be needed from /usr/share/info/data
   # to /usr/lpp/info/data.  Then remove them from /usr/share/info/data.
   for old_file in PShead isofonts.def krsmfonts.def krsmprops.def
   do
      if [ ! -s $DEFAULT_INFO_DATA_DIR/$old_file ]
      then
        cp -p $SHARE_DATA_DIR/$old_file $DEFAULT_INFO_DATA_DIR
        if [ $? -eq 0 ]
        then
           rm $SHARE_DATA_DIR/$old_file
           ODMDIR=/usr/share/lib/objrepos odmdelete -o inventory -q"loc0=$SHARE_DATA_DIR/$old_file" >/dev/null 2>&1
        fi
      fi
   done

   # Create symbolic link from /usr/lpp/info/data to /usr/share/info/data
   # for every file/directory/link found in /usr/share/info/data
   listing=`ls $SHARE_DATA_DIR`
   for file in $listing
   do
      # If the file already exists in /usr/lpp/info/data, then do not
      # link it to what's in /usr/share/info/data
      if [ ! -s $DEFAULT_INFO_DATA_DIR/$file ]
      then
        ln -s $SHARE_DATA_DIR/$file  $DEFAULT_INFO_DATA_DIR/$file
      fi
   done

   # Remove any /usr/lpp/info/data/32 that may have been created.  This
   # directory is used for migration, so if the customer has something
   # named like this in /usr/share/info/data, it won't be migrated.
   if [ -L $MIG_INFO_DATA_DIR ]
   then
      rm -f $MIG_INFO_DATA_DIR
   fi
fi

# 
# Create /usr/lpp/info/data/32 directory to where 3.2 ispaths file
# symlink is created. 
#
# Note: migration will continue whether creation of /usr/lpp/info/data/32 
# or ispaths file is successful or not.
#
MIGSAVE_INFO_DATA_DIR=$MIGSAVE/$DEFAULT_INFO_DATA_DIR
if [ -s ${MIGSAVE_INFO_DATA_DIR}/ispaths -o -s ${MIGSAVE_INFO_DATA_DIR}/ispaths.min -o -s ${MIGSAVE_INFO_DATA_DIR}/ispaths.full ]
then
   mkdir ${MIG_INFO_DATA_DIR} > /dev/null 2>&1
   if [ $? -eq 0 ]
   then
#
#     Note: the ispaths file is copied instead of moved so that an original
#     copy will be in the $MIGSAVE directory if the customer needs it.
#
      if [ -s ${MIGSAVE_INFO_DATA_DIR}/ispaths ]
      then
         cp ${MIGSAVE_INFO_DATA_DIR}/ispaths ${MIG_INFO_DATA_DIR} > /dev/null 2>&1
         if [ $? -eq 0 ]
         then
            ln -s ${MIG_INFO_DATA_DIR}/ispaths ${DEFAULT_INFO_DATA_DIR}/ispaths
         fi
      fi

#
#     Migrate ispaths.min and ispaths.full too.
#
      if [ -s ${MIGSAVE_INFO_DATA_DIR}/ispaths.min ]
      then
         cp ${MIGSAVE_INFO_DATA_DIR}/ispaths.min ${MIG_INFO_DATA_DIR} > /dev/null 2>&1
      fi
      if [ -s ${MIGSAVE_INFO_DATA_DIR}/ispaths.full ]
      then
         cp ${MIGSAVE_INFO_DATA_DIR}/ispaths.full ${MIG_INFO_DATA_DIR} > /dev/null 2>&1
      fi
   fi
# 
#  Migrate sys, rom, and key file paths in the ispaths file to point
#  to /usr/lpp/info/32 or /usr/lpp/info/lib/32 directory
#
#  Note: migration of sys, rom, and key file paths is performed whether
#  there is any migration to /usr/lpp/info/32 or /usr/lpp/info/lib/32 
#  directory.
#
   for ispaths_file in ${MIG_INFO_DATA_DIR}/ispaths ${MIG_INFO_DATA_DIR}/ispaths.min ${MIG_INFO_DATA_DIR}/ispaths.full
   do
      if [ -s ${ispaths_file} ]
      then
#
#        If ispaths file is a symbolic link and points to ispaths.full or
#        ispaths.min, no ispaths file migration
#        is necessary at this point because ispaths.full or ispaths.min will 
#        be migrated. If the symbolic link points to any other file
#         then ispaths file migration will be performed.
#
#        Note: for symbolic link, just follow one level of link. Do not take
#        care of more than one level of link.
#
         if [ -L ${ispaths_file} ]
         then
            long_listing=`ls -l ${ispaths_file}`
            for link in ${long_listing}
            do
              :
            done
            relpath=${link#${MIG_INFO_DATA_DIR}/}
            if [ "${relpath}" = "ispaths.min" -o "${relpath}" = "ispaths.full" -o "${relpath}" = "ispaths" ]
            then
               continue
            else
               relpath=${link#./}
               if [ "${relpath}" = "ispaths.min" -o "${relpath}" = "ispaths.full" -o "${relpath}" = "ispaths" ]
               then
                  continue
               else
                  cd ${MIG_INFO_DATA_DIR}
                  ispaths_file=${link}
               fi
            fi
         fi

         enable_write=FALSE
         if [ ! -w ${ispaths_file} ]
         then
            chmod +w $ispaths_file
            enable_write=TRUE
         fi
      
#
#        if path contains %L, replace with 32/%L.
#
         sed "s/%L/32\/%L/g" ${ispaths_file} > ${ispaths_file}.conv
#
#        Note: cp and rm commands (instead of mv command) are used to
#        preserve the permission associated with the ispaths file
#
         cp ${ispaths_file}.conv ${ispaths_file}
         rm ${ispaths_file}.conv

         if [ "${enable_write}" = TRUE ]
         then
            chmod -w $ispaths_file
         fi
      fi
   done
fi

#
# If /usr/lpp/info/data/32/ispaths is empty or doesn't exist, copy
# ispaths.full or ispaths.min over it.
#
if [ ! -s $MIG_INFO_DATA_DIR/ispaths ]
then
   if [ -s $MIG_INFO_DATA_DIR/ispaths.min ]
   then
     cp $MIG_INFO_DATA_DIR/ispaths.min $MIG_INFO_DATA_DIR/ispaths
   else
     if [ -s $MIG_INFO_DATA_DIR/ispaths.full ]
     then
       cp $MIG_INFO_DATA_DIR/ispaths.full $MIG_INFO_DATA_DIR/ispaths
     fi
   fi
fi

#
# If the default ispaths file doesn't exist or is an empty link,
# copy the /usr/lpp/info/data/32/ispaths file to the default.
#
if [ ! -s $DEFAULT_INFO_DATA_DIR/ispaths ]
then
  if [ -L $DEFAULT_INFO_DATA_DIR/ispaths ]
  then
    rm $DEFAULT_INFO_DATA_DIR/ispaths
  fi
  cp $MIG_INFO_DATA_DIR/ispaths $DEFAULT_INFO_DATA_DIR
fi

#
# Create isprime file for default 3.2 libraries.
#
if [ -d $MIG_INFO_DATA_DIR ]
then
cat -> $MIG_INFO_DATA_DIR/isprime <<\EOF
1 Topic & Task Index
2 Commands
3 Books
4 Programming Reference
EOF
fi

#
# Move /usr/lpp/info/data/compilers to /usr/lpp/info/data/compiler32 and
# migrate sys, rom, and key file paths in 3.2.5 compilers ispaths.
# Also, adjust for 3.2.5 patch by changing "compilers" to "compilers32"
# in rom and key file paths.
#
if [ -s ${DEFAULT_INFO_DATA_DIR}/compilers ]
then
   mv ${DEFAULT_INFO_DATA_DIR}/compilers ${DEFAULT_INFO_DATA_DIR}/compilers32
   if [ $? -eq 0 ]
   then
      cp $MIGSAVE_INFO_DATA_DIR/compilers/ispaths $DEFAULT_INFO_DATA_DIR/compilers32
      ispaths_file="${DEFAULT_INFO_DATA_DIR}/compilers32/ispaths"
   else
      cp $MIGSAVE_INFO_DATA_DIR/compilers/ispaths $DEFAULT_INFO_DATA_DIR/compilers
      ispaths_file="${DEFAULT_INFO_DATA_DIR}/compilers/ispaths"
   fi

   if [ -f ${ispaths_file} ]
   then
      enable_write=FALSE
      if [ ! -w ${ispaths_file} ]
      then
         chmod +w ${ispaths_file}
         enable_write=TRUE
      fi

      # if path contains %L, replace with 32/%L.
      sed "s/%L/32\/%L/g" ${ispaths_file} > ${ispaths_file}.conv

      # if path contains compilers, replace with compilers32.
      sed "s/\/compilers\//\/compilers32\//g" ${ispaths_file}.conv > ${ispaths_file}

      rm -f ${ispaths_file}.conv

      if [ "${enable_write}" = TRUE ]
      then
         chmod -w $ispaths_file
      fi
   fi
fi 

# Now we do migration for databases.
#
# This list is extracted from the Appendix B of AIX 4 LPP and Option
# Name Registration document.  Put the En_US and en_US entries first
# since those are most likely to be there.
LANG_LIST="En_US en_US ar_AA Ar_AA bg_BG cs_CZ da_DK Da_DK de_CH De_CH de_DE De_DE el_GR en_GB En_GB es_ES Es_ES fi_FI Fi_FI fr_BE Fr_BE fr_CA Fr_CA fr_CH Fr_CH fr_FR Fr_FR hr_HR hu_HU is_IS Is_IS it_IT It_IT iw_IL Iw_IL ja_JP Ja_JP ko_KR mk_MK nl_BE Nl_BE nl_NL Nl_NL no_NO No_NO pl_PL pt_BR pt_PT Pt_PT ro_RO ru_RU si_SI sh_SP sk_SK sr_SP sv_SE Sv_SE tr_TR zh_TW"

DEFAULT_INFO_DIR=/usr/lpp/info
MIG_INFO_DIR=/usr/lpp/info/32
DEFAULT_INFO_LIB_DIR=/usr/lpp/info/lib
MIG_INFO_LIB_DIR=/usr/lpp/info/lib/32

# First check to see if there are any databases to be migrated.
LIBRARIES=no
for lang in $LANG_LIST
do
  if [ -d $DEFAULT_INFO_DIR/$lang -o -d $DEFAULT_INFO_LIB_DIR/$lang ]
  then
    LIBRARIES=yes
    break
  fi
done

# If no libraries were found, exit.  It's not an error, so exit 0.
if [ $LIBRARIES = no ]
then
  exit 0
fi

INFO_COUNT=0
INFO_LIB_COUNT=0

#
# Create /usr/lpp/info/32 directory to where 3.2 LANG directories 
# or symbolic links in /usr/lpp/info will migrate
#
mkdir ${MIG_INFO_DIR} > /dev/null 2>&1
if [ $? -ne 0 ] 
then
   echo "Unable to create $MIG_INFO_DIR to migrate 3.2 libraries"
   exit -1
fi

#
# Create /usr/lpp/info/lib/32 directory to where 3.2 LANG directories
# or symbolic links in /usr/lpp/info/lib will migrate
#
if [ -d ${DEFAULT_INFO_LIB_DIR} ]
then
   mkdir ${MIG_INFO_LIB_DIR} > /dev/null 2>&1
   if [ $? -ne 0 ] 
   then
      rm -rf ${MIG_INFO_DIR}
      echo "Unable to create $MIG_INFO_LIB_DIR to migrate 3.2 libraries"
      exit -1
   fi
fi

#
# For any LANG directory or symbolic link found in /usr/lpp/info,
# move the directory or symbol link to /usr/lpp/info/32 and
# create a symbol link from /usr/lpp/info/LANG to /usr/lpp/info/32/LANG
#
# Note: The symbol link created from /usr/lpp/info/LANG to 
# /usr/lpp/info/32/LANG is mainly for 3.2 SMIT
#
# For any LANG directory or symbolic link found in /usr/lpp/info/lib,
# move the directory or symbol link to /usr/lpp/info/lib/32;
#
for lang in ${LANG_LIST}
do
   if [ -d ${DEFAULT_INFO_DIR}/${lang} -o -L ${DEFAULT_INFO_DIR}/${lang} ]
   then
      mv ${DEFAULT_INFO_DIR}/${lang} ${MIG_INFO_DIR}/${lang}
      if [ $? -eq 0 ]
      then
         ln -s ${MIG_INFO_DIR}/${lang} ${DEFAULT_INFO_DIR}/${lang}
         if [ $? -eq 0 ]
         then
            INFO_COUNT=`expr $INFO_COUNT + 1`
         fi
      fi
   fi
   if [ -d ${DEFAULT_INFO_LIB_DIR}/${lang} -o -L ${DEFAULT_INFO_LIB_DIR}/${lang} ]
   then
      # Do NOT move $DEFAULT_INFO_LIB_DIR/en_US because we installed
      # 4.1 stuff there.
      if [ $lang != en_US ]
      then
        mv ${DEFAULT_INFO_LIB_DIR}/${lang} ${MIG_INFO_LIB_DIR}/${lang}
        if [ $? -eq 0 ]
        then
           INFO_LIB_COUNT=`expr $INFO_LIB_COUNT + 1`
        fi
      else
        if [ "`ls -R $DEFAULT_INFO_LIB_DIR/en_US`" != "sys.sys" ]
        then
          # More than what we installed is in the en_US directory.
          mkdir $MIG_INFO_LIB_DIR/en_US
          for i in `ls $DEFAULT_INFO_LIB_DIR/en_US`
          do
            if [ $i != sys.sys ]
            then
              mv $i $MIG_INFO_LIB_DIR/en_US
            fi
          done
        fi
      fi
   fi
done

#
# For any symbolic link found in /usr/lpp/info/lib/32, 
# if the symbolic link points to /usr/lpp/info/lib/xxx, 
# relink to /usr/lpp/info/lib/32/xxx;
# if the symbol link points to /usr/lpp/info/xxx,
# keep the symbolic link as it is,
# otherwise move the symbolic link entry back to /usr/lpp/info/lib
#
if [ $INFO_LIB_COUNT -gt 0 ]
then
   for dir_name in `ls ${MIG_INFO_LIB_DIR}`
   do
      if [ -L ${MIG_INFO_LIB_DIR}/${dir_name} ]
      then
         long_listing=`ls -l ${MIG_INFO_LIB_DIR}/${dir_name}`
#
#        Get the entry where it links to
#
#        Note: The entry where it links to is the last field in the
#        long listing.
#
         for link_to in ${long_listing}
         do
           :
         done

         relpath=${link_to#${DEFAULT_INFO_LIB_DIR}/}
         if [ -d ${MIG_INFO_LIB_DIR}/${relpath} -o -L ${MIG_INFO_LIB_DIR}/${relpath} ]
         then
            rm -f ${MIG_INFO_LIB_DIR}/${dir_name}
            ln -s ${MIG_INFO_LIB_DIR}/${relpath} ${MIG_INFO_LIB_DIR}/${dir_name}
         else
            relpath=${link_to#${DEFAULT_INFO_DIR}/}
            if [ -d ${DEFAULT_INFO_DIR}/${relpath} -o -L ${DEFAULT_INFO_DIR}/${relpath} ]
            then
               :
            else
               mv ${MIG_INFO_LIB_DIR}/${dir_name} ${DEFAULT_INFO_LIB_DIR}/${dir_name}
            fi
         fi
      fi
   done
#
#  If the en_US entry is not found in /usr/lpp/info/lib/32,
#  create a symbolic link entry.
#
#  Note: this link is created because default/shipped lang in 4.1 is ISO (en_US)
#  and path entries for sys, rom, and key files in ispaths may point to
#  /usr/lpp/info/lib/%L/...
#
   if [ ! \( -L ${MIG_INFO_LIB_DIR}/en_US -o -d ${MIG_INFO_LIB_DIR}/en_US \) ]
   then
      ln -s ${MIG_INFO_LIB_DIR}/En_US ${MIG_INFO_LIB_DIR}/en_US
   fi
fi

#
#  If the en_US entry is not found in /usr/lpp/info, 
#  create a symbolic link entry.
#
#  Note: this link is created because default/shipped lang in 4.1 is ISO,
#  3.2 SMIT looks for aixmin/aix databases under /usr/lpp/info/%L, and
#  path entries for sys, rom, and key files in ispaths may point to
#  /usr/lpp/info/%L/...
#
if [ $INFO_COUNT -gt 0 -a ! -L ${DEFAULT_INFO_DIR}/en_US ]
then
   ln -s ${MIG_INFO_DIR}/En_US ${DEFAULT_INFO_DIR}/en_US
fi

#
#  If the en_US entry is not found in /usr/lpp/info/32, 
#  create a symbolic link entry.
#
#  Note: this link is created because default/shipped lang in 4.1 is ISO,
#  invoking "info -l 32" may not work if this isn't done.
#
if [ ! -s $MIG_INFO_DIR/en_US ]
then
   ln -s $MIG_INFO_DIR/En_US $MIG_INFO_DIR/en_US
fi

exit 0
