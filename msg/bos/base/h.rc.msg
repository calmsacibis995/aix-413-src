$ ENGLISH_SOURCE_VERSION= 1.4
$ @(#)51        1.1 2/9/94 09:01:28
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

$quote #
$set 1
1 #Use the -r flag with this command.\n\
You did not specify the -r flag when you used the rmdel or\n\
the cdc command. This flag is always required.\n#
$set 4
1 #You must be the owner of the file or the creator of the delta\n\
to use this command.\n\
Either you do not own the file you are trying to process or\n\
your user name is not the same as that of the user who created the delta.\n\
Only the creator of a delta or the owner of the SCCS file can use the\n\
rmdel or the cdc command with the specified delta.\n#
$set 5
1 #You can remove only the most recently created delta on a branch,\n\
or the latest trunk delta if it has no branches.\n\
You have attempted to remove a delta that is not the newest (most\n\
recent) delta in its branch of the delta tree. Only the newest delta can\n\
be removed.  To remove this delta, you must first remove any deltas that\n\
come after it.\n#
$set 9
1 #You cannot remove this delta or change its commentary.\n\
The delta contains an MR (Modification Request) number which has\n\
been marked \"delivered\" (unchangeable).\n\
The delta you are processing with rmdel or cdc contains an MR entry\n\
which has been marked \"delivered\" (the last character is an *).\n\
You are not allowed to change or remove such a delta.\n#
$set 10
1 #You cannot invoke rmdel or cdc except by the commands\n\
rmdel or cdc.\n\
You invoked rmdel or cdc by some name other than rmdel or cdc.\n\
The program's execution depends on being invoked by one of these names only.\n#
$set 11
1 #Specify an SID with the -r flag.\n\
You specified the -r flag, but did not specify the SID of the\n\
delta to be processed.  Both are required.\n#
$set 12
1 #Another user is working on the delta you specified.\n\
Wait until the delta is not in use.\n\
Someone is in the process of making a delta based upon the same delta (SID)\n\
you specified with the -r flag. You cannot remove the delta at this time\n\
because its removal would interfere with other work in progress.\n#
