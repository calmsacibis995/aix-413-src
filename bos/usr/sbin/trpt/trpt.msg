$ @(#)14	1.1.1.1  src/bos/usr/sbin/trpt/trpt.msg, cmdnet, bos411, 9428A410j 9/29/93 11:48:20
$ 
$ COMPONENT_NAME: TCPIP trpt.msg
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
$ messages for trpt
$

$set MS_TRPT

$quote "

USAGE		"usage: trpt [-afjst] [-p hex-address]\n"
TOO_MANY_PCB    "-p: too many pcb's specified\n"
$ TOO_MANY_PCB	This message is printed when too many tcpcb addresses are 
$ TOO_MANY_PCB	specified.
NO_NMLST        "trpt: no namelist\n"
$ NO_NMLST	This message is printed when there is no name list.
PRINT_SRC       "(src=%s,%d, "
$ PRINT_SRC	This message prints information on the source.
PRINT_DEST      "dst=%s,%d)"
$ PRINT_DEST	This message prints information on the destination.
PRINT_WIN       "(win=%x)"
$ PRINT_WIN	This message prints information on the window.
RXTSHFT		" (t_rxtshft=%d)"
PR2		"\tsnd_wl1 %x snd_wl2 %x snd_wnd %x\n"
PR1		"\trcv_nxt %x rcv_wnd %x snd_una %x snd_nxt %x snd_max %x\n"
TRPT		"trpt: "
MLC_PCBS	"trpt: no memory for tcp_pcbs: "
MLC_DEBUG	"trpt: no memory for tcp_debug: "
