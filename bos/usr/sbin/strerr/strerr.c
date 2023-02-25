static char sccsid[] = "@(#)55	1.5.1.1  src/bos/usr/sbin/strerr/strerr.c, cmdpse, bos411, 9428A410j 11/16/93 09:00:23";
/*
 *   COMPONENT_NAME: CMDPSE
 *
 *   ORIGINS: 27 63
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/stropts.h>
#include <sys/strlog.h>
#include <time.h>

#include "strerr_msg.h"
#include <locale.h>
nl_catd catd;
#define MSGSTR(n,s)	catgets(catd,MS_STRERR,n,s)

#define	ADMIN	"root"
#define	DIR	"/var/adm/streams"
#define	LOG	"/dev/slog"

FILE *errfile();
void email();
extern FILE *fopen(), *popen();

/* lint considerations */
#define	Fprintf	(void)fprintf
#define	Sprintf	(void)sprintf
#define	Strcat	(void)strcat
#define	Strcpy	(void)strcpy
#define	Strncpy	(void)strncpy

char	*Admin;

/*
 * strerr [admin]
 */

main(argc, argv)
	int argc;
	char **argv;
{
	struct strbuf	ctlbuf;
	struct strbuf	databuf;
	struct log_ctl	logctl;
	struct strioctl	stri;
	char	buf[LOGMSGSZ];
	int	log_fd;
	int	flags;
	char	*cp;
	FILE	*fp;

	setlocale(LC_ALL, "");
	catd = catopen(MF_STRERR, NL_CAT_LOCALE);

	Admin = argc > 1 ? argv[1] : ADMIN;

	/* Open a stream to the log device */
	if ((log_fd = open(LOG, 2)) == -1) {
		perror(MSGSTR(MSG1, "strerr: couldn't open log device"));
		exit(1);
	}

	/* Register for error messages */
	stri.ic_cmd = I_ERRLOG;
	stri.ic_timout = -1;
	stri.ic_dp = (char *)0;
	stri.ic_len = 0;
	if (ioctl(log_fd, I_STR, (char *)&stri) == -1) {
		Fprintf(stderr, MSGSTR(MSG2, "strerr: already registered\n"));
		exit(1);
	}

	ctlbuf.buf = (char *)&logctl;
	ctlbuf.maxlen = sizeof(logctl);
	databuf.buf = buf;
	databuf.maxlen = sizeof(buf);
	flags = 0;
	fp = (FILE *)0;

	while (getmsg(log_fd, &ctlbuf, &databuf, &flags) != -1) {
		char *cp;
		int *ap;
		int i;

		if (logctl.flags & SL_NOTIFY) {
			char buf1[LOGMSGSZ+10];
			Sprintf(buf1, "%06d %s\n", logctl.seq_no, buf);
			email(Admin, buf1);
		}

		/*
		 * write to the appropriate file
		 * NB: NOT fatal if null return: could still send email
		 */
		if (!(fp = errfile(logctl.ttime, fp)))
			continue;

		cp = ctime(&logctl.ttime) + sizeof("Mon Day DD ") - 1;
		cp[8] = 0;
		fprintf(fp, "%06d %s %08x %2d ", logctl.seq_no, cp,
			logctl.ltime, (signed char)logctl.level);
		putc(logctl.flags & SL_FATAL  ? 'F' : '.', fp);
		putc(logctl.flags & SL_NOTIFY ? 'N' : '.', fp);
		putc(logctl.flags & SL_TRACE  ? 'T' : '.', fp);
		fprintf(fp, " %d %d ", logctl.mid, logctl.sid);
		i = (int)buf + strlen(buf);
		ap = (int *)(i + (WORDLEN - i % WORDLEN));
		fprintf(fp, buf, ap[0], ap[1], ap[2], ap[3]);
		putc('\n', fp);
		fflush(fp);
	}
}

/*
 * errfile - return FILE pointer to appropriate error file
 *
 * returns FILE pointer or NULL
 */

FILE *
errfile(t, fp)
	time_t t;
	FILE *fp;
{
	char buf[BUFSIZ];
	static struct tm *otmp;	/* last time */
	struct tm *tmp;		/* this time */

	if (!otmp) {
		otmp = (struct tm *)malloc(sizeof(struct tm));
		(void)memset(otmp, 0, sizeof(struct tm));
	}

	/* use same file if same day */
	tmp = localtime(&t);
	if (fp) {
		if ((tmp->tm_mday == otmp->tm_mday) &&
		    (tmp->tm_mon == otmp->tm_mon))
			return fp;
		(void)fclose(fp);
	}

	/* update last time used, create a new file */
	*otmp = *tmp;
	Sprintf(buf, "%s/error.%02d-%02d", DIR, tmp->tm_mon+1, tmp->tm_mday);
	if (!(fp = fopen(buf, "a+"))) {
		extern int errno;
		Sprintf(buf, "strerr: cannot open '%s' (errno %d)\n",buf,errno);
		email(Admin, buf);
	}
	return fp;
}

/*
 * send email to sysadmin
 */

void
email(who, message)
	char	*who;
	char	*message;
{
	char	cmd_buf[128];
	FILE	*fp;

	Sprintf(cmd_buf, "mail %s", who);
	if (!(fp = popen(cmd_buf, "w")))
		return;
	Fprintf(fp, "Streams Error Logger message notification:\n\n");
	Fprintf(fp, "%s", message);
	(void)pclose(fp);
}
