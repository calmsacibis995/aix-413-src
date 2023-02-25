$ ENGLISH_SOURCE_VERSION= 1.9
$ @(#)70	1.1  src/msg/bos/base/rcp.msg, bosmsg, msg411, GOLD410 2/9/94 09:13:21
$ 
$ COMPONENT_NAME: TCPIP rcp.msg
$ 
$ FUNCTIONS: malloc, read 
$
$ ORIGINS: 10  26  27 
$
$ (C) COPYRIGHT International Business Machines Corp. 1985, 1989
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for rcp
$

$set MS_RCP

$quote "

NOACCT		"rcp: %s does not have an account on this machine\n"
$				User does not have an account
UNKSERV         "rcp: shell/tcp: unknown service\n"
$				Shell/tcp service is not availalable (getservbyname())
IDERR           "who are you?\n"
$				Can't find a password file entry for the user.
USAGE           "Usage: rcp [-p] f1 f2; or: rcp [-rp] f1 ... fn d2\n"
$				Incorrect command option.
RCPERR          "rcp: %s: %s.\n"
$				Expected a directory name in the argument. Prints the
$				argument value followed by system error message for ENOTDIR.	
USERNMERR       "rcp: invalid user name %s\n"
$				Argument contains illegal username characters.
FNMERR          "rcp: %s: %s\n"
$				Can't open the source filename. Prints the filename followed
$				by the system error message.
FILERR          "rcp: %s: not a plain file\n"
$				Expected a plain file argument (not a special/block file).
$				Prints argument name.
FILSZ           "rcp: %s: %s\n"
$				During a copy, the number of bytes read did not equal the
$				amount requested. Prints file name.
RCPDIR          "rcp: %s: %s\n"
$				Could not open a source directory. Prints the directory
$				name followed by the system error message.
NMTOOBIG        "%s/%s: Name too long.\n"
$				A source file name is too long for the system. 
$				Prints the directory name followed by the file name. 
$				Copy continues skipping the file.
LOSTCONN        "rcp: lost connection\n"
$				Communication between host/remote lost.
AMBTGT          "rcp: ambiguous target\n"
$				Target files/directory not entered.
UNEXP           "unexpected '\\n'"
$				First char read from remote equaled a CR, not part of the
$				rcp convention.
LOSTCON2        "lost connection"
$				Occurs when read request of 1 byte fails.
SECDELIM        "mtime.sec not delimited"
$				When set file times option specified, the received modfication
$				millisecond	time delimiter is missing.
MSECDELIM       "mtime.usec not delimited"
$				Same as above except microseconds.
ATIMDELIM       "atime.sec not delimited"
$				Same as above ecept access time seconds.
ATIMMSDELIM     "atime.usec not delimited"
$				Same as above except access microseconds.
SHMSG			"%s\n"
$				Error or other message from remote shell.
CTRLREC         "expected control record"
$				The first byte of the record did not match any of the
$				control record identifiers expected.
BADMODE         "bad mode"
$				The mode bytes following the identifier were not octal digits.
MODELIM         "mode not delimited"
$				No space (' ') char following mode.
SZDELIM         "size not delimited"
$				No space char following size.
TIMSET          "rcp: can't set times on %s: %s\n"
$				Could set new times for a file or directory. Prints the file or
$				directory name and sys_errlist message.
$				Message is used in two places (files/directories).
CREATERR		"rcp: %s: %s\n"
$				Can't create new file. Prints name and sys_errlist message.
LOSTCON3        "rcp: dropped connection"
$				read() returned 0 indicating a lost connection.
RDERR			"rcp: %s\n"
$				read() during copy returned an error. Prints sys_errlist
$				message.
WRTERR          "rcp: error in writing to %s: check for ulimit \n"
$				During a file copy, the bytes written did not equal that
$				requested. Prints the file name and sys_errlist message.
PROTOERR        "rcp: protocol is at fault %s\n"
$				This message is prefaced with the specific protocol error
$				printed earlier. The current path name is printed followed 
$				by an exit.
STATERR         "rcp: fstat: %s\n"
$				Error on call to stat. Prints sys_errlist message.
MEMERR          "rcp: malloc: out of memory\n"
$				malloc() call failed.
NOSPACE		" ERROR:  could not allocate space needed.  Try again\n"
MISSCOLON	"Missing semicolon on hostname\n "
NOROOM		"Not enough room in get_user() to allocate username space\n"
NOACCOUNT	"%s does not have an account on this machine\n"
NME_NT_FND      "host: name %s NOT FOUND\n"
BADADDR		"%s: address misformed\n"
BADHOST		"%s: host not found\n"
OK_MBTOWC	"okname: Error converting to widechar (%s)\n"
