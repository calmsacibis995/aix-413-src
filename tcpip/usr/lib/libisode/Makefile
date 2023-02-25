# @(#)74	1.7  src/tcpip/usr/lib/libisode/Makefile, tcpip, tcpip411, 9434A411a 8/18/94 14:09:45
# COMPONENT_NAME:
#
# FUNCTIONS:
#
# ORIGINS: 27
#
# IBM CONFIDENTIAL -- (IBM Confidential Restricted when
# combined with the aggregated modules for this product)
#                  SOURCE MATERIALS
# (C) COPYRIGHT International Business Machines Corp. 1988, 1994
# All Rights Reserved
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

.include <${MAKETOP}tcpip/usr/Make.cfg>

LIBRARIES	= libisode.a
EXPLIB_TARGETS	= export_libisode.a

LOCALCFLAGS	= ${ISODE_PATHS} -DPEPSYPATH=\".:${PEPSYDIRM}\" \
		-DPEPSY_VERSION=2

COMPAT_OFILES	= general.o logger.o select.o signals.o \
		asprintf.o explode.o implode.o isofiles.o isohost.o \
		sel2str.o sprintb.o str2elem.o str2sel.o str2vec.o \
		tailor.o baduser.o chkpassword.o getpassword.o putenv.o \
		chrcnv.o cmd_srch.o lexequ.o lexnequ.o log_tai.o nochrcnv.o \
		rcmd_srch.o smalloc.o sstr2arg.o strdup.o \
		isoservent.o servbyname.o servbyport.o servbysel.o \
		isoaddrs.o str2saddr.o str2taddr.o saddr2str.o taddr2str.o \
		na2norm.o norm2na.o na2str.o pa2str.o internet.o dgram.o \
		bridge.o camtec.o sunlink.o ubcx25.o x25addr.o

TSAP_OFILES	= tsaprovider.o tsaperror.o tsapstate.o \
		fd2tpkt.o str2tpkt.o text2tpkt.o \
		tsaprespond.o tsapinitiate.o tsaplose.o \
		tsaplisten.o tsapmgmt.o tsapmisc.o \
		tp0ts.o ts2tcp.o ts2x25.o ts2bridge.o \
		ts2bsd.o ts2sunlink.o 

SSAP_OFILES	= ssaprovider.o ssaperror.o \
		str2spkt.o text2spkt.o tsdu2spkt.o \
		ssapexec.o ssaprespond.o ssapinitiate.o ssapexpd.o \
		ssaptyped.o ssapcapd1.o ssapcapd2.o ssaptoken.o \
		ssapactivity.o ssapmajor1.o ssapmajor2.o ssapminor1.o \
		ssapminor2.o ssapresync1.o ssapresync2.o ssapabort.o \
		ssapreport.o ssaprelease1.o ssaprelease2.o ssapwrite.o \
		ssapactchk.o ssapselect.o ssaplose.o 

PSAP_OFILES	= addr2ref.o bit2prim.o bitstr2strb.o bit_ops.o dg2ps.o \
		fdx2ps.o flag2prim.o gtime.o int2strb.o isobject.o num2prim.o \
		obj2prim.o objectbyname.o objectbyoid.o ode2oid.o oid_cmp.o \
		oid_cpy.o oid_free.o oid2ode.o pe2pl.o pe2ps.o pe2qb_f.o \
		pe2ssdu.o pe2text.o pe2uvec.o pe_alloc.o pe_cmp.o pe_cpy.o \
		pe_error.o pe_expunge.o pe_extract.o pe_free.o pe_pullup.o \
		pl2pe.o pl_tables.o prim2bit.o prim2flag.o prim2num.o \
		prim2oid.o prim2qb.o prim2real.o prim2set.o prim2str.o \
		prim2time.o ps2pe.o ps_alloc.o ps_error.o ps_flush.o \
		ps_free.o ps_get_abs.o ps_io.o ps_prime.o qb2pe.o qb2prim.o \
		qb2str.o qb_free.o qb_pullup.o qbuf2pe.o qbuf2pe_f.o \
		qbuf2ps.o real2prim.o seq_add.o seq_addon.o seq_del.o \
		seq_find.o set_add.o set_addon.o set_del.o set_find.o \
		sprintoid.o sprintref.o ssdu2pe.o std2ps.o str2oid.o \
		str2pe.o str2prim.o str2ps.o str2qb.o strb2bitstr.o \
		strb2int.o time2prim.o time2str.o tm2ut.o ts2ps.o ut2tm.o \
		uvec2ps.o

DRFILES		= enc.o dec.o fre.o prnt.o util.o
SUPFILES	= vprint.o pepsy_strings.o py_advise.o
PEPSY_PYOFILES	= UNIV_tables.o 
PEPSY_OFILES	= ${DRFILES} ${SUPFILES} ${PEPSY_PYOFILES}

PEPY_PYOFILES	= UPEPY.o VPEPY.o
PEPY_OFILES	= py_pp.o testdebug.o ${PEPY_PYOFILES}


PSAP2_PYOFILES	= PS_tables.o
PSAP2_OFILES	= psaprovider.o psap2error.o \
		psapexec.o psaprespond.o psapinitiate.o psaptoken.o \
		psapactivity.o psapmajor1.o psapmajor2.o psapminor1.o \
		psapminor2.o psapresync1.o psapresync2.o psapabort.o \
		psapreport.o psaprelease1.o psaprelease2.o psapselect.o \
		psaplose.o \
		${PSAP2_PYOFILES}

ACSAP_PYOFILES	= ACS_tables.o OACS_tables.o DSE_tables.o DASE_tables.o
ACSAP_OFILES	= acsaprovider.o acsapblock.o acsaperror.o acsaplose.o \
		acsaprespond.o acsapinitiat.o acsapreleas1.o acsapreleas2.o \
		acsapreleas3.o acsapabort1.o acsapabort2.o acsapfindpci.o \
		acsapaddr.o \
		acsapdse.o aetdase.o isoalias.o \
		acsapstub.o aetseq.o isoentity.o \
		oid2aei.o sprintaei.o \
		is2paddr.o is2saddr.o is2taddr.o \
		acserver1.o acserver2.o \
		dse-c.o \
		${ACSAP_PYOFILES}

ROSAP_PYOFILES	= ROS_tables.o 
ROSAP_OFILES	= rosaperror.o \
		rosapinvoke.o rosapresult.o rosapuerror.o rosapureject.o \
		rosapintr.o rosapwait.o rosapservice.o rosapapdu.o \
		rosapasync.o rosapselect.o rosaplose.o \
		ro2rts.o \
		ro2ps.o \
		ro2ssexec.o ro2ssrespond.o ro2ssinitiat.o ro2ssreleas1.o \
		ro2ssreleas2.o ro2ss.o ro2ssthorn.o ${ROSAP_PYOFILES}

RTSAP_PYOFILES	= RTS_tables.o 
RTSAP_OFILES	= rtsaperror.o \
		rtsappturn.o rtsapgturn.o rtsaptrans.o rtsapwait.o \
		rtsapasync.o rtsapselect.o rtsaplose.o \
		rtsapdtrans.o rtsaputrans.o \
		rt2psrespond.o rt2psinitiat.o rt2psabort.o rt2psreleas1.o \
		rt2psreleas2.o rt2ps.o \
		rt2ssexec.o rt2ssrespond.o rt2ssinitiat.o rt2ssreleas1.o \
		rt2ssreleas2.o rt2ss.o \
		${RTSAP_PYOFILES}

RONOT_PYOFILES	= RONOT_tables.o 
RONOT_OFILES	= ronotbind1.o ronotbind2.o ronotunbind1.o \
		ronotunbind2.o ronotabort.o ronotlose.o \
		${RONOT_PYOFILES}

ROSY_OFILES	= rystub.o rydiscard.o ryoperation.o ryopinvoke.o \
		rygenid.o rydispatch.o rydsresult.o \
		rydsureject.o rywait.o ryopblock.o rydsblock.o \
		ryfind.o rylose.o

OFILES		= ${COMPAT_OFILES} ${TSAP_OFILES} ${SSAP_OFILES} \
		${PSAP_OFILES} ${PEPSY_OFILES} ${PEPY_OFILES} \
		${PSAP2_OFILES} ${ACSAP_OFILES} ${ROSAP_OFILES} \
		${RTSAP_OFILES} ${RONOT_OFILES} ${ROSY_OFILES}

ILIST		= ACS-types.h DSE-types.h libisode.a
IDIR		= /usr/include/isode/pepsy/
IMODE		= 644

libisode.a_IDIR	= /usr/lib/

.include <${RULES_MK}>
