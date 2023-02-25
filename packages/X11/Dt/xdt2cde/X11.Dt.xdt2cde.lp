# @(#)21        1.1  src/packages/X11/Dt/xdt2cde/X11.Dt.xdt2cde.lp, pkgxcde, pkg410 2/23/94 09:12:41
#
#   COMPONENT_NAME: pkgxcde
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# X11.Dt.xdt2cde.lp
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

X11.Dt.xdt2cde N U en_US AIX CDE Migration Tool
