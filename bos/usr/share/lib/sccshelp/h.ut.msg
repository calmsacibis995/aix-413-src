$ @(#)07        1.1 9/12/91 07:01:24
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
$ (C) COPYRIGHT International Business Machines Corp. 1991
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$quote #
$set 1
1 #"directory `...' does not exist"\n\
The directory was not found.  Check for typos.#
$set 2
1 #"directory `...' unwritable"\n\
The real or effective user does not have write permission in the directory.\n\
Do a "ls -ld ..." command to check the modes.#
$set 3
1 #"file table full"\n\
The system file table is full, and temporarily no more opens can be accepted.\
\nIf this error recurs, contact your system support personnel.#
$set 4
1 #"`...' does not exist"\n\
The file does not exist.  Check for typos.#
$set 5
1 #"`...' unreadable"\n\
The file probably exists, but you do not have read permission.\n\
It is also possible that one or more directories in the path\n\
is not readable or searchable.#
$set 6
1 #"`...' unwritable"\n\
The file exists, but you do not have write permission.\n\
It is also possible that one or more directories in the path\n\
is not readable or searchable.#
$set 7
1 #"`...' unreadable or unwritable"\n\
The file exists, but you do not have read permission,\n\
or write permission, or both.\n\
It is also possible that one or more directories in the path\n\
is not readable or searchable.#
$set 8
1 #"file too big"\n\
The being written reached the file size limit (default 1 megabyte).#
$set 9
1 #"out of space"\n\
There is no more dynamic memory left.\n\
Try reducing the size or complexity of something.#
$set 10
1 #"no space!"\n\
The file system (on which the file which caused the error is being written)\n\
is out of space.  This is very serious!!\n\
If you can remove any files, do so.#
$set 11
1 #"... = ..., function = `...'"\n\
An unusual and unexpected error occurred.\n\
Check "man 2 intro".  If this does not explain the problem,\n\
contact your system support personnel.#
$set 12
1 #"SIGNAL: ..."\n\
Some unexpected condition occurred, either a hardware, system, or software\n\
error.  Consult "man signal", and if the cause is not evident, report\n\
this to your system support personnel.\n\
\n\
(If required, a core dump will be produced if the file "dump.core" exists\n\
in the current directory.)#
$set 20
1 #"cannot link `...' to `...'"\n\
An attempt was made to rename a file,\n\
but a file with the target name already exists.#
$set 21
1 #"cannot link `...' to `...'"\n\
An attempt was made to rename a file,\n\
but the target name is on a different device than the file being renamed.#

