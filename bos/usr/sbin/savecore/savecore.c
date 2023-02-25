static char sccsid[] = "@(#)49        1.5.1.7  src/bos/usr/sbin/savecore/savecore.c, cmddump, bos411, 9428A410j 3/22/94 14:25:27";
/*
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
 *
 * FUNCTIONS: savecore
 *
 * ORIGINS: 26, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989,1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 * Copyright (c) 1980,1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * NAME: savecore
 * FUNCTION: save a core dump of the operating system
 *  Savecore is meant to be called near the end of the /etc/rc file
 */

#include <stdio.h>
#include <locale.h>
#include <errno.h>
#include <ras.h>
#include <sys/param.h>
#include <sys/limits.h>
#include <sys/dir.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/syslog.h>
#include <sys/dump.h>
#include <sys/vnode.h>
#include <savecr_msg.h> 

#define MCS_CATALOG "savecr.cat"
nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_SAVECR,n,s) 

#define	DAY	(60L*60L*24L)
#define	LEEWAY	(3*DAY)

#define eq(a,b) (!strcmp(a,b))
#define KILO	1024
#define	BUFPAGES	(16*KILO/NBPG)		/* 16K */
#define SYSDUMP	"/dev/sysdump"
#define SYSNAME	"/unix"
#define COPYFILENAME	"/var/adm/ras/copyfilename"

struct dumpinfo dmp;			/* dump information */
char    *sysname;			/* optional system name */
int	dumplo = 0;			/* where dump starts on dumpdevice */
int	dumpmag = DMP_MAGIC;		/* magic number in dump */
int dumpfd;

off_t	Lseek();
int getdumpinfo(struct dumpinfo *);
int check_dump();
char * path(char *);
int check_space();
int read_number(char *);
int save_dump();
void remove_old_dumps();


static int  copycore = 0;
static int  markinvalid = 0;
static int  forcecopy = 0;
static char *dir_name;			/* directory to save dumps in */

main(argc, argv)
	char **argv;
	int argc;
{
	char *cp;
	int oerrno;
	int c;
	int rv;
	extern optind;
	extern char *optarg;
	struct stat statbuf;

        setlocale(LC_ALL,"");                           /* set Locale */

        /* Set global Progname for cat_* functions */
        setprogname();

	catd = catopen(MF_SAVECR, NL_CAT_LOCALE);


        catinit(MCS_CATALOG);                /* init message catalog */

        while((c = getopt(argc,argv,"cfd")) != EOF) {
                switch(c) {
                case 'd':
			copycore++;
                        break;
                case 'c':
                        markinvalid++;
                        break;
                case 'f':
                        forcecopy++;
                        break;
                default:
                	usage();
                }
        }

/* if both -c and -f are specifed, -f is ignored */

	if (forcecopy && markinvalid)
		forcecopy=0;

/* testing the input arguments */

        if (argc <= 1) /* savecore without any arguments */
                usage();  
        if (argc == 2) /* savecore with one argument */
	 {
	      if ((!markinvalid) && (optind == 2)) /* directory not specified */
                 usage(); 
        }
        else /* argc > 2 */
	  {
                if (argc == optind) /*  directory not specified */   
                 usage(); 
	   }
	if (!markinvalid) /* check only if - c flag is not specified */
	 {
	   if (( optind + 1) == argc) /* only directory is specified */
	    {
	      sysname = SYSNAME;
              dir_name = argv[argc-1];
	     }
	    else /* both directory and the optional [system] are specified */
	     {
	    	sysname = argv[argc-1];
		dir_name = argv[argc-2];
	     }
	   }
	/* Initialize the syslog */
	openlog("savecore", LOG_ODELAY, LOG_AUTH);
	if (!markinvalid)
	{
          if (stat(dir_name,&statbuf) < 0)
          {
		oerrno = errno;
		perror(dir_name);
		errno = oerrno;
		syslog(LOG_ERR, "%s: %m", dir_name);
                exit(-1);
          }
	  /*******************************************************
	   check to see if the input file is a directory
	  ********************************************************/
          if (statbuf.st_type != VDIR) {
            cat_fatal(CAT_NOT_A_DIR,"%s is not a directory.\n",dir_name); 
		exit(-1);
	  }

	  /*******************************************************
   	   check whether the specified directory can be accessed
	  ********************************************************/

	  if (access(dir_name, W_OK) < 0) {
		oerrno = errno;
		perror(dir_name);
		errno = oerrno;
		syslog(LOG_ERR, "%s: %m", dir_name);
		exit(1);
	  }
	}
	/*******************************************************
   	 Get the file descriptor for /dev/sysdump
	********************************************************/
	dumpfd = Open(SYSDUMP, 0);


	/*******************************************************
   	 Get the information about the dump.
	 If unable to get the dump information or no previous
	 dump exists then exit with a non-zero return code.
	********************************************************/
	if (rv = getdumpinfo(&dmp)) {
		close(dumpfd);
		exit(rv);
	}

	/*******************************************************
	 Checks to see if a new core dump really exists and
	 it is a valid dump.
	********************************************************/

	if (rv = check_dump()) {
		close(dumpfd);
		exit(rv);
	}

	/*******************************************************
	 Remove old dumps so that the directory does not run 
	 out of space.  We will leave the last dump and delete
	 all others.
        ********************************************************/

	remove_old_dumps();
	
	/*******************************************************
	 Check to see if there is enough available space left on the
    	 current device for the new core dump.
	********************************************************/

	if (rv = check_space()) {
		close(dumpfd);
		exit(rv);
	}

	/*******************************************************
 	 save the new dump in dir_name/core.n (where n is a number)
	********************************************************/

	save_dump();
	/* syslog(LOG_CRIT,MSGSTR(REBOOT, "reboot"));  we don't want to do this*/
	close(dumpfd);
	exit(0);
}


/*
 * NAME: usage
 * FUNCTION: display usage statement
 */
usage()
{
        cat_eprint(USAGE,
            "Usage:\nsavecore [-cfd] Directory [System]\n\n\
Save a core dump of the system and the kernel.\n\
-c 	Does not copy the dump, but marks the dump as invalid\n\
-f 	Copies the dump regardless of the validity\n\
-d 	Copies only the dump\n");

        exit(1);
}

/*
 * NAME: getdumpinfo
 * 
 * FUNCTION: get dump information from the kernel by calling ioctl
 */
int getdumpinfo(struct dumpinfo *dmp)
{
	int fd;
	

	if (ioctl(dumpfd,DMP_IOCINFO,dmp) == -1) {
		perror("savecore: ioctl");
		return(1);
	}


	if (dmp->dm_size <= 0 && dmp->dm_devicename[0] == '\0') /* check for new dump */
		{
		cat_eprint(NO_DUMP,
		"No dump available to copy.\n");
		return(1);
		}

	return(0);
}

/*
 * NAME: check_dump
 * FUNCTION:
 *   Checks to see if a new core dump really exists and it is a valid dump.
 */
int check_dump()
{
	register int ifd; 
	int word;
	struct stat *status;


/* if it is a force copy there is no need to check previous dump */

	if (forcecopy)
		return(0); 

	if (markinvalid)
	{
		bzero(&dmp, sizeof(dmp)); 
        	if (0 > ioctl(dumpfd, DMP_IOCSTAT2, &dmp))
		{
			perror("savecore: ioctl(DMP_IOCSTAT2)");
                	return(1);
		}
		cat_print(MARK_INVALID, 
		"Dump is marked invalid and is not copied.\n");
		close(dumpfd);
		exit(0);
	}
	/* If the dump is already copied(i.e. dump is marked invalid),
	   don't copy it unless -f is used				*/
	if ( !(dmp.dm_flags & DMPFL_NEEDCOPY))
	{
		cat_eprint(NO_RECENT_DUMP,
		"Dump was marked invalid. Use -f to force copy.\n");
		return(1);
	}
	/*******************************************************
	 Verify to see if there is a dump in the device.
	*******************************************************/

	ifd = Open(dmp.dm_devicename, O_RDONLY);

	Lseek(ifd,dumplo,L_SET);
	Read(ifd, (char *)&word, sizeof (word));
	/* The reason for checking the magic number 
	   at the beginning of the dump and the first word
	   of the second block, is that in some cases,
	   the first block is used by LVM.
        */
	if (word != dumpmag) {
	        Lseek(ifd,512,L_SET);
	        Read(ifd, (char *)&word, sizeof (word));
	}
	close(ifd);
	if (word != dumpmag) {
		cat_eprint(BADMAGIC,
			"The dump has a bad magic number\n");
		return(-1);
	}
	return(0); 
}

/*
 * NAME: path
 * FUNCTION: return a string with the full path name for the 
 *   file.
 */
char *
path(file)
char *file;
{
	char *cp;

	cp = (char *)malloc((size_t)(strlen(file) + strlen(dir_name) + 2));
	if (cp == 0) {
                cat_fatal(NOALLOC,
                        "There is not enough memory now for I/O buffers.\n");
        }

	(void) strcpy(cp, dir_name);
	(void) strcat(cp, "/");
	(void) strcat(cp, file);
	return (cp);
}

/*
 * NAME: check_space
 * FUNCTION:  Check to see if there is enough available space left on the
 *    current device for the new core dump.
 */
int check_space()
{
	struct statfs dsb;
	struct stat statbuf1;
	register char *ddev;
	int dfd, spacefree;
	int oerrno;
	off_t space_size;
	int minfree;

	if (statfs(dir_name, &dsb) < 0) {
		oerrno = errno;
		perror(dir_name);
		errno = oerrno;
		syslog(LOG_ERR, "%s: %m", dir_name);
		return(-1);	
	}
	
 	spacefree = (dsb.f_bfree * dsb.f_bsize) / KILO;
	if (copycore)
	  space_size = dmp.dm_size;
	else
	{
	    if (stat(sysname, &statbuf1) < 0) {
		oerrno = errno;
		perror(sysname);
		errno = oerrno;
		syslog(LOG_ERR, "%s: %m", sysname);
		return(-1);	
	    }
	  space_size = dmp.dm_size + statbuf1.st_size;
	}

	minfree = read_number("minfree");    

 	if ((spacefree < minfree) ||
		 (spacefree < ((space_size / KILO) + minfree)))
	{
	  if (copycore)
	   {
		cat_eprint(NO_SPACE2,"Not enough space on %s to copy the dump.\n",dir_name); 
		syslog(LOG_WARNING,MSGSTR(NO_SPACE2,"Not enough space on %s to copy the dump.\n"),dir_name);
		return (-1);
	   }
	  else
	   {
		cat_eprint(NO_SPACE1,"Not enough space on %s to copy the dump and the %s.\n",dir_name,sysname); 
		syslog(LOG_WARNING,MSGSTR(NO_SPACE1,"Not enough space on %s to copy the dump and the %s\n"),dir_name,sysname);
		return (-1);
	   }
	}
	return (0);
}

/*
 * NAME: read_number
 * FUNCTION: read a number from the file fn
 */
int read_number(fn)
	char *fn;
{
	char lin[MAX_INPUT];
	register FILE *fp;

	fp = fopen(path(fn), "r");
	if (fp == NULL)
		return (0);
	if (fgets(lin, MAX_INPUT, fp) == NULL) {
		fclose(fp);
		return (0);
	}
	fclose(fp);
	return (atoi(lin));
}

/*
 * NAME: save_dump
 * FUNCTION: save the new dump in dir_name/core.n (where n is a number)
 */
int save_dump()
{
	register int n;
	register int m;
	register char *cp;
	register char *wkptr;
	register int ifd, ofd, bounds;
	register FILE *fp;
	char *corename;
	char *sys_name;
	char buffer[BUFSIZ];
	int num;
	int count;

	cp = (char *)malloc(BUFSIZ);
	if (cp == 0) {
		cat_fatal(NOALLOC,
			"There is not enough memory now for I/O buffers.\n");
	}
	bounds = read_number("bounds");
	/* save the kernel */
	if (!copycore) /* if -d flag is not specified */
	{
		ifd = Open(sysname, O_RDONLY);
		sprintf(cp, "%s.%d","vmunix", bounds);
		sys_name = path(cp);
		ofd = Create(sys_name, 0644);
		cat_print(SYS_BYTES,"Saving %s in %s\n",
			sysname,sys_name);
		while((n = Read(ifd, cp, BUFSIZ)) > 0)
			Write(ofd, cp, n);
		close(ifd);
		close(ofd);
	}

	/* copy the dump */
	ifd = Open(dmp.dm_devicename, O_RDONLY);
	sprintf(cp, "vmcore.%d", bounds);
	corename = path(cp);
	ofd = Create(corename, 0644);
	Lseek(ifd, (off_t)dumplo, L_SET);
	cat_print(BYTES,"Saving %d bytes of system dump in %s\n",
			dmp.dm_size,corename);
	syslog(LOG_NOTICE,MSGSTR(BYTES, 
		"Saving %d bytes of system dump in %s\n"),dmp.dm_size,corename);
	count = 0;
	while((m = Read(ifd,cp,BUFSIZ)) > 0)	
	{
		Write(ofd, cp, m);
		count += m;
		if (count >= dmp.dm_size)
		  break;
	}
		
	close(ifd);
	close(ofd);
	/* mark the dump invalid */
       	ioctl(dumpfd, DMP_IOCSTAT2, &dmp);
	/* Save the file name to /var/adm/ras/copyfilename */
	ofd = Open(COPYFILENAME,(O_WRONLY | O_CREAT | O_TRUNC));
	Write(ofd,corename,strlen(corename));
	close(ofd);
	/* Update the bound */	
	fp = fopen(path("bounds"), "w");
	fprintf(fp, "%d\n", bounds+1); /* increment of bounds */
	fclose(fp);
	free((void *)cp);
	return(0);
}

/*
 * Open the file with specified mode.
 */
Open(name, rw)
	char *name;
	int rw;
{
	int fd;
	int oerrno;

	fd = open(name, rw);
	if (fd < 0) {
	        oerrno = errno;
		perror(name);
		errno = oerrno;
		syslog(LOG_ERR, "%s: %m", name);
		exit(1);
	}
	return (fd);
}

Read(fd, buff, size)
	int fd, size;
	char *buff;
{
	int ret;
	int oerrno;

	ret = read(fd, buff, size);
	if (ret < 0) {
		oerrno = errno;
		perror("read");
		errno = oerrno;
		syslog(LOG_ERR, "read: %m");
		exit(1);
	}
	return (ret);
}

off_t
Lseek(fd, off, flag)
	int fd, flag;
	long off;
{
	long ret;
	int oerrno;

	ret = lseek(fd, off, flag);
	if (ret == -1) {
		oerrno = errno;
		perror("lseek");
		errno = oerrno;
		syslog(LOG_ERR, "lseek: %m");
		exit(1);
	}
	return (ret);
}

Create(file, mode)
	char *file;
	int mode;
{
	register int fd;
	int oerrno;

	fd = creat(file, (mode_t)mode);
	if (fd < 0) {
		oerrno = errno;
		perror(file);
		errno = oerrno;
		syslog(LOG_ERR, "%s: %m", file);
		exit(1);
	}
	return (fd);
}

Write(fd, buf, size)
	int fd, size;
	char *buf;
{
	int oerrno;

	if (write(fd, buf, size) < size) {
		oerrno = errno;
		perror("write");
		errno = oerrno;
		syslog(LOG_ERR, "write: %m");
		exit(1);
	}
}

void
remove_old_dumps()
{
register int old_bounds;
register int current_bounds;
char buffer[BUFSIZ];

	/* If there is no bounds file, then this is the first dump. */
	current_bounds = read_number("bounds");
	if ( current_bounds == 0 )
		return;
	
	old_bounds = current_bounds - 1;

	/* Make a backup of the last dump taken. */ 
	sprintf(buffer, "mv %s/vmcore.%d %s/old_vmcore.%d > /dev/null 2>&1", dir_name, old_bounds, dir_name, old_bounds);

	system(buffer);
	
	/* Remove all dumps. */
	bzero(buffer,sizeof(buffer));
	sprintf(buffer, "rm -f %s/vmcore* >/dev/null 2>&1", dir_name);

	system(buffer);

	/* Copy the last dump back to it's original name. */
	sprintf(buffer, "mv %s/old_vmcore.%d %s/vmcore.%d > /dev/null 2>&1", dir_name, old_bounds, dir_name, old_bounds);

	system(buffer);
}
