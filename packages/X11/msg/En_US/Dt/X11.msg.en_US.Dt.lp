#  @(#)54        1.3  src/packages/X11/msg/en_US/Dt/X11.msg.en_US.Dt.lp, pkgx, pkg41J, 9517A_all 4/24/95 10:59:25
#
#   COMPONENT_NAME: pkgx
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
#[prolog]
#[full option name] [ quiesce char] [content] [lang] [description]
#[comments]
#       quiesce char
#               Y - Must stop subsystem; do not invoke bosboot
#               N - do not need to stop subsystem, do not invoke bosboot
#               B - Must stop subsystem, must invoke bosboot and reboot
#               b - do not need to stop subsystem, must invoke bosboot and reboot
#
#       content
#               U - /usr code only
#               B - /usr and /(root) code
#               H - /usr/share code only
#
#       lang - language that the DEFAULT messages for this option are in.
#
#       description - formal description in AIX 4.1 LPP and LPP Option
#               Name Registration guide

X11.msg.en_US.Dt N U en_US AIX CDE Msg - en_US
