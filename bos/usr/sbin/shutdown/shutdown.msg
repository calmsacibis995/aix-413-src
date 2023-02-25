$ @(#)56	1.10.3.1  src/bos/usr/sbin/shutdown/shutdown.msg, cmdoper, bos411, 9431A411a 8/4/94 15:56:04
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
$ (C) COPYRIGHT International Business Machines Corp. 1989, 1994
$ All Rights Reserved  
$
$ US Government Users Restricted Rights - Use, duplication or  
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.  
$ messages for the shutdown.sh shell script  
$ Note to translators: Do not add a newline character ( 
$ \ or  
$ is not there already.  
$quote "         define quote character.

$len 
$set 1        Define initial set#
$ Message numbers have been assigned a range of 061-066
$ Do not translate shutdown, %d, %s, %1$d, %1$s, or any flags eg. -c, -F etc. 

1                    "Usage: shutdown -[c|d|h|m|r|v] -[Fik] [[+Number]|[HH:MM]]\
[Message]\n"
$       HH designates a 2-digit number for hour; MM designates a 2-digit 
$       number for minutes.  
$ NOT USED

2                    "shutdown: 0481-061 Specify the -c or the -F flag.\n"

3                    "shutdown: 0481-062 The time must be based on the 24 hour \
clock.\n"

4                    "tomorrow at"
$ NOT USED
$       This, and the next message follow the cluster of messages 19-21.  
$       There will be a time placed on the same line after this message. 

5                    "at"
$ NOT USED

6                    "\n\n\
System maintenance is about to begin.\n\
Try again later."

7                    "shutdown: In %s minutes the remote services will no \
longer be available.\n"

8                    "shutdown: In %s minutes the system will shut down.\n"

9                    "shutdown: In %s hours, %s minutes remote services will \
no longer be available.\n"

10                   "shutdown: In %s hours, %s minutes the system will shut \
down.\n"

11                   "shutdown: 0481-063 Specify time as +number or hh:mm.\n"
$ HH is the 2-digit designation for hour; MM is the 2-digit designation for minutes.  

12                   "in %s minutes"
$ NOT USED
$       This, and the next 2 messages follow the cluster of messages 19-21.  

13                   "now."
$ NOT USED

14                   "in 1 minute."
$ NOT USED

15                   "\n\
SHUTDOWN PROGRAM\n"

16                   "The system will now halt - do you want to do this? \
(%1$s or %2$s): \n"

17                   "The system will now reboot - do you want to do this? \
(%1$s or %2$s):  \n"
$       To reboot is to start the system again.  

18                   "The system will now go into maintenance mode - \
do you want to do this? (%1$s or %2$s):  \n"

19                   "shutdown: PLEASE LOG OFF NOW !!!\n"
$ This message is followed by message 20 and message 21.  

20                   "System maintenance is in progress.\n"
$       This message is preceded by message 19 and is followed by message 21.  

21                   "All processes will be killed"
$ NOT USED
$       This message is preceded by messages 19 and 20.  

22                   "shutdown:  THE SYSTEM IS BEING SHUT DOWN NOW\n"

26                   "\n\
Wait for \'....Halt completed....\' before stopping. \n"

27                   "\n\
Wait for reboot."
$       To reboot is to start the system again.  
$ NOT USED

28                   "\n\
Wait for \'INIT: MAINTENANCE MODE\' before continuing.\n"
$       Do not translate INIT:.  

29                   "Process accounting has stopped.\n"

30                   "TCP/IP interfaces are deleted.\n"
$       Do not translate TCP/IP.  

31                   "Error reporting has stopped.\n"

32                   "All processes currently running will now be killed...\n"

33                   "Unmounting the file systems...\n"
$       To unmount is to remove the file system from use.  

34                   "\n\
Distributed Services is currently active."

35                   "Remote services will no longer be available.\n"

36                   "Do you wish to continue? (%1$s or %2$s): \n"

37                   "System maintenance is about to begin.\n"

38                   "Remote services will no longer be available.\n"
$ NOT USED

39                   "Remote services are no longer available.\n"

40                   "\n\
Unmounting the remote file systems...\n"
$       To unmount is to remove the file system from use.  

41                   "\n\
Distributed Services has stopped.\n"

42                   "\n\
shutdown -k is finished.\n\
The system is still up.\n"

43                   "All processes will be killed tomorrow at %s.\n"

44                   "All processes will be killed at %s.\n"

45                   "All processes will be killed in %s minutes.\n"

46                   "All processes will be killed now.\n"

47                   "All processes will be killed in 1 minute.\n"

48                   "Remote services will become unaccessable tomorrow at %s\n"

49                   "Remote services will become unaccessable at %s.\n"

50                   "Remote services will become unaccessable in %s minutes.\n"

51                   "Remote services will no longer be available.\n"

52                   "Remote services will become unaccessable in 1 minute.\n"

53                   "shutdown: 0481-064 shutdown -t mmddHHMM[yy] is not \
valid with the -v or -h options.\n"
$ Do not change the order, the characters may be changed.
$ mm is the 2-digit designation for month;
$ dd is the 2 digit designation for day;
$ HH is the 2-digit designation for hour;
$ MM is the 2-digit designation for minutes;
$ yy is the 2-digit designation for year.

54                   "shutdown: 0481-065 Input date (mmddHHMM[yy]) must be \
after %s %s\n"
$ Do not change the order, the characters may be changed.
$ mm is the 2-digit designation for month;
$ dd is the 2 digit designation for day;
$ HH is the 2-digit designation for hour;
$ MM is the 2-digit designation for minutes;
$ yy is the 2-digit designation for year.

55                   "shutdown: 0481-066 Input date (mmddHHMM[yy]) must \
include valid numbers.\n\
mm(1-12), dd(1-31), HH(0-23), MM(0-59), yy([19]70-99 or [20]00-38)\n"
$ Do not change the order, the characters may be changed.
$ mm is the 2-digit designation for month;
$ dd is the 2 digit designation for day;
$ HH is the 2-digit designation for hour;
$ MM is the 2-digit designation for minutes;
$ yy is the 2-digit designation for year.

56                   "shutdown: 0481-063 Specify time as mmddHHMM[yy].\n"
$ Do not change the order, the characters may be changed.
$ mm is the 2-digit designation for month;
$ dd is the 2 digit designation for day;
$ HH is the 2-digit designation for hour;
$ MM is the 2-digit designation for minutes;
$ yy is the 2-digit designation for year.

57                   "\n\
Wait for \'Rebooting...\' before stopping. \n"
$       Rebooting is to start the system again.

58                   "Usage: shutdown [ -c][ -d][ -F][ -i][ -k][ -m][ -r]\
[ -hv | -t mmddHHMM[yy]][ + Time][ Message]\n"
$ Do not change the order, the characters may be changed.
$ mm is the 2-digit designation for month;
$ dd is the 2 digit designation for day;
$ HH is the 2-digit designation for hour;
$ MM is the 2-digit designation for minutes;
$ yy is the 2-digit designation for year.
59 "WARNING:  The -t option is only supported on systems that have\n\
a power supply which automatically turns power off at shutdown\n\
and an alarm to allow reboot at a later time.  Systems without\n\
this capability may hang or may reboot immediately after shutdown.\n"
