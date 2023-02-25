#!/bin/ksh
# @(#)26        1.4  src/packages/devices/rspc/base/rte/root/devices.rspc.base.rte.unpost_u.sh, pkgrspc, pkg41J, 9524D_all 6/13/95 14:39:22
#
#
#   COMPONENT_NAME: PKGRSPC
#
#   FUNCTIONS: Removes Customized Device and Attribute entries
#              to handle reject of AIX 4.1.3 software.
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
########################################################################

name=$(odmget -qPdDvLn=bus/sys/pci CuDv | grep name | awk -F\" '{print $2}')
odmdelete -qPdDvLn=bus/sys/pci -o CuDv> /dev/null
    for device in $name
    do
        odmdelete -qname=$device -o CuAt > /dev/null
    done

name=$(odmget -qPdDvLn=adapter/isa_sio/s1a CuDv | grep name | awk -F\" '{print $2}')
odmdelete -qPdDvLn=adapter/isa_sio/s1a -o CuDv> /dev/null
    for device in $name
    do
        odmdelete -qname=$device -o CuAt > /dev/null


        child=$(odmget -q parent=$device CuDv | grep name | awk -F\" '{print $2}')
        odmdelete -q parent=$device -o CuDv > /dev/null
        for dev in $child
        do
            odmdelete -q name=$dev -o CuAt > /dev/null
        done
    done

name=$(odmget -qPdDvLn=adapter/isa_sio/s2a CuDv | grep name | awk -F\" '{print $2}')
odmdelete -qPdDvLn=adapter/isa_sio/s2a -o CuDv> /dev/null
    for device in $name
    do
        odmdelete -qname=$device -o CuAt > /dev/null


        child=$(odmget -q parent=$device CuDv | grep name | awk -F\" '{print $2}')
        odmdelete -q parent=$device -o CuDv > /dev/null
        for dev in $child
        do
            odmdelete -q name=$dev -o CuAt > /dev/null
        done
    done

name=$(odmget -qPdDvLn=adapter/isa_sio/isa_keyboard CuDv | grep name | awk -F\" '{print $2}')
odmdelete -qPdDvLn=adapter/isa_sio/isa_keyboard -o CuDv> /dev/null
    for device in $name
    do
        odmdelete -qname=$device -o CuAt > /dev/null


        child=$(odmget -q parent=$device CuDv | grep name | awk -F\" '{print $2}')
        odmdelete -q parent=$device -o CuDv > /dev/null
        for dev in $child
        do
            odmdelete -q name=$dev -o CuAt > /dev/null
        done
    done

name=$(odmget -qPdDvLn=adapter/isa_sio/fda CuDv | grep name | awk -F\" '{print $2}')
odmdelete -qPdDvLn=adapter/isa_sio/fda -o CuDv> /dev/null
    for device in $name
    do
        odmdelete -qname=$device -o CuAt > /dev/null


        child=$(odmget -q parent=$device CuDv | grep name | awk -F\" '{print $2}')
        odmdelete -q parent=$device -o CuDv > /dev/null
        for dev in $child
        do
            odmdelete -q name=$dev -o CuAt > /dev/null
        done
    done

name=$(odmget -qPdDvLn=adapter/isa_sio/isa_mouse CuDv | grep name | awk -F\" '{print $2}')
odmdelete -qPdDvLn=adapter/isa_sio/isa_mouse -o CuDv> /dev/null
    for device in $name
    do
        odmdelete -qname=$device -o CuAt > /dev/null


        child=$(odmget -q parent=$device CuDv | grep name | awk -F\" '{print $2}')
        odmdelete -q parent=$device -o CuDv > /dev/null
        for dev in $child
        do
            odmdelete -q name=$dev -o CuAt > /dev/null
        done
    done

name=$(odmget -qPdDvLn=adapter/isa_sio/ppa CuDv | grep name | awk -F\" '{print $2}')
odmdelete -qPdDvLn=adapter/isa_sio/ppa -o CuDv> /dev/null
    for device in $name
    do
        odmdelete -qname=$device -o CuAt > /dev/null


        child=$(odmget -q parent=$device CuDv | grep name | awk -F\" '{print $2}')
        odmdelete -q parent=$device -o CuDv > /dev/null
        for dev in $child
        do
            odmdelete -q name=$dev -o CuAt > /dev/null
        done
    done

name=$(odmget -qPdDvLn=adapter/isa_sio/baud CuDv | grep name | awk -F\" '{print $2}')
odmdelete -qPdDvLn=adapter/isa_sio/baud -o CuDv> /dev/null
    for device in $name
    do
        odmdelete -qname=$device -o CuAt > /dev/null


        child=$(odmget -q parent=$device CuDv | grep name | awk -F\" '{print $2}')
        odmdelete -q parent=$device -o CuDv > /dev/null
        for dev in $child
        do
            odmdelete -q name=$dev -o CuAt > /dev/null
        done
    done

name=$(odmget -qPdDvLn=bus/pci/isa CuDv | grep name | awk -F\" '{print $2}')
odmdelete -qPdDvLn=bus/pci/isa -o CuDv> /dev/null
    for device in $name
    do
        odmdelete -qname=$device -o CuAt > /dev/null
    done

name=$(odmget -q "PdDvLn like '*/isa/*'" CuDv | grep name | awk -F\" '{print $2}')
odmdelete -q "PdDvLn like '*/isa/*'" -o CuDv> /dev/null
for child in $name
do
    odmdelete -q name=$child -o CuAt > /dev/null
done

name=$(odmget -q "PdDvLn like '*/pci/*'" CuDv | grep name | awk -F\" '{print $2}')
odmdelete -q "PdDvLn like '*/pci/*'" -o CuDv> /dev/null
    for device in $name
    do
        odmdelete -qname=$device -o CuAt > /dev/null

        child=$(odmget -q parent=$device CuDv | grep name | awk -F\" '{print $2}')
        odmdelete -q parent=$device -o CuDv > /dev/null
        for dev in $child
        do
            odmdelete -q name=$dev -o CuAt > /dev/null
        done

    done

name=$(odmget -q "PdDvLn=memory/sys/totmem" CuDv | grep name | awk -F\" '{print $2}')
odmdelete -q "PdDvLn=memory/sys/totmem" -o CuDv> /dev/null
for child in $name
do
    odmdelete -q name=$child -o CuAt > /dev/null
done

name=$(odmget -q "PdDvLn=processor/sys/proc_rspc" CuDv | grep name | awk -F\" '{print $2}')
odmdelete -q "PdDvLn=processor/sys/proc_rspc" -o CuDv> /dev/null
for child in $name
do
    odmdelete -q name=$child -o CuAt > /dev/null
done
