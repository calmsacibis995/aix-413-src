# @(#)35  1.9  src/rspc/kernext/disp/pciwfg/Makefile, pciwfg, rspc41J, 9517A_all 4/24/95 04:11:30
# Makefile
#
#   COMPONENT_NAME: (PCIWFG) Western Digita PCI Graphics Adapter 
#
#   FUNCTIONS:
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}bos/kernext/Kernext.mk>

EXPINC_SUBDIRS          = inc
KERNEL_EXT              = wfgdd wfgdd_pin
IDIR                    = /usr/lib/drivers/pci/
ILIST                   = wfgdd wfgdd_pin
VPATH                   +=:./config:./intr:./ksr:../common
LIBS                    = ${KERNEXT_LIBS}
CFLAGS                  += $(RS2_601) \
                        -DPM_SUPPORT -DRESIDUAL_NOT_FIX -DPANEL_PROTECTION \
                        -DNO_H8_CONTROL

wfgdd_ENTRYPOINT        = wfg_config
wfgdd_pin_ENTRYPOINT    = wfg_intr

wfgdd_IMPORTS           = -bI:wfgdd_pin.exp 
wfgdd_EXPORTS           = -bE:wfgdd.exp -bE:vca.exp
wfgdd_pin_IMPORTS       = -bI:wfgdd.exp
wfgdd_pin_EXPORTS       = -bE:wfgdd_pin.exp

wfgdd_pin_OFILES        =               \
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
			vtt_dpm.o       \
	                ksr_diag.o      \
	                wfg_err.o       \
	                open.o          \
	                close.o         \
	                interrupt.o     \
	                intr_diag.o     \
	                init_wfg.o      \
	                wfg_pm.o        \
	                configb_diag.o 

wfgdd_OFILES            =               \
	                config.o        \
	                ioctl.o         \
	                define.o        \
	                enable_wfg.o    \
	                configa_diag.o  \
			rw_pci_cfg.o

INCFLAGS = -I./inc

.include <${RULES_MK}>
