# @(#)06	1.3  src/packages/devices/sio/ktma/com/packdep.mk, pkgdevgraphics, pkg41J, 9509A_all 2/15/95 15:09:24
#
#   COMPONENT_NAME: pkgdevgraphics
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

USR_LIBLPP_LIST     += devices.sio.ktma.com.pre_d

USR_LIBLPP_UPDT_LIST     += devices.sio.ktma.com.CuDv.odmdel

ROOT_LIBLPP_LIST	+= devices.sio.ktma.com.trc 

USR_ODMADD_LIST		+= sm_inputdd.add%devices.sio.ktma.com  \
                       std_keyboard.add%devices.sio.ktma.com \
                       std_mouse.add%devices.sio.ktma.com \
                       std_tablet.add%devices.sio.ktma.com
