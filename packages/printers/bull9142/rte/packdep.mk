# @(#)64 1.1 src/packages/printers/bull9142/rte/packdep.mk, pkgdevprint, pkg411, GOLD410 4/28/94 08:40:55
#
# COMPONENT_NAME: pkgdevprint
#
# FUNCTIONS: packaging definitions
#
# ORIGINS: 83
#
# LEVEL 1, 5 Years Bull Confidential Information
#
# *_LIBLPP_LIST	contains the list of files that need to be added to 
#		the respective liblpp.a other than the copyright, .al,
#		.tcb, .size, and .inventory files.  For example, any
#		config scripts would be included here.
#		The variable may be left blank if there are no additional
#		files, but the option will exist.
#

USR_LIBLPP_LIST		+= printers.bull9142.rte.pre_d

ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+= printer.bull9142.diag.add%printers.bull9142.rte \
			   printer.bull9142.add%printers.bull9142.rte
ROOT_ODMADD_LIST	+=

INFO_FILES      += printers.bull9142.rte.prereq
