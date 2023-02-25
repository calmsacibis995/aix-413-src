#!/bin/ksh
# @(#)86	1.58  src/packages/bos/rte/usr/bos.rte.pre_i.sh, pkgbos, pkg41J, 9518A_all 4/26/95 14:36:57
#
#   COMPONENT_NAME: pkgbos
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# Note:  bos.rte.pre_i is only called during the migration path of BOS install.

# /dev/hd4 is mounted over /mnt
# /dev/hd2 is mounted over /mnt/usr
# /dev/hd9var is mounted over /mnt/var
# /dev/<tmplv> (usually hd3) is mounted over /mnt/tmp
# This script also depends on /mmt/tmp to have been cleared out by the calling
# routine.

function Breakpt
{
exec >/dev/console 2>&1
while read -r cmd?"Enter command>> "
do
        eval "$cmd"
done
}

#
# Restoring the saved boot images failed, so spit out an error message and
# remove the boot images, since they're not useful anymore
#
function FailedBootRestore
{
	dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 80 \
'The previous boot image could not be recovered.  Reboot the system in\n\
maintenance mode and use the bosboot command to restore the boot image.'

	BOOTSVDIR=/mnt/usr/lib/boot/iplrom.boot
	rm -f $BOOTSVDIR/bootimage.save*

	return 0
}

#
# If IPLROM emulation was written to the boot disk, then we need to recover
# the previously saved boot record and boot image
#
function RestoreBootImage
{
	# Determine the boot disk
	BLVDISK=`lslv -l hd5 | grep hdisk | head -1 | cut -d' ' -f1`

	# Check to see if we need to restore the bootlist
	if [[ -f /mnt/usr/lib/boot/bootlist.altered ]]
	then
	    bootlist -m normal ${BLVDISK}
	    rm -f /mnt/usr/lib/boot/bootlist.altered 
	fi

	# Check to see if we have to restore a boot image which was replaced
	# by IPLROM emulation
	BOOTSVDIR=/mnt/usr/lib/boot/iplrom.boot
	if [ ! -f $BOOTSVDIR/bootrec.save ] || 
	   [ ! -f $BOOTSVDIR/bootimage.save -a ! -f $BOOTSVDIR/bootimage.save.Z ]
	then
		return 0
	fi
	dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 81 \
'Recovering previous boot image...'

	# Get the length and offset from the boot record
	boot_offset=`od -t u -j 48 -w4 -N4 $BOOTSVDIR/bootrec.save | 
			awk 'NF==2 {print $2 + 0}'`
	boot_length=`od -t u -j 36 -w4 -N4 $BOOTSVDIR/bootrec.save | 
			awk 'NF==2 {print $2 + 0}'`
	if [ -z "$boot_offset" ] || [ -z "$boot_length" ]
	then
	    FailedBootRestore && return 1
	fi

	# Restore the boot record
	dd if=$BOOTSVDIR/bootrec.save of=/dev/$BLVDISK bs=512 count=1 conv=sync || return 1
	rm -f $BOOTSVDIR/bootrec.save

	# Restore the image, which may have been compressed
	if [ -f $BOOTSVDIR/bootimage.save.Z ]
	then
	    zcat $BOOTSVDIR/bootimage.save.Z |
	       dd of=/dev/$BLVDISK bs=512 count=$boot_length seek=$boot_length conv=sync || FailedBootRestore && return 1
	else
	       dd if=$BOOTSVDIR/bootimage.save of=/dev/$BLVDISK bs=512 count=$boot_length seek=$boot_length conv=sync || FailedBootRestore && return 1
	fi
	
	# Restore the bootlist which was altered for network boot in normal
	bootlist -m normal $BLVDISK || FailedBootRestore && return 1

	# Remove the saved boot image
	rm -f $BOOTSVDIR/bootimage.save* 

	echo
	return 0
}

function Reboot
{
        cd /../
        PROMPT=`/usr/lpp/bosinst/bidata -b -g control_flow -f PROMPT`

	    dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 68 'The migration cannot continue.\n\
To reboot the system, turn the key to the NORMAL position and\n\
press Enter.  To reboot from other media and select another\n\
install method, turn the key to the SERVICE position and press Enter:\n'
        if [ "$PROMPT" != "no" ]
        then
		    read ans
        fi

	# Check to see if we have to restore a boot image which was replaced
	# by IPLROM emulation
	RestoreBootImage

	export LIBPATH=/usr/ccs/lib:/usr/lib
	sync;sync;sync
        if [ $BOOTTYPE -ne $NETWORK ]
	then
	    cp -p /mnt/usr/sbin/reboot /usr/sbin/reboot >/dev/null 2>&1
	    if [ $? != 0 ]
	    then
	       rm -f /usr/sbin/reboot >/dev/null 2>&1
	       cp -p /mnt/usr/sbin/reboot /../tmp/reboot >/dev/null 2>&1
	       PATH=$PATH:/../tmp
	    fi
	    /usr/sbin/umount all >/dev/null 2>&1
	    reboot -q
	else
	    cp -p /usr/sbin/reboot /../tmp/reboot >/dev/null 2>&1
	    cp -p /usr/sbin/umount /../tmp/umount >/dev/null 2>&1
	    nimclient -S shutdown
	    if [ $# -ne 0 ]
	    then
   	        nimclient -o change -a force=yes -a ignore_lock=yes -a \
            		info="$1"
	    fi
	    nimclient -R failure
	    /../tmp/umount all >/dev/null 2>&1
	    /../tmp/reboot -q
	 fi
}

# 
# pageit:  Display a file, 20 lines at a time, prompting after each 20 lines
#
# Usage:  pageit filename
function pageit
{
    if [ $# = 0 ]
    then
       return 1
    fi

    cat $1 | (
    i=0;
    while [ $i -lt 24 ]
    do
        line || exit 0
        (( i=i+1 ))
        if [ $i -eq 21 ]
        then
           echo 
	   dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 1 117 \
">>> 0 Continue\n"
	   dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 1 17 \
"   99 Previous Menu"
           echo 
	   dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 1 118 \
">>> Choice[0] :"
           read ans < /dev/console
           if [ -n "$ans" ] && [ $ans = "99" ]
           then
              exit 1
           fi
           i=0
        fi
    done;
    exit 0)
    return $?
}


# 
# Migration_Help:  Display help screen.
#
# Usage:  Migration_Menu
function Migration_Help
{
  dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 3 23 \
"                  Help for Migration Confirmation\n\n\
The Migration Confirmation screen displays information regarding file \n\
movement and removal, fileset removal, and potential file system expansion.\n\
\n\
To continue with the migration installation, press Enter.\n\
\n\
Type number 1 and press Enter to see which base system configuration files\n\
will be stored in /tmp/bos and not automatically merged into the system.\n\
\n\
Type number 2 and press Enter to see which filesets (installable options)\n\
will be removed from the system and will not be replaced with new filesets.\n\
Base message filesets in the list may be replaced depending on the \n\
language selection.\n\
\n\
Type number 3 and press Enter to see which directories will have the\n\
current contents removed during the migration installation.\n\
\n\
Type number 4 and press Enter to cancel the migration installation.\n\
Place the switch in the service position to reboot from the installation\n\
media or place the switch in the normal position to reboot your system.\n\
\n\
>>> 99  Previous Menu\n\n\
>>> Choice[99]: \
"
read reply
echo >/dev/console
}

# 
# Migration_Menu:  Display information screen to the user and ask if user
#                  wishes to continue.  If answer is no, BOS installation
#                  halts, and the user must reboot.
# Usage:  Migration_Menu
function Migration_Menu
{
	if [ "$SIZE_NEEDED_FOR_FILES" -gt "$TMP_SPACE" ]
	then
		INCREASE_TMP_WARNING="`dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 63  'WARNING: /tmp is currently %s 1024-byte blocks and will be increased to\\n\
              %s 1024-byte blocks.' $FREE_TMP $SIZE_NEEDED_FOR_FILES`"
		INCREASE_TMP_WARNING="$TCB_MSG"$INCREASE_TMP_WARNING
	else
		if [ -n "$TCB_MSG" ]
		then
		   INCREASE_TMP_WARNING=$TCB_MSG
		else
		   INCREASE_TMP_WARNING='

'
		fi
	fi

	DONE=
	while [ "$NOT" "$DONE" ]
	do
		# If get here, we are okay and the user can select to continue
		# with the migration.  Display the confirmation menu and prompt
		# user.
		echo "$CLEAR\c"
		dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 62 "                           Migration Confirmation\n\n\
  Either type 0 and press Enter to continue the installation, or type the\n\
  number of your choice and press Enter.\n\n\
    1  List the saved Base System configuration files which will not be\n\
       merged into the system.  These files are saved in /tmp/bos.\n\n\
    2  List the filesets which will be removed and not replaced.\n\n\
    3  List directories which will have all current contents removed.\n\n\
    4  Reboot without migrating.\n\n\
>>> 0  Continue with the migration.\n\n\
   88  Help ?\n\n\
+---------------------------------------------------------------------------\n\
  WARNING: Selected files, directories, and filesets (installable options)\n\
    from the Base System will be removed.  Choose 2 or 3 for more information.\n\n\
  %s\n\
>>> Choice[0]: " "`echo \"$INCREASE_TMP_WARNING\"`"
		read REPLY other

		if [ "$REPLY" = 1 ]
		then
			echo "$CLEAR\c"
			pageit /mnt/tmp/bos/cfgfiles.moved 2>/dev/null
			if [ $? = 0 ] 
			then
			        echo
				dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 64 'End of list.  Press Enter  '
			read stuff
			fi
		elif [ "$REPLY" = 2 ]
		then
			echo "$CLEAR\c"
			pageit /mnt/tmp/bos/filesets.gone 2>/dev/null
			if [ $? = 0 ] 
			then
			        echo
				dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 64 'End of list.  Press Enter  '
			        read stuff
			fi
		elif [ "$REPLY" = 3 ]
		then
			echo "$CLEAR\c"
			pageit /mnt/tmp/bos/directories.gone 2>/dev/null
			if [ $? = 0 ] 
			then
			        echo
				dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 64 'End of list.  Press Enter  '
			        read stuff
			fi
		elif [ "$REPLY" = 4 ]
		then
			cd /../
			RestoreBootImage
			export LIBPATH=/usr/ccs/lib:/usr/lib
			sync;sync;sync
			if [ $BOOTTYPE -ne $NETWORK ]
			then
			    cp -p /mnt/usr/sbin/reboot /usr/sbin/reboot >/dev/null 2>&1
			    if [ $? != 0 ]
			    then
			       cp -p /mnt/usr/sbin/reboot /../tmp/reboot >/dev/null 2>&1
			       PATH=$PATH:/../tmp
			    fi
			    /usr/sbin/umount all >/dev/null 2>&1
			    reboot -q
			else
			    cp -p /usr/sbin/reboot /../tmp/reboot >/dev/null 2>&1
			    cp -p /usr/sbin/umount /../tmp/umount >/dev/null 2>&1
			    nimclient -S shutdown
   		    	    nimclient -o change -a force=yes -a ignore_lock=yes -a \
             			info="Voluntary migration cancellation"
			    nimclient -R failure
			    /../tmp/umount all >/dev/null 2>&1
			    /../tmp/reboot -q
			fi
		elif [ "$REPLY" = 81360 ]
		then
			/usr/bin/ksh >/dev/console
		elif [ "$REPLY" = 88 ]
		then
			Migration_Help > /dev/console 2>&1
		elif [ "$REPLY" = 0 -o -z "$REPLY" ]
		then
			DONE=TRUE
		fi
	done
}

# optional_32_migration
# Determine whether something needs to be migrated or removed based upon
# certain factors such as whether something had been configured or whether
# something is available on the media
function optional_32_migration
{
	if [ -n "$SETX" -a "$SETX" -eq 1 ]
	then
		set -x
	fi
    ODMDIR=$MROOT/etc/objrepos
	#Install only those dlc packages which were actually configured on 3.2
	grep bosext2.dlc /tmp/3.2.installed | cut -d':' -f2 | cut -d'.' -f2 |
	sed 's/dlc//' |
	while read dlctype 
	do
		DLC32="bosext2.dlc$dlctype.obj"
		PKG41="bos.dlc.$dlctype bos.dlc.com"
		case $dlctype in 
			8023)	type='IEEE_ethernet'	
					PKG41="$PKG41 bos.dlc.com_enet"
					;;
			ether)	type='ethernet'	
					PKG41="$PKG41 bos.dlc.com_enet"
					;;
			fddi)	type='fddi'	
					;;
			qllc) 	type='x25_qllc'
					;;
			sdlc) 	type='sdlc'
					;;
			token) 	type='tokenring'
					;;
			*)		type=$dlctype
					;;
		esac
		rc=$(odmget -q PdDvLn=dlc/dlc/$type CuDv)
		if [ -n "$rc" ]
		then
			echo $DLC32 $PKG41 >> /mnt/tmp/bos/incompat.pkgs
		else
			echo $DLC32 >> /tmp/3.2.remove
		fi

	done

	# If FORTRAN was installed, determine if this is a level which will run
	# as is on 4.1; otherwise, deinstall it.
	cat > /tmp/xlf.remove <<!!
xlfcmp\.obj
xlfcmpm.*\.msg
!!
	grep -wf /tmp/xlf.remove /tmp/3.2.installed | cut -d: -f2 |
	while read fileset level
	do
		version=`echo $level | cut -d. -f1`
		if [ -n "$version" ] && [ $version -lt 3 ]
		then
			echo $fileset >> /tmp/3.2.remove
		fi
	done
	rm -rf /tmp/xlf.remove

	# Correct bad inventory information for bos.obj, which is always lpp #1
	cat > /tmp/inv.fix <<!!
inventory:
	loc2 = /etc/xroute
!!
    	ODMDIR=$MROOT/usr/lib/objrepos \
	odmchange -q "lpp_id=1 and loc0=/usr/sbin/xroute" -o inventory /tmp/inv.fix >/dev/null 2>&1
	rm -f /tmp/inv.fix
}


#
# Catch signals. If one occurs, reboot!
#

trap "
dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 11 29 \
'%s: Interrupted by user.\n' bos.rte.pre_i > /dev/console
if [ $BOOTTYPE -eq $NETWORK ]
then
   niminfo=`dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 11 29 \
'%s: Interrupted by user.\n' bos.rte.pre_i`
   Reboot "$niminfo" >/dev/console 2>&1
else
   Reboot >/dev/console 2>&1
fi
" 1 2 3 15

if [ -n "$SETX" -a "$SETX" -eq 1 ]
then
	set -x
fi
# Initialize PATH variables.
#   LIBPATH will be set so commands first look in RAM file system and then on
#           hard disk for things they need.
#   PATH    will be set so commands will be found first on the disk and then
#           in the RAM file system.
# The setting of these variables in this way will allow a 3.2 migration to
# use 4.1 kernel and libraries to execute 3.2 commands.
LIBPATH=/usr/ccs/lib:/usr/lib:/mnt/usr/ccs/lib:/mnt/usr/lib
PATH=/usr/bin:/usr/sbin:/mnt/usr/bin:/mnt/usr/sbin:
export LIBPATH PATH

# Initialize defined values.
NOT=!
export NOT

####
# Stop the update of the status line while in the migration menu.
if [ -s /../Update_Status.pn ]
then
	kill -STOP `cat /../Update_Status.pn`
fi

# The following code will, prior to beginning the migration, do any checking
# which could prevent the BOS migration from completing.


####
# Must use lslpp to determine packages currently installed and file names to
# remove.  However, lslpp will not follow ODMDIR or ODMPATH, so we must mount
# the disk ODM directories over RAM ODM directories.
mount $MROOT/etc/objrepos /etc/objrepos
mkdir -p /usr/lib/objrepos 2>/dev/null
mount $MROOT/usr/lib/objrepos /usr/lib/objrepos
mkdir -p /usr/share/lib/objrepos 2>/dev/null
mount $MROOT/usr/share/lib/objrepos /usr/share/lib/objrepos

# Create file which will contain names of product options to be installed at
# the new level so that the pg won't fail if there are none and the user
# chooses to look at them.
>/mnt/tmp/bos/install_options

if [ "$BOSLEVEL" = "3.2" ]
then
	# Produce list of installed filesets on this system
	/mnt/usr/bin/lslpp -qlc -Ous > /tmp/3.2.installed
	# Produce list of potential obsolesced filesets on this system
	# WARNING: Do not put ANY blanks in the list of fileset names
	cat > /tmp/3.2.remove <<!!
aic\.obj
aic12\.obj
aicm.*\.msg
aic12m.*\.msg
bosadt\.xde\.obj
bosext1\.ecs\.obj
bosext1\.vdidd\.obj
bosext2\.lrn\.info
bsm.*\.msg
gPfonts\..*
INedkj\.obj
OpenGL\..*
pcsim
pcsim\.obj
pcsimm.*\.msg
softPEX\.obj
softgP\.obj
txtfmt\.graf\.obj
txtfmtkj\..*
vdigft\.obj
vdiplot\.obj
xlccmp.obj
X11_3d\..*
X11_3diEn_US.info
!!
	optional_32_migration
	# Determine which of these filesets are installed on this system
	grep -wf /tmp/3.2.remove /tmp/3.2.installed | cut -d':' -f2,4 | \
		sed 's/ .*:/:/' | sort -u | \
		awk -F: '{printf("%-28s %s\n",$1,$2)}' >/mnt/tmp/bos/filesets.gone
	sort -f /mnt/tmp/bos/filesets.gone > /mnt/tmp/bos/gone.sort 2>/dev/null
	if [ $? = 0 ]
	then
		mv /mnt/tmp/bos/gone.sort /mnt/tmp/bos/filesets.gone   
	fi

    # Remove specifically listed files
    cat /mnt/tmp/bos/bos.rte.*.rmlist 2>/dev/null | grep -v "/$" | \
				sed 's:^:/mnt:' > /mnt/tmp/bos/rmlist.recheck
 
	# Add files from the obsolete packages to the list of removable files
	while read fs description
	do
	    		/mnt/usr/bin/lslpp -Or -qfc $fs 2>/dev/null | \
				cut -d: -f3 >> /mnt/tmp/bos/bos.rte.root.rmlist
	    		/mnt/usr/bin/lslpp -Ou -qfc $fs 2>/dev/null | \
				cut -d: -f3 | grep -v /inst_root/ \
					    >> /mnt/tmp/bos/bos.rte.usr.rmlist
	done < /mnt/tmp/bos/filesets.gone


# If this is a 3.2 to 4.x migration, determine which product options were on the
# system that cannot run in a 4.x environment and therefore must be reinstalled
# at the latest level.
	sed -e '/^#/d' -e 's/[ 	].*//' \
			/mnt/tmp/bos/incompat.pkgs >/tmp/3.2.list
	cat /tmp/3.2.installed | /mnt/usr/bin/grep -wf /tmp/3.2.list | \
		cut -d: -f2 | sed 's/[	 ].*$//' \
					>/mnt/tmp/bos/install_options
	sort -u /mnt/tmp/bos/install_options >/tmp/3.2.list
	/mnt/usr/bin/grep -wf /tmp/3.2.list /mnt/tmp/bos/incompat.pkgs | \
		while read PKG_32 PKGS_41
		do
			set -- $PKGS_41
			while [ "$#" -gt 0 ]
			do
				echo $1
				shift
			done
		done >/mnt/tmp/bos/install_options
else # Determine which options to migrate from the current 4.1 level
     # Get the list of installed filesets and their levels
     { LIBPATH=/mnt/usr/ccs/lib:/mnt/usr/lib:/usr/ccs/lib:/usr/lib
         /mnt/usr/bin/lslpp -qlc -Ous |
			    cut -d: -f2,3 | sed 's/:/ /' > /tmp/installed.list
     }
    # Build list of files which need to be checked for conflicts with bos.rte
    cat /mnt/tmp/bos/bos.rte.*.rmlist 2>/dev/null | grep -v "/$" | \
				sed 's:^:/mnt:' > /mnt/tmp/bos/rmlist.recheck

fi

# Gather list of files to be removed.
if [ "$BOSLEVEL" = 3.2 ]
then
	# if 3.2 -> 4.1 migration, use bos.rte.*.rmlist to generate list.

	# First use grep to get the real files listed in the stanza name, and
	# filter that through sed, deleting everything after the colon.  Then,
	# use grep to get the "links" and "symlinks" fields, allowing one or
	# more spaces or tabs before and after the keywords and equals signs.
	# Filter that through sed to get rid of keywords and equals, filter
	# again to get rid of blank lines, filter again to replace commas with
	# a <crlf> (this will put each file name on one line), and the final
	# filter gets rid of leading white space and prepends /mnt onto each
	# file name in one swell foop.
	cat /mnt/tmp/bos/bos.rte*rmlist | \
		sed -e 's:^:/mnt:' | sed -e 's:/$::' >/mnt/tmp/bos/files_deleted
else

	# Otherwise, this is a 4.X -> 4.X migration, so just use the files
	# in bos.rte as those to be deleted.

	# Use lslpp to get file names from /, /usr, and /usr/share SWVPD.
	# Filter that through sed to get rid of comment lines.  Filter that
	# through cut to strip off extraneous information.  Filter through sed
	# again to delete anything after the filename, and finally filter to
	# prepend /mnt onto every filename.
	{ LIBPATH=/mnt/usr/ccs/lib:/mnt/usr/lib:/usr/ccs/lib:/usr/lib \
		/mnt/usr/bin/lslpp -Our -qfc bos.rte\* 2>/dev/null; } | \
		cut -d: -f3 | \
		sed -e 's/ .*$//' -e 's:^:/mnt:' >/mnt/tmp/bos/files_deleted
		ls /mnt/usr/share/lib/objrepos/* >> /mnt/tmp/bos/files_deleted
		/mnt/usr/sbin/slibclean
fi


# Sort "files_deleted" in reverse order to ensure that the files get deleted
# before the directories which contain them.
sort -ro /mnt/tmp/bos/files_deleted /mnt/tmp/bos/files_deleted

#
# Now read in the lpp_name file from the install media
#

case "$BOOTTYPE" in

    $CDROM )
        restbyname -xqvf/SPOT/usr/sys/inst.images/bos ./lpp_name; rc="$?" > /dev/null
        if [ "$rc" -ne 0 ]
        then
	    # cannot restore the lpp_name file... abort
	    dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 69 'Cannot restore the lpp_name file from CDROM.\n'
	    Reboot >/dev/console 2>&1
        fi
        ;;

    $TAPE )
	#
	# for migration installs, the tape block size will always be 512 bytes.
	#
        tctl -f /dev/$BOOTDEV.1 rewind
        tctl -f /dev/$BOOTDEV.1 fsf 3
	restbyname -xqvf/dev/$BOOTDEV.1 -h -S ./lpp_name >/dev/null;  rc="$?"
        if [ "$rc" -ne 0 ]
        then
           # cannot restore the lpp_name file... abort 
	    dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 70 'Cannot restore the lpp_name file from tape.\n' >/dev/console
	    Reboot >/dev/console 2>&1
        fi
        ;;

     $NETWORK )
	case "$NIM_BOS_FORMAT" in

	   rte )
		# Restore the lpp_name file from the image
		restbyname -xqvf$NIM_BOS_IMAGE -h -S ./lpp_name > /dev/null; rc="$?"
        	if [ "$rc" -ne 0 ]
        	then
           	# cannot restore the lpp_name file... abort 
	        dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 71 'Cannot restore the lpp_name file from network.\n' >/dev/console
		niminfo=`dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 71 'Cannot restore the lpp_name file from network.\n'`
	        Reboot "$niminfo" >/dev/console 2>&1

        	fi
	   ;;
		
	   spot )
		# Image may not be available. Must create an lpp_name file
		# using the bidata program to read the /image.data file

		> ./lpp_name
		for fs_name in `/usr/lpp/bosinst/bidata -i -g fs_data -f FS_NAME`
		do
		  case "$fs_name" in 
			# migration only impacts these directories
			/|/var|/usr )
			  echo ${fs_name} `/usr/lpp/bosinst/bidata -i -g fs_data -f FS_SIZE -c FS_NAME -v ${fs_name}` >> lpp_name
			;;
		  esac
		done
		;;

	   * )
		# migration only supports network installs from rte or spot
	        dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 72 'Migration installations from the network are only supported when\n\
the BOS runtime files are in a SPOT or part of an lpp_source resource.\n'
	        niminfo=`dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 72 'Migration installations from the network are only supported when\n\
the BOS runtime files are in a SPOT or part of an lpp_source resource.\n'`
	        Reboot "$niminfo" >/dev/console 2>&1
		;;

	esac

	;;
	
    * )
	#
	# unknown device... abort
	#
	echo " Attempted to install from an unknown device. "  > /dev/console
	dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 73 'You attempted a migration install from an unsupported device.\n' >/dev/console
	Reboot >/dev/console 2>&1
	;;

esac
#
# Files processed by this code
#
FILES_ADDED=/mnt/tmp/bos/files_added
FILE_SYSTEMS=/mnt/tmp/bos/file_systems
FILES_DELETED=/mnt/tmp/bos/files_deleted
TMP_DIRS_DELETED=/mnt/tmp/bos/files_removed
DIRS_DELETED=/mnt/tmp/bos/dirs_deleted

#
# Initialize this flag to a null string.  Will be set if the
# dirs_deleted file gets created.
#
DEL_FLAG=

#
# Return code from adjfs when it cannot expand a filesystem
#
FS_EXPAND_ERROR=5

#
# Now extract all the size information from the lpp_name file. 
#
grep "^/" lpp_name | sed -e 's:^:/mnt:' > $FILES_ADDED

#
# Now create a sorted list of the list of filesystems.  The list is 
# sorted by mount point pathname in reverse order.  This will cause 
# /mnt/usr/lpp/gorp to appear before /mnt/usr.  Only those filesystems 
# whose mount points begin with /mnt will be processed.
#
/usr/bin/df -k 2>/dev/null | awk '{print $7, $1, $3}' | grep '^/mnt' | sort -r > $FILE_SYSTEMS

#
# Now create a list of directories that this script will subsequently 
# delete if this is a AIX 3.2 -> 4.1 migration install.  Obviously,
# any changes to the 3.2 -> 4.1 specific processing portion of this script
# must be relfected here.
#

# Determine directories which will be removed
if [ "$BOSLEVEL" = "3.2" ]
then
	cat > /mnt/tmp/bos/directories.gone <<!!
/dev
/lpp/X11rte
/lpp/bos
/lpp/bosext1
/lpp/bosext2
/lpp/bosnet
/lpp/txtfmt
/tmp
/usr/include/DPS
/usr/lib/drivers
/usr/lib/methods
/usr/lib/microcode
/usr/lpp/bos/deinstl
/usr/lpp/bos/inst_U*
/usr/lpp/bos/inst_ptf
/usr/lpp/bos/inst_root
/usr/lpp/bos/_bbl_
/usr/lpp/bos/_gt0_adapter_
/usr/lpp/bos/_gt4_adapter_
/usr/lpp/bos/_new_syscalls_
/usr/lpp/bos/_scdisk_
/usr/lpp/bos/_scsi_
/usr/lpp/bos/_sctape_
/usr/lpp/bos/_serdasd_
/usr/lpp/bos/_slip_
/usr/lpp/bosadt
/usr/lpp/bosext1
/usr/lpp/bosext2
/usr/lpp/bosnet
/usr/lpp/bosperf
/usr/lpp/bsl
/usr/lpp/bsm{LANG}
/usr/lpp/diagnostics
/usr/lpp/DPS
/usr/lpp/fddi
/usr/lpp/txtfmt
/usr/lpp/X11/Xamples/lib/DPS
/usr/lpp/X11/lib/X11/fonts/Type1/DPS
/usr/lpp/X11dev
/usr/lpp/X11fnt
/usr/lpp/X11rte
/usr/share/lpp/bos
/usr/share/lpp/bosadt
/usr/share/lpp/bosext1
/usr/share/lpp/bosext2
/usr/share/lpp/txtfmt
/var/spool/lpd/qdir
/var/spool/qdaemon
!!
else
	echo /tmp > /mnt/tmp/bos/directories.gone
fi

if [ "$BOSLEVEL" = "3.2" ]
then
   # account for the directories that will be removed
   cat /mnt/tmp/bos/directories.gone |
     sed 's:^/:/mnt/:' | xargs du -s >$TMP_DIRS_DELETED 2>/dev/null
			
   # account for the miscellaneous files that will be removed
   echo /mnt/var/spool/lpd/stat/numfile \
        /mnt/usr/lib/objrepos/GAI \
        /mnt/usr/lib/objrepos/GAI.vc \
        /mnt/usr/lib/objrepos/XINPUT \
        /mnt/usr/lib/objrepos/XINPUT.vc \
	| xargs du -s >> $TMP_DIRS_DELETED 2>/dev/null
	
   # reformat this file so the pathname preceeds the size information
   cat $TMP_DIRS_DELETED | awk '{print $2, $1}' > $DIRS_DELETED
   rm $TMP_DIRS_DELETED
   DEL_FLAG=" -d $DIRS_DELETED "

fi 

# Link in the files which chfs needs into the ram filesystem
# 3.2 chfs calls the fs cmds using the /etc symlink
if [ "$BOSLEVEL" = 3.2 ]
then 
    CHFS_CMD_DIR=/etc
else
    CHFS_CMD_DIR=/usr/sbin
fi

ln -s /mnt/usr/sbin/chlv     /mnt/usr/sbin/extendlv \
      /mnt/usr/sbin/getlvodm /mnt/usr/sbin/lquerylv \
      /mnt/usr/sbin/putlvcb  /mnt/usr/sbin/lqueryvg \
      /mnt/usr/sbin/lslv     $CHFS_CMD_DIR

#
# Now call adjfs to determine if any filesystems need to be expanded 
# prior to proceeding with the installation.  adjfs will also expand any
# filesystem that it determines to need more space
#
ADJFS_ERR=`/usr/lpp/bosinst/adjfs -f $FILE_SYSTEMS -s $FILES_ADDED -x $FILES_DELETED $DEL_FLAG`; rc=$?

rm /../lpp_name $FILE_SYSTEMS $FILES_ADDED $DIRS_DELETED >/dev/null 2>&1

if [ "$rc" -ne 0 ]
then
	case "$rc" in

	  $FS_EXPAND_ERROR )
		# Cannot expand one of the filesystems
		FSYS_NAME=`echo $ADJFS_ERR | sed -e 's:^/mnt::' | cut -d" " -f1`
		FS_SPACE_REQUIRED=`echo $ADJFS_ERR | cut -d" " -f2`
		dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 75 'Cannot expand the %s filesystem. An additional\n\
%s 512-byte blocks are required.\n' $FSYS_NAME $FS_SPACE_REQUIRED >/dev/console
		if [ $BOOTTYPE -eq $NETWORK ]
		then
		    niminfo=`dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 75 'Cannot expand the %s filesystem. An additional\n\
%s 512-byte blocks are required.\n' $FSYS_NAME $FS_SPACE_REQUIRED`
		    Reboot "$niminfo" >/dev/console 2>&1
		else
		    Reboot >/dev/console 2>&1
		fi
 	  ;; 
	  * )
		# Cannot proceed with the installation
		dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 74 'The BOS pre-installation procedure detected an internal processing error (%s).\n' $rc >/dev/console
		if [ $BOOTTYPE -eq $NETWORK ]
		then
		    niminfo=`dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 74 'The BOS pre-installation procedure detected an internal processing error (%s).\n' $rc`
		    Reboot "$niminfo" >/dev/console 2>&1
		else
		    Reboot >/dev/console 2>&1
		fi
	  ;;

	esac
fi

# Now unmount the ODM directories which were mounted above.
umount /etc/objrepos
umount /usr/lib/objrepos
umount /usr/share/lib/objrepos

# Determine list of files which will be left in /tmp/bos
grep user_merge /mnt/tmp/bos/bos.rte.cfgfiles |
	awk '{print $1}' > /mnt/tmp/bos/cfgfiles.moved
if [ "$BOSLEVEL" = "3.2" ]
then
   grep v4preserve /mnt/tmp/bos/bos.rte.cfgfiles |
	awk '{print $1}' >> /mnt/tmp/bos/cfgfiles.moved
fi
sort -o /mnt/tmp/bos/cfgfiles.moved /mnt/tmp/bos/cfgfiles.moved

####
# Ensure that /tmp has enough space to contain the saved files and run the
# bosboot command.

# Sum sizes of working files in /tmp/bos
du -sk /mnt/tmp/bos | read WORKING_TMP_SIZE junk

# Sum sizes of files to be saved.
# First filter cfgfiles through sed twice to get rid of everything after the
# file name and to prepend /mnt onto the file name.  Then have du tell us
# the size of the file in k-bytes and add those sizes up.
du -sk `cat /mnt/tmp/bos/bos.rte.cfgfiles | \
	sed -e 's/[	 ].*$//' \
		-e '/^[ 	]*$/d' -e 's:^:/mnt:'` 2>/dev/null | \
		awk 'BEGIN { count = 0 } { count += $1 } END { print count }' \
			| read SIZE_NEEDED_FOR_FILES

# Have to add in the working space plus a buffer for directory space
(( SIZE_NEEDED_FOR_FILES = $SIZE_NEEDED_FOR_FILES + $WORKING_TMP_SIZE + 100 ))

# Determine size of unmerged files in k-bytes
du -sk `grep user_merge /mnt/tmp/bos/bos.rte.cfgfiles | \
	sed -e 's/[	 ].*$//' \
		-e '/^[ 	]*$/d' -e 's:^:/mnt:'` 2>/dev/null | \
		awk 'BEGIN { count = 0 } { count += $1 } END { print count }' \
			| read SIZE_NEEDED_FOR_UNMERGED

# Determine size of 3.2-only unmerged files in k-bytes
if [ $BOSLEVEL = "3.2" ]
then
     du -sk `grep v4preserve /mnt/tmp/bos/bos.rte.cfgfiles | \
	sed -e 's/[	 ].*$//' \
		-e '/^[ 	]*$/d' -e 's:^:/mnt:'` 2>/dev/null | \
		awk 'BEGIN { count = 0 } { count += $1 } END { print count }' \
			| read SIZE_OF_UNMERGED_32
(( SIZE_NEEDED_FOR_UNMERGED = $SIZE_NEEDED_FOR_UNMERGED + $SIZE_OF_UNMERGED_32 ))
fi

# Old version of sysck.cfg is left in /tmp/bos for reference
if [ -f /mnt/etc/security/sysck.cfg ]
then
   du -sk /mnt/etc/security/sysck.cfg | read TCBCKSIZE junk
   (( SIZE_NEEDED_FOR_UNMERGED = $SIZE_NEEDED_FOR_UNMERGED + $TCBCKSIZE ))
fi   

# Figure out how much room will be left over in /tmp after replacing the
# merged files.
du -sk /mnt/tmp | read TOTAL_TMP_FILES junk

# Get size of available /tmp space in k-bytes
/usr/bin/df -k /mnt/tmp 2>/dev/null | tail +2 | read junk TMP_SPACE FREE_TMP junk2

(( TMP_FS_OVERHEAD = $TMP_SPACE - $TOTAL_TMP_FILES - $FREE_TMP ))

(( RECOVERED_SPACE = $SIZE_NEEDED_FOR_FILES - $SIZE_NEEDED_FOR_UNMERGED - \
				$TMP_FS_OVERHEAD - $WORKING_TMP_SIZE - 100 ))
(( MIN_REQUIRED = $SIZE_NEEDED_FOR_UNMERGED + 7800 + \
				$TMP_FS_OVERHEAD + $WORKING_TMP_SIZE + 100 ))

if [ $RECOVERED_SPACE -lt 7800 ]
then
     (( BOSBOOT_SPACE_NEEDED = 7800 - $RECOVERED_SPACE ))
else
     BOSBOOT_SPACE_NEEDED=0
fi

# Now add size needed to make a boot image (that would be 7800 Kblocks).
(( SIZE_NEEDED_FOR_FILES = $SIZE_NEEDED_FOR_FILES + \
			   $TMP_FS_OVERHEAD + $BOSBOOT_SPACE_NEEDED ))

# Make sure that there is at least enough space for the bosboot and the
# saved unmerged files
if [ $SIZE_NEEDED_FOR_FILES -lt $MIN_REQUIRED ]
then
    SIZE_NEEDED_FOR_FILES=$MIN_REQUIRED
fi

# Get number of free partitions and convert them to k-bytes.
#
VGid=`getlvodm -v rootvg`
FIRSTDISK=`getlvodm -w $VGid | line | sed 's/^.*[ 	][ 	]*//'`
FREE_PPS=`lqueryvg -p"$FIRSTDISK" -F`
# Get physical partition size in megabytes.
PP_SIZE=`lsvg rootvg | sed -n 's/^.*PP SIZE:[ 	]*\([0-9][0-9]*\).*/\1/p'`
# Now convert to k-bytes.
(( FREE_PPS = $FREE_PPS * $PP_SIZE * 1024 ))
# Now reduce FREE_PPS by 4 percent to account for file system overhead.
FREE_PPS=`echo $FREE_PPS | \
	awk '{ needed = $1 } END { print int(needed * 0.96) }'`

export SIZE_NEEDED_FOR_FILES TMP_SPACE FREE_PPS PP_SIZE


PROMPT=`/usr/lpp/bosinst/bidata -b -g control_flow -f PROMPT`

# Check that we have the minimum amount of space available in /tmp
# to process the configuration files.
(( TOTAL_FREE = $FREE_TMP + $FREE_PPS ))

if [ "$SIZE_NEEDED_FOR_FILES" -gt "$TOTAL_FREE" ]
then
	dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 61 \
"  The total space needed to contain the configuration files is\n  %s 1024-byte blocks.  The total /tmp space plus the total space not\n  already allocated is %s 1024-byte blocks.  The migration cannot\n  continue.\n  To reboot the system from disk, turn the key to the NORMAL position and\n  press Enter.  To reboot from other media in order to select another\n  install method, turn the key to the SERVICE position and press Enter: " \
"$SIZE_NEEDED_FOR_FILES" "$TOTAL_FREE"

    if [ "$PROMPT" != no ]
    then
        read ans
    fi

    RestoreBootImage
    export LIBPATH=/usr/ccs/lib:/usr/lib
    sync;sync;sync
    if [ $BOOTTYPE -ne $NETWORK ]
    then
        cp -p /mnt/usr/sbin/reboot /usr/sbin/reboot >/dev/null 2>&1
	if [ $? != 0 ]
	then
	   rm -f /usr/sbin/reboot >/dev/null 2>&1
	   cp -p /mnt/usr/sbin/reboot /../tmp/reboot >/dev/null 2>&1
	   PATH=$PATH:/../tmp
	fi
        /usr/sbin/umount all >/dev/null 2>&1
        reboot -q
    else
	# Have to copy the commands into the RAM filesystem
        # because the nimclient failure renders the spot
        # inaccessible
        cp -p /usr/sbin/reboot /../tmp/reboot >/dev/null 2>&1
        cp -p /usr/sbin/umount /../tmp/umount >/dev/null 2>&1
	nimclient -S shutdown
	niminfo=`dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 61 \
"  The total space needed to contain the configuration files is\n  %s 1024-byte blocks.  The total /tmp space plus the total space not\n  already allocated is %s 1024-byte blocks.  The migration cannot\n  continue.\n  To reboot the system from disk, turn the key to the NORMAL position and\n  press Enter.  To reboot from other media in order to select another\n  install method, turn the key to the SERVICE position and press Enter: " \
"$SIZE_NEEDED_FOR_FILES" "$TOTAL_FREE"`

   	nimclient -o change -a force=yes -a ignore_lock=yes -a \
             		    info="$niminfo"
        nimclient -R failure
	/../tmp/umount all >/dev/null 2>&1
	/../tmp/reboot -q 
    fi
fi
if [ "$PROMPT" != "no" ]
then
Migration_Menu >/dev/console 2>&1
fi

####
# Let the users know that something is happening
echo "$CLEAR\c" > /dev/console
dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 11 37  \
"Saving system configuration files in /tmp/bos..." > /dev/console


####
# Append the list of product options to install onto the end of device.pkgs
cat /mnt/tmp/bos/install_options > /../tmp/mig.pkgs
rm -f /mnt/tmp/bos/install_options

####
# Remove the unneeded files in /mnt/tmp.
find /mnt/tmp/* /mnt/tmp/.* -prune -print | 
egrep -v "^/mnt/tmp/.$|^/mnt/tmp/..$|/mnt/tmp/bos$|/mnt/tmp/lost+found$" |
xargs rm -rf

cd /mnt

####
# If necessary, increase /mnt/tmp enough to hold the config files.  Checking
# has already been done to ensure that there is enough free space to expand
# the file system.
if [ "$SIZE_NEEDED_FOR_FILES" -gt "$TMP_SPACE" ]
then
	# Change SIZE_NEEDED_FOR_FILES to 512-byte blocks instead of
	# 1024-byte blocks so it can be given as a parameter to chfs.
	(( SIZE_NEEDED_FOR_FILES = $SIZE_NEEDED_FOR_FILES * 2 ))

	# figure out which device is associated with /mnt/tmp since we
	# cannot use /tmp (that's /tmp in RAM) or /mnt/tmp (since
	# /mnt/tmp is not the name of the fs in /etc/filesystems).
	# Get the device associated with the migrating /tmp and send to chfs

	tmpdev=`mount 2>&1 | grep " /mnt/tmp " | awk '{print $1}'`
	if [ $? = 0 ]
	then
	   /mnt/usr/sbin/chfs -a size="$SIZE_NEEDED_FOR_FILES" $tmpdev
	   rc="$?"
	else
	   rc=1
	fi

	if [ "$rc" -ne 0 ]
	then
		dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 65  "Migration could not be completed because /tmp could not be extended.\nReboot by turning the switch to normal and pressing Enter.  " >/dev/console
		if [ "$PROMPT" != "no" ]
		then
		    read ans
		fi
		DONE=TRUE
		RestoreBootImage
		export LIBPATH=/usr/ccs/lib:/usr/lib
		sync;sync;sync
		cd /
		if [ $BOOTTYPE -ne $NETWORK ]
		then
		    cp -p /mnt/usr/sbin/reboot /usr/sbin/reboot >/dev/null 2>&1
		    if [ $? != 0 ]
		    then
	   	       rm -f /usr/sbin/reboot >/dev/null 2>&1
	   	       cp -p /mnt/usr/sbin/reboot /../tmp/reboot >/dev/null 2>&1
	   	       PATH=$PATH:/../tmp
		    fi
		    /usr/sbin/umount all >/dev/null 2>&1
		    reboot -q
		else
		    # Have to copy the commands into the RAM filesystem
		    # because the nimclient failure renders the spot
		    # inaccessible
		    cp -p /usr/sbin/reboot /../tmp/reboot >/dev/null 2>&1
		    cp -p /usr/sbin/umount /../tmp/umount >/dev/null 2>&1
		    nimclient -S shutdown
		    niminfo=`dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 65  "Migration could not be completed because /tmp could not be extended.\nReboot by turning the switch to normal and pressing Enter.  "`
   		    nimclient -o change -a force=yes -a ignore_lock=yes -a \
             		    info="$niminfo"
		    nimclient -R failure
		    /../tmp/umount all >/dev/null 2>&1
		    /../tmp/reboot -q
		fi
	fi
fi

####
# Special processing for terminfo.
if [ -d /mnt/usr/share/lib/terminfo -a "$BOSLEVEL" = 3.2 ]
then
	# move terminfo to a new name, using weird name to avoid conflicts
	mv /mnt/usr/share/lib/terminfo /mnt/usr/share/lib/tCeOrMmPiAnTfo
	mkdir /mnt/usr/share/lib/terminfo
	chown 2:2 /mnt/usr/share/lib/terminfo
	chmod 755 /mnt/usr/share/lib/terminfo
	mv /mnt/usr/share/lib/tCeOrMmPiAnTfo /mnt/usr/share/lib/terminfo/compat
        ODMDIR=/mnt/usr/share/lib/objrepos \
	      odmdelete -o inventory -q "loc0 like /usr/share/lib/terminfo*" >/dev/null 2>&1
fi

####
# sync to ensure that all ODM writes have been flushed to the disk
#
sync
sync


####
# Save configuration files to /mnt/tmp/bos under file path names relative to
# root.

sed -e 's:[ 	].*$::' \
	-e '/^[ 	]*$/d' \
	-e 's:^:\.:' /mnt/tmp/bos/bos.rte.cfgfiles | \
	cpio -padu /mnt/tmp/bos >/dev/null 2>&1

##
#
dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 11 38  \
"\nRemoving obsolete filesets, directories, and files..." > /dev/console

####
# Now remove the bos.rte files and directories.  If a directory is not
# empty, rmdir will not remove it.
#  First copy rmdir to the RAM file system because we can no longer use it
#  from the disk once it gets removed.
if [ ! -f /usr/bin/rmdir ]
then
	cp -p /mnt/usr/bin/rmdir /usr/bin/rmdir
	cp -p /mnt/usr/bin/xargs /usr/bin/xargs
fi

#Perform 3.2->* specific processing 
if [ $BOSLEVEL = "3.2" ]
then
    # Clean out the PTF specific SWVPD info
    for odmdir in /mnt/tmp/bos/etc/objrepos \
		 /mnt/tmp/bos/usr/lib/objrepos \
		 /mnt/tmp/bos/usr/share/lib/objrepos
    do
       ODMDIR=$odmdir \
	  odmdelete -o inventory -q "loc0 like */inst_[prU]*/*" >/dev/null 2>&1 
    done

    # Remove device support and clean out the device info from the SWVPD
    # for 3.2->4.1 migration (also from packaging directories getting wiped).
    rm -rf /mnt/dev/* /mnt/usr/lib/methods/* /mnt/usr/lib/drivers/* \
		    /mnt/usr/lib/microcode/* /mnt/usr/lpp/diagnostics/*
    for devdir in /dev /usr/lib/methods /usr/lib/drivers /usr/lib/microcode \
		   /usr/lpp/diagnostics /etc/methods /etc/drivers \
		   /etc/microcode /usr/lpp/bosext2 /usr/lpp/fddi \
		   /usr/include/DPS /usr/lpp/DPS /usr/lpp/bosperf \
		   /usr/lpp/X11/Xamples/lib/DPS /usr/lpp/X11/lib/X11/fonts/Type1/DPS
    do
       ODMDIR=/mnt/tmp/bos/usr/lib/objrepos \
	      odmdelete -o inventory -q "loc0 like $devdir/*" >/dev/null 2>&1
    done

    # Remove old BOS and X11 3.2 package files
    rm -rf /mnt/usr/lpp/bos/inst_U* /mnt/usr/lpp/bos/inst_ptf \
	   /mnt/usr/lpp/bos/inst_root /mnt/usr/lpp/bos/deinstl \
	   /mnt/lpp/bos \
	   /mnt/usr/lpp/bosext1 /mnt/usr/lpp/bosext2 \
	   /mnt/usr/lpp/bosadt  /mnt/usr/lpp/bosnet \
	   /mnt/usr/lpp/bosperf /mnt/usr/include/DPS \
	   /mnt/usr/lpp/bsl   /mnt/usr/lpp/DPS \
	   /mnt/usr/lpp/fddi  /mnt/usr/lpp/txtfmt \
	   /mnt/usr/lpp/X11dev  /mnt/usr/lpp/X11fnt \
	   /mnt/usr/lpp/X11rte  /mnt/lpp/X11rte /mnt/lpp/txtfmt \
	   /mnt/lpp/bosext1 /mnt/lpp/bosext2 /mnt/lpp/bosnet \
	   /mnt/usr/share/lpp/bos /mnt/usr/share/lpp/bosadt \
	   /mnt/usr/share/lpp/bosext1 /mnt/usr/share/lpp/bosext2 \
	   /mnt/usr/share/lpp/txtfmt

    # Clean out the PTF specific SWVPD info
    for odmdir in /mnt/tmp/bos/etc/objrepos \
		 /mnt/tmp/bos/usr/lib/objrepos \
		 /mnt/tmp/bos/usr/share/lib/objrepos
    do
       ODMDIR=$odmdir \
	  odmdelete -o inventory -q "loc0 like */inst_[prU]*/*" >/dev/null 2>&1 
    done

    # Clean out obsolete printer and queueing files
    rm -f /mnt/var/spool/lpd/stat/numfile \
	  /mnt/var/spool/lpd/qdir/* \
	  /mnt/var/spool/qdaemon/*

    # Clean out obsolete graphics databases
    rm -f /mnt/usr/lib/objrepos/GAI \
          /mnt/usr/lib/objrepos/GAI.vc \
          /mnt/usr/lib/objrepos/XINPUT \
          /mnt/usr/lib/objrepos/XINPUT.vc

fi # 3.2 processing

xargs rm -f < /mnt/tmp/bos/files_deleted 2>/dev/null
xargs /usr/bin/rmdir < /mnt/tmp/bos/files_deleted 2>/dev/null
rm -f /mnt/tmp/bos/files_deleted 2>/dev/null

# Anything left should be a non-empty directory
# This code absolutely depends on directories ending in "/"
# in the rmlist file.
rm -f /mnt/tmp/bos/dirs.moved
rm -f /mnt/tmp/bos/dirs.removed
if [ -f /mnt/tmp/bos/rmlist.recheck ]
then
	while read FILE
	do
	    if [ -d $FILE -a ! -L $FILE ]
	    then
		# Conflict between directory on the system and the
		# files and links in the bos.rte image
		mv $FILE $FILE.save.$$ > /dev/null 2>&1
		if [ $? = 0 ] 
		then
		   echo $FILE $FILE.save.$$ >> /mnt/tmp/bos/dirs.moved
		else
		   # One more try - shouldn't ever happen really, but
		   # the consequences are dire if it fails
		   mv $FILE $FILE.save.$$.$$ > /dev/null 2>&1
		   if [ $? = 0 ] 
		   then
		      echo $FILE $FILE.save.$$.$$ >> /mnt/tmp/bos/dirs.moved
		   else
		      # drastic measure - have to unlink the old dir
		      unlink $FILE >/dev/null 2>&1
		      echo $FILE >> /mnt/tmp/bos/dirs.removed
		   fi
		fi
	    fi
	done < /mnt/tmp/bos/rmlist.recheck
	rm -f /mnt/tmp/bos/rmlist.recheck 2>&1 
fi

exit 0
