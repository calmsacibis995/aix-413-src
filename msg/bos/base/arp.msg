$ ENGLISH_SOURCE_VERSION= 1.5.1.10
$ @(#)79	1.1  src/msg/bos/base/arp.msg, bosmsg, msg411, GOLD410 2/9/94 08:46:13
$
$
$ COMPONENT_NAME: (NETCMD) Network commands. 
$
$ FUNCTIONS: 
$ 
$ ORIGINS: 27 
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1988, 1989 
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for arp
$ Message numbers have been assigned a range of 231-255.
$ Do not translate arp.
$

$set MS_ARP

$quote "

CANT_OPEN       "0821-231 arp: Cannot open %s.\n"
$ CANT_OPEN	This message is printed when the named file cannot be opened.
BAD_LINE        "0821-232 arp: The line %s is not correct.\n"
$ BAD_LINE	This message is printed when the named line is bad.
PERM_DENIED     "0821-233 arp: Permission is denied.\n"
$ PERM_DENIED	This message is printed when trying to set an ARP entry
$ PERM_DENIED	when not the superuser.
NME_NT_FND	"0821-234 arp: host name %s NOT FOUND\n"
BADADDR		"0821-235 arp: %s: address misformed\n" 
UNKWN_HST       "0821-236 arp: %s: unknown host\n"
$ Do not translate host.
$ UNKNWN_HST	This message is printed when the named host is unknown.
PRNT_INFO       "%s (%s) at "
$ PRNT_INFO	This message prints the host name and the associated address.
INCOMPLETE      "(incomplete)"
$ incomplete means not complete
$ TYPE_EN_PRINT	This message prints the type of an arp entry(ethernet).
TYPE_EN_PRINT	" [ethernet]"
$ Do not translate.
$ TYPE_8023_PRINT	This message prints the type of an arp entry(802.3).
TYPE_8023_PRINT	" [802.3]"
$ Do not translate
$ TYPE_8025_PRINT	This message prints the type of an token ring arp entry.
TYPE_8025_PRINT	" [token ring]"
$ Do not translate
$ TYPE_X25_PRINT	This message prints the type of a X.25 arp entry.
TYPE_X25_PRINT	" [X.25]"
$ Do not translate
$ INCOMPLETE	This message is printed when the ARP entry is incomplete.
PERMANENT       " permanent"
$ Leave a space at the beginning.
$ PERMANENT	This message is printed when the ARP entry is permanent.
PUBLISHED       " published"
$ Leave a space at the beginning.
$ PUBLISHED	This message is printed when the ARP entry is published.
TRAILERS        " trailers"
$ Leave a space at the beginning.
$ TRAILERS	This message is printed when trailer encapsulations can be
$ TRAILERS	sent to the ARP entry.
NO_ENTRY        "0821-237 arp: Entry %s (%s) was not found in local arp table.\n"
$ NO_ENTRY	This message is printed when there's no entry on the named
$ NO_ENTRY	host.
ENT_NT_FND      "0821-238 arp: Entry %s (%s) was not found in local arp table.\n"
$ ENT_NT_FND	This message is printed when trying to delete an entry that
$ ENT_NT_FND	cannot be found.
DELETED         "Entry %s (%s) was deleted from local arp table.\n"
$ DELETED	This message prints the host name and the associated address
$ DELETED	that's been deleted.
BAD_NAMLST      "0821-239 arp: Namelist %s is not found.\n"
$ BAD_NAMLST	This message is printed when the named kernel has a bad
$ BAD_NAMLST	name list.
NAMLST_WRONG    "0821-240 arp: Namelist %s is not valid.\n"
$ NAMLST_WRONG	This message is printed when the named kernel name list 
$ NAMLST_WRONG	is wrong.
NO_MEM_ARPTAB   "0821-241 arp: There is not enough memory available now for arptab.\n"
$ Do not translate arptab.
$ NO_MEM_ARPTAB	This message is printed no memory is available for
$ NO_MEM_ARPTAB	arptab.
NO_RD_ARPTAB    "0821-242 arp: Cannot read arptab."
$ Do not translate arptab.
$ NO_RD_ARPTAB	This message is printed when there is an error reading
$ NO_RD_ARPTAB	arptab.
NEW_PRINT       "  %s (%s) at "
$ Leave a space at the beginning.
$ NEW_PRINT	This message prints the host name and the associated address.
BAD_HW_ADDR    "0821-247 arp: The hardware address %s is not correct.\n"
$ BAD_HW_ADDR	This message is printed when the named hardware address is
$ BAD_HW_ADDR	invalid.
BAD_TKNRG_ADDR  "0821-248 arp: The Token Ring address %s is not correct.\n"
$ Token Ring is a technical term do not translate unless you know the
$ correct technical meaning.
$ BAD_TKNRG_ADDR	This message is printed when the named Token Ring 
$ BAD_TKNRG_ADDR	address is invalid.
TKNRG_BYTES     "rt_bytes = %d, i = %d\n"
$ Do not translate.
$ TKNRG_BYTES	This message is printed to show the number of rt bytes.
BAD_TKNRG_RTE   "0821-249 arp: The Token Ring route %s is not correct.\n"
$ Token Ring is a technical term do not translate unless you know the
$ correct technical meaning.
$ BAD_TKNRG_RTE	This message is printed to show that the named Token Ring
$ BAD_TKNRG_RTE	route is invalid.
USAGE1          "Usage:\tarp HostName\n\tarp -a[n] [/dev/kmem]\n\tarp -d HostName\n\tarp -s ether HostName ether_address [temp] [pub]\n\tarp -s 802.3 HostName ether_address [temp] [pub]\n"
$ USAGE1	This message is printed to show how to use the command.
USAGE2          "\tarp -s 802.5 HostName token_address [token_route] [temp] [pub]\n\tarp -f FileName [type]\n"
$ USAGE2	This message is printed to show how to use the command.
AT		"%s (%s) at "
RT		" rt=%x"
$ Do not translate
BAD_FDDI_RTE   "0821-249 arp: The FDDI Ring route %s is not correct.\n"
$ FDDI Ring is a technical term do not translate unless you know the
$ correct technical meaning.
$ BAD_FDDI_RTE	This message is printed to show that the named Token Ring
$ BAD_FDDI_RTE	route is invalid.
$ TYPE_FDDI_PRINT	This message prints the type of an fddi ring arp entry.
TYPE_FDDI_PRINT	" [fddi]"
USAGE3          "\tarp -s fddi HostName fddi_address [fddi_route] [temp] [pub]\n"
$ USAGE3	This message is printed to show how to use the command.
USAGE4          "\tarp -s fddi HostName fddi_address [fddi_route] [temp] [pub]\n\tarp -s fcs hostname Nport [ipa] [temp] [pub]\n"
$ USAGE4	This message is printed to show how to use the command.
BAD_IPA		"arp: invalid ipa '%s'\n"
$ BAD_IPA user typed in an invalid ipa (Initial Process Associator).
BAD_NPORT	"arp: invalid Nport address '%s'\n"
$ BAD_NPORT user typed in an invalid FCS Nport address.
TYPE_FCS_PRINT	" [fcs]"
$ TYPE_FCS_PRINT	This message prints the type of a fcs arp entry.
