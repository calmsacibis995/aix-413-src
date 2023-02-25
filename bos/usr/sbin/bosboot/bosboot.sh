#!/usr/bin/ksh
# @(#)53  1.26.4.39  src/bos/usr/sbin/bosboot/bosboot.sh, bosboot, bos41J, 9510A_all 3/2/95 08:45:22
#
# COMPONENT_NAME: (BOSBOOT) Base Operating System Boot
#
# FUNCTIONS: bosboot.sh
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
# NAME: bosboot.sh
#
# FUNCTION: This command handles the Base Operating System boot process
#	that interfaces with the machine boot ROS (Read Only Storage)
#	EPROM. It is capable of creating two fundamental types of boot
#	images: an AIX boot image or a ROS EMulation boot image.
#
#	AIX boot images ------------------------------------------------
#
#	An AIX boot image is created from a RAM (Random Access
#	Memory) disk file system and a kernel. This boot image is
#	transferred to a particular media that the ROS boot code
#	recognizes. ROS will load the image from the media into memory
#	whereupon ROS will transfer control to the images kernel. The
#	associated RAM disk file system contains device configuration
#	routines that are executed in order to make available the
#	booted machines devices and file systems.
#
#	The boot image itself varies for each type of platform and the
#	type of device booted and is by default compressed to fit on
#	certain media and to lessen real memory requirements. The RAM
#	disk file system contains differing configuration files depending
#	upon the platform and the boot device. A mkfs proto file is
#	supplied for each type of platform and boot device. Currently
#	supported devices are disk, cdrom, tape, and network. If a network
#	device is used, the boot image will be read from a server that
#	supports the bootp protocol, and the boot image will be
#	transferred over a LAN. This script is intended to be compatible
#	with future network devices because network boot images are
#	created by the logic in the default cases of the case statements.
#	The main requirement for network boot image creation is that a
#	valid proto file exists in the /usr/lib/boot/network directory,proto
#	and that it be name prefix.proto, where prefix is a valid ODM
#	prefix, i.e, ent for ethernet, or tok for token ring.  The
#	complete boot image may also be uncompressed which saves on the
#	time to boot the system.
#
#	Besides creating a boot image, the bosboot command will save base
#	device configuration data for disk.
#
#	ROS Emulation boot images ---------------------------------------
#
#	A ROS Emulation boot image is created from a file containing the
#	ROS Emulation code. This image is used to provide network boot
#	capability to machines whose ROS does not provide it. A ROS Emulation
#	boot image is transferred to a diskette, tape, or boot logical volume.
#	ROS will load the image from the media into memory whereupon ROS
#	will transfer control to the ROS Emulation code. This code emulates
#	the later version ROS code that supports network boot.
#
#	Note:	This command requires su privileges. Do not reboot the
#		machine if bosboot fails with a message not to do so
#		while creating a boot disk. The	problem should be
#		resolved and bosboot run to successful completion.
#
#		Also, although the -w option is supported for tape (and
#		diskette), network devices do not need this option and
#		hard disk is not currently supported.
#
# SYNTAX:	Usage:
#
#	bosboot -a [ -d device ] [-U] [ -p proto ] [ -k kernel ] [ -l lvdev ]
#		   [ -b file ] [-D|-I] [-L] [-M serv|norm|both] [-q].
#
#	bosboot -w file [-d device] [-q].
#
#	bosboot -r file [ -d device ] [ -l lvdev ] [ -M serv|norm|both ] [-q].
#
#		Where:
#		-a		Create complete boot image and device.
#		-w file		Copy given boot image file to device.
#		-r file		Create ROS Emulation boot image.
#		-d device	Device for which to create the boot image.
#		-U		Create uncompressed boot image.
#		-p proto	Use given proto file for RAM disk file system.
#		-k kernel	Use given kernel file for boot image.
#		-l lvdev	Target boot logical volume for boot image.
#		-b file		Use given file name for boot image name.
#		-D		Load Low Level Debugger.
#		-I		Load and Invoke Low Level Debugger.
#		-L		Enable MP locks instrumentation (MP kernels)
#		-M norm|serv|both	Boot mode - normal or servic
#		-O offset	boot image offset for CDROM file system.
#		-q		Query size required for boot image.
#
#
# EXAMPLES:
#		Create a bootimage on a default boot logical volume on hdisk0.
#	bosboot -ad /dev/hdisk0
#
#		Create an uncompressed boot image for boot disk.
#	bosboot -aU
#
#		Create compressed fs and boot image file for ethernet boot.
#	bosboot -ad /dev/ent0
#
#		Write a given tape boot image /tmp/tape.image to tape device.
#	bosboot -d/dev/rmt0 -w /tmp/tape.image
#
#		Create a ROS Emulation boot image on a diskette
#	bosboot -r /usr/fred/rosemu -d /dev/fd0
routine=bosboot

#
# Define external commands.
#
compress=/usr/bin/compress	# Kernel compression program
expand=/usr/sbin/bootexpand	# Kernel expansion code
savebase=/usr/sbin/savebase	# Base configuration save command
bootinfo=/usr/sbin/bootinfo	# Boot information command
mkram=/usr/sbin/mkram		# RAM disk compression program
mkboot=/usr/sbin/mkboot		# Boot image creation program
rm=/usr/bin/rm			# to avoid any aliases
chdev=/usr/sbin/chdev
update_proto=/usr/lib/boot/update_proto
sort=/usr/bin/sort
cat=/usr/bin/cat

#
# Define constants
#
MESSAGE="dspmsg -s 150 $routine.cat"
BOOTFILES=/usr/lib/boot

#
# Define variable defaults.
#
ORIGIN=${ODMDIR:-/etc/objrepos}	# source objrepos directory
unset RAMFS_ODMDIR		# ram filesystem objrepos directory
OBJDIR=/tmp/boot_IBM		# Default temporary object directory
action=n			# Action flag required argument
device=n			# Device required argument
mode=both			# Default boot mode option
rose=n				# Default ROS Emulation option
norm=				# Default mkboot normal record option
serv=				# Default mkboot service record option
save_b=				# Default mkboot to update save base fields
boot_lv=			# Default mkboot boot logical volume option
wrtblv=				# Default mkboot write to blv option
boothdr=			# Default mkboot boot header option
rosemu=				# Default mkboot ROS Emulation option
memsize="$bootinfo -r"		# Determine real memory size
set_start=			# CDROM set bootrecord bootimage start location
offset=2			# CDROM set bootrecord bootimage start location
bootsize=			# size of boot image
warnmsg=			# Print out warning message.
doverify=			# verify flag
platform=`$bootinfo -T`		# target platform of bootimages
softROS=/usr/lib/boot/aixmon	# rspc
mkbout=/tmp/mkb${RANDOM}.out	# Temporary file for mkboot output
ddout=/tmp/ddblv${RANDOM}.out	# Temporary file for dd output
imagename=`$MESSAGE 70 'bootimage'` # Default boot image name
#
# Define default files.
#
filesys=/tmp/Bootram.fs		# Default boot logical file system
bootimage=			# Default boot logical volume image
proto=				# Default boot logical volume proto
kernel=/unix			# Default kernel
blvdev=/dev/hd5			# Default boot logical volume device
outimage=			# output image file or device
temp_image=/tmp/..b_image	# Temporary boot image
temp_fs=/tmp/..b_fs		# Temporary RAM file system
empty_fs=/tmp/..b_efs		# Temporary empty RAM file system
dskt_im=/tmp/dskt.bootimage	# Default diskette boot image
tape_im=/tmp/tape.bootimage	# Default tape boot image
cd_im=/tmp/cd.bootimage		# Default cdrom boot image
dskt_p=dskt.proto	# Default diskette prototype
tape_p=tape.proto	# Default tape prototype
cd_p=cd.proto	# Default cdrom prototype
disk_p=disk.proto	# Default disk prototype
tmp_p=/tmp/proto.tmp		# Temporary proto file
pentry=/tmp/pentry.tmp		# Temporary proto entry file
pentry_unsort=/tmp/pentry.unsort.tmp	# Temporary unsorted proto entry file
protoext=$BOOTFILES/protoext	# proto extension files directory
tmpimage=			# tmp file to determine output image file system
#
# Set boot creation flags.
#
dofile=n			# Write boot image to a file
docopy=n			# Copy given image to device
docomp=y			# Create compressed boot image (default)
dolld=				# Create loadable/invokable LL debugger
doquery=n			# Estimate disk space required
dolockinstr=			# Enable MP locks instrumentation

# Function: display usage.
usage () {
$MESSAGE 2 '\nusage:\t%s -a [-d device] [-p proto] [-k kernel] [-l lvdev]\n\t\t   [-b file] [-M serv|norm|both] [-D|-I] [-LOUq]\n\t%s -w file [-d device] [-q]\n\t%s -r file [-d device] [-l lvdev] [-M serv|norm|both] [-q]\n' $routine $routine $routine >&2
$MESSAGE 3 '\tWhere:\n' >&2
$MESSAGE 9 '\t-a		Create boot image and write to device or file.\n' >&2
$MESSAGE 8 '\t-w file		Copy given boot image file to device.\n' >&2
$MESSAGE 105 '\t-r file		Create ROS emulation network boot image.\n' >&2
$MESSAGE 24 '\t-d device	Device for which to create the boot image.\n' >&2
$MESSAGE 6 '\t-U		Create uncompressed boot image.\n' >&2
$MESSAGE 14 '\t-p proto	Use given proto file for RAM disk file system.\n' >&2
$MESSAGE 16 '\t-k kernel	Use given kernel file for boot image.\n' >&2
$MESSAGE 17 '\t-l lvdev	Target boot logical volume for boot image.\n' >&2
$MESSAGE 18 '\t-b file		Use given file name for boot image name.\n' >&2
$MESSAGE 19 '\t-D		Load Low Level Debugger.\n' >&2
$MESSAGE 20 '\t-I		Load and Invoke Low Level Debugger.\n' >&2
$MESSAGE 21 '\t-M norm|serv|both	Boot mode - normal or service.\n' >&2
$MESSAGE 22 '\t-O offset	boot image offset for CDROM file system.\n' >&2
$MESSAGE 23 '\t-q		Query disk space required to create boot image.\n' >&2
$MESSAGE 25 '\t-L		Enable MP locks instrumentation.\n' >&2
}

#	This program calls a number of functions
#	dependent upon the action options or arguments
#	and the given device. The functions are grouped
#	into high-level and low-level functions:
#
#	High-level Functions:
#		1.Create_Ramfs()	- create the RAM file system image
#		2.Create_BootImage()    - create kernel + ram fs image
#		3.Create_ROSE_BootImage()   - create ROS Emulation boot image
#		4.Copy_Device()		- copy boot image to device
#		5.Copy_GivenImage()     - copy given boot image to device
#
#	Low-level Functions:
#		0. Cleanup files
#		1. Working room test
#		2. Initialize predefine db
#		3. Strip libraries
#		4. Valid proto file test
#		5. mkfs proto
#		6. Strip kernel
#		7. Set mkboot flags
#		8. Call mkram
#		9. Call mkboot with compress
#		10. Call cat fs
#		11. Call mkboot image
#		12. Calculate boot image size
#		13. Copy boot image to device
#		14. Call savebase
#
# Begin defining state functions.
#

# Function: cleanup
cleanup () {
# Cleanup function.  May be called during normal execution with the r flag,
# at abnormal exit with the a flag, or at normal exit with the n flag
	trap '' 1 2 15
	cd /
	${rm} -f /tmp/librpcsvc.a /tmp/libodm.a /tmp/liblvm.a /tmp/libcfg.a
	${rm} -f /tmp/libbsd.a /tmp/libsrc.a /tmp/libs.a
	${rm} -f $mkbout $ddout $tmp_p $pentry $pentry_unsort
	${rm} -fr $OBJDIR > /dev/null 2>&1
	if [ "$1" = "-a" ]	# abnormal exit cleanup mode
	then
		[ "$docopy" = y -o "$bootimage" = /dev/null ] && \
			bootimage=
		${rm} -f $filesys /tmp/shr.o $bootimage $tmpimage
	elif [ "$1" = "-n" ]	# normal exit cleanup
	then
		[ "$docopy" = y -o "$bootimage" = /dev/null -o "$dofile" = y ] && \
			bootimage=
		case $dev_prefix in
			rmt | fd );;
			*) bootimage= ;;
		esac
		${rm} -f $filesys $bootimage
	fi
	if [ "$1" != "-r" ]
	then
		${rm} -f $temp_fs $skernel $empty_fs $temp_image
	else
		unset -f valid_dev check_diskspace create_db \
		strip_lib check_proto mk_ramfs
	fi
} # End of cleanup

# Function: check_diskpace
check_diskspace () {
# Test for enough file space to create boot image.
# R = RAM file sysytem size, K = kernel size, E = boot expand size
# for compressed boot image, peak useage in /tmp:
#	MAX(R/2 + 2K, R + K/2) (write to device directly)
#	MAX(R/2 + 2K, R + K + E) (if image in /tmp)
#	MAX(R/2 + 2K, R + K/2) (if image in other file system) +
#		R/2 + K/2 + E space in other file system
# for uncompressed boot image, peak useage in /tmp:
#	R + 2K (write to device directly)
#	2R + 2K (if image in /tmp)
#	R + 2K (if image in other file system) +
#		R + K space in other file system
#
	trap 'error_func 0' 1 2 15
	if [ "$rose" = y ]
	then
		# ROS Emulation
		ramfsize=0
	else
		# check for existence of proto file
		# df returns 1kb sizes.
		# proto uses 512b sizes so convert to 1kb size.
		[ ! -s $proto ] && error_func 41 $proto
		[ ! -s $kernel ] && error_func 40 $kernel
		ramfsize=`fgrep bootrec $proto`
		ramfsize=`echo $ramfsize | cut -f2 -d' '`
		(( ramfsize /= 2 ))
	fi
	kernsize=`ls -sL $kernel`
	kernsize=`echo $kernsize| cut -f1 -d ' '`

	# determine the file system of /tmp
	tmpfs=`export LC_MESSAGES=C; df -k /tmp 2>/dev/null`
	tmpfs=`echo $tmpfs | cut -f15 -d' '`

	# determine file system of the bootimage file
	outfs=
	[ "$bootimage" -a "$bootimage" != "/dev/null" ] && {
		tmpimage=$bootimage._$RANDOM_  # tmp file
		touch $tmpimage || error_func 62 $imagename
		fsinfo=`export LC_MESSAGES=C; df -k $tmpimage 2>/dev/null`
		${rm} -f $tmpimage
		outfs=`echo $fsinfo | cut -f15 -d' '`
	}

	# determine disk space needed
	if [ "$rose" = y ]	# ROS Emulation
	then
		if [ "$bootimage" ]
		then
			(( imagesize = kernsize + 1 ))
		else
			imagesize=0
		fi
		outfs=
	elif [ "$docomp" = y ] # compressed image
	then
		expndsize=`ls -sL $expand`
		expndsize=`echo $expndsize | cut -f1 -d' '`
		(( imagesize= ramfsize + kernsize/2 ))
		(( imagesize2 = ramfsize/2 + 2 * kernsize ))
		case "$outfs" in
			"") ;;
			"$tmpfs") ((imagesize = ramfsize + kernsize + expndsize )) ;;
			*) ((outsize = ramfsize/2 + kernsize/2 + expndsize ));;
		esac
		if [ "$imagesize2" -gt "$imagesize" ]
		then
			imagesize=$imagesize2
		fi
	else		# uncompressed image
		(( imagesize= ramfsize + 2 * kernsize ))
		case "$outfs" in
			"");;
			"$tmpfs") (( imagesize = imagesize + ramfsize ));;
			*) (( outsize = ramfsize + kernsize ));;
		esac
	fi

	[ "$doquery" = y ] && {
		error_func 55
		error_func 58 $tmpfs $imagesize
		[ "$outfs" ] && [ "$outfs" != "$tmpfs" ] && \
			error_func 58 $outfs $outsize
		return
	}

	# Check /tmp space
	freeland=`export LC_MESSAGES=C; df -k $tmpfs`
	freeland=`echo $freeland | cut -f11 -d' '`
	[ "$imagesize" -gt "$freeland" ] && \
		error_func 2 $imagename $tmpfs $freeland $imagesize
	# Check other file system space
	[ "$outfs" ] && [ "$outfs" != "$tmpfs" ] && {
		freeland=`echo $fsinfo | cut -f11 -d' '`
		[ "$outsize" -gt "$freeland" ]\
			&& error_func 2 $imagename $outfs $freeland $imagesize
	}
} # End of check_diskspace

# Function: create_db
create_db () {
#
# Create a device database to go with the boot image
#
	trap 'error_func 0' 1 2 15
	mkdir $OBJDIR
	cd $OBJDIR
	# remove runtime from PdAt
	cp $ORIGIN/PdAt* $OBJDIR
	ODMDIR=$OBJDIR odmdelete -q"uniquetype=runtime" -o PdAt >/dev/null
	# don't need printers to boot
	ODMDIR=$OBJDIR odmdelete -q"uniquetype like printer*" -o PdAt >/dev/null
	# save remaining PdAt
	ODMDIR=$OBJDIR odmget PdAt > $OBJDIR/pdat.add

	# Create databases.
	ODMDIR=$OBJDIR /usr/bin/odmcreate -c /usr/lib/cfgodm.ipl >/dev/null 2>&1

	# get Config_Rules
	ln -fs $ORIGIN/Config_Rules $OBJDIR
	ODMDIR=$ORIGIN odmget -q"resource=ddins and value3=;" CuDvDr | \
	ODMDIR=$OBJDIR odmadd

	# get PdDv and PdCn
	ln -fs $ORIGIN/PdDv $OBJDIR/PdDv
	ln -fs $ORIGIN/PdDv.vc $OBJDIR/PdDv.vc
	ln -fs $ORIGIN/PdCn $OBJDIR/PdCn
	ODMDIR=$OBJDIR odmadd $OBJDIR/pdat.add
	rm -f $OBJDIR/pdat.add

	cd /
} # End of create_db

# Function strip_lib
strip_lib () {
# Strip required libraries
#
	trap 'error_func 0' 1 2 15
	cd /tmp
	#
	# Housekeep and strip libraries.
	#
	for blibs in librpcsvc.a libodm.a liblvm.a libcfg.a \
		libbsd.a libsrc.a
	do
		[ ! -s /usr/lib/"$blibs" ] && continue
		${rm} -f $blibs
		ar x /lib/$blibs shr.o
		strip shr.o >/dev/null
		ar cq $blibs shr.o
		${rm} -f shr.o
	done
} # End of strip_lib

# Function build_proto
build_proto () {
# build a proto file for RAM disk.
	trap 'error_func 0' 1 2 15
	base=${proto##*/}	# remove preceeding dir path
	[[ $base = ${platform}.* ]] && base=${base#*\.}	# remove platform prefix
	[ ! -s $protoext/${platform}.pcfg ] && error_func 40 ${platform}.pcfg
	>$pentry_unsort		# clean up pentry_unsort file
	for grp in `$cat $protoext/${platform}.pcfg`
	do
	    $cat $protoext/${base}.ext.${grp}* >> $pentry_unsort 2>/dev/null
	done
	$cat $pentry_unsort | $sort -u > $pentry

	$update_proto $pentry $proto > $tmp_p
	[ $? -ne 0 ] && error_func 68
	proto=$tmp_p	# let proto to new one
} # End of build_proto

# Function check_proto
check_proto () {
# Test availability of RAM files.
	trap 'error_func 0' 1 2 15
	awk '$NF ~ /^\// && $2 !~ /^[Ll]/ { print($NF) }' $proto |
	while read pf
	do
		[ ! -s "$pf" ] && error_func 41 $pf
	done
} # End of check_proto

# Function mk_ramfs
mk_ramfs () {
# Create RAM file system with mkfs.
# mkfs requires file to exist.
	trap 'error_func 0' 1 2 15
	cd /
	touch $filesys || error_func 62 $filesys
	touch /bootrec || error_func 62 /bootrec
	echo y | mkfs -V jfs -p $proto $filesys
	rc="$?"; [ "$rc" -ne 0 ] && error_func 42 $filesys
	# now do some run-time cleanup
	cleanup -r
} # End of mk_ramfs

# Function strip_ker
strip_ker () {
# Strip kernel of symbol table to save space.
	trap 'error_func 0' 1 2 15
	skernel=/tmp/${kernel##*/}.strip
	cp $kernel $skernel
	rc="$?"; [ "$rc" -ne 0 ] && error_func 43 $kernel
	strip -Kernel $skernel >/dev/null
} # End of strip_ker

# Function set_mkbootflags
set_mkbootflags () {
# Set flags required for mkboot.
	trap 'error_func 0' 1 2 15
	ll_deb=$dolld
	lock_instr=$dolockinstr
	outimage=$bootimage

	[ "$rose" = y ] && {
		rosemu=-r
		boothdr=-h
	}
	case "$mode" in
		( n* | N* )  norm=-i;;
		( s* | S* )  serv=-s;;
		( b* | B* )  norm=-i
			     serv=-s;;
		* )	mode=both
			norm=-i
			serv=-s;;
	esac

	case "$dev_prefix" in
		hdisk )
			boot_lv="-l $blvdev"
			[ ! "$bootimage" ] && {
				wrtblv=-w
				outimage=$blvdev
			} || {
				norm=
				serv=
			}
			;;
		rmt)	save_b=-b;;
		cd)	save_b=-b;;
		fd)	save_b=-b;;
		*)	# network devices
			save_b=-b
			case $platform in
				# rspc systems do not like the bootrecord
				#	on network boot images
				rspc)	norm=
					serv=
					;;
			esac
			;;
	esac

	#
	# platform dependent processing
	#
	case $platform in
		rspc) PLATFORM_SPECIFIC_ARGS="-A $softROS"
			;;
		*)	PLATFORM_SPECIFIC_ARGS=
			;;
	esac

} # End of set_mkbootflags

# Function compress_ramfs
compress_ramfs () {
# Compress RAM file system.
	trap 'error_func 0' 1 2 15
	$mkram $filesys		# compress in place
	mv $filesys $temp_fs	# save in temp_fs
} # End of compress_ramfs

# Function mkboot_compress
mkboot_compress () {
# Call mkboot and compress boot image (kernel essentially) .
	trap 'error_func 0' 1 2 15
	touch $empty_fs || error_func 62 $empty_fs
	ODMDIR=${RAMFS_ODMDIR-$ORIGIN} $mkboot -d $device $ll_deb $save_b \
	$lock_instr -k $skernel -f $empty_fs | $compress > $temp_image
	rc="$?"; [ "$rc" -ne 0 ] && error_func 45 $temp_image
	${rm} -f $empty_fs $skernel
	filesys=$temp_image
} # End of mkboot_compress

# Function cat_ker_ramfs
cat_ker_ramfs () {
# Call cat to combine compressed RAM file system with kernel.
	trap 'error_func 0' 1 2 15
	cat $temp_fs >> $filesys
	${rm} $temp_fs
} # End of cat_ker_ramfs

# Function mkboot_image
mkboot_image () {
# Call mkboot to create an image.  Needs 2 arguments.
# for  uncompressed image:  $1=kernel,  $2=ram fs.
# for  compressed image:    $1=bootexpand, $2=kernel+ramfs.
#
	trap 'error_func 0' 1 2 15
	[ ! -s "$1" -a ! -r "$2" ] && error_func 46 "$1" "$2"
	# From now on, output "Do not boot" warning on error
	warnmsg=y
	ODMDIR=${RAMFS_ODMDIR-$ORIGIN} $mkboot -d $device $ll_deb $save_b \
	$lock_instr -k $1 $boothdr $rosemu -f $2 $norm $serv $boot_lv \
	$set_start $wrtblv $PLATFORM_SPECIFIC_ARGS 2>$mkbout | \
	dd ibs=1b obs=64b of=$outimage 2>$ddout
	rc=$?
	[ -s $mkbout ] && {  # check for mkboot error
		cat $mkbout
		error_func 45 $imagename
	}
	[ $rc -ne 0 ] && {   # check for dd error
		cat $ddout
		error_func 48 $imagename
	}
} # End of mkboot_image

# Function cal_size
cal_size () {
# Calculate and display boot image size.
	trap 'error_func 0' 1 2 15
	if [ ! -s $ddout -a "$bootimage" ]
	then
		# boot image is given
		bootsize=`ls -l $bootimage | awk '{printf("%d",$5/512)}'`
	else
		# $ddout contains the image size
		cat $ddout | read a b rest
		[ "$a" = "dd:" ] && a=$b  # adjust for NLS
		bootsize=`echo $a | awk -F+ '{print ($1+$2)}'`
	fi
	error_func 51 $bootsize
} # End of cal_size

# Function copy_target
copy_target () {
# Copy boot image to device.
	trap 'error_func 0' 1 2 15
	case "$dev_prefix" in
		rmt )	# Configure tape for 512 byte blocks.
			# Do not change the tape if block size is
			# already 512 (reconfiguring causes tape rewind).

			# drop trailing dot and digits from tape dev name
			ldevice=${dev_basename%%*(.*([0-9]))}
			CMD="lsattr -E -O -a block_size"
			BSIZE=`$CMD -l $ldevice | grep -v block_size`
			if [ "$BSIZE" -ne 512 ]
			then
				${chdev} -l $ldevice \
					-a block_size=512 >/dev/null 2>&1
			fi

			dd conv=sync if=$bootimage of=$device > /dev/null 2>&1
			rc="$?"; [ "$rc" -ne 0 ] && error_func 48 $device

			# See if a no rewind device was used if so
			# then do not reconfigure the tape device
			# back to the original block size as this
			# will cause rewind.

			# drop all but any digits following the last dot
			norewind="${device##*.}"

			if [ "$BSIZE" != 512 -a "$norewind" != 5 -a \
				"$norewind" != 1 ]
			then
				${chdev} -l $ldevice \
					-a block_size=$BSIZE >/dev/null 2>&1
			fi
			;;
		fd )	# Copy for diskette
			dd ibs=1b obs=90b conv=sync if=$bootimage \
			of=$device > /dev/null 2>&1
			rc="$?"; [ "$rc" -ne 0 ] && error_func 48 $device
			;;
		* )	# mv for anything else
			;;
	esac
} # End of copy_target

# Function call_sb
call_sb () {
# Call savebase.
	trap 'error_func 0' 1 2 15
	$savebase -o $ORIGIN -d $device -m $mode
	rc="$?"; [ "$rc" -ne 0 ] && error_func 49 $device
} # End of call_sb

#
# Finish defining low-level functions.
#

# Function: error handler.
error_func () {
case "$1" in
	1 )	$MESSAGE 1 '\n0301-150 %s: Invalid or no boot device specified!' \
			$routine >&2
		[ ! "$2" ] && usage || $MESSAGE 31 '\n\t %s \n' "" >&2
		exit $1
		;;
	2 )	$MESSAGE 30 '\n0301-152 %s: not enough file space to create:' \
			$routine >&2
		$MESSAGE 31 '\n\t %s \n' $2 >&2
		$MESSAGE 32 '\t %s has %s free KB.\n' $3 $4 >&2
		$MESSAGE 33 '\t %s needs %s KB.\n' $2 $5 >&2
		exit $1
		;;
	4 )	$MESSAGE 66 '\n0301-174 %s: Invalid device %s specified!\n' $routine $2 >&2
		exit $1
		;;
	40 )	$MESSAGE 40 '\n0301-153 %s: %s not found.\n' \
			$routine $2 >&2
		;;
	41 )	$MESSAGE 41 '\n0301-154 %s: missing proto file: %s\n' \
			$routine $2 >&2
		;;
	42 )	$MESSAGE 42 '\n0301-155 %s: mkfs failed for %s.\n' \
			$routine $2 >&2
		;;
	43 )	$MESSAGE 43 '\n0301-156 %s: cp of %s failed.\n' \
			$routine $2 >&2
		;;
	44 )	$MESSAGE 44 '\n0301-157 %s: strip of %s failed.\n' \
			$routine $2 >&2
		;;
	45 )	$MESSAGE 45 '\n0301-158 %s: mkboot failed to create %s.\n' \
			$routine $2 >&2
		;;
	46 )	$MESSAGE 46 '\n0301-159 %s: Either %s or %s is missing!\n' \
			$routine $2 $3 >&2
		;;
	48 )	$MESSAGE 48 '\n0301-161 %s: dd failed to copy %s.\n' \
			$routine $2 >&2
		;;
	49 )	$MESSAGE 49 '\n0301-162 %s: savebase failed with %s.\n' \
			$routine $2 >&2
		;;
	51 )	$MESSAGE 51 '\n%s: Boot image is %s 512 byte blocks.\n' \
			$routine $2
		return
		;;
	52 )	$MESSAGE 52 '\n0301-166 %s: Device not ready: %s.\n' \
			$routine $2 >&2
		exit 2
		;;
	54 )	$MESSAGE 54 '\n0301-168 %s: The current boot logical volume, %s,\n\tdoes not exist on %s.\n' $routine $2 $3 >&2
		exit $1
		;;
	55 )	$MESSAGE 55 '\nFilesystem\t'
		$MESSAGE 56 'KB required\n'
		return
		;;
	58 )	$MESSAGE 58 '%s\t\t%s\n' $2 $3
		return
		;;
	60 )	$MESSAGE 60 '\n0301-170 %s: -w is invalid for device %s.\n' $routine $2 >&2
		exit $1
		;;
	62 )	$MESSAGE 62 '\n0301-172 %s: %s is not writable.\n' $routine $2 >&2
		exit $1
		;;
	64 )	$MESSAGE 64 '\n%s: Memory or boot logical volume size is too small.\n\t Bootimage will be compressed.\n' $routine
		return
		;;
	68 )	$MESSAGE 68 '\n0301-176 %s: update_proto failed to create a proto file.\n' $routine >&2
		;;
	106 )	$MESSAGE 106 '\n0301-175 %s: -r is invalid for device %s.\n' $routine $2 >&2
		exit $1
		;;
	* )	# Issue warning.
		;;

esac
[ "$doquery" = y ] && exit $1
[ "$warnmsg" = y ] && \
$MESSAGE 100 '\n0301-165 %s: WARNING! %s failed - do not attempt to boot device.\n' \
	$routine $routine >&2
cleanup -a
exit $1
}

# Function: check for valid physical boot device.
valid_dev () {
	trap 'error_func 0' 1 2 15
	device=$1

	[ "$device" = "/dev/ipldevice" -o "$device" = "ipldevice" ] && {
		# convert link to file name
		ls -i /dev/ipldevice | read inode_num trash
		device=`ls -i /dev |
			awk '$1==INODE && $2 != "ipldevice" \
			{print $2; exit}' INODE=$inode_num`
		# now device looks like rhdisk0, so we need to
		# strip off that pesky leading 'r'
		device=${device#r}
	}

	dev_basename=${device##*/}	# get rightmost pattern
	device=/dev/"$dev_basename"	# form absolute pathname
	dev_prefix=${dev_basename%%*([0-9]|[0-9].)} # strip trailing digits

	# diskettes are no longer a valid type, except for ROS Emulation
	[ "$dev_prefix" = "fd" -a "$rose" = n ] && error_func 4 $device

	# look for this prefix in PdDv
	[ "$dev_prefix" != "null" ] && {
		ODMDIR=$ORIGIN odmget -q prefix=$dev_prefix PdDv 2>/dev/null |
		fgrep $dev_prefix >/dev/null 2>&1
		[ $? -ne 0 ] && error_func 1
	}

	[ -w "$device" -a "$doquery" != y ] && {
		# check to see if media is ready
		case "$dev_prefix" in
			rmt )	# tape device
				2>/dev/null >"$device" || \
					error_func 52 "$device";;
			fd )	# floppy diskette
				2>/dev/null >"$device" || \
					error_func 52 "$device";;
		esac
	}

	# verify that the blvdev is on the device if hdisk specified
	[ "$dev_prefix" = hdisk ] && {
	# check whether booted from hard disk or something else
	[ `$bootinfo -t` != 1 -a -s "/../BOOTFS_COOKIE" ] && {
		# did NOT boot from hard disk
		RAMFS_ODMDIR=/../etc/objrepos
		search_str=${device##*[/r]}

		# read pvid from ram fs ODM
		pvid=`ODMDIR=$RAMFS_ODMDIR odmget -q \
			"name=$search_str and attribute=pvid" CuAt |
			awk '$1 ~ /value/ {gsub(/"/,"",$3);print $3}'`

		# match pvid to diskname from hard disk ODM
		disk_odm_devname=`ODMDIR=$ORIGIN odmget -q "value=$pvid" CuAt |
			awk '$3 ~ /hdisk/ {gsub(/"/,"",$3);print $3}'`
		# if the device name differs between the ramfilesystem and
		# the disk filesystem ODM's, then save the ramfs device name
		[[ "$disk_odm_devname" != "$search_str" ]] && {
			disk_odm_devname=/dev/$disk_odm_devname
		} || {
			unset disk_odm_devname
		}
	}
	target_lv=${blvdev##*/}
	blv_place=`export LC_MESSAGES=C; lslv -l $target_lv 2>/dev/null |
			awk '$1 ~ /^hdisk/ {print $1; exit}'`
	if [ -n "$disk_odm_devname" ]
	then
		[[ "$disk_odm_devname" != /dev/*${blv_place} ]] && \
				error_func 54 $blvdev $device
	else
		[[ "$device" != /dev/*${blv_place} ]] && \
				error_func 54 $blvdev $device
	fi
	}
}	# End of valid_dev

#
# High-Level bosboot functions
#
# Function: Create_Ramfs
Create_Ramfs() {
#create a RAM file system from a proto file
	trap 'error_func 0' 1 2 15
	create_db		# initialize database
	strip_lib		# strip libraries
	build_proto		# build a proto file
	check_proto		# check files in proto
	[ "$doverify" = y ] && {
		cleanup -r	# clean and exit now
		exit 0
	}
	mk_ramfs		# make ram file system
} # End of Create_Ramfs

# Function: Create_BootImage
Create_BootImage() {
#create a boot image with kernel and ram fs
	trap 'error_func 0' 1 2 15
	# note: ram fs is compressed first
	# to reduce disk space usage in /tmp.
	[ "$docomp" = y ] && compress_ramfs
	strip_ker		# strip kernel
	set_mkbootflags		# set mkboot flags
	original_device="${device}"	# save the device name
	if [ "${dev_prefix}" = "rmt" ]
	then	# need to strip off the trailing dot and digit if they exist
		device=${original_device%.*([0-9])}
	fi
	if [ "$docomp" = y ]	# if compressed boot image,
	then			# then
		mkboot_compress	# compress kernel
		cat_ker_ramfs	# put compress kernel and compress ram together
		mkboot_image $expand $filesys	# prefix boot expand
	else
		mkboot_image $skernel $filesys	# uncompress image
	fi
	device="${original_device}"	# restore the device name
	cal_size		# output boot image size in 512 blocks
} # End of Create_BootImage

# Function: Create_ROSE_BootImage
Create_ROSE_BootImage() {
#create a ROS Emulation boot image
	trap 'error_func 0' 1 2 15
	set_mkbootflags			# set mkboot flags
	mkboot_image $kernel $filesys	# uncompress image
	cal_size			# output boot image size in 512 blocks
} # End of Create_ROSE_BootImage

# Function: Copy_Device
Copy_Device() {
# hdisk: no need to copy (already written to blv).
# tape and diskette: need to copy to the target device
# others: no need to copy (bootimage is built as the target file).
#
	trap 'error_func 0' 1 2 15
	case "$dev_prefix" in
		hdisk )
			[ ! "$bootimage" -a "$rose" = n ] && call_sb
			;;
		rmt | fd )
			[ "$dofile" = n ] && copy_target
			;;
		*)	# No need to copy for cd, net and other devices
			;;
	esac
} # End of Copy_Device

# Function: Copy_GivenImage
Copy_GivenImage () {
# if query, no disk space needed
# hdisk, cd, net devices : not allowed to copy given image.
#
	trap 'error_func 0' 1 2 15
	[ "$doquery" = y ] && {
		error_func 55
		exit 0		# no disk space needed
	}
	case "$dev_prefix" in
		fd | rmt )
			cal_size
			copy_target ;;
		*)
			error_func 60 $device;;
	esac
	exit 0
} # End of Copy_GivenImage
# Finish defining High-Level Functions

#
#  Main
#
unalias ar awk cat cd cp cut dd df echo export fgrep getopt ls \
	lsattr lslv mkfs mv odmget odmadd odmcreate read strip touch trap
#
set +o noclobber	# unset noclobber option
export PATH=/usr/bin:/etc:/usr/sbin
#
# if default kernel /unix does not exist, use /usr/lib/boot/unix as default.
[ "`ls $kernel 2>/dev/null`" != "$kernel" ] && kernel=/usr/lib/boot/unix
#
export ODMDIR=$ORIGIN

set -- `getopt ab:d:DIk:M:O:Ll:p:qUvw:r:T: $*`

if [ $? != 0 ]
then
	usage
	exit 1
fi

while [ $1 != -- ]
do
	case $1 in

		-a)	[ "$action" = y ] && {
				usage; exit 1
			}
			action=y
			shift;;
		-b)	[ "$bootimage" ] && {
				usage; exit 1
			}
			bootimage=$2	# Boot image file name
			[[ "$bootimage" != /* ]] && bootimage=`pwd`/$bootimage
			[ "$bootimage" != /dev/null -a \
				\( -c $bootimage -o -b $bootimage \) ]	&&\
				error_func 4 $bootimage
			dofile=y
			shift;shift;;
		-U)	docomp=n	# Do not Compress boot image
			shift;;
		-d)	device=$2	# Target boot device
			shift;shift;;
		-D)	[ "$dolld" ] && {
				usage; exit 1
			}
			dolld=-D	# Load LL Debugger
			shift;;
		-I)	[ "$dolld" ] && {
				usage; exit 1
			}
			dolld=-I	# Invoke LL Debugger
			shift;;
		-k)	kernel=$2	# Kernel file name
			[[ "$kernel" != /* ]] && kernel=`pwd`/$kernel
			shift;shift;;
		-M)	mode=$2		# Boot mode
			shift;shift;;
		-O)	offset="$2"	# Boot image location
			shift;shift;;
		-L)	dolockinstr=-L	# Enable MP locks instrumentation
			shift;;
		-l)	blvdev=$2	# Target boot logical volume
			# Ensure lv is of type boot
			QSTR="name=${blvdev##*/} and attribute=type and value=boot"
			lvtype=`ODMDIR=$ORIGIN odmget -q"$QSTR" CuAt`
			[ -z "$lvtype" ] && error_func 4 $blvdev
			blvdev=/dev/${blvdev##*/}
			shift;shift;;
		-p)	proto=$2	# Boot proto file
			[[ "$proto" != /* ]] && proto=`pwd`/$proto
			shift;shift;;
		-q)	doquery=y	# Query size
			shift;;
		-w)	docopy=y	# Write boot image to device.
			[ "$action" = y -o "$bootimage" ] && {
				usage; exit 1
			}
			bootimage=$2
			[ ! -f $bootimage ] && error_func 40 $bootimage
			[[ "$bootimage" != /* ]] && bootimage=`pwd`/$bootimage
			action=y
			shift; shift;;
		-v)	doverify=y
			[ "$action" = y ] && {
				usage; exit 1
			}
			action=y
			shift
			;;
		-r)	[ "$action" = y ] && {  # ROS Emulation
				usage; exit 1
			}
			kernel=$2
			[ ! -s $kernel ] && error_func 40 $kernel
			[[ "$kernel" != /* ]] && kernel=`pwd`/$kernel
			action=y
			rose=y
			shift; shift;;
		-T)	platform=$2
			shift;shift;;
		*)	usage; exit 1
	esac
done

# Action flag is required program arguments.
if [ "$action" = n ]
then
	usage
	exit 2
fi

# if no device is named, default to the disk the system booted from
[ "$device" = n ] && {
	[ -s "$proto" ] && error_func 1  # must specify device if proto is given
	if [ `$bootinfo -t` -eq 1 ]  #hdisk boot
	then
		device=/dev/`$bootinfo -b` # get device name
	else
		error_func 1
	fi
}
# if ROS Emulation, check for incompatible options
[ "$rose" = y ] && {
	[ "$bootimage" -o "$dolld" -o "$proto" ] && {
		usage; exit 1
	}
	docomp=n
	filesys=$empty_fs
	touch $filesys
}

# Validate the device
valid_dev $device

#
# Set shell variables as derived from device type and program
# action input arguments.
#
case $dev_prefix in
	fd )	# Diskette
		[ ! "$bootimage" ] && bootimage=$dskt_im
		[ ! "$proto" ] && proto=$BOOTFILES/${platform}.dskt.proto
		;;
	rmt )	# Tape
		[ ! "$bootimage" ] && bootimage=$tape_im
		[ ! "$proto" ] && proto=$BOOTFILES/${platform}.tape.proto
		OBJDIR=/tmp/tapeboot_IBM
		;;
	cd )	# CDROM
		[ "$rose" = y ] && error_func 106 $device
		[ ! "$bootimage" ] && bootimage=$cd_im
		[ ! "$proto" ] && proto=$BOOTFILES/${platform}.cd.proto
		OBJDIR=/tmp/cdboot_IBM
		set_start="-p $offset"
		;;
	hdisk )	# Disk
		# bootimage = NULL means write image directly to blv
		[ ! "$proto" ] && proto=$BOOTFILES/${platform}.disk.proto
		# Force RAM file system compress on 8M machines.
		[ "$rose" = n -a "$docomp" = n ] && \
			[ `${memsize}` -le 8192 -o \
			`$bootinfo -s ${blvdev##*/}` -lt 8 ] && {
				error_func 64
				docomp=y
			}
		;;
	* ) # Network
		[ "$rose" = y ] && error_func 106 $device
		[ ! "$bootimage" ] && bootimage=/tmp/${dev_prefix}.bootimage
		[ -z "$proto" ] && \
			proto=${BOOTFILES}/network/${platform}.${dev_prefix}.proto
		OBJDIR=/tmp/netboot_IBM
		;;
	esac
# Set up imagename for messages
[ "$bootimage" ] && imagename=$bootimage

# Set environment
cd /
trap 'error_func 0' 1 2 15
#
# If copying the given boot image to device, do it here and exit.
[ "$docopy" = y ] && Copy_GivenImage
#
# Build Ram disk file system and boot image
#
check_diskspace		# Check for disk space
[ "$doquery" = y ] && exit 0		# -q option
if [ "$rose" = n ]
then
	Create_Ramfs		# Create a RAM file system
	Create_BootImage	# Create a boot image
else
	Create_ROSE_BootImage	# Create a ROS emulation boot image
fi
Copy_Device		# Copy boot image to target if required
cleanup -n

exit 0
# End of bosboot
