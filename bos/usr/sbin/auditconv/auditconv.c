static char sccsid[] = "@(#)09  1.4  src/bos/usr/sbin/auditconv/auditconv.c, cmdsaud, bos411, 9434A411a 8/19/94 08:23:12";
/*
 * COMPONENT_NAME: (CMDSAUD) security: auditing subsystem
 *
 * FUNCTIONS: auditconv command
 *
 * ORIGINS: 83
 *
 */
/*
 * LEVEL 1,  5 Years Bull Confidential Information
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/audit.h>
#include <sys/errno.h>
#include <sys/priv.h>
#include <ulimit.h>
#include <ctype.h>
#include "locale.h"

#include "auditcv_msg.h"
nl_catd catd;
#define	MSGSTR(n,s) catgets(catd,MS_AUDITCV,n,s)

typedef	struct pre41_rec
{
	ushort_t ah_magic;	/* magic value new format = AUD_REC_MAGIC */
	ushort_t ah_length;	/* length of tail of this record */
	char	ah_event[16];	/* event name with null terminator */
	ulong_t ah_result;	/* the audit status - see auditlog for values */
	uid_t	ah_ruid;	/* real user id */
	uid_t	ah_luid;	/* login user id */
	char	ah_name[MAXCOMLEN];	/* program name with null terminator */
	pid_t	ah_pid;		/* process id */
	pid_t	ah_ppid;	/* process id of parent */
	time_t	ah_time;	/* time in secs */
	long	ah_ntime;	/* nanosecs offset from ah_time */
	/* record tail follows */
};

FILE	*Input, *Output;

char	*a_rec, *b_rec;
int	a_alloc, b_alloc;
int	a_recp, b_recp;
int	a_end;
int	a_bin;

struct	aud_bin	bh, bt;
int	cvtbin_size;
int	packed;


/*
 * Read from input file to input buffer
 */

int rd(siz)
	int	siz;
{
	/* 
	 * Check for data already available 
 	 */

	if (a_recp + siz < a_end)
		return (0);

	if (!a_bin)
	{
		int	cnt;
		struct	aud_bin	bin;

		/* 
		 * Check that we have enough room in the buffer 
		 * for the data 
		 */

		if (a_recp + siz > a_alloc)
		{
			a_alloc = a_recp + siz;
			a_rec = (char *) realloc(a_rec, a_alloc);
			if (!a_rec)
				exit1(0,MSGSTR(ERRNOSPACE,"** Out of memory"),0);
		}
		
		cnt = fread(&(a_rec[a_end]), (size_t) 1,
			(size_t) (a_recp + siz) - a_end, Input);

		if (cnt > 0) a_end += cnt;
		else return(-1);
	}
	return (((a_recp + siz) <= a_end) ? 0 : -1);
}

/* 
 * Read a bin from the input file
 */

int getbin()
{
	int	sav_recp;
	int	bin_len;
	char	*unpacked_rec;

	packed = 0;
	sav_recp = a_recp;
	if (rd(sizeof(struct aud_bin)))
		goto fail;

	/* 
	 * Verify (stream of bins) assumption 
	 */

	bcopy(&(a_rec[a_recp]), &bh, sizeof(bh));
	if (bh.bin_magic != AUDIT_MAGIC)
		goto fail;

	/* skip the bin header */
	a_recp += sizeof(struct aud_bin);

	if (bh.bin_plen && (bh.bin_plen != bh.bin_len))
	{
		packed++;
		bin_len = bh.bin_plen;
	}
	else bin_len = bh.bin_len;

	if (rd(bin_len + sizeof(struct aud_bin)))
		goto fail;

	bcopy(&(a_rec[a_recp + bin_len]), &bt, sizeof(bt));
	if (bt.bin_magic != AUDIT_MAGIC)
		goto	fail;
	if (!bt.bin_tail)
		goto fail;
	if ((bh.bin_len != bt.bin_len) || (bh.bin_plen != bt.bin_plen))
		goto fail;

	/* 
	 * Unpack bin if necessary 
	 */

	if (packed)
	{
		unpacked_rec = (char *) auditpack(1, a_rec+sav_recp);
		if (unpacked_rec == NULL)
			exit1(0,MSGSTR(ERRPACK,"** auditpack failed"),0);

		free(a_rec);

		bcopy(unpacked_rec, &bh, sizeof(bh));
		a_alloc = bh.bin_len + 2 * sizeof(struct aud_bin);
		a_recp = sizeof(struct aud_bin);
		a_rec = unpacked_rec;
		a_end = a_alloc;
	}

	/* 
	 * Care about the output bin
	 */

	cvtbin_size = 0;
	bcopy(&bh, b_rec, sizeof(bh));
	b_recp = sizeof(struct aud_bin);

	/* 
	 * Consume the bin tail! 
	 */

	a_end -= sizeof(struct aud_bin);

	return (0);
fail:
	return(-1);
}

/* 
 * Read audit record from the input bin
 */

char *getaud(ah)
	struct	pre41_rec	*ah;
{
	int	sav_recp;

	sav_recp = a_recp;

	/* 
	 * Read next header 
	 */

	if (rd(sizeof(struct pre41_rec)))
	{
		a_recp = sav_recp;
		return(NULL);
	}

	bcopy(&(a_rec[a_recp]), ah, sizeof(struct pre41_rec));

	a_recp += sizeof(struct pre41_rec);

	/* 
	 * Read tail if required
	 */

	if (ah->ah_length)
	{
		if (rd(ah->ah_length))
		{
			a_recp = sav_recp;
			return(NULL);
		}
	}

	a_recp += ah->ah_length;

	return(&(a_rec[sav_recp + sizeof(struct pre41_rec)]));
}

/* 
 * Convert the audit record and put it in the converted bin
 */

void putaud(pre, tail)
	struct	pre41_rec	*pre;
	char			*tail;
{
	struct	aud_rec		cur;
	int			siz;

	siz = sizeof(struct aud_rec) + pre->ah_length;

	/* 
	 * Realloc if buffer is too small
	 */

	if (b_recp + siz > b_alloc)
	{
		b_alloc = b_recp + siz;
		b_rec = (char *) realloc(b_rec, b_alloc);

		if (!b_rec)
			exit1(0,MSGSTR(ERRNOSPACE,"** Out of memory"),0);
	}
		
	/* 
	 * Convert format
	 */

	cur.ah_magic = pre->ah_magic;
	cur.ah_length = pre->ah_length;
	strncpy(cur.ah_event, pre->ah_event, sizeof(pre->ah_event));
	cur.ah_result = pre->ah_result;
	cur.ah_ruid = pre->ah_ruid;
	cur.ah_luid = pre->ah_luid;
	strncpy(cur.ah_name, pre->ah_name, sizeof(pre->ah_name));
	cur.ah_pid = pre->ah_pid;
	cur.ah_ppid = pre->ah_ppid;
	cur.ah_tid = 0;
	cur.ah_time = pre->ah_time;
	cur.ah_ntime = pre->ah_ntime;

	/* 
	 * Put the new record in the converted bin
	 */

	bcopy(&cur, &(b_rec[b_recp]), sizeof(struct aud_rec));
	b_recp += sizeof(struct aud_rec);

	if (cur.ah_length)
	{
		bcopy(tail, &(b_rec[b_recp]), cur.ah_length);
		b_recp += cur.ah_length;
	}

	cvtbin_size += siz;
}

/*
 * Write a bin to the converted trail
 */

void putbin()
{
	char		*file_rec;
	int		len;

	/*
	 * Adjust converted bin size
	 */

	bh.bin_len = bt.bin_len = cvtbin_size;

	bcopy(&bh, b_rec, sizeof(struct aud_bin));


	/* 
	 * Pack bin if necessary 
	 */

	file_rec = b_rec;
	len = bh.bin_len + sizeof(struct aud_bin);

	if (packed)
	{
		file_rec = (char *) auditpack(0, b_rec);
		if (file_rec == NULL)
			exit1(0,MSGSTR(ERRPACK,"** auditpack failed"),0);

		bcopy(file_rec, &bh, sizeof(struct aud_bin));
		bt.bin_plen = bh.bin_plen;
		len = bh.bin_plen + sizeof(struct aud_bin);
	}

	/*
	 * Write bin
	 */

	if (fwrite(file_rec, 1, len, Output) <= 0)
		exit1(0,MSGSTR(ERRWRITE,
			"** Error writing to output bin file"),0);
	if (fwrite(&bt, 1, sizeof(struct aud_bin), Output) <= 0)
		exit1(0,MSGSTR(ERRWRITE,
			"** Error writing to output bin file"),0);

	if (packed)
		free(file_rec);
}

/*
 * Convert the input trail to the output trail
 */

void AuditConvert()
{
	char			*p;
	struct	pre41_rec	pre;

	a_alloc = 20 * 1024;
	a_rec = (char *) malloc(a_alloc);
	if (!a_rec)
		exit1(0,MSGSTR(ERRNOSPACE,"** Out of memory"),0);
	a_recp = a_end = a_bin = 0;
	
	b_alloc = 20 * 1024;
	b_rec = (char *) malloc(b_alloc);
	if (!b_rec)
		exit1(0,MSGSTR(ERRNOSPACE,"** Out of memory"),0);
	b_recp = 0;
	
	while (1)
	{
		/* 
		 * Reset pointers if buffer is empty 
		 */

		if (a_recp >= a_end)
		{
			if (a_bin)
				putbin();

			a_recp = a_end = 0;
			a_bin = 0;
		}

		/* 
		 * Try to fill the buffer with a bin from the trail 
		 */

		if (!a_bin)
		{
			/* 
			 * Copy data down to allow reading bin 
			 */

			if (a_recp != 0)
			{
				bcopy(&(a_rec[a_recp]), a_rec, a_end - a_recp);
				a_end -= a_recp;
				a_recp = 0;
			}

			if (getbin() == 0)
			{
				a_bin = 1;

				/* 
				 * Normally, we don't want to
			 	 * iterate here but an empty (!) bin 
				 * would cause us to quit.
				 */

				if (a_recp == a_end)
					continue;
			}
		}

		/* 
		 * if at EOF, return NULL 
		 */

		if (a_recp >= a_end)
			break;

		/* 
		 * Extract an audit record from the input trail
		 */

		if ((p = getaud(&pre)) == NULL)
			exit1(0,MSGSTR(ERRCORR,
				"** Corrupted input bin file"),0);

		/* 
		 * Convert format and copy the new record to the output trail
		 */

		putaud(&pre, p);
	}

	free(a_rec);
	free(b_rec);
}

void Usage()
{
	errno = EINVAL;
	exit1(0,MSGSTR(USAGE,"Usage: auditconv infile outfile"),0);
}

main(int argc, char **argv)
{
	setlocale(LC_ALL, "");
	catd = catopen(MF_AUDITCV, NL_CAT_LOCALE);

	/*
	 * Are we allowed to run the command ?
	 */

	if (privcheck(SET_PROC_AUDIT))
	{
		errno = EPERM;
		exit1(0,MSGSTR(ERRPRIV,
			"** SET_PROC_AUDIT privilege required"),0);
	}

	if (argc != 3)
		Usage();
	 
	/*
	 * Open input and output files 
	 */

	if ((Input = fopen(argv[1], "r")) == NULL)
	{
		strerror(errno);
		exit(1);
	}
	if ((Output = fopen(argv[2], "w")) == NULL)
	{
		strerror(errno);
		exit(1);
	}

	/*
	 * Up ulimit value to guarantee write into bin file
	 */

	if (ulimit(SET_FSIZE, 1 << 21) == -1)
		exit1(0,MSGSTR(ERRLIMIT,
			"** Error trying to raise ulimit value"),0);

	/*
	 * Now convert the audit trail
	 */

	AuditConvert();

	/*
	 * Close files
	 */

	fclose(Input);
	fclose(Output);
}
