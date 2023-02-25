static char sccsid[] = "@(#)63  1.21.2.6  src/bos/usr/sbin/savebase/savebase.c, cmdcfg, bos411, 9432B411a 8/9/94 08:12:42";
/*
 *   COMPONENT_NAME: CMDCFG - savebase.c
 *
 *   FUNCTIONS: DATA
 *		do_error
 *		main
 *		write_data
 *		check_for_updates
 *		get_lvcb
 *		update_header
 *		update_br
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * DESCRIPTION:   Extracts customization information about the base devices
 *   from ODM, compresses this data, then writes it to the boot record on
 *   disk.  If "-d device" is specified, it is assumed to specify the hard
 *   disk on which to update the boot record.  If "-d device" is not
 *   specified, then the boot logical volume is updated.  In this case,
 *   any mirrors of the BLV are also updated automatically by LVM.
 */
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <cmdlvm.h>
#include <aouthdr.h>
#include <filehdr.h>
#include <scnhdr.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/cfgodm.h>
#include <sys/cfgdb.h>
#include <sys/bootrecord.h>
#include <sys/dasd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <mkboot.h>


#define DATA(a,b,c)		{ if (verbose) printf(a,b,c); }

#define TMP_FILE		"/tmp/savebase"
#define TMP_FILE_PACKED		"/tmp/savebase.Z"
#define PACK			"compress"
#define DEST_DISK		"/dev/ipldevice"
#define DEST_FILE		"/etc/basecust"
#define RUNTIME  		"runtime"
#define FILE_MODE   		0 
#define NORM_MODE   		1
#define SERV_MODE		2
#define BOTH_MODE		3
#define NUM_MIRRORS		3
#define DEV_MAX_NM_SZ		_D_NAME_MAX+1+4  
#define TOTAL_DEVNO		7


union first_buffer
{       int total;
        unsigned char buf[DBSIZE];
};

static int verbose;
static IPL_REC boot_record;
int  norm_fd;
int  serv_fd;
int  dest_fd;
char *destination;
int  normserv;
int  lock_id;		/* odm lock id			*/

main( argc, argv )
int argc;
char *argv[];
{
	char 	*origin;
	int 	d_specified;
	int 	dest_is_device;
	int 	c;
	extern 	int optind;
	extern 	char *optarg;
	struct 	Class *cudv;
	struct 	CuDv CuDv;
	struct 	Class *cuat;
	struct 	CuAt CuAt;
	struct 	Class *cudep;
	struct 	CuDep CuDep;
	struct 	Class *cudvdr;
	struct 	CuDvDr CuDvDr;
	struct 	Class *cuvpd;
	struct 	CuVPD CuVPD;
	FILE 	*fd;
	int 	fd_in;
	int 	blknum;
	int 	cudv_cnt;
	int	cuat_cnt;
	int 	cudep_cnt;
	int 	cudvdr_cnt;
	int 	cuvpd_cnt;
	int 	rc;
	uchar 	buf[DBSIZE];
	union 	first_buffer fbuf;
	char 	temp[100];
	int 	num;
	int 	i;
	int 	norm_top;
	int 	serv_top;
	char 	nc='\0';
	char 	*filename;
	struct 	PdAt pdat;
	char 	norm_blv_name[DEV_MAX_NM_SZ];
	char 	serv_blv_name[DEV_MAX_NM_SZ];
	struct 	stat sbuf;

	/* initialize internal variables */
	origin = NULL;
	verbose = d_specified = dest_is_device = FALSE;
	normserv = BOTH_MODE;
	destination = DEST_DISK;

	/* acquire data base lock */
	if ( (lock_id=odm_lock( "/etc/objrepos/config_lock", ODM_WAIT)) == -1)
	   do_error("Unable to acquire the device configuration database\n\tlock. The errno is %d.\n", 
		    	0, errno);

	/* parse the arguments */
	while ((c = getopt(argc,argv,"vo:d:m:")) != EOF) {
	    switch (c) {
		case 'v' :
		    verbose = TRUE;
		    break;
		case 'o' :
		    origin = optarg;
		    break;
		case 'd' :
		    destination = optarg;
		    d_specified = TRUE;
		    break;
	        case 'm' :
				if( optarg[0] == 'N' || optarg[0] == 'n' )
				    normserv = NORM_MODE;
				else if( optarg[0] == 'S' || optarg[0] == 's' )
				    normserv = SERV_MODE;
				else if (optarg[0] != 'B' && optarg[0] != 'b' )
					 do_error("Usage: [-v] [-o origin] [-d destination] [-m norm|serv|both]\n",0,1);

					
				break;
	       	case '?' :
		default  :
		    do_error("Usage: [-v] [-o origin] [-d destination] [-m norm|serv|both]\n",0,1);
		    break;
	    }
	}


	if (optind < argc)
		do_error("invalid argument - %s\n",argv[optind],1);

        /* start up ODM */
        odm_initialize();

	/* if origin set, set the path */
	if (origin) {
       		/* set the ODMDIR */
        	DATA("setting ODMDIR to '%s'\n",origin,NULL);
        	if ((int)odm_set_path( origin ) < 0)
                	do_error("unable to odm_set_path :odmerrno = %d\n",
                        	                odmerrno,1);
	}


	rc = stat(destination ,&sbuf);

	if (rc == -1 && !d_specified) {
		/* file not present, write to basecust */
		destination = DEST_FILE;
		normserv= FILE_MODE;
	}
	else if (S_ISCHR(sbuf.st_mode) || S_ISBLK(sbuf.st_mode) ) {
		dest_is_device = TRUE;
		if ((dest_fd = open(destination,0)) < 0) {
			DATA("can\'t open %s; setting destination to %s\n",
				DEST_DISK,DEST_FILE);
			destination = DEST_FILE;
			normserv= FILE_MODE;
                }

		/* read the boot record */
		num = read(dest_fd,&boot_record,sizeof(IPL_REC));
		if (num != sizeof(IPL_REC)) { 
			sprintf(temp,"error reading ipl record: expecting %d,read %d\n", sizeof(IPL_REC),num );
	   	   	do_error(temp,0,1);
		}

		DATA("successfully read the ipl record for '%s'\n", 
		      destination,NULL);

		/* check the IPL record magic number (make sure its valid) */
		if (boot_record.IPL_record_id != (unsigned)IPLRECID)
   			do_error("invalid IPL record id = 0x%x\n",
			boot_record.IPL_record_id,1);

		DATA("boot_lv_length = %d, basecn_start = %d \n", 
		      boot_record.boot_lv_length,boot_record.basecn_start);
		DATA("boot_lv_start = %d", boot_record.boot_lv_start, NULL);

		/* check if normal and service start location are the same */
		if ( (normserv == BOTH_MODE) && (boot_record.basecs_start 
			== boot_record.basecn_start ))
			normserv = NORM_MODE;

		/* check ROS emulation flags in boot_record */
		if ( normserv == NORM_MODE && 
		     boot_record.boot_emulation == 1 )
               		do_error("can not update normal customized savebase area due to ROS emulation code\n",NULL,1);
        
		else if ( normserv == SERV_MODE && 
			  boot_record.ser_emulation == 1 )
               		do_error("can not update service customized savebase area due to ROS emulation code\n",NULL,1);

		else if ( normserv == BOTH_MODE ) {
        		if (boot_record.ser_emulation == 1 && boot_record.boot_emulation == 1 )
               			do_error("can not update normal or service customized savebase area due to ROS emulation code\n",NULL,1);
        		if (boot_record.boot_emulation == 1 )
               			normserv=SERV_MODE;
        		if (boot_record.ser_emulation == 1 )
               			normserv=NORM_MODE;
        		/* if neither emulation flags set, normserv does not change */
		}


		if (normserv == NORM_MODE || normserv == BOTH_MODE) {
		   /* check the IPL record base start location  */
		   if (boot_record.basecn_start == 0 )
	   	     do_error("savebase: invalid IPL normal record offset: 0x0.\n", NULL, 1);

		  DATA("boot_record.basecn_start = 0x%x bytes\n", 
			boot_record.basecn_start*UBSIZE,NULL);

 	          /* get normal blv from boot logical volume cb */
  	          get_lvcb(boot_record.boot_lv_start, norm_blv_name);    
		}

		if (normserv == SERV_MODE || normserv == BOTH_MODE) {
		   if (boot_record.basecs_start == 0 )
   	   	     do_error("savebase: invalid IPL service record offset: 0x0.\n", NULL, 1);
		   DATA("boot_record.basercs_start = 0x%x bytes\n", 
		         boot_record.basecs_start*UBSIZE,NULL);
		   /* get service blv from service logical volume cb */
		   get_lvcb(boot_record.ser_lv_start, serv_blv_name);

		}
        	close(dest_fd);
	} /* end else if */
	else
		normserv = FILE_MODE;


	/* check for the runtime attr - if not there, add it */
	if ((rc = (int)odm_get_first(PdAt_CLASS,PHASE1_DISALLOWED,&pdat)) == -1)
		do_error("unable to access the PdAt class\n",NULL,1);
	else if ((char *)rc == NULL)  {
	     /* no runtime PdAt - add it */
             DATA("no runtime PdAt - adding it\n",NULL,NULL);

 	     strcpy( pdat.uniquetype, RUNTIME );
             pdat.attribute[0] = nc;
             pdat.deflt[0] = nc;
             pdat.values[0] = nc ;
             pdat.width[0] = nc;
             pdat.type[0] = nc;
             pdat.generic[0] = nc;
             pdat.rep[0] = nc;
             pdat.nls_index = 0;
             if (odm_add_obj(PdAt_CLASS,&pdat) == -1)
              	   do_error("unable to add the runtime PdAt\n",NULL,1);
         }


	/* create the temporary file */
	if ((fd = fopen(TMP_FILE,"w+")) == NULL)
		do_error("unable to creat : errno = %d\n", errno, 1);

	DATA("created '%s' temporary file\n",TMP_FILE,NULL);

	/* write dummy place holders for the object counts */
	cudv_cnt = 0;
	fwrite(&cudv_cnt,4,1,fd);
	cuat_cnt = 0;
	fwrite(&cuat_cnt,4,1,fd);
	cudep_cnt = 0;
	fwrite(&cudep_cnt,4,1,fd);
	cuvpd_cnt = 0;
	fwrite(&cuvpd_cnt,4,1,fd);
	cudvdr_cnt = 0;
	fwrite(&cudvdr_cnt,4,1,fd);

	/* open CuDv object class */
	cudv = odm_open_class(CuDv_CLASS);
	if ((int)cudv == -1)
		do_error("CuDv could not be opened\n",NULL,1);

	rc = (int)odm_get_first(cudv,"",&CuDv);

	while(rc != 0) {
		if (rc == -1)
			do_error("Error reading CuDv\n",NULL,1);

		/* CuDv.status is not saved because restbase resets status */
 		fprintf(fd,"%s%c%x%c%s%c%s%c%s%c%s%c%s%c%c",
				CuDv.name,      nc,
				CuDv.chgstatus, nc,
				CuDv.ddins,     nc,
				CuDv.location,  nc,
				CuDv.parent,    nc,
				CuDv.connwhere, nc,
				CuDv.PdDvLn_Lvalue, nc,
				(char)cudv_cnt );   /* sync character */
		cudv_cnt++;

		rc = (int)odm_get_next(cudv,&CuDv);
	}

	DATA("saved %d CuDv objects\n",cudv_cnt,NULL);
	odm_close_class(cudv);


	/* open CuAt object class */
	cuat = odm_open_class(CuAt_CLASS);
	if ((int)cuat == -1)
		do_error("CuAt could not be opened\n",NULL,1);

	rc = (int)odm_get_first(cuat,"",&CuAt);

	while(rc != 0) {
		if (rc == -1)
			do_error("Error reading CuAt\n",NULL,1);

 		fprintf(fd,"%s%c%s%c%s%c%s%c%s%c%s%c%x%c%c",
				CuAt.name,	nc,
				CuAt.attribute,	nc,
				CuAt.value,	nc,
				CuAt.type,	nc,	
				CuAt.generic,	nc,
				CuAt.rep,	nc,
				CuAt.nls_index,	nc,
				(char)cuat_cnt );   /* sync character */

		cuat_cnt++;

		rc = (int)odm_get_next(cuat,&CuAt);
	}

	DATA("saved %d CuAt objects\n",cuat_cnt,NULL);
	odm_close_class(cuat);


	/* open CuDep object class */
	cudep = odm_open_class(CuDep_CLASS);
	if ((int)cudep == -1)
		do_error("CuDep could not be opened\n",NULL,1);

	rc = (int)odm_get_first(cudep,"",&CuDep);

	while(rc != 0) {
		if (rc == -1)
			do_error("Error reading CuDep\n",NULL,1);

 		fprintf(fd,"%s%c%s%c%c",
				CuDep.name,    	 nc,
				CuDep.dependency,nc,
				(char)cudep_cnt );   /* sync character */


		cudep_cnt++;

		rc = (int)odm_get_next(cudep,&CuDep);
	}

	DATA("saved %d CuDep objects\n",cudep_cnt,NULL);
	odm_close_class(cudep);


	/* open CuVPD object class */
	cuvpd = odm_open_class(CuVPD_CLASS);
	if ((int)cuvpd == -1)
		do_error("CuDvDr could not be opened\n",NULL,1);

	rc = (int)odm_get_first(cuvpd,"",&CuVPD);

	while(rc != 0) {
		if (rc == -1)
			do_error("Error reading CuVPD\n",NULL,1);

 		fprintf(fd,"%s%c%x%c",
				CuVPD.name,		nc,
				CuVPD.vpd_type,		nc);
		fwrite(CuVPD.vpd,512,1,fd);

		/* put sync character */
		fputc((char)cuvpd_cnt,fd);

		cuvpd_cnt++;

		rc = (int)odm_get_next(cuvpd,&CuVPD);
	}

	DATA("saved %d CuVPD objects\n",cuvpd_cnt,NULL);
	odm_close_class(cuvpd);

	/* open CuDvDr object class */
	cudvdr = odm_open_class(CuDvDr_CLASS);
	if ((int)cudvdr == -1)
		do_error("CuDvDr could not be opened\n",NULL,1);

	rc = (int)odm_get_first(cudvdr,"",&CuDvDr);

	while(rc != 0) {
		if (rc == -1)
			do_error("Error reading CuDvDr\n",NULL,1);

 		fprintf(fd,"%s%c%s%c%s%c%s%c%c",
				CuDvDr.resource,	nc,
				CuDvDr.value1,		nc,	
				CuDvDr.value2,		nc,
				CuDvDr.value3,		nc,
				(char)cudvdr_cnt );   /* sync character */

		cudvdr_cnt++;

		rc = (int)odm_get_next(cudvdr,&CuDvDr);
	}

	DATA("saved %d CuDvDr objects\n",cudvdr_cnt,NULL);
	odm_close_class(cudvdr);


	/* go back and write the total object counts */
	fseek(fd,0L,0);
	fwrite(&cudv_cnt,4,1,fd);
	fwrite(&cuat_cnt,4,1,fd);
	fwrite(&cudep_cnt,4,1,fd);
	fwrite(&cuvpd_cnt,4,1,fd);
	fwrite(&cudvdr_cnt,4,1,fd);

	/* all done - close the file */
	fclose( fd );


	DATA("calling '%s' to compress '%s'\n",PACK,TMP_FILE);
	/* now compress the file */
	odm_run_method(PACK,TMP_FILE,NULL,NULL);
 
	/* open the packed file */
	if ((fd_in = open(TMP_FILE_PACKED,0)) < 0) {
		DATA("can\'t open %s; trying %s\n",TMP_FILE_PACKED,TMP_FILE);
		/* can't open packed file, try original file */
		if ((fd_in = open(TMP_FILE,0)) < 0)
			do_error("unable to open '%s'\n",TMP_FILE,1);
		else 
			filename=TMP_FILE;	

	}
	else
		filename=TMP_FILE_PACKED;

	if (dest_is_device)
	{

		if (normserv == NORM_MODE || normserv == BOTH_MODE) {
		   if (( norm_fd = open (norm_blv_name,2 )) < 0 ) {
			sprintf(temp,"unable to open %s:errno %d\n",
			destination,errno);
			do_error(temp,0,1);
		   }
	 
		   norm_top = (boot_record.basecn_start - 
				boot_record.boot_lv_start)*UBSIZE;

		   if (lseek(norm_fd, (long)norm_top, 0) < 0) {
		      sprintf(temp,"unable to lseek to 0x%x : errno = %d\n",
		    		norm_top,errno);
	   	      do_error(temp,NULL,1);
		   }
		}

		if (normserv == SERV_MODE || normserv == BOTH_MODE) {

		   if (( serv_fd = open (serv_blv_name,2 )) < 0 ) {
			sprintf(temp,"unable to open %s:errno %d\n",
			destination,errno);
			do_error(temp,0,1);
		   }

		   serv_top = (boot_record.basecs_start -
				boot_record.ser_lv_start)*UBSIZE;

           	   if (lseek(serv_fd, (long)serv_top, 0) < 0) {
		      sprintf(temp,"unable to lseek to 0x%x : errno = %d\n",
		    			serv_top,errno);
	   	      do_error(temp,NULL,1);
		   }
		}
		rc = check_for_updates(filename, norm_blv_name, serv_blv_name); 
		if (rc)
			do_error("could not get boot header/record info",NULL,1);
	}
	else
	{	DATA("creating the destination file '%s'\n", destination,NULL);

		/* create the dummy file using normal file descriptor */
		if ((norm_fd = creat(destination,0644)) < 0) {
		   sprintf(temp,"unable to create %s :errno = %d\n",
			   destination,errno);
   		   do_error(temp,0,1);
		}
		norm_top = 0;
	}

	/* initialize the first buffer - total is in first 4 bytes */
	fbuf.total = -1;
	num = read(fd_in,&(fbuf.buf[4]),DBSIZE-4);
	if ( num <1 ) 
		do_error("Could not read first buffer\n",NULL,1);

	num += 4;
	write_data ( num, fbuf.buf);
	fbuf.total = num;
	
	/* read from the packed file & write to the destination */

        while ((num = read(fd_in,buf,DBSIZE)) > 0)  {
		fbuf.total += num;
		write_data(num, buf);
	}

	DATA("wrote 0x%x bytes to '%s'\n",fbuf.total,destination);

	/* go back & update the total num bytes written */

	if ( normserv != SERV_MODE )  {  /* update file or normal blv */
	   	if (lseek(norm_fd, norm_top,0) < 0)
	        	do_error("unable to lseek to '%d'\n",norm_top,1);
	   	DATA("updating 'total' at offset 0x%x\n",
		      lseek(norm_fd,0L,1),NULL);
	}

	if (normserv == SERV_MODE || normserv == BOTH_MODE ){
		/* update service blv */
	   	if (lseek(serv_fd, serv_top,0) < 0)
	        	do_error("unable to lseek to '%d'\n",serv_top ,1);

	        DATA("updating 'total' at offset 0x%x\n",
		      lseek(serv_fd,0L,1),NULL);
	}
	/* write first block which contains total count */
	write_data(DBSIZE, fbuf.buf);

	DATA("savebase : successful completion\n",NULL,NULL);

	/* close the files */
	close( fd_in );
	close( norm_fd );
	close( serv_fd );
	
	/* cleanup */
	unlink( TMP_FILE );    
	unlink( TMP_FILE_PACKED ); 

	/* unlock the database */
	odm_unlock(lock_id);

	/* close the odm */
	odm_terminate();

	/* bye bye */
	exit( 0 );
}


/*---------------------------------- do_error -------------------------------*/
do_error(str1,str2,exit_code)
char *str1;
char *str2;
int exit_code;
{
	if (verbose)
	   fprintf(stderr,str1,str2);

	/* clean up */
	unlink( TMP_FILE );
	unlink( TMP_FILE_PACKED );

	/* unlock the database */
	odm_unlock(lock_id);

	exit( exit_code );
}

/*---------------------------------- check_for_updates ----------------------
 * NAME: check_for_updates 
 *
 * FUNCTION: if compressed file size is > 128K, the bootheader and boot
 *	record(s) need to be updated. 
 *
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:  0 on successful updates
 *	     1 exit through do_error
 *
 *---------------------------------------------------------------------------*/
int
check_for_updates(filename, norm_blv, serv_blv)
char    *filename;		/* compressed file name  */
char	*norm_blv; 		/* normal blv file name  */
char	*serv_blv; 		/* service blv file name */
{
	struct stat sbuf;
	int	    rc;
	int	    i;
	int	    fd;
	uint	    cur_num_blks; /* current num of 512 byte savebase blocks */
 	int	    num_blks;	  /* num of 128K blocks for compressed file  */
 	int	    nnum_more_blks; /* num of additional 128K blocks needed  */
 	int	    snum_more_blks; /* number of additional 128K blocks need */
	int	    update_flag;    /* TRUE, FALSE to update bootrecord and 
				       header*/
		

	nnum_more_blks = snum_more_blks = 0;
	update_flag = FALSE;

	/* obtain compressed file info */
	rc = stat(filename ,&sbuf);
	if (rc <0 )
		do_error("Could not get file status on %s\n",filename,1);

	/* calculate number of blks current compressed file is */
	num_blks=sbuf.st_size/SBSIZE + 1;

	DATA("compressed file size is = %d \n",sbuf.st_size, NULL);

	/* based on mode, determine current number of 512 byte savebase blocks.
         * bootrecord values do not include the first 2 blks, therefore 
	 * 2 blks added.  
	 * The first 2 blks are reserved --blk1 is for the lvcb, 
         * blk2 is for install info.
	 */

	if (normserv == NORM_MODE || normserv == BOTH_MODE )  {
		cur_num_blks = boot_record.boot_lv_length - 
			      (boot_record.basecn_start -
			       boot_record.boot_lv_start) + 2 ;
		DATA("cur_num_blks = %d \n",cur_num_blks, NULL);

		/* check if compressed file size greater than savebase area */

		if (sbuf.st_size > cur_num_blks*UBSIZE ) {
			/* calculate number of additional blocks
			   needed in savebase area */

			nnum_more_blks = num_blks -
				         ((cur_num_blks*UBSIZE)/SBSIZE);

			DATA("norm num more blks is = %d \n",
			      nnum_more_blks, NULL);
			update_flag = TRUE;
		}
	}


	/* bootrecord values do not include the first 2 blks, therefore 
         * 2 blks added.
	 * The first 2 blks are reserved- blk1 is for the lvcb, blk2 is 
         * for install 
	 * info. */

	if ( (boot_record.basecs_start == boot_record.basecn_start)  ||
	      normserv == SERV_MODE || normserv == BOTH_MODE )   {
		cur_num_blks = boot_record.ser_lv_length  - 
			       (boot_record.basecs_start -
			        boot_record.ser_lv_start) + 2 ;

		DATA("cur_num_blks = %d \n",cur_num_blks, NULL);

		/* check if compressed file size greater than savebase area */
		if (sbuf.st_size > cur_num_blks*UBSIZE ) {

			/* calculate number of additional blocks needed 
			   in savebase area */

			snum_more_blks = num_blks - 
					((cur_num_blks*UBSIZE)/SBSIZE);
			DATA("serv num more blks is = %d \n",
			      snum_more_blks, NULL);
			update_flag = TRUE;
		}
	}

 
	/* if compressed file size is > current customized area, 
           update boot records and boot header */

	if (update_flag) {

		/* update boot record(s) */
		rc = update_br ( nnum_more_blks, snum_more_blks);
 
		/* process the boot header */
		if (normserv == NORM_MODE || normserv == BOTH_MODE ){
			/* get boot header for normal blv */
			fd = open(norm_blv,O_RDWR);
			if (fd < 0)
				do_error("Could not open normal blv\n",NULL,1);
			rc = update_header(fd, sbuf.st_size);  
			close(fd);
  			if (!rc)
				do_error("Could not update normal boot header\n"
					  ,NULL,1);
		   	DATA("successfully updated normal blv header, new savebase file size= 0x%x bytes\n",sbuf.st_size,NULL);
		}

		if ( normserv == SERV_MODE || normserv == BOTH_MODE ) {  
			/* get boot header for normal blv */
			fd = open(serv_blv,O_RDWR);
			if (fd < 0)
				do_error("Could not open service blv\n",NULL,1);
			rc = update_header(fd, sbuf.st_size);  
			close(fd);
  			if (!rc)
				do_error("Could not update service boot header\n",NULL,1);
		   	DATA("successfully updated service blv header, new savebase file size= 0x%x bytes\n",sbuf.st_size,NULL);
		}
	
	} /* end if */
	return(0);
}

/*---------------------------------- update_br --------------------------------
 * NAME: update_br
 *
 * FUNCTION: Update the boot record and any mirrored disks.  This is
 *           done by obtaining the devnos and querying the database
 *  	     to obtain the hdisk names.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES: 	DEVNO array may contain up to 7 device numbers.
 * 		     -  the 1,2,3 locations are reserved for normal boot
 *			mirrored disks
 *		     -	the 4,5,6 locations are reserved for service boot
 *			mirrored disks
 *		     -	location 0 is reserved for the current boot ipldevice 
 *			devno
 *
 *			The array is initialized to -1 to indicate there
 *			is not a mirrored disk for either normal boot or
 *			service.
 *		       
 *			DEVNAME array may contain up to 7 device names
 *			The array is initialized to NULL to indicate there
 *			in not a mirrored disk 
 *	             -  location 0 is reserved for the current boot ipldevice
 *			name  
 *
 * RETURNS:  0 - success
 *           1 - exit through do_error call
 *
 *---------------------------------------------------------------------------*/
int
update_br (nsize, ssize)
int     nsize; 		/* additional num of normal blocks for savebase area  */
int     ssize; 		/* additional num of service blocks for savebase area */
{
	IPL_REC		disk_boot_record;
	struct	stat	sbuf;
	int		m,i,rc,num;
	int		devno[]   ={ -1, -1, -1, -1, -1, -1, -1};
	typedef         char		diskname[DEV_MAX_NM_SZ];
	diskname	devname[7]={NULL,NULL,NULL,NULL,NULL,NULL,NULL};
	struct	xlate_arg	xlate_arg;
	char   		sstr[256];
	struct	CuDvDr	cudvdr;
	int		disk_fd;
	int		loop_flag;
	long 		seek_pos;
	char	    	temp[100];
		

	/* check mode */
	if (normserv == NORM_MODE || normserv == BOTH_MODE ) {
		/* loop through the possible mirrors to obtain devnos */
		for (m=1; m<= NUM_MIRRORS; m++ ) {
			xlate_arg.lbn=0;
			xlate_arg.mirror = m;
			rc = ioctl( norm_fd, XLATE, &xlate_arg);
			if (rc<0) { /* if mirror does not exist */
				continue;
			}
			/* fill in devno array */
			devno[m]=xlate_arg.p_devt;
		}
	} /* end if normal mode */

	if (normserv == SERV_MODE || normserv == BOTH_MODE ) {
		/* loop through the possible mirrors to obtain devnos */
		for (m=1; m<= NUM_MIRRORS; m++ ) {
			xlate_arg.lbn=0;
			xlate_arg.mirror = m;
			rc = ioctl( serv_fd, XLATE, &xlate_arg);
			if (rc<0) { /* if mirror does not exist */
				continue;
			}
			/* fill in devno array */
			devno[m+NUM_MIRRORS]=xlate_arg.p_devt;
		}
	} /* end if service mode */

		 
 	/* get the devno & device name for the destination disk */
	rc = stat(destination,&sbuf);
	if (rc < 0 )
		do_error("Could not get file status on %s\n",destination,1);

	devno[0]=sbuf.st_rdev;
	strcpy (devname[0],destination);
	
	loop_flag = FALSE;

	/* obtain physical mirrored disk names & complete the devname array */
	for (m=1; m < TOTAL_DEVNO; m++) {
		/* check if there is a devno at this location */ 
		if (devno[m] == -1)
			 continue;
			
		/* obtain disk name from odm */
		sprintf(sstr,"resource=devno and value1=%d and value2=%d",
		            major(devno[m]), minor(devno[m]) );
		rc= (int)odm_get_first (CuDvDr_CLASS, sstr, &cudvdr);
		if ( rc == -1) {
			do_error("update_br: unable to obtain devno %d from odm\n",
			       	devno[m], odmerrno,1);	
		}
		else if (rc == 0)  /* goto next devno */
			continue;

		sprintf(devname[m],"/dev/%s", cudvdr.value3);
	} /* end for */


	/* open disk to update the lengths in boot record */

	for (m=0; m < TOTAL_DEVNO; m++) {
		   
		if ( !strcmp(devname[m],NULL) )
			continue;

		/* ensure no duplicates */
		for (i=1; i<=m; i++){
                        if (devno[m] == devno[i-1]){
				loop_flag = TRUE;
				break;
			}
		}

		if (loop_flag) {
			loop_flag=FALSE;
			continue;
		}

		/* open devname to read the boot record */
		if ((disk_fd = open(devname[m],O_RDWR)) < 0) {
			do_error("update_br: unable to open mirror %s\n",
				  devname[m],1);
		}

		/* read the boot record to update the lengths */

		num = read(disk_fd, &disk_boot_record, sizeof(IPL_REC) );
		if (num != sizeof(IPL_REC) ){
			close(disk_fd);
			do_error("update_br: error reading ipl record for disk %s\n",	
				  devname[m],1);
		}
		if ((normserv == NORM_MODE || normserv == BOTH_MODE) && m < 3 ) {
			/* update the boot rec. with the new savebase length */
			disk_boot_record.boot_lv_length = 
			disk_boot_record.boot_lv_length +  
			(nsize*SBSIZE / UBSIZE );

			disk_boot_record.boot_code_length = 
			disk_boot_record.boot_code_length +  
			(nsize*SBSIZE / UBSIZE );
		}
		if ((boot_record.basecs_start == boot_record.basecn_start)  ||
		   (normserv == SERV_MODE || normserv == BOTH_MODE) && m >= 3) {
			/* update the boot record with the new savebase len*/
			disk_boot_record.ser_lv_length = 
			disk_boot_record.ser_lv_length +  
			(ssize*SBSIZE / UBSIZE );

			disk_boot_record.ser_code_length = 
			disk_boot_record.ser_code_length +  
			(ssize*SBSIZE / UBSIZE );
		}
		/* seek to the beginning to write */
		seek_pos = lseek( disk_fd, 0L, 0 );
   		if (seek_pos < 0) {
	     		sprintf(temp,"update_br: unable to lseek to 0x%x : errno = %d\n",(long) 0,errno);
   	     		do_error(temp,NULL,1);
		}

		/* write boot_record updates */
		num = write(disk_fd, &disk_boot_record, sizeof(IPL_REC) ) ;
		if (num != sizeof(IPL_REC) ) {
			close(disk_fd);
			do_error("update_br: error writing ipl record for disk %s\n",devname[m],1);
		}
		DATA("update_br: successfully updated ipl rec for disk %s\n",
		      devname[m],NULL);
		close(disk_fd);
		
	} /* end for */

	return (0); 	    /* changed boot records and or ipl rec */ 
}
/*---------------------------------- update_header ----------------------
 * NAME: update_header
 *
 * FUNCTION: Seek to the third block on the blv. Obtain the text_start
 *	     point to seek to the start of the boot header.
 *	     Once the boot header info had been obtained, update boot
 *	     location [7] with the new savebase size.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:  1 if the boot header was successfully modified
 *	     0 boot header not changed
 *
 *---------------------------------------------------------------------------*/
int
update_header(fd, fsize)
int     fd;		/* blv file descriptor   */
off_t	fsize;		/* compressed file size  */
{
	int	    num;
	char	    temp[100];
	int	    num_blks;	/* number of 128K blocks of new savebase area */
	long	    text_start;
	long	    boot_header[128];/* buffer for reading and writing  */
	long	    seek_pos;	     /* byte position to save from seek	*/
		

	/* skip first 2 blocks on the blv to get to header info, first 2 blks
	 * are reserved	 */

  	seek_pos = lseek (fd, (long)2 *UBSIZE, 0);   
   	if (seek_pos < 0) {
	     sprintf(temp,"update_header: unable to lseek to 0x%x : errno = %d\n",
	    		(long)2*UBSIZE ,errno);
   	     do_error(temp,NULL,1);
	}

	num = read( fd, &read_buffer, UBSIZE);
	if ( num != sizeof(read_buffer) ) {
		sprintf(temp,"update_header: error reading norm blv: expecting %d, read %d\n",sizeof(read_buffer), num);
		do_error(temp,NULL,1);
	}
	/* check boot header magic number */          
	if ( XTOC.filehdr.f_magic == RS_TOC_NUM  ||
	     XTOC.filehdr.f_magic == OLD_TOC_NUM ) {
		/* find kernel entry point */
		text_start =
			XTOC.scnhdr[XTOC.aouthdr.o_sntext-1].s_scnptr;

		/* seek to start of boot header */
		seek_pos = lseek (fd, (long)text_start + (long)2*UBSIZE, 0);
   		if (seek_pos < 0) {
	     		sprintf(temp,"update_header: unable to lseek to 0x%x : errno = %d\n", (long)text_start + (long)2*UBSIZE, errno);
   	     		do_error(temp,NULL,1);
		}
		num = read (fd, &boot_header, UBSIZE);
		if ( num != UBSIZE ) {
			sprintf(temp,"update_header: error reading boot header: expecting %d, read %d\n",UBSIZE,num);
			do_error(temp,NULL,1);
		}
		/* update boot header with new savebase max size */
		num_blks =fsize / SBSIZE + 1 ;
		boot_header[7] = boot_header[6] + num_blks*SBSIZE;

		/* seek to start of boot header for write, use previous value */
		seek_pos = lseek (fd, (long)text_start + (long)2*UBSIZE, 0);
   		if (seek_pos < 0) {
	     		sprintf(temp,"update_header: unable to lseek to 0x%x : errno = %d\n", seek_pos,errno);
   	     		do_error(temp,NULL,1);
		}

		num = write (fd, &boot_header, UBSIZE);
		if ( num != UBSIZE ) {
			strcpy(temp,"update_header: error writing boot header");
			do_error(temp,NULL,1);
		}
		DATA("update_header: successfully updated blv header\n",
		      NULL,NULL);

		return(1);  /* successfully changed boot header */
	}
	return (0); 	    /* did not change boot header */ 
}


/*---------------------------------- write data -----------------------------
 * NAME: write_data
 *
 * FUNCTION: Based on the mode, write the data to the correct file or blv.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:  error on write failure
 *
 *---------------------------------------------------------------------------*/
 
write_data( num_bytes, buf)
int  num_bytes;		/* number of bytes read    */
uchar *buf;		/* buffer to write         */
{
	if (num_bytes < DBSIZE )
		for (; num_bytes < DBSIZE; buf[num_bytes++] = 0 );

	if (normserv != SERV_MODE )
		if (write(norm_fd,buf,DBSIZE) < 0)
                    do_error("unable to write to destination: errno = %d\n",
			      errno,1);

	if (normserv == SERV_MODE || normserv == BOTH_MODE ) 
		if (write(serv_fd,buf,DBSIZE) < 0)
                    do_error("unable to write to destination: errno = %d\n",
		              errno,1);
		
}

/*---------------------------------- get_lvcb --------------------------------
 * NAME: get_lvcb
 *
 * FUNCTION: Seek to the physical sector passed. Read a block and
 *           obtain the logical volume control block and 
 *           the logical name of the blv.
 *
 * EXECUTION ENVIRONMENT:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:  0 - success
 *           1 - exit through do_error call
 *
 *---------------------------------------------------------------------------*/
int
get_lvcb ( psn, lname)
unsigned int     psn; 		/* physical sector number  */
char    *lname; 		/* logical name of blv     */
{
        unsigned int     num;
	struct	 LVCB *lvcb;
 	char	 buf[512];
        char     temp[100];


	/* seek to physical sector number on blv */
        if( lseek( dest_fd, (off_t) psn*UBSIZE, 0) < 0 ) {
		sprintf(temp,"get_lvcb: unable to lseek to 0x%x : errno = %d\n",
			psn*UBSIZE,errno);
                do_error(temp,NULL,1);
        }

	/* get logical volume control block */
        num = read(dest_fd, &buf, sizeof(buf) );

        if( num != sizeof(buf) ) {
		sprintf(temp,"get_lvcb: error reading lvcb struct: expecting %d, read %d\n",sizeof(struct LVCB), num);
		do_error(temp,NULL,1);
        }

	lvcb = (struct LVCB *)buf;

	/* check magic number in lvcb */
	if ( strcmp(lvcb->validity_tag,LVCB_TAG) ) {
		DATA ("get_lvcb: logical volume tag mismatch, validity_tag %s not found\n",
			LVCB_TAG,NULL);
		do_error("get_lvcb: invalid lvcb validity tag!\n",NULL,1);   
	}

	/* build logical volume name */
        sprintf(lname,"/dev/%s",lvcb->lvname);
	DATA ("get_lvcb: logical volume name is %s \n",lname, NULL);
	return (0);
}
