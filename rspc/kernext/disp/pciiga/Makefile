# @(#)39	1.6  src/rspc/kernext/disp/pciiga/Makefile, pciiga, rspc41J, 9515B_all 3/22/95 16:37:49
#
#   COMPONENT_NAME: (PCIIGA) S3 PCI Graphics Adapter Device Driver
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
KERNEL_EXT              = igadd igadd_pin
IDIR                    = /usr/lib/drivers/pci/
ILIST                   = igadd igadd_pin
VPATH                   +=:./config:./intr:./ksr:./pm:./ras:../common
LIBS                    = ${KERNEXT_LIBS}
CFLAGS                  += $(RS2_601) # -DGS_TRACE


#            -D__SYSPEST_LVL=100 \
#            -DDEBUG \
#            -g -qcheck -qextchk -qattr -qlist -qlistopt     \
#            -qnoro -qsource -qxref  -qflag=I:I

igadd_ENTRYPOINT        = iga_config
igadd_pin_ENTRYPOINT    = iga_intr
igadd_IMPORTS           = -bI:igadd_pin.exp
igadd_EXPORTS           = -bE:igadd.exp

igadd_pin_IMPORTS       = -bI:igadd.exp
igadd_pin_EXPORTS       = -bE:igadd_pin.exp

igadd_pin_OFILES        =                 \
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
                          iga_err.o       \
                          open.o          \
                          close.o         \
                          interrupt.o     \
                          init_iga.o      \
                          configb_diag.o  \
			  vtt_dpm.o       \
			  iga_pm.o        \
			  iga_memtrc.o    \
			  enable_iga.o rw_pci_cfg.o    
      
igadd_OFILES            =               \
                          config.o        \
                          ioctl.o         \
                          define.o        \
                          configa_diag.o

INCFLAGS = -I./inc

.include <${RULES_MK}>
