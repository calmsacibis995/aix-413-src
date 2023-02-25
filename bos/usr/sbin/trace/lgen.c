static char sccsid[] = "@(#)22	1.4  src/bos/usr/sbin/trace/lgen.c, cmdtrace, bos411, 9428A410j 6/11/91 09:22:49";

/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: lgen
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * This code is not part of any option of the shipped trace commands.
 * Therefore, this code will be removed in a later release under a cleanup defect.
 * For that reason, no messages from this file will be added to the 
 * cmdtrace message catalog and no ILS function will be added.
 */

#include <stdio.h>
#include <string.h>

#define T_S 0x00000000
#define T_L 0x00020000
#define T_G 0x00040000
#define T_V 0x00060000

#define jstrtok(a,b) (unsigned char *)(strtok(a,b))

extern char *optarg;

static int listflg;
static int Lineno;
static char *Infile;
static char *Outfile;
static FILE *Infp;
static FILE *Outfp;

static unsigned cvt();

static unsigned event[128];

lgen(argc,argv)
char *argv[];
{
	int i;
	int c;
	unsigned int n;
	char mode;
	unsigned char *cp;
	unsigned char *tcp;
	unsigned traceid;
	unsigned char line[128];

	while((c = getopt(argc,argv,"li:o:")) != EOF) {
		switch(c) {
		case 'l':
			listflg++;
			break;
		case 'i':
			Infile = optarg;
			break;
		case 'o':
			Outfile = optarg;
			break;
		}
	}
	if(Infile == NULL) {
		Infile = "stdin";
		Infp = stdin;
	} else {
		if((Infp = fopen(Infile,"r")) == NULL) {
			perror(Infile);
			exit(1);
		}
	}
	if(Outfile == NULL) {
		Outfile = "stdout";
		Outfp = stdout;
	} else {
		if((Outfp = fopen(Outfile,"w+")) == NULL) {
			perror(Outfile);
			exit(1);
		}
	}
	while(fgets(line,128,Infp)) {
		Lineno++;
		if(line[0] == '*')
			continue;
		/*
		 * get traceid
		 */
		cp = jstrtok(line," \t\n");
		if(cp == NULL)
			continue;
		traceid = strtol(cp,0,16);
		/*
		 * get mode
		 */
		cp = jstrtok(0," \t\n");
		if(strlen(cp) != 1)
			error("bad format for type: '%s'",cp);
		mode = cp[0];
		switch(mode) {
		case 's':
			event[0] = (traceid << 20) | T_S;
			putevent(1);
			continue;
		case 'l':
			cp = jstrtok(0," \t\n");
			event[0] = (traceid << 20) | T_L;
			event[1] = cvt(cp);
			putevent(2);
			continue;
		case 'g':
			event[0] = (traceid << 20) | T_G;
			for(i = 1; i <= 5; i++) {
				cp = jstrtok(0," \t\n");
				event[i] = cvt(cp);
			}
			putevent(6);
			continue;
		case 'v':
			cp = jstrtok(0," \t\n");
			n = strtol(cp,0,0);
			if(n == 0)
				error("bad format '%s' for trcgen vcount");
			event[0] = (traceid << 20) | n | T_V;
			event[1] = cvt(jstrtok(0," \t\n"));
			tcp = (unsigned char *)&event[2];
			for(i = 0; i < n; i++) {
				cp = jstrtok(0," \t\n");
				tcp[i] = cvt(cp);
			}
			n = (n + 3) & ~3;
			putevent(n/4 + 2);
			continue;
		default:
			error("bad mode %c",mode);
		}
	}
	exit(0);
}

static putevent(n)
{
	int i;

	if(listflg) {
		for(i = 0; i < n; i++)
			fprintf(Outfp,"%08x ",event[i]);
		fprintf(Outfp,"\n");
	} else {
		write(fileno(Outfp),event,n * 4);
	}
}

static unsigned cvt(str)
char *str;
{
	unsigned n;
	int negflg;

	negflg = 0;
	if(str == NULL)
		return(0);
	if(str[0] == '\'')
		return(str[1]);
	if(str[0] == '-') {
		negflg++;
		str++;
	}
	n = strtol(str,0,0);
	if(negflg)
		n = ~n + 1;
	return(n);
}

static error(s,a,b,c,d,e)
{

	eprint("error line %d: ",Lineno);
	eprint(s,a,b,c,d,e);
	eprint("\n");
	exit(1);
}

static eprint(s,a,b,c,d,e)
{

	fprintf(stderr,(const char *) s,a,b,c,d,e);
}

