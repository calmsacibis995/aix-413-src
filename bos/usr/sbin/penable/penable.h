/* @(#)68	1.13  src/bos/usr/sbin/penable/penable.h, cmdoper, bos411, 9428A410j 1/13/94 08:28:20 */
/*
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Codes for name program is run under , ie action to take */
#define START    0			/* pstart    */
#define ENABLE  01			/* penable   */
#define DISABLE 02			/* pdisable  */
#define HOLD    04			/* phold     */
#define SHARE  010			/* pshare    */
#define DELAY  020			/* pdelay    */

#define AVAIL  252		/* device is available */
#define UNAVAIL 253		/* device is defined, but not available */
#define UNDEF  254		/* device is not defined */
#define BAD    255	/* error code for bad name or end of attribute file */
			/* 255 because it could be stored into a character */
#define UNSELECTED 0
#define SELECTED   1
#define FOUND      2

#define MAXREC  512
#define MAXATR  32

#define	MAXLLEN 2048		/* max inittab cmd line length (see init.h) */
#define	MAXACT 	20		/* max action length in inittab (see init.h) */
#define	MAXCMDL	1024		/* max command length in inittab (see init.h) */

#define MAXGETTYS 1024		/* max number of gettys running on a system */

/*
 * init responds to certain signals in different ways
 *      S_enable ...... tells init to reexamine enabled ports
 *      S_reboot ...... tells init to kill everyone and come up single
 *      SIGTERM  ...... equivalent to S_reboot
 *      SIGALRM  ...... used for timeouts and reenabling ports
 */
#define S_enable    SIGHUP  	/* use symbolic signal because various */
                            	/* systems use different signals */
FILE *fpinit;			/* FILE pointer for /etc/inittab */
char linebuf[MAXLLEN];		/* inittab line bufffer */
char *itfn = "/etc/inittab";  	/* inittab file name */
char *itfn2 = "/etc/inittabXXXXXX";  	/* temp inittab file name */
#define	ITFN2SZ	20		/* size of itfn2 string + \0 + extra */
#define GETTY "/usr/sbin/getty "     /* command running on normal port    */
char request;  			/* holds request code base on program name */
char mytty[20] = "console";     /* name of my controlling tty */

static char devpfx[] = "/dev/";  /* prefix for device name */
#define PFXLEN 5
static char ttypfx[] = "tty";   /* prefix for device number */
#define TTYLEN 3

typedef char bool;
bool allflag  = FALSE;          /* "all" argument specified */

struct devlist
{
	struct devlist *dl_next;
	short          dl_slct;    	/* this device selected */
	char           dl_name[PFXLEN+14+1];
};
struct devlist *fstdev, *lstdev;

struct atrlist
{
	struct atrlist *al_next;
	struct ATTR     al_attr;
};
struct atrlist *lstatr = NULL;

/*
 * penable's tty definition
 */
struct tty_entry {
    char *ty_name;              /* terminal device name */
    int ty_status;              /* status flags (see above for defines) */
    int ty_config;		/* configuration level of device */
};

#ifdef _NO_PROTO
extern  void  exit();
extern  char *strncpy(), *ttyname();
extern  char *malloc();
extern  void free();
struct devlist *adddev();
char *undev();
int getname();
int isgetty(); 
int getcmd();
int getact();
int stanname();
char *gettyline();
void killgetty();
char whatflg();
int updact();
int isportact();
char get_request();
int lookup ();
char 	get_request();
#else /* ~ _NO_PROTO */

extern void exit(int);
extern char *strncpy(char *, const char *, size_t);
extern char *ttyname(int);
extern void *malloc(size_t);
extern void free(void *ptr);
struct devlist *adddev(char *,short);
char *undev(char *);
int getname(char *);
int isgetty(char *); 
int getcmd(char *, char *);
int getact(char *, char *);
int stanname(char *,char *);
char *gettyline(char, char *);
void killgetty(char *);
char whatflg(char *);
int updact(char *);
int isportact(char *);
char get_request(char *);
int lookup (char *);
char 	get_request(char *);
#endif /* _NO_PROTO */
