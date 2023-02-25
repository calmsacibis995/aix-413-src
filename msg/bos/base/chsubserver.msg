$ ENGLISH_SOURCE_VERSION= 1.2
$ @(#)44	1.2  src/msg/bos/base/chsubserver.msg, bosmsg, msg411, 9435A411a 8/25/94 16:03:11
$ 
$ COMPONENT_NAME: TCPIP chsubserver.msg
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
$ messages for chsubserver shell script
$
$quote "
$ It is assumed that any words that are all capitalized will NOT be translated.

$set CHSUBSERVER
CHSUB1 "usage: %s [-a|-d|-c] -v service_name -p protocol [-t socket_type ] [-w ] wait_or_nowait ] [-u user ] [-g program ] [-V new_service_name ] [-P new_protocol ] [-T new_socket_type ] [-W new_wait_or_nowait ] [-U new_user ] [-G new_program ] [ -r server ] [-C config_file ] [ program_and_args ] ]\n"
$
$ Where service_name is the service to add (-a), delete (-d) or change (-c) in
$ the config_file.  The config_file is the configuration file to be altered.
$ The default configuration file is /etc/inetd.conf.
$ Where protocol is the protocol of the service_name to be altered. It can be
$ either tcp or udp.
$ Where socket_type is the socket type of the service_name to be altered.
$ It can be either dgram, stream, sunrpc_tcp or sunrpc_udp.
$ Where wait_or_nowait is the wait characteristic of the service_name to be
$ altered. It can be either wait or nowait.
$ Where user is the user that the service_name program should run as.
$ Where program is the program to be run if the service_name is requested.
$ Where program_and_args are the program and its arguments to be run if the
$ service_name is requested.
$ All the arguments that begin with new_ are new versions of the arguments
$ explained above. For example new_protocol is the new protocol of the
$ new_service_name to be altered.
$ Where server is the program to send a SIGHUP signal to, services_name is
$ a service name in /etc/services and /etc/inetd.conf. 
$ And config_file is an alternate configuration file.  The default is
$ /etc/inetd.conf.
$
CHSUB2 "%s: error in updating %s\n"
$ An error occurred while updating the configuration file. The default
$ configuration file is /etc/inetd.conf.
CHSUB3 "%s: %s not in %s\n"
$ The service_name being changed did not exist in the configuration file.
