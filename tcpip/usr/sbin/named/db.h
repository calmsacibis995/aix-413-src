/* @(#)88	1.4  src/tcpip/usr/sbin/named/db.h, tcpnaming, tcpip411, GOLD410 6/3/93 09:51:03 */
/* 
 * COMPONENT_NAME: TCPIP db.h
 * 
 * FUNCTIONS: DATASIZE, HASHSIZE 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1985, 1990 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that this notice is preserved and that due credit is given
 * to the University of California at Berkeley. The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission. This software
 * is provided ``as is'' without express or implied warranty.
 *
 *	db.h	4.16 (Berkeley) 6/1/90
 */

/*
 * Global structures and variables for data base routines.
 */

#define INVBLKSZ	7	/* # of namebuf pointers per block */
#define INVHASHSZ	919	/* size of inverse hash table */

	/* max length of data in RR data field */
#define MAXDATA		2048

/*
 * Hash table structures.
 */
struct databuf {
	struct	databuf *d_next;	/* linked list */
	u_long	d_ttl;			/* time to live */
	short	d_flags;
	short	d_zone;			/* zone number */
	short	d_class;		/* class number */
	short	d_type;			/* type number */
	short	d_mark;			/* place to mark data */
	short	d_size;			/* size of data area */
	u_long	d_nstime;		/* NS response time, milliseconds */
	char	d_data[MAXDATA]; 	/* the data is malloc'ed to size */
};
#define DATASIZE(n) (sizeof(struct databuf) - MAXDATA + n)

/*
 * d_flags definitions
 */
#define DB_F_HINT       0x01	/* databuf belongs to fcachetab */
				/* The following two definitions have been */
				/* added according to RFC 1123 so that */
				/* negative responses may be cached */
#define DB_NAMEERROR    0x02	/* name doesn't exist */
#define DB_DATAERROR    0x04	/* name exists, but data for */
				/* class/type doesn't */

struct namebuf {
	char	*n_dname;		/* domain name */
	u_int	n_hashval;		/* hash value of n_dname */
	struct	namebuf *n_next;	/* linked list */
	struct	databuf *n_data;	/* data records */
	struct	namebuf *n_parent;	/* parent domain */
	struct	hashbuf *n_hash;	/* hash table for children */
};

struct invbuf {
	struct	invbuf *i_next;		/* linked list */
	struct	namebuf	*i_dname[INVBLKSZ];	/* domain name */
};

struct hashbuf {
	int	h_size;			/* size of hash table */
	int	h_cnt;			/* number of entries */
	struct	namebuf	*h_tab[1];	/* malloc'ed as needed */
};
#define HASHSIZE(s) (s*sizeof(struct namebuf *) + 2*sizeof(int))

#define HASHSHIFT	3
#define HASHMASK	0x1f

/*
 * Flags to updatedb
 */
#define DB_NODATA	0x01	/* data should not exist */
#define DB_MEXIST	0x02	/* data must exist */
#define DB_DELETE	0x04	/* delete data if it exists */
#define DB_NOTAUTH	0x08	/* must not update authoritative data */
#define DB_NOHINTS      0x10	/* don't reflect update in fcachetab */
				/* The following two flags have been */
				/* added according to RFC 1123 so that */
				/* negative responses may be cached */
#define DB_INDNAMEERROR 0x20	/* indicate that the name doesn't exist */
#define DB_INDDATAERROR 0x40	/* indicate that data for the class/type */
				/* doesn't exist although the name does */

#define DB_Z_CACHE      (0)	/* cache-zone-only db_dump()  */
#define DB_Z_ALL        (-1)	/* normal db_dump() */

/*
 * Error return codes
 */
#define OK		0
#define NONAME		-1
#define NOCLASS		-2
#define NOTYPE		-3
#define NODATA		-4
#define DATAEXISTS	-5
#define NODBFILE	-6
#define TOOMANYZONES	-7
#define GOODDB		-8
#define NEWDB		-9
#define AUTH		-10

extern struct hashbuf *hashtab;		/* root hash table */
extern struct invbuf *invtab[];		/* inverse hash table */
extern struct hashbuf *fcachetab;	/* hash table for cache read from file*/

extern struct namebuf *nlookup();
extern struct namebuf *savename();
extern struct databuf *savedata();
extern struct databuf *rm_datum();
extern struct hashbuf *savehash();
extern struct invbuf *saveinv();
extern char *savestr();
extern char *malloc(), *realloc(), *calloc();
