# @(#)65	1.2  src/packages/X11/x_st_mgr/rte/usr/X11.x_st_mgr.rte.insize.S, pkgxstmgr, pkg411, GOLD410 6/3/94 07:41:04
#
#   COMPONENT_NAME: pkgxstmgr
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# The 10 temporary blocks allow me to copy /etc/services and /etc/inetd.conf to
# /tmp to hold them.  The +2 to root allows for the possibility that each of the
# two files have increased by one block in the process of adding one line to 
# them.
/tmp +10 10
/ +2
