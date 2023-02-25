# @(#)10 1.2 src/packages/printers/bullpr90/rte/packdep.mk, pkgdevprint, pkg411, GOLD410 7/4/94 09:13:29
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

USR_LIBLPP_LIST		+= printers.bullpr90.rte.pre_d

ROOT_LIBLPP_LIST	+=

USR_ODMADD_LIST		+= printer.bullpr90.diag.add%printers.bullpr90.rte \
			   printer.bullpr90.add%printers.bullpr90.rte
ROOT_ODMADD_LIST	+=

INFO_FILES      	+= printers.bullpr90.rte.prereq
