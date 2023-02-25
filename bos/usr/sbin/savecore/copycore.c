static char sccsid[] = "@(#)32  1.9  src/bos/usr/sbin/savecore/copycore.c, cmddump, bos411, 9428A410j 3/14/94 09:17:14";

/*
 * COMPONENT_NAME: CMDDUMP
 *
 * FUNCTIONS: copycore
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*              include file for message texts          */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/err_rec.h>
#include <sys/dump.h>
#include <sys/ioctl.h>
#include <sys/cfgodm.h>
#include <sys/errids.h>
#include <sys/vmount.h>
#include <sys/limits.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <locale.h>
#include <savecr_msg.h>
#include <errlg/SWservAt.h>

#define MCS_CATALOG  "savecr.cat"

#define COPYFILENAME "/var/adm/ras/copyfilename"

extern int errno;
extern int odmerrno;

/*
 * NAME: main copycore
 *
 * FUNCTION:
 *      Extracts variable autocopydump from ODM and if it is set 
 *	to something other than /dev/null or "", this routine will
 *	mount filesystems and perform a savecore operation copying 
 *	the "dump" to the filename in autocopydump.
 *	If savecore fails to copy the dump and if the savecore 
 *	returns -1, or the value of autocopydump is /dev/null,
 *	copycore will then check the value of forcecopydump
 *	attribute to determine if an attempt should be made to allow
 *	customers to copy the dump out to the external media via 
 *	copydumpmenu().
 *
 *	The following error conditions are detected by savecore()
 *	1) ioctl(/dev/sysdump,DMP_IOCINFO) fails
 *		Since copycore() should detect this error b/4 
 *		calling savecore(), hence we should not concern
 *		ourselves with this error condition from savecore().
 *
 *	2) No previous dump exists 
 *		Since copycore() should detect this error b/4 
 *		calling savecore(), hence we should not concern
 *		ourselves with this error condition from savecore().
 *		
 *	3) No recent dump
 *		Since copycore() should detect this error b/4 
 *		calling savecore(), hence we should not concern
 *		ourselves with this error condition from savecore().
 *
 *	4) No valid dump exists in the dump device,i.e. bad magic number
 *		It's very unlikely that condition should exist if savecore()
 *		is invoked from copycore(). But if it does, we can't copy
 *		the dump via menu anyway, since copydumpmenu will check
 *		for the same thing.  In this instance, savecore() will
 *		return(1).  copycore() will log a fail to copy message
 *		and return(0) (so rc.boot don't attempt to configure the
 *		console and invoke the copydumpmenu).
 *
 *	5) Unable to get the status of the directory(statfs() fails)
 *		savecore() will return -1 to copycore().  copycore()
 *		will log a fail to copy message and checks the value
 *		of forcecopydump attribute.  If it's set to true,
 *		then copycore() creates /needcopydump file and return(-1) to
 *		rc.boot.	
 *
 *	6) Unable to get the stat for the directory(stat() fails)
 *		savecore() will return -1 to copycore().  copycore()
 *		will log a fail to copy message and checks the value
 *		of forcecopydump attribute.  If it's set to true,
 *		then copycore() creates /needcopydump file and return(-1) to
 *		rc.boot.	
 *
 *	7) Don't have enough space
 *		savecore() will return -1 to copycore().  copycore()
 *		will log a fail to copy message and checks the value
 *		of forcecopydump attribute.  If it's set to true,
 *		then copycore() creates /needcopydump file and return(-1) to
 *		rc.boot.	
 *
 *	8) read/write/open problems
 *		It's very unlikely that condition should exist if savecore()
 *		is invoked from copycore(). But if it does, we can't copy
 *		the dump via menu anyway, since copydumpmenu will encounter
 *		the same problems.  In this instance, savecore() will
 *		return(1).  copycore() will log a fail to copy message
 *		and return(0) (so rc.boot don't attempt to configure the
 *		console and invoke the copydumpmenu).
 *
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 *
 *      This routine is called only from rc.boot 
 *
 *
 */
int verbose = FALSE;

main(argc,argv)
int argc; 
char *argv[];


{
	int for_testing = FALSE;
        struct CuAt *cuat;
        struct SWservAt *autocopydump;
        struct SWservAt *copyfilesystem;
	char buffer[BUFSIZ];
	char crit[BUFSIZ];
	char *mountpt;
	char *filesys;
	char *p;
	char *odm_msg;
	char *msg_str;
	char msg_tmp[MAXPATH];
	char *buf;
	int return_code;
	struct SWservAt *forcecopydump;
	struct listinfo stat_info;
	struct dumpinfo dmp;
	struct dumpinfo dump;
	int cnt,rv;
	int mountflg = 0;
	int fd, copyfile_fd;
	int dumpfd;
	char c;
	
	bzero(msg_tmp,MAXPATH);

	if ((argc > 1) && ( strcmp(argv[1], "testing") == 0))
       	  for_testing = TRUE;
	if ((argc > 1) && ( strcmp(argv[1], "-v") == 0))
       	  verbose = TRUE;

 
	setlocale(LC_ALL,"");
	catinit(MCS_CATALOG);
	/*********************************************************
	   Initialization for accessing the ODM data base 
	   If unable to do the initialization, assumming there
	   is no dump to copy, return zero   			
	*********************************************************/

	if (verbose) printf("Before odm_initialize()\n");
        if (0 > odm_initialize()) 
	{
	    if (odm_err_msg(odmerrno,&odm_msg) == 0)
		fprintf(stderr,odm_msg);
	    else
		cat_eprint(CAT_ODM_INIT_ERR,
		"Unable to initialize an ODM session.\n");	
            return(0);
	}
	if (verbose) printf("odm_initialize() is OK\n");

	/*********************************************************
	   Access the NVRAM via /dev/sysdump to see if there 
	   is a recent dump on the paging space.
	*********************************************************/

	if (rv = (getdumpinfo(&dmp)))
	{
			 /* i.e. unable to get the info from 
			   NVRAM via /dev/sysdump		  */
	  return(0);   /* assume there is no dump to copy,
			* continue with the rest of the boot      */
	}
	if (verbose) printf("getdumpinfo() is OK\n");

	/* if there is not a recent dump, then continue with the
	   rest of the boot process				  */
	if ( (dmp.dm_size <= 0) || (dmp.dm_devicename[0] == '\0') ||
	     ( !(dmp.dm_flags & DMPFL_NEEDCOPY)) )
	{
	  return(0);
	}
	if (verbose) printf("Initial Checking is OK\n");

        /* delete /var/adm/ras/copyfilename (since there is a
           recent dump).
        */
	unlink(COPYFILENAME);

	if (!for_testing) /* only do this when invoked from rc.boot */
	{
	  if (verbose) printf("call from rc.boot. dump_dev = %s \n",
		dmp.dm_devicename);
	  /* Verify if the type of dump device is paging	  */
	  sprintf(crit,"name = '%s' and attribute ='type' and value = 'paging'",
  		rmdev(dmp.dm_devicename));
  	  cuat = odm_get_list(CuAt_CLASS,crit,&stat_info,1,1);
	  if ( !cuat )
	  {
	    if (verbose) printf("Unable to check for the type of dump dev \n");
	    return(0);
			/* Unable to check the type of dump device,
			   assume it's not paging 		  */
	  }
	}
	if (verbose) printf("Dump device is paging space\n");

	/*********************************************************
	   get value for autocopydump				 
	*********************************************************/

        autocopydump = (struct SWservAt *)ras_getattr("autocopydump", 0, &cnt);
        if ((autocopydump != NULL) &&
		(strcmp(autocopydump->value,"\"\"") != 0) &&
		(strcmp(autocopydump->value,"/dev/null") != 0))	
	{

	  /* now retrieve the copyfilesystem attribute */
          copyfilesystem = (struct SWservAt *)ras_getattr("copyfilesystem", 0, &cnt);
          if (copyfilesystem == NULL)
	  {
	    cat_eprint(CAT_GETATTR,
		"Unable to get attribute %s from ODM object class SWservAt.\n","copyfilesystem");	
	  }
	  else
	  {
	    /* Assumptions: / is mounted in /mnt
	    *		  /usr is mounted in /usr
	    *		  /var is mounted in /var
	    *  bosboot should mount all of the above filesystem before
	    *  invoking copycore. (/var is needed for savecore)
	    */

	    /* now we must mount filesystem for the directory specified by
	    *  autocopydump->value
	    */
	    rv = 0;
	    p = strchr(copyfilesystem->value,',');
	    mountpt = p + 1;
	    *p = '\0';
	    filesys = copyfilesystem->value;
	    if (verbose) printf("filesys = %s, mountpt = %s \n",filesys,mountpt);
	    if ( for_testing )
		sprintf(buffer,"/usr/sbin/savecore -d %s",autocopydump->value);
	    else /* when invoked from rc.boot */
	    {
		if (verbose) printf("building the savecore command line for rc.boot\n");
	        if ( (strcmp(mountpt,"/") == 0)  )
	        {
	          sprintf(buffer,"/usr/sbin/savecore -d /mnt/%s",autocopydump->value);
	        }
	        else
	        {
	          if  ((strcmp(mountpt, "/usr") != 0) && (strcmp(mountpt, "/var") != 0))
		  {
		     if (verbose) printf("Going to mount the file system\n");
		     sprintf(buffer,"mount %s",mountpt);
	             rv = system(buffer);
		     if (rv)
	                 /* check for the return code from the mount step */
	             {
			if (verbose) printf("unable to mount the file system\n");
	  	        perror("copycore:mount()");
		        rv = 1;
	             }
		     else mountflg = 1;
		  }
	          sprintf(buffer,"/usr/sbin/savecore -d %s",autocopydump->value);
	        }
	    }
	    if (verbose) printf("rv = %d, command = %s\n",rv,buffer);
	    if ( rv == 0 )
	    {
	      return_code = system(buffer);
	      if (verbose) printf("return_code = %d \n", return_code);

	      sprintf(buffer,"umount %s",mountpt);
	      if (return_code != 0)
	      {
		  /* savecore failed:  Check forcecopydump variable to see
		   * if the a manual copy attempt should be tried
		  */

		  cat_eprint(CAT_COPY_FAIL,
			"Failed to copy the dump from %s to %s.\n",dmp.dm_devicename,
			autocopydump->value);
    	       	  if ( !for_testing && mountflg &&  (system(buffer) != 0 ))
	      	  { /* if unable to unmount the filesystem of the copy directory,
		     * put out an error message and go on with the boot process.
		     */
		     perror("copycore:umount()");
	          }
		  if (return_code == 1)
		     return(0);		/* see block comment for explanation */
	      }
	      else  /* everything is OK, umount and exit */
	      {
		 if (verbose) printf("savecore return 0\n");
		  /* log a unsuccessful message to boot log and return (0) */
		  cat_print(CAT_COPY_SUCCESS,
		 	"Successfully copied the dump from %s to %s.\n",
			dmp.dm_devicename,autocopydump->value);

    	          if ( !for_testing && mountflg && (system(buffer) != 0) )
	          { /* if unable to unmount the filesystem of the copy directory,
		     * put out an error message and go on with the boot process.
		    */
		    perror("copycore:umount()");
	          }
		return(0);
	      }
	    }
	  }
	}
	/* we should only get here if one of the followings is true:
	   - unable to retrieve the autocopydump attribute
	   - the value of the autocopydump is blank or /dev/null
  	   - fail to mount the filesystem (to copy to)
	   - savecore fails and the return code is other than 1
	*/
	if (verbose) printf("savecore fails, check forcecopydump\n");

        /* log a failing message to /var/adm/ras/copyfilename file */
	/* We are logging the message number of the failed copy message.  */
	/* We have to do this, because during boot we don't have access   */
 	/* to our message catalogs.  After boot, sysdumpdev will do a lookup */
	/* on this message number, and print the correct message.	*/
        sprintf(msg_tmp,"15 DEVICE:%s FILE:%s ",dmp.dm_devicename, autocopydump->value);
	msg_str = stracpy(msg_tmp);

        copyfile_fd = open(COPYFILENAME,(O_WRONLY | O_CREAT | O_TRUNC));
        if (copyfile_fd < 0)
                perror(COPYFILENAME);
        else
        {
                if (write(copyfile_fd,msg_str,strlen(msg_str)) < strlen(msg_str)
)
                {
                        perror("write");
                }
        }
	
        forcecopydump = (struct SWservAt *)ras_getattr("forcecopydump", 0, &cnt);
	/* check the return of ras_getattr call */
	if (verbose) printf("forcecopydump = %s\n",forcecopydump->value);

	if ( (forcecopydump == NULL) || (strcmp(forcecopydump->value,"TRUE") == 0))
	{
		/* set needcopydump */
	        if (verbose) printf("creating the /needcopydump\n");
		/* The forcecopydump attribute is set to true, so we need to */
		/* log the message number 18 to /var/adm/ras/copyfilename.   */
		/* This is the message number of the allowed copy message.   */
		/* After boot, the sysdumpdev command will do a lookup up on */
		/* this message number, and print the correct message. 	     */		
		sprintf(msg_tmp,"18");
		msg_str = stracpy(msg_tmp);

                if (write(copyfile_fd,msg_str,strlen(msg_str)) < strlen(msg_str))
                {
                        perror("write");
                }
		fd = creat("/needcopydump",O_WRONLY);
		close(fd);
		return(1);
	}
	else
		{
        	dumpfd = open("/dev/sysdump",0);
       		if (dumpfd < 0 )
			{
			perror("copycore: open(/dev/sysdump)");
			return;
			}
		bzero(&dump, sizeof(dump));
		if ( ioctl(dumpfd, DMP_IOCSTAT2, &dump) < 0)
			perror("copycore: ioctl(DMP_IOCSTAT2)");
		close(dumpfd);
		return;
		}
			
	
}


/* Routine removes the /dev from the start of the string */
#define DEVICE "/dev/"
rmdev(dm_devicename)
char *dm_devicename;
{
char *ptr;
ptr = dm_devicename;
if (strncmp(dm_devicename,DEVICE,strlen(DEVICE)) == 0)
        {
        ptr+=strlen(DEVICE);
        return(ptr);
        }

}

#define SYSDUMP "/dev/sysdump"

/*
 * NAME: getdumpinfo
 *
 * FUNCTION: get dump infomation from the NVRAM via /dev/sysdump
 */
static
int getdumpinfo(struct dumpinfo *dmp)
{
        int fd;

        fd=open(SYSDUMP,0);
        if (fd < 0 )
        {
                perror("(get_dump_info) open(/dev/sysdump)");
                return(1);
        }
        if (ioctl(fd,DMP_IOCINFO,dmp) == -1) {
                perror("(getdumpinfo) ioctl(DMP_IOCINFO)");
                close(fd);
                return(-1);
        }
close(fd);
return(0);
}


