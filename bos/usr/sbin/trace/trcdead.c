static char sccsid[] = "@(#)16  1.7.2.4  src/bos/usr/sbin/trace/trcdead.c, cmdtrace, bos411, 9428A410j 5/31/94 16:10:43";

/*
 * COMPONENT_NAME: CMDTRACE   system trace logging and reporting facility
 *
 * FUNCTIONS: trcdead
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

#include <stdio.h>
#include <sys/trchdr.h>
#include "trace.h"

#define ISBETWEEN(x,a,b) ( (a) <= (x) && (x) <= (b) )

#define DMPFMT  "/usr/lib/ras/dmprtns/dmpfmt"

extern char *optarg;
extern optind;
extern verboseflg;

static FILE *Dlogfp;
static char *Dlogfile;
static char *Dumpfile;

static char Trctyp_[10] = "trctyp_@";
static char BufferA[10] = "bufferA@";
static char BufferB[10] = "bufferB@";

struct trctyp trctyp;

trcdead(argc,argv)
char *argv[];
{
        int c;
        int channel;
        char cmd[512];
        FILE *fp;

        channel = 0;
        while((c = getopt(argc,argv,"1234567o:vDH")) != EOF) {
                switch(c) {
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                        channel = c - '0';
                        break;
                case 'o':
                        Dlogfile = optarg;
                        break;
                case 'v':
                        verboseflg++;
                        break;
                case 'D':
                        if(debuginit("Btrace") < 0)
                                exit(1);
                        break;
                case 'H':
                case '?':
                       cat_eprint(M(CAT_TRC_DEAD),"\
Usage:  trcdead [-o filename] dumpimage\n\
\n\
-o filename     Write trace entries to 'filename'.\n\
dumpimage       System dump image.\n\
\n\
A trace session must be in progress when the system dump\n\
is invoked.\n");

                        exit(2);
                }
        }
        if(optind == argc)
		{
                       cat_eprint(M(CAT_TRC_DEAD),"\
Usage:  trcdead [-o filename] dumpimage\n\
\n\
-o filename     Write trace entries to 'filename'.\n\
dumpimage       System dump image.\n\
\n\
A trace session must be in progress when the system dump\n\
is invoked.\n");
                exit(1);
		}
        Dumpfile = argv[optind];
        if(Dlogfile) {
                if(streq(Dlogfile,"-")) {
                        Dlogfp = stdout;
                }
        } else {
                if(channel) {
                        Dlogfile = jalloc(strlen(LOGFILE_DFLT) + 8);
                        sprintf(Dlogfile,"%s.%d",LOGFILE_DFLT,channel);
                } else {
                        Dlogfile = LOGFILE_DFLT;
                }
        }
        if(Dlogfp == 0 && (Dlogfp = fopen(Dlogfile,"w+")) == 0) {
                perror(Dlogfile);
                exit(1);
        }
        if(access(Dumpfile,004)) {
                perror(Dumpfile);
                exit(1);
        }
        Trctyp_[7] = '0' + channel;
        BufferA[7] = '0' + channel;
        BufferB[7] = '0' + channel;
        /*
         * read in the trctyp structure from the dump image
         */
        sprintf(cmd,"%s -x -C trace -A %s %s",DMPFMT,Trctyp_,Dumpfile);
        if((fp = popen(cmd,"r")) == NULL) {
                perror(cmd);
                exit(1);
        }
        if(fread(&trctyp,sizeof(trctyp),1,fp) != 1) {
                       cat_eprint(M(CAT_NO_TRACE),"\
No trace data available in this dumpimage,\n\
because the trace session was not active at the time of the dump.\n");
                exit(1);
        }
        pclose(fp);
        switch(trctyp.t_type) {
        case TRCT_A:
                xfer(BufferB);
                xfer(BufferA);
                break;
        case TRCT_B:
                xfer(BufferA);
                xfer(BufferB);
                break;
        }
        exit(0);
}

static xfer(bufname)
char *bufname;
{
        int c;
        char *tmpfile;
        FILE *tmpfp;
        char cmd[512];

        tmpfile = tmpnam(0);
        sprintf(cmd,"%s -x -C trace -A %s %s > %s",DMPFMT,bufname,Dumpfile,tmpfile);
        if(shell(cmd)) {
                Debug("error from DMPFMT '%s'\n",cmd);
                exit(1);
        }
        if((tmpfp = fopen(tmpfile,"r")) == 0) {
                perror(tmpfile);
                exit(1);
        }
        unlink(tmpfile);
        while(EOF != (c = getc(tmpfp)))
                putc(c,Dlogfp);
        fclose(tmpfp);
}

