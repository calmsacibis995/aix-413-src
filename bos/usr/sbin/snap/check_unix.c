static char sccsid[] = "@(#)88  1.1  src/bos/usr/sbin/snap/check_unix.c, cmdsnap, bos411, 9433A411a 8/5/94 13:09:38";
/*
 * COMPONENT_NAME: cmdsnap
 *
 * FUNCTIONS: search,init_dump,check_dump
 *
 * ORIGINS: 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993, 1994
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "check.h"
#include <sys/dump.h>
#include <errno.h>
#include <filehdr.h>	
#include <aouthdr.h>
#include <syms.h>
#include <ctype.h>
#include <nlist.h>
#include <storclass.h>
#include <sys/syspest.h>
#include <sys/malloc.h>
#include <sys/ldr.h>
#include <sys/ldr/ld_rbytes.h>
#include <sys/ldr/ld_data.h>
#include <sys/param.h>

#define   	UNIVERSAL	'    '	
#define DATA 0x2  
#define MAXBUF 1024
/* these access functions assume machine supports unaligned load */
#define      LANG(typchk)  (*(ushort*)(typchk))
#define      HASH(typchk)  (*(ulong*)((ulong)(typchk)+2))
#define      LANGHASH(typchk)  (*(ulong*)((ulong)(typchk)+6))

extern struct uthread x_thread;
extern struct nlist *Proc;
extern struct nlist *Thread;
extern struct nlist *Ppd, *Kernel_heap;
extern struct nlist *Ublock, *g_kxsrval;
extern struct cm *cm;

static struct exp_index	exwk;
struct uthread x_thread;
struct user x;
char dumpfile[MAXPATHLEN];
cpu_t ncpus;

#undef MST
#define MST x_thread.ut_save

char	*namelist = "/unix";
int fd_fullsym=0;	/* file desc for use reading real symbol table */
int fd_strtab;		/* file desc for use reading string table */
long f_symptr;		/* local copy of pointer to symbols from fd_fullsym */
long f_nsyms;		/* local copy of number of symbols from fd_fullsym */
int  maxstrlen;		/* length of longest string in string table */

/***** knlist/loadlist symbol table related *****/
/* NOTE:
 * This table contains exported and unexported symbol names.
 * The exported symbol names MUST appear first in the table.
 * Those not exported from the kernel must be at the end of this
 * list and also put into the notexported_symnames list.
 */
char *symnames[] = {
	"v" , "hinode",
	"file","utsname", "time", "lbolt", "panicstr", "kernel_heap",
	"pinned_heap", "vmker","mbufs","mbclusters",
	"rootvfs", "g_kxsrval", "buf_hdr", "kernel_anchor", "ifnet", 
	"_system_configuration", "dbp_dmodsw", "dbp_fmodsw",
	"dbp_sth_open_streams", "dbp_streams_runq",
	/* Start of unexported symbols */
	"proc", "thread", "ppda", "__ublock",
	"crash_csa","crash_LED", "gpa_vnode"};

char *notexported_symnames[] = { "proc", "thread", "ppda", "__ublock",
	"crash_csa","crash_LED","gpa_vnode"};

#define NUM_OF_EXPORTED	(sizeof(symnames)/sizeof(symnames[0]) \
                         - sizeof(notexported_symnames)/sizeof(symnames[0]))

#define NUM_OF_SYMS	sizeof(symnames)/sizeof(symnames[0])

struct  nlist  nltbl[NUM_OF_SYMS+1];

/*
 * NAME: search
 *                                                                    
 * FUNCTION: 
 *  	Search the symbol table in an unstriped namelist file (e.g. /unix)
 *
 * RETURNS:
 *	 0 if no match (this is impossible since addr is unsigned),
 *	-1 if error, 
 *	*struct syment that matched 
 *
 */
struct syment *
search(addr,name)
unsigned addr;
char *name;
{
	long f_strtab;
	int saveidx;
	int found = 0;
	struct syment sym;
	union auxent aux;
	static struct syment save;
	int 	i,j,k,rc,first, value = 0;
	char	c,buf[MAXBUF],readbuf[MAXBUF];
	char	typstr[3], classtr[3];
	static struct smtype {
		unsigned char type;
		char str[3];
	} smtype[] = {
		XTY_ER,"ER",
		XTY_SD,"SD",
		XTY_LD,"LD",
		XTY_CM,"CM",
		XTY_EM,"EM",
		XTY_US,"US"
	};

#define NSMTYP	(sizeof(smtype)/sizeof(struct smtype))

	static struct smclass {
		unsigned char class;
		char str[3];
	} smclass[] = {
		XMC_PR,"PR",
		XMC_RO,"RO",
		XMC_DB,"DB",
		XMC_GL,"GL",
		XMC_XO,"XO",
		XMC_SV,"SV",
		XMC_TI,"TI",
		XMC_TB,"TB",
		XMC_RW,"RW",
		XMC_TC0,"T0",
		XMC_TC,"TC",
		XMC_DS,"DS",
		XMC_UA,"UA",
		XMC_BS,"BS",
		XMC_UC,"UC"
	};

#define NSMCLAS	(sizeof(smclass)/sizeof(struct smclass))
#define MAXCLASS 0xff;

	save.n_value = 0;

	/* test if namelist file was found */
	if(!fd_fullsym) 
		return ((struct syment *)-1);

	/* make sure that there is a symbol table in the namelist file */
	if (!f_nsyms) 
		return((struct syment *)-1);

	/* Create a pointer to the beginning of the string table */
	f_strtab = f_symptr + (SYMESZ * f_nsyms);
		
	/* Go to the beginning of the symbol table */
	lseek(fd_fullsym,f_symptr,0);

	/*
	 *  Search through the symbol table entries for a matching symbol
	 *  or address.
	 */
	for (i=0,saveidx=0,sym.n_numaux = 0;i<f_nsyms;i++) {
		errno = 0;

		/*
		 * If the last syment read had aux entries, then
		 * seek past the aux entries
		 */
		i += sym.n_numaux;
		lseek(fd_fullsym,sym.n_numaux*SYMESZ,1);

		/* read the next syment */
		if(read(fd_fullsym,&sym,SYMESZ) != SYMESZ) {
			return ((struct syment *)-1);
		}

		/* 
		 * if n_zeroes = 0 then look in string tab for name 
		 */
		if(sym.n_zeroes == 0) {
			/* 
			 * seek into string table 
			 */
			lseek(fd_strtab, f_strtab + sym.n_offset,0);
			/*
			 * We read in the longest string possible
			 * since we don't know how long the string
			 * will be.  Then, we use strcpy and copy
			 * just the string wanted from the read buffer
			 * into the working buffer.
			 */
			read(fd_strtab,readbuf,maxstrlen);
			strcpy(buf,readbuf);
		}  
		/*
		 * else get sym name from syment structure
		 */
		else
			strcpy(buf,sym.n_name);

		if (name[0] != '.' && buf[0] == '.') 
			rc = strcmp(name, &buf[1]);
		else 
			rc = strcmp(name, buf);
	
		/* if we found our match */
		if (rc == 0) {
			char *s;
			unsigned char smtyp,smclas;
			long scnlen;

			if (sym.n_numaux) {
				/* read the aux entry */
				if(read(fd_fullsym,&aux,AUXESZ) != AUXESZ) {
					return ((struct syment *)-1);
				}

				sym.n_numaux--;
				smtyp = aux.x_csect.x_smtyp & 0x07;
				smclas = aux.x_csect.x_smclas;
				scnlen = aux.x_csect.x_scnlen;

			}

                        save = sym;

			/*
			 * return the match and let the caller
			 * print out the desired info 
			 */
			return(&save);
		}
	} /* end for */

	if (!found)
		return(0);
	return(&save);
}

/*
 * NAME: main
 *                                                                    
 * FUNCTION: 
 *       This calls a routine to read in information about
 * 	a given dumpfile and check that it matches /unix.
 *
 * RETURNS: 
 *	NONE 
 */
int
main(argc, argv)
int argc;
char *argv[];
{
	strcpy(dumpfile,argv[1]);

	init_dump();

	check_dump(); /* exits */
}

/*
 * NAME: check_dump
 *
 * FUNCTION:
 *         Open and position pointer to the symbol table in an
 *         unstripped /unix.  Check to see if the dumpfile supplied 
 *         matches the /unix file.   
 *
 * EXITS:
 *	0 - the dumpfile and /unix match
 *	1 - the dumpfile and /unix do not match
 *     -1 - error
 *
 *      Globals: sets   fd_fullsym      - file descriptor set at symtab
 *               sets   fd_strtab       - file descriptor for string table
 */
int
check_dump()
{
        struct filehdr fh;
        struct syment *sp;
        ulong  addr;
        struct aouthdr ah;
        int    i, cnt, len;
        char buf[1024];

        /* open namelist */
        if(namelist[0]=='\0')
                exit (-1);

        if((fd_fullsym=open(namelist,0))<0) {
			exit (-1);
        }
        if (read(fd_fullsym,&fh,sizeof(fh)) != sizeof(fh))
            exit (-1); 

        if (!fh.f_nsyms) {
            exit (-1);
        }
        f_nsyms = fh.f_nsyms;
        f_symptr = fh.f_symptr;

        /* dup namelist fd to use on string table */
        if((fd_strtab=open(namelist,0))<0) {
            exit (-1); 
        }

        /* Seek to the string table */
        lseek(fd_strtab, f_symptr + (f_nsyms * SYMESZ),0);

        /* find the length of the longest string in the table */
        for (i = 0, len = 0; (cnt = read(fd_strtab,buf,1024)) != 0; i=0) {
                while(1) {
                        for (;i < cnt && buf[i] != '\0';i++,len++);
                        if (i == cnt)
                                break;
                        i++;
                        if (len > maxstrlen)
                                maxstrlen = len;
                        len = 0;
                }
        }
        maxstrlen += 1;

        /*
         * Check to see if the dumpfile matches the namelist.  This is
         * done by checking to see that 'endcomm' in the dumpfile corresponds
         * to the sum of the sizes of the text, data, and bss areas in the
         * namelist.
         */
        if (fh.f_opthdr) 
			{
            if (read(fd_fullsym,&ah,sizeof(ah)) == sizeof(ah)) 
				{
            	if (sp = search(0, "endcomm")) 
					{
                    if(readmem(&addr, sp->n_value, sizeof(sp->n_value),CURPROC)
                                   == sizeof(sp->n_value)) 
						{
                        if (addr != ( ((ah.tsize + 0x7) & 0xFFFFFFF8)
                                      + ah.dsize + ah.bsize)) 
							{
							/* The dumpfile does NOT match the /unix */
							exit (1);
							}
						else
							{
							/* The dumpfile MATCHES the /unix */
							exit (0);
							}
                        }
                    else 
						{
						/* The dumpfile does NOT match the /unix */
						exit (1);
                        }
                    }
				else
					{
					/* There was no endcomm found. */ 
					exit (1);
					}
                }
        	}

	exit (-1);
}

/*
 * NAME:  init_dump
 *
 * FUNCTION: This routine is called from main and is used to initialize
 *	     the dump file.  	
 *
 */
init_dump()
{
        extern  char *get_cdtname();
        extern int bos_cdt, proc_cdt;
        extern int thrd_cdt;
        char    *p;
        int     i;
        int procslot;
        int threadslot;

        /*
         * Open the dumpfile that is to be used.  This is
         * the file that was designated on the command line.
         */
        if((mem = open(dumpfile, 0)) < 0) {
                exit(-1);
        }

        /* determine if we are working on a dump */
        for(i=1;i<MAXCDTS;i++) {
                if((p = get_cdtname(i))==0)
                        break;
                /* look for bos cdt & save index */
                if(!strcmp(p,"bos"))
                        bos_cdt = i;
                /* look for proc cdt & save index */
                else if(!strcmp(p,"proc"))
                        proc_cdt = i;
                else if(!strcmp(p,"thrd"))
                        thrd_cdt = i;
        }

}
