# @(#)71	1.8  src/rspc/kernext/disp/pcigga/Makefile, pcigga, rspc41J, 9513A_all 3/22/95 10:49:37
#
#   COMPONENT_NAME: (PCIGGA) Weitek PCI Graphics Adapter Device Driver
#
#   FUNCTIONS:
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

EXPINC_SUBDIRS          = inc
KERNEL_EXT              = ggadd ggadd_pin 
IDIR                    = /usr/lib/drivers/pci/
ILIST                   = ggadd ggadd_pin ggadd_video
VPATH                   +=:./config:./intr:./ksr:./ras:./pm:../common
LIBS                    = ${KERNEXT_LIBS}
CFLAGS                  += $(RS2_601) \
#	                -D__SYSPEST_LVL=0 \
#			-DDEBUG \
#	                -qLIST  \
#	                -g -qcheck -qextchk -qattr -qlist -qlistopt     \
#	                -qnoro -qsource -qxref  -qflag=I:I

ggadd_ENTRYPOINT        = gga_config
ggadd_pin_ENTRYPOINT    = gga_intr
#ggadd_video_ENTRYPOINT  = video_ioctl
ggadd_IMPORTS           = -bI:ggadd_pin.exp -bI:ggadd_video.exp
ggadd_EXPORTS           = -bE:ggadd.exp

ggadd_pin_IMPORTS       = -bI:ggadd.exp -bI:ggadd_video.exp
ggadd_pin_EXPORTS       = -bE:ggadd_pin.exp

#ggadd_video_IMPORTS       = -bI:ggadd.exp -bI:ggadd_pin.exp
ggadd_video_EXPORTS       = -bE:ggadd_video.exp

ggadd_pin_OFILES        =               \
	                copyps.o        \
	                draw.o          \
	                font.o          \
	                load_pal.o      \
	                makegp.o        \
	                moveit.o        \
	                reset.o         \
	                vtt_act.o       \
	                vtt_cfl.o       \
	                vtt_clr.o       \
	                vtt_cpl.o       \
	                vtt_dact.o      \
	                vtt_defc.o      \
	                vtt_init.o      \
	                vtt_movc.o      \
	                vtt_rds.o       \
	                vtt_scr.o       \
	                vtt_stct.o      \
	                vtt_term.o      \
	                vtt_text.o      \
	                vtt_setm.o      \
	                ksr_diag.o      \
	                intr_diag.o     \
	                gga_err.o       \
	                open.o          \
	                close.o         \
	                interrupt.o     \
	                init_gga.o      \
	                configb_diag.o 	\
			vtt_dpm.o	\
			gga_pm.o 	\
			gga_memtrc.o	\
			gga_trace.o	\
	                enable_gga.o    \
	                rw_pci_cfg.o

ggadd_OFILES            =               \
	                config.o        \
	                ioctl.o         \
	                define.o        \
	                configa_diag.o


INCFLAGS = -I./inc 

.include <${RULES_MK}>
