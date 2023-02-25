# @(#)72  1.1  src/rspc/kernext/pcmcia/ssisa/pcm.trc.S, pcmciaker, rspc411, 9433A411a 8/6/94 15:08:03
#
#   COMPONENT_NAME: PCMCIAKER
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
* /etc/trcfmt
+342 1.0 "pcmcia_ss" \
        $DD1, \ 
	0x80 { "get_adp_cnt:" }, \
	0x83 { "get_ss_info:"  "errno:" X1 "devno=" X4 \
		"r=" X2 "retcode=" X1 O1.0 \
		"compliance=" X2 "numadapter=" D1 "firstadapter=" D1 }, \
	0x84 { "inq_adapter:"  "errno:" X1 "devno=" X4 \
		"r=" X2 "retcode=" X1 O1.0 \
		"pbuffer=" X4 \
		"numsockets=" D1 "numwindows=" D1 "numedcs=" D1 }, \
	0x85 { "get_adapter:"  "errno:" X1 "devno=" X4 \
		"r=" X2 "retcode=" X1 O1.0 \
		"state=" X1 "scrouting=" X1 }, \
	0x86 { "set_adapter:"  "errno:" X1 "devno=" X4 \
		"r=" X2 "retcode=" X1 O1.0 \
		"state=" X1 "scrouting=" X1 }, \
	0x87 { "inq_window:"  "errno:" X1 "devno=" X4 \
		"r=" X2 "retcode=" X1 O1.0 \
		"pbuffer=" X4 \
		"window=" D1 "wndcaps=" X2 "sockets=" X1 }, \
	0x88 { "get_window:"  "errno:" X1 "devno=" X4 \
		"r=" X2 "retcode=" X1 O1.0 \
		"window=" D1 "socket=" D1 "size=" X2 \
		"state=" X1 "speed=" X1 "base=" X2 }, \
	0x89 { "set_window:"  "errno:" X1 "devno=" X4 \
		"r=" X2 "retcode=" X1 O1.0 \
		"window=" D1 "socket=" D1 "size=" X2 \
		"state=" X1 "speed=" X1 "base=" X2 }, \
	0x8A { "get_page:"  "errno:" X1 "devno=" X4 \
		"r=" X2 "retcode=" X1 O1.0 \
		"window=" D1 "page=" D1 "state=" X1 O1.0 \
		"offset=" X2 }, \
	0x8B { "set_page:"  "errno:" X1 "devno=" X4 \
		"r=" X2 "retcode=" X1 O1.0 \
		"window=" D1 "page=" D1 "state=" X1 O1.0 \
		"offset=" X2 }, \
	0x8C { "inq_socket:"  "errno:" X1 "devno=" X4 \
		"r=" X2 "retcode=" X1 O1.0 \
		"pbuffer=" X4 \
		"socket=" D1 "scintcaps=" X1 "scrptcaps=" X1 \
			"ctlindcaps" X1 }, \
	0x8D { "get_socket:" "errno:" X1 "devno=" X4 \
		"r=" X2 "retcode=" X1 O1.0 \
		"socket=" D1 "scintmask" X1 "vcclevel=" X1 "vpplevels=" X1 \
		"state=" X1 "ctlind=" X1 "ireqrouting=" X1 "iftype=" X1 }, \
	0x8E { "set_socket:"  "errno:" X1 "devno=" X4 \
		"r=" X2 "retcode=" X1 O1.0 \
		"socket=" D1 "scintmask" X1 "vcclevel=" X1 "vpplevels=" X1 \
		"state=" X1 "ctlind=" X1 "ireqrouting=" X1 "iftype=" X1 }, \
	0x8F { "get_status:"  "errno:" X1 "devno=" X4 \
		"r=" X2 "retcode=" X1 O1.0 \
		"socket=" D1 "cardstate" X1 "socketstate=" X1 "ctlind=" X1 \
		"ireqrouting=" X1 "iftype=" X1 }, \
	0x90 { "reset_socket:" "errno:" X1 "devno=" X4 \
		"r=" X2 "retcode=" X1 O1.0 \
		"socket=" D1 }, \
	0x95 { "inq_edc:"    }, \
	0x96 { "get_edc:"    }, \
	0x97 { "set_edc:"    }, \
	0x98 { "start_edc:"  }, \
	0x99 { "pause_edc:"  }, \
	0x9A { "resume_edc:" }, \
	0x9B { "stop_edc:"   }, \
	0x9C { "read_edc:"   }, \
	0x9D { "get_vendor_info:" "errno:" X1 "devno=" X4 \
		"r=" X2 "retcode=" X1 O1.0 \
		"pbuffer=" X4 \
		"type=" D1 "release=" X2 }, \
	0x9E { "ack_interrupt:" "errno:" X1 "devno=" X4 \
		"sockets=" X1 "ack_stat=" X1 }, \
	0x9F { "prior_handler:" }, \
	0xA0 { "ss_addr:" }
