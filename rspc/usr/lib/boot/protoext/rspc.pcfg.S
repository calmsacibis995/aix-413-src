# @(#)68    1.4  src/rspc/usr/lib/boot/protoext/rspc.pcfg.S, bosboot, rspc41J, 9510A 2/17/95 14:41:55
#
# COMPONENT_NAME: rspccfg
#
# FUNCTIONS:  proto config file
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993, 1995
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#	NAME:	rspc.pcfg
#
#	FUNCTION:	This file provides the set of proto extension
#		files needed to be added to the base proto for the
#		rspc platform. It is used by bosboot to generate the
#		names of the proto extension files. The scheme used in
#		generating the proto extension file names is:
#
#		<base>.proto.ext.<entry-in-this-file>*
#
#		e.g., for base=disk, with entries in this file to be:
#			base.
#			scsi.
#		the proto extension file names generated will be:
#		disk.proto.ext.base.*
#		disk.proto.ext.scsi.*
#
#		NOTE: make sure a dot "." is included if it is necessary
#		to delimit the field.
#
base.
isa.
pci.
scsi.disk.
sys.pci.
isa_sio.
ide.
