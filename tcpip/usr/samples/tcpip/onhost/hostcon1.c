static char sccsid[] = "@(#)01	1.7  src/tcpip/usr/samples/tcpip/onhost/hostcon1.c, tcpip, tcpip411, GOLD410 2/13/94 14:56:41";
/*
 * COMPONENT_NAME: TCPIP hostcon1.c
 *
 * FUNCTIONS: dbgprcms, dbgiar, putiacbuf, putiactn, DoIac, dokeys,
 *            buryhostecho, init3270, cmstouserbuf, GetStatusArea,
 *            usertocmsbuf, keywordset, StartupCommands
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1986, 1988, 1989
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
 DEFECTIVE, YOU (AND NOT IBM OR AN AUTHORIZED RISC System/6000* WORKSTATION
 DEALER) ASSUME THE ENTIRE COST OF ALL NECESSARY SERVICING, REPAIR OR
 CORRECTION.

 * RISC System/6000 is a trademark of International Business Machines
   Corporation.
*/
/* Decode 3270 screens and control login.
 */
/*
static char AIXwhatC[] = "hostcon1.c	1.20 PASC 1.20";
 */
 
#include <stdio.h>
#include <sys/types.h>
 
#ifdef	BSD42LIB
#include <strings.h>
#include <sys/socket.h>
#endif	BSD42LIB
 
#ifdef	SYSVLIB
#ifndef AIXV4
#define index strchr
#endif 
#include <string.h>
#endif	SYSVLIB
 
#define EXTERN extern
#include "hostcon0.h"
#include "onhost0.h"
 
#define touserbuf (TOuserbuf+CShlen)
#define bfSIZE	(BFSIZE-CShlen)
#define TSOREAD ((hostsys == hosttso) && kbd && havecursor)
 
extern nl_catd catd;

extern char findkwbuf[];
extern char *spcmd[];
extern int spcmdkwd0;
extern int cmsretcode;
extern char echo[];
extern int echoed;
 
char *StageIs[] = {  /* must match UserStageType in hostcon0.h */
	"GotNothing", "GotReady", "GotRestart", "SentLogin",
	"SentPassword", "SentIpl", "SentExec", "WantToQuit",
	"GotNucx", "StartupFailed", "LostHost"
	};
char *StageBlip = "-CRLPIEQSFH";  /* StageIs blip characters */
 
char	*nullword = "";
 
/* After an SBA, Read3270 .. AltErase Screen are normal characters */
int markit[] = { Read3270, EraseScreen, AltEraseScreen,
	InsCursor, RepChar, SbaChar, StartChar, IAC , Esc, ReadBuffer};
int savemark[sizeof(markit)];
 
int	badnegotiation;
int	EptoA[256];
char	hisopts[256];
char	myopts[256];
char	StatusArea[StatusAreaLen],  OldStatusArea[9];
char	*Screenpmax = StatusArea+ sizeof(StatusArea) - 1;
char	userid[9] = "RUNNIN";  /* VM userid for RUNNING/userid test */
int	ScreenAddr = 0;
extern int kbd;
char	*lastchecked;
extern int tsomore;
extern int NDflag;
#define	NDtest	" OnHost warning: password will show"
extern hosttype hostsys;
UserStageType OldUserStage;
char TermType[] = "IBM-3278-2";
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> dbgprcms makes debug print of 3270 data stream                 *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
dbgprcms (a,m,s)
char a[], *s;
int m;
{
	/* write a[0..m-1] to stddbg converting host chars
	 * EptoA converts Ebcdic to Ascii
	 * and gives 0 for non-printable characters
	 */
	int j, k, e, puteol, n;
 
	fprintf(stddbg,s,m);
	for(k=1, j = 0, puteol = 0; (j<m)&&(j<2500); j++, k++) {  /* limit array print */
		n = byte(a[j]);
		e = EptoA[ n ];
		if ( e == '\n')
			puteol++;
		else if ( e )
			putc( (char) e, stddbg);
		else switch(n) {
		case InsCursor:
			fprintf(stddbg," IC ");
			break;
		case RepChar:
			fprintf(stddbg," REP %x,%x,%x",byte(a[++j]),byte(a[++j]),byte(a[++j]));
			break;
		case SbaChar:
			n = mod64(byte(a[++j]));
			n = (n << 6) | mod64(byte(a[++j]));
			fprintf(stddbg,"\nSBA %d,%d ", n/80, n%80);
			k = 1;
			break;
		case StartChar:
			fprintf(stddbg," SC 0x%x",byte(a[++j]));
			break;
		case EOR:
			fprintf(stddbg," EOR ");
			break;
		case IAC:
			fprintf(stddbg," IAC ");
			break;
		default:
			 fprintf(stddbg," 0x%x ",n);
			if ( k > 75)
				puteol++;
			k += 4;
		}
		if ( (k % 78) == 0)
			puteol++;
		if(puteol) {
			putc('\n',stddbg);
				k = 1;
			puteol = 0;
		}
	}
	putc('\n',stddbg);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> dbgiar makes debug print of telnet negotiations                *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
dbgiar(k)
int k;
{
	if ( k >= Tablow)
		fprintf(stddbg,iactab[k-Tablow]);
	else if ( k >= 24)
		fprintf(stddbg,iac24[k-24]);
	else fprintf(stddbg,iac0[k]);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> putiacbuf used by DoIac to output telnet negotiation chars     *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
putiacbuf(c1,c2,c3)
int c1, c2, c3;
{
	if (ntocms+3 > bfSIZE) {
		printf(catgets(catd,HOSTCON1_ERR,HC_PUTIAC_FULL,
			"buffer full in putiacbuf\n"));
		exit(99);
	}
	tocmsbuf[ntocms++] = (char) c1;
#ifdef	LDSF
	if (isldsf && (c2 == IAC) )
			return(0);
#endif	LDSF
	tocmsbuf[ntocms++] = (char) c2;
	tocmsbuf[ntocms++] = (char) c3;
	debug2((stddbg,"putiac %x %x %x\n",c1,c2,c3));
	return(0);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> putiactn used by DoIac to output telnet negotiation chars      *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
putiactn(c1,c2,c3)
int c1, c2, c3;
{
	TOuserbuf[ntouser++] = (char) c1;
	TOuserbuf[ntouser++] = (char) c2;
	TOuserbuf[ntouser++] = (char) c3;
/*	debug2((stddbg,"putiactn %x %x %x\n",c1,c2,c3)); */
	return(0);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> DoIac does telnet terminal type negotiation                    *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
char *
DoIac(k0,q0,qmax)
int k0;
char *q0, *qmax;
{
	register int k;
	register char *q;
	int choice;
	/* got input (other than EOR) which began IAC k0 */
 
	choice = 0;
	k = k0;
	q = q0;
	if (debugvar > 2) dbgiar(k);
	q0 = q0-2; /* this should point to the IAC */
	if ( k >= WILL) { /* will wont do dont */
		choice = k;
		if (q <= qmax) k = byte(*q++); else return(q0);
		if (debugvar > 2)  dbgiar(k);
	}
	if ( k != SB) {
		switch(choice) {
		case(DONT):
			if(myopts[k]) {
				putiacbuf(IAC,WONT,k);
				myopts[k] = '\0';
			}
			break;
		case(DO):
			if(!myopts[k]) {
				putiacbuf(IAC,WILL,k);
				myopts[k] = '\1';
			}
			break;
		case(WILL):
			if(!hisopts[k]) {
				putiacbuf(IAC,DO,k);
				hisopts[k] = '\1';
			}
			break;
		case(WONT):
			if(hisopts[k]) {
				putiacbuf(IAC,DONT,k);
				hisopts[k] = '\0';
			}
		}
	}
	else {
		if (q <= qmax) k = byte(*q++); else return(q0);
		if (debugvar > 2) dbgiar(k);
		if ( k != Terminal_type) {
			printf(catgets(catd,HOSTCON1_ERR,HC_TERM_TYPE,
				"cmscon3 expects terminal type\n"));
			exit(99);
		}
		if (q <= qmax) k = byte(*q++); else return(q0);
		if (k) { /* they want us to send terminal type */
			debug2((stddbg,"Send "));
			/* skip iac and se */
			if (q <= qmax) k = byte(*q++); else return(q0);
			if (q <= qmax) k = byte(*q++); else return(q0);
			putiacbuf(IAC,SB,Terminal_type);
			if (ntocms+3+strlen(TermType) > bfSIZE) {
				printf(catgets(catd,HOSTCON1_ERR,HC_DOIAC_FULL,
					"buffer full in DoIac\n"));
				exit(99);
			}
			tocmsbuf[++ntocms] = '\0'; /* IS */
			(void) strcpy(tocmsbuf+ntocms,TermType);
			ntocms += strlen(TermType);
			putiacbuf(IAC,SE,0);
			ntocms--; /* erase the 0 */
		}
		else { /* Is -- they tell us terminal type */
			printf(catgets(catd,HOSTCON1_ERR,HC_SEND_TERM,
				"We expect to send terminal type\n"));
			badnegotiation++;
		}
	}
	debug2((stddbg,"\n"));
	return(q);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> dokeys sends 3270 chars corresponding to a PF key              *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
dokeys(s)
char *s;
{
	int k, rc;
 
	k = 0;
	/* input is Ea or E1 .. E9 E0 E- E= where E = ESC or ^ */
	rc = 0;
	debug((stddbg,"got pfkey request for %c\n",s[1]));
	switch(s[1]) {
		case '0':
			k = PF10Key;
			break;
		case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			k = PF1Key + s[1] - '1';
			break;
		case '-':
			k = PF10Key +1;
			break;
		case '=':
			k = PF10Key +2;
			break;
		case 'a':
			k = PA1Key;
			break;
		case 'b':
			k = PA2Key;
			break;
		case 'c':
			k = PA3Key;
			break;
		case 'e':
			k = EnterKey;
			break;
		case 'z':
			k = ClearKey;
			break;
		default:
			rc = 1;
	}
	if (rc == 0)  {  /* clear pfk flag */
		tocmsbuf[ntocms++] = (char) k;
		tocmsbuf[ntocms++] = c3270[cursor >> 6];
		putiacbuf( (int) c3270[mod64(cursor)],IAC,EOR);
	}
	return(rc);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> buryhostecho watches the host output for the echo caused when  *
 *      VM redisplays the input line.  When found, it is buried.     *
 *                                                                   *
 *  *(p0-1)  will contain a newline character.  Look for previous    *
 *  newline then look for echo, which may be presented as several    *
 *  line segments following possible arbitrary initial output.       *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
char *
buryhostecho(p0)
char *p0;
{
	register char *p;
	int must;
 
	p = p0 - 2;
	while( ( p > touserbuf) && ( *p != '\n') ) p--;
	if ( *p == '\n')
		p++;
	lastchecked = p;
	if (2 < debugvar) {
		fprintf(stddbg,"checkline %d, ==>",(p0-p)-1);
		(void) fwrite(p,1,(p0-p)-1,stddbg);
		fprintf(stddbg,"<==\n");
		fprintf(stddbg,"echoed is %d, ==>%s<==\n",echoed,echo);
 
		if (4 < debugvar) {  /* loud debug */
			fprintf(stdout,"checkline %d, ==>",(p0-p)-1);
			(void) fwrite(p,1,(p0-p)-1,stdout);
			fprintf(stdout,"<==\n");
			fprintf(stdout,"echoed is %d, ==>%s<==\n",echoed,echo);
		}
	}
   if (0 == strncmp(lastchecked,NDtest,strlen(NDtest)))
		NDflag = -1;  /* suppress display of test and pwd */
   else {
	if ( echoed == 0)
		return( p0);
	if ( ( must = ( echoed < 0)) != 0)
		echoed = -echoed;
	if ( echoed <= 80) {
		if ( strncmp(echo,p,echoed) == 0) {
			echoed = 0;
		} else {
			if ( must) echoed = 0;
			return( p0);
		}
	} else {
		if ( strncmp(echo,p,80) == 0) {
			echoed = 80 - echoed;
		} else {
			if ( must) echoed = 0;
			return( p0);
		}
	}
   }
	debug2((stddbg,"buryhostecho: buried the following line\n%s\n",
		echo));
	if (p > touserbuf)
		p0 = p;
	else
		p0 = touserbuf;
	if ( echoed != 0)
		(void) strcpy(echo,echo+80);
	return( p0);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> init3270 initializes tables for 3270 options and ascii/ebcdic  *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
init3270()
{
	register int j, k;
	register char m;
 
	k = sizeof(hisopts) / sizeof(hisopts[0]);
	for(j=0; j < k ;j++)
		hisopts[j] ='\0';
	for(j=0; j < k ;j++)
		myopts[j] ='\0';
	/* EtoA[64] = 32 = Ascii blank so	*/
	/* AtoE[32] = 64 = Ebcdi blank		*/
	k = sizeof(AtoE) / sizeof(AtoE[0]);
	for (j=0; j < k ; j++)
		AtoE[ EtoA[j] ] = j;
	/* translate null to blank for full screen applications */
	EtoA[0] = 32;
	for (m = ' '; m < '~'; m++)
		EptoA[ AtoE[m] ] = m;
	/* mark characters we want to recognize */
	k = sizeof(markit) / sizeof(markit[0]);
	for (j=0; j < k ; j++) {
		savemark[j] = EtoA[markit[j]];
		EtoA[markit[j]] = -(j+1);
	}
	EtoA[Esc] = savemark[-(mEsc+1)];
	c3270[0] = 0x40;
	k = 1;
	/* see 3278 data stream for meaning of following */
	for(m=0xc1,j=1;j<=9 ;j++) c3270[k++] = m++;
	for(m=0x4a,j=1;j<=7 ;j++) c3270[k++] = m++;
	for(m=0xd1,j=1;j<=9 ;j++) c3270[k++] = m++;
	for(m=0x5a,j=1;j<=8 ;j++) c3270[k++] = m++;
	for(m=0xe2,j=1;j<=8 ;j++) c3270[k++] = m++;
	for(m=0x6a,j=1;j<=6 ;j++) c3270[k++] = m++;
	for(m=0xf0,j=1;j<=10;j++) c3270[k++] = m++;
	for(m=0x7a,j=1;j<=6 ;j++) c3270[k++] = m++;
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> cmstouserbuf converts 3270 data stream in fromcmsbuf to ascii, *
 *      strips off control chars, adds newlines, sets touserbuf      *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
cmstouserbuf(nin)
int nin;
{
	/* move characters fromcmsbuf --> touserbuf
	 * fromcmsbuf[0..nin-1] has data from host
	 * data in touserbuf[0.. ntouser-1] not yet sent to user
	 */
	register int screenaddr;
	register char *p, *q, *pmax, *qmax, *usermaxp;
	char *q1, *p1, devnull[cmsUserEnterLen+1], *devnullmax, *userfrontp;
	int k, n;
 
	n = 0;
	usermaxp = &touserbuf[bfSIZE-1];
	userfrontp = &touserbuf[ntouser]; /* first free char */
	if ( (usermaxp - userfrontp) < 20) {
		debug((stddbg,"buffer nearly full in cmstouserbuf\n"));
		return(nin);
	}
	devnullmax = devnull + UserEnterLen -1;
	screenaddr = ScreenAddr; /* make register copy */
	/* screenaddr is 0		to	LastUserfield
	 *		LastUserField+1	to	StatusAreaField-1
	 *		StatusAreaField	to	LastScreenField	
	 */
	if (screenaddr <= LastUserField) {
		p = userfrontp;
		pmax = usermaxp;
		n = ntouser; /* used for debug print only */
	}
	else if (screenaddr < StatusAreaField) {
		p = devnull;
		pmax = devnullmax;
		n = 0; /* used for debug print only */
	}
	else {
		p = StatusArea + (screenaddr-StatusAreaField);
		pmax = Screenpmax;
	}
	q = fromcmsbuf;
	qmax = &fromcmsbuf[nin-1];
	debug2((stddbg,"cmstouser nfrCMS=%d ntoUser=%d pmax=%d %d %d n=%d\n",
		nin,ntouser,pmax==usermaxp,pmax==devnullmax,pmax==Screenpmax,n));
	/* when connection is first made, all data from the host
	 * will be negotiations which will be handled by DoIac
	 * note StartUpCommands will not get called if DoIac
	 * has generated a response because ntocms will not be zero
	 */
 
	while  (q <= qmax) {
	/* debug2((stddbg,"q=%x,qmax=%x,c=%x,k=%d,touserb=%x,p=%x,pmax=%x,usermaxp=%x\n",
		q,qmax,*q,EtoA[byte(*q)],touserbuf,p,pmax,usermaxp)); */
 
		/* Store normal character or process flagged character, detect */
		/* end of screen fields, divert p and pmax to other buffers.   */
		/* Only characters stored in the output area flow to user.     */
		/* pmax determines state and detects screen wrap.              */
 
		if ( ( k = EtoA[byte( *q++)]) >=0 ) {
			if ( p <= pmax) {
				*p++ = (char) k;
				screenaddr++;
 
				if ( screenaddr == ( LastUserField + 1))
					goto changesba;
			}
			else {
			debug2((stddbg,"pmax==> %x %x %x %x %d %d\n",
				pmax,usermaxp,devnullmax,Screenpmax,screenaddr,StatusAreaField));
 
				q--;
				if (pmax == usermaxp)
					break;
				if (pmax == Screenpmax)
					screenaddr = 0;
				goto changesba;
			}
		}
		else switch( k) {
		case(mReadBuffer):  /* return clear screen to host */
			debug2((stddbg,"ReadBuffer\n"));
			p1 = tocmsbuf + ntocms;
			*p1++ = 0x60;	
			*p1++ = 0x40;	
			*p1++ = 0x40;	
			for(n=0;n<1920;n++)
				*p1++ = 0;	
			*p1++ = IAC;
			*p1++ = EOR;
			q++;		/* skip it */
			ntocms = p1 - tocmsbuf;
			break;
 
		case(mEraseScreen):
		case(mAltEraseScreen):
			if ( qmax < ( q + 1)) {
				q--; goto done;
			}
			if ( tsomore < 0) {
				tsomore = 0;
			}
			havecursor = kbd = screenaddr = 0;
			(void) strcpy(StatusArea," erased");
		case(mRead3270):
			if ( qmax < ( q + 1)) {
				q--; goto done;
			}
			kbd = (*q++) & 0x02;
			debug2((stddbg,"Write/Erase/Alt:kbd=%x ",kbd));
 
			if ( ( k == mRead3270) && ( *q == IAC) && ( *(q+1) == EOR)) {
				telluser = Running;
				q += 2;
			}
			break;
 
		case(mInsCursor):
			cursor = screenaddr;
			havecursor = 1;
			telluser = Running;
			debug((stddbg,"INS_CURS=%d,%d ", cursor/80,cursor%80));
			break;
 
		case(mRepChar):
			if ( (q+3) > qmax) {
				q--; goto done;
			}
			k = mod64(*q++);
			k = (k << 6) | mod64(*q++);
			n = byte(*q++);
			if ( n == GescChar )
				q++;;
			screenaddr = k;
			debug((stddbg,"REP_ %d,%d ", k/80,k%80));
			goto changesba;
 
		case(mSbaChar):
			if ( ( q + 1) > qmax) {
				q--; goto done;
			}
			k = mod64(*q++);
			k = (k << 6) | mod64( *q++);
			screenaddr = k;
			EtoA[markit[0]] = savemark[0];
			EtoA[markit[1]] = savemark[1];
			EtoA[markit[2]] = savemark[2];
	changesba:
			if (Running)
				debug((stddbg,"\nSBA=%d,%d ",screenaddr/80,screenaddr%80));
			if (pmax == usermaxp) {
				if ( (p > touserbuf)  && *(p-1) != '\n' ) {
					*p++ = '\n';
					if (Running)
						debug2((stddbg,"sba screenaddr=%d n=%d 0x%x 0x%x 0x%x 0x%x\n",
							screenaddr,p-touserbuf,*(p-4),*(p-3),*(p-2),*(p-1)));
					p = buryhostecho(p);
				}
				userfrontp = p;
			}
			debug2((stddbg,"\nSBA=%d,%d ",screenaddr/80,screenaddr%80));
			if (screenaddr <= LastUserField) {
				p = userfrontp;
				pmax = usermaxp;
			}
			else if (screenaddr < StatusAreaField) {
				p = devnull;
				pmax = devnullmax;
			}
			else {
				p = StatusArea + (screenaddr-StatusAreaField);
				pmax = Screenpmax;
				debug((stddbg,"=STATUS+%d %d ",screenaddr-StatusAreaField, qmax-q));
			}
			break;
 
		case(mStartChar):
			if (!NDflag)  /* make flag sticky */
				NDflag = (NDmask == (NDmask & *q));
			if  (screenaddr == LastUserField)
				goto nostartc;
 
			debug((stddbg,"StartChar:screenad=%d lastu=%d c=%x\n",
				screenaddr,LastUserField,*(p-1)));
 
			if ( q > qmax) {
	retry:
				q--; goto done;
			}
			if (p <= pmax)
				*p++ = ' ';
			else {
				if (screenaddr < LastUserField)
					goto retry;
			}
nostartc:		screenaddr++;
			q++;
			break;
 
		case(mIAC):
			if ( q > qmax) {
				q--; goto done;
			}
			k = byte(*q++);
			if ( k == EOR) {
				debug((stddbg,"EOR "));
				EtoA[markit[0]] = -1;
				EtoA[markit[1]] = -2;
				EtoA[markit[2]] = -3;
				if (pmax == usermaxp) {
					if ( (p > touserbuf)  && *(p-1) != '\n' ) {
				debug((stddbg,"EOR:screenad=%d lastu=%d c=%x\n",
					screenaddr,LastUserField,*(p-1)));
						*p++ = '\n';
						p = buryhostecho(p);
					}
				}
			}
			else {
				q1 = q;
				q = DoIac( k,q,qmax);
				if ( q < q1 )
					goto done;
			}
			break;
		}
	}
done:
	if (pmax != usermaxp)
		p = userfrontp;
	if ( (ntouser = p - touserbuf) > 0 ) {
		p--;
		while ( ( touserbuf <= p) && ( byte(*p) != '\n'))
			p--;
		nnltouser = p + 1 - touserbuf;
		if ( ( 6 <= nnltouser) && ( !tsomore) && ( hostsys != hostcms)) {
			tsomore = !strncmp(p-6,"\n *** \n",6);  /* pickup tso more... */
			if ( tsomore) {
				ntouser = nnltouser = nnltouser - 6;
			}
		}
	}
	else nnltouser = 0;
	qmax = &fromcmsbuf[nin-1];
	n = qmax+1 - q; /* number of characters not used */
	pmax = &fromcmsbuf[n-1];
	for (p=fromcmsbuf;p <= pmax;)
		*p++ = *q++;
	ScreenAddr = screenaddr; /* unmake register copy */
	GetStatusArea();
	debug2((stddbg,"cmstouser unused=%d ntouser=%d\n",n,ntouser));
	return(n);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> GetStatusArea                                                  *
 *                                                                   *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
GetStatusArea()
{
	register char *p, *pmax;
 
	pmax = StatusArea + sizeof(OldStatusArea) -1;
 
	debug2((stddbg,"\nStatusArea=%s=\n",StatusArea));
 
	for ( p = StatusArea; p <= pmax; p++) {
		OldStatusArea[p-StatusArea] = *p;
		}
	OldStatusArea[8] = '\0'; /* for debug and strcmp */
 
	debug((stddbg,"OldStatusArea=%s\n",OldStatusArea));
 
	screenwas = ScreenStatus;  /* for state change check */
	ScreenStatus = UndefinedScreen;
 
	if (hostsys != hosttso)   /* skip VM status tests if TSO */
	switch (OldStatusArea[1]) {
	case 'C':
		if (strcmp(OldStatusArea+1,"CP READ") == 0) {
			ScreenStatus = CPread;
			if (screenwas != ScreenStatus)
				telluser = Running;
		}
		break;
 
	case 'V':
		if (strcmp(OldStatusArea+1,"VM READ") == 0) {
			ScreenStatus = VMread;
			if (screenwas != ScreenStatus)
				telluser = Running;
		}
		break;
 
	case 'H':
		    if (strcmp(OldStatusArea+1,"HOLDING") == 0)
			goto needclear;
		break;
 
	case 'M':
		    if (strcmp(OldStatusArea+1,"MORE...") == 0) {
needclear:
			ScreenStatus = VMRunning;
			putiacbuf(ClearKey,IAC,EOR);
			havecursor = kbd = 0;
			}
		break;
 
	case 'N':
		if (strcmp(OldStatusArea+1,"NOT ACC") == 0)
			ScreenStatus = VMRunning;
		break;
 
	case ' ':
		if (strcmp(OldStatusArea+2,userid) == 0)
			ScreenStatus = VMRunning;
		break;
 
	case 'R':
		if (strcmp(OldStatusArea+1,"RUNNING") == 0)
			ScreenStatus = VMRunning;
		break;
 
	default:
		;
	}
	if (debugvar) {
		fprintf(stddbg,"ScreenStatus is ");
		switch (ScreenStatus) {
		case CPread:
			fprintf(stddbg,"CPread\n");
			break;
		case VMread:
			fprintf(stddbg,"VMread\n");
			break;
		case VMRunning:
			fprintf(stddbg,"VMrunning\n");
			break;
		case UndefinedScreen:
			fprintf(stddbg,"Undefined\n");
			break;
		}
	}
	if ( ( ScreenStatus == UndefinedScreen) && kbd && havecursor)
		if ( 0 < tsomore) {
			putiacbuf(EnterKey,IAC,EOR);
			havecursor = kbd = 0;
			tsomore = -1;
		}
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> usertocmsbuf converts fromuserbuf to ebcdic, adds 3270 control *
 *      chars to build 3270 data stream in tocmsbuf                  *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
usertocmsbuf(n)
int n;
{
	/*  move characters fromuserbuf --> tocmsbuf
	 * fromuserbuf[0..n-1] has user data to go to host
	 * tocmsbuf[0..ntocms-1] has data not yet sent to host
	*/
	register char *p,  *pmax, *cmsfrontp;
	register char *q,*qmax;
 
	cmsfrontp = tocmsbuf + ntocms;
	p = cmsfrontp;
	pmax = &tocmsbuf[bfSIZE-3]; /* -3 allows for IAC and EOR */
	debug2((stddbg,"usertocmsbuff: ntocms=%d n=%d pmax=%d\n",
		ntocms,n,pmax-tocmsbuf));
	q = fromuserbuf;
	qmax = &fromuserbuf[n-1];
	if (cursor != UserEnterField )
		debug2((stddbg,"cursor %d %d %d UserEnter %d %d %d\n",
			cursor,cursor/80,cursor%80,
			UserEnterField,UserEnterField/80,UserEnterField%80));
	*p++ =  EnterKey;
	if ( !((n == 1) && (*q == '\n')) ) {
		*p++ = c3270[(cursor+n) >> 6];
		*p++ = c3270[mod64(cursor+n)];
		*p++ =  SbaChar;
	}
	*p++ =  c3270[cursor >> 6];
	*p++ =  c3270[mod64(cursor)];
	if ( pmax > p + 130)
		pmax = p+130;
	while ( (p<=pmax) && (q<=qmax) )
		*p++ = AtoE[*q++];
	if (*(q-1) == '\n')
		p--;
 
#ifdef	LDSF
	if (isldsf == 0) {
		*p++ = IAC;
		*p++ = EOR;
	}
#else
	*p++ = IAC;
	*p++ = EOR;
#endif	LDSF
 
	kbd = havecursor = 0;
	/* move remaining chars to front of buffer */
	cmsfrontp = p;
	for(p=fromuserbuf;q<=qmax;)
			*p++ = *q++;
	ntocms = cmsfrontp - tocmsbuf;
	return(p-fromuserbuf);
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> keywordset called by StartupCommands to find keywords needed   *
 *      for next login stage.  If keywordi < 0 then findkeyword will *
 *      search.  If keywordi = 99 then findkeyword will not search.  *
 *    The entries from onhost.profil have been read, normalized, and *
 *      stored in spcmd beginning with index spcmdkwd0.  If an entry *
 *      matches the argument, nextstage, then the results are put in *
 *      the keywords array; else an error is reported.               *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
void
keywordset(nextstage)
UserStageType nextstage;
{
	register int j, jmax ;
	register char *p;
 
	UserStage = nextstage;
	keywordi  =  keywordbad = 99;
 
	/* set keywords to null for debug print */
	keywords[0] = keywords[1] = keywords[2] = keywords[3] = keywords[4] = nullword;
	if (UserStage == GotNothing) {
		OldUserStage = UserStage;
		return;
	}
	if ((UserStage == GotReady)
	 || (UserStage == GotRestart)
	 || (UserStage == GotNucx))
		return;
 
	for(j=spcmdkwd0; spcmd[j];j++){
		if ( strcmp(StageIs[(int) nextstage], spcmd[j]) == 0)
			break;
	}
	if ( (p = spcmd[j]) == NULL){
		printf(catgets(catd,HOSTCON1_ERR,HC_PROF_KEY,
			"%s keywords not found in onhost.profil\n"),StageIs[(int)nextstage]);
		return;
	}
 
	while (*p++); /* go past end of keyword */
	jmax = sizeof(keywords)/sizeof(keywords[0]);
	jmax--;
	for(j=0; *p && (j < jmax); j++) {
		keywords[j] = p;
		if ( (*p == '-') && (*(p+1) == '\0')) {
			keywordbad = j;
		}
		while (*p++); /* go past end of word */
		while( *p && (*p == ' ') ) p++;
	}
	keywords[j] = NULL;
	keywordi  = -1;
}
 
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                   *
====> StartupCommands called from anyio to control automatic login   *
 *      using keywords from onhost.profil                            *
 *                                                                   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
 
int
StartupCommands(CMScmd,didtimeout, settimeout)
char	*CMScmd;
int	didtimeout, *settimeout;
{
	/* command sequence leading to login and preparing host
	 * the connection is open but we have sent no messages
	 * sending the first message should give an empty screen
	 * and then the VM screen
	 */
 
	int n, tb;
	
if (debugvar) {
	fprintf(stddbg,"stage=%s keywords=",StageIs[(int) UserStage]);
	for (n=0; n < 10; n++)
		if (keywords[n] && keywords[n][0])
			fprintf(stddbg,"%s, ",keywords[n]);
		else break;
	fprintf(stddbg,"\n");
	if ( (keywordi >= 0) && (keywordi < 32) )
		fprintf(stddbg,"got keyword=%s\n", keywords[keywordi]);
	debug2((stddbg,"StartupCommands didtmt=%d keywordi=%d keywordbad=%d\n",
		didtimeout,keywordi,keywordbad));
}
	CMScmd[0] = '\0';
	switch (UserStage) {
 
	case GotNothing:
		if ( havecursor && ConnectOnly )  {
			ConnectOnly = 0;
			keywordset( GotNucx);
		}
		else if ( havecursor || (ScreenStatus == VMRunning) ) {
			putiacbuf(EnterKey,IAC,EOR);
			havecursor = kbd = 0;
			keywordset( GotReady);
		}
		break;
 
	case GotReady:
		if ((ScreenStatus == CPread) || TSOREAD) {
			(void) strcpy(CMScmd , "l ");
			n = strlen(CMScmd);
			if (vmid) {
				(void) strcpy(CMScmd+n,vmid);
			}
			else {
				(void) fputs("\nEnter host userid:\n",stdout);
				(void) getstdin(CMScmd+n,'e');
			}
			keywordset ( SentLogin);
			strncpy(userid,CMScmd+n,6); 

			/* reformat and save userid for VMRunning test */
			userid[6] = '\0';  
			for (tb=n=0; n < 6; n++) {  /* upper case, pad blanks */
				if (tb) 
					userid[n] = ' ';
				else if ('a' <= userid[n])
					userid[n] &= (char)0xdf;
				else if (0 != (tb = ('\0' == userid[n])))
					n--;
			}
		}
		break;
 
	case SentLogin:
		if (keywordi >= keywordbad) {
			UserStage = StartupFailed;
		}
		else if (kbd && havecursor && (keywordi >= 0 )) {
			if (vmpwd) {
				(void) strcpy(CMScmd,vmpwd);
			}
			else {
				(void) fputs("\nEnter host password:\n",stdout);
				(void) getstdin(CMScmd,'n');
			}
			keywordset(SentPassword);
		}
		break;
 
	case SentPassword:
		if (keywordi >= keywordbad ) {
			UserStage = StartupFailed;
		}
		else if (ScreenStatus == CPread) {
			if (keywordi >= 0 ) {
				(void) strcpy(CMScmd , "b");
				keywordset ( GotRestart);
			}
			else {
				keywordset( SentIpl);
				(void) strcpy(CMScmd,iplcmd);
			}
		}
		else if (ScreenStatus == VMread) {
			(void) strcpy(CMScmd, "onhostld");
			keywordset ( SentExec);
		}
		else if TSOREAD
 			if (0 == strncmp(lastchecked+1,"**",2)) {
				/* tso session manager wants control */
				putiacbuf(EnterKey,IAC,EOR);
				havecursor = kbd = 0;
				keywordset ( GotRestart);
			}
			else {
				(void) strcpy(CMScmd, "onhostld");
				keywordset ( SentExec);
			}
		break;
 
	case GotRestart:
		if (hostsys == hosttso) {
			if TSOREAD {
				/* escape to TSO session manager */
				putiacbuf(ClearKey,IAC,EOR);
				havecursor = kbd = 0;
				keywordset ( SentIpl);
			}
		}
		else  /* reconnect without disturbing application */
			keywordset ( GotNucx);
		break;
 
	case SentIpl:
		if (keywordi >= keywordbad )
			UserStage = StartupFailed;
		else if (ScreenStatus == CPread)
			UserStage = WantToQuit;
		else if (ScreenStatus == VMread) {
			(void) strcpy(CMScmd, "onhostld");
			keywordset ( SentExec);
		}
		else if TSOREAD {
			if (keywordi >= 0) {
				(void) strcpy(CMScmd, "onhostld");
				keywordset ( SentExec);
			}
			else  /* terminate TSO session manager */
				(void) strcpy(CMScmd, "end");
		}
		break;
 
	case SentExec:
		if (keywordi >= keywordbad ) {
			UserStage = StartupFailed;
		}
		else if (ScreenStatus == CPread)
			UserStage = WantToQuit;
		else if (keywordi >= 0)
			if ((ScreenStatus == VMread) || TSOREAD)
				keywordset( GotNucx);
		break;

	default:
		;
	}
 
	/* blip progress */
	if (OldUserStage != UserStage) {
		putchar(StageBlip[(int) UserStage]);
		OldUserStage = UserStage;
	}
	else 
		putchar('=');
	(void)fflush(stdout);
 
	n = strlen(CMScmd);
	*settimeout = 0;
	switch (UserStage) {

	case StartupFailed:
		printf("\n%s\n", findkwbuf);  /* display host msg */
	case GotNucx:
	case LostHost:
	case WantToQuit:
		n = -1;
		quitanyio++;
		break;
	case SentIpl:
	case SentPassword:
	     /*	*settimeout = 1; do not request time out */
		break;
	default:
		;
	}
	debug((stddbg,"StartupCommands: line for host, length %d: %s\n",
		n,CMScmd));
	if (debugvar>2) {
		int j;
		fprintf(stddbg,"Leave StartupCommands stage=%s keywords=",
			StageIs[(int) UserStage]);
		for (j=0; j < 10; j++)
		if (keywords[j] && keywords[j][0])
			fprintf(stddbg,"%s, ",keywords[j]);
		else
			break;
		fprintf(stddbg,"\n");
	}
	return(n); /* n = number of bytes in CMScmd */
}
