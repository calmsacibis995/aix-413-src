static char sccsid[] = "@(#)36  1.14.2.7  src/bos/usr/sbin/fuser/fuser.c, cmdfs, bos411, 9428A410j 3/8/94 12:33:55";
/*
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: fuser
 *
 * ORIGINS: 3, 27, 83
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
 */
/*
 * LEVEL 1,5 Years Bull Confidential Information
 */

#include <nl_types.h>
#include "fuser_msg.h"
#include "locale.h"

#define	MSGSTR(N,S)	catgets(catd,MS_FUSER,N,S)
nl_catd catd;

#include <stdlib.h>
#include <unistd.h>
#include "fcntl.h"
#include "pwd.h"
#include "stdio.h"

#include "sys/param.h"
#include "sys/types.h"

#include "sys/dir.h"
#define _KERNEL
#include "sys/file.h"
#undef _KERNEL
#include "jfs/inode.h"
#include "sys/vnode.h"
#include "sys/syspest.h"

#include <procinfo.h>
#include <sys/thread.h>
#include <sys/uthread.h>
#include <sys/proc.h>

#include "sys/signal.h"
#include "sys/stat.h"
#include "sys/sysmacros.h"
#include "sys/var.h"

#define memf "/dev/mem"
#define FDSSIZE  sizeof(struct fdsinfo)
#define PROCSIZE sizeof(struct procsinfo)

/* Note that this macro is misnamed.  It doesn't just check to see if
   two inodes are equal.  It first tries that, then it checks to see if
   the first inode is the block device on which the second inode resides. */
#define ieq( X, Y) (((X.i_dev == Y.i_dev) && (X.i_number == Y.i_number)) || \
	( ((X.i_mode & IFMT) == IFBLK) && (X.i_rdev == Y.i_dev) ))

struct gnode *
vnode_to_gnode( vn_offset, mem)
struct vnode *vn_offset;
int mem;
{
	/* takes a pointer to a vnode and converts it to a */
	/* pointer to an gnode.				   */
	struct vnode vn;
	
	if (vn_offset == (struct vnode *)NULL) return ((struct gnode *)NULL);
	if (lseek (mem, (long)vn_offset, 0) < 0) 
		perror("lseek");
	if (read (mem, (char *)&vn, sizeof(struct vnode)) < 0)
		perror("read");

	return(vn.v_gnode);
}

struct inode *
vnode_to_inode( vn_offset, mem)
struct vnode *vn_offset;
int mem;
{
	/* takes a pointer to a vnode and converts it to a */
	/* pointer to an inode.				   */
	struct vnode vn;
	struct gnode gn;
	int first_time = 1;
	
      retry:
	if (vn_offset == (struct vnode *)NULL) return ((struct inode *)NULL);
	if (lseek (mem, (long)vn_offset, 0) < 0) 
		perror("lseek");
	if (read (mem, (char *)&vn, sizeof(struct vnode)) < 0)
		perror("read");

	/* Read in gnode to get inode * */
	if (lseek (mem, (long)vn.v_gnode, 0) < 0)
		perror("lseek");
	if (read (mem, (char *)&gn, sizeof(struct gnode)) < 0)
		perror("read");
	/* If this is a device or fifo, we want to look at the pfs vnode.
	 * Note that if it's a pipe, the type will be VFIFO but the pfsvnode
	 * will be NULL, which is fine since the user can't be asking about
	 * a pipe.
	 */
	if (first_time &&
	    (gn.gn_type == VCHR ||
	     gn.gn_type == VBLK ||
	     gn.gn_type == VMPC ||
	     gn.gn_type == VFIFO)) {
		vn_offset = vn.v_pfsvnode;
		first_time = 0;
		goto retry;
	}
	return((struct inode *)gn.gn_data);
}

/* note that the code in main that uses this for open local file
   pointers uses the fact that it returns the spec gnode rather
   than the pfs gnode in tracking mpx devices.  the pfs gnode for
   an mpx 1) says it's a character device rather than an mpx (it
   refers to the base mpx) and 2) doesn't have the channel (again,
   it's the base)

   this program should be completely rewritten */

struct gnode *
file_to_gnode(file_adr, memdev)
int memdev;
struct file *file_adr;
{	/* Takes a pointer to one of the system's file entries
	 * and returns the gnode to which it refers.
	 * "file_adr" is a system pointer, usually
	 * from the user area. */
	int mem;
	struct file f;
	struct gnode *gp;
	union /* used to convert pointers to numbers */
	{	char	*ptr;
		unsigned addr; /* must be same length as char * */
	} convert;

	convert.ptr = (char *) file_adr;
	if ( convert.addr == 0 ) return( NULL);
	if ( memdev >= 0 ) mem = memdev;
	else if( (mem = open( memf, O_RDONLY)) == -1 )
	{	perror( memf);
		return( NULL);
	}

	/* read the file table entry */
	rread( mem, (long) convert.addr, (char *) &f, sizeof f);
	if ( memdev != mem ) close( mem);
	if (f.f_type == DTYPE_VNODE) {
		if (( gp = vnode_to_gnode(f.f_vnode, mem)) == NULL)
			return (NULL);
		return (gp);
	}
	else return( NULL);
}

file_to_inode( file_adr, in, memdev)
int memdev;
struct inode *in;
struct file *file_adr;
{	/* Takes a pointer to one of the system's file entries
	 * and copies the inode  to which it refers into the area
	 * pointed to by "in". "file_adr" is a system pointer, usually
	 * from the user area. */
	int mem;
	struct file f;
	struct inode *ip;
	union /* used to convert pointers to numbers */
	{	char	*ptr;
		unsigned addr; /* must be same length as char * */
	} convert;

	convert.ptr = (char *) file_adr;
	if ( convert.addr == 0 ) return( -1);
	if ( memdev >= 0 ) mem = memdev;
	else if( (mem = open( memf, O_RDONLY)) == -1 )
	{	perror( memf);
		return( -1);
	}

	/* read the file table entry */
	rread( mem, (long) convert.addr, (char *) &f, sizeof f);
	if ( memdev != mem ) close( mem);
	if (f.f_type == DTYPE_VNODE) {
		if ((ip = vnode_to_inode(f.f_vnode, mem)) == NULL)
			return (-1);
		return (read_inode(ip, in, memdev));
	}
	else return( -1);
}

/***********************************************************************
 * NAME: getprocdata
 * FUNCTION: allocate and read in tables returned by call to getprocs().
 *
 * EXECUTION ENVIRONMENT: process only
 *
 * (NOTES): process exits if this routine fails
 *
 *
 * RETURNS: number of process table entries.
**************************************************************************/

int getprocdata (proc_array, fds_array)
struct procsinfo (**proc_array)[];       /* pointer to array of structs  */
struct fdsinfo   (**fds_array)[];        /* same as above                */
{
	pid_t   index = 0;
	int    procnum;                   /* number of process table entries  */
	int    proctabsiz, fdstabsiz;     /* maximum table sizes   */

	/* query for the number of processes - value returned by getprocs()  */
	if ((procnum = getprocs(NULL, PROCSIZE, NULL, FDSSIZE, &index,
						     INT_MAX)) == -1) {
	    fprintf(stderr, MSGSTR(GETPROCER, "Can't get process structure\n"));
	    exit(1);
	}
	else {
	    /* Allocate more space than necessary. Of course, it is always
	     * possible that more processes will be created following the
	     * 2nd call to getprocs(). Also, getprocs() can not guarantee
	     * a consistent proc table image is returned.
	     */
	    procnum *= 1.1;        /* max number of entries for procsinfo[] */
	    proctabsiz = procnum * PROCSIZE; /* sort of like this in 3.2.5 */
	   index = PROCSIZE;
	    /* Grab space to hold proc table data for all processes     */
	    *proc_array = (struct procsinfo (*) []) malloc((size_t) proctabsiz);
	    if ( *proc_array == NULL) {    /* Out of space*/
		fprintf(stderr, MSGSTR(MALLOC,"getprocdata: malloc failed\n"));
		exit (1);
	    }

	    fdstabsiz = procnum * FDSSIZE;
	    /* Grab space for all file descriptors used by all processes */
	    *fds_array = (struct fdsinfo (*) []) malloc((size_t) fdstabsiz);
	    if ( *fds_array == NULL) {    /* Out of space*/
		fprintf(stderr, MSGSTR(MALLOC,"getprocdata: malloc failed\n"));
		exit (1);
	    }
	}

	index = 0;
	/* retrieve all proc data and fd data   */
	if ((procnum = getprocs(*proc_array, PROCSIZE, *fds_array, FDSSIZE,
						  &index, procnum)) == -1) {
	    fprintf(stderr, MSGSTR(GETPROCER, "Can't get process structure\n"));
	    exit(1);
	}

	return(procnum);
} /* end getprocdata    */
/***********************************************************************/

main( argc, argv)
int argc;
char **argv;
{	int mem, file = 0, gun = 0, uname = 0;
	register i, j, k;
	struct inode ina, inb, *ip;
	struct procsinfo proc_info;
	long proctabsiz, totalproc;
	int c, fuser_pid;
	int remote = 0;
	int rfd;
	struct gnode *gp, *user_gp, gn;
	extern int optind;
	int exitcode;

	struct procsinfo (*proc_array)[];
	struct procsinfo fuser_proc;
	struct fdsinfo   (*fds_array)[];
	struct fdsinfo   fuser_fds;

	(void) setlocale(LC_ALL, "");

	catd = catopen(MF_FUSER, NL_CAT_LOCALE);

	if ( argc < 2 )
	{
		fprintf(stderr, MSGSTR(USAGE, "Usage:  %s [-ku] File ...\n"),
			argv[0]);
		exit(1);
	}

	/* open file to access memory */
	if ( (mem = open( memf, O_RDONLY)) == -1)
	{       fprintf( stderr, "%s: ", argv[0]);
		perror( memf);
		exit( 1);
	}

	/* For each of the arguments in the command string */
	while ((c = getopt(argc, argv, "ku")) != EOF)
		switch(c) {
			case 'k':
				++gun;
				break;
			case 'u':
				++uname;
				break;
			default:
				fprintf(stderr, MSGSTR(ILLOPT, "Illegal option %c ignored.\n"), c);
				break;
		}
	
	/*
	 * now that we have the kernel symbols, we must lower our privilege.
	 */
	seteuid(getuid());
	totalproc = getprocdata(&proc_array, &fds_array);
	fuser_pid = getpid();
	/* now find ourselves in the proc table. */
	for (i = 0; i < totalproc; i++) {
	    if (fuser_pid == (*proc_array)[i].pi_pid) break;
	}
	if (i == totalproc) {
		fprintf(stderr, MSGSTR(FINDSELFERR, "Cannot find fuser in process table.\n"));
		exit(1);
	}

	if (optind >= argc)
	{
		fprintf(stderr, MSGSTR(USAGE, "Usage:  %s [-ku] File ...\n"), argv[0]);
		exit(1);
	}

	remote = exitcode = 0;
	for (; optind < argc; optind++) {
		/* First print its name on stderr (so the output can
		 * be piped to kill) */
		fprintf( stderr, "%s: ", argv[optind]);
		/* then convert the path into an inode */
		if (path_to_inode( argv[optind], &remote, &ina) == -1)
		{
			exitcode++;
			continue;
		}
		if (remote) {
			rfd = open(argv[optind], O_RDONLY|O_NDELAY);
			if (rfd < 0) {
				fprintf(stderr, MSGSTR(OPENERR, "Cannot find or open %s.\n"), argv[optind]);
				exit(1);
			}

			/* read file table for fuser file information           */
			if (getprocs(&fuser_proc, PROCSIZE, &fuser_fds,
					 FDSSIZE, &fuser_pid, 1) == -1) {
			    fprintf(stderr, MSGSTR(GETPROCER, "Can't get process structure\n"));
			    exit(1);
			}

			/* get gnode from the fuser file descriptor table */
			if ((user_gp = file_to_gnode(fuser_fds.pi_ufd[rfd].fp, mem)) == NULL)
				continue;
		}

		for ( j = 0; j < totalproc; j++)
		{
			/* ignore defunct processes */
			if ((*proc_array)[j].pi_state == SZOMB)
				continue;

			if ((*proc_array)[j].pi_pid == fuser_pid)
				continue;

			if (remote) {
			    gp = vnode_to_gnode((struct vnode *)(*proc_array)[j].pi_cdir,
										    mem);
			    if (gp != NULL) {
				if (gp == user_gp) {
				    fprintf( stdout, " %7d", (int) (*proc_array)[j].pi_pid);
				    (void) fflush(stdout);
				    fprintf( stderr, "c");
				    if (uname) puname( (int) (*proc_array)[j].pi_uid);
				    if (gun) kill( (int) (*proc_array)[j].pi_pid, 9);
				    continue;
				}
			    }
			    gp = vnode_to_gnode((struct vnode *)(*proc_array)[j].pi_rdir,
										    mem);
			    if (gp != NULL) {
				if (gp == user_gp) {
				    fprintf( stdout, " %7d",(int)(*proc_array)[j].pi_pid);
				    (void) fflush(stdout);
				    fprintf( stderr, "r");
				    if (uname) puname( (int)(*proc_array)[j].pi_uid);
				    if (gun) kill( (int) (*proc_array)[j].pi_pid, 9);
				    continue;
				}
			    }
			    /* then, for each file */
			    for ( k = 0; k < (*proc_array)[j].pi_maxofile; k++) {
				/* check if it is the fs being checked */
				if ((*fds_array)[j].pi_ufd[k].fp == NULL) continue;
				if ((gp = file_to_gnode( (*fds_array)[j].pi_ufd[k].fp,
							      mem)) == NULL) continue;
				if (gp == user_gp)
				{
				    fprintf(stdout, " %7d",(int)(*proc_array)[j].pi_pid);
				    (void) fflush(stdout);
				    if (uname) puname((int)(*proc_array)[j].pi_uid);
				    if (gun) kill((int)(*proc_array)[j].pi_pid, 9);
				    break;
				}
			    }
			    close(rfd);
			}
			else {  /* not remote   */
			    ip = vnode_to_inode((*proc_array)[j].pi_cdir, mem);
			    if ( ip != NULL && read_inode( ip, &inb, mem) == 0 ) {
				if ( ieq( ina, inb) ) {
				    fprintf( stdout, " %7d", (int) (*proc_array)[j].pi_pid);
				    (void) fflush(stdout);
				    fprintf( stderr, "c");
				    if (uname) puname( (int) (*proc_array)[j].pi_uid);
				    if (gun) kill( (int) (*proc_array)[j].pi_pid, 9);
				    continue;
				}
			    }
	
			    ip = vnode_to_inode((*proc_array)[j].pi_rdir, mem );
			    if ( ip != NULL && read_inode( ip, &inb, mem) == 0 ) {
				if ( ieq( ina, inb) ) {
				    fprintf( stdout, " %7d", (int) (*proc_array)[j].pi_pid);
				    (void) fflush(stdout);
				    fprintf( stderr, "r");
				    if (uname) puname( (int) (*proc_array)[j].pi_uid);
				    if (gun) kill( (int) (*proc_array)[j].pi_pid, 9);
				    continue;
				}
			    }

			    /* then, for each file */
			    for ( k = 0; k < (*proc_array)[j].pi_maxofile; k++) {
				/* check if it is the fs being checked */
				if ((*fds_array)[j].pi_ufd[k].fp == NULL) continue;
				if ((gp = file_to_gnode((*fds_array)[j].pi_ufd[k].fp,
							mem)) == NULL) continue;
				/* Read in gnode */
				if (lseek(mem, (long)gp, 0) < 0) {
				    perror("lseek");
				    continue;
				}
				if (read(mem, (char *)&gn, sizeof(struct gnode)) < 0) {
				    perror("read");
				    continue;
				}
				/* if it's an mpx device we want to
				   look at the spec gnode,
				   not the pfs gnode since the pfs
				   gnode doesn't have the channel */
				if (gn.gn_type == VMPC)
				    if ((ina.i_rdev == gn.gn_rdev) &&
						(ina.i_gnode.gn_chan == gn.gn_chan)) {
					fprintf(stdout, " %7d", (int) (*proc_array)[j].pi_pid);
					(void)fflush(stdout);
					if (uname) puname((int)(*proc_array)[j].pi_uid);
					if (gun) kill((int)(*proc_array)[j].pi_pid, 9);
					break;
				    } else
					/* mpx, but no match */
					continue;
					
				/* not an mpx device */
				if (file_to_inode( (*fds_array)[j].pi_ufd[k].fp, &inb,
								   mem)) continue;

				if ( ieq( ina, inb) ) {
				    fprintf(stdout, " %7d", (int) (*proc_array)[j].pi_pid);
				    (void) fflush(stdout);
				    if (uname) puname( (int) (*proc_array)[j].pi_uid);
				    if (gun) kill( (int) (*proc_array)[j].pi_pid, 9);
				    break;
				}
			    }
			} /* end file not remote        */
		}

		fprintf( stderr, "\n");
	}
	printf( "\n");
	return exitcode ? 1 : 0;
	/* NOTREACHED */
}

path_to_inode( path, rem, in)
char *path;
int *rem;
struct inode *in;
{	/* Converts a path to inode info by the stat system call */

	char path1[PATH_MAX], path2[PATH_MAX];
	struct stat s;
	struct stat ls;
	char *basename();
	int loopcount = 0;

	if ( statx( path, &s, 0, STX_NORMAL) == -1 )
	{	perror( "");
		return( -1);
	}
	if (s.st_flag & FS_REMOTE)
		*rem = 1;
	else *rem = 0;
	in->i_mode = s.st_mode;
	in->i_dev = s.st_dev;
	in->i_number = s.st_ino;
	in->i_rdev = s.st_rdev;
	/* if it's an mpx device, note the channel since we'll need this to
	   compare with the open files. */
	if (S_ISMPX(in->i_mode)) {
		/* this is really ugly, but stat() doesn't return the
		   channel, so we have to get it from the real path */
		/* copy the path to a local copy we can mess with */
		strcpy(path1, path);
		while (1) {
			/* let's not run forever */
			if (loopcount++ > MAXSYMLINKS)
				return -1;
			if (lstat(path1, &ls) == -1)
				return -1;
			/* stop when we find something that's not a symlink */
			if (!S_ISLNK(ls.st_mode))
				break;
			/* get what the link points to */
			if (readlink(path1, path2, PATH_MAX) == -1)
				return -1;
			/* stash the path back into our working copy
			   and start the loop over */
			strcpy(path1, path2);
		}
		/* /dev/pts/0 should not match /dev/pts, but if /dev/pts is open
		   the inode number will be filled in, so the match will succeed
		   without looking at the channel
		   but since it doesn't make sense for the base mpx to be open
		   we don't have to worry about that */
		if (!isdigit(*(basename(path1))))
			in->i_gnode.gn_chan = -1;
		else
			in->i_gnode.gn_chan = atoi(basename(path1));
	}
	return( 0);
}

puname( uid)
int uid;
{	struct passwd *p;

	p = getpwuid((uid_t)uid);
	if ( p == NULL ) return;
	fprintf( stderr, "(%s)", p->pw_name);
}

read_inode( inode_adr, i, memdev)
int memdev;
struct inode *i, *inode_adr;
{	/* Takes a pointer to one of the system's inode entries
	 * and reads the inode into the structure pointed to by "i" */

	int mem;
	union /* used to convert pointers to numbers */
	{	char	*ptr;
		unsigned addr; /* must be same length as char * */
	} convert;

	convert.ptr = (char *) inode_adr;
	if ( convert.addr == 0 ) return( -1);
	if ( memdev >= 0 ) mem = memdev;
	else if( (mem = open( memf, O_RDONLY)) == -1 )
	{	perror( memf);
		return( -1);
	}

	/* read the inode */
	rread( mem, (long) convert.addr, (char *) i, sizeof (struct inode));
	if ( memdev != mem ) close( mem);
	/* removes bits to distinguish small and large FS */
	i->i_dev = brdev(i->i_dev);
	i->i_rdev = brdev(i->i_rdev);
	if(i->i_count == 0)
		return(-1);
	return( 0);
}

/* reads in the inode   */
rread( device, position, buffer, count)
char *buffer;
int count, device;
long position;
{	/* Seeks to "position" on device "device" and reads "count"
	 * bytes into "buffer". Zeroes out the buffer on errors.
	 */
	int i;

	if ( lseek( device, position, 0) == (long) -1 )
	{	fprintf( stderr, MSGSTR(SEEKERR, "Seek error for file number %d: "), device);
		perror( "");
		for ( i = 0; i < count; buffer++, i++) *buffer = '\0';
		return;
	}
	if ( read( device, buffer, (unsigned) count) == -1 )
	{	fprintf( stderr, MSGSTR(READERR, "Read error for file number %d: "), device);
		perror( "");
		for ( i = 0; i < count; buffer++, i++) *buffer = '\0';
	}
}

