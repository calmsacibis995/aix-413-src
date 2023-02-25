# @(#)75    1.16  src/bos/usr/share/lib/terminfo/Makefile, terminfo, bos41J, 9515A_all 3/30/95 16:08:19
#
#   COMPONENT_NAME: TERMINFO
#
#   FUNCTIONS:
#
#   ORIGINS: 27, 83
#
#   This module contains IBM CONFIDENTIAL code. -- (IBM
#   Confidential Restricted when combined with the aggregated
#   modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1995
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# LEVEL 1, 5 Years Bull Confidential Information
#

IBMTERMS	= dumb ibmpc ibm3101 ibm3151 ibm3161 ibm3162 ibm3163 ibm3164 lft
DECTERMS	= vt100 vt320 vt330 vt340 vs100 
PCTERMS		= sun
TVITERMS	= tvi912 tvi920 tvi925 tvi950 
WYSETERMS	= wyse30 wyse50 wyse60 wyse100 wyse350

OTHERS		= ${DECTERMS} ${IBMTERMS} ${PCTERMS} ${TVITERMS} ${WYSETERMS}
SHIP_DIR	= /usr/share/lib/terminfo

NOUPDATE_LIST = ${OTHERS:!${ECHO} " "${OTHERS} | \
		${SED} "s? .? ${SHIP_DIR}/:&/:&?g; s?: ??g"!}

ILIST	= \
	README adds.ti annarbor.ti ansi.ti att.ti beehive.ti bull.ti cdc.ti \
	colorscan.ti datamedia.ti dec.ti diablo.ti falco.ti fortune.ti \
	general.ti hardcopy.ti hazeltine.ti hds.ti heath.ti homebrew.ti \
	hp.ti ibm.ti lsi.ti microterm.ti misc.ti pc.ti perkinelmer.ti \
	pmcons.ti print.ti special.ti sperry.ti tektronix.ti teleray.ti \
	televideo.ti ti.ti tymshare.ti visual.ti vt220-pc.ti wyse.ti \
	${OTHERS}
 
IDIR		= ${SHIP_DIR}/
dumb_IDIR	= ${SHIP_DIR}/d/
ibmpc_IDIR	= ${SHIP_DIR}/i/
ibm3151_IDIR	= ${SHIP_DIR}/i/
ibm3101_IDIR	= ${SHIP_DIR}/i/
ibm3151_IDIR	= ${SHIP_DIR}/i/
ibm3161_IDIR	= ${SHIP_DIR}/i/
ibm3162_IDIR	= ${SHIP_DIR}/i/
ibm3163_IDIR	= ${SHIP_DIR}/i/
ibm3164_IDIR	= ${SHIP_DIR}/i/
lft_IDIR    	= ${SHIP_DIR}/l/
vt100_IDIR	= ${SHIP_DIR}/v/
vt320_IDIR	= ${SHIP_DIR}/v/
vt330_IDIR	= ${SHIP_DIR}/v/
vt340_IDIR	= ${SHIP_DIR}/v/
vs100_IDIR	= ${SHIP_DIR}/v/
sun_IDIR	= ${SHIP_DIR}/s/
tvi912_IDIR	= ${SHIP_DIR}/t/
tvi925_IDIR	= ${SHIP_DIR}/t/
tvi920_IDIR	= ${SHIP_DIR}/t/
tvi950_IDIR	= ${SHIP_DIR}/t/
wyse30_IDIR	= ${SHIP_DIR}/w/
wyse50_IDIR	= ${SHIP_DIR}/w/
wyse60_IDIR	= ${SHIP_DIR}/w/
wyse100_IDIR	= ${SHIP_DIR}/w/
wyse350_IDIR	= ${SHIP_DIR}/w/

.include <${RULES_MK}>

${DECTERMS}: dec.built
${IBMTERMS}: ibm.built
${PCTERMS}:  pc.built
${TVITERMS}: tvi.built
${WYSETERMS}: wyse.built

dec.built: dec.ti
	${TOUCH} dec.built
	(TERMINFO=.; export TERMINFO; ${TIC} -v ${.ALLSRC})
	${MV} ?/* .
	
ibm.built: ibm.ti
	${TOUCH} ibm.built
	(TERMINFO=.; export TERMINFO; ${TIC} -v ${.ALLSRC})
	${MV} ?/* .
	
pc.built: pc.ti
	${TOUCH} pc.built
	(TERMINFO=.; export TERMINFO; ${TIC} -v ${.ALLSRC})
	${MV} ?/* .
	
tvi.built: televideo.ti
	${TOUCH} tvi.built
	(TERMINFO=.; export TERMINFO; ${TIC} -v ${.ALLSRC})
	${MV} ?/* .

wyse.built: wyse.ti
	${TOUCH} wyse.built
	(TERMINFO=.; export TERMINFO; ${TIC} -v ${.ALLSRC})
	${MV} ?/* .
