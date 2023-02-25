# 
# 
#
# ORIGINS: 27 
#
#
# 

XFILES		= \
	bootparam_prot.x	\
	ether.x			\
	klm_prot.x		\
	mount.x			\
	nfs_prot.x		\
	nlm_prot.x		\
	rex.x			\
	rnusers.x		\
	rquota.x		\
	rstat.x			\
	sm_inter.x		\
	spray.x			\
	yp.x			\
	yppasswd.x		\
	ypupdate_prot.x

DATAFILES	= \
	klm_prot.h		\
	mount.h			\
	nfs_prot.h		\
	nlm_prot.h		\
	rex.h			\
	rstat.h			\
	rusers.h		\
	rwall.h			\
	sm_inter.h		\
	spray.h			\
	yppasswd.h

INCLUDES	= \
	${DATAFILES} \
	exportent.h

IDIR		= /usr/include/rpcsvc/

ILIST		= ${DATAFILES} ${XFILES}

.include <${RULES_MK}>
