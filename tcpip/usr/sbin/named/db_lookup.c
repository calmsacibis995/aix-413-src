static char sccsid[] = "@(#)91	1.7  src/tcpip/usr/sbin/named/db_lookup.c, tcpnaming, tcpip411, GOLD410 6/3/94 10:05:55";
/* 
 * COMPONENT_NAME: TCPIP db_lookup.c
 * 
 * FUNCTIONS: match, nlookup 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
#ifndef lint
static char sccsid[] = "db_lookup.c	4.17 (Berkeley) 6/1/90";
#endif
*/

/*
 * Table lookup routines.
 */

/*
 * The following changes have been made according to RFC 1123:
 *
 * Negative responses are cached.  If a name error is cached, there
 * is a name record and a data record with d_flag set to DB_NAMEERROR,
 * but no data portion.  match() has been changed to return "match"
 * if DB_NAMEERROR is set.
 */

#include <sys/types.h>
#include <stdio.h>
#include <arpa/nameser.h>
#include "db.h"

struct hashbuf *hashtab;	/* root hash table */
struct hashbuf *fcachetab;	/* hash table of cache read from file */

#ifdef DEBUG
extern int debug;
extern FILE *ddt;
#endif

/* 
 * Lookup 'name' and return a pointer to the namebuf;
 * NULL otherwise. If 'insert', insert name into tables.
 * Wildcard lookups are handled.
 */
struct namebuf *
nlookup(name, htpp, fname, insert)
	char *name;
	struct hashbuf **htpp;
	char **fname;
	int insert;
{
	register struct namebuf *np;
	register char *cp;
	register int c;
	register unsigned hval;
	register struct hashbuf *htp;
	struct namebuf *parent = NULL;

	htp = *htpp;
	hval = 0;
	*fname = "???";
	for (cp = name; c = *cp++; ) {
		if (c == '.') {
			parent = np = nlookup(cp, htpp, fname, insert);
			if (np == NULL)
				return (NULL);
			if (*fname != cp)
				return (np);
			if ((htp = np->n_hash) == NULL) {
				if (!insert) {
					if (np->n_dname[0] == '*' && 
					    np->n_dname[1] == '\0')
						*fname = name;
					return (np);
				}
				htp = savehash((struct hashbuf *)NULL);
				np->n_hash = htp;
			}
			*htpp = htp;
			break;
		}
		hval <<= HASHSHIFT;
		hval += (isupper(c) ? tolower(c) : c) & HASHMASK;
	}
	c = *--cp;
	*cp = '\0';
	/*
	 * Lookup this label in current hash table.
	 */
	for (np = htp->h_tab[hval % htp->h_size]; np != NULL; np = np->n_next) {
		if (np->n_hashval == hval &&
		    strcasecmp(name, np->n_dname) == 0) {
			*cp = c;
			*fname = name;
			return (np);
		}
	}
	if (!insert) {
		/*
		 * Look for wildcard in this hash table.
		 * Don't use a cached "*" name as a wildcard,
		 * only authoritative.
		 */
		hval = ('*' & HASHMASK)  % htp->h_size;
		for (np = htp->h_tab[hval]; np != NULL; np = np->n_next) {
			if (np->n_dname[0] == '*'  && np->n_dname[1] == '\0' &&
			    np->n_data && np->n_data->d_zone != 0) {
				*cp = c;
				*fname = name;
				return (np);
			}
		}
		*cp = c;
		return (parent);
	}
	np = savename(name);
	np->n_parent = parent;
	np->n_hashval = hval;
	hval %= htp->h_size;
	np->n_next = htp->h_tab[hval];
	htp->h_tab[hval] = np;
	/* increase hash table size */
	if (++htp->h_cnt > htp->h_size * 2) {
		*htpp = savehash(htp);
		if (parent == NULL) {
			if (htp == hashtab)
			    hashtab = *htpp;
			else
			    fcachetab = *htpp;
		}
		else
			parent->n_hash = *htpp;
		htp = *htpp;
	}
	*cp = c;
	*fname = name;
	return (np);
}

/*
 * Does the data record match the class and type?
 */
match(dp, class, type)
	register struct databuf *dp;
	register int class, type;
{
#ifdef DEBUG
	if (debug >= 5)
		fprintf(ddt,"match(0x%x, %d, %d) %d, %d\n", dp, class, type,
			dp->d_class, dp->d_type);
#endif
	if (dp->d_flags & DB_NAMEERROR)
		return (1);
	if (dp->d_class != class && class != C_ANY)
		return (0);
	if (dp->d_type != type && type != T_ANY)
		return (0);
	return (1);
}
