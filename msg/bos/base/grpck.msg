$ ENGLISH_SOURCE_VERSION= 1.10
$
$ COMPONENT_NAME: (CMDSADM) security: system administration
$
$ FUNCTIONS: none
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$set MS_SCCSID

$ This is the SCCSID string.  It needs to be a message so it will
$ show-up as the output of the what command.

M_SCCSID        "@(#)31 1.1  src/msg/bos/base/grpck.msg, bosmsg, msg411, GOLD410 2/9/94 09:00:13"

$set MS_GRPCK

$
$ Messages for the GRPCK command
$

$quote "

M_USAGE		"3001-221  grpck [ -p | -y | -n | -t ] [ group(s) ... | ALL ]"
M_BADCHAR	"3001-222  Bad character in group name \"%s\".\n"
M_BADNAME	"3001-223  Invalid group name \"%s\".\n"
M_BADETCREQ	"3001-224  No stanza in /etc/group for requested group \"%s\".\n"
M_BADSECREQ	"3001-225  No stanza in /etc/security/group for requested group \"%s\".\n"
M_BADGROUP	"3001-226  Bad group id \"%s\" in \"%s\".\n"
M_BADUSER	"3001-227  User name \"%s\" not found in password file.\n"
M_NONUNIQNAM	"3001-228  Group name \"%s\" is not unique.\n"
M_NONUNIQNUM	"3001-229  Group id \"%s\" for group \"%s\" is not unique.\n"
M_BADADMS	"3001-230  Administrator \"%s\" is not a valid user id.\n"
M_BADOPENEG	"3001-231  Could not open /etc/group, errno: \"%d\".\n"
M_BADOPENESG	"3001-232  Could not open /etc/security/group, errno: \"%d\".\n"
M_BADADMIN	"3001-233  Invalid admin value in /etc/security/group for \"%s\"\n"
M_NOMEM	 	"3001-234  Out of core memory.\n"
M_BADPUT	"3001-235  Error writing to user database, errno:\"%s\".\n          Do you wish to discard changes and exit grpck? \n"
M_FIXAGRPS	"3001-236  Remove non-existent administrative users for \"%s\" ?\n"
M_FIXUSERS	"3001-237  Remove non-existent users for \"%s\" ?\n"
M_NOETCGRP	"3001-238  Missing stanza for \"%s\" in /etc/group.\n"
M_NOSECGRP	"3001-239  Missing stanza for \"%s\" in /etc/security/group.\n"
M_BADFORMAT	"3001-240  Bad format \"%s\" in line %d of /etc/group.\n"
M_NULLNAME	"3001-241  Null group name in line %d of /etc/group.\n"
M_OVERWRITE	"3001-242  The database has been changed by another process.\n          Do you wish grpck to overwrite those changes? \n"
M_OLDDBM	"3001-243  The DBM files for /etc/passwd are out of date.\n          Use the mkpasswd command to update these files.\n"
M_YPGRP		"3001-244  \"%s\" appears to be a yp entry and is not being checked.\n"
