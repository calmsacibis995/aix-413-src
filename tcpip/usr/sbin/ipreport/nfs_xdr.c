static char sccsid[] = "@(#)90	1.3  src/tcpip/usr/sbin/ipreport/nfs_xdr.c, tcpras, tcpip411, GOLD410 2/7/92 16:21:59";
/*
 * COMPONENT_NAME: TCPIP nfs_xdr.c
 *
 * ORIGINS: 24 27
 *
 * FUNCTIONS: MINDIRSIZ, nextdp, reclen, roundtoint, rrokfree, rrokwakeup,
 *            xdr_attrstat, xdr_creatargs, xdr_diropargs, xdr_diropres,
 *            xdr_drok, xdr_fattr, xdr_fhandle, xdr_fsok, xdr_getrddirres,
 *            xdr_linkargs, xdr_putrddirres, xdr_rddirargs, xdr_rdlnres,
 *            xdr_rdresult, xdr_readargs, xdr_rnmargs, xdr_rrok, xdr_saargs,
 *            xdr_sattr, xdr_slargs, xdr_srok, xdr_statfs, xdr_timeval,
 *            xdr_writeargs
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1987 by Sun Microsystems, Inc.
 *
 * This file contains the XDR routines to decode and print nfs rpc
 * packets for the ipreport command.
 *
 * It was mostly stolen and modified from nfs312:com/sysx/nfs/nfs_xdr.c
 *
 * Modified to print out data Sat Aug 10 1991
 *
 */

#include "sys/param.h"
#include "sys/pri.h"
#include "sys/systm.h"
#include "sys/user.h"
#include "sys/vnode.h"
#include "sys/file.h"
#include "dirent.h"
#include "sys/mbuf.h"
#include "sys/errno.h"
#include "sys/time.h"
#include "sys/vfs.h"
#include "rpc/types.h"
#include "rpc/xdr.h"
#undef NFSSERVER
#include "nfs.h"
#include "net/spl.h"
#include "netinet/in.h"
#include "ipr.h"


char *xdropnames[] = { "encode", "decode", "free" };



/*
 * These are the XDR routines used to serialize and deserialize
 * the various structures passed as parameters accross the network
 * between NFS clients and servers.
 */

/* clf wrote this one */
myxdr_void(xdrs,blah)
XDR *xdrs;
char *blah;
{
	return (TRUE);
}

/* clf wrote this one */
xdr_enum_nfsstat (xdrs,status)
XDR *xdrs;
enum nfsstat *status;
{
	if (xdr_enum(xdrs,status)){
/*		if (xdrs->x_op==XDR_DECODE) */
/*			prt_nfsstat(*status);*/
		return (TRUE);
	}
	return (FALSE);
}


/*
 * File access handle
 * The fhandle struct is treated a opaque data on the wire
 */
bool_t
xdr_fhandle(xdrs, fh)
	XDR *xdrs;
	fhandle_t *fh;
{

	if (xdr_opaque(xdrs, (caddr_t)fh, NFS_FHSIZE)) {
		return (TRUE);
	}
	return (FALSE);
}


/*
 * Arguments to remote write and writecache
 */
bool_t
xdr_writeargs(xdrs, wa)
	XDR *xdrs;
	struct nfswriteargs *wa;
{
	extern struct xdr_ops xdrmbuf_ops;	/* XXX */
	int len;

	/*
	 * NOTE: we do an xdr_bytes on the encode side for transmit.  If we
	 * decide to do xdrmbuf_putmbuf, we will have to fill in the xmem
	 * descipter for the funky mbuf.  We are the only ones to have
	 * adressability to the buffer (from VMM) at this time.
	 */
	if (xdr_fhandle(xdrs, &wa->wa_fhandle) &&
	    xdr_long(xdrs, (long *)&wa->wa_begoff) &&
	    xdr_long(xdrs, (long *)&wa->wa_offset) &&
	    xdr_long(xdrs, (long *)&wa->wa_totcount)) 
	{
		if (xdr_bytes(xdrs, &wa->wa_data, (u_int *)&wa->wa_count,
			NFS_MAXDATA) )
		{
			return (TRUE);
		}
		else 	/* handy was decremented by xdrmem_getbytes*/
			xdrs->x_handy+=wa->wa_count;
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_writeargs: %s FAILED\n",
	    xdropnames[(int)xdrs->x_op]);
#endif

/*	IPREPORT SPECIFIC KLUDGE KODE 
	If we've arrived at this point, we're probably here because we're
	trying to decode a fragmented write call.  The normal xdr stuff
	doesn't do that so we're violating several layers 
	to cram it in here and make it work.  Sorry. -clf.

	IF YOU CHANGE THIS CODE, TEST IT ON LARGE WRITES.
	
	The following code is stolen and modified from xdrmem_getbytes.
*/

	len=wa->wa_count;
        if (xdrs->x_handy < len)  {
		len=xdrs->x_handy;
		bcopy(xdrs->x_private, wa->wa_data, len);
		xdrs->x_private += len;
		xdrs->x_handy -= len;
/* WARNING, the following line may make you a little queazy.  Done to warn
   c_write_dump() later, that this was a fragged packet, and is shorter 
   than he would normally think.  We never fill in the mbuf anyway.
*/
		wa->wa_mbuf=(struct mbuf *) len;
		return (TRUE);
	}

/* guess i was wrong.  FOAD for some other reason */
	return (FALSE);
}


/*
 * File attributes
 */
bool_t
xdr_fattr(xdrs, na)
	XDR *xdrs;
	register struct nfsfattr *na;
{
	register long *ptr;

#ifdef NFSDEBUG
	dprint(nfsdebug, 6, "xdr_fattr: %s\n",
	    xdropnames[(int)xdrs->x_op]);
#endif
	if (xdrs->x_op == XDR_ENCODE) {
		ptr = XDR_INLINE(xdrs, 17 * BYTES_PER_XDR_UNIT);
		if (ptr != NULL) {
			IXDR_PUT_ENUM(ptr, na->na_type);
			IXDR_PUT_LONG(ptr, na->na_mode);
			IXDR_PUT_LONG(ptr, na->na_nlink);
			IXDR_PUT_LONG(ptr, na->na_uid);
			IXDR_PUT_LONG(ptr, na->na_gid);
			IXDR_PUT_LONG(ptr, na->na_size);
			IXDR_PUT_LONG(ptr, na->na_blocksize);
			IXDR_PUT_LONG(ptr, na->na_rdev);
			IXDR_PUT_LONG(ptr, na->na_blocks);
			IXDR_PUT_LONG(ptr, na->na_fsid);
			IXDR_PUT_LONG(ptr, na->na_nodeid);
			IXDR_PUT_LONG(ptr, na->na_atime.tv_sec);
			IXDR_PUT_LONG(ptr, na->na_atime.tv_usec);
			IXDR_PUT_LONG(ptr, na->na_mtime.tv_sec);
			IXDR_PUT_LONG(ptr, na->na_mtime.tv_usec);
			IXDR_PUT_LONG(ptr, na->na_ctime.tv_sec);
			IXDR_PUT_LONG(ptr, na->na_ctime.tv_usec);
			return (TRUE);
		}
	} else {
		ptr = XDR_INLINE(xdrs, 17 * BYTES_PER_XDR_UNIT);
		if (ptr != NULL) {
			na->na_type = IXDR_GET_ENUM(ptr, enum nfsftype);
			na->na_mode = IXDR_GET_LONG(ptr);
			na->na_nlink = IXDR_GET_LONG(ptr);
			na->na_uid = IXDR_GET_LONG(ptr);
			na->na_gid = IXDR_GET_LONG(ptr);
			na->na_size = IXDR_GET_LONG(ptr);
			na->na_blocksize = IXDR_GET_LONG(ptr);
			na->na_rdev = IXDR_GET_LONG(ptr);
			na->na_blocks = IXDR_GET_LONG(ptr);
			na->na_fsid = IXDR_GET_LONG(ptr);
			na->na_nodeid = IXDR_GET_LONG(ptr);
			na->na_atime.tv_sec = IXDR_GET_LONG(ptr);
			na->na_atime.tv_usec = IXDR_GET_LONG(ptr);
			na->na_mtime.tv_sec = IXDR_GET_LONG(ptr);
			na->na_mtime.tv_usec = IXDR_GET_LONG(ptr);
			na->na_ctime.tv_sec = IXDR_GET_LONG(ptr);
			na->na_ctime.tv_usec = IXDR_GET_LONG(ptr);
/*			if (xdrs->x_op==XDR_DECODE)  prt_fattr( na);*/
			return (TRUE);
		}
	}
	if (xdr_enum(xdrs, (enum_t *)&na->na_type) &&
	    xdr_u_long(xdrs, &na->na_mode) &&
	    xdr_u_long(xdrs, &na->na_nlink) &&
	    xdr_u_long(xdrs, &na->na_uid) &&
	    xdr_u_long(xdrs, &na->na_gid) &&
	    xdr_u_long(xdrs, &na->na_size) &&
	    xdr_u_long(xdrs, &na->na_blocksize) &&
	    xdr_u_long(xdrs, &na->na_rdev) &&
	    xdr_u_long(xdrs, &na->na_blocks) &&
	    xdr_u_long(xdrs, &na->na_fsid) &&
	    xdr_u_long(xdrs, &na->na_nodeid) &&
	    xdr_timeval(xdrs, &na->na_atime) &&
	    xdr_timeval(xdrs, &na->na_mtime) &&
	    xdr_timeval(xdrs, &na->na_ctime) ) {
		return (TRUE);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_fattr: %s FAILED\n",
	    xdropnames[(int)xdrs->x_op]);
#endif
	return (FALSE);
}

/*
 * Arguments to remote read
 */
bool_t
xdr_readargs(xdrs, ra)
	XDR *xdrs;
	struct nfsreadargs *ra;
{

	if (xdr_fhandle(xdrs, &ra->ra_fhandle) &&
	    xdr_long(xdrs, (long *)&ra->ra_offset) &&
	    xdr_long(xdrs, (long *)&ra->ra_count) &&
	    xdr_long(xdrs, (long *)&ra->ra_totcount) ) {
		if (xdrs->x_op==XDR_DECODE)  {
/*			printf("%s",beg_line);*/
/*			printf("READARGS:  offset %d count %d totalcount %d\n",*/
/*			    ra->ra_offset, ra->ra_count, ra->ra_totcount);*/
		}
		return (TRUE);
	}
	return (FALSE);
}

/*
 * Info necessary to free the mapping which is also an mbuf
 */
struct rrokinfo {
	int	(*func)();
	int	done;
	struct vnode *vp;
	struct buf *bp;
	char	*data;
	u_int	count;
};

/*
 * Status OK portion of remote read reply
 */
bool_t
xdr_rrok(xdrs, rrok)
	XDR *xdrs;
	struct nfsrrok *rrok;
{
	int len;
	void freeit();

	if (xdr_fattr(xdrs, &rrok->rrok_attr)) {
		{			/* client side */
			if (xdrs->x_op == XDR_ENCODE &&
			    rrok->rrok_data != NULL /*clf&&
			    xdrmbuf_putbuf(xdrs, rrok->rrok_data,
				 rrok->rrok_count, freeit, rrok->rrok_data)*/) {
#ifdef NFSDEBUG
				dprint(nfsdebug, 6, "rrok copy avoided\n");
#endif
				rrok->rrok_data = 0;
				return(TRUE);
			}
			if (xdr_bytes(xdrs, &rrok->rrok_data,
			    (u_int *)&rrok->rrok_count, NFS_MAXDATA) ) {
				if (xdrs->x_op==XDR_DECODE)  {
/*					printf("%s",beg_line);*/
/*					printf("RROK:  %d addr %x\n",*/
/*					    rrok->rrok_count, rrok->rrok_data);*/
/*					hex_dump(rrok->rrok_data, */
/*						rrok->rrok_count);*/
				}
				return (TRUE);
			}
			else 	/* handy was decremented by xdrmem_getbytes*/
				xdrs->x_handy+=rrok->rrok_count;
			
		}
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_rrok: %s FAILED\n",
	    xdropnames[(int)xdrs->x_op]);
#endif
/*	IPREPORT SPECIFIC KLUDGE KODE 
	If we've arrived at this point, we're probably here because we're
	trying to decode a fragmented read reply.  The normal xdr stuff
	doesn't do that so we're violating several layers and object 
	oriented stuff to cram it in here and make it work.  Sorry. -clf.

	IF YOU CHANGE THIS CODE, TEST IT ON LARGE READS.
	
	The following code is stolen and modified from xdrmem_getbytes.
*/

	len=rrok->rrok_count;
        if (xdrs->x_handy < len)  {
		len=xdrs->x_handy;
		bcopy(xdrs->x_private, rrok->rrok_data, len);
		xdrs->x_private += len;
		xdrs->x_handy -= len;
		/* stick the real size into where the vp should be.. blech! */
		rrok->rrok_vp=(struct vnode *) len;
		return (TRUE);
	}

/* guess i was wrong.  FOAD for some other reason */
	return (FALSE);
}

struct xdr_discrim rdres_discrim[2] = {
	{ (int)NFS_OK, xdr_rrok },
	{ __dontcare__, NULL_xdrproc_t }
};

/*
 * Reply from remote read
 */
bool_t
xdr_rdresult(xdrs, rr)
	XDR *xdrs;
	struct nfsrdresult *rr;
{

	rr->rr_data=(char *)(&rr->rr_vp+sizeof( char *));
	if (xdr_union(xdrs, (enum_t *)&(rr->rr_status),
	      (caddr_t)&(rr->rr_ok), rdres_discrim, xdr_void) ) {
		if (xdrs->x_op==XDR_DECODE)  {
/*			printf("%s",beg_line);*/
/*			printf("READ RESULT: \n");*/
/*			prt_nfsstat(rr->rr_status);*/
		}
		return (TRUE);
	}
	return (FALSE);
}

/*
 * File attributes which can be set
 */
bool_t
xdr_sattr(xdrs, sa)
	XDR *xdrs;
	struct nfssattr *sa;
{

	if (xdr_u_long(xdrs, &sa->sa_mode) &&
	    xdr_u_long(xdrs, &sa->sa_uid) &&
	    xdr_u_long(xdrs, &sa->sa_gid) &&
	    xdr_u_long(xdrs, &sa->sa_size) &&
	    xdr_timeval(xdrs, &sa->sa_atime) &&
	    xdr_timeval(xdrs, &sa->sa_mtime) ) {
		if (xdrs->x_op==XDR_DECODE)  {
		}
		return (TRUE);
	}
	return (FALSE);
}

struct xdr_discrim attrstat_discrim[2] = {
	{ (int)NFS_OK, xdr_fattr },
	{ __dontcare__, NULL_xdrproc_t }
};

/*
 * Reply status with file attributes
 */
bool_t
xdr_attrstat(xdrs, ns)
	XDR *xdrs;
	struct nfsattrstat *ns;
{

	if (xdr_union(xdrs, (enum_t *)&(ns->ns_status),
	      (caddr_t)&(ns->ns_attr), attrstat_discrim, xdr_void) ) {
		if (xdrs->x_op==XDR_DECODE)  {
/*			printf("%s",beg_line); */
/*			printf( "ATTRSTAT: stat %d\n", ns->ns_status);*/
		}
		return (TRUE);
	}
	return (FALSE);
}

/*
 * NFS_OK part of read sym link reply union
 */
bool_t
xdr_srok(xdrs, srok)
	XDR *xdrs;
	struct nfssrok *srok;
{

	if (xdr_bytes(xdrs, &srok->srok_data, (u_int *)&srok->srok_count,
	    NFS_MAXPATHLEN) ) {
		return (TRUE);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_srok: %s FAILED\n",
	    xdropnames[(int)xdrs->x_op]);
#endif
	return (FALSE);
}

struct xdr_discrim rdlnres_discrim[2] = {
	{ (int)NFS_OK, xdr_srok },
	{ __dontcare__, NULL_xdrproc_t }
};

/*
 * Result of reading symbolic link
 */
bool_t
xdr_rdlnres(xdrs, rl)
	XDR *xdrs;
	struct nfsrdlnres *rl;
{
	char fname[NFS_MAXPATHLEN+1];

	if (xdr_union(xdrs, (enum_t *)&(rl->rl_status),
	      (caddr_t)&(rl->rl_srok), rdlnres_discrim, xdr_void) ) {
		strncpy(fname,rl->rl_data,rl->rl_count);
		if (xdrs->x_op==XDR_DECODE)  {
		}
		return (TRUE);
	}
	return (FALSE);
}

/*
 * Arguments to readdir
 */
bool_t
xdr_rddirargs(xdrs, rda)
	XDR *xdrs;
	struct nfsrddirargs *rda;
{

	if (xdr_fhandle(xdrs, &rda->rda_fh) &&
	    xdr_u_long(xdrs, &rda->rda_offset) &&
	    xdr_u_long(xdrs, &rda->rda_count) ) {
		if (xdrs->x_op==XDR_DECODE)  {
/*			printf("%s",beg_line);*/
/*			printf( "RDDIRARGS: off %d, cnt %d\n",*/
/*					    rda->rda_offset,*/
/*					    rda->rda_count);*/
		}
		return (TRUE);
	}
	return (FALSE);
}

/*
 * Directory read reply:
 * union (enum status) {
 *	NFS_OK: entlist;
 *		boolean eof;
 *	default:
 * }
 *
 * Directory entries
 *	struct  direct {
 *		off_t   d_off;	       	       * offset of next entry *
 *		u_long  d_ino;	       * inode number of entry *
 *		u_short d_reclen;	       * length of this record *
 *		u_short d_namlen;	       * length of string in d_name *
 *		char    d_name[MAXNAMLEN + 1];  * name no longer than this *
 *	};
 * are on the wire as:
 * union entlist (boolean valid) {
 * 	TRUE: struct dirent;
 *	      u_long nxtoffset;
 * 	      union entlist;
 *	FALSE:
 * }
 * where dirent is:
 * 	struct dirent {
 *		u_long	de_fid;
 *		string	de_name<NFS_MAXNAMELEN>;
 *	}
 */

#define	nextdp(dp)	((struct dirent *)((int)(dp) + (dp)->d_reclen))
#define MINDIRSIZ(dp)	(sizeof(struct dirent) - (NFS_MAXNAMLEN + 1) + \
				(dp)->d_namlen)

/*
 * ENCODE ONLY
 */
bool_t
xdr_putrddirres(xdrs, rd)
	XDR *xdrs;
	struct nfsrddirres *rd;
{
	struct dirent *dp;
	char *name;
	int size;
	int xdrpos;
	u_int namlen;
	bool_t true = TRUE;
	bool_t false = FALSE;
	int needed;

	if (xdrs->x_op==XDR_DECODE)  {
	}
	if (xdrs->x_op != XDR_ENCODE) {
		return (FALSE);
	}
	if (!xdr_enum(xdrs, (enum_t *)&rd->rd_status)) {
		return (FALSE);
	}
	if (rd->rd_status != NFS_OK) {
		return (TRUE);
	}

	xdrpos = XDR_GETPOS(xdrs);
	for (size = rd->rd_size, dp = rd->rd_entries;
	     size > 0;
	     size -= dp->d_reclen, dp = nextdp(dp) ) {
		if (dp->d_reclen == 0 || MINDIRSIZ(dp) > dp->d_reclen) {
#ifdef NFSDEBUG
			dprint(nfsdebug, 2, "xdr_putrddirres: bad directory\n");
#endif
			return (FALSE);
		}
#ifdef NFSDEBUG
		dprint(nfsdebug, 10,
		    "xdr_putrddirres: entry %d %s(%d) %d %d %d %d\n",
		    dp->d_ino, dp->d_name, dp->d_namlen, dp->d_offset,
		    dp->d_reclen, XDR_GETPOS(xdrs), size);
#endif
		if (dp->d_ino == 0) {
			continue;
		}
		name = dp->d_name;
		namlen = dp->d_namlen;
		/*
		 * Put the check to see if it fits before the insertion.
		 * Sun bug was to overfill the buffer then check.
		 */
		needed = ((namlen + 3) & ~3) + 6 * BYTES_PER_XDR_UNIT;
		if (needed + XDR_GETPOS(xdrs) - xdrpos >= rd->rd_bufsize) {
			rd->rd_eof = FALSE;
			break;
		}

		if (!xdr_bool(xdrs, &true) ||
		    !xdr_u_long(xdrs, &dp->d_ino) ||
		    !xdr_bytes(xdrs, &name, &namlen, NFS_MAXNAMLEN) ||
		    !xdr_u_long(xdrs, (u_long *) &dp->d_offset) ) {
			return (FALSE);
		}
	}
	if (!xdr_bool(xdrs, &false)) {
		return (FALSE);
	}
	if (!xdr_bool(xdrs, &rd->rd_eof)) {
		return (FALSE);
	}
	return (TRUE);
}


/*
 * It appears as though this macro should round to an int if and only if it
 * is not already on an int boundary.  I can see no reason for rounding up
 * an additional int if you're already on an int boundary which is what this
 * macro formerly did.
 *
 * #define roundtoint(x)   (((x) + sizeof(int)) & ~(sizeof(int) - 1))
 */

#define roundtoint(x)	(((x) + (sizeof(int) - 1)) & ~(sizeof(int) - 1))

/*
 * It appears as though this macro should return space required for
 * (namlen+1)+(2*long)+(2*short) rather than (namlen+1)+(2*short)+(long).
 * since a dp contains 2 longs, 2 shorts and a name.
 *
 * #define reclen(dp)	roundtoint(((dp)->d_namlen + 1 + sizeof(u_long) +\
 *				2 * sizeof(u_short)))
 */

#define reclen(dp)	roundtoint(((dp)->d_namlen + 1 + (2*sizeof(u_long)) +\
				(2*sizeof(u_short))))

/*
 * DECODE ONLY
 */
bool_t
xdr_getrddirres(xdrs, rd)
	XDR *xdrs;
	struct nfsrddirres *rd;
{
	char *z;
	struct dirent *dp;
	int size;
	bool_t valid;
        off_t offset = (off_t)-1;
	int hdrprted=0;

	
	/* set rdentries to point exactly past the end of the rdok struct.
	   we're depending on the caller to leave room there for the answer,
	   (which rpc_dump does with &resp.)
	*/
	rd->rd_entries= (struct dirent *)((&rd->rd_entries) + 
						sizeof(rd->rd_entries));

	if (!xdr_enum(xdrs, (enum_t *)&rd->rd_status)) {
		return (FALSE);
	}
	if (rd->rd_status != NFS_OK) {
		return (TRUE);
	}

#ifdef NFSDEBUG
	dprint(nfsdebug, 6, "xdr_getrddirres: %s size %d\n",
	    xdropnames[(int)xdrs->x_op], rd->rd_size);
#endif
	size = MAXPACKET;	/* fake size.  we don't want to save the size 
				   from the call to this result, but we could
				   if we had too. we could add room in
				   the callinfo struct.

				   As a result of this laziness, we'll miss
				   over fill outs i think.
				 */
	dp = rd->rd_entries;

	for (;;) {

		if (!xdr_bool(xdrs, &valid)) {
			return (FALSE);
		}
		if (valid) {
			if (!xdr_u_long(xdrs, &dp->d_ino) ||
			    !xdr_u_short(xdrs, &dp->d_namlen) ||
			    (DIRSIZ(dp) > size) ||
			    !xdr_opaque(xdrs, dp->d_name, (u_int)dp->d_namlen)||
			    !xdr_u_long(xdrs, (u_long *) &dp->d_offset) ) {
				if (xdrs->x_op==XDR_DECODE)  {
					printf("%s",beg_line);
					printf("xdr_getrddirres: entry error\n");
				}
			/*
			 * If this entry won't fit then server more than likely
			 * overfilled his buffer.  Use everything we got thus
			 * far but set rd->rd_eof to false and ignore this
			 * entry, we'll get it on the next read.
			 */
				if (DIRSIZ(dp) > size) 
					rd->rd_eof = FALSE;
				goto over_fill_out;
			}

			dp->d_reclen = DIRSIZ(dp);
			dp->d_name[dp->d_namlen] = '\0';
			offset = dp->d_offset;

			if (xdrs->x_op==XDR_DECODE)  {
				if (!hdrprted){
/*					getrddirres_prthdr();*/
					hdrprted=1;
				}
/*				printf("%s",beg_line);*/
/*				printf( "GETRDDIRRES: entry %d %s(%d) %d %d\n",*/
/*				    dp->d_ino, dp->d_name, dp->d_namlen,*/
/*				    dp->d_reclen, dp->d_offset);*/
			}
		} else {
			break;
		}
		size -= reclen(dp);
		dp = nextdp(dp);
		/*
		 * Need to make sure we have room for dp->d_namlen (when we
		 * go back to the top of this for loop) which consumes offsets
		 * 10 and 11 of dp.  Since the min size for dp is 16 we use
		 * this number rather than 11.
		 */
		if (size < 16) {
			rd->rd_eof = FALSE;
			goto over_fill_out;
		}
	}
	if (!xdr_bool(xdrs, &rd->rd_eof)) {
		return (FALSE);
	}
over_fill_out:
	rd->rd_offset = offset;
	rd->rd_size = (int)dp - (int)(rd->rd_entries);
	if (xdrs->x_op==XDR_DECODE)  {
/*		printf("%s",beg_line); */
/*		printf( "GETRDDIRRES: returning size %d offset %d eof %d\n",*/
/*	    		rd->rd_size, rd->rd_offset, rd->rd_eof);*/
	}
	return (TRUE);
}

/*
 * Arguments for directory operations
 */
bool_t
xdr_diropargs(xdrs, da)
	XDR *xdrs;
	struct nfsdiropargs *da;
{

	if (xdr_fhandle(xdrs, &da->da_fhandle) &&
	    xdr_string(xdrs, &da->da_name, NFS_MAXNAMLEN) ) {
		if (xdrs->x_op==XDR_DECODE)  {
/*			printf("%s",beg_line);*/
/*			printf("DIR OPER ARGS:  '%s'\n", da->da_name);*/
		}
		return (TRUE);
	}
	return (FALSE);
}

/*
 * NFS_OK part of directory operation result
 */
bool_t
xdr_drok(xdrs, drok)
	XDR *xdrs;
	struct nfsdrok *drok;
{

	if (xdr_fhandle(xdrs, &drok->drok_fhandle) &&
	    xdr_fattr(xdrs, &drok->drok_attr) ) {
		return (TRUE);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_drok: FAILED\n");
#endif
	return (FALSE);
}

struct xdr_discrim diropres_discrim[2] = {
	{ (int)NFS_OK, xdr_drok },
	{ __dontcare__, NULL_xdrproc_t }
};

/*
 * Results from directory operation 
 */
bool_t
xdr_diropres(xdrs, dr)
	XDR *xdrs;
	struct nfsdiropres *dr;
{

	if (xdr_union(xdrs, (enum_t *)&(dr->dr_status),
	      (caddr_t)&(dr->dr_drok), diropres_discrim, xdr_void) ) {
		if (xdrs->x_op==XDR_DECODE)  {
/*			printf("%s",beg_line);*/
/*			printf("DIR OPER RES: stat %d\t", dr->dr_status);*/
/*			prt_nfsstat(dr->dr_status);*/
		}
		return (TRUE);
	}
	return (FALSE);
}

/*
 * Time structure
 */
bool_t
xdr_timeval(xdrs, tv)
	XDR *xdrs;
	struct timeval *tv;
{

	if (xdr_long(xdrs, &tv->tv_sec) &&
	    xdr_long(xdrs, &tv->tv_usec) ) {
		if (xdrs->x_op==XDR_DECODE)  {
		}
		return (TRUE);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_timeval: FAILED\n");
#endif
	return (FALSE);
}

/*
 * arguments to setattr
 */
bool_t
xdr_saargs(xdrs, argp)
	XDR *xdrs;
	struct nfssaargs *argp;
{

	if (xdrs->x_op==XDR_DECODE)  {
	}
	if (xdr_fhandle(xdrs, &argp->saa_fh) &&
	    xdr_sattr(xdrs, &argp->saa_sa) ) {
		return (TRUE);
	}
	return (FALSE);
}

/*
 * arguments to create and mkdir
 */
bool_t
xdr_creatargs(xdrs, argp)
	XDR *xdrs;
	struct nfscreatargs *argp;
{

	if (xdrs->x_op==XDR_DECODE)  {
	}
	if (xdr_diropargs(xdrs, &argp->ca_da) &&
	    xdr_sattr(xdrs, &argp->ca_sa) ) {
		return (TRUE);
	}
	return (FALSE);
}

/*
 * arguments to link
 */
bool_t
xdr_linkargs(xdrs, argp)
	XDR *xdrs;
	struct nfslinkargs *argp;
{

	if (xdrs->x_op==XDR_DECODE)  {
	}
	if (xdr_fhandle(xdrs, &argp->la_from) &&
	    xdr_diropargs(xdrs, &argp->la_to) ) {
		return (TRUE);
	}
#ifdef NFSDEBUG
	dprint(nfsdebug, 2, "xdr_linkargs: FAILED\n");
#endif
	return (FALSE);
}

/*
 * arguments to rename
 */
bool_t
xdr_rnmargs(xdrs, argp)
	XDR *xdrs;
	struct nfsrnmargs *argp;
{
	if (xdrs->x_op==XDR_DECODE)  {
	}

	if (xdr_diropargs(xdrs, &argp->rna_from) &&
	    xdr_diropargs(xdrs, &argp->rna_to) ) {
		return (TRUE);
	}
	return (FALSE);
}

/*
 * arguments to symlink
 */
bool_t
xdr_slargs(xdrs, argp)
	XDR *xdrs;
	struct nfsslargs *argp;
{

	if (xdrs->x_op==XDR_DECODE)  {
	}

	if (xdr_diropargs(xdrs, &argp->sla_from) &&
	    xdr_string(xdrs, &argp->sla_tnm, (u_int)MAXPATHLEN) &&
	    xdr_sattr(xdrs, &argp->sla_sa) ) {
		return (TRUE);
	}
	return (FALSE);
}

/*
 * NFS_OK part of statfs operation
 */
xdr_fsok(xdrs, fsok)
	XDR *xdrs;
	struct nfsstatfsok *fsok;
{

	if (xdr_long(xdrs, (long *)&fsok->fsok_tsize) &&
	    xdr_long(xdrs, (long *)&fsok->fsok_bsize) &&
	    xdr_long(xdrs, (long *)&fsok->fsok_blocks) &&
	    xdr_long(xdrs, (long *)&fsok->fsok_bfree) &&
	    xdr_long(xdrs, (long *)&fsok->fsok_bavail) ) {
		if (xdrs->x_op==XDR_DECODE)  {
/*			printf("%s",beg_line);*/
/*			printf(*/
/*		    "STATFS OK: tsz %d bsz %d blks %d bfree %d bavail %d\n",*/
/*				fsok->fsok_tsize, fsok->fsok_bsize, */
/*				fsok->fsok_blocks, fsok->fsok_bfree,*/
/*				fsok->fsok_bavail);*/
		}
		return (TRUE);
	}
	return (FALSE);
}

struct xdr_discrim statfs_discrim[2] = {
	{ (int)NFS_OK, xdr_fsok },
	{ __dontcare__, NULL_xdrproc_t }
};

/*
 * Results of statfs operation
 */
xdr_statfs(xdrs, fs)
	XDR *xdrs;
	struct nfsstatfs *fs;
{

	if (xdr_union(xdrs, (enum_t *)&(fs->fs_status),
	      (caddr_t)&(fs->fs_fsok), statfs_discrim, xdr_void) ) {
		return (TRUE);
	}
	return (FALSE);
}
