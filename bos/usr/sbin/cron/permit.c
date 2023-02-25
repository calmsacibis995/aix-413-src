static char sccsid[] = "@(#)07	1.8  src/bos/usr/sbin/cron/permit.c, cmdcntl, bos411, 9428A410j 4/25/91 17:16:59";
/*
 * COMPONENT_NAME: (CMDCNTL) commands needed for basic system needs
 *
 * FUNCTIONS: permit
 *
 * ORIGINS: 3, 27, 18
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989, 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/*
 * @OSF_COPYRIGHT@
static char rcsid[] = "RCSfile: permit.c,v Revision: 1.3 (OSF) Date: 90/10/07 22:06:41 ";
*/

/*
 * module used for crontab and at
 */                                                                   

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>
#include <pwd.h>
#include "cron.h"

struct stat globstat;
#define exists(file)    (stat(file,&globstat) == 0)
#undef ROOT     /* defined as 0 in cron.h */
#define ROOT    "root"

int per_errno;  /* status info from getuser */


/*
 * NAME: getuser
 *                                                                    
 * FUNCTION: 
 *      get user name
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *      This function will returns the user name
 *                                                                   
 * (NOTES:) NULL passed as a flag parameter means to list everything
 *          for the administrator.
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: user name
 */  

char *getuser(uid)
int uid;
{
        struct passwd *nptr;

        if ((nptr=getpwuid((uid_t)uid)) == NULL) {
                per_errno=1;
                return(NULL); }
        if ((strcmp(nptr->pw_shell,SHELL)!=0) &&
            (strcmp(nptr->pw_shell,"")!=0)) {
                per_errno=2;
                /* return NULL if you want crontab and at to abort
                   when the users login shell is not /bin/sh otherwise
                   return pw_name
                */
                return(nptr->pw_name);
        }
        return(nptr->pw_name);
}


/*
 * NAME: allowed
 *                                                                    
 * FUNCTION: 
 *      check if the user is allowed or not.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *      This function will returns the 0/1.
 *                                                                   
 * (NOTES:) NULL passed as a flag parameter means to list everything
 *
 * (DATA STRUCTURES:) Effects on global data structures, similar to NOTES.
 *
 * RETURNS: 0 user is not allowed.
 *          1 user is allowed.
 */

allowed(user,allow,deny)
char *user,*allow,*deny;
{
        if ( exists(allow) ) {
                if ( within(user,allow) ) return(1);
                else return(0); }
        else if ( exists(deny) ) {
                if ( within(user,deny) ) return(0);
                else return(1); }
        else if ( strcmp(user,ROOT)==0 ) return(1);
                else return(0);
}

/*
 * NAME: whithin
 *                                                                    
 * FUNCTION: 
 *      checks if the username is found in the fila of filename.
 *                                                                    
 * EXECUTION ENVIRONMENT:
 *                                                                   
 *      This function will returns the 0/1 according to the existence of 
 *      the user name record.
 *                                                                   
 * (NOTES:) NULL passed as a flag parameter means to list everything
 *          for the administrator.
 *
 * (DATA STRUCTURES:) 
 *
 * RETURNS: 0 if username file is not found int filename.
 *          1 if username file is found int filename.
 */

int within(username,filename)
char *username,*filename;
{
        char line[UNAMESIZE];
        FILE *cap;
        int i;

        if((cap = fopen(filename,"r")) == NULL)
                return(0);
        while ( fgets(line,UNAMESIZE,cap) != NULL ) {
                for ( i=0 ; line[i] != '\0' ; i++ ) {
                        if ( isspace(line[i]) ) {
                                line[i] = '\0';
                                break; }
                }
                if ( strcmp(line,username)==0 ) {
                        fclose(cap);
                        return(1); }
        }
        fclose(cap);
        return(0);
}

