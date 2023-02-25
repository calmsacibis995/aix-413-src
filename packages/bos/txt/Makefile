# @(#)24        1.6  src/packages/bos/txt/Makefile, pkgtxt, pkg411, GOLD410 7/5/94 16:01:44
#
# COMPONENT_NAME: pkgtxt
#
# FUNCTIONS: Makefile
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

VPATH		 = bib:bib/data
VPATH		:= ${VPATH}:hplj/fnt
VPATH		:= ${VPATH}:ibm3812/fnt
VPATH		:= ${VPATH}:ibm3816/fnt
VPATH		:= ${VPATH}:spell:spell/data
VPATH		:= ${VPATH}:tfs:tfs/data
VPATH		:= ${VPATH}:ts
VPATH		:= ${VPATH}:xpv:xpv/rte:xpv/dps

BFF		= bos.txt

ILIST		= ${BFF} ${BFF}.data
IDIR		= /

OPTIONS		= bos.txt.bib \
		  bos.txt.hplj.fnt \
		  bos.txt.ibm3812.fnt \
		  bos.txt.ibm3816.fnt \
		  bos.txt.spell \
		  bos.txt.tfs \
		  bos.txt.ts \
		  bos.txt.xpv.rte \
		  bos.txt.xpv.dps

SHARE_OPTIONS	= bos.txt.bib.data \
		  bos.txt.spell.data \
		  bos.txt.tfs.data

.include <bib/packdep.mk>
.include <bib/data/packdep.mk>
.include <hplj/fnt/packdep.mk>
.include <ibm3812/fnt/packdep.mk>
.include <ibm3816/fnt/packdep.mk>
.include <spell/packdep.mk>
.include <spell/data/packdep.mk>
.include <tfs/packdep.mk>
.include <tfs/data/packdep.mk>
.include <ts/packdep.mk>
.include <xpv/rte/packdep.mk>
.include <xpv/dps/packdep.mk>

.include <${RULES_MK}>

