$ @(#)93        1.4  src/tcpip/usr/bin/whois/whois.msg, tcpadmin, tcpip411, GOLD410 4/12/91 17:31:17
$ 
$ COMPONENT_NAME: TCPIP whois.msg
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
$ message for whois
$

$set MS_WHOIS

$quote "

NME_NT_FND	"whois: Unknown host %s\n"
BAD_ADDR	"whois: Address %s misformed\n"
SOCKET	 	"whois: socket"
BIND	 	"whois: bind"
UNKSERVICE	 "whois: whois/tcp: unknown service\n"
CONNECT	 	"whois: connect"
FDOPEN	 	"fdopen"
USAGE	 	"usage: whois [-h host] name\n"
