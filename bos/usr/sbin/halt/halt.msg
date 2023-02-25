$ @(#)02	1.4.1.1  src/bos/usr/sbin/halt/halt.msg, cmdoper, bos411, 9428A410j 8/28/92 09:52:29
$
$ COMPONENT_NAME: (CMDOPER) commands needed for basic system needs  
$
$ FUNCTIONS:   
$
$ ORIGINS: 27  
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when  
$ combined with the aggregated modules for this product)  
$                  SOURCE MATERIALS  
$
$ (C) COPYRIGHT International Business Machines Corp. 1989  
$ All Rights Reserved  
$
$ US Government Users Restricted Rights - Use, duplication or  
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.  
$ messages for halt  
$quote "         define quote character.

$len 
$set MS_HALT        Define initial set#


USAGE                "Usage: halt [-y][-q][-l][-n]\n"
$ Do not translate halt. 

DIALUP               "Halt not recommended on a dialup. If you are really sure use halt -y\n"
$ Do not translate halt. 

LOGHLT               "Halted by %s.\n"

CAUTION              "Caution: A process or processes will not terminate. \n"
$ Do not translate halt. 

NOT_USED             "Halt completed . . .\n"
$ Do not translate halt. 

HALTOVER             "....Halt completed....\n"
$ Do not translate halt.  This message should match the '....Halt completed....'
$ message in shutdown.msg.
