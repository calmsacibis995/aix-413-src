static char sccsid[] = "@(#)31	1.3  src/tcpip/usr/ccs/lib/libsmit/libsmit.c, tcpip, tcpip411, GOLD410 12/8/92 07:17:38";
/*
 * COMPONENT NAME: (TCPIP) Sysmgt commands 
 *
 * FUNCTIONS: smit_compare_attr, smit_parse_stanza, cfgcopsz, cfgcclsf,
 *            cgfcrdsz, cfgcread
 *
 * ORIGINS: 27 
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include        <stdio.h>
#include        <sys/file.h>
#include        <fcntl.h>
#include        <errno.h>
#include        <sys/lockf.h>
#include        <sys/stat.h>
#include	<string.h>
#include	"smitcfg.h"
#define SPACE   '\040'

/* 
 * smit_compare_attr -> compares a string against an array of attributes.
 *                      returns - index into the array if a match is found,
 *                                otherwise, returns  -1.
 */

int smit_compare_attr(attr_list, string, lenflg)
	char	*attr_list[],
		*string;
	int	lenflg;
{
	int	ndx = 0; 
	int	len = 0; 

	while (attr_list[ndx] != NULL) {
		len = (lenflg) ? strlen(attr_list[ndx]) : 
			         strlen(attr_list[ndx]) - 1;
		if (!strncmp(string, attr_list[ndx], len))
			return(ndx);
		++ndx;
	}
	return(-1);
}

/* 
 * smit_parse_stanza ->  parses configuration stanza file entries for the 
 *                       TCP/IP SMIT hi-level print server commands.
 *                       returns (void).
 */

void smit_parse_stanza(stanza, name, attributes)
	char *stanza, *name, *attributes;
{
	char *cp, *cp2;

	*attributes = '\0';
	*name = '\0';
	cp = stanza;
	cp2 = cp + strlen(stanza);;
	while (cp < cp2) { 
		if (*cp == '\n')
			*cp = '\0';
		cp++;
	}
	cp = stanza;
	while (cp < cp2) {
		if (!strcmp(cp,"queue:") || !strcmp(cp,"device:")) {
		} else {
			cp += strspn(cp," \n\t,");
			if (!strncmp(cp,"name=",strlen("name="))) {
				cp += strlen("name=");
				strcpy(name,cp);
			} else {
				strcat(attributes," ");
				strcat(attributes,cp);
			}
		}
		cp += strlen(cp);
		cp++;
	}
}

int cfgcclsf(sfptr)
	CFG__SFT *sfptr;
{
	int fclose();
	void free();

    	int rc = CFG_SUCC;

    	rc = fclose(sfptr->sfile);

    	if (rc != 0) {                      /* if error returned            */
		rc = CFG_UNIO;              /* set standard return code     */
		strcpy(msgc2,sfptr->spath); /* set global message insert    */
    	}

    	if (sfptr->defbuf != NULL) {        /* free allocated storage       */
		free(sfptr->defbuf);
		free(sfptr->defmap);
    	}
    	free(sfptr);

    	return(rc);
}

CFG__SFT *cfgcopsf(fname)
	char *fname;
{
	FILE *fopen();
	int fclose();
/*	char *strcpy(); 	*/
	char *malloc();
    	FILE *fptr;
    	CFG__SFT *sfptr;
    	CFG__SFT *retval = NULL;

    	fptr = fopen(fname,"r+");
    	if (fptr != NULL) {
		sfptr = (CFG__SFT *)malloc(sizeof(CFG__SFT));
		if (sfptr != NULL) {
	    		sfptr->sfile = fptr;
	    		strcpy(sfptr->spath,fname);
	    		sfptr->defbuf = NULL;
	    		retval = sfptr;
		}
		else {
	    		fclose(fptr);
		}
    	}

    	if (retval == NULL)         /* if open failed set name variable */
		strcpy(msgc2,fname);
    	return(retval);
}

int cfgcrdsz(sfptr,buf,nbyte,stanza)
	CFG__SFT *sfptr;
	char *buf;
	int  nbyte;
	char *stanza;
{
	long ftell();
	void rewind();
	int cfgcread();
	char *malloc();
	char *calloc();

    	long startloc;              /* stanza search start location */
    	int  nameln = 0;            /* length of stanza name */
    	int  retc = 0;              /* return code */
    	struct {                    /* flags */
		unsigned quit : 1;  /* loop control */
		unsigned rewound : 1; /* search has wrapped to start of file */
    	} flag;

    	startloc = ftell(sfptr->sfile); /* get current location in file */

    	buf[0] = '\0';                      /* initialize buffer            */
    	nameln = strlen(stanza);            /* set length of stanza name    */
    	flag.quit = 0;                      /* initialize loop control      */
    	flag.rewound = 0;                   /* initialize wrap indicator    */
    	while (!flag.quit) {                /* while searching              */
					/* read one stanza              */
		retc = cfgcread(sfptr,buf,nbyte);
		switch (retc) {              /* switch on return from read   */
			case CFG_SUCC:       /* case: read successful        */
			case CFG_SZBF:       /* successful but stanza too big*/
				if ((stanza == NULL)
					|| ((buf[nameln] == ':')
					&& (strncmp(stanza,buf,nameln) == 0))) {
		    			flag.quit = 1; 
				} else if (flag.rewound) {
		    			if (ftell(sfptr->sfile) >= startloc) {
						retc = CFG_SZNF;
						flag.quit = 1;
		    				}
	    			}
	    			break;

			case CFG_EOF:   /* case: end of file reached    */
	    			if (stanza != NULL) {
					if (flag.rewound) {     
		    				flag.quit = 1;
		    				retc = CFG_SZNF;
					} else {                 
		    				cfgcrwnd(sfptr);
		    				flag.rewound = 1;
					}

	    			} else 
					flag.quit = 1;
	    
				break;

			default:             /* case: other                  */
	    			flag.quit = 1;         
	    			break;                
		}
    	}
    	return(retc);
}

int cfgcread(sfptr,buf,nbyte)
	CFG__SFT *sfptr;
	char *buf;
	int  nbyte;
{
	int ungetc();
    	int c;                     /* temporary character storage */
    	int i = 0;                  /* index variable */
    	int rc = 0;                 /* return code */
    	char namebuf[MAXVAL];       /* stanza name buffer */
    	struct {                    /* flags */
		unsigned nmfnd : 1;     /* name found */
		unsigned snfnd : 1;     /* stanza end found */
		unsigned one_nl : 1;    /* one newline found */
    	} flag;

    	flag.nmfnd = 0;           
    	while (!flag.nmfnd) {    
		i = 0;          
		while ((c = getc(sfptr->sfile)) != SPACE && (c != '\n')) {
	    		if (feof(sfptr->sfile)) {   /* return on end of file  */
				flag.nmfnd = 1;
				break;
	    		}
					/* if 1st character is an *     */
					/* this is a comment. so read to*/
					/* end of line.                 */
	    		if (c == '*' && i == 0) {
	     			while ((c = getc(sfptr->sfile)) != '\n') {
	       				if (feof(sfptr->sfile)) { 
						flag.nmfnd = 1;
						break;
	       				}
	     			}
	     			break;
	    		}

	    		if (i < sizeof(namebuf)-2) {
				namebuf[i++] = c;  
				if (c == ':')  
		    			flag.nmfnd = 1; 
	    		} else                       
				flag.nmfnd = 0;     
		}
    	}
    	if (feof(sfptr->sfile))             /* return on end of file        */
		rc = CFG_EOF;
    	else {
		ungetc(c,sfptr->sfile);     /* put back space or newline    */
		namebuf[i] = '\0';           /* terminate stanza name        */
		if (i < nbyte) {             /* if stanza name < buffer size */
	    		strcpy(buf,namebuf); /* move stanza name to buffer   */
	    		flag.snfnd = 0;     /* initialize loop flags        */
	    		flag.one_nl = 0;
					/* while end of stanza not found*/
					/*   and buffer space left      */
	    		while ((!flag.snfnd) && (i < nbyte)) {
				c = getc(sfptr->sfile);
				if (feof(sfptr->sfile)) {
		    			flag.snfnd = 1; 
		    			buf[i++] = '\n';
				} else {
		    			buf[i++] = c;  
		    			if (c == '\n')
						if (flag.one_nl) flag.snfnd = 1;
						else flag.one_nl = 1;
		    			else { 
						if (c > ' ') 
							flag.one_nl = 0;}
					}
	    			}
		    	if (i == nbyte) {     /* if buffer was filled         */
				rc = CFG_SZBF; /* return buffer too small    */
				i--;          /* prepare to insert terminator */
		    	} else
				rc = CFG_SUCC; /* indicate successful read   */
		    	buf[i] = '\0';        /* terminate buffer with null   */
		} else
		    rc = CFG_SZBF;           /* return buffer too small      */

    	}
    	if (ferror(sfptr->sfile)) {         /* check for error on read      */
		rc = CFG_UNIO;              /* set return code              */
		clearerr(sfptr->sfile);     /* reset error indication       */
		strcpy(msgc2,sfptr->spath); /* set message insert variable  */
    	}
    	return(rc);
}
