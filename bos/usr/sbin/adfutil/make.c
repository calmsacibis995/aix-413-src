static char sccsid[] = "@(#)01	1.3  src/bos/usr/sbin/adfutil/make.c, cmdadf, bos411, 9428A410j 6/15/90 16:58:19";
/*
 * COMPONENT_NAME: (cmdadf) adfutil - process Adapter Description Files
 *
 * FUNCTIONS: (make)
 *             attachcl(), attachfl(), attachni(), attachpl(),
 *             attachrgl(), attachrl(), attachsl(),
 *             makeadft(), makecl(), makefl(), makefr(),
 *             makeni(), makenml(), makepl(), makergl(),
 *             makerl(), makesl()
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

#include "symbol.h"
#include "externs.h"

/*
 * NAME: makeadft
 *                                                                    
 * FUNCTION: make an empty adft struct init to NULL
 *                                                                    
 * RETURN: pointer to adft
 *	
 */  

struct adft *
makeadft(ulId, sbName, iBytes, pslRes, pniItem)
ulong ulId;
char *sbName;
int iBytes;
struct sl *pslRes;
struct ni *pniItem;
{
    struct adft *padft;

    padft = (struct adft *) malloc(sizeof(struct adft));
    if (NULL == padft)
	Fatal("makeadft");
    padft->cardId = ulId;
    padft->cardName = sbName;
    padft->iPos = iBytes;
    padft->psl = pslRes;
    padft->pni = pniItem;
    return padft;
}

/*
 * NAME: attachcl
 *                                                                    
 * FUNCTION: add pclItem to front of pcList
 *                                                                    
 * RETURN: pointer to new list
 *	
 */  

struct cl *
attachcl(pclItem, pcList)
struct cl *pclItem, *pcList;
{
    pclItem->pclNext = pcList;
    return pclItem;
}

/*
 * NAME: makecl
 *                                                                    
 * FUNCTION: make a choice list struct, fill with POS and resource info
 *                                                                    
 * RETURN: pointer to struct
 *	
 */  

struct cl *
makecl(pplPos, prlRes, psbItem)
struct pl *pplPos;
struct rl *prlRes;
char *psbItem;
{
    struct cl *pcl;

    pcl = (struct cl *) malloc(sizeof(struct cl));
    if (NULL == pcl)
	Fatal("makecl");
    pcl->ppl = pplPos;
    pcl->prl = prlRes;
    pcl->item = psbItem;
    pcl->ffunc = 0;
    pcl->pclNext = NULL;
    return pcl;
}

/*
 * NAME: attachfl
 *                                                                    
 * FUNCTION: add filename to front of filename list
 *                                                                    
 * RETURN: pointer to new list
 *	
 */  

struct fl *
attachfl(file, pList)
char *file;
struct fl *pList;
{
    struct fl *pItem;

    pItem = makefl(file);
    pItem->pflNext = pList;
    return pItem;
}

/*
 * NAME: makefl
 *                                                                    
 * FUNCTION: make a filename list struct, fill with filename
 *                                                                    
 * RETURN: pointer to struct
 *	
 */  

struct fl *
makefl(filename)
char *filename;
{
    struct fl *pfl;

    pfl = (struct fl *) malloc(sizeof(struct fl));
    if (NULL == pfl)
	Fatal("makefl");
    pfl->filen = filename;
    pfl->pflNext = NULL;
    return pfl;
}

/*
 * NAME: makefr
 *                                                                    
 * FUNCTION: make a fixed resource struct, fill with pointers to POS and
 *    resource info
 *                                                                    
 * RETURN: pointer to struct
 *	
 */  

struct fr *
makefr(pplPos, prlRes)
struct pl *pplPos;
struct rl *prlRes;
{
    struct fr *pfr;

    pfr = (struct fr *) malloc(sizeof(struct fr));
    if (NULL == pfr)
	Fatal("makefr");
    pfr->ppl = pplPos;
    pfr->prl = prlRes;
    return pfr;
}

/*
 * NAME: attachni
 *                                                                    
 * FUNCTION: add pItem(pointer to nameditem) to front of pList (list of ni)
 *                                                                    
 * RETURN: pointer to list of nameditems
 *	
 */  

struct ni *
attachni(pItem, pList)
struct ni *pItem, *pList;
{
    register struct ni *tItem;

    tItem = pItem;
    while (tItem) {
	if (NULL == tItem->pniNext) break; 
	tItem = tItem->pniNext;
    }
    tItem->pniNext = pList;
    return pItem;
}

/*
 * NAME: makeni
 *                                                                    
 * FUNCTION: make a nameditem, fill with prompt string, choice list, and
 *     help string
 *                                                                    
 * RETURN: pointer to nameditem
 *	
 */  

struct ni *
makeni(sbPrompt, pclChoice, sbHelp)
char *sbPrompt;
struct cl *pclChoice;
char *sbHelp;
{
    struct ni *pni;

    pni = (struct ni *) malloc(sizeof(struct ni));
    if (NULL == pni)
	Fatal("makeni");
    pni->blocknum = 0;
    pni->prompt = sbPrompt;
    pni->comment = sbHelp;
    pni->pcl = pclChoice;
    pni->pniNext = NULL;
    return pni;
}

/*
 * NAME: makenml
 *                                                                    
 * FUNCTION: make a pointer to number list, and number to front of list
 *                                                                    
 * RETURN: pointer to new list
 *	
 */  

struct nml *
makenml(ulNum, pnmList)
ulong ulNum;
struct nml *pnmList;
{
    struct nml *pnml;

    pnml = (struct nml *) malloc(sizeof(struct nml));
    if (NULL == pnml)
	Fatal("makenml");
    pnml->lval = ulNum;
    pnml->pnmlNext = pnmList;
    return pnml;
}

/*
 * NAME: attachpl
 *                                                                    
 * FUNCTION: add pointer to POS item to front of POS list
 *                                                                    
 * RETURN: pointer to new list
 *	
 */  

struct pl *
attachpl(pplItem, ppList)
struct pl *pplItem, *ppList;
{
    pplItem->pplNext = ppList;
    return pplItem;
}

/*
 * NAME: makepl
 *                                                                    
 * FUNCTION: make a POS list entry 
 *                                                                    
 * RETURN: new pointer
 *	
 */  

struct pl *
makepl(ulNum, pByte)
ulong ulNum;
char *pByte;
{
    struct pl *ppl;

    ppl = (struct pl *) malloc(sizeof(struct pl));
    if (NULL == ppl)
	Fatal("makepl");
    ppl->posreg = (int) ulNum;
    ppl->posbyte = pByte;
    ppl->pplNext = NULL;
    return ppl;
}

/*
 * NAME: attachrgl
 *                                                                    
 * FUNCTION: add range list item to front of list
 *                                                                    
 * RETURN: new list pointer
 *	
 */  

struct rgl *
attachrgl(pItem, pList)
struct rgl *pItem, *pList;
{
    pItem->prglNext = pList;
    return pItem;
}

/*
 * NAME: makergl
 *                                                                    
 * FUNCTION: make a range list pointer filled with start and end of range
 *                                                                    
 * RETURN: pointer to item
 *	
 */  

struct rgl *
makergl(ulStart, ulEnd)
ulong ulStart, ulEnd;
{
    struct rgl *prgl;

    prgl = (struct rgl *) malloc(sizeof(struct rgl));
    if (NULL == prgl)
	Fatal("makergl");
    prgl->start = ulStart;
    prgl->end = ulEnd;
    prgl->prglNext = NULL;
    return prgl;
}

/*
 * NAME: attachrl
 *                                                                    
 * FUNCTION: add resource list entry to front of list
 *                                                                    
 * RETURN: pointer to new list
 *	
 */  

#define NEXT(h,n,p) \
	if (n) { \
	    (n)->prlNext = (p); \
	    (n) = (n)->prlNext; \
	} else \
	    (h) = (n) = (p); \

struct rl *
attachrl(pItem, pList)
struct rl *pItem, *pList;
{
    pItem->prlNext = pList;
    return pItem;

#ifdef notdef
    /* this code was removed because it was to complicated */
    struct rl *head, *next;

    if ((NULL == prlIo) && (NULL == prlInt) &&
	(NULL == prlArb) && (NULL == prlMem))
	return NULL;
    head = next = NULL;
    if (NULL != prlIo) {
	head = next = prlIo;
    }
    if (NULL != prlInt) {
	NEXT(head,next,prlInt);
    }
    if (NULL != prlArb) {
	NEXT(head,next,prlArb);
    }
    if (NULL != prlMem){
	NEXT(head,next,prlMem);
    }
    return head;
#endif
}

/*
 * NAME: makerl
 *                                                                    
 * FUNCTION: make a resource list entry filled with resource type and pointer
 *    to the appropiate data (rgl, nml)
 *                                                                    
 * NOTES: 
 *    pList can be a pointer to a rgl or nml.  Note, that to save space
 *    it might have been convient to collapse the types.  This was not
 *    done because it was desired to keep clarity at the risk of space.
 *    Also, if more info was desired on a per resource basis, the info
 *    could have been added.
 *
 * RETURN: pointer to resource list
 *	
 */  

struct rl *
makerl(rType, pList)
enum rs rType;
char *pList;
{
    struct rl *prl;

    prl = (struct rl *) malloc(sizeof(struct rl));
    if (NULL == prl)
	Fatal("makerl");
    prl->rsType = rType;
    prl->prlNext = NULL;
    switch (rType) {
	case rsIo :
	    prl->un.Io.prgl = (struct rgl *) pList;
	    break;
	case rsInt :
	    prl->un.Int.pnml = (struct nml *) pList;
	    break;
	case rsArb : 
	    prl->un.Arb.pnml = (struct nml *) pList;
	    break;
	case rsMem :
	    prl->un.Mem.prgl = (struct rgl *) pList;
	    break;
	default :
	    yyerror("makerl: Unknown type in case %X", rType);
	    break;
    }
    return prl;
}

/*
 * NAME: makesl
 *                                                                    
 * FUNCTION: make a pointer to a "system resource"
 *                                                                    
 * NOTES: fixed resources, Microcode, Unsupported keywords
 *
 * RETURN: pointer to sl 
 *	
 */  

struct sl *
makesl(sType, pList)
enum st sType;
char *pList;
{
    struct sl *psl;

    psl = (struct sl *) malloc(sizeof(struct sl));
    if (NULL == psl)
	Fatal("makesl");
    psl->stType = sType;
    psl->pslNext = NULL;
    switch (sType) {
	case stFixed :
	    psl->un.Fixed.pfr = (struct fr *) pList;
	    break;
	case stMicro :
	    psl->un.Micro.pfl = (struct fl *) pList;
	    break;
	case stUnsup : 
	    psl->un.Unsup.untype = (int) pList;
	    break;
	default :
	    yyerror("makesl: Unknown type in case %X", sType);
	    break;
    }
    return psl;
}

/*
 * NAME: attachsl
 *                                                                    
 * FUNCTION: add system resource item to front of list
 *                                                                    
 * RETURN: new pointer to list
 *	
 */  

struct sl *
attachsl(pslItem, pList)
struct sl *pslItem, *pList;
{
    pslItem->pslNext = pList;
    return pslItem;
}

/* End of File */
