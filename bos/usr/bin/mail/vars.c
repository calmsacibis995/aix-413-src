static char sccsid[] = "@(#)78	1.4  src/bos/usr/bin/mail/vars.c, cmdmailx, bos411, 9428A410j 4/17/91 15:15:31";
/* 
 * COMPONENT_NAME: CMDMAILX vars.c
 * 
 * FUNCTIONS: MSGSTR, assign, findgroup, hash, lookup, printgroup, 
 *            value, vcopy, vfree 
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * #ifndef lint
 * static char *sccsid = "vars.c       5.2 (Berkeley) 6/21/85";
 * #endif not lint
 */

#include "rcv.h"

#include "mail_msg.h" 
extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_MAILX,n,s) 

/*
 * Mail -- a mail program
 *
 * Variable handling stuff.
 */

/*
 * Assign a value to a variable.
 */

assign(name, value)
	char name[], value[];
{
	register struct var *vp;
	register int h;

	h = hash(name);
	vp = lookup(name);
	if (vp == NOVAR) {
		vp = (struct var *) calloc(sizeof *vp, 1);
		vp->v_name = vcopy(name);
		vp->v_link = variables[h];
		variables[h] = vp;
	}
	else
		vfree(vp->v_value);
	vp->v_value = vcopy(value);
}

/*
 * Free up a variable string.  We do not bother to allocate
 * strings whose value is "" since they are expected to be frequent.
 * Thus, we cannot free same!
 */

vfree(cp)
	register char *cp;
{
	if (!equal(cp, ""))
		cfree(cp);
}

/*
 * Copy a variable value into permanent (ie, not collected after each
 * command) space.  Do not bother to alloc space for ""
 */

char *
vcopy(str)
	char str[];
{
	register char *top, *cp, *cp2;

	if (equal(str, ""))
		return("");
	if ((top = calloc(strlen(str)+1, 1)) == NULL)
		panic (MSGSTR(MEMORY, "Out of memory")); /*MSG*/
	cp = top;
	cp2 = str;
	while (*cp++ = *cp2++)
		;
	return(top);
}

/*
 * Get the value of a variable and return it.
 * Look in the environment if its not available locally.
 */

char *
value(name)
	char name[];
{
	register struct var *vp;

	if ((vp = lookup(name)) == NOVAR)
		return(getenv(name));
	return(vp->v_value);
}

/*
 * Locate a variable and return its variable
 * node.
 */

struct var *
lookup(name)
	char name[];
{
	register struct var *vp;
	register int h;

	h = hash(name);
	for (vp = variables[h]; vp != NOVAR; vp = vp->v_link)
		if (equal(vp->v_name, name))
			return(vp);
	return(NOVAR);
}

/*
 * Locate a group name and return it.
 */

struct grouphead *
findgroup(name)
	char name[];
{
	register struct grouphead *gh;
	register int h;

	h = hash(name);
	for (gh = groups[h]; gh != NOGRP; gh = gh->g_link)
		if (equal(gh->g_name, name))
			return(gh);
	return(NOGRP);
}

/*
 * Print a group out on stdout
 */

printgroup(name)
	char name[];
{
	register struct grouphead *gh;
	register struct group *gp;

	if ((gh = findgroup(name)) == NOGRP) {
		printf(MSGSTR(NOTGRP, "\"%s\": not a group\n"), name); /*MSG*/
		return;
	}
	printf("%s\t", gh->g_name);
	for (gp = gh->g_list; gp != NOGE; gp = gp->ge_link)
		printf(" %s", gp->ge_name);
	printf("\n");
}

/*
 * Hash the passed string and return an index into
 * the variable or group hash table.
 */

hash(name)
	char name[];
{
	register unsigned h;
	register char *cp;

	for (cp = name, h = 0; *cp; h = (h << 2) + *cp++)
		;
	if ((int) h < 0)
		h = -h;
	if ((int) h < 0)
		h = 0;
	return((int) h % HSHSIZE);
}
