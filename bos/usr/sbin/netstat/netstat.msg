$ @(#)29	1.15.6.1  src/bos/usr/sbin/netstat/netstat.msg, cmdnet, bos411, 9435B411a 8/30/94 17:46:44
$ COMPONENT_NAME: (NETCMD) Network commands.   
$ FUNCTIONS:   
$ ORIGINS: 27   
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when  
$ combined with the aggregated modules for this product)  
$                  SOURCE MATERIALS  
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1994
$ All Rights Reserved  
$ US Government Users Restricted Rights - Use, duplication or  
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.  
$ messages for netstat  
$ Do not translate netstat,host, %s, %d, %m, %c, %1$s, %2$s, etc... . 
$ len 
$ GLOBAL SPACING NOTE:
$ Some of these messages are used in table formats, therefore the
$ spacing is very important to the formatting of the data.
$ When you have reference to this comment on a message, each
$ translated word must have less than or equal the amount of characters
$ that were in the original word.  If the translated word has less
$ characters than the original word, it must be padded with spaces to 
$ equal the size of the original word. This is necessary so the words 
$ will start and end in the same places as the words in the original message.
$
$ Message numbers have been assigned a range of 157-188.

$set MS_NETSTAT        Define initial set#

$quote "         define quote character.

$ NOTE:
$	Many of the messages herein are of the form(s):
$
$	SYM		"%d data packet%s (%d byte%s)"
$	SYM1		"%d data packets%s (%d byte%s)"
$	SYM2		"%d data packet%s (%d bytes%s)"
$	SYM3		"%d data packets%s (%d bytes%s)"
$	
$	The %s specifier in such messages must be flush against
$	the word it is originally (although this word may be translated),
$	and the order of the % specifiers *must* remain as they
$	are in this file.  I know it's confusing to have "word%s"
$	and "words%s", but it is a simple way to solve a really
$	nasty problem in the code.
$
$ ALSO:
$	Sometimes - especially when the plural form occurs
$	at the end of the string - the %s has been left off,
$	even though the printf() has an argument for the %s.
$	No problem.  We don't want that argument in the NLS
$	case and it will be ignored by the printf().
$
$ AND FURTHERMORE: :-)
$	Do not change the order of the messages with numbered symbols.
$	These messages are indexed according to an algorithm that
$	"understands" the order they are currently in.

PLURAL	""
$ NOTE: PLURAL *must* be the empty string ("") for every data set.
$ [Yeah, it's grody, but again, it's a(nother) simple way to solve...]

UNK_ADDR_FAM         "%s: unknown address family\n"
$  1\tThis message is printed when the -f option is given an  
$  1\targument other than inet or unix.  
NOT_SUPPORT          	"%s: Not Supported\n"	
USAGE	"usage: netstat [-Aan] [-f address_family] [core unix netinet addr]\n"
USAGE1  "               [-D]\n"
USAGE2	"               [-imnrsv] [-f address_family] [-p proto] [core unix netinet addr]\n"
USAGE3	"               [-n] [-I interface] interval [core unix netinet addr]\n"
$  2\tTranslate only address_family, system, proto(col), interface, interval
$  2\tsystem is the namelist of the system, eg. /vmunix
$  2\t\tThis message is printed when the netstat command is   
$  2\t\tincorrectly used.  
NO_NMLST             "%s: no namelist\n"
$  3\tThis message is printed when there is no namelist for the  
$  3\tnamed system.  
NO_STATS             "%s: no stats routine\n"
$  4\tnamelist is a Unix term. namelist is is the symbole table of %s.
$  4\tThis message is printed when there is no routine to print  
$  4\tprotocol statistics.  
NO_PROTO             "%s: unknown or uninstrumented protocol\n" 
$  5\tThis message is printed when -p is used to print an unknown  
$  5\tprotocol.  
CANT_OPEN            "cannot open "
$  6\tThis message is printed when kmem cannot be opened.  
SYM_NO_NMLST         "imp_softc: symbol not in namelist\n"
$ Do not translate namelist, imp_softc. 
$  7\tThis message is printed when the symbol is not in the name  
$  7\tlist.  
NIMP_NO_SYM	     "nimp: symbol not in namelist\n"
$  Do not translate nimp
IMP_STATS	     "imp%d statistics:\n"
$  Do not translate imp
$ **** DO NOT CHANGE THE ORDER OF MESSAGES THRU "END ORDER" ****
IMP_INMSG	 "\t%u input message\n"
$ singular
IMP_INMSG2	 "\t%u input messages\n"
$ plural
IMP_OUTMSG	 "\t%u output message\n"
IMP_OUTMSG2	 "\t%u output messages\n"
IMP_BLOCKED	 "\t%u times output blocked at least %d sec.\n"
IMP_INCOM	 "\t%u \"incomplete\" message\n"
IMP_INCOM2	 "\t%u \"incomplete\" messages\n"
IMP_LOST_RFNM	 "\t%u lost RFNM message\n"
IMP_LOST_RFNM2	 "\t%u lost RFNM messages\n"
IMP_LATE_RFNM	 "\t%u late/bogus RFNM/incomplete message\n"
IMP_LATE_RFNM2	 "\t%u late/bogus RFNM/incomplete messages\n"
IMP_UNKMSG	 "\t%u unknown message type\n"
IMP_UNKMSG2	 "\t%u unknown message types\n"
$ **** END ORDER ****
SYM_MISSING          "some symbols missing %s, %s, %s\n"
$ Do not translate namelist.
$  8\tThis message is printed when the symbols were not in the name  
$  8\tlist.  
IMP_HSTTAB           "IMP%d Host Table\n"
$ Do not translate IMP. 
$  9\tThis message is printed as the title for the IMP host table  
$  9\tinformation.  
HEADING              "Flags Host                Qcnt Q Address RFNM Timer\n"
$ see GLOBAL SPACING NOTE
$ Do not translate Qcnt. Do not translate "Q". Do not translate RFNM. 
$  10\tThis message is originally coded with certain numbers of  
$  10 \tcolumns allocated for each title, this must be preserved.  
$  10\tThis is the original format line:  
$  10\t"%1$-5.5s %2$-15.15s %3$-4.4s %4$-9.9s %5$-4.4s %6$s 
$ "  
SYM_NO_DEF           "ifnet: symbol not defined\n"
$ Do not translate ifnet. 
$  11\tThis is printed when the symbol is not defined.  
HEADING1	     "Name  Mtu   Network     Address            Ipkts Ierrs    Opkts Oerrs"
$ see GLOBAL SPACING NOTE
$ Do not translate Ierrs, Ipkts, Mtu, Oerrs, Opkts. 
SYM_NOT_FOUND        "can't find symbol: %s\n"
$ see GLOBAL SPACING NOTE
$  13\tThis is printed when the symbol is not found  
$  12\tThis message is originally coded with certain numbers of  
$  12 \tcolumns allocated for each title, this must be preserved.  
$  12\tThis is the original format line:  
$  12\t"%1$-7.7s %2$-5.5s%3$-11.11s %4$-15.15s %5$-7.7s %6$-5.5s %7$-7.7s%8$-5.5s"  
HEADING2             "  Coll"
$ see GLOBAL SPACING NOTE
$ Do not translate Coll. 
$ Coll stands for collisions.
$  14\tThis message is originally coded with certain numbers of  
$  14 \tcolumns allocated for each title, this must be preserved.  
$  14\tThis is the original format line:  
$  14\t"%-5.5s"  
HEADING3             "  Time"
$ see GLOBAL SPACING NOTE
$  15\tThis message is originally coded with certain numbers of  
$  15 \tcolumns allocated for each title, this must be preserved.  
$  15\tThis is the original format line:  
$  15\t"%-5.5s"  
HEADING3a	     "  Drop"
$ see GLOBAL SPACING NOTE
LINK		     "<Link>"
$ do not translate
NAME_NUM             "%-7.7s %-7lu "
$ Do not translate . 
$  16\tThis prints the name and mtu number.  
NONE11               "none       "
$ Do not translate.
$  17\tThis message is originally coded with certain numbers of  
$  17\tcolumns allocated for each title, this must be preserved.  
$  17\tThis is the original format line:  
$  17\t"%-11.11s "  
NONE15               "none           "
$ Do not translate.
$  18\tThis message is originally coded with certain numbers of  
$  18\tcolumns allocated for each title, this must be preserved.  
$  18\tThis is the original format line:  
$  18\t"%-15.15s "  
NETNAME              "%-11.11s "
$ Do not translate . 
$  19\tThis message prints out the net name (this is in an ifdef  
$  19\tthat won't be used).  
NETNAME2             "%-11.11s "
$ Do not translate . 
$  20\tThis message prints out the net name.  
RTENAME              "%-15.15s "
$ Do not translate . 
$  21\tThis message prints out the route name.  
BANNER1              "    input   %-6.6s     output      "
$  22\tThis is the banner for printing a summary of interface  
$  22\tstatistics. The number of input and output packets go under input and
$  22\t output columns.  
BANNER2              "     input   (Total)    output    "
$  23\tThis is the banner for printing the total of the summary of   
$  23\tinterface statistics.  
VALUES1              " packets  errs  packets  errs colls "
$ see GLOBAL SPACING NOTE
$  Used for the header of a table. Do not increase width of columns.
$  24\tThis message is originally coded with certain numbers of  
$  24\tcolumns allocated for each title, this must be preserved.  
$  24\tThis is the original format line:  
$  24\t"%-7.7s %-5.5s %-7.7s %-5.5s %-5.5s "  
DROPS		     "drops "
$
ACT_INT_CONN         "Active Internet connections"
$ Do not translate Internet. 
$  25\tThis message is the first banner for printing the active  
$  25\tInternet connections.  
INCL_SRVRS           " (including servers)"
$  26\tThis message is the banner for printing the server information  
$  26\twhen the -a option has been specified.  
PCB                  "PCB/ADDR "
$ Do not translate . 
$ Be sure same number of spaces are left at end.
$  27\t\tThis message is originally coded with certain numbers of  
$  27\t\tcolumns allocated for each title, this must be preserved.  
$  27\t\tThis is the original format line:  
$  27 \t\t"%-8.8s "  
LA_MSG1              "Proto Recv-Q Send-Q  Local Address      Foreign Address    (state)\n"
$ see GLOBAL SPACING NOTE
$ Do not translate Proto and "Q". 
$  28\tThis message is originally coded with certain numbers of  
$  28\tcolumns allocated for each title, this must be preserved.  
$  28\tThis is the original format line:  
$  28\t"%1$-5.5s %2$-6.6s %3$-6.6s  %4$-18.18s %5$-18.18s %6$s 
$ "  
LA_MSG2              "Proto Recv-Q Send-Q  Local Address          Foreign Address        (state)\n"
$ see GLOBAL SPACING NOTE
$  29\tThis message is originally coded with certain numbers of  
$  29\tcolumns allocated for each title, this must be preserved.  
$  29\tThis is the original format line:  
$  29\t"%-5.5s %-6.6s %-6.6s  %-22.22s %-22.22s %s 
$ "  
NAME_SOCKB           "%-5.5s %6d %6d "
$ Do not translate . 
$  30\tThis message prints out the socket name, as well as send and  
$  30\treceive values.  
PACK_SENT	"\t%u packet sent\n"
$  31\tThis prints the number of packets sent (singular).  
PACK_SENT2	"\t%u packets sent\n"
$  32\tThis prints the number of packets sent (plural).  
PACK_BYTE	"\t\t%u data packet%s (%u byte)\n"
$  33\tThis prints the number of data packets and bytes sent (both  
$  33\tsingular).  
PACK_BYTE1	"\t\t%u data packet%s (%u bytes)\n"
$  35\tThis prints the number of data packets and bytes sent (one  
$  35\tsingular and one plural).  
PACK_BYTE2	"\t\t%u data packets%s (%u byte)\n"
$  34\tThis prints the number of data packets and bytes sent (one  
$  34\tsingular and one plural).  
PACK_BYTE3	"\t\t%u data packets%s (%u bytes)\n"
$  36\tThis prints the number of data packets and bytes sent (both  
$  36\tplural).  
PACK_RXMIT	"\t\t%u data packet%s (%u byte) retransmitted\n"
$  37\tThis prints the number of data packets and bytes retransmitted  
$  37\t(both singular).  
PACK_RXMIT1	"\t\t%u data packet%s (%u bytes) retransmitted\n"
$  38\tThis prints the number of data packets and bytes retransmitted  
$  38\t(one singular and one plural).  
PACK_RXMIT2	"\t\t%u data packets%s (%u byte) retransmitted\n"
$  39\tThis prints the number of data packets and bytes retransmitted  
$  39\t(one singular and one plural).  
PACK_RXMIT3	"\t\t%u data packets%s (%u bytes) retransmitted\n"
$  40\tThis prints the number of data packets and bytes retransmitted  
$  40\t(both plural).  
ACK_PACK            "\t\t%u ack-only packet%s (%u delayed)\n"
$ Do not translate ACK.
$  41\tThis prints the number of ACK-only packets and the   
$  41\tnumber of packets delayed (singular).  
ACK_PACK2            "\t\t%u ack-only packets%s (%u delayed)\n"
$ Do not translate ACK.
$  42\tThis prints the number of ACK-only packets and the   
$  42\tnumber of packets delayed (plural).  
URG_PACK            "\t\t%u URG only packet\n"
$ Do not translate URG. 
$  43\tThis prints the number of URG only packets (singular).  
URG_PACK2            "\t\t%u URG only packets\n"
$ Do not translate URG. 
$  44\tThis prints the number of URG only packets (plural).  
WPROBE_PACK         "\t\t%u window probe packet\n"
$  45\tThis prints the number of window probe packets (singular).  
$ Do not translate window probe. 
WPROBE_PACK2         "\t\t%u window probe packets\n"
$ Do not translate window probe.
$  46\tThis prints the number of window probe packets (plural).  
WUPDT_PACK          "\t\t%u window update packet\n"
$ Do not translate window probe.
$  47\tThis prints the number of window update packets (singular).  
WUPDT_PACK2          "\t\t%u window update packets\n"
$ Do not translate window update.
$  48\tThis prints the number of window update packets (plural).  
CTRL_PACK           "\t\t%u control packet\n"
$ Do not translate control.
$  49\tThis prints the number of control packets (singular).  
CTRL_PACK2           "\t\t%u control packets\n"
$ Do not translate control.
$  50\tThis prints the number of control packets (plural).  
PACK_RCV            "\t%u packet received\n"
$ Do not translate control.
$  51\tThis prints the number of packets received (singular).  
PACK_RCV2            "\t%u packets received\n"
$  52\tThis prints the number of packets received (plural).  
ACK4_BYTE          "\t\t%u ack%s (for %u byte)\n"
$  53\tThis prints the number of ACKs for the number of   
$  53\tbytes (both singular).  
ACK4_BYTE1          "\t\t%u ack%s (for %u bytes)\n"
$  54\tThis prints the number of ACKs for the number of   
$  54\tbytes (one singular and one plural).  
ACK4_BYTE2          "\t\t%u acks%s (for %u byte)\n"
$  55\tThis prints the number of ACKs for the number of   
$  55 \tbytes (one singular and one plural).  
ACK4_BYTE3          "\t\t%u acks%s (for %u bytes)\n"
$  56\tThis prints the number of ACKs for the number of   
$  56\tbytes (both plural).  
DUP_ACK             "\t\t%u duplicate ack\n"
$  57\tThis message prints the number of duplicate ACKs  
$  57\t(singular).  
DUP_ACK2             "\t\t%u duplicate acks\n"
$  58\tThis message prints the number of duplicate ACKs  
$  58\t(plural).  
UNSENT_ACK          "\t\t%u ack for unsent data\n"
$  59\tThis prints the number of ACKs for unsent data  
$  59\t(singular).  
UNSENT_ACK2          "\t\t%u acks for unsent data\n"
$  60\tThis prints the number of ACKs for unsent data  
$  60\t(plural).  
RCV_INSEQ          "\t\t%u packet%s (%u byte) received in-sequence\n"
$  61\tin-sequence means sequentially.
$  61\tThis prints the number of packets and bytes received in  
$  61\tsequence (both singular).  
RCV_INSEQ1          "\t\t%u packet%s (%u bytes) received in-sequence\n"
$  62\tin-sequence means sequencially.  
$  62\tThis prints the number of packets and bytes received in  
$  62\tsequence (one singular and one plural).  
RCV_INSEQ2          "\t\t%u packets%s (%u byte) received in-sequence\n"
$  63\tin-sequence means sequencially.  
$  63\tThis prints the number of packets and bytes received in  
$  63\tsequence (one singular and one plural).  
RCV_INSEQ3          "\t\t%u packets%s (%u bytes) received in-sequence\n"
$  64\tin-sequence means sequencially.  
$  64\tThis prints the number of packets and bytes received in  
$  64\tsequence (both plural).  
DUP_PACK           "\t\t%u completely duplicate packet%s (%u byte)\n"
$  65\tThis message prints the number of duplicate  
$  65\tpackets and bytes (both singular).  
DUP_PACK1           "\t\t%u completely duplicate packet%s (%u bytes)\n"
$  66\tThis message prints the number of duplicate  
$  66\tpackets and bytes (one singular and one plural).  
DUP_PACK2           "\t\t%u completely duplicate packets%s (%u byte)\n"
$  67\tThis message prints the number of duplicate  
$  67\tpackets and bytes (one singular and one plural).  
DUP_PACK3           "\t\t%u completely duplicate packets%s (%u bytes)\n"
$  68\tThis message prints the number of duplicate  
$  68\tpackets and bytes (both plural).  
DUP_DATA           "\t\t%u packet%s with some dup. data (%u byte duped)\n"
$  69\tThis prints the number of packets with duplicate data and   
$  69\tthe number of duplicate bytes (both singular).  
DUP_DATA1           "\t\t%u packet%s with some dup. data (%u bytes duped)\n"
$  70\tThis prints the number of packets with duplicate data and   
$  70\tthe number of duplicate bytes (one singular and one plural).  
DUP_DATA2           "\t\t%u packets%s with some dup. data (%u byte duped)\n"
$  71\tThis prints the number of packets with duplicate data and   
$  71\tthe number of duplicate bytes (one singular and one plural).  
DUP_DATA3           "\t\t%u packets%s with some dup. data (%u bytes duped)\n"
$  72\tThis prints the number of packets with duplicate data and   
$  72\tthe number of duplicate bytes (both plural).  
OUT_ORDER          "\t\t%u out-of-order packet%s (%u byte)\n"
$  73\tThis prints the number of out-of-order packets and the  
$  73\tnumber of bytes (both singular).  
OUT_ORDER1          "\t\t%u out-of-order packet%s (%u bytes)\n"
$  74\tThis prints the number of out-of-order packets and the  
$  74\tnumber of bytes (one singular and one plural).  
OUT_ORDER2          "\t\t%u out-of-order packets%s (%u byte)\n"
$  75\tThis prints the number of out-of-order packets and the  
$  75\tnumber of bytes (one singular and one plural).  
OUT_ORDER3          "\t\t%u out-of-order packets%s (%u bytes)\n"
$  76\tThis prints the number of out-of-order packets and the  
$  76\tnumber of bytes (both plural).  
DATA_AFTWIN        "\t\t%u packet%s (%u byte) of data after window\n"
$ window is a technical tcpip term do not translate unless you know
$ the correct meaning.
$ DATA_AFWIN11\tThis prints the number of packets and bytes of data after the  
$ DATA_AFWIN11\twindow (both singular).  
DATA_AFTWIN1        "\t\t%u packet%s (%u bytes) of data after window\n"
$ window is a technical tcpip term do not translate unless you know
$ the correct meaning.
$ DATA_AFWIN12\tThis prints the number of packets and bytes of data after the  
$ DATA_AFWIN12\twindow (one singular and one plural).  
DATA_AFTWIN2        "\t\t%u packets%s (%u byte) of data after window\n"
$ window is a technical tcpip term do not translate unless you know
$ the correct meaning.
$ DATA_AFWIN21\tThis prints the number of packets and bytes of data after the  
$ DATA_AFWIN21\twindow (one singular and one plural).  
DATA_AFTWIN3        "\t\t%u packets%s (%u bytes) of data after window\n"
$ window is a technical tcpip term do not translate unless you know
$ the correct meaning.
$ DATA_AFWIN22\tThis prints the number of packets and bytes of data after the  
$ DATA_AFWIN22\twindow (both plural).  
WIN_PROBE           "\t\t%u window probe\n"
$  window probe is a TCPIP term. Do not translate.
$  81\tThis message prints the number of window probes (singular).  
WIN_PROBE2           "\t\t%u window probes\n"
$ Do not translate window, probes or updates.                 
$  82\tThis message prints the number of window probes (plural).  
WIN_UPD             "\t\t%u window update packet\n"
$ Do not translate window, probes or update.                 
$  83\tThis prints the number of window update packets (singular).  
WIN_UPD2             "\t\t%u window update packets\n"
$ Do not translate window, probes or update.                 
$  84\tThis prints the number of window update packets (plural).  
AFTER_CLOSE         "\t\t%u packet received after close\n"
$  85\tclose is the packet that terminates the connection.
$  85\tThis prints the number of packets received after the close  
$  85\t(singular).  
AFTER_CLOSE2         "\t\t%u packets received after close\n"
$ Do not translate close. (a TCPIP term)
$  86\tclose is the packet that terminates the connection.
$  86\tThis prints the number of packets received after the close  
$  86\t(plural).  
BAD_SUM             "\t\t%u discarded for bad checksum\n"
$ Do not translate Checksum. (a TCPIP term)
$  87\tThis prints the number of bytes discarded because of a bad  
$  87\tchecksum (singular).  
BAD_SUM2             "\t\t%u discarded for bad checksums\n"
$ Do not translate Checksums. (a TCPIP term)
$  88\tThis prints the number of bytes discarded because of a bad  
$  88\tchecksum (plural).  
BAD_OFF             "\t\t%u discarded for bad header offset field\n"
$  89\tThis prints the number of bytes discarded because of a bad  
$  89\theader offset field (singular).  
BAD_OFF2             "\t\t%u discarded for bad header offset fields\n"
$  90\tThis prints the number of bytes discarded because of a bad  
$  90\theader offset field (plural).  
BAD_SHORT            "\t\t%u discarded because packet too short\n"
$  91\tThis prints the number of bytes discarded because the  
$  91\tthe packet is too short.  
CONN_REQ            "\t%u connection request\n"
$ CON_REQ1\tThis prints the number of connection requests (singular).  
CONN_REQ2            "\t%u connection requests\n"
$ CON_REQ2\tThis prints the number of connection requests (plural).  
CONN_ACC            "\t%u connection accept\n"
$  94\tThis prints the number of connection acceptances (singular).  
CONN_ACC2            "\t%u connection accepts\n"
$  95\tThis prints the number of connection acceptances (plural).  
CONN_EST            "\t%u connection established (including accepts)\n"
$  96\tThis prints the number of established connections (including  
$  96\tacceptances) (singular).  
CONN_EST2            "\t%u connections established (including accepts)\n"
$  97\tThis prints the number of established connections (including  
$  97\tacceptances) (plural).  
CONN_CLOSE         "\t%u connection%s closed (including %u drop)\n"
$  drop is a tcpip term do not translate unless you know the correct meaning
$  98\tThis prints the number of closed connections and dropped  
$  98\tconnections (singular).  
CONN_CLOSE1         "\t%u connection%s closed (including %u drops)\n"
$  drops is a tcpip term do not translate unless you know the correct meaning
$  99\tThis prints the number of closed connections and dropped  
$  99\tconnections (one singular and one plural).  
CONN_CLOSE2         "\t%u connections%s closed (including %u drop)\n"
$  drop is a tcpip term do not translate unless you know the correct meaning
$  100\tThis prints the number of closed connections and dropped  
$  100\tconnections (one singular and one plural).  
CONN_CLOSE3         "\t%u connections%s closed (including %u drops)\n"
$  drops is a tcpip term do not translate unless you know the correct meaning
$  101\tThis prints the number of closed connections and dropped  
$  101\tconnections (plural).  
CONN_DRP            "\t%u embryonic connection dropped\n"
$  dropped is a tcpip term do not translate unless you know the correct meaning
$  102\tThis prints the number of embryonic connections dropped  
$  102\t(singular).  
CONN_DRP2            "\t%u embryonic connections dropped\n"
$  dropped is a tcpip term do not translate unless you know the correct meaning
$  103\tThis prints the number of embryonic connections dropped  
$  103\t(plural).  
UPD_RTT            "\t%u segment%s updated rtt (of %u attempt)\n"
$ Do not translate rtt. 
$  rtt = round trip time.
$  104\tThis prints the number of updated rtt segments in the number  
$  104\tof attempts (both singular).  
UPD_RTT1            "\t%u segment%s updated rtt (of %u attempts)\n"
$ Do not translate rtt. 
$  105\tThis prints the number of updated rtt segments in the number  
$  105\tof attempts (one singular and one plural).  
UPD_RTT2            "\t%u segments%s updated rtt (of %u attempt)\n"
$ Do not translate rtt. 
$  106\tThis prints the number of updated rtt segments in the number  
$  106\tof attempts (one singular and one plural).  
UPD_RTT3            "\t%u segments%s updated rtt (of %u attempts)\n"
$ Do not translate rtt. 
$  107\tThis prints the number of updated rtt segments in the number  
$  107\tof attempts (both plural).  
XMIT_TOUT           "\t%u retransmit timeout\n"
$ Do not translate timeout.
$  108\tThis prints the number of retransmit timeouts (singular).  
XMIT_TOUT2           "\t%u retransmit timeouts\n"
$ Do not translate timeouts.
$  109\tThis prints the number of retransmit timeouts (plural).  
CONN_DRP_XMIT       "\t\t%u connection dropped by rexmit timeout\n"
$ dropped is a tcpip term do not translate unless you know the correct meaning
$  110\tThis prints the number of connections dropped by  
$  110\tretransmit timeouts (singular).  
CONN_DRP_XMIT2       "\t\t%u connections dropped by rexmit timeout\n"
$ dropped is a tcpip term do not translate unless you know the correct meaning
$  111\tThis prints the number of connections dropped by  
$  111\tretransmit timeouts (plural).  
PERS_TOUT           "\t%u persist timeout\n"
$ Do not translate persist, timeout. 
$  112\tThis prints the number of persist timeouts (singular).  
PERS_TOUT2           "\t%u persist timeouts\n"
$ Do not translate persist, timeouts. 
$  113\tThis prints the number of persist timeouts (plural).  
KEEP_TOUT           "\t%u keepalive timeout\n"
$ Do not translate keepalive, timeout. 
$  114\tThis prints the number of keepalive timeouts (singular).  
KEEP_TOUT2           "\t%u keepalive timeouts\n"
$ Do not translate keepalive, timeouts. 
$  115\tThis prints the number of keepalive timeouts (plural).  
KEEP_PROBE          "\t\t%u keepalive probe sent\n"
$ Do not translate keepalive, probe. 
$  116\tThis prints the number of keepalive probes sent (singular).  
KEEP_PROBE2          "\t\t%u keepalive probes sent\n"
$ Do not translate keepalive, probe. 
$  117\tThis prints the number of keepalive probes sent (plural).  
KEEP_DROP           "\t\t%u connection dropped by keepalive\n"
$ dropped is a tcpip term do not translate unless you know the correct meaning
$ Do not translate keepalive, probe. 
$  118\tThis prints the number of connections dropped by keepalive  
$  118\t(singular).  
KEEP_DROP2           "\t\t%u connections dropped by keepalive\n"
$ dropped is a tcpip term do not translate unless you know the correct meaning
$ Do not translate keepalive, probe. 
$  119\tThis prints the number of connections dropped by keepalive  
$  119\t(plural).  
INCMPLT_HDR         "%s:\n\t%u incomplete header\n"
$  120\tThis prints the number of incomplete headers (singular).\t  
INCMPLT_HDR2         "%s:\n\t%u incomplete headers\n"
$  121\tThis prints the number of incomplete headers (plural).\t  
BAD_DATA_LEN        "\t%u bad data length field\n"
$  122\tThis prints the number of bad data length fields (singular).  
BAD_DATA_LEN2        "\t%u bad data length fields\n"
$  123\tThis prints the number of bad data length fields (plural).  
BAD_CHKSUM          "\t%u bad checksum\n"
$ Do not translate checksum.
$  124\tThis prints the number of bad checksums (singular).  
BAD_CHKSUM2          "\t%u bad checksums\n"
$ Do not translate checksums.
$  125\tThis prints the number of bad checksums (plural).  
SOCKET_OVRFLW1       "\t%d socket overflow\n"
$ Do not translate socket, 126\tThis prints the number of socket overflows (singular).  
SOCKET_OVRFLW2       "\t%d socket overflows\n"
$ Do not translate socket. 
$  127\tThis prints the number of socket overflows (plural).  
TOT_PACK_RCVD        "%s:\n\t%u total packets received\n"
$  128\tThis prints the total number of packets received.  
BAD_HDR_CHKSUM      "\t%u bad header checksum\n"
$  129\tThis prints the number of bad header checksums   
$  129\t(singular).  
BAD_HDR_CHKSUM2      "\t%u bad header checksums\n"
$  130\tThis prints the number of bad header checksums   
$  130\t(plural).  
SZ_TOO_SMALL         "\t%u with size smaller than minimum\n"
$ Do not translate IP. 
$  131\tIP is the Internet Protocol.
$  131\tThis prints the number of IPs packets with a size smaller than the  
$  131\tminimum.  
DSIZE_LESS_LEN       "\t%u with data size < data length\n"
$ Do not translate IP.
$  132\tIP is the Internet Protocol. 
$  132\tThis prints the number of IPs with a data size less  
$  132\tthan its data length.  
HLEN_LESS_DSIZE      "\t%u with header length < data size\n"
$ Do not translate IP.
$  133\tIP is the Internet Protocol.
$  133\tThis prints the number of IPs with a header length  
$  133\tless than its data size.  
DLEN_LESS_HLEN       "\t%u with data length < header length\n"
$ Do not translate IP.
$  134\tIP is the Internet Protocol.
$  134\tThis prints the number of IPs with a data length  
$  134\tless than its header length.  
FRAG_RCVD           "\t%u fragment received\n"
$  135\tThis prints the number of fragments received (singular).  
FRAG_RCVD2           "\t%u fragments received\n"
$  136\tThis prints the number of fragments received (plural).  
OUT_OF_SP           "\t%u fragment dropped (dup or out of space)\n"
$ Dropped is a tcpip term do not translate unless you know the correct meaning
$  137\tThis prints the number of fragments dropped, duplicated, or  
$  137\tout of space (singular).  
OUT_OF_SP2           "\t%u fragments dropped (dup or out of space)\n"
$ Dropped is a tcpip term do not translate unless you know the correct meaning
$  138\tThis prints the number of fragments dropped, duplicated, or  
$  138\tout of space (plural).  
FRAG_DRPD_TOUT      "\t%u fragment dropped after timeout\n"
$ Dropped is a tcpip term do not translate unless you know the correct meaning
$  139\tThis prints the number of fragments dropped after   
$  139\ttimeout (singular).  
FRAG_DRPD_TOUT2      "\t%u fragments dropped after timeout\n"
$ Dropped is a tcpip term do not translate unless you know the correct meaning
$  140\tThis prints the number of fragments dropped after   
$  140\ttimeout (plural).  
PACKET_FWD          "\t%u packet forwarded\n"
$  141\tThis prints the number of packets forwarded (singular).  
PACKET_FWD2          "\t%u packets forwarded\n"
$  142\tThis prints the number of packets forwarded (plural).  
PACKET_NO_FWD       "\t%u packet not forwardable\n"
$  143\tThis prints the number of non-forwardable packets  
$  143\t(singular).  
PACKET_NO_FWD2       "\t%u packets not forwardable\n"
$  144\tThis prints the number of non-forwardable packets  
$  144\t(plural).  
REDIRECT_SNT        "\t%u redirect sent\n"
$ Do not translate redirect.
$  145\tThis prints the number of redirects sent (singular).  
REDIRECT_SNT2        "\t%u redirects sent\n"
$ Do not translate redirects.
$  146\tThis prints the number of redirects sent (plural).  
CALLS_TO_ICMP       "%s:\n\t%u call to icmp_error\n"
$ Do not translate icmp_error. 
$  147\tThis prints the number of calls to icmp_error   
$  147\t(singular).  
CALLS_TO_ICMP2       "%s:\n\t%u calls to icmp_error\n"
$ Do not translate icmp_error. 
$  148\tThis prints the number of calls to icmp_error   
$  148\t(plural).  
ERR_NO_GEN          "\t%u error not generated 'cuz old message was icmp\n"
$ Do not translate ICMP. 
$  149\tThis prints the number of errors not generated, because the  
$  149\told message is ICMP (singular).  
ERR_NO_GEN2          "\t%u errors not generated 'cuz old message was icmp\n"
$ Do not translate ICMP. 
$  150\tThis prints the number of errors not generated, because the  
$  150\told message is ICMP (plural).  
OTPT_HISTOGRAM       "\tOutput histogram:\n"
$  151\tThis is the banner for the output histogram.  
ECHO_REPLY           "echo reply"
$  152\tThis prints input/output statistics for echo replies.  
ONE                  "#1"
$  153\t\tThis prints input/output statistics for #1.  
TWO                  "#2"
$  154\t\tThis prints input/output statistics for #2.  
DEST_UNRCH           "destination unreachable"
$  155\tThis prints input/output statistics for unreachable   
$  155\tdestinations.  
SRC_QUENCH           "source quench"
$  156\tThis prints input/output statistics for source quenches.  
RT_REDIRECT          "routing redirect"
$  157\tThis prints input/output statistics for routing redirects.  
SIX                  "#6"
$  158\t\tThis prints input/output statistics for #6.  
SEVEN                "#7"
$  159\t\tThis prints input/output statistics for #7.  
ECHO                 "echo"
$  160\t\tThis prints input/output statistics for echos.  
NINE                 "#9"
$  161\t\tThis prints input/output statistics for #9.  
TEN                  "#10"
$  162\t\tThis prints input/output statistics for #10.  
TIME_EXCEED          "time exceeded"
$  163\tThis prints input/output statistics for times exceeded.  
PARAM_PROB           "parameter problem"
$  164\tThis prints input/output statistics for parameter problems.  
TIMESTAMP            "timestamp"
$ Do not translate Timestamp. 
$  165\tThis prints input/output statistics for Timestamps.  
TIMEST_REPLY         "timestamp reply"
$ Do not translate Timestamp. 
$  166\tThis prints input/output statistics for Timestamp replies.  
INFO_REQ             "information request"
$  167\tThis prints input/output statistics for information requests.  
INFO_REQ_REPLY       "information request reply"
$  168\tThis prints input/output statistics for information   
$  168\trequest replies.  
ADDR_REQ             "address mask request"
$  169\tThis prints input/output statistics for address mask requests.  
ADDR_REPLY           "address mask reply"
$  170\tThis prints input/output statistics for address mask request  
$  170\treplies.  
BAD_CODE_FIELD      "\t%u message with bad code fields\n"
$  171\tThis prints the number of messages with bad code  
$  171\tfields (singular).  
BAD_CODE_FIELD2      "\t%u messages with bad code fields\n"
$  172\tThis prints the number of messages with bad code  
$  172\tfields (plural).  
MSG_LESS_MINLN      "\t%u message < minimum length\n"
$  173\tThis prints the number of messages that were less  
$  173\tthan the minimum length (singular).  
MSG_LESS_MINLN2      "\t%u messages < minimum length\n"
$  174\tThis prints the number of messages that were less  
$  174\tthan the minimum length (plural).  
MSG_BAD_LEN         "\t%u message with bad length\n"
$  175\tThis  prints the number of messages with bad lengths (singular).  
MSG_BAD_LEN2         "\t%u messages with bad length\n"
$  176\tThis  prints the number of messages with bad lengths (plural).  
INPT_HISTOGRAM       "\tInput histogram:\n"
$  177\tThis prints the banner for the input histogram.  
MSG_RESP_GEN        "\t%u message response generated\n"
$  178\tThis prints the number of message responses generated   
$  178\t(singular).  
MSG_RESP_GEN2        "\t%u message responses generated\n"
$  179\tThis prints the number of message responses generated   
$  179\t(plural).  
UNEX_CHG_MBSTAT      "unexpected change to mbstat; check source\n"
$ Do not translate mbstat. 
$  180\tThis message is printed when there has been an  
$  180\tunexpected change to mbstat.  
NO_SYM_NAMELIST      "mbstat: symbol not in namelist\n"
$ Do not translate mbstat. 
$  181\tThis message is printed when the named symbol is not  
$  181\tin the namelist.  
BAD_READ             "mbstat: bad read\n"
$ Do not translate mbstat. 
$  182\tThis message is printed when mbstat has a bad read.  
MBUFS_IN_USE         "%u mbufs in use:\n"
$ Do not translate mbufs. 
$  183\tThis prints the number of mbufs in use (over the total   
$  183\tnumber of mbufs).  
MSG_FREE             ""
$  184\tThis is a dummy message that should always be placed in the  
$  184\tcatalog before the messages ranging from MSG_DATA to  
$  184\tMSG_IFADDR and must be in order from MSG_DATA to MSG_IFADDR.  
MSG_DATA             "\t%d mbufs allocated to data\n"
$ Do not translate mbufs. 
$  185\tThis message prints the number of mbufs allocated to data.  
MSG_HEADER           "\t%d mbufs allocated to packet headers\n"
$ Do not translate mbufs. 
$  186\tThis message prints the number of mbufs allocated to packet  
$  186\theaders.  
MSG_SOCKET           "\t%d mbufs allocated to socket structures\n"
$ Do not translate mbufs, socket. 
$  187\tThis message prints the number of mbufs allocated to socket  
$  187\tstructures.  
MSG_PCB              "\t%d mbufs allocated to protocol control blocks\n"
$ Do not translate mbufs. 
$  188\tThis message prints the number of mbufs allocated to protocol  
$  188\tcontrol blocks.  
MSG_RTABLE           "\t%d mbufs allocated to routing table entries\n"
$ Do not translate mbufs. 
$  189\tThis message prints the number of mbufs allocated to routing  
$  189\ttable entries.  
MSG_HTABLE           "\t%d mbufs allocated to IMP host table entries\n"
$ Do not translate mbufs, IMP. 
$  190\tThis message prints the number of mbufs allocated to IMP host   
$  190\ttable entries.  
MSG_ATABLE           "\t%d mbufs allocated to address resolution tables\n"
$ Do not translate mbufs. 
$  191\tThis message prints the number of mbufs allocated to address  
$  191\tresolution tables.  
MSG_SONAME           "\t%d mbufs allocated to socket names and addresses\n"
$ Do not translate mbufs. 
$  192\tThis message prints the number of mbufs allocated to socket  
$  192\tnames and addresses.  
MSG_ZOMBIE           "\t%d mbufs allocated to zombie process information\n"
$ Do not translate mbufs, zombie. zombie is UNIX technical term. 
$  193\tThis message prints the number of mbufs allocated to zombie  
$  193\tprocess information.  
MSG_SOOPTS           "\t%d mbufs allocated to socket options\n"
$ Do not translate mbufs, sockets. 
$  194\tThis message prints the number of mbufs allocated to socket  
$  194\toptions.  
MSG_FTABLE           "\t%d mbufs allocated to fragment reassembly queue headers\n"
$ Do not translate mbufs. 
$  195\tThis message prints the number of mbufs allocated to fragment   
$  195\treassembly queue headers.  
MSG_RIGHTS           "\t%d mbufs allocated to access rights\n"
$ Do not translate mbufs. 
$  196\tThis message prints the number of mbufs allocated to access  
$  196\trights.  
MSG_IFADDR           "\t%d mbufs allocated to interface addresses\n"
$ Do not translate mbufs. 
$  197\tThis message prints the number of mbufs allocated to interface  
$  197\taddresses.  
MBUF_ALLOC_TYPE      "\t%d mbufs allocated to <mbuf type %d>\n"
$ Do not translate mbuf. 
$ Do not translate mbufs. 
$  198\tThis prints the number of mbufs allocated to the  
$  198\tspecific type of mbuf.  
MBUFS_MSSNG          "*** %d mbufs missing ***\n"
$ Do not translate mbufs. 
$  199\tThis prints the number of mbufs missing.  
MAPPAGE_IN_USE       "%d/%d mapped pages in use\n"
$  200\tThis prints the number of mapped pages in use (over  
$  200\tthe total number of mapped pages).  
ALLOC_TO_NTWK        "%d Kbytes allocated to network  (%d%% in use)\n"
$  Kbyte= 1024 bytes.
$  201\tThis prints the number of kbytes allocated to the network  
$  201\tand prints the percentage in use.  
REQ_MEM_DENIED       "%d requests for memory denied\n"
$  202\tThis prints the number of requests for memory that were  
$  202\tdenied.  
$
REQ_MEM_DELAY	     "%u requests for memory delayed\n"
PROTO_DRAIN	     "%u calls to protocol drain routines\n"
$
RTHST_SYM_NO_NL      "rthost: symbol not in namelist\n"
$ Do not translate rthost and namelist. 
$  203\tThis message is printed when rthost doesn't find the  
$  203\tnamed symbol in the namelist.  
RTNET_SYM_NO_NL      "rtnet: symbol not in namelist\n"
$ Do not translate rtnet and namelist. 
$  204\tThis message is printed when rtnet doesn't find the  
$  204\tnamed symbol in the namelist.  
HSHSZ_SYM_NO_NL      "rthashsize: symbol not in namelist\n"
$ Do not translate rthashsize and namelist. 
$  205\tThis message is printed when rthashsize doesn't find the  
$  205\tnamed symbol in the namelist.  
RTING_TABS           "Routing tables\n"
$  206\tThis prints the routing table header.  
DESTHDING	     "Destination      Gateway            Flags  Refcnt Use       Interface\n"
$ see GLOBAL SPACING NOTE
$ Do not translate Refcnt. 
$  207\tThis message is originally coded with certain numbers of  
$  207\tcolumns allocated for each title, this must be preserved.  
$  207\tThis is the original format line:  
$  207 \t"%-16.16s %-18.18s %-6.6s  %6.6s%8.8s  %s"
$ "  
RTNME                "%-20.20s "
$ Do not translate . 
$  208\t\tThis prints the route name that is obtained from the address.  
RT_NETMASKS	     "Netmasks:\n"
$ Do not translate . 
RT_TREEPF	     "\nRoute Tree for Protocol Family %d:\n"
RT_ROOTNODE	     "(root node)\n"
RT_GET_EST	     "getkerninfo: get routing table estimate"
$ Do not translate getkerninfo
RT_OUT_SP	     "out of space\n"
RT_GET_TBL	     "getkerninfo: retrieval of routing table"
$ Do not translate getkerninfo
$
$
NAMELINE             "%-8.8s %-6d %-10d "
$ Do not translate . 
$  209\tThis prints the name, reference count, and indicates whether  
$  209\tthe route is in use.  
NAME_IFNET           "%s%d\n"
$ Do not translate . 
$  210\tThis prints the ifnet name.  
RTST_NO_SYM_NL       "rtstat: symbol not in namelist\n"
$ Do not translate rtstat and namelist. 
$ RTST_SYM_NO_NL\tThis message is printed when rtstat doesn't find the  
$ RTST_SYM_NO_NL\tnamed symbol in the namelist.  
ROUTING              "routing:\n"
$  212\tThis message is printed as the routing information header.  
BAD_RTE_REDRCT      "\t%d bad routing redirect\n"
$  213\tThis prints the number of bad routing redirects   
$  213\t(singular).  
BAD_RTE_REDRCT2      "\t%d bad routing redirects\n"
$  214\tThis prints the number of bad routing redirects   
$  214\t(plural).  
DYN_CREAT_RTE       "\t%d dynamically created route\n"
$  215\tThis prints the number of dynamically created routes  
$  215\t(singular).  
DYN_CREAT_RTE2       "\t%d dynamically created routes\n"
$  216\tThis prints the number of dynamically created routes  
$  216\t(plural).  
NEW_GTWAY           "\t%d new gateway due to redirects\n"
$  217\tThis prints the number of new gateways due to redirects   
$  217\t(singular).  
NEW_GTWAY2           "\t%d new gateways due to redirects\n"
$  218\tThis prints the number of new gateways due to redirects   
$  218\t(plural).  
DEST_UNREACH        "\t%d destination found unreachable\n"
$  219\tThis prints the number of unreachable destinations found\t  
$  219\t(singular).  
DEST_UNREACH2        "\t%d destinations found unreachable\n"
$  220\tThis prints the number of unreachable destinations found\t  
$  220\t(plural).  
WILDCARD_RTE        "\t%d use of a wildcard route\n"
$  221\tThis prints the number of uses of a wildcard route (singular).  
$ Do not translate wildcard route. (Technical term)
WILDCARD_RTE2        "\t%d uses of a wildcard route\n"
$ Do not translate wildcard route. (Technical term)
$  222\tThis prints the number of uses of a wildcard route (plural).  
FLE_NO_NMLST         "nfile or file not in namelist.\n"
$ Do not translate file, nfile and namelist. 
$  223\tThis message is printed when the nfile or file is not in the  
$  223\tnamelist.  
NFILE_BAD_RD         "nfile: bad read.\n"
$  224\tThis message is printed when nfile cannot read.  
FTA_BAD_RD           "File table address, bad read.\n"
$  225\tThis message is printed when the file table address can't  
$  225\tbe read.  
OUT_OF_MEM_FTAB      "Out of memory (file table).\n"
$  226\tThis message is printed when the file table runs out  
$  226\tof memory.  
FTAB_RD_ERR          "File table read error.\n"
$  227\tThis message is printed when the file table has a read error.  
ACT_UNX_DM_SOCK      "Active UNIX domain sockets\n"
$  DO NOT translate UNIX domain sockets.
$  228\tThis message is printed as a banner for active UNIX. 
$  228\tdomain sockets.  
ADDR_HDING           "SADR/PCB  Type   Recv-Q Send-Q Inode    Conn     Refs     Nextref  Addr\n"
$ see GLOBAL SPACING NOTE
$ Do not translate. 
$  229\tThis message is originally coded with certain numbers of  
$  229\tcolumns allocated for each title, this must be preserved.  
$  229\tThis is the original format line:  
$  229 \t"%-8.8s %-6.6s %-6.6s %-6.6s %8.8s %8.8s %8.8s %8.8s Addr" 
ADDR_INFO            "%8x %-6.6s %6d %6d %8x %8x %8x %8x"
UPCB_INFO            "\n%8x\n"
$ Do not translate . 
$  230\tThis prints socket information (address, type, etc.).  
ETH_STATS            "\nETHERNET STATISTICS (%1$s%2$d) :\n"
$  231\tThis is the message banner for device statistics.  
TOK_STATS            "\nTOKEN STATISTICS (%1$s%2$d) :\n"
$  232\tThis is the message banner for device statistics.  
HDW_ADDR             "\nHardware Address: "
$  233\tThis message prints the hardware address of the interface.  
QUERY_STAT           "ioctl CIO_QUERY for %s device failed\n"
$ Do not translate ioctl and CIO_QUERY. 
$  234\tThis message is printed when the CIO_QUERY ioctl on   
$  234\tthe interface fails.  
CANT_OPEN_DEV        "cannot open device %s\n"
$  235\tThis message is printed when the device file cannot be  
$  235\topened.  
TX_BYTE_CNT          "\nTransmit Byte Count: %.1f"
$  236\tThis message prints the transmit byte count.  
RCV_BYTE_CNT         "\tReceive Byte Count: %.1f"
$  237\tThis message prints the receive byte count.  
TX_FRAME_CNT         "\nTransmit Frame Count: %.1f"
$ TX_RCV_CNT\tThis message prints the transmit frame count.  
RCV_FRAME_CNT        "\tReceive Frame Count: %.1f"
$  238\tThis message prints the receive frame count.  
TX_ERR_CNT           "\nTransmit Error Count: %d"
$  239\tThis message prints the transmit error count.  
RCV_ERR_CNT          "\t\tReceive Error Count: %d"
$  240\tThis message prints the receive error count.  
MAX_NETIDS           "\nMax Netid's in use: %d"
$ Do not translate Netid's. 
$  241\tThis message prints max netids in use.  
MAX_TRANS            "\t\tMax Transmits queued: %d"
$  242\tThis message prints the maximum transmissions queued.  
MAX_RCVS             "\nMax Receives queued: %d"
$  243\tThis message prints the maximum receives queued.  
MAX_STAT_BLK         "\t\tMax Stat Blks queued: %d"
$  244\tThis message prints the maximum statistics blocks queued.  
INTRPT_LOST          "\nInterrupts lost: %d"
$  245\tThis message prints the total number of interrupts lost.  
WDT_INTRPT_LOST      "\t\tWDT Interrupts lost: %d"
$  246 This message prints the number of WDT interrupts lost.  
TM_INTRPT_LOST       "\nTimeout Ints lost: %d"
$  247 This message prints the number of timeout interrupts lost.  
STATUS_LOST          "\t\tStatus lost: %d"
$  248\tThis message prints the number of times status is lost.  
RCV_PCKTS_LOST       "\nReceive Packets Lost: %d"
$  249 This message prints the number of receive packets lost.  
NO_MBUFS_ERR         "\t\tNo Mbuf Errors: %d"
$  250\tThis message prints the number of No Mbufs errors.  
NO_MBUFS_EXT_ERR     "\nNo Mbuf Extension Errors: %d"
$  251 This message prints the number of No Mbufs Ext Errors.  
RCV_INTRPT_CNT       "\tReceive Int Count: %d"
$  252 This message prints the number of receive interrupts.  
TX_INTRPT_CNT        "\nTransmit Int Count: %d"
$  253\tThis message prints the number of transmit interrupts.  
CRC_ERR_CNT          "\tCRC Error Count: %d"
$  254\tThis message prints the number of CRC errors.  
ALIGN_ERR_CNT        "\nAlign Error Count: %d"
$  255\tThis message prints the number of Alignment errors.  
RCV_OVRN_CNT         "\t\tRecv Overrun Count: %d"
$  256\tThis message prints the number of receive overruns.  
PKTS_SHORT           "\nPackets Too Short: %d"
$  257\tThis message prints the number of packets which were too short.  
PKTS_LONG            "\t\tPackets Too Long: %d"
$  258\tThis message prints the number of packets which were too long.  
NO_RESOURCES         "\nNo Resources Count: %d"
$  259\tThis message prints No resoruces count.  
RCV_PKTS_DISC        "\t\tRecv Pkts Discarded: %d"
$  260\tThis message prints the number of receive packets discarded.  
XMIT_MAX_COLL        "\nXmit Max Collisions: %d"
$  261\tThis message prints the maximum collisions count.  
XMIT_CARR_LOST       "\t\tXmit Carrier Lost: %d"
$  262\tThis message prints the number of times carrier is lost.  
XMIT_UN_COUNT        "\nXmit Underrun Count: %d"
$  263\tThis message prints the transmit underrun count.  
XMIT_LOST_CNT        "\t\tXmit CTS Lost Count: %d"
$ XMIT_LOST_COUNT This message prints the transmit CTS lost count.  
XMIT_TIM             "\nXmit Timeouts: %d"
$  264 This message prints the number of transmit timeouts.  
PARITY_ERR           "\t\tParity Errors: %d"
$  265 This message prints the number of parity errors.  
DIAG_OVFLOW          "\nDiag Overflow Count: %d"
$  266 This message prints the Diag overflow count.  
EXEC_Q_OVFLOW        "\t\tExecute Q Overflows: %d"
$  267 This message prints the Execute Q overflow count.  
EXEC_CMD_ERR         "\nExecute Cmd Errors: %d"
$ EXEC_MCD_ER This message prints the Execute Cmd Errors count.  
HOST_EOLB	"\t\tHost side End of List Bit: %d"
$ HOST_EOLB This message prints Host size End of List Bit seen
ADPT_EOLB	"\nAdpt side End of List Bit: %d"
$ ADPT_EOLB This message prints Adapter/586 End of List Bit seen
ADPT_PACK	"\tAdapter pkts to be uploaded: %d"
$ ADPT_PACK This message prints adpt rec packets to be uploaded to host
HOST_PACK	"\nAdapter pkts uploaded: %d"
$ HOST_PACK This message prints adpt rec packets acutally uploaded to host
START_RECP	"\tStart receptions to adpt: %d"
$ START_RECP This message prints start receptions issued to adapter
REC_DMA		"\nReceive DMA timeouts(lock up): %d"
$ REC_DMA This message prints receive DMA timeouts due to lock up
PKTS_NONETID         "\t\tPackets Rejected No NetID: %d"
$  268\tThis message prints out the number of packets rejected that  
$  268\tdo not have a NETID.  
PKTS_VAL_ID          "\nPackets Accepted Valid NetID: %d"
$  269\tThis message prints out the number of packets accpeted that  
$  269\thave a correct net ID.  
OVFLOW_PKTS          "\tOverflow Packets Received: %d"
$  270\tThis message prints out the number of overflow packets received.  
TX_ADAPT_ER          "\nPackets Transmitted and Adapter Errors Detected: %d"
$  270\tThis message prints out the number of packets transmitted   
$  270\tand the adapter detected an error during transmission.  
SHORT_CABLE          "\n\tShorted Cable Errors\t\t: %d"
$  270\tThis message prints out the number of shorted cable errors.  
X25_STATS		"\nX.25 STATISTICS (%1$s%2$d) :"
$ This message prints out the header for X.25 statistics
DATA_P_TX		"\nData Packets Xmit: %d"
$ This message prints out the number of data packets transmitted.
CALL_REQ_P_TX		"\t\tCall Req Pkts Xmit: %d"
$ This message prints out the number of call request packets transmitted. 
CALL_ACC_P_TX		"\nCall Accpt Pkts Xmit: %d"
$ This message prints out the number of call accept packets transmitted.
CLR_REQ_P_TX		"\t\tClear Req Pkts Xmit: %d"
$ This message prints out the number of clear request packets transmitted.
CLR_CONF_P_TX		"\nClear Conf Pkts Xmit: %d"
$ This msg prints out the number of clear confirmation packets transmitted. 
RST_REQ_P_TX		"\t\tReset Req Pkts Xmit: %d"
$ This msg prints out the number of reset request packets transmitted. 
RST_CONF_P_TX		"\nReset Conf Pkts Xmit: %d"
$ This msg prints out the number of reset confirmation packets transmitted. 
RSTRT_P_TX		"\t\tRestart Pkts Xmit: %d"
$ This msg prints out the number of restart packets transmitted. 
RSTRT_CONF_P_TX		"\nRestart Conf Pkts Xmit: %d"
$ This msg prints out the number of restart confirmation packets transmitted. 
DATA_P_RX		"\tData Pkts Recv: %d"
$ This msg prints out the number of data packets received. 
INC_CALL_P_RX		"\nIncoming Call Pkts Recv: %d"
$ This msg prints out the number of call packets received. 
CALL_CONN_P_RX		"\tCall Conn Pkts Recv: %d"
$ This msg prints out the number of call connection packets received. 
CLR_IND_P_RX		"\nClear Ind Pkts Recv: %d"
$ This msg prints out the number of clear indication packets received. 
CLR_CONF_P_RX		"\t\tClear Conf Pkts Recv: %d"
$ This msg prints out the number of clear confirmation packets received. 
RST_IND_P_RX		"\nReset Ind Pkts Recv: %d"
$ This msg prints out the number of reset indication packets received. 
RST_CONF_P_RX		"\t\tReset Conf Pkts Recv: %d"
$ This msg prints out the number of reset confirmation packets received. 
RSTRT_P_RX		"\nRestart Pkts Recv: %d"
$ This msg prints out the number of restart packets received. 
RSTRT_CONF_P_RX		"\t\tRestart Conf Pkts Recv: %d"
$ This msg prints out the number of restart confirmation packets received. 
$
$ =========== start of NS module messages ===========
$
$ Do not translate NS
$ 
NS_ACT_CONN	"Active NS connections"
NSERR_STATS	"NS error statistics:\n"
NSERR_OHIST	"Output Error Histogram:\n"
NSERR_IHIST	"Input Error Histogram:\n"
$
$ msgs below come in pairs:
$	first message is singular version,
$	second message is plural version
$
$ sometimes there is no plural form; these are called "singletons",
$ and may be distinguished by their alignment:
$
$ 		singletons
$		|	plural forms
$		|	|
$		v	v
$
NS_NONUCN		"connection dropped due to no new sockets"
NS_NONUCN2		"connections dropped due to no new sockets"
NS_GONAWY		"connection terminated due to our end dying"
NS_GONAWY2		"connections terminated due to our end dying"
NS_NOCONN		"connection dropped due to inability to connect"
NS_NOCONN2		"connections dropped due to inability to connect"
NS_NOTME		"connection incompleted due to mismatched id's"
NS_NOTME2		"connections incompleted due to mismatched id's"
NS_WRNCON		"connection dropped due to mismatched id's"
NS_WRNCON2		"connections dropped due to mismatched id's"
NS_BDREAS		"packet dropped out of sequence"
NS_BDREAS2		"packets dropped out of sequence"
NS_LSTDUP		"packet duplicating the highest packet"
NS_LSTDUP2		"packets duplicating the highest packet"
NS_NOTYET		"packet refused as exceeding allocation"
NS_NOTYET2		"packets refused as exceeding allocation"
NS_CONNATTEMPT		"connection initiated"
NS_CONNATTEMPT2		"connections initiated"
NS_ACCEPTS		"connection accepted"
NS_ACCEPTS2		"connections accepted"
NS_CONNECTS		"connection established"
NS_CONNECTS2		"connections established"
NS_DROPS		"connection dropped"
NS_DROPS2		"connections dropped"
NS_CONNDROPS		"embryonic connection dropped"
NS_CONNDROPS2		"embryonic connections dropped"
NS_CLOSED		"connection closed (includes drops)"
NS_CLOSED2		"connections closed (includes drops)"
NS_SEGSTIMED		"packet where we tried to get rtt"
NS_SEGSTIMED2		"packets where we tried to get rtt"
NS_RTTUPDATED		"time we got rtt"
NS_RTTUPDATED2		"times we got rtt"
NS_DELACK		"delayed ack sent"
NS_DELACK2		"delayed acks sent"
NS_TIMEOUTDROP		"connection dropped in rxmt timeout"
NS_TIMEOUTDROP2		"connections dropped in rxmt timeout"
NS_REXMTTIMEO	"retransmit timeout"
NS_PERSISTTIMEO	"persist timeout"
NS_KEEPTIMEO	"keepalive timeout"
NS_KEEPPROBE		"keepalive probe sent"
NS_KEEPPROBE2		"keepalive probes sent"
NS_KEEPDROPS		"connection dropped in keepalive"
NS_KEEPDROPS2		"connections dropped in keepalive"
NS_SNDTOTAL		"total packet sent"
NS_SNDTOTAL2		"total packets sent"
NS_SNDPACK		"data packet sent"
NS_SNDPACK2		"data packets sent"
NS_SNDBYTE		"data byte sent"
NS_SNDBYTE2		"data bytes sent"
NS_SNDREXMITPACK	"data packet retransmitted"
NS_SNDREXMITPACK2	"data packets retransmitted"
NS_SNDREXMITBYTE	"data byte retransmitted"
NS_SNDREXMITBYTE2	"data bytes retransmitted"
NS_SNDACKS		"ack-only packet sent"
NS_SNDACKS2		"ack-only packets sent"
NS_SNDPROBE		"window probe sent"
NS_SNDPROBE2		"window probes sent"
NS_SNDURG		"packet sent with URG only"
NS_SNDURG2		"packets sent with URG only"
NS_SNDWINUP		"window update-only packet sent"
NS_SNDWINUP2		"window update-only packets sent"
NS_SNDCTRL		"control (SYN|FIN|RST) packet sent"
NS_SNDCTRL2		"control (SYN|FIN|RST) packets sent"
NS_SNDVOID		"request to send a non-existant packet"
NS_SNDVOID2		"requests to send a non-existant packet"
NS_RCVTOTAL		"total packet received"
NS_RCVTOTAL2		"total packets received"
NS_RCVPACK		"packet received in sequence"
NS_RCVPACK2		"packets received in sequence"
NS_RCVBYTE		"byte received in sequence"
NS_RCVBYTE2		"bytes received in sequence"
NS_BADSUM		"packet received with ccksum errs"
NS_BADSUM2		"packets received with ccksum errs"
NS_RCVBADOFF		"packet received with bad offset"
NS_RCVBADOFF2		"packets received with bad offset"
NS_RCVSHORT		"packet received too short"
NS_RCVSHORT2		"packets received too short"
NS_RCVDUPPACK		"duplicate-only packet received"
NS_RCVDUPPACK2		"duplicate-only packets received"
NS_RCVDUPBYTE		"duplicate-only byte received"
NS_RCVDUPBYTE2		"duplicate-only bytes received"
NS_RCVPARTDUPPACK	"packet with some duplicate data"
NS_RCVPARTDUPPACK2	"packets with some duplicate data"
NS_RCVPARTDUPBYTE	"dup. byte in part-dup. packet"
NS_RCVPARTDUPBYTE2	"dup. bytes in part-dup. packet"
NS_RCVOOPACK		"out-of-order packet received"
NS_RCVOOPACK2		"out-of-order packets received"
NS_RCVOOBYTE		"out-of-order byte received"
NS_RCVOOBYTE2		"out-of-order bytes received"
NS_RCVPACKAFTERWIN	"packet with data after window"
NS_RCVPACKAFTERWIN2	"packets with data after window"
NS_RCVBYTEAFTERWIN	"byte rcvd after window"
NS_RCVBYTEAFTERWIN2	"bytes rcvd after window"
NS_RCVAFTERCLOSE	"packet rcvd after 'close'"
NS_RCVAFTERCLOSE2	"packets rcvd after 'close'"
NS_RCVWINPROBE	"rcvd window probe packet"
NS_RCVDUPACK	"rcvd duplicate ack"
NS_RCVACKTOOMUCH	"rcvd ack for unsent data"
NS_RCVACKTOOMUCH2	"rcvd acks for unsent data"
NS_RCVACKPACK	"rcvd ack packet"
NS_RCVACKBYTE		"byte acked by rcvd acks"
NS_RCVACKBYTE2		"bytes acked by rcvd acks"
NS_RCVWINUPD	"rcvd window update packet"
IDP_TOOSMALL		"packet smaller than a header"
IDP_TOOSMALL2		"packets smaller than a header"
IDP_TOOSHORT		"packet smaller than advertised"
IDP_TOOSHORT2		"packets smaller than advertised"
IDP_BADSUM		"packet with bad checksums"
IDP_BADSUM2		"packets with bad checksums"
NSERR_ERROR		"call to ns_error"
NSERR_ERROR2		"calls to ns_error"
NSERR_OLDSHORT		"error ignored due to insufficient addressing"
NSERR_OLDSHORT2		"errors ignored due to insufficient addressing"
NSERR_OLDNSERR		"error request in response to error packets"
NSERR_OLDNSERR2		"error requests in response to error packets"
NSERR_TOOSHORT		"error packet received incomplete"
NSERR_TOOSHORT2		"error packets received incomplete"
NSERR_BADCODE		"error packet received of unknown type"
NSERR_BADCODE2		"error packets received of unknown type"
NS_UNSPEC_DEST		"Unspecified Error at Destination"
NS_UNSPEC_DEST2		"Unspecified Errors at Destination"
NS_CHKSUM_DEST		"Bad Checksum at Destination"
NS_CHKSUM_DEST2		"Bad Checksums at Destination"
NS_NOLSTN_SOCK		"No Listener at Socket"
NS_NOLSTN_SOCK2		"No Listeners at Socket"
NS_PKT_DEST_SP		"Packet Refused due to lack of space at Destination"
NS_PKT_DEST_SP2		"Packets Refused due to lack of space at Destination"
NS_UNSPEC_GATE		"Unspecified Error while gatewayed"
NS_UNSPEC_GATE2		"Unspecified Errors while gatewayed"
NS_CHKSUM_GATE		"Bad Checksum while gatewayed"
NS_CHKSUM_GATE2		"Bad Checksums while gatewayed"
NS_PKT_TOOFWD		"Packet forwarded too many times"
NS_PKT_TOOFWD2		"Packets forwarded too many times"
NS_PKT_TOOBIG		"Packet too large to be forwarded"
NS_PKT_TOOBIG2		"Packets too large to be forwarded"
NS_UNK_INTRANS	"Unknown XNS error code 0%o in transit"
NS_UNK_ATDEST	"Unknown XNS error code 0%o in transit"
$
$ =========== start of ISO module messages ===========
$
$ Do not translate ISO
$ 
$ msgs with codes beginning PISO_ come in pairs:
$	first message is singular version,
$	second message is plural version
$
ISO_ESH_SR		"\t%d esh sent, %d esh received\n"
ISO_ISH_SR		"\t%d ish sent, %d ish received\n"
ISO_RD_SR		"\t%d rd sent, %d rd received\n"
ISO_ES_NOMEM		"\t%d pdus not sent due to insufficient memory\n"
ISO_ES_BADCSUM		"\t%d pdus received with bad checksum\n"
ISO_ES_BADVERS		"\t%d pdus received with bad version number\n"
ISO_ES_BADTYPE		"\t%d pdus received with bad type field\n"
ISO_ES_TOOSMALL		"\t%d short pdus received\n"
ISO_CNS_SENT		"%s:\n\t%d total packets sent\n"
ISO_CNS_FRAGMENTS	"\t%d total fragments sent\n"
ISO_CNS_TOTAL		"\t%d total packets received\n"
ISO_CNS_TOOSMALL	"\t%d with fixed part of header too small\n"
ISO_CNS_BADHLEN		"\t%d with header length not reasonable\n"
PISO_CNS_BADCSUM	"\t%d incorrect checksum\n"
PISO_CNS_BADCSUM2	"\t%d incorrect checksums\n"
ISO_CNS_BADADDR		"\t%d with unreasonable address lengths\n"
ISO_CNS_NOSEG		"\t%d with forgotten segmentation information\n"
ISO_CNS_NOPROTO		"\t%d with an incorrect protocol identifier\n"
ISO_CNS_BADVERS		"\t%d with an incorrect version\n"
ISO_CNS_TTLEXPIRED	"\t%d dropped because the ttl has expired\n"
ISO_CNS_CACHEMISS	"\t%d clnp cache misses\n"
ISO_CNS_CONGEST_SET	"\t%d clnp congestion experience bits set\n"
ISO_CNS_CONGEST_RCVD	"\t%d clnp congestion experience bits received\n"
PISO_CLTPS_HDROPS	"%s:\n\t%u incomplete header\n"
PISO_CLTPS_HDROPS2	"%s:\n\t%u incomplete headers\n"
PISO_CLTPS_BADLEN	"\t%u bad data length field\n"
PISO_CLTPS_BADLEN2	"\t%u bad data length fields\n"
PISO_CLTPS_BADSUM	"\t%u bad checksum\n"
PISO_CLTPS_BADSUM2	"\t%u bad checksums\n"
ISO_CB_NO_NL		"%s control block: symbol not in namelist\n"
ISO_ISOPCB_A		"prev 0x%x next 0x%x isop_prev 0x%x isop_next 0x%x???\n"
ISO_TPCB_A		"addr: 0x%x, prev 0x%x\n"
ISO_ACT_CONN		"Active ISO net connections"));
ISO_X25_CONN		"Active X25 net connections"));
ISO_TP_NOTCFG		"TP not configured\n\n"));
ISO_RECVING		"%*sReceiving:\n"
PISO_TS_PARAM_IGN	"\t%*s%d variable parameter ignored\n"
PISO_TS_PARAM_IGN2	"\t%*s%d variable parameters ignored\n"
PISO_TS_INV_PCODE	"\t%*s%d invalid parameter code\n"
PISO_TS_INV_PCODE2	"\t%*s%d invalid parameter codes\n"
PISO_TS_INV_PVAL	"\t%*s%d invalid parameter value\n"
PISO_TS_INV_PVAL2	"\t%*s%d invalid parameter values\n"
PISO_TS_INV_DUTYPE	"\t%*s%d invalid dutype\n"
PISO_TS_INV_DUTYPE2	"\t%*s%d invalid dutypes\n"
PISO_TS_NEGOTFAILED	"\t%*s%d negotiation failure\n"
PISO_TS_NEGOTFAILED2	"\t%*s%d negotiation failures\n"
PISO_TS_INV_DREF	"\t%*s%d invalid destination reference\n"
PISO_TS_INV_DREF2	"\t%*s%d invalid destination references\n"
PISO_TS_INV_SUFX	"\t%*s%d invalid suffix parameter\n"
PISO_TS_INV_SUFX2	"\t%*s%d invalid suffix parameters\n"
ISO_TS_INB_LENGTH	"\t%*s%d invalid length\n"
PISO_TS_BAD_CSUM	"\t%*s%d invalid checksum\n"
PISO_TS_BAD_CSUM2	"\t%*s%d invalid checksums\n"
PISO_TS_DT_OOO		"\t%*s%d DT out of order\n"
PISO_TS_DT_OOO2		"\t%*s%d DTs out of order\n"
PISO_TS_DT_NIW		"\t%*s%d DT not in window\n"
PISO_TS_DT_NIW2		"\t%*s%d DTs not in window\n"
PISO_TS_DT_DUP		"\t%*s%d duplicate DT\n"
PISO_TS_DT_DUP2		"\t%*s%d duplicate DTs\n"
PISO_TS_XPD_NIW		"\t%*s%d XPD not in window\n"
PISO_TS_XPD_NIW2	"\t%*s%d XPDs not in window\n"
PISO_TS_XPD_DUP		"\t%*s%d XPD w/o credit to stash\n"
PISO_TS_XPD_DUP2	"\t%*s%d XPDs w/o credit to stash\n"
PISO_TS_LCDT_REDUCED	"\t%*s%d time local credit reneged\n"
PISO_TS_LCDT_REDUCED2	"\t%*s%d times local credit reneged\n"
PISO_TS_CONCAT_RCVD	"\t%*s%d concatenated TPDU\n"
PISO_TS_CONCAT_RCVD2	"\t%*s%d concatenated TPDUs\n"
ISO_SENDING		"%*sSending:\n"
PISO_TS_XPDMARK_DEL	"\t%*s%d XPD mark discarded\n"
PISO_TS_XPDMARK_DEL2	"\t%*s%d XPD marks discarded\n"
PISO_TS_XPD_INTHEWAY	"\t%*sXPD stopped data flow %d time\n"
PISO_TS_XPD_INTHEWAY2	"\t%*sXPD stopped data flow %d times\n"
PISO_TS_ZFCDT		"\t%*s%d time foreign window closed\n"
PISO_TS_ZFCDT2		"\t%*s%d times foreign window closed\n"
ISO_MISC		"%*sMiscellaneous:\n"
PISO_TS_MB_SMALL	"\t%*s%d small mbuf\n"
PISO_TS_MB_SMALL2	"\t%*s%d small mbufs\n"
PISO_TS_MB_CLUSTER	"\t%*s%d cluster\n"
PISO_TS_MB_CLUSTER2	"\t%*s%d clusters\n"
ISO_TS_QUENCH		"\t%*s%d source quench \n"
PISO_TS_RCVDECBIT	"\t%*s%d dec bit\n"
PISO_TS_RCVDECBIT2	"\t%*s%d dec bits\n"
ISO_MBUF_CHAINS		"\t%*sM:L ( M mbuf chains of length L)\n"
ISO_OVER		"\t%*s%d: over 16\n"
ISO_TS_EOT_INPUT	"\t%*s%d EOT rcvd\n"
ISO_TS_EOT_SENT		"\t%*s%d EOT sent\n"
ISO_TS_EOT_USER		"\t%*s%d EOT indication%s\n"
ISO_CONNECTS		"%*sConnections:\n"
PISO_TS_XDT_FMT		"\t%*s%d connection used extended format\n"
PISO_TS_XDT_FMT2	"\t%*s%d connections used extended format\n"
PISO_TS_USE_TXPD	"\t%*s%d connection allowed transport expedited data\n"
PISO_TS_USE_TXPD2	"\t%*s%d connections allowed transport expedited data\n"
PISO_TS_CSUM_OFF	"\t%*s%d connection turned off checksumming\n"
PISO_TS_CSUM_OFF2	"\t%*s%d connections turned off checksumming\n"
PISO_TS_CONN_GAVEUP	"\t%*s%d connection dropped due to retrans limit\n"
PISO_TS_CONN_GAVEUP2	"\t%*s%d connections dropped due to retrans limit\n"
PISO_TS_TP4_CONN	"\t%*s%d tp 4 connection\n"
PISO_TS_TP4_CONN2	"\t%*s%d tp 4 connections\n"
PISO_TS_TP0_CONN	"\t%*s%d tp 0 connection\n"
PISO_TS_TP0_CONN2	"\t%*s%d tp 0 connections\n"
ISO_RRT			"\n%*sRound trip times, listed in (sec: usec):\n"
ISO_RRT_TBL		"\t%*sCategory     Smoothed avg | Deviation    | Deviation/Avg\n"
ISO_TPDUS_RECVD		"\n%*sTpdus RECVD [%d valid, %3.6f %% of total (%d); %d dropped]\n"
ISO_TS_DT_RCVD		"\t%*sDT  %6d   AK  %6d   DR  %4d   CR  %4d \n"
ISO_TS_XDP_RCVD		"\t%*sXPD %6d   XAK %6d   DC  %4d   CC  %4d   ER  %4d\n"
ISO_TS_TPDU_SENT	"\n%*sTpdus SENT [%d total, %d dropped]\n"
ISO_TS_DT_SENT		"\t%*sDT  %6d   AK  %6d   DR  %4d   CR  %4d \n"
ISO_XPD_SENT		"\t%*sXPD %6d   XAK %6d   DC  %4d   CC  %4d   ER  %4d\n"
ISO_RETRANS		"\n%*sRetransmissions:\n"
ISO_TS_RETRANS_CR	"\t%*sCR  %6d   CC  %6d   DR  %6d \n"
ISO_TS_RETRANS_DT	"\t%*sDT  %6d (%5.2f%%)\n"
ISO_TS_RETRANS_XDP	"\t%*sXPD %6d (%5.2f%%)\n"
ISO_E_TIMERS		"\n%*sE Timers: [%6d ticks]\n"
PISO_TS_ESET1		"%*s%6d timer set "
PISO_TS_ESET12		"%*s%6d timers set "
PISO_TS_ESET2		"\t%6d timer expired "
PISO_TS_ESET22		"\t%6d timers expired "
PISO_TS_ESET3		"\t%6d timer cancelled\n"
PISO_TS_ESET32		"\t%6d timers cancelled\n"
ISO_C_TIMERS		"\n%*sC Timers: [%6d ticks]\n"
PISO_TS_CSET1		"%*s%6d timer set "
PISO_TS_CSET12		"%*s%6d timers set "
PISO_TS_CSET2		"\t%6d timer expired "
PISO_TS_CSET22		"\t%6d timers expired "
PISO_TS_CSET3		"\t%6d timer cancelled\n"
PISO_TS_CSET32		"\t%6d timers cancelled\n"
PISO_TS_CCAN_INACT	"%*s%6d inactive timer cancelled\n"
PISO_TS_CCAN_INACT2	"%*s%6d inactive timers cancelled\n"
ISO_PATHOS		"\n%*sPathological debugging activity:\n"
PISO_TS_ZDEBUG		"\t%*s%6d CC sent to zero dref\n"
PISO_TS_ZDEBUG2		"\t%*s%6d CCs sent to zero dref\n"
PISO_TS_YDEBUG		"\t%*s%6d random DT dropped\n"
PISO_TS_YDEBUG2		"\t%*s%6d random DTs dropped\n"
PISO_TS_VDEBUG		"\t%*s%6d illegally large XPD TPDU\n"
PISO_TS_VDEBUG2		"\t%*s%6d illegally large XPD TPDUs\n"
ISO_TS_LDEBUG		"\t%*s%6d faked reneging of cdt\n"
ISO_ACK_REASONS		"\n%*sACK reasons:\n"
ISO_ACK_DONT		"\t%*s%6d not acked immediately\n"
ISO_ACK_STRAT_EACH	"\t%*s%6d strategy==each\n"
ISO_ACK_STRAT_FULLWIN	"\t%*s%6d strategy==fullwindow\n"
ISO_ACK_DUP		"\t%*s%6d duplicate DT\n"
ISO_ACK_EOT		"\t%*s%6d EOTSDU\n"
ISO_ACK_REORDER		"\t%*s%6d reordered DT\n"
ISO_ACK_USRRCV		"\t%*s%6d user rcvd\n"
ISO_ACK_FCC		"\t%*s%6d fcc reqd\n"
SOL_STATS            "\nSERIAL OPTICAL STATISTICS (%1$s%2$d) :\n"
$  This is the message banner for device statistics.  
MSB_COUNT	"\nMSB Frames Collapsed: %d"
LSB_COUNT	"\t\tLSB Frames Collapsed: %d"
$ The following FDDI messages need not be translated because they are
$ used to print out device drivers statistics.
FDDI_1  	"\nRx ovflw      = %d"
FDDI_2		"\t\tSt que ovflow = %d"
FDDI_3  	"\nRx que ovflow = %d"
FDDI_4  	"\t\tRx no buf     = %d"
FDDI_5  	"\nPkts rejected = %d"
FDDI_6  	"\t\tAdapter err   = %d"
FDDI_7  	"\nSMT Err Low  = 0x%x"
FDDI_8  	"\t\tSMT Err Hi   = 0x%x"
FDDI_9  	"\nSMT Evt Low  = 0x%x"
FDDI_10  	"\t\tSMT Evt Hi   = 0x%x"
FDDI_11  	"\nConn Policy  = 0x%x"
FDDI_12  	"\t\tPort Event   = 0x%x"
FDDI_13  	"\nSetCount Lo  = 0x%x"
FDDI_14  	"\t\tSetCount Hi  = 0x%x"
FDDI_15  	"\nACI Code     = 0x%x"
FDDI_16  	"\t\tPurge Frames = 0x%x"
FDDI_17  	"\nUcode Vers   = 0x%x"

$ new netstat -v output messages for fddi - NO TRANSLATION NECESSARY!

FDDI_18  "\n\nTx ints\t\t= %d"
FDDI_19  "\t\tRx ints\t\t= %d"
FDDI_20  "\nTx que ovflw\t= %d"
FDDI_21  "\t\tRx que ovflw\t= %d"
FDDI_22  "\nSt que ovflw\t= %d"
FDDI_23  "\t\tRx cmd cmplt\t= %d"
FDDI_24  "\nPkts rejected\t= %d"
FDDI_25  "\t\tAdapter err\t= %d"
FDDI_26  "\nRx no buf\t= %d"
FDDI_27  "\n\nSMT Err Low\t= 0x%x"
FDDI_28  "\t\tSMT Err Hi\t= 0x%x"
FDDI_29  "\nSMT Evt Low\t= 0x%x"
FDDI_30  "\t\tSMT Evt Hi\t= 0x%x"
FDDI_31  "\nConn Policy\t= 0x%x"
FDDI_32  "\t\tPort Event\t= 0x%x"
FDDI_33  "\nSetCount Lo\t= 0x%x"
FDDI_34  "\t\tSetCount Hi\t= 0x%x"
FDDI_35  "\nACI Code\t= 0x%x"
FDDI_36  "\t\tPurge Frames\t= 0x%x"
FDDI_37  "\nUcode Vers\t= 0x%x"
FDDI_38  "\n\nECM State Machine: 	 	[%d] =	%s\n"
FDDI_39  "PCM State Machine (Port A): 	[%d] = 	%s\n"
FDDI_40  "PCM State Machine (Port B):  	[%d] =	%s\n"
FDDI_41  "CFM State Machine: 	 	[%d] =	%s\n"
FDDI_42  "RMT State Machine:  		[%d] =	%s\n"
FDDI_43  "\nMax Stat Blks queued: %d"
$  This message prints the maximum FDDI statistics blocks queued.  
UDP_OVERFLOW	"\t%u socket buffer overflows\n"
$  This message prints the number of buffer overflows caused by udp packets.
NONDD	"No network device driver information is available.\n"
BADFORK	"Fork attempt for netstat command failed.\n"
BADEXEC	"netstat exec attempt for %s command failed.  Error number was %d.\n"
ESTFAIL	"Initial retrieval of kernel network device driver data failed.\n"
ACTFAIL	"Actual retrieval of kernel network device driver data failed.\n"
NO_OPEN "Unable to open device %s, errno = %d\n"
NO_DEVCON "Unable to connect to device %s, errno = %d\n"
NO_MEM  "Out of memory\n"
NO_DEVSTATS "Unable to get statistics on device %s, errno = %d\n"
UDP_RECV "\t%u datagram%s received\n"
$ this message prints the number of UDP packets (ie. datagrams) received singular
UDP_RECV2 "\t%u datagrams%s received\n"
$ this message prints the number of UDP packets (ie. datagrams) received plural
UDP_DROPPED "\t%u dropped due to no socket\n"
$ this message prints the number of UDP packets (ie. datagrams) dropped due
$ to the socket going away.
UDP_DROPPED_MULTI "\t%u broadcast/multicast datagram%s dropped due to no socket\n"
$ this message prints the number of UDP broadcast/multicast packets
$ (ie. datagrams) dropped due to the socket going away. singular.
UDP_DROPPED_MULTI2 "\t%u broadcast/multicast datagrams%s dropped due to no socket\n"
$ this message prints the number of UDP broadcast/multicast packets
$ (ie. datagrams) dropped due to the socket going away. plural.
UDP_DELIVERED "\t%u delivered\n"
$ this message prints the number of UDP packets actually delivered.
UDP_OUTPUT "\t%u datagram%s output\n"
$ this message prints the number of UDP packets output. singular
UDP_OUTPUT2 "\t%u datagrams%s output\n"
$ this message prints the number of UDP packets output. plural
IP_BAD_OPT "\t%u with bad options\n"
$ this message prints the number of UDP packets with bad options in them.
IP_BAD_VER "\t%u with incorrect version number\n"
$ this message prints the number of UDP packets with bad version numbers
$ in them.
IP_REASS "\t%u packet%s reassembled ok\n"
$ this message prints the number of UDP packets that were reassembled. singular
IP_REASS2 "\t%u packets%s reassembled ok\n"
$ this message prints the number of UDP packets that were reassembled. plural
IP_FOR_US "\t%u packet%s for this host\n"
$ this message prints the number of UDP packets for this host. singular
IP_FOR_US2 "\t%u packets%s for this host\n"
$ this message prints the number of UDP packets for this host. plural
IP_BAD_PROTO "\t%u packet%s for unknown/unsupported protocol\n"
$ this message prints the number of UDP packets with a bad protocol. singular
IP_BAD_PROTO2 "\t%u packets%s for unknown/unsupported protocol\n"
$ this message prints the number of UDP packets with a bad protocol. plural
IP_FROM_US "\t%u packet%s sent from this host\n"
$ this message prints the number of UDP packets from this host. singular
IP_FROM_US2 "\t%u packets%s sent from this host\n"
$ this message prints the number of UDP packets from this host. plural
IP_RAWOUT "\t%u packet%s sent with fabricated ip header\n"
$ this message prints the number of UDP packets with fabricated headers. singular
IP_RAWOUT2 "\t%u packets%s sent with fabricated ip header\n"
$ this message prints the number of UDP packets with fabricated headers. plural
IP_ODROPPED "\t%u output packet%s dropped due to no bufs, etc.\n"
$ this message prints the number of UDP packets dropped. singular
IP_ODROPPED2 "\t%u output packets%s dropped due to no bufs, etc.\n"
$ this message prints the number of UDP packets dropped. plural
IP_ODISCARD "\t%u output packet%s discarded due to no route\n"
$ this message prints the number of UDP packets discarded due to no route. singular
IP_ODISCARD2 "\t%u output packets%s discarded due to no route\n"
$ this message prints the number of UDP packets discarded due to no route. plural
IP_FRAGED "\t%u output datagram%s fragmented\n"
$ this message prints the number of UDP packets fragmented on output. singular
IP_FRAGED2 "\t%u output datagrams%s fragmented\n"
$ this message prints the number of UDP packets fragmented on output. plural
IP_OFRAG "\t%u fragment%s created\n"
$ this message prints the number of UDP packets fragments created. singular
IP_OFRAG2 "\t%u fragments%s created\n"
$ this message prints the number of UDP packets fragments created. plural
IP_CANT_FRAG "\t%u datagram%s that can't be fragmented\n"
$ this message prints the number of UDP packets that can't be fragmented. singular
IP_CANT_FRAG2 "\t%u datagrams%s that can't be fragmented\n"
$ this message prints the number of UDP packets that can't be fragmented. plural
IP_MULTI_DROP "\t%u IP Multicast packet%s dropped due to no receiver\n"
$ this message prints the number of UDP Multicast packets that were dropped. singular
IP_MULTI_DROP2 "\t%u IP Multicast packets%s dropped due to no receiver\n"
$ this message prints the number of UDP Multicast packets that were dropped. plural
IP_OVERFLOW "\t%u ipintrq overflow%s\n"
$ this message prints the number of ipintrq overflows. singular
$ do not translate ipintrq
IP_OVERFLOW2 "\t%u ipintrq overflows%s\n"
$ this message prints the number of ipintrq overflows. plural
$ do not translate ipintrq
IGMP_RECV "\t%u message%s received\n"
$ this message prints the number of IP Multicast messages received. singular
IGMP_RECV2 "\t%u messages%s received\n"
$ this message prints the number of IP Multicast messages received. plural
IGMP_TOO_SHORT "\t%u message%s received with too few bytes\n"
$ this message prints the number of short IP Multicast messages received. singular
IGMP_TOO_SHORT2 "\t%u messages%s received with too few bytes\n"
$ this message prints the number of short IP Multicast messages received. plural
IGMP_BAD_CKSUM "\t%u message%s received with bad checksum\n"
$ this message prints the number of IP Multicast messages with bad checksum
$ values. singular
IGMP_BAD_CKSUM2 "\t%u messages%s received with bad checksum\n"
$ this message prints the number of IP Multicast messages with bad checksum
$ values. plural
IGMP_RCV_QUERIES "\t%u membership quer%s received\n"
$ this message prints the number of IP Multicast messages membership queries
$ received. singular
IGMP_RCV_QUERIES2 "\t%u membership queries%s received\n"
$ this message prints the number of IP Multicast messages membership queries
$ received. plural
IGMP_RCV_BADQUERIES "\t%u membership quer%s received with invalid field(s)\n"
$ this message prints the number of bad IP Multicast messages membership queries
$ received. singular
IGMP_RCV_BADQUERIES2 "\t%u membership queries%s received with invalid field(s)\n"
$ this message prints the number of bad IP Multicast messages membership queries
$ received. plural
IGMP_RCV_RPTS "\t%u membership report%s received\n"
$ this message prints the number of IP Multicast membership reports received. singular
IGMP_RCV_RPTS2 "\t%u membership reports%s received\n"
$ this message prints the number of IP Multicast membership reports received. plural
IGMP_RCV_BADRPTS "\t%u membership report%s received with invalid field(s)\n"
$ this message prints the number of bad IP Multicast membership reports
$ received. singular
IGMP_RCV_BADRPTS2 "\t%u membership reports%s received with invalid field(s)\n"
$ this message prints the number of bad IP Multicast membership reports
$ received. plural
IGMP_RCV_OURRPTS "\t%u membership report%s received for groups to which we belong\n"
$ this message prints the number of IP Multicast membership reports
$ received for groups to which we belong. singular
IGMP_RCV_OURRPTS2 "\t%u membership reports%s received for groups to which we belong\n"
$ this message prints the number of IP Multicast membership reports
$ received for groups to which we belong. plural
IGMP_SND_RPTS "\t%u membership report%s sent\n"
$ this message prints the number of IP Multicast membership reports we sent. singular
IGMP_SND_RPTS2 "\t%u membership reports%s sent\n"
$ this message prints the number of IP Multicast membership reports we sent. plural
