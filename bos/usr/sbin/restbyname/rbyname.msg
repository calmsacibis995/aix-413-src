$ @(#)26	1.20.1.9  src/bos/usr/sbin/restbyname/rbyname.msg, cmdarch, bos411, 9428A410j 6/10/94 17:00:19
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
$ $ Error numbers 100-149, 700-749 are reserved for this file.
$
$quote "
$set MS_RBYNAME

ABORT "The restore is complete.\n"
$ The OBSODS message precedes this message.

ARCIVD "The number of archived files is %u.\n"
$ %u is the number of files on the archive.

ASS0CNT "Ignoring data and continuing.\n"
$ This message follows the RDER2 message.

BADEOF "restore: 0511-701 A unexpected end of file condition has occurred.\n\
\t There is no writer on the input pipe.\n"

BADOPEN "restore: 0511-104 Cannot open file.\n"
$ The user receives the BADREAD message with this message also.

BADREAD "restore: 0511-105 Cannot read the specified file.\n"
$ The perror message follows this message.

BDNBW "restore: 0511-106 The backup format does not contain a volume record.\n"
$ This message precedes either the MNT or the MOUNT message.

BRDOP "restore: 0511-107 Cannot read from pipe. There is an unpacking error.\n"
$ Do not translate pipe, unpacking

CKSUMR "Checksum error\n"
$ The user will never see this message. It is here for debugging purposes.

CLTOOLRG "Warning: Cluster size %1$d is too large; reduced to %2$d.\n"
$ %1$d is the cluster size that is too large.
$ %2$d is the size to which the cluster is reduced.
$ This is an informational message.

CLTOOSML "Cluster size %1$d is too small; increased to %2$d.\n"
$ %1$d is the cluster size that is too small.
$ %2$d is the size to which the cluster is enlarged.
$ This is an informational message.

CLUST "Cluster size is %1$d bytes (%2$d blocks).\n"
$ %1$d is the cluster size in bytes.
$ %2$d is the cluster size in blocks.
$ This message appears when restore is invoked with the -v (verbose) flag.

CREATE "restore: 0511-702 Cannot open or create "
$ This message is followed by a perror message containing the filename

DATEB "The backup date is: %s"
$ %s is the backup date.
$ This message appears when restore is invoked with the -v (verbose) flag.

DFSYS "Disk %1$s, File system %2$s, User %3$s\n"
$ %1$s is the disk number.
$ %2$s is the file system.
$ %3$s is the user.
$ This message appears when restore is invoked with the -v (verbose) flag.

EDUNP "restore: 0511-108 There was an error during the unpacking of %s.\n"
$ %s is a file name.
$ This message is informational only.

ENDOD "restore: 0511-109 Reached the end of the backup.\n"
$ This message appears if there is an error condition and if the program
$ encounters the end record. restore terminates.

EXTRC "Extracting %s."
$ %s is a file.
$ This message is followed by the INODEN or the NOTON message.

FBFRM "Files are backed up from: %s."
$ %s is the date and time of the backup.

FBKIN "Files are backed up by inode.\n"
$ This message can appear when restore is invoked with the -v (verbose) flag.

FBKNM "Files are backed up by name.\n"
$ This message can appear when restore is invoked with the -v (verbose) flag.

FORGZ "The original size of %1$s is %2$d; restored size is %3$d.\n"
$ %1$s is the file name. %2$d is the original size. %3$d is the restored size.
$ This message comes after UPCSZ.
$ The x variable is replaced by a hexadecimal value.
$ This is an informational message.

FOUTP "restore: 0511-110 There is an unpacking error.\n"
$ Do not translate unpacking.

FSDSER "restore: 0511-111 Cannot read the FS_DS header.\n"
$ Do not translate FS_DS.

FTOBE "Files to be extracted: %d.\n"
$ %d is the number of files to be extracted.
$ This message appears when restore is invoked with the -v (verbose) flag.

FWRERR "restore: 0511-112 There is a write error on the internal temporary file.\n"

INODEN "Inode number %u.\n"
$ This message appears with the EXTRC message.
$ This is an informational message. The program extracts the file into the
$ inode number of that file.

INTERR "restore: 0511-113 Internal error; the read request is too large.\n"

INTERRM "restore: 0511-114 There is an internal error on the input medium.\n"

INUNP "restore: 0511-115 There is an internal unpacking error.\n"

IOCTLERR "restore: 0511-116 Cannot reverse tape record.\n"

LNKTO "%1$s\nis linked to %2$s.\n"
$ This is an informational message.
$ %1$s and %2$s are files.
$ This message is preceeded by an "x" character and possibly the size of
$ the file being extracted or listed.

MINFM "restore: 0511-117 Cannot restore %s.\n\
\tMinidisk format is no longer supported.\n"
$ %s is a minidisk.

MISSHDR "restore: 0511-119 There is a missing header block.\n"
$ This message precedes RSYNC.

MNT "Mount volumes %1$d through %2$d on %3$s through %4$d.\n\
\tPress the Enter key to continue.\n"
$ %1$d and %2$d are disk volumes.
$ %3$s and %4$c are disk drives.

MOUNT "Mount volume %1$d on %2$s.\n\
\tPress the Enter key to continue.\n"
$ %1$d is the volume number.
$ %2$s is the disk drive.

NEWVL "New volume on %s:\n"
$ %s is media.
$ This message appears when restore is invoked with the -v (verbose) flag.
$ The CLUST message follows this message.

NOFORM "restore: 0511-120 The backup format is not recognized.\n"

NOMKDIR "restore: 0511-705 Cannot make directory '%s': "
$ This message appears before a 'perror()' message.
$ %s is the name of a directory.

NOSPC "restore: 0511-121 Out of space.\n"
$ This message appears before the RHRZS message.

NOTEMP "restore: 0511-122 Cannot open temporary file %s.\n"

NOTON "File does not exist on the backup media.\n"
$ This message goes with the EXTRC message.

OBSODSN "restore: 0511-124 This is a Distributed Services remote backup.\n\
\tCannot restore remotely. Enter <y> to continue locally: "
$ This message appears before the ABORT message if the user answers no.
$ Please leave a space at the end of the prompt for answer.

ONL1ST "restore: 0511-125 Only the first %d directories will be searched.\n"
$ %d is the number of directories (3,000).

OPNER "restore: 0511-126 Cannot open %s: "
$ A perror message follows this message.
$ %s is the name of the device.

OPTNC "restore: 0511-127 Cannot specify the -%1$c flag with the -%2$c flag.\n"
$ -%1$c and -%2$c are flags.

OPTNX "restore: 0511-128 Cannot specify the %s flag and the -X flag.\n"
$ %s is a flag.

PIPEF "restore: 0511-129 Cannot create pipe.\n"
$ Do not translate pipe

PRREORD "restore: 0511-130 Header type %d is not recognized.\n"
$ %d is a header type.

QUIT5 "restore: 0511-131 There is no space for the output buffer.\n"

QUIT6 "Sector limit %1$d is not a multiple of cluster %2$d.\n"
$ %1$d is a sector limit.
$ %2$d is a cluster of which the sector limit is not a multiple.

RDER1 "restore: 0511-132 Data read error on sector %d; data is ignored.\n"
$ %d is a sector number.

RDER2 "restore: 0511-133 There is a data read error."
$ This message precedes the ASS0CNT message.

RENAME "restore: 0511-704 Cannot restore the file "
$ This message is followed by a perror message containing the filename

RESNCBM "Starting again at the bit map.\n"
$ This message follows the RSYNC message.
$ Do not translate bit map.

RESNCCL "Starting again at the clear inode map.\n"
$ This message follows the RSYNC message.
$ Do not translate clear inode map.

RESNCF "Starting again at file %s.\n"
$ %s is a file.
$ This message follows the RSYNC message.

RESNCI "Starting again at inode %d.\n"
$ %d is an inode.
$ This message follows the the RYSYNC message.

RESTRD "The number of restored files is %u.\n"
$ %u is the number of restored files.
$ This informational message appears at the end of the restore.

RESYNC "restore: 0511-134 Starting again at the file index.\n"

RHRUL "restore: 0511-135 Reached the user file size limit of %1$d blocks\n\
\twhile trying to restore file %2$s.\n"
$ Do not translate ulimit
$ %1$d is the file size limit in blocks.
$ %2$s is the file.

RHRZS "restore: 0511-136 Ran out of free space while trying to restore %s.\n"
$ %s is a file.

RSYNC "restore: 0511-137 Trying to find the next header. Data may be lost.\n"

RUTWF "restore: 0511-138 Cannot write to file %s.\n"
$ %s is a file.

SDMTM "Restore is searching directories. This may take several minutes.\n"

SEEKERR "restore: 0511-139 Seek error wd = %d\n"
$ %d is the wd number.
$ wd is the postion within the internal buffer. Leave this message as it is,
$ according to James Wang.
$ Do not translate wd.

SHMATF "restore: 0511-141 Cannot attach shared memory.\n"

SHMGETF "restore: 0511-142 Cannot get a shared memory segment.\n"

SIGRE "External interrupt received. Restore is ending.\n"

STATFAIL "restore: 0511-140 Cannot get information about %s.\n"
$ %s is input medium.

SYLNFL "restore: 0511-143 Cannot create a symbolic link.\n"
$ The perror message follows this message.

TMPCREAT "restore: 0511-703 Cannot open or create a temporary file to\n\
\t restore "
$ This message is followed by a perror message containing the filename

TTYERR "restore: 0511-118 An error occurred reading from the tty.\n"

UNKHDR "restore: 0511-144 Header type %d is not recognized and is ignored.\n"
$ %d is a header type.
$ The restore program continues after this message.

UNKIFMT "restore: 0511-145 File type %o is not recognized.\n"
$ %o is an octal file type.

UNKNHD "restore: 0511-146 Header type %d is not recognized and is ignored.\n"
$ %d is a header type.

UPCSZ "restore: 0511-147 Unpacking file size error.\n"
$ This message precedes the FORGZ message.

USERA "The user is %s.\n"
$ %s is the user name.
$ This message follows the FBKNM message.

VOLBU "restore: 0511-123 The volume on %s is not in backup format.\n"
$ %s is media.

VOLNM "The volume number is %d.\n"
$ %d is the volume number.
$ This message appears when restore is invoked with the -v (verbose) flag.

WRONG "restore: 0511-103 The volume is not correct. Restore expects volume %d.\n"
$ %d is the volume number.
$ This message precedes the DATEB message.

EACLIMP "restore: 0511-102 Import of Access Control Lists failed.\n"

EPCLIMP "restore: 0511-101 Import of Process Control Lists for file %s failed.\n"

USAGE "Usage for Backup by Name:\n"

USAGE1 "restore -x[MDqv] [-b Number] [-f Device] [-s Number] [File ...]\n\
\tExtracts files by name.\n"

USAGE2 "restore -T|-t [-qv] [-b Number] [-f Device] [-s Number] [File ...]\n\
\tLists a table of contents.\n"

USAGE3 "restore -X Number [-Mdqv] [-b Number] [-f Device] [-s Number] [File ...]\n\
\tExtracts beginning at a specified volume number.\n"

USAGE4 "Usage for Version 2 Backup by Inode:\n"

USAGE5 "restore -r[d] [-f Device] [File ...]\n"

USAGE6 "Usage for Backup by Inode:\n"

USAGE7 "restore -t[Bhvy] [-b Number] [-f Device] [-s Number] [File ...]\n\
\tLists a table of contents.\n"

USAGE8 "restore -x[Bhmvy] [-b Number] [-f Device] [-s Number] [File ...]\n\
\tExtracts files by name.\n"

USAGE9 "restore -i[hmvy] [-b Number] [-f Device] [-s Number]\n\
\tRestores files interactively\n"

USAGE10 "restore -r[Bvy] [-b Number] [-f Device] [-s Number]\n\
\tRestores full file systems.\n"

USAGE11 "restore -R[Bvy] [-b Number] [-f Device] [-s Number]\n\
\tRestores full file systems.\n"

IOCTLT "restore: 0511-100 Cannot skip tape record.\n"

QUIT7 "restore: 0511-148 There is no space for an internal buffer.\n"

NAMETOLONG "restore: 0511-149 %d exceeds maxmimum path length.\n"

CAPAC "restore: 0511-700 Diskette capacity must be consistent for all drives\n"

TOTSIZ "The total size is %u bytes.\n"
$ %u is the total size of files restored or listed.
$ This message appears before the RESTRD or ARCIVD messages.
