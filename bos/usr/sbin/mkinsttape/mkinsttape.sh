#!/bin/ksh
# @(#)13  1.2.4.13  src/bos/usr/sbin/mkinsttape/mkinsttape.sh, bosinst, bos41J, 9523C_all 6/9/95 09:08:57
#
# COMPONENT_NAME: (BOSINST) Base Operating System Installation
#
# FUNCTIONS: mkinsttape
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1989, 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# NAME: mkinsttape.sh
#
# FUNCTION:
#	Create the BOS install/maintenance tape file image by copying required
#	files to a relative position from root, using inslists to 
#	backup the files to /tmp/bosinst.tape, and then to remove the file
#	copies.
#
#	Requirements:	root authority required.
#			/tmp file system large enough for output image.
#			backup/restore commands
#
# EXECUTION ENVIRONMENT:
#
#	SYNTAX:		mkinsttape [/file]
#				/file defaults to /tmp/bosinst.tape
#
# DATA STRUCTURES:
#
######################################################
cleanup()
{
	cd /
	ec=$1
	[ "$ec" -ne 0 ] && echo "mkinsttape failed"
	for i in $dirlist; do umount $i; done
	rmdir $mntdir 2>/dev/null
	cd /tmp
	rm -fr $temp	
	cd $cwd
	exit $ec
}
######################################################
#
# MAIN
#
DEVICE=${1:-'/tmp/bosinst.tape'}

trap "trap '' 0 1 2 15; cleanup 1" 0 1 2 15

root='/'
mntdir=/ethan
temp=/tmp/bosinst
libs=$temp/usr/lib
ccslibs=$temp/usr/ccs/lib
diaglibs=$temp/usr/lpp/diagnostics/lib
filelist=$temp/filelist
bosinstdir=/usr/lpp/bosinst/tape
bosbootdir=/usr/lib/boot/tape
tapeblksz=/tapeblksz
cwd=`pwd`
needed=5000

[ `expr "$DEVICE" : "[/]"` -eq 0 ] && {
	echo "\nmkinsttape: Absloute path must be used on target device: $DEVICE\n" 
	trap '' 0 1 2 15
	cleanup 2
	}
rm -fr $temp 2> /dev/null
	#
	# Make sure there's some working space in /tmp
	#
tmpfree=`df -k /tmp | awk 'NR == 2 { print $3 }'`
[ "$tmpfree" -lt "$needed" ] && {
	echo "/tmp needs $needed KB free space."
	trap '' 0 1 2 15
	cleanup 3
}

mkdir -p $libs/objrepos || { trap '' 0 1 2 15; cleanup 5; }
mkdir -p $ccslibs || { trap '' 0 1 2 15; cleanup 5; }
mkdir -p $diaglibs || { trap '' 0 1 2 15; cleanup 5; }

	#
	# Strip libraries before they go in the install image.
	#
cd $temp || { trap '' 0 1 2 15; cleanup 6; }
# If diagnostics have been installed, strip libraries needed for pretest also.
if [ -s /usr/lpp/diagnostics/bin/dskpt.dep ]
then
	depfile=/usr/lpp/diagnostics/bin/dskpt.dep
else
	depfile=
fi

grep -h -e /usr/lib/lib -e /usr/ccs/lib/lib -e /usr/lpp/diagnostics/lib/lib \
	$bosinstdir/tapefiles1 $depfile \
	| while read libname rest
		do
			ar x $libname shr.o
			strip shr.o 2>/dev/null
			ar cq .$libname shr.o  || { trap '' 0 1 2 15; cleanup 7; }
			rm -f shr.o
		done

cd $root || { trap '' 0 1 2 15; cleanup 10; }

> $filelist

if [ -s $tapeblksz ]
then
     echo .$tapeblksz > $filelist
fi

# Now include the map files.
ls -1 ./tmp/vgdata/*/*.map 2>/dev/null >> $filelist

# Only put device drivers in list if they exist on the machine.
for i in `grep /usr/lib/drivers $bosinstdir/tapefiles1`
do
	if [ -f $i ]
	then
		echo $i
	fi
done >> $filelist

# There may be additional tape device support 
# specified in /usr/lib/boot/tape.tapefiles1.[tape type].
# Only put additional device drivers in list if they exist
# on the machine.
for i in `cat $bosbootdir/* 2>/dev/null`
do
	if [ -f $i ]
	then
		echo $i
	fi
done >> $filelist

# The following will append to filelist everything in tapefiles1 except
# the BosMenus.cat files and the device drivers.  Since I got the drivers
# files in the above "for" loop, I will filter the file list through grep to
# take out the drivers.  The BosMenus.cat files will be handled later.
cat $bosinstdir/tapefiles1 | grep -v BosMenus.cat | \
	grep -v /usr/lib/drivers >> $filelist

if [ -f /usr/lib/nls/msg/$LANG/BosMenus.cat ]
then
	echo /usr/lib/nls/msg/$LANG/BosMenus.cat >> $filelist
fi

# If the diagnostics package has been installed, include in the bosinst.tape
# image the files needed to support pretest.
if [ -x /usr/lpp/diagnostics/bin/diagdskpt -a \
	-x /usr/lpp/diagnostics/bin/get_diag_stanzas ]
then
	/usr/lpp/diagnostics/bin/get_diag_stanzas $libs/objrepos
	# only include the pretest files which exist.
	for i in `cat /usr/lpp/diagnostics/bin/dskpt.dep`
	do
		if [ -f "$i" ]
		then
			echo $i
		fi
	done >> $filelist

fi

# Copy objrepos files which are not already there from /usr/lib/objrepos to
# /tmp/bosinst/usr/lib/objrepos.
cd $root
grep -h /usr/lib/objrepos $depfile $bosinstdir/tapefiles1 \
	| while read objfile rest
		do
			if [ ! -f $temp$objfile ]
			then
				cp $objfile $temp$objfile
			fi
		done

	#
	# Mount and link such that we can pick up files modified by mkinsttape
	# along with system files and have it look like it came directly off the
	# system.
	#
# First mount over $mntdir all jfs file systems as they look from /.
mkdir $mntdir 2>/dev/null
dirlist=
mount | grep jfs | while read dev dir other
do
	mount $dir $mntdir$dir
	dirlist="$mntdir$dir $dirlist"
done

# Symbollically link directories into /tmp/bosinst/usr/lib so $mntdir/usr/lib
# looks like /usr/lib.
ln -s /usr/lib/boot /usr/lib/drivers /usr/lib/methods /usr/lib/nls \
		$temp/usr/lib

# Then mount $libs and $ccslibs over $mntdir/usr/lib and $mntdir/usr/ccs/lib
# so that we get the stripped libraries and the pretest diagnostics ODM.
mount $libs $mntdir/usr/lib
dirlist="$mntdir/usr/lib $dirlist"
mount $ccslibs $mntdir/usr/ccs/lib
dirlist="$mntdir/usr/ccs/lib $dirlist"
# If diagnostics is installed, mount the library directory
if [ -s /usr/lpp/diagnostics/bin/dskpt.dep ]
then
	mount $diaglibs $mntdir/usr/lpp/diagnostics/lib
	dirlist="$mntdir/usr/lpp/diagnostics/lib $dirlist"
fi
	#
	# Make backup image.
	#
cd $mntdir || { trap '' 0 1 2 15; cleanup 8; }
cat $filelist | sed 's:^/:\./:' | backup -irpf$DEVICE
rc=$?
[ "$rc" -ne 0 ] && { trap '' 0 1 2 15; cleanup 9; }

	#
	# image created exit w/ return code 0
	#
cd $root
trap '' 0 1 2 15
cleanup 0
