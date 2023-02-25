# @(#)88	1.2  src/packages/X11/Dt/ToolTalk/usr/X11.Dt.ToolTalk.prereq.S, pkgxcde, pkg411, GOLD410 5/13/94 10:13:11
#
#   COMPONENT_NAME: pkgxcde
#
#   FUNCTIONS: prereq
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#	[type] - *prereq, *coreq, *ifreq
#	[option] - full name of option with requisite
#	[expression]	"=","<",">" 
#		v - version
#		r - release
#		m - modification number
#		f - fix number
#		p - update of PTF number
#	{o [expression]} - or condition for the option
#[type] [option] [expression] {o [expression]
#
*coreq xlC.rte 3.1.0.0
*prereq bos.net.tcp.client 4.1.0.0
