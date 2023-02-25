# @(#)03        1.6  src/packages/devices/scsi/disk/rspc/packdep.mk, pkgrspc, pkg41J, 9517B_all 4/27/95 10:54:47
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

USR_LIBLPP_LIST		+= devices.scsi.disk.rspc.pre_d

ROOT_LIBLPP_LIST    += 

USR_ODMADD_LIST		+= cdrom.scsi.enhcdrom2.add%devices.scsi.disk.rspc \
                           cdrom.scsi.enhcdrom4.add%devices.scsi.disk.rspc \
                           disk.scsi.270mb.add%devices.scsi.disk.rspc \
                           disk.scsi.270mb2.add%devices.scsi.disk.rspc \
                           disk.scsi.340mb.add%devices.scsi.disk.rspc \
                           disk.scsi.360mb.add%devices.scsi.disk.rspc \
                           disk.scsi.540mb2.add%devices.scsi.disk.rspc \
                           disk.scsi.540mb3.add%devices.scsi.disk.rspc \
                           disk.scsi.540mb4.add%devices.scsi.disk.rspc \
                           disk.scsi.540mb5.add%devices.scsi.disk.rspc \
                           disk.scsi.730mb2.add%devices.scsi.disk.rspc \
                           disk.scsi.810mb.add%devices.scsi.disk.rspc \
                           disk.scsi.1000mb2.add%devices.scsi.disk.rspc \
                           disk.scsi.1080mb.add%devices.scsi.disk.rspc
			


INFO_FILES		+= devices.scsi.disk.rspc.prereq

