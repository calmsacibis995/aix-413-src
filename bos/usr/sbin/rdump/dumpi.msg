$ @(#)35	1.35  src/bos/usr/sbin/rdump/dumpi.msg, cmdarch, bos411, 9428A410j 10/21/93 12:50:12
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
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1993
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ $ Message numbers 200-259, 800-824 reserved for this file.
$
$quote "
$set MS_DUMP
$ Do not translate dump or backup

ABORTDUM "Do you want to end the backup? "
$ Leave a space at the end of the message.
$ This message precedes the YESNO message.
$ The user receives this message after giving an interrupt signal.

ABORTING "0511-202 Internal error; received a segmentation violation.\n"

BADCODE "0511-203 Internal error; exit status %d is not recognized.\n"
$ %d is the exit status.

BADFS "0511-204 Cannot get file system status.\n"

BADFSIZE "0511-803 The superblock filesystem size is corrupted.\n"

BADCOMPRESS "0511-804 Unknown compression algorithm.\n"

BADKEY "0511-205 %c is not a recognized flag.\n\
\tUsage: rdump [-b Number1] [-c] [-l Number2] [-u] [-w|-W]\n\
\t       [-Level] -f Machine:Device [Filesystem]\n"
$ Do not translate %c%.

BADRANGE "0511-213 The device range specified is not valid.\n"

BREADERR "0511-250 There are more than %1$d block read errors from %2$s.\n"
$ %1$d is the maximum number of read errors allowed.
$ %2$s is the disk.
$ This message precedes UNRECERR and CONTINUE

CANTGINO "0511-252 Cannot get block for inode %d.\n"

CANTODISK "0511-206 Cannot open %s.\n"
$ %s is the disk on which the file system resides.

CANTODT "0511-207 Cannot open %s for backup table information.\n"
$ %s is /etc/filesystems.

CANTOUTM "0511-208 Cannot open the /etc/utmp file.\n"
$ Do not translate /etc/utmp

CANTREC "Cannot recover. Start again with different media.\n"
$ This message follows the MEDIAWRE message.

CANTRESB "0511-253 Cannot read superblock on %s.\n"
$ %s is the disk on which the file system resides.

CHANGM "Change Media: Insert volume number %d.\n"
$ %d is the disk volume number.
$ This message precedes the CHANGM2 message.

CHANGM2 "Change media now.\7\7\n"
$ Do not translate \7, this is an octal value that causes a bell to ring.
$ This message follows the CHANGM message.

CHILDR "Child process %1$d returns status %2$o.\n"
$ %1$d is a child process.
$ %2$o is the status.
$ This is an informational message.

CONTINU "Do you want to attempt to continue? "
$ The YESNO message follows this message.
$ There is no new line after this message. Leave a space at the end.

CORDIR "The directory is damaged at inode number %d.\n"
$ %d is the inode number.
$ This is an informational message. There is no recovery; the backup continues.

CORRUPTJFS "0511-802 The %s JFS filesystem superblock is corrupted.\n"
$ %s is the disk on which the file system resides.

DONE "The backup is complete.\n"
$ This message is no longer used.

DONE2 "The backup is complete.\7\7\n"
$ Keep the \7\7 exactly as is, following the message.
$ This message appears after the dump is complete and the tape has been 
$ rewound.

DUMP1   "backup: "

DUMPBLKS "There are %1$ld 1k blocks on %2$d volumes.\n"
$ %1$ld is the number of 1k blocks.
$ %2$d is the number of volumes.

DUMPCOMP "0511-212 Cannot read the command pipe."
$ The perror message follows this message.
$ Do not translate pipe

DUMPD "The date of this level %1$c backup is %2$s.\n"
$ %1$c is the level number of the backup.
$ %2$s is the date of the backup.

DUMPDL "The date of the last level %1$c backup is %2$s.\n"
$ %1$c is the level of the backup.
$ %2$s is date of the backup.

DUMPFSYS "Backup these file systems:\n"
$ This message informs the user which file systems should be backed up.
$ This message precedes the DUMPLL message.

DUMPING "Backing up %s "
$ %s is the disk file system.
$ The DUMPD and DUMPDL messages precede this message on separate lines.
$ The TOHOST (for remote backup) or the TOLOCLA (for local backup)
$ message follows this message on the same line.

DUMPL "There is a level %1$c backup on %2$s.\n"
$ %1$c is the level number.
$ %2$s is the date.

DUMPLL "%1$c %2$8s\t%3$6s  Last backup: Level %4$c, Date %5$s\n"
$ %1$c is substituted by a space or a > character. The file systems with
$ %2$8s is the name of the file system.
$ %3$6s is the name of the device.
$ %4$c is the level number.
$ %5$s is the date.
$ This message is invoked by the -w or the -W flag.

ENTIRE "The backup command has ended abnormally.\n"
$ This message follows any error message that terminates.

EPOCH "the epoch"
$ Do not translate.

ERRCOMP "0511-214 Cannot write the command pipe."
$ The perror message follows this message.
$ Do not translate pipe

ESTBLK "There are an estimated %1$ld 1k blocks.\n"
$ %1$ld is the number of 1k blocks.

FINISH "%1$3.2f%% of the backup is done. It should be finished in %2$d:%3$02d.\n"
$ %1$3.2f%% is the percentage of the backup that has been done.
$ %2$d is the amount of minutes in which the backup should be finished.
$ %3$02d is the amount of seconds in which the backup should be finished.

FINISH2 "%1$3.2f%% of the backup is done. It is overdue by %2$d:%3$02d.\n"
$ %1$3.2f%% is the percentage of the backup that has been done.
$ %2$d is the amount of minutes by which the backup is overdue.
$ %3$02d is the amount of seconds by which the backup is overdue.

FOPENTTY "0511-215 The fopen call on /dev/tty failed.\n"
$ Do not translate fopen, /dev/tty.

FORKF "0511-216 Cannot create a child process for the parent process %d.\n"
$ %d is the parent process.

FSEEK "0511-217 The fseek call failed."
$ The perror message follows this message.
$ Do not translate fseek

FTOKF "0511-218 Cannot generate a token for the semaphore system.\n"
$ The perror message follows.
$ Do not translate semaphore. Token is a UNIX technical term.

DUMPFTRUNC "0511-219 Cannot truncate update files."
$ The perror message follows.

IOCFAIL "0511-220 Cannot get information about %s.\n"
$ %s is media.

IMPOSS "Cannot read %1$s [block %2$d]: count=%3$d, got=%4$d\n"
$ %1$s is a disk.
$ %2$d is the block number.
$ %3$d and %4$d are for debugging purposes only.
$ This message is informational only; the program does not terminate.

INTR "The interrupt signal has been received.\n"

LASTDU "Date of last backup done (Backup file systems identified with >):\n"
$ The > precedes any file system that needs to be backed up.
$ Do not translate the > character.

LOCKF "0511-222 Cannot create the temporary lock file.\n"
$ The perror message follows this message.

LOSTCON "rdump: 0511-223 The remote host connection is lost.\n"
$ Do not translate rdump

LSEEKF "The lseek call failed.\n"
$ This is an informational message. The backup continues.
$ Do not translate lseek

UNCOMPRESS "Uncompress failed.\n"
$ This is an informational message. The backup continues.
$ Do not translate Uncompress

MALLOCE "There is not enough memory available now.\n"
$ This is an informational message. The backup continues somehow.

MAP1 "Mapping regular files. This is Pass 1.\n"
MAP2 "Mapping directories. This is Pass 2.\n"
MAP3 "Backing up directories. This is Pass 3.\n"
MAP4 "Backing up regular files. This is Pass 4.\n"

MASTER "0511-224 There is a master and slave protocol error.\n"
$ Do not translate master, slave

MEDIAOPEN "0511-225 Cannot open %s.\n"
$ %s is media.

MEDIAWRE "0511-210 There is a media write error on %s.\n"
$ This message precedes the CANTREC message
$ %s is media.

MEDIAWRE2 "0511-211 There is a media write error %1$d blocks into volume %2$d.\n"
$ %1$d is the number of blocks.
$ %2$d is the volume number.
$ This message precedes the MEDIAEE message.

MEDIAEE ""
$ This is a null message.

MNTCTLE "0511-226 Cannot get the status of the mounted file systems.\n"

NEEDATT "Please respond: %s (yes or no): "
$ %s is the message that must be responded to.
$ This message appears if the user does not respond within 2 minutes.

NEEDKEY "0511-227 Specify the -f flag with device host:tape.\n"
$ Do not translate host:tape

NEWVOL "Is the next volume ready?"

NEWVOLS "Are the next volumes ready?"

NOTFS "0511-209 %s is not a file system.\n"
$ %s is the disk on which the file system resides.

NOTJFS "0511-800 %s is not recognized as a JFS filesystem.\n"
$ %s is the disk on which the file system resides.

NOTSUPJFS "0511-801 %s is not a supported JFS filesystem version.\n"
$ %s is the disk on which the file system resides.

NOTNATIV "0511-228 Not a native file system.\n"
$ native refers to an AIX version 3.* file system.

OPENTF "0511-229 Cannot open %s.\n"
$ %s is the output device.

OPRETRY "\tDo you want to try again? "
$ The YESNO message appears next. Leave a space at the end.

PIPEB "0511-230 Pipe communication is ended. Try the backup again later.\n"
$ Do not translate pipe

PROTO "0511-231 Communication protocol error: %s not recognized.\n"
$ %s is a code number.

PROTO1 "0511-232 Communication protocol error: rmtgets failed.\n"
$ Do not translate rmtgets.

DUMPPWAIT "0511-233 Child process %3$d ended; parent %1$d expected %2$d.\n"
$ The system continues in a loop until the expected child is found.

REPLACE1 "Replace the faulty media with a new one.\n"
REPLACE2 "If the faulty media is a tape, then please wait\n\
for the tape to rewind before replacing it.\n"
REPLACE3 ""
$ All three of these messages are printed on order.

RESTART "Do you want to start again? "

REWRITE1 "0511-234 Received a signal that is not known.\n\
\tAttempting to write again.\n"

REWRITE "This backup volume will be written again.\n"
$ This message follows the REPLACE messages.

SEMCTL2 "0511-235 Cannot remove semaphores.\n"
$ Do not translate semaphores
$ The perror message follows this message.

SEMCTLE "0511-236 Cannot set semaphore values.\n"
$ Do not translate semaphore
$ The perror message follows this message.

SEMGETE "0511-237 Could not get a semaphore.\n"
$ Do not translate semaphore
$ The perror message follows this message.

SLAVEOP "0511-238 The slave process could not open the disk.\n"
$ Do not translate slave
$ The perror message follows this message.

SLAVES1 "0511-239 %d slave processes generated too many processes.\n"
$ The perror message follow if the pipe or fork fails.
$ %d is the number 3.

TOHOST "to %1$s on host %2$s.\n"
$ %1$s is a tape.
$ %2$s is a host.
$ This message is for remote communication.
$ This message follows the DUMPING message.

TOLOCLA "to %s.\n"
$ This message follows the DUMPING message.
$ %s is media.
$ This message precedes the DUMPING message.
$ This message is for local communication.

TPBSIZE "0511-241 There is an internal error.\n"

TREWIND "The tape is rewinding.\n"

TRYREW "0511-242 SIGHUP signal received. Attempting to write again.\n"
$ Do not translate SIGHUP.

TRYREW1 "0511-243 SIGTRAP signal received. Attempting to write again.\n"
$ Do not translate SIGTRAP.

TRYREW2 "0511-244 SIGFPE signal received. Attempting to write again.\n"
$ Do not translate SIGFPE.

TRYREW3 "0511-245 SIGBUS signal received. Attempting to write again.\n"
$ Do not translate SIGBUS.

TRYREW4 "0511-246 SIGALRM signal received. Attempting to write again.\n"
$ Do not translate SIGALRM.

TRYREW5 "0511-247 SIGTERM signal received. Attempting to write again.\n"
$ Do not translate SIGTERM.

UNKIF "In update file %1$s, line %2$d is not in the correct format.\n"
$ This is an informational message only.
$ %1$s is the update file.
$ %2$d is the line number.

UNKSERV "rdump: 0511-248 Internal error; the remote service request is not necessary.\n"

UNKSIG "0511-249 Received a signal that is not recognized; there is no recovery.\n"
$ $ Is it necessary to say "There is no recovery?"

UNRECERR "There is an error from which the system cannot recover.\n"
$ This message follows the BREADERR message and precedes the CONTINUE message.

VOLBEG "Volume %1$d begins with blocks from inode %2$d.\n"
$ %1$d is the volume.
$ %2$d is the inode number.
$ This is an informational message.

WARNING1 "0511-251 The file system is still mounted; data may not be consistent.\n\
        Use the umount command to unmount the filesystem; then do the backup.\n"
$ Do not translate umount, unmount, backup

YESNO " Yes or No?\n"
$ Leave 1 space at the beginning of the message
$ The user receives this message whenever the program needs a query.

EEXACL "0511-255 Export of Access Control Lists failed.\n"
EEXPCL "0511-256 Export of Process Control Lists failed.\n"

USAGE "Usage: backup [-b Number1] [-f Device] [-l Number2]\n\
              [-u] [-c] [-w|-W] [-Level] [Filesystem]\n"

RBADKEY "0511-205 %c is not a recognized flag.\n\
\tUsage: rdump [-b Number1] [-d Density] -f Machine:Device [-l Number2]\n\
\t    [-sSize] [-u] [-c] [-w|-W] [-Level] [Filesystem]\n"
$ Do not translate %c%.
RDUMP1   "rdump: "
RENTIRE "The rdump command has ended abnormally.\n"
$ This message follows any error message that terminates.

RUSAGE "Usage: rdump [-b Number1] [-d Density] -f Machine:Device \n\
\t    [-sSize] [-u] [-c] [-w|-W] [-Level] [Filesystem]\n"

RBADKEY1 "0511-205 %c is not a recognized flag.\n\
\tUsage: rdump [-b Number1] [-d Density] -f Machine:Device \n\
\t    [-sSize] [-u] [-c] [-w|-W] [-Level] [Filesystem]\n"
$ Do not translate %c%.

BADKEY1 "0511-205 %c is not a recognized flag.\n\
\tUsage: backup [-b Number1] [-c] [-l Number2] [-u] [-w|-W]\n\
\t       [-Level] -f Machine:Device [Filesystem]\n"
$ Do not translate %c%.

MSQCTL2 "0511-257 Cannot remove message queue.\n"
$ Do not translate message queue
$ The perror message follows this message.

MSQGETE "0511-258 Could not get a message queue.\n"
$ Do not translate message queue
$ The perror message follows this message.

ERRCLS "dump: 0511-259 There is an error on closing device %s.\n"
$ This message precedes the RESTART message.
