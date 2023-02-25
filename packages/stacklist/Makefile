# @(#)17        1.21  src/packages/stacklist/Makefile, pkg, pkg41J, 9523B_all 6/7/95 16:54:00
#
#   COMPONENT_NAME: PKG
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#


OTHERS		= stack.server stack.client stack.powerdt \
		  stack.hypertape stack.hypercd stack.development \
		  stack.blkmux stack.des stack.dps stack.dsmit \
		  stack.essl stack.icraft stack.infoxl stack.hcon \
		  stack.perfagent stack.perfmgr stack.netware \
		  stack.sx25lite stack.sx25 stack.uimx stack.ums \
		  stack.opengl stack.pex stack.sna stack.sna16 \
		  stack.sna64 stack.snaunlimit stack.wabi stack.soft5080 \
		  stack.soft5080japan stack.hacmp stack.hacmpclvm \
		  stack.nnu stack.dce_cdmf stack.dce_cds stack.dce_security  \
		  stack.dce_edfs stack.dce_dfsnfs stack.dce_tools \
		  stack.cics stack.encina_client stack.encina_server \
		  stack.encina_ppcexec stack.encina_ppcgate stack.encina_sfs \
		  stack.encina_monitor stack.encina_monitor_suite \
		  stack.dce_client stack.internet_server stack.netbios \
		  stack.commonpoint_cls stack.commonpoint_adt

SCPP	  	= ${CPP} -P ${_GENINC_}

ILIST		= ${OTHERS}

IDIR			= /MASTER_STACK/
stack.development_IDIR	= /

.include <${RULES_MK}>

stack.server : aix.master
	${SCPP} -DSERVER  ${aix.master:P} | \
		${SED} '/^ $$/d' > ${.TARGET}

stack.client : aix.master
	${SCPP}  -DCLIENT ${aix.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.powerdt : aix.master ums.master
	${SCPP}   -DPERSONAL ${aix.master:P} | \
		${SED} '/^ $$/d' > ${.TARGET}

stack.development : aix.master blkmux.master cics.master dce.master \
		des.master dev.master dps.master dsmit.master \
		encina.master essl.master hacmp.master hcon.master \
		hyper.master icraft.master infoxl.master \
		internet_server.master netware.master \
		nnu.master opengl.master perf.master pex.master sna.master \
		soft5080.master sx25.master uimx.master ums.master \
		wabi.master netbios.master
	${SCPP}   -DDEV ${dev.master:P} | \
		${SED} '/^ $$/d' > ${.TARGET}

stack.hypercd : hyper.master
	${SCPP}  -DHYPERCD ${hyper.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.hypertape : hyper.master
	${SCPP}  -DHYPERTAPE ${hyper.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.blkmux : blkmux.master
	${SCPP}   ${blkmux.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.cics : cics.master encina.master
	${SCPP}   ${cics.master:P} |${SED} '/^ $$/d' > ${.TARGET}
	${SCPP}   -DENCINACLIENT -DENCINAMSG -DENCINASFS -DENCINASERVER \
		-DENCINAPPCEXEC ${encina.master:P} | ${SED} '/^ $$/d' >> ${.TARGET}

stack.commonpoint_cls : commonpt.master
	${SCPP}   -DCOMMONPTCLS ${commonpt.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.commonpoint_adt : commonpt.master
	${SCPP}   -DCOMMONPTADT ${commonpt.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.dce_client : dce.master
	${SCPP}   -DDCECLIENT -DDCE ${dce.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.dce_cdmf : dce.master
	${SCPP}   -DDCECDMF ${dce.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.dce_cds : dce.master
	${SCPP}   -DDCECDS -DDCE ${dce.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.dce_security : dce.master
	${SCPP}   -DDCESECUR -DDCE ${dce.master:P} | \
		${SED} '/^ $$/d' > ${.TARGET}

stack.dce_edfs : dce.master
	${SCPP}   -DDCEEDFS -DDCE ${dce.master:P} | \
		${SED} '/^ $$/d' > ${.TARGET}

stack.dce_dfsnfs : dce.master
	${SCPP}   -DDCEDFSNFS -DDCE ${dce.master:P} | \
		${SED} '/^ $$/d' > ${.TARGET}

stack.dce_tools : dce.master
	${SCPP}   -DDCETOOLS ${dce.master:P} | \
		${SED} '/^ $$/d' > ${.TARGET}

stack.des : des.master dce.master
	${SCPP}   -DDES -DDCE ${des.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.dps : dps.master
	${SCPP}   ${dps.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.dsmit : dsmit.master
	${SCPP}   ${dsmit.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.encina_client : encina.master
	${SCPP}   -DENCINACLIENT  -DENCINAMSG ${encina.master:P} | \
		${SED} '/^ $$/d' > ${.TARGET}

stack.encina_server : encina.master
	${SCPP}   -DENCINASERVER  -DENCINAMSG ${encina.master:P} | \
		${SED} '/^ $$/d' > ${.TARGET}

stack.encina_ppcexec : encina.master
	${SCPP}   -DENCINAPPCEXEC  -DENCINAMSG ${encina.master:P} | \
		${SED} '/^ $$/d' > ${.TARGET}

stack.encina_ppcgate : encina.master
	${SCPP}   -DENCINAPPCGATE  -DENCINAMSG ${encina.master:P} | \
		${SED} '/^ $$/d' > ${.TARGET}

stack.encina_sfs : encina.master
	${SCPP}   -DENCINASGS  -DENCINAMSG ${encina.master:P} | \
		${SED} '/^ $$/d' > ${.TARGET}

stack.encina_monitor : encina.master
	${SCPP}   -DENCINAMONITOR  -DENCINAMSG ${encina.master:P} | \
		${SED} '/^ $$/d' > ${.TARGET}

stack.encina_monitor_suite : encina.master
	${SCPP}   -DENCINAMONITOR -DENCINACLIENT -DENCINASERVER -DENCINASFS \
		-DENCINAPPCEXEC -DENCINAMSG ${encina.master:P} | \
		${SED} '/^ $$/d' > ${.TARGET}

stack.essl : essl.master
	${SCPP}   ${essl.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.icraft : icraft.master
	${SCPP}   ${icraft.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.infoxl : infoxl.master
	${SCPP}   ${infoxl.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.hacmp : hacmp.master
	${SCPP}   -DHACMP ${hacmp.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.hacmpclvm : hacmp.master
	${SCPP}   -DHACMPCLVM ${hacmp.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.hcon : hcon.master
	${SCPP}   ${hcon.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.internet_server : internet_server.master
	${SCPP}   ${internet_server.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.netware : netware.master
	${SCPP}   ${netware.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.nnu : nnu.master
	${SCPP}   ${nnu.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.perfagent : perf.master
	${SCPP}   ${perf.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.perfmgr : perf.master
	${SCPP}   -DPERFMGR ${perf.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.sx25lite : sx25.master
	${SCPP}   -DSX25LITE ${sx25.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.sx25 : sx25.master
	${SCPP}   -DSX25 ${sx25.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.soft5080 : soft5080.master
	${SCPP}   -DSOFT5080 ${soft5080.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.soft5080japan : soft5080.master
	${SCPP}   -DSOFT5080JAPAN ${soft5080.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.uimx : uimx.master
	${SCPP}   ${uimx.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.ums : ums.master
	${SCPP}   ${ums.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.opengl : opengl.master
	${SCPP}   ${opengl.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.pex : pex.master
	${SCPP}   ${pex.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.sna : sna.master
	${SCPP}   -DSNA ${sna.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.sna16 : sna.master
	${SCPP}   -DSNA16 ${sna.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.sna64 : sna.master
	${SCPP}   -DSNA64 ${sna.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.snaunlimit : sna.master
	${SCPP}   -DSNAUNLIMIT ${sna.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.wabi : wabi.master
	${SCPP}   ${wabi.master:P} |${SED} '/^ $$/d' > ${.TARGET}

stack.netbios : netbios.master
	${SCPP}   ${netbios.master:P} |${SED} '/^ $$/d' > ${.TARGET}
