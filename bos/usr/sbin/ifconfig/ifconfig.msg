$ @(#)09	1.8  src/bos/usr/sbin/ifconfig/ifconfig.msg, cmdnet, bos411, 9428A410j 3/15/94 19:58:18
$ COMPONENT_NAME: (NETCMD) Network commands.
$ FUNCTIONS:
$ ORIGINS: 27
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1989
$ All Rights Reserved
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ Messages for ifconfig
$
$ Note:
$	N_  codes are for info msgs, and error msgs with progname separate
$	NE_ codes are for error messages that include the progname
$	NC_ codes are for error codes only (used only with correspond N_)
$		e.g.:
$			fprintf(stderr, MSGSTR(NE_X, "x: failed\n"));
$		or
$			fprintf(stderr, MSGSTR(NC_X, ""));
$			fprintf(stderr, "%s: ", progname);
$			perror(MSGSTR(N_X, "function"));
$
$ Do not translate ifconfig,netconfig,host, %s, %d, %m, %c, %1$s, %2$s, etc....
$
$ Message numbers have been assigned a range of 500-599.

$len
$quote " 	        define quote character.
$set MS_IFCONFIG        Define initial set#

$ NOTE:
$ Lines beginning with "$ :" are deleted lines. (Ignore them)
$ A further revision will remove them, but they are
$ currently left in to document the removed codes
$ (perhaps useful for idd?)
$ -- Dave Burton 2-20-91

$ :N_AUTHORITY	"\tauthority %s"
$ 1	Output when reporting protection authorities.
$ :NE_BADCLASS	"0821-500 ifconfig: invalid classification protection level (%s)\n"
$ 2	Error message.
$ :NE_BADKMEM	"0821-501 readaddr: open /dev/kmem"
$ Do not translate.
$ 3	Error message for syscall failure.
$ :NE_BADKMEM2	"0821-502 readsymbol: open /dev/kmem"
$ Do not translate.
$ 3	Error message for syscall failure.
$ :NE_BADLSEEK	"0821-503 readaddr: lseek fail"
$ Do not translate.
$ 4	Error message for syscall failure.
$ :NE_BADLSEEK2	"0821-504 readsymbol: lseek fail"
$ Do not translate.
$ 5	Error message for syscall failure.
$ :NE_BADPROT	"0821-505 netconfig: invalid protection authority flag(s) (%s)\n"
$ 6	Error message.
$ :NE_BADREAD	"0821-506 readaddr: read fail"
$ Do not translate.
$ 7	Error message for syscall failure.
$ :NE_BADREAD2	"0821-507 readsymbol: read fail"
$ Do not translate.
$ 8	Error message for syscall failure.
NE_BADVAL	"0821-508 %s: bad value\n"
$ 9	Output when cannot resolve inet address.
N_BROADCAST	"broadcast %s"
$ Do not translate.
$ 10	Output when reporting broadcast address.
NE_CFGKMOD	"0821-509 sysconfig(SYS_CFGKMOD) failed! \n"
$ Translate only "failed!"
$ 11	Output when sysconfig cannot do a CFGKMOD.
NE_EENTRY	"0821-510 ifconfig: error calling entry point for "
$ 12	Output when sysconfig cannot call INIT/TERM routine for loaded driver.
NC_ENCAP	"0821-511 "
N_ENCAP		"Encapsulation Routing"
$ 13	Output when reporting socket options set.
NE_ERR_LOAD	"0821-512 ifconfig: error loading "
$ 14	Output when sysconfig cannot load driver.
NE_EUNLOAD	"0821-513 ifconfig: error unloading "
$ 15	Output when sysconfig cannot unload driver.
N_FLAGS		"flags"
$ Do not translate.
$ 16	Output when printing interface status.
N_INET		"\tinet %s "
$ Do not translate.
$ 17	Output when printing inet status.
NE_KEXT		"0821-514 ifconfig: error querying kernel extension "
$ 18	Output when sysconfig cannot query kernel about load status.
NE_KLOAD	"0821-515 ifconfig: error loading "
$ 19	Output when sysconfig cannot load driver.
N_METRIC	" metric %d"
$ Do not translate.
$ 20	Output when reporting interface status (keyword).
$ :N_MTU		"\n\tmtu: %lu "
$ Do not translate.
$ 21	kernel variable names.
N_NETMASK	"netmask 0x%x "
$ Do not translate.
$ 22	keyword.
N_NOPERM	"%s: permission denied\n"
$ 23	Perror produced message expansion for EPERM.
NE_NOSTAT	"0821-516 ifconfig: can't stat driver path, "
$ Do not translate "stat".
$ 24	A pathname will be printed following this via perror().
N_NOSUCH	"%s: no such interface\n"
$ 25	Perror produced message expansion for ENXIO.
NE_NOTLOAD	"0821-517 ifconfig: %s not loaded\n"
$ 26	Output when sysconfig called to TERM an unloaded driver.
$ :NE_NOT_INIT	"0821-518 network not initialized.\n"
$ 27	Output when checking to determine if ifconfig can do anything.
N_NS		"\tns %s "
$ Do not translate.
$ 28	Output when reporting XNS address (keyword/acronym).
$ :N_SECURITY	"\n\tsecurity %s"
$ 29	Output when reporting inet status (keyword).
NE_SIOCGIFADDR		"0821-519 ioctl (SIOCGIFADDR)"
$ Do not translate.
$ 30	Error message for syscall failure.
$ :NC_SIOCGIFDSTADDR	"0821-520 "
$ :N_SIOCGIFDSTADDR	"ioctl (SIOCGIFDSTADDR)"
NE_SIOCGIFDSTADDR	"0821-521 ioctl (SIOCGIFDSTADDR)"
$ Do not translate.
$ 31	Error message for syscall failure.
NC_SIOCGIFFLAGS		"0821-522 "
N_SIOCGIFFLAGS		"ioctl (SIOCGIFFLAGS)"
$ Do not translate.
$ 32	Error message for syscall failure.
NE_SIOCGIFNETMASK	"0821-523 ioctl (SIOCGIFNETMASK)"
$ Do not translate.
$ 33	Error message for syscall failure.
$ :N_SIOCGIFSECURITY	"0821-524 ioctl (SIOCGIFSECURITY)"
$ Do not translate.
$ 34	Error message for syscall failure.
$ :NE_SIOCGIFSECURITY	"0821-525 ioctl (set security)"
$ Translate only "set".
$ 35	Error message for syscall failure.
$ :NC_SIOCIFBRDADDR	"0821-526 "
$ :N_SIOCIFBRDADDR		"ioctl (SIOCSIFBRDADDR)"
$ Do not translate.
$ 36	Error message for syscall failure.
$ :NC_SIOCIFDSTADDR	"0821-527 "
$ :N_SIOCIFDSTADDR		"ioctl (SIOCSIFDSTADDR)"
$ Do not translate.
$ 37	Error message for syscall failure.
NE_SIOCIFMETRIC		"0821-528 ioctl (SIOCGIFMETRIC)"
$ Do not translate.
$ 38	Error message for syscall failure.
$ :NC_SIOCSIFADDR		"0821-529 "
$ :N_SIOCSIFADDR		"ioctl (SIOCSIFADDR)"
$ Do not translate.
$ 39	Error message for syscall failure.
$ :NE_SIOCSIFAUTHORITY	"0821-530 ioctl (set authority)"
$ Translate only "set".
$ 40	Error message for syscall failure.
NE_SIOCSIFMETRIC	"0821-531 ioctl (set metric)"
$ Translate only "set".
$ 41	Error message for syscall failure.
NE_SIOCSIFMTU		"0821-532 ioctl (SIOCSIFMTU)"
$ Do not translate.
$ 42	Error message for syscall failure.
$ :NC_SIOCSIFNETMASK	"0821-533 "
$ :N_SIOCSIFNETMASK	"ioctl (SIOCSIFNETMASK)"
$ Do not translate.
$ 43	Error message for syscall failure.
$ :NE_SIOCSIFREMMTU	"0821-534 ioctl (SIOCSIFREMMTU)"
$ Do not translate.
$ 44	Error message for syscall failure.
$ :NE_SIOCSIFSECURITY	"0821-535 ioctl (set security)"
$ Translate only "set".
$ 45	Error message for syscall failure.
$ :NC_SOCKET	"0821-536 "
$ :N_SOCKET	" socket"
$ Do not translate.
$ 46	Output when reporting xns status (keyword).
NE_BADIFACE	"0821-537 ifconfig: unknown interface: %s\n"
$ 47	Invocation error message.
N_USAGE		"usage: %s interface\n%s%s%s%s%s%s%s%s%s%s"
$ Translate only "interface".
$ 48	Command line invocation help.
N_USAGE2	"\t[ af ] [ address [ dest_addr ] ] [ up ] [ down ] [detach]\n"
$ Translate only "address", "dest_addr" (destination address).
$ 49	Command line invocation help.
N_USAGE3	"\t[ netmask mask ] [ broadcast broad_addr ]\n"
$ Translate only "mask","broad_addr".
$ 50	Command line invocation help.
N_USAGE4	"\t[ metric n ]\n"
$ Do not translate.
$ 51	Command line invocation help.
N_USAGE5	"\t[ mtu n ]\n"
$ Do not translate.
$ 52	Command line invocation help.
$ :N_USAGE6	"\t[ remmtu n]\n"
$ Do not translate.
$ 53	Command line invocation help.
N_USAGE7	"\t[ trailers | -trailers ]\n"
$ Do not translate.
$ 54	Command line invocation help.
N_USAGE9	"\t[ hwloop | -hwloop ]\n"
$ Do not translate.
$ 55	Command line invocation help.
N_USAGEA	"\t[ allcast | -allcast ]\n"
$ Do not translate.
$ 56	Command line invocation help.
N_USAGEB	"\t[ arp | -arp ]\n"
$ Do not translate.
$ 57	Command line invocation help.
N_USAGEC	"\t[ bridge | -bridge ]\t[snap | -snap ]\n"
$ Do not translate.
$ 58	Command line invocation help.
N_USAGED	"\t[ debug | -debug ]\n"
$ Do not translate.
$ 59	Command line invocation help.
$ :N_USAGEE	"\t[ security [none | unclassified | confidential | secret | top_secret]\n"
$ Do not translate.
$ 60	Command line invocation help.
$ :N_USAGEF	"\t[ authority [ genser | siop | dsccs-spintcom | dsccs-criticom ]\n"
$ Do not translate.
$ 61	Command line invocation help.
$
$ New messages for 44 port
$
NE_SOCKET	"0821-538 ifconfig: socket"
$ do not translate
NC_SIOCDIFADDR	"0821-539 "
$ do not translate
N_SIOCDIFADDR	"ioctl (SIOCDIFADDR)"
$ do not translate
NC_SIOCAIFADDR	"0821-540 "
$ do not translate
N_SIOCAIFADDR	"ioctl (SIOCAIFADDR)"
$ do not translate
NE_SIOCGIFADDR_ISO	"0821-541 ioctl (SIOCGIFADDR_ISO)"
$ do not translate
N_ISO	"\tiso %s "
$ do not translate
NE_SIOCGIFNETMASK_ISO	"0821-542 ioctl (SIOCGIFNETMASK_ISO)"
$ do not translate
N_NETMASK2	" netmask %s "
$ do not translate
NC_SIOCGIFDSTADDR_ISO	"0821-543 "
$ do not translate
N_SIOCGIFDSTADDR_ISO	"ioctl (SIOCGIFDSTADDR_ISO)"
$ do not translate
NE_ATTEMPT	"0821-544 Attempt to set XNS netmask will be ineffectual\n"
$ do not translate XNS
NE_NEGNSEL	"0821-545 Negative NSEL length is absurd\n"
$ do not translate NSEL
NE_ISONSEL	"0821-546 Setting NSEL length valid only for iso\n"
$ do not translate NSEL, iso
NC_SIOCSIFFLAGS	"0821-547 "
$ note: no message exists - prints name directly
NME_NT_FND	"0821-548 ifconfig: host name %s NOT FOUND\n"
BADADDR		"0821-549 ifconfig: %s: address misformed\n"
UNKWN_HST	"0821-550 ifconfig: %s: unknown host\n"
$
NOT_SUPPORT	"0821-551 ifconfig: %s: Not Supported.\n"
BADSUBCHAN	"0821-552 bad subchannel address\n"
NE_SIOCSIFSUBCHAN "0821-553 ioctl (set subchannel addr.)"
NE_SIOCIFDETACH "0821-554 ioctl (SIOCIFDETACH)."
NE_SIOCIFATTACH "0821-555 ioctl (SIOCIFATTACH)."
