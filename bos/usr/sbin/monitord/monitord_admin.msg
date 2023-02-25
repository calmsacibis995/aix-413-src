$ @(#)01	1.2  src/bos/usr/sbin/monitord/monitord_admin.msg, netls, bos411, 9428A410j 4/4/94 11:08:11
$
$   COMPONENT_NAME: netls
$
$   FUNCTIONS: none
$
$   ORIGINS: 27
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1994
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ Message File:  monitord_admin.msg
$
$ This message file contains administrator messages for the monitord
$ daemon.
$

$ Define the quote character.
$quote "

$set MONITORD_ADMIN
$ Message catalog set for monitord administrator messages.
$
$ All messages in this file should be translated.
$
$ The following words are technical terms and should not be translated unless
$ there is a specific translation in the target language:
$    monitord
$    fork
$    daemon
$    pipe
$    lockfile
$    HEARTBEAT


LOCK_UNABLE 	"0349-010 Monitord is unable to create the lockfile /etc/security/monitord_lock\n"
$ The monitord lock file is a regular file that is create to let the
$ monitord daemon determine if an instance of itself is already running.

FORK_DAEMON 	"0349-011 Monitord is unable to fork the daemon process.\n"
$ A separate process will execute the creation of the daemon.

FORK_CHILD 	"0349-012 Monitord is unable to fork a child for daemon creation.\n"
$ A fork subroutine must be executed as a step in creating the daemon process.

CHANGE_GRP	"0349-013 Monitord is unable to change process groups.\n"
$ A daemon must change the process group that it belongs to.  Otherwise it
$ will belong to the same group that was inherited from its parent process.

PIDMAXOUT 	"0349-014 Monitord ran out of local storage for login tracking.\n"
$ The array that monitord uses to track login processes is full.  

PIPE_NOCREATE 	"0349-015 The monitord pipe could not be created.\n"
$ The pipe is the technical term for an interprocess communication mechanism.

PIPE_NOOPEN 	"0349-016 The monitord pipe could not be opened.\n"
$ The pipe is the technical term for an interprocess communication mechanism.

PIPE_NOREAD 	"0349-017 The monitord pipe could not be read.\n"
$ The pipe is the technical term for an interprocess communication mechanism.

LOOP_EXIT 	"0349-018 The action loop has been exited unexpectedly.\n"
$ The action loop is an infinite loop that waits for input and executes
$ an action based on the input that is received.

CFG_NOSAVE	"0349-019 Monitord could not write to the file /etc/security/monitord.cfg\n"
$ The monitord daemon was not able to save configuration data to a file.

NOT_ROOT	"You must have root privileges to start monitord.\n"
$ Only the root user has permission to run the monitord daemon.

SOFT_USAGE "usage: monitord [-s] [-t Minutes]\n"
$ This is the usage message when SOFTSTOP is defined.

USAGE 	"usage: monitord [-t Minutes]\n"
$ This is the error message that is returned when incorrect arguments are
$ specified when starting monitord.

BAD_MINUTES "monitord: minutes argument must be a whole number.\n"
$ This is the error message that is returned when the Minutes argument is
$ specified incorrectly.

LOCK_MSG "Monitord is running, or cleanup is required.\n : Remove /etc/security/monitord_lock before restarting.\n"
$ The existence of a lockfile indicates that the monitord daemon was started.
$ The colon ':' character is used to conform to the error reporting format
$ used in this application.

NO_SERVERS "Monitord could not find any servers with valid AIX licenses installed.\n"
$ Monitord attempted to locate a license server that could satisfy its license
$ requests.  This message is written to the error log file.

SIG_CAUGHT 	"A signal was caught by monitord.  Monitord shutting down.\n"
$ An interrupt signal was sent to the monitord daemon, causing it to exit.

INIT_FAIL	"SERVER INITIALIZATION FAILED\n"
$ Monitord was unable to contact initialize itself with a server.

SERVER_UNAVAIL	"SERVER UNAVAILABLE FOR LICENSE CHECKOUT\n"
$ A license could not be checked out because the server could not be contacted.

NO_LICS		"NO LICENSE AVAILABLE\n"
$ No licenses are available to be checked out from the server.

REQUEST_FAILED	"MONITORD WAS UNABLE TO OBTAIN A USER LOGIN LICENSE\n"
$ Monitord could not obtain a license for the login process.

SOFT_POLICY	" : Softstop policy.  Login allowed.\n"
$ If SOFTSTOP is defined, then the user is allowed to login without a license.

HEART_FAIL	"HEARTBEAT FAILED\n"
$ The periodic check for the server failed.

BAD_LIC_RELEASE	"INVALID LICENSE RELEASE\n"
$ A license was returned to the server, but it was not a valid license.

NO_SVR_RELEASE	"SERVER UNAVAILABLE FOR LICENSE RELEASE\n"
$ The server could not be reached when attempting to release a license.

NOCLEAN		"CLEANUP FAILED -- LICENSES MAY STILL BE HELD\n"
$ An error occurred when monitord attempted to release held licenses before 
$ shutting down.

