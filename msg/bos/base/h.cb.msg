$ ENGLISH_SOURCE_VERSION= 1.4
$ @(#)36        1.1 2/9/94 09:00:33
$
$ COMPONENT_NAME: CMDSCCS       source code control system
$
$ FUNCTIONS:
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

$
$quote "
$set 1
1 "Cannot use the comb command while there is an outstanding delta\n\
\tto the file.\n\
\tUse the delta command or the unget command; then use the comb command.\n\
\tA p-file exists which shows that someone is in the process of making a\n\
\tdelta on the specified SCCS file.  The comb command may only be used when\n\
\tno one else is modifying the specified SCCS file.\n"
$set 2
1 "The -p flag and the -c flag are mutually exclusive.\n\
\tThe -p flag specifies the SID of the oldest delta for the resulting procedure\n\
\tto preserve. All older deltas are combined in the reconstructed file.\n\
\tThe -c flag specifies a list of deltas (SIDs) that the shell procedure will\n\
\tpreserve. The procedure combines all other deltas.\n"
$set 4
1 "The flags specified on the command line would create no changes\n\
\tto the SCCS file.\n\
\tThe comb command as specified cannot do anything to the specified SCCS file.\n"

