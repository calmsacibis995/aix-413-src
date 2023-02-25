# @(#)84        1.13  src/packages/devices/scsi/disk/rte/packdep.mk, pkgdevbase, pkg411, 9432A411a 8/5/94 08:18:24
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

USR_LIBLPP_LIST		+= devices.scsi.disk.rte.namelist \
						devices.scsi.disk.rte.rm_inv  \
						devices.scsi.disk.rte.pre_d

ROOT_LIBLPP_LIST    += 

USR_ODMADD_LIST		+= sm_cdrom.add%devices.scsi.disk.rte \
			   sm_rwopt.add%devices.scsi.disk.rte \
			   cdrom.scsi.cdrom1.add%devices.scsi.disk.rte \
			   cdrom.scsi.enhcdrom.add%devices.scsi.disk.rte \
			   cdrom.scsi.enhcdrom3.add%devices.scsi.disk.rte \
                           cdrom.scsi.oscd.add%devices.scsi.disk.rte \
                           disk.scsi.1000mb.add%devices.scsi.disk.rte \
            		   disk.scsi.1000mbde.add%devices.scsi.disk.rte \
                           disk.scsi.1200mb.add%devices.scsi.disk.rte \
                           disk.scsi.1370mb.add%devices.scsi.disk.rte \
                           disk.scsi.160mb.add%devices.scsi.disk.rte \
                           disk.scsi.2000mb.add%devices.scsi.disk.rte \
                           disk.scsi.2000mbde.add%devices.scsi.disk.rte \
                           disk.scsi.200mb.add%devices.scsi.disk.rte \
                           disk.scsi.320mb.add%devices.scsi.disk.rte \
                           disk.scsi.355mb.add%devices.scsi.disk.rte \
                           disk.scsi.400mb.add%devices.scsi.disk.rte \
                           disk.scsi.540mb.add%devices.scsi.disk.rte \
                           disk.scsi.670mb.add%devices.scsi.disk.rte \
                           disk.scsi.730mb.add%devices.scsi.disk.rte \
                           disk.scsi.857mb.add%devices.scsi.disk.rte \
                           disk.scsi.osdisk.add%devices.scsi.disk.rte \
			   disk.scsi.2000mb16bit.add%devices.scsi.disk.rte \
			   disk.scsi.2000mb16bitde.add%devices.scsi.disk.rte \
			   disk.scsi.1000mb16bit.add%devices.scsi.disk.rte \
			   disk.scsi.1000mb16bitde.add%devices.scsi.disk.rte \
			   disk.scsi.1100mb.add%devices.scsi.disk.rte \
			   disk.scsi.1100mb16bit.add%devices.scsi.disk.rte \
			   disk.scsi.1100mb16bitde.add%devices.scsi.disk.rte \
			   disk.scsi.2200mb.add%devices.scsi.disk.rte \
			   disk.scsi.2200mb16bit.add%devices.scsi.disk.rte \
			   disk.scsi.2200mb16bitde.add%devices.scsi.disk.rte \
			   disk.scsi.4500mb16bit.add%devices.scsi.disk.rte \
			   disk.scsi.4500mb16bitde.add%devices.scsi.disk.rte \
			   rwoptical.scsi.osomd.add%devices.scsi.disk.rte \
			   cdrom.scsi.scsd.add%devices.scsi.disk.rte \
			   disk.scsi.scsd.add%devices.scsi.disk.rte \
			   rwoptical.scsi.scsd.add%devices.scsi.disk.rte
			


INFO_FILES		+= devices.scsi.disk.rte.prereq

