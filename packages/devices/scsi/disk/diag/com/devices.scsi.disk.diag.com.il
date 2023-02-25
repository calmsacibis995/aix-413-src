# @(#)66	1.2  src/packages/devices/scsi/disk/diag/com/devices.scsi.disk.diag.com.il, pkgdevdiag, pkg411, GOLD410 4/20/94 19:12:35
#
# COMPONENT_NAME: pkgdevdiag
#
# FUNCTIONS:  inslist
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993,1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# -------------------------------------------------------------------------
#
# LPP_NAME: devices.scsi.disk.diag.com
#
# This file has the following format:  
#
#   There will be one line per file or directory.  There will be an 
#   additional line for each if there are hard links or symlinks that 
#   need to be created for this command.  There should be no blanks between
#   the lines of this file. The comment character of the file is a "#".
#
#	<type> <owner> <group> <mode>	<file or directory name>
#
#   and if there are links to be added :
#
#	<link type>			<source> <target> [<target> ....]
#
#   The valid entries for each field is as follows :
#
#      type -> F (file), D (directory), V (volatile size file), C (character
#              device), B (block device), A (apply only file), I (inventory
#              only file), T (security file) and U (security file of volatile
#              size)
#
#      owner -> 0 (root), 1 (daemon), 2 (bin), 3 (sys), 4 (adm), 5 (uucp) and
#		100 (guest)
#
#      group -> 0 (system), 1 (staff), 2 (bin), 3 (sys), 4 (adm), 5 (uucp),
#		6 (mail), 7 (security), 8 (cron), 9 (printq), 10 (audit),
#		28 (ecs) and 100 (usr)
#
#      mode -> standard modes as defined on the target system
#
#      filename -> full path name to the file or directory
#
#      link type -> H (hard links) and S (symlinks)
#
#      source -> full path name to the source file or directory for the link
#
#      target -> full path name to the target file or directory for the link
#
# This file should only be edited by the owner of the product that the file
# represents.
#
# NOTE : Special handling of the root part for an LPP.  Instead of having two
# 	 inslists in 4.1, there will be only one inslist.  The /usr/lpp/<lpp>/
#	 inst_root/  will be stripped off of each file name and the file type
#	 will be indicated with a lower case letter.  For example, the entry
#	 for /etc will be :
#		 d 0 0 755	/etc
#	 Which will be translated into :
#		 D 0 0 755	/etc
#
#
#                        devices.scsi.disk.diag.com
#
F 0 0 544       /usr/lpp/diagnostics/bin/uformat dd-diag
F 0 0 544       /usr/lpp/diagnostics/bin/srwfor dd-diag
F 0 0 544       /usr/lpp/diagnostics/bin/kazfor dd-diag
F 0 0 444       /usr/lpp/diagnostics/catalog/default/ufmt.cat dd-diag
F 0 0 444       /usr/lpp/diagnostics/catalog/default/kazfor.cat dd-diag
