/* @(#)89	1.3  src/bos/usr/sbin/adfutil/dtype.h, cmdadf, bos411, 9428A410j 6/15/90 16:57:51 */
/*
 * COMPONENT_NAME: (cmdadf) adfutil - process Adapter Description Files
 *
 * FUNCTIONS: dtype.h -- definitions for data types
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
#define DEVICE
#define SYSMEM
#define INITPROG
#define LOCK
#define ADDRESS
*/
#define UNKNOWN 999

enum rs {	/* resource types */
    rsIo,
    rsInt,
    rsArb,
    rsMem
};

enum st {	/* system types */
    stFixed,
    stMicro,
    stUnsup
};

enum ty {	/* ?? */
    tyId,
    tyBytes,
    tyName,
    tyFixed,
    tyItem
};

struct cl {	/* choice list */
    struct pl *ppl;
    struct rl *prl;
    char *item;
    int ffunc;
    struct cl *pclNext;
};

struct pl {	/* pos list */
    int posreg;
    char *posbyte;
    struct pl *pplNext;
};

struct rgl {	/* range list */
    ulong start;
    ulong end;
    struct rgl *prglNext;
};

struct nml {	/* number list */
    ulong lval;
    struct nml *pnmlNext;
};

struct rl {	/* resource list */
    enum rs rsType;
    struct rl *prlNext;
    union {
	struct {
	    struct rgl *prgl;
	} Io;
	struct {
	    struct nml *pnml;
	} Int;
	struct {
	    struct nml *pnml;
	} Arb;
	struct {
	    struct rgl *prgl;
	} Mem;
    } un;
};

struct fl {	/* file list */
    char *filen;
    struct fl *pflNext;
}; 

struct fr {	/* fixed resources */
    struct pl *ppl;	/*    POS setting list */
    struct rl *prl;	/*    resouce list */
};

struct sl {	/* system list */
    enum st stType;
    struct sl *pslNext;
    union {
	struct {
	    struct fr *pfr;
	} Fixed;
	struct {
	    struct fl *pfl;
	} Micro;
	struct {
	    int untype;
	} Unsup;
    } un;
};

struct ni {	/* named item */
    int blocknum;
    char *prompt;
    char *comment;
    struct cl *pcl;	/*    choice list */
    struct ni *pniNext;
};

struct adft {	/* adf type */
    ulong cardId;
    char *cardName;
    int iPos;
    struct sl *psl;	/* system resources list */
    struct ni *pni;	/* named items */
};
