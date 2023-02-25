#!/bin/bsh
# @(#)89	1.4  src/bos/usr/sbin/install/scripts/inudocm.sh, cmdinstl, bos411, 9428A410j 6/28/93 14:28:31
# COMPONENT_NAME: (CMDINSTL) command install
#
# FUNCTIONS: inudocm
#                                                                    
# ORIGINS: 27 
#
# (C) COPYRIGHT International Business Machines Corp. 1991
# All Rights Reserved
# Licensed Material - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#                                                                    
#
# NAME: inudocm
#                                                                    
# FUNCTION: List all customer information and instructions.
#
# ARGUMENTS:
#	All arguments are passed directly to installp.
#
# RETURN VALUE DESCRIPTION:
#    the return code from installp
#
/usr/sbin/installp -i $*
