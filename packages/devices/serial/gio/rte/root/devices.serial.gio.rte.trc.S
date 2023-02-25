# @(#)30	1.3  src/packages/devices/serial/gio/rte/root/devices.serial.gio.rte.trc.S, pkgdevgraphics, pkg41J, 9516B_all 4/18/95 16:14:39
#
#   COMPONENT_NAME: INPUTDD
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
#
#----------------------------------------------------------------------
#
#  Serial GIO device driver trace format template 
#
#----------------------------------------------------------------------
#
* /etc/trcfmt
+722 1.0 "SGIODD:  " \
        {{ $a =  $HD & 0xfc00 }} \
        {{ $b =  $a / 1024 }} \
        {{ $func =  $b & 0x3f }} \
        {{ $c  = $HD - $a }} \
        {{ $routine  = $c & 0x03ff }} \
        $routine, \
        01  { "sconfig" }, \
        02  { "s_open " }, \
        03  { "s_close" }, \
        04  { "s_ioctl" }, \
        05  { "add_dev" }, \
        06  { "rem_dev" }, \
        07  { "fnd_dev" }, \
        08  { "c_sikpr" }, \
        09  { "t_sikpr" }, \
        0a  { "sikproc" }, \
        0b  { "poll_up" }, \
        0c  { "perf_io" }, \
        0d  { "dial_io" }, \
        0e  { "dial_rd" }, \
        0f  { "lpfk_io" }, \
        10  { "lpfk_rd" }, \
        11  { "q_sikpr" }, \
        12  { "r_sikpr" }, \
        13  { "s_log  " }, \
        14  { "s_uring" }, \
        15  { "s_rring" }, \
        16  { "s_pring" }, \
        17  { "s_flush" }, \
        18  { "reg_pm " }, \
        19  { "ureg_pm" }, \
        1a  { "pmhndlr" }, \
        1b  { "sttyopn" }, \
        \*  { $routine%X2"    " } \
        $func%D2, \
        0  { " [ent ]" }, \
        32 { " [exit]" }, \
        \* { " ["$func%X2"]"  } \
        HT & 0x07, \
        02 {O2 "  d0=" X4 }, \
        06 {O2 "  d0=" X4 " d1=" X4 " d2=" X4 " d3=" X4 " d4=" X4 }

