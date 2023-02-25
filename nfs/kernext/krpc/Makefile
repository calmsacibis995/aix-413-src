#
#
#   ORIGINS: 27
#
#

IDIR            = /usr/lib/drivers/
ILIST           = nfs_krpc.ext


OTHERS =
OTHERS += nfs_krpc.ext
nfs_krpc.ext: nfs_krpc.ext.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include        <${RULES_MK}>
