static char sccsid[] = "@(#)96	1.2  src/rspc/kernext/isa/tty/sf_db.c, isatty, rspc411, GOLD410 6/28/94 13:25:26";
/*
 *   COMPONENT_NAME: isatty
 *
 *   FUNCTIONS: k_in k_out sf_print db_termios db_termiox k_rs
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * sf_db.c -	debug routines for sf tty
 */
#include <sys/types.h>
#include <sys/systemcfg.h>		/* runtime checks		*/
#include <sys/sysmacros.h>		/* minor and major macros	*/
#include <sys/intr.h>			/* interrupt handler stuff	*/
#include <sys/pin.h>			/* pinning stuff		*/
#include <sys/adspace.h>		/* address space junk		*/
#include <sys/ioacc.h>			/* bus access macros		*/
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
#include "common_open.h"		/* Common open discipline fns	*/
#include <sys/rs.h>			/* rs's ioctl's			*/
#include <sys/inline.h>
#include "sty.h"
#include "sf.h"

/*
 * definitions for the iir register on the 16550
 */
char *i_show[] = {
	"Modem status change",
	"Xmit interrupt",
	"fifo full",
	"lsr changed",
	"Unknown  8",
	"Unknown 10",
	"Input data",
	"Unknown 14",
	"**gok",
	0,
};

#if ttyDBG
/*
 * Debug assists.
 * There are two levels of debug code here.  The `ttyDBG' code should always
 * be enabled in the system, as it provides TTY debug support to the kernel
 * debugger, as well as to the crash command.
 * The `DEBUG' code is for those situations where one needs a little more.
 * This should never be turned on in a production build, but should be useful
 * for tracking down problems in the field as well as in development.
 * Also, note that this code functions both as the kernel debug assist code,
 * as well as the user level "crash" component when a "sf" type tty
 * is encountered.
 */

#define disp(var, elem, n)	db_d2("elem", var##elem, n);

static struct sfcom *koff_sc;

#ifdef FOR_CRASH
uchar k_in(int port) { return 0; }
uchar k_out(int port, uchar val) { return 0; }
#else

/*
 * k_in -	"inbyte" support for debug assist routines.
 *
 * Input:
 *	port	-	I/O port address on ISA bus
 *
 * Returns:
 *	byte read
 */
static uchar
k_in(int port) {
	register volatile uchar *p;
	register uchar val;

	p = (void *) io_att(koff_sc);

	val = p[port];

	io_det(p);

	return val;
}

/*
 * k_out -	"outbyte" support for debug assist routines.
 *
 * Input:
 *	port	-	I/O port address on ISA bus
 *	val	-	byte to store
 *
 * Returns:
 *	val, so that "k_out" can be used in expressions.
 */
static uchar
k_out(int port, uchar val) {
	register volatile uchar *p;

	p = (void *) io_att(koff_sc);

	p[port] = val;	eieio();

	io_det(p);

	return val;
}
#endif

/*
 * sf_print -	debug print function for sandalfoot native serial porks
 *
 * Input:
 *	sc	-	^ to device
 *	verb	-	flag: !0 ==> verbose
 */
void
sf_print(sc, verb)
register struct sfcom *sc; {
	static char *mdm_status[] = {
		" delta CTS",	" delta DSR",	" Ring Fell",
		" delta CD",	" CTS",		" DSR",
		" RI",		" DCD",		0,
	};
	static char *temp[] = {
		" Rx Data",	   " Tx Hold Empty",
		" Rx Line Status", " Modem Status",
		0,
	};
	static char *par[] = {
		"odd",	"even",	"mark",	"space",	0,
	};
	register struct sty_tty *tp;
	register unit;
	uchar lcr, rbr, iir, lsr, msr, ier, dlm, dll;

	if (for_CRASH) {
		/*
		 * crash mode, we need to slurp in the data ourselves
		 */
		static struct sfcom zot;

		tty_read_mem(sc, &zot, sizeof (zot));
		sc = &zot;
	}

	tp   = &sc->sf_tty;
	unit = minor(tp->t_dev);

	/*
	 * print common "sty" portion first
	 */
	sty_print(tp, verb);

	if (!for_CRASH) {
		/*
		 * only kernel level access to the UART
		 */
		koff_sc = sc;	/* for k_in/k_out	*/

		lcr = k_in(LCR(0));

		k_out(LCR(0), lcr|DLAB);
		dlm = k_in(DLM(0));
		dll = k_in(DLL(0));

		k_out(LCR(0), lcr&~DLAB);
		iir = k_in(IIR(0));
		rbr = k_in(RBR(0));
		lsr = k_in(LSR(0));
		msr = k_in(MSR(0));
		ier = k_in(IER(0));

		db_d2(0,0);	/* flush	*/

#define d4(x)	db_d2("x", x, 4)

		d4(unit); d4(lcr); d4(iir); d4(rbr); d4(lsr); d4(msr);
		d4(dlm);  d4(dll);

		db_d2(0,0);	/* flush	*/

		if (ier) {
		    printd("\nInterrupts enabled for:");
		    dump_bits(ier, temp);
		} else
		    printd("\nNo interrupts enabled");

		printd("\n%d data bits, %s, %s parity, %s set, dlab %s"
		       , 5+(lcr & (WLS0|WLS1))
		       , lcr & STB ? "2 stop bits" : "1 stop bit"
		       , lcr & PEN ? par[(lcr>>4) & 3] : "no"
		       , lcr & BREAK ? "break" : "no break"
		       , lcr & DLAB ? "set" : "clear");

		{				    /* modem control */
			static char *temp[8] = {
			    " DTR",
			    " RTS",
			    " Out1",
			    " Out2",
			    " Loop",
			    0,
			};
			printd("\nmodem control bits:");
			dump_bits(k_in(MCR(0)), temp);
		}
		printd("\nscratch register = 0x%2x", k_in(SCR(0)));

		{
			printd("\nDivisor = 0x%02x%02x => ", dlm, dll);
			if (dlm || dll) {
			    int temp = (sc->sf_xtal * 100) / 16 / (dlm * 256 + dll);
			    printd("%d.%02d baud", temp / 100, temp % 100);
			} else
			    printd("stopped\n");
		} {
			static char *i_source[8] = {
			    "modem status",
			    "transmit empty",
			    "receive data",
			    "receive line status",
			    "",
			    "",
			    "char timeout",
			    0,
			};

			printd("\nReceive char = %x, %s pending. fifo's %s"
			       , rbr
			       , (iir & 1
				  ? "no interrupt" : i_source[(iir>>1)&7])
			       , iir & 0x80 ? "enabled" : "disabled");

			printd("\nModem Status:");
			dump_bits(msr, mdm_status);
			printd("\nLast Modem Status:");
			dump_bits(sc->sf_msr, mdm_status);

			{
			    static char *temp[] = {
				" data ready",
				" overrun",
				" parity error",
				" framing error",
				" break",
				" xmit hold empty",
				" xmit empty",
				" error in fifo",
				0,
			    };
			    printd("\nLine status:");
			    dump_bits(lsr, temp);
			}
		}
		printd("\n");

		k_out(LCR(0), lcr);	       /* restore dlab */
	}

#define sf8(elem)		disp(sc->sf_, elem, 8)
#define sfc8(elem)		disp(sc->sf_conf., elem, 8)

	sfc8(bid); sfc8(xtal); sfc8(port);
	sfc8(irq); sfc8(flags); sfc8(rtrig); sfc8(tbc);
	sf8(rcnt); sf8(msr);    sf8(delta);  sf8(open);
	sf8(ethrei); sf8(xmit); sf8(ioff);   sf8(flox);
	sf8(watch); sf8(cconf); sf8(uconf); sf8(dbg);
	sf8(xcnt); sf8(xdata);

	db_d2(0,0);	/* flush */

	printd("\ndefault termios:");
	db_termios(&tp->t_dflt.tios);
	printd("\ndefault termiox:");
	db_termiox(&tp->t_dflt.tiox);
	printd("\n");
}

#undef d8
#define d8(elem)		disp(t->c_, elem, 8);

/*
 * db_termios -	debug print of a termios struct
 */
db_termios(register struct termios *t) {

	db_d2(0,0);	/* flush	*/

	d8(iflag);	/* input modes */
	d8(oflag);	/* output modes */
	d8(cflag);	/* control modes */
	d8(lflag);	/* line discipline modes */

	db_d2(0,0);	/* flush	*/

	printd("\nc_cc: ");
	dump_data(&t->c_cc[0], &t->c_cc[0], sizeof (t->c_cc));
}

#undef d8
#define d8(elem)		disp(t->c_, elem, 8);

/*
 * db_termiox -	debug print of a termiox struct
 */
db_termiox(register struct termiox *t) {
	static char *h_bits[] = {
		"RTSXOFF",
		"CTSXON",
		"DTRXOFF",
		"CDXON",
	};
	static char *s_bits[] = {
		"DTR_OPEN",
		"WT_OPEN",
		"RI_OPEN",
	};

	db_d2(0,0);	/* flush	*/

	printd("\nhflag: ");	dump_bits(t->x_hflag & (1|2|4|8), h_bits);
	printd("\nsflag: ");	dump_bits(t->x_sflag & (1|2|4),   s_bits);
}

/*
 * k_rs	-	display information on SandalFoot "rs" devices
 *
 * Input:
 *	c	-	argument count
 *	v	-	argument vector
 */
k_rs(int c, char **v) {
#ifdef DEBUG
	register struct sfcom *sc;
	register unit;
	static struct krash_func krs = {
		k_rs,
		"rs",
		"rs [0|1]",
		"print info on sandalfoot serial ports"
	};
	static once;

	if (c == 0 && !once) {
		krash_add(&krs);
		++once;
		return;
	} else if (c == -1) {
		krash_del(&krs);
		once = 0;
		return;
	}

	for (sc = &sfcom[unit = 0]; unit < NSFCOM; ++unit, ++sc)
		sf_print(sc, 1);
#endif /* DEBUG		*/
}
#endif /* ttyDBG	*/
