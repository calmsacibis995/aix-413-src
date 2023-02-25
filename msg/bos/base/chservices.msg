$ ENGLISH_SOURCE_VERSION= 1.2
$ @(#)42	1.2  src/msg/bos/base/chservices.msg, bosmsg, msg411, 9435A411a 8/25/94 16:03:00
$ 
$ COMPONENT_NAME: TCPIP chservices.msg
$ 
$ FUNCTIONS:
$
$ ORIGINS: 10  26  27 
$
$ (C) COPYRIGHT International Business Machines Corp. 1993
$ All Rights Reserved
$ Licensed Materials - Property of IBM
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$ messages for chservices shell script
$
$quote "
$ It is assumed that any words that are all capitalized will NOT be translated.

$set CHSERVICES
CHSERV1 "usage: %s [-a|-d|-c] -v service_name -p protocol -n port [-V new_service_name] [-P new_protocol] [-N new_port] [-u \"aliases [aliases]...\"]\n"
$ Where service_name is a service name in /etc/services.
$ Where protocol is a protocol name in /etc/services (ie. udp or tcp).
$ Where port is a port number in /etc/services.
$ Where new_service_name is a new service name for /etc/services.
$ Where new_protocol is a new protocol name for the new_service_name for
$ /etc/services (ie. udp or tcp).
$ Where new_port is a new port number for the new_service_name for
$ /etc/services.
$
CHSERV2 "%s: error in updating %s\n"
$ An error occurred while updating the configuration file. The default
$ configuration file is /etc/services.
CHSERV3 "%s: %s not in %s\n"
$ The service_name parameter did not exist in the configuration file. The
$ default configuration file is /etc/services.
