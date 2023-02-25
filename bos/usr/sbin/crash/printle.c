static char sccsid[] = "@(#)99        1.5  src/bos/usr/sbin/crash/printle.c, cmdcrash, bos411, 9428A410j 3/12/94 10:47:42";
/*
 * COMPONENT_NAME: (CMDCRASH) 
 *
 * FUNCTIONS: call_le,get_mod
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
#include	"sys/malloc.h"
#include	"sys/syspest.h"
#include	<sys/ldr/ld_data.h>

#include	<sys/inode.h>
#include	<sys/proc.h>
#include	<sys/file.h>
#include	<errno.h>

extern	struct	uarea	x;
extern  char    *dumpfile;
extern  int	dumpflag;
int	initialized = 0;



int
getle(addr, lename, taddr, daddr)
unsigned addr, *taddr, *daddr;
char *lename;
{
	int found;
	long la_address;

	if (readmem(&la_address, (long)SYM_VALUE(K_anchor),
	sizeof(la_address)) == -1)
		return -1;
	do {
		found = get_mod(addr, &la_address);
	} while ((found == 0) && (la_address != 0));
	if (found == 1){
		static  char name[1024];
		char	*bs, next[2];
		int	i;
		long	filename_addr;
		struct	loader_entry lbuf;

		/* Read in the loader entry */
		if (readmem(&lbuf, la_address, sizeof(lbuf)) == -1)
			return -1;

		filename_addr = lbuf.le_filename;
		name[0] = next[1] = '\0';
		for (i=0; i<2; i++) {
			do {
				if (readmem(next, (long)filename_addr, 1) !=1 )
					return 0;
				strcat(name, next);
				filename_addr++;
			} while (*next != '\0');
		}
		if ((bs = (char *)strrchr(name, '/')) != NULL)
			bs++;
		else
			bs = name;
		if (lename != NULL)
			strcpy(lename, bs);
		if (taddr != NULL)
			*taddr = lbuf.le_file;
		if (daddr != NULL)
			*daddr = lbuf.le_data;
	}
	return found;
}

/*
 *	Read in the loader anchor and the loader entry address             
 *
*/
call_le(find, flag)
union {
	int addr;
	char *txt;
} find;
int flag;
{
	long	la_address;
	long	le_address;
	long	next_le;
	int	found;

	/* Read in the loader anchor address     */
		if(readmem(&la_address, (long)SYM_VALUE(K_anchor),
		     sizeof(la_address)) == -1) {
			printf( catgets(scmc_catd, MS_crsh, LE_MSG_1, "Cannot read kernel loader anchor address from address 0x%8x\n") , SYM_VALUE(K_anchor));
				return;
			}
	found = 0;


	if (flag == FIND) {
	do
			found = get_mod(find.addr,&la_address);
		while ((found == 0) && (la_address != 0) );
	if (found == 1)
		prle(la_address,&next_le);
	else
		if(found == 0)
			    printf( catgets(scmc_catd, MS_crsh, LE_MSG_2, "No loader entry found for module address 0x%8x\n") , find.addr);
	}
	else if (flag == FINDTXT) {
		found = 0;
		do {
			int rc;
			rc = get_modtxt(find.txt, &la_address);
			if (rc == 1) {
				prle(la_address, &la_address);
				found = 1;
			}
			if (rc < 0)
				break;
			    
		} while (la_address != 0);
		if (found == 0)
			printf(catgets(scmc_catd, MS_crsh, LE_MSG_2,
			    "No loader entry found for module %s\n"), find.txt);
	}
	else {
	while (la_address != 0) {
		next_le = la_address;
		la_address = 0;
		prle(next_le,&la_address);
	}
	}
}


/*
 *	Gather the loader entry together and put it into the structure lbuf
 *
*/
prle(le_addr,le_naddr)
long	le_addr, *le_naddr;
{
	struct	buf {
		struct	loader_entry lebuf;
		long	depends[15];
		};
	struct	buf lbuf;
	int	i, j;
	char	next[2], name[1024], member[1024], flags[86];
	char	*n_ptr, *m_ptr;
 	long	filename_addr;

	/* Read in the loader entry      */
		if(readmem(&lbuf, (long)le_addr,
		     sizeof(lbuf)) != sizeof(lbuf)) {
			printf( catgets(scmc_catd, MS_crsh, LE_MSG_3, "Cannot read loader entry from address 0x%8x\n") ,le_addr) ;
				return;
			}

	*le_naddr = lbuf.lebuf.le_next;

	/* Read in print file name   */
	n_ptr = name;
	m_ptr = member;
	*n_ptr = NULL;
	*m_ptr = NULL;
	next[1] = NULL;

	filename_addr = lbuf.lebuf.le_filename;
	if(readmem(next, (long)filename_addr,1) != 1) {
		printf( catgets(scmc_catd, MS_crsh, LE_MSG_4, "Cannot read file name from address 0x%8x\n") ,lbuf.lebuf.le_filename);
		return;
		}


	if (*next != NULL) 			/* check for member name*/
		do {
		strcat(m_ptr,next);
		filename_addr++;
		if(readmem(next, (long)filename_addr,1) != 1) {
		printf( catgets(scmc_catd, MS_crsh, LE_MSG_4, "Cannot read file name from address 0x%8x\n") ,lbuf.lebuf.le_filename);
		return;
		}

		}
		while (*next != NULL);
	filename_addr++;
	if(readmem(next, (long)filename_addr,1) != 1) {
		printf( catgets(scmc_catd, MS_crsh, LE_MSG_4, "Cannot read file name from address 0x%8x\n") ,lbuf.lebuf.le_filename);
		return;
		}


	if (*next != NULL) 			/* check for file name*/
		do {
		strcat(n_ptr,next);
		filename_addr++;
		if(readmem(next, (long)filename_addr,1) != 1) {
		printf( catgets(scmc_catd, MS_crsh, LE_MSG_4, "Cannot read file name from address 0x%8x\n") ,lbuf.lebuf.le_filename);
		return;
		}

		}
		while (*next != NULL);

	/* parse flag field */
	flags[0] = NULL;
	switch (lbuf.lebuf.le_flags & 0xf ) {
		case 0x1: strcat(flags,"UNLD ");
		break;
		case 0x2: strcat(flags,"TEXT ");
		break;
		case 0x3: strcat(flags,"UNLD TEXT ");
		break;
		case 0x4: strcat(flags,"KERN ");
		break;
		case 0x5: strcat(flags,"UNLD KERN ");
		break;
		case 0x6: strcat(flags,"TEXT KERN ");
		break;
		case 0x7: strcat(flags,"UNLD TEXT KERN ");
		break;
		case 0x8: strcat(flags,"SYSC ");
		break;
		case 0x9: strcat(flags,"UNLD SYSC ");
		break;
		case 0xA: strcat(flags,"TEXT SYSC ");
		break;
		case 0xB: strcat(flags,"UNLD TEXT SYSC ");
		break;
		case 0xC: strcat(flags,"KERN SYSC ");
		break;
		case 0xD: strcat(flags,"UNLD KERN SYSC ");
		break;
		case 0xE: strcat(flags,"TEXT KERN SYSC ");
		break;
		case 0xF: strcat(flags,"UNLD TEXT KERN SYSC ");
		break;
	}
	switch (lbuf.lebuf.le_flags & 0xf0 ) {
		case 0x10: strcat(flags,"EXTN ");
		break;
		case 0x20: strcat(flags,"DATX ");
		break;
		case 0x30: strcat(flags,"EXTN DATX ");
		break;
		case 0x40: strcat(flags,"DATA ");
		break;
		case 0x50: strcat(flags,"EXTN DATA ");
		break;
		case 0x60: strcat(flags,"DATX DATA ");
		break;
		case 0x70: strcat(flags,"EXTN DATX DATA ");
		break;
		case 0x80: strcat(flags,"LIBR ");
		break;
		case 0x90: strcat(flags,"EXTN LIBR ");
		break;
		case 0xA0: strcat(flags,"DATX LIBR ");
		break;
		case 0xB0: strcat(flags,"EXTN DATX LIBR ");
		break;
		case 0xC0: strcat(flags,"DATA LIBR ");
		break;
		case 0xD0: strcat(flags,"EXTN DATA LIBR ");
		break;
		case 0xE0: strcat(flags,"DATX DATA LIBR ");
		break;
		case 0xF0: strcat(flags,"EXTN DATX DATA LIBR ");
		break;
	}
	switch (lbuf.lebuf.le_flags & 0xf00 ) {
		case 0x100: strcat(flags,"LIEX ");
		break;
		case 0x200: strcat(flags,"DAEX ");
		break;
		case 0x300: strcat(flags,"LIEX DAEX ");
		break;
		case 0x400: strcat(flags,"ASIS ");
		break;
		case 0x500: strcat(flags,"LIEX ASIS ");
		break;
		case 0x600: strcat(flags,"DAEX ASIS ");
		break;
		case 0x700: strcat(flags,"LIEX DAEX ASIS ");
		break;
		case 0x800: strcat(flags,"TMAP ");
		break;
		case 0x900: strcat(flags,"LIEX TMAP ");
		break;
		case 0xA00: strcat(flags,"DAEX TMAP ");
		break;
		case 0xB00: strcat(flags,"LIEX DAEX TMAP ");
		break;
		case 0xC00: strcat(flags,"ASIS TMAP ");
		break;
		case 0xD00: strcat(flags,"LIEX ASIS TMAP ");
		break;
		case 0xE00: strcat(flags,"DAEX ASIS TMAP ");
		break;
		case 0xF00: strcat(flags,"LIEX DAEX ASIS TMAP ");
		break;
	}
	switch (lbuf.lebuf.le_flags & 0xf000 ) {
		case 0x9000:
		case 0x1000: strcat(flags,"DMAP ");
		break;
		case 0xA000:
		case 0x2000: strcat(flags,"NOAD ");
		break;
		case 0xB000:
		case 0x3000: strcat(flags,"DMAP NOAD ");
		break;
		case 0xC000:
		case 0x4000: strcat(flags,"EXLE ");
		break;
		case 0xD000:
		case 0x5000: strcat(flags,"DMAP EXLE ");
		break;
		case 0xE000:
		case 0x6000: strcat(flags,"NOAD EXLE ");
		break;
		case 0xF000:
		case 0x7000: strcat(flags,"DMAP NOAD EXLE ");
		break;
	}
	switch (lbuf.lebuf.le_flags & 0xf0000000 ) {
		case 0x40000000:
		case 0x50000000:
		case 0x60000000:
		case 0x70000000: strcat(flags,"EXTN ");
		break;
		case 0x80000000:
		case 0x90000000:
		case 0xA0000000:
		case 0xB0000000:
		case 0xC0000000:
		case 0xD0000000:
		case 0xE0000000:
		case 0xF0000000: strcat(flags,"LDNG ");
		break;
	}

	/* Print the loader entry */
	printf("LoadList entry at 0x%08x\n",le_addr);
	printf("  Module start:0x%08x  Module filesize:0x%08x  *end:0x%08x\n",lbuf.lebuf.le_file,lbuf.lebuf.le_filesize,lbuf.lebuf.le_file+lbuf.lebuf.le_filesize);
	printf("  *data:0x%08x  data length:0x%08x\n",lbuf.lebuf.le_data,lbuf.lebuf.le_datasize);
	printf("  Use-count:0x%04x  load_count:0x%04x  *file:0x%08x\n",lbuf.lebuf.le_usecount,lbuf.lebuf.le_loadcount,lbuf.lebuf.le_fp);
	printf("  flags:0x%08x %s\n",lbuf.lebuf.le_flags,flags);
	printf("  *exp:0x%08x  *lex:0x%08x  *deferred:0x%08x\n",lbuf.lebuf.le_exports,lbuf.lebuf.le_lex,lbuf.lebuf.le_defered);
	printf("  Name: %s  %s\n",name,member);
	printf("  ndepend:0x%04x  maxdepend:0x%04x\n",lbuf.lebuf.le_ndepend,lbuf.lebuf.le_maxdepend);
	if (lbuf.lebuf.le_ndepend > 0)
	   for (i=0;i<lbuf.lebuf.le_ndepend;i++) {
		printf("  *depend[%02d]:0x%08x\n",i,lbuf.depends[i-1]);
 	   }
printf("  le_next:  %08x\n\n",*le_naddr);

	return;
}

/*
 *	Given a string, find the corresponding loader entry
*/
get_modtxt(le_txt, le_naddr)
char 	*le_txt;
long	*le_naddr;
{
	char	next[2], name[1024];
	int	i;
 	long	filename_addr;
	struct	loader_entry lbuf;

	/* Read in the loader entry */
	if (readmem(&lbuf, *le_naddr, sizeof(lbuf)) == -1) {
		printf(catgets(scmc_catd, MS_crsh, LE_MSG_3,
		    "Cannot read loader entry at 0x%8x\n"), *le_naddr);
		return -1;
	}
	filename_addr = lbuf.le_filename;
	name[0] = next[1] = '\0';
	for (i=0; i<2; i++) {
		do {
			if (readmem(next, (long)filename_addr, 1) != 1) {
				*le_naddr = lbuf.le_next;
				return 0;
			}
			strcat(name, next);
			filename_addr++;
		} while (*next != '\0');
		strcat(name, "-");
	}

	for (i=0; i<strlen(name); i++)
		if (strncmp(name+i, le_txt, strlen(le_txt)) == 0)
			return 1;
	*le_naddr = lbuf.le_next;
	return 0;
}


/*
 *	Given a module address, find the corresponding loader entry
*/
get_mod(le_addr,le_naddr)
long	le_addr, *le_naddr;
{
	struct	buf {
		struct	loader_entry lebuf;
		};
	struct	buf lbuf;
	long	module_end;

	/* Read in the loader entry      */
		if(readmem(&lbuf, (long)*le_naddr,
		     sizeof(lbuf)) == -1) {
			printf( catgets(scmc_catd, MS_crsh, LE_MSG_3, "Cannot read loader entry from address 0x%8x\n") ,le_addr) ;
				return(-1);
			}

	module_end = lbuf.lebuf.le_file+lbuf.lebuf.le_filesize;
	if ((lbuf.lebuf.le_file <= le_addr) && (le_addr <= module_end))
		return(1);
	else
		{
		*le_naddr = lbuf.lebuf.le_next;
		return(0);
		}
}



