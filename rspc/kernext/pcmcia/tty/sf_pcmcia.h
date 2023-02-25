/* @(#)24	1.2  src/rspc/kernext/pcmcia/tty/sf_pcmcia.h, pcmciafax, rspc41J, 9512A_all 3/21/95 00:06:52 */
/*
 *   COMPONENT_NAME: pcmciafax
 *
 *   FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994, 1995
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
#ifndef _H_SF_PCMCIA
#define _H_SF_PCMCIA

/* function prototypes used by sf_pcmcia.c */

int CheckCS(void);
int RegisterClient(struct sfcom *);
int Callback(int, int, int, char *, char *, int, char *);
int CardInsertion(struct sfcom *);
int CardRemoval(struct sfcom *);
int CheckCard(int);
int CheckTuple(int, int, chk_tpl_pkt_t *);
int ReadTuple(int, int, CSGetTplDataPkt *, int);
int CheckConfigured(struct sfcom *);
int RequestIO(struct sfcom *);
int RequestIRQ(struct sfcom *);
int RequestConfiguration(struct sfcom *);
int RequestWindow(struct sfcom *);
int ReleaseIO(struct sfcom *);
int ReleaseIRQ(struct sfcom *);
int ReleaseWindow(struct sfcom *);
int ReleaseConfiguration(struct sfcom *);
int DeregisterClient(struct sfcom *);
int AccessConfigurationRegister(struct sfcom *, unsigned char, unsigned char, unsigned char *);
int ResetCard(struct sfcom *);
int ModifyConfiguration(struct sfcom *, unsigned int, unsigned char, unsigned char, unsigned char);
int GetStatus(struct sfcom *, unsigned int *, unsigned int *);
int PowerVcc(struct sfcom *, int);
int hold_lock(struct sfcom *);
int release_lock(struct sfcom *);

unsigned long get_TPCC_RADR(CSGetTplDataPkt *);
int ioaddr_to_TPCE_INDEX(int, long, unsigned char *);
int setup_ring_resume(struct sfcom *);
int clear_ring_resume(struct sfcom *);
int enable_pccard(struct sfcom *);
int disable_pccard(struct sfcom *);


/* debug portion */
#ifdef DEBUG
#define DEBUG_0(str)                     printf(str)
#define DEBUG_1(str, p1)                 printf(str, p1)
#define DEBUG_2(str, p1, p2)             printf(str, p1, p2)
#define DEBUG_3(str, p1, p2, p3)         printf(str, p1, p2, p3)
#define DEBUG_4(str, p1, p2, p3, p4)     printf(str, p1, p2, p3, p4)
#define DEBUG_5(str, p1, p2, p3, p4, p5) printf(str, p1, p2, p3, p4, p5)
#else
#define DEBUG_0(str)
#define DEBUG_1(str, p1)
#define DEBUG_2(str, p1, p2)
#define DEBUG_3(str, p1, p2, p3)
#define DEBUG_4(str, p1, p2, p3, p4)
#define DEBUG_5(str, p1, p2, p3, p4, p5)
#endif /* DEBUG */

#endif /* _H_SF_PCMCIA */
