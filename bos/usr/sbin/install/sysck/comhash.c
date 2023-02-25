static char sccsid[] = "@(#)23  1.6  src/bos/usr/sbin/install/sysck/comhash.c, cmdinstl, bos411, 9428A410j 1/15/94 17:49:25";
/*
 * COMPONENT_NAME: cmdinstl
 *
 * FUNCTIONS: hash, hash_install, hash_lookup
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#include <stdio.h>
#include "sysdbio.h"

struct tcb_bucket        *tcb_ht[HASHSIZE]; /* hash table */

/*
 * This file contains code which implements a hash table search and
 * management facility.  Collisions and overflow is handled by chaining.
 * Buckets are added to the head of the chain.
 */

/*
 * NAME: hash
 *                                                                    
 * FUNCTION: convert the null terminated string into a hash index
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process.
 *                                                                   
 * RETURNS: hash key created from character string.
 */  

unsigned hash (char *s)
{
	unsigned hashval; /* Hash value for key */

        for (hashval = 0; *s != '\0'; s++)
                hashval = *s + 31 * hashval;
        return hashval % HASHSIZE;
}


/*
 * NAME:	hash_install
 *                                                                    
 * FUNCTION:	Add hash bucket to hash table.  Bucket is added to the head
 *              of the chain pointed to hash table entry link.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process
 *                                                                   
 * RETURNS:	Pointer to added bucket on success, Null on error
 */  

struct tcb_bucket *hash_install(struct tcbent *data)
{
        struct tcb_bucket *bp;
        unsigned hashval;

        bp = (struct tcb_bucket *) malloc(sizeof(*bp));
        if (bp == NULL)
                return NULL;
        hashval = hash(data->tcb_name);
        bp->next = tcb_ht[hashval];
        tcb_ht[hashval] = bp;
        bp->tcbp = data;
        return bp;
}

/*
 * NAME: hash_lookup
 *                                                                    
 * FUNCTION: Search hash table for object and return associated data
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *	User process.
 *                                                                   
 * RETURNS: Pointer to object, or NULL if not found.
 */  
struct tcbent *hash_lookup(char *s)
{
        struct tcb_bucket *bp;         /* pointer to hashtable entry */
        for (bp = tcb_ht[hash(s)]; bp != NULL; bp = bp->next) {
                if (strcmp(s,bp->tcbp->tcb_name) == 0)
                        return bp->tcbp;     /* found */
        }
        return NULL;                    /* not found */
}
