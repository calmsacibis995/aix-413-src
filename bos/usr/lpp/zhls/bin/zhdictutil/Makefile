#
#
#   ORIGINS: 27
#
#

ILIST		= zhdictutil ZHdictutil
IDIR		= /usr/lpp/zhls/bin/
ZHdictutil_IDIR	= /usr/lpp/X11/lib/X11/ZH_CN/app-defaults/


OTHERS =
OTHERS += zhdictutil
zhdictutil: zhdictutil.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
