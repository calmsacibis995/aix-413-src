static char sccsid[] = "@(#)23	1.12  src/bos/usr/sbin/audit/audutil.c, cmdsaud, bos411, 9428A410j 6/14/91 15:44:42";
/*
 * COMPONENT_NAME: (CMDSAUD) security: auditing subsystem
 *
 * FUNCTIONS: audutil support library for audit commands
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include	<stdio.h>
#include	<unistd.h>
#include	"audutil.h"
#include	<errno.h>

#undef	DPRINTF
#ifdef	DEBUGX
#	define	DPRINTF(args)	printf args
#else
#	define	DPRINTF(args)
#endif

xfork(){

	int	i;

	i = fork();
	if (i < 0)
		exit1 (1, "** fork failed\n", 0);
	return (i);
}

char *
xalloc(int len){

	extern	char	*malloc ();
	char	*p;

	p = malloc (len);
	if (p)
		return (p);
	exit1 (-1, "out of memory", 0);
}

char *
xralloc(char *buf, int len){

	extern	char	*realloc ();
	char	*p;

	p = realloc (buf, len);
	if (p)
		return (p);
	exit1 (0, "out of memory", 0);
}

char *
xcalloc(int n, int siz){

	extern	char	*calloc ();
	char	*p;

	p = calloc (n, siz);
	if(p)return (p);

	exit1 (0, "out of memory", 0);
}

void
exit1(int doperror, char *msg, char *arg){

	int	errno_sav;
        static char ttydev[] = "/dev/tty";
	
	errno_sav = errno;

	if(isatty(STDERR_FILENO)){

        	freopen(ttydev, "w", stderr);

	}

	errno = errno_sav;

	fprintf(stderr, msg, arg);

	fprintf(stderr, "\n", msg);
	if (doperror){

		perror ("");

	}

	exit(errno_sav);

}


/* list manipulation routines */
list_len (char *list){

	char	*p;

	p = list;
	while(*p++){
		while (*p++)
			;
	}
	return (p - list);
}

list_print(FILE *fp, char *str){

	if(*str == '\0')return;

	while(1){
		while(1){
			char	c;

			c = *str++;
			if (c == '\0')
				break;
			putc (c, fp);
		}
		if (*str == '\0')break;
		putc (',', fp);
	}
}

list_find(char *list, char *str){

	char	*p;

	p = list;
	while(*p){
		if(strcmp (p, str) == 0){
			return (1);
		}
		while (*p++)
			;
	}
	return (NULL);
}

list_add(char *list, char *str){

	while(*list){
		while (*list++)
			;
	}
	while(*list++ = *str++);

	*list++ = 0;
}


#ifdef	DEBUG
#include	"varargs.h"

char	*DebugName;

dprintf (va_alist)
va_dcl{
	va_list	ap;
	char	*fmt;
	static	int	eol = 1;

	va_start (ap);
	fflush (stdout);
	fflush (stderr);

	fmt = va_arg (ap, char *);
	if (eol){
		if (DebugName == NULL)
			DebugName = "?????";
		fprintf (stderr, "%-12s", DebugName);
	}
	vfprintf (stderr, fmt, ap);
	fflush (stderr);
	while (*fmt)
		fmt++;
	eol = (fmt[-1] == '\n');

	va_end (ap);
}

char *
list_dprintf (char *list){

	static	char	buf[BUFSIZ];
	char	*bufp;

	if (list == NULL)
		return ("NULL");

	bufp = buf;
	if (*list){
		while (1) {
			while (1) {
				char	c;

				c = *list++;
				if (c == '\0')
					break;
				*bufp++ = c;
			}
			if (*list == '\0')
				break;
			*bufp++ = ',';
		}
	}
	*bufp++ = '\0';
	return (buf);
}

class_dprintf(char *indent){

	struct	audit_class	*ae;
	int	ae_len;
	int	ae_cnt;
	int	i;

	/* get current audit events */
	if((ae_cnt = auditevents (AUDIT_GET, &ae_len, sizeof (ae_len))) < 0){
		if(errno != ENOSPC)exit1(1, "auditevents", 0);
	}
	if((ae_cnt == 0) || (ae_len == 0)){
		fprintf (stderr, "\tnone\n");
		return;
	}

	ae = (struct audit_class *) xalloc (ae_len);
	ae_cnt = auditevents (AUDIT_GET, ae, ae_len);
	if (ae_cnt < 0)exit1 (1, "auditevents", 0);
	
	/* find the specified class */
	for(i = 0; i < ae_cnt; i++){
		fprintf (stderr, "%s%s = %s\n", indent, ae[i].ae_name, 
		list_dprintf (ae[i].ae_list));
	}
	
	free ((char *) ae);
}
#endif
