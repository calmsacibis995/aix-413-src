#
#
#
#   ORIGINS: 27
#
#
#

IDIR            = /usr/lib/drivers/
ILIST           = nfs_kdes_null.ext


OTHERS =
OTHERS += nfs_kdes_null.ext
nfs_kdes_null.ext: nfs_kdes_null.ext.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include        <${RULES_MK}>
