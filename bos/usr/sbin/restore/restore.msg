$ @(#)51	1.20  src/bos/usr/sbin/restore/restore.msg, cmdarch, bos411, 9428A410j 10/12/93 09:37:32
$
$ COMPONENT_NAME: (CMDARCH) archive files
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989,1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ $ Message numbers 150-161, 260-273 reserved for this file.
$
$quote "
$set MS_RESTORE

EXECF "restore: 0511-153 Cannot run %s.\n"
$ %s is /etc/restbyname or /etc/restbyinode.

BADFMT "restore: 0511-260 The archive is not in backup format.\n"

BADOP1 "restore: 0511-154 Cannot use the %c flag to restore backups by name.\n"
$ %c is a flag.

BIGBLK "restore: 0511-261 Blocks required (%d) to read tape blocksize of (%d)\n\
\t exceeds maximum number of blocks (%d) allowed by restore.\n"

BADOP2 "restore: 0511-155 Cannot use the %c flag to restore backups by i-node.\n"
$ %c is a flag.

EOFERR "restore: 0511-262 Read an unexpected end of file.\n"

IOCTLE "restore: 0511-156 Cannot get device information.\n"

IOCTLT "restore: 0511-157 Cannot skip tape records.\n"

MFLAG "restore: 0511-267 The -x or -X option must be specified when using the\n\
\t-M option.\n"

MNTV "Please mount volume %1$d on %2$s.\n\
\tPress the Enter key to continue.\n"
$ %1$d is the volume number.
$ %2$s is the device.

NODRIVE "restore: 0511-158 Cannot open %s: "
$ %s is media.
$ A perror message follows which gives the reason that restore
$ cannot open the media.

NOMEM "restore: 0511-263 There is not enough memory available at this time.\n"

NOREW "restore: 0511-264 Use a no-rewind, no-retension tape device, such as\n\
\t /dev/rmt0.1, with the -s option.\n"

NOSTDIN "restore: 0511-266 Cannot read user response from standard input.\n"

BADREW "restore: 0511-265 The no-rewind tape device specified is not valid.\n\
\tFor restore, the no-rewind device specification must be either .1 or .5.\n"
$ both of the 'restore' words in this message refer to the command name and
$ should not be translated.

PFLAG "restore: 0511-159 Specify the -p flag for tape devices only.\n"

READERR "restore: 0511-160 Cannot read the backup media\n\t "

SKIPE "restore: 0511-161 The skip option is available for tape media only.\n"

SKIPINV "restore: 0511-150 Specify a skip number between 1 and 100.\n"

STATFAIL "restore: 0511-151 Cannot get information about %s.\n"
$ %s is media.

TOOLRG "Cluster size %1$d is too large; reduced to %2$d.\n"
$ %1$d is the cluster size that is too large.
$ %2$d is the size to which the cluster is reduced.
$ This is an informational message.

TOOSML "Cluster size %1$d is too small; increased to %2$d.\n"
$ %1$d is the cluster size that is too small.
$ %2$d is the size to which the cluster is enlarged.
$ This is an informational message.

USAGE "Usage for Backup by Name:\n"

USAGE1 "restore -x[Mdqv] [-b Number] [-f Device] [-s Number] [File ...]\n\
\tExtracts files by name.\n"

USAGE2 "restore -T|-t [-qv] [-b Number] [-f Device] [-s Number]\n\
\tLists a table of contents or information about the backup.\n"

USAGE3 "restore -X Number [-Mdqv] [-b Number] [-f Device] [-s Number] [File ...]\n\
\tExtracts beginning at a specified volume number.\n"

USAGE4 "Usage for Version 2 Backup by Inode:\n"

USAGE5 "restore -r[d] [-f Device] [File ...]\n"

USAGE6 "Usage for Backup by Inode:\n"

USAGE7 "restore -t[Bhqvy] [-b Number] [-f Device] [-s Number] [File ...]\n\
\tLists a table of contents.\n"

USAGE8 "restore -x[Bhmqvy] [-b Number] [-f Device] [-s Number] [File ...]\n\
\tExtracts files by name.\n"

USAGE9 "restore -i[hmqvy] [-b Number] [-f Device] [-s Number]\n\
\tRestores files interactively\n"

USAGE10 "restore -r[Bqvy] [-b Number] [-f Device] [-s Number]\n\
\tRestores full file systems.\n"

USAGE11 "restore -R[Bvy] [-b Number] [-f Device] [-s Number]\n\
\tRestores full file systems.\n"

BADP "restore: 0511-152 Specify the -p flag for tape media only.\n"

CHBLKSZ "Changing user specified number of blocks to %d.\n"
 
MNT "Please mount volumes %1$d - %2$d on %3$s - %4$d.\n\
\tPress the Enter key to continue.\n"
