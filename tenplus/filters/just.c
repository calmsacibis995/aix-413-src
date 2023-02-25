#if !defined(lint)
static char sccsid[] = "@(#)64  1.9  src/tenplus/filters/just.c, tenplus, tenplus411, GOLD410 8/5/91 16:08:50";
#endif /* lint */

/*
 * COMPONENT_NAME: (INED) INed Editor
 *
 * FUNCTIONS:	main, ch, wcs
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <locale.h>
#include <limits.h>

#include <ctype.h>
#include <chardefs.h>
#include <database.h>
#include <libutil.h>

int	MultibyteCodeSet;	/* global code set flag */

#define LBUF 512
#define TAB L'\b' 

char file[PATH_MAX];
int just;
FILE *i;

wchar_t ch(void);
wchar_t *wcs(char *);

/*ARGSUSED*/
main(int argc,char **argv)
{
 int j,noutd;
 int nind = 0;
 int onind = 0;
 char ipflag = 0;
 char *args[50],rflag,bflag,**d,nflag,dontflag,*cmdchar;
 char cmdch;
 wchar_t buf[LBUF],*firstl,e;
 wchar_t *wq, *wp, *firsti,*thisstart,*r;
 register char **c, *q, *p;
 /* toofar is used to check whether it was tried to read a line
    longer than LBUF characters */
 int toofar;

/* set the local environment */
(void) setlocale(LC_ALL, "");

 if (MB_CUR_MAX == 1)
      MultibyteCodeSet = 0;
 else MultibyteCodeSet = 1;

/* find out what name I was called by */
 for(q = argv[0]; *q; q++ );
 while((q >= argv[0]) && (*q != DIR_SEPARATOR))
	q--;

 if (q[1] == 'j')
	just = 1;
 else   just = 0;

 cmdchar = "~~";
 cmdch = '~';
 rflag = dontflag = 0;
 d = argv;
 d++;
 while (*d != NULL && **d == '-' &&
        (*(*d + 1) == 'd' || *(*d + 1) == 'x') ) {
	while (*++(*d)) switch (**d) {
		case 'x': rflag = 1; break;
		case 'd': dontflag = 1; cmdchar = "."; cmdch = '\\'; break;
		}
	d++;
	}
 if (dontflag) i = stdout;      /* Write on std output if -d flag */
    else {
	p = file;
	for (q = "/var/tmp/IjustXXXXXX"; *p++ = *q++; );
	p = mktemp (file);
	if ((i = fopen(file,"w")) == NULL) exit (-2);
	}

 if (just == 0) (void) fputs(".na\n",i);
 (void) fputs(".hy 0\n",i);                  /* hyphenate off */
 if (dontflag == 0) (void) fputs(".pl 1\n",i);    /* If running nroff, inhibit paging */

 /* Copy argument list either to roff or to roff file */
 c = args;

 c++;
 while (*d) {
	if (**d == '.') {
		p = *d;
		while (*p) 
                  (void) fputc(*p++,i);
		(void) fputc('\n',i);
		}
	  else *c++ = *d;
	d++;
	}
 *c++ = file;
 *c++ = 0;

 if (dontflag == 0) 
       (void) fprintf(i,".c2 \032\n.cc %c\n%cec %c\n",cmdch,cmdch,cmdch);
 nflag = bflag = noutd = nind = 0;
 wp = buf;
/* Main loop - once per line */
 for (;;) {
     again:
	thisstart = wp;
/* Read the next input line */
	wq = 0;
	while (!(toofar = ((wp - buf) >= LBUF)) &&
               (*wp++ = e = ch()) != L'\n')
	    if (e == L'\0') {
		if (bflag == 1) break; goto endit; }
	    else if (e == L'.' && wp == thisstart + 1) {
/* If an nroff command pass it through without affecting text processing */
		if (dontflag == 0) { --wp;
                                    (void) wsprintf(wp,"%sti 0-%cn(.i\n%c&.",
					cmdchar,cmdch,cmdch);
                                    while (*wp != L'\0') wp++; }
		wq = wp;
	        while (!(toofar = ((wp - buf) >= LBUF)) &&
                       (*wp++ = e = ch()) != L'\n') 
                  if (e == L'\0') {
			if (bflag == 1) break; goto endit; }
                if (toofar) exit(-2);
/* Check if fill to be turned off or on */
		if ((*wq == L'F' && wq[1] == L'\n') ||
			(*wq == L'f' && wq[1] == L'i')) {rflag &= ~2; wq = L"fi\n";}
		else if ((*wq == L'N' && wq[1] == L'F') ||
				   (*wq == L'n' && wq[1] == L'f')) {
			rflag |= 2;
			for (wq = wcs(cmdchar); *wq != L'\0'; *wp++ = *wq++);
			for (wq = L"in 0\n"; *wq != L'\0'; *wp++ = *wq++);
			nind = 0;
			wq = L"nf\n";
			}
/* Stuff between .FS and .FE shouldn't be handled at all..... */
		else if(*wq == L'F' && wq[1] == L'S')
			rflag |= 4;
		else if(*wq == L'F' && wq[1] == L'E')
			rflag &= ~4;
/* See whether a new paragraph is started with .PS instead of blank line */
		else if ((*wq == L'P' && wq[1] == L'S'))   /* version 1.8 */
			bflag=1; /* make it think it got a blank ln */
		     else goto noflip;
/* Pass fill command through */
		bflag = 0;
		if (dontflag == 0) {
			for (r = wcs(cmdchar); *r != L'\0'; *wp++ = *r++);
			while ((*wp++ = *wq++) != L'\n');
			}
		noflip:
/* Then add break command */
		if (dontflag == 0) {for (wq = wcs(cmdchar); *wq != '\0';
                                         *wp++ = *wq++);
				    for (wq = L"br\n"; (*wq != L'\0'); *wp++ = *wq++);  }
		if (bflag == 0) goto contin;
		goto again;
		}
        if (toofar) exit(-2);
/*
 * case to handle "text.~\n" (inhibit nroff action on input lines terminated
 * by ".".
 */
	if((wp - 2 >= buf) && (*(wp - 2) == L'~'))
	{
		wp -= 2;
		*wp++ = L'\\';
		*wp++ = L'&';
		*wp++ = L'\n';
	}

	if (buf - wp > LBUF) exit(-2);
/* This is where scanned, maybe compacted text in buffer gets output */
	if (rflag & 2) goto copyit;
	if (rflag & 4) goto copyit;
	if (0) { contin:
		     wq = buf;
		     *wp = L'\0';
		     if (ipflag) {
			/* There's one embedded TAB */
			while (*wq != TAB) wq++;
			*wq = L'\0';
			(void) fputws(buf,i);
			(void) fprintf(i,"%ch'|%d'",cmdch,nind - noutd,ipflag);
			ipflag = 0;
			while (is_wspc(*++wq));
			}
		     if (wp - wq) (void) fputws(wq,i);
		     continn:
		     wp = buf;
		     continue; }
	/* bflag - 0 for last line blank
		   1 for last line first nonblank
		   2 for in middle of text. */
	if (wp - thisstart == 1 && bflag != 1) {     /* blank line */
		bflag = 0;
		/* nroff versions treat leading blank lines inconsistently
		   so copy them directly to output file. */
		if (nflag || dontflag) goto contin;
		(void) fputc('\n',i);
		goto continn;
		}
	nflag = 1;
	if (bflag == 0) {
			wq = thisstart;  /* first nonblank line */
			noutd = 0;
			for (;;) {
				if (wq >= wp) break;
				if (is_wspc(*wq)) noutd++;
                                else if (*wq == L'\t')
                                       noutd = 0177770 & (noutd + 8);
				else break;
				wq++; }
			bflag = 1;
			firsti = wq;
			continue;
		 }
	if (bflag == 1) {
		firstl = thisstart;
		onind = nind;
		if (wp - firstl == 1) {  /* Isolated blank line */
			bflag = 0;
			if (noutd > nind)
				(void) fprintf(i,"%sti +%d\n",cmdchar,noutd - nind);
			if (noutd < nind)
				(void) fprintf(i,"%sti %d\n",cmdchar,noutd - nind);
			}
		else {  wq = firstl;     /* second nonblank line */
			nind = 0;
			for (;;) {
				if (wq >= wp) break;
				if (is_wspc(*wq)) nind++;
                                else if (*wq == L'\t')
                                       nind = 0177770 & (nind + 8);
				else break;
				wq++; }
			bflag = 2;
			onind = nind - onind;
			if (onind > 0) (void) fprintf(i,"%sin +%d\n",cmdchar,onind);
			if (onind < 0) (void) fprintf(i,"%sin %d\n",cmdchar,onind);
			if (noutd > nind)
				(void) fprintf(i,"%sti +%d\n",cmdchar,noutd - nind);
			if (noutd < nind) {
				/* Check for block para. with label */
				/* Assumes no embedded tabs */
				if (nind - noutd < 3) goto notip;
				firsti += nind - noutd;
				if (firsti + 1 >= firstl) {
					/* 1st line entirely to left of 2nd */
					firstl[-1] = TAB;
					goto itsip; }
				/* Here firsti points to the spot in the first
				   nonblank line that is directly above the
				   first nonblank in the second line:
					    v
				     xx.x     first line ...
					    second line ...
				   The criterion for an indented para. is:
				   the character before the pointer must be
				   a blank; the one before that must not be
				   alphabetic. */
				if (!is_wspc(*--firsti)) goto notip;
				if (is_wspc(*--firsti)) goto itsip1;
		  		if (iswalpha (*firsti)) goto notip;
			      itsip1:
				while (is_wspc_or_wtab(*firsti))
						firsti--;
				*++firsti = TAB;
			      itsip:
				ipflag = nind;
			      notip:
				(void) fprintf(i,"%sti -%d\n",cmdchar,nind - noutd);
		     }        }
	 }
      copyit:
	*wp = L'\0';         /* copy, stripping extra blanks */
	wp = wq = buf;
	/* skip initial blanks */
	stripinit:
	if ((rflag & 2) == 0) while (is_wspc_or_wtab(*wq)) wq++;
	if (rflag) {    /* -x flag - copy, preserving internal blanks */
		while (*wq != L'\0') {*wp++ = *wq;
			    if (*wq++ == L'\n') goto stripinit; }
		goto contin; }
	for (;;) {      /* Otherwise, squeeze internal multiple blanks */
		if (*wq == L'\0') goto contin;
		*wp++ = *wq;
		if (is_wspc(wq[1]) && (*wq == L'.' || *wq == L':'))
					      {++wq; *wp++ = *wq++; continue; }
		if (is_wspc_or_wtab(*wq) || *wq++==L'\n' || 
                    wq[-1] == TAB)
		    while (is_wspc_or_wtab(*wq)) wq++;
		}
	}

endit:
 (void)fclose(i);
 if (dontflag == 0) {
	if ((j = fork()) == 0) {
		       args[0] = "nroff";
		       (void)execvp("nroff", args);
		       exit(-2);
		       }
	while (wait((int *)&i) != j);
	(void)unlink(file);
 }

 exit (0);
 return(0);
 /*NOTREACHED*/
}


/**********************************************************

CH

Returns the next wide character from the standard input.
Returns null wide character if no characters
were available.

**********************************************************/

wchar_t ch(void)
{
  /* buffer holding last read chunk of wide characters */
  static wchar_t linebuf[LBUF];
  /* pointer to next wide char to be returned */
  static wchar_t *nxtchar = linebuf;
  /* first_call is TRUE when ch is called for first time */
  static int first_call = TRUE;

  if (first_call || *nxtchar == L'\0')
       if (fgetws(linebuf, LBUF, stdin) == NULL)
            return L'\0';
       else nxtchar = linebuf;

  first_call = FALSE;

  return *nxtchar++;
}


/**********************************************************

WCS

Takes a pointer to a multibyte string and returns a pointer
to the wide char equivalent of this string. The wide char
string is stored in a buffer internal to wcs (not on the heap).
So, after the next call to wcs, the wide char string will be
gone.

**********************************************************/

wchar_t *wcs(char *mb_s)
{
  static wchar_t widechar_s[LBUF];

  (void) mbstowcs(widechar_s, mb_s, LBUF);
  return(widechar_s);
}

