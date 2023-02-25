# @(#)95        1.2  src/packages/devices/scsi/tape/rspc/packdep.mk, pkgrspc, pkg411, 9440A411d 10/4/94 16:54:15
#
# COMPONENT_NAME: pkgdevbase
#
# FUNCTIONS: packaging definitions
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1993,1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# *_LIBLPP_LIST	contains the list of files that need to be added to 
#		the respective liblpp.a other than the copyright, .al,
#		.tcb, .size, and .inventory files.  For example, any
#		config scripts would be included here.
#		The variable may be left blank if there are no additional
#		files, but the option will exist.
#

USR_LIBLPP_LIST		+= devices.scsi.tape.rspc.pre_d

ROOT_LIBLPP_LIST    += 

USR_ODMADD_LIST		+= sm_tape_rspc.add%devices.scsi.tape.rspc \
                           tape.scsi.4mm2gb2.add%devices.scsi.tape.rspc

INFO_FILES              += devices.scsi.tape.rspc.prereq
