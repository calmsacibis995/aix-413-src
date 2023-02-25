$ @(#)64	1.2  src/tcpip/usr/bin/nslookup/nslookup.msg, tcpras, tcpip411, GOLD410 6/3/93 11:40:20
$
$   COMPONENT_NAME: TCPIP
$
$   FUNCTIONS: data
$
$   ORIGINS: 26 27 65
$
$
$   (C) COPYRIGHT International Business Machines Corp. 1993
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
$ ALL RIGHTS RESERVED 
$
$
$ OSF/1 1.1 snapshot 4

$set MS_NSLOOKUP

$quote "

$ Do not translate nslookup
CANT_RES	"*** Can't initialize resolver.\n"
FIND_SERV	"*** Can't find server name for address %s: %s\n"
DEF_SERV	"*** Default servers are not available\n"
INIT_ADDR	"*** Can't find initalize address for server %s: %s\n"
R_ARROW		"> "
$ Don't translate >
USAGE		"Usage:\n"
USAGE1		"\tnslookup             # interactive mode using default server\n"
USAGE2		"\tnslookup - server    # interactive mode using 'server'\n"
USAGE3		"\tnslookup host                # just look up 'host' using default server\n"
USAGE4		"\tnslookup host server # just look up 'host' using 'server'\n"
INV_NAME	"SetDefaultServer: invalid name %s\n"
$ Don't translate SetDefaultServer
FIND_ADDR	"*** Can't find address for server %s: %s\n"
ERR_WRITE	"*** Can't open %s for writing\n"
R_ARROW1	"> %s\n"
$ Don't translate >
NO_INFO_OPT	"*** No %s information is available for %s\n"
REQ_TOUT	"*** Request to %s timed-out\n"
NO_HOST		"*** %s can't find %s: %s\n"
INV_OPTION	"*** Invalid set command\n"
OPTIONS1	"Default Server:"
OPTIONS2	"Host:"
OPTIONS3	"Set options:\n"
OPTIONS4	"  %sdebug  \t"
$ Don't translate 
OPTIONS5	"  %sdefname\t"
$ Don't translate 
OPTIONS6	"  %ssearch\t"
$ Don't translate 
OPTIONS7	"  %srecurse\t"
$ Don't translate 
OPTIONS8	"  %svc\n"
$ Don't translate 
OPTIONS9	"  %saa\t\t"
$ Don't translate 
OPTIONS10	"  %sd2\t\t"
$ Don't translate 
OPTIONS11	"  %signorec\t" 
$ Don't translate 
OPTIONS12	"  %sprimary\n"
$ Don't translate 
OPTIONS13	"  query=%s\t"
$ Don't translate 
OPTIONS14	"  class=%s\t"
$ Don't translate 
OPTIONS15	"  timeout=%d\t"
$ Don't translate 
OPTIONS16	"  retry=%d\t"
$ Don't translate 
OPTIONS17	"  domain=%s\t"
$ Don't translate 
OPTIONS18	"  search list: "
$ Don't translate 
OPTIONS19	"%s "
$ Don't translate 
OPTIONS20	"\n root=%s\n"
$ Don't translate 
OPTIONS21	"\n"
OPTIONS22	"State info:\n"
OPTIONS23	"  current packet id:       %d\n"
OPTIONS24	"  number of name servers:  %d\n" 
OPTIONS25	"  name_server addresses:   %s\n"
OPTIONS26	"                           %s\n"
OPTIONS27	"Host:"
OPTIONS28	"Host:"
OPTIONS29	"Host:"
HEADER1		"    HEADER:\n"
$ Don't translate HEADER
HEADER2		"\topcode = %s"
$ Don't translate 
HEADER3		", id = %d"
$ Don't translate 
HEADER4		", rcode = %s\n"
$ Don't translate 
HEADER5		"\theader flags: "
HEADER6		" response"
$ Don't translate 
HEADER7		" query"
$ Don't translate 
HEADER8		", auth. answer"
$ Don't translate 
HEADER9		", truncation"
$ Don't translate 
HEADER10	", want recursion"
$ Don't translate 
HEADER11	", recursion avail."
$ Don't translate 
HEADER12	", primary"
$ Don't translate 
HEADER13	"\n\tquestions = %d"
$ Don't translate 
HEADER14	", answers = %d"
$ Don't translate 
HEADER15	", auth.records = %d"
$ Don't translate 
HEADER16	", addtional = %d\n\n"
$ Don't translate 
QUEST1		"    QUESTIONS:\n"
$ Don't translate 
QUEST3		", type = %s"
$ Don't translate 
QUEST4		", class = %s\n"
$ Don't translate 
ANSWERS		"    ANSWERS:\n"
$ Don't translate 
AUTH		"    AUTHORITY RECORDS:\n"
$ Don't translate 
ADD_REC		"    ADDITIONAL RECORDS:\n"
$ Don't translate 
LINE		"\n------------\n"
$ Don't translate 
NAME_TRUNC	"(name truncated?)\n"
RES_DEBUG2_OPT	 "\n\ttype = %s, class = %s, ttl = %u, dlen = %d"
$ Don't translate 
NEWLINE		"\n"
$ Don't translate 
TAB		"\t"
$ Don't translate 
INET_ADDR	"\tinet address = %s\n"
PROTO		", protocol = %d"
PORT		", port = %d\n"
T_DEFAULT	"\taddress, class = %d, len = %d\n"
T_CANON_NME	"\tcanonical name = "
T_MX_OPT	"\tpreference = %d"
T_MX_OPT1	", mail exchanger = "
T_MG_OPT	"\tmail group member = "
T_MB_OPT	"\tmail box = "
T_MR_OPT	"\tmailbox rename = "
T_NS_OPT	"\tnameserver = "
T_PTR_OPT	"\thost name = "
T_HINFO_OPT	"\tCPU=%.*s"
T_HINFO_OPT1	"\tOS=%.*s\n"
T_SOA_OPT1	"\torigin = "
T_SOA_OPT2	"\n\tmail addr = "
T_SOA_OPT3	"\n\tserial=%ld"
T_SOA_OPT4	", refresh=%ld"
T_SOA_OPT5	", retry=%ld"
T_SOA_OPT6	", expire=%ld"
T_SOA_OPT7	", min=%ld\n"
T_MINFO_OPT	"\trequests = "
T_MINFO_OPT1	"\n\terrors = "
T_UINFO_OPT	"\t%s\n"
T_GID_OPT	"\t%cid %ld\n"
T_GID_OPT1	"\t%cid of length %ld?\n"
T_WKS_OPT	"\tinet address = %s, protocol = %d\n\t"
T_WKS_OPT1 	"  %s"
$ Don't translate 
T_WKS_OPT2	" #%d"
$ Don't translate 
T_NULL_OPT	"(type NULL, dlen %d)\n"
T_DEFAULT_OPT	"\t???\n"
$ Don't translate 
ERR_PACKET	"packet size error (%#x != %#x)\n"
SEND_REQUEST	"SendRequest failed\n"
$ Don't translate SendRequest
FAILED		"Failed: %s, num. answers = %d, ns = %d, additional = %d\n"
$ Don't translate Failed
NON_ANSWER	"Non-authoritative answer:\n"
TOOBIG		"Size (%d) too big\n"
AUTHORITIVE	"Authoritative answers can be found from:\n"
RES_MKQ		"Res_mkquery failed\n"
$ Don't translate Res_mkquery
LIST_HOST	"ListHosts: invalid request %s\n"
$ Don't translate ListHosts
LH_MKQ		"ListHosts: Res_mkquery failed\n"
$ Don't translate ListHosts
HOST_DOM	"Host or domain name"
T_ANY_RES	"Resource record info"
INTER_ADDR	"Internet address"
CPU		"CPU"
OS		"OS"
ALIAS_NAME	"Alias"
METRIC		"Metric"
HOST		"Host"
PROTOCOL	"Protocol"
SERVICES	"Services"
POUND		"#"
$ Don't translate
RECORD_REC	"%sReceived %d record%s.\n"
ERR_PRINTING_OPT	"*** Error during listing of %s: %s\n"
ERR_READING	"ListHosts: error receiving zone transfer:\n"
$ Don't translate ListHosts
ERR_READING1	"  result: %s, answers = %d, authority = %d, additional = %d\n"
DLEN		" (dlen = %d?)"
$ Don't translate dlen
STARS		" ***\n"
$ Don't translate
SPACE		" "
$ Don't translate
T_PTR3		" %s = "
$ Don't translate
R_PARENTH	")"
$ Don't translate
VIEW		" view %s"
$ Don't translate
GREP_CMD	"grep \"^ \" %s | sort | more"
$ Don't translate
NO_CHOST	"Finger: no current host defined.\n"
FINGER		" finger %s"
FING_UNKNOWN	"Finger: unknown service\n"
FING		"Finger"
$ Don't translate Finger or finger in the above messages
TCP		"tcp"
$ Don't translate
CAP_W		"/W "
$ Don't translate
LS		" ls %s %s"
$ Don't translate
OPT_A		"-a"
$ Don't translate
OPT_H		"-h"
$ Don't translate
OPT_M		"-m"
$ Don't translate
OPT_S		"-s"
$ Don't translate
OPT_D		"-d"
$ Don't translate
LIST_HOSTS	"ListHosts"
$ Don't translate
S		"s"
$ Don't translate
NULL_STRG	""
$ Don't translate
ROOT		"(root)"
$ Don't translate
SMALL_R		"r"
$ Don't translate
SEND_REQ	"------------\nSendRequest(), len %d\n"
$ Don't translate
SEND_REQ1	"SendRequest"
$ Don't translate
TIMED_OUT	"Timeout %d\n"
OLD_ANSWER	"------------\nOld answer:\n"
TRUNCATED_ANS	"truncated answer\n"
GOT_ANSWER	"------------\nGot answer (%d bytes):\n"
GOT_ANSWER1	"------------\nGot answer:\n"
CALLOC_FAIL	"Calloc failed\n"
$ Don't translate Calloc
ADDRS		"Addresses:"
ADDRS1		"Address:"
ALIASES		"\nAliases:"
SERVED_BY	"Served by:\n"
ARROWS		">> %s"
$ Don't translate
ARROWS_FILE	"> %s"
$ Don't translate
ERR_SUCCESS	"Success"
ERR_FORMAT	"Format error"
ERR_SERVER	"Server failed"
ERR_NODOMAIN	"Non-existent domain"
ERR_NOT_IMP	"Not implemented"
ERR_QUERY	"Query refused"
ERR_NOCHANGE	"No change"
ERR_NOINFO	"No information"
ERR_UNSPEC	"Unspecified error"
ERR_TIMEOUT	"Timed out"
ERR_NOAUTH	"Non-authoritative answer"
BAD_VALUE	"BAD ERROR VALUE"
IN		"IN"
$ Don't translate
CHAOS		"CHAOS"
$ Don't translate
ANY		"ANY"
$ Don't translate
UNKN_QUERY	"unknown query class: %s\n"
UNKN_TYPE	"unknown query type: %s\n"
A		"A"
$ Don't translate
NS		"NS"
$ Don't translate
MX		"MX"
$ Don't translate
CNAME		"CNAME"
$ Don't translate
SOA		"SOA"
$ Don't translate
MB		"MB"
$ Don't translate
MG		"MG"
$ Don't translate
MR		"MR"
$ Don't translate
WKS		"WKS"
$ Don't translate
PTR		"PTR"
$ Don't translate
HINFO		"HINFO"
$ Don't translate
UINFO		"UINFO"
$ Don't translate
MHINFO		"MHINFO"
$ Don't translate
AXFER		"AXFER"
$ Don't translate
MAILB		"MAILB"
$ Don't translate
UNIFO		"UNIFO"
$ Don't translate
UID		"UID"
$ Don't translate
GID		"GID"
$ Don't translate
ADDRESS		"address"
NAMESER		"name server"
MAILEXCH	"mail exchanger"
CANN_NAME	"cannonical name"
START_AUTH	"start of authority zone"
MAIL_DOMAIN	"mailbox domain name"
MAIL_GRP	"mail group member"
MAIL_REN	"mail rename name"
NULL_RESOURCE	"null resource record"
KNOWN_SERVICE	"well known service"
DOMAIN_PTR	"domain name pointer"
HOST_TYPE	"host"
MAIL_MINFO	"mailbox (MINFO)"
$ Don't translate MINFO
ZONE_TRANS	"zone transfer"
MAIL_BOX	"mail box"
ANY_TYPE	"any type"
USER_INFO	"user info"
USER_ID		"user id"
GROUP_ID	"group id"
FORMAT_ARPA	"%u.%u.%u.%u.in-addr.arpa"
$ Don't translate
ALIASED_TO	"Aliased to \"%s\"\n\n"
FORMAT1		"%-30s"
$ Don't translate
FORMAT2		"[%s]"
$ Don't translate
FORMAT3		"%.*s.%.*s"
$ Don't translate
FORMAT4		"[%s]\n"
$ Don't translate
FORMAT5		" %-30s\n"
$ Don't translate
FORMAT6		" %-10s %s\n"
$ Don't translate
FORMAT7		" %3s %s\n"
$ Don't translate
FORMAT8		" %-4s %s\n"
$ Don't translate
FORMAT9		" %-5s"
$ Don't translate
FORMAT10	" %s"
$ Don't translate
FORMAT11	" (%d, %d)"
$ Don't translate
FORMAT12	"\t\t\t%lu"
$ Don't translate
FORMAT13	"\t%lu"
$ Don't translate
FORMAT14	"%.*s"
$ Don't translate
FORMAT15	" %-10s"
$ Don't translate
FORMAT16	" %.*s"
$ Don't translate
FORMAT17	"\t\t%lu"
$ Don't translate
FORMAT18	" %-3d "
$ Don't translate
FORMAT19	" %-3s "
$ Don't translate
FORMAT20	" %d"
$ Don't translate
FORMAT21	" %s. ("
$ Don't translate
FORMAT22	"%s%d"
$ Don't translate
FORMAT23	" type = %-5s"
$ Don't translate
FORMAT24	", class = %-5s"
$ Don't translate
FORMAT25	"\t%lu\n"
$ Don't translate
FORMAT26	"\r\n"
$ Don't translate
FORMAT27	"%-7s  %s\n"
$ Don't translate
FORMAT28	"\n\t"
$ Don't translate
FORMAT29	"%c %s"
$ Don't translate
FORMAT30	"- %s\n\t"
$ Don't translate
FORMAT31	"\n\n"
$ Don't translate
FORMAT32	"%d"
$ Don't translate
HELPMSG		"\
Commands: \t(identifiers are shown in uppercase, [] means optional)\n\
NAME\t\t- print info about the host/domain NAME using default server\n\
NAME1 NAME2\t- as above, but use NAME2 as server\n\
help or ?\t- print info on common commands; see nslookup(1) for details\n\
set OPTION\t- set an option\n\
    all\t\t- print options, current server and host\n\
    [no]debug\t- print debugging information\n\
    [no]d2\t- print exhaustive debugging information\n\
    [no]defname\t- append domain name to each query\n\
    [no]recurse\t- ask for recursive answer to query\n\
    [no]vc\t- always use a virtual circuit\n\
    domain=NAME\t- set default domain name to NAME\n\
    srchlist=N1[/N2/.../N6] - set domain to N1 and search list to N1,N2, etc.\n\
    root=NAME\t- set root server to NAME\n\
    retry=X\t- set number of retries to X\n\
    timeout=X\t- set initial time-out interval to X seconds\n\
    querytype=X\t- set query type, e.g., A,ANY,CNAME,HINFO,MX,NS,PTR,SOA,WKS\n\
    type=X\t- synonym for querytype\n\
    class=X\t- set query class to one of IN (Internet), CHAOS, HESIOD or ANY\n\
server NAME\t- set default server to NAME, using current default server\n\
lserver NAME\t- set default server to NAME, using initial server\n\
finger [USER]\t- finger the optional USER at the current default host\n\
root\t\t- set current default server to the root\n\
ls [opt] DOMAIN [> FILE] - list addresses in DOMAIN (optional: output to FILE)\n\
    -a \t\t-  list canonical names and aliases\n\
    -h \t\t-  list HINFO (CPU type and operating system)\n\
    -s \t\t-  list well-known services\n\
    -d \t\t-  list all records\n\
    -t TYPE \t-  list records of the given type (e.g., A,CNAME,MX, etc.)\n\
view FILE\t- sort an 'ls' output file and view it with more\n\
exit\t\t- exit the program, ^D also exits\n"
$
$ Don't translate any text left of the \t-, A, CNAME, MX, DOMAIN, CHAOS,
$ HESIOD, HINFO, MX, NS, PTR, SOA, WKS, ANY, N1...N6, NAME, NAME2, USER. 
$ items right of the \t- are nslookup commands and items to the far right
$ are command descriptions.
