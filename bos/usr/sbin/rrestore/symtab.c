static char sccsid[] = "@(#)02        1.10.1.3  src/bos/usr/sbin/rrestore/symtab.c, cmdarch, bos411, 9428A410j 10/21/93 10:56:43";
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
static char sccsid[] = "(#)symtab.c	5.1 (Berkeley) 5/28/85";
#endif not lint
*/

/*
 * These routines maintain the symbol table which tracks the state
 * of the file system being restored. They provide lookup by either
 * name or inode number. They also provide for creation, deletion,
 * and renaming of entries. Because of the dynamic nature of pathnames,
 * names should not be saved, but always constructed just before they
 * are needed, by calling "myname".
 */

#include "restore.h"
#include <sys/stat.h>
#include <sys/audit.h>
#include <sys/priv.h>

/*
 * The following variables define the inode symbol table.
 * The primary hash table is dynamically allocated based on
 * the number of inodes in the file system (maxino), scaled by
 * HASHFACTOR. The variable "entry" points to the hash table;
 * the variable "entrytblsize" indicates its size (in entries).
 */
#define HASHFACTOR 5
static struct entry **entry;
static long entrytblsize;

/*
 * Look up an entry by inode number
 */
struct entry *
lookupino(inum)
	ino_t inum;
{
	register struct entry *ep;

	if (inum < ROOTINO || inum >= maxino)
		return (NIL);
	for (ep = entry[inum % entrytblsize]; ep != NIL; ep = ep->e_next)
		if (ep->e_ino == inum)
			return (ep);
	return (NIL);
}

/*
 * Add an entry into the entry table
 */
addino(inum, np)
	ino_t inum;
	struct entry *np;
{
	struct entry **epp;

	if (inum < ROOTINO || inum >= maxino)
		panic(MSGSTR(ADDINO, "addino: out of range %d\n"), inum);
	epp = &entry[inum % entrytblsize];
	np->e_ino = inum;
	np->e_next = *epp;
	*epp = np;
	if (dflag)
		for (np = np->e_next; np != NIL; np = np->e_next)
			if (np->e_ino == inum)
				badentry(np, MSGSTR(DUPINUM, "duplicate inum"));
}

/*
 * Delete an entry from the entry table
 */
deleteino(inum)
	ino_t inum;
{
	register struct entry *next;
	struct entry **prev;

	if (inum < ROOTINO || inum >= maxino)
		panic(MSGSTR(DELETINO, "deleteino: out of range %d\n"), inum);
	prev = &entry[inum % entrytblsize];
	for (next = *prev; next != NIL; next = next->e_next) {
		if (next->e_ino == inum) {
			next->e_ino = 0;
			*prev = next->e_next;
			return;
		}
		prev = &next->e_next;
	}
	panic(MSGSTR(DELINON, "deleteino: %d not found\n"), inum);
}

/*
 * Look up an entry by name
 */
struct entry *
lookupname(name)
	char *name;
{
	register struct entry *ep;
	register char *np, *cp;
	char buf[MAXPATHLEN];

	cp = name;
	for (ep = lookupino(ROOTINO); ep != NIL; ep = ep->e_entries) {
		for (np = buf; *cp != '/' && *cp != '\0'; )
			*np++ = *cp++;
		*np = '\0';
		for ( ; ep != NIL; ep = ep->e_sibling)
			if (strcmp(ep->e_name, buf) == 0)
				break;
		if (ep == NIL)
			break;
		if (*cp++ == '\0')
			return (ep);
	}
	return (NIL);
}

/*
 * Look up the parent of a pathname
 */
struct entry *
lookupparent(name)
	char *name;
{
	struct entry *ep;
	char *tailindex;

	tailindex = rindex(name, '/');
	if (tailindex == 0)
		return (NIL);
	*tailindex = '\0';
	ep = lookupname(name);
	*tailindex = '/';
	if (ep == NIL)
		return (NIL);
	if (ep->e_type != NODE)
		panic(MSGSTR(NOTDIR, "%s is not a directory\n"), name);
	return (ep);
}

/*
 * Determine the current pathname of a node or leaf
 */
char *
myname(ep)
	register struct entry *ep;
{
	register char *cp;
	static char namebuf[MAXPATHLEN];

	for (cp = &namebuf[MAXPATHLEN - 2]; cp > &namebuf[ep->e_namlen]; ) {
		cp -= ep->e_namlen;
		bcopy(ep->e_name, cp, (long)ep->e_namlen);
		if (ep == lookupino(ROOTINO))
			return (cp);
		*(--cp) = '/';
		ep = ep->e_parent;
	}
	panic(MSGSTR(PATHMAX, "%s: pathname too long\n"), cp);
	return(cp);
}

/*
 * Unused symbol table entries are linked together on a freelist
 * headed by the following pointer.
 */
static struct entry *freelist = NIL;

/*
 * add an entry to the symbol table
 */
struct entry *
addentry(name, inum, type)
	char *name;
	ino_t inum;
	int type;
{
	register struct entry *np, *ep;

	if (freelist != NIL) {
		np = freelist;
		freelist = np->e_next;
		bzero((char *)np, (long)sizeof(struct entry));
	} else {
		np = (struct entry *)calloc((size_t)1, (size_t)sizeof(struct entry));
		if (np == NIL)
			panic(MSGSTR(NOMEMSYM, "no memory to extend symbol table\n"));
	}
	np->e_type = type & ~LINK;
	ep = lookupparent(name);
	if (ep == NIL) {
		if (inum != ROOTINO || lookupino(ROOTINO) != NIL)
			panic(MSGSTR(BADNAMEA, "bad name to addentry %s\n"), name);
		np->e_name = savename(name);
		np->e_namlen = strlen(name);
		np->e_parent = np;
		addino(ROOTINO, np);
		return (np);
	}
	np->e_name = savename(rindex(name, '/') + 1);
	np->e_namlen = strlen(np->e_name);
	np->e_parent = ep;
	np->e_sibling = ep->e_entries;
	ep->e_entries = np;
	if (type & LINK) {
		ep = lookupino(inum);
		if (ep == NIL)
			panic(MSGSTR(LNNONEXT, "link to non-existant name\n"));
		np->e_ino = inum;
		np->e_links = ep->e_links;
		ep->e_links = np;
	} else if (inum != 0) {
		if (lookupino(inum) != NIL)
			panic(MSGSTR(DUPENT, "duplicate entry\n"));
		addino(inum, np);
	}
	return (np);
}

/*
 * delete an entry from the symbol table
 */
freeentry(ep)
	register struct entry *ep;
{
	register struct entry *np;
	ino_t inum;

	if (ep->e_flags != REMOVED)
		badentry(ep, MSGSTR(NOTMARKR, "not marked REMOVED"));
	if (ep->e_type == NODE) {
		if (ep->e_links != NIL)
			badentry(ep, MSGSTR(FREEDIR, "freeing referenced directory"));
		if (ep->e_entries != NIL)
			badentry(ep, MSGSTR(FREENOND, "freeing non-empty directory"));
	}
	if (ep->e_ino != 0) {
		np = lookupino(ep->e_ino);
		if (np == NIL)
			badentry(ep, MSGSTR(LOOKINO,"lookupino failed"));
		if (np == ep) {
			inum = ep->e_ino;
			deleteino(inum);
			if (ep->e_links != NIL)
				addino(inum, ep->e_links);
		} else {
			for (; np != NIL; np = np->e_links) {
				if (np->e_links == ep) {
					np->e_links = ep->e_links;
					break;
				}
			}
			if (np == NIL)
				badentry(ep, MSGSTR(LINKNOF, "link not found"));
		}
	}
	removeentry(ep);
	freename(ep->e_name);
	ep->e_next = freelist;
	freelist = ep;
}

/*
 * Relocate an entry in the tree structure
 */
moveentry(ep, newname)
	register struct entry *ep;
	char *newname;
{
	struct entry *np;
	char *cp;

	np = lookupparent(newname);
	if (np == NIL)
		badentry(ep, MSGSTR(CANTROOT, "cannot move ROOT"));
	if (np != ep->e_parent) {
		removeentry(ep);
		ep->e_parent = np;
		ep->e_sibling = np->e_entries;
		np->e_entries = ep;
	}
	cp = rindex(newname, '/') + 1;
	freename(ep->e_name);
	ep->e_name = savename(cp);
	ep->e_namlen = strlen(cp);
	if (strcmp(gentempname(ep), ep->e_name) == 0)
		ep->e_flags |= TMPNAME;
	else
		ep->e_flags &= ~TMPNAME;
}

/*
 * Remove an entry in the tree structure
 */
removeentry(ep)
	register struct entry *ep;
{
	register struct entry *np;

	np = ep->e_parent;
	if (np->e_entries == ep) {
		np->e_entries = ep->e_sibling;
	} else {
		for (np = np->e_entries; np != NIL; np = np->e_sibling) {
			if (np->e_sibling == ep) {
				np->e_sibling = ep->e_sibling;
				break;
			}
		}
		if (np == NIL)
			badentry(ep, MSGSTR(PALIST, "cannot find entry in parent list"));
	}
}

/*
 * Table of unused string entries, sorted by length.
 * 
 * Entries are allocated in STRTBLINCR sized pieces so that names
 * of similar lengths can use the same entry. The value of STRTBLINCR
 * is chosen so that every entry has at least enough space to hold
 * a "struct strtbl" header. Thus every entry can be linked onto an
 * apprpriate free list.
 *
 * NB. The macro "allocsize" below assumes that "struct strhdr"
 *     has a size that is a power of two.
 */
struct strhdr {
	struct strhdr *next;
};

#define STRTBLINCR	(sizeof(struct strhdr))
#define allocsize(size)	(((size) + 1 + STRTBLINCR - 1) & ~(STRTBLINCR - 1))

static struct strhdr strtblhdr[allocsize(MAXNAMLEN) / STRTBLINCR];

/*
 * Allocate space for a name. It first looks to see if it already
 * has an appropriate sized entry, and if not allocates a new one.
 */
char *
savename(name)
	char *name;
{
	struct strhdr *np;
	long len;
	char *cp;

	if (name == NULL)
		panic(MSGSTR(BADNAME, "bad name\n"));
	len = strlen(name);
	np = strtblhdr[len / STRTBLINCR].next;
	if (np != NULL) {
		strtblhdr[len / STRTBLINCR].next = np->next;
		cp = (char *)np;
	} else {
		cp = malloc((size_t)allocsize(len));
		if (cp == NULL)
			panic(MSGSTR(NOSPACST, "no space for string table\n"));
	}
	(void) strcpy(cp, name);
	return (cp);
}

/*
 * Free space for a name. The resulting entry is linked onto the
 * appropriate free list.
 */
freename(name)
	char *name;
{
	struct strhdr *tp, *np;
	
	tp = &strtblhdr[strlen(name) / STRTBLINCR];
	np = (struct strhdr *)name;
	np->next = tp->next;
	tp->next = np;
}

/*
 * Useful quantities placed at the end of a dumped symbol table.
 */
struct symtableheader {
	long	volno;
	long	stringsize;
	long	entrytblsize;
	time_t	dumptime;
	time_t	dumpdate;
	ino_t	maxino;
	long	ntrec;
};

/*
 * dump a snapshot of the symbol table
 */
dumpsymtable(filename, checkpt)
	char *filename;
	long checkpt;
{
	register struct entry *ep, *tep;
	register ino_t i;
	struct entry temp, *tentry;
	long mynum = 1, stroff = 0;
	FILE *fd;
	struct symtableheader hdr;

	vprintf(stdout, MSGSTR(CHECKPT, "Checkpointing the restore\n"));
	if ((fd = fopen(filename, "w")) == NULL) {
		perror(MSGSTR(FOPENE, "fopen"));
		panic(MSGSTR(SAVEFIL, "cannot create save file %s for symbol table\n"),
			filename);
	}
	clearerr(fd);
	/*
	 * Assign indicies to each entry
	 * Write out the string entries
	 */
	for (i = ROOTINO; i < maxino; i++) {
		for (ep = lookupino(i); ep != NIL; ep = ep->e_links) {
			ep->e_index = mynum++;
			(void) fwrite((void *)ep->e_name, (size_t)sizeof(char),
			       (size_t)allocsize(ep->e_namlen), fd);
		}
	}
	/*
	 * Convert pointers to indexes, and output
	 */
	tep = &temp;
	stroff = 0;
	for (i = ROOTINO; i < maxino; i++) {
		for (ep = lookupino(i); ep != NIL; ep = ep->e_links) {
			bcopy((char *)ep, (char *)tep,
				(long)sizeof(struct entry));
			tep->e_name = (char *)stroff;
			stroff += allocsize(ep->e_namlen);
			tep->e_parent = (struct entry *)ep->e_parent->e_index;
			if (ep->e_links != NIL)
				tep->e_links =
					(struct entry *)ep->e_links->e_index;
			if (ep->e_sibling != NIL)
				tep->e_sibling =
					(struct entry *)ep->e_sibling->e_index;
			if (ep->e_entries != NIL)
				tep->e_entries =
					(struct entry *)ep->e_entries->e_index;
			if (ep->e_next != NIL)
				tep->e_next =
					(struct entry *)ep->e_next->e_index;
			(void) fwrite((void *)tep, (size_t)sizeof(struct entry), (size_t)1, fd);
		}
	}
	/*
	 * Convert entry pointers to indexes, and output
	 */
	for (i = 0; i < entrytblsize; i++) {
		if (entry[i] == NIL)
			tentry = NIL;
		else
			tentry = (struct entry *)entry[i]->e_index;
		(void) fwrite((void *)&tentry, (size_t)sizeof(struct entry *), (size_t)1, fd);
	}
	hdr.volno = checkpt;
	hdr.maxino = maxino;
	hdr.entrytblsize = entrytblsize;
	hdr.stringsize = stroff;
	hdr.dumptime = dumptime;
	hdr.dumpdate = dumpdate;
	hdr.ntrec = ntrec;
	(void) fwrite((void *)&hdr, (size_t)sizeof(struct symtableheader), (size_t)1, fd);
	if (ferror(fd)) {
		perror(MSGSTR(FWRITEE, "fwrite"));
		panic(MSGSTR(EWRSYMF, "output error to file %s writing symbol table\n"),
			filename);
	}
	(void) fclose(fd);
}

/*
 * Initialize a symbol table from a file
 */
initsymtable(filename)
	char *filename;
{
	char *base;
	long tblsize;
	register struct entry *ep;
	struct entry *baseep, *lep;
	struct symtableheader hdr;
	struct stat stbuf;
	register long i;
	int fd;

	vprintf(stdout, MSGSTR(INITSYM, "Initialize symbol table.\n"));
	if (filename == NULL) {
		entrytblsize = maxino / HASHFACTOR;
		entry = (struct entry **)
			calloc((size_t)entrytblsize, (size_t)sizeof(struct entry *));
		if (entry == (struct entry **)NIL)
			panic(MSGSTR(NOMEMENT, "no memory for entry table\n"));
		ep = addentry(".", ROOTINO, NODE);
		ep->e_flags |= NEW;
		return;
	}
	if ((fd = open(filename, 0)) < 0) {
		perror(MSGSTR(OPEN, "open"));
		panic(MSGSTR(CANTOSYMF, "cannot open symbol table file %s\n"), filename);
	}
	if (fstat(fd, &stbuf) < 0) {
		perror(MSGSTR(STATFAIL, "stat"));
		panic(MSGSTR(CANTSTSYF, "cannot stat symbol table file %s\n"), filename);
	}
	tblsize = stbuf.st_size - sizeof(struct symtableheader);
	base = calloc((size_t)sizeof(char), (size_t)tblsize);
	if (base == NULL)
		panic(MSGSTR(ALLOCSYM, "cannot allocate space for symbol table\n"));
	if (read(fd, base, (int)tblsize) < 0 ||
	    read(fd, (char *)&hdr, sizeof(struct symtableheader)) < 0) {
		perror(MSGSTR(READ, "read"));
		panic(MSGSTR(SYMREAD, "cannot read symbol table file %s\n"), filename);
	}
	switch (command) {
	case 'r':
		/*
		 * For normal continuation, insure that we are using
		 * the next incremental tape
		 */
		if (hdr.dumpdate != dumptime) {
			if (hdr.dumpdate > dumptime)
				fprintf(stderr, MSGSTR(INCLOW,"Incremental archive too low\n"));
			else
				fprintf(stderr, MSGSTR(INCHIGH, "Incremental archive too high\n"));
			done(1);
		}
		break;
	case 'R':
		/*
		 * For restart, insure that we are using the same tape
		 */
		curfile.action = SKIP;
		dumptime = hdr.dumptime;
		dumpdate = hdr.dumpdate;
		if (!bflag)
			newtapebuf(hdr.ntrec);
		getvol(hdr.volno);
		break;
	default:
		panic(MSGSTR(INITSYC, "initsymtable called from command %c\n"), command);
		break;
	}
	maxino = hdr.maxino;
	entrytblsize = hdr.entrytblsize;
	entry = (struct entry **)
		(base + tblsize - (entrytblsize * sizeof(struct entry *)));
	baseep = (struct entry *)(base + hdr.stringsize - sizeof(struct entry));
	lep = (struct entry *)entry;
	for (i = 0; i < entrytblsize; i++) {
		if (entry[i] == NIL)
			continue;
		entry[i] = &baseep[(long)entry[i]];
	}
	for (ep = &baseep[1]; ep < lep; ep++) {
		ep->e_name = base + (long)ep->e_name;
		ep->e_parent = &baseep[(long)ep->e_parent];
		if (ep->e_sibling != NIL)
			ep->e_sibling = &baseep[(long)ep->e_sibling];
		if (ep->e_links != NIL)
			ep->e_links = &baseep[(long)ep->e_links];
		if (ep->e_entries != NIL)
			ep->e_entries = &baseep[(long)ep->e_entries];
		if (ep->e_next != NIL)
			ep->e_next = &baseep[(long)ep->e_next];
	}
}
