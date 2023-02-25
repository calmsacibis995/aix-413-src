static char sccsid[] = "@(#)10	1.3  src/bos/usr/sbin/install/inu_tape_dd/inu_tape_dd.c, cmdinstl, bos411, 9428A410j 6/17/94 10:45:36";
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 *   FUNCTIONS: build_file
 *		inu_signal
 *		main
 *		next_tape_position
 *		position_tape
 *		rewind_one_pos
 *		sighandle
 *		unlink_file
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <stdio.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/tape.h>
#include <sys/ioctl.h>
#include <sys/devinfo.h>
#include <fcntl.h>
#include <sys/limits.h>
#include <stdlib.h>

static int fd;
static int hit_tape_mark=0;
static int sig_interupted=0;
static int rewind_one_file=0;

static void inu_signal(int sig, void (* funky)())
{
        struct sigaction action;
        bzero(&action,sizeof(action));
        action.sa_handler=funky;
        sigaction(sig,&action,(struct sigaction *)NULL);
}

static void sighandle()
{
	sig_interupted=1;
}


static unlink_file (char * inu_temp_dir, char * file_name)
{
	char file_path[PATH_MAX+1];
	sprintf(file_path,"%s/%s",inu_temp_dir,file_name);
	unlink(file_path);
	
}

static build_file (char * inu_temp_dir, char * file_name, int count)
{
	char file_path[PATH_MAX+1];
	FILE *fd;
	
	sprintf(file_path,"%s/%s",inu_temp_dir,file_name);

	fd=fopen(file_path,"w");
	
	/* we are counting on installp to handle this error
	   should it ever occur
	 */
	if(fd==(FILE *)NULL)
		return;
	fprintf(fd,"%d",count);
	fclose(fd);
}


static void rewind_one_pos()
{
	struct stop targ;
	int rc;
	
	targ.st_op = STRSF;
	targ.st_count = 1 + hit_tape_mark;
   	ioctl(fd, STIOCTOP, &targ);
	targ.st_op = STFSF;
	targ.st_count = 1;
   	ioctl(fd, STIOCTOP, &targ);
	close(fd);
	exit(0);
}

static void next_tape_position()
{
	struct stop targ;
	int rc;
	int rewind_tape= -1;
	int current_tape_position= -1;
	int next_tape_position= -1;
	char inu_temp_dir_path[PATH_MAX+1];
	char *spam;


	spam=getenv("INU_POSITION_TAPE");
	if(spam==NULL || atoi(spam)!=1)
	{
		/* skip to the next file? */
		if(hit_tape_mark==0)
		{
			targ.st_op = STFSF;
			targ.st_count = 1;
   			ioctl(fd, STIOCTOP, &targ);
		}
		close(fd);
		exit(0);
	}

	spam=getenv("INUTEMPDIR");

	/* if there is no INUTEMPDIR then we can't tell installp
	 * about what we want to do so do nothing.
	 */
	if(spam==NULL)
	{
		/* skip to the next file? */
		if(hit_tape_mark==0)
		{
			targ.st_op = STFSF;
			targ.st_count = 1;
   			ioctl(fd, STIOCTOP, &targ);
		}
		close(fd);
		exit(0);
	}

	strcpy(inu_temp_dir_path,spam);

	unlink_file(inu_temp_dir_path,"INU_TAPE_REWIND_DONE");
	unlink_file(inu_temp_dir_path,"INU_NEW_TAPE_POSITION");
	unlink_file(inu_temp_dir_path,"UNKNOWN_TAPE_POSITION");

	spam=getenv("INU_REWIND_TAPE");
	if(spam!=NULL)
		rewind_tape=atoi(spam);

	/* what is the current tape position? */
	spam=getenv("INU_CURRENT_TAPE_POSITION");
	if(spam!=NULL)
		current_tape_position=atoi(spam);


	/* what is the next tape position? */
	spam=getenv("INU_NEXT_TAPE_POSITION");
	if(spam!=NULL)
		next_tape_position=atoi(spam);

	
	/* don't try and position the tape on the next image? */
	if(next_tape_position== -1 || rewind_tape== -1 || 
		(next_tape_position == (current_tape_position + 1)))
	{
		/* skip to the next file? */
		if(hit_tape_mark==0)
		{
			targ.st_op = STFSF;
			targ.st_count = 1;
   			ioctl(fd, STIOCTOP, &targ);
		}
		close(fd);
		exit(0);
	}


	if(next_tape_position <= current_tape_position)
		rewind_tape=1;

	/* rewind the tape? */
	if(rewind_tape==1)
	{
		targ.st_op = STREW;
		targ.st_count = 1;
   		if(ioctl(fd, STIOCTOP, &targ)!=0)
		{
			build_file(inu_temp_dir_path,"UNKNOWN_TAPE_POSITION",0);
			exit(1);
		}
		current_tape_position=0;
		hit_tape_mark=1;
		/* leave the tape at the begining? */
		if(next_tape_position==0)
			hit_tape_mark=0;
	}
	
	targ.st_op = STFSF;
	targ.st_count = next_tape_position - current_tape_position - hit_tape_mark;

	/* leave the tape alone? */
	if(targ.st_count > 0)
   		if(ioctl(fd, STIOCTOP, &targ) != 0)
		{
			build_file(inu_temp_dir_path,"UNKNOWN_TAPE_POSITION",0);
			exit(1);
		}

	close(fd);
	
	if (rewind_tape)
		build_file(inu_temp_dir_path,"INU_TAPE_REWIND_DONE",0);
	build_file(inu_temp_dir_path,"INU_NEW_TAPE_POSITION",next_tape_position);

	exit(0);
}

static void position_tape()
{
	inu_signal(SIGPIPE,SIG_IGN);
	
	/* we close these to close all pipes so we can continue
	   with out affecting anybody else
	 */
	close(0);
	close(1);
	close(2);
	if(rewind_one_file)
		rewind_one_pos();
	next_tape_position();
}

/*

	r - rewind one tape position after processing current image
	d device
	b tape_read_buffer size 

*/
	

main(argc,argv)
int argc;
char *argv[];
{
	char *buf;
	int rc;
	char *bz;
	char *tape;
	extern char *optarg;
	extern int optind;
	int read_size=5120;
	int flag;
	int count=0;

	/* pipe breaking on stdout tells us to stop reading tape ... */
	inu_signal(SIGPIPE,sighandle);

	flag=getopt(argc,argv,"rb:d:");
	while(flag!=EOF)
	{
		switch(flag)
		{
		case 'r':
			rewind_one_file++;
			break;
		case 'd':
			tape=optarg;
			break;
		case 'b':
			read_size=atoi(optarg) * 512;
			break;
			
		}
		flag=getopt(argc,argv,"rb:d:");
	}


	if(tape==NULL)
		exit(1);

	/* wait on our tape device */
	lock_tape_restore();

	buf=malloc(read_size);
	if(buf==NULL)
		exit(1);

	fd=open(tape,O_RDONLY);
	if(fd == -1)
		exit(1);

	/* should we ever get rid of the chdev's in installp */
	if((bz=getenv("INU_TAPE_BLOCK_SIZE")) != NULL)
	{
        	struct stchgp targ={0,512};
        	targ.st_blksize=atoi(bz);
        	ioctl(fd, STIOCHGP, &targ);
	}

	/* read/write until our pipe breaks */
	while(1)
	{
		rc=read(fd,buf,(size_t)read_size);
		if(rc == 0)
		{
			hit_tape_mark=1;
			free(buf);
			position_tape();
		}
		if(sig_interupted == 1)
		{
			free(buf);
			position_tape();
		}
		if(rc < 0)
			exit(1);
		rc=write(1,buf,rc);
		if(sig_interupted == 1)
		{
			free(buf);
			position_tape();
		}
	}
	
	exit(0);

}
