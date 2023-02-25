/*static char sccsid[] = "src/tcpip/usr/samples/tcpip/onhost/onhost0.h, tcpip, tcpip411, GOLD410 3/22/91 14:51:02";
 *
 * COMPONENT_NAME: TCPIP onhost0.h
 *
 * FUNCTIONS:
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
 
/* Include file for onhost and hostconnect.
 */
 
/*
static char AIXwhatO[] = "@(#)	onhost0.h	1.7 PASC 1.7";
 */
 
/* NL include files and catalog descriptor, if needed */
#include <locale.h>
#include <nl_types.h>
#include "onhost_msg.h"
#include "hostconn_msg.h"
 
/* msgs between onhost and hostconnect */
 
/* hostconnect is the server.  onhost or a similar program is a client.
 * Messages between server and client contain the following commands,
 *  CSclose, CSipl are 1 byte messages
 *  CSem3278 is followed by 1 byte giving emulator type
 *  CSopen is followed by CSdata .. CScwd
 *  CSdata is of the form: CSdata n1 n2 x data where data is n bytes,
 *  n = 256 *n1 +n2, and x = CScmscmd or CScmsinp or CSoutput
 *
 * client can send CSopen, CSclose, CScmscmd, CScmsinp,
 *   CSgetstate, CSem3278, CSipl
 */
#define	CSopen		128	/* client opens a connection	*/
#define	CSclose		129	/* client closes a connection	*/
#define	CSipl  		130	/* client requests ipl		*/
#define	CSstate		131	/* see cmsstate below		*/
#define	CSgetstate	133	/* request cmsstate 		*/
#define	CSem3278	134	/* enter full-screen 3278-2 mode*/
 
#define	CShlen		  4	/* length of CSdata,count,type  */
/* CSdata is followed by a half-word count			*/
/* and CScmscmd or CScmd ...					*/
#define	CSdata		200	/* client/server sends data	*/
#define	CScmscmd	201	/* command for CMS 		*/
#define	CScmsinp	202	/* input for CMS 		*/
#define	CSoutput	203	/* data is normal output 	*/
#define	CSerrmsg	204	/* data is an error message 	*/
#define	CScwd 		206	/* current working directory 	*/
 
/* following are bits in cmsstate */
#define needNDread	1
#define needCMSinp	2
#define needCMScmd	4
#define needCPcmd	8
#define lostconnection	16
#define FullScreen	32

/* for compatibility of sv_handler typing */
#ifdef	AIXV3
#ifndef	__SIGVOID
#define	__SIGVOID	void
#endif	__SIGVOID
#endif	AIXV3

/* Type Functions */
char *getenv();
char *getwd();
char *getcwd();
char *NextItem();
char *gets();
char *ttyname();
char *getlogin();
char *malloc();
int open();
int read();
int write();
int close();
int dbgprcms();
int StartupCommands();
int usertocmsbuf();
int shutdown();
int docmssock();
int _flsbuf();
int srch();
int dokeys();
int putiacbuf();
int exit();
int select();
int putiactn();
int ioctl();
int findkeyword();
int alarm();
int pause();
int getargs();
int findprofile();
int opensockfile();
int init3270();
int getpid();
int fork();
int makeconnection();
int kill();
int getstdin();
int execlp();
int chmod();
int socket();
int atoi();
int connect();
int gethostname();
int bcopy();
int bind();
int getsockname();
int listen();
int accept();
int unlink();
int fstat();
int gettimeofday();
int stat();
int getuid(), geteuid(), setuid();
int sfsname();
int site();
int cmstouserbuf();
unsigned long inet_addr();
void keywordset();
void getem3278();
void openmsg();
void setecho();
void usage();
