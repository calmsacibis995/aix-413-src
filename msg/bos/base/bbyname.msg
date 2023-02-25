$ ENGLISH_SOURCE_VERSION= 1.16.4.1
$ @(#)96	1.2  src/msg/bos/base/bbyname.msg, bosmsg, msg411, 9433A411a 8/11/94 12:40:46
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
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ $ Error numbers 75-99, 430-449, 750-799 are reserved for this file.
$
$quote "
$set MS_BBYNAME

ANSYORN "Do you want to continue? (yes or no)  "
$ Leave space at end of this line.

BACKERR "backup: 0511-075 Backup header type %d is not recognized.\n"
$ $ A problem occurred while re-ordering the header.
$ %d is the backup header type.

BADLIM "backup: 0511-081 The -l option value must be a non-zero multiple of %d,\
\n        the number of sectors per track.\n"

BADREAD "backup: 0511-076 Cannot read the specified file.\n"

BADREGX "backup: 0511-077 %s is not a valid regular expression.\n"
$ %s was specified as a regular expression.

BAUPTO "Backing up to %s.\n"
$ %s is the media.

BLKSVOL "there are %1$ld blocks on %2$d volumes.\n"
$ This message appears after the DONETIM message.

CANTSY "backup: 0511-078 Cannot read the symbolic link.\n"
$ This message precedes the perror routine which names the link and gives
$ the reason it cannot be read.

CLUST "Cluster %1$d bytes (%2$d blocks).\n"
$ This is an informational message.
$ %1$d is the number of bytes in the cluster.
$ %2$d is the number of blocks in the cluster.

DONETIM "Backup finished on %s; "
$ %s is the day of the week, date, time of day, and year.
$ This message appears before the BLKSVOL message.

FILTOL "backup: 0511-080 %s: The path name is too long.\n"
$ %s is a path name.

IGNDISK "backup: 0511-082 Specify the -l flag for disk media only.\n"

MEDIAERR "backup: 0511-079 The backup media may be damaged.\n\
\tUse a different disk to run the backup.\n"

MNT "Mount volumes %1$d - %2$d on %3$s - %4$d.\n\
\tPress Enter to continue. "
$ %1$d-%2$d are disk volumes.
$ %3$s-%4$c are disk drives.

MOUNT "Mount volume %1$d on %2$s.\n\
\tPress Enter to continue. "
$ %1$d is the volume number.
$ %2$s is the disk drive.

OPNER "backup: 0511-089 Cannot open %1$s: "
$ %1$s is the name of the archive device.
$ The perror mesasge follows this message with explanation of error.

QUIT5 "backup: 0511-090 There is no space for the output buffer.\n"

REDUCED "Cluster size %1$d is too large. It is reduced to %2$d.\n"
$ %1$d is a cluster size.
$ %2$d is the cluster size to which %1$d is reduced.

SHMDT "backup: 0511-097 The shmdt call failed.\n"
$ Do not translate shmdt.

SYLNTO "backup: 0511-098 %s: The symbolic link is too long.\n"
$ %s is a symbolic link.

VFSEEK "backup: 0511-430 A seek error occurred"
$ Do not translate seek
$ This will be followed by a : which will be followed by a perror.

VFREAD "backup: 0511-431 A read error occurred"
$ Do not translate read
$ This will be followed by a : which will be followed by a perror.

VFWRIT "backup: 0511-432 A write error occurred"
$ Do not translate write
$ This will be followed by a : which will be followed by a perror.

VOLUME "Volume %1$d on %2$s\n"
$ %1$d is the volume number.
$ %2$s is the media.
$ This message follows the CLUST message if the -v (verbose) flag is on.

INO "backup: 0511-433 %s inode is too large for AIXv2: %ld.\n"
$ %s is the file name.

MODE "backup: 0511-434 %s mode value is too large for AIXv2: %ld.\n"
$ %s is the file name.

NLINK "backup: 0511-435 %s nlink value is too large for AIXv2: %ld.\n"
$ %s is the file name.

UID "backup: 0511-436 %s uid is too large for AIXv2: %ld.\n"
$ %s is the file name.

GID "backup: 0511-437 %s gid is too large for AIXv2: %ld.\n"
$ %s is the file name.

MAJDEV "backup: 0511-438 %s major device number\n\
\tis too large for AIXv2: %ld.\n"
$ %s is the file name.

MINDEV "backup: 0511-439 %s minor device number\n\
\tis too large for AIXv2: %ld.\n"
$ %s is the file name.

MAJRDEV "backup: 0511-440 %s remote major device number\n\
\tis too large for AIXv2: %ld.\n"
$ %s is the file name.

MINRDEV "backup: 0511-441 %s remote minor device number\n\
\tis too large for AIXv2: %ld.\n"
$ %s is the file name.

DONTBKUP "backup: 0511-442 %s will not be backed up.\n"
$ %s is the file name.
$ This message is followed by the ANSYORN msg.

NOABUF "backup: 0511-443 There is not enough memory for the Access Control List.\n"

NOPBUF "backup: 0511-444 There is not enough memory for the Process Control List.\n"

VERS2 "Version 2 Backup in Progress.\n"

SOCK_TYPE "backup: %s socket is not backed up.\n"

INTERN "backup: 0511-445 An internal packing error occurred.\n"

CAPAC "backup: 0511-448 Diskette capacity must be consistent for all drives.\n"

INPUTERR "backup: 0511-446 An error occurred reading from standard input.\n"

ERRCLOSE "backup: 0511-447 An error occurred closing "
$ This message is followed by a perror() message contain the media name.

NOACCESS "backup: 0511-449 An error occurred accessing "
$ This message is followed by a perror() message contain the media name.

TOTSIZ "The total size is %d bytes.\n"
$ %d is the total size of files restored or listed.
