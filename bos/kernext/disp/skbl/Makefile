# @(#)40        1.1 8/17/94 src/bos/kernext/disp/skbl/Makefile, bluedd, bos411, 9433B411a 16:56:36
#
# COMPONENT_NAME: (bluedd) SKBL Device Driver
#
# FUNCTIONS: Main SKBL DD Makefile
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
# (C) COPYRIGHT International Business Machines Corp. 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#

.include <${MAKETOP}/bos/kernext/Kernext.mk>

#SUBDIRS	= cdd
#EXPINC_SUBDIRS	= inc

VPATH	       +=:../bl/config:../bl/ras:../bl/intr:../bl/ksr:../bl/rcm

KERNEL_EXT	= skbldd skblddpin

IDIR		= /usr/lib/drivers/
ILIST	 	= skbldd skblddpin

INCFLAGS     =	-I../inc			\
		-I../bl/inc			\
		-I../../inc/${TARGET_MACHINE}	\
                -I../../inc

LOCALCFLAGS =	-DBL_SKBL			\
		-DKERNEL                        \
 		-qlist				\
        	-DGS_DEBUG                      \
        	-DDEBUG				
#       	-D_DISPLAYTRACEFLAG             \
#       	-D__SYSPEST_LVL=0               \

LIBS	     =	${KERNEXT_LIBS}			\
		librts.a

skbldd_ENTRYPOINT	= bl_config
#skbldd_IMPORTS		= 
skbldd_IMPORTS		=  
skbldd_EXPORTS		= -bE:

skblddpin_IMPORTS	= 
#skblddpin_EXPORTS	= -bE:
skblddpin_EXPORTS	= -bE: -bE:

skbldd_OFILES 	 = bl_close.o bl_config.o bl_init.o bl_mon.o bl_open.o	\
		   bl_ddf.o bl_diag_dma.o bl_diag_req.o bl_gp.o bl_term.o
 
skblddpin_OFILES = bl_diag.o bl_dmp.o bl_err.o bl_memtrc.o bl_trace.o	\
		   bl_intr.o bl_vttcfl.o bl_vttclr.o bl_vttcpl.o	\
		   bl_vttdefc.o bl_vttmovc.o bl_vttrds.o bl_vttscr.o	\
		   bl_vttsetm.o bl_vttstct.o bl_vttsubrs.o bl_vtttext.o	\
		   bl_hw_init.o bl_vttact.o bl_vttdact.o bl_vttinit.o	\
		   bl_vttterm.o bl_hw_color.o bl_hw_crtc.o bl_blt.o 	\
		   bl_vpd.o bl_mbx.o

.include <${RULES_MK}>
