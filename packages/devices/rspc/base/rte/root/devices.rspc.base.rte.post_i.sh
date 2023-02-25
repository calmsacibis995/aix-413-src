#!/bin/ksh
# @(#)26        1.1  src/packages/devices/rspc/base/rte/root/devices.rspc.base.rte.post_i.sh, pkgrspc, pkg41J, 9525E_all 6/21/95 09:59:32
#
#
#   COMPONENT_NAME: PKGRSPC
#
#   FUNCTIONS: updates connwhere and location code values for updating
#              from AIX 4.1.2 to AIX 4.1.3, or from 411 to 413 
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#######################################################################
odmget -qPdDvLn=bus/sys/pci CuDv |sed -e s/'connwhere = "."'/'connwhere = "4.0"'/g >$SAVEDIR/tmp1
odmdelete -qPdDvLn=bus/sys/pci -o CuDv> /dev/null
odmadd $SAVEDIR/tmp1

par=$(odmget -qPdDvLn=adapter/isa/sio CuDv | grep parent | awk '{print $3}')
#echo $par
odmdelete -qPdDvLn=adapter/isa/sio -o CuDv > /dev/null

odmget -qPdDvLn=adapter/isa_sio/s1a CuDv |sed -e s/'connwhere = "."'/'connwhere = "PNP05011"'/g -e s/"sio0"/$par/g  >$SAVEDIR/tmp1
odmdelete -qPdDvLn=adapter/isa_sio/s1a -o CuDv> /dev/null
odmadd $SAVEDIR/tmp1

odmget -qPdDvLn=adapter/isa_sio/s2a CuDv | sed -e s/'connwhere = "."'/'connwhere = "PNP05012"'/g -e s/"sio0"/$par/g  >$SAVEDIR/tmp1
odmdelete -qPdDvLn=adapter/isa_sio/s2a -o CuDv> /dev/null
odmadd $SAVEDIR/tmp1

odmget -qPdDvLn=adapter/isa_sio/isa_keyboard CuDv | sed -e s/'connwhere = "."'/'connwhere = "PNP0303ffffffff"'/g -e s/"sio0"/$par/g  >$SAVEDIR/tmp1
odmdelete -qPdDvLn=adapter/isa_sio/isa_keyboard -o CuDv> /dev/null
odmadd $SAVEDIR/tmp1

odmget -qPdDvLn=adapter/isa_sio/fda CuDv | sed -e s/'connwhere = "."'/'connwhere = "PNP0700ffffffff"'/g -e s/"sio0"/$par/g  >$SAVEDIR/tmp1
odmdelete -qPdDvLn=adapter/isa_sio/fda -o CuDv> /dev/null
odmadd $SAVEDIR/tmp1

odmget -qPdDvLn=adapter/isa_sio/isa_mouse CuDv | sed -e s/'connwhere = "."'/'connwhere = "PNP0F03ffffffff"'/g -e s/"sio0"/$par/g  >$SAVEDIR/tmp1
odmdelete -qPdDvLn=adapter/isa_sio/isa_mouse -o CuDv> /dev/null
odmadd $SAVEDIR/tmp1

odmget -qPdDvLn=adapter/isa_sio/ppa CuDv | sed -e s/'connwhere = "."'/'connwhere = "PNP0400ffffffff"'/g -e s/"sio0"/$par/g  >$SAVEDIR/tmp1
odmdelete -qPdDvLn=adapter/isa_sio/ppa -o CuDv> /dev/null
odmadd $SAVEDIR/tmp1

odmget -qPdDvLn=adapter/isa_sio/baud CuDv | sed -e s/'connwhere = "."'/'connwhere = "IBM000Effffffff"'/g -e s/"sio0"/$par/g  >$SAVEDIR/tmp1
odmdelete -qPdDvLn=adapter/isa_sio/baud -o CuDv> /dev/null
odmadd $SAVEDIR/tmp1

# delete memory devices
uniq=memory/sys/simm
dev=$(odmget -qPdDvLn=$uniq CuDv | grep name | awk -F\" '{print $2}')
odmdelete -q PdDvLn=$uniq -o CuDv > /dev/null
if [[ -n ${dev} ]]
then
    for name in $dev
    do
	odmdelete -q name=$name -o CuAt > /dev/null
    done
fi

# delete processor devices
uniq=processor/sys/proc1
dev=$(odmget -qPdDvLn=$uniq CuDv | grep name | awk -F\" '{print $2}')
odmdelete -q PdDvLn=$uniq -o CuDv > /dev/null
if [[ -n ${dev} ]]
then
    for name in $dev
    do
	odmdelete -q name=$name -o CuAt > /dev/null
    done
fi

####################################################################
###
###  fix location codes
###
####################################################################

# get ISA bus (bus1)      - PdDvLn=bus/pci/isa
#     set location code = 04-A0
uniq=bus/pci/isa
odmget -q PdDvLn=$uniq CuDv | \
    sed -e s/'location = "..-.."'/'location = "04-A0"'/g >$SAVEDIR/tmp1
if [ -s $SAVEDIR/tmp1 ]
then
    odmdelete -q PdDvLn=$uniq -o CuDv > /dev/null
    odmadd $SAVEDIR/tmp1
fi

# get scsi device (scsi0) - PdDvLn=adapter/pci/ncr810
#     set location code = 04-B0
uniq=adapter/pci/ncr810
dev=$(odmget -qPdDvLn=$uniq CuDv | grep name | awk -F\" '{print $2}')
odmget -q PdDvLn=$uniq CuDv | \
    sed -e s/'location = "..-.."'/'location = "04-B0"'/g >$SAVEDIR/tmp1
if [ -s $SAVEDIR/tmp1 ]
then
    odmdelete -q PdDvLn=$uniq -o CuDv > /dev/null
    odmadd $SAVEDIR/tmp1

    # get children
    odmget -q parent=$dev CuDv | \
	sed -e s/'location = "..-..'/'location = "04-B0'/g >$SAVEDIR/tmp1
    if [ -s $SAVEDIR/tmp1 ]
    then
	odmdelete -q parent=$dev -o CuDv > /dev/null
	odmadd $SAVEDIR/tmp1
    fi
fi

# get paud0 device        - PdDvLn=adapter/isa_sio/baud
#     set location code = 01-A0
uniq=adapter/isa_sio/baud
dev=$(odmget -qPdDvLn=$uniq CuDv | grep name | awk -F\" '{print $2}')
odmget -q PdDvLn=$uniq CuDv | \
    sed -e s/'location = "..-..-.."'/'location = "01-A0"'/g >$SAVEDIR/tmp1
if [ -s $SAVEDIR/tmp1 ]
then
    odmdelete -q PdDvLn=$uniq -o CuDv > /dev/null
    odmadd $SAVEDIR/tmp1

    # get children
    odmget -q parent=$dev CuDv | \
	sed -e s/'location = "..-..-..'/'location = "01-A0-00'/g >$SAVEDIR/tmp1
    if [ -s $SAVEDIR/tmp1 ]
    then
	odmdelete -q parent=$dev -o CuDv > /dev/null
	odmadd $SAVEDIR/tmp1
    fi
fi

# get fd0 device          - PdDvLn=adapter/isa_sio/fda
#     set location code = 01-B0
uniq=adapter/isa_sio/fda
dev=$(odmget -qPdDvLn=$uniq CuDv | grep name | awk -F\" '{print $2}')
odmget -q PdDvLn=$uniq CuDv | \
    sed -e s/'location = "..-..-.."'/'location = "01-B0"'/g >$SAVEDIR/tmp1
if [ -s $SAVEDIR/tmp1 ]
then
    odmdelete -q PdDvLn=$uniq -o CuDv > /dev/null
    odmadd $SAVEDIR/tmp1

    # get children
    odmget -q parent=$dev CuDv | \
	sed -e s/'location = "..-..-..'/'location = "01-B0-00'/g >$SAVEDIR/tmp1
    if [ -s $SAVEDIR/tmp1 ]
    then
	odmdelete -q parent=$dev -o CuDv > /dev/null
	odmadd $SAVEDIR/tmp1
    fi
fi

# get ppa0 device         - PdDvLn=adapter/isa_sio/ppa
#     set location code = 01-C0
uniq=adapter/isa_sio/ppa
dev=$(odmget -qPdDvLn=$uniq CuDv | grep name | awk -F\" '{print $2}')
odmget -q PdDvLn=$uniq CuDv | \
    sed -e s/'location = "..-..-.."'/'location = "01-C0"'/g >$SAVEDIR/tmp1
if [ -s $SAVEDIR/tmp1 ]
then
    odmdelete -q PdDvLn=$uniq -o CuDv > /dev/null
    odmadd $SAVEDIR/tmp1

    # get children
    odmget -q parent=$dev CuDv | \
	sed -e s/'location = "..-..-..'/'location = "01-C0-00'/g >$SAVEDIR/tmp1
    if [ -s $SAVEDIR/tmp1 ]
    then
	odmdelete -q parent=$dev -o CuDv > /dev/null
	odmadd $SAVEDIR/tmp1
    fi
fi

# get sa0 device          - PdDvLn=adapter/isa_sio/s1a
#     set location code = 01-D0
uniq=adapter/isa_sio/s1a
dev=$(odmget -qPdDvLn=$uniq CuDv | grep name | awk -F\" '{print $2}')
odmget -q PdDvLn=$uniq CuDv | \
    sed -e s/'location = "..-..-.."'/'location = "01-D0"'/g >$SAVEDIR/tmp1
if [ -s $SAVEDIR/tmp1 ]
then
    odmdelete -q PdDvLn=$uniq -o CuDv > /dev/null
    odmadd $SAVEDIR/tmp1

    # get children
    odmget -q parent=$dev CuDv | \
	sed -e s/'location = "..-..-..'/'location = "01-D0-00'/g >$SAVEDIR/tmp1
    if [ -s $SAVEDIR/tmp1 ]
    then
	odmdelete -q parent=$dev -o CuDv > /dev/null
	odmadd $SAVEDIR/tmp1
    fi
fi

# get sa1 device          - PdDvLn=adapter/isa_sio/s2a
#     set location code = 01-E0
uniq=adapter/isa_sio/s2a
dev=$(odmget -qPdDvLn=$uniq CuDv | grep name | awk -F\" '{print $2}')
odmget -q PdDvLn=$uniq CuDv | \
    sed -e s/'location = "..-..-.."'/'location = "01-E0"'/g >$SAVEDIR/tmp1
if [ -s $SAVEDIR/tmp1 ]
then
    odmdelete -q PdDvLn=$uniq -o CuDv > /dev/null
    odmadd $SAVEDIR/tmp1

    # get children
    odmget -q parent=$dev CuDv | \
	sed -e s/'location = "..-..-..'/'location = "01-E0-00'/g >$SAVEDIR/tmp1
    if [ -s $SAVEDIR/tmp1 ]
    then
	odmdelete -q parent=$dev -o CuDv > /dev/null
	odmadd $SAVEDIR/tmp1
    fi
fi

# get sioka0 device       - PdDvLn=adapter/isa_sio/isa_keyboard
#     set location code = 01-F0
uniq=adapter/isa_sio/isa_keyboard
dev=$(odmget -qPdDvLn=$uniq CuDv | grep name | awk -F\" '{print $2}')
odmget -q PdDvLn=$uniq CuDv | \
    sed -e s/'location = "..-..-.."'/'location = "01-F0"'/g >$SAVEDIR/tmp1
if [ -s $SAVEDIR/tmp1 ]
then
    odmdelete -q PdDvLn=$uniq -o CuDv > /dev/null
    odmadd $SAVEDIR/tmp1

    # get children
    odmget -q parent=$dev CuDv | \
	sed -e s/'location = "..-..-..'/'location = "01-F0-00'/g >$SAVEDIR/tmp1
    if [ -s $SAVEDIR/tmp1 ]
    then
	odmdelete -q parent=$dev -o CuDv > /dev/null
	odmadd $SAVEDIR/tmp1
    fi
fi

# get sioma0 device       - PdDvLn=adapter/isa_sio/isa_mouse
#     set location code = 01-G0
uniq=adapter/isa_sio/isa_mouse
dev=$(odmget -qPdDvLn=$uniq CuDv | grep name | awk -F\" '{print $2}')
odmget -q PdDvLn=$uniq CuDv | \
    sed -e s/'location = "..-..-.."'/'location = "01-G0"'/g >$SAVEDIR/tmp1
if [ -s $SAVEDIR/tmp1 ]
then
    odmdelete -q PdDvLn=$uniq -o CuDv > /dev/null
    odmadd $SAVEDIR/tmp1

    # get children
    odmget -q parent=$dev CuDv | \
	sed -e s/'location = "..-..-..'/'location = "01-G0-00'/g >$SAVEDIR/tmp1
    if [ -s $SAVEDIR/tmp1 ]
    then
	odmdelete -q parent=$dev -o CuDv > /dev/null
	odmadd $SAVEDIR/tmp1
    fi
fi

###  modifing non-integrated isa devices
uniq=$(odmget -q "PdDvLn like '*/isa/*'" CuDv | \
    grep PdDvLn | awk -F\" '{print $2}')

if [[ -n ${uniq} ]]
then
    for name in $uniq
    do
	conn=$(odmget -q PdDvLn=$name CuDv | grep connwhere | awk -F\" '{print $2}')

	odmget -q "PdDvLn=$name" CuDv | \
	    sed -e s/'location = "....."'/'location = "01-0'$conn'"'/g > $SAVEDIR/tmp1

	if [ -s $SAVEDIR/tmp1 ]
	then
	    odmdelete -q PdDvLn=$name -o CuDv > /dev/null
	    odmadd $SAVEDIR/tmp1
	fi
    done
fi


###  modifing non-integrated pci devices
uniq=$(odmget -q "PdDvLn like '*/pci/*'" CuDv | \
    grep PdDvLn | awk -F\" '{print $2}')

if [[ -n ${uniq} ]]
then
    for name in $uniq 
    do
	conn=$(odmget -q PdDvLn=$name CuDv | grep connwhere | awk -F\" '{print $2}')
	conn_check=100	
	if [ $conn -gt $conn_check ]
	then
	    conn_part=`expr $conn - 96 `
	    newconn=`expr $conn_part / 8 `

	    odmget -q "PdDvLn=$name" CuDv | \
		sed -e s/'location = "....."'/'location = "04-0'$newconn'"'/g > $SAVEDIR/tmp1

	    if [ -s $SAVEDIR/tmp1 ]
	    then
		odmdelete -q PdDvLn=$name -o CuDv > /dev/null
		odmadd $SAVEDIR/tmp1
	    fi
	fi
    done
fi

rm -rf $SAVEDIR/tmp1

