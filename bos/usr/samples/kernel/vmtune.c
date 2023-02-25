static char sccsid[] = "@(#)59  1.7  src/bos/usr/samples/kernel/vmtune.c, kersamp, bos41J, 9520B_all 5/18/95 12:46:21";
/*
 * COMPONENT_NAME: (SYSVMM) Virtual Memory Manager
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1993, 1995
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * This program assumes that /unix is the running kernel.
 *
 * The validity checks can be enhanced.
 * The capability to input minperm and maxperm as a number of frames is easy
 * to add.  For now, both are inputted as a percentage of real memory
 *
 * If no parameters are specified, then the current settings are shown.
 * If vmtune is executed with a bad flag (ex. -?), then the usage is shown.
 *
 */

#include <stdio.h>                                                              
#include <fcntl.h>                                                              
#include <nlist.h>                                                              
#include <sys/sysinfo.h>                                                        
#include <sys/m_param.h>                                                        
#include <sys/vminfo.h>                                                         
#include <sys/iostat.h>                                                         
#include <sys/vmker.h>
#include "vmdefs.h"
#include "vmpfhdata.h"
#include "vmsys.h"
#include <sys/ioctl.h>
#include <sys/dasd.h>
#include <sys/vmuser.h>
                                                                                

#define	PFHOFFSET	((off_t)&((struct vmmdseg *)0)->pf)
#define VMKERMAXPERM    ((off_t)&((struct vmkerdata *)0)->maxperm)
#define VMKERPFRSVDBLKS ((off_t)&((struct vmkerdata *)0)->pfrsvdblks)
#define PFMINPERM       ((off_t)&((struct pfhdata *)0)->minperm)
#define PFMINPGAHEAD    ((off_t)&((struct pfhdata *)0)->minpgahead)
#define PFMAXPGAHEAD    ((off_t)&((struct pfhdata *)0)->maxpgahead)
#define PFMINFREE       ((off_t)&((struct pfhdata *)0)->minfree)
#define PFMAXFREE       ((off_t)&((struct pfhdata *)0)->maxfree)
#define PFNPSWARN       ((off_t)&((struct pfhdata *)0)->npswarn)
#define PFNPSKILL       ((off_t)&((struct pfhdata *)0)->npskill)
#define VMKERPDNPAGES   ((off_t)&((struct vmkerdata *)0)->pd_npages)



/*
 * VALID VALUES
 */
#define MINMINPERM_P   1.0  /* smallest possible value of minperm as % of mem */
#define MAXMINPERM_P 100.0  /* largest possible value of minperm as % of mem */
#define MINMAXPERM_P   1.0  /* smallest possible value of maxperm as % of mem */
#define MAXMAXPERM_P 100.0  /* largest possible value of maxperm as % of mem */
#define MINMINPA       0    /* smallest possible value of minpgahead */       
#define MAXMINPA      4096  /* largest possible value of minpgahead */
#define MINMAXPA       0    /* smallest possible value of maxpgahead */
#define MAXMAXPA      4096  /* largest possible value of maxpgahead */
#define MINMINFREE     8    /* smallest possible value of minfree */
#define MAXMINFREE 204800   /* largest possible value of minfree */
#define MINMAXFREE    16    /* smallest possible value of maxfree */
#define MAXMAXFREE 204800   /* largest possible value of maxfree */
#define MINLVMBUFCNT   1    /* smallest possible value of lvm_bufcnt */
#define MAXLVMBUFCNT  64    /* largest possible value of lvm_bufcnt */
#define MINPDNPAGES    1    /* smallest chunk size for deleting pers. pages */
#define MAXPDNPAGES    MAXFSIZE/PSIZE	/* largest chunk size for pers. pages */
#define MINRANDWRT	0   /* smallest value for max random write threshold */
                                                                                
#define READ 0
#define WRITE 1


/* If adding more nlist symbols, give each symbol a define with value
 * equal 1 greater than previous symbol.  Then set NUMVARS to be 1 greater
 * than the last symbol.  These values are used as indexes into the ndata
 * array.
 */
#define VMKER 0
#define VMMPFHDATA 1
#define NUMCLUST 2
#define NUMFSBUFS 3
#define MAXRANDWRT 4
#define NUMVARS 5

struct nlist ndata[] = {
	{ "vmker" },
	{ "vmm_pfhdata" },
	{ "numclust" },
	{ "numfsbufs" },
	{ "maxrandwrt" },
	NULL
};

/*
 * If adding more knlist symbols, give each symbol a define with value
 * equal 1 greater than previous symbol.  Then set KNNUMVARS to be 1 greater
 * than the last symbol.  These values are used as indexes into the kndata
 * array.
 */

#define HD_PBUF_CNT 0
#define HD_PBUF_MIN 1
#define HD_PBUF_GRAB 2
#define HD_PVS_OPN 3
#define LVM_BUFCNT 4
#define NUMKNVARS 5

struct nlist kndata[] = {
	{ "hd_pbuf_cnt" },
	{ "hd_pbuf_min" },
        { "hd_pbuf_grab" },
        { "hd_pvs_opn" },
        { "lvm_bufcnt" },
	NULL
};

#define PBUF_GRAB_INCR 1

int t_frames;			/* total number of memory frames        */
struct vmkerdata vmker_s;	/* struct used to hold vmkerdata values */
struct pfhdata   vmpfh_s;	/* struct used to hold vmpfhdata values */
int vars[NUMVARS];		/* array used to hold values for ndata  */
int knvars[NUMKNVARS];		/* array used to hold values for kndata */

main(argc, argv)                                                                
int  argc;                                                                      
char *argv[];                                                                   
{                                                                               
	int    	kmemfd,memfd;
	int    	nrc, flag,i;
	int 	Mflag=0, pflag=0, Pflag=0, rflag=0, Rflag=0, fflag=0, Fflag=0;
	int	kflag=0, wflag=0, cflag=0, bflag=0, Bflag=0, uflag=0, Nflag=0;
	int	Wflag=0, exitflag=0;
	
	extern 	char *optarg;
	extern 	int optind;
	struct 	vmkerdata *vmker_ptr = NULL;
	struct 	pfhdata *vmpfh_ptr = NULL;
	float  	maxperm_p, minperm_p, maxpin_p;
	int 	minfree, maxfree;
	int 	minpgahead, maxpgahead;
	int	npswarn, npskill, pd_npages;
	int     hd_pbuf_cnt, orig_hd_pbuf_grab=0, orig_hd_pbuf_min=0, lvm_bufcnt=0;
	int 	vmmsrval;
	

	/* Open memory */                     

	if ((kmemfd=open("/dev/kmem", O_RDWR)) < 0)
		pexit("Can't open /dev/kmem!");                         

									
	if ( ! (nrc = nlist("/unix", ndata)) )
	{
#ifdef Version_3_nlist
		nrc = lseek(kmemfd, ndata[VMKER].n_value, SEEK_SET);
		if ( nrc >= 0 )
			read(kmemfd, &vmker_ptr, sizeof(struct vmkerdata *) );
#else
		vmker_ptr = (struct vmkerdata *) ndata[VMKER].n_value;

		/*
		 * Get the address of the pfhdata structure
		 */

		nrc = lseek(kmemfd, ndata[VMMPFHDATA].n_value, SEEK_SET);
		if ( nrc >= 0 )
			read(kmemfd, &vmpfh_ptr, sizeof(struct vmkerdata *) );
		else
			pexit("lseek for vmm_pfhdata failed");
#endif
	}
	else
	{
		pexit("nlist on /unix failed");
	}

	if ( !vmker_ptr )
	{
		fprintf(stderr,"%s: invalid vmker ptr\n", argv[0]);
		exit(1);
	}

	/*
	 * get variables exported by kernel to other modules
	 */
	if (  nrc = knlist(kndata, NUMKNVARS, sizeof(struct nlist)) )
		perror("knlist");

	/*
	 * Open /dev/mem to get VMM stats
	 */
	if ((memfd=open("/dev/mem", O_RDWR)) < 0)
		pexit("Can't open /dev/mem!");  
	
									
	/*
	 * Get the vmker values from /dev/kmem
	 */
	rd_vm_vars(kmemfd, &vmker_s, vmker_ptr ); 

	/* read nlist (ndata) symbols */
        rdwr_gen_vars(kmemfd,&vars[NUMCLUST],ndata[NUMCLUST].n_value, READ );
        rdwr_gen_vars(kmemfd,&vars[NUMFSBUFS],ndata[NUMFSBUFS].n_value, READ );
        rdwr_gen_vars(kmemfd,&vars[MAXRANDWRT],ndata[MAXRANDWRT].n_value, READ );


	/* read knlist (kndata) symbols */
        rdwr_gen_vars(kmemfd,&knvars[HD_PBUF_CNT],kndata[HD_PBUF_CNT].n_value,
                READ );
        rdwr_gen_vars(kmemfd,&knvars[HD_PBUF_MIN],kndata[HD_PBUF_MIN].n_value,
                READ );
        rdwr_gen_vars(kmemfd,&knvars[HD_PBUF_GRAB],kndata[HD_PBUF_GRAB].n_value,
                READ );
        rdwr_gen_vars(kmemfd,&knvars[HD_PVS_OPN],kndata[HD_PVS_OPN].n_value,
                READ );
	rdwr_gen_vars(kmemfd, &knvars[LVM_BUFCNT], kndata[LVM_BUFCNT].n_value,
		READ );


	/*
	 * Get the value of the VMM segment register
	 * We need this for readx and writex (for vmpfhdata)
	 */
	vmmsrval = vmker_s.vmmsrval;

	/*
	 * Get the total number of memory frames
	 * Compensate for PPC memory architecture
	 */
	t_frames = vmker_s.nrpages - vmker_s.badpages;



	/*
	 * Get the pfhdata values from /dev/mem
	 */
	rd_vm_vars_x(memfd, &vmpfh_s, vmpfh_ptr, vmmsrval );


	/*
	 * print the current values 
	 */
	display_values( vmker_s, vmpfh_s, vars, knvars, "current" );


	/*
	 * if no arguments are present, exit
	 */
	if ( argc < 2 )
	{
   		close(kmemfd);                                        
   		close(memfd);                                              
		exit(0);
	}


	/*
	 * Parse the command line
	 */
	while ( (flag = getopt(argc,argv,"W:B:c:b:k:w:M:p:P:r:R:f:F:u:N:")) != EOF )
	{
		switch ( flag ) {
		case 'M':
			maxpin_p = (float)atoi(optarg);
			Mflag = 1;
			break;
		case 'p':
			minperm_p = (float)atoi(optarg);
			pflag = 1;
			break;
		case 'P':
			maxperm_p = (float)atoi(optarg);
			Pflag = 1;
			break;
		case 'c':
			vars[NUMCLUST] = atoi(optarg);
			cflag = 1;
			break;
		case 'b':
			vars[NUMFSBUFS] = atoi(optarg);
			bflag = 1;
			break;
		case 'B':
			hd_pbuf_cnt = atoi(optarg);
			Bflag = 1;
			break;
		case 'u':
			lvm_bufcnt = atoi(optarg);
			uflag = 1;
			break;
		case 'k':
			npskill = atoi(optarg);
			kflag = 1;
			break;
		case 'w':
			npswarn = atoi(optarg);
			wflag = 1;
			break;
		case 'r':
			minpgahead = atoi(optarg);
			rflag = 1;
			break;
		case 'R':
			maxpgahead = atoi(optarg);
			Rflag = 1;
			break;
		case 'f':
			minfree = atoi(optarg);
			fflag = 1;
			break;
		case 'F':
			maxfree = atoi(optarg);
			Fflag = 1;
			break;
		case 'N':
			pd_npages = atoi(optarg);
			Nflag = 1;
			break;
		case 'W':
			vars[MAXRANDWRT] = atoi(optarg);
			Wflag = 1;
			break;
		case '?':
			show_usage (argv[0]);
			exitflag++;
		}
	}



	/*
	 * perform validity checks
	 */

	if ( Mflag )
	{
		if ( maxpin_p < 100 && maxpin_p > 1  )
		{
			vmker_s.pfrsvdblks = ((100-maxpin_p)*t_frames)/100;
		}
		else
		{
		     fprintf(stderr,"maxpin must be greater than 0 and less than 100\n");
		     exitflag++;
		}
	}

	if ( uflag )
	{
		if ( lvm_bufcnt < MINLVMBUFCNT || lvm_bufcnt > MAXLVMBUFCNT )
		{
		     fprintf(stderr,"lvm_bufcnt must be greater than %d and less than %d\n",MINLVMBUFCNT-1, MAXLVMBUFCNT-1);
		     exitflag++;
		}
	}

	if ( Bflag )
	{
		/* can't decrease number of pbufs in system */
		if ( hd_pbuf_cnt <= knvars[HD_PBUF_CNT] )
		{
		    fprintf(stderr,"hd_pbuf_cnt must be greater than %d\n",
						knvars[HD_PBUF_CNT]);
		    exitflag++;
		}

	}

	if ( pflag )
	{
		if ( minperm_p >= MINMINPERM_P && minperm_p <= MAXMINPERM_P )
			vmpfh_s.minperm = (minperm_p*t_frames)/100;
		else
		{
		     fprintf(stderr,"minperm_p must be between %3.1f and %3.1f \n",
				 MINMINPERM_P, MAXMINPERM_P);
		     exitflag++;
		}
	}

	if ( Pflag )
	{
		if ( maxperm_p >= MINMAXPERM_P && maxperm_p <= MAXMAXPERM_P )
			vmker_s.maxperm = (maxperm_p*t_frames)/100;
		else
		{
		     fprintf(stderr,"maxperm_p must be between %3.1f and %3.1f\n",
				 MINMAXPERM_P, MAXMAXPERM_P);
		     exitflag++;
		}
	}



	if ( fflag )
	{
		if ( minfree >= MINMINFREE && minfree <= MAXMINFREE )
			vmpfh_s.minfree = minfree;
		else
		{
		     fprintf(stderr,"minfree value must be between %d and %d \n",
				 MINMINFREE, MAXMINFREE);
		     exitflag++;
		}
	}

	if ( Fflag )
	{
		if ( maxfree >= MINMAXFREE && maxfree <= MAXMAXFREE )
			vmpfh_s.maxfree = maxfree;
		else
		{
		     fprintf(stderr,"maxfree value must be between %d and %d\n",
			 	MINMAXFREE, MAXMAXFREE);
		     exitflag++;
		}
	}

	if ( rflag )
	{
		if ( minpgahead >= MINMINPA && minpgahead <= MAXMINPA )
			vmpfh_s.minpgahead = minpgahead;
		else
		{
		  fprintf(stderr,"minpgahead value must be between %d and %d\n",
			 MINMINPA, MAXMINPA);
		  exitflag++;
		}
	}

	if ( wflag )
	{
		if ( npswarn > 0 || npswarn < vmker_s.numpsblks )
			vmpfh_s.npswarn = npswarn;
		else
		{
			fprintf(stderr,"npswarn value must be greater than 0\n");
			exitflag++;
		}
	}
	if ( kflag )
	{
		if ( npskill > 0 || npskill < vmker_s.numpsblks )
			vmpfh_s.npskill = npskill;
		else
		{
			fprintf(stderr,"npskill value must be greater than 0\n");
			exitflag++;
		}
	}
	if ( cflag )
	{
		if ( vars[NUMCLUST] < 0 )
		{
			fprintf(stderr,"numclust value must be greater than or equal to 0\n");
			exitflag++;
		}
	}
	if ( bflag )
	{
		if ( vars[NUMFSBUFS] <= 0 )
		{
			fprintf(stderr,"numfsbufs value must be greater than 0\n");
			exitflag++;
		}
	}


	if ( Rflag )
	{
		if ( maxpgahead >= MINMAXPA && maxpgahead <= MAXMAXPA )
			vmpfh_s.maxpgahead = maxpgahead;
		else
		{
		  fprintf(stderr,"maxpgahead value must be between %d and %d\n",
			 MINMAXPA, MAXMAXPA);
		  exitflag++;
		}
	}

	if ( Nflag )
	{
                if ( pd_npages < MINPDNPAGES || pd_npages > MAXPDNPAGES )
                {
                  fprintf(stderr,"pd_npages value must be between %d and %d\n",
                         MINPDNPAGES, MAXFSIZE/PSIZE);
                  exitflag++;
                }
                else
                {
                        vmker_s.pd_npages = pd_npages;
                }

	}

	if ( Wflag )
	{
		if ( vars[MAXRANDWRT] < MINRANDWRT )
		{
		   fprintf(stderr,"maxrandwrt value must be >= %d\n",MINRANDWRT);
		   exitflag++;
		}
	}
		
		
	if ( vmpfh_s.minperm > vmker_s.maxperm )
	{
	     	fprintf(stderr,"minperm_p must be <= maxperm_p\n");
	     	exitflag++;
	}

	if ( vmpfh_s.minfree > vmpfh_s.maxfree )
	{
	     	fprintf(stderr,"minfree value must be <= maxfree\n");
	     	exitflag++;
	}
			
	if ( vmpfh_s.minpgahead  > vmpfh_s.maxpgahead )
	{
	  	fprintf(stderr,"minpgahead value must be <= maxpgahead\n");
	  	exitflag++;
	}

	/* check to see if time to quit */
	if ( exitflag > 0 )
		exit(exitflag);

	if ( uflag )
	{
			knvars[LVM_BUFCNT] = lvm_bufcnt;
			rdwr_gen_vars(kmemfd, &knvars[LVM_BUFCNT],
				kndata[LVM_BUFCNT].n_value, WRITE );
	}

	if ( Bflag )
	{
		int fd;
		if ( (fd = open("/dev/rootvg", O_RDWR, 0)) > 0 )
		{
			/* grab_incr_size is set to 1 to give us the smallest
			 * granularity possible for increasing the total
			 * number of pbufs.  The actual granularity will
			 * depend on the number of disks
			 */
			int new_grab_size,  ioctl_rc=0;

			if ( ! knvars[HD_PVS_OPN] )
				pexit("no disks active");

			/*
			 * LVM uses following formula to calculate pbufs:
			 * numpbufs=((hd_pvs_opn-1)*hd_pbuf_grab) + hd_pbuf_min;
			 * First pv gets 64 pbufs, and others get 16 each.
			 * So we need to set hd_pbuf_min such that the numpbufs
			 * will be exactly equal to what the user wanted.
			 * The grab value used in the PBUFCNT ioctl must be
			 * greater than hd_pbuf_grab in order for LVM to
			 * increase the pbufs. That's why we use a grab value 
			 * of hd_pbuf_grab+PBUF_GRAB_INCR;
			 * Alternatively, we could have set hd_pvs_opn to 1,
			 * set hd_pbuf_min to user's requested number of pubfs,
			 * called the ioctl with any value, and reset 
			 * hd_pvs_opn back to original value. However, if a
			 * varyon or varyoff was happening just at this split
			 * instant, things may get out of whack.
			 */
			orig_hd_pbuf_grab = knvars[HD_PBUF_GRAB];
			orig_hd_pbuf_min = knvars[HD_PBUF_MIN];
			new_grab_size = orig_hd_pbuf_grab + PBUF_GRAB_INCR;
				
			knvars[HD_PBUF_MIN] = hd_pbuf_cnt -
				( (knvars[HD_PVS_OPN] - 1) *  new_grab_size ) ;
			/*
			 * change hd_pbuf_min so lvm will change
			 * the hd_pbuf_cnt value with the ioctl
			 */
                        rdwr_gen_vars(kmemfd, &knvars[HD_PBUF_MIN],
                                kndata[HD_PBUF_MIN].n_value, WRITE );


			ioctl_rc = ioctl(fd,PBUFCNT,&new_grab_size);

			/*
			 * Set hd_pbuf_grab back to original since the ioctl
			 * may have changed the value.  This is done because
			 * a larger grab value will generate a much larger pbuf
			 * pool when a new disk is added. This is okay if the
			 * new disk requires a lot of pbufs. However, if the
			 * disk doesn't, then don't penalize the user.  If it
			 * does require more pbufs, then use vmtune again to
			 * increase it.
			 */
			knvars[HD_PBUF_GRAB] = orig_hd_pbuf_grab;
                        rdwr_gen_vars(kmemfd, &knvars[HD_PBUF_GRAB],
                                kndata[HD_PBUF_GRAB].n_value, WRITE );


			if ( ioctl_rc < 0 )
			{
				perror("PBUFCNT ioctl failed");
			/* reset hd_pbuf_min back to original value */
				knvars[HD_PBUF_MIN] = orig_hd_pbuf_min;
                                rdwr_gen_vars(kmemfd, &knvars[HD_PBUF_MIN],
                                        kndata[HD_PBUF_MIN].n_value, WRITE );

			}
			else
                        {
                        /*
                         * Set hd_pbuf_min to a value such that the original
                         * formula for calculating numpbufs is still true.
			 * If hd_pbuf_min is set to hd_pbuf_cnt, then the
			 * number of pbufs added when a new disk is added
			 * will not really be the grab size as it is when
			 * not using vmtune. That's why hd_pbuf_min is set
			 * such that the formula still works (since the grab
			 * size won't be changing either).
                         */
                                knvars[HD_PBUF_MIN] = hd_pbuf_cnt -
			( (knvars[HD_PVS_OPN] - 1) * (knvars[HD_PBUF_GRAB] ) ) ;
                                rdwr_gen_vars(kmemfd, &knvars[HD_PBUF_MIN],
                                        kndata[HD_PBUF_MIN].n_value, WRITE );

        		}
		}
		else
		{
        		fprintf(stderr,"error opening /dev/rootvg, fd=%d\n",fd);
		}
    	}

	if ( Mflag )
		wr_value(kmemfd, (off_t) vmker_ptr+VMKERPFRSVDBLKS, 
						vmker_s.pfrsvdblks, -1 );

	if ( pflag )
		wr_value(memfd, (off_t) vmpfh_ptr+PFMINPERM, vmpfh_s.minperm,
						vmmsrval );

	if ( Pflag )
		wr_value(kmemfd, (off_t) vmker_ptr+VMKERMAXPERM,
						 vmker_s.maxperm, -1 );

	if ( fflag )
		wr_value(memfd, (off_t) vmpfh_ptr+PFMINFREE, vmpfh_s.minfree,
						vmmsrval );

	if ( Fflag )
		wr_value(memfd, (off_t) vmpfh_ptr+PFMAXFREE, vmpfh_s.maxfree,
						vmmsrval );

	if ( rflag )
		wr_value(memfd, (off_t) vmpfh_ptr+PFMINPGAHEAD,
						vmpfh_s.minpgahead, vmmsrval );

	if ( Rflag )
		wr_value(memfd, (off_t) vmpfh_ptr+PFMAXPGAHEAD,
						vmpfh_s.maxpgahead, vmmsrval );

	if ( wflag )
		wr_value(memfd, (off_t) vmpfh_ptr+PFNPSWARN, vmpfh_s.npswarn,
						vmmsrval );

	if ( kflag )
		wr_value(memfd, (off_t) vmpfh_ptr+PFNPSKILL, vmpfh_s.npskill,
						vmmsrval );

	if ( cflag )
                rdwr_gen_vars(kmemfd,&vars[NUMCLUST],ndata[NUMCLUST].n_value,
                        WRITE );

	if ( bflag )
                rdwr_gen_vars(kmemfd,&vars[NUMFSBUFS],ndata[NUMFSBUFS].n_value,
                        WRITE );

        if ( Nflag )
                wr_value(kmemfd, (off_t) vmker_ptr+VMKERPDNPAGES,
                                                 vmker_s.pd_npages, -1 );

	if ( Wflag )
		rdwr_gen_vars(kmemfd,&vars[MAXRANDWRT],ndata[MAXRANDWRT].n_value,			WRITE );




	/*
	 * read 'em again
	 */
	rd_vm_vars(kmemfd, &vmker_s, vmker_ptr ); 
	rd_vm_vars_x(memfd, &vmpfh_s, vmpfh_ptr, vmmsrval );
        rdwr_gen_vars(kmemfd,&vars[NUMCLUST],ndata[NUMCLUST].n_value, READ );
        rdwr_gen_vars(kmemfd,&vars[NUMFSBUFS],ndata[NUMFSBUFS].n_value, READ );
        rdwr_gen_vars(kmemfd,&vars[MAXRANDWRT],ndata[MAXRANDWRT].n_value, READ );
        rdwr_gen_vars(kmemfd,&knvars[HD_PBUF_CNT],kndata[HD_PBUF_CNT].n_value,
                READ );
        rdwr_gen_vars(kmemfd,&knvars[HD_PBUF_MIN],kndata[HD_PBUF_MIN].n_value,
                READ );
        rdwr_gen_vars(kmemfd,&knvars[HD_PBUF_GRAB],kndata[HD_PBUF_GRAB].n_value,
                READ );
        rdwr_gen_vars(kmemfd,&knvars[HD_PVS_OPN],kndata[HD_PVS_OPN].n_value,
                READ );
	rdwr_gen_vars(kmemfd, &knvars[LVM_BUFCNT], kndata[LVM_BUFCNT].n_value,
		READ );


	/*
	 * Now display the values again
	 */
	display_values( vmker_s, vmpfh_s, vars, knvars, "new" );


   	close(kmemfd);                                        
   	close(memfd);                                              
	exit(0);
}







wr_value ( fd, offset, value, vmmsrval )
int fd;
off_t offset;
int value, vmmsrval;
{
	int rdvalue=0;

	if (lseek(fd, offset, SEEK_SET) < 0)
		pexit("lseek failed!");

	if ( vmmsrval != -1 )
	{
		if (writex(fd,(char *)&value, sizeof(int), vmmsrval) != sizeof(int))
			pexit("writex to memory failed!");
	}
	else
	{
		if (write(fd,  &value, sizeof(int) ) != sizeof(int))
			pexit("write to memory failed!");
	}
}



/*
 * Read vmker data structure values from /dev/kmem
 */

rd_vm_vars( fd, vmker_s, vmker_ptr )
int fd;
struct vmkerdata *vmker_s;
struct vmkerdata *vmker_ptr;
{

	int addr;

	if (lseek(fd, (off_t) vmker_ptr, SEEK_SET) == -1)
		pexit("lseek for vmker failed!");

	/* read  vmker data structure from the kernel data segment */
	if (read(fd, vmker_s, sizeof(struct vmkerdata)) !=
						 sizeof(struct vmkerdata))
		pexit("read for vmker failed!");
}




/*
 * Read pfhdata structure values from  /dev/mem
 */

rd_vm_vars_x( fd, vmpfh_s, vmpfh_ptr, vmmsrval )
int fd;
struct pfhdata *vmpfh_s;
struct pfhdata *vmpfh_ptr;
int vmmsrval;
{
	if (lseek(fd, (off_t) vmpfh_ptr, SEEK_SET) == -1)
		pexit("lseek for vmpfhdata failed!");

	/*
	 * Read in the inital values in vmm data segment
	 * We need the vmm seg register val from the vmker data structure also
	 * We get this from the previous read of vmker
	 */            
	if (readx(fd,(char *) vmpfh_s,sizeof(struct pfhdata),vmmsrval) !=
						 sizeof(struct pfhdata))
		pexit("readx of vmpfhdata failed!");
}



rdwr_gen_vars(fd, sym, offset, mode)
int fd;
int *sym;
off_t offset;
int mode;
{

        if (lseek(fd, offset, SEEK_SET) == -1)
                pexit("rdwr_gen_vars: first lseek failed");

#ifdef Version_3_nlist
       /* read the toc to handle version 3 nlist */
	if (read(fd, &offset, sizeof(off_t)) != sizeof(off_t))
		pexit("rdwr_gen_vars: first read failed");

	/* lseek again to the real address */
	if (lseek(fd, offset, SEEK_SET) == -1)
		pexit("rdwr_gen_vars: second lseek failed");
#endif

        if (mode == READ)
        {
            if (read(fd, sym, sizeof(int)) != sizeof(int))
                pexit("rdwr_gen_vars: read failed");
        }
        else
        {
            if (write(fd, sym, sizeof(int)) != sizeof(int))
                pexit("rdwr_gen_vars: write failed");
        }

}




									
/*
 * Display the current values of the interesting variables
 */

display_values( vmker_s, vmpfh_s, vars, knvars, values )
struct vmkerdata vmker_s;
struct pfhdata vmpfh_s;
int vars[];
int knvars[];
char *values;
{

	float maxperm_p, minperm_p, maxpin_p, numperm_p;

	maxperm_p = 100.0 * (float)vmker_s.maxperm / (float)t_frames;
	minperm_p = 100.0 * (float)vmpfh_s.minperm / (float)t_frames;
	maxpin_p = 100.0-(100.0 *  (float)vmker_s.pfrsvdblks  / (float)t_frames);
	numperm_p = 100.0 * (float)vmker_s.numperm / (float)t_frames;

	printf("vmtune:  %s values:", values);
	printf("\n  -p       -P        -r          -R         -f       -F       -N        -W\n");
	printf("minperm  maxperm  minpgahead maxpgahead  minfree  maxfree  pd_npages maxrandwrt\n");
	printf("%7d  %7d   %5d      %5d    %7d  %7d    %7d  %7d\n", vmpfh_s.minperm, vmker_s.maxperm, vmpfh_s.minpgahead, vmpfh_s.maxpgahead, vmpfh_s.minfree, vmpfh_s.maxfree,vmker_s.pd_npages,vars[MAXRANDWRT]);
	printf("\n  -M       -w       -k       -c         -b          -B          -u\n");
	printf("maxpin   npswarn  npskill  numclust  numfsbufs   hd_pbuf_cnt  lvm_bufcnt\n");
	printf("%7d  %7d  %7d  %7d  %7d      %7d     %7d\n", t_frames - vmker_s.pfrsvdblks, vmpfh_s.npswarn, vmpfh_s.npskill, vars[NUMCLUST], vars[NUMFSBUFS], knvars[HD_PBUF_CNT],knvars[LVM_BUFCNT]);
	printf("\n");
	printf("number of valid memory pages = %d\tmaxperm=%3.1f%% of real memory\n", t_frames, maxperm_p);
	printf("maximum pinable=%3.1f%% of real memory\tminperm=%3.1f%% of real memory\n", maxpin_p,minperm_p);
	printf("number of file memory pages = %d\tnumperm=%3.1f%% of real memory\n\n", vmker_s.numperm,numperm_p);

}



/*
 * Show a usage and exit
 */

show_usage ( name )
char *name;
{
	fprintf(stderr,"\nUsage: %s [-p n] [-P n] [-r n] [-R n] [-f n] [-F n][-M n][-w n][-k n]\n",name);
	fprintf(stderr,"               [-c n][-b n][-B n][-N n][-u n][-W n]\n");
	fprintf(stderr,"\t\t-p\t minperm expressed as a %% of real memory (ex. 4)\n");
	fprintf(stderr,"\t\t-P\t maxperm expressed as a %% of real memory (ex. 80)\n");
	fprintf(stderr,"\t\t-r\t specifies value of minpgahead in 4k pages\n");
	fprintf(stderr,"\t\t-R\t specifies value of maxpgahead in 4k pages\n");
	fprintf(stderr,"\t\t-f\t specifies value of minfree in 4k pages\n");
	fprintf(stderr,"\t\t-F\t specifies value of maxfree in 4k pages\n");
	fprintf(stderr,"\t\t-M\t maximum pinable memory frames expressed\n");
	fprintf(stderr,"\t\t  \t as a %% of real memory\n");
	fprintf(stderr,"\t\t-w\t specifies value of npswarn in 4k pages\n");
	fprintf(stderr,"\t\t-k\t specifies value of npskill in 4k pages\n");
	fprintf(stderr,"\t\t-c\t specifies value of numclust\n");
	fprintf(stderr,"\t\t-b\t specifies value of numfsbufs\n");
	fprintf(stderr,"\t\t-B\t specifies value of hd_pbuf_cnt\n");
        fprintf(stderr,"\t\t-N\t specifies value of pd_npages\n");
	fprintf(stderr,"\t\t-u\t specifies value of lvm_bufcnt\n");
	fprintf(stderr,"\t\t-W\t specifies value of maxrandwrt\n");

}

pexit(string)
char *string;
{
	perror(string);
	exit(1);
}
