/* @(#)85	1.2  src/nfs/usr/include/nfs/netpriv.h, sysxnfs, nfs411, GOLD410 2/13/94 20:29:58 */
/*
 *   COMPONENT_NAME: SYSXNFS
 *
 *   FUNCTIONS: PRIV_CLR
 *		PRIV_ISSET
 *		PRIV_SETB
 *		PRIV_ZERO
 *
 *   ORIGINS: 26,27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * The following macros will test and set bits in the privilege bit array.
 * These routines are based on the Berkeley routines to test and set file
 * bits.  We have a minor alteration to the macro to use n-1 instead of "n"
 * since privileges are not zero based.
 */

#ifndef __NETPRIV_HEADER__
#define __NETPRIV_HEADER__
#define NPRBITS (sizeof(unsigned long) * NBBY)

#define PRIV_SETB(n, p) \
	((p)->pv_priv[((n)-1)/NPRBITS] |= (1 << (((n)-1) % NPRBITS)))

#define PRIV_CLR(n, p) \
	((p)->pv_priv[((n)-1)/NPRBITS] &= ~(1 << (((n)-1) % NPRBITS)))

#define PRIV_ISSET(n, p) \
	((p)->pv_priv[((n)-1)/NPRBITS] & (1 << (((n)-1) % NPRBITS)))

#define PRIV_ZERO(p) \
	bzero((char *)(p), sizeof(*(p)))

/*
 * This array is a translation table for local to network privs.  There is
 * a 1 to 1 correspondence now, but we cannot rely on it.  We index into the
 * array using the net privilege number and get our privilege number from it.
 */
long pcl_net_to_local[] = {
	 1,	/* BYPASS_DAC_WRITE */
	 2,	/* BYPASS_DAC_READ */
	 3,	/* BYPASS_DAC_EXEC */
	 4,	/* BYPASS_DAC_KILL */
	 5,	/* BYPASS_RAC */
	 6,	/* BYPASS_MAC_WRITE */
	 7,	/* BYPASS_MAC_READ */
	 8,	/* BYPASS_TPATH */
	 9,	/* BYPASS_DAC */
	10,	/* SET_OBJ_DAC */
	11,	/* SET_OBJ_RAC */
	12,	/* SET_OBJ_MAC */
	13,	/* SET_OBJ_INFO */
	14,	/* SET_OBJ_STAT */
	15,	/* SET_OBJ_PRIV */
	16,	/* Not Defined */
	17,	/* Not Defined */
	18,	/* Not Defined */
	19,	/* Not Defined */
	20,	/* SET_PROC_DAC */
	21,	/* SET_PROC_RAC */
	22,	/* SET_PROC_MAC */
	23,	/* SET_PROC_INFO */
	24,	/* SET_PROC_ENV */
	25,	/* SET_PROC_ACCT */
	26,	/* SET_PROC_AUDIT */
	27,	/* Not Defined */
	28,	/* Not Defined */
	29,	/* Not Defined */
	30,	/* Not Defined */
	31,	/* Not Defined */
	32,	/* Not Defined */
	33,	/* Not Defined */
	34,	/* Not Defined */
	35,	/* Not Defined */
	36,	/* Not Defined */
	37,	/* Not Defined */
	38,	/* Not Defined */
	39,	/* Not Defined */
	40,	/* AUDIT_CONFIG */
	41,	/* ACCT_CONFIG */
	42,	/* DEV_CONFIG */
	43,	/* FS_CONFIG */
	44,	/* GSS_CONFIG */
	45,	/* LVM_CONFIG */
	46,	/* NET_CONFIG */
	47,	/* RAS_CONFIG */
	48,	/* RAC_CONFIG */
	49,	/* SYS_CONFIG */
	50,	/* SYS_OPER */
	51,	/* TPATH_CONFIG */
	52,	/* VMM_CONFIG */
	53,	/* Not Defined */
	54,	/* Not Defined */
	55,	/* Not Defined */
	56,	/* Not Defined */
	57,	/* Not Defined */
	58,	/* Not Defined */
	59,	/* Not Defined */
	60,	/* Not Defined */
	61,	/* Not Defined */
	62,	/* Not Defined */
	63,	/* Not Defined */
	64	/* Not Defined */
};

#define PRIV_MAX_TRANS (sizeof(pcl_net_to_local) / sizeof(long))

/*
 * Since there is currently a 1 to 1 correspondence we define pcl_local_to_net
 * to be the same array.  If there is a split in the correspondence we will
 * have to put in another array
 */

#define pcl_local_to_net	pcl_net_to_local

extern int nfsdebug;
#ifndef dprint
#define dprint if (nfsdebug > 2) __dprint__
#endif

/*
 * Funtion prototypes from en_decode.c
 */
extern int net_id_to_pce_local(access_identifier *, struct pce_id **);
extern int net_id_to_ace_local(access_identifier *, struct ace_id **);
extern int pce_local_to_net_id(access_identifier *, struct pce_id *);
extern int ace_local_to_net_id(access_identifier *, struct ace_id *);
extern int net_priv_to_local(pcl_privlist *, priv_t *);
extern int local_to_net_priv(pcl_privlist *, priv_t *);
extern int net_pclentry_to_local(pcl_entry_xdr *, struct pcl_entry **);
extern int local_to_net_pclentry(pcl_entry_xdr *, struct pcl_entry *);
extern int net_pcl_to_local(pcl_xdr *, struct pcl **);
extern int local_to_net_pcl(pcl_xdr *, struct pcl *);
extern int free_pcl_entry_xdr(pcl_entry_xdr *);
extern int free_pcl_xdr(pcl_xdr *);
extern int net_aclentry_to_local(acl_entry_xdr *, struct acl_entry **);
extern int acc_local_to_net(acc_permlist *, int, int);
extern int local_to_net_aclentry(acl_entry_xdr *, struct acl_entry *);
extern int net_acl_to_local(acl_xdr *, struct acl **);
extern int local_to_net_acl(acl_xdr *, struct acl *);
extern int free_netid(access_identifier *);
extern int free_acl_entry_xdr(acl_entry_xdr *);
extern int free_acl_xdr(acl_xdr *);

/*
 * Function prototypes from pacl_ops.c
 */
extern acl_pcl_inuse *acl_pcl_stat_1(fh_cookie *, struct svc_req *, struct exportinfo *, struct ucred *);
extern rep_acl_pcl_cookie *pcl_stat_long_1(fh_cookie *, struct svc_req *, struct exportinfo *, struct ucred *);
extern rep_acl_pcl_cookie *acl_stat_long_1(fh_cookie *, struct svc_req *, struct exportinfo *, struct ucred *);
extern acl_pcl_reply *acl_read_1(acl_pcl_req_block *, struct svc_req *, struct exportinfo *, struct ucred *);
extern acl_pcl_reply *pcl_read_1(acl_pcl_req_block *, struct svc_req *, struct exportinfo *, struct ucred *);
extern int *acl_write_1(acl_pcl_block *, struct svc_req *, struct exportinfo *, struct ucred *);
extern int *pcl_write_1(acl_pcl_block *, struct svc_req *, struct exportinfo *, struct ucred *);
extern struct	cache_block *get_cache_block(fh_cookie *, u_long, int *, struct ucred *, struct exportinfo *);
extern int	sum_block(char *, int);
extern char	*get_acl(struct vnode *, int *, u_long *, struct ucred *);
extern char	*get_pcl(struct vnode *, int *, u_long *, struct ucred *);
extern struct write_data *get_wr_block(acl_pcl_cookie *, int, int *, struct exportinfo *);
extern void burn_wr(struct write_data *, int);

/*
 * Functions for the XDR routines
 */
extern bool_t	xdrspec_create(XDR *, caddr_t, u_int, enum xdr_op);

#endif
