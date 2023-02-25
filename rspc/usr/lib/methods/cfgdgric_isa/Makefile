#  @(#)37        1.1  src/rspc/usr/lib/methods/cfgdgric_isa/Makefile, dd_artic, rspc41J, 9508A_notx 2/15/95 14:38:20
#
# COMPONENT_NAME: dd_artic -- ARTIC Diagnostic Device driver.
#
# FUNCTIONS: Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1991, 1993
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#############################################################################

#-----------------------------------------------------------------------#
#                                                                       #
#       This makefile makes driver cfg method for artic                 #
#                                                                       #
#-----------------------------------------------------------------------#

PROGRAMS        = cfgdgric_isa

OFILES          = cfgdgric_isa.o

LIBS            = -lcfg -lodm -lrts -ls

INCFLAGS = -I${MAKETOP}/rspc/kernext/isa 

ILIST      = cfgdgric_isa
IDIR       = /usr/lib/methods/

.include <${RULES_MK}>
