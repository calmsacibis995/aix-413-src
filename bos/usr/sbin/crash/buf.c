static char sccsid[] = "@(#)32	1.14  src/bos/usr/sbin/crash/buf.c, cmdcrash, bos411, 9435A411a 8/25/94 12:59:07";
/*
 * COMPONENT_NAME: (CMDCRASH) 
 *
 * FUNCTIONS: prbufhdr,prbuffer,itoa,putch
 *
 * ORIGINS: 27,3
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (Copyright statements and/or associated legends of other
 * companies whose code appears in any part of this module must
 * be copied here.)
 */


/*              include file for message texts          */
#include "crash_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */
#include	"crash.h"
#include	<sys/dir.h>
#include	<sys/ino.h>
#include	<sys/inode.h>
#include	<sys/buf.h>
#include	<IN/PCdefs.h>

prbufhdr(i,flag)
register  int  i;		/* slot number */
register  int  flag;		/* flag - short or long display */
{
	register  int  b_flags;
	struct	buf	bbuf;
	long	bptr;

	if(i == -1)
		return;
	if(i >= v.v_bufhw ) {
		printf( catgets(scmc_catd, MS_crsh, BUF_MSG_1, "Buffer %3d above maximum limit.Specify a value that is less than %d.\n") , i,v.v_bufhw);
		return;
	}

	if(readmem(&bptr, (long)SYM_VALUE(Buf),
		sizeof(bptr), CURPROC) != sizeof(bptr)) {
		printf( catgets(scmc_catd, MS_crsh, BUF_MSG_2, "0452-002: Error: Cannot retrieve address of buffer table.\n") );
		return;
	}

	if(readmem(&bbuf, bptr + (i * sizeof(bbuf)),
		sizeof(bbuf), CURPROC) != sizeof(bbuf)) {
		printf( catgets(scmc_catd, MS_crsh, BUF_MSG_3, "0452-003: Error: Cannot read buffer header %u.\n") ,i);
		return;
	}

	if (flag == SHORT_BUF) {
		printf("%3d %04x %04x %7ld  ", i,
			major(bbuf.b_dev), minor(bbuf.b_dev),
			bbuf.b_blkno, bbuf.b_un.b_addr);
	}
	else {

		printf("    b_forw:    0x%08x,  ", bbuf.b_forw);
		printf("b_back:    0x%08x,  ", bbuf.b_back);
		printf("b_vp:      0x%08x\n", bbuf.b_vp);
		printf("    av_forw:   0x%08x,  ", bbuf.av_forw);
		printf("av_back:   0x%08x,  ", bbuf.av_back);
		printf("b_iodone:  0x%08x\n", bbuf.b_iodone);
		printf("    b_dev:     0x%08x,  ", bbuf.b_dev);
		printf("b_blkno:   %10d,  ", bbuf.b_blkno);
		printf("b_addr:    0x%08x\n", bbuf.b_un.b_addr);
		printf("    b_bcount:  %10d,  ", bbuf.b_bcount);
		printf("b_error:   %10d,  ", bbuf.b_error);
		printf("b_resid:   %10d\n", bbuf.b_resid);
		printf("    b_work:    0x%08x,  ", bbuf.b_work);
		printf("b_options: 0x%08x,  ", bbuf.b_options);
		printf("b_event:   0x%08x\n", bbuf.b_event);
		printf("    b_start.tv_sec:    %10d,  ", bbuf.b_start.tv_sec);
		printf("b_start.tv_nsec:     %10d\n", bbuf.b_start.tv_nsec);
		printf("    b_xmemd.aspace_id: 0x%08x,  ", 
			bbuf.b_xmemd.aspace_id);
		printf("b_xmemd.subspace_id: 0x%08x\n", 
			bbuf.b_xmemd.subspace_id);
		printf("    b_flags:");
	}
	b_flags = bbuf.b_flags;
		if ( b_flags & B_WRITE ) 	printf(  catgets(scmc_catd, MS_crsh, BUF_MSG_4, " write" ) ) ;
		if ( b_flags & B_READ ) 	printf(  catgets(scmc_catd, MS_crsh, BUF_MSG_5, " read" ) ) ;
		if ( b_flags & B_DONE ) 	printf(  catgets(scmc_catd, MS_crsh, BUF_MSG_6, " done" ) ) ;
		if ( b_flags & B_ERROR ) 	printf(  catgets(scmc_catd, MS_crsh, BUF_MSG_7, " error" ) ) ;
		if ( b_flags & B_BUSY ) 	printf(  catgets(scmc_catd, MS_crsh, BUF_MSG_8, " busy" ) ) ;
		if ( b_flags & B_INFLIGHT ) 	printf(  catgets(scmc_catd, MS_crsh, BUF_MSG_9, " inflight" ) ) ;
		if ( b_flags & B_AGE ) 		printf(  catgets(scmc_catd, MS_crsh, BUF_MSG_10, " age" ) ) ;
		if ( b_flags & B_ASYNC ) 	printf(  catgets(scmc_catd, MS_crsh, BUF_MSG_11, " async" ) ) ;
		if ( b_flags & B_DELWRI ) 	printf(  catgets(scmc_catd, MS_crsh, BUF_MSG_12, " delwri" ) ) ;
		if ( b_flags & B_STALE ) 	printf(  catgets(scmc_catd, MS_crsh, BUF_MSG_13, " stale" ) );
	printf("\n");
}


prbuffer(c, sw)
register  int  c;
int  sw;
{
	char	buffer[BSIZE];
	struct	buf	bufhdr;
	long	bptr;
	register  int  *ip, i;
	struct	direct	*dp;
	struct	dinode	*dip;
	char	ch;
	int	bad;
	int	j;
	char	*cp;
	char	*itoa();
	int	file;
	int	*address;
	char	name[20];

	if(c ==  -1)
		return;
	printf( catgets(scmc_catd, MS_crsh, BUF_MSG_14, "\nBUFFER FOR BUF_HDR %d:") , c);
	if(c >= v.v_bufhw) {
		printf( catgets(scmc_catd, MS_crsh, BUF_MSG_15, "Buffer %d above maximum limit.Specify a value less than %d.\n") ,c,v.v_bufhw);
		return;
	}
	if(readmem(&bptr, (long)SYM_VALUE(Buf),
		sizeof(bptr), CURPROC) != sizeof(bptr)) {
		printf( catgets(scmc_catd, MS_crsh, BUF_MSG_16, "0452-016: Cannot read pointer to buffer table from address 0x%8x\n") ,
			SYM_VALUE(Buf));
		return;
	}
	if(readmem(&bufhdr, bptr + (c * sizeof(bufhdr)),
		sizeof(bufhdr), CURPROC) != sizeof(bufhdr)) {
		printf( catgets(scmc_catd, MS_crsh, BUF_MSG_17, "0452-017: Cannot read buffer header %d from address 0x%8x\n") ,c,
			bptr + c*sizeof(bufhdr));
		return;
	}
	if(readmem(buffer, (long)bufhdr.b_un.b_addr,
		sizeof(buffer), CURPROC) != sizeof(buffer)) {
		printf( catgets(scmc_catd, MS_crsh, BUF_MSG_18, "0452-018: Cannot read buffer %d from address 0x%8x\n") ,c,
			bufhdr.b_un.b_addr);
		return;
	}
	switch(sw) {

	default:
		printf( catgets(scmc_catd, MS_crsh, BUF_MSG_19, "0452-019: Wrong display mode specified.\n") );
		break;

	case DECIMAL:
	case HEX:
	case OCTAL:
		for(i=0, address = (int *)0, ip = (int *)buffer;
			address < (int *)(sizeof buffer);
			i++, address++, ip++) {
			if(((int)address % 020) == 0)
				printf("\n%5.5o:\t", address);
			switch(sw) {
				case DECIMAL:
					printf("%8.8u ", *ip);
					break;
				case HEX:
					printf("%8.8x ", *ip);
					break;
				case OCTAL:
					printf("%8.8o ", *ip);
			}
		}
		printf("\n");
		break;


	case WRITE:
		strcpy(name, "buf.");
		strcat(name, itoa(c));
		if((file = creat(name, 0666)) < 0) {
			printf( catgets(scmc_catd, MS_crsh, BUF_MSG_21, "0452-020: Cannot create file %s.\n") ,name);
			break;
		}
		if(write(file, buffer, sizeof buffer) != sizeof buffer)
			printf( catgets(scmc_catd, MS_crsh, BUF_MSG_22, "0452-021: Cannot write to  file %s.\n") ,name);
		else
			printf( catgets(scmc_catd, MS_crsh, BUF_MSG_23, "file:  %s\n") , name);
		close(file);
		break;

	case CHAR:
	case BYTE:
		for(i=0, cp = buffer; cp != &buffer[(sizeof buffer)];
		    i++, cp++) {
			if(i % (sw == CHAR ? 16 : 8) == 0)
				printf("\n%5.5o:\t", i);
			if(sw == CHAR) putch(*cp);
			else printf(" %4.4o", *cp & 0377);
		}
		printf("\n");
		break;

	case INODE:
		for(i=1, dip = (struct dinode *) buffer; 
		    dip <= (struct dinode *)(&buffer[(sizeof buffer)]) - 1;
		    i++, dip++) {
			printf("\ni#: %ld  md: ", (bufhdr.b_blkno - 2) *
				FsINOPB(bufhdr.b_dev) + i);
			switch(dip->di_mode & IFMT) {
			case IFCHR: ch = 'c'; break;
			case IFBLK: ch = 'b'; break;
			case IFDIR: ch = 'd'; break;
			case IFREG: ch = 'f'; break;
			case IFIFO: ch = 'p'; break;
			default:    ch = '-'; break;
			}
			putc(ch, stdout);
			printf("%s%s%s%3o",
				dip->di_mode & ISUID ? "u" : "-",
				dip->di_mode & ISGID ? "g" : "-",
				dip->di_mode & ISVTX ? "t" : "-",
				dip->di_mode & 0777);
			printf("  ln: %u  uid: %u  gid: %u  sz: %ld",
				dip->di_nlink, dip->di_uid,
				dip->di_gid, dip->di_size);
			if((dip->di_mode & IFMT) == IFCHR ||
				(dip->di_mode & IFMT) == IFBLK ||
				(dip->di_mode & IFMT) == IFIFO)
				printf("\nmaj: %d  min: %1.1o\n",
					major(dip->di_rdev),
					minor(dip->di_rdev));
			else
				for(j = 0; j < NDADDR; j++) {
					if(j % 7 == 0)
						putc('\n', stdout);
					printf("a%d: %ld  ", j, 
						dip->di_rdaddr[j]);
				}
                        {
                           struct tm *ptm;
                           char *pbuf, buf[40];
                           pbuf = buf;

                           ptm = localtime(&dip->di_atime);
                           strftime(pbuf, 39, "%c", ptm);
			   printf("\nat: %s\n", pbuf);
                           ptm = localtime(&dip->di_mtime);
                           strftime(pbuf, 39, "%c", ptm);
			   printf("mt: %s\n", pbuf);
                           ptm = localtime(&dip->di_ctime);
                           strftime(pbuf, 39, "%c", ptm);
			   printf("ct: %s\n", pbuf);
                        }
		}
		printf("\n");
		break;

	case DIRECT:
		printf("\n");
		for(i=0, dp =(struct direct *)  buffer; 
		    dp < (struct direct *) &buffer[(sizeof buffer)]; i++,
		    dp = (struct direct *)((char *)dp + dp->d_reclen)) {
			bad = 0;
			/*
			 * Don't attempt to print in directory format if
			 * it does not look like a valid directory entry.
			 */
			if ((dp->d_reclen > 512)
			    || (dp->d_namlen > MAXNAMLEN)
			    || (dp->d_namlen > (dp->d_reclen - sizeof(dp->d_ino)
						- sizeof(dp->d_reclen)
						- sizeof(dp->d_namlen)))) {
				printf( catgets(scmc_catd, MS_crsh, BUF_MSG_24, "0452-023: Buffer %2d does not contain directory entries\n") ,i);
				return;
			}
			/*
			 * Don't print a partial directory entry
			 */
			if (((char *)dp + dp->d_reclen) >
			    &buffer[(sizeof buffer)]) {
				printf( catgets(scmc_catd, MS_crsh, BUF_MSG_25, "0452-024: Buffer %2d does not contain directory entries\n") ,i);
				return;
			}
			/*
			 * Don't print out null directory entrys
			 */
			if (dp->d_reclen == 0)
				return;
			/*
			 * Dont't print out a deleted directory entry
			 */
			if (!dp->d_ino) {
				printf("%2d: %5u\n", i, dp->d_ino);
				continue;
			}
			/*
			 * See if all characters in the filename are printable
			 */
			for(cp = dp->d_name; cp != &dp->d_name[dp->d_namlen];
			     cp++)
				if((*cp < 040 || *cp > 0176) && *cp != '\0')
					bad++;
			printf("%2d: %5u  ", i, dp->d_ino);
			/*
			 * if unprintable then show string with unprintables
			 * escaped
			 */
			if(bad) {
				for(cp = dp->d_name; 
					cp != &dp->d_name[dp->d_namlen];
					cp++)
					putch(*cp);
			/*
			 * Else print the string as is
			 */
			} else
				printf("%s", dp->d_name);
			putc('\n', stdout);
		}
		break;

	}
}

char	*
itoa(n)
	register  int  n;
{
	register  int  i;
	static    char d[10];

	d[9] = '\0';
	for(i = 8; i >= 0; --i) {
		d[i] = n % 10 + '0';
		n /= 10;
		if(n == 0)
			break;
	}
	return(&d[i]);
}

putch(c)
	register  char  c;
{
	c &= 0377;
	if(c < 040 || c > 0176) {
		putc('\\', stdout);
		switch (c) {
		case '\0': c = '0'; break;
		case '\t': c = 't'; break;
		case '\n': c = 'n'; break;
		case '\r': c = 'r'; break;
		case '\b': c = 'b'; break;
		default:   c = '?'; break;
		}
	} else
		putc(' ', stdout);
	putc(c, stdout);
	putc(' ', stdout);
}
