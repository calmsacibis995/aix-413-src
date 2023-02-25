# @(#)84  1.6  src/rspc/kernext/disp/pcifga/Makefile, pcifga, rspc411, 9432B411a 8/9/94 11:32:51
#
#   COMPONENT_NAME: (PCIFGA) Weitek PCI Graphics Adapter Device Driver
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
KERNEL_EXT              = fgadd fgadd_pin
IDIR                    = /usr/lib/drivers/pci/
ILIST                   = fgadd fgadd_pin
VPATH                   +=:./config:./intr:./ksr:../common
LIBS                    = ${KERNEXT_LIBS}
CFLAGS                  += $(RS2_601) \
#	                -D__SYSPEST_LVL=0 \
#			-DDEBUG \
#	                -g -qcheck -qextchk -qattr -qlist -qlistopt     \
#	                -qnoro -qsource -qxref  -qflag=I:I

fgadd_ENTRYPOINT        = fga_config
fgadd_pin_ENTRYPOINT    = fga_intr
fgadd_IMPORTS           = -bI:fgadd_pin.exp
fgadd_EXPORTS           = -bE:fgadd.exp

fgadd_pin_IMPORTS       = -bI:fgadd.exp
fgadd_pin_EXPORTS       = -bE:fgadd_pin.exp

fgadd_pin_OFILES        =               \
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
	                fga_err.o       \
	                open.o          \
	                close.o         \
	                interrupt.o     \
	                init_fga.o      \
	                Prog_ICD.o      \
	                configb_diag.o  vtt_dpm.o

fgadd_OFILES            =               \
	                config.o        \
	                ioctl.o         \
	                define.o        \
	                enable_fga.o    \
	                configa_diag.o  \
	                rw_pci_cfg.o    


INCFLAGS = -I./inc 

.include <${RULES_MK}>
