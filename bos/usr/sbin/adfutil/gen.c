static char sccsid[] = "@(#)95	1.7  src/bos/usr/sbin/adfutil/gen.c, cmdadf, bos411, 9428A410j 6/15/90 16:58:06";
/*
 * COMPONENT_NAME: (cmdadf) adfutil - process Adapter Description Files
 *
 * FUNCTIONS: (gen) 
 *             genChoice(), genClose(), genFixed(), genFunction(),
 *             genGroup(), genItems(), genMicro(), genNumber(),
 *             genRange(), genResEntry(), genResource(), genSystem(),
 *             geninit(), genodm(),
 *             makePdAt(), makePdAt_l(), makegenmap(),
 *             processTrans(), resourceInit(), DeleteCurr()
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

#include <sys/cfgodm.h>
#include "symbol.h"
#include "externs.h"
#include "genodm.h"

struct PdAt_l {
    struct PdAt *pAttr;
    struct PdAt_l *pNext;
};

struct def_keys {
    int  def_type;
    char *typecode;
    char *sbAttr;
    int  chcnt;
    int  cnt;
    int  currcnt;
    struct PdAt_l *pList;
};

static struct def_keys def_attrs[] = {
#define ARBLVL 0
	{ ARBLVL, "A", "arblvl",   6, 0, 0, NULL }, 
#define INTLVL 1
	{ INTLVL, "I", "intlvl",   6, 0, 0, NULL },
#define IOADDR 2
	{ IOADDR, "O", "ioaddr",   6, 0, 0, NULL },
#define MADDR 3
	{ MADDR,  "M", "maddr",    5, 0, 0, NULL },
#define GROUP 4 
	{ GROUP,  "G", "group",    5, 0, 0, NULL },
#define FNAME 5
	{ FNAME,  "f", NULL,       1, 0, 0, NULL },
#define FUNCT 6
	{ FUNCT,  "F", "func",     4, 0, 0, NULL },
	{ 0,      "",         0, 0, 0, NULL }
};

/* static int cItem = 0; */
static int fdeflt;
static char untype[98];
static char delcrit[256];
static struct genmap *currAt, *transAt;

#define AddCurr(x) \
    if (currAt) { \
	(x)->pNext = currAt; \
	currAt = x; \
    } currAt = x
#define AddTrans(x, y) \
    (x)->trans_t = y; \
    if (transAt) { \
	(x)->pNext = transAt; \
	transAt = x; \
    } transAt = x

static struct PdAt *makePdAt();
static struct PdAt_l *makePdAt_l();
static struct genmap *makegenmap();
static void processTrans();
static void genGroup();
static void genItems();
static void genChoice();
static void genRange();
static void genNumber();
static void DeleteCurr();
static int geninit();
static int genClose();
static void genSystem();
static void genFixed();
static void resourceInit();
static void genResource();
static void genResEntry();
static void genFunction();
void genMicro();

/*
 * NAME: genodm
 *                                                                    
 * FUNCTION: entry point for adding generated info into database
 *
 * DATA STRUCTURES:
 *    struct adf proot - pointer to info generated by parser
 *                                                                    
 * RETURN: none
 *	
 */  

genodm()
{
     char sb[15];

     if (!geninit())
	 return 1;

     /*
     sprintf(sb, "%#X", proot->cardId);
     device->pEntry = make_entry(proot->cardName, sb, NULL);
     */

     genSystem(proot->psl);

     genItems(proot->pni);
     genClose();
}

/*
 * NAME: geninit
 *                                                                    
 * FUNCTION: 
 *    initialize database
 *    get Predefined Device entry for device
 *    get Predefined Attribute entries for device
 *    add Attribute entries to Current list of Attributes
 *                                                                    
 * DATA STRUCTURES:
 *    curr - list of entries in PdAt that might be changed by ADF
 *
 * RETURN: none
 *	
 */  

static int
geninit()
{
    struct PdDv *pDv;
    struct PdAt *pAt;
    struct listinfo *pInfo;
    struct genmap *temp;
    char search[255];
    int found, i;

    if (0 > odm_initialize()) {
	fprintf(stderr, NL_MSG(ODM_INIT, "*** FAIL: odm_initialize()\n"));
	return 0;
    }

    currAt = NULL;
    transAt = NULL;
    sprintf(search, "devid = 0x%x", proot->cardId);
    pInfo = (struct listinfo *) malloc(sizeof(struct listinfo));
    if (!pInfo) {
	fprintf(stderr, NL_MSG(MAL_FAIL, "*** FAIL: malloc()\n"));
	return 0;
    }

    /* get Predefine Device */

    pDv = (struct PdDv *) odm_get_list(PdDv_CLASS, search, pInfo, 1, 1);
    if (-1 == (int)pDv) {
	 fprintf(stderr, NL_MSG(DATA_FAIL,
                 "*** FAIL: database error (search %s)\n"), search);
	 Fatal("geninit()");
    }
    if (!pDv || (pInfo->num > 1)) {
	fprintf(stderr, NL_MSG(MULT_OBJS, 
                "*** ERROR: search (%s) returned %d objects\n"),
                 search, pInfo->num);
	return 0;
    }
  
    if (odmdebug) {
	printf("\nPredefine Device : search = (%s)\n", search);
	printf("class(%s) subclass(%s) type(%s) prefix(%s)\n", 
	    pDv->class, pDv->subclass, pDv->type, pDv->prefix);
	printf("uniquetype = %s\n", pDv->uniquetype);
    }

    /* get Predefine Attribute */

    sprintf(search, "uniquetype = %s/%s/%s", pDv->class, pDv->subclass, 
	pDv->type);
    sprintf(untype, "%s/%s/%s", pDv->class, pDv->subclass, pDv->type);
    pAt = (struct PdAt *) odm_get_list(PdAt_CLASS, search, pInfo, 1, 1);
    if (odmdebug) {
	printf("search (%s) on PdAt returned %d objects\n", search, pInfo->num);
    }

    /* go through PdAt's, add necessary ones to curr list */

    for ( ; pInfo->num > 0; pInfo->num--) {
	if (index(pAt->type, 'f')) {
	    def_attrs[FNAME].cnt++;
	    found = FNAME;
	} else for (found = -1, i=0; /* no test */; i++) {
	    if (def_attrs[i].chcnt != 0) {
		if (0 == strncmp(pAt->attribute, def_attrs[i].sbAttr, 
			def_attrs[i].chcnt)) {
		    def_attrs[i].cnt++;
		    found = i;
		    break;
		}
	    } else break;
	}
	if (found >= 0) {
	    if (odmdebug)
		printf("%-32s added to current list\n", pAt->attribute);
	    temp = makegenmap(0, pAt, sizeof(struct PdAt));
	    AddCurr(temp);
	}
	pAt++;
    }
    return 1;
}

/*
 * NAME: genClose
 *                                                                    
 * FUNCTION: change database entries, close database
 *                                                                    
 * RETURN: none
 *	
 */  

static int
genClose()
{
    int rc, count = 0;

    if (odmdebug)
	printf("genClose[ENTRY]\n");
    processTrans(transAt, &count);
    odm_terminate();
    if (odmdebug)
	printf("genClose[EXIT]: processed %d objects\n", count);
}

/*
 * NAME: processTrans
 *                                                                    
 * FUNCTION: do each database transaction found in list
 *                                                                    
 * NOTES: Titem(Transaction ITEM) is a singly-linked list that is scanned
 *    til the bottom is found, and items are processed from the end of the 
 *    list.  This was so that items were processed as they were added.
 *
 * RETURN: pcnt is increased by one for each transaction processed
 *	
 */  

static void
processTrans(Titem, pcnt)
struct genmap *Titem;
int *pcnt;
{
    int rc;

    if (!Titem) return;

    processTrans(Titem->pNext, pcnt);
    (*pcnt)++;
    switch (Titem->trans_t) {
    case t_ADD:
	if (odmdebug)
	    printf("processTrans[ADDOBJ]: untype = %s attr = %s\n",
		((struct PdAt *) Titem->pEntry)->uniquetype,
		((struct PdAt *) Titem->pEntry)->attribute);
	rc = odm_add_obj(PdAt_CLASS, Titem->pEntry);
	if (-1 == rc) {
	    fprintf(stderr, NL_MSG(ADD_OBJ, 
                    "*** FAIL[processTrans]: odm_add_obj (attr = %s)\n"),
                    ((struct PdAt *) Titem->pEntry)->attribute);
	    fprintf(stderr, NL_MSG(PROC_CON, "*** Processing Continued\n"));
	}
	break;
    case t_DELETE:
	if (odmdebug)
	    printf("processTrans[DELOBJ]: untype = %s attr = %s\n",
		((struct PdAt *) Titem->pEntry)->uniquetype,
		((struct PdAt *) Titem->pEntry)->attribute);
        sprintf(delcrit, "attribute = %s", 
	    ((struct PdAt *)Titem->pEntry)->attribute);
	rc = odm_rm_obj(PdAt_CLASS, delcrit); 
	if (-1 == rc) {
	    fprintf(stderr, NL_MSG(REM_OBJ, 
                    "*** ERROR[processTrans]: odm_rm_obj (attr = %s)\n"),
                    ((struct PdAt *) Titem->pEntry)->attribute);
	    fprintf(stderr, NL_MSG(PROC_CON, "*** Processing Continued\n"));
	}
	break;
    case t_CHANGE:
	if (odmdebug)
	    printf("processTrans[CHGOBJ]: attr = %s\n",
		((struct PdAt *) Titem->pEntry)->attribute);
    default:
	break;
    }
}

/*
 * NAME: makegenmap
 *                                                                    
 * FUNCTION: make and fill a genmap struct
 *                                                                    
 * NOTES: define MEM_PROBLEM_NOTDEF if there is a problem with memory
 *    and that pEntry needs to be copied first
 *
 * RETURN: pointer to new struct genmap
 *	
 */  

static struct genmap *
makegenmap(cType, pEntry, EntrySize)
int cType;
char *pEntry;
{
    struct genmap *pGMap = (struct genmap *) malloc(sizeof(struct genmap));
#ifdef MEM_PROBLEM_NOTDEF
    char *pcopy;
#endif

    if (!pGMap)
	Fatal("makegenmap[pGMap]");
#ifdef MEM_PROBLEM_NOTDEF
    if (NULL == (pcopy = (char *) malloc(EntrySize))) 
        Fatal("makegenmap[pcopy]");
    bcopy(pEntry, pcopy, EntrySize);
#endif
    pGMap->ClassType = cType;
    pGMap->trans_t   = 0;
#ifdef MEM_PROBLEM_NOTDEF
    pGMap->pEntry    = pcopy;
#else
    pGMap->pEntry    = pEntry;
#endif
    pGMap->pNext     = NULL;
    return pGMap;
}

static void
genSystem(pSys)
struct sl *pSys;
{
    if (!pSys) return;

    genSystem(pSys->pslNext);

    switch (pSys->stType) {
    case stFixed:
	genFixed(pSys->un.Fixed.pfr);
	break;
    case stMicro:
	genMicro(pSys->un.Micro.pfl);
	break;
    case stUnsup:
    default:
	break;
    }
}

static void
genFixed(pFixed)
struct fr *pFixed;
{
    if (!pFixed) return;

    resourceInit();

    genResource(pFixed->prl, 1);

    genResEntry(false);
}

void
genMicro(pfl)
struct fl *pfl;
{
    char afile[80], dosfile[20];
    char *rootn, *of;

    if (ffile) {
	fprintf(stderr, NL_MSG(MIC_ERR, 
                "Warning : Can't process MICROCODE FILES with -f option\n"));
	return;
    }
    while (pfl) {
	if (odmdebug) 
	    printf("MICROCODE FILE %s\n", pfl->filen);
	rootn = rindex(pfl->filen, '/');
	if (rootn) {
	    of = pfl->filen;
	    rootn++;
	} else {
	    sprintf(afile, "/etc/microcode/%s", pfl->filen);
	    of = afile;
	    rootn = pfl->filen;
	}
	sprintf(dosfile, "A:%s", rootn);
	if (!getdosfile(sbDevice, dosfile, of, 0)) {
	    fprintf(stderr, NL_MSG(MIC_TRANS, 
                    "Can't transfer %s to %s from device %s\n"), 
                    rootn, of, sbDevice);
	} else {
	    fprintf(stdout, NL_MSG(MIC_TRANS_OK,
		    "Successful transfer of %s\n"), of);
	}
	pfl = pfl->pflNext;
    }
}

/*
 * NAME: genItems
 *                                                                    
 * FUNCTION: process NamedItems of an ADF
 *                                                                    
 * NOTES: this is a recursive procedure because the list of named items 
 *    is reversed of the order it was read in
 *
 * DATA STRUCTURES:
 *    def_attrs is a struct of the possible defined attributes that might
 *    be found under a named item
 *
 * RETURN: none
 *	
 */  

static void
genItems(pItems)
struct ni *pItems;
{
    struct genmap *temp;
    struct PdAt_l *tList;

    if (!pItems) return;

    genItems(pItems->pniNext);

    resourceInit();

    genChoice(pItems->pcl, 0);

    genResEntry(true);

    if (tList = def_attrs[FUNCT].pList) {
	if (odmdebug) 
	    printf("%s (ADDED)\n\tvalues (%3d): %s\n\tdeflt  (%3d): %s\n\twidth  (%3d): %s\n\n",
		tList->pAttr->attribute,
		strlen(tList->pAttr->values), tList->pAttr->values,
		strlen(tList->pAttr->deflt),  tList->pAttr->deflt,
		strlen(tList->pAttr->width),  tList->pAttr->width);
	temp = makegenmap(0, tList->pAttr, sizeof(struct PdAt));
	AddTrans(temp, t_ADD);
    }
}

/*
 * NAME: genChoice
 *                                                                    
 * FUNCTION: process Choices of a NamedItem
 *                                                                    
 * NOTES: this is a recursive procedure because the list of choices
 *    is reversed of the order it was read in
 *
 * DATA STRUCTURES:
 *    def_attrs is changed for the resources found under the choice
 *
 * RETURN: none
 *	
 */  

static void
genChoice(pclItem, depth)
struct cl *pclItem;
int depth;
{
    if (!pclItem) return;

    genChoice(pclItem->pclNext, ++depth);

    if (pclItem->prl)
	genResource(pclItem->prl, depth);
    else if (pclItem->ffunc)
	genFunction(pclItem->item);
}

static void
resourceInit()
{
    fdeflt = 1;
    def_attrs[ARBLVL].pList = NULL;
    def_attrs[INTLVL].pList = NULL;
    def_attrs[IOADDR].pList = NULL;
    def_attrs[MADDR].pList  = NULL;
    def_attrs[FUNCT].pList  = NULL;
    def_attrs[GROUP].pList  = NULL;
}

static void
genResource(rsl, depth)
struct rl *rsl;
int depth;
{
    while (rsl) {
	switch (rsl->rsType) {
	case rsIo:
	    if (odmdebug) 
		printf("%sIo resource \n",(fdeflt ? "Default " : "        "));
	    genRange(&def_attrs[IOADDR], def_attrs[IOADDR].pList, 
		rsl->un.Io.prgl, depth);
	    break;
	case rsInt:
	    if (odmdebug) 
		printf("%sInt resource \n",(fdeflt ? "Default " : "        "));
	    genNumber(&def_attrs[INTLVL], def_attrs[INTLVL].pList, 
		rsl->un.Int.pnml, depth);
	    break;
	case rsArb:
	    if (odmdebug) 
		printf("%sArb resource \n",(fdeflt ? "Default " : "        "));
	    genNumber(&def_attrs[ARBLVL], def_attrs[ARBLVL].pList, 
		rsl->un.Arb.pnml, depth);
	    break;
	case rsMem:
	    if (odmdebug) 
		printf("%sMem resource \n",(fdeflt ? "Default " : "        "));
	    genRange(&def_attrs[MADDR], def_attrs[MADDR].pList, 
		rsl->un.Mem.prgl, depth);
	    break;
	default:
	   break;
	}
	rsl = rsl->prlNext;
    }
    if (fdeflt) fdeflt = false;
}

static void
genResEntry(fgroup)
int fgroup;
{
    int i;
    int rescnt;
    struct genmap *temp;
    struct PdAt_l *tList;

    rescnt = 0;
    for (i=0; i < GROUP; i++) 
	if (tList = def_attrs[i].pList) {
	    rescnt++;
	    while (tList) {
		if (odmdebug) 
		    printf("%s (ADDED)\n\tvalues (%3d): %s\n\tdeflt  (%3d): %s\n\twidth  (%3d): %s\n\n",
			tList->pAttr->attribute,
			strlen(tList->pAttr->values), tList->pAttr->values,
			strlen(tList->pAttr->deflt),  tList->pAttr->deflt,
			strlen(tList->pAttr->width),  tList->pAttr->width);
		if (fgroup) genGroup(tList->pAttr);
		temp = makegenmap(0, tList->pAttr, sizeof(struct PdAt));
		AddTrans(temp, t_ADD);
		tList = tList->pNext;
	    }
	}
    if (fgroup) {
	tList = def_attrs[GROUP].pList;
	if ((rescnt > 1) && (!def_attrs[FNAME].pList)) {
	    if (odmdebug) 
		printf("%s (ADDED)\n\tvalues (%3d): %s\n\tdeflt  (%3d): %s\n\twidth  (%3d): %s\n\n",
		    tList->pAttr->attribute,
		    strlen(tList->pAttr->values), tList->pAttr->values,
		    strlen(tList->pAttr->deflt),  tList->pAttr->deflt,
		    strlen(tList->pAttr->width),  tList->pAttr->width);
	    temp = makegenmap(0, tList->pAttr, sizeof(struct PdAt));
	    AddTrans(temp, t_ADD);
	}
	if (def_attrs[FNAME].pList) {
	    (void) strcpy(def_attrs[FNAME].pList->pAttr->values, 
		tList->pAttr->values);
	    tList = def_attrs[FNAME].pList;
	    if (odmdebug) 
		printf("%s (ADDED)\n\tvalues (%3d): %s\n\tdeflt  (%3d): %s\n\twidth  (%3d): %s\n\n",
		    tList->pAttr->attribute,
		    strlen(tList->pAttr->values), tList->pAttr->values,
		    strlen(tList->pAttr->deflt),  tList->pAttr->deflt,
		    strlen(tList->pAttr->width),  tList->pAttr->width);
	    temp = makegenmap(0, tList->pAttr, sizeof(struct PdAt));
	    AddTrans(temp, t_ADD);
	}
    }
}

static void
genFunction(funcName)
char *funcName;
{
    struct PdAt_l *pdList;
    char sbfunc[25];
    int ls0, ls1;

    if (def_attrs[FUNCT].pList) {
	genResEntry(1);
	pdList = def_attrs[FUNCT].pList;
	resourceInit();
	def_attrs[FUNCT].pList = pdList;
	sprintf(sbfunc, ",%s", funcName);
    } else {
	pdList =
	def_attrs[FUNCT].pList = makePdAt_l(FUNCT, def_attrs[FUNCT].pList);
	sprintf(sbfunc, "%s", funcName);
	sprintf(pdList->pAttr->deflt, "%s", funcName);
    }
    ls0 = strlen(pdList->pAttr->values);
    ls1 = strlen(sbfunc);
    if (ls0 + ls1 <= 255) {
	(void) strcat(pdList->pAttr->values, sbfunc);
    } else if (odmdebug)
	fprintf(stderr, NL_MSG(BUFF_FULL, 
                "(genFuction)BUFFER FULL : %s not added to values\n"), sbfunc);
    def_attrs[FNAME].sbAttr = funcName;
    def_attrs[FNAME].chcnt  = strlen(funcName);
    def_attrs[FNAME].pList = makePdAt_l(FNAME, def_attrs[FNAME].pList);
}

/*
 * NAME: genRange
 *                                                                    
 * FUNCTION: change ADF range (lownum - highnum) into database info
 *                                                                    
 * NOTES:
 *
 * DATA STRUCTURES:
 *    Attribute's value string
 *
 * RETURN:
 *	
 */  

static void
genRange(pdef, pdList, prgl, depth)
struct def_keys *pdef;
struct PdAt_l *pdList;
struct rgl  *prgl;
int depth;
{
    int width;
    int ls0, ls1;
    char sbstart[25];

    if (!prgl) return;

    if (pdList)
	genRange(pdef, pdList->pNext, prgl->prglNext, depth);
    else
	genRange(pdef, pdList, prgl->prglNext, depth);

    if (fdeflt)
	pdList = pdef->pList = makePdAt_l(pdef->def_type, pdef->pList);

    width = prgl->end - prgl->start + 1;
    if (odmdebug) {
	printf("\t\t%#x, %#x : w %d\n", prgl->start, prgl->end, width);
    }

    ls0 = strlen(pdList->pAttr->values);
    if (0 == ls0) 
	sprintf(sbstart, "%#x", prgl->start);
    else
	sprintf(sbstart, ",%#x", prgl->start);

    if (NULL == pdList->pAttr->width[0]) 
	sprintf(pdList->pAttr->width, "%#x", width);
    if (NULL == pdList->pAttr->deflt[0]) 
	sprintf(pdList->pAttr->deflt, "%#x", prgl->start);

    ls1 = strlen(sbstart);
    if (ls0+ls1 <= 255) {
	(void) strcat(pdList->pAttr->values, sbstart);
    } else if (odmdebug)
	fprintf(stderr, NL_MSG(BUFF_FULL, 
                "(genRange)BUFFER FULL : %s not added to values\n"), sbstart);
}

/*
 * NAME: genNumber
 *                                                                    
 * FUNCTION: change ADF number into database info
 *                                                                    
 * NOTES:
 *
 * DATA STRUCTURES:
 *    Attribute's value string
 *
 * RETURN:
 *	
 */  

static void
genNumber(pdef, pdList, pnml, depth)
struct def_keys *pdef;
struct PdAt_l *pdList;
struct nml  *pnml;
int depth;
{
    int ls0, ls1;
    char sblval[25];

    if (!pnml) return;

    if (pdList)
	genNumber(pdef, pdList->pNext, pnml->pnmlNext, depth);
    else
	genNumber(pdef, pdList, pnml->pnmlNext, depth);

    if (fdeflt) 
	pdList = pdef->pList = makePdAt_l(pdef->def_type, pdef->pList);

    if (odmdebug) {
	printf("\t\t%d\n", pnml->lval);
    }

    ls0 = strlen(pdList->pAttr->values);
    if (0 == ls0)
	sprintf(sblval, "%d", pnml->lval);
    else
	sprintf(sblval, ",%d", pnml->lval);

    if (NULL == pdList->pAttr->deflt[0]) 
	sprintf(pdList->pAttr->deflt, "%d", pnml->lval);

    ls1 = strlen(sblval);
    if (ls0+ls1 <= 255) {
	(void) strcat(pdList->pAttr->values, sblval);
    } else if (odmdebug)
	fprintf(stderr, NL_MSG(BUFF_FULL, 
                "(genNumber)BUFFER FULL : %s not added to values\n"), sblval);
}

/*
 * NAME: makePdAt
 *                                                                    
 * FUNCTION: make an empty pointer to database Attribute
 *                                                                    
 * RETURN: pointer to PdAt
 *	
 */  

static struct PdAt *
makePdAt()
{
    struct PdAt *pAt = (struct PdAt *) malloc(sizeof(struct PdAt));

    if (!pAt) Fatal("makePdAt()");

    pAt->uniquetype[0] = NULL;
    pAt->attribute[0]  = NULL;
    pAt->deflt[0]      = NULL;
    pAt->values[0]     = NULL;
    pAt->width[0]      = NULL;
    pAt->type[0]       = NULL;
    pAt->generic[0]    = NULL;
    pAt->rep[0]        = NULL;

    return pAt;
}

/*
 * NAME: makePdAt_l
 *                                                                    
 * FUNCTION: add resource in a PdAt, add PdAt to pAtList
 *                                                                    
 * RETURN: pointer to new list
 *	
 */  

static struct PdAt_l *
makePdAt_l(restype, pAtList)
int restype;
struct PdAt_l *pAtList;
{
    struct PdAt_l *pAtL = (struct PdAt_l *) malloc(sizeof(struct PdAt_l));

    if (!pAtL) Fatal("makePdAt_l()");

    pAtL->pNext = pAtList;
    pAtL->pAttr = makePdAt();
    if (def_attrs[restype].cnt) 
	DeleteCurr(restype);
    if (restype != FNAME) {
	sprintf(pAtL->pAttr->attribute, "%s%d", def_attrs[restype].sbAttr, 
	    def_attrs[restype].currcnt++);
	strcpy(pAtL->pAttr->generic, "D");	/* different for sharedarb */
    } else {
	sprintf(pAtL->pAttr->attribute, "%s", def_attrs[restype].sbAttr);
    }
    strcpy(pAtL->pAttr->type, def_attrs[restype].typecode);
    strcpy(pAtL->pAttr->rep, "l"); 		/* ell */
    strcpy(pAtL->pAttr->uniquetype, untype);

    return pAtL;
}

/*
 * NAME: genGroup
 *                                                                    
 * FUNCTION: 
 *                                                                    
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURN:
 *	
 */  

static void
genGroup(pPdAt)
struct PdAt *pPdAt;
{
    struct def_keys *pdef = &def_attrs[GROUP];
    struct PdAt_l *pList;
    char sbname[25];
    int ls0, ls1;

    if (!pdef->pList) {
	pList = pdef->pList = makePdAt_l(pdef->def_type, NULL);
	sprintf(sbname, "%s", pPdAt->attribute);
    } else {
	pList = pdef->pList;
	sprintf(sbname, ",%s", pPdAt->attribute);
    }

    ls0 = strlen(pList->pAttr->values);
    ls1 = strlen(sbname);
    if (ls0+ls1 <= 255) {
	(void) strcat(pList->pAttr->values, sbname);
    } else if (odmdebug)
	fprintf(stderr, NL_MSG(BUFF_FULL, 
                "(genGroup)BUFFER FULL : %s not added to values\n"), sbname);
}

/*
 * NAME: DeleteCurr
 *                                                                    
 * FUNCTION: move (any) database items of def_type from "curr" list to
 *    "trans" list for deletion from database
 *                                                                    
 * RETURN: none
 *	
 */  

static void
DeleteCurr(def_type)
int def_type;
{
    struct genmap *temp, *head;

    head = temp = currAt;
    while (def_attrs[def_type].cnt) {
	if (0 == strncmp(((struct PdAt *)temp->pEntry)->attribute, 
	    def_attrs[def_type].sbAttr, def_attrs[def_type].chcnt)) {
	    def_attrs[def_type].cnt--;
	    if (odmdebug)
		printf("deleting %s from current list\n", 
		    ((struct PdAt *)temp->pEntry)->attribute);
	    if (head == temp) {
		temp = temp->pNext;
		head->pNext = NULL;
		AddTrans(head, t_DELETE);
		if (head == currAt)
		    currAt = head = temp;
		else
		    head = temp;
	    } else {
		head->pNext = temp->pNext;
		temp->pNext = NULL;
		AddTrans(temp, t_DELETE);
		temp = head->pNext;
	    }
	} else {
	    head = temp;
	    temp = temp->pNext;
	    if (!temp) break;
	}
    }
}
