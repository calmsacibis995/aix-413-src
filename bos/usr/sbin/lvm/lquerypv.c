static char sccsid[] = "@(#)35	1.11.1.4  src/bos/usr/sbin/lvm/lquerypv.c, cmdlvm, bos41B, 412_41B_sync 1/3/95 09:08:27";

/*
 * COMPONENT_NAME: (cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS: getflg, ck_required, setdef, main, output
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * FILE: lquerypv.c
 *
 * FILE DESCRIPTION: Source file for lquerypv program.  "lquerypv" provides
 *     a command level user interface to the lvm low level function lvm_querypv.
 *
 *     For additional information on this program and the other lvm low level command
 *     programs see the specification "Logical Volume Manager Low-level Command
 *     Interface" and also the interface documentation to the lvm library functions.
 */


#include <lvm.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>

/*              include file for message texts          */
#include "cmdlvm_def.h"
#include "cmdlvm_msg.h"
#include <locale.h>



/*
 * EXTERNAL PROCEDURES CALLED: NONE
 */

/* local library functions */
extern int setflg();
extern int get_uniq_id();
extern char *lvm_msg();

/*
 * GLOBAL VARIABLES
 */

/* flags for required values */
#define VGID    1     /* error return code if neither vg id nor pv name is specified on command line */
#define PVID	2     /* error return code if pv id not specified on command line */

#define ALIGN 16
#define UBSIZE 512
#define DONTSEEK -1
#define ISFIFO  (S_ISFIFO(sbuf.st_mode))
#define ISREG   (S_ISREG(sbuf.st_mode))
#define ISBLK   (S_ISBLK(sbuf.st_mode))
#define ISCHR   (S_ISCHR(sbuf.st_mode))
#define READLEN ((pastit-position)>UBSIZE) || ISCHR ? UBSIZE: pastit-position
#define STDINSTR "-"                    /* Use standard in              */


/* external variables */
extern int optind;		 /* index to next argv argument - incremented by getopt      */
extern char *optarg;		 /* pointer to command line argument string - set by getopt  */
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */


/* DUMMY VALUE TO BE REPLACED LATER */
static char ODM_type[]= "ODMtype";

extern char *Prog;					       /* points to name of current Program  */

/* Pointer to command line parameter associated with command line option.	  */
/* This variable only contain a value if corresponding flag is true - see below.  */
static char *g_value;
static char *p_value;
static char *N_value;
static char *h_path_value;

/* Each flag corresponds to a command option - flag true if option specified on command line */
static int g_flag;
static int p_flag;
static int N_flag;
static int s_flag;
static int c_flag;
static int P_flag;
static int n_flag;
static int a_flag;
static int D_flag;
static int d_flag;
static int A_flag;
static int t_flag;
static int h_flag;

off_t	h_offset_value = 0;
int	h_length_value = 0x100;
int	fd = 0;
off_t	seekval;		/* Where to seek to             */
ulong	seekoffset;	/* offset from seekval          */
struct	stat sbuf;	/* fstat output                 */


ulong getval(s)
char *s;
{
        ulong i;

        sscanf(s,"%lx",&i);
        return(i);
}


/*
  convert pvid to ascii string (33 bytes total)
 */

#define makehex(x) "0123456789abcdef"[x&15]

char *pvidtoa(ip)
struct unique_id *ip;
{
	int i;
	char *ptr = (char*)ip;
	static char buf[33];
	
	for(i=0;i<32;i++) {
		if (i&1)
			buf[i] = makehex(ptr[i/2]);
		else
			buf[i] = makehex(ptr[i/2]>>4);
	}
	buf[32] = '\0';
	return(buf);
}

/*
 * NAME: getflg
 *
 * FUNCTION: Parse command line options until the first non-option is encountered.
 *
 * (NOTES:) For each option in the command line, a global flag is set and a global
 *     value is loaded with any accompaning command argument.
 *
 * RETURN VALUE DESCRIPTION: Return negative value (-1) if illegal option found.
 *			     Return positive value (1) if options parsed correctly.
 *			     Return zero if no command options found.
 */

int getflg(argc,argv)

int argc;	 /* number of command line string in referenced by argv */
char **argv;	 /* array of command line strings */
{
	struct unique_id uid;
	int rc; 		/* getopt return value */

	if (!argc) return(0);	/* if no command line options to parse then return */

	/* switch on each option contained in the command line */
	while ((rc = getopt(argc, argv, "g:p:N:hscPnaDdAti")) != EOF) {
		switch (rc) {
		case 'g':
			g_flag = 1;
			g_value = optarg;
			break;
		case 'p':
			p_flag = 1;
			p_value = optarg;
			break;
		case 'N':
			N_flag = 1;
			N_value = optarg;
			break;
		case 'h':
			if (argc > 5)
				return(-1);
			h_flag = 1;
			if (argc == 5)
			{
				h_length_value = getval(argv[4]);
				h_length_value = (h_length_value + ALIGN-1) & -ALIGN;
			}
			if (argc >= 4)
			{
				h_offset_value = getval(argv[3]);
				h_offset_value &= -ALIGN;
			}
			if (argc >= 3)
				h_path_value = argv[2];
			break;
		case 's':
			s_flag = 1;
			break;
		case 'c':
			c_flag = 1;
			break;
		case 'P':
			P_flag = 1;
			break;
		case 'n':
			n_flag = 1;
			break;
		case 'a':
			a_flag = 1;
			break;
		case 'D':
			D_flag = 1;
			break;
		case 'd':
			d_flag = 1;
			break;
		case 'A':
			A_flag = 1;
			break;
		case 't':
			t_flag = 1;
			break;
		case 'i':
			mkuuid(&uid);
			printf("%s\n",pvidtoa(&uid));
			exit(0);
		default:
		case '?':
			Usage_error();
			return(-1);
		}
	}
	
	return(1);
}


/*
 * NAME: ck_required
 *
 * FUNCTION: Check input flag indicators to determine if all required fields
 *     have been received.  Print error message if missing field is found.
 *
 * RETURN VALUE DESCRIPTION: If zero then no error.  If not zero then error (the
 *    non-zero value indicates which required flag was not set).
 *
 */

static int ck_required()
{
	/* allow the usage of -h to override any required flags */
	if (h_flag)
		return(0);
	if (!p_flag) {
		fprintf(stderr, lvm_msg(MISSING_PVID), Prog);
		Usage_error();
		return(PVID);
	}
	if (!g_flag && !N_flag) {
		fprintf(stderr, lvm_msg(MISSING_VGPV), Prog);
		Usage_error();
		return(VGID);
	}
	return(0);
}


/*
 * NAME: output
 *
 * FUNCTION: Send requested data to stdout - if tag has been selected then tag the data.
 *
 * (NOTES): The flags specify what data was received from lvm - this is the data to
 *          be output.  The "A" and "a" flags represent a group of data fields.
 *
 *          A tag field is output with each value if t_flag is true.
 *
 * RETURN VALUE DESCRIPTION: No return value.
 */


static int output(q,pv_id)

struct querypv *q;     /* lvm structure containing the query data to be output */
struct unique_id *pv_id;
{
	int tag = 12;  /* printf format value - used for string width and precision */
	register i;        /* offset into map */
	struct pp_map *p;  /* map partition record */
	char *ptr;         /* pointer to output string - the map name */

	/* if no query flags were set then use the default - the "a" flag */
	if (!A_flag && !d_flag && !a_flag && !s_flag && !c_flag && !D_flag
	 && !P_flag && !n_flag) a_flag = 1;

	if (A_flag || a_flag || s_flag) {
		if (t_flag) printf("%-*.*s\t",tag,tag,"PP Size:");
		printf("%1d\n",q->ppsize);
	}
	if (A_flag || a_flag || c_flag) {
		if (t_flag) printf("%-*.*s\t",tag,tag,"PV State:");
		printf("%1d\n",q->pv_state);
	}
	if (A_flag || a_flag || P_flag) {
		if (t_flag) printf("%-*.*s\t",tag,tag,"Total PPs:");
		printf("%1d\n",q->pp_count);
	}
	if (A_flag || a_flag || n_flag) {
		if (t_flag) printf("%-*.*s\t",tag,tag,"Alloc PPs:");
		printf("%1d\n",q->alloc_ppcount);
	}
	if (A_flag || a_flag || D_flag) {
		if (t_flag) printf("%-*.*s\t",tag,tag,"Total VGDAs:");
		printf("%1d\n",q->pvnum_vgdas);
	}
	if (A_flag || d_flag) {                     /* output the physical partition maps  */
		if (q->pp_map != (struct pp_map *)0) {   /* if the map is not empty */
			ptr = "PVMAP: ";
			for (i=0;i<q->pp_count;++i) {
				if (t_flag) printf("%s\t",ptr);
				p = q->pp_map + i;
				printf("%08x%08x:%-4d ",pv_id->word1,pv_id->word2,i+1);
				printf("%1d %-8s ",p->pp_state,ODM_type);
				printf("%08x%08x.%-3d ",p->lv_id.vg_id.word1,p->lv_id.vg_id.word2,p->lv_id.minor_num);
				printf("%-5d ",p->lp_num);
				printf("%08x%08x:%-4d ",p->fst_alt_vol.word1,p->fst_alt_vol.word2,p->fst_alt_part);
				printf("%08x%08x:%-4d\n",p->snd_alt_vol.word1,p->snd_alt_vol.word2,p->snd_alt_part);
			}
		}
	}
}


/*
 * Dump 16 bytes at "b".
 */
void
dump16(a,b,l)
ulong a;
unsigned char *b;
int l;
{

	int i;
	char txt[17];

	txt[16] = '\0';				/* null end of string */

	printf("%08X  ",a);
	for (i=0; i<=15; i++, b++)
	{
		if ((i%4) == 0)  printf(" "); 
		if (i<l) printf("%02X",*b);
		else {
			printf("..");
			*b = '\0';
		}
		txt[i] = ((*b < 0x20) || (*b >= 0x7f))?'.':*b;
	}
	printf("  |%s|\n",txt);
}

/*
 * NAME: dumpf
 *
 * FUNCTION:
 *   Dump the open file descriptor fd.
 *	Start at "offset", go for "datal" bytes.
 *	The data is dumped in hex.  The lines are of the form:
 *	oooooo	xxxxxxxx xxxxxxxx xxxxxxxx xxxxxxxx  |ASCII|
 */

int
dumpf()
{
 	ulong	position, pastit;
	int	dataread, leftover;
	char	buf[UBSIZE];

	/*
	 * Position the file for reading.
	 * If it's a nonfifo, seek to offset.
	 * If the offset lies outside the file, say so and quit.
	 * If it's a fifo, we'll just read it until time to
	 * start dumping data.
	 */

	if (seekval!= DONTSEEK)
	{
		if (lseek(fd,seekval,SEEK_SET) == -1)
			return(1);

		position = (ulong)h_offset_value;
	}
	else
		position = 0;		/* We can't seek (fifo)	*/

	/*
	 * We are now positioned for reading.
	 */
	pastit = (ulong)h_offset_value + (ulong)h_length_value;
	while ((position < pastit) && ((dataread = read(fd,buf,READLEN)) > 0))
	{
		for (; ((leftover=dataread-seekoffset) > 0)&&(position<pastit);
		     seekoffset+=16, position+=16)
		{
			if (position >= (ulong)h_offset_value)
				dump16(position, &buf[seekoffset], leftover);
		}
		seekoffset = 0;
	}

	if (dataread == -1)
		return(1);
	else
		return(0);
}
		
hdf()
{
	/* Open the file for reading. */
	if (strcmp(h_path_value,STDINSTR))
		/* We must open the specified file (not stdin) */
		if ((fd=open(h_path_value,O_RDONLY)) == -1)
			return(1);

        /* Get the file's type */
        if (fstat(fd,&sbuf)==-1)
                return(1);

	if (ISREG || ISBLK)
	{
		seekval = h_offset_value;
		seekoffset = 0;
	}
        else
	{
		if (ISFIFO)
		{
                        seekval = DONTSEEK;
                        seekoffset = 0;
                }
                else
		{
			if (ISCHR)
			{
				seekval = (h_offset_value & (-UBSIZE));
				seekoffset = (ulong)h_offset_value % UBSIZE;
                        }
			else
                        	return(1); /* can't dump this file */
		}
	}

        return(dumpf());

}


/*
 * NAME: Usage_error
 *
 * FUNCTION: routine for displaying usage error message.
 *
 * RETURN VALUE DESCRIPTION: NONE
 */

int Usage_error()
{
	fprintf(stderr, lvm_msg(QPV_USAGE), Prog);
}


/*
 * NAME: main
 *
 * FUNCTION: lquerypv reads command line input from the user,
 *    parses it, performs basic validation, formats it, and calls
 *    lvm_querypv passing the formatted values.  If an error is
 *    encountered then a message is displayed.
 *
 * RETURN VALUE DESCRIPTION: Zero if no error. Non-zero otherwise.
 */

main(argc,argv)

int argc;	   /* number of command line strings in argc */
char **argv;	   /* array of command line strings - from user */
{
	int retval;	   /* return code */
	char *value0 = "0";
	struct unique_id vg_id, pv_id;	      /* vg and pv id's used to request the query info */
	struct querypv *querypv;   /* lvm record the query info is returned in */
	char *pvname;

	/* set up for NLS */
	setlocale(LC_ALL,"");

	/* open the message catalog */
	scmc_catd = catopen("cmdlvm.cat",NL_CAT_LOCALE);

	/* Initialize Prog for use in usage_error function if usage error occurs */
	Prog = argv[0];

	/* setflg sets command option flags and command option values. If error then exit. */
	if ((retval = setflg(argc,argv,NULL)) < 0) exit(lvm_rc(retval));

	if ((retval=ck_required()) == 0)
	{  /* ck_required confirms required parameters were supplied */
		
		if (!h_flag)
		{

			/* format vg and pv id's and call lvm to do the query */
			get_uniq_id(p_value, &pv_id);

			if (N_flag)
			{/* is pv name specified */
				pvname = N_value;
				g_value = value0;
				/* zero out vg id if pv name specified */
			}
			else
				pvname = NULL;

			get_uniq_id(g_value, &vg_id);

			if (retval =lvm_querypv(&vg_id,&pv_id,&querypv,pvname))
			{
				errmsg(retval);
				exit(lvm_rc(retval));
			}
			else
			{
				output(querypv,&pv_id);
				/* output the query data received from lvm */
				lvm_freebuf(querypv);
			}
		}
		else
			hdf();
	}
	exit(lvm_rc(retval));
}

