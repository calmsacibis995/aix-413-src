static char sccsid[] = "@(#)89	1.5  src/tcpip/usr/sbin/ipreport/nfs_dump.c, tcpras, tcpip411, GOLD410 9/25/93 11:39:36";
/*
 * COMPONENT_NAME: TCPIP nfs_dump.c
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * This file contains the dump routines to print nfs rpc packets for the
 * ipreport command.
 * 
 * Created to Sat Aug 10 1991 from nfs_xdr.c but basically all new.
 * 
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

char *nfs_beg="NFS: ";

/*
 * print File access handle
 * The fhandle struct is treated a opaque data on the wire
 */
prt_fhandle(fh)
	fhandle_t *fh;
{
	reset_beg_line(nfs_beg);

	printf("%s",beg_line);
	printf( "Fhandle: ");
	hex_dump_line(fh,NFS_FHSIZE/2);
	printf( "\n");
	printf("%s",beg_line);
	printf( "         ");
	hex_dump_line((char *)fh+(NFS_FHSIZE/2),NFS_FHSIZE/2);
	printf( "\n");
}

prt_void() {
	printf("%s",beg_line);
	printf("XDR_VOID\n");
}

char * prt_type_str(tp)
int tp;
{
	/* File types */

	switch (tp )
	{
        case NFNON: return("NFNON");
		    break;
	case NFREG: return("NFREG"); 		/* regular file */   
		    break;
	case NFDIR: return("NFDIR"); 		/* directory */    
		    break;
	case NFBLK: return("NFBLK"); 		/* block special */   
		    break;
	case NFCHR: return("NFCHR"); 		/* character special */   
		    break;
	case NFLNK: return("NFLNK"); 		/* symbolic link */   
		    break;
	default:    return("Unknown"); 		/* error */   
		    break;
	}
};


prt_fattr( struct nfsfattr *na)
{

	reset_beg_line(nfs_beg);
	printf("%s",beg_line);
	printf("File Attribute:\n");
	printf("%s",beg_line);
	printf("\tType=%d(%s) Mode=%o Nlink=%d Uid=%u Gid=%u \n%s\tSize=%d Blocksize=%d Rdev=%d Blocks=%d Fsid=%d Nodeid=%d\n",
	na->na_type,prt_type_str(na->na_type),
	na->na_mode,
	na->na_nlink,
	na->na_uid,
	na->na_gid,
	beg_line,
	na->na_size,
	na->na_blocksize,
	na->na_rdev,
	na->na_blocks,
	na->na_fsid,
	na->na_nodeid);

	printf("%s",beg_line);
	printf("\tAccess Time: ");
	prt_timeval(&na->na_atime);
	printf("\n");

	printf("%s",beg_line);
	printf("\tModify Time: ");
	prt_timeval(&na->na_mtime);
	printf("\n");

	printf("%s",beg_line);
	printf("\tCreate Time: ");
	prt_timeval(&na->na_ctime);
	printf("\n");
}

/* returns 1 if NFS_OK else 0 */
prt_nfsstat(status)
int status;
{
	reset_beg_line(nfs_beg);
	printf("%s",beg_line);
	printf("NFS Stat: (%d) ",status);
	switch (geterrno(status)) {
	case NFSERR_PERM 	: printf("Error- Not owner \n");break;
	case NFSERR_NOENT 	: printf("Error- No such file or directory \n");
					break;
	case NFSERR_IO 		: printf("Error- IO error \n");break;
	case NFSERR_NXIO 	: printf("Error- No such device or address \n");
					break;
	case NFSERR_ACCES 	: printf("Error- Permission denied \n");break;
	case NFSERR_EXIST 	: printf("Error- File exists or Directory not empty\n");break;
	case NFSERR_NODEV 	: printf("Error- No such device \n");break;
	case NFSERR_NOTDIR 	: printf("Error- Not a directory \n");break;
	case NFSERR_ISDIR 	: printf("Error- Is a directory \n");break;
	case NFSERR_FBIG 	: printf("Error- File too large \n");break;
	case NFSERR_NOSPC 	: printf("Error- No space left on device \n");
					break;
	case NFSERR_ROFS 	: printf("Error- Read-only file system \n");
					break;
	case NFSERR_NAMETOOLONG	: printf("Error- File name too long \n");break;
	case NFSERR_DQUOT 	: printf("Error- Disc quota exceeded \n");break;
	case NFSERR_STALE 	: printf("Error- Stale NFS file handle \n");
					break;
	case NFSERR_WFLUSH 	: printf("Error- write cache flushed \n");break;
	case NFS_OK 		: printf("NFSOK \n");
				  return(1);
	default 		: printf("Error- UNKNOWN NFS STATUS\n");break;
	}
	return(0);
}

prt_star_nfsstat(status)
int *status;
{
	return(prt_nfsstat(*status));
}

prt_attrstat (ns)
struct nfsattrstat *ns;
{
	if (prt_nfsstat(ns->ns_status))
		prt_fattr( &ns->ns_attr);
}

/*
 * print NFS_OK part of directory operation result
 */
prt_drok(drok)
	struct nfsdrok *drok;
{
	reset_beg_line(nfs_beg);

	prt_fhandle(&drok->drok_fhandle) ;
	prt_fattr(&drok->drok_attr) ;
}

/*
 * dump directory operation 
 */
bool_t
prt_diropres ( dr)
	struct nfsdiropres *dr;
{
	reset_beg_line(nfs_beg);

	if (prt_nfsstat(dr->dr_status))
		prt_drok(&dr->dr_drok);
}


getrddirres_prthdr()
{
		printf("%s",beg_line);
		printf( 
	"Getrddirres: Entry Inode Name(Name length) Reclength Offset\n");
/*		printf( */
/*	"-----------------------------------------------------------\n");*/
}

#define	nextdp(dp)	((struct dirent *)((int)(dp) + (dp)->d_reclen))

prt_getrddirres (rd)
	struct nfsrddirres *rd;
{
	char *z;
	struct dirent *dp;
	int size;
	bool_t valid;
        off_t offset = (off_t)-1;
	int eof,hdrprted=0;

	reset_beg_line(nfs_beg);

	prt_nfsstat(rd->rd_status);

	if (rd->rd_status != NFS_OK) {
		return (TRUE);
	}

	
	printf("%s",beg_line);
	printf( "Getrddirres: offset=%d size=%d eof=%s\n",
			rd->rd_offset, size=rd->rd_size,
			(eof=rd->rd_eof) ? "TRUE" : "FALSE" );
	dp=rd->rd_entries;

	getrddirres_prthdr();
	if (dp) {
		for (;size>0;) 
		{ 
			printf("%s",beg_line);
			printf( "Getrddirres: entry %u %s(%d) %d %d\n", 
				dp->d_ino, dp->d_name, dp->d_namlen,
				dp->d_reclen, dp->d_offset); 
			size-=dp->d_reclen;
			dp = nextdp(dp);
		}
	}
}

/*
 * print Result of reading symbolic link
 */
prt_rdlnres(rl)
	struct nfsrdlnres *rl;
{
	char fname[NFS_MAXPATHLEN+1];
	reset_beg_line(nfs_beg);


	prt_nfsstat( rl->rl_status);

	if (rl->rl_status != NFS_OK) return;

	strncpy(fname,rl->rl_data,rl->rl_count);
	printf("%s",beg_line);
	printf("Read Symlink Result: linkname %s\n", fname);

}

/*
 * print Status OK portion of remote read reply
 */
prt_rrok(rrok)
	struct nfsrrok *rrok;
{
	void freeit();
	int count;

	reset_beg_line(nfs_beg);

	prt_fattr(&rrok->rrok_attr);
	printf("%s",beg_line);
	printf("Remote Read OK:\tcount=%d \n", rrok->rrok_count, 
						rrok->rrok_data);
	
	
/* IPREPORT SPECIFIC KLUDGE KODE (see similar comments in nfs_xdr.c.) 
	We stuck the real size into where the vp should be on fragmented
	Packets..  Blech! 
*/
	if (! (count=(int)rrok->rrok_vp))
		count= rrok->rrok_count;

	hex_dump(rrok->rrok_data, count);
}

/*
 * print Reply from remote read
 */
prt_rdresult(rr)
	struct nfsrdresult *rr;
{
	reset_beg_line(nfs_beg);

	prt_nfsstat(rr->rr_status);

	if (rr->rr_status != NFS_OK) return;

	prt_rrok(&rr->rr_ok);
}


/*
 * print NFS_OK part of statfs operation
 */
prt_fsok(fsok)
	struct nfsstatfsok *fsok;
{
	reset_beg_line(nfs_beg);

	printf("%s",beg_line);
	printf( "StatFS OK: tsize=%d bsize=%d blocks=%d bfree=%d bavail=%d\n",
			fsok->fsok_tsize, fsok->fsok_bsize, 
			fsok->fsok_blocks, fsok->fsok_bfree,
			fsok->fsok_bavail);
}

/*
 * print Results of statfs operation
 */
prt_statfs(fs)
	struct nfsstatfs *fs;
{
	reset_beg_line(nfs_beg);

	prt_nfsstat(fs->fs_status);
	if (fs->fs_status != NFS_OK) return;
	prt_fsok(&fs->fs_fsok);
}


/*  rpc Call nfs NULLPROC dump*/
c_null_dump(xdrs)
XDR	*xdrs;
{
	printf("%s",beg_line);
	printf("NULL PROC\n");
}

prt_time(time)
int time;
{
	char *c;

	c=(ctime(&time));
	c[strlen(c)-1]=(char)0;
	printf(c);
}

prt_timeval(time)
struct timeval *time;
{
	if (time) {
		printf("Sec=%d Usec=%d (",time->tv_sec, time->tv_usec);
		prt_time(time->tv_sec);
		printf(")");
	}
}


/*
 * print File attributes which can be set
 */
bool_t
prt_sattr( sa)
struct nfssattr *sa;
{
	reset_beg_line(nfs_beg);

	printf("%s",beg_line);
	printf( "Set Attributes:\tmode %o uid %d gid %d size %d \n",
			    sa->sa_mode, sa->sa_uid, sa->sa_gid, sa->sa_size);

	printf("%s",beg_line);
	printf("\tAccess Time: ");
	prt_timeval(&sa->sa_atime);
	printf("\n");

	printf("%s",beg_line);
	printf("\tModify Time: ");
	prt_timeval(&sa->sa_mtime);
	printf("\n");
	return (TRUE);
}

/* rpc Call nfs SETATTR dump*/
c_setattr_dump(argp)
struct nfssaargs *argp;
{
	reset_beg_line(nfs_beg);

	printf("%s",beg_line);
	printf("Set Attr Args:\n");

	prt_fhandle(&argp->saa_fh);
	prt_sattr( &argp->saa_sa) ;
	return(TRUE);
}

prt_nfsdiropargs(da)
struct nfsdiropargs *da;
{

	prt_fhandle(&da->da_fhandle);
	printf("%s",beg_line);
	printf("File Name=\'%s\'\n",da->da_name);

	return(TRUE);
}

/* rpc Call nfs LOOKUP DUMP*/
c_lookup_dump(da)
struct nfsdiropargs *da;
{

	prt_nfsdiropargs(da);

	return(TRUE);
}

/* dump rpc nfs readlink call arguments */
c_readlink_dump(fh)
fhandle_t *fh;
{
	prt_fhandle(fh);
	return(TRUE);
}

/* dump rpc nfs read call arguments */
c_read_dump(ra)
struct nfsreadargs *ra;
{
	printf("%s",beg_line);
	printf("Read Args: Offset=%d Count=%d Totalcount=%d\n",
		ra->ra_offset,
		ra->ra_count,
		ra->ra_totcount);
	prt_fhandle(&ra->ra_fhandle);
	
	return(TRUE);
}

/* rpc Call nfs WRITE DUMP arguments */
c_write_dump(wa)
struct nfswriteargs *wa;
{
	int count;

	prt_fhandle(&wa->wa_fhandle);
	printf("%s",beg_line);
	printf("Write Args: Begin_Offset=%d Offset=%d Total_Count=%d Count=%d\n",
			wa->wa_begoff, wa->wa_offset, wa->wa_totcount, 
			wa->wa_count);
	
/* IPREPORT SPECIFIC KLUDGE KODE (see similar comments in nfs_xdr.c.) 
	We stuck the real size into where the mbuf should be on fragmented
	Packets..  Blech! 
*/
	if (! (count=(int)wa->wa_mbuf))
		count= wa->wa_count;

	hex_dump(wa->wa_data,count);
	return(TRUE);
}

/* rpc Call nfs CREATE DUMP arguments */
c_create_dump(argp)
struct nfscreatargs *argp;
{

	prt_nfsdiropargs(&argp->ca_da);
	prt_sattr( &argp->ca_sa);

	return(TRUE);
}

/* rpc Call nfs REMOVE DUMP arguments */
c_remove_dump(da)
struct nfsdiropargs *da;
{
	prt_nfsdiropargs(da);

	return(TRUE);
}

/* rpc Call nfs RENAME DUMP arguments */
c_rename_dump(argp)
struct nfsrnmargs *argp;
{

	printf("%s",beg_line);
	printf("From:\n");
	prt_nfsdiropargs(&argp->rna_from);
	printf("%s",beg_line);
	printf("To:\n");
	prt_nfsdiropargs(&argp->rna_to);

        return(TRUE);

}

/* rpc Call nfs LINK DUMP arguments */
c_link_dump(argp)
struct nfslinkargs *argp;
{

	printf("%s",beg_line);
	printf("From:\n");
	prt_fhandle(&argp->la_from);

	printf("%s",beg_line);
	printf("To:\n");
	prt_nfsdiropargs(&argp->la_to);
	
        return(TRUE);
}

/* rpc Call nfs SYMbolic LINK DUMP arguments */
c_symlink_dump(argp)
struct nfsslargs *argp;
{

	printf("%s",beg_line);
	printf("From:\n");
	prt_nfsdiropargs(&argp->sla_from);

	printf("%s",beg_line);
	printf("New Name: %s\n",argp->sla_tnm);

	prt_sattr( &argp->sla_sa);
	
        return(TRUE);
}

/* rpc Call nfs MaKe DIRectory DUMP arguments */
c_mkdir_dump(argp)
struct nfscreatargs *argp;
{
	prt_nfsdiropargs(&argp->ca_da);
	prt_sattr( &argp->ca_sa);

	return(TRUE);
}

/* rpc Call nfs ReMove DIRectory DUMP arguments */
c_rmdir_dump(da)
struct nfsdiropargs *da;
{

	prt_nfsdiropargs(da);
	return(TRUE);
}



/* rpc Call nfs READDIR dump*/
c_readdir_dump(rda)
struct nfsrddirargs *rda;
{
	reset_beg_line(nfs_beg);

	printf("%s",beg_line);
	
	printf("Read Dir:\toffset=%d count=%d\n",rda->rda_offset, 
						 rda->rda_count);
	prt_fhandle(&rda->rda_fh);

	return(TRUE);
}

