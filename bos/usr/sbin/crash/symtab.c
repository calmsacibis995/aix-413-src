static char sccsid[] = "@(#)12  1.37  src/bos/usr/sbin/crash/symtab.c, cmdcrash, bos411, 9438B411a 9/20/94 11:57:31";
/*
 * COMPONENT_NAME: (CMDCRASH) 
 *
 * FUNCTIONS: rdsymtab,search,symsrch,prnm,nmsrch,prod,init_fullsym
 *            srch_knlist, get_non_exported
 *
 * ORIGINS: 27,3,83
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* 
 * LEVEL 1,5 Years Bull Confidential Information
 */  

/*              include file for message texts          */
#include "crash_msg.h" 
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

#include "crash.h"
#include <sys/dump.h>
#include <errno.h>
/***** full symbol table related *****/
#include <filehdr.h>		/* for reading symbol table in /unix */
#include <aouthdr.h>		/* for reading section sizes in /unix */
#include <syms.h>		    /* for reading symbol table in /unix */
#include <ctype.h>
#include <nlist.h>
#include <storclass.h>
/* To support dump_knlist() */
#include	<sys/malloc.h>
#include	<sys/syspest.h>
#include 	<sys/ldr.h>
#include 	<sys/ldr/ld_rbytes.h>
#include 	<sys/ldr/ld_data.h>
static   struct exp_index	exwk;
/* type  check field - this seems not to be declared elsewhere */
#define   	UNIVERSAL	'    '	/* four blanks are the universal hash value */
/* these access functions assume machine supports unaligned load */
#define      LANG(typchk)  (*(ushort*)(typchk))
#define      HASH(typchk)  (*(ulong*)((ulong)(typchk)+2))
#define      LANGHASH(typchk)  (*(ulong*)((ulong)(typchk)+6))


#define DATA 0x2  /* for scnm field in nlist structure */

#define MAXBUF 1024

struct nlist *symsrch();
extern 	int	dumpflag;
extern  struct   uthread x_thread;

#undef MST
#define MST x_thread.ut_save

extern	char	*namelist;
int fd_fullsym=0;	/* file desc for use reading real symbol table */
int fd_strtab;		/* file desc for use reading string table */

long f_symptr;		/* local copy of pointer to symbols from fd_fullsym */
long f_nsyms;		/* local copy of number of symbols from fd_fullsym */
int  maxstrlen;		/* length of longest string in string table */

extern struct cm *cm;

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
	"crash_csa","crash_LED", "gpa_vnode", "nhino"};

char *notexported_symnames[] = { "proc", "thread", "ppda", "__ublock",
	"crash_csa","crash_LED","gpa_vnode", "nhino"};

#define NUM_OF_EXPORTED	(sizeof(symnames)/sizeof(symnames[0]) \
                         - sizeof(notexported_symnames)/sizeof(symnames[0]))

#define NUM_OF_SYMS	sizeof(symnames)/sizeof(symnames[0])

struct  nlist  nltbl[NUM_OF_SYMS+1];

/* #define Display_Each_Symbol 1 */
/* #define Debug 1 */

/*
 * NAME: rdsymtab
 *                                                                    
 * FUNCTION: 
 *	There are 2 parts to the symbol table functionality:
 *	1) Open and position pointer to the symbol table in an
 *	   unstripped /unix.  This is to allow ts,ds,nm, trace and 
 *	   map to convert symbol names to addresses and vice versa.
 *	2) Make a local array of symbols gotten from the kernel
 *	   via the knlist() call or from the dump via the search() call.  
 *	   These symbols must be exported by the kernel in order to
 *	   be available to knlist().  These symbols are necessary
 *	   for crash to find the major kernel data structures.
 *
 * RETURNS:
 *	-1	on an error
 *	 0	on successful completion
 */
rdsymtab()
{
	int rc,i;
	struct syment *sp, *search();
	static void get_non_exported();
	
	/* Do part 1 of symbol initialization */
	init_fullsym();

	/* 	
	 *	Fill in addresses of symbols we are interested in
	 * 	using knlist() or search().
	 */

	for(i=0;i<NUM_OF_SYMS;i++) {
		nltbl[i].n_name = symnames[i];
		nltbl[i].n_value = 0;
	}
        nltbl[NUM_OF_SYMS].n_name = "";


	/* if this is a dump */
	if (dumpflag) {
                printf( catgets(scmc_catd, MS_crsh, SYMTAB_MSG_1, "Reading in Symbols ") );
		printf("\n");
                rc = nlist(namelist, &nltbl[0]);
                get_non_exported();
	}
	/* if this is not a dump */
	else {
		/* get the exported symbol values by using the knlist system call */
		rc = knlist(nltbl,NUM_OF_EXPORTED,sizeof (struct nlist));
		/* get the other symbol values */
                get_non_exported();
	}

	/* Display all symbols that were not found */
	for(i=1;i<NUM_OF_SYMS;i++) 
		if(!nltbl[i].n_value)
			printf( catgets(scmc_catd, MS_crsh, SYMTAB_MSG_3, "Symbol %s has null value.\n") , nltbl[i].n_name);

}

/* 
 * NAME: get_non_exported
 *
 * FUNCTION:
 * 	Search the addresses of the non exported symbols 
 *	
 * RETURNS:
 *	0  upon successful completion
 *	-1 otherwise
 *
 */
static void 
get_non_exported()
{
	int i;
	int rc;
	struct thread thread;
	int go_on = TRUE;
	int slot = 0;
	int cnt=0;
	ulong deref;
	
	V = symsrch("v");	
	g_kxsrval = symsrch("g_kxsrval");

	/* Read in the var structure for the vars subcommand */
	if((rc=readmem(&v,(long)SYM_VALUE(V), sizeof(v), CURPROC)) != sizeof(v)){
		printf( catgets(scmc_catd, MS_crsh, MISC_MSG_3, 
		"0452-179: Cannot read v structure from address 0x%8x.\n") ,SYM_VALUE(V));
	}

	for (i=NUM_OF_EXPORTED; i < NUM_OF_SYMS; i++) {
		if (!strcmp(nltbl[i].n_name, "proc")) {
			/* fetch it into the var structure */
			nltbl[i].n_value = v.vb_proc;
		}
		else if (!strcmp(nltbl[i].n_name, "thread")) {
			/* fetch it into the var structure */
			nltbl[i].n_value = v.vb_thread;
		}
		else {
			/* fetch it into /unix */
			rc = nlist(namelist, &nltbl[i]);
			/* the following code is a work around due to nlist incertain behaviour */
			if ((nltbl[i].n_sclass == C_HIDEXT) && (nltbl[i].n_scnum == DATA)) {
				readmem(&deref, (long) nltbl[i].n_value, sizeof (ulong), 0);
				nltbl[i].n_value = deref;
			}
		}
	}
}


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
 *		if matchname = 0, and sym.n_zeros = 0
 *			then *name is filled in with the name from the string
 *			table
 *
 *		if matchname = 1 and addr = 1
 *			then search will print out all symbol table
 *			entries that match.
 */
struct syment *
search(addr,name,matchname)
unsigned addr;
char *name;
int  matchname;
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
	if(!fd_fullsym) {
		printf( catgets(scmc_catd, MS_crsh, SYMTAB_MSG_4, "0452-353: No namelist file opened.\n") );
		return ((struct syment *)-1);
	}

	/* make sure that there is a symbol table in the namelist file */
	if (!f_nsyms) {
		printf( catgets(scmc_catd, MS_crsh, SYMTAB_MSG_6, "0452-355: No symbol table found in %s.\n") ,namelist);
		return((struct syment *)-1);
	}

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
			printf( catgets(scmc_catd, MS_crsh, SYMTAB_MSG_7, "Read error on symbol:%d, errno:%d.\n") ,i,errno);
			return ((struct syment *)-1);
		}

		/* If we are matching an address */
		if(!matchname) {
			/* forget no name symbols */
			if((sym.n_zeroes == 0) && (sym.n_offset == 0)) 
				continue;
			/*
			 * See if the value of this syment is closer
			 * to the value we are looking for.  If so,
			 * save the current value.
			 */
			
			if(sym.n_value<=addr && sym.n_value>value) {
				value = sym.n_value;
				saveidx = i;
			}
			/* if it matches exactly, then we're done */
			if (value == addr)
				break;

			/* Get the next syment */
			continue;
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
					printf( catgets(scmc_catd, MS_crsh, SYMTAB_MSG_7, "Read error on symbol:%d, errno:%d.\n") ,i+1,errno);
					return ((struct syment *)-1);
				}

				sym.n_numaux--;
				smtyp = aux.x_csect.x_smtyp & 0x07;
				smclas = aux.x_csect.x_smclas;
				scnlen = aux.x_csect.x_scnlen;

			}

                        save = sym;

			/*
			 * addr is used as a flag when matching names.
			 * If set, it indicates that the match should
			 * be printed and then the search should
			 * continue for other matches.
			 */
			if (addr) {
				/*
				 * print out the symbol, its value and
				 * its storage class type.
				 */
				found++;
				if (smtyp == XTY_ER) {
					if (smclas == XMC_PR)
						smtyp = XTY_LD;
					smclas = MAXCLASS;
				}
				printf("\t%08X ", save.n_value);
				if (smtyp == XTY_LD || smtyp == XTY_ER)
					printf("        ");
				else
					printf("%06X  ", scnlen);
				strcpy(classtr,"  ");
				for (j = 0; j< NSMCLAS; j++)
					if (smclass[j].class == smclas)
						strcpy(classtr,smclass[j].str);
				strcpy(typstr,"  ");
				for (j = 0; j< NSMTYP; j++)
					if (smtype[j].type == smtyp)
						strcpy(typstr,smtype[j].str);
				printf("%s %s    ", classtr, typstr);
				if (save.n_sclass == C_EXT)
					printf("%s\n",buf);
				else
					printf("<%s>\n",buf);

				/* go look for another match */
				continue;
			}
			/* Else we only want the first match */
			else
				/*
				 * return the match and let the caller
				 * print out the desired info 
				 */
				return(&save);
		}
	} /* end for */

	/*
	 * If matching an address, look up the name associated with the syment
	 * that matched closest.
	 */
	if(!matchname) {
		/* Seek into the symbol table to the syment that was closest */
		lseek(fd_fullsym, f_symptr + (saveidx * SYMESZ), 0);
		/* Read in the syment */
		if (read(fd_fullsym,&save,SYMESZ) != SYMESZ) {
			printf( catgets(scmc_catd, MS_crsh, SYMTAB_MSG_7, "Read error on symbol:%d, errno:%d.\n") ,saveidx,errno);
			return((struct syment *)-1);
		}
		/* If name is not in the syment struct */
		if(save.n_zeroes == 0) {
			/* 
			 * seek into string tab 
			 */
			lseek(fd_strtab, f_strtab + save.n_offset,0);
			/*
			 * read from string table and store in buf
			 */
			read(fd_strtab,readbuf,maxstrlen);
			strcpy(name,readbuf);
		}  
		/* else if the name was in the syment struct */
		else {	
			/*
			 * get sym name from syment structure
			 */
			strcpy(name,save.n_name);
		} /* else */
		return(&save);

	}
	if (!found)
		return(0);
	return(&save);
}


/*
 * NAME: symsrch
 *                                                                    
 * FUNCTION:  Searches the nlist structures that were read in upon 
 *		initialization for the given symbol.
 *                                                                    
 * RETURNS:
 *	0		if symbol not found
 *	struct *nlist	pointer to the nlist structure where symbol was found
 */  
struct nlist *

symsrch(s)
register char *s;
{
	register struct nlist  *found;
	register int i;

	found = 0;
	for(i=0; i < NUM_OF_SYMS; i++) {
		if(strcmp(nltbl[i].n_name, s) == 0) {
			found = &nltbl[i];
			break;
		}
	}
	return(found); 
}


/*
 * NAME: nmsrch
 *                                                                    
 * FUNCTION: finds all instances of the given symbol in the symbol table
 *		and displays them to the screen in assembler format
 *                                                                    
 */  
struct syment *
nmsrch(s)
	register  char  *s;
{
	char	ct[20];
	register  struct  syment  *sp;
	struct syment *search();

	if(strlen(s) > 19)
		return(0);
	if((sp = search(0, s, MATCHNAME)) == NULL) {
		strcpy(ct, ".");
		strcat(ct, s);
		sp = search(0, ct, MATCHNAME);
	}
	return(sp);
}

int
memread(addr, buf, cnt)
{
	ulong	ext;

	if (cm->cm_thread != -1) {
		/* verify that this is a valid segment for the process */
		if (verify(cm) == 0) {
			printf(catgets(scmc_catd, MS_crsh, SYMTAB_MSG_30,
				"Segment %d has become invalid for "
				"currently mapped thread\n"), cm->cm_reg);
			return(-1);
		}
		ext = cm->cm_segid;
	}

	if (!dumpflag && cm->cm_thread != -1) {
		lseek(mem, addr, 0);
		if (readx(mem, buf, cnt, ext) != cnt)
			return -1;
	}
	else {
		if (readmem_thread(buf, addr, cnt, cm->cm_thread) != cnt)
		       return -1;
	}
	return cnt;
}

/*
 * NAME: symtoaddr
 *                                                                    
 * FUNCTION:  Returns the address for the given symbol.  Handles
 *	* redirection, and $r# registers.  returns -1L on error.
 */  
unsigned long
symtoaddr(sym)
char *sym;
{
	struct syment *sp;
	unsigned long addr;

	if (*sym == '*') {
		sym++;
		if ((addr = symtoaddr(sym)) == -1)
			return -1;
		if (memread(addr, &addr, sizeof(addr)) < 0)
			return -1;
	}
	else if (*sym == '$') {
		ulong u_segval;

		(void) getuarea_thread(cm->cm_thread, &u_segval);

		sym++;
		switch (*sym) {
		case 'r': /* gp regs */
		    sym++;
		    if (isdigit(*sym)) {
			    int r = atoi(sym);
			    if (r < NGPRS)
				addr = MST.gpr[r];
			    else
				return -1L;
		    }
		    else
			    return -1L;
		    break;
		case 'i': /* iar */
		    addr = MST.iar;
		    break;
		case 'l': /* lr */
		    addr = MST.lr;
		    break;
		default:
		    return -1L;
		    break;
		}
	}
	else if (isdigit(*sym)) {
		addr = strtoul(sym, (char **)NULL, 16);
	}
	else {
		if ((sp = nmsrch(sym)) == NULL)
			return -1L;
                if ((sp->n_sclass == C_HIDEXT) && (sp->n_scnum == DATA)) {
                     /* due to the 4.1 binder change, the following dereference
                        is necessary for C_HIDEXT symbols 
                     */
                     readmem(&addr, (long) sp->n_value, sizeof(long), 0);
                }
                else addr = (long)sp->n_value;
	}
	return addr;
}

/*
 * NAME: prod 
 *                                                                    
 * FUNCTION: This is crash's od subcommand.  It prints 'units' data units to
 *		the screen using 'style' format starting from address 'addr'
 *                                                                    
 */  
prod(addr, units, style)
	unsigned	addr;
	int	units;
	char	*style;
{
	register  int  i;
	register  struct  prmode  *pp;
	char	space[4096];
	int	cun, j;
	int	word;
	int	*words = (int *)space;
	int	seg;
	ulong	ext;
	int	readx();
	long	lword, lw;
	long	*lwords = (long *)space;
	char	ch;
	unsigned char	c;
	char	*chs = space;
	char	ascii[17], s[5];
	extern	struct	prmode	prm[];
	extern	decode(unsigned long, unsigned long, char *);

	if(units == -1)
		return;

	seg = 0;
	
	/*
	 * if crash's map has been changed from the default using the
	 * cm subcommand
	 */
	if(cm->cm_thread != -1) {
		/* verify that this is a valid segment for the process or the thread */
		if(verify(cm) == 0) {
			printf( catgets(scmc_catd, MS_crsh, SYMTAB_MSG_30, "Segment %d has become invalid for currently mapped thread\n") ,
			cm->cm_reg);
			return(-1);
		}
		ext = cm->cm_segid;
	}

	/* convert the style string to an integer code */
	for(pp = prm; pp->pr_sw != 0; pp++) {
		if(strcmp(pp->pr_name, style) == 0)
			break;
	}

	/* Display the data using the format specified */
	switch(pp->pr_sw) {
	default:
		/* This was an invalid format */
		printf( catgets(scmc_catd, MS_crsh, SYMTAB_MSG_17, "Invalid mode") );
		break;

	case OCTAL:
	case DECIMAL:
		if(addr & 01) {
			printf( catgets(scmc_catd, MS_crsh, SYMTAB_MSG_18,
				"warning: word alignment performed\n") );
			addr &= ~01;
		}
		while (units > 0) {
			cun = (units > 4096/NBPW)? 4096/NBPW: units;
			units -= cun;
			if((!dumpflag) && (cm->cm_thread != -1)) {
				lseek(mem, addr, 0);
				if(readx(mem, words, NBPW*cun, ext)
					!= NBPW*cun) {
					printf( catgets(scmc_catd, MS_crsh,
						SYMTAB_MSG_19, "  read error"));
					units = 0;
					break;
				}
			}
			else {
				if(readmem_thread(words, addr, NBPW*cun, cm->cm_thread) 
				   != NBPW*cun) {
					units = 0;
					break;
				}
			}
			for(i = 0; i < cun; i++, addr += NBPW) {
				word = words[i];
				if (word == 0xbad) {
					units = 0;
					break;
			}
				if(i % 8 == 0) {
					if(i != 0)
						putc('\n', stdout);
					printf(FMT, (int)addr);
					printf(":");
				}
			printf(pp->pr_sw == OCTAL ? " %7.7o" :
				"  %5u", word);
		}
		}
		break;

	case LOCT:
	case LDEC:
		if(addr & 01) {
			printf( catgets(scmc_catd, MS_crsh, SYMTAB_MSG_20,
				"warning: word alignment performed\n") );
			addr &= ~01;
		}
		while (units > 0) {
			cun = (units > 4096/sizeof(long))?
				4096/sizeof(long): units;
			units -= cun;
			if((!dumpflag) && (cm->cm_thread != -1)) {
				lseek(mem, addr, 0);
				if(readx(mem, lwords, sizeof(long)*cun, ext) 
				   != sizeof(long)*cun) {
					printf( catgets(scmc_catd, MS_crsh,
						SYMTAB_MSG_21, "  read error"));
					units = 0;
					break;
				}
			}
			else {
				if(readmem_thread(lwords, addr, sizeof(long)*cun,
				   cm->cm_thread) != sizeof(long)*cun) {
					units = 0;
					break;
				}
			}
			for(i = 0; i < cun; i++, addr += sizeof(long)) {
				lword = lwords[i];
				if (lword == 0xbad) {
					units = 0;
					break;
			}
				if(i % 4 == 0) {
					if(i != 0)
						putc('\n', stdout);
					printf(FMT, (int)addr);
					printf(":");
				}
			printf(pp->pr_sw == LOCT ? " %12.12lo" :
				"  %10lu", lword);
		}
		}
		break;

	case CHAR:
	case BYTE:
		while (units > 0) {
			cun = (units > 4096)? 4096: units;
			units -= cun;
			if((!dumpflag) && (cm->cm_thread != -1)) {
				lseek(mem, addr, 0);
				if(readx(mem, chs, sizeof(char)*cun, ext) 
				   != sizeof(char)*cun) {
					printf( catgets(scmc_catd, MS_crsh,
						SYMTAB_MSG_22, "  read error"));
					units = 0;
					break;
				}
			}
			else {
				if(readmem_thread(chs, addr, sizeof(char)*cun,
				    cm->cm_thread) != sizeof(char)*cun) {
					units = 0;
					break;
				}
			}
			for(i = 0; i < cun; i++, addr += sizeof(char)) {
				if(i % (pp->pr_sw == CHAR ? 16 : 8) == 0) {
					if(i != 0)
						putc('\n', stdout);
					printf(FMT, (int)addr);
					printf(":");
				}
				ch = chs[i];
			if(pp->pr_sw == CHAR)
				putch(ch);
			else
				printf(" %4.4o", ch & 0377);
		}
		}
		break;
	case HEX:
		if(addr & 01) {
			printf( catgets(scmc_catd, MS_crsh, SYMTAB_MSG_23,
				"warning: word alignment performed\n") );
			addr &= ~01;
		}
		while (units > 0) {
			cun = (units > 4096/sizeof(long))?
				4096/sizeof(long): units;
			units -= cun;
			if((!dumpflag) && (cm->cm_thread != -1)) {
				lseek(mem, addr, 0);
				if(readx(mem, lwords, sizeof(long)*cun, ext) 
				   != sizeof(long)*cun) {
					printf( catgets(scmc_catd, MS_crsh,
						SYMTAB_MSG_24, "  read error"));
					units = 0;
					break;
				}
			}
			else {
				if(readmem_thread(lwords, addr, sizeof(long)*cun,
				   cm->cm_thread) != sizeof(long)*cun) {
					units = 0;
					break;
				}
			}
			for(i = 0; i < cun; i++, addr += sizeof(long)) {
				lword = lwords[i];
				if (lword == 0xbad) {
					units = 0;
					break;
				}
			if(i % 4 == 0) {
				if(i != 0)
					putc('\n', stdout);
				printf(FMT, (int)addr);
				printf(":");
			}
				printf(" %08x", lword);
			}
		}
		break;
	case ASCII:		/* hex with ascii sidebar */
		while (units > 0) {
			cun = (units > 4096/sizeof(long))?
				4096/sizeof(long): units;
			units -= cun;
			if((!dumpflag) && (cm->cm_thread != -1)) {
				lseek(mem, addr, 0);
				if(readx(mem, lwords, sizeof(long)*cun, ext) 
				   != sizeof(long)*cun) {
					printf( catgets(scmc_catd, MS_crsh,
						SYMTAB_MSG_24, "  read error"));
					units = 0;
					break;
				}
			}
			else {
				if(readmem_thread(lwords, addr, sizeof(long)*cun,
				   cm->cm_thread) != sizeof(long)*cun) {
					units = 0;
					break;
				}
			}
			*ascii = '\0';
		        for(i = 0; i < cun; i++, addr += sizeof(long)) {
		       	     lword = lwords[i];
			     if (lword == 0xbad) {
				units = 0;
				break;
		             }
			     if(i % 4 == 0) {
				if(i != 0) {
					printf("  |%s|\n", ascii);
					*ascii = '\0';
				}
				printf(FMT, (int)addr);
				printf(":");
			     }
			     printf(" %08x", lword);
                             s[4] = '\0';
                             lw = lword;
                             for (j=3; j>=0; j--, lw=lw >> 8) {
                                c =  lw & 0xff;
                                if (c < ' ' || c > 127)
                                        s[j] = '.';
                                else
                                        s[j] = c;
                             }
			     strcat(ascii, s);
		        }
			if (*ascii != '\0') {
				for (; i % 4 != 0; i++)
					printf("         ");
				printf("  |%s|", ascii);
			}
			if (units > 0)
				printf("\n");
		}
		break;

	case INST:
		if(addr & 03) {
			printf( catgets(scmc_catd, MS_crsh, SYMTAB_MSG_18, "warning: word alignment performed\n") );
			addr &= ~03;
		}
		for(i = 0; i < units; i++, addr += NBPW) {
			if((!dumpflag) && (cm->cm_thread != -1)) {
				lseek(mem, addr, 0);
				if(readx(mem, &word, NBPW, ext) != NBPW) {
					printf( catgets(scmc_catd, MS_crsh, SYMTAB_MSG_19, "  read error") );
					break;
				}
			}
			else {
				if(readmem_thread(&word, addr, NBPW, cm->cm_thread) 
				   != NBPW) {
					break;
				}
				if (word == 0xbad)
					break;
			}
			/* Decode the instruction. */
			decode(word,addr,ascii);
			printf("%08x %s\n",addr,ascii);
		}
		break;
	}
	putc('\n', stdout);
}


/*
 * NAME: init_fullsym
 *                                                                    
 * FUNCTION: 
 *	   Open and position pointer to the symbol table in an
 *	   unstripped /unix.  This is to allow ts,ds,nm, and map to
 *	   convert symbol names to addresses and vice versa.
 *
 * RETURNS: 
 *	NONE 
 *
 *	Globals: sets 	fd_fullsym	- file descriptor set at symtab
 *		 sets   fd_strtab	- file descriptor for string table
 */
int
init_fullsym()
{
	struct filehdr fh;
	struct syment *sp;
	ulong  addr;
	struct aouthdr ah;
	int    i, cnt, len;
	char buf[1024];

#ifdef Debug
	printf("init_fullsym:namelist:%s\n",namelist);
#endif
	/* open namelist */
	if(namelist[0]=='\0') 
		return;
	
	if((fd_fullsym=open(namelist,0))<0) {
		printf( catgets(scmc_catd, MS_crsh, SYMTAB_MSG_25, "cannot open namelist file\n") );
		fd_fullsym = 0;
		return;
	}
	if (read(fd_fullsym,&fh,sizeof(fh)) != sizeof(fh))
		fatal( catgets(scmc_catd, MS_crsh, SYMTAB_MSG_5, "read error in namelist file") );
	if (!fh.f_nsyms) {
		printf( catgets(scmc_catd, MS_crsh, SYMTAB_MSG_6, "0452-355: No symbol table found in %s.\n") ,namelist);
		return;
	}
	f_nsyms = fh.f_nsyms;
	f_symptr = fh.f_symptr;

	/* dup namelist fd to use on string table */
	if((fd_strtab=open(namelist,0))<0) {
		fatal( catgets(scmc_catd, MS_crsh, SYMTAB_MSG_26, "cannot dup namelist file") );
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
	if (fh.f_opthdr) {
		if (read(fd_fullsym,&ah,sizeof(ah)) == sizeof(ah)) {
			if (sp = search(0, "endcomm", MATCHNAME)) {
				if(readmem(&addr, sp->n_value,
				   sizeof(sp->n_value), CURPROC) 
				   == sizeof(sp->n_value)) {
                                        /* Because the text section is aligned at the
                                           double word boundary, we need to round tsize 
                                           up to the double word boundary
                                        */
					if (addr != ( ((ah.tsize + 0x7) & 0xFFFFFFF8) 
                                                + ah.dsize + ah.bsize)) {
						printf( catgets(scmc_catd, MS_crsh, SYMTAB_MSG_27, 
						"WARNING: dumpfile does not appear to match namelist\n"));
					}
				}
                                else {
						printf( catgets(scmc_catd, MS_crsh, SYMTAB_MSG_27, 
						"WARNING: dumpfile does not appear to match namelist\n"));
                                }
			}
		}
	}
			
#ifdef Debug
	printf("init_fullsym: fd_fullsym:%d,fd_strtab:%d\n",fd_fullsym,
		fd_strtab);
#endif

	/* maybe malloc space for copy of symtab */
}

/* srch_knlist
 *
 * Function:  Do a knlist or a dump_knlist depending upon whether we're
 *	on a running system or a dump.
 *
 * Return:  Same as knlist()
 *	0=success
 *	-1=error
 */
int
srch_knlist(
struct  nlist *nl,		/* pointer to array of nlist struct	*/
int	nelem,			/* number of user nlist struct		*/
int	sizelem)		/* size of expected nlist structure	*/
{
	return( (dumpflag)?dump_knlist(nl,nelem,sizelem):
	                   knlist(nl,nelem,sizelem) );
}
 
 /*
  * dump_knlist
  *
  * Function:  Do a knlist on a dump.
  */
int
dump_knlist(
struct  nlist *nl,		/* pointer to array of nlist struct	*/
int	nelem,			/* number of user nlist struct		*/
int	sizelem)		/* size of expected nlist structure	*/
{
	register int ret;		/* remember return code		*/
	uint	elem;
#define	MAX_EXT 32
	struct exp_index *ex;
	uint	actual;
	struct loader_entry kexwk;
	struct loader_entry *kernel_exports;

	ret = 0;
	/* parameter ok and number of symbols is 1 or more	*/
	if (sizelem != sizeof(struct nlist) || nelem < 1)
	{
		return(-1);
	}

	/* Setup kernel_exports pointer */
	if ((kernel_exports=symtoaddr("**kernel_exports"))==-1) return(-1);
	if (memread(kernel_exports,&kexwk,sizeof(kexwk))!=sizeof(kexwk))
		return(-1);

	/* while elements repeat search, zero tmp, increment user ptr	*/
	for (elem=0;elem<nelem;elem++){
		/*
		 * Call the xcoff loader routine to look for symbols.
		 * If symbol found, load tmp nl array with values from
		 *	ldsymp structure returned from loader:
		 *	n_value, n_scnum, n_type, n_sclass, n_numaux
		 */
		ex = ld_symfind(kexwk.le_exports,nl[elem].n_name);

		if (ex){
			nl[elem].n_value = (long)ex->exp_location;
		}
		else {	
			char *stmp = nl[elem].n_name;
			ret = -1;
			bzero(&nl[elem],sizeof nl[elem]);
			nl[elem].n_name = stmp;
		}
			
	}
	/* successful search, return GOOD value	*/
	return(ret);
}


/* ld_hash computes a universal hash of the input string
 * by using random values, it avoids being sensitive to
 * patterns in the input.
 * It is called with a string pointer and a MAXIMUM length
 * It returns the hash and the true length of the string NOT including the NULL
 */

unsigned
ld_hash(
caddr_t      sp,
ulong        *sl)
{
    unsigned               hash;
    ulong                  i,len,inlen,byte;
    i = 0;
    hash = 0;
    inlen = len = *sl;
    /* rbytes is 128 words long */
    for(; len && (byte = (ulong)*sp++); len--)
	hash = hash + rbytes[i = (i + byte)&(127)];
    *sl = inlen - len;
    return hash;
    }

/* ld_symfind will find a symbol in the hash table and return
 * the control block (exp_index) which describes it
 * it is passed the loader exports table which it is to search
 */

struct exp_index *
ld_symfind(
struct loader_exports	*lx,
caddr_t	symstring)
{
#define SYM_MAX 64
    ulong	symlength,hash;
    struct exp_index	*ex;
    struct loader_exports lxwk;
    char symwk[SYM_MAX];

    if ((NULL == lx) || (strlen(symstring) > SYM_MAX))
    	return NULL;
    symlength = 1<<30;             /* hash will compute true length */
    hash = ld_hash(symstring,&symlength);
    if (memread(lx,&lxwk,sizeof(lxwk))!=sizeof(lxwk)) {
	return(NULL);
    }
    if (memread(&(lxwk.hashes[hash & lxwk.hash_mask]),&ex,sizeof(ex))!=sizeof(ex))
	return(NULL);
    for(; ex != NULL; ex = exwk.next_index) {
	if (memread(ex,&exwk,sizeof(exwk))!=sizeof(exwk)) {
		return(NULL);
	}
        if (exwk.fullhash != hash) continue;
        if ( (symlength == exwk.syml) &&
	     (memread(exwk.sym,symwk,symlength)==symlength) &&
	     (0==memcmp(symwk,symstring,symlength)) )
	       return &exwk;
    }
    return NULL;
}
