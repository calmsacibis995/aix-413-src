# @(#)68	1.3  src/packages/devices/scsi/disk/diag/com/packdep.mk, pkgdevdiag, pkg411, GOLD410 4/20/94 19:15:03
#
# COMPONENT_NAME: pkgdevdiag
#
# FUNCTIONS: packaging definitions
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# *_LIBLPP_LIST contains the list of files that need to be added to 
#               the respective liblpp.a other than the copyright, .al,
#               .tcb, .size, and .inventory files.  For example, any
#               config scripts would be included here.
#               The variable may be left blank if there are no additional
#               files, but the option will exist.
#

USR_LIBLPP_LIST         += devices.scsi.disk.diag.com.namelist

USR_ODMADD_LIST         += diag.disksa.add%devices.scsi.disk.diag.com
INFO_FILES		+= devices.scsi.disk.diag.com.prereq
