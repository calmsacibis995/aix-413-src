# @(#)81  1.6  src/rspc/kernext/disp/pcihga/Makefile, pcihga, rspc411, 9432B411a 8/8/94 19:47:42
#
#   COMPONENT_NAME: (PCIHGA) S3 PCI Graphics Adapter Device Driver
#
#   FUNCTIONS:
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1989, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

EXPINC_SUBDIRS          = inc
KERNEL_EXT              = hgadd hgadd_pin
IDIR                    = /usr/lib/drivers/pci/
ILIST                   = hgadd hgadd_pin
VPATH                   +=:./config:./intr:./ksr:../common
LIBS                    = ${KERNEXT_LIBS}
CFLAGS                  += $(RS2_601) \
#			-D__SYSPEST_LVL=0 \
#			-DDEBUG \
#			-g -qcheck -qextchk -qattr -qlist -qlistopt     \
#			-qnoro -qsource -qxref  -qflag=I:I

hgadd_ENTRYPOINT        = hga_config
hgadd_pin_ENTRYPOINT    = hga_intr
hgadd_IMPORTS           = -bI:hgadd_pin.exp
hgadd_EXPORTS           = -bE:hgadd.exp

hgadd_pin_IMPORTS       = -bI:hgadd.exp
hgadd_pin_EXPORTS       = -bE:hgadd_pin.exp

hgadd_pin_OFILES        =               \
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
	                hga_err.o       \
	                open.o          \
	                close.o         \
	                interrupt.o     \
	                init_hga.o      \
	                configb_diag.o  vtt_dpm.o

hgadd_OFILES            =               \
	                config.o        \
	                ioctl.o         \
	                define.o        \
	                enable_hga.o    \
	                configa_diag.o  \
			rw_pci_cfg.o	

INCFLAGS = -I./inc

.include <${RULES_MK}>
