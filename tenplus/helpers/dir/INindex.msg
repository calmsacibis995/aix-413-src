$ @(#)35	1.3  src/tenplus/helpers/dir/INindex.msg, tenplus, tenplus411, GOLD410 3/11/94 17:13:56
$
$ COMPONENT_NAME: (INED) INed Editor
$ 
$ ORIGINS:  27
$ 
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1991
$ All Rights Reserved
$ 
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ TRANSLATOR:  In this file, many command (function) names are used.
$ They are all in UPPER CASE.  They should not be translated.
$ Examples:  BOX-MARK, ENTER, TAB, RIGHT-ARROW, and many more.
$ The sequences (1) (2) (3) (4) (5) (6) (7) and (8) are command names, and
$ should be left unchanged.
$ Do not translate PUT-COPY, PUT-DOWN, RESTORE, DELETE, PICK-UP,
$ PICK-COPY, ENTER, BOX-MARK, TEXT-MARK, CANCEL, EXECUTE, LOCN-MENU
$
$ Some of the messages in this catalog occur in pairs: a short
$ message, and a long message.  The long message explains the short message.
$
$ The following phrases appear repeatedly in this message catalog.  Please
$ translate them the same way each time:
$       Show Details About Files.
$       Show More Details About This File.
$
$quote "
$
$set MS_ININDEX
$
5000 "Dummy message to add Helper message offset."
$ Translator: do not translate the above message at all
$
$ Translator:  do not translate anthing that looks like %s or %d or
$ %1$s or %2$d etc.
$
$ Translator: do not translate "#<number>" at the end of messages
$
IABORT "File restoration is canceled.  The file is in your putdir directory.#1"
$ Do not translate "putdir".

H_IABORT "0611-301 You did not to put the file in the current directory after using\n\
PUT-DOWN, PUT-COPY, or RESTORE.  The file is in your putdir directory, which\n\
is usually in $HOME/.putdir.  To move or copy the file, go to your putdir\n\
directory and use the pick and put operations.\n\
\n\
Press CANCEL to continue."
$ Do not translate "putdir" or $HOME.
$
IALREADY "You are already looking at that display.#1"

H_IALREADY "0611-302 You used LOCAL-MENU and selected an option. You are viewing\n\
the display that that option provides already.\n\
\n\
Press CANCEL to continue."
$
IASCIISV "Do not delete %s.#1"
$ Here, %s will be replaced with the name of a file or directory.

H_IASCIISV "0611-303 You are trying to copy or delete a file whose name begins\n\
with the ... character sequence.  These files cannot be deleted or copied.\n\
\n\
Press CANCEL to continue."
$
IBADCHACL "Cannot change the access control list.#1"

H_IBADCHACL "0611-304 You have copied a file, but the system cannot copy the access\n\
control list from the old file to the new file.  Check that you own the\n\
new file.\n\
\n\
Press CANCEL to continue."
$
IBADCHAR "File name %s contains characters which are not valid.#1"

H_IBADCHAR "0611-305 You tried to rename a file, but the name that you chose contained a\n\
character that is not valid, such as a / character or an underlined character.\n\
The name will be restored to its old value.\n\
\n\
Press CANCEL to continue."
$
IBADCHGRP "Cannot change the file group ownership to %s.#1"
$ Here, %s will be replaced with the name or number of a user group.

H_IBADCHGRP "0611-306 You tried to change the group ownership of a file by changing\n\
either the group number or the group name of the file to a value that\n\
is not valid.  To change the group ownership, assign a correct value to\n\
one of these fields.\n\
\n\
Press CANCEL to continue."
$
IBADCHMOD "Cannot change the file permissions to %s.#1"

H_IBADCHMOD "0611-307 You tried to change the file permissions by typing over the\n\
existing r, w, x, or - characters in the permissions field.  Either you\n\
typed in a value that is not valid, or you do not own the file and therefore\n\
cannot change the permissions.\n\
\n\
Make sure that you own the file. If you do own the file, change the\n\
values so that:\n\
  o The first column is either r to indicate read permission, or - to\n\
    indicate no read permission.\n\
  o The second column is either w to indicate write permission, or -\n\
    to indicate no write permission.\n\
  o The third column is either x to indicate execute permission, or -\n\
    to indicate no execute permission.\n\
\n\
Press CANCEL to continue."
$ Do not translate rwx.
$
IBADCHOWN "Cannot change the file ownership to %s.#1"

H_IBADCHOWN "0611-308 You tried to change the owner or the user number of a file.\n\
Either you provided a value that is not valid to these fields, or the system\n\
has made an error.  Make sure you are providing a valid value to these\n\
fields when you modify them.  If necessary, contact your system\n\
administrator.\n\
\n\
Press CANCEL to continue."
$
IBADCOPY "Cannot restore file %s.#1"

H_IBADCOPY "0611-309 You are trying to put a file into a directory with\n\
PUT-COPY, PUT-DOWN, or RESTORE, but the system cannot copy the\n\
file into this directory. Check that there is enough disk space.\n\
\n\
Press CANCEL to continue."
$
IBADCPIO "Cannot save or restore the directory tree %s.#1"
$ Here, %s will be replaced with the name of a directory.

H_IBADCPIO "0611-310 You have tried to save or to restore a directory and all the\n\
files and subdirectories in that directory.  However, the system could\n\
not save or restore the directory you were trying to\n\
manipulate with SAVE, PUT-DOWN, PUT-COPY, or RESTORE.  Contact your\n\
system administrator.\n\
\n\
Press CANCEL to continue."
$
IBADFLAG "Internal error (The command input type %d is not expected.)#1"
$ Here, %d is replaced with a number.

H_IBADFLAG "The File Manager cannot perform the operation you last\n\
specified.  It will restart on the current directory.  The last\n\
request you entered may not have been successful.\n\
\n\
Press CANCEL to continue."
$
IBADFORMTP "Internal error (restart received a form type specifier that is not correct.)#1"
$ Do not translate restart, form type specifier

H_IBADFORMTP "The File Manager cannot perform the operation you last specified.\n\
The File Manager will restart on the current directory.  The last\n\
request you entered may not have been successful.\n\
\n\
Press CANCEL to continue."
$
IBADGID "Group number %s is not valid. The group number must be an integer.#1"
$ Here, %s will be replaced with a string that the user typed.

H_IBADGID "0611-311 You tried to change the group identifier of a file, but you\n\
did not type in a positive integer.  The value entered in the Gid field\n\
must be a positive integer.\n\
\n\
To enter a new file group, enter an integer that represents a\n\
group.\n\
\n\
Press CANCEL to continue."
$ Do not translate "Gid".
$
IBADKEY "Key %d is not recognized.#1"

H_IBADKEY "The File Manager does not recognize the operation you last specified.\n\
The File Manager will restart on the current directory.  The last request\n\
you entered may not have been successful.\n\
\n\
Press CANCEL to continue."
$
IBADNAME "You cannot rename file %s.#1"
$ Here, %s will be replaced with the name of a file.

H_IBADNAME "0611-312 You tried to rename a file, but the name you have chosen is reserved\n\
for system files.  You cannot assign file names beginning with the ...\n\
character sequence.  The system has restored the original file name.\n\
\n\
To change the name of a file, assign a valid file name.\n\
\n\
Press CANCEL to continue."
$
IBADPATH "Internal error (Egetpath returned a path that is not valid.)#1"
$ Do not translate Egetpath

H_IBADPATH "The File Manager has encountered an error.  Contact your system\n\
administrator.\n\
\n\
Press CANCEL to continue."
$
IBADSTAT "Internal error (status %d is not known)#1"
$ Do not translate %d

H_IBADSTAT "The File Manager cannot perform the operation you last specified.\n\
The File Manager will restart on the current directory.  The last request\n\
you entered may not have been successful.\n\
\n\
Press CANCEL to continue."
$
IBADSTVEC "Internal Error (restart received a status vector that is not valid.)#1"
$ Do not translate restart, status vector

H_IBADSTVEC "The File Manager has encountered an error.  Contact your system\n\
administrator.\n\
\n\
Press CANCEL to continue."
$
IBADTIME "The time specification %s is not correct.#1"
$ Here, %s will be replaced with a string that the user typed.

H_IBADTIME "0611-313 You tried to change the file access or modification date or time.\n\
The time you specified cannot be interpreted by the system.\n\
\n\
The fields that require a correctly formatted date or time are:\n\
\tModification Date\n\
\tTime of last file access\n\
\tTime of last data modification\n\
\tTime of last file status change\n\
\n\
Enter the date and time in the format originally used in these fields,\n\
or enter NOW to specify the current time.\n\
\n\
Press CANCEL to continue."
$ Do not translate the word NOW.
$
IBADUID "User number %s is not valid. The user number must be a positive integer.#1"
$ Here, %s will be replaced with a string typed by the user.

H_IBADUID "0611-314 You tried to change the user identifer of the file. The value\n\
in the Uid field must be a positive integer.\n\
\n\
To enter a new file owner identifier, enter an integer that represents a user.\n\
\n\
Press CANCEL to continue."
$ Do not translate "Uid".
$
IBROKEN "Trying to repair a damaged .index file."
$ Do not translate ".index".
$
ICLOSE "Cannot close file %s.#1"

H_ICLOSE "The File Manager cannot perform the operation you last specified.\n\
The File Manager will restart on the current directory.  The last request\n\
you entered may not have been successful.\n\
\n\
Press CANCEL to continue."
$
ICLOSEDIR "Cannot close directory %s.#1"

H_ICLOSEDIR "The File Manager cannot perform the operation you last specified.\n\
The File Manager will restart on the current directory.  The last request\n\
you entered may not have been successful.\n\
\n\
Press CANCEL to continue."
$
IDEL "Internal Error (Cannot delete line from field %s.)#1"

H_IDEL "The File Manager cannot perform the operation you last specified.\n\
The File Manager will restart on the current directory.  The last request\n\
you entered may not have been successful.\n\
\n\
Press CANCEL to continue."
$
IDIRSV "Cannot delete the current directory.#1"

H_IDIRSV "0611-315 You tried to delete or copy the current directory using\n\
DELETE-LINE, PICK-UP, or PICK-COPY.  You cannot delete or copy a\n\
directory while you are in it.\n\
\n\
To delete or copy the current directory, you can use ZOOM-OUT to the\n\
parent directory level, then use DELETE-LINE, PICK-UP, or PICK-COPY.\n\
\n\
Press CANCEL to continue."
$
IDUPMOD "The file name %s already exists.#1"

H_IDUPMOD "0611-316 There is already another file in this directory with\n\
that name. The old name will be restored.\n\
\n\
To rename the file, assign the file a name that is unique in\n\
this directory.\n\
\n\
Press CANCEL to continue."
$
IDUPNAME "The file name %s already exists.\n\
Press CANCEL to cancel the file restoration,\n\
or enter a new name:                    #1"
$ Please leave the trailing spaces in this message.  Make it long
$ enough so that the last line of this message is at least as long as
$ the earlier lines.  Don't make it any shorter than it is here.
$ Please put this message on at least two lines, rather than on one long line.

H_IDUPNAME "0611-317 You are trying to put a file into a directory by using\n\
PUT-COPY, PUT-DOWN, or RESTORE.  The file has the same name as another\n\
file in this directory.  To put the file in this directory, enter a new\n\
name for the file or press CANCEL.\n\
\n\
Press CANCEL to continue."
$
IFINDNAME "Internal Error (Cannot find file name.)#1"

H_IFINDNAME "The File Manager cannot perform the operation you last specified.\n\
The File Manager will restart on the current directory.  The last request\n\
you entered may not have been successful.\n\
\n\
Press CANCEL to continue."
$
IFORM "Internal Error (Cannot find index form.#1"
$ Do not translate "index form"

H_IFORM "The File Manager cannot perform the operation you last specified.\n\
The File Manager will restart on the current directory.  The last request\n\
you entered may not have been successful.\n\
\n\
Press CANCEL to continue."
$
IINDEXSV "You cannot delete %s.#1"

H_IINDEXSV "0611-318 You are trying to copy or delete a file whose name begins with\n\
.index.  These files cannot be deleted or copied.\n\
\n\
Press CANCEL to continue."
$ Do not translate ".index".
$
IINSREC "Internal Error (Cannot insert record %d in SFILE object.)#1"
$ Do not translate  SFILE object

H_IINSREC "The File Manager cannot perform the operation you last specified.\n\
The File Manager will restart on the current directory.  The last request\n\
you entered may not have been successful.\n\
\n\
Press CANCEL to continue."
$
IMODFIELD "Internal Error (Cannot modify field %s.)#1"

H_IMODFIELD "The File Manager cannot perform the operation you last specified.\n\
The File Manager will restart on the current directory.  The last request\n\
you entered may not have been successful.\n\
\n\
Press CANCEL to continue."
$
INOARGS "The specified command does not take any input parameters.#1"

H_INOARGS "0611-319 You tried to provide a parameter to a command that does not\n\
accept any input parameters.\n\
\n\
Press CANCEL to continue."
$
INOBLANK "You cannot delete a file by blanking out its name.#1"

H_INOBLANK "0611-320 You deleted all of the characters of the name of a file,\n\
but you have not deleted the file.  Files must be deleted by using\n\
DELETE-LINE or PICK-UP.  The name of the file will be restored.\n\
\n\
Press CANCEL to continue."
$
INOCHANGE "Cannot change the owner permissions on this file.#1"

H_INOCHANGE "0611-321 You tried to change the owner permissions of the current\n\
directory or the .index file.  You must own both of these files in order\n\
to work in this directory using the File Manager.  You cannot change\n\
the permissions of these files.\n\
\n\
Press CANCEL to continue."
$ Do not translate ".index".
$
INOCHDIR "Cannot move to directory %s.#1"

H_INOCHDIR "0611-322 You tried to copy a directory, but the system cannot move to\n\
the specified directory. It is not copied.\n\
\n\
Press CANCEL to continue."
$
INOCHDIRF "Cannot change to directory %s.#1"

H_INOCHDIRF "0611-323 The File Manager tried to start again on the specified directory,\n\
but the File Manager cannot move to the specified directory. Make sure that\n\
the directory still exists.\n\
\n\
Press CANCEL to continue."
$
INODEBUG "Cannot write debugging output into .po#1"
$ Do not translate ".po"

H_INODEBUG "The File Manager cannot open the debugging output file, so all\n\
debugging information will be lost.\n\
\n\
Press CANCEL to continue."
$
INODIRMOD "You do not have permission to modify this directory.#1"

H_INODIRMOD "0611-324 You tried to edit a directory that does not provide you with\n\
access permission.  If you own the directory, change the access permission\n\
to allow write permission.  If you do not own the directory, have the\n\
the owner of the directory or the system administrator change the\n\
permissions, then try again.\n\
\n\
Press CANCEL to continue."
$
INOFILE "There is no file for this directory slot.#1"

H_INOFILE "0611-325 You are trying to edit directory details on a line that has no\n\
file listed.   Create the file, then modify its details.\n\
\n\
Press CANCEL to continue."
$
INOFRESTOR "File %s was permanently deleted from\n\
your putdir directory and cannot be restored.#1"

H_INOFRESTOR "0611-326 You have deleted a file from your putdir directory and have tried\n\
to restore it.  Once a file has been removed from the putdir directory with\n\
PICK-UP or DELETE-LINE, it is gone and cannot be restored.\n\
\n\
To copy a file from the putdir directory to another directory, use PICK-COPY.\n\
\n\
Press CANCEL to continue."
$ Do not translate "putdir".
$
INOJOIN "JOIN is not available when editing directories.#1"

H_INOJOIN "0611-327 You cannot join two lines in the File Manager using\n\
ENTER DELETE-LINE or ENTER PICK-UP.  Lines in the File Manager cannot be\n\
combined because each line is used to display the name of a separate file.\n\
\n\
Press CANCEL to continue."
$
INOMKDIR "Cannot to create directory %s.#1"

H_INOMKDIR "0611-328 You tried to save or create a directory, but the File Manager\n\
cannot do this.  Contact your system administrator.\n\
\n\
Press CANCEL to continue."
$
INOMV "Cannot rename directory %s.#1"

H_INOMV "0611-329 You tried to rename a directory by typing over the old name,\n\
but the system failed to accept the new name.  Contact your system\n\
administrator.\n\
\n\
Press CANCEL to continue."
$
INOPRF "Cannot find the editor profile files.#1"

H_INOPRF "0611-330 The system cannot find the editor profile files, possibly\n\
because the system version was not installed correctly.  Contact\n\
your system administrator.\n\
\n\
Press CANCEL to continue."
$
INOPD "There is no available place to save the files.#1"

H_INOPD "0611-331 You tried to move or copy files using DELETE-LINE,\n\
PICK-UP, or PICK-COPY.  The files are ordinarily saved before they are\n\
moved, but the File Manager cannot perform the save. Either you do not\n\
have write permission in your putdir directory, or your user id has\n\
changed.  Make sure your $HOME/.putdir directory allows you write\n\
permission.\n\
\n\
Press CANCEL to continue."
$ Do not translate "putdir", "$HOME/.putdir".
$
INOREGION "This command does not accept region input parameters.#1"

H_INOREGION "0611-332 You tried to provide a region input to a command that\n\
does not accept region inputs.  Region inputs are defined using\n\
ENTER, BOX-MARK, or TEXT-MARK and the positioning functions such as\n\
the UP-ARROW or TAB keys.\n\
\n\
Press CANCEL to continue."
$
INOREMOVE "Cannot remove lines while viewing the file details.#1"

H_INOREMOVE "0611-333 You tried to remove a line while viewing detailed information\n\
about a file.  You cannot remove entire lines from this display using\n\
DELETE-LINE and PICK-UP.\n\
\n\
To edit the file details, position the cursor in a field that can be\n\
edited, then type over the value displayed.\n\
\n\
Press CANCEL to continue."
$
INORESTORE "Lines cannot be restored while viewing directory details.#1"

H_INORESTORE "0611-334 You tried to put a line in the display of detailed information\n\
about a file.  You are not allowed to put data into fields in this\n\
display using PUT-DOWN, PUT-COPY, or RESTORE.\n\
\n\
To edit the file details, position the cursor in a field that can be\n\
edited, then type over the value displayed.\n\
\n\
Press CANCEL to continue."
$
INOTAVAIL "This function is not available from the current display.#1"

H_INOTAVAIL "0611-335 You selected one of the LOCAL-MENU options that is not\n\
available at this level in the file.  To determine which of the options\n\
can be used, use LOCAL-MENU.\n\
\n\
Press CANCEL to continue."
$
IOPEN "Cannot open file %s.#1"

H_IOPEN "The File Manager cannot perform the operation you last specified.\n\
The File Manager will restart on the current directory.  The last request\n\
you entered may not have been successful.\n\
\n\
Press CANCEL to continue."
$
IOPENDIR "Cannot open directory %s.#1"

H_IOPENDIR "The File Manager cannot perform the operation you last specified.\n\
The File Manager will restart on the current directory.  The last request\n\
you entered may not have been successful.\n\
\n\
Press CANCEL to continue."
$
IOWNSET "Changing the ownership or group membership of a file will turn off\n\
the set-user-ID and set-group-ID bits.\n\
Press EXECUTE to confirm the change.\n\
Press CANCEL to retain the old ownership and group.#1"
$ The phrases "set-user-ID" and "set-group-ID" should be translated
$ consistently with the way they are translated in the CHMOD entry in the
$ Commands Reference Manual.

H_IOWNSET "0611-336 You tried to change the owner or group membership of the file.\n\
Changing owner or group membership automatically clears the set-user-id\n\
and set-group-id bits.  If you do not want to change these bits because you\n\
are writing a program that uses them, press CANCEL.\n\
Otherwise, press EXECUTE.\n\
\n\
Press CANCEL to continue."
$ The phrases "set-user-ID" and "set-group-ID" should be translated
$ consistently with the way they are translated in the CHMOD entry in the
$ Commands Reference Manual.
$
IPARSV "Cannot delete the parent directory.#1"

H_IPARSV "0611-337 You are trying to delete or copy the parent directory\n\
using DELETE-LINE, PICK-UP, or PICK-COPY.  While you are in a\n\
directory, you cannot delete or copy the parent directory.\n\
\n\
Press CANCEL to continue."
$
IPUTCURS "Internal Error (Cannot put cursor in field %s.)#1"

H_IPUTCURS "The File Manager cannot perform the operation you last specified.\n\
The File Manager will restart on the current directory.  The last request\n\
you entered may not have been successful.\n\
\n\
Press CANCEL to continue."
$
IREADFILE "Cannot read file %s.#1"

H_IREADFILE "The File Manager cannot perform the operation you last specified.\n\
The File Manager will restart on the current directory.  The last request\n\
you entered may not have been successful.\n\
\n\
Press CANCEL to continue."
$
IREADFIELD "Internal Error (Cannot read field %s.)#1"

H_IREADFIELD "The File Manager cannot perform the operation you last specified.\n\
The File Manager will restart on the current directory.  The last request\n\
you entered may not have been successful.\n\
\n\
Press CANCEL to continue."
$
IRENAME "Cannot rename file %s.#1"

H_IRENAME "The File Manager cannot perform the operation you last specified.\n\
The File Manager will restart on the current directory.  The last request\n\
you entered may not have been successful.\n\
\n\
Press CANCEL to continue."
$
IRESTOR "Restoring directory hierarchy %s."
$
IRMDIR "Cannot remove directory %s.#1"

H_IRMDIR "0611-338 You tried to use the File Manager to delete or restore a\n\
directory, but the File Manager cannot do so.  Contact your\n\
system administrator.\n\
\n\
Press CANCEL to continue."
$
IROFIELD "This field cannot be modified.#1"

H_IROFIELD "0611-339 You tried to change one of the fields in the display of\n\
detailed file information. You tried to edit a field in this display \n\
that cannot be edited.  To edit a field, position the cursor in a field\n\
that can be edited, and type in the value you wish to insert.\n\
\n\
Press CANCEL to continue."
$
ISAVE "Saving the directory hierarchy %s."
$
ISGIDOFF "Changing the file permission implies that\n\
the set-group-ID bit will be turned off.\n\
Press EXECUTE to confirm the permission change.\n\
Press CANCEL to retain the old permissions.#1"

H_ISGIDOFF "0611-340 You tried to change the permissions for a file.  This\n\
automatically clears the set-group-id bit.  If you do not want\n\
to change this bit because you are writing a program that uses it,\n\
press CANCEL.  Otherwise, press EXECUTE.\n\
\n\
Press CANCEL to continue."
$ The phrase "set-group-ID bit" should be translated
$ consistently with the way it is translated in the CHMOD entry in the
$ Commands Reference Manual.
$
ISPECIAL "Cannot save a special file.#1"

H_ISPECIAL "0611-341 You tried to save a special system file.  These files cannot\n\
be saved.\n\
\n\
Press CANCEL to continue."
$
ISTAT "Cannot get information about file %s.#1"

H_ISTAT "The File Manager cannot perform the operation you last specified.\n\
The File Manager will restart on the current directory.  The last request\n\
you entered may not have been successful.\n\
\n\
Press CANCEL to continue."
$
ISTATDIR "Cannot get information about directory %s.#1"

H_ISTATDIR "The File Manager cannot perform the operation you last specified.\n\
The File Manager will restart on the current directory.  The last request\n\
you entered may not have been successful.\n\
\n\
Press CANCEL to continue."
$
ISTATS "Getting details about each file."
$
ISTICKY "Changing the set_user-ID bit, set_group-ID bit, or the\n\
permissions of a file will turn off the save-text or sticky bit.\n\
Press EXECUTE to confirm the change.\n\
Press CANCEL to retain the old status.#1"
$ The "sticky" bit is the bit associated with a file that says that the
$ program that is stored in the file is one that will be run frequently,
$ and consequently should be kept in memory, ready for the next use.
$ Translate the phrase "sticky bit" as it is translated in the CHMOD section
$ of the Commands Reference Manual.
$ The phrases "set-user-ID" and "set-group-ID" should be translated
$ consistently with the way they are translated in the CHMOD entry in the
$ Commands Reference Manual.

H_ISTICKY "0611-342 You are trying to change file permissions.  This\n\
automatically clears the save-text  or sticky bit. If you do not\n\
want to change this bit because you are writing a program that uses\n\
it, press CANCEL.  Otherwise, press EXECUTE.\n\
\n\
Press CANCEL to continue."
$
ITOOLONG "Internal Error (Field %s is too long.)#1"

H_ITOOLONG "The File Manager cannot perform the operation you last specified.\n\
The File Manager will restart on the current directory.  The last request\n\
you entered may not have been successful.\n\
\n\
Press CANCEL to continue."
$
IU9MENU1 "special functions menu display#1"

H_IU9MENU1 "You used LOCAL-MENU to display File Manager functions.  To select\n\
an option, move the cursor to one of the items and press EXECUTE,\n\
or use the function key preceding the option.  Press CANCEL to\n\
return to the menu.\n\
\n\
(1) Display Visible Files.  Displays files that are normally visible.\n\
(2) Display All Files.  Displays all files, including hidden files as\n\
    specified in the File Manager profile $HOME/profiles/indexprf.\n\
(4) Show Details About Files.  Displays  the status of multiple files.\n\
(5) Show More Details About This File. Displays a detailed file status.\n\
\n\
Press CANCEL to continue."
$ Do not translate "indexprf" or "$HOME/profiles/indexprf"
$
MN_IU9MENU1 "\t       File Manager\n\
\n\
Select a menu option by moving the cursor to\n\
an item and presssing EXECUTE, or press CANCEL\n\
to remove the menu or HELP to display\n\
help information.\n\n\
@(1)              Display Visible Files\n\
@(2)              Display All Files\n\
@(4)              Show Details About Files\n\
@(5)              Show More Details About This File#4"
$
$ Do not translate the '@', '\n', '\t', or '#4'
$ Indent the text exactly as shown.
$ Keep all lines about 50 characters long.

H_IU9MENU11 "Select this option to display visible files.  These are all files\n\
in your directory except those defined as hidden.  The File Manager\n\
normally hides files specified as hidden by the system or by your\n\
indexprf file $HOME/profiles/indexprf in your profiles directory.\n\
\n\
Hidden files as defined by the system are:\n\
   (1) Files whose name starts with the . character such as .putdir.\n\
   (2) Files with the suffixes .bak, .old, or .index  such as note.bak.\n\
\n\
Edit the indexprf file to define your own set of hidden files.\n\
\n\
Press CANCEL to continue."
$ Do not translate "indexprf" or "$HOME/profiles/indexprf"
$ Do not translate .bak .old .index note.bak .putdir
$
H_IU9MENU12 "Select this option to display all files in the directory you are\n\
editing.  This option displays both the files normally displayed by\n\
the File Manager, and those files defined as hidden by the system or\n\
by the indexprf file in your profiles directory.\n\
\n\
Hidden files as defined by the system are:\n\
   (1) files whose name starts with the . character such as .putdir.\n\
   (2) files with the suffixes .bak, .old, or .index such as note.bak.\n\
\n\
Hidden files can also be defined in your indexprf file\n\
$HOME/profiles/indexprf.  The files listed in the indexprf file are used\n\
in place of the system definition to determine which files are hidden.\n\
\n\
Press CANCEL to continue."
$ Do not translate "indexprf" or "$HOME/profiles/indexprf"
$ Do not translate .putdir .bak .old .index note.bak
$
H_IU9MENU13 "Select this option to see file status information on the files\n\
displayed on the initial File Manager screen.\n\
\n\
Press CANCEL to continue."
$
H_IU9MENU14 "Select this option to show detailed file status information about the\n\
file on which the cursor was located.  Detailed file status information\n\
consists of:\n\
\t(1) Description, including the full path of the file and the size.\n\
\t(2) Ownership, including the owner, group, uid, and gid of the file.\n\
\t(3) Permissions of the file.\n\
\t(4) File access times and dates, including the times of the last file\n\
\t    access and last data modification.\n\
Some of these fields can be modified by typing over the existing\n\
value.  Other fields cannot be edited.\n\
\n\
Press CANCEL to continue."
$
IU9MENU2 "special functions menu display#1"

H_IU9MENU2 "You used LOCAL-MENU to display the File Manager functions.  To select\n\
an option, move the cursor to one of the items and press EXECUTE, or use\n\
the function key or the command preceding the option.\n\
Press CANCEL to return to the menu.\n\
\n\
(3) Return to the normal directory display.  Returns to the initial File\n\
    Manager display of the File and Description fields.\n\
(4) ZOOM-OUT  Show Details About Files. Displays the status of multiple\n\
    files.\n\
\n\
Press CANCEL to continue."
$
MN_IU9MENU2 "\t       File Manager\n\
\n\
Select a menu option by moving the cursor to\n\
an item and presssing EXECUTE, or press CANCEL\n\
to remove the menu or HELP to display\n\
help information.\n\n\
@(3)              Return to normal directory display\n\
@(4) or ZOOM-OUT  Show Details About Files#2"
$
$ Do not translate the '@', '\n', '\t', or '#2'
$ Indent the text exactly as shown.
$ Keep all lines about 50 characters long

H_IU9MENU21 "Select this option to return to the normal directory screen which\n\
displays the File and Description fields.\n\
\n\
Press CANCEL to continue."

H_IU9MENU22 "Select this option to see the file status information of the files\n\
displayed on the initial File Manager screen.\n\
\n\
Press CANCEL to continue."
$
IU9MENU3 "special functions menu display#1"

H_IU9MENU3 "You used LOCAL-MENU to display the File Manager functions.  To select\n\
an option, move the cursor to one of the items and press EXECUTE, or use\n\
the function key or command preceding the option.\n\
Press CANCEL to return to the menu.\n\
\n\
(1) Display Visible Files.  Displays files that are normally visible.\n\
(2) Display All Files.  Displays all files, including hidden files as\n\
    specified in the File Manager profile $HOME/profiles/indexprf.\n\
(3) Return to Normal Directory Display.  Returns to the initial File\n\
    Manager display of the File and Description fields.\n\
(5) ZOOM-IN  Show More Details About This File. Displays detailed file\n\
    status.\n\
\n\
Press CANCEL to continue."
$ Do not translate "indexprf" or "$HOME/profiles/indexprf"
$
MN_IU9MENU3 "\t       File Manager\n\
\n\
Select a menu option by moving the cursor to\n\
an item and presssing EXECUTE, or press CANCEL\n\
to remove the menu or HELP to display\n\
help information.\n\n\
@(1)              Display Visible Files\n\
@(2)              Display All Files\n\
@(3)              Return to Normal Directory Display\n\
@(5) or ZOOM-IN   Show More Details About This File#4"
$
$ Do not translate the '@', '\n', '\t', or '#4'
$ Indent the text exactly as shown.
$ Keep all lines about 50 characters long
$
H_IU9MENU31 "Select this option to display visible files.  These are all files\n\
in your directory except those defined as hidden.  The File Manager\n\
normally hides files specified as hidden by the system or by the\n\
indexprf file $HOME/profiles/indexprf in your profiles directory.\n\
\n\
Hidden files as defined by the system are:\n\
   (1) files whose name starts with the . character such as .putdir.\n\
   (2) files with the suffixes .bak, .old, or .index such as note.bak.\n\
\n\
Edit the indexprf file to define your own set of hidden files.\n\
\n\
Press CANCEL to continue."
$ Do not translate "indexprf" or "$HOME/profiles/indexprf"
$ Do not translate .bak .old .index note.bak .putdir
$
H_IU9MENU32 "Select this option to display all files in the directory you are\n\
editing.  This option displays both the files normally displayed by\n\
the File Manager, and those files defined as hidden by the system or\n\
by the indexprf file in your profiles directory.\n\
\n\
Hidden files as defined by the system are:\n\
   (1) files whose name starts with the . character such as .putdir.\n\
   (2) files with the suffixes .bak, .old, or .index such as note.bak.\n\
\n\
Hidden files can also be defined in the indexprf file\n\
$HOME/profiles/indexprf.  The files listed in the indexprf file are used\n\
in place of the system definition to determine which files are hidden.\n\
\n\
Press CANCEL to continue."
$ Do not translate "indexprf" or "$HOME/profiles/indexprf"

H_IU9MENU33 "Select this option to return to the normal directory screen which\n\
displays the File and Description fields.\n\
\n\
Press CANCEL to continue."

H_IU9MENU34 "Select this option to show detailed file status information about the\n\
file on which the cursor was located.  Detailed file status information\n\
consists of:\n\
\t(1) Description, including the full path of the file and the size.\n\
\t(2) Ownership, including the owner, group, uid, and gid of the file.\n\
\t(3) Permissions of the file.\n\
\t(4) File access times and dates, including the times of the last file\n\
\t    access and last data modification.\n\
Some of these fields can be modified by typing over the existing\n\
value.  Other fields cannot be edited.\n\
\n\
Press CANCEL to continue."
$
IUPD "Internal Error (Cannot update field %s.)#1"

H_IUPD "The File Manager cannot perform the operation you last specified.\n\
The File Manager will restart on the current directory.  The last request\n\
you entered may not have been successful.\n\
\n\
Press CANCEL to continue."
$
IUTIME "File %s: cannot reset the time to %s.#1"

H_IUTIME "0611-343 You tried to change the file access or modification times.\n\
The File Manager cannot make the change.  Contact your\n\
system administrator.\n\
\n\
Press CANCEL to continue."
$
IWRITEFILE "Internal Error (Cannot write file %s.)#1"

H_IWRITEFILE "The File Manager cannot perform the operation you last specified.\n\
The File Manager will restart on the current directory.  The last request\n\
you entered may not have been successful.\n\
\n\
Press CANCEL to continue."
$
IWRITEFIELD "Internal Error (Cannot write field %s.)#1"

H_IWRITEFIELD "The File Manager cannot perform the operation you last specified.\n\
The File Manager will restart on the current directory.  The last request\n\
you entered may not have been successful.\n\
\n\
Press CANCEL to continue."
