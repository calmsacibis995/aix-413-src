#!/bin/ksh
# @(#)47        1.1  src/packages/bos/txt/ts/usr/bos.txt.ts.unpost_i.sh, pkgtxt, pkg411, GOLD410 12/23/92 09:08:52
#
# COMPONENT_NAME: pkgtxt
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993.
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
##
# NAME: bos.txt.ts.unpost_i
#
# FUNCTION: script called to do cleanup if install fails for
#           Transcript font families.
#
#
# RETURN VALUE DESCRIPTION:
#          always 0
##
#
#
##########
# Ignore user interrupts
trap "" 1 2 3 15

cd /usr/lib/ps/ditroff.font

# This logic equivalent to:
# 	make -f Makefile.fonts cleanall

# font family directories
rm -rf [A-Z]*

# troff dependent postscript font directory
rm -rf /usr/lib/font/devpsc

# files used to make fonts and font families
rm -f *.o ? ?? core DESC *.out *.font *.aux *.CKP *.BAK temp* *.map 

# Restore interrupts
trap 1 2 3 15
exit 0
