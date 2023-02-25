static char sccsid[] = "@(#)64	1.32  src/tcpip/usr/bin/telnet/fs3270.c, tcptelnet, tcpip411, GOLD410 6/2/94 13:16:24";
/*
 *   COMPONENT_NAME: TCPTELNET
 *
 *   FUNCTIONS: MSGSTR
 *		ADDR_TO_RC
 *		INCREMENT
 *		fs_init
 *		convert_char
 *		convert_char
 *		fs_wsf
 *		send_query_reply
 *		if
 *		for
 *		for
 *		if
 *		for
 *		format_screen
 *		redisplay_single_attribute
 *		if
 *		if
 *		if
 *		if
 *		set_bufaddr
 *		erase_to
 *		erase_all_unprotected
 *		send_nonull
 *		send_all_fields
 *		send_single_field
 *		send_single_field
 *		send_single_field
 *		send_single_field
 *		if
 *		if
 *		if
 *		if
 *		if
 *		fs_dup
 *		fs_enter
 *		fs_eraseinput
 *		fs_fieldmark
 *		if
 *		fs_pf1
 *		fs_pf10
 *		fs_pf11
 *		fs_pf12
 *		fs_pf13
 *		fs_pf14
 *		fs_pf15
 *		fs_pf16
 *		fs_pf17
 *		fs_pf18
 *		fs_pf19
 *		fs_pf2
 *		fs_pf20
 *		fs_pf21
 *		fs_pf22
 *		fs_pf23
 *		fs_pf24
 *		fs_pf3
 *		fs_pf4
 *		fs_pf5
 *		fs_pf6
 *		fs_pf7
 *		fs_pf8
 *		fs_pf9
 *		fs_reset
 *		fs_right
 *		fs_sysreq
 *		fs_tab_in
 *		fs_tab_in
 *		if
 *		if
 *		while
 *		fs_up
 *		fs_wordnext
 *		fs_wordprev
 *		x_system
 *		tty_litout
 *		for
 *		if
 *		if
 *		fs_endpush
 *		fs_autopush
 *		fs_eng_lang
 *		if
 *		if
 *		reverse_field
 *		fs_autorev
 *		Set_NoBidi_Mode
 *		fs_check_to_end_push
 *		fs_autoshape
 *		fs_initial
 *		fs_final
 *		while
 *		if
 *		switch
 *		putneighbours
 *		fs_shape
 *		fs_setcursorshape
 *		fs_draw_screen
 *
 *   ORIGINS: 10,27,38
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* 
 *	Copyright (c) October 1984, Paul G. Milazzo, all rights reserved
 *
 *  fs3270.c - simple IBM 3278 emulator designed to be used with 4.2bsd telnet
 *
 *	version 0.0 - October 14, 1984, Paul G. Milazzo, Rice University
 *	version 0.1 - July 9, 1985, Paul G. Milazzo, Rice University
 *		merged contributed FS mode enhancements
 *	version 0.2 - August 5, 1985, Paul G. Milazzo, Rice University
 *		dynamic key bindings, beginnings of better FS algorithms
 *	version 0.3 - September 17, 1985, Paul G. Milazzo, Rice University
 *		faster FS algorithms, working EAU, part of PT order,
 *		correct SYS REQ, now does default & alternate sizes,
 *		works with mismatched real and emulated terminal widths,
 *		has ERASE INPUT, DUP, FIELD MARK keys, correct autoskip,
 *		keyboard locking works again, reset key works with
 *		keyboard locked
 *	version 0.4 - September 18, 1985, Paul G. Milazzo, Rice University
 *		I fixed bugs introduced by the "faster FS algorithms", fixed
 *		an apparently long-standing (but never noticed) bug with
 *		cursor positioning after Erase/Write command, and added a
 *		work-around for an apparent curses bug - insch() ignores
 *		standout mode.	I'm told SYS REQ still doesn't work - sigh...
 *	version 0.5 - Feb 20, 1984
 *		I fixed several bugs that related to the first attribute
 *		variable.  This variable was not properly updated when
 *		attributes were deleted.  Additionally, a bug was fixed in
 *		erase_to_unprotected order.  This bug caused this to
 *		infinite loop whenever it was called.  A second problem
 *		was fixed in erase_to_unprotected and repeat_to that caused
 *		the routines to do nothing if they were called to change
 *		the entire screen. (poor end conditions on while loops.)
 *		The major change with this revision was a performance
 *		fix. The offset fields for attribute bytes are now chained
 *		together to allow fast access to each field.  It is no
 *		longer necessary to scan the entire screen when doing reads
 *		or writes.  This really makes commands that yous the screen
 *		intelligently (xedit) run much faster.  This has also
 *		reduced the number of calls to curses.
 *		Added compile time option to trace writes from system.
 *	version 0.6 - Jan 29, 1993
 *		I added support for the program tab order.  This will clear
 *		fields correctly now.  I also added some debug messages to
 *		make the debugging more complete, but screen.trace files now
 *		are very large beasts.  I also added support for the attention
 * 		key bindings.  So now an entry in /etc/3270keys.hft or
 * 		/etc/map3270 like attention=\e[036q would cause an IAC BRK
 *		to the remote machine if a Control-F12 was pressed.  Neat, eh?
 */
#include	<ctype.h>

#ifdef ARAB 
#include        <stdio.h>
#include        "arab_tables.h"
#define MAXLINELENGTH  256
#define BLANK        32
#define LAM          245
#define ALEF1        161
#define ALEF2        162
#define ALEF3        163
#define ALEF4        165
#define LAMALEF1     156
#define LAMALEF2     157
#define LAMALEF3     158
#define LAMALEF4     159
#define LTR          0
#define RTL          1
#endif /* ARAB */

#ifdef	TRUE
#undef	TRUE
#endif
#ifdef	FALSE
#undef	FALSE
#endif
#include	<cur01.h>
#include	<arpa/telnet.h>
#include	"ibm3270.h"
 
#include <iconv.h>
#include <langinfo.h>
#include <nl_types.h>
#include "telnet_msg.h"

#ifdef BIDI
#include <bidi/bdescapes.h>
#include <sys/lc_layout.h>
static char          *locale_name; 
LayoutObject         *plh;            
#endif /* BIDI */

extern nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_TELNET,n,s)
 
#define		CURSES_FIXED	1	/* a matter of opinion */

/*-----------------------------------------------------------------------------
* Globals needed for structured field support
*----------------------------------------------------------------------------*/
static int mod_3278 = M5_3278;
int   wsf_length;                 /* length of a structured field */
int   wsf_flag = FALSE;           /* flag indicating structured field */
char  reply_mode = RM_FIELD;      /* reply mode */
unsigned char   *wsf_nextp;       /* pointer to next structured field in DS */
 
extern unsigned char	addrmap[];/* 3270 buffer address code map */
 
extern	int	etype;		/* 0=nothing special terminal type, 1=user
				   specified particular negotiation */
extern	struct	ucb	ucb;

struct screenchar {
    short		offset; /* offset from attribute byte */
    unsigned char	ch;	/* EBCDIC character at this screen location */
    unsigned char	attr;	/* attributes for the current field */
};
static struct screenchar	screen[SCREEN_SIZE];
static struct screenchar	*sbufp;
static short			attributes_added[COMMAND_BUFFER_SIZE];
static short		    	attributes_deleted[COMMAND_BUFFER_SIZE];
static short		    	characters_changed[COMMAND_BUFFER_SIZE];
static int			num_attributes_added;
static int			num_attributes_deleted;
static int			num_characters_changed;
static int			num_attributes = 0;
static struct screenchar	*first_attr;
 
extern char		ttyobuf[]; /* terminal output buffer */
extern char		*tfrontp; /* -> telnet's terminal output queue */
extern char		*tbackp; /* -> telnet's terminal output queue */
extern char		*nfrontp; /* -> telnet's network output queue */

extern	int	connected;	/* connected to a host? */

struct termtype {
    int		rows;
    int		cols;
    int		defrows;
    int		defcols;
    char	*name;
};
 
int				fs_keyboard_locked = TRUE;
 
extern int			lo_prot_color;	      /* lo intensity, prot   */
extern int			lo_unprot_color;      /* lo intensity, unprot */
extern int			hi_prot_color;	      /* hi intensity, prot   */
extern int			hi_unprot_color;      /* hi intensity, unprot */
extern int			background_color;     /* global background color */

static int			maxlinelength;
static int			maxlines;
static int			deflinelength;
static int			deflines;
static int			linelength;
static int			lines;
static int			cursorposition = 0;
static char			last_aid = AID_NONE;
static int			insert_mode = FALSE;
 
static int			X_col = 9;		/* for "X SYSTEM" */
static int			X_row = 0;		/* for "X SYSTEM" */
unsigned char *handle= 'B';
FILE	*dumpfno;
LayoutValues layout;
LayoutTextDescriptor value_description; 
/* imported functions */
void				prof_init ();
char				*getenv ();
 
/* exported functions */
# include	"keyfunctions.h"
void				fs_alarm ();
void				fs_command ();
void				fs_end ();
void				fs_init ();
void				fs_redraw ();
void				fs_schedredraw ();
char				*fs_select_term ();
 
/* private functions */
#ifdef	DEBUG3270
#define	TN_LOG(prspec)	fprintf prspec
#else	/* DEBUG3270 */
#define	TN_LOG(prspec)
#endif	/* DEBUG3270 */

#define ADDR_TO_RC(a,r,c) {(r) = (a) / linelength; (c) = (a) % linelength;}
#define INCREMENT(r,c)	 {if(++(c)==linelength){(c)=0;if(++(r)==lines)(r)=0;}}
/* execute move if the row changes */
#define INCREMENT_MOVE(r,c)	 {if(++(c)==linelength)\
	{(c)=0;{if(++(r)==lines)(r)=0;}move(r,c);}}
static void			clear_screen ();
static void			erase_all_unprotected ();
static void			erase_to ();
static void			format_screen ();
static void			update_offsets ();
void				fs_wsf ();
void				send_query_reply ();
void				fswrite ();
static int			interpret_wcc ();
static void			repeat_to ();
static int			screenaddr ();
static void			send_all_fields ();
static void			send_modified_fields ();
static void			send_single_field ();
static void			redisplay_single_attribute ();
static void			send_pa ();
static int			set_attribute ();
static int                      checkcode ();
static void			set_bufaddr ();

char				*code1, *code2;
iconv_t				cd1, cd2;
 
/* "fake" ASCII character codes for special 3270 functions */
 
#define DO_DUP			-1
#define DO_FIELDMARK		-2
 
extern char		initscr_done;


#ifdef BIDI   
void             fs_endpush();
void             check_to_start_autopush();
void             check_to_start_autorev();
void             flip_modified_rows ();
#endif /* BIDI */

#ifdef ARAB
void             fs_translate();
void             putneighbours();
void             getneighbours();
void             fs_shape();
void             fs_convertchar();
void             fs_reversestring();
void             fs_setcursorshape();
void             fs_draw_screen();
#endif /* ARAB */

#ifdef BIDI
int              modified_lines[MAX];
static  int      rev_line                 = FALSE;
static  int      field_push               = FALSE;
static  int      in_push_boundry_mode     = TRUE;
static  int      saved_insert_mode_status = TRUE; 
#endif /* BIDI */



/* 
 * Initialize for full-screen mode 
 * Since we are using extended curses for turning cursor to block mode,
 * and since vi does not handle extended curses, we are setting our
 * TERM to $TERM.U to get block mode cursor capability without
 * changing entry for $TERM. If no $TERM.U entry is present
 * we'll use $TERM . 
 */
 
void fs_init ()
{
    register char	*ptr;
    static char		buffer[1024];
    extern char		*getenv();
    extern char		*Def_term;
    extern char		My_term;
#ifdef BIDI
     int                 i; 
     char                *lang;
     int                 chk; 
     int                 *err,*var;
#endif /* BIDI */
#ifdef ARAB
     char                *env; 
#endif /* ARAB */

    if (!(ptr = getenv("TERM")))
		ptr = Def_term;
    Def_term = (char *)malloc(strlen(ptr) + 4);
    strcpy(Def_term, ptr);
    strcat(Def_term, ".U");
    if (tgetent (buffer, Def_term) != 1) {
		strcpy(Def_term, ptr);
    }
 
    do_colors = TRUE;	/* for curses	*/
    My_term = TRUE;
	 if (!initscr_done) {
		 initscr_done = TRUE;
		 initscr ();
	 }
#ifdef BIDI    /*  BIDI support  */
      for (i = 0; i < (sizeof(modified_lines)/sizeof(modified_lines[0])); i++)
         modified_lines[i] = FALSE;
    
#endif /* BIDI */


    prof_init ();	/* read in the key binding profile */
#ifdef	CURSES_BG_RESOLVED
    if (background_color) {
	NORMAL = background_color;
	refresh();
    }
#else
    background_color = B_BLACK;
#endif	/* CURSES_BG_RESOLVED */
    clear_screen ();
#ifdef DEBUG3270
    dumpfno = fopen("screen.trace", "a");
#endif
    /* initialize the NLS EBCDIC/AIX and AIX/EBCDIC conversion descriptors. */
	code1 = nl_langinfo(CODESET); 
	if (!(code2 = getenv("RM_HOST_LANG")))
		code2 = "IBM-037";
	cd1 = iconv_open(code1, code2);		/* from EBCDIC to AIX */
	cd2 = iconv_open(code2, code1);		/* from AIX to EBCDIC */
        if (cd1 == -1 || cd2 == -1) {
		code1 = "IBM-850";
		code2 = "IBM-037";
		cd1 = iconv_open(code1, code2);
		cd2 = iconv_open(code2, code1);
        }
       if (cd1 == -1 || cd2 == -1) {
                fprintf(stderr, MSGSTR(NO_CONVD, "Sorry, iconv doesn't support your codesets so cannot telnet to a 3270\n"));
		sleep(3);
                quit();
        }
#ifdef BIDI

#ifdef ARAB

 /* initialize BIDI library */

  lang=getenv("LANG");
  locale_name = setlocale(LC_CTYPE,NULL);
  TN_LOG((dumpfno,"locale = %s \n",locale_name));
  chk = layout_object_create(locale_name,&plh);
  if (chk != 0)
      TN_LOG ((dumpfno,"ERROR INITIALIZING LAYOUT CONTEXT \n"));

 /* get initial settings from environment */
  TN_LOG((dumpfno, "ARAB: initializing\n******"));
  env=getenv("ARB_NUM_SWAP");
  if (env[0]=='1')
       NUMSWAP_ON=TRUE;
  else NUMSWAP_ON=FALSE;
  env=getenv("ARB_SYM_SWAP");
  if (env[0]=='1')
       SYMSWAP_ON=TRUE;
  else SYMSWAP_ON=FALSE;
  TN_LOG((dumpfno,"NUM_SWAP %d SYM_SWAP %d \n",NUMSWAP_ON,SYMSWAP_ON));
  /*allocating memory for libi18n.a*/
  value_description=(LayoutTextDescriptor)malloc(sizeof(LayoutTextDescriptor));
  layout = (LayoutValues)malloc(2*sizeof(LayoutValues));
  layout[0].name=ActiveBidirection;
  layout[0].value=/*&var;*/malloc(sizeof(int));
  layout[1].name=0;
  chk=layout_object_getvalue(plh,layout,err);
  if (chk!=0)
     TN_LOG((dumpfno,"ERROR:layout_object_getvalue \n"));
  BIDI_ON=*layout[0].value;
  layout[0].name=ActiveShapeEditing;
  layout[0].value=malloc(sizeof(int));
  layout[1].name=0;
  chk=layout_object_getvalue(plh,layout,err);
if (chk!=0)
     TN_LOG((dumpfno,"ERROR:2.layout_object_getvalue \n"));
  ARABIC_ON=*layout[0].value;
  TN_LOG((dumpfno," ARABIC %d BIDI %d \n",ARABIC_ON, BIDI_ON));
  HEBREW_ON=((BIDI_ON)&&(!ARABIC_ON));
  TN_LOG((dumpfno,"HEBREW_ON=%d\n",HEBREW_ON));
  env = getenv("BIDI");
  if (env!=NULL) {
        TN_LOG((dumpfno,"BIDI=on saving enviroment\n"));
        BDSaveEnv(-1); /*to save original enviroment settings */
    }
  free(value_description);
  free(layout);

#endif /* ARAB */

  prof_init ();    /* read in the key binding profile */
  Set_NoBidi_Mode();

#endif /* BIDI */
}
int
convert_char(cd, inchar)
	iconv_t cd;  
	char 	inchar;
{
	size_t	inbytes, outbytes;
	char	outchar = inchar;  /* This is incase the iconv() fails */
				   /* Then just send the inchar not converted */
	char	*conv_input, *conv_output;

	conv_input = &inchar;
	conv_output = &outchar;
	inbytes = 1;
	outbytes = 1;

	iconv(cd, &conv_input, &inbytes, &conv_output, &outbytes);
	return((int)outchar);
}

 
/* exit full-screen mode */
 
void fs_end ()
{
  int     chk;
  char    *bidi;
  extern char *getenv();
#ifdef BIDI
bidi=getenv("BIDI");
if (bidi!=NULL) 
  {
  TN_LOG((dumpfno,"BIDI is on . Retrieving enviroment \n"));
  BDRestoreEnv(1);
  }
#endif /* BIDI */

	erase();
	refresh();
	endwin ();

#ifdef BIDI       
  /*chk = layout_object_free(plh);*/
  /*if (chk != 0)
     TN_LOG((dumpfno,"ERROR FREEING LAYOUT CONTEXT \n"));  */
#endif /* BIDI */
#ifdef DEBUG3270
	fclose(dumpfno);
#endif
#ifdef ARAB
#ifndef _POWER
 fs_setcursorshape(HFSNGLUS);
#endif /* _POWER */
#endif 
}
 
/* clear the screen to all nulls */
 
static void clear_screen ()
{
    sbufp = screen;
    cursorposition = 0;
 
    memset ((char *)sbufp, NULL, sizeof (screen));
    num_attributes = 0;
    first_attr = 0;
#ifdef BIDI        /* BIDI support -  if in "push" then it is now over. */
    if (push_mode)
      fs_endpush(FALSE);
    if (field_rev)
         fs_fldrev();
#endif /* BIDI */
}
 
/* process a complete 3270 data stream from the host */
 

void fs_command ()
{
#ifdef BIDI
    register struct screenchar  *sp;
    register int                invisible;
    register int                rowww,col;
    static   char               tmpc;
    register int                row;
    register struct screenchar *last_in_row;
    static   int                cur_loc;
    int                         direction;

#endif /* BIDI */

    register unsigned char	*bp;

    refresh(); 
    bp = tbackp;
 
    if (bp == tfrontp) {
	    return;
    }
    TN_LOG((dumpfno, "\n******"));
    /* first, figure out which command we've been given */
    switch (*bp++) {
	case CMD_EWA:
	case CMD_SNA_EWA:
	case CMD_EW:
	case CMD_SNA_EW:
	    if ((*(bp-1) == CMD_EWA) || (*(bp-1) == CMD_SNA_EWA)) {
		lines = maxlines;
		linelength = maxlinelength;
        	TN_LOG((dumpfno, "EWA:"));
	    }
	    else {
		lines = deflines;
		linelength = deflinelength;
		TN_LOG((dumpfno, "EW:"));
	    }
	    clear_screen ();
 	    clear (); 
	    xstandend(curscr);
	    /* fall through... */
	case CMD_WRITE:
	case CMD_SNA_WRITE:
            TN_LOG((dumpfno, "WRITE:\n"));
	    fswrite (bp);
	    last_aid = AID_NONE;
	    break;
	case CMD_RB:
	case CMD_SNA_RB:
	    send_all_fields (last_aid);
	    break;
	case CMD_RM:
	case CMD_SNA_RM:
	    send_modified_fields (last_aid);
	    break;
	case CMD_EAU:
	case CMD_SNA_EAU:
	    erase_all_unprotected ();
	    last_aid = AID_NONE;
	    fs_keyboard_locked = FALSE;
	    x_system(0);
	    break;
	case CMD_SEL:
	case CMD_SENSE:
	case CMD_SID:
            fprintf(stderr, MSGSTR(FS_GOT_CMD, "\r\n>got cmd %02x<\r\n"),
                    *(bp-1)); /*MSG*/
	    break;
	case CMD_NOOP:
	    break;
        /* A4638 *-------------------------------------------------------------
        *  Add limited structured field support - rbw, 07/24/89
        *--------------------------------------------------------------------*/
	case CMD_WSF:
	case CMD_SNA_WSF:
            TN_LOG((dumpfno, "WSF:\n"));
	    fs_wsf (bp, (int) (tfrontp - tbackp - 1));
	    break;
	default:
            TN_LOG((dumpfno, "\n>Illegal cmd %02x<\n", *(bp-1)));
            fprintf(stderr, MSGSTR(FS_ILL_CMD, "\r\n>Illegal cmd %02x<\r\n"),
                    *(bp-1)); /*MSG*/
	    break;
    }
 
    tbackp = tfrontp = ttyobuf;
#ifdef BIDI
if (screen_reversed) 
  {
    direction = RTL;
    fs_draw_screen(direction);
    refresh();
    flip_modified_rows();
    refresh();
    }
#ifdef NEVER
 (sbufp + sbufp->offset)->attr |= ATTR_MDT;
#endif /* NEVER */
#endif /* BIDI */
}
 
/* fs_wsf * A4638 *************************************************************
* 
* function: Process a Write Structured Field data stream
*
* synopsis: fs_wsf (bp, length);
*
* input:    unsigned char   *bp;         pointer to first structured field
*           int             length;      length of WSF data stream
*
* output:   none
*
******************************************************************************
*/
 
void fs_wsf (bp, length)
register unsigned char	*bp;
register int            length;
{
   register unsigned char   c;                 /* cur byte being processed */
   register int             state;             /* fsm for parsing data stream*/
   register int             field_complete;    /* flag for each struc field */
   unsigned char            sf_id;             /* structured field id */

/* begin */
 
   while (length > 0)
   {
      wsf_length = (*(bp+0) << 8) | *(bp+1);   /* extract field length */
      if (wsf_length == 0)
      {
         /*--------------------------------------------------------------------
         * If the length of the structured field is zero, assume that it is the
         * only structured field in the WSF command and set wsf_length to 
         * length.  The only case where this has been observed is when PVM 
         * sends a Read Partition (Query).
         *-------------------------------------------------------------------*/
         wsf_length = length;
      }
      wsf_nextp = bp + wsf_length;    /* set pointer to next structured field */
      bp += 2;                        /* increment past length field */
      field_complete = 0;             /* not finished with this field yet */
      state = WSF_ST_FID;             /* initial state is FID */

      while (! field_complete)
      {
         c = *bp++;

         switch (state)
         {
            case WSF_ST_FID:          /* get the field id */
               sf_id = c;
               switch (sf_id)
               {
                  case SF_READ:
                  case SF_SET_REPLY:
                  case SF_OB3270DS:
                     state = WSF_ST_PID;
                     break;

                  case SF_ERASE:
                     state = WSF_ST_ERASE;
                     break;

                  default:
                     state = WSF_ST_NEXT_FIELD;
                     break;
               }
               break;

            case WSF_ST_PID:         
               /*--------------------------------------------------------------
               * Partitions are not supported, so set the next state based on
               * the structured field id.
               *-------------------------------------------------------------*/
               switch (sf_id)
               {
                  case SF_READ:
                     state = WSF_ST_RP;
                     break;

                  case SF_SET_REPLY:
                     state = WSF_ST_SRM;
                     break;

                  case SF_OB3270DS:
                     state = WSF_ST_3270DS;
                     break;
 
                  default:
                     state = WSF_ST_NEXT_FIELD;
                     break;
               }
               break;

            case WSF_ST_RP:             /* Read Partition */
               switch (c)
               {
                  case RP_QUERY:                  /* Query */
                     send_query_reply();
                     break;

                  case RP_QUERY_LIST:             /* Query List */
                     /* not supported */
                     break;

                  case RP_RM:                     /* Read Modified */
                  case RP_RMA:                    /* Read Modified All */
                     send_modified_fields (AID_NONE);
                     break;

                  case RP_RB:                     /* Read Buffer */
                     send_all_fields (AID_NONE);
                     break;
               }
               state = WSF_ST_NEXT_FIELD;
               break;

            case WSF_ST_ERASE:           /* Erase/Reset */
               clear_screen();
               reply_mode = RM_FIELD;
               state = WSF_ST_NEXT_FIELD;
               break;

            case WSF_ST_SRM:             /* Set Reply Mode */
               if ((RM_FIELD == c) || (RM_EXTFIELD == c) || (RM_CHAR == c))
                  reply_mode = c;
               else
                  reply_mode = RM_FIELD;
               state = WSF_ST_NEXT_FIELD;
               break;

            case WSF_ST_3270DS:          /* Outbound 3270 Data Stream */
               switch (c)
               {
                  case OBDS_EW:              /* Erase/Write */
                  case OBDS_EWA:             /* Erase/Write Alternate */
                     if (OBDS_EWA == c)
                     {
                        lines = maxlines;
                        linelength = maxlinelength;
                     }
                     else
                     {
                        lines = deflines;
                        linelength = deflinelength;
                     } 
                     clear_screen();
                     clear();
                     xstandend (curscr);
                     /* fall through */

                  case OBDS_WRITE:           /* Write */
                     wsf_flag = TRUE;
                     fswrite (bp);
                     wsf_flag = FALSE;
                     last_aid = AID_NONE;
                     break;

                  case OBDS_EAU:             /* Erase All Unprotected */
                     erase_all_unprotected();
                     last_aid = AID_NONE;
                     fs_keyboard_locked = FALSE;
                     x_system (0);
                     break;

                  case OBDS_BSC_COPY:       /* BSC Copy */
                     /* not supported */
                     break;

                  default:
                     break;
               }
               state = WSF_ST_NEXT_FIELD;
               break;

            case WSF_ST_NEXT_FIELD:
               /*--------------------------------------------------------------
               * set up to process the next field
               *-------------------------------------------------------------*/
               length -= wsf_length;
               bp = wsf_nextp;
               field_complete = 1;
               break;
         }
      }
   }
}

/* send_query_reply * A4638 **************************************************
* 
* function:  Send reply to a Read Partition (Query) Structured Field
*
* synopsis:  send_query_reply();
*
* input:     none
*
* output:    none
*
* The query reply contains the following structured fields:  color,
* highlighting, usable area, and character sets.  The only variable data in
* the response is in the usable area structured field; some of the information
* is dependent on which model (2, 3, 4, or 5) of 3278 is being emulated.
* Refer to Chapter 1 of "3274 Control Unit Description and Programmer's Guide"
* (GA23-0061) and the "Data Stream Programmer's Reference" (GA23-0059) for 
* information about the contents of the query reply structured field.
*
******************************************************************************
*/

void send_query_reply()
{
   int j;
   static char query_reply[] = {
/*00*/	0x88,			/* query reply AID */
	/*--------------------------------------------------------------------
	* query reply (color) structured field
	*-------------------------------------------------------------------*/
/*01*/	0x00, 0x16,		/* length for query reply (color) */
	0x81,			/* query reply structured field id */
/*04*/	0x86,			/* this is a color query reply */
	0x00,			/* print info */
/*06*/	0x08,			/* number of color pairs */
	0x00, 0xf4,		/* color pair 1 */ 	
/*09*/	0xf1, 0x00,		/* color pair 2 */ 	
	0xf2, 0x00,		/* color pair 3 */ 	
/*13*/	0xf3, 0x00,		/* color pair 4 */ 	
	0xf4, 0x00,		/* color pair 5 */ 	
/*17*/	0xf5, 0x00,		/* color pair 6 */ 	
	0xf6, 0x00,		/* color pair 7 */ 	
/*21*/	0xf7, 0x00,		/* color pair 8 */ 	
	/*--------------------------------------------------------------------
	* query reply (highlighting) structured field
	*-------------------------------------------------------------------*/
/*23*/	0x00, 0x0d,		/* length for query reply (highlighting) */
	0x81,			/* query reply structured field id */
/*26*/	0x87,			/* this is a highlighting query reply */
	0x04,			/* number of highlighting pairs */
/*28*/	0x00, 0xf0,		/* highlighting pair 1 */ 	
	0xf1, 0xf1,		/* highlighting pair 2 */ 	
/*32*/	0xf2, 0xf2,		/* highlighting pair 3 */ 	
	0xf4, 0xf4,		/* highlighting pair 4 */ 	
	/*--------------------------------------------------------------------
	* query reply (usable area) structured field
	*-------------------------------------------------------------------*/
/*36*/	0x00, 0x17,		/* length for query reply (usable area) */
	0x81,			/* query reply structured field id */
/*39*/	0x81,			/* this is a usable area query reply */
	0x04,			/* 12/14-bit addressing allowed */
/*41*/	0x00,			/* char cells/cel units */
	0x00, 0x00,		/* width - depends on model size */
/*44*/	0x00, 0x00,		/* height - depends on model size */
	0x00,			/* unit of measure is inches */
/*47*/	0x00, 0x00, 0x00, 0x00,	/* horizontal dot spacing - model dependent */
	0x00, 0x02, 0x00, 0x85,	/* vertical dot spacing - model dependent */
/*55*/	0x09,			/* default width of dot matrix block */
	0x00,			/* default height of block - model dependent */
/*57*/  0x00, 0x00,		/* buffer size - model dependent */
	/*--------------------------------------------------------------------
	* query reply (character sets) structured field
	*-------------------------------------------------------------------*/
/*59*/	0x00, 0x10,		/* length for query reply (char sets) */
	0x81,			/* query reply structured field id */
/*62*/	0x85,			/* this is a char sets query reply */
	0x00,
/*64*/	0x00,
	0x09,			/* display */
/*66*/	0x10,			/* display */
        0x00, 0x00, 0x00, 0x00, /* supported LOAD PS format types */
/*71*/  0x03,                   /* length of each descriptor */
        0x00, 0x00, 0x00,       /* char set descriptor */
	/*--------------------------------------------------------------------
	* terminate message
	*-------------------------------------------------------------------*/
	IAC, EOR
   };

   switch (mod_3278)
   {
      case M5_3278:
         query_reply[43] = 0x84;       /* usable area byte 7 */
         query_reply[45] = 0x1b;       /* usable area byte 9 */
         query_reply[47] = 0x00;       /* usable area byte 11 */
         query_reply[48] = 0x01;       /* usable area byte 12 */
         query_reply[49] = 0x00;       /* usable area byte 13 */
         query_reply[50] = 0x71;       /* usable area byte 14 */
         query_reply[56] = 0x0c;       /* usable area byte 20 */
         query_reply[57] = 0x0d;       /* usable area byte 21 */
         query_reply[58] = 0xec;       /* usable area byte 22 */
         break;

      case M4_3278:
         query_reply[43] = 0x50;       /* usable area byte 7 */
         query_reply[45] = 0x2b;       /* usable area byte 9 */
         query_reply[47] = 0x00;       /* usable area byte 11 */
         query_reply[48] = 0x02;       /* usable area byte 12 */
         query_reply[49] = 0x00;       /* usable area byte 13 */
         query_reply[50] = 0x89;       /* usable area byte 14 */
         query_reply[56] = 0x0c;       /* usable area byte 20 */
         query_reply[57] = 0x0d;       /* usable area byte 21 */
         query_reply[58] = 0x70;       /* usable area byte 22 */
         break;

      case M3_3278:
         query_reply[43] = 0x50;       /* usable area byte 7 */
         query_reply[45] = 0x20;       /* usable area byte 9 */
         query_reply[47] = 0x00;       /* usable area byte 11 */
         query_reply[48] = 0x02;       /* usable area byte 12 */
         query_reply[49] = 0x00;       /* usable area byte 13 */
         query_reply[50] = 0x89;       /* usable area byte 14 */
         query_reply[56] = 0x10;       /* usable area byte 20 */
         query_reply[57] = 0x0a;       /* usable area byte 21 */
         query_reply[58] = 0x00;       /* usable area byte 22 */
         break;

      default:                         /* default to a Model 2*/
         query_reply[43] = 0x50;       /* usable area byte 7 */
         query_reply[45] = 0x18;       /* usable area byte 9 */
         query_reply[47] = 0x00;       /* usable area byte 11 */
         query_reply[48] = 0x02;       /* usable area byte 12 */
         query_reply[49] = 0x00;       /* usable area byte 13 */
         query_reply[50] = 0x89;       /* usable area byte 14 */
         query_reply[56] = 0x10;       /* usable area byte 20 */
         query_reply[57] = 0x07;       /* usable area byte 21 */
         query_reply[58] = 0x80;       /* usable area byte 22 */
         break;
   }
      
   for (j = 0; j < sizeof (query_reply); j++)
      *nfrontp++ = query_reply[j];
}

/* implement a write command */
 
void fswrite (bp)
register unsigned char	*bp;
{
    register int	row;
    register int	column;
    register int	addr1;
    register int	addr2;
    int			beeping, nulling = 0;
    int                 j;
    int                 num_pairs;     /* number of type/value pairs (SFE) */
    struct screenchar	*ap;
    struct screenchar	*old_ap = (struct screenchar *)NULL;
    char    attr;

    /* A4589 *-----------------------------------------------------------------
    * Need new variable 'end' that points to the end of the screen buffer
    *------------------------------------------------------------------------*/
    int     end = (int) screen + linelength * lines;              /* A4589 */
    
    row = cursorposition / linelength;
    column = cursorposition % linelength;
    beeping = interpret_wcc (*bp++);
 
    num_attributes_added = 0;
    num_attributes_deleted = 0;
    num_characters_changed = 0;
    standend();

    /*------------------------------------------------------------------------
    * references to wsf_flag and wsf_nextp added for structured field support
    *-----------------------------------------------------------------------*/
    while (bp < tfrontp) {
        /*---------------------------------------------------------------------
        * if we're processing a structured field, and we've reached the end of
        * the structured field, then break out of the while loop
        *--------------------------------------------------------------------*/
        if ((wsf_flag) && (bp >= wsf_nextp))
           break;

	switch (*bp) {
	    case ORDER_SF:
            case ORDER_SFE:
		nulling = 0;
                if (*bp == ORDER_SFE) 
                {
                   /*---------------------------------------------------------
                   * Only field mode attributes are supported, so the only 
                   * attribute type we look at is 3270 Field Attribute (0xc0).
                   * All other attribute types are ignored.
                   *--------------------------------------------------------*/
                   attr = ATTR_DISPLAY;
                   num_pairs = *(++bp);
                   if (num_pairs < 0)
                      num_pairs = 0;
                   for (j = 0; j < num_pairs; j++)
                   {
                      if ( *(++bp) == 0xc0)          /* field attribute */
                         attr = *(++bp);
                      else
                         ++bp;
                   }
                }
                else
                {
		   attr = *++bp;
                }
		TN_LOG((dumpfno, "\n\tadding attribute (%x) at %d,%d", attr, row,  column));
		/* write a space to the screen if we are adding */
		/* a new attribute */
		if ((sbufp->attr & ATTR_TOP) == 0) {
		    attributes_added[num_attributes_added++] = sbufp - screen;
		    num_attributes++;
#ifdef BIDI
              	    modified_lines[row] = TRUE;
                    if (screen_reversed) flip_modified_rows();
#endif /* BIDI */
		    mvaddch(row, column,  ' ');
                    standend();
	            TN_LOG((dumpfno, "(new)"));
		}
		
		/* force the field to be rewritten if the attributes */
		/* are different */
		if ((attr | ATTR_TOP) != (sbufp->attr | ATTR_TOP)) {
		    attr |= ATTR_LMDT;
		    TN_LOG((dumpfno, "(different)"));
		}
		/* A4589 *-----------------------------------------------------
		* If the row and column go to (0, 0), then the screen buffer
		* pointer needs to be set back to the beginning of the buffer.
		*------------------------------------------------------------*/
		INCREMENT (row,column);
		sbufp->attr = attr | ATTR_TOP;
		sbufp->ch = EBCDIC_NUL;
		sbufp++;
		if (((int) sbufp >= end) && (0 == row) && (0 == column))
		   sbufp = screen;
		break;
            case ORDER_MF:
		nulling = 0;
		TN_LOG((dumpfno, "\ngot ORDER_MF, help"));
                /*-------------------------------------------------------------
                * Modify Field order is not supported, so skip over the type/
                * value pairs.
                *------------------------------------------------------------*/
                num_pairs = *(++bp);
                if (num_pairs < 0)
                   num_pairs = 0;
                bp += 2 * num_pairs;
                break;
	    case ORDER_SBA:
		nulling = 0;
		addr1 = (int)*++bp;
		set_bufaddr (addr1 = screenaddr (addr1, (int)*++bp), FALSE);
		ADDR_TO_RC (addr1,row,column);
		TN_LOG((dumpfno, "\n\tmoving to %d,%d", row,  column));
		break;
	    case ORDER_IC:
		nulling = 0;
		TN_LOG((dumpfno, "\n\tcursor to %d,%d", row,  column));
		cursorposition = sbufp - screen;
		break;
	    case ORDER_PT:
		/* should conditionally fill field with nulls when data
		 * came from the host
		 */
		if (*(bp+1) == ORDER_PT) {
			TN_LOG((dumpfno, "\n\tTWO ORDER_PT found in a row"));
			fs_tab_in (1, nulling);
			bp++;
		} else {
			TN_LOG((dumpfno, "\n\tORDER_PT found"));
			fs_tab_in (0, nulling);
		}
		break;
	    case ORDER_RA:
		nulling = 0;
		TN_LOG((dumpfno, "\ngot ORDER_RA, help"));
		addr1 = (int)*++bp;
		addr2 = (int)*++bp;
		repeat_to (addr1 = screenaddr (addr1, addr2), *++bp);
		ADDR_TO_RC (addr1,row,column);
		break;
	    case ORDER_EUA:
		nulling = 0;
		TN_LOG((dumpfno, "\ngot ORDER_EUA, help"));
		addr1 = (int)*++bp;
		erase_to (addr1 = screenaddr (addr1, *++bp));
		ADDR_TO_RC (addr1,row,column);
		break;
            case ORDER_SA:
		nulling = 0;
		TN_LOG((dumpfno, "\ngot ORDER_SA, help"));
                /*-----------------------------------------------------------
                * skip Set Attribute orders; only field attributes are 
                * supported.
                *----------------------------------------------------------*/
                bp += 2;
                break;
	    case EBCDIC_GE:	/* unstuff 2-byte I/O Interface Codes */
		nulling = 0;
		TN_LOG((dumpfno, "\ngot ge, help"));
                /*--------------------------------------------------------
                * APAR 203604, 05/05/89 rbw - change next char to a hyphen;
                * extended chars not supported
                *---------------------------------------------------------*/
                *(bp+1) = 0x60;                     /* EBCDIC '-' (hyphen) */
		break;
	    default:
		nulling = 1;
		if (sbufp->attr & ATTR_TOP) {
		    attributes_deleted[num_attributes_deleted++] 
		        = sbufp - screen;
		    num_attributes--;
	            TN_LOG((dumpfno, "\n\tdeleting attribute at %d,%d", row,  column));
		}
		if (*bp != sbufp->ch) {
		    unsigned char my_ch = *bp;
		    characters_changed[num_characters_changed++]
			     = sbufp - screen;
	           /* TN_LOG((dumpfno, "\n\tchanging char %c to %c at %d,%d", sbufp->ch, convert_char(cd1, my_ch), row,  column));*/
		}
		TN_LOG((dumpfno, "%c", convert_char(cd1, *bp)));
		/* A4589 *-----------------------------------------------------
		* If the row and column go to (0, 0), then the screen buffer
		* pointer needs to be set back to the beginning of the buffer.
		*------------------------------------------------------------*/
		INCREMENT (row,column);
		sbufp->ch = *bp;
		sbufp->attr = 0;
		sbufp++;
		if (((int) sbufp >= end) && (0 == row) && (0 == column))
		   sbufp = screen;
		break;
	}
	bp++;
    }
    update_offsets();
#ifdef BIDI
    if(!screen_reversed)
#endif /* BIDI */
    format_screen ();
    set_bufaddr (cursorposition, FALSE);
    touchwin(stdscr);
#ifdef BIDI
    if (screen_reversed) flip_modified_rows ();
#endif /* BIDI */
    refresh ();
 
    if (beeping)
	fs_alarm ();

}
 
/* interpret a Write Control Character */
 
static int interpret_wcc (wcc)
unsigned char	wcc;
{
    register struct screenchar	*sp;
    register int		i;

    
    if (wcc & WCC_RESTORE) {
	fs_keyboard_locked = FALSE;
	x_system(0);
	}
 
    /* Reset all Modified Data Tags */
    if (wcc & WCC_RMDT) {	
	sp = first_attr;
	for (i = 0; i < num_attributes; i++) {
	       sp->attr &= ~ATTR_MDT;
	       sp = sp + sp->offset;
	}
    }
 

    return wcc & WCC_ALARM;
}
 
/* update the offset fields for the added and deleted attributes */
static void update_offsets()
{
    register struct screenchar	*sp;
    register struct screenchar	*end;
    register int		offset;
    register int		i;
    register struct screenchar	*prev_attr;
    register struct screenchar	*spm1;
	

    end = screen + linelength * lines;
    offset = 0;
 
    if (num_attributes) {
	/* update offset fields for attributes added */
	for (i = 0; i < num_attributes_added; i++) {
	    sp = screen + attributes_added[i];
	    offset = 0;
	    sp++;
	    
	    if (sp >= end) {
		offset = linelength * lines;
		sp = screen;
	    }
	    while((sp->attr & ATTR_TOP) == 0) {
		sp->offset = --offset;
		sp++;
		if (sp >= end) {
		    offset += linelength * lines;
		    sp = screen;
		}
	    }
	}
    
	/* update offset fields for attributes deleted */
	offset = 0;
	for (i = 0; i < num_attributes_deleted; i++) {
	    sp = screen + attributes_deleted[i];
	    spm1 = sp-1;
	    if (spm1 < screen) {
	    	spm1 = end - 1;
		if (spm1->attr & ATTR_TOP) offset = linelength * lines;
		else offset = spm1->offset;
	         }
	    else {
		if (spm1->attr & ATTR_TOP) offset = 0;
		else offset = spm1->offset;
	    	}
	    while((sp->attr & ATTR_TOP) == 0) {
		if (sp >= end) {
		    offset += linelength * lines;
		    sp = screen;
		    if (sp->attr & ATTR_TOP) {
			sp->offset = --offset;
			break;
			}
		}
		sp->offset = --offset;
		sp++;
	    }
	}
 
	/* chain the offsets of the attribute bites together */
	prev_attr = end - 1;
	if ((prev_attr->attr & ATTR_TOP) == 0) 
	    prev_attr = prev_attr + prev_attr->offset;
	    
	for (i = 0; i < num_attributes; i++) {
	    sp = prev_attr;
	    spm1 = sp - 1;
	    if (spm1 < screen) spm1 = end - 1;
	    if (spm1->attr & ATTR_TOP) prev_attr = spm1;
	    else prev_attr = spm1 + spm1->offset;
	    prev_attr->offset = sp - prev_attr;
	}
 
	if (screen->attr & ATTR_TOP) first_attr = screen;
	else {
	    first_attr = screen + screen->offset; /* last attrib */
	    first_attr = first_attr + first_attr->offset; /* next attrib */
	}
 
	/* set a bit in the attribute bite if any character was changed */
	/* in this field */
	for (i = 0; i < num_characters_changed; i++) {
	    sp = screen + characters_changed[i];
	    if (sp->attr & ATTR_TOP) sp->attr |= ATTR_LMDT;
	    else (sp + sp->offset)->attr |= ATTR_LMDT;
	}
    }
    else if (num_attributes_deleted) { /* no attributes but there were */
	for (sp = screen; sp <= end; sp++) sp->offset=0;
    }

}
 
/* given a set of attribute bytes, complete formatting the screen */
static void format_screen ()
{
    register struct screenchar	*sp;
    register struct screenchar	*end;
    register int		row;
    register int		column;
    register struct screenchar	*bottom_right;
    int				invisible;
    unsigned char		attribute;
    int				i;
    

    end = screen + linelength * lines;
    bottom_right = end - 1;

/* curses does not seem to be saving field attributes as I would expect.
 * Because of this we must redisplay a field even if it hasn't changed. If
 * curses recovers, delete the ifdef CURSES_SAVES_ATTRS below.
 */ 

    /* set the first attribute */
    if (num_attributes) {
	sp = first_attr;
	for (i = 0; i < num_attributes; i++) {
#ifdef	CURSES_SAVES_ATTRS
	    if (sp->attr & ATTR_LMDT) {
#endif	/* CURSES_SAVES_ATTRS */
		sp->attr &= ~ATTR_LMDT;
		attribute = sp->attr & ~ATTR_TOP;
		invisible = set_attribute (attribute, 1);
		redisplay_single_attribute(sp, attribute, invisible);
		standend();
#ifdef	CURSES_SAVES_ATTRS
	    }
#endif	/* CURSES_SAVES_ATTRS */
	    sp = sp + sp->offset;
	}
    }
    else {
	sp = screen;
        attribute = '\0';
	row = column = 0;
	clear();
	xstandend(curscr);
	move(row, column);
	invisible = set_attribute (attribute, 1);
    
	for (sp = screen; sp < end; sp++) {
	    sp->attr = attribute;
	    if (sp->ch && !invisible && !checkcode(sp->ch)) {
		    move(row,column);
#ifdef BIDI
                    if (screen_reversed)              /*BIDI mode  */
                    modified_lines[row] = TRUE;
#endif /* BIDI */
		    addch (convert_char(cd1, sp->ch));
	            TN_LOG((dumpfno, "\n\tformat_screen() addch(%c) at %d,%d", sp->ch, row,  column));
	    } else {
                move(row,column);
                addch (' ');
#ifdef DEBUG3270
                if (checkcode(sp->ch))
#endif /* DEBUG3270 */
                    TN_LOG((dumpfno, "\n\tformat_screen() checkcode(0x%02x) substitution", sp->ch));
                    TN_LOG((dumpfno, "\n\tformat_screen() addch(BLANK) at %d,%d", sp->ch, row,  column));
                }
		INCREMENT (row,column);
	}
        move(row,column);
        standend();
    }

}
 
static void redisplay_single_attribute(sp_ptr, attribute, invisible)
struct screenchar		*sp_ptr;
int				invisible;
unsigned char			attribute;
{
    register struct screenchar	*sp;
    register struct screenchar	*end;
    register int		row;
    register int		column;
    register struct screenchar	*bottom_right;
    int	attrib_on = !invisible;
 

    end = screen + linelength * lines;
    bottom_right = end - 1;
 
    sp = sp_ptr+1;
    if (sp >= end) sp = screen;
 
    ADDR_TO_RC(sp-screen,row,column);
    move(row, column);
    while((sp->attr & ATTR_TOP) == 0) {
	sp->attr = attribute;
	    if (sp->ch && !invisible && !checkcode(sp->ch)) {
		if (!attrib_on) {
			set_attribute(attribute, 0);
			attrib_on = 1;
			}
#ifdef BIDI
                if (screen_reversed) modified_lines[row] = TRUE;
#endif /* BIDI */
		addch (convert_char(cd1, sp->ch));
	        TN_LOG((dumpfno, "\n\tredisplay_single_attribute() addch(%c) at %d,%d", convert_char(cd1, sp->ch), row,  column));
		}
	    else {
		if (attrib_on) {
		    standend();
		    attrib_on = 0;
		    }
#ifdef BIDI
                if (screen_reversed) modified_lines[row] = TRUE;
#endif /* BIDI */
		addch (' ');
#ifdef DEBUG3270
                if (checkcode(sp->ch))
#endif /* DEBUG3270 */
                TN_LOG((dumpfno, "\n\tredisplay_single_attribute() checkcode(0x%02x) substitution", sp->ch));
	        TN_LOG((dumpfno, "\n\tredisplay_single_attribute() addch(BLANK) at %d,%d", row,  column));
		standend();
		}
	sp++;
	if (sp >= end) sp = screen;
	INCREMENT_MOVE(row,column);	
    }

}

/* This routine checks to see if a character is in a limited subset of   */
/* control codes for a printer.  These format control orders do not have */
/* a display function.  Even though the control codes are stored in the  */ 
/* character bufffer they are actually displayed as a space.             */

static int checkcode (character)
register unsigned char  character;
{
	switch (character) {            /* Printer Control Codes */
        	case    '\014':         /* FF Form Feed          */
        	case    '\015':         /* CR Carriage Return    */
        	case    '\025':         /* NL New Line           */
        	case    '\031': {       /* EM End of Medium      */
                	return(TRUE);
                	break;
        	}
        	default: return(FALSE);
	}
}
 
static int set_attribute (attribute, data)
register unsigned char	attribute;
int data;	/* if data from host then we always display it */
{
    if (data && (attribute & ATTR_DISPLAY) == ATTR_DISPLAY)
	{
	 TN_LOG((dumpfno,"1.set_attribute: invisible field\n"));
         return TRUE; /* a non-displayable field, so return invisible true */
        }

    switch (attribute & ATTR_DISPLAY) {
	case ATTR_LN:
	case ATTR_LP:
	    if (attribute & ATTR_PROT) {
		if (lo_prot_color)
			colorout(lo_prot_color | background_color);
		else
	    		standend ();
		}
	    else {
		if (lo_unprot_color)
			colorout(lo_unprot_color | background_color);
		else
			standend();
		}
	    return FALSE;
 
	case ATTR_HP:
	    if (attribute & ATTR_PROT)	{
		if (hi_prot_color)
			colorout(hi_prot_color | background_color);
		else
			colorout(BOLD);
		}
	    else {
		if (hi_unprot_color) 
			colorout(hi_unprot_color | background_color);
		else
	    		colorout (BOLD);
		}
	    return FALSE;
	default:
	    standend ();
	    if (data && !(attribute & ATTR_MDT))
		    return FALSE;
	    else
                     return TRUE;
    }

}
 
/* calculate the screen address given the two address bytes */
 
static int screenaddr (addr1, addr2)
int	addr1;
int	addr2;
{
    return (addr1 & TWELVE_BIT)
		? ((addr1 & ADDR_PART) << 6) + (addr2 & ADDR_PART)
		: ((addr1 & ADDR_PART) << 8) + addr2;
}
 
/* set the buffer address for subsequent operations */
 
static void set_bufaddr (bytenumber, save)
register int	bytenumber;
int		save;
{
    register int	screensize;
 
    screensize = linelength * lines;
 
    if (bytenumber < 0)
	bytenumber += screensize;
    else if (bytenumber >= screensize)
	bytenumber -= screensize;
 
    move (bytenumber / linelength, bytenumber % linelength);
    if (save)
	cursorposition = bytenumber;
 
    sbufp = screen + bytenumber;
}
 
/* implement the Repeat to Address order */
 
static void repeat_to (addr, ch)
register int	addr;
unsigned char	ch;
{
    register int		times;
    register int		i;
    register struct screenchar	*end;


    end = screen + linelength * lines;
 
    addr %= (linelength * lines);
    times = addr - (sbufp - screen);
    if (addr <= sbufp - screen) times = (linelength * lines) + times;
 
{
char ach;
ach = convert_char(cd1, ch);
 
if (isprint(ach)) TN_LOG((dumpfno, "\n\trepeat (%c)", ach));
else TN_LOG((dumpfno, "\nrepeat (\\%o)", ach));
TN_LOG((dumpfno, " from %d, to %d times %d\n", sbufp-screen, addr, times));
}
    for (i = 0; i < times; i++) {
	if (sbufp->attr & ATTR_TOP) {
	    attributes_deleted[num_attributes_deleted++] = sbufp - screen;
	    num_attributes--;
	}
	if (ch != sbufp->ch) { 
	    characters_changed[num_characters_changed++] = sbufp - screen;
	}
	sbufp->ch = ch;
	sbufp->attr = 0;
	sbufp++;
	if (sbufp >= end) sbufp = screen;
    }

}
 
/* implement the Erase Unprotected to Address order */
 
static void erase_to (addr)
register int	addr;
{
    register int		times;
    register int		i;
    register char		attr;
    register struct screenchar  *sp;
    register struct screenchar	*end;
    

    end = screen + linelength * lines;
 
    /* find the previous attribute byte. At this point, the offsets */
    /* are wrong since attributes may have been added or deleted */
    if (num_attributes) {
	sp = sbufp;
	while ((sp->attr & ATTR_TOP) == 0) {
	    sp--;
	    if (sp <= screen) sp = end - 1;
	}
	attr = sp->attr & ATTR_PROT;
    }
    else attr = '\0';
 
    addr %= (linelength * lines);
    times = addr - (sbufp - screen);
    if (addr <= sbufp - screen) times = (linelength * lines) + times;
 
    TN_LOG((dumpfno, "\n\terase unprot from %d, %d times\n", sbufp-screen, times));
    for (i = 0; i < times; i++) {
	if (sbufp->attr & ATTR_TOP)
	    attr = sbufp->attr & ATTR_PROT;
	else if (! (attr)) {
	    sbufp->ch = 0;
	    characters_changed[num_characters_changed++] = sbufp - screen;
	}
	sbufp++;
	if (sbufp >= end) sbufp = screen;
    }

}
 
/* send a PA or CLEAR key */
static void send_pa (aid)
register int	aid;
{
    extern char	*VE;
    fs_keyboard_locked	= TRUE;
    x_system(1);
#ifndef STICKYINSERT
    if (insert_mode == TRUE) {
	_tputvs(VE);
    	insert_mode = FALSE;
	}
#endif
 
    TN_LOG((dumpfno, "send_pa aid : %x\n", aid));
 
    /* construct a 3270 "short read" data stream */
    *nfrontp++ = aid;
    *nfrontp++ = IAC;
    *nfrontp++ = EOR;
    last_aid = aid;
}
 
static void erase_all_unprotected ()
{
    register struct screenchar	*sp;
    register int		row;
    register int		column;
    register int		invisible;
    struct screenchar		*end;
 
 
    TN_LOG((dumpfno, "\n\terase_all_unprotected()"));
    row = 0;
    column = 0;
    end = screen + linelength * lines;
    if (num_attributes) {
	invisible = set_attribute (screen->attr, 0);
	move(0, 0);
	for (sp = screen; sp < end; sp++) {
	    if (!(sp->attr & ATTR_PROT)) {
	        sp->ch = EBCDIC_NUL;
		sp->attr &= ~ATTR_MDT;
		if ((sp->attr & ATTR_TOP) &&
		    !(invisible = set_attribute (sp->attr, 0))) {
		    move (row, column);
		}
		if (!invisible) {
#ifdef BIDI
                  if (screen_reversed) modified_lines[row] = TRUE;
#endif /* BIDI */
		    addch (' ');
               }
	    }
	    INCREMENT_MOVE(row,column);
	}
    }
    else {	/* no attribute bytes - clear entire buffer */
	clear_screen ();
	clear ();
	xstandend(curscr);
    }
    fs_home ();

}
 
/*
 * send_nonull -	send entire screen, but no NULLs
 *
 * In response to a CMD_RM or CMD_RMA, when there are no attributes,
 * we send the entire screen, with NULLs suppressed.
 */
static void send_nonull (aid) 
{
    register struct screenchar	*sp;
    register struct screenchar	*end;
 
    end = screen + linelength * lines;
 
    if (aid == AID_SYSREQ) {	/* construct TEST REQUEST header */
	*nfrontp++ = EBCDIC_SOH;
	*nfrontp++ = convert_char(cd2, '%');
	*nfrontp++ = convert_char(cd2, '/');
	*nfrontp++ = EBCDIC_STX;
    } else {			/* construct read header */
	*nfrontp++ = aid;
	*nfrontp++ = addrmap[cursorposition >> 6];
	*nfrontp++ = addrmap[cursorposition & ADDR_PART];
    }
 
    for (sp = screen; sp < end; sp++)
	if (sp->ch)
		*nfrontp++ = sp->ch;

    *nfrontp++ = IAC;
    *nfrontp++ = EOR;
    last_aid = aid;
}

static void send_all_fields (aid)
int	aid;
{
    register struct screenchar	*sp;
    register struct screenchar	*end;
 

    end = screen + linelength * lines;
 
    /* construct a 3270 data stream in response to a Read Buffer */
    /* send the entire screen, don't worry about wrap or anything */
    if (aid == AID_SYSREQ) {	/* construct TEST REQUEST header */
	*nfrontp++ = EBCDIC_SOH;
	*nfrontp++ = convert_char(cd2, '%');
	*nfrontp++ = convert_char(cd2, '/');
	*nfrontp++ = EBCDIC_STX;
    }
    else {			/* construct read header */
	*nfrontp++ = aid;
	*nfrontp++ = addrmap[cursorposition >> 6];
	*nfrontp++ = addrmap[cursorposition & ADDR_PART];
    }
 
    for (sp = screen; sp < end; sp++) {
	if (sp->attr & ATTR_TOP) {
	    *nfrontp++ = ORDER_SF;
	    *nfrontp++ = sp->attr;
	}
	else *nfrontp++ = sp->ch;
    }
    *nfrontp++ = IAC;
    *nfrontp++ = EOR;
    last_aid = aid;

}
 
/* construct a 3270 data stream as if responding to a Read Modified */
 
static void send_modified_fields (aid)
register int	aid;
{
    register struct screenchar	*sp;
    register int		i;
    extern char			*VE;
     

#ifndef STICKYINSERT
    if(insert_mode == TRUE) {
	_tputvs(VE);
    	insert_mode = FALSE;
	}
#endif
    if (num_attributes) {
	if (aid == AID_SYSREQ) {	/* construct TEST REQUEST header */
	    *nfrontp++ = EBCDIC_SOH;
	    *nfrontp++ = convert_char(cd2, '%');
	    *nfrontp++ = convert_char(cd2, '/');
	    *nfrontp++ = EBCDIC_STX;
	}
	else {			/* construct read header */
	    fs_keyboard_locked = TRUE;
	    x_system(1);
	    *nfrontp++ = aid;
	    *nfrontp++ = addrmap[cursorposition >> 6];
	    *nfrontp++ = addrmap[cursorposition & ADDR_PART];
	}
 
	sp = first_attr;
 
	for (i = 0; i < num_attributes; i++) {
	    if (sp->attr & ATTR_MDT) send_single_field(sp);
	    sp = sp + sp->offset;
	}
    
	*nfrontp++ = IAC;
	*nfrontp++ = EOR;
	last_aid = aid;
    }
    else {
	fs_keyboard_locked = TRUE;
	x_system(1);
	send_nonull(aid);
    }
}
 
/* send a single modified field */
static void send_single_field(sp_ptr)
register struct screenchar	*sp_ptr;
{
    register struct screenchar	*sp;
    register struct screenchar	*end;
    register char		*last_non_white;
    register int		field_start;
 

    sp  = sp_ptr;
    end = screen + linelength * lines;
 
    field_start = sp - screen + 1;
    if (field_start >= end - screen)
        field_start = 0;
    *nfrontp++ = ORDER_SBA;
    *nfrontp++ = addrmap[field_start >> 6];
    *nfrontp++ = addrmap[field_start & ADDR_PART];
    last_non_white = nfrontp;
 
    sp++;
    if (sp >= end) sp = screen;
    while((sp->attr & ATTR_TOP) == 0) {
	if (sp->ch) {
	    *nfrontp++ = sp->ch;
	    last_non_white = nfrontp;
	}
	sp++;
	if (sp >= end) sp = screen;
    }
    /* truncate trailing white space off last field */
    nfrontp = last_non_white;

}
 
/* force curses to redraw the screen as soon as possible */
 
void fs_schedredraw ()
{
#ifdef BIDI
int i,x;
    if(screen_reversed)
      /*for (i = 0; i < 27; i++)unicode*/
     for (i = 0; i < (sizeof(modified_lines) / sizeof(modified_lines[0])); i++)
       modified_lines[i] = FALSE;/*unicode*/
#endif /* BIDI */
TN_LOG((dumpfno, "fs_redraw\n"));
    clearok (stdscr, TRUE);
}
 
/* redraw the screen if it hasn't recently been done */
 
void fs_redraw ()
{
#ifdef BIDI
    flip_modified_rows();
#endif /* BIDI */
    refresh ();
}
 
/* select the appropriate IBM 3270 to emulate based on the TERM variable */

char *fs_select_term ()
{
    register int		term_rows;
    register int		term_cols;
    static struct termtype	terminfo[] = {
	27,	132,	24,	80,	"IBM-3278-5",
	43,	80,	24,	80,	"IBM-3278-4",
	32,	80,	24,	80,	"IBM-3278-3",
	24,	80,	24,	80,	"IBM-3278-2",
	12,	80,	12,	40,	"IBM-3278-1",
	12,	40,	12,	40,	"IBM-3277-1",
	-1,	-1,	-1,	-1,	(char *)NULL
    };
    static char			buffer[1024];
    register struct termtype	*tp = terminfo;
    char			*term;

    if ((term = getenv ("TERM")) == (char *)NULL) {
        fputs(MSGSTR(NO_TERM, "No environment-specified terminal type.\n"),
                stderr); /*MSG*/
	exit(1);
    }
    if (tgetent (buffer, term) != 1) {
        fprintf(stderr, MSGSTR(NO_TERMINFO, "No terminfo entry for \"%s\".\n"),
                term); /*MSG*/
	exit(1);
    }
    term_rows = tgetnum ("li");
    term_cols = tgetnum ("co");

    while (tp->rows > term_rows || tp->cols > term_cols)
    {
	tp++;
        mod_3278++;             /* increment 3278 model number */
    }
    maxlines = tp->rows;
    maxlinelength = tp->cols;
    lines = deflines = tp->defrows;
    linelength = deflinelength = tp->defcols;

    if (term_rows > tp->rows)
	X_row = tp->rows;		/* set up line for "X SYSTEM" */

    return tp->name;
}
 
/* sound the terminal alarm */
 
void fs_alarm ()
{
	beep();
}
 
int fs_backspace ()
{
#ifdef BIDI
     if (field_rev)
         set_bufaddr (cursorposition + 1, TRUE);
   else
#endif /* BIDI */
        set_bufaddr (cursorposition - 1, TRUE);
 
    TN_LOG((dumpfno, "fs_backspace\n"));
 
    if ((sbufp->attr & ATTR_TOP) || (sbufp->attr & ATTR_PROT)) {
	fs_alarm ();
	set_bufaddr (cursorposition + 1, TRUE);
	return;
    }
#ifdef BIDI
    if ((push_mode) && (cursorposition < push_start_loc))
    {
        fs_alarm ();
        /*set_bufaddr (cursorposition + 1, TRUE);*/
        return;
    }
#endif /* BIDI */
   if (insert_mode)
	fs_delete ();
    else {
	if (!set_attribute (sbufp->attr, 0)) {
#ifdef BIDI
            if (screen_reversed)            /*  BIDI mode  */
                modified_lines[ cursorposition / linelength ] = TRUE;
	    addch (' ');
#endif /* BIDI */
	    move (cursorposition / linelength, cursorposition % linelength);
	}
	sbufp->ch = EBCDIC_SP;
	(sbufp + sbufp->offset)->attr |= ATTR_MDT;
#ifdef ARAB
TN_LOG((dumpfno, "\nARAB:entering fs_shape from backspace with sbufp at %c\n",sbufp->ch));
        if (!field_rev)
           fs_shape(sbufp,handle);     /* shape around removed characeter */
#endif /* ARAB */

#ifdef BIDI
        if (push_mode)
         {
            check_push_outof_bounds();
            num_chars_pushed--;
         }
      if (field_rev)
          num_chars_reversed--;
#endif /* BIDI */
  }
}
 
int fs_backtab ()
{
    register struct screenchar	*sp;
    register struct screenchar	*oldsp;
    register struct screenchar	*end;
#ifdef BIDI
    if (field_rev)
        rev_fld_btab();
    else
    {
#endif /* BIDI */
    end = screen + linelength * lines;
    /* no attributes, return */
    if (num_attributes == 0)
	return;
 
    sp = sbufp;
    if ((sp->attr & ATTR_TOP) == 0) {
    	if (sp->offset == (short)(-1)) { /* go to prev attr */
	    sp = sp - 1;
	}
	else { /* go to first char in field */
	    sp = sp + sp->offset + 1;
	    if (sp >= end) sp = screen;
	}
    }
    oldsp = sp;
 
    /* the previous code guarantees that we are either on an attribute 
       or at the beginning of a field.  This is the invariant for
       the following loop. Now we iterate to find the 
       first character of the next field that is not protected */

    while (sp->attr & (ATTR_TOP | ATTR_PROT)) {
	if (sp->attr & ATTR_TOP) {
	    sp = sp - 1;	/* go to the prev field */
	    if (sp < screen) sp = end - 1;
	    if ((sp->attr & ATTR_TOP) == 0) { /* go to beginning */
	         sp = sp + sp->offset + 1;
		 if (sp >= end) sp = screen;
	    }
	}
	else {
	    sp = sp - 1;
	    if (sp < screen) sp = end - 1;
	}
	if (sp == oldsp)
	    return;
    }
    set_bufaddr (sp - screen, TRUE);

#ifdef BIDI
    if (push_mode)
        check_push_outof_bounds();
    }
#endif /* BIDI */
}
 
int fs_character (ch)
register int    ch;
{
    register struct screenchar  *sp;
    register int                column;
    register int                row;
    register struct screenchar  *end;
    register int                invisible;
    int                         cp;
#ifdef BIDI
    int                         i;
#endif /* BIDI */
#ifdef ARAB
    int                         increment,direction;
    char                        temp;
    char                        *display,*bidi;    
#endif /* ARAB */

    sp = sbufp;
    end = screen + linelength * lines;
    TN_LOG((dumpfno, "fs_character ch: %c (%x)\n", ch & 0xff, ch & 0xff));

    /* can't write on protected fields or attribute bytes */
    if ((sbufp->attr & ATTR_TOP) || (sbufp->attr & ATTR_PROT)) {
        fs_alarm ();
        return;
    }
#ifdef ARAB 
   display = getenv("DISPLAY");
   bidi = getenv("BIDI");
   TN_LOG((dumpfno,"DISPLAY=%s, BIDI%s\n",display,bidi));
   if (national_lang_mode && ARABIC_ON){
    /* map character if we are not in a BIDI window and we are in
       National language layer and Arabic is enabled */
    
     if  (bidi==NULL)
     {
         TN_LOG((dumpfno, "NOT XXXX using telnet mapping: converting char:%c to char%c\n",
                 ch & 0xff, ARAB_KYB_MAP[ch] & 0xff));
         ch=ARAB_KYB_MAP[ch];    /* convert character to Arabic */
     }
     else {
         /* if we are using the mapping of the window, then we need to
            convert the Arabic numerals to Hindi numerals, if we are in
            National language layer  */
         TN_LOG((dumpfno, "XXXX using bterm or aixterm mapping: converting number:%c to char",
                 ch ));

             if ((ch >= 0x30) && (ch <= 0x39))
                 ch += 0x80;
         TN_LOG((dumpfno, "%c\n",
                  ch ));
     } 
    }
   /* convert character to desired format : numeric shape, symmetric swapping   and special shape selection */
       temp = ch;
       fs_convertchar(&temp);
       ch = temp;
    TN_LOG((dumpfno, "ARAB 2 : converting char:%c to char %c",
                 temp, ch ));
#endif /* ARAB */

#ifdef BIDI
    if ((RTL_autopush_enabled) || (LTR_autopush_enabled))
       check_to_start_autopush(ch);

    if ((RTL_autofldrev_enabled) || (LTR_autofldrev_enabled))
       check_to_start_autorev();
#endif /* BIDI */

    invisible = set_attribute (sbufp->attr, 0); 
    if (invisible)
       TN_LOG((dumpfno,"fs_character: invisible is true\n"));
    increment=1;
#ifdef BIDI
    if (field_rev && push_mode && rev_line) 
         increment = -1;
#endif /* BIDI */
    if (insert_mode) { /* push the buffer over one until the first null */
#ifdef BIDI
TN_LOG((dumpfno,"fs_insert scr %d fld %d sp %d\n",screen_reversed,field_rev,sp));
        if (screen_reversed || (field_rev && rev_line))  /*  BIDI support */
          /* find last char of "push" region to be moved over */
          for (i = num_chars_pushed; i > 0 ; i--)
          {
            sp += increment;
            if (sp >= end)
              sp = screen;
          }
#endif /* BIDI */
        /* find the last character to be pushed over */
        while (sp->ch && !(sp->attr & ATTR_TOP)) {
            sp += increment;
            if (sp >= end)
                sp = screen;
        }
        if (sp->attr & ATTR_TOP)
         {
            sp += increment;
            if (sp < screen)
                sp = end;
         }
        if (sp->ch && sp->ch != EBCDIC_SP) {    /* "squishable" blanks */
            fs_alarm ();
            return;
        }

        if (!invisible) {
            cp = sp - screen;   /* cursor position of last char to move */
            if (cp < 0)
               cp = -cp;
            row = cp / linelength;
            if ((column = cp % linelength) != linelength - 1)
             {
#ifdef BIDI
              if (screen_reversed) modified_lines[row] = TRUE;
#endif /* BIDI */
                mvdelch (row, column);

              }
        }
        while (sp != sbufp) {   /* push characters */
            sp->ch = (sp - increment)->ch;
            if (!invisible) {
                if (column == 0) {                   /* beginning of line */
#ifdef BIDI
                    if (screen_reversed) modified_lines[row] = TRUE;
#endif /* BIDI */
#ifdef CURSES_FIXED
                    mvinsch (row--, column, convert_char(cd1, sp->ch));
#else
                    mvinsch (row, column, ' ');
                    mvaddch (row--, column, convert_char(cd1, sp->ch));
#endif
                    column = linelength;
                }
                else if (column == linelength - 1)   /* end of line */
                 {
#ifdef BIDI
                   if (screen_reversed) modified_lines[row] = TRUE;
#endif /* BIDI */
                    mvdelch (row, column);
                 }
                column = column - increment;
            }
            sp -= increment;
            if (sp < screen)
                sp = end;
        }

#ifdef CURSES_FIXED
        if (!invisible)
#ifdef BIDI
          {
            if (screen_reversed) modified_lines[row] = TRUE;
            if ( (field_rev) && (!push_mode) && !field_push)
                column++;
            if ((push_mode) && (insert_mode) && (!in_push_boundry_mode))
                column++;
#endif /* BIDI */
            check_to_start_autopush(ch); 
            if ((!screen_reversed) && (!push_mode))
#ifdef BIDI
            if ((push_mode) && (insert_mode) && (!in_push_boundry_mode))
                 column--;

            if (push_mode) num_chars_pushed++;

            if (field_rev) num_chars_reversed++;
        }
#endif /* BIDI */
#else
        if (!invisible) {
#ifdef BIDI
            if (screen_reversed) modified_lines[row] = TRUE;
TN_LOG((dumpfno, "\n\tfs_char field %d push %d insert %d col %d\n",field_rev,push_mode,insert_mode,column));
            if ((push_mode) && (insert_mode) && (!in_push_boundry_mode))
                 column++;
#endif /* BIDI */
            mvinsch (row, column, ' ');
            mvaddch (row, column,
                     ch == DO_DUP ? '*' : ch == DO_FIELDMARK ? ';' : ch);
#ifdef BIDI
             if ((push_mode) && (insert_mode) && (!in_push_boundry_mode))
                 column--;

            if (push_mode) num_chars_pushed++;

            if (field_rev) num_chars_reversed++;
#endif /* BIDI */
        }
#endif /* CURSES_FIXED */
TN_LOG((dumpfno, "\n\tfs_char moving row column %d,%d\n", row,  column));
    }
    else if (!invisible)        /* overstrike mode */
#ifdef BIDI
        {
             if (screen_reversed || (field_rev && push_mode))
                 modified_lines[cursorposition / linelength] = TRUE;
TN_LOG((dumpfno, "\n\tfs_cursor  %d %d push %d no %d \n", cursorposition,cursorposition % linelength,push_mode,num_chars_pushed));
             if (field_rev && push_mode && !rev_line)
             {
                for (i = num_chars_pushed; i > 0 ; i--)
                {
                   sp -= increment;
                   if (sp < screen)
                      sp = screen;
                }
                if ((sp->attr & ATTR_TOP) || (sp->attr & ATTR_PROT)) {
                    fs_alarm ();
                    return;
                }
                cp = sp - screen;     /* cursor position of last char to move */
                if (cp < 0)
                   cp = -cp;
                while (sp != sbufp) {   /* push characters */
                    sp->ch = (sp + increment)->ch;
                    sp += increment;
                    if (sp >= end)
                        sp = end;
                }
              }
               if (push_mode)
               {
#endif /* BIDI */
                  addch (ch == DO_DUP ? '*' : ch == DO_FIELDMARK ? ';' : ch);
#ifdef BIDI
                  set_bufaddr (cursorposition , FALSE);
                }
                else 
                 {
                  if ((!screen_reversed) && (!autopush_mode))
                     addch (ch == DO_DUP ? '*' : ch == DO_FIELDMARK ? ';' : ch);
                 }
                if (push_mode && !rev_line) num_chars_pushed++;

                if (field_rev) num_chars_reversed++;
        }
#endif /* BIDI */
    (sbufp + sbufp->offset)->attr |= ATTR_MDT;
#ifdef BIDI
    if ((push_mode) && (insert_mode) && (!in_push_boundry_mode))
         sbufp++;
    if ((!rev_line) && (push_mode) && (!insert_mode) && (!in_push_boundry_mode))
         sbufp--;
#endif /* BIDI */
    sbufp->ch = ch == DO_DUP ? EBCDIC_DUP :
                ch == DO_FIELDMARK ? EBCDIC_FM :
                convert_char(cd2, ch);
#ifdef ARAB
    fs_shape(sbufp,handle); /* to shape new character and its surroundings */
#endif /* ARAB */
#ifdef BIDI
    if ((push_mode) && (insert_mode) && (!in_push_boundry_mode))
         sbufp--;

    if ((!rev_line) && (push_mode) && (!insert_mode) && (!in_push_boundry_mode))
         sbufp++;
#endif /* BIDI */
    if (ch == DO_DUP)
        fs_tab ();
    else {
#ifdef BIDI
        if (field_rev)        /* BIDI support */
          {
             if (insert_mode)
             {
                if (field_push)
                     set_bufaddr (cursorposition+1, TRUE);
                else set_bufaddr (cursorposition, TRUE);
             }
             else if (!push_mode || rev_line)             /* overwrite, !push */
                set_bufaddr (cursorposition - 1, TRUE);
             else                                  /* overwrite,   push */
                set_bufaddr (cursorposition, TRUE);

TN_LOG((dumpfno, "\n cursor pos %d  % line %d\n",cursorposition,cursorposition %linelength));
             if ((sbufp + 1) == field_org_start)
             {
                        set_bufaddr(field_end, TRUE);
#endif /* BIDI */
                        set_bufaddr (cursorposition + 1, TRUE);
#ifdef BIDI
                        fs_fldrev();
#endif /* BIDI */
        if (sbufp->attr & ATTR_TOP) {
            if ((sbufp->attr & ATTR_PROT) && (sbufp->attr & ATTR_NUMERIC))
                fs_tab ();              /* autoskip field */
            else
             if (!push_mode)
                set_bufaddr (cursorposition + 1, TRUE);
        }
#ifdef BIDI
    }
  }
        else
        {
        if (!push_mode)
            set_bufaddr (cursorposition + 1, TRUE);
        else
            set_bufaddr (cursorposition, TRUE);

TN_LOG((dumpfno, "\n\tcursorpos in fs_char %d", cursorposition % linelength));

        if (sbufp->attr & ATTR_TOP)
        {
            if ((sbufp->attr & ATTR_PROT) && (sbufp->attr & ATTR_NUMERIC))
                fs_tab ();              /* autoskip field */
            else
                if (!push_mode)
                   set_bufaddr (cursorposition + 1, TRUE);
        }
        }
    }
TN_LOG((dumpfno,"fs_character: cursorposition=%d\n",cursorposition));
if (((national_lang_mode) && (LTR_autopush_enabled||push_mode)) && (!screen_reversed))
  {
     direction = LTR;
     fs_draw_screen(direction);
  }
#endif /* BIDI */
}

int fs_clear ()
{
    clear_screen ();
    lines = deflines;
    linelength = deflinelength;
    send_pa (AID_CLEAR);
}
 
int fs_delete ()
{
    register struct screenchar	*sp;
    register struct screenchar *last_in_row;
    register int		column;
    register int		row;
    register struct screenchar	*end;
    register int		invisible;
    static   char               tmpc;
    int                         increment;
    int                         st;
    int                         direction;    

    sp = sbufp;
    end = screen + linelength * lines;
    /* can't delete protected fields or attribute bytes */
    if ((sbufp->attr & ATTR_TOP) || (sbufp->attr & ATTR_PROT)) {
	fs_alarm ();
	return;
    }
    TN_LOG((dumpfno,"\nfs_delete: end=%d\n",*end));
    (sbufp + sbufp->offset)->attr |= ATTR_MDT;
     increment = 1;
#ifdef BIDI
    if (field_rev) 
         increment = -1;
#endif /* BIDI */ 
    if (!(invisible = set_attribute (sbufp->attr, 0))) {
	row = cursorposition / linelength ;
	if ((column = cursorposition % linelength) != 0) {
#ifdef BIDI
         if (screen_reversed) modified_lines[row] = TRUE;
         if ((push_mode) && (sp->ch != EBCDIC_NUL)) num_chars_pushed--;
         if (field_rev) num_chars_reversed--;
#endif /* BIDI */
        delch ();
       }
    }
    /*sp->ch=EBCDIC_NUL;*/
    sp += increment; 
    while (sp != sbufp && !(sp->attr & ATTR_TOP)) {	/* push characters */
	if (sp >= end) {
	    sp = screen;
	    if (!invisible)
		row = 0;
	}
	(sp - increment)->ch = sp->ch;
 
	if (!invisible) {
	    if (column == 0)			   /* beginning of line */
             {
#ifdef BIDI
                if (screen_reversed) modified_lines[row] = TRUE;
#endif /* BIDI */    
		mvdelch (row, column);
                column = column + increment;
             }
	    else if (column == linelength - 1) {   /* end of line */
#ifdef BIDI
            if (screen_reversed) modified_lines[row] = TRUE;
#endif /* BIDI */
		mvaddch (row++, column, convert_char(cd1, sp->ch));
		column = 0;
	    }
	    else				   /* middle of line */
		column= column+ increment;
	}
        sp += increment;
    }
 
    (sp - increment)->ch = EBCDIC_NUL;
    mvinsch(row,column,' ');
    /*(sbufp + sbufp->offset)->attr |= ATTR_MDT;*/
    if (!invisible) {
#ifdef CURSES_FIXED
	mvinsch (row, column, ' ');
#else
	mvinsch (row, column, ' ');
	mvaddch (row, column, ' ');
#endif
#ifdef BIDI   
        if (sbufp->ch == EBCDIC_NUL && push_mode &&
                        cursorposition != push_start_loc && (!screen_reversed))
            set_bufaddr(cursorposition - 1, TRUE);
        if (screen_reversed) modified_lines[row] = TRUE;
#endif /* BIDI */
	move (cursorposition / linelength, cursorposition % linelength);
    }
#ifdef ARAB
     /* shape around deleted character */
     fs_shape(sbufp,handle);
#endif /* ARAB */
TN_LOG((dumpfno,"fs_delete: row=%d, column=%d\n",row,column));
if ((screen_reversed) && (push_mode))                                           
    {
      mvdelch(row,column);
      mvaddch(row,column,' ');
      modified_lines[row] = TRUE;
      direction = RTL;
      fs_draw_screen(direction);
    }
if(field_rev) 
   { 
    direction = LTR;
    fs_draw_screen(direction);
   }
if(screen_reversed)
  { 
  direction = RTL;
  fs_draw_screen(direction);
 } 

}
 
int fs_down ()
{
    set_bufaddr (cursorposition + linelength, TRUE);
#ifdef BIDI
    if (push_mode) check_push_outof_bounds();
#endif /* BIDI */
refresh();
}
 
int fs_dup ()
{
    fs_character (DO_DUP);
}
 
int fs_enter ()
{
    send_modified_fields (AID_ENTER);
#ifdef BIDI
    if (field_rev) fs_fldrev();
#endif /* BIDI */
}
 
int fs_eraseeof ()
{
    register struct screenchar	*sp;
    register struct screenchar	*end;
    register int		invisible;
 

    TN_LOG((dumpfno, "\n\tfs_eraseeof()"));
    sp = sbufp;
    end = screen + linelength * lines;
    if (num_attributes) {
	if (sbufp->attr & (ATTR_TOP | ATTR_PROT)) {
	    fs_alarm ();
	    return;
	}
	invisible = set_attribute ((sbufp + sbufp->offset)->attr |= ATTR_MDT,0);
 
	while (! (sp->attr & ATTR_TOP)) {
	    sp->ch = EBCDIC_NUL;
	    if (! invisible)
             {
#ifdef BIDI
                if (screen_reversed)            /*  BIDI mode  */
                modified_lines[ cursorposition / linelength ] = TRUE;
#endif /* BIDI */
		addch (' ');
             }
	    if (++sp >= end)
		sp = screen;
	}
#ifdef ARAB
TN_LOG((dumpfno, "\nARAB:entering fs_shape from eraseeof pointing to %c",sbufp->ch));
           fs_shape(sbufp,handle);  /* reshape around removed character */
#endif /* ARAB */
    }
    else {
	clear_screen ();
	clear ();
	xstandend(curscr);
    }
#ifdef BIDI
    if (screen_reversed) flip_modified_rows ();
    if (field_rev) fs_fldrev();
#endif /* BIDI */
    touchwin(stdscr);
    refresh ();
    set_bufaddr (cursorposition, FALSE);

}
 
int fs_eraseinput ()
{
    erase_all_unprotected ();
#ifdef BIDI
    if (push_mode) check_push_outof_bounds();
    if (field_rev) fs_fldrev();
#endif
}
 
int fs_fieldmark ()
{
    fs_character (DO_FIELDMARK);
}
 
/* act like a 3270 "Local Home" key */
 
int fs_home ()
{
    register struct screenchar	*sp;
    register struct screenchar	*end;
 

    end = screen + linelength * lines;
    if (num_attributes) {
	sp = first_attr;
	while (sp->attr & ATTR_PROT) {
	    /* there are no unprotected fields */
	    if (sp > sp + sp->offset) {	
		set_bufaddr (0, TRUE);
		return;
	    }
	    sp = sp + sp->offset;
	}
	sp++; /* first character in field */
	if (sp >= end) sp = screen;
	set_bufaddr (sp - screen, TRUE);
    }
    else set_bufaddr (0, TRUE);
#ifdef BIDI
    if (push_mode) check_push_outof_bounds();
    if (field_rev) fs_fldrev();
#endif

}
 
int fs_insertmode ()
{
	extern char	*VS, *VE;
 
	if (insert_mode == TRUE) {
		insert_mode = FALSE;
		_tputvs(VE);
		}
	else   {
    		insert_mode = TRUE;
		_tputvs(VS);
		}
}
 
 
int fs_left ()
{
#ifdef BIDI
    /* switch the arrow keys */
    if (screen_reversed && !switched_already)
    {
      switched_already = TRUE;
      fs_right();
      switched_already = FALSE;
      return;
    }

    if (push_mode)
    {
        if ((cursorposition == push_start_loc) &&
            (!in_push_boundry_mode))
                InBoundryCheck();
        else
                if (cursorposition > push_start_loc)
                        set_bufaddr (cursorposition - 1, TRUE);
                else
                        fs_alarm();
    }
    else
#endif /* BIDI */

    set_bufaddr (cursorposition - 1, TRUE);
}
 
int fs_pa1 ()
{
    send_pa (AID_PA1);
}
 
int fs_pa2 ()
{
    send_pa (AID_PA2);
}
 
int fs_pa3 ()
{
    send_pa (AID_PA3);
}
 
int fs_penselect ()
{
    fprintf(stderr, MSGSTR(NO_PEN_SEL, "\rSorry, no pen select yet\r")); /*MSG*/
}
 
int fs_pf1 ()
{
    send_modified_fields (AID_PF1);
}
 
int fs_pf10 ()
{
    send_modified_fields (AID_PF10);
}
 
int fs_pf11 ()
{
    send_modified_fields (AID_PF11);
}
 
int fs_pf12 ()
{
    send_modified_fields (AID_PF12);
}
 
int fs_pf13 ()
{
    send_modified_fields (AID_PF13);
}
 
int fs_pf14 ()
{
    send_modified_fields (AID_PF14);
}
 
int fs_pf15 ()
{
    send_modified_fields (AID_PF15);
}
 
int fs_pf16 ()
{
    send_modified_fields (AID_PF16);
}
 
int fs_pf17 ()
{
    send_modified_fields (AID_PF17);
}
 
int fs_pf18 ()
{
    send_modified_fields (AID_PF18);
}
 
int fs_pf19 ()
{
    send_modified_fields (AID_PF19);
}
 
int fs_pf2 ()
{
    send_modified_fields (AID_PF2);
}
 
int fs_pf20 ()
{
    send_modified_fields (AID_PF20);
}
 
int fs_pf21 ()
{
    send_modified_fields (AID_PF21);
}
 
int fs_pf22 ()
{
    send_modified_fields (AID_PF22);
}
 
int fs_pf23 ()
{
    send_modified_fields (AID_PF23);
}
 
int fs_pf24 ()
{
    send_modified_fields (AID_PF24);
}
 
int fs_pf3 ()
{
    send_modified_fields (AID_PF3);
}
 
int fs_pf4 ()
{
    send_modified_fields (AID_PF4);
}
 
int fs_pf5 ()
{
    send_modified_fields (AID_PF5);
}
 
int fs_pf6 ()
{
    send_modified_fields (AID_PF6);
}
 
int fs_pf7 ()
{
    send_modified_fields (AID_PF7);
}
 
int fs_pf8 ()
{
    send_modified_fields (AID_PF8);
}
 
int fs_pf9 ()
{
    send_modified_fields (AID_PF9);
}
 
int fs_reset ()
{
    extern char	*VE;
    fs_keyboard_locked = FALSE;
    x_system(0);
    if(insert_mode == TRUE) {
	_tputvs(VE);
    	insert_mode = FALSE;
	}
}
 
/* act like a 3270 "Local Newline" key */
 
int fs_return ()
{
    register struct screenchar	*sp;
    register struct screenchar	*end;
 
    sp = sbufp;
    end = screen + linelength * lines;
    sp += (linelength - cursorposition % linelength) - 1; /* end of line */
    if (sp >= end) sp = screen;
    
    set_bufaddr (sp - screen, TRUE);
#ifdef BIDI
    if (push_mode) check_push_outof_bounds();
#endif
    fs_tab();
}
 
int fs_right ()
{
#ifdef BIDI
    /* switch the arrow keys */
    if (screen_reversed && !switched_already)
    {
      switched_already = TRUE;
      fs_left();
      switched_already = FALSE;
      return;
    }

    if (push_mode)
    {
        if ( (num_chars_pushed > 0) &&
             (cursorposition == push_start_loc) &&
             (in_push_boundry_mode) )
                OutBoundryCheck();
        else
        {
                if (!outof_right_push_bounds())
                     set_bufaddr (cursorposition + 1, TRUE);
                else
                        fs_alarm();
        }
    }
    else
#endif /* BIDI */
    set_bufaddr (cursorposition + 1, TRUE);
}
 
int fs_sysreq ()
{
    send_modified_fields (AID_SYSREQ);
    lines = deflines;
    linelength = deflinelength;
}
 
/* act like a 3270 "Local Tab" key */
 
int fs_tab ()
{
    register struct screenchar	*sp;
    register struct screenchar	*end;
    register struct screenchar	*oldsp;
 
    if (num_attributes == 0) return;
    
    oldsp = sp = sbufp;
    end = screen + linelength * lines;
 
    while (! (sp->attr & ATTR_TOP) || (sp->attr & ATTR_PROT)) {
	if (++sp >= end)
	    sp = screen;
	if (sp == oldsp)
	    break;
    }
    if (!field_rev)
      set_bufaddr (sp - screen + 1, TRUE);
#ifdef BIDI
    if (push_mode) check_push_outof_bounds();
    if (field_rev) 
       {
        fs_fldrev();
        set_bufaddr(sp-screen + 1,FALSE);
       }
#endif

}
 
/* act like a 3270 Program Tab order */
 
int fs_tab_in (two_PT_in_a_row, nulling)
int two_PT_in_a_row, nulling;
{
    register struct screenchar	*sp, *tmpsp; 
    register struct screenchar	*end;
    char found_attr = 0;
    int nrow, ncol;
 
    TN_LOG((dumpfno, "\n\tfs_tab_in(%d) called", two_PT_in_a_row));
    
again1:
    tmpsp = sp = sbufp;
    end = screen + linelength * lines;
 
    /* if the current location is an unprotected field attribute */
    if ((sp->attr & ATTR_TOP) && !(sp->attr & ATTR_PROT)) {
        TN_LOG((dumpfno,
            "\n\tfs_tab_in(), we are at an attribute and its unprotected\n"));
	if (++sp >= end) { 		/* handle wrap */
           set_bufaddr (0, FALSE);
	   if (two_PT_in_a_row)
		goto again;
	} else {
           set_bufaddr (sp - screen, FALSE);
	   if (two_PT_in_a_row)
		goto again;
        }
    } else {
	/* Find 1st attribute between sp and the end of the screen */
	found_attr = 0;
	do {
	   if (tmpsp->attr & ATTR_TOP) {
	      found_attr = 1;
	      ADDR_TO_RC(tmpsp - screen, nrow, ncol);
	      TN_LOG((dumpfno,
   "\n\tfs_tab_in(), found 1st attribute after sp at tmpsp=%d,%d(%d); end=%d\n",
	      nrow, ncol, tmpsp, end));
	   } else {
		   tmpsp++;
	   }
        } while (found_attr == 0 && tmpsp <= end);
	if (found_attr) {
	    /* Found 1st attribute between sp and the end of the screen */
	    /* and tmpsp is pointing a it, so we null out from sp to tmpsp. */
	    while (sp < tmpsp) {
		if (nulling) {
			ADDR_TO_RC(sp - screen, nrow, ncol);
			TN_LOG((dumpfno,
				"\n\tfs_tab_in(),Nulling 1 out sp=%d,%d(%d)\n",
							nrow, ncol, sp));
			sp->ch = EBCDIC_NUL;
			sp->attr = EBCDIC_NUL;
		}
		sp++;
	    }
	    /* if the attribute we found is a protected attribute then */
 	    /* scan from this attribute to the end of the screen for an */
	    /* unprotected attribute */
	    if (sp->attr & ATTR_PROT) {
	        ADDR_TO_RC(sp - screen, nrow, ncol);
	        TN_LOG((dumpfno,
		"\n\tfs_tab_in(),found a protected attribute at sp=%d,%d(%d)\n",
							nrow, ncol, sp));
		found_attr = 0;
		tmpsp = ++sp;
		do {
/*
	           ADDR_TO_RC(tmpsp - screen, nrow, ncol);
	           TN_LOG((dumpfno,
    "\n\tfs_tab_in(),looking for an unprotected attribute at tmpsp=%d,%d(%d)\n",
							nrow, ncol, tmpsp));
*/
		   if ((tmpsp->attr & ATTR_TOP) && !(tmpsp->attr & ATTR_PROT))
		          found_attr = 1;
		   else
		      tmpsp++;
		} while (found_attr == 0 && tmpsp < end);
		/* If we found an unprotected attribute we set the sp to the */
		/* 1st position in that unprotected field */
		if (found_attr) {
	           TN_LOG((dumpfno,
	"\n\tfs_tab_in(),found an unprotected attribute at tmpsp=%d,%d(%d)\n",
							nrow, ncol, tmpsp));
                   set_bufaddr (tmpsp - screen + 1, FALSE);
		   if (two_PT_in_a_row)
			goto again;
                } else {
	           TN_LOG((dumpfno,
	"\n\tfs_tab_in(),could not find an unprotected attribute before the end of the screen\n setting sp to 0,0"));
                   set_bufaddr (0, FALSE);
		   if (two_PT_in_a_row)
			goto again;
		}
	    } else {
	        if (!(sp->attr & ATTR_PROT)) {
	           ADDR_TO_RC(sp - screen, nrow, ncol);
	           TN_LOG((dumpfno,
	      "\n\tfs_tab_in(),found a unprotected attribute at sp=%d,%d(%d)\n",
							nrow, ncol, sp));
                   set_bufaddr (sp - screen + 1, FALSE);
	           if (two_PT_in_a_row)
		      goto again;
		}
	    }
        } else { /* Could not find 1st attribute between sp and the end of */
		 /* the screen, so we fill from sp to end of the screen */
	   while (sp < end) {
	      if (nulling) {
		      ADDR_TO_RC(sp - screen, nrow, ncol);
		      TN_LOG((dumpfno,
				"\n\tfs_tab_in(), Nulling 2 out sp=%d,%d(%d)\n",
	      						nrow, ncol, sp ));
		      sp->ch = EBCDIC_NUL;
		      sp->attr = EBCDIC_NUL;
	      }
	      sp++;
	   }
	   set_bufaddr (0, FALSE);
	   if (two_PT_in_a_row)
		goto again;
	}
    }
    touchwin(stdscr);
    refresh();
    return;
again:
    two_PT_in_a_row = 0;
    goto again1;
}
 
int fs_up ()
{
    set_bufaddr (cursorposition - linelength, TRUE);
#ifdef BIDI
    if (push_mode) check_push_outof_bounds();
#endif
}
 
int fs_wordnext ()
{
    register struct screenchar	*sp;
    register struct screenchar	*end;
    register struct screenchar	*oldsp;
 
    end = screen + linelength * lines;
    oldsp = sp = sbufp;
 
    /* 
     * find the next word
     */
    while (1) {
	if (++sp >= end)
	    sp = screen;
	if (sp == oldsp)
	    return;
	if (! (sp->attr & ATTR_TOP) && 
	    ! (sp->attr & ATTR_PROT) &&
	      (isalnum(convert_char(cd1, sp->ch)))) {
	    break;
	}
 
    }
    /* 
     * find the end of it
     */
    while (1) {
	if (++sp >= end)
	    sp = screen;
	if (sp == oldsp)
	    return;
	if (! (isalnum(convert_char(cd1, sp->ch)))) {
	    break;
	}
    }
    set_bufaddr (sp - screen, TRUE);
}
 
int fs_wordprev ()
{
    register struct screenchar	*sp;
    register struct screenchar	*end;
    register struct screenchar	*oldsp;
 
    end = screen + linelength * lines;
    oldsp = sp = sbufp;
 
    /* 
     * find the prev word
     */
    while (1) {
	if (--sp < screen)
	    sp = end;
	if (sp == oldsp)
	    return;
	if (! (sp->attr & ATTR_TOP) && 
	    ! (sp->attr & ATTR_PROT) &&
	      (isalnum(convert_char(cd1, sp->ch)))) {
	    break;
	}
 
    }
    /* 
     * find the beginning of it
     */
    while (1) {
	if (--sp < screen)
	    sp = end;
	if (sp == oldsp)
	    return;
	if (! (isalnum(convert_char(cd1, sp->ch)))) {
	    break;
	}
    }
    set_bufaddr (sp - screen + 1, TRUE);
 
}

 
/*
 * Display "X SYSTEM" when keyboard is locked.
 * We'll display this message only if terminal has enough room for this.
 */
x_system(onoff)
{
	if (!X_row)		/* no space to put "X SYSTEM" line	*/
		return;
 
	if (onoff) {
		if (lo_prot_color)
			colorout(lo_prot_color | background_color);
                mvaddstr(X_row, X_col, MSGSTR(X_SYSTEM_LABEL, "X SYSTEM")); /*MS
G*/
		}
	else 
                mvaddstr(X_row, X_col, MSGSTR(X_SYSTEM_BLANK, "        ")); /*MS
G*/

	standend();
    	set_bufaddr (cursorposition, TRUE);
	touchwin(stdscr);
#ifdef BIDI
        if (screen_reversed) flip_modified_rows ();
#endif
	refresh();
}
		
tty_litout( enter_mode ) 
register int enter_mode;
{ 
	char			tmp[10];
	static WINDOW		*sv_stdscr;
	static int		newwin_opened = 0;
#ifdef BIDI
        int                     i;
#endif /* BIDI */

        TN_LOG((dumpfno, "tty_litout(%d)\n", enter_mode));
 	if (!connected)	{	/* no need to save or restore since */
		return;		/* we're not even connected	    */
	}
 
	if (!enter_mode) {
		extern char *VE, *TE;

		newwin_opened = 1;
		sv_stdscr = stdscr;
		stdscr = newwin(stdscr->_maxy, stdscr->_maxx,
				stdscr->_begy, stdscr->_begx);
		touchwin(stdscr);
		refresh();
		_tputvs(VE);
		_tputvs(TE);
	}
 
 
	if (enter_mode && newwin_opened)
                printf(MSGSTR(PRESS_ENTER, "\nPress ENTER to continue...")); /*M
SG*/
	fflush(stdout); 
	if (enter_mode && newwin_opened)
		/*gets(tmp);unicode*/
                  read(fileno(stdin), &tmp, sizeof(tmp));/*unicode*/
	if (enter_mode && newwin_opened) {
		extern char	*CL, *TE;
		delwin(stdscr);
		_tputvs(TE);
		stdscr = newwin(0, 0, 0, 0);
		touchwin(stdscr);
#ifdef BIDI
                if (screen_reversed) flip_modified_rows ();
#endif
		refresh();
		delwin(stdscr);
		stdscr = sv_stdscr;
#ifdef BIDI
                if (screen_reversed)
                  /*for (i = 0; i < 27; i++)unicode*/
                 for (i = 0; i < (sizeof(modified_lines) /                                                              sizeof(modified_lines[0])); i++)
                    modified_lines[i] = TRUE;/*unicode*/
                    /*for (i = 0;i < (sizeof(modified_lines) / 
                                               sizeof(modified_lines[0]); i++)*/
#endif
		touchwin(stdscr);
#ifdef BIDI
                if (screen_reversed) flip_modified_rows ();
                Set_NoBidi_Mode();
#endif
		refresh();
		newwin_opened = 0;
	}
} 
 
extern int sendbrk();

/* Implement the attention key by sending a IAC BRK */
int fs_attention ()
{
	(void) sendbrk();
}



#ifdef BIDI

/* BIDI support - this procedure is called when a refresh is   */
/* requested. Must check to see if any lines were changed and    */
/* if yes must flip those lines and put them into stdscr (to go  */
/* right-to-left).     */
void flip_modified_rows ()
{
  register struct screenchar     *last_in_row;
  register struct screenchar     *sp;

  static int  cur_loc;
  register int  row, rowww, col;
  static int  invisible;
  static char tmpc;

/*   for (row = 0; row < (lines - 1); row++)   does NOT include 24th line */
  for (row = 0; row < lines; row++)
  if (modified_lines[row])
  {
      sp = screen + row * linelength;
      last_in_row  = sp + (linelength - 1);
      for (cur_loc = 0; cur_loc < linelength; cur_loc++)
        {
          /* must shift over the character and its attribute */
          invisible = set_attribute(sp->attr, 0); 
          if (screen_reversed)
              ADDR_TO_RC(last_in_row - sp, rowww, col);
          if (!screen_reversed)
              ADDR_TO_RC(sp - screen, rowww, col);
          move(row, col);
          tmpc = sp->ch;
#ifdef ARAB
          if (screen_reversed)
          {
              if (SYMSWAP_ON)     /* symmetric swap the characters */
                   fs_translate(TOCHRREV,&tmpc);
              if (NUMSWAP_ON)    /* numeric swap the characters */
                   fs_translate(TONUMSWAP,&tmpc);
          }
#endif /* ARAB */
          tmpc = convert_char(cd1, tmpc);
          if (tmpc != NULL)
   /*   TN_LOG((dumpfno, "WRITE addch %c  \n",tmpc));*/

          if ( (tmpc == NULL) || (invisible) )
            addch(' ');
          else
            addch(tmpc);
          sp++;
        }
    }
    col = cursorposition % linelength;
    if (screen_reversed)
         move((cursorposition / linelength), linelength - col - 1);   /* (80-col
umn)  - 1 */
    else move((cursorposition / linelength), col );
}

/* BIDI support - This procedure will reverse the whole screen.  */
void fs_scrrev ()
{
  register struct screenchar *first_in_row;
  register struct screenchar *last_in_row;
  register struct screenchar *sp;
  register int    rowww, col;
  static   int    cur_loc;
  static   int    row;
  static   int    invisible;
  static   char   tmpc;

TN_LOG((dumpfno, "\n\tbegin of scrrev arabic %d bidi %d\n",ARABIC_ON,BIDI_ON));

#ifdef ARAB
  if (!ARABIC_ON && !BIDI_ON)
     return;
#endif /* ARAB */

TN_LOG((dumpfno, "\n\t continue of scrrev\n"));
  clear ();
  xstandend(curscr);
  refresh();

  if ( !screen_reversed)         /* from LTR to RTL */
  {                               /* BIDI mode  */
    if (push_mode)
        fs_eng_lang();
    else
        fs_national_lang();

    screen_reversed  = TRUE;

    for (row = 0; row < (lines); row++)
    {
        sp = screen + row * linelength;
        last_in_row  = sp + (linelength - 1);
        for (cur_loc = 0; cur_loc < linelength; cur_loc++)
        {
            /* must shift over the character and its attribute */
            invisible = set_attribute(sp->attr,0); 
            ADDR_TO_RC((last_in_row - sp), rowww, col);
            move(row, col);
            tmpc=sp->ch;
#ifdef ARAB
            if (SYMSWAP_ON)     /* symmetric swap the characters */
               fs_translate(TOCHRREV,&tmpc);
            if (NUMSWAP_ON)    /* numeric swap the characters */
               fs_translate(TONUMSWAP,&tmpc);
#endif /* ARAB */
            tmpc = convert_char(cd1, tmpc);
            if ((tmpc == NULL) || (invisible))
                addch(' ');
            else
                addch(tmpc);

            sp++;
        }
    }
    col = linelength - (cursorposition % linelength);
    move((cursorposition / linelength), col - 1);   /* (80-column) - 1 */
  }

  else         /* RTL to LTR */
  {
    if (push_mode)
        fs_national_lang();
    else
        fs_eng_lang();

    screen_reversed  = FALSE;      /* english mode */

    for (row = 0; row < lines; row++)
    {
        sp = screen + row * linelength;
        for (cur_loc = 0; cur_loc < linelength; cur_loc++)
        {
            /* display the character and its attribute */
            invisible = set_attribute(sp->attr,0);
            ADDR_TO_RC(sp - screen, rowww, col);
            move(rowww, col);
            tmpc =   convert_char(cd1, sp->ch);
            if ((tmpc == NULL) || (invisible))
              addch(' ');
            else
                addch(tmpc);

            sp++;
        }
    }
    move(cursorposition / linelength, cursorposition % linelength);
  }

  /* BIDI support */
  if (field_rev)
      flip_langs();

  refresh();

}

/* BIDI support - This procedure will set the push mode flag  */
/* to true and reset all appropriate variables.                 */
void fs_push ()
{

#ifdef ARAB
  if (!ARABIC_ON && !BIDI_ON)
   {
TN_LOG((dumpfno,"fs_push: not bidi, returning\n"));
     return;
   }
#endif /* ARAB */

TN_LOG((dumpfno,"fs_push \n"));
  push_start_loc       = cursorposition;
  before_push_insert_status = insert_mode;
  if (!field_rev || rev_line)
     insert_mode          = TRUE;
TN_LOG((dumpfno,"fs_push field %d revv %d insert %d\n",field_rev,rev_line,insert_mode));
  push_mode            = TRUE;
  num_chars_pushed     = 0;
  in_push_boundry_mode = TRUE;

  if (!autopush_mode)
  {
TN_LOG((dumpfno,"fs_push : not autopush \n"));
        flip_langs();
        if (field_rev && !rev_line && insert_mode)
        {
           push_mode            = FALSE;
           in_push_boundry_mode = FALSE;
           field_push           = TRUE;
        }
  }
TN_LOG((dumpfno,"fs_push : exiting procedure\n"));
}
/* BIDI support - This procedure will try to place the cursor       */
/* at the end of the push region and reset all appropriate variables. */
void fs_endpush (full)
register int full;
{
    register struct screenchar  *sp;
    register struct screenchar  *end;
    int i, increment;

#ifdef ARAB
  if (!ARABIC_ON && !BIDI_ON)
     return;
if (screen_reversed)
#ifndef _POWER
   fs_setcursorshape(HFDBLUS);
 else
   fs_setcursorshape(HFSNGLUS);
#endif /* _POWER */
#endif /* ARAB */

TN_LOG((dumpfno,"fs_endpush scr %d fld %d full %d cursor pos %d\n",screen_reversed,field_rev,full,cursorposition % linelength));
    if (push_mode)
    {
TN_LOG((dumpfno,"fs_endpush  pushstart %d %d num %d\n",push_start_loc,push_start_loc % linelength,num_chars_pushed));
    if (full && (push_start_loc != 0))
    {

        if (field_rev)              /*  && rev_line) */
             increment = -1;
        else increment = 1;
        sp  = sbufp;
        end = screen + linelength * lines;
#ifdef ARAB
  fs_shape(sbufp,handle);      /* shape at end of push and refresh */
#endif /* ARAB */

        /* find the first character after the pushed region */
        for (i = num_chars_pushed; (sp->ch && (i > 0)); i--)
        {
                sp = sp + increment;
                if (sp >= end)
                   sp = screen;
        }

        if (sp->attr & ATTR_TOP)
        if (--sp < screen)
           sp = end;

        /* place cursor at new location */
        move ((sp - screen) / linelength, (sp - screen) % linelength);

        /* reset  position of cursor */
        if (field_rev)              /*  && rev_line) */
             set_bufaddr (push_start_loc - num_chars_pushed, TRUE);
        else set_bufaddr (push_start_loc + num_chars_pushed, TRUE);
        sbufp = sp;
    }
    else
        set_bufaddr(cursorposition, TRUE);


    num_chars_pushed = 0;
    push_mode        = FALSE;
    insert_mode      = before_push_insert_status;
    push_start_loc   = 0;
    full             = FALSE;

    flip_langs ();

    } /* if push mode */
    else if (field_push)
         {
            set_bufaddr (push_start_loc + num_chars_pushed, TRUE);
            num_chars_pushed = 0;
            push_mode        = FALSE;
            insert_mode      = before_push_insert_status;
            push_start_loc   = 0;
            full             = FALSE;
            field_push       = FALSE;

            flip_langs ();
         }

}


/* BIDI support - This procedure will set autopush_mode to true,thus enabling
  auto screen orintation according to the language layer.Will set all 
  appropriate flags   */
void fs_autopush ()
{
#ifdef ARAB
  if (!ARABIC_ON && !BIDI_ON)
     return;
#endif /* ARAB */

TN_LOG((dumpfno,"fs_autopush scr %d fld %d \n",screen_reversed,field_rev));
  if (screen_reversed || field_rev)     /* BIDI mode */
    if (RTL_autopush_enabled)
      {
        RTL_autopush_enabled = FALSE;
        autopush_mode        = FALSE;
        fs_endpush(TRUE);
      }
    else
      RTL_autopush_enabled   = TRUE;
  else                     /* english mode */
    if (LTR_autopush_enabled)
      {
        LTR_autopush_enabled = FALSE;
        autopush_mode        = FALSE;
        fs_endpush(TRUE);
      }
    else
      LTR_autopush_enabled   = TRUE;
}

void check_to_start_autopush(key)
unsigned char key;
{
register struct screenchar *last_in_row;
register struct screenchar *sp;
register int                column;
register int                row,col,rowww;
static   int                cur_loc;
static   char               tmpc;
register int                invisible;

/*
 fprintf(stderr,"the key is =%d  ", key);
*/
if ((screen_reversed) && (national_lang_mode))  
  {
   autopush_mode = FALSE;
   fs_endpush(TRUE);
  }
TN_LOG((dumpfno,"check autopush scr %d fld %d RTL %d LTR %d\n",screen_reversed,field_rev,RTL_autopush_enabled,LTR_autopush_enabled));
  if ((screen_reversed && RTL_autopush_enabled)
     || (field_rev && RTL_autopush_enabled) )
  {
     if ((((key > 47) && (key < 58)) ||         /* key is 0-9 or A-Z */
           ((key > 64) && (key < 91)) &&
           (national_lang_mode)) ||               /* and in RTL language */
           (latin_lang_mode /*&& (key != ' ')*/))  /* or in LTR lang and  */
           {                                         /* char != space       */
             autopush_mode = TRUE;
TN_LOG((dumpfno,"check_to_start_autopush : autopush_mode = TRUE\n"));
             if (!push_mode)
               fs_push();
           }
     else
        if (autopush_mode)
        {
TN_LOG((dumpfno,"check_to_start_autopush: setting autopush_mode = FALSE\n"));
          autopush_mode = FALSE;
          fs_endpush(TRUE);
        }
     return;
  }

  if ( ((!screen_reversed) && LTR_autopush_enabled)
     || (field_rev && LTR_autopush_enabled) )
    if (national_lang_mode)      /* in BIDI lang */
      {
        autopush_mode = TRUE;
        if (!push_mode)
          fs_push();
      }
    else
      if (autopush_mode)
      {
        autopush_mode = FALSE;
        fs_endpush(TRUE);
      }
TN_LOG((dumpfno,"check_to_start_autopush: exiting procedure\n"));
}


/* BIDI support - This procedure will change the lang to english  .And change cursor shape to single US line*/
fs_eng_lang ()
{
#ifdef ARAB
  if (!ARABIC_ON || BIDI_ON)
#endif /* ARAB */
        BDLatinLang(-1); 
        latin_lang_mode  = TRUE;
        national_lang_mode = FALSE;
#ifndef _POWER
        fs_setcursorshape(HFSNGLUS);
#endif /* _POWER */
}

/* BIDI support - This procedure will change the lang to BIDI . And change cursor to line at the middle*/
fs_national_lang ()
{
TN_LOG((dumpfno,"National is ARABIC  %d  BIDI %d\n",ARABIC_ON,BIDI_ON));
#ifdef ARAB
  if (!ARABIC_ON || BIDI_ON)
#endif /* ARAB */
        BDNaturalLang(-1);
        latin_lang_mode  = FALSE;
        national_lang_mode = TRUE;
#ifndef _POWER
        fs_setcursorshape(HFMIDLINE);
#endif /* _POWER */
}


check_push_outof_bounds()
{
        if ((cursorposition < push_start_loc) ||
            (cursorposition > push_start_loc + num_chars_pushed))
                fs_endpush(FALSE);
}


int outof_left_push_bounds()
{
        if (cursorposition <= push_start_loc)
        {
                fs_alarm();
                return(1);
        }
        else
                return(0);
}

int outof_right_push_bounds()
{
        if (cursorposition >= (push_start_loc + num_chars_pushed - 1))
        {
                fs_alarm();
                return(1);
        }
        else
                return(0);
}


fs_fldrev()
{
    register struct screenchar  *end;
    register struct screenchar  *rev_start_p;
    register int                col;
    register int                from_cursorp;
    register int                from_col;
    register int                row;
    register int                offset;

#ifdef ARAB
  if (!ARABIC_ON && !BIDI_ON)
     return;
#endif /* ARAB */

TN_LOG((dumpfno,"fs_fldrev cursor %d\n",cursorposition));
    if (push_mode)
        fs_endpush(FALSE);

    end = screen + linelength * lines;

    if (!field_rev)
    {
        field_rev          = TRUE;
        num_chars_reversed = 0;
        reverse_field();
    }
    else
    {
        field_rev = FALSE;
        reverse_field();
        if (screen_reversed)
            fs_national_lang();
        else
            fs_eng_lang(); 
    }
}

reverse_field()
{
        register struct screenchar      *sp;
        register struct screenchar      *end;
        register struct screenchar      *oldsp;
        static   int    loc = 0;
        static   int    cur_column = 0;
                 int    startf = 0;

TN_LOG((dumpfno,"fs_reverse field loc %d cur_col %d\n",loc, cur_column));
        end   = screen + linelength * lines;
        oldsp = sp = sbufp;

        /* find beginning of the field */
        while (1)
        {
                if (sp < screen)
                        sp = end;
                if ((sp-1)->attr & ATTR_TOP)
                        break;
                sp--;
                if (sp == oldsp)
                        return;
        }

        field_org_start = sp;
        /* find the end of it */
        sp = oldsp;
        while (1)
        {
                if (++sp >= end)
                        sp = screen;
                if (sp == oldsp)
                        break;
                cur_column = (sp - screen) % linelength;
                if ((sp->attr & ATTR_TOP) || (sp->attr & ATTR_PROT))
                {
                        sp--;
                        break;
                }
                if (cur_column == linelength - 1)
                        break;
        }

        field_end       = sp - screen;

        if (field_org_start == oldsp)
                startf = field_end;
        else
                startf = oldsp - screen;


        if ((startf %linelength) == linelength - 1)
        {
          if (loc == startf)                     /* To toggle field reverse */
               startf = startf - linelength + 1;
          else rev_line = TRUE;
        } else rev_line = FALSE;

        set_bufaddr (startf, TRUE);

        loc = startf;
        flip_langs();
}

rev_fld_btab()
{
    register struct screenchar  *sp;
    register struct screenchar  *end;
    register struct screenchar  *oldsp;
    static   int    loc = 0;
    static   int    cur_column = 0;

TN_LOG((dumpfno,"fs_rev field btab \n"));
    end   = screen + linelength * lines;
    oldsp = sp = sbufp;

    /* find the end of it */
    while (1)
    {
        if (++sp >= end)
            sp = screen;
        if (sp == oldsp)
            break;
        cur_column = (sp - screen) % linelength;
        if ((sp->attr & ATTR_TOP) || (sp->attr & ATTR_PROT))
        {
            sp--;
            break;
        }
        if (cur_column == linelength - 1)
           break;

    }

    field_end       = sp - screen;
    set_bufaddr (field_end, TRUE);

    flip_langs();
}


void fs_autorev ()
{
#ifdef ARAB
  if (!ARABIC_ON && !BIDI_ON)
     return;
#endif /* ARAB */

TN_LOG((dumpfno,"fs_autorev \n"));
  if (screen_reversed)     /* BIDI mode */
    if (RTL_autofldrev_enabled)
      {
        RTL_autofldrev_enabled  = FALSE;
        autofldrev_mode         = FALSE;
        already_in_num_fld      = FALSE;
        already_in_alphanum_fld = FALSE;
        fs_fldrev();
      }
    else
      RTL_autofldrev_enabled   = TRUE;
  else                     /* english mode */
    if (LTR_autofldrev_enabled)
      {
        LTR_autofldrev_enabled  = FALSE;
        autofldrev_mode         = FALSE;
        already_in_num_fld      = FALSE;
        already_in_alphanum_fld = FALSE;
        fs_fldrev();
      }
    else
    {
      LTR_autofldrev_enabled   = TRUE;
      check_to_start_autorev();
    }
}


void check_to_start_autorev()
{
TN_LOG((dumpfno,"check field autorev scr %d fld %d \n",screen_reversed,field_rev
));
        if (!(sbufp->attr & ATTR_PROT))
        {
                if (sbufp->attr & ATTR_NUMERIC)
                {
                        if ((!already_in_num_fld) &&
                            (screen_reversed) && (!field_rev))
                                {
                                        already_in_num_fld = TRUE;
                                        fs_fldrev();
                                }
                }
                else
                        if (!(sbufp->attr & ATTR_NUMERIC))
                        {
                                if ((!already_in_alphanum_fld) &&
                                    (!field_rev))
                                {

                                        already_in_alphanum_fld = TRUE;

                                        fs_fldrev();
                                }

                        }
                        else
                        {
                                already_in_num_fld      = FALSE;
                                already_in_alphanum_fld = FALSE;
                        }
        }
        else
        {
                already_in_num_fld      = FALSE;
                already_in_alphanum_fld = FALSE;
        }
}


flip_langs()
{

TN_LOG((dumpfno,"flip langs scr %d push %d fld %d \n",screen_reversed,push_mode,
field_rev));
         if (( !screen_reversed && push_mode && !field_rev) ||
                          /* if LTR screen and push off and field reverse on */
             ( !screen_reversed && !push_mode && field_rev) ||
                          /* or RTL screen and push off and field reverse off*/
             ( screen_reversed && !push_mode && !field_rev) ||
                           /* or RTL screen and push on and field reverse on */
             ( screen_reversed && push_mode && field_rev))
            fs_national_lang();
        else
            fs_eng_lang();
}


Set_NoBidi_Mode ()
{
  /*if (BIDI_ON)
     BDSaveEnv(1);*/
#ifdef ARAB
        BDVisualOn(1);
  if (BIDI_ON)
   BDNumCsdOff(1);
#endif /* ARAB */
     if (!national_lang_mode)
        fs_eng_lang();
     else
        fs_national_lang();
  if (BIDI_ON)
  {
     BDDisableKeys(-1);
     BDDisableAutoPush(-1);
  }
}

static OutBoundryCheck()
{
        if (in_push_boundry_mode)
        {
                in_push_boundry_mode = FALSE;
                /*  CHANGE THE CURSOR SIZE  */
                insert_mode = saved_insert_mode_status;/* restore flags */
        }
}

static InBoundryCheck()
{
        if ((!in_push_boundry_mode) && (cursorposition == push_start_loc))
        {
                in_push_boundry_mode = TRUE;
                /*  CHANGE THE CURSOR SIZE  */
                /* save flags */
                saved_insert_mode_status = insert_mode;
                insert_mode          = TRUE;     /* insert input mode */
        }
}



fs_check_to_end_push(func)
register   int   (*func)();
{
        if ( (func == fs_clear) ||
             (func == fs_tab) ||
             (func == fs_backtab) ||
             (func == fs_return) ||
             (func == fs_up) ||
             (func == fs_down) ||
             (func == fs_home) ||
             (func == fs_eraseinput) ||
             (func == fs_eraseeof) ||
             (func == fs_reset) ||
             (func == fs_enter) ||
             (func == fs_pa1) ||
             (func == fs_pa2) ||
             (func == fs_pa3) ||
             (func == fs_penselect) ||
             (func == fs_pf1) ||
             (func == fs_pf2) ||
             (func == fs_pf3) ||
             (func == fs_pf4) ||
             (func == fs_pf5) ||
             (func == fs_pf6) ||
             (func == fs_pf7) ||
             (func == fs_pf8) ||
             (func == fs_pf9) ||
             (func == fs_pf10) ||
             (func == fs_pf11) ||
             (func == fs_pf12) ||
             (func == fs_pf13) ||
             (func == fs_pf14) ||
             (func == fs_pf15) ||
             (func == fs_pf16) ||
             (func == fs_pf17) ||
             (func == fs_pf18) ||
             (func == fs_pf19) ||
             (func == fs_pf20) ||
             (func == fs_pf21) ||
             (func == fs_pf22) ||
             (func == fs_pf23) ||
             (func == fs_pf24) ||
             (func == fs_wordnext) ||
             (func == fs_wordprev) ||
             (func == fs_fldrev) )
                fs_endpush(FALSE);
}
#endif /* BIDI */

#ifdef ARAB
/* to toggle autoshaping on and off */
void fs_autoshape()
  {
/*TN_LOG((dumpfno,"ARAB:in autoshape\n"));*/
    if (ASD_ON)
        {
          ASD_ON = FALSE;
          SHAPE  = ISOLATED;
        }
    else
        {
          ASD_ON = TRUE;
          SHAPE  = AUTO;
        }
  }

/****************************************/
/* to set isolated shaping on */
void fs_isolated()
  {
/*TN_LOG((dumpfno,"ARAB:in isolated shape\n"));*/
    SHAPE  = ISOLATED;
    ASD_ON = FALSE;
  }

/****************************************/
/* to set initial shaping on */
void fs_initial()
  {
/*TN_LOG((dumpfno,"ARAB:in initial shape\n"));*/
    SHAPE  = INITIAL;
    ASD_ON = FALSE;
  }
/****************************************/
/* to set middle shaping on */
void fs_middle()
  {
/*TN_LOG((dumpfno,"ARAB:in middle shape\n"));*/
    SHAPE  = MIDDLE;
    ASD_ON = FALSE;
  }
/****************************************/
/* to set final shaping on */
void fs_final()
  {
TN_LOG((dumpfno,"ARAB:in final shape\n"));
    SHAPE  = FINAL;
    ASD_ON = FALSE;
  }
/***************************************************************/
/* fs_fieldshape : to shape the current field :                */
/* Pseudocode :                                                */
/*   if attribute byte or protected field return               */
/*   find field start and field end.                           */
/*   find line start and line end.                             */
/*   take the larger of line start and field start.            */
/*   take the smaller of field end and line end                */
/*   move string into temporary array                          */
/*   convert to ASCII                                          */
/*   call LayoutObjectTransform                                 */
/*   handle lam alef on two cells                              */
/*   convert to EBCDIC                                         */
/*   put string back into screen buffer                        */
/*   tag line as modified                                      */
/*   refresh                                                   */
/***************************************************************/

void fs_fieldshape()
  {

    struct screenchar *pcurrent;  /* pointer to current character */
    struct screenchar *fieldstart;/* pointer to start of field   */
    struct screenchar *fieldend;  /* pointer to end of field     */
    struct screenchar *linestart; /* pointer to start of line    */
    struct screenchar *lineend;   /* pointer to end of line      */
    struct screenchar *segstart;  /* pointer to start of segment */
    struct screenchar *segend;    /* pointer to end of segment   */
    struct screenchar *screenend; /* pointer to end of screen    */
    struct screenchar *tempp;     /* temporary pointer           */
    char linebuff[MAXLINELENGTH]; /* line buffer                 */
    char tempbuff[MAXLINELENGTH]; /*temprary line buffer         */
    size_t/*char*/ *inputbuffer, *outputbuffer;
    unsigned int *inputsize, *outputsize;
    unsigned int *toinputbuffer,*tooutputbuffer;
    unsigned char *bidilevel;
    int  i,counter;               /* counter                     */
    int row,col;                  /* row and column              */
    static int last_len = 0;          /* length of atts array */
    unsigned short *buf_atts;  /* atts array */
    int size_of_str;                  /* size of atts array */
    int increment;                /* increment for shaping       */
    int  rc;                       /* return code from shaping */
    int  y,z;                      /*counters*/
    LayoutTextDescriptor value_description;
    LayoutValues  layout;
    int           *err;
    int           x;


  /* check to see if ARABIc enabled or not */
  if (!ARABIC_ON)  return;

   /* initialize pcurrent */
   pcurrent = sbufp;

   /*   if attribute byte or protected field return               */
   if ((pcurrent->attr & ATTR_TOP) || (pcurrent->attr & ATTR_PROT))
    return;

   /*   find field start and field end.                           */
   screenend = screen + linelength * lines; /* find the end of the screen */
   fieldstart = pcurrent;
   while (!(fieldstart->attr & ATTR_TOP)) /* find field attribute byte */
    {
      fieldstart--;
    }
   fieldstart++;                 /* point to first character after attribute */
   fieldend = pcurrent;
   while (!(fieldend->attr & ATTR_TOP) && (fieldend <=screenend))
      /*find attribute byte of next field or screen end*/
      fieldend++;

   fieldend--;                 /* point to last character before next field*/


   /*   find line start and line end.  */
   row = (pcurrent-screen) / linelength;
   linestart = screen + (row*linelength);
   lineend   = linestart + linelength-1;

   /*   take the larger of line start and field start. */
   if (linestart>fieldstart)
      segstart=linestart;
   else
      segstart=fieldstart;

   /*   take the smaller of fieldend and lineen  */
  if (lineend<fieldend)
      segend=lineend;
   else
      segend=fieldend;

TN_LOG((dumpfno,"ARAB:after determining segment start and end \n  screen=%d  screenend=%d  current=%d  segstart=%d  segend=%d\n",screen,screenend,pcurrent,segstart,segend));

TN_LOG((dumpfno,"ARAB:after determining segment start and end \n  screen=%d  screenend=%d  current=%d  segstart=%d  segend=%d\n",screen,screenend,pcurrent,segstart,segend));
   /*   move string into temporary array  */
    memset(linebuff,NULL,MAXLINELENGTH);
    tempp=segstart;
    counter = 0 ;
    while (tempp<=segend)
    {
      linebuff[counter]=tempp->ch;
      counter++;
      tempp++;
     }
   x = strlen(linebuff);
TN_LOG((dumpfno,"string length of linebuff = %d \n",x));
   if (field_rev)
      {
       TN_LOG((dumpfno,"fs_fieldshape: field_rev is true\n"));
      }
   /*   convert to ASCII  */
    for (i=0;i<=counter;i++)
      fs_translate(EBCTOASC,&(linebuff[i]));
TN_LOG((dumpfno,"ARAB:after translation : linebuffer=%s\n",linebuff));

  inputsize=malloc(sizeof(int));
  outputsize=malloc(sizeof(int));
  inputbuffer=malloc(MAXLINELENGTH*sizeof(char));
  outputbuffer=malloc(MAXLINELENGTH*sizeof(char));
  memset(inputbuffer,NULL,MAXLINELENGTH);
  memcpy(inputbuffer,linebuff,MAXLINELENGTH);
  memcpy(outputbuffer,linebuff,MAXLINELENGTH);

TN_LOG((dumpfno,"inputbuffer=%s,outputbuffer=%s\n",inputbuffer,outputbuffer));

 value_description = (LayoutTextDescriptor)malloc(sizeof(LayoutTextDescriptor));
  layout = (LayoutValues)malloc(2*sizeof(LayoutValues));
  if (!screen_reversed)
     {
      value_description->in  =ORIENTATION_LTR|TEXT_VISUAL|TEXT_STANDARD;
      value_description->out =ORIENTATION_LTR|TEXT_VISUAL|TEXT_SPECIAL;
     }
   else
     {
TN_LOG((dumpfno,"fs_fieldshape : screen is reversed\n"));
      value_description->in=ORIENTATION_RTL|TEXT_VISUAL|TEXT_STANDARD;
      value_description->out=ORIENTATION_RTL|TEXT_VISUAL|TEXT_SPECIAL;
     }
  layout[0].name =Orientation|TypeOfText|ArabicSpecialShaping;
  layout[0].value = (caddr_t)value_description;
  layout[1].name = 0; /*end of data*/
  *inputsize = strlen(linebuff);
  *outputsize =*inputsize;
  rc = layout_object_setvalue(plh,layout,err);
  free(layout);
  free(value_description);
  if (rc != 0)
     TN_LOG((dumpfno,"ERROR in procedure fieldshape,setting bidi attributes,                    rc =%d \n",rc));

TN_LOG((dumpfno,"ARAB : before  layout_object_transform :  inputbuffer = %s  inputsize= %d   outputbuffer= %s  outputsize= %d\n",inputbuffer,*inputsize,outputbuffer,*outputsize));


  rc=layout_object_transform(plh,inputbuffer,inputsize,outputbuffer,outputsize,NULL,NULL,NULL);

TN_LOG((dumpfno,"ARAB:rc=%d\n",rc));
TN_LOG((dumpfno,"ARAB : after  layout_object_transform :  inputbuffer = %s    inputsize= %d   outputbuffer= %s  outputsize= %d\n",inputbuffer,*inputsize,outputbuffer,*outputsize));

     memset(linebuff,NULL,MAXLINELENGTH);
     memcpy(linebuff,outputbuffer,counter);

TN_LOG((dumpfno,"fs_fieldshape: linebuffer after shaping = %s \n",linebuff));

     /* convert lam alef on two cells to one cell + space */

   /* convert to EBCDIC */
    for (i=0;i<=counter;i++)
      fs_translate(ASCTOEBC,&(linebuff[i]));

   /* put string back into screen buffer */
    tempp=segstart;
    i =0;
    while (tempp<=segend)
    {
      tempp->ch=linebuff[i];
      i++;
      tempp++;
    }


   /* tag line as modified */
    modified_lines[row]=TRUE;


   flip_modified_rows ();
  /*  set_bufaddr(cursorposition,TRUE);*/

   free(inputbuffer);
   free(outputbuffer);
   free(inputsize);
   free(outputsize);
   free(tempbuff);
   /*fs_redraw();*/
   (sbufp + sbufp->offset)->attr |= ATTR_MDT;

  }


/***************************************************************/
/* fs_fieldbase  : to deshape the current field :              */
/* Pseudocode :                                                */
/*   if attribute byte or protected field return               */
/*   find field start and field end.                           */
/*   find line start and line end.                             */
/*   take the larger of line start and field start.            */
/*   take the smaller of field end and line end                */
/*   move string into temporary array                          */
/*   convert to base shapes                                    */
/*   put string back into screen buffer                        */
/*   tag line as modified                                      */
/*   not sure whether needs refresing or not                   */
/***************************************************************/

void fs_fieldbase()
  {

    struct screenchar *pcurrent;  /* pointer to current character */
    struct screenchar *fieldstart;/* pointer to start of field   */
    struct screenchar *fieldend;  /* pointer to end of field     */
    struct screenchar *linestart; /* pointer to start of line    */
    struct screenchar *lineend;   /* pointer to end of line      */
    struct screenchar *segstart;  /* pointer to start of segment */
    struct screenchar *segend;    /* pointer to end of segment   */
    struct screenchar *screenend; /* pointer to end of screen    */
    struct screenchar *tempp;     /* temporary pointer           */
    char linebuff[MAXLINELENGTH]; /* line buffer                 */
    int  i,counter;               /* counter                     */
    int row,col;                  /* row and column              */



  /* check to see if ARABIC enabled or not */
  if (!ARABIC_ON)  return;

   /* initialize pcurrent */
   pcurrent = sbufp;

TN_LOG((dumpfno,"ARAB:in fs_fieldbase ATTR_TOP %d PROT %d\n",pcurrent->attr & ATTR_TOP,pcurrent->attr & ATTR_PROT));

   /*   if attribute byte or protected field return               */
   if ((pcurrent->attr & ATTR_TOP) || (pcurrent->attr & ATTR_PROT))
    return;

TN_LOG((dumpfno,"ARAB:in fs_fieldbase continue \n"));

   /* find field start and field end. */
   screenend = screen + linelength * lines; /* find the end of the screen */
   fieldstart = pcurrent;
   while (!(fieldstart->attr & ATTR_TOP)) /* find field attribute byte */
      fieldstart--;
   fieldstart++;                 /* point to first character after attribute */
   fieldend = pcurrent;
   while (!(fieldend->attr & ATTR_TOP) && (fieldend <=screenend))
      /*find attribute byte of next field or screen end*/
      fieldend++;

   fieldend--;                 /* point to last character before next field*/


   /*   find line start and line end. */
   row = (pcurrent-screen) / linelength;
   linestart = screen + (row*linelength);
   lineend   = linestart + linelength-1;

   /*   take the larger of line start and field start. */
   if (linestart>fieldstart)
      segstart=linestart;
   else
      segstart=fieldstart;

   /*   take the smaller of fieldend and lineend.*/
   if (lineend<fieldend)
      segend=lineend;
   else
      segend=fieldend;

TN_LOG((dumpfno,"ARAB:after determining segment start and end \n screen=%d screenend=%d current=%d segstart=%d segend=%d\n\n",screen,screenend,pcurrent,segstart,segend));

   /*   move string into temporary array  */
    memset(linebuff,NULL,MAXLINELENGTH);
    tempp=segstart;
    counter = 0 ;
    while (tempp<=segend)
    {
      linebuff[counter]=tempp->ch;
      counter++;
      tempp++;
     }

   /*   convert to base shapes */
    for (i=0;i<=counter;i++)
      fs_translate(TOISOLATED,&(linebuff[i]));

TN_LOG((dumpfno,"ARAB:after translation :linebuffer=%c%c%c%c%c%c%c\n",linebuff[0],linebuff[1],linebuff[2],linebuff[3],linebuff[4],linebuff[5],linebuff[6]));

   /*   put string back into screen buffer  */
    tempp=segstart;
    i =0;
    while (tempp<=segend)
    {
      tempp->ch=linebuff[i];
      i++;
      tempp++;
    }

   /*   tag line as modified  */
    modified_lines[row]=TRUE;

         flip_modified_rows ();
         (sbufp + sbufp->offset)->attr |= ATTR_MDT;
  }
void fs_translate(effect,ch)
int             effect;
unsigned char   *ch;
  {

/*TN_LOG((dumpfno,"ARAB:in fs_translate,effect = %d ",effect ));*/

  /* check to see if ARABIc enabled or not */
  if (!ARABIC_ON)  return;

    switch (effect) {
    case TOINITIAL:                         /* initial */
    *ch = initialmap[*ch];
     TN_LOG((dumpfno,"ARAB:in fs_translate,converting to initial\n"));
       break;
    case TOMIDDLE:                          /* middle */
    *ch = middlemap[*ch];
     TN_LOG((dumpfno,"ARAB:in fs_translate,converting to middle\n"));
       break;
    case TOFINAL:                           /* final */
    *ch = finalmap[*ch];
     TN_LOG((dumpfno,"ARAB:in fs_translate,converting to final\n"));
       break;
    case TOISOLATED:                        /* isolated */
    *ch = isolatedmap[*ch];
     TN_LOG((dumpfno,"ARAB:in fs_translte,converting to isolated\n"));
       break;
    case ASCTOEBC:                          /* ASCII to EBCDIC */
    *ch = asc_ebc[*ch];
     TN_LOG((dumpfno,"ARAB:in fs_translate,converting ASCII to EBCDIC\n"));
       break;
    case EBCTOASC:                          /* EBCDIC to ASCII */
    *ch = ebc_asc[*ch];
     TN_LOG((dumpfno,"ARAB:in fs_translate ,converting EBCDIC to ASCII\n"));
       break;
    case TONUMSWAP:                            /* SWAP Numerics   */
    *ch = numswapmap[*ch];
       break;
    case TOCHRREV:                          /* SYMMETRIC SWAP characters  */
    *ch = chrrevmap[*ch];
       break;
    default:                              /* None of the above */
       break;
    } /* endswitch */
  }
void putneighbours(pcurrent,arbchar,pre,post)
struct screenchar *pcurrent;        /* current character */
char *arbchar;                      /* arrayof characters */
int pre,post;                       /* number of characters before and after
                                      the current character  */

  {
    int i,j;                    /* counter */
    struct screenchar *temp;    /* temporary pointer to the array */

    /* put back current char, which is the fourth in the array of chars */
    pcurrent->ch = arbchar[3];

    /* put back the predecessors , there are up to 3 */
    temp = pcurrent-1;
    i = 2;
    for (j=pre; j>0; j--)
      {
        temp->ch = arbchar[i];
        temp--;
        i--;
      }

    /* put back the successors, there are up to 3 */
    temp = pcurrent+1;
    i = 4;
    for (j=post; j>0; j--)
      {
        temp->ch = arbchar[i];
        temp++;
        i++;
      }

  }

/* try to find 3 predecessors and 3 successors for pcurrent */
void getneighbours(pcurrent,arbchar,pre,post)
struct screenchar *pcurrent; /* current character */
char *arbchar;               /* array to save neighbours */
int *pre,*post;              /* return number of pre and post found */
  {
     struct screenchar  *screenend;   /* point to end of screen buffer */
     struct screenchar *temp;         /* temporary buffer pointer */
     int nomore;                      /* to indicate if more chars found */
     int i;                           /* counter */


     screenend = screen + linelength * lines; /* find the end of the screen */
TN_LOG((dumpfno,"screen end = %c\n",screenend->ch));

    /* initialize array with nulls */
    memset(arbchar,NULL,8);

    /* place the current character in the fourth position of the array */
TN_LOG((dumpfno,"getnei : current char = %x\n",pcurrent->ch));
    arbchar[3] = pcurrent->ch;

    /* try to find the predecessors */
    *pre = 0;
    temp = pcurrent-1;
TN_LOG((dumpfno,"trying to locate pres : 1st temp = %x\n",temp->ch));
    nomore = FALSE;
    i = 2;
    while (!nomore)
      {
if (!(temp->attr & ATTR_TOP))
TN_LOG((dumpfno,"attr cond true\n"));
if (temp>=screen)
TN_LOG((dumpfno,"screen cond true\n"));

         /* if not Attribute and not start of screen and not yet 3 found */
         if (!(temp->attr & ATTR_TOP) && (temp>=screen) && ((*pre)<3))
            {
TN_LOG((dumpfno,"in if stat\n"));
             (*pre)++;
             arbchar[i] = temp->ch;
TN_LOG((dumpfno," getnei:finding previous characters , pre = %d char = %c  arbchar = %s\n",*pre,temp->ch,arbchar));
             temp--;
             i--;
            }
          else nomore = TRUE;
      }
    /* try to find the successors */
    *post = 0;
    temp = pcurrent+1;
    nomore = FALSE;
    i = 4;
    while (!nomore)
      {
          /* if not Attribute and not end of screen and not yet 3 found */
         if (!(temp->attr & ATTR_TOP)
                          /* && !(temp->attr & ATTR_PROT) && (temp->ch) */
             && (temp<=screenend) && ((*post)<3))
            {
             (*post)++;
             if (temp->ch)
                  arbchar[i] = temp->ch;
           /*  else arbchar[i] = 64;  */
TN_LOG((dumpfno," getnei:finding next characters , post = %d char = %c  arbchar=%s\n",*post,temp->ch,arbchar));
             temp++;
             i++;
            }
          else nomore = TRUE;
      }

}

/*****************************************************************************/
 /* to handle the shape of a character -pointed to by pcurrent- and its      */
 /* surrounding characters in RTL and LTR screens                            */
 /* Pseudocode :                                                             */
 /*     check to see if shaping needed or not, if not return                 */
 /*     get current character and the 3 preceding and the three succeeding   */
 /*     translate characters to ASCII for shaping                            */
 /*     call the shaping API                                                 */
 /*     handle lam alef on two cells                                         */
 /*     translate characters back to EBCDIC                                  */
 /*     put the characters back in the screen buffer                         */
 /*     tag this line as modified                                            */
 /*     refresh                                                              */
/*****************************************************************************/

void fs_shape(pcurrent,handle)
struct screenchar *pcurrent; /* pointer to current character */
unsigned char handle;
 {
     int            do_shape = FALSE;  /* to shape or not to shape */
     static char   *arbchar,*arbtempbuff;  /*buffer of characters to be shaped*/
     int            i;                     /* loop counter */
     int            pre,post,temp;     /* number of characters around current*/
     int            rc;                /* return code from shaping */
     size_t/*unsigned long*/  effect;            /*,increment; needed for API call */

     size_t/*char*/           *inputbuffer;
     int            inputsize,outputsize;
     char           *outputbuffer;
     unsigned long  *index;
     struct screenchar *linestart; /* pointer to start of line    */
     int x,z;
     int stln,FLAG,startpos,endpos;
     LayoutValues layout;
     LayoutTextDescriptor value_description;
     int   *err;

TN_LOG((dumpfno,"ARAB:in fs_shape ARABIC %d\n",ARABIC_ON));
  /* check to see if ARABIC enabled or not */
  if (!ARABIC_ON) return;

  /* check to see if shaping needed or not, if not return */
     if ( ASD_ON )   /* if auto shaping enabled */
       {                  /* if LTR screen and push on and field reverse off */
         if (( !screen_reversed && push_mode && !field_rev) ||
                          /* if LTR screen and push off and field reverse on */
             ( !screen_reversed && !push_mode && field_rev) ||
                          /* or RTL screen and push off and field reverse off*/             ( screen_reversed && !push_mode && !field_rev) ||
                           /* or RTL screen and push on and field reverse on */
             ( screen_reversed && push_mode && field_rev)   ||
                      /* or RTL screen, push on, field reverse on, insert on */
             ( screen_reversed && !push_mode && field_push))
                  do_shape = TRUE;
        }

TN_LOG((dumpfno,"ARAB:in fs_shape do shape %d\n",do_shape));
     if (!do_shape)   return;

     if (!arbchar)
     {
        arbchar = (char *) malloc (9 * sizeof (char));
        memset (arbchar,0,9);
     }

    /* get current character and the three preceding and the three succeeding */
      getneighbours(pcurrent,arbchar,&pre,&post);

    /* translate characters to ASCII for shaping */
      for (i=(3-pre); i<=(3+post); i++)
           fs_translate(EBCTOASC,&(arbchar[i]));

TN_LOG((dumpfno,"before Edit : setting input: arbchar = %c\n%c\n%c\n%c\n%c\n%c\n%c\n",arbchar[0],arbchar[1],arbchar[2],arbchar[3],arbchar[4],arbchar[5],arbchar[6]));

TN_LOG((dumpfno," pre= %d  post= %d \n",pre,post));
      outputbuffer=malloc(9*sizeof(char));
      memset(outputbuffer,NULL,9);
      effect = EDITREPLACE;  /*edit mode */
      if (!field_rev) 
       {
        inputbuffer = &arbchar[3-pre];
        temp = pre;       /*to reserve the value of pre*/
        index = &temp;
       }
      else
        {
          inputbuffer = &arbchar[3];
          *index=0; 
         }
      outputsize =  pre+post+1;
TN_LOG((dumpfno,"fs_shape : temp =%d\n",temp));
      value_description=(LayoutTextDescriptor)malloc(sizeof(LayoutTextDescriptor));
      layout = (LayoutValues)malloc(2*sizeof(LayoutValues));
      if (!screen_reversed)
       {
         value_description->in=ORIENTATION_LTR|TEXT_VISUAL/*|TEXT_STANDARD*/;
         value_description->out=ORIENTATION_RTL|TEXT_VISUAL|TEXT_SPECIAL;
       }
     else
       {
         value_description->in=ORIENTATION_RTL|TEXT_VISUAL/*|TEXT_STANDARD*/;
         value_description->out=ORIENTATION_LTR|TEXT_VISUAL|TEXT_SPECIAL;
       }

      layout[0].name = Orientation|TypeOfText|ArabicSpecialShaping;
      layout[0].value = (caddr_t)value_description;
      layout[1].name = 0; /*end of data*/
      inputsize = -1;
      rc = layout_object_setvalue(plh,layout,err);
      free(value_description);
      free(layout);
  if (rc != 0)
     TN_LOG((dumpfno,"ERROR in procedure fs_shape, setting bidi attrributes"));

TN_LOG((dumpfno,"before Edit \n inputbuffer=%s index=%d inputsize=%d  outputbuffer= %s  outputsize= %d\n",inputbuffer,*index,inputsize,outputbuffer,outputsize));
       rc = layout_object_editshape(plh,effect,index,inputbuffer,&inputsize,outputbuffer,&outputsize);

TN_LOG((dumpfno,"after Edit \n inputbuffer=%s index=%d inputsize=%d  outputbuffer=%s  outputsize = %d\n",inputbuffer,*index,inputsize,outputbuffer,outputsize));
        if ((*index==0) && (!field_rev))            /*outputbuffer has same                                                         length of inputbuffer*/ 
            {
            TN_LOG((dumpfno,"!field_rev && index=0\n"));
            memcpy(&(arbchar[3-pre]),outputbuffer,outputsize);
            }

          /*outputbuffer is a character shorter than inputbuffer*/
          if ((*index==1) && (!field_rev)) 
           {
            TN_LOG((dumpfno,"!field_rev && index=1\n"));
            memcpy(&(arbchar[0]),inputbuffer,1);
            memcpy(&(arbchar[1]),outputbuffer,outputsize);
           } 
          if (field_rev) 
             {
              TN_LOG((dumpfno,"field_rev \n"));
              memcpy(&(arbchar[3]),outputbuffer,outputsize);
             }
TN_LOG((dumpfno,"after memcpy= %c\n%c\n%c\n%c\n%c\n%c\n%c\n ",arbchar[0],arbchar[1],arbchar[2],arbchar[3],arbchar[4],arbchar[5],arbchar[6]));


TN_LOG((dumpfno,"ARAB:before translate 2\n chars=%c\n%c\n%c\n%c\n%c\n%c\n%c pre%d post %d\n",arbchar[0],arbchar[1],arbchar[2],arbchar[3],arbchar[4],arbchar[5],arbchar[6],pre,post));
     /* translate characters back to EBCDIC */
     for (i=(3-pre); i<=(3+post); i++)
           fs_translate(ASCTOEBC,&(arbchar[i]));

     /* put the characters back in the screen buffer */
        putneighbours(pcurrent,arbchar,pre,post);

     /* tag this line as modified */
        modified_lines[(pcurrent-screen)/linelength] = TRUE;

        flip_modified_rows ();
  }


/* When new character is typed in, it must be checked    */
/*   for symmetric swapping and numeric swapping and for */
/*   special shape selection                             */
void fs_convertchar(ch)
char *ch;
  {
TN_LOG((dumpfno,"ARAB:in fs_convertchar ARABIC %d ch %d\n",ARABIC_ON,ch));

  /* check to see if ARABIC enabled or not */
  if (!ARABIC_ON)  {
       return;
  }

    fs_translate(ASCTOEBC,ch);
    if (screen_reversed)        /*if on a RTL screen */
      {
        if (SYMSWAP_ON)             /* check symmetric swapping */
           fs_translate(TOCHRREV,ch);
        if (NUMSWAP_ON)              /* check numeric swapping */
           fs_translate(TONUMSWAP,ch);
       }
    switch (SHAPE)
    {
      case INITIAL :
           fs_translate(TOINITIAL,ch);
           break;
      case MIDDLE  :
           fs_translate(TOMIDDLE,ch);
           break;
      case FINAL   :
           fs_translate(TOFINAL,ch);
           break;
      case ISOLATED:
            fs_translate(TOISOLATED,ch);
           break;
      default :
           break;
    }
   fs_translate(EBCTOASC,ch);
}


#ifndef _POWER
void fs_setcursorshape(flag)
int      flag;       /* if it is TRUE then turn cursor on  */
{

 char opflag=0;
 struct termio io;              /* place to hold ioctl data */
 struct hfcursorx hfcursorx = {    /* struct used to change cursor shape  */
        HFINTROESC,HFINTROLBR,HFINTROEX,0,0,0,sizeof(struct hfcursorx)-3,
                HFCURSORH, HFCURSORL, 2,0, 0,0};

         /*  set cursor shape if not on a tty */
                hfcursorx.hf_shape = flag;

         /*  turn off output processing so tty won't look at data.  */

            ioctl(0, TCGETA, &io);
        if ((io.c_oflag & OPOST) == OPOST) {
                io.c_oflag &= ~OPOST;
                
                 /*  set a flag if OPOST turned off */
                    opflag = 1;
            }
        
         /*  send request to hft */
            ioctl(0, TCSETA, &io);
            write(1, &hfcursorx, sizeof(hfcursorx));
            if (opflag) {
                
                 /*  restore tty flags */
                    io.c_oflag |= OPOST;
                    ioctl(0, TCSETA, &io);
            }

        return;

} /* End of cursor_shape() */
#endif /* _POWER */

/* The following procedure is used to write the actual screen buffer to the 
    screen when some characters on the screen are changed incorrectly*/
void fs_draw_screen(direction)
int direction;
{

register struct screenchar  *sp,*last_in_row;
register int                invisible,row;
register int                rowww,col;
static   char               tmpc;
static   int                cur_loc;

if (direction==LTR)
 {
   for (row = 0; row < lines; row++)
    {
        sp = screen + row * linelength;
        for (cur_loc = 0; cur_loc < linelength; cur_loc++)
        {
            /* display the character and its attribute */
            invisible = set_attribute(sp->attr,0);
            ADDR_TO_RC(sp - screen, rowww, col);
            move(rowww, col);
            tmpc =   convert_char(cd1, sp->ch);
            if ((tmpc == NULL) || (invisible))
              addch(' ');
            else
                addch(tmpc);

            sp++;
        }
    }
    move(cursorposition / linelength, cursorposition % linelength);
 }
else
 {
      for (row = 0; row < (lines); row++)
       {
         sp = screen + row * linelength;
         last_in_row  = sp + (linelength - 1);
         for (cur_loc = 0; cur_loc < linelength; cur_loc++)
          {
            /* must shift over the character and its attribute*/
            invisible = set_attribute(sp->attr,0); 
            ADDR_TO_RC((last_in_row - sp), rowww, col);
            move(row, col);
            tmpc=sp->ch;
#ifdef ARAB
            if (SYMSWAP_ON)      /*symmetric swap the characters*/
               fs_translate(TOCHRREV,&tmpc);
            if (NUMSWAP_ON)     /*numeric swap the characters*/
               fs_translate(TONUMSWAP,&tmpc);
#endif /* ARAB */
            tmpc = convert_char(cd1, tmpc);
            if ((tmpc == NULL) || (invisible))
               addch(' ');
            else
               addch(tmpc);
            sp++;
           }
         }
 }
 col = linelength - (cursorposition % linelength);
 move((cursorposition / linelength), col - 1);   /* (80-column) - 1 */

}
#endif /* ARAB */
