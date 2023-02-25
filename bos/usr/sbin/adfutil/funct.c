static char sccsid[] = "@(#)93	1.3  src/bos/usr/sbin/adfutil/funct.c, cmdadf, bos411, 9428A410j 6/15/90 16:58:02";
/*
 * COMPONENT_NAME: (cmdadf) adfutil - process Adapter Description Files
 *
 * FUNCTIONS: (funct) MakeString(), Fatal(), hash(), searchHash(),
 *                    Lookup(), Insert(), tabinit(), pause()
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
#include <string.h>
#include "externs.h"
#include "tn.h"

extern int errno;

/*
 * NAME: MakeString
 *                                                                    
 * FUNCTION: allocate storage for string, return pointer to copy 
 *                                                                    
 * RETURN: pointer to new copy
 *	
 */  

char *
MakeString(sb)
char *sb;
{
    char *sbNew = malloc(strlen(sb) + 1);

    if (NULL == sbNew)
	Fatal("MakeString");
    strcpy(sbNew, sb);
    return sbNew;
}

/*
 * NAME: Fatal
 *                                                                    
 * FUNCTION: print system error on stderr, exit error status
 *                                                                    
 * RETURN: this function never returns
 *	
 */  

Fatal(sb)
char *sb;
{
    int sav_error = errno;

    perror(sb);
    if ((!ffile) && sbFile)
	unlink(sbFile);
    exit(sav_error);
}

/*
 * NAME: hash
 *                                                                    
 * FUNCTION: generate hash value for string of chars
 *                                                                    
 * NOTES: changes string to all lower case
 *
 * RETURN: new hash value, string in lower case
 *	
 */  

int 
hash(sb)
char *sb;
{
    int w;

    for (w = 0; *sb != '\0'; ) {
	*sb = tolower(*sb);
	w += *sb++;
    }
    return(w % iheMax);
}

/*
 * NAME: searchHash
 *                                                                    
 * FUNCTION: find pointer to hashed entry for given string
 *                                                                    
 * RETURN:
 *    NULL = no entry found
 *    phe  = entry
 */  

struct he *
searchHash(sb)
char *sb;
{
    int ihe;
    struct he *phe;

    ihe = hash(sb);
    for (phe = rgphe[ihe]; phe != NULL; phe = phe->phe) {
	if (strcmp(sb, phe->sb) == 0)
	    return phe;
    }
    return NULL;
}

/*
 * NAME: Lookup
 *                                                                    
 * FUNCTION: find entry in hash table, add if necessary, return pointer to
 *    entry.
 *                                                                    
 * RETURN: pointer to hash'd entry
 *	
 */  

struct he *
Lookup(sb)
char *sb;
{
    struct he *phe, *searchHash();
    int ihe;

    phe = searchHash(sb);
    if (phe == NULL) {
	phe = (struct he *) malloc(sizeof(struct he));
	if (NULL == phe)
	    Fatal("Lookup");
	ihe = hash(sb);
	phe->sb = MakeString(sb);
	phe->tokid = -1;
	phe->phe = rgphe[ihe];
	rgphe[ihe] = phe;
	return phe;
    } else 
	return phe;
}

/*
 * NAME: Insert
 *                                                                    
 * FUNCTION: Insert string and token-id into table
 *                                                                    
 * RETURN: none
 *	
 */  

Insert(sb, tokenid)
char *sb;
int tokenid;
{
    struct he *phe, *searchHash();
    int ihe;

    phe = searchHash(sb);
    if (phe == NULL) {
	phe = (struct he *) malloc(sizeof(struct he));
	if (NULL == phe)
	    Fatal("Lookup");
	ihe = hash(sb);
	phe->sb = MakeString(sb);
	phe->tokid = tokenid;
	phe->phe = rgphe[ihe];
	rgphe[ihe] = phe;
    } else 
	yyerror("Insert on token %s\n", sb);
}

/*
 * NAME: tabinit
 *                                                                    
 * FUNCTION: add known keywords into hash table before scanning begins so
 *    that these entries may be found if they exist in the data
 *                                                                    
 * RETURN: none
 *	
 */  

tabinit()
{
    Insert("adapterid", ID);
    Insert("adaptername", NAME);
    Insert("address", ADDRESS);
    Insert("arb", ARB);
    Insert("begin", BBEGIN);
    Insert("choice", CHOICE);
    Insert("device", DEVICE);
    Insert("dynarballoc", DYNARB);
    Insert("end", END);
    Insert("fixedresources", FIXED);
    Insert("function", FUNCTION);
    Insert("help", HELP);
    Insert("initprog", INITPROG);
    Insert("input", INPUT);
    Insert("int", INT);
    Insert("io", IO);
    Insert("lock", LOCK);
    Insert("mem", MEM);
    Insert("microcode", MICROCODE);
    Insert("nameditem", ITEM);
    Insert("nodma", NODMA);
    Insert("numbytes", BYTES);
    Insert("pos", POS);
    Insert("prompt", PROMPT);
    Insert("rs422", RS422);
    Insert("scsi", SCSI);
    Insert("sharedarb", SHAREDARB);
    Insert("sysmem", SYSMEM);
    Insert("vcheck", VCHECK);
}

/*
 * NAME: pause
 *                                                                    
 * FUNCTION: read stdin until enter is pressed on keyboard
 *                                                                    
 * RETURN: none
 *	
 */  

pause(sb)
char *sb;
{
    char ch;

    if (fquiet) return;
    fprintf(stdout, sb);
    fflush(stdout);
/* turn off echo */
    while (EOF != (ch = getchar())) {
	if (ch == '\n') break;
    }
/* turn on echo */
}
