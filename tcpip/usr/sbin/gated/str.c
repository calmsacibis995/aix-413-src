static char sccsid[] = "@(#)76	1.5  src/tcpip/usr/sbin/gated/str.c, tcprouting, tcpip411, GOLD410 12/6/93 18:01:31";
/*
 *   COMPONENT_NAME: tcprouting
 *
 *   FUNCTIONS: ARG
 *		PUTC
 *		__PF2
 *		__PF3
 *		__PF6
 *		__PF7
 *		fprintf
 *		sprintf
 *		strerror
 *		tochar
 *		todigit
 *		vsprintf
 *
 *   ORIGINS: 27,36
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *  str.c,v 1.18.2.2 1993/08/27 18:26:17 jch Exp
 */

/* Gated Release 3.0 */
/* Copyright (c) 1990,1991,1992,1993 by Cornell University. All rights reserved. */
/* Refer to Particulars and other Copyright notices at the end of this file. */


#define	INCLUDE_CTYPE
#define	INCLUDE_TIME

#include "include.h"
#ifdef	FLOATING_POINT
#include "math.h"
#endif	/* FLOATING_POINT */
#ifdef	PROTO_INET
#include "inet.h"
#endif	/* PROTO_INET */


#ifdef	vax11c
extern char *gd_error();

#define	strerror(str)	gd_error(str)
#endif

/*
 *	Make a copy of a string and lowercase it
 */
char *
gd_uplow __PF2(str, const char *,
	       up, int)
{
    register const char *src = str;
    register char *dst;
    size_t len = strlen(str) + 1;
    static char *buf;
    static size_t buflen;

    /* Make sure the buffer is big enough */
    if (len > buflen) {
	if (buf) {
	    task_mem_free((task *) 0, buf);
	}
	buflen = len;
	buf = (char *) task_mem_calloc((task *) 0, sizeof (char), buflen);
    }

    /* Can convert to ... */
    if (up) {
	/* upper case */
	
	for (dst = buf; *src; src++) {
	    *dst++ = islower(*src) ? toupper(*src) : *src;
	}
    } else {
	/* lower case */
	
	for (dst = buf; *src; src++) {
	    *dst++ = isupper(*src) ? tolower(*src) : *src;
	}
    }
    *dst = (char) 0;

    return buf;
}


/*
 *	Gated version of fprintf
 */
#ifdef	STDARG
/*VARARGS2*/
int
fprintf(FILE * stream, const char *format,...)
#else	/* STDARG */
/*ARGSUSED*/
/*VARARGS0*/
int
fprintf(va_alist)
va_dcl

#endif	/* STDARG */
{
    int rc;
    va_list ap;
    char buffer[BUFSIZ];

#ifdef	STDARG
    va_start(ap, format);
#else	/* STDARG */
    const char *format;
    FILE *stream;

    va_start(ap);

    stream = va_arg(ap, FILE *);
    format = va_arg(ap, const char *);
#endif	/* STDARG */
    rc = vsprintf(buffer, format, &ap);
    fputs((char *) buffer, stream);

    va_end(ap);
    return rc;
}


/*
 *	Gated version of sprintf
 */
#ifdef	STDARG
/*VARARGS2*/
int
sprintf(char *s, const char *format,...)
#else	/* STDARG */
/*ARGSUSED*/
/*VARARGS0*/
int
sprintf(va_alist)
va_dcl

#endif	/* STDARG */
{
    int rc;
    va_list ap;

#ifdef	STDARG

    va_start(ap, format);
#else	/* STDARG */
    const char *format;
    char *s;

    va_start(ap);

    s = va_arg(ap, char *);
    format = va_arg(ap, const char *);
#endif	/* STDARG */
    rc = vsprintf(s, format, &ap);

    va_end(ap);
    return rc;
}

/* 11-bit exponent (VAX G floating point) is 308 decimal digits */
#define	MAXEXP		308
/* 128 bit fraction takes up 39 decimal digits; max reasonable precision */
#define	MAXFRACT	39

#define	DEFPREC		6

#define	BUF		(MAXEXP+MAXFRACT+1)	/* + decimal point */

#define	PUTC(ch)	*dp++ = ch; cnt++;

#define	ARG() \
	_ulong = flags&LONGINT ? va_arg(*argp, long) : \
	    flags&SHORTINT ? va_arg(*argp, short) : va_arg(*argp, int);

#define	todigit(c)	((c) - '0')
#define	tochar(n)	((n) + '0')

/* have to deal with the negative buffer count kludge */
#define	NEGATIVE_COUNT_KLUDGE

#define	LONGINT		0x01		/* long integer */
#define	LONGDBL		0x02		/* long double; unimplemented */
#define	SHORTINT	0x04		/* short integer */
#define	ALT		0x08		/* alternate form */
#define	LADJUST		0x10		/* left adjustment */
#define	ZEROPAD		0x20		/* zero (as opposed to blank) pad */
#define	HEXPREFIX	0x40		/* add 0x or 0X prefix */


#ifdef	FLOATING_POINT
static char *
round __PF6(fract, double,
	    expon, int *,
	    start, register char *,
	    end, register char *,
	    ch, char,
	    signp, char *)
{
    double tmp;

    if (fract)
	(void) modf(fract * 10, &tmp);
    else
	tmp = (double) todigit(ch);
    if (tmp > 4)
	for (;; --end) {
	    if (*end == '.')
		--end;
	    if (++*end <= '9')
		break;
	    *end = '0';
	    if (end == start) {
		if (expon) {		/* e/E; increment exponent */
		    *end = '1';
		    ++*expon;
		} else {		/* f; add extra digit */
		    *--end = '1';
		    --start;
		}
		break;
	    }
	}
    /* ``"%.3f", (double)-0.0004'' gives you a negative 0. */
    else if (*signp == '-')
	for (;; --end) {
	    if (*end == '.')
		--end;
	    if (*end != '0')
		break;
	    if (end == start)
		*signp = 0;
	}
    return start;
}


static char *
exponent __PF3(p, register char *,
	       expon, register int,
	       fmtch, char)
{
    register char *t;
    char expbuf[MAXEXP];

    *p++ = fmtch;
    if (expon < 0) {
	expon = -expon;
	*p++ = '-';
    } else
	*p++ = '+';
    t = expbuf + MAXEXP;
    if (expon > 9) {
	do {
	    *--t = tochar(expon % 10);
	} while ((expon /= 10) > 9);
	*--t = tochar(expon);
	for (; t < expbuf + MAXEXP; *p++ = *t++) ;
    } else {
	*p++ = '0';
	*p++ = tochar(expon);
    }
    return p;
}


static int
cvt __PF7(number, double,
	  prec, register int,
	  flags, int,
	  signp, char *,
	  fmtch, char,
	  startp, char *,
	  endp, char *)
{
    register char *p, *t;
    register double fract;
    int dotrim, expcnt, gformat;
    double integer, tmp;

    dotrim = expcnt = gformat = 0;
    fract = modf(number, &integer);

    /* get an extra slot for rounding. */
    t = ++startp;

    /*
     * get integer portion of number; put into the end of the buffer; the
     * .01 is added for modf(356.0 / 10, &integer) returning .59999999...
     */
    for (p = endp - 1; integer; ++expcnt) {
	tmp = modf(integer / 10, &integer);
	*p-- = tochar((int) ((tmp + .01) * 10));
    }
    switch (fmtch) {
	case 'f':
	    /* reverse integer into beginning of buffer */
	    if (expcnt)
		for (; ++p < endp; *t++ = *p) ;
	    else
		*t++ = '0';
	    /*
	     * if precision required or alternate flag set, add in a
	     * decimal point.
	     */
	    if (prec || BIT_TEST(flags, ALT))
		*t++ = '.';
	    /* if requires more precision and some fraction left */
	    if (fract) {
		if (prec)
		    do {
			fract = modf(fract * 10, &tmp);
			*t++ = tochar((int) tmp);
		    } while (--prec && fract);
		if (fract)
		    startp = round(fract, (int *) NULL, startp,
				   t - 1, (char) 0, signp);
	    }
	    for (; prec--; *t++ = '0') ;
	    break;
	case 'e':
	case 'E':
	  eformat:if (expcnt) {
		*t++ = *++p;
		if (prec || BIT_TEST(flags, ALT))
		    *t++ = '.';
		/* if requires more precision and some integer left */
		for (; prec && ++p < endp; --prec)
		    *t++ = *p;
		/*
		 * if done precision and more of the integer component,
		 * round using it; adjust fract so we don't re-round
		 * later.
		 */
		if (!prec && ++p < endp) {
		    fract = 0.0;
		    startp = round((double) 0,
				   &expcnt,
				   startp,
				   t - 1,
				   *p,
				   signp);
		}
		/* adjust expcnt for digit in front of decimal */
		--expcnt;
	    }
	    /* until first fractional digit, decrement exponent */
	    else if (fract) {
		/* adjust expcnt for digit in front of decimal */
		for (expcnt = -1;; --expcnt) {
		    fract = modf(fract * 10, &tmp);
		    if (tmp)
			break;
		}
		*t++ = tochar((int) tmp);
		if (prec || BIT_TEST(flags, ALT))
		    *t++ = '.';
	    } else {
		*t++ = '0';
		if (prec || BIT_TEST(flags, ALT))
		    *t++ = '.';
	    }
	    /* if requires more precision and some fraction left */
	    if (fract) {
		if (prec)
		    do {
			fract = modf(fract * 10, &tmp);
			*t++ = tochar((int) tmp);
		    } while (--prec && fract);
		if (fract)
		    startp = round(fract, &expcnt, startp,
				   t - 1, (char) 0, signp);
	    }
	    /* if requires more precision */
	    for (; prec--; *t++ = '0') ;

	    /* unless alternate flag, trim any g/G format trailing 0's */
	    if (gformat && !BIT_TEST(flags, ALT)) {
		while (t > startp && *--t == '0') ;
		if (*t == '.')
		    --t;
		++t;
	    }
	    t = exponent(t, expcnt, fmtch);
	    break;
	case 'g':
	case 'G':
	    /* a precision of 0 is treated as a precision of 1. */
	    if (!prec)
		++prec;
	    /*
	     * ``The style used depends on the value converted; style e
	     * will be used only if the exponent resulting from the
	     * conversion is less than -4 or greater than the precision.''
	     *	-- ANSI X3J11
	     */
	    if (expcnt > prec || (!expcnt && fract && fract < .0001)) {
		/*
		 * g/G format counts "significant digits, not digits of
		 * precision; for the e/E format, this just causes an
		 * off-by-one problem, i.e. g/G considers the digit
		 * before the decimal point significant and e/E doesn't
		 * count it as precision."
		 */
		--prec;
		fmtch -= 2;		/* G->E, g->e */
		gformat = 1;
		goto eformat;
	    }
	    /*
	     * reverse integer into beginning of buffer,
	     * note, decrement precision
	     */
	    if (expcnt)
		for (; ++p < endp; *t++ = *p, --prec) ;
	    else
		*t++ = '0';
	    /*
	     * if precision required or alternate flag set, add in a
	     * decimal point.  If no digits yet, add in leading 0.
	     */
	    if (prec || BIT_TEST(flags, ALT)) {
		dotrim = 1;
		*t++ = '.';
	    } else
		dotrim = 0;
	    /* if requires more precision and some fraction left */
	    if (fract) {
		if (prec) {
		    do {
			fract = modf(fract * 10, &tmp);
			*t++ = tochar((int) tmp);
		    } while (!tmp);
		    while (--prec && fract) {
			fract = modf(fract * 10, &tmp);
			*t++ = tochar((int) tmp);
		    }
		}
		if (fract)
		    startp = round(fract, (int *) NULL, startp,
				   t - 1, (char) 0, signp);
	    }
	    /* alternate format, adds 0's for precision, else trim 0's */
	    if (BIT_TEST(flags, ALT))
		for (; prec--; *t++ = '0') ;
	    else if (dotrim) {
		while (t > startp && *--t == '0') ;
		if (*t != '.')
		    ++t;
	    }
    }
    return t - startp;
}
#endif	/* FLOATING_POINT */


int
vsprintf(dest, fmt0, argp)
char *dest;
const char *fmt0;
va_list *argp;
{
    register const char *fmt;		/* format string */
    register char *dp;			/* Destination pointer */
    register int ch;			/* character from fmt */
    register int cnt;			/* return value accumulator */
    register int n;			/* random handy integer */
    register char *t;			/* buffer pointer */
#ifdef	FLOATING_POINT
    double _double;			/* double precision arguments %[eEfgG] */
    char softsign;			/* temporary negative sign for floats */
#endif	/* FLOATING_POINT */
    u_long _ulong;			/* integer arguments %[diouxX] */
    int base = 10;			/* base for [diouxX] conversion */
    int dprec;				/* decimal precision in [diouxX] */
    int fieldsz;			/* field size expanded by sign, etc */
    int flags;				/* flags as above */
    int fpprec;				/* `extra' floating precision in [eEfgG] */
    int prec;				/* precision from format (%.3d), or -1 */
    int realsz;				/* field size expanded by decimal precision */
    int size;				/* size of converted field or string */
    int width;				/* width from format (%8d), or 0 */
    char sign;				/* sign prefix (' ', '+', '-', or \0) */
    const char *digs;			/* digits for [diouxX] conversion */
    char buf[BUF];			/* space for %c, %[diouxX], %[eEfgG] */
    int error_number = errno;

    dp = dest;
    fmt = fmt0;
    digs = "0123456789abcdef";
    for (cnt = 0;; ++fmt) {
	for (; (ch = *fmt) && ch != '%'; ++fmt) {
	    PUTC(ch);
	}
	if (!ch) {
	    PUTC(ch);
	    return --cnt;
	}
	flags = 0;
	dprec = 0;
	fpprec = 0;
	width = 0;
	prec = -1;
	sign = '\0';

      rflag:
	switch (*++fmt) {
	case ' ':
	    /*
	     * ``If the space and + flags both appear, the space
	     * flag will be ignored.''
	     *	-- ANSI X3J11
	     */
	    if (!sign)
		sign = ' ';
	    goto rflag;

	case '#':
	    BIT_SET(flags, ALT);
	    goto rflag;

	case '*':
	    /*
	     * ``A negative field width argument is taken as a
	     * - flag followed by a  positive field width.''
	     *	-- ANSI X3J11
	     * They don't exclude field widths read from args.
	     */
	    if ((width = va_arg(*argp, int)) >= 0)
		goto rflag;
	    width = -width;
	    /* FALLTHROUGH */

	case '-':
	    BIT_SET(flags, LADJUST);
	    goto rflag;

	case '+':
	    sign = '+';
	    goto rflag;

	case '.':
	    if (*++fmt == '*')
		n = va_arg(*argp, int);
	    else {
		n = 0;
		while (isascii(*fmt) && isdigit(*fmt))
		    n = 10 * n + todigit(*fmt++);
		--fmt;
	    }
	    prec = n < 0 ? -1 : n;
	    goto rflag;

	case '0':
	    /*
	     * ``Note that 0 is taken as a flag, not as the
	     * beginning of a field width.''
	     *	-- ANSI X3J11
	     */
	    BIT_SET(flags, ZEROPAD);
	    goto rflag;

	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	    n = 0;
	    do {
		n = 10 * n + todigit(*fmt);
	    } while (isascii(*++fmt) && isdigit(*fmt));
	    width = n;
	    --fmt;
	    goto rflag;

	case 'L':
	    BIT_SET(flags, LONGDBL);
	    goto rflag;

	case 'h':
	    BIT_SET(flags, SHORTINT);
	    goto rflag;

	case 'l':
	    BIT_SET(flags, LONGINT);
	    goto rflag;

	case 'c':
	    *(t = buf) = va_arg(*argp, int);
	    size = 1;
	    sign = '\0';
	    goto pforw;

	case 'D':
	    BIT_SET(flags, LONGINT);
	    /*FALLTHROUGH*/

	case 'd':
	case 'i':
	    ARG();
	    if ((long) _ulong < 0) {
		_ulong = -_ulong;
		sign = '-';
	    }
	    base = 10;
	    goto number;
	case 'e':
	case 'E':
	case 'f':
	case 'g':
	case 'G':
#ifdef	FLOATING_POINT
	    _double = va_arg(*argp, double);
	    /*
	     * don't do unrealistic precision; just pad it with
	     * zeroes later, so buffer size stays rational.
	     */
	    if (prec > MAXFRACT) {
		if ((*fmt != 'g' && *fmt != 'G') || BIT_TEST(flags, ALT))
		    fpprec = prec - MAXFRACT;
		prec = MAXFRACT;
	    } else if (prec == -1)
		prec = DEFPREC;
	    /*
	     * softsign avoids negative 0 if _double is < 0 and
	     * no significant digits will be shown
	     */
	    if (_double < 0) {
		softsign = '-';
		_double = -_double;
	    } else
		softsign = 0;
	    /*
	     * cvt may have to round up past the "start" of the
	     * buffer, i.e. ``intf("%.2f", (double)9.999);'';
	     * if the first char isn't NULL, it did.
	     */
	    *buf = (char) 0;
	    size = cvt(_double,
		       prec,
		       flags,
		       &softsign,
		       *fmt,
		       buf,
		       buf + sizeof(buf));
	    if (softsign)
		sign = '-';
	    t = *buf ? buf : buf + 1;
	    goto pforw;
#else	/* FLOATING_POINT */
	    assert(FALSE);
	    break;
#endif	/* FLOATING_POINT */

	case 'n':
	    if (BIT_TEST(flags, LONGINT))
		*va_arg(*argp, long *) = cnt;
	    else if (BIT_TEST(flags, SHORTINT))
		*va_arg(*argp, short *) = cnt;
	    else
		*va_arg(*argp, int *) = cnt;
	    break;

	case 'O':
	    BIT_SET(flags, LONGINT);
	    /*FALLTHROUGH*/

	case 'o':
	    ARG();
	    base = 8;
	    goto nosign;

	case 'p':
	    /*
	     * ``The argument shall be a pointer to void.  The
	     * value of the pointer is converted to a sequence
	     * of printable characters, in an implementation-
	     * defined manner.''
	     *	-- ANSI X3J11
	     */
	    /* NOSTRICT */
	    _ulong = (u_long) va_arg(*argp, void *);
	    base = 16;
	    goto nosign;

	case 'T':
	    /* Time */
	    {
		time_t timeval = va_arg(*argp, time_t);
		struct tm *tm;

		if (BIT_TEST(flags, ALT)) {
		    tm = gmtime(&timeval);
		} else {
#ifdef	time_boot
		    timeval += time_boot;
#endif	/* time_boot */
		    tm = localtime(&timeval);
		}
		t = buf + BUF;
		*--t = (char) 0;
		*--t = digs[tm->tm_sec % 10];
		*--t = digs[tm->tm_sec / 10];
		*--t = ':';
		*--t = digs[tm->tm_min % 10];
		*--t = digs[tm->tm_min / 10];
		*--t = ':';
		*--t = digs[tm->tm_hour % 10];
		*--t = digs[tm->tm_hour / 10];
	    }
	    goto string;

	case 'A':
	    /* socket address */
	    {
		/* XXX - How about being able to specify a different delimiter */
		char delim = '.';
		int len;
		int trailing = FALSE;
		register byte *cp, *cp1;
		register sockaddr_un *addr;

		cp = cp1 = (byte *) 0;
		addr = va_arg(*argp, sockaddr_un *);

		if (!addr) {
		    (void) strcpy(buf, "(null)");
		    t = buf;
		    goto string;
		}
		/* Get the size */
		len = socksize(addr);
		assert(len);
		t = buf + BUF;
		*--t = (char) 0;
		*buf = (char) 0;

		switch (socktype(addr)) {
		case AF_UNSPEC:
		    (void) strcpy(buf, "*Unspecified");
		    if (socksize(addr)) {
			(void) strcat(buf, ":");
			*--t = '*';

			base = 10;
			cp1 = (byte *) addr;
			cp = cp1 + len ;
		    } else {
			(void) strcat(buf, "*");
			t = (char *) 0;
		    }
		    break;

#ifdef	PROTO_UNIX
		case AF_UNIX:
		    len -= (sizeof(addr->un) - 1);
		    strncpy(buf, addr->un.sun_path, len);
		    buf[len] = (char) 0;
		    t = buf;
		    goto string;
#endif	/* PROTO_UNIX */

#ifdef	PROTO_INET
		case AF_INET:
		    base = 10;

		    if (BIT_TEST(flags, ALT)) {
			if (_ulong = ntohs(addr->in.sin_port)) {
			    do {
				*--t = digs[_ulong % base];
			    } while (_ulong /= base);
			    *--t = '+';
			}
		    }
		    cp1 = (byte *) &sock2ip(addr);

		    /* Reset length in case this address has been trimmed */
		    cp = cp1 + (len = sizeof(sock2ip(addr)));

		    if (sock2ip(addr)) {
			trailing = TRUE;	/* Skip trailing zeros */
		    }
		    break;
#endif	/* PROTO_INET */

		case AF_LL:
		    base = 16;

		    
		    strcpy(buf, trace_state(ll_type_bits, addr->ll.sll_type));
		    strcat(buf, " ");

		    cp1 = (byte *) addr->ll.sll_addr;
		    cp = cp1 + addr->ll.sll_len - ((caddr_t) cp1 - (caddr_t) addr);
		    switch (addr->ll.sll_type) {
		    case LL_SYSTEMID:
			/* Must be even length */
			assert(!((cp - cp1) & 0x01));

			/* Print two bytes at a time */
			while (cp > cp1 + 1) {
			    _ulong = *--cp;
			    *--t = digs[_ulong % base];
			    *--t = digs[_ulong / base];

			    _ulong = *--cp;
			    *--t = digs[_ulong % base];
			    *--t = digs[_ulong / base];

			    *--t = delim;
			}
			/* Skip over first delim */
			t++;
			break;

		    default:
			/* Use default printing method */
			delim = ':';
			break;
		    }
		    break;
		    
#ifdef	PROTO_ISO
		case AF_ISO:
		    base = 16;
		    trailing = FALSE;

		    /* Point at ISO address */
		    cp1 = addr->iso.siso_addr;
		    cp = (byte *) addr + len;

		    len = cp - cp1;

		    if (len > 3) {
			if (~len & 0x01) {
			    /* Length (excluding first byte) is odd, print the odd byte */

			    _ulong = *--cp;
			    *--t = digs[_ulong % base];
			    *--t = digs[_ulong / base];

			    *--t = delim;
			}
		    } else if (len < 3) {
			/* Print at least three digits so the address is recognizable */
			/* as an ISO address */

			cp = cp1 + (len = 3);
		    }

		    /* Print two bytes at a time */
		    while (cp > cp1 + 1) {
			_ulong = *--cp;
			*--t = digs[_ulong % base];
			*--t = digs[_ulong / base];

			_ulong = *--cp;
			*--t = digs[_ulong % base];
			*--t = digs[_ulong / base];

			*--t = delim;
		    }

		    /* Finally do the first byte */
		    _ulong = *--cp;
		    *--t = digs[_ulong % base];
		    *--t = digs[_ulong / base];
		    break;
#endif	/* PROTO_ISO */

#ifdef	SOCKADDR_DL
		case AF_LINK:
		    /* Format link level address as: '#<index> <name> <link_address>" */
		    /* Alternate : 'index <index> name <name> addr <link_address>" */

		    /* Assume index is always present */
		    (void) strcpy(buf, BIT_TEST(flags, ALT) ? "index " : "#");
		    cp = (byte *) t;
		    base = 10;
		    _ulong = addr->dl.sdl_index;
		    do {
			*--t = digs[_ulong % base];
		    } while (_ulong /= base);
		    (void) strcat(buf, t);
		    t = (char *) cp;

		    /* Add name if present */
		    if (addr->dl.sdl_nlen) {
			(void) strcat(buf, BIT_TEST(flags, ALT) ? " name " : " ");
			(void) strncat(buf, addr->dl.sdl_data, addr->dl.sdl_nlen);
		    }
		    /* Set up to display the link level address in HEX if present */
		    if (addr->dl.sdl_alen + addr->dl.sdl_slen) {
			(void) strcat(buf, BIT_TEST(flags, ALT) ? " addr " : " ");
			base = 16;
			cp1 = (byte *) addr->dl.sdl_data + addr->dl.sdl_nlen;
			cp = cp1 + addr->dl.sdl_alen + addr->dl.sdl_slen;
		    } else {
			cp = cp1 = (byte *) 0;
		    }
		    break;
#endif	/* SOCKADDR_DL */

#ifdef	KRT_RT_SOCK
		case AF_ROUTE:
		    (void) strcpy(buf, "*RoutingSocket*");
		    t = (char *) 0;
		    break;
			    
#endif	/* KRT_RT_SOCK */
#ifdef	notdef
		case AF_CCITT:
		    /* XXX - x25_net (DNIC, 4 nibbles) */
		    /* XXX - x25_addr (address, null terminated) */
		    break;
#endif	/* notdef */

		case AF_STRING:
		    /* Copy string into buffer */
		    strncpy(buf, addr->s.ss_string, addr->s.ss_len);
		    t = buf;
		    goto string;
		    
		default:
		    base = 16;

		    cp1 = (byte *) addr;
		    cp = cp1 + len;
		}

		if (t) {
		    if (cp > cp1) {
			if (cp > (cp1 + len)) {
			    cp = cp1 + len;
			}
			do {

			    _ulong = *--cp;
			    /* Skip trailing zeros if desired */
			    if (trailing) {
				if (_ulong) {
				    trailing = FALSE;
				} else {
				    continue;
				}
			    }
			    do {
				*--t = digs[_ulong % base];
			    } while (_ulong /= base);

			    *--t = delim;
			} while (cp > cp1);
			if (trailing) {
			    trailing = FALSE;
			    *--t = '0';
			} else {
			    t++;
			}
		    }
		    if (*buf) {
			t -= strlen(buf);
			(void) strncpy(t, buf, strlen(buf));
		    }
		} else {
		    t = buf;
		}
	    }
	    goto string;

	case 'm':
	    (void) strcpy(t = buf, (char *) strerror(error_number));
	    goto string;

	case 's':
	    t = va_arg(*argp, char *);
	    if (!t) {
		(void) strcpy(buf, "(null)");
		t = buf;
	    }

	string:
	    if (prec >= 0) {
		for (size = 0; (size < prec) && t[size]; size++) ;
	    } else
		size = strlen(t);
	    sign = '\0';
	    goto pforw;

	case 'U':
	    BIT_SET(flags, LONGINT);
	    /*FALLTHROUGH*/

	case 'u':
	    ARG();
	    base = 10;
	    goto nosign;

	case 'B':
	    digs = "_|";
	    /* FALLTHROUGH */

	case 'b':
	    ARG();
	    base = 2;
	    goto binhex;

	case 'X':
	    digs = "0123456789ABCDEF";
	    /* FALLTHROUGH */

	case 'x':
	    ARG();
	    base = 16;

	binhex:
	    /* leading 0x/X only if non-zero */
	    if (BIT_TEST(flags, ALT) && _ulong != 0)
		BIT_SET(flags, HEXPREFIX);

	    /* unsigned conversions */
	nosign:
	    sign = '\0';
	    /*
	     * ``... diouXx conversions ... if a precision is
	     * specified, the 0 flag will be ignored.''
	     *	-- ANSI X3J11
	     */
	number:
	    if ((dprec = prec) >= 0)
		BIT_RESET(flags, ZEROPAD);

	    /*
	     * ``The result of converting a zero value with an
	     * explicit precision of zero is no characters.''
	     *	-- ANSI X3J11
	     */
	    t = buf + BUF;
	    if (_ulong != 0 || prec != 0) {
		do {
		    *--t = digs[_ulong % base];
		    _ulong /= base;
		} while (_ulong);
		if (BIT_TEST(flags, ALT) && base == 8 && *t != *digs)
		    *--t = *digs;	/* octal leading 0 */
	    }	
	    size = buf + BUF - t;

	pforw:
	    /*
	     * All reasonable formats wind up here.  At this point,
	     * `t' points to a string which (if not flags&LADJUST)
	     * should be padded out to `width' places.  If
	     * flags&ZEROPAD, it should first be prefixed by any
	     * sign or other prefix; otherwise, it should be blank
	     * padded before the prefix is emitted.  After any
	     * left-hand padding and prefixing, emit zeroes
	     * required by a decimal [diouxX] precision, then print
	     * the string proper, then emit zeroes required by any
	     * leftover floating precision; finally, if LADJUST,
	     * pad with blanks.
	     */

	    /*
	     * compute actual size, so we know how much to pad
	     * fieldsz excludes decimal prec; realsz includes it
	     */
	    fieldsz = size + fpprec;
	    if (sign)
		fieldsz++;
	    if (BIT_TEST(flags, HEXPREFIX))
		fieldsz += 2;
	    realsz = dprec > fieldsz ? dprec : fieldsz;

	    /* right-adjusting blank padding */
	    if ((flags & (LADJUST | ZEROPAD)) == 0 && width)
		for (n = realsz; n < width; n++) {
		    PUTC(' ');
		}
	    /* prefix */
	    if (sign) {
		PUTC(sign);
	    }
	    if (BIT_TEST(flags, HEXPREFIX)) {
		PUTC(*digs);
		PUTC((char) *fmt);
	    }
	    /* right-adjusting zero padding */
	    if ((flags & (LADJUST | ZEROPAD)) == ZEROPAD)
		for (n = realsz; n < width; n++) {
		    PUTC(*digs);
		}
	    /* leading zeroes from decimal precision */
	    for (n = fieldsz; n < dprec; n++) {
		PUTC(*digs);
	    }

	    /* the string or number proper */
	    bcopy(t, (char *) dp, size);
	    dp += size;
	    cnt += size;
	    /* trailing f.p. zeroes */
	    while (--fpprec >= 0) {
		PUTC(*digs);
	    }
	    /* left-adjusting padding (always blank) */
	    if (BIT_TEST(flags, LADJUST))
		for (n = realsz; n < width; n++) {
		    PUTC(' ');
		}
	    digs = "0123456789abcdef";
	    break;

	case '\0':			/* "%?" prints ?, unless ? is NULL */
	    PUTC((char) *fmt);
	    return --cnt;

	default:
	    PUTC((char) *fmt);
	}
    }
    /* NOTREACHED */
}


/*
 * ------------------------------------------------------------------------
 * 
 * 	GateD, Release 3
 * 
 * 	Copyright (c) 1990,1991,1992,1993 by Cornell University
 * 	    All rights reserved.
 * 
 * 	THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY
 * 	EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT
 * 	LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * 	AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * 	Royalty-free licenses to redistribute GateD Release
 * 	3 in whole or in part may be obtained by writing to:
 * 
 * 	    GateDaemon Project
 * 	    Information Technologies/Network Resources
 * 	    200 CCC, Garden Avenue
 * 	    Cornell University
 * 	    Ithaca, NY  14853-2601  USA
 * 
 * 	GateD is based on Kirton's EGP, UC Berkeley's routing
 * 	daemon	 (routed), and DCN's HELLO routing Protocol.
 * 	Development of GateD has been supported in part by the
 * 	National Science Foundation.
 * 
 * 	Please forward bug fixes, enhancements and questions to the
 * 	gated mailing list: gated-people@gated.cornell.edu.
 * 
 * 	Authors:
 * 
 * 		Jeffrey C Honig <jch@gated.cornell.edu>
 * 		Scott W Brim <swb@gated.cornell.edu>
 * 
 * ------------------------------------------------------------------------
 * 
 *       Portions of this software may fall under the following
 *       copyrights:
 * 
 * 	Copyright (c) 1988 Regents of the University of California.
 * 	All rights reserved.
 * 
 * 	Redistribution and use in source and binary forms are
 * 	permitted provided that the above copyright notice and
 * 	this paragraph are duplicated in all such forms and that
 * 	any documentation, advertising materials, and other
 * 	materials related to such distribution and use
 * 	acknowledge that the software was developed by the
 * 	University of California, Berkeley.  The name of the
 * 	University may not be used to endorse or promote
 * 	products derived from this software without specific
 * 	prior written permission.  THIS SOFTWARE IS PROVIDED
 * 	``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES,
 * 	INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * 	MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
