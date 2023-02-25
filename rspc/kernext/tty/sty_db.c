static char sccsid[] = "@(#)97	1.5  src/rspc/kernext/tty/sty_db.c, isatty, rspc411, GOLD410 7/12/94 22:00:38";
/*
 *   COMPONENT_NAME: isatty
 *
 *   FUNCTIONS: lldb_register sty_print my_tty_dump magic_debug_stuff
 *	krash krash_add krash_del k_query k_help k_trace _db_trace
 *	db_trace_display kt_usage strcon db_snap sty_pcmds sty_scmds
 *	dump_bits dump_flags dump_data dump_hex db_d2 sty_printd k_str
 *	knget db_read_byte kgetstr find_mtype ioctl_string mblk_print
 *	qflags qbflags queue_print all_queue_print oneline_mblk all_mblk_print
 *	dbg_tbuf fix_tbuf tty_read_str
 *
 *   ORIGINS: 27 83
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <sys/types.h>
#include <sys/systemcfg.h>		/* runtime checks		*/
#include <sys/sys_resource.h>		/* key possition check		*/
#include <sys/limits.h>			/* for MAX_CANON value		*/
#include <sys/sysmacros.h>		/* minor and major macros	*/
#include <sys/intr.h>			/* interrupt handler stuff	*/
#include <sys/pin.h>			/* pinning stuff		*/
#include <sys/adspace.h>		/* io_att and io_det		*/
#include <sys/iocc.h>			/* iocc space values		*/
#include <sys/ioacc.h>			/* bus access macros		*/
#include <sys/dma.h>			/* dma stuff			*/
#include <sys/xmem.h>			/* x-memory stuff for dma	*/
#include <sys/malloc.h>			/* malloc services		*/
#include <sys/errids.h>			/* get the error id's		*/
#include <sys/errno.h>			/* error codes			*/
#include <sys/uio.h>			/* struct uio			*/
#include "ttydrv.h"			/* instead of the old sys/tty.h	*/
#include <sys/termio.h>			/* struct termios		*/
#include <sys/termiox.h>		/* struct termiox		*/
#include <unistd.h>			/* need _POSIX_VDISABLE def	*/
#include <sys/ttydefaults.h>		/* TTY defaults			*/
#include <sys/ioctl.h>			/* Ioctl declerations		*/
#include <sys/device.h>			/* for config stuff		*/
#include <sys/dbg_codes.h>		/* temp for debugger thing	*/
#include <sys/mstsave.h>		/* temp for debugger thing	*/
#include <sys/low.h>			/* temp for debugger thing	*/
#include <sys/lockl.h>			/* for lockl and unlockl	*/
#include <sys/atomic_op.h>		/* for atomic operations	*/
#include <sys/trchkid.h>		/* trace hook id's		*/
#include <sys/sysinfo.h>		/* sysinfo stuff		*/
#include <sys/signal.h>			/* SIGSAK			*/
#include <sys/pri.h>
#include <sys/sleep.h>			/* EVENT_NULL			*/
#include <sys/except.h>			/* PIO stuff			*/
#include <sys/dump.h>			/* dump structures		*/
#include <sys/str_tty.h>		 /* Streams defines		*/
#include <sys/stream.h>			/* Streams defines		*/
#include <sys/stropts.h>		/* Streams defines		*/
#include <sys/strlog.h>			/* Streams defines		*/
#include <sys/strconf.h>		/* Streams defines		*/
#include <string.h>			/* string handling function	*/
#include "sty.h"			/* dds for config		*/
#include "common_open.h"		/* Common open discipline fns	*/

#if ttyDBG
static void (*saved_dump_ptr)();
static void my_tty_dump(char *);
static was_nl;					/* last was newline	*/
static db_hz = 100;				/* ticks / second	*/


#ifndef FOR_CRASH
/*
 * lldb_register -	add hooks to tty debugger kernel extension.
 *
 * Input:
 *	add	-	flag: !0 ==> add, otherwise delete
 *	name	-	module name
 *	func	-	^ to function to display data
 */
lldb_register(int add, char* name, void (*func)()) {
	struct str_module_conf regis;
	extern void (*db_tty_dump_ptr)();

	strncpy(regis.name, name, sizeof (regis.name));	/* module name	*/
	regis.type = 'd';				/* driver	*/
	regis.print_funcptr = func;			/* print func	*/

	if (add)
		tty_db_register(&regis);
	else
		tty_db_unregister(&regis);

	if (!saved_dump_ptr) {
		saved_dump_ptr = db_tty_dump_ptr;
		db_tty_dump_ptr = my_tty_dump;
	}
}
#endif

#undef disp
#define disp(var, elem, n)	db_d2("elem", var##elem, n);
#define d8(elem)		disp(tp->t_, elem, 8);

/*
 * sty_print -	print common "sty_tty" structure
 */
sty_print(register struct sty_tty *tp, int verb) {

	db_d2("name", tp->t_name, 's');

	d8(dev);	d8(ioctl);	d8(close);	d8(ctl);
	d8(service);	d8(proc);	d8(rsrv);	d8(wsrv);

#undef queue	/* streams gorp	*/
	d8(queue);	d8(outmsg);	d8(inmsg);	d8(hptr);
	d8(event);	d8(lock);	d8(timeout);	d8(alloctimer);
	d8(draining);	d8(bufcall);	d8(hupcl);	d8(excl);
	d8(cflag);	d8(softpacing); d8(hardpacing);	d8(ospeed);
	d8(ispeed);	d8(stopbits);	d8(parity);	d8(csize);
	d8(wopen);	d8(carrier);	d8(clocal);	d8(isopen);
	d8(busy);	d8(stop);	d8(block);	d8(ctlx);
	d8(sak);	d8(localstop);	d8(cread);	d8(open_disc);
	d8(openRetrieve);
	d8(flow_disc);	d8(xon_pending);
	d8(xoff_pending); d8(modem_chg);
	d8(rptr); d8(wptr);

	db_d2(0,0);

	printd("\n");

	if (verb && ttyDBG)
		dump_trace(tp->t_tbuf);
}

#if defined(KOFF) || !defined(TTYDBG) || defined(FOR_CRASH)
static void my_tty_dump(char *cmd_line) {;}
#else	/* !KOFF && TTYDBG && !FOR_CRASH */
/*
 * they could have chosen to make this much easier for me.....
 */

label_t sty_top;
static  sty_top_set;

/*
 * my_tty_dump -	the tty debug kernel extension is not quite
 *			sufficient for our porpoises...
 */
static void
my_tty_dump(char *cmd_line) {
	register char *p;

	/*
	 * skip first token
	 */
	for (p = cmd_line; *p && *p != ' ' && *p != '\t'; ++p)
		;

	/*
	 * advance to next
	 */
	while (*p && (*p == ' ' || *p == '\t'))
		++p;

	if (setjmpx(&sty_top)) {
		sty_top_set = 0;
		return;
	}

	sty_top_set = 1;

	/*
	 * check for magic debug command
	 */
	if (*p == ':')
		magic_debug_stuff(p+1);
	else
		if (saved_dump_ptr)
			(*saved_dump_ptr)(cmd_line);

	sty_top_set = 0;
	clrjmpx(&sty_top);
}

/*
 * magic_debug_stuff -	perform magic debug stuff
 */
magic_debug_stuff(p)
register char *p; {

	krash(p);
};

/*
 * extractions from KOFF ":k" mode functions
 */
static k_query(), k_help(), k_trace();

struct krash_func krash_func[] = {
	{
		k_query,
		"?",
		"display krash mode function table",
		"\
?\n\
Display a list of `krash' mode functions.\n",
	}, {
		k_help,
		"h",
		"provide help for krash mode functions",
		"\
help func func*\n\
Print help string for the indicated `krash' mode functions.\n",
	}, {
		k_trace,
		"t",
		"display debug/trace buffer",
		"\
trace [-f] [-t<nnn>] [-s<nnn>] [-S<nnn>]\n\
-f	flush trace table\n\
-t<nnn>	show only the last <nnn> entries\n\
-S<nnn> show only slot <nnn> entries\n\
-s<nnn>	set time reporting scale to <nnn> (default 10)\n"
	},
	{ 0, }, { 0, }, { 0, }, { 0, }, { 0, }, { 0, }, { 0, }, { 0, },
	{ 0, }, { 0, }, { 0, }, { 0, }, { 0, }, { 0, }, { 0, }, { 0, },
	{ 0, }, { 0, }, { 0, }, { 0, }, { 0, }, { 0, }, { 0, }, { 0, },
	{ 0, }, { 0, }, { 0, }, { 0, }, { 0, }, { 0, }, { 0, }, { 0, },
	{ 0, }, { 0, }, { 0, }, { 0, }, { 0, }, { 0, }, { 0, }, { 0, },
};

/*
 * krash	-	crash(8) type command interface for debugger
 *
 * Description:
 *	We parse into an argv[]-type array.  Argv[0] is looked up in the
 *	krash_func[] table, and the matching function is invoked.
 */
krash(buf)
char *buf; {
	char *p;
	register argc, i, len;
	static char *argv[32];
	register struct krash_func *kp, *ep;

	ep  = &krash_func[sizeof (krash_func) / sizeof (krash_func[0])];
	p   = buf;
	len = strlen(buf);

	for (argc = 0, p = buf; argc < 31 && *p && p < buf + len; ) {
		if (*p == ' ') {
			++p;
			continue;
		}

		argv[argc++] = p;

		do {
			switch (*p) {
			    case ' ':
			    case '\t':
				*p++ = 0;
				break;

			    case 0:
			    case '\n':
			    case '\r':
				*p = 0;
				break;

			    default:
				++p;
				continue;
			}
			break;
		} while (*p && p < buf + len);
	}


	for (kp = krash_func; kp < ep; ++kp)
		if (kp->name && strcmp(argv[0], kp->name) == 0) {
			(*kp->func)(argc, argv);
			return;
		}

	printd("eh?\n");
}

/*
 * k_query -	print the krash-mode function table
 */
static
k_query() {
	register struct krash_func *kp, *ep;

	ep = &krash_func[sizeof (krash_func) / sizeof (krash_func[0])];

	printd("\nThe following functions are available in krash mode:\n");

	for (kp = krash_func; kp < ep; ++kp)
		if (kp->name)
			printd("%8s %s\n", kp->name, kp->synopsis);
}

/*
 * k_help -	provide help for krash mode functions
 */
static
k_help(c, v)
register char **v; {
	register i, found;
	register struct krash_func *kp, *ep;

	ep = &krash_func[sizeof (krash_func) / sizeof (krash_func[0])];

	for (i = 1; i < c; ++i) {
		found = 0;

		for (kp = krash_func; kp < ep; ++kp)
			if (kp->name && strcmp(v[i], kp->name) == 0) {
				found = 1;
				printd("\n%s", kp->help);
				break;
			}

		if (!found)
			printd("\n`%s' is not a krash mode function", v[i]);
	}
}

/*
 * krash_add -	add entries to the debugger `krash(8)' mode function table
 *
 * Input:
 *	tp	-	^ to new table entry
 *
 * Returns:
 *	!0	-	 OK - entry added
 *	 0	-	!OK - entry not added
 */
krash_add(tp)
register struct krash_func *tp; {
	register struct krash_func *kp, *ep;;

	ep = &krash_func[sizeof (krash_func) / sizeof (krash_func[0])];

	for (kp = krash_func; kp < ep; ++kp)
		if (!kp->name) {
			*kp = *tp;
			return !0;
		}

	return 0;
}

/*
 * krash_del -	delete entries
 *
 * Input:
 *	tp	-	^ to old table entry
 */
krash_del(tp)
register struct krash_func *tp; {
	register struct krash_func *kp, *ep;

	ep = &krash_func[sizeof (krash_func) / sizeof (krash_func[0])];

	for (kp = krash_func; kp < ep; ++kp)
		if (kp->name && kp->name == tp->name)
			kp->name = 0;
}

/*
 * db_trace -	debugger trace facility
 *
 * This facility was borrowed from KOFF.  It is being left in the code due
 * to the fact that it has been quite useful during development and test.
 * For debugging problems in the field, "special" versions of the drivers
 * can be build with -DDEBUG to collect the traces.
 */
struct trace db_tbuf[TRACE_SIZE];	/* trace buffer		*/
int db_tbuf_sz = sizeof (db_tbuf);	/* for user programmes	*/
static struct trace *slot = db_tbuf;	/* current trace slot	*/
long db_tflags = -1;			/* trace flags		*/
int db_tprint = 0;			/* when desperate, patch*/

/*
 * dbg_tbuf -	return pointer to trace buffer
 *
 * Note:
 *	This allow us to provide the pointer, while keeping the type
 *	declaration out of the main, non-debuf portion of the code.
 */
void *
(dbg_tbuf)() {

	return &db_tbuf[0];
}

/*
 * _db_trace -	add trace entries to buffer
 */
_db_trace(id, fmt, a0, a1, a2, a3)
void *id;
char *fmt; {
	register struct trace *tp;
	register x;

	x = disable();

	if (fmt == 0) {
		/*
		 * special entry to clear trace buffer
		 */
		bzero(&db_tbuf[0], sizeof (db_tbuf));
		slot = db_tbuf;
		enable(x);
		return;
	}

	tp = slot;

	tp->ticks = tod;
	tp->pslot = PROCMASK(getpid());
	tp->id    = id;
	tp->fmt   = fmt;
	tp->a0    = a0;
	tp->a1    = a1;
	tp->a2    = a2;
	tp->a3    = a3;

	if (db_tprint)
		db_trace_display(tp);

	if (++tp >= &db_tbuf[TRACE_SIZE])
		tp = db_tbuf;

	tp->fmt = 0;		/* zero format ==> end of trace entries	*/

	slot = tp;

	enable(x);
}

static
k_trace(c, v)
char **v; {
	register i;
	register struct trace *tp;
	register slot, num_slots, mark_slot, skip, tail = 0, only_slot = -1;
	int size;

	for (i = 1; i < c; ++i)
		if (**++v == '-') {
			switch (*++*v) {
			    case 's':	/* scale	*/
				db_hz = strcon(++*v, 'd');
				if (db_hz <= 0)
					db_hz = 10;
				printd("\nscale now %d ticks/second", db_hz);
				break;

			    case 'f':	/* flush	*/
				_db_trace(0, 0);
				return;

			    case 'S':	/* Slot	*/
				only_slot = strcon(++*v, 'h');
				break;

			    case 't':	/* tail	*/
				tail = strcon(++*v, 'd');
				break;

			    default:
				kt_usage();
			}
		} else
			kt_usage();

	/*
	 * find next slot
	 */
	num_slots = db_tbuf_sz / sizeof (struct trace);
	skip      = (tail ? num_slots - tail : 0);

	for (tp = db_tbuf, slot = 0; slot < num_slots; ++slot, ++tp)
		if (tp->fmt == 0)
			break;

	was_nl = 0;

	for (mark_slot = slot; ++slot != mark_slot; ) {
		if (slot >= num_slots) {
			slot = -1;
			continue;
		}

		if (skip > 0 && --skip)
			continue;

		tp = &db_tbuf[slot];

		if (tp->fmt == 0)
			continue;

		if (only_slot != -1 && tp->pslot != only_slot)
			continue;

		db_trace_display(tp);
	}
}

static
kt_usage() {

	printd("usage: trace [-f] [-t<nnn>] [-s<nnn>]\n");
}

/*
 * strcon -	string conversion
 */
strcon(p, fmt)
register char *p; {
	register long val = 0;
	register c, num, base;

	base = 0;

	if (*p == '0')
		switch (*++p) {
		    case 'x':
		    case 'h':
			++p;
			base = 0x10;
			break;

		    case 'o':
			++p;
			base = 010;
			break;
		}

	if (!base)
		switch (fmt) {
		    case 'h':
		    case 'x':
			base = 0x10;
			break;

		    case 'd':
			base = 10;
			break;

		    case 'o':
			base = 010;
			break;
		}

	while (c = *p++) {
		switch (num = c) {
		    case '0': case '1': case '2': case '3': case '4':
		    case '5': case '6': case '7': case '8': case '9':
			num -= '0';
			break;

		    case 'a': case 'b': case 'c':
		    case 'd': case 'e': case 'f':
			num -= 'a' - 10;
			break;

		    case 'A': case 'B': case 'C':
		    case 'D': case 'E': case 'F':
			num -= 'A' - 10;
			break;

		    default:
			num = base + 1;
			break;
		}

		if (num >= base) {
			printd("funny digit '%c' for base %d number\n"
			       , c, base);
			return 0;
		}

		val *= base;
		val += num;
	}

	return val;
}

#endif	/* !KOFF && TTYDBG && !FOR_CRASH */

/*
 * dump_trace -	dump trace buffer
 *
 * Input:
 *	tbuf	-	^ to start of trace buffer
 *
 * Implied:
 *	trace buffer size is constant
 */
dump_trace(struct trace *tbuf) {

	if (for_CRASH && tbuf) {
		struct trace db_tbuf[TRACE_SIZE];
		register struct trace *tp;
		register slot, mark_slot;

		tty_read_mem(tbuf, db_tbuf, sizeof (db_tbuf));

		/*
		 * find next slot
		 */
		for (tp = db_tbuf, slot = 0; slot < TRACE_SIZE; ++slot, ++tp)
			if (tp->fmt == 0)
				break;

		was_nl = 0;

		for (mark_slot = slot; ++slot != mark_slot; ) {
			if (slot >= TRACE_SIZE) {
				slot = -1;
				continue;
			}

			tp = &db_tbuf[slot];

			if (tp->fmt == 0)
				continue;

			db_trace_display(tp);
		}
	}
}

/*
 * db_trace_display -	display debugger trace buffer
 *
 * Input:
 *	tp	-	^ to trace slot
 *
 * Note:
 *	This is supposed to work both from within the kernel, and from
 *	within crash.  I doubt it will work using crash on a dump file...
 */
db_trace_display(tp)
register struct trace *tp; {
	register char *p;
	register unsigned long ticks;

	if (for_CRASH) {
		static char fmt[256];

		/*
		 * gain addressability to tp->fmt
		 */
		tty_read_str(tp->fmt, fmt, sizeof (fmt));
		tp->fmt = fmt;

		/*
		 * for any '%s' in fmt, make sure we have addressabliity
		 */
		fix_tbuf(tp);
	}

	ticks = tp->ticks.tv_sec * db_hz
		+ (tp->ticks.tv_nsec * db_hz) / NS_PER_SEC;

	printf("%s%4x:%2x:%8x "
	       , was_nl ? "" : "\n"
	       , ticks & 0xffff, tp->pslot & 0xff, tp->id);

	/*
	 * try to make it look pretty...
	 */
	for (p = tp->fmt; *p && *p != '%'; ++p)
		if (*p == ':')
			break;

	if (p[0] == ':') {
		p[0] = 0;
		printf("%10s:", tp->fmt);
		printf(&p[1], tp->a0, tp->a1, tp->a2, tp->a3);
		p[0] = ':';
	} else
		printf(tp->fmt, tp->a0, tp->a1, tp->a2, tp->a3);

	/*
	 * check to see if some turkey is putting "\n"'s in the format
	 * strings.
	 */
	for (p = tp->fmt; *p; ++p)
		;

	was_nl = (p[-1] == '\n');
}

#ifdef FOR_CRASH
/*
 * fix_tbuf -	fix addressability in trace buffer slot where required
 *
 * Input:
 *	tp	-	^ to strace buffer slot
 *
 * Description:
 *	Whenever tp->fmt has a "%s" or a "%w.ps" in it, the corresponding
 *	string valued argument must have its addressability guaranteed if
 *	we are to pass the thing to printf.
 */
fix_tbuf(tp)
struct trace *tp; {
	register char *p;
	register long *ap = &tp->a0;
	register i;
	static char strbuf[4][512];

	for (p = tp->fmt, i = 0; *p && i < 4; ) {
		switch (*p++) {
		    case '%':
			for (; *p;) {
				switch (*p++) {
				    case '0': case '1': case '2': case '3':
				    case '4': case '5': case '6': case '7':
				    case '8': case '9': case '.': case '-':
					continue;

				    case 's':
					tty_read_str(*ap, strbuf[i], 512);
					*ap = (void*) strbuf[i];
					break;

				    default:
					break;
				}
				break;
			}
			++ap;
			break;

		    default:
			break;
		}
	}
}

/*
 * tty_read_str -	obtain addressability to a string
 */
tty_read_str(from, to, max)
char *from, *to; {
	register i;

	to[--max] = 0;

	/*
	 * XXX: perhaps if we were smart, or daring, we would just fill the
	 * buffer.  However, what are all those mips for if noone uses them?
	 */
	for (i = 0; i < max; ++i) {
		tty_read_mem(from, to, 1);

		if (!*to)
			break;

		++from, ++to;
	}

}
#endif
#ifndef FOR_CRASH
/*
 * debugger assist routines exported by the Streams/TTY module
 */
/*
 * sty_pcmds -	return ASCII name of a proc_commands
 */
char *
sty_pcmds(enum proc_commands cmd) {
	register char *p;

	switch (cmd) {
	    case T_WFLUSH:	p = "T_WFLUSH";		break;
	    case T_RESUME:	p = "T_RESUME";		break;
	    case T_OUTPUT:	p = "T_OUTPUT";		break;
	    case T_SUSPEND:	p = "T_SUSPEND";	break;
	    case T_RFLUSH:	p = "T_RFLUSH";		break;
	    case T_BLOCK:	p = "T_BLOCK";		break;
	    case T_UNBLOCK:	p = "T_UNBLOCK";	break;
	    default:		p = "T_**gok";		break;
	}

	return p;
}

char *
sty_scmds(enum service_commands cmd) {
	register char *p;

	switch (cmd) {
	    case TS_SCONTROL:	p = "TS_SCONTROL";	break;
	    case TS_GCONTROL:	p = "TS_GCONTROL";	break;
	    case TS_GSTATUS:	p = "TS_GSTATUS";	break;
	    case TS_GBAUD:	p = "TS_GBAUD";		break;
	    case TS_SBAUD:	p = "TS_SBAUD";		break;
	    case TS_SBPC:	p = "TS_SBPC";		break;
	    case TS_SPARITY:	p = "TS_SPARITY";	break;
	    case TS_SSTOPS:	p = "TS_SSTOPS";	break;
	    case TS_SBREAK:	p = "TS_SBREAK";	break;
	    case TS_CBREAK:	p = "TS_CBREAK";	break;
	    case TS_PRIOUTPUT:	p = "TS_PRIOUTPUT";	break;
	    case TS_LOOP:	p = "TS_LOOP";		break;
	    case TS_PROC:	p = "TS_PROC";		break;
	    default:		p = "TS_**gok";		break;
	}

	return p;
}
#endif

/*
 * these routines are normally found in KOFF...
 */


/*
 * dump_bits -	display string description for a bit mask
 *
 * Input:
 *	bits	-	bit mask
 *	desc	-	array of char descriptions
 *
 * Needless to Say:
 *	there should be no more bits on in `bits' than there are descriptions
 */
dump_bits(ulong bits, char **desc) {
	register ulong bit;

	for (bit = 1; bit; bit <<= 1, ++desc)
		if (bits & bit)
			printd(" %s", *desc ? *desc : "**null");
}

/*
 * dump_flags -	dump some flags
 *
 * Input:
 *	flags	-	flag bit mask
 *	fp	-	flag description structure
 *
 * Contrast this with `dump_bits'.
 */
dump_flags(register ulong flags, register struct db_flags *fp) {

	for (; fp->bitmask; ++fp)
		if (fp->bitmask & flags)
			printd(" %s", fp->what);
}

/*
 * dump_data -	dump some data
 *
 * Input:
 *	addr	-	address to print
 *	data	-	^ to data
 *	len	-	# bytes
 */
dump_data(addr, p, len)
register unsigned long addr;
register unsigned char *p; {
	register i, j, res;

	for (i = 0; i < len; i += 16) {
		printd("%8x: ", addr);
		addr += 0x10;

		res = (len - i) > 16 ? 16 : (len - i);

		for (j = 0; j < res; ++j) {
			printd("%02x", p[j]);
			if (((j+1) % 4) == 0)
				printd(" ");
		}

		while (j < 16) {
			printd("  ");
			if (((j+1) % 4) == 0)
				printd(" ");
			++j;
		}

		for (j = 0; j < res; ++j)
			if (p[j] < ' ' || p[j] > '~')
				printd(".");
			else
				printd("%c", p[j]);

		printd("\n");
		p += 16;
	}
}

/*
 * dump_hex	-	dump 32 bit HEX
 *
 * Input:
 *	addr	-	address to print
 *	data	-	^ to data
 *	len	-	# bytes
 */
dump_hex(addr, p, len)
register unsigned long addr;
register unsigned char *p; {
	register i, j, res;

	for (i = 0; i < len; i += 32) {
		printd("%8x: ", addr);
		addr += 0x20;

		res = (len - i) > 32 ? 32 : (len - i);

		for (j = 0; j < res; ++j) {
			printd("%02x", p[j]);
			if (((j+1) % 4) == 0)
				printd(" ");
		}
		printd("\n");
		p += 32;
	}
}

#ifndef KOFF
/*
 * db_d2 -	display on two lines at once!
 *
 * Input:
 *	hdr	-	what to print on first line
 *	val	-	value to print on second line
 *	prec	-	precision of `val', or 's' for string val.
 *
 * The LousyLosingDeBugger's printf doesn't support a full printf, so
 * "precision" is ignored, and everything is forced to width 8.
 */
db_d2(hdr, val, prec)
char *hdr; {
	static char oline[2][80+1];
	static char *b0 = &oline[0][0], *b1 = &oline[1][0];

	if (!hdr) {
		if (oline[0][0]) {
			*b0 = *b1 = 0;
			printd("\n%s\n%s", oline[0], oline[1]);
		}

		*(b0 = &oline[0][0]) = 0;
		*(b1 = &oline[1][0]) = 0;

		return;
	}

	if (b0 != &oline[0][0]) {
		*b0++ = ' ';
		*b1++ = ' ';
	}

	sprintf(b0, "%8s", hdr);

	if (prec == 's')
		sprintf(b1, "%8s", val);		/* kluge. sigh.	*/
	else
		sprintf(b1, "%8x", val);

	b0 += 8;
	b1 += 8;

	if (b0 >= &oline[0][8 + (7*9)])
		db_d2(0);
}
#endif

#ifndef FOR_CRASH
/*
 * sty_printd -	"sty" debugger printf, with pagination
 *
 * *sigh*  why does't this world class LLDB kernel debugger handle this?
 */
sty_printd(fmt, a0, a1, a2, a3, a4, a5, a6, a7)
char *fmt; {
	char	*p;
	int i;
	static nlines;

	if (fmt == 0) {
		/*
		 * reset pagination counter
		 */
		nlines = 0;
		return;
	}

	(printf)(fmt, a0, a1, a2, a3, a4, a5, a6, a7);

	if (!sty_top_set)
		return;

	p = fmt;
	do {
		if (*p != '\n')
			continue;
		if ((++nlines % 24) == 0) {
			if (fmt[strlen(fmt)-1] != '\n')
				(printf)("\n");
			if (debpg() == FALSE) {
				sty_top_set = 0;
				longjmpx(&sty_top, 1);
			}
		}
	} while (*p++);
}
#endif
#endif	/* ttyDBG	*/

#if !defined(DEBUG) || defined(FOR_CRASH)
k_str() {;}
#else /* DEBUG && !FOR_CRASH */
/*
 * These routines are added either to KOFF, or to the magic_debug_stuff
 * mode in LLDB when DEBUG is enabled.  They permit me to display streams
 * queues, without having to spend half an hour wrestling with a
 * recalcitrant LLDB.
 * Most of the display routines were lifted from the BULL KDB code from
 * PSE.  Can you say "Architecture"?  Can you say "Cohesion"?  I knew
 * you could!
 */
k_str();
struct krash_func kstr = {
	k_str,
	"str",
	"print info on STREAMS",
	"str -[mMqQi?] <addr>\n\
-m <addr>	print mblk\n\
-M <addr>	print Mblk\n\
-q <addr>	print q\n\
-Q <addr>	print Q\n\
-i <addr>	print as ioctl"
};

/*
 * STREAMS debug assists
 *
 * snarfed from kernext/pse/str_kdb.c
 */
#define kdbprintf	printd
#define kgetstruct	db_snap

/*
 * db_snap -	snap memory
 */
db_snap(from, to, cc)
register char *from, *to; {

	get_from_memory(from, 1, to, cc);
}

/*
 * k_str -	interface KOFF style ":k" interface with KDB
 */
k_str(c, v)
char **v; {
	int i;
	ulong addr;
	static const char *ioctl_string(long ioc);
	static void mblk_print(mblk_t *m);
	static void all_mblk_print(queue_t *q);
	static void queue_print(queue_t	*qp);
	static void all_queue_print(queue_t *qp);
	static once;

	if (!once) {
		krash_add(&kstr);
		++once;
		return;
	}

	printd("\n");

	for (i = 1; i < c; i += 2, ++v) {
		switch (**++v) {
		    case '-':
			if (i + 1 >= c) {
				printd("ni!\n");
				return;
			}

			addr = strcon(v[1], 'h');

			switch (v[0][1]) {
			    case 'm':
				mblk_print((mblk_t *)addr);
				break;

			    case 'q':
				queue_print((queue_t *)addr);
				break;

			    case 'Q':
				all_queue_print((queue_t *) addr);
				break;

			    case 'M':
				all_mblk_print((queue_t *) addr);
				break;

			    case 'i':
				printd("\n%8X=\"%s\""
				       , addr, ioctl_string(addr));
				break;
			}
		}
	}
}

static ulong
knget(addr) {
	ulong zot;

	db_snap(addr, &zot, sizeof (zot));

	return zot;
}

static uchar
db_read_byte(addr) {
	uchar zot;

	db_snap(addr, &zot, sizeof (zot));

	return zot;
}

static char *
kgetstr(addr)
{
	static char string[128];
	int count;

	for (count = 0; count < sizeof(string) - 1; count += sizeof(char),
			addr += sizeof(char)) {
			string[count] = db_read_byte(addr);
	}
	string[count] = '\0';

	return(string);
}

char *
find_mtype(mtype) {
	char *mname = 0;

	switch (mtype) {
		case M_DATA: mname = "M_DATA"; break;
		case M_PROTO: mname = "M_PROTO"; break;
		case M_BREAK: mname = "M_BREAK"; break;
		case M_PASSFP: mname = "M_PASSFP"; break;
		case M_SIG: mname = "M_SIG"; break;
		case M_DELAY: mname = "M_DELAY"; break;
		case M_CTL: mname = "M_CTL"; break;
		case M_IOCTL: mname = "M_IOCTL"; break;
		case M_SETOPTS: mname = "M_SETOPTS"; break;
		case M_RSE: mname = "M_RSE"; break;
		case M_IOCACK: mname = "M_IOCACK"; break;
		case M_IOCNAK: mname = "M_IOCNAK"; break;
		case M_PCPROTO: mname = "M_PCPROTO"; break;
		case M_PCSIG: mname = "M_PCSIG"; break;
		case M_FLUSH: mname = "M_FLUSH"; break;
		case M_STOP: mname = "M_STOP"; break;
		case M_START: mname = "M_START"; break;
		case M_HANGUP: mname = "M_HANGUP"; break;
		case M_ERROR: mname = "M_ERROR"; break;
		case M_READ: mname = "M_READ"; break;
		case M_HPDATA: mname = "M_HPDATA"; break;
		case M_COPYIN: mname = "M_COPYIN"; break;
		case M_COPYOUT: mname = "M_COPYOUT"; break;
		case M_IOCDATA: mname = "M_IOCDATA"; break;
		case M_PCRSE: mname = "M_PCRSE"; break;
		case M_STOPI: mname = "M_STOPI"; break;
		case M_STARTI: mname = "M_STARTI"; break;
		case M_NOTIFY: mname = "M_NOTIFY"; break;
	}
	return (mname);
}

const static struct {
	long	code; 
	char	*name;
} Tioc_namelist[] = {
	TIOCMODG ,"TIOCMODG",
	TIOCMODS ,"TIOCMODS",
	TIOCM_LE ,"TIOCM_LE",
	TIOCM_DTR ,"TIOCM_DTR",
	TIOCM_RTS ,"TIOCM_RTS",
	TIOCM_ST ,"TIOCM_ST",
	TIOCM_SR ,"TIOCM_SR",
	TIOCM_CTS ,"TIOCM_CTS",
	TIOCM_CAR ,"TIOCM_CAR",
	TIOCM_CD ,"TIOCM_CD",
	TIOCM_RNG ,"TIOCM_RNG",
	TIOCM_RI ,"TIOCM_RI",
	TIOCM_DSR ,"TIOCM_DSR",
	TIOCEXCL ,"TIOCEXCL",
	TIOCNXCL ,"TIOCNXCL",
	TIOCFLUSH ,"TIOCFLUSH",
	TCGETS,"TCGETS",
	TCSETS,"TCSETS",
	TCSETSF,"TCSETSF",
	TCSETSW,"TCSETSW",
	TCGETA ,"TCGETA",
	TCSETA ,"TCSETA",
	TCSETAW ,"TCSETAW",
	TCSETAF ,"TCSETAF",
	TCSBREAK ,"TCSBREAK",
	TCSBRK ,"TCSBRK",
	TCXONC ,"TCXONC",
	TCFLSH ,"TCFLSH",
	TIOCGETD ,"TIOCGETD",
	TIOCSETD ,"TIOCSETD",
	TIOCSBRK ,"TIOCSBRK",
	TIOCCBRK ,"TIOCCBRK",
	TIOCSDTR ,"TIOCSDTR",
	TIOCCDTR ,"TIOCCDTR",
	TIOCGPGRP ,"TIOCGPGRP",
	TIOCSPGRP ,"TIOCSPGRP",
	TIOCOUTQ ,"TIOCOUTQ",
	TIOCSTI ,"TIOCSTI",
	TIOCNOTTY ,"TIOCNOTTY",
	TIOCPKT ,"TIOCPKT",
	TIOCPKT_DATA ,"TIOCPKT_DATA",
	TIOCPKT_FLUSHREAD ,"TIOCPKT_FLUSHREAD",
	TIOCPKT_FLUSHWRITE ,"TIOCPKT_FLUSHWRITE",
	TIOCPKT_STOP ,"TIOCPKT_STOP",
	TIOCPKT_START ,"TIOCPKT_START",
	TIOCPKT_NOSTOP ,"TIOCPKT_NOSTOP",
	TIOCPKT_DOSTOP ,"TIOCPKT_DOSTOP",
	TIOCSTOP ,"TIOCSTOP",
	TIOCSTART ,"TIOCSTART",
	TIOCMSET ,"TIOCMSET",
	TIOCMBIS ,"TIOCMBIS",
	TIOCMBIC ,"TIOCMBIC",
	TIOCMGET ,"TIOCMGET",
	TIOCREMOTE ,"TIOCREMOTE",
	TIOCGWINSZ ,"TIOCGWINSZ",
	TIOCSWINSZ ,"TIOCSWINSZ",
	TIOCUCNTL ,"TIOCUCNTL",
	TIOCCONS ,"TIOCCONS",
/* COMPAT_BSD_4.3 */
	TIOCHPCL, "TIOCHPCL",
	TIOCGETP, "TIOCGETP",
	TIOCSETP, "TIOCSETP",
	TIOCSETN, "TIOCSETN",
	TIOCSETC, "TIOCSETC",
	TIOCGETC, "TIOCGETC",
	TIOCSLTC, "TIOCSLTC",
	TIOCGLTC, "TIOCGLTC",
	TIOCLBIS, "TIOCLBIS",
	TIOCLBIC, "TIOCLBIC",
	TIOCLSET, "TIOCLSET",
	TIOCLGET, "TIOCLGET",
/* COMPAT_BSD_4.3 */
	I_NREAD, "I_NREAD",
	I_PUSH, "I_PUSH",
	I_POP, "I_POP",
	I_LOOK, "I_LOOK",
	I_FLUSH, "I_FLUSH",
	I_SRDOPT, "I_SRDOPT",
	I_GRDOPT, "I_GRDOPT",
	I_STR, "I_STR",
	I_SETSIG, "I_SETSIG",
	I_GETSIG, "I_GETSIG",
	I_FIND, "I_FIND",
	I_LINK, "I_LINK",
	I_UNLINK, "I_UNLINK",
	I_PEEK, "I_PEEK",
	I_FDINSERT, "I_FDINSERT",
	I_SENDFD, "I_SENDFD",
	I_RECVFD, "I_RECVFD",
	I_FLUSHBAND, "I_FLUSHBAND",
	I_SWROPT, "I_SWROPT",
	I_GWROPT, "I_GWROPT",
	I_LIST, "I_LIST",
	I_ATMARK, "I_ATMARK",
	I_CKBAND, "I_CKBAND",
	I_GETBAND, "I_GETBAND",
	I_CANPUT, "I_CANPUT",
	I_SETCLTIME, "I_SETCLTIME",
	I_GETCLTIME, "I_GETCLTIME",
	I_PLINK, "I_PLINK",
	I_PUNLINK, "I_PUNLINK",
	I_GETMSG, "I_GETMSG",
	I_PUTMSG, "I_PUTMSG",
	I_GETPMSG, "I_GETPMSG",
	I_PUTPMSG, "I_PUTPMSG",
	I_PIPE, "I_PIPE",
	I_FIFO, "I_FIFO",
	0, (char *) 0};

const char *
ioctl_string(ioc)
long ioc; {
	static char buf[12];
	int	i;

	for (i = 0; Tioc_namelist[i].name != (char *) 0; i++)
		if (Tioc_namelist[i].code == ioc)
			return(Tioc_namelist[i].name);

	/*
	 * not found, return printable hex value
	 */
	sprintf(buf, "0x%x", ioc);
	return buf;
}



/*
 * STREAMS data block print
 */
static void
mblk_print (m)
	mblk_t	* m;
{
	char * type;
	mblk_t storem;
	mblk_t * mp = &storem;
	dblk_t	stored;
	dblk_t	* dp = &stored;

	if (!m)
		return;
	kdbprintf("mblk (%8X): ", m);
	while (m) {
		kgetstruct(m, &storem, sizeof(mblk_t));	
		kdbprintf("b_next %8X b_prev %8X b_cont %8X\n",
			mp->b_next, mp->b_prev, mp->b_cont);
		kdbprintf("b_rptr %8X b_wptr %8X (size %d)\n",
			mp->b_rptr, mp->b_wptr, mp->b_wptr - mp->b_rptr);
		kdbprintf("b_band %d b_flag ", (int)mp->b_band);
		if (mp->b_flag) {
			if (mp->b_flag & MSGMARK)
				kdbprintf("MSGMARK ");
			if (mp->b_flag & MSGNOLOOP)
				kdbprintf("MSGNOLOOP ");
			if (mp->b_flag & MSGDELIM)
				kdbprintf("MSGDELIM ");
			if (mp->b_flag & MSGNOTIFY)
				kdbprintf("MSGNOTIFY ");
			if (mp->b_flag & MSGCOMPRESS)
				kdbprintf("MSGCOMPRESS ");
			kdbprintf("\n");
		} else
			kdbprintf("0\n");
		if (!mp->b_datap) {
			kdbprintf("dblk (%8X):\n", mp->b_datap);
			m = mp->b_cont;
			if (m)
				kdbprintf("\n");
			continue;
		}
		kgetstruct(mp->b_datap, &stored, sizeof(dblk_t));	
		kdbprintf("dblk (%8X): db_freep %8X db_frtnp %8X\n",
			mp->b_datap, dp->db_freep, dp->db_frtnp);
		kdbprintf("    db_base %8X db_lim %8X)\n",
			dp->db_base, dp->db_lim);
		kdbprintf("    db_ref %d db_iswhat %d db_type ",
			dp->db_ref, dp->db_iswhat);
		if (type = find_mtype((int)dp->db_type)) {
			kdbprintf("%s", type);
			if (dp->db_type == M_IOCTL) {
				struct iocblk   *iocp;

				iocp = (struct iocblk *) mp->b_rptr;
				kdbprintf(" (%s)\n", 
					ioctl_string(knget(&iocp->ioc_cmd)));
			} else
				kdbprintf("\n");
		} else
			kdbprintf("<%d>\n", (int)dp->db_type);
		kdbprintf("    db_size %d, db_msgaddr %8X\n",
			dp->db_size, dp->db_msgaddr);
		if ((dp->db_lim - dp->db_base) != dp->db_size)
			kdbprintf("    COMPUTED SIZE (db_lim - db_base): %d\n",
				     dp->db_lim - dp->db_base);
		m = mp->b_cont;
		if (m)
			kdbprintf("\n");
	}
}

static void
qflags (qflag)
	unsigned long	qflag;
{
	kdbprintf("q_flag ");
	if (qflag & QREADR)
		kdbprintf("READ");
	else
		kdbprintf("WRITE");
	if (qflag & QNOENB)
		kdbprintf(" | QNOENB");
	if (qflag & QFULL)
		kdbprintf(" | QFULL");
	if (qflag & QWANTR)
		kdbprintf(" | QWANTR");
	if (qflag & QWANTW)
		kdbprintf(" | QWANTW");
	if (qflag & QUSE)
		kdbprintf(" | QUSE");
	else
		kdbprintf(" (QUSE NOT set!)");
	if (qflag & QENAB)
		kdbprintf(" | QENAB");
	if (qflag & QBACK)
		kdbprintf(" | QBACK");
	if (qflag & QOLD)
		kdbprintf(" | QOLD");
	if (qflag & QHLIST)
		kdbprintf(" | QHLIST");
	if (qflag & QSAFE)
		kdbprintf(" | QSAFE");
	if (qflag & QWELDED)
		kdbprintf(" | QWELDED");
	kdbprintf("\n");
}

static void
qbflags (qbflag)
	unsigned long	qbflag;
{
	kdbprintf(" qb_flag ");
	if (qbflag == 0) {
		kdbprintf("0\n");
		return;
	}
	if (qbflag & QB_FULL)
		kdbprintf("QB_FULL");
	if (qbflag & QB_WANTW)
		kdbprintf(" QB_WANTW");
	if (qbflag & QB_BACK)
		kdbprintf(" QB_BACK");
	kdbprintf("\n");
}

static void
queue_print (qp)
	queue_t	*qp;
{
	queue_t store;
	queue_t *q = &store;
	qband_t storeb, *band;
	qband_t	*qband = &storeb;
	struct module_info *p;

	if (!qp)
		return;
	kgetstruct(qp, &store, sizeof(queue_t));
	kdbprintf("queue (%8X):\n", qp);
	p = (struct module_info *) knget(&q->q_qinfo->qi_minfo);
	kdbprintf("q_qinfo %8X (module name %s)\n", q->q_qinfo, 
		kgetstr(knget(&p->mi_idname)));
	kdbprintf("q_first %8X q_last %8X q_count %d\n",
		q->q_first, q->q_last, q->q_count);
	kdbprintf("q_next %8X q_link %8X q_ptr %8X\n",
		q->q_next, q->q_link, q->q_ptr);
	qflags(q->q_flag);
	kdbprintf("q_hiwat %d q_lowat %d\n",
		q->q_hiwat, q->q_lowat);
	kdbprintf("q_minpsz %d q_maxpsz %d\n",
		q->q_minpsz, q->q_maxpsz);
	kdbprintf("q_other %8X\n", q->q_other);

	kdbprintf("q_bandp %8X q_nband %d\n", q->q_bandp, q->q_nband);
	for (band = q->q_bandp; band; band = qband->qb_next) {
		kgetstruct(band, &storeb, sizeof(qband_t));
		kdbprintf("    band %d: qb_first %8X qb_last %8X qb_count %d\n",
			(int)(band - q->q_bandp),
			qband->qb_first, qband->qb_last, qband->qb_count);
		kdbprintf("    qb_hiwat %d qb_lowat %d",
			qband->qb_hiwat, qband->qb_lowat);
		qbflags(qband->qb_flag);
	}
}

static void
all_queue_print(q)
	queue_t	* q;
{
	void	queue_print();

	q = (queue_t *) knget(&q->q_other);
	while (knget(&q->q_next))
		q = (queue_t *) knget(&q->q_next);
	q = (queue_t *) knget(&q->q_other);
	while (q) {
		queue_print(q);
		kdbprintf("\n");
		q = (queue_t *) knget(&q->q_next);
	}
}

static void
oneline_mblk(m, leader, index)
	mblk_t *m;
	int leader, index;
{
	mblk_t	store;
	mblk_t	*mp = &store;
	uchar	type;

	if (leader)
		kdbprintf("%c ", leader);
	kdbprintf("[%4D] mp %8X: ", index % 10000, m);
	kgetstruct(m, &store, sizeof(mblk_t));
	switch (type = (uchar) knget(&(mp->b_datap)->db_type)) {
	case M_IOCTL:
	case M_IOCACK:
	case M_IOCNAK:
	{
		struct iocblk	*iocp = (struct iocblk *) mp->b_rptr;
		char		*ptr, buff[20];

		strcpy(buff, ioctl_string(knget(&iocp->ioc_cmd)));
		ptr = buff;
		if (strlen(buff) > 8) {
			if (strncmp(buff, "TIOC", 4) == 0)
				ptr = buff + 4;
			else if (strncmp(buff, "TC", 2) == 0)
				ptr = buff + 2;
		}
		kdbprintf("%s %-8s ", type == M_IOCTL ? "IOC" :
				   (type == M_IOCACK ? "ACK" : "NAK"), ptr);
		break;
	}
	case M_NOTIFY:
		kdbprintf("%s %-5D ", "NOTIFY", knget(mp->b_rptr));
		break;
	case M_READ:
		kdbprintf("%s %-7D ", "READ", knget(mp->b_rptr));
		break;
	case M_FLUSH:
		kdbprintf("FLUSH %c%c%s  ", (*mp->b_rptr) & FLUSHR ? 'R' : ' ',
			(char)knget(mp->b_rptr) & FLUSHW ? 'W' : ' ',
			mp->b_flag & MSGNOTIFY ? "(N)" : "   ");
		break;
	case M_DATA:
		kdbprintf("DATA %s %s ",
				mp->b_flag & MSGNOTIFY ? "(N)" : "   ",
				mp->b_flag & MSGCOMPRESS ? "(C)" : "   ");
		if ((mp->b_cont == 0) && ((mp->b_wptr - mp->b_rptr) < 30)) {
			dump_data(mp->b_rptr
				  , mp->b_rptr, (mp->b_wptr - mp->b_rptr));
			kdbprintf("\n");
			return;
		}

		break;
	default: {
		char	buff[30];
		sprintf(buff, "%s", find_mtype(type) + 2);
		kdbprintf("%-12s ", buff);
		}
	}
	kdbprintf("rptr %8X (sz = %d), cont %8X\n", 
		mp->b_rptr, mp->b_wptr - mp->b_rptr, mp->b_cont);
}

static void
all_mblk_print(queue_t *q)
{
	int	i = 1;
  	mblk_t	*mp = (mblk_t *) knget(&q->q_first);
	unsigned char *rptr, *wptr, type;

  	while (mp) {
		oneline_mblk(mp, 0, i);
    	    	mp = (mblk_t *) knget(&mp->b_next);
		i++;
  	}
}
#endif /* DEBUG && !FOR_CRASH */
