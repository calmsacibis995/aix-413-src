$ @(#)78	1.3  src/tcpip/usr/bin/rwho/rwho.msg, tcpadmin, tcpip411, GOLD410 1/3/90 11:35:42
$ 
$ COMPONENT_NAME: TCPIP rwho.msg
$ 
$ FUNCTIONS: 
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
$ messages for rwho
$

$set MS_RWHO

$quote "

USAGE		"usage: rwho [-a]\n"
RWHOCD          "/usr/spool/rwho"
$				Perror will print this followed by the system error message
$				when the chgdir() system call fails. If the rwho default
$				dir is changed in rwho.c, this string should be changed to
$				also.
TOOMANYUSERS    "too many users\n"
$				If 1000 or more users are found, the program exits.
$				NUSER is set to 1000 and is the size of an array to hold
$				host names, idle times, and user information.
HOSTNM          "%s:%s"
$				Format for host name and tty name. These are printed in a
$				buffer[BUFSIZ] and then output in TERM format below.
TERM            "%-8.8s %-*s %.12s"
$				Format for user name followed by the host and tty name in
$				the second %s and finally the user login time. The time
$				is Mon dd hh:mm format. This is done using ctime() and
$				printing the 4th - 16th bytes. If use is made of an
$				NL time function, then the code will have be changed based
$				on the format of the time string.
IDLE            " %2d"
$				Format for idle time hours. Only printed if -a flag.
SPC3            "   "
$				Space for hours idle time if not printed. 
SPC1            " "
$				Space after login time between idle time when no -a flag.
IDLE2           ":%02d"
$				Idle time in minutes format.
CRLF            "\n"
$				Line feed after each user record.
