static char sccsid[] = "@(#)28	1.3  src/bos/usr/sbin/crash/errpt.c, cmdcrash, bos411, 9435A411a 8/25/94 12:58:47";
/*
 * COMPONENT_NAME: (CMDCRASH) 
 *
 * FUNCTIONS: errpt
 *
 * ORIGINS: 27,3
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * (Copyright statements and/or associated legends of other
 * companies whose code appears in any part of this module must
 * be copied here.)
 */

#include <stdio.h>
#include <errno.h>
#include <dbxstclass.h>
#include <sys/dump.h>
#include <filehdr.h>
#include <aouthdr.h>
#include <syms.h>
#include <sys/erec.h>

#include "crash_msg.h" 
#include "crash.h"
extern nl_catd  scmc_catd;
extern 	int	dumpflag;
extern	char	*namelist;
extern struct cm *cm;

static int
getint(addr)
unsigned long *addr;
{
	int i;
	if (memread(addr, &i, sizeof(i)) < 0)
		i = -1;
	return i;
}

static int
show_err(addr)
unsigned long *addr;
{
	/* prints the errlog entry starting at address addr.
	 * returns 1 if we should stop, 0 if ok.
	 */

	char *errids;
	char errmess[128], cmd[512];
	int i, esiz;
	int data[ERR_REC_MAX];
	struct erec err;
	FILE *fp;
        struct tm *ptm;
        char *pbuf, buf[40];
 
        pbuf = buf;

	memread(addr, &err, sizeof(err));

	if (err.erec_rec.error_id == 0)
		return 1;

	if ((errids = getenv("ERRIDS")) == NULL)
	    errids = "/usr/include/sys/errids.h";

	errmess[0] = '\0';

	sprintf(cmd, "/bin/grep -i %#08x %s 2>/dev/null | "
		"/bin/sed -e 's/^# *define *//' "
			 "-e 's/[ ^I]0x[0-9a-fA-F]*[ ^I]/ /' "
			 "-e 's/\\/\\*//' "
			 "-e 's/\\*\\///' "
			 "-e 's/ERRID_//' "
			 "-e 's/[ 	][ 	]*$//'",
		err.erec_rec.error_id, errids);
	fp = popen(cmd, "r");
	if (fp != NULL) {
	    fgets(errmess, sizeof(errmess), fp);
	    pclose(fp);
	}
	if (errmess[0] == '\0')
		sprintf(errmess, "%#08x\n", err.erec_rec.error_id);

        ptm = localtime(&err.erec_timestamp);
        strftime(pbuf, 39, "%c", ptm);
        printf("    %-25.25s: %s", pbuf, errmess);

	printf("\tResource Name: %-16.16s\n", err.erec_rec.resource_name);

	esiz = &err.erec_rec.detail_data - &err;
	memread((char *)addr+esiz, data, err.erec_len - esiz);
	for (i=0; i < (err.erec_len - esiz)/sizeof(int); i++) {
		if (i > 0 && i % 4 == 0)
			fputs("\n\t", stdout);
		else if (i == 0)
			putchar('\t');
		printf("%08x ", data[i]);
	}
	if (i > 0)
		putchar('\n');
	return 0;
}

int
errpt(num)
int num;
{
	/* Prints the error log from the dump.
	 * Prints all errors not picked up by errdemon.
	 * num is the number of errors to print that were not
	 * already picked up by errdemon.
	 */
	unsigned long *addr, symtoaddr();
	unsigned int from, next, caddr;
	unsigned int froms[2];
	struct errc_io errio;
	int diff, odcnt, i, tot_odcnt, print_cnt, off;
	int diffs[2];

	if ((addr = symtoaddr("*errc")) == -1) {
	    printf(NLcatgets(scmc_catd, MS_crsh, MAIN_MSG_35,
		"Symbol not found.\n"));
	    return -1;
	}
	memread(addr+5, &errio, sizeof(errio));

	/* print out messages not picked up by errdemon before crash */
	if (errio.e_outptr != errio.e_inptr) {
		printf(NLcatgets(scmc_catd, MS_crsh, ERRPT_MSG_2,
			"ERRORS NOT READ BY ERRDEMON (MOST RECENT LAST):\n") );
		if (errio.e_outptr < errio.e_inptr) {
			/* easy breazy */
			diffs[0] = errio.e_inptr - errio.e_outptr;
			diffs[1] = 0;
			froms[0] = errio.e_outptr;
			froms[1] = 0;
		}
		else {
			/* wrap around in buffer */
			diffs[0] = errio.e_end - errio.e_outptr;
			diffs[1] = errio.e_inptr - errio.e_start;
			froms[0] = errio.e_outptr;
			froms[1] = errio.e_start;
		}
		for (i=0; i<2; i++) {
			diff = diffs[i];
			if (diff == 0)
				break;
			from = froms[i];
			tot_odcnt = 0;
			while (tot_odcnt < diff) {
				odcnt = getint(from);
				next = from + odcnt;
				if (odcnt == 0)
					tot_odcnt = diff;
				else
					tot_odcnt = tot_odcnt + odcnt;

				if (show_err(from) == 0)
					from = next;
				else
					break;
			}
		}
	}

	/* print last num errors logged to errlog */
	if (errio.e_outptr > errio.e_start && num > 0) {
		/* backup but don't go before errio.e_start
		 * look for a number whose offset is equal to 
		 * the distance from errio.e_outptr.
		 */

		print_cnt = 0;
		printf(NLcatgets(scmc_catd, MS_crsh, ERRPT_MSG_2,
			"\nLAST %d ERRORS READ BY ERRDEMON (MOST RECENT FIRST):\n"),
			num );
		caddr = errio.e_outptr - sizeof(int);
		while (caddr > errio.e_start) {
			off = getint(caddr);
			diff = errio.e_outptr - caddr;
			if (off == diff) {
				if (show_err(caddr) == 0)
					errio.e_outptr = caddr;
				else
					break;
				print_cnt++;
				if (print_cnt >= num)
					break;
			}
			caddr -= sizeof(int);
		}
	}
	return 0;
}
