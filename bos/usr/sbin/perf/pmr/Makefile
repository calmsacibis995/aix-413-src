#
#
#
#   ORIGINS: 27
#
#
#

IDIR                    = /usr/sbin/perf/pmr/
ILIST                   = iostat_ nfsstat_ sar_ \
monitor perfpmr tprof_ config_ netpmon_ pprof_ trace_ \
filemon_ netstat_ ps_ vmstat_ infp_trace


OTHERS=

OTHERS+=iostat_
iostat_:iostat_.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=nfsstat_
nfsstat_:nfsstat_.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=sar_
sar_:sar_.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=monitor
monitor:monitor.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=perfpmr
perfpmr:perfpmr.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=tprof_
tprof_:tprof_.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=config_
config_:config_.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=netpmon_
netpmon_:netpmon_.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=pprof_
pprof_:pprof_.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=trace_
trace_:trace_.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=filemon_
filemon_:filemon_.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=netstat_
netstat_:netstat_.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=ps_
ps_:ps_.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=vmstat_
vmstat_:vmstat_.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

OTHERS+=infp_trace
infp_trace:infp_trace.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
