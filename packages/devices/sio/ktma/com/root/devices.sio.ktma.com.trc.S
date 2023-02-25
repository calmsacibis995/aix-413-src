# @(#)08	1.2  src/packages/devices/sio/ktma/com/root/devices.sio.ktma.com.trc.S, pkgdevgraphics, pkg41J, 9519A_all 5/9/95 08:30:43
#
#   COMPONENT_NAME: INPUTDD
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
#
#----------------------------------------------------------------------
#
#  keyboard/tablet/mouse device drivers trace format template 
#
#----------------------------------------------------------------------
#
* /etc/trcfmt
+709 1.0 "INPUTDD:  " \
        {{ $a =  $HD & 0xfc00 }} \
        {{ $b =  $a / 1024 }} \
        {{ $func =  $b & 0x3f }} \
        {{ $c  = $HD - $a }} \
        {{ $routine  = $c & 0x03ff }} \
        $routine, \
        01  { "put_iq " }, \
        02  { "put_oq " }, \
        03  { "get_oq " }, \
        04  { "flush_q" }, \
        05  { "get_iq " }, \
        06  { "wait_oq" }, \
        07  { "sleep  " }, \
        08  { "putring" }, \
        09  { "log    " }, \
        0a  { "uring  " }, \
        0b  { "put_oq1" }, \
        0c  { "put_oq2" }, \
        0d  { "startwd" }, \
        0e  { "rring  " }, \
        0f  { "rflush " }, \
        20  { "kcnfig " }, \
        21  { "kmpx   " }, \
        22  { "kopen  " }, \
        23  { "kclose " }, \
        24  { "kioctl " }, \
        25  { "kintr  " }, \
        26  { "ksend_q" }, \
        27  { "k_wdog " }, \
        28  { "ksetup " }, \
        29  { "keyproc" }, \
        2a  { "sstatus" }, \
        2b  { "procsak" }, \
        2c  { "un_sak " }, \
        2e  { "put_key" }, \
        2f  { "polappl" }, \
        30  { "put_sq " }, \
        31  { "nsound " }, \
        32  { "appkill" }, \
        33  { "procevt" }, \
        34  { "keyictl" }, \
        35  { "sv_proc" }, \
        36  { "k_s8042" }, \
        40  { "mconfig" }, \
        41  { "mopen  " }, \
        42  { "mclose " }, \
        43  { "mioctl " }, \
        44  { "mintr  " }, \
        45  { "msndcmd" }, \
        46  { "msend_q" }, \
        47  { "m_wdog " }, \
        48  { "mseproc" }, \
        49  { "m_s8042" }, \
        50  { "ktscfg " }, \
        51  { "ktsmpx " }, \
        52  { "ktsopen" }, \
        53  { "ktsclse" }, \
        54  { "ktsictl" }, \
        55  { "ktsintr" }, \
        60  { "tconfig" }, \
        61  { "topen  " }, \
        62  { "tclose " }, \
        63  { "tioctl " }, \
        64  { "tabintr" }, \
        65  { "tsend_q" }, \
        66  { "t_wdog " }, \
        67  { "tabproc" }, \
        \*  { $routine%X2"    " } \
        $func%D2, \
        0  { " [ent ]" }, \
        32 { " [exit]" }, \
        \* { " ["$func%X2"]"  } \
        HT & 0x07, \
        02 {O2 "  d0=" X4 }, \
        06 {O2 "  d0=" X4 " d1=" X4 " d2=" X4 " d3=" X4 " d4=" X4 }

