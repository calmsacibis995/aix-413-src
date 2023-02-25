$ ENGLISH_SOURCE_VERSION= 1.10.2.5
$ @(#)04        1.2  src/msg/bos/base/ksh.msg, bosmsg, msg411, GOLD410 4/7/94 11:58:27
$
$ COMPONENT_NAME: (CMDKSH) Korn Shell
$
$ FUNCTIONS:
$
$ ORIGINS: 27, 85
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
$ (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
$ ALL RIGHTS RESERVED
$
$ OSF/1 1.2
$

$
$quote  "
$set MS_KSH
$ Do not translate ksh


$ $
$ $ from msg.c
$ $

$ $     information or multiple message text

e_access        "Permission denied."
e_alias         " ksh alias not found."
$ The alias is printed out before this message.
$ alias refers to the kshell alias.
e_aliname       "Invalid alias name."
e_ambiguous     "Job name is ambiguous."
e_atline        " at line "
$ This is preceded by E_SYNBAD and is followed by a line number.
e_format        "The umask permission format is not correct, use chmod parameter syntax."
$ Do not translate umask, chmod
e_found         " not found."
$ This is preceded by a filename, or whatever could not be found.
e_function      "Arithmetic expression contains a function that is not known."
e_ident         "This is not an identifier."
$ This is preceded by the bad identifier name, which is followed by a colon.
e_logout        "Use 'exit' to logout."
$ Do not translate exit.
e_mailmsg       "You have mail in $_"
$       do not translate $_
e_off           "off"
$ This is used in a table for output from a query command
e_on            "on"
$ This is used in a table for output from a query command
e_pwd           "Cannot access parent directories."
e_readonly      "This variable is read only."
$ This is preceded by the read only variable name, which is followed by
$ a colon.
e_timewarn      "\r\n\007Shell will time out in 60 seconds."
$       do not translate \r\n\007
e_timeout       "Timed out waiting for input."

e_unexpected    " is not expected."
$ This is preceded by the symbol that was not expected, which is followed
$ by a colon.  Leave the space at the front.

e_unmatched     " is not matched."
$ This is preceded by the symbol that was not matched, which is followed
$ by a colon.  Leave the space at the front.

e_heading       "Current option settings are:"
$       header for printing current options in args.c

e_endoffile     "end of file"
$       end of file token as printed in cmd.c

$ $     job control information messages
e_coredump      "(coredump)"
$ Do not translate
e_Done          " Done"
$ Leave a space at the start. This comes out on the line to show
$ when a background job has completed.
e_jobusage      "Specify a process identifier or a %job number."
$ This tells the user to specify a process id or a % that is
$ followed by the job number.
e_no_jctl       "0403-001 Job control has been disabled."
e_no_job        "0403-002 The specified job does not exist."
e_no_proc       "0403-003 The specified process does not exist."
e_Running       " Running"
$ This is a status message, that is preceded by a job number.
e_running       "There are running jobs."
e_terminate     "There are stopped jobs."
e_unknown       "<job name not known>"

e_real          "\nreal"
$        Information message from time builtin command
$        Actual elapsed time
e_sys           "sys"
$        Information message from time builtin command
$        Kernel time
e_user          "user"
$        Information message from time builtin command
$        User time

$ $     information messages in name.c

is_reserved     " is a reserved word."
$ This and the next 8 messages are output from the kshell whence command
$ They are preceded by the name that is being queried.
is_builtin      " is a shell builtin."
$ builtin is referring to a function that is contained within the
$ shell program.
is_alias        " is an alias for "
$ This is followed by an alias name
is_function     " is a function."
is_xalias       " is an exported alias for "
$ This is followed by an alias name
is_talias       " is a tracked alias for "
$ This is followed by an alias name
is_xfunction    " is an exported function."
is_ufunction    " is an undefined function."
is_             " is "
$ This is followed by


$ $     general error messages

e_argexp        "0403-004 Specify a parameter with this command."
e_create        "0403-005 Cannot create the specified file."
e_exec          "0403-006 Execute permission denied."
e_file          "0403-007 Generated or received a file descriptor number that is not valid."
e_nargs         "0403-008 The number of parameters specified is not correct."
e_number        "0403-009 The specified number is not valid for this command."
e_option        "0403-010 A specified flag is not valid for this command."
e_subst         "0403-011 The specified substitution is not valid for this command."
e_testop        "0403-012 A test command parameter is not valid."
e_toobig        "0403-013 There is not enough memory available to run the command."
e_txtbsy        "0403-015 Cannot access a required executable file. It is in use."
e_open          "0403-016 Cannot find or open the file."
e_pipe          "0403-017 Cannot create a pipe."
$ Do not translate pipe.

e_prohibited    "0403-018 Login shells that are setuid or setgid are not allowed."
$ Do not translate setuid/setgid, login
$       error for setuid/setgid shells in main.c

e_restricted    "0403-019 The operation is not allowed in a restricted shell."
$       error for restricted shell operation

e_bracket       "0403-021 A ] character is missing."

e_history       "0403-022 Cannot access or open the history file."
$ Do not translate history.
$       error messge when history file open fails

$ $     error messages from failed execve in service.c
e_arglist       "0403-027 The parameter list is too long."

e_space         "0403-029 There is not enough memory available now."
$       error message for allocating memory in blok.c

e_fork          "0403-030 The fork function failed. Too many processes already exist."
$ Do not translate fork.
$       from fork command in xec.c

e_swap          "0403-031 The fork function failed. There is not enough memory available."
$ Do not translate fork

e_no_start      "0403-032 Cannot start job control."
$       error messages for job control

$ $     error messages from builtin cd command
e_longname      "0403-035 The specified path name is too long."
e_direct        "0403-036 The specified directory is not valid."
e_notdir        "0403-037 The specified path name is not a directory."

e_query         "0403-039 No query process for pipe."
$ Do not translate pipe
$       error message for coprocess in builtin.c

$ $     error messages in function getch in macro.c
e_nullset       "0403-040 Parameter null or not set."
e_notset        "0403-041 Parameter not set."

e_pexists       "0403-042 Process already exists for pipe."
$ Do not translate pipe
$       error messages for coprocess pipe in xec.c

e_fexists       "0403-043 The noclobber option was specified and the file already exists."
$ Do not translate noclobber
$       error messages for noclobber option in io.c

e_flimit        "0403-044 Trying to open too many files."
$       error message for max files open in io.c

e_ulimit        "0403-045 The specified value is outside the user's allowable range."
$       error message from ulimit builtin command in builtin.c

e_subscript     "0403-046 The specified subscript cannot be greater than 512."
$       error message in shlib/growaray.c

$ $
$ $ added to msg.c for tsh 
$ $

e_nottrusted	"0403-047 Command must be trusted to run in the trusted shell."
$ trusted is a term used for security classification
e_nooption	"0403-048 The specified flag is not supported in the trusted shell." 
$ trusted is a term used for security classification
e_nousralias	"0403-049 User aliases are not supported in the trusted shell."
$ trusted is a term used for security classification
e_notrkalias	"0403-050 Tracked aliases are not supported in the trusted shell."
$ trusted is a term used for security classification
e_nousrfunc	"0403-051 User functions are not supported in the trusted shell."
$ trusted is a term used for security classification
e_nohistcap	"0403-052 History is not supported in the trusted shell."
$ trusted is a term used for security classification

$ $
$ $ from strdata.c
$ $

e_moretokens    "0403-053 Expression is not complete; more tokens expected."
$ do not translate token.
e_paren         "0403-054 There must be an equal number of ( and ) characters."
e_badcolon      "0403-055 A : character is not used correctly."
e_divzero       "0403-056 Cannot divide by zero."
e_synbad        "0403-057 Syntax error"
$ This is followed by   E_ATLINE  which is followed by a colon then by the
$ bad symbol. This then is followed by  E_UNEXPECTED or E_UNMATCHED
$ Example:
$ cmd: Syntax error at line 3: * is not expected.
e_notlvalue     "0403-058 Assignment requires an lvalue."
$ Do not translate lvalue.
e_recursive     "0403-059 There cannot be more than 9 levels of recursion."

$ $     messages that are currently not used because they are
$ $     inside an ifdef that is not defined

e_link          "remote link inactive"
$ Message not used...do not translate
e_multihop      "multihop attempted"
$ Message not used...do not translate
e_newtty        "Switching to new tty driver..."
$ Message not used...do not translate
e_oldtty        "Reverting to old tty driver..."
$ Message not used...do not translate

e_invmul        "0403-065 An incomplete or invalid multibyte character encountered."
e_trap		"bad trap"
e_intbase	"base"

$ $	The following messages for signals should be the same messages
$ $	for signals in libc.msg. You may not want to translate these at all.
$ $	If they are translated, see the libc.msg and the documentation 
$ $	for the signal system call and translate them in whatever way the
$ $	documentation for that system call uses.

e_SIGABRT	"IOT/Abort trap"
e_SIGALRM	"Alarm clock"
e_SIGBUS	"Bus error"
e_SIGCHLD	"Child exited"
e_SIGCLD	"Child exited"
e_SIGCONT	"Continued"
e_SIGEMT	"EMT trap"
e_SIGFPE	"Floating point exception"
e_SIGHUP	"Hangup"
e_SIGILL	"Illegal instruction"
e_SIGINT	"Interrupt"
e_SIGIO		"I/O possible/complete"
e_SIGIOT	"IOT/Abort trap"
e_SIGKILL	"Killed"
e_SIGQUIT	"Quit"
e_SIGPIPE	"Broken pipe"
e_SIGPROF	"Profiling timer expired"
e_SIGPWR	"Power-failure"
e_SIGSEGV	"Segmentation fault"
e_SIGSTOP	"Stopped (SIGSTOP)"
e_SIGSYS	"Bad system call"
e_SIGTERM	"Terminated"
e_SIGTINT	"Interrupt"
e_SIGTRAP	"Trace/BPT trap"
e_SIGTSTP	"Stopped (SIGTSTP)"
e_SIGTTIN	"Stopped (SIGTTIN)"
e_SIGTTOU	"Stopped (SIGTTOU)"
e_SIGURG	"Urgent I/O condition"
e_SIGUSR1	"User defined signal 1"
e_SIGUSR2	"User defined signal 2"
e_SIGVTALRM	"Virtual timer expired"
e_SIGWINCH	"Window size changes"
e_SIGXCPU	"Cputime limit exceeded"
e_SIGXFSZ	"Filesize limit exceeded"
e_SIGLOST	"IOT/Abort trap"
e_SIGLAB	"Security label changed"
e_SIGMSG	"Monitor mode data"
e_SIGDANGER	"Paging space low"
e_SIGMIGRATE	"Migrate process"
e_SIGPRE	"Programming exception"
e_SIGGRANT	"Monitor mode granted"
e_SIGRETRACT	"Monitor mode retracted"
e_SIGSOUND	"Sound completed"
e_SIGSAK 	"Secure attention"


$ messages for the command builtin
$
is_spcbuiltin	" is a special shell builtin."
e_SIGINFO	"Information request"
