/* @(#)23	1.11  src/bos/usr/sbin/grpck/grpck.h, cmdsadm, bos411, 9428A410j 5/15/91 14:01:05 */
/*
 * COMPONENT_NAME: (TCBADM) grpck - Check group information
 *
 * FUNCTIONS: NONE
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


#define	MSGSTR(Num, Str) grpckcatgets (Num, Str)

#define	DEF_USAGE	"grpck [ -p | -y | -n | -t ] [ group(s) ... | ALL ]\n"
#define DEF_BADCHAR	"Bad character in group name \"%s\".\n"
#define DEF_BADNAME	"Invalid group name \"%s\".\n"
#define	DEF_NOGRNAME	"No Group name for the entry \"%s\".\n"
#define DEF_BADETCREQ	"No stanza in /etc/group for requested group \"%s\".\n"
#define DEF_BADSECREQ	"No stanza in /etc/security/group for requested group \"%s\".\n"
#define DEF_BADGROUP	"Bad group id \"%s\" in \"%s\".\n"
#define DEF_BADUSER	"User name \"%s\" not found in password file.\n"
#define DEF_NONUNIQNAM	"Group name \"%s\" is not unique.\n"
#define DEF_NONUNIQNUM	"Group id \"%s\" for group \"%s\" is not unique.\n"
#define DEF_BADADMS	"Administrator \"%s\" is not a valid user id.\n"
#define DEF_BADADMIN	"Invalid admin value in /etc/security/group for \"%s\".\n"
#define	DEF_BADOPENEG	"Could not open /etc/group, errno: \"%d\".\n"
#define	DEF_BADOPENESG	"Could not open /etc/security/group, errno: \"%d\".\n"
#define	DEF_NOMEM	"Out of core memory, errno: \"%d\".\n"
#define	DEF_BADPUT	"Error writing back to user database, errno: \"%d\".\nDo you wish to discard changes and exit grpck? \n"
#define DEF_FIXAGRPS	"Remove non-existent administrative users for \"%s\" ? "
#define DEF_FIXUSERS	"Remove non-existent users for \"%s\" ? "
#define DEF_NOETCGRP	"Missing stanza for \"%s\" in /etc/group.\n"
#define DEF_NOSECGRP	"Missing stanza for \"%s\" in /etc/security/group.\n"
#define DEF_BADFORMAT	"Bad format \"%s\" in line %d of /etc/group.\n"
#define DEF_NULLNAME	"Null group name in line %d of /etc/group.\n"
#define DEF_OVERWRITE	"The database has been changed by another process.\nDo you wish grpck to overwrite those changes? \n"
#define DEF_OLDDBM	"The DBM files for /etc/passwd are out of date.\nUse the mkpasswd command to update these files.\n"
#define DEF_YPGRP	"\"%s\" appears to be a yp entry and is not being checked.\n"

#define MAXATTRSIZ 4096
