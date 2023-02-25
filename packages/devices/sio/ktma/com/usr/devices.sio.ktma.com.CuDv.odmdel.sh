# !/bin/ksh
# @(#)42	1.3  src/packages/devices/sio/ktma/com/usr/devices.sio.ktma.com.CuDv.odmdel.sh, pkgdevgraphics, pkg41J, 9523C_all 6/8/95 07:50:41
#
#   COMPONENT_NAME: pkgdevgraphics
#
#   FUNCTIONS: none
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

ODMDIR=/etc/objrepos
export ODMDIR

for DEV in `lsdev -C -c keyboard 2>/dev/null | sed "s/ .*//"`
do
  odmdelete -o CuAt -q "name=$DEV" >/dev/null
done

odmdelete -o CuDv -q "PdDvLn='keyboard/sio.ka/kb101'" >/dev/null
odmdelete -o CuDv -q "PdDvLn='keyboard/sio.ka/kb102'" >/dev/null
odmdelete -o CuDv -q "PdDvLn='keyboard/sio.ka/kb106'" >/dev/null

odmdelete -o CuDv -q "PdDvLn='tablet/sio.ta/6093_m11'" >/dev/null
odmdelete -o CuDv -q "PdDvLn='tablet/sio.ta/6093_m12'" >/dev/null

odmdelete -o CuDv -q "PdDvLn='mouse/sio.ma/mse_3b'" >/dev/null
odmdelete -o CuDv -q "PdDvLn='mouse/sio.ma/mse_2b'" >/dev/null

odmdelete -o CuDv -q "PdDvLn='keyboard/sio.kta/kb101'" >/dev/null
odmdelete -o CuDv -q "PdDvLn='keyboard/sio.kta/kb102'" >/dev/null
odmdelete -o CuDv -q "PdDvLn='keyboard/sio.kta/kb106'" >/dev/null
odmdelete -o CuDv -q "PdDvLn='tablet/sio.kta/6093_m11'" >/dev/null
odmdelete -o CuDv -q "PdDvLn='tablet/sio.kta/6093_m12'" >/dev/null

odmdelete -o CuDv -q "PdDvLn='keyboard/sioka/ps2'" >/dev/null
odmdelete -o CuDv -q "PdDvLn='mouse/sioma/mse_3b'" >/dev/null
odmdelete -o CuDv -q "PdDvLn='mouse/sioma/mse_2b'" >/dev/null
