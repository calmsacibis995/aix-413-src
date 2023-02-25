$ ENGLISH_SOURCE_VERSION= 1.5
$ @(#)84	1.1  src/msg/bos/base/init.msg, bosmsg, msg411, GOLD410 2/9/94 09:03:22
$
$ comment   
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
$quote "         define quote character.

$len 
$set MS_INIT        Define initial set#
$ Message numbers have been assigned a range of 036-047
$ Do not translate %d, %s, %1$d, %1$s, etc ...


M_OPEN               "0481-036 Cannot open %1$s; errno is %2$d. \n"
$ Do not translate errno 

M_CREATE             "0481-037 Cannot create %s.\n"

M_NEWLVL             "The new run level is %c.\n"

M_SUSER              "SINGLE-USER MODE. \n"
$ Do not translate SINGLE-USER MODE 

M_NENTRY             "0481-038 Cannot find entries for root or su in /etc/passwd.\n"
$ Do not translate root or su or /etc/passwd.  

M_EXEC               "0481-039 The execlp function for %1$s failed with errno %2$d. \n"
$ Do not translate errno or execlp 

M_SUPROMPT           "Do you want to exit SINGLE-USER MODE?\n\
Press Y for Yes, N for No or Enter for the default runlevel: \n"
$ Do not translate SINGLE-USER MODE  
$ Leave a space at the end of the message.  

M_PTAB               "0481-040 The internal process table is full. \n"

M_RAPID              "0481-041 The command is respawning too rapidly. Check for possible errors.\n\
pid:%1$s \"%2$s\"\n"
$ Do not translate pid:%1$s \"%2$s\" 
$ respawning is a UNIX technical term 

M_CMDFAIL            "0481-042 Cannot run command %1$s; exec system call failed with errno %2$d. \n"

M_DEFINED            "The system is initialized. The console is defined.\n"

M_REMOTE             "\n\
Init: The system console, %s, is remote.\n\
Press the <DEL> character key to gain control again.\n"
$ Do not translate init, DEL  

M_NEWLINE            "\n"

M_LVLPROMPT          "Enter a run level between 0 and 9 or s, S, m, or M: "

M_CHANGE             "will change to state %c.\n"

M_BADCHAR            "\n\
0481-043 %3.3o is not a valid character. \n\n"

M_UTMP               "0481-044 Cannot write the utmp entry: %2.2s.\n"
$ Do not translate utmp.  

M_USAGE              "Usage: init [0123456789abcMmNQqSs]\n"

M_NOTTY              "0481-045 Standard input is not a display device. \n"

M_MOVCNTL            "Moving control to the console device.\n"

M_SUENTER            "\n\
SINGLE-USER MODE entered from %s. \n"

M_NOSIG              "0481-046 Cannot send a signal to init.\n"
$ Do not translate init.  

M_PWD                "Password:"

M_SORRY              "0481-047 The password is not valid. \n"

M_DANGER             "Paging space is low. Use the chps command to extend the paging space. \n"
$ Do not translate chps 

M_BRCEXEC "init: 0481-301: The exec subroutine failed, could not execute /etc/brc.\n"
$ Do not translage /etc/brc, exec or init

M_BRCFORK "init: 0481-302: The fork subroutine failed, could not execute /etc/brc.\n"
$ Do not translage /etc/brc, fork or init
