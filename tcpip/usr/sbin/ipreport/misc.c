static char sccsid[] = "@(#)88	1.2  src/tcpip/usr/sbin/ipreport/misc.c, tcpras, tcpip411, GOLD410 10/25/91 09:56:45";
/*
 * COMPONENT_NAME: TCPIP misc.c
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *
 *
 * routines all the different programs and protocols can use in ipreport 
 *
 * Created Sat Aug 10 1991 
 */


#include "sys/param.h"
#include "sys/pri.h"
#include "sys/systm.h"
#include "sys/user.h"
#include "sys/vnode.h"
#include "sys/file.h"
#include "dirent.h"
#include "sys/mbuf.h"
#include "sys/errno.h"
#include "sys/time.h"
#include "sys/vfs.h"
#include "rpc/types.h"
#include "rpc/xdr.h"
#include "nfs.h"
#include "net/spl.h"
#include "netinet/in.h"




void *mymalloc(size)
size_t size;
{
        register char *ans;

        ans = malloc(size);
        if ( NULL != ans ) {
                bzero(ans,size);
        }
        else {
		perror("malloc");
		exit(1);
        }
        return(ans);
}

/* copy a string into allocated space. */
char *scopy(str)
register char *str;
{       register char *new;

        new = mymalloc(strlen(str)+1);
        strcpy(new,str);
        return(new);
}

