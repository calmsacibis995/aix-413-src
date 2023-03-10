#! /bin/sh
#
#   COMPONENT_NAME: cmdnfs
#
#   FUNCTIONS: mk
#
#   ORIGINS: 24
#
#
#	@(#)MAKEDEV	1.2 90/07/25 4.1NFSSRC Copyr 1990 SMI
#
# Device "make" file.  Valid arguments:
#	std	standard devices
#	local	configuration specific devices
# Tapes:
#	ar*	Archive 1/4" tape
#	tm*	Tapemaster 1/2" tape
#	st*	SCSI (Archive) 1/4" tape
#	xt*	Xylogics 472 1/2" tape
# Disks:
#	fd*	Floppy Disk (Intel 82072)
#	ip*	IPI Disk
#	sd*	SCSI Disk
#	xd*	Xylogics 7053
#	xy*	Xylogics 450 & 451
#	sr0	Generic CD-ROM disk
# Parallel port
#	pp*	Paradise parallel port
# Terminal multiplexors:
#	mcp*	Sun ALM-2 16-line terminal multiplexer
#	mti*	Systech MTI-800A/1600A
#	oct*	Central Data Octal card
#	ttys	First SCSI board UARTS ttys0-ttys3
#	ttyt	Second SCSI board UARTS ttyt0-ttyt3
# Pseudo terminals:
#	pty*	set of 16 master and slave pseudo terminals
# Printers:
#	pp*	Paradise parallel port
#	vpc*	Versatec and Centronics (Systech VPC-2200)
#	vp*	Versatec (Ikon interface)
# Graphics/windows:
#	bwone*	Multibus monochrome frame buffer
#	bwtwo*	Memory monochrome frame buffer
#	cgone*	Multibus color board
#	cgtwo*	VME color board
#	cgthree* Memory color frame buffer
#	cgfour* Memory color frame buffer w/overlay
#	cgsix*	"Lego" accelerated 8 bit color frame buffer
#	cgeight* 24-bit memory color frame buffer w/overlay
#	cgnine* VME 24-bit color frame buffer w/overlay
#	gpone*	VME graphics processor (GP/GP+/GP2)
#	taac*	TAAC-1 applications accelerator
#	win*	(up to) 128 windows
# Misc:
#	audio	Sun-4/60 STREAMS audio driver
#	audioctl Sun-4/60 audio control pseudo-device
#	des	DES chip driver
#	fpa	Sun FPA board
#	pc*	Sun IPC driver
#	sbus	Sun-4/60 SBus interface
#	sky	Sky FPP board
#	sqz	Sun memory squeezer
#

MYPATH=::"/usr/lpp/nfs/sun_diskless"
PATH=::${MYPATH}:/usr/bin:/usr/ucb:/usr/sbin:/etc:/bin:.
umask 77

# mk name b/c major minor [mode] [group]
mk() {
      MINOR_NUMBER=`expr 256 \* $3 + $4`
      mknod $1 $2 0 $MINOR_NUMBER &&
      chmod ${5-666} $1 &&
      test -n "$6" && chgrp $6 $1
      return 0
}


args="$@"
for i in $args
do

unit=`expr $i : '[a-z][a-z]*\([0-9][0-9]*\)'`
case $unit in
"") unit=0 ;;
esac

case $i in

std)
	mk console	c 0 0 622
	mk tty		c 2 0
	mk mem		c 3 0 640 kmem
	mk kmem		c 3 1 640 kmem
	mk null		c 3 2
	mk zero		c 3 12
	mk drum		c 7 0 640 kmem
	mk ttya		c 12 0
	mk ttyb		c 12 1
	mk mouse	c 13 0
	mk klog		c 16 0 600
	mk fb		c 22 0
	mk kbd		c 29 0
	mk nit		c 37 40 600
	mk dump		c 41 0 660 kmem
	mk vd		c 57 0 644

	# for RFS
	mk spx		c 37 35 644	# streams pipe driver
	mk tcp		c 37 63 666	# tcp stream head

	# for CD-ROM
	mk sr0		b 18 0 555
	mk rsr0		c 58 0 555

	# Multibus, Sun-2 only
	mk mbmem	c 3 3 600
	mk mbio		c 3 4 600

	# EEPROM, all machines except Sun-2
	mk eeprom	c 3 11 640 kmem

	# Sun-4/330 only
	mk ttyc		c 12 4
	mk ttyd		c 12 5

	# Sun-4c specific devices
	mk openprom	c 70 0 640 kmem

	# recursive call to make standard floppies/disks
	#   and misc devices with their own lines
	$0 fd0
	$0 sd0 sd1 sd2 sd3 sd4 sd5 sd6
	$0 st0 st1 st2 st3
	$0 sbus audio

	# non-Sun-4c devices

	# 16-bit VMEbus
	mk vme16d16	c 3 5 600
	ln vme16d16 vme16
	mk vme24d16	c 3 6 600
	ln vme24d16 vme24

	# 32-bit VMEbus
	mk vme32d16	c 3 7 600
	mk vme16d32	c 3 8 600
	mk vme24d32	c 3 9 600
	mk vme32d32	c 3 10 600
	ln vme32d32 vme32

	mk des		c 11 0
	;;

ar*)
	case $unit in
	0|1|2|3)
		rew=`expr $unit '*' 4`
		norew=`expr $rew + 16`
		mk rar$unit	c 8 $rew
		mk nrar$unit	c 8 $norew
		;;
	*)
		echo "bad unit number in: $i; use ar0 thru ar3"
		;;
	esac
	;;

mti*)
	case $unit in
	[0-7])
		eval `echo $unit | awk ' { unit = $1; u = 16 * $1 } ; END {
		    for (i = 0; i < 16; i++)
			printf("mk tty%s%x c 10 %d; ",unit,i,u+i); } ;'`
		;;
	*)
		echo "bad unit for mti in: $i, use mti0 thru mti7"
		;;
	esac
	;;

mt*|st*|xt*)
	case $i in
	mt*) chr=5 ;;
	st*) blk=11; chr=18 ;;
	xt*) chr=30 ;;
	esac
	case $unit in
	[0-3]) ;;
	*)
		dev=`expr $i : '\(..\).*'`
		echo "bad unit for tape in: $i; use ${dev}0 thru ${dev}3"
		continue
		;;
	esac
	for x in 0 4 8 12 16 20 24 28 ; do
		eval un$x=`expr $unit + $x`
		eval rm -f mt\$un$x nmt\$un$x rmt\$un$x nrmt\$un$x
	done

	case $i in
	mt*|xt*)
		mk rmt$un0	c $chr $un0
		mk rmt$un8	c $chr $un8
		mk nrmt$un0	c $chr $un4
		mk nrmt$un8	c $chr $un12
		ln nrmt$un0	rmt$un4
		ln nrmt$un8	rmt$un12
		;;
	st*)
		mk rst$un0	c $chr $un0
		mk rst$un8	c $chr $un8
		mk rst$un16	c $chr $un16
		mk rst$un24	c $chr $un24
		mk nrst$un0	c $chr $un4
		mk nrst$un8	c $chr $un12
		mk nrst$un16	c $chr $un20
		mk nrst$un24	c $chr $un28

		ln rst$un0	rmt$un0
		ln rst$un8	rmt$un8
		ln rst$un16	rmt$un16
		ln rst$un24	rmt$un24
		ln nrst$un0	nrmt$un0
		ln nrst$un8	nrmt$un8
		ln nrst$un16	nrmt$un16
		ln nrst$un24	nrmt$un24
		ln nrmt$un0	rmt$un4
		ln nrmt$un8	rmt$un12
		;;
	esac
	;;

ip*|sd*|xd*|xy*)
	case $i in
	ip*) name=ip blk=22 chr=65 ;;
	sd*) name=sd blk=7  chr=17 ;;
	xd*) name=xd blk=10 chr=42 ;;
	xy*) name=xy blk=3  chr=9  ;;
	esac
	minor=`expr $unit '*' 8`
	mk ${name}${unit}a	b $blk `expr $minor + 0` 640 operator
	mk ${name}${unit}b	b $blk `expr $minor + 1` 640 operator
	mk ${name}${unit}c	b $blk `expr $minor + 2` 640 operator
	mk ${name}${unit}d	b $blk `expr $minor + 3` 640 operator
	mk ${name}${unit}e	b $blk `expr $minor + 4` 640 operator
	mk ${name}${unit}f	b $blk `expr $minor + 5` 640 operator
	mk ${name}${unit}g	b $blk `expr $minor + 6` 640 operator
	mk ${name}${unit}h	b $blk `expr $minor + 7` 640 operator
	mk r${name}${unit}a	c $chr `expr $minor + 0` 640 operator
	mk r${name}${unit}b	c $chr `expr $minor + 1` 640 operator
	mk r${name}${unit}c	c $chr `expr $minor + 2` 640 operator
	mk r${name}${unit}d	c $chr `expr $minor + 3` 640 operator
	mk r${name}${unit}e	c $chr `expr $minor + 4` 640 operator
	mk r${name}${unit}f	c $chr `expr $minor + 5` 640 operator
	mk r${name}${unit}g	c $chr `expr $minor + 6` 640 operator
	mk r${name}${unit}h	c $chr `expr $minor + 7` 640 operator
	;;

fd*)
	case $unit in
	0|1)
		minor=`expr $unit '*' 8`

		mk fd${unit}a	b 16 $minor
		mk fd${unit}b	b 16 `expr $minor + 1`
		mk fd${unit}c	b 16 `expr $minor + 2`
		rm -f fd${unit}
		ln fd${unit}c fd${unit}

		mk rfd${unit}a	c 54 $minor
		mk rfd${unit}b	c 54 `expr $minor + 1`
		mk rfd${unit}c	c 54 `expr $minor + 2`
		rm -f rfd${unit}
		ln rfd${unit}c rfd${unit}
		;;
	*)
		echo "bad unit number in: $i; use fd0 or fd1"
		;;
	esac
	;;

pp*)
	unit=`expr $i : '..\(.*\)'`
	mknod pp${unit}         c 56 $unit
	mknod ppdiag${unit}     c 56 `expr $unit + 16`
	chgrp operator pp${unit} ppdiag${unit}
	chmod 666 pp${unit}
	chmod 600 ppdiag${unit}
	;;

oct*)
	case $unit in
	0) ch=m ;; 
	1) ch=n ;; 
	2) ch=o ;;
	*) echo "bad unit for oct in: $i, use oct0 thru oct2" ; continue ;;
	esac
	eval `echo $ch $unit | awk ' { ch = $1; u = 8 * $2 } ; END {
	    for (i = 0; i < 8; i++)
		printf("mk tty%s%d c 1 %d; ",ch,i,u+i); } ;'`
	;;

sbus)
	mk sbus0 c  3 32 600
	mk sbus1 c  3 33 600
	mk sbus2 c  3 34 600
	mk sbus3 c  3 35 600
	;;

ttys)
	mk ttys0 c 12 4
	mk ttys1 c 12 5
	mk ttys2 c 12 6
	mk ttys3 c 12 7
	;;

ttyt)
	mk ttyt0 c 12 8
	mk ttyt1 c 12 9
	mk ttyt2 c 12 10
	mk ttyt3 c 12 11
	;;

mcp*)
	case $unit in
	[0-7]) ch=`echo $unit | tr "0-7" "h-o"` ;;
	*) echo "bad unit for mcp in: $i, use mcp0 thru mcp7" ; continue ;;
	esac
	for x in 0 1 2 3 4 5 6 7 8 9 a10 b11 c12 d13 e14 f15 ; do
		case $x in
		[0-9]*)	c=$x
			i=$x ;;
		*)	c=`expr substr $x 1 1`
			i=`expr substr $x 2 99` ;;
		esac
		mk tty$ch$c c 44 `expr $unit \* 16 + $i`
	done
	mk mcpp$unit c 66 $unit
	;;

bwone*)
	mk bwone$unit c 26 $unit
	;;

bwtwo*)
	mk bwtwo$unit c 27 $unit
	;;

cgone*)
	mk cgone$unit c 14 $unit
	;;

cgtwo*)
	mk cgtwo$unit c 31 $unit
	;;

cgthree*)
	mk cgthree$unit c 55 $unit
	;;

cgfour*)
	mk cgfour$unit c 39 $unit
	;;

cgsix*)
	mk cgsix$unit c 67 $unit
	;;

cgeight*)
	mk cgeight$unit c 64 $unit
	;;

cgnine*)
	mk cgnine$unit c 68 $unit
	;;

gpone*)
	mk gpone${unit}a c 32 `expr $unit \* 4`
	mk gpone${unit}b c 32 `expr $unit \* 4 + 1`
	mk gpone${unit}c c 32 `expr $unit \* 4 + 2`
	mk gpone${unit}d c 32 `expr $unit \* 4 + 3`
	;;

taac*)
	mk taac$unit c 62 $unit
	;;

vpc*)
	mk vpc$unit c 28 `expr $unit \* 2`
	mk lp$unit c 28 `expr $unit \* 2 + 1`
	;;

vp*)
	mk vp$unit c 6 $unit
	;;

pty*)
	class=$unit
	case $class in
	0) offset=0 name=p;;
	1) offset=16 name=q;;
	2) offset=32 name=r;;
	3) offset=48 name=s;;
	4) offset=64 name=t;;
	5) offset=80 name=u;;
	6) offset=96 name=v;;
	7) offset=112 name=w;;
	8) offset=128 name=x;;
	9) offset=144 name=y;;
	10) offset=160 name=z;;
	11) offset=176 name=P;;
	12) offset=192 name=Q;;
	13) offset=208 name=R;;
	14) offset=224 name=S;;
	15) offset=240 name=T;;
	*) echo "bad unit for pty in: $i; use pty0 thru pty15" ; continue ;;
	esac

	eval `echo $offset $name | awk ' { b=$1; n=$2 } ; END {
		for (i = 0; i < 16; i++)
			printf("mk tty%s%x c 20 %d; \
				mk pty%s%x c 21 %d; ", \
				n, i, b+i, n, i, b+i); } ;'`
	;;

win*)
	class=$unit
	case $class in
	0) offset=0 ;;
	1) offset=32 ;;
	2) offset=64 ;;
	3) offset=96 ;;
	*) echo "bad unit for win in: $i; use win0 thru win3" ; continue ;;
	esac
	eval `echo $offset | awk ' { b=$1 } ; END {
		for (i = b; i < (b + 32); i++)
			printf("mk win%d c 15 %d;", i, i); \
	} ;'`
	;;

sky)
	mk sky c 24 0
	;;

fpa)
	mk fpa c 34 32
	;;

spx)
	mknod spx c 37 35               ; chmod 0644 spx
	;;

audio)
	mk audio c 37 69
	mk audioctl c 69 128
	;;

pc*)
	case $unit in
	[0-7])
		mk pc$unit c 38 $unit
		;;
	*)
		echo "bad unit number in: $i; use pc0 thru pc7"
		;;
	esac
	;;

pp*)
	mk pp$unit c 56 $unit 666 operator
	mk ppdiag$unit c 56 `expr $unit + 16` 600 operator
	;;

sqz)
	mk sqz0 c 52 0
	mk sqz1 c 52 1
	mk sqz2 c 52 2
	mk sqz3 c 52 3
	;;

sr0)
	mk sr0 b 18 0 555
	;;

rsr0)
	mk rsr0 c 58 0 555
	;;

local)
	sh MAKEDEV.local
	;;

*)
	echo "I don't know how to MAKEDEV $i."
	;;
esac
done
