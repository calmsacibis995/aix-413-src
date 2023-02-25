static char sccsid[] = "@(#)08  1.9  src/bos/usr/sbin/acct/acctcon2.c, cmdacct, bos411, 9428A410j 12/21/93 14:05:22";
/*
 * COMPONENT_NAME: (CMDACCT) Command Accounting
 *
 * FUNCTIONS: none 
 *
 * ORIGINS: 3,9,27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *	acctcon2 <ctmp >ctacct
 *	reads std. input (ctmp.h/ascii format)
 *	converts to tacct.h form, writes to std. output
 */

#include <sys/types.h>
#include "acctdef.h"
#include <stdio.h>
#include <locale.h>
#include "ctmp.h"
#include "tacct.h"
#include "acct_msg.h"

nl_catd catd;
#define MSGSTR(Num, Str) catgets(catd, MS_ACCT, Num, Str)

struct	ctmp	cb;
struct	tacct	tb;

main(int argc, char **argv)
{
	unsigned long devtemp;
	int rc;
	int errors = 0;
	setlocale(LC_ALL,"");
	catd = catopen(MF_ACCT, NL_CAT_LOCALE);

	tb.ta_sc = 1;
        /*
         * For each line of standard input, convert the ctmp record
         * into a tacct record and write to standard output.  A valid
         * ctmp record consists of 7 fields, of which 6 are used by
         * acctcon2.
         */

        while ((rc = scanf("%lu\t%lu\t%s\t%lu\t%lu\t%lu\t%*[^\n]",
		&devtemp,
		&cb.ct_uid,
		cb.ct_name,
		&cb.ct_con[0],
		&cb.ct_con[1],
                &cb.ct_start)) != EOF) {

                /*
                 * Verify that the correct number of fields were seen
                 * by scanf().  If the wrong number of fields is read,
                 * acctcon2 will attempt to re-sychronize by scanning
                 * for end of line and then reading the next line.
                 */

                if (rc != 6) {
                        errors++;

                        while ((rc = getchar ()) != EOF && rc != '\n') ;

                        continue;
                }
		cb.ct_tty = (dev_t)devtemp;
		tb.ta_uid = cb.ct_uid;
		CPYN(tb.ta_name, cb.ct_name);
		tb.ta_con[0] = MINS(cb.ct_con[0]);
		tb.ta_con[1] = MINS(cb.ct_con[1]);
		fwrite((void *)&tb, (size_t)sizeof(tb),(size_t) 1, stdout);
	}
        /*
         * When no input errors are seen, acctcon2 exits to indicate
         * the output is valid, otherwise a descriptive message is
         * produced.
         */

        if (errors != 0) {
                fprintf (stderr, MSGSTR(BADCTMP, "%s: corrupted ctmp file\n"),
                        argv[0]);

                exit (1);
        } else
                exit (0);
}
