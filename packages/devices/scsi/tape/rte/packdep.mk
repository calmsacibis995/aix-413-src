# @(#)89        1.7  src/packages/devices/scsi/tape/rte/packdep.mk, pkgdevbase, pkg411, 9434A411a 8/22/94 11:06:23
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

USR_LIBLPP_LIST		+= devices.scsi.tape.rte.namelist \
						devices.scsi.tape.rte.pre_d
ROOT_LIBLPP_LIST    += 

USR_ODMADD_LIST		+= sm_tape.add%devices.scsi.tape.rte \
			   tape.scsi.1200mb-c.add%devices.scsi.tape.rte \
                           tape.scsi.150mb.add%devices.scsi.tape.rte \
                           tape.scsi.3490e.add%devices.scsi.tape.rte \
                           tape.scsi.4mm2gb.add%devices.scsi.tape.rte \
                           tape.scsi.4mm4gb.add%devices.scsi.tape.rte \
                           tape.scsi.525mb.add%devices.scsi.tape.rte \
                           tape.scsi.8mm.add%devices.scsi.tape.rte \
                           tape.scsi.8mm5gb.add%devices.scsi.tape.rte \
                           tape.scsi.8mm7gb.add%devices.scsi.tape.rte \
                           tape.scsi.9trk.add%devices.scsi.tape.rte \
                           tape.scsi.ost.add%devices.scsi.tape.rte
