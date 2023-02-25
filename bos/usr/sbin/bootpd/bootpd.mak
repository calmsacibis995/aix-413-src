#
# @(#)Makefile
#
# @(#)31        1.4  src/bos/usr/sbin/bootpd/bootpd.mak, cmdnet, bos411, 9428A410j 12/18/91 %U
#
# (C) COPYRIGHT International Business Machines Corp. 1991
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# Permission to use, copy, modify, and distribute this program for any
# purpose and without fee is hereby granted, provided that this copyright,
# permission and disclaimer notice appear on all copies and supporting
# documentation; the name of IBM not be used in advertising or publicity
# pertaining to distribution of the program without specific prior
# permission; and notice be given in supporting documentation that copying
# and distribution is by permission of IBM.  IBM makes no representations
# about the suitability of this software for any purpose.  This program
# is provided "as is" without any express or implied warranties, including,
# without limitation, the implied warranties of merchantability and fitness
# for a particular purpose.
#
# ORIGINS: 27
#-----------------------------------------------------------------------#
#   Makefile for bootpd   - BOOTP server daemon
#-----------------------------------------------------------------------#
#
#  You should make bootpd from the /usr/lib/dwm directory.  The file
#  /usr/lib/dwm/makefile explains how to make bootpd.
#
CC_OBJ = hash.o version.o bootpd.o readfile.o 
CFLAGS = -DPRIVATE=static -DFEATURE $(DEFINES)

bootpd : $(CC_OBJ)
	cc $(CFLAGS) -o bootpd $(CC_OBJ)
