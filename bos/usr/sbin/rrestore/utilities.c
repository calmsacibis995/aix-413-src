static char sccsid[] = "@(#)04	1.11  src/bos/usr/sbin/rrestore/utilities.c, cmdarch, bos411, 9428A410j 10/21/93 11:44:23";
/*
 * COMPONENT_NAME: (CMDARCH) archive files
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
#ifndef lint
static char sccsid[] = "(#)utilities.c	5.2 (Berkeley) 8/5/85";
#endif not lint
*/

#include "restore.h"
#include <string.h>

/*
 * Insure that all the components of a pathname exist.
 */
pathcheck(name)
	char *name;
{
	register char *cp;
	struct entry *ep;
	char *start;

/*	start = index(name, '/'); */
	start = strchr(name, '/');
	if (start == 0)
		return;
	for (cp = start; *cp != '\0'; cp++) {
		if (*cp != '/')
			continue;
		*cp = '\0';
		ep = lookupname(name);
		if (ep == NIL) {
			ep = addentry(name, psearch(name), NODE);
			newnode(ep);
		}
		ep->e_flags |= NEW|KEEP;
		*cp = '/';
	}
}

/*
 * Change a name to a unique temporary name.
 */
mktempname(ep)
	register struct entry *ep;
{
	char oldname[MAXPATHLEN];

	if (ep->e_flags & TMPNAME)
		badentry(ep, MSGSTR(MKTEMP, "mktempname: called with TMPNAME"));
	ep->e_flags |= TMPNAME;
	(void) strcpy(oldname, myname(ep));
	freename(ep->e_name);
	ep->e_name = savename(gentempname(ep));
	ep->e_namlen = strlen(ep->e_name);
	renameit(oldname, myname(ep));
}

/*
 * Generate a temporary name for an entry.
 */
char *
gentempname(ep)
	struct entry *ep;
{
	static char name[MAXPATHLEN];
	struct entry *np;
	long i = 0;

	for (np = lookupino(ep->e_ino); np != NIL && np != ep; np = np->e_links)
		i++;
	if (np == NIL)
		badentry(ep, MSGSTR(NOTILIST, "not on ino list"));
	(void) sprintf(name, "%s%d%d", TMPHDR, i, ep->e_ino);
	return (name);
}

/*
 * Rename a file or directory.
 */
renameit(from, to)
	char *from, *to;
{
	if (rename(from, to) < 0) {
		fprintf(stderr, MSGSTR(CANTRES, "Warning: cannot rename %s to %s"), from, to);
		(void) fflush(stderr);
		perror("");
		return;
	}
	vprintf(stdout, MSGSTR(RENAME, "rename %s to %s\n"), from, to);
}

/*
 * Create a new node (directory).
 */
newnode(np)
	struct entry *np;
{
	char *cp;

	if (np->e_type != NODE)
		badentry(np, MSGSTR(NEWNODE, "newnode: not a node"));
	cp = myname(np);
	if (mkdir(cp, (mode_t)0777) < 0) {
		np->e_flags |= EXISTED;
		fprintf(stderr, MSGSTR(WARNING, "Warning: "));
		(void) fflush(stderr);
		perror(cp);
		return;
	}
	vprintf(stdout, MSGSTR(MAKENODE, "Make node %s\n"), cp);
}

/*
 * Remove an old node (directory).
 */
removenode(ep)
	register struct entry *ep;
{
	char *cp;

	if (ep->e_type != NODE)
		badentry(ep, MSGSTR(REMNODE, "removenode: not a node"));
	if (ep->e_entries != NIL)
		badentry(ep, MSGSTR(REMNONE, "removenode: non-empty directory"));
	ep->e_flags |= REMOVED;
	ep->e_flags &= ~TMPNAME;
	cp = myname(ep);
	if (rmdir(cp) < 0) {
		fprintf(stderr, MSGSTR(WARNING, "Warning: "));
		(void) fflush(stderr);
		perror(cp);
		return;
	}
	vprintf(stdout, MSGSTR(REMOVEN, "Remove node %s\n"), cp);
}

/*
 * Remove a leaf.
 */
removeleaf(ep)
	register struct entry *ep;
{
	char *cp;

	if (ep->e_type != LEAF)
		badentry(ep, MSGSTR(REMLEAF, "removeleaf: not a leaf"));
	ep->e_flags |= REMOVED;
	ep->e_flags &= ~TMPNAME;
	cp = myname(ep);
	if (unlink(cp) < 0) {
		fprintf(stderr, MSGSTR(WARNING, "Warning: "));
		(void) fflush(stderr);
		perror(cp);
		return;
	}
	vprintf(stdout, MSGSTR(REMLEAF2, "Remove leaf %s\n"), cp);
}

/*
 * Create a link.
 */
linkit(existing, new, type)
	char *existing, *new;
	int type;
{

	if (type == SYMLINK) {
		if (symlink(existing, new) < 0) {
			fprintf(stderr,
				MSGSTR(SYMLNW, "Warning: cannot create symbolic link %s->%s: "),
				new, existing);
			(void) fflush(stderr);
			perror("");
			return (FAIL);
		}
	} else if (type == HARDLINK) {
		if (link(existing, new) < 0) {
			fprintf(stderr,
				MSGSTR(HARDLNW, "Warning: cannot create hard link %s->%s: "),
				new, existing);
			(void) fflush(stderr);
			perror("");
			return (FAIL);
		}
	} else {
		panic(MSGSTR(LINKIT, "linkit: unknown type %d\n"), type);
		return (FAIL);
	}
	vprintf(stdout, MSGSTR(CRLN, "Create %s link %s->%s\n"),
		type == SYMLINK ? MSGSTR(SYMBOLC, "symbolic") : MSGSTR(HARD, "hard"), new, existing);
	return (GOOD);
}

/*
 * find lowest number file (above "start") that needs to be extracted
 */
ino_t
lowerbnd(start)
	ino_t start;
{
	register struct entry *ep;

	for ( ; start < maxino; start++) {
		ep = lookupino(start);
		if (ep == NIL || ep->e_type == NODE)
			continue;
		if (ep->e_flags & (NEW|EXTRACT))
			return (start);
	}
	return (start);
}

/*
 * find highest number file (below "start") that needs to be extracted
 */
ino_t
upperbnd(start)
	ino_t start;
{
	register struct entry *ep;

	for ( ; start > ROOTINO; start--) {
		ep = lookupino(start);
		if (ep == NIL || ep->e_type == NODE)
			continue;
		if (ep->e_flags & (NEW|EXTRACT))
			return (start);
	}
	return (start);
}

/*
 * report on a badly formed entry
 */
badentry(ep, msg)
	register struct entry *ep;
	char *msg;
{

	fprintf(stderr, MSGSTR(BADENTRY, "bad entry: %s\n"), msg);
	fprintf(stderr, MSGSTR(NAME, "name: %s\n"), myname(ep));
	fprintf(stderr, MSGSTR(PANAME, "parent name %s\n"), myname(ep->e_parent));
	if (ep->e_sibling != NIL)
		fprintf(stderr, MSGSTR(SIBLING, "sibling name: %s\n"), myname(ep->e_sibling));
	if (ep->e_entries != NIL)
		fprintf(stderr, MSGSTR(NEXTENT, "next entry name: %s\n"), myname(ep->e_entries));
	if (ep->e_links != NIL)
		fprintf(stderr, MSGSTR(NEXTLNN, "next link name: %s\n"), myname(ep->e_links));
	if (ep->e_next != NIL)
		fprintf(stderr, MSGSTR(NEXTHASH, "next hashchain name: %s\n"), myname(ep->e_next));
	fprintf(stderr, MSGSTR(ENTRYTYP, "entry type: %s\n"),
		ep->e_type == NODE ? MSGSTR(NODEE, "NODE") : MSGSTR(LEAFN, "LEAF"));
	fprintf(stderr, MSGSTR(INODEN, "inode number: %ld\n"), ep->e_ino);
	panic(MSGSTR(FLAGS, "flags: %s\n"), flagvalues(ep));
}

/*
 * Construct a string indicating the active flag bits of an entry.
 */
char *
flagvalues(ep)
	register struct entry *ep;
{
	static char flagbuf[BUFSIZ];

	(void) strcpy(flagbuf, "|NIL");
	flagbuf[0] = '\0';
	if (ep->e_flags & REMOVED)
		(void) strcat(flagbuf, "|REMOVED");
	if (ep->e_flags & TMPNAME)
		(void) strcat(flagbuf, "|TMPNAME");
	if (ep->e_flags & EXTRACT)
		(void) strcat(flagbuf, "|EXTRACT");
	if (ep->e_flags & NEW)
		(void) strcat(flagbuf, "|NEW");
	if (ep->e_flags & KEEP)
		(void) strcat(flagbuf, "|KEEP");
	if (ep->e_flags & EXISTED)
		(void) strcat(flagbuf, "|EXISTED");
	return (&flagbuf[1]);
}

/*
 * Check to see if a name is on a dump tape.
 */
ino_t
dirlookup(name)
	char *name;
{
	ino_t ino;

	ino = psearch(name);
	if (ino == 0 || BIT(ino, dumpmap) == 0)
		fprintf(stderr, MSGSTR(NOTONARCH, "%s is not on the archive\n"), name);
	return (ino);
}

/*
 * Elicit a reply.
 */
reply(question)
	char *question;
{
	char c;
	char ans[10];
	int i, j;
	char * catmsg_str;

	catmsg_str = MSGSTR(YESNO,""); /* to help determine if we have catalog */
	do	{
		for (i=0; i<10; i++)
			ans[i]='\0';
        if ( *catmsg_str == NULL )
          fprintf(stderr,"%s? [ yes or no ] ", question);
        else
		  fprintf(stderr,"%s? [ %s ] ",question,catmsg_str);
		(void) fflush(stderr);
		i = 1;
		c = getc(terminal);
		ans[0] = c;
		while (c != '\n' && (c = getc(terminal)) != '\n') {
			if (feof(terminal))
				return (FAIL);
			if (i < 9)
				ans[i++] = c;
		}
        if ( *catmsg_str == NULL ) 
		  {                        /* must be English response */
           if (strcmp(ans,"yes") == 0 )
             j = 1;
           else 
             if (strcmp(ans,"no") == 0 )
		  	   j = 0;
             else
			   j = -1;
          }
        else 
		  j = rpmatch(ans);

	} while (j == -1);
	if (j)
		return (GOOD);
	return (FAIL);
}

/*
 * handle unexpected inconsistencies
 */
/* VARARGS1 */
panic(msg, d1, d2)
	char *msg;
	long d1, d2;
{

	fprintf(stderr, msg, d1, d2);
	if (reply(MSGSTR(ABORT, "abort")) == GOOD) {
		if (reply(MSGSTR(CORE, "dump core")) == GOOD)
			abort();
		done(1);
	}
}
