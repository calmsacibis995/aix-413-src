$ ENGLISH_SOURCE_VERSION= 1.2.1.1
$ @(#)60	1.1  src/msg/bos/base/libtcp_ocs.msg, bosmsg, msg411, GOLD410 4/26/94 16:54:58
$ 
$ COMPONENT_NAME: TCPIP libtcp_ocs.msg
$ 
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ (C) COPYRIGHT International Business Machines Corp. 1992
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$ messages for libtcp_ocs.a
$

$set MS_LIBTCP_OCS

$quote "

ERR_ADDR   	"%s: socket address equals zero.\n"
$               This message indicates an invalid destination address found 
$		in the local request. This error message is written 
$  		to syslog for OCS support.
ERR_FAMILY	"%s: socket address family error: %d.\n"
$               This message indicates an invalid address family type found 
$		in the local request. This error message is written 
$  		to syslog for OCS support.
ERR_INIT	"%s: error initializing odm: %d.\n"
$               This message indicates an odm_initialize() error occurred. 
$		This error message is written to syslog for OCS support.
ERR_PATH	"%s: error setting odm default path: %d.\n"
$               This message indicates an odm_set_path() error occurred for
$		ODM object class, OCSvhost. This error message is written 
$		to syslog for OCS support.
ERR_LOCK	"%s: error locking odm database: %d.\n"
$               This message indicates an odm_lock() error occurred.
$		This error message is written to syslog for OCS support.
ERR_OPEN	"%s: error opening odm object %s: %d.\n"
$               This message indicates an odm_open_class() error occurred for
$		ODM object class, OCSvhost. This error message is written 
$		to syslog for OCS support.
ERR_SEARCH	"%s: error searching odm object %s: %d.\n"
$               This message indicates an odm_get_first() error occurred for
$		ODM object class, OCSvhost. This error message is written 
$		to syslog for OCS support.
ERR_CLOSE	"%s: error closing odm object %s: %d.\n"
$               This message indicates an odm_close_class() error occurred for
$		ODM object class, OCSvhost. This error message is written 
$		to syslog for OCS support.
ERR_UNLOCK 	"%s: error unlocking odm database: %d.\n"
$               This message indicates an odm_unlock() error occurred.
$		This error message is written to syslog for OCS support.
ERR_TERM	"%s: error terminating odm: %d.\n"
$               This message indicates an odm_terminate() error occurred.
$		This error message is written to syslog for OCS support.
ERR_GETSERV	"%s: getservbyname() error: %d.\n"
$               This message indicates that the AIX/ESA Login Monitor
$		service, lm, is not specified properly in /etc/services.
$		This error message is written to syslog for OCS support.
ERR_GETHOST	"%s: gethostbyname() error: %d.\n"
$               This message indicates that the AIX/ESA host machine
$		name is not specified properly in /etc/hosts.
$		This error message is written to syslog for OCS support.
ERR_INVPROTO    "%s: invalid protocol name: %s.\n"
$               This message indicates that the protocol name specified 
$  		in the ODM object class, OCSvhost, is not supported.
$		This error message is written to syslog for OCS support.
ERR_LMSOCK	"%s: error opening socket to LM: %d.\n"
$               This message indicates a socket() error occurred to
$		AIX/ESA host machine where the Login Monitor resides. This 
$		error message is written to syslog for OCS support.
ERR_BINDLA	"%s: error binding local address: %d.\n"
$               This message indicates a bind() error using the local
$		address for this OCS node. This error message is written 
$		to syslog for OCS support.
ERR_CONNLM      "%s: error connecting socket to LM: %d.\n"
$               This message indicates a connect() error occurred to the 
$		AIX/ESA host machine where the Login Monitor resides. This 
$		error message is written to syslog for OCS support.
ERR_VTPTYPE	"%s: protocol type error: %d.\n"
$               This message indicates that the virtual terminal protocol 
$		type used to communicate with the AIX/ESA Login Monitor
$		is invalid. This error message is written to syslog for 
$		OCS support.
ERR_MALLOC      "%s: malloc error on LM message.\n"
$               This message indicates that a malloc() error occurred
$		when dynamically allocating space for the variable-length
$ 		Login Monitor message. This error message is written to 
$		syslog for OCS support.
ERR_SENDLM 	"%s: error transmitting msg to LM: %d.\n"
$               This message indicates that a send() error occurred
$	        when sending Login Monitor a request to open the
$ 		correct ESA device. This error message is written to 
$		syslog for OCS support.
ERR_RCVLM	"%s: error receiving msg from LM: %d.\n"
$               This message indicates that a recv() error occurred
$	        when Login Monitor returns an acknowledgement. This 
$		error message is written to syslog for OCS support.
ERR_STATLM      "%s: status error from LM: %d.\n"
$               This message indicates that the acknowledgement from
$		Login Monitor indicates that AIX/ESA device was NOT 
$		successfully opened. This error message is written 
$	  	to syslog for OCS support.
ERR_ADMIN       "%s: error opening OCS administration device: %d.\n" 
$               This message indicates that the OCS administration
$		device was not successfully opened. This error 
$               message is written to syslog for OCS support.
ERR_TBMID       "%s: ioctl() OCS_GET_TBMID error occurred: %d.\n"
$               This message indicates that the ioctl command
$               OCS_GET_TBMID failed on the OCS administration
$		device. This error message is written to syslog 
$               for OCS support.
ERR_TBMCON      "%s: ioctl() OCS_IS_TBM_CONNECTED error occurred: %d.\n"
$               This message indicates that the ioctl command
$               OCS_IS_TBM_CONNECTED failed on the OCS administration
$		device. This error message is written to syslog 
$               for OCS support.
ERR_NOHOST      "%s: No host nodes are connected: %d.\n" 
$               This message indicates that there are no host
$	        nodes connected to this OCS node from the list 
$               of possible host nodes. This error message is 
$               written to syslog for OCS support.
ERR_CUAT        "%s: odm_get_list() error occurred: %s: %d.\n" 
$               This message indicates that an odm_get_list() error 
$               occurred for ODM object class, CuAt. This error message 
$               is written to syslog for OCS support.
ERR_OCSNODE     "%s: No OCS node name associated with: %s.\n"
$               This message indicates that the CuAt entry was found
$               but there was no hostnode/ocsnode match. This error 
$               message is written to syslog for OCS support.
ERR_HARRAY      "%s: malloc error on Host array.\n"
$               This message indicates that a malloc() error occurred
$		when dynamically allocating space for the array of
$ 		possible host names. This error message is written to 
$		syslog for OCS support.
