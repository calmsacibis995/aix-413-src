$ ENGLISH_SOURCE_VERSION= 1.20
$ @(#)82	1.2  src/msg/bos/base/restore2.msg, bosmsg, msg411, 9430C411a 7/23/94 12:26:18
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
$quote "
$set MS_RESTORE
$ The panic message routine that follows many messages first prints the message,
$ then asks the user if he or she wants to abort. If so, the routine asks the
$ user if he or she would like a core dump.  If the user does not want to abort,
$ the program program tries to continue.
$
$ The BADENTRY message routine that follows many messages is in the following
$ order: BADENTRY, the designated message, NAME, PANAME. After this sequence,
$ 0-4 of the following messages could print: SIBLING, NEXTENT, NEXTLNN, or
$ NEXTHASH.
$ The following 3 messages will always appear: ENTRYTYP, INODEN, and FLAGS.
$ The ENTRYTYP or INODEN message calls the NODEE or the LEAF message.
$ The FLAGS message calls the panic message routine.

ABORT "Do you want to end the restore"
$ This message calls a reply routine that adds the ? character at the end
$ of the message and prompts the user for a yes or a no reply.

ACTIVEF "restore: 0511-276 Internal error; active file into volume 1\n"
$ The panic message routine follows this message.
$ Leave the message verbatim--do not translate or modify.

ADDINO "restore: 0511-277 Internal error; addino failed.\n\
\tInode %d is out of range.\n"
$ The panic message routine follows this message.
$ %d is an inode.
$ Do not translate addino.

ADDLN "Restore is adding links.\n"
$ This is an informational message the user receives with the -v (verbose) flag.

ALLOCF "restore: 0511-278 Cannot allocate space for the archive buffer.\n"

ALLOCSYM "restore: 0511-279 Cannot allocate space for the symbol table.\n"
$ The panic message routine follows this message.

BADBLKS "restore: 0511-280 File system block size %d is not valid.\n"
$ %d is the file system block size.

BADENTRY "restore: 0511-281 There is an internal error condition:\n\
\t%s\n"
$ %s is the message that follows.
$ The BADENTRY message routine follows this message.
$ The panic routine follows the BADENTRY message routine.

BADF "restore: 0511-282 Cannot look up first inode %d.\n"
$ The panic message routine follows this message.
$ %d is an inode.

BADKEY "restore: 0511-283 %c is not a valid flag.\n"
$ This message is followed by the USAGE messages.

BADNAME "restore: 0511-284 Internal error; file name is not valid.\n"
$ The panic message routine follows this message.

BADNAMEA "restore: 0511-285 Addentry file %s is not valid.\n"
$ The panic message routine follows this message.
$ Do not translate addentry.
$ %s is a file.

BADRANGE "restore: 0511-289 The device range specified is not valid.\n"

BADOPTM "restore: 0511-286 Cannot use the %1$c flag with the %2$c flag.\n"
$ %1$c is a flag. %2$c is a flag.

BADSEEKP "restore: Internal error; seek pointer %d is not correct in rst_seekdir.\n"
$ This is an informational message; the program continues.
$ Seek pointer %d is a number.
$ Do not translate rst_seekdir.

BEGINIC "Beginning incremental restore.\n"
$ This is an informational message the user receives with the -v (verbose) flag.

BEGINLE0 "Beginning level 0 restore.\n"
$ This is an informational message the user receives with the -v (verbose) flag.

BLKPOS "restore: 0511-287 Specify a positive integer for the block size.\n"

CALEXTL "Calculating extraction list.\n"
$ This is an informational message the user receives with the -v (verbose) flag.

CALNODE "Calculating node updates.\n"
$ This is an informational message the user receives with the -v (verbose) flag.

CANEXT "Continuing extraction of new leaves.\n"
$ This is an informational message the user receives with the -v (verbose) flag.
$ Leaves are files.  Within the UNIX "inverted tree structure," the leaves or
$ files are at the terminus of the branches or directories.

CANKEEP "Cannot KEEP and change modes.\n"
$ Do not translate KEEP.
$ This message becomes part of the BADENTRY message sequence.

CANPIPE "restore: 0511-288 Cannot change volumes on pipe input.\n"
$ Do not translate pipe
$ This message is followed by ABORT, then by the panic message routine.

CANREMO "Cannot remove directory.\n"
$ The panic message routine follows this message.

CANTCRF "0511-290 Cannot create file.\n"
$ The perror message follows this message.

CANTCRS "restore: 0511-291 The mknod call failed.\n"
$ The perror message follows this message.
$ Do not translate mknod.

CANTFIND "restore: 0511-292 Cannot find the file removal list.\n"

CANTFINL "restore: 0511-293 Cannot find the file dump list.\n"

CANTOPEN "restore: 0511-294 Cannot open %s.\n"
$ %s is media.

CANTOPNUL "restore: 0511-295 Cannot open /dev/null."
$ The perror message routine follows this message.
$ Do not translate /dev/null.

CANTOPTTY "restore: 0511-296 Cannot open /dev/tty."
$ The perror message routine follows this message.
$ Do not translate /dev/tty.

CANTOSYMF "restore: 0511-297 Cannot open symbol table file %s.\n"
$ The panic message routine follows this message.
$ %s is the symbol table file.

CANTRES "restore: 0511-298 Cannot rename %1$s to %2$s.\n"
$ The perror message follows this message.
$ %1$s and %2$s are files.
$ Do not translate rename.

CANTROOT "Cannot move ROOT."
$ This message is included in the BADENTRY message routine.
$ Do not translate ROOT.

CANTSTAT "restore: 0511-300 Cannot get information about the current directory.\n"

CANTSTSYF "restore: 0511-301 Cannot get information about symbol table file %s.\n"
$ The panic message routine follows this message.

CHECKPT "Checkpointing the restore.\n"
$ The user receives this message with the -v (verbose) flag.
$ To checkpoint is to take a snapshot of the current status.

CHKSUM "restore: Checksum error %1$o is at inode %2$d, file %3$s.\n"
$ The program continues; the routine that called it will decide what to do next.
$ %1$o is the checksum error number.
$ %2$d is the inode number.
$ %3$s is the file name.

CHECKSYM "Checking the symbol table.\n"
$ The user receives this flag with the -v (verbose) flag.

CMDCANTC "Continued command lines are discarded.\n"
$ The program continues.

CNCRDIR "restore: 0511-303 Cannot create temporary directory %s.\n"
$ The FOPENE message follows this message.
$ The perror message follows the FOPENE message.
$ %s is a directory.

CNCRMO "restore: 0511-304 Cannot create temporary file %s.\n"
$ The FOPENE message follows this message.
$ The perror message follows the FOPENE message.
$ %s is a temporary file.

COMMANDS1 "The available commands are:\n"
$ COMMANDS2, COMMANDS3, COMMANDS4, COMMANDS5, COMMANDS6, COMMANDS7, COMMANDS8,
$ COMMANDS9, COMMANDS10, COMMANDS11, COMMANDS12, COMMANDS13, COMMANDS14,
$ COMMANDS15, and COMMANDS16 follow the COMMANDS1 message and provide a form
$ of help text for the user.

COMMANDS2 "\tls [Parameter] - Lists a directory.\n"
$ Do not translate ls.

COMMANDS3 "\tcd Directory - Changes a directory.\n"
$ Do not translate cd.

COMMANDS4 "\tpwd - Prints the current directory.\n"
$ Do not translate pwd.

COMMANDS5 "\tadd [Parameter] - Adds the specified parameter to list of"
$ Do not translate add.

COMMANDS6 "\tfiles to be extracted.\n"
$ This message is preceded by the COMMANDS5 message.

COMMANDS7 "\tdelete [Parameter] - Deletes the specified parameter from"
$ Do not translate delete.

COMMANDS8 "\tthe list of files to be extracted.\n"
$ This message is preceded by the COMMANDS7 message.

COMMANDS9 "\textract - Extracts requested files.\n"
$ Do not translate extract.

COMMANDS10 "\tsetmodes - Sets the modes of requested directories.\n"
$ Do not translate setmodes.

COMMANDS11 "\tquit - Exits the program immediately.\n"
$ Do not translate quit.

COMMANDS12 "\tverbose - Toggles the verbose flag on and off.\n"
$ Do not translate verbose.

COMMANDS13 "\t(This command is useful with the ls command.)\n"
$ This message is preceded by the COMMANDS12 message.

COMMANDS14 "\thelp or ? - Prints the available commands list.\n"
$ Do not translate help or the ? character.

COMMANDS15 "If no directory is specified, the current"

COMMANDS16 "\tdirectory is the default.\n"
$ This message is preceded by the COMMANDS15 message.

CONTFAIL "restore: 0511-305 Cannot continue.\n"
$ The perror message follows this message.

CONTIN "Do you want to continue"
$ This message calls a reply routine that adds the ? character at the end
$ of the message and prompts the user for a yes or a no reply.

CONVNEWF "Converting to new file system format.\n"
$ This is an informational message.

CORE "restore: Do you want to dump core"
$ This message calls a reply routine that adds the ? character at the end
$ of the message and prompts the user for a yes or a no reply.
$ The YESNO message follows this message.
$ To dump core means to save a memory image into a core file.

CORRDIR "Directory %s is damaged.\n"
$ This is an informational message. The program continues.
$ %s is a directory.

CORSYM "restore: 0511-307 The symbol table is damaged.\n"
$ The panic message routine follows this message.

CRLN "Creating %1$s link %2$s->%3$s\n"
$ %1$s is the word, hard, or the word, symbolic.
$ %2$s is the new file created.
$ %3$s is the existing file.
$ -> is the symbol for link. Do not translate.

DEBGOFF "The debugging mode is off.\n"
$ Debugging means checking for bugs or problems with the program.

DEBGON "The debugging mode is on.\n"
$ Debugging means checking for bugs or problems with the program.

DELETINO "restore: 0511-308 Internal error; inode %d out of range.\n"
$ The panic message routine follows this message.
$ %d is an inode.
$ Do not translate deleteino.

DELINON "restore: 0511-309 Inode %d not found.\n"
$ %d is an inode number.
$ Do not translate deleteino.
$ The panic message routine follows this message.

DELLN "The restore command deleted hard link %1$s to directory %2$s.\n"
$ %1$s is a hard link.
$ %2$s is a directory.
$ This is an informational message.

DUMPDATE "The dump date is %s.\n"
$ %s is the current date.

DUMPFROM "Dumped from: %s.\n"
$ %s is the date of the last dump.

DUMPPOS "restore: 0511-310 The dump number must be a positive integer.\n"

DUPENT "Internal error; an inode number appears in the symbol table more than once.\n"
$ The panic routine follows this message.

DUPINUM "There is a duplicate inode number.\n"
$ This message is part of the BADENTRY message sequence.

ENDRUN "restore: 0511-311 Found end-of-archive mark before expected.\n"
$ The panic routine follows this message.

ENTERD "Enter the device name. The default is %s.\n"
$ This message follows the MOUNT message.
$ %s is the default device name.

ENTRYTYP "The entry type is %s.\n"
$ %s is a node or a leaf.
$ This message is part of the BADENTRY message sequence.

EOAHE "The end of archive header was read."
$ This message will be followed by a newline or the PREDICT message.

ESETDIR "restore: 0511-312 There is an error setting directory modes.\n"
$ The panic routine follows this message.

EWRITINO "restore: 0511-313 Error extracting inode %1$d of file %2$s.\n"
$ The READ or the WRITE message follows.
$ %1$d is an inode.
$ %2$s is a file.

EWRSYMF "restore: 0511-314 %s: Output error while writing symbol table.\n"
$ %s is the file containing the symbol table.
$ The panic message routine follows this message.

EXPECF "Expected next file inode number %1$d, but received %2$d.\n"
$ %1$d is the inode number expected.
$ %2$d is the inode number received.
$ The program skips over the inode number and continues.

EXTNEW "Extracting new leaves.\n"
$ The user receives this message with the -v (verbose) flag.
$ Leaves are files. Within the UNIX "inverted tree structure," leaves are at
$ the terminus of branches or directories.

EXTRDIR "Extracting directories from media.\n"
$ The user receives this message with the -v (verbose) flag.

EXTREQ "Extracting requested files.\n"
$ The user receives this message with the -v (verbose) flag.

EXTRFILE "Extracting file %s.\n"
$ %s is a file name.
$ The user receives this message with the -v (verbose) flag.

EXTSFILE "Extracting special file %s.\n"
$ %s is a file name.
$ The user receives this message with the -v (verbose) flag.

FILECON "A file continuation header, inode %d, was read."
$ %d is an inode number.
$ This message will be followed by a newline or the PREDICT message.

$ This is an informational message.

FILEHE "A file header, inode %d, was read."
$ %d is the inode number.
$ This message will be followed by a newline or the PREDICT message.

FINDUNRE "Looking for names that are not referenced.\n"
$ The user receives this message with the -v (verbose) flag.

FLAGS "The internal flags are %s.\n"
$ This message is the final message in the BADENTRY message sequence.  FLAGS
$ then calls the panic message routine.

FOPENE "fopen"
$ A colon and the fopen perror message follow this message.
$ Do not translate fopen.

FREEDIR "The system cannot free a symbol table referenced directory.\n"
$ This message becomes part of the BADENTRY message sequence.

FREENOND "The system cannot free a directory that is not empty.\n"
$ This message becomes part of the BADENTRY message sequence.

FWRITEE "fwrite"
$ A colon and the fwrite perror message follow this message.
$ Do not translate fwrite.

GETHEAD "restore: 0511-315 Inode type %d is not known.\n"
$ The panic routine follows this message.
$ Do not translate gethead.
$ %d is an inode type.

HARD "hard"
$ Do not translate
$ This message can replace %1$s in the CRLN message.

HARDLNW "Warning: Cannot create hard link %1$s->%2$s: "
$ Do not translate hard
$ %1$s is the new file.
$ %2$s is the existing file.
$ The perror message follows this message; restore continues.
$ -> is the symbol for link. Do not translate. The -> symbol shows that the
$ two files are linked.

HOLE "restore: 0511-316 There is a hole in the map.\n"
$ The perror message routine follows this message.

IMCOP "There is an operation that is not complete.\n"
$ This message becomes part of the BADENTRY message routine.

IMPOS "[%s] %s: impossible state\n"
$ Do not translate this message. It is a debugging message.

INCHIGH "restore: 0511-317 The specified restore level is higher than expected.\n"

INCLOW "restore: 0511-318 The specified restore level is lower than expected.\n"

INCSTATE "[%s] %s: inconsistent state\n"
$ Do not translate this message. It is a debugging message.

INITSYC "initsymtable called from command %c\n"
$ Do not translate this message. It is a debugging message.

INITSYM "Initializing the symbol table.\n"
$ The user receives this message with the -v (verbose) flag.

INODEN "The inode number is %ld.\n"
$ This message is part of the BADENTRY message sequence.
$ %ld is the inode number.

IOCFAIL "restore: 0511-319 Cannot get information on %s.\n"
$ %s is media.

IOCTLE "ioctl STFSF"
$ A colon and the perror message follow this message.
$ Do not translate ioctl STFSF.

LEAFN "LEAF"
$ This message can appear as part of the BADENTRY message sequence.

LINKIT "restore: 0511-320 Type %d is not known.\n"
$ The panic routine follows this message.
$ %d is a type.
$ Do not translate linkit.

LINKNOF "The link is not found.\n"
$ This message becomes part of the BADENTRY message sequence.

LNNONEXT "restore: 0511-321 Internal error; cannot link to name.\n"
$ The panic message routine follows this message.

LOOKINO "The lookupino routine failed.\n"
$ This message becomes part of the BADENTRY message sequence.
$ Do not translate lookupino.

MAKENODE "Making node %s.\n"
$ %s is a node.
$ This is an informational message the user receives with the -v (verbose) flag.

MARKREM "Marking entries to be removed.\n"
$ This is an informational message the user receives with the -v (verbose) flag.

MASKH "The dump mask header was read."
$ Do not translate mask header.
$ This message will be followed by a newline or the PREDICT message.

MDOTDOT "Warning: The .. entry is missing from directory %s.\n"
$ Do not translate ..
$ %s is a directory.
$ This is an informational message.

MEDIAOPEN "restore: 0511-322 Cannot open %s.\n"
$ %s is media.

MISSBLK "restore: 0511-323 Specify block size.\n"

MISSDEV "restore: 0511-324 Specify a media device.\n"

MISSDOT "Warning: The . entry is  missing from directory %s\n"
$ Do not translate .
$ %s is a directory.
$ This is an informational message.

MISSDUMP "restore: 0511-325 Specify dump number.\n"

MISSINO "restore: 0511-326 Inode number %d is missing.\n"
$ The panic message routine follows this message.
$ %s is an inode number.

MISSNAM "Warning: File name %s is missing.\n"
$ %s is a file name.

MISSQ "restore: 0511-327 There is a missing %c character.\n"
$ %c is the single quote or the double quote character.

MKTEMP "Cannot call mktempname with the TMPNAME constant.\n"
$ This message becomes part of the BADENTRY message sequence.
$ Do not translate mktempname.
$ Do not translate TMPNAME.

MODENOTS "Directory mode, owner, and times not set.\n"
$ This message follows the FOPENE message, the fopen perror message,
$ and the NOMODEFI message.

MOUNT "Mount volume %d;\n"
$ %d is a volume number.
$ This message precedes the ENTERD message.

MOUNTV "Mount volume %1$d on %2$s and press the Enter key to continue.\n"
$ %1$d is the media volume number.
$ %2$s is the device for that volume.

MOUNTV2 "Mount volumes %d-%d and press the Enter key to continue.\n"
$ %d-%d is the range of archive volumes.  Such as "1-5".

MULTIDP "restore: 0511-328 Do not use the skip option with pipe input.\n"
$ Do not translate skip, pipe

MUSTITR "restore: 0511-329 Specify the -i, -t, -r, -R, or -x flag.\n"
$ Do not translate -i, -t, -r, -R, or -x.

NAME "name: %s\n"
$ %s is a node or a leaf.
$ This is the 2nd message in the BADENTRY sequence.

NAMLONG "%1$s%2$s: Path name cannot contain more than %3$d characters.\n"
$ This will "wrap" because of a very long path name. DO NOT put in any
$ new lines, except at the very end.
$ %1$s%2$s is the full path name.
$ %3$d is the maximum number of characters.
$ This is an informational message.

NEEDF "restore: 0511-330 Specify the -f flag and device host:tape.\n"
$ Do not translate -f.
$ Do not translate host:tape.

NEWNODE "The newnode routine did not receive a node.\n"
$ This message becomes part of the BADENTRY sequence.

NEXTENT "The next entry name is %s.\n"
$ %s is the entry name.
$ This message may appear as part of the BADENTRY message sequence.

NEXTHASH "The next hashchain name is %s.\n"
$ %s is the hashchain name.
$ Do not translate hashchain.
$ This message may appear as part of the BADENTRY message sequence.

NEXTLNN "The next link name is %s.\n"
$ %s is the link name.
$ This message may appear as part of the BADENTRY message sequence.

NEXTVOL "Specify the next volume number: "
$ The system echoes the user's input after the colon.

NODEE "NODE"
$ This message may appear as part of the BADENTRY message sequence.

NODIRI "restore: 0511-331 Cannot find directory inode %d.\n"
$ %d is the directory inode number.
$ The panic message routine follows this message.

NODIRINO "restore: 0511-332 Cannot find directory inode %1$d named %2$s.\n"
$ %1$d is the inode number.
$ %2$s is the name that is associated with the inode number.
$ The panic message routine follows this message.

NOHEAD "restore: 0511-333 There is no header after the volume mark.\n"
$ The panic message routine follows this message.

NOMEMDUMP "restore: 0511-334 There is no memory available for the file dump list.\n"
$ The panic message routine follows this message.

NOMEMENT "restore: 0511-335 There is no memory available for the entry table.\n"
$ The panic message routine follows this message.

NOMEMREV "restore: 0511-336 There is no memory available for the file removal list.\n"
$ The panic message routine follows this message.

NOMEMSYM "restore: 0511-337 There is no memory available to extend the symbol table.\n"
$ The panic message routine follows this message.

NOMEMTAB "restore: 0511-338 There is no memory available for the directory table.\n"
$ The panic message routine follows this message.

NOMODEFI "restore: 0511-339 Cannot open mode file %s.\n"
$ %s is the mode file.
$ The FOPENE message and the fopen perror message precede this message.
$ The MODENOTS message follows this message.

NOROOT "restore: 0511-340 The root directory is not on media.\n"
$ The panic message routine follows this message.

NOSPACST "restore: 0511-341 There is no memory for the string table.\n"
$ The panic message routine follows this message.

NOTATB "restore: 0511-342 There is no header at the beginning of the file.\n"
$ The panic message routine follows this message.

NOTDIR "restore: 0511-343 %s is not a directory.\n"
$ %s is the string that is not a directory.
$ The panic message routine follows this message.

NOTDIR1 "restore: 0511-344 %s is not a directory.\n"
$ %s is a string that is not a directory.

NOTDUMPM "restore: 0511-346 The media is not in dump format.\n"
$ The MOUNT and the ENTERD message follow this message.

NOTILIST "The file entry is not on the inode list.\n"
$ This message becomes part of the BADENTRY message sequence.

NOTMARKR "The file entry is not marked REMOVED."
$ This message becomes part of the BADENTRY message sequence.

NOTONLIST "%s is not on the extraction list.\n"
$ %s is a file or a directory.
$ This is an informational message.

NOTONARCH "%s is not on the archive.\n"
$ %s is a file or directory.
$ This is an informational message.

NOTVOL1 "Insert volume 1 of the dump.\n"

OPEN "open"
$ A colon and the open perror message follow this message.
$ Do not translate open.

OPENDIR "opendir"
$ A colon and the opendir perror message follow this message.
$ Do not translate opendir.

OUTMEM "There is no more memory for the %s command.\n"
$ %s is the command given by the user.
$ The system returns to restore's interactive shell; the program continues.

PALIST "Cannot find an entry in the parent list.\n"
$ This message becomes part of the BADENTRY message sequence.

PANAME "The parent name is %s.\n"
$ %s is the parent name.
$ This message is the 3rd message of the BADENTRY message sequence.

PARTIALB "restore: 0511-347 Expected full block read number %2$d, but received %1$d.\n"
$ The panic message routine follows this message.

PATHMAX "restore: 0511-348 %s: The path name is too long.\n"
$ %s is the path name.

PERIOD ".\n"
$ This message will follow the VOLHEAD, MASKH, REMMASK, FILEHE,
$ FILECON, EOTHE, and PREDICT messages used with the -D option.
$ This message does not require any type of translation.

PREDICT ";\texpected %1$d blocks, but received %2$d blocks.\n"
$ This message follows MASKH or REMMASK or FILEHE or FILECON or EOTHE.
$ It is a consistency check.
$ %1$d and %2$d and numbers of blocks.

READ "read"
$ A colon and the read perror message follow this message.
$ Do not translate read.

REMLEAF "The removeleaf routine did not receive a leaf.\n"
$ This message becomes part of the BADENTRY message sequence.
$ Do not translate removeleaf.

REMLEAF2 "Removing leaf %s.\n"
$ The user receives this message with the -v (verbose) flag.
$ %s is a leaf.

REMMASK "The remove mask header was read."
$ Do not translate remove mask header.
$ This message will be followed by a newline or the PREDICT message.

REMNODE "The removenode routine did not find a node.\n"
$ This message becomes part of the BADENTRY message routine.
$ Do not translate removenode.

REMNONE "The removenode routine did not find an empty directory.\n"
$ This message becomes part of the BADENTRY message routine.
$ Do not translate removenode.

REMOLD "Removing nodes (directories) that are not current.\n"
$ The user receives this message with the -v (verbose) flag.

REMOVEN "Node %s has been removed.\n"
$ The user receives this message with the -v (verbose) flag.

RENAME "%1$s renamed to %2$s.\n"
$ The user receives this message with the -v (verbose) flag.
$ %1$s and %2$s are files.

RESTINT "restore: 0511-350 The restore is interrupted.\n\
\tDo you want to continue"
$ This message calls a reply routine that adds the ? character at the end
$ of the message and prompts the user for a yes or a no reply.

RESTOR "Media read error found while restoring %s.\n"
$ %s is a file name.
$ This is an informational message.

RESYNC "Media read error found while trying to syncronize again.\n"
$ This is an informational message.

RESYNCR "Synchronizing restore again; skipped %d blocks.\n"
$ This is an informational message.
$ %d is the number of blocks the program skipped over (data that is not valid).

SAVEFIL "Cannot create save file %s for the symbol table.\n"
$ %s is the save file.
$ The FOPENE message followed by the open perror message precede this message.
$ The panic message routine follows this message.

SEEKERR "restore: 0511-351 fclear failed while extracting inode %1$d, name %2$s.\n"
$ %1$d is the inode number.
$ %2$s is the inode name.
$ Do not translate fclear.
$ A perror message follows this message.

SETDOT "Do you want to set the owner or the mode for the current\n\
\tdirectory"
$ The program appends the ? character and calls a reply routine.

SETMOT "Setting directory mode, owner, and times.\n"
$ The user receives this message with the -v (verbose) flag.

SETUPA "Media read error found while trying to set up archive.\n"
$ This is an informational message.

SIBLING "The sibling name is %s.\n"
$ %s is the sibling name.
$ This message can appear as part of the BADENTRY message sequence.

SKIPI "Media read error found while skipping over inode %d.\n"
$ This is an informational message.

SOCKET "Skipping socket %s.\n"
$ %s is the socket.
$ The user receives this message with the -v (verbose) flag.

STATFAIL "stat"
$ A colon and the stat system call perror message follow this message.
$ Do not translate stat.
$ This message follows the CANTSTSYS message and the panic message routine

SWAP "Performing byte swapping.\n"
$ This is an informational message.

SWAPQ "Performing quad swapping.\n"
$ This is an informational message.

SYMBOLC "symbolic"
$ Do not translate
$ This message may replace %1$s in the CLRN message.

SYMLNW "restore: 0511-353 Cannot create symbolic link %1$s->%2$s: "
$ %1$s is the new file.
$ %2$s is the old file.
$ -> is the symbol for link. Do not translate. It shows that the two files
$ are linked.
$ A perror message follows the colon.

SYMLONG "restore: 0511-354 The symbolic link length %4$d is too long\n\
\tin name %1$s->%2$s%3$s.\n"
$ %4$d is the length of the specified string.
$ %1$s is the file name.
$ %2$s%3$s is the path name to which %1$s is trying to link.

SYMREAD "restore: 0511-355 Cannot read symbol table file %s.\n"
$ %s is a symbol table file.
$ The read perror message precedes this message.

MEDIABI "The media block size is %d.\n"
$ %d is the media block size.
$ The user receives this message with the -v (verbose) flag.

MEDIABNOT "restore: 0511-356 %1$d is not a multiple of dump block size %2$d.\n"
$ %1$d is the media block size.
$ %2$d is the dump block size.

MEDIAMES1 "You have not read any media yet.\n"
$ The MEDIAMES2, MEDIAMES3, MEDIAMES4, and MEDIAMES5 messages go with this
$ message.

MEDIAMES2 "Unless you know which volume your file or files are on, you should start\n\
with the last volume and work towards the first volume.\n"

MEDIAMES3 ""
$ This is a null message

MEDIAMES4 ""
$ This is a null message

MEDIAMES5 ""
$ This is a null message

MEDIAREADE "restore: 0511-357 There is a media read error."
$ The perror message follows this message.

TYPELEAF "The type should be LEAF."
$ This message becomes part of the BADENTRY message sequence.

UNALLOCSY "restore: 0511-358 Symbolic link %s has a block not allocated.\n"
$ %s is a file name.

UNEXPECF "There is a file on the archive that is not expected.\n"
$ This message becomes part of the BADENTRY message routine.

UNEXTDIR "restore: 0511-359 Directory %s is not extracted.\n"
$ %s is a directory.
$ The panic message routine follows this message.

UNKCMD "restore: 0511-360 The %s command is not known. Type ? for help.\n"
$ %s is the command that is not known.
$ Do not translate the ? character.

UNKFILE "restore: 0511-361 Cannot find the file on the archive.\n"
$ The panic message routine follows this message.

UNKFMODE "File mode 0%2$o in file %1$s is not known.\n"
$ This is an informational message.
$ 0%2$o is the file mode in octal notation.
$ %1$s is a file.

UNREFF "Action flags are associated with a file that is no longer referenced.\n"
$ This message becomes part of the BADENTRY message sequence.

USAGE "Usage:\n"
$ This message is followed by the restore or rrestore usage statements.

USAGE1 "-t[Dhvy] [-f Device] [-s Number] [-b Number] [File ...]\n"
USAGE2 "-x[Dhmvy] [-f Device] [-s Number] [-b Number] [File ...]\n"
USAGE3 "-i[Dhmvy] [-f Device] [-s Number] [-b Number]\n"
USAGE4 "-r[Dvy] [-f Device] [-s Number] [-b Number]\n"
USAGE5 "-R[Dvy] [-f Device] [-s Number] [-b Number]\n"
$ Each of these messages is preceded by "restore " which is the program name.

RRUSAGE1 "-t[Dhvy] -f Host:Device [-s Number] [-b Number] [File ...]\n"
RRUSAGE2 "-x[Dhmvy] -f Host:Device [-s Number] [-b Number] [File ...]\n"
RRUSAGE3 "-i[Dhmvy] -f Host:Device [-s Number] [-b Number]\n"
RRUSAGE4 "-r[Dvy] -f Host:Device [-s Number] [-b Number]\n"
RRUSAGE5 "-R[Dvy] -f Host:Device [-s Number] [-b Number]\n"
$ Each of these messages is preceded by "rrestore " which is the program name.

VERIFY "Verifying the directory structure.\n"
$ The user receives this message with the -v (verbose) flag.

VERIFYA "Verifying archive and initializing maps.\n"
$ The user receives this message with the -v (verbose) flag.

VOFF "The verbose mode is off.\n\
\tPress the v key to turn the verbose mode on.\n"
$ verbose mode is the optional mode that gives the user a great deal of
$ informational messages.

VOLHEAD "Volume header\n"
$ Do not translate this message.  It is here for debugging purposes.

VOLPOS "restore: 0511-362 Volume numbers must be positive integers.\n"

VON "The verbose mode is on.\n\
\tPress the v key to turn the verbose mode off.\n"
$ verbose mode is the optional mode that gives the user a great deal of
$ informational messages.

WANTED "\tSpecified dump with date of %s."
$ %s is the dump date.
$ This is an informational message.

WARNING "Warning: "
$ This message sometimes precedes the perror message.

WRITE "write"
$ A colon and the write perror message follow this message.

WRITEEE "restore: 0511-363 Write error while extracting inode %1$d,\n\
\twith the name of %2$s.\n"
$ The WRITE message and the write perror message follow this message.

WRONGDATE "restore: 0511-364 The dump dated %s is not correct."
$ This message is followed by the WANTED message.
$ %s is the date of the dump.

WRONGVOL "restore: 0511-365 %d is not the correct volume.\n"
$ %d is the volume that is not correct.
$ The MOUNT and the ENTERD messages follow this message.

YESNO "yes or no"
$ [yes or no] as according to language specified.

YOUHAVE "You have read volumes"
$ A colon follows this message; the program then supplies the volume numbers.

ZEROLEN "%s: The zero length symbolic link is ignored.\n"
$ %s is a file name.
$ The user receives this message with the -v (verbose) flag.

EACLIMP "restore: 0511-274 Import of Access Control Lists failed.\n"

EPCLIMP "restore: 0511-275 Import of Process Control Lists for file %s failed.\n"

