static char sccsid[] = "@(#)02	1.8.1.3  src/tcpip/usr/samples/tcpip/onhost/hostcon2.c, tcpip, tcpip411, GOLD410 2/17/94 10:13:23";
/*
 * COMPONENT_NAME: TCPIP hostcon2.c
 *
 * FUNCTIONS: docmssock, findprofile, makefile, opendebug, opensockfile,
 *            srch, bump, GetLine, getalias, usage, getargs, getstdin,
 *            Strcmp, findkeyword, openldsf, makeconnection 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1986, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/*
	    NOTICE TO USERS OF THE SOURCE CODE EXAMPLES

 INTERNATIONAL BUSINESS MACHINES CORPORATION PROVIDES THE SOURCE CODE
 EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS, "AS IS" WITHOUT
 WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT
 LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE
 OF THE SOURCE CODE EXAMPLES, BOTH INDIVIDUALLY AND AS ONE OR MORE GROUPS,
 IS WITH YOU.  SHOULD ANY PART OF THE SOURCE CODE EXAMPLES PROVE
 DEFECTIVE, YOU (AND NOT IBM OR AN AUTHORIZED DEALER) ASSUME THE ENTIRE 
 COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.

*/
/* Background part and miscellaneous fns.
 */
/* 
static char AIXwhatC[] = "hostcon2.c	1.21 PASC 1.21";
 */
 
#include <errno.h>
#include <sys/types.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
 
#ifdef	BSD42LIB
#include <strings.h>
#include <sys/file.h>
#endif	BSD42LIB
 
#ifdef	SYSVLIB
#include <string.h>
#include <fcntl.h>
#ifndef AIXV4
#define index strchr
#define rindex strrchr
#endif 
#endif	SYSVLIB
 
#ifdef	TNOLD
#include <bsd/sys/time.h>
#else
#include <sys/time.h>
#endif

#define EXTERN extern
extern char version[];
#include "hostcon0.h"
#include "onhost0.h"
 
extern nl_catd catd;
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
struct sockaddr_in sin;
struct timeval gtod;
 
#ifdef	SYSVLIB
#define u_short ushort
#define u_long ulong
#endif	SYSVLIB
 
int uid, euid;
 
extern hosttype hostsys;
unsigned profsize;
#define spcmdsize  256
#define VMmaxline  130
char *spcmd[spcmdsize];
int spcmdkwd0;
char	echo[256];
int	echoed;  /* supress echo when vm rewrites input line to screen */
 
char argf_file[256];	/* data from arg f file */
 
char findkwbuf[81];
 
#define Sockaddr struct	sockaddr_in
Sockaddr	sname;
 
#define SMALL 32
 
char aliasfile[] = "onhost.alias";
char alias[SMALL];
char last_alias[] = "onhost.last";
char sockp0[] = "noalias";
char *sockpath;
char iplcmd0[] = "ipl cms";  /* type 0 system ipl default */
char *item;
char *iam;
int style;
 
char InetAddr[SMALL];
u_short LocalPort;
char Auth[SMALL];
int authen = 0;  /* 0 = onhost connection not yet authenticated */
char wuser[SMALL];
char wpass[SMALL];
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> docmssock will make a socket and read onhost.alias file, does  *
 *      accept and waits for onhost to connect, closes connection to *
 *      onhost.  Creates hostcon.alias (or .noalias) and onhost.last *
 *      so onhost can find socket.                                   *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
docmssock(what)
oncmsConnect what;
{
	/* make or break the connection to onhost */
	/*  the user writes to cms.stdin and we read from localread */
	/*  we write to localwrite and user reads from cms.stdout   */
	int rc;
 
	FILE *fds;
	char *s;
	int on = 1;
	int saclen;
	char tmpc256[256];
	int	af = AF_INET;
	struct	hostent *hp;
 
	rc = 0;
	switch(what) {
	case(oncmsMake):
		/* create a socket and leave it where onhost can find it */
		if ( (listensock = socket(af,SOCK_STREAM,0)) < 0) {
			perror("socket() for onhost socket");
			rc = 1;
		}
		/* gethostid does not seem to work so ... */
		if (gethostname(tmpc256,sizeof(tmpc256))) {
			perror("gethostname() for local system");
			rc = 2;
		}
		if (0 == (hp = gethostbyname(tmpc256)) ) {
			perror("gethostbyname() for local system");
			rc = 2;
		}
		if (rc == 0) {
		  sname.sin_family = AF_INET;
		  bcopy(hp->h_addr, (char *) &sname.sin_addr.s_addr,hp->h_length);
		  if (sname.sin_addr.s_addr == -1)
			rc = 3;
		}
		if (rc == 0) {
		  sname.sin_port =  0; /* system picks the port */
		  if (bind(listensock,(struct sockaddr *) &sname ,sizeof(sname),0)<0) {
			perror("bind() for onhost socket");
			rc = 4;
		  }
		  else {
			saclen = sizeof(sname);
			if (getsockname(listensock,(caddr_t) &sname ,&saclen,0)<0) {
				perror("getsockname() for onhost socket");
				rc = 5;
			} else {
			LocalPort = ntohs(sname.sin_port);
			(void) strcpy (InetAddr,inet_ntoa(sname.sin_addr));
			debug2((stddbg,"InetAddr =%s LocalPort =%d\n",InetAddr,LocalPort));
			}
		  }
		}
		if (rc == 0) {
			if (makefile() )
				rc = 9;
		}
		if ( rc == 0)
			if ( listen(listensock,1) < 0) {
				perror("listen() for onhost socket");
				rc = 7;
			}
		if ( rc != 0)
		  debug((stddbg,"can't make or save onhost socket, rc=%d\n",rc));
		debug2((stddbg,"socket for onhost saved in %s, rc=%d\n",sockfile,rc));
 
		if (0 != (s = getenv ("HOME"))) {  /* create file so onhost can find alias */
			(void) strcpy(argf_file,s);
		} else
			(void) strcpy(argf_file,".");
		(void) strcat(argf_file,"/");
		(void) strcat(argf_file,last_alias);
		if (0 == (fds=fopen(argf_file,"w"))) {
			sprintf(tmpc256, catgets(catd,HOSTCON2_ERR,HC_SYS_WRITE,
					"could not write %s"), argf_file);
			perror(tmpc256);
			rc = 8;
		} else {
			fprintf(fds,"%s\n",sockpath);
			fclose(fds);
		}
		if (rc != 0) {
			fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_HOST_ACCESS,
				"The host with alias \"%s\" will not be accessed\n"),alias);
			return(rc);
		}
		break;
	case(oncmsOpen):
		saclen = sizeof(sname);
		localread = accept(listensock,(struct sockaddr *) &sname,&saclen);
		if (localread < 0) {
			perror("accept() for onhost socket");
			rc++;
		} else {
			localwrite = localread;
			authen = 0;
			(void) strcpy (InetAddr,inet_ntoa(sname.sin_addr));
			debug2((stddbg,"InetAddr of client is %s\n",InetAddr));
		}
#ifdef	FIONBIO
	 	(void) ioctl(localread, (int) FIONBIO, (char *) &on);
		debug2((stddbg,"oncmsOpen: localread FIONBIO\n"));
#endif	FIONBIO
		opendebug();
		break;
	case(oncmsClose):
		if (localread >= 0)
			if( close(localread) )
				 rc++;
		localread = -1;
		debug((stddbg,"docmssock: close requested\n"));
		if (debugvar) {
			if (stddbg != stderr )
				(void) fflush(stddbg);
		}
		break;
	case(oncmsUnmake):
		if ( sockfile[0] != '\0' ) {
			if (unlink(sockfile))
				rc++;
			sockfile[0] = '\0';
		}
		if (0 != (s = getenv ("HOME"))) {
			(void) strcpy(argf_file,s);
			(void) strcat(argf_file,"/");
			(void) strcat(argf_file,last_alias);
		}
		(void) unlink(argf_file);
		break;
	}
	return(rc);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> findprofile reads onhost.profil and points spcmd[] to lines    *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
findprofile()
{
	/* find file onhost.profil and read it into profp
	 * set spcmd[n] to point to line n of profile
	 */
 
	register char *p, *q;
	char c, name[256], path[256];
	int j, n, psize, fd, gotfile;
	struct stat statbuf;
	char	*profp;
 
	fd = 0;
	profp = NULL;
	gotfile = 0;
	if (0 != (p = getenv("PATH"))) {
		(void) strncpy(path,p,sizeof(path));
		p = q = path;
		c = *p;
		while(c){ /* for all items in path */
			while( *q && (*q != ':') )
				q++;
			c = *q;
			*q = '\0';
			if (*p)
				(void) strcpy(name,p);
			else (void) strcpy(name,".");
			(void) strcat(name,"/onhost.profil");
			if ( (fd = open(name,O_RDONLY) ) > 0 ) { /* got file */
				if (fstat(fd,&statbuf) ) {
					fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_CANT_STAT,
						"could not stat %s\n"),name);
					goto ret;
				}
				gotfile++;
				profsize = (unsigned) (statbuf.st_size+1);
				profp = malloc( profsize);
				if (profp == NULL)
					fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_CANT_MALLOC,
						"findprofile: could not malloc\n"));
				break;
			} /* got file */
			p = ++q;
		} /* end for all items in path */
	}
	if (profp == NULL)
		goto ret;
	psize = (int) profsize;
	n = read(fd,profp,psize);
	if (n <= 0) {
		fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_CANT_READ,
			"could not read file %s\n"),name);
		n = 0;
	}
	*(profp+n) = '\0';
	spcmdkwd0 = n = 0;
	p = profp;
	/* check onhost.profil - the first numeric field after Version. */
	do {
		while ( *p && (*p != 'V'))
			p++;
		p++;
	} while ( *p && (0 != strncmp("ersion",p,6)));
	while ( *p && ( (*p < '0') || (*p > '9')))
		p++;
	if (*p)  {
		if (strncmp(version,p,strlen(version)))  {
			fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_PROF_VER,
				"onhost.profil Version is not %s, cannot continue\n"),version);
			*profp = '\0'; /* force an error */
		}
	}  else  {
		fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_PROF_FOUND,
			"onhost.profil Version not found, cannot continue\n"));
			*profp = '\0'; /* force an error */
	}
	p = profp;
	/* we expect key t data newline
	 * key is one word yyy is tab, data is one or more words
	 * {|} demarcates command items and keyword items
	 */
	while (*p) {
		if ( (*p == '{') && (*(p+1) == '|') && (*(p+2) == '}') )
			spcmdkwd0 = n;
		if (n < spcmdsize-1)
			spcmd[n++] = p;
		else {
			fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_SPCMD_FULL,
				"findprofile: spcmd table full, size=%d\n"),n);
			break;
		}
		if (*p == '#')
			n--;
		while ( (*p & 0377) > ' ') p++;
		*p++ = '\0'; /* mark end of key */
		while ( *p && (*p != '\n') )
			p++;
		if (*p)
			*p++ = '\0';
	}
	spcmd[n] = NULL;
	if (debugvar > 2)
		for(n=0;spcmd[n] != NULL;n++)
			fprintf(stddbg,"%d %s \n",n,spcmd[n]);
	if (spcmdkwd0) {
		spcmd[spcmdkwd0++] = NULL;
		j=spcmdkwd0;
		/* A typical keyword entry is
		 * SentPassword\0RECONNECTED AT,-,DMKLO,HCPLGN,HCPLOG;
		 * we make it (so keywordset has it easy)
		 * word \0 bb word \0 bb ... word \0 \0
		 * where bb is zero or more blanks
		 * we insisted on the ; so we can get fit in \0 \0
	 	 */
		for( ;spcmd[j];j++) {
			p = spcmd[j];
			while(*p++);
			n = 0;
			while (*p) {
				/* allow blanks after comma */
				while ( *p && (*p & 0377) <= ' ')
					*p++ = ' ';
				while (*p &&
				   (*p != ',') && (*p != ';') ) p++;
				if (*p == ';') {
					n++;
					*p = '\0';
				}
				if (*p == '\0') break;
				*p++ = '\0'; /* mark end of key */
			}
			if (n == 0) {
				printf(catgets(catd,HOSTCON2_ERR,HC_PROF_LINE,
				"error in onhost.profil. line beginning %s does not end in ;\n"),
					spcmd[j]);
				spcmdkwd0 = 0;
			}
		}
	}
ret:
	if (spcmdkwd0 && spcmd[0]) /* then we got what we wanted */
		return(0);
	if ( !gotfile )
		fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_PROF_PATH,
			"hostconnect cannot find onhost.profil in path %s\n"),path);
	else
		fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_PROF_FORMAT,
				"onhost.profil may be in wrong format.\n"));
	return(1); /* error exit */
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> makefile creates a file and writes some data                   *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
makefile()
{
	FILE	*ostream;
	char	buf[256];
 
	if ( (ostream = fopen(sockfile,"w")) == NULL) {
		(void) sprintf(buf,catgets(catd,HOSTCON2_ERR,HC_PROF_FORMAT,
			"could not write %s"),sockfile);
		perror(buf);
		return(1);
	}
	if (0 != gettimeofday (&gtod,0))  /* setup authentication string */
		perror(catgets(catd,HOSTCON2_ERR,HC_ONHOST_AUTH,
			"gettimeofday() can't provide onhost authentication"));
	sprintf(Auth,"%ld",gtod.tv_usec);
	Auth[6] = '\0';
	fprintf(ostream,"%s %d %s\n",InetAddr, LocalPort, Auth);
	fclose(ostream);
	(void) chmod(sockfile,00500);
	return(0);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> opendebug setsup up debug file                                 *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
opendebug()
{
	/* if debugvar == 0 then no debug print
	 * if debugvar == 1 then limited debug print to stderr
	 * if debugvar > 1 then debug print to file
	 * if debugvar > 2 then detailed debug print to file
	*/
	if (debugvar > 1) {
		if (stddbg != stderr )
			(void) fflush(stddbg);
		else
			if ((stddbg = fopen("hostconn.debug","w")) == NULL) {
				stddbg = stderr;
				fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_OPEN_DEBUG,
					"can not open hostconn.debug\n"));
			} else
				(void) chmod("hostconn.debug",0600);
	}
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> opensockfile opens hostcon.alias (or noalias) for socket info  *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
opensockfile()
{
	char *p;
 
	if (sockpath[0] != '/') {
		if (0 != (p = getenv("HOME")))
			(void) strcpy(sockfile,p);
		else (void) strcpy(sockfile,".");
		(void) strcat(sockfile,"/hostcon.");
#ifdef	SHORT
		sockpath[6] = '\0';  /* limit file name (hostcon.xxxxxx) to 14 chars */
#endif	SHORT
		(void) strcat(sockfile,sockpath);
	}
	else (void) strcpy(sockfile,sockpath);
	if ( (open(sockfile,O_RDONLY) ) > 0 ) {
		fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_ALREADY_CONN,
			"The host with alias %s may already be connected.\n"),alias);
		fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_ENTER_CMD,
			"Enter onhost -a %s [command] to try to use this host.\n"),alias);
		return(1);
	}
	return(0);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> srch searches cmdbuf for the special command keywords found in *
 *      onhost.profil and substitutes text if line is a cmd, and     *
 *      and supresses host echo produced when VM redisplays input    *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
srch(cmdbuf,cmd)
char *cmdbuf;
int cmd;
{
	/* see if cmdbuf has a command which is in spcmd
	 * spcmd entries have form xxx yyy
	 * in cmdbuf, replace xxx by yyy
	 * if command is pvs or fvs or ... then
	 * return 0 but write a special msg
	 */
	int j, n;
	register char *p, *q, *r,  *s, c;
	char line[256];
 
	q = p = 0;
	n = strlen(cmdbuf);
	if (VMmaxline < n )  {  /* limit VM line */
		cmdbuf[VMmaxline]='\0';
		n = strlen(cmdbuf);
	}
 
	r = 0;
	if ( cmd) {
		for(s=cmdbuf;*s == ' ';s++)
			;
		c = *s;
		p = spcmd[0];
		for ( j=1; p; p = spcmd[j++]) {
			/* if (c < *p) break;    assumes table is ordered */
			if (c == *p) {
				r = p;
				q = s;
				while (*p && (*++p == *++q) );
				if ( (*p == 0) &&
					( (*q == ' ') || (*q == '\n') ) )
					 break;
				r = 0;
			}
		}
		if ( r == 0) {  /* prepend default if spcmd not found */
			p = " onhost ";
			r = p;
			q = cmdbuf;
		}
	}
 
	if (r != 0) {
		line[0] = '\0';  /* copy (and substitute) from profil line */
		s = ++p;
		while (*s && (*++s != '$')) ;
		if (*s) {
			if ( strncmp(++s,"FTPAUPW",7) == 0) { /* $FTP addr user pass wdir */
				(void) strncat(line,p,s-p-1);
				p = s+7;
				(void) strcat(line,InetAddr);  /* nicer to select by letter but .. */
				if (strlen(wuser) == 0) {
					(void) strcat(line,wwdir);
				} else {
				(void) strcat(line," ");
				(void) strcat(line,wuser);
				(void) strcat(line," ");
				(void) strcat(line,wpass);
				(void) strcat(line,rindex(wwdir,' '));
				}
			} else {  /* copy through $ */
				(void) strncat(line,p,s-p);
				p = s;
			}
		}
 
		(void) strcat(line,p);    /* get rest of profil line */
		if (q[n=strlen(q)-1] == '\012')  /* drop trailing lf if present */
			q[n]='\0';
		s = q;
		(void) strncat(line,s,VMmaxline-strlen(line)); /* append user text */
		line[VMmaxline]='\0';  /* and limit VM line */
		(void) strcpy(cmdbuf,line);
		n = strlen(cmdbuf);
	}
 
	if (hostsys != hosttso) /* tso echo is offset when displayed */
		echo[0]='\0';
	else {
		echo[0]=' ';
		echo[1]='\0';
	}
	if (VMmaxline <= n)  {
		(void) strncat(echo,cmdbuf,VMmaxline); /* supress VM echo */
		echo[VMmaxline]='\0';
	}
	else
		(void) strcat(echo,cmdbuf);
	echoed = strlen(echo);
 
	return(n);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> bump along string to next word                                 *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
char *
bump(pp)
char	**pp;
{
	/* p points to begin of word, terminate word and find next word */
	register char	*p;
 
	p = *pp;
	while(*p && (*p != ' ') &&  (*p != '\t') && (*p != '\n') ) p++;
	if (*p)
		*p++ = '\0';
	while(*p && ( (*p == ' ') || (*p == '\t') || (*p == '\n') ) ) p++;
	*pp = p;
	if ( (*p == '-') || (*p == '\0') )
		return(NULL);
	else return(p);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> GetLine gets a line from a stream                              *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
GetLine (fds,line)
FILE *fds;
char *line;
{
	int n;
	char *p;
	int t;
 
	p = line;
	do {
		*p++ = t = fgetc (fds);
	} while ((t!=EOF) && (t!='\n'));
 
	*--p = '\0';
 
	n = p - line;
 
	if (t==EOF)
		return(EOF);
	else
		return(n);
}
 
char *
NextItem (line,b1)
char **line;
char b1;
{
	char *p, *start;
	int something;
 
	something = 0;
	p = start = *line;
	for (;;) {
 
		if ((*p=='\0') || (*p=='('))
			break;
		
		if ((*p==b1) || (*p=='\t'))
			if (something)
				break;
			else
				start++;
	
		something = 1;
		p++;
	}
 
	if ((*p!='\0') && (*p!='(')) {
		*p = '\0';
		*line = ++p;
	} else
		*line = p;
 
	if (something)
		return(start);
	else
		return(0);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> getalias reads the onhost.alias file, sets mode 600 if needed, *
 *     gets local workstation name and password, then looks up alias *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int getalias()
{
	FILE *fds;
	int Lines;
	int cnt, i;
	char *entry;
	char *INaddr;
	char *id;
	char *pwd;
	static char buf[80];
	char *b;
	char *s;
	struct stat stat_buf;
 
/* default everything in case onhost.alias is missing */
	(void) strcpy(wuser,"");
	(void) strcpy(wpass,"?");
	iplcmd = iplcmd0;  /* type 0 system default */
	style = STYLEdefault;
	id = pwd = INaddr = 0;
	sockpath = sockp0;
	(void) strcpy(argf_file,aliasfile);
	if (stat(argf_file,&stat_buf)) {
		if (0 != (s = getenv ("HOME"))) {
			(void) strcpy(argf_file,s);
			(void) strcat(argf_file,"/");
			(void) strcat(argf_file,aliasfile);
		}
		if (stat(argf_file,&stat_buf)) {
			fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_HOME_DIR,
				"%s not in current or home directory - "),aliasfile);
			fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_DEFAULTS_SET,
				"Default values have been set.\n"));
			return(0);  /* ignore missing alias file */
		}
	}
	i = stat_buf.st_mode & 0777;
	if (i!=0600)
		if (chmod(argf_file,0600))  {
			fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_600_SET,
				"%s should have 600 access, please chmod\n"),aliasfile,i);
		}
	if ((fds=fopen(argf_file,"r"))==NULL) {
		fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_CANT_OPEN,
			"can not open %s\n"),aliasfile);
		perror("because");
		fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_DEFAULTS_SET,
			"Default values have been set.\n"));
		return(0);  /* ignore problem */
	}
	cnt = GetLine (fds,buf);
	b = buf;
	/* 1st line of onhost.alias is [userid [passwd]] em3278-2 */
	if (0 != (id = NextItem(&b,' ')))
		if (0 != (pwd = NextItem(&b,' ')))
			if (NextItem(&b,' ')) { /* line is complete */
				(void) strcpy(wuser,id);
				(void) strcpy(wpass,pwd);
			} else /* line is userid em3278-2 */
				(void) strcpy(wuser,id);
		else {} /* line is em3278-2 */
	else {} /* line is void */
	if (!strcmp(alias,sockp0))  {  /* do not look for noalias alias */
		fclose(fds);
		return(0);
	}
	Lines = 1;
	while (cnt!=EOF)  {
		b = buf;
		Lines++;
		cnt = GetLine(fds,buf);
		if (0 != (entry = NextItem(&b,' ')))  {   /* alias name */
			if (strcmp(alias,entry))
				continue;
		}
		else
			continue;
		cnt = EOF;
		sockpath = alias;
		INaddr = NextItem(&b,' ');  /* internet address or ldsf */
		if (0 != (item = NextItem(&b,' ')))
			style = atoi(item);
		id = NextItem(&b,' ');  /* host userid */
		pwd = NextItem(&b,' ');  /* host password */
		item = NextItem(&b,'(');  /* move to left paren if there */
                if (*b=='(') b++;
		if (0 != (item = NextItem(&b,')')))  /* optional (ipl string) */
			iplcmd = item;  /* else clause could set default according to style */
			debug((stddbg,"alias %s is for style %d host %s %s %s iplcmd=%s.\n",
				alias,style,INaddr,id,pwd,iplcmd));
	}  /* end while reading alias file */
	fclose(fds);
	if (strcmp(sockpath,alias))  {
		printf(catgets(catd,HOSTCON2_ERR,HC_NO_ALIAS,
			"alias not found in %s\n"),argf_file);
		return(1);
	}
	if (INaddr)
		phost = INaddr;
	else  {
		printf(catgets(catd,HOSTCON2_ERR,HC_INTERNET,
			"internet address not set\n"));
		return(1);
	}
	vmid = id;
	vmpwd = pwd;
	return(0);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> usage prints a bit of information about hostconnect            *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void 
usage(iam,zip)
char *iam;
int zip;
{
	fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_USAGE,
		"(%s) usage: "),version);
	fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_PROMPT1,
		"%s [-?d[n][a alias] [ihost [style]]\n"),iam);
	if (zip == 0)
		return;
	fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_PROMPT2,
		" where a = use alias to identify host connection\n"));
	fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_PROMPT3,
		"   d, d0 = debug output to stddbg\n"));
	fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_PROMPT4,
		"  d1, d2 = debug output to hostconn.debug (d2: more detail)\n"));
	fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_PROMPT5,
		"   alias = identify entry in onhost.alias file which describes host\n"));
	fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_PROMPT6,
		"   ihost = Internet address or domain name of host\n"));
	fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_PROMPT7,
		"   style = 0 or 2 for CMS, 4 for TSO, 6 for an unknown host type;\n\t\t add one to bypass automatic login for connection only (then use\n\t\t onhost to login and execute onhostld)\n"));
	fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_PROMPT8,
		"will connect to the IBM host described by the alias entry in onhost.alias,\n"));
	fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_PROMPT9,
		"or, if the alias is noalias, to the host named ihost.\n"));
#ifdef	LDSF
	fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_PROMPT10,
		"(A special connection to a VM peer of AIX/370 will be made if ihost is LDSF.)\n"));
#endif	LDSF
 
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> getargs is called from main to check and record opts and args  *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
getargs(argc, argv0)
int argc;
char **argv0;
{
 
	register char *p;
	char **argv;
 
	uid = getuid();
	euid = geteuid();
	(void) setuid(uid);
	iam = argv0[0]; 
	(void) strcpy(alias,sockp0);  /* set default alias in case no -a */
	argv = argv0;
	while ( (--argc > 0) && ((*++argv)[0] == '-') ) {
		p = *argv;
		while (p && *++p) {
			if (*p == 'd') {
				char c;
 
				debugvar = 1;
				c = *(p+1);
				if ( (c >= '0') && (c <= '9') ) {
					debugvar += c - '0';
					p++;
				}
				opendebug();
			}
			else if (*p == 'a') {
				argc--;
				if (!argc || !strlen(strcpy(alias,*++argv))) {
					fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_EMPTY_ALIAS,
						"no alias following -a\n"));
					argc--;
				}
			}
			else if (*p == '?') {
				argc--;
				usage(iam,1);
				return(1);
				}
			else {
				fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_BAD_OPTION,
					"unrecognized option: %c\n"),*p);
				usage(iam,0);
				argc = -1;
				return(1);
			}
		}
	}
	if (getalias())
		return(1);
	if (!strcmp(alias,sockp0))  {  /* no alias set - same as -a noalias */
		if (argc-- > 0)
			phost = argv[0];  /* get hostname */
		else
			phost = 0;
		if (argc-- > 0)
			style = atoi(argv[1]);  /* allow style as second arg */
	}
	if STYLEnologin
		ConnectOnly = 1;
	if STYLEcms
		hostsys = hostcms;
	else if STYLEtso
		hostsys = hosttso;
	else if STYLEunk
		hostsys = hostunk;

	if ( hostsys == hostcms) {
		LastUserField = cmsLastUserField;
		UserEnterField = cmsUserEnterField;
		UserEnterLen = cmsUserEnterLen;
	} else {
		LastUserField = unkLastUserField;
		UserEnterField = unkUserEnterField;
		UserEnterLen = unkUserEnterLen;
	}
	if ( !phost) {
		usage(iam,0);
		return(1);
	}
	return(0);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> getstdin gets a line from stdin and optionally supresses echo  *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
getstdin(line,c)
char	*line, c;
{
	int tot;
 
	if (c == 'n')
		setecho(0);
	(void) fgets(line,256,stdin);
	tot = strlen(line);
	if (tot > 0)
		line[tot-1] = '\0';
	else line[0] = 0;
	if (c == 'n')
		setecho(1);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> Strcmp returns 0 if the strings are equal up to the end of     *
 *      the first string (which is about 8 characters long).         *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
#define CM 0xdf
 
int
Strcmp(a,b)
register char *a, *b;
{
	while (*a) {
		if ( (*a++ & CM) != (*b++ & CM) )
			return(1);
	}
	return(0);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> findkeyword searches buf for keywords                          *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
findkeyword(buf,len)
char *buf;
int  len;
{
	/* look for keywords in buf
	 * exit if keywordi >= 0
	 * if succesful, keywordindex is index of s in buf
	 * and keyword is keywords[keywordi]
	 */
	register char *p, *pmax, c;
	int j;
	char *s;
 
	if (keywordi >= 0)
		return;
	for(j = 0;;j++) {
		s = keywords[j];
		if ( (s == NULL) || ( (c = s[0]) == '\0') )
			break;
		if (c == '-')
			continue;
		p = buf;
		pmax = p + len -1;
		if (c == '\n') {
			if ( (p < pmax) && ( Strcmp(s+1,p) == 0) ){
				keywordindex = 0;
				debug((stddbg,"got keyword %s\n",s+1));
				keywordi= j;
				return; /* success */
			}
		}
		/* s points to keyword
		 * c is s[0]
		 * p points to buffer
		 * look for kewyword in buffer
		 */
		c = c & CM;
		while (p <= pmax) {
			while ( (p <= pmax) &&  ( (*p & CM) != c))
				p++;
			if (p < pmax) { /* found first letter */
				if ( Strcmp(s,p) == 0 ) {
					keywordindex = p-buf;
					debug((stddbg,"got keyword %s\n",s));
					keywordi= j;
					(void) strncpy(findkwbuf,p,sizeof(findkwbuf));
					findkwbuf[sizeof(findkwbuf)-1] = '\0';
					if (0 != (p = index(findkwbuf,'\n')) )
						*p = '\0';
					return; /* success */
				}
				p++; /* try again */
			} /* found first letter */
		} /* end while */
	} /* end for */
}
 
 
 
 
 
 
 
 
#ifdef	LDSF
char hexchars[] = "0123456789abcdef";
/*  123.. is room for sitename.  xx can be 00 - ff	*/
/* ldsfdev[] = "/1234567890abcde/dev/ldsfxx		*/
static char ldsfdev[32];
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> openldsf trys to open a /dev/ldsf00..ff  (AIX/370 & VM only)   *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
openldsf()
{
	struct stat sb;
	int fd0, ok, n, ix;
 
	ldsfdev[0] = '/';
	(void) strcpy(ldsfdev+1, sfsname(site(0)));
	(void) strcat(ldsfdev, "/dev/ldsf0");
	ix = strlen(ldsfdev) - 1;
	n = ok = 0;
	for (n = 0; !ok  && (n < 256) ; n++){ /* loop through all devs */		
		if ( n <= 15)
			ldsfdev[ix] = hexchars[n];
		else {
			ldsfdev[ix] = hexchars[n % 16];
			ldsfdev[ix+1] = hexchars[n/16];
			ldsfdev[ix+2] = '\0';
		}
		fd0 = open(ldsfdev,O_RDWR);
		if (fd0 < 0) {
			debug2((stddbg,"open error on %s errno=%d\n",ldsfdev,errno));
			if (errno == ENOENT) {
				if (n)
					printf(catgets(catd,HOSTCON2_ERR,HC_LDSF_USED,
					"All /dev/ldsf are in use: last dev tried was %s\n"),
					ldsfdev);
				else 
					printf(catgets(catd,HOSTCON2_ERR,HC_LOCAL_HOST,
						"no local host system, specify a remote hostname\n"));
				return(0);
			}
		}
		else if ( fstat(fd0,&sb) ) {
			debug2((stddbg,"stat error on %s errno=%d\n",ldsfdev,errno));
		}
		else if ( (sb.st_mode & S_IFMT) != S_IFCHR ) {
			debug2((stddbg,"%s is not a character device\n",ldsfdev));
		}
		else ok++;
		if (ok == 0 ) {
			if ( fd0 >= 0) {
				(void) close(fd0);
				printf(catgets(catd,HOSTCON2_ERR,HC_READ_ERR,
					"error reading %s\n"),ldsfdev);
				return(0);
			}
			if (errno == EACCES ) {
				printf(catgets(catd,HOSTCON2_ERR,HC_CANT_ACCESS,
					"cannot access %s\n"),ldsfdev);
				printf(catgets(catd,HOSTCON2_ERR,HC_IN_BIN,
					" Does bin own /dev/ldsf and hostconnect?\n"));
				printf(catgets(catd,HOSTCON2_ERR,HC_SUID_SET,
					" Does hostconnect have suid bit set?\n"));
				return(0);
			}
			else if ( errno != EEXIST) {
				printf(catgets(catd,HOSTCON2_ERR,HC_CHAR_DEV,
					"could not open %s as characters device\n"),ldsfdev);
				return(0);
			}
			/* else try next device */
		}
	}
	remotesock = fd0;
	isldsf++;
	debug2((stddbg,"using %s\n",ldsfdev));
	return(1);
}
#endif	LDSF
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> makeconnection connects to host using TCP/IP (or perhaps LDSF) *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
makeconnection(hostname)
char *hostname;
{
int telnetport;
 
	/* connect to host using TCP/IP (or LDSF of local)
	 * if succesful, set connected and define remotesock
	 */
	struct	servent *sp;
	register struct hostent *host;
	int on = 1;
	struct stat statbuf;
 
	debug((stddbg,"%s (hostconnect version %s) connecting to %s\n",iam,version,hostname));
 
#ifdef LDSF
	if ( strcmp(hostname,"LDSF") == 0) {
		if ( setuid(euid))  /* restore effective userid so we can use ldsf */
			debug((stddbg,"unable to restore euid %d from uid %d\n",euid,uid));
		connected = openldsf();
		if ( setuid(uid))  /* then restore userid */
			debug((stddbg,"unable to restore uid %d from euid %d\n",uid,euid));
		debug2((stddbg,"reset to uid=%d euid=%d\n",uid,euid));
		return;
	}
#endif	LDSF
	if ( (sp = getservbyname("telnet", "tcp") ) == 0) {
		fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_TCP_MISSING,
			"telnet/tcp not in /etc/services - using port 23\n"));
		telnetport = 23;
	}
	else
		telnetport = sp->s_port;
	host = gethostbyname(hostname);
	if (host) {
		sin.sin_family = host->h_addrtype;
		bcopy(host->h_addr, (caddr_t)&sin.sin_addr, host->h_length);
	} else {
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = inet_addr(hostname);
		if (sin.sin_addr.s_addr == -1) {
			fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_UNKNOWN_HOST,
				"%s: unknown host\n"), hostname);
			return;
		}
	}
	sin.sin_port = telnetport;
	errno = 0;
	(void) stat(sockfile,&statbuf);
#ifdef	AIXV3
	errno = 2;
#endif	AIXV3
	if (errno != ENOENT) {
		if (errno) {
			fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_CANT_ACCESS,
				"cannot access %s\n"),sockfile);
			perror("because");
		}
		else {
			fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_ALREADY_CONN,
				"The host with alias %s may already be connected.\n"),alias);
			fprintf(stderr,catgets(catd,HOSTCON2_ERR,HC_ENTER_CMD,
				"Enter onhost -a %s [command] to use this host.\n"),alias);
		}
		sockfile[0] = '\0';
		return;
	}
	errno = 0;
	remotesock = socket(AF_INET, SOCK_STREAM, 0);
	if (remotesock < 0) {
		perror("socket() for remote host");
		return;
	}
	if ( connect(remotesock, (struct sockaddr *) &sin, sizeof (sin)) < 0) {
		perror("connect() for remote host");
		return;
	}
	errno = 0;
#ifdef	FIONBIO
	(void) ioctl(remotesock, (int) FIONBIO, (char *) &on);
	debug2((stddbg,"FIONBIO to remote errno=%d\n",errno));
#endif	FIONBIO
	connected++;
}
