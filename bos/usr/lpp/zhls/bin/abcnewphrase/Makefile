#
#
#
#   ORIGINS: 27
#
#
#

ILIST		= abcnewphrase Abcnewphrase_ZH
IDIR		= /usr/lpp/zhls/bin/
Abcnewphrase_ZH_IDIR	= /usr/lpp/X11/lib/X11/ZH_CN/app-defaults/


OTHERS =
OTHERS += abcnewphrase
abcnewphrase: abcnewphrase.ibmstub
	-${RM} -f ${.TARGET}
	${CP} -f ${.ALLSRC} ${.TARGET}

.include <${RULES_MK}>
