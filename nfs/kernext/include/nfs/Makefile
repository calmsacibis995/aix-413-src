# 
# 
#
# ORIGINS: 27 
#
#
# 
.include	<${MAKETOP}nfs/NFS.mk>

VPATH		= ${MAKETOP}/nfs/usr/include/nfs

DATAFILES	= \
	export.h \
	nfs.h \
	nfs_clnt.h \
	nfs_trc.h \
	rnode.h \
	nfs_fscntl.h

INCLUDES	= \
	${DATAFILES} \
	nfs_swap.h \
	sunglue.h \
	dnlc.h \
	pacl.h \
	netpriv.h
	
IDIR		= /kernel/include/nfs/

.include <${RULES_MK}>
