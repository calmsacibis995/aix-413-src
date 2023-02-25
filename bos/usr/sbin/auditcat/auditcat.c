static char sccsid[] = "@(#)92  1.20  src/bos/usr/sbin/auditcat/auditcat.c, cmdsaud, bos411, 9428A410j 11/12/93 14:38:02";
/*
 * COMPONENT_NAME: (CMDSAUD) security: auditing subsystem
 *
 * FUNCTIONS: auditcat command
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   

#include	"audutil.h"
#include	"fcntl.h"
#include	"stdio.h"
#include	"sys/audit.h"
#include	"sys/stat.h"
#include	"sys/types.h"
#include	"sys/errno.h"
#include	"sys/priv.h"
#include 	"locale.h"
#include	"varargs.h"
#include	"unistd.h"

DEBUGNAME("auditcat")

#include "auditcat_msg.h"
nl_catd catd;
#define	MSGSTR(n,s) catgets(catd,MS_AUDITCAT,n,s)

#define iosize 32*1024


/* 
 * The input bin 
 */

char	*BinFile;
char	*binbuf;
int	binsize;

/* 
 * The output trail 
 */

char	*TrailFile = (char *)NULL;
int	trail;
int	trailflag = 0;
int	dorecovery;
int	pack;
int	packed;
int	unpack;

main(int argc, char *argv[]){

	extern 	int	optind;
	extern 	char	*optarg;

	struct 	aud_bin	*pbin;
	char 	*bufp;
	int	bsz;
	int	c;

	setlocale(LC_ALL, "");
	catd = catopen(MF_AUDITCAT, NL_CAT_LOCALE);

	if(privcheck(SET_PROC_AUDIT)){

		errno = EPERM;
		exit1(EPERM, 
		MSGSTR(ERRPRIV, 
		"** SET_PROC_AUDIT privilege required"), 
		0);

	}

        /*
         * Suspend auditing
         */

        if(auditproc(0, AUDIT_STATUS, AUDIT_SUSPEND, 0) < 0){

		fail(errno, 
                MSGSTR(ERRPROC,
                "** cannot suspend process auditing "),
                0);

        }

	dorecovery = pack = packed = unpack = 0;

	while((c = getopt (argc, argv, "upo:r")) != EOF){

		switch (c){
			case 'o':
				TrailFile = optarg;
				trailflag++;
				break;
			case 'r':
				dorecovery++;
				break;
			case 'u':
				unpack++;
				if(pack){

					fail(EINVAL, 
					MSGSTR(USAGE, 
			"Usage: auditcat [-p|-u] [-o trail] [-r] [bin_file]")
					);

				}
				break;
			case 'p':
				pack++;
				if(unpack){

					fail(EINVAL, 
					MSGSTR(USAGE, 
			"Usage: auditcat [-p|-u] [-o trail] [-r] [bin_file]")
					);

				}
				break;
			default:

				fail(EINVAL, 
				MSGSTR(USAGE, 
			"Usage: auditcat [-p|-u] [-o trail] [-r] [bin_file]")
				);

		};
	};

	if(optind < (argc - 1)){

		errno = EINVAL;
		fail(errno,
		MSGSTR(USAGE, 
		"Usage: auditcat [-p|-u] [-o trail] [-r] [bin_file]")
		);

	}

	/* 
	 * We will be writing only to the end of the trail file 
	 */

	if(trailflag){

		if((trail = open(TrailFile, 
		O_RDWR | O_CREAT | O_APPEND, 0640)) < 0){

			errno = EACCES;
			fail(errno, "%s\"%s\"",
			MSGSTR(ERRTRAIL, 
			"** cannot create trail file "), 
			TrailFile);

		}
	}
	else {
		TrailFile = "STDOUT";
		trail = fileno (stdout);
	}

	/* 
	 * Get input data 
	 */

	if(optind == (argc - 1)){
		int	fd;

		BinFile = argv[optind];
		if((fd = open(BinFile, O_RDWR)) < 0){

			errno = EACCES;
			fail(errno, "%s\"%s\"",
			MSGSTR(ERRBIN, 
			"** cannot open bin file "), 
			BinFile);

		}

		if(dorecovery){

			recover(fd);
			binread(fd);
		}
		else {

			binread(fd);
		}
		close (fd);
	}
	else {

		BinFile = "STDIN";
		binread(fileno (stdin));
	}


	bufp = binbuf;
	bsz = binsize;
	if(pack){

		if(!packed){
			pbin = (struct aud_bin *)auditpack(AUDIT_PACK, binbuf);

			if(errno){

				fail(errno, "%s\"%s\"",
				MSGSTR(ERRPACKING, 
				"** auditpack failed packing "), 
				TrailFile);

			}

			bsz = pbin->bin_plen + 2 * sizeof (struct aud_bin);
			bufp = (char *) pbin;
		}
		else {

			errno = EINVAL;
			fail(errno, "\"%s\"%s", BinFile,
			MSGSTR(ERRPACKED, 
			" is already packed "));

		}
	}

	if(unpack){

		if(packed){

			pbin = (struct aud_bin *)auditpack(AUDIT_UNPACK, 
			binbuf);

			if(errno){

				fail(errno, "%s\"%s\"",
				MSGSTR(ERRUNPACK, 
				"** auditpack failed unpacking "), 
				TrailFile);

			}
			bsz = pbin->bin_len + 2 * sizeof (struct aud_bin);
			bufp = (char *) pbin;
		}
		else {

			errno = EINVAL;
			fail(errno, "\"%s\"%s", BinFile,
			MSGSTR(ERRUNPACKED, 
			" is already unpacked")
			);
		}
	}
	
	while (bsz > 0) {

		int	cnt;

		cnt = (bsz < iosize) ? bsz : iosize;
		cnt = write(trail, bufp, cnt);
		if(cnt <= 0) {

			errno = EIO;
			fail(errno, "%s\"%s\"",
			MSGSTR(ERRWRITE, 
			"** failed writing to trail "), 
			TrailFile);

		}
		bsz -= cnt;
		bufp += cnt;
	}

	fsync (trail);
	exit(0);
}


binread(int fd){

	struct	aud_bin	bin;
	char	*bufp;
	int	bsz;
	extern	char	*malloc ();

	if(read (fd, &bin, sizeof (bin)) != sizeof (bin)){
		errno = EINVAL;

		fail(errno, "%s\"%s\"",
		MSGSTR(ERRBIN, 
		"** cannot open bin file "), 
		BinFile);

	}

	if((bin.bin_magic != AUDIT_MAGIC) || (bin.bin_tail != AUDIT_HEAD)){

		errno = EINVAL;
		fail(errno, "%s\"%s\"",
		MSGSTR(ERRBADBIN, 
		"** corrupted bin header in "), 
		BinFile);
	}

	if(bin.bin_plen && (bin.bin_plen != bin.bin_len)){

		packed++;
		binsize = bin.bin_plen + 2 * sizeof (bin);
	}
	else {

		binsize = bin.bin_len + 2 * sizeof (bin);

	}

	/* 
	 * Allocate buffer to hold the contents of the bin 
	 */

	binbuf = malloc(binsize);

	if(binbuf == NULL){

		fail(ENOMEM,
		MSGSTR(ERRNOMEM, 
		"** out of memory")
		);
	}

	/* 
	 * Read the contents of the bin into this buffer 
	 */

	bcopy (&bin, binbuf, sizeof (bin));
	bufp = &(binbuf[sizeof(bin)]);
	bsz = binsize - sizeof (bin);
	while(bsz > 0){
		int	cnt;

		cnt = (bsz < iosize) ? bsz : iosize;
		cnt = read (fd, bufp, cnt);
		if(cnt <= 0){

			errno = EINVAL;
			fail(errno, "%s\"%s\"",
			MSGSTR(ERRRDBIN, 
			"** cannot read contents of bin "), 
			BinFile);

		}
		bufp += cnt;
		bsz -= cnt;
	}
}


recover(int fd){

	struct	stat	stbuf;
	struct	aud_bin	bin;
	struct	aud_bin	bintrail;
	struct aud_bin *bin_t;
	ulong_t time;
	off_t	pos;

	/* 
	 * Verify that have bin header ...  
	 */

	if(read(fd, &bin, sizeof (bin)) != sizeof (bin)){

		errno = EINVAL;
		fail(EINVAL, "%s\"%s\"",
		MSGSTR(ERRBADBIN, 
		"** corrupted bin header in "), 
		BinFile);

	}

	if((bin.bin_magic != AUDIT_MAGIC) || (bin.bin_tail != AUDIT_HEAD)){

		fail(EINVAL, "%s\"%s\"",
		MSGSTR(ERRBADBIN, 
		"** corrupted bin header in "), 
		BinFile);

	}

	time = bin.bin_time;

	/* 
	 * Verify valid trail to append 
	 */

	if(strcmp(TrailFile, "STDOUT") != 0){

		/* 
		 * Go to beginning 
		 */

		lseek(trail, 0, SEEK_SET);

		if(read(trail, &bintrail, sizeof(bin)) != sizeof(bin)){

			errno = EINVAL;
			fail(errno, "%s\"%s\"",
			MSGSTR(ERRRDTRAIL, 
			"** cannot read trail file "), 
			TrailFile);

		}

		if((bintrail.bin_magic != AUDIT_MAGIC) || 
		(bintrail.bin_tail != AUDIT_HEAD)){

			errno = EINVAL;
			fail(errno, "%s\"%s\"",
			MSGSTR(ERRRDTRAIL, 
			"** cannot read trail file "), 
			TrailFile);

		}

		if(fstat(trail, &stbuf) < 0){

			errno = EACCES;
			fail(errno, "%s\"%s\"",
			MSGSTR(ERRSTAT, 
			"** cannot stat() "), 
			TrailFile);

		}

		/* 
		 * Audit info inside? 
		 */

		if(stbuf.st_size > (2 * sizeof (struct aud_bin))){

			pos = stbuf.st_size - sizeof (struct aud_bin);
			bread(trail, pos, &bintrail, sizeof(bin));

			if((bintrail.bin_magic != AUDIT_MAGIC) ||
			(bintrail.bin_tail != AUDIT_BIN_END)){

				fail(errno, "%s\"%s\"",
				MSGSTR(ERRRDTRAIL, 
				"** cannot read trail file "), 
				TrailFile);

			}
		}

		/* 
		 * Do not append to earlier trail 
		 */

		if(bin.bin_time < bintrail.bin_time){

			errno = EINVAL;
			fail(errno, "%s",
			MSGSTR(ERRTRAILTIME, 
			"** bin older than trail ")
			); 

		}
	}

	if(fstat(fd, &stbuf) < 0){

		errno = EACCES;
		fail(errno, "%s\"%s\"",
		MSGSTR(ERRSTAT, 
		"** cannot stat() "), 
		TrailFile);

	}

	if(stbuf.st_size <= (2 * sizeof (struct aud_bin))){

		errno = EINVAL;
		fail(errno, "%s\"%s\"",
		MSGSTR(ERRRDBIN, 
		"** cannot read contents of bin "), 
		BinFile);

	}

	/* 
	 * Read (possible) bin tail from end of file 
	 */

	pos = stbuf.st_size - sizeof (struct aud_bin);
	lseek(fd, -sizeof(struct aud_bin), SEEK_END);
	if(read(fd, &bin, sizeof(bin)) != sizeof (bin)){

		errno = EINVAL;
		fail(EINVAL, "%s\"%s\"",
		MSGSTR(ERRBADBINTAIL, 
		"** corrupted bin tailer in "), 
		BinFile);

	}

	if((bin.bin_magic != AUDIT_MAGIC) ||
	(bin.bin_tail != AUDIT_BIN_END)){

		int cnt;

		/* 
		 * Go to end 
		 */

		lseek(fd, 0, SEEK_END);

		bin.bin_magic = AUDIT_MAGIC;
		bin.bin_version = AUDIT_VERSION;
		bin.bin_tail = AUDIT_BIN_END;
		bin.bin_plen = 0;
		bin.bin_time = time;
		bin.bin_len = stbuf.st_size - sizeof(struct aud_bin);
		if((cnt = write(fd, &bin, sizeof(bin))) < sizeof(bin)){

			errno = EIO;
			fail(errno, "%s\"%s\"",
			MSGSTR(ERRWRITE, 
			"** failed writing to bin"), 
			BinFile);

		}

		lseek(fd, 0, SEEK_SET);

		bin.bin_magic = AUDIT_MAGIC;
		bin.bin_version = AUDIT_VERSION;
		bin.bin_tail = AUDIT_HEAD;
		bin.bin_plen = 0;
		bin.bin_time = time;
		bin.bin_len = stbuf.st_size - sizeof(struct aud_bin);
		if((cnt = write(fd, &bin, sizeof(bin))) < sizeof(bin)){

			errno = EIO;
			fail(errno, "%s\"%s\"",
			MSGSTR(ERRWRITE, 
			"** failed writing to trail "), 
			BinFile);

		}
	}

	/* 
	 * Return to beginning of bin 
	 */

	lseek(fd, 0, SEEK_SET);

	/* 
	 * Go to end of trail 
	 */

	lseek(trail, 0, SEEK_END);

	return;
}


/* 
 * Read routine for recover() 
 */

#define	BLKSIZ	4096
#define	BLKMASK	((BLKSIZ/2) - 1)

bread(int fd, off_t pos, char *buf, int len){
	static	off_t	blkpos = -1;
	static	char	blk[BLKSIZ];

	/* 
	 * Assure that the start of the required object
	 * falls in the first half of the buffer 
	 */

	if(blkpos != (pos & ~BLKMASK)){

		blkpos = pos & ~BLKMASK;
		lseek (fd, blkpos, 0);
		if(read (fd, blk, sizeof (blk)) <= 0){

			errno = EIO;
			fail(EIO, "%s\"%s\"",
			MSGSTR(ERRRDTRAIL, 
			"** cannot read trail file "), 
			TrailFile);

		}
	}

	bcopy (&(blk[pos & BLKMASK]), buf, len);
}

static int
fail (va_alist)
va_dcl {

	va_list	ap;
	char	*msg;
	int	doperror;
	static char ttydev[] = "/dev/tty";

	va_start (ap);
	doperror = va_arg(ap, int);
	msg = va_arg (ap, char *);

        if(isatty(STDERR_FILENO)){

                freopen(ttydev, "w", stderr);

        }

	fprintf(stderr, "auditcat: ");
	vfprintf(stderr, msg, ap);
	putc('\n',stderr);
	fflush(stderr);
	errno = doperror;
	if(doperror){

		perror ("auditcat");

	}

	exit(errno);
}
