$ ENGLISH_SOURCE_VERSION= 1.5
$ @(#)47	1.2  src/msg/bos/base/netls.msg, bosmsg, msg411, 9430C411a 7/23/94 12:25:24
$
$   COMPONENT_NAME: libnetls
$
$   FUNCTIONS: none
$
$   ORIGINS: 84
$
$
$ --------------------------------------------------------------------------
$
$ netls.msg
$
$ --------------------------------------------------------------------------
$
$quote "
$delset 1
$set 1
$
$ All of the messages in this file are ADMIN.
$
$ The message numbers 1-99 are reserver for the installation script called
$ netls_config
$
$ tidy_exit()
$
1 "An error occured while $2."
2 "error message was:"
3 "NCS setup  cancelled"
$
$ intr_exit()
$
4 "INTERRUPT, installation cancelled"
$
$ main program
$
5 "\nA number of daemons can be automatically started by this \ninstallation when the machine reboots by updating the files in \n/etc/rc[012].d.  Please indicate if you want this to occur. \n"
6 "Do you want the 'llbd' started automatically when the machine boots?"
7 "Do you want 'netlsd' started automatically when the machine boots?"
8 "\n\nNCS must be properly installed and setup for iFOR/LS to function properly.\n\n"
9 "There exists a inititialized database already for the glbd"
10 "Do you wish to use that database when starting the 'glbd' daemon?"
11 "\n\nChecking cell name configurations. Please wait....... \n\n"
12 "\n \nA NCS location broker configuration ordinarily contains one or more \nGlobal Location Broker daemons belonging to one GLB cell, known as the \ndefault cell.  Most configurations use the default cell. \n \nFor some sites, it is desirable to assign some machines to an alternate \ncell, where the GLB object has a different identification.\n"
13 "\n No existing GLB cells were found:  this machine may be assigned to \n either the default cell, or a new alternate cell can be created. \n"
14 "\nThere are already existing GLB cells on this network:  you may choose \nto join one of them, or you may create a new one for this machine. \n \nYou must choose which cell you wish this system to belong to.\n"
15 "[1,2,3]"
16 "three"
17 "[1,2,3,4]"
18 "four"
19 "\\\\t4\\) Choose an existing alternate cell for the system Cell Name.\n"
20 "\nPress enter for a list of available cells.\n"
21 "The following GLB servers were found in alternate cells:"
22 " Svr_Name    Type of Server  Cell_Name             Uuid "
23 " --------    --------------  --------      --------------------------"
24 "\\\\n        You have $number options:\\\\n"
25 "\n\n    WARNING: \n	If you continue[1] the install without choosing a Cell Name \n	then iFOR/LS will not function. If you do not understand these options \n	input [1] to continue and consult your system administrator or the \n	documentation Managing NCS Software" 
26 "Please indicate your choice $choices: "
27 "\n\nConsult documentation Managing NCS Software for further information. \n \nYou have chosen to cancel this setup script, you must run the \n\n         /usr/lib/netls/conf/netls_config \n\nshell script at some point.  This script will generate a shell script which \nwill contain the commands needed to initialize NCS and to \nstart up the NCS and iFOR/LS daemons the first time.  After that, the \ndaemons will start automatically whenever the system is booted. \n"
28 "Will add this system to the default GLB cell"
29 "Will create default GLB cell for this system."
30 "Will create new alternate GLB cell for this system."
31 "Please enter Alternate Cell number[1,2,?]: "
32 "alternate_$ans2"
33 "\"$ans2\" is not a valid Alternate Cell number"
34 "Please enter Alternate Cell number. "
35 "Will add system to be a member of the $search Cell."
36 "Continuing setup..."
37 "\nAfter this installation has been done, you must run the \n\n"
38 "The llbd daemon will be started."
39 "echo Starting the llbd daemon"
40 "	\nshell script.  It contains the commands needed to initialize NCS and to \nstart up the NCS and iFOR/LS daemons the first time.  After that, the \ndaemons will start automatically whenever the system is booted.\n"
41 "sleep 5	# give the llbd time to start"
42 "Using existing GLB database"
43 "The glbd daemon will be started"
44 "echo Starting the glbd daemon" 
45 "	1\) Continue with installation without choosing a Cell_Name.\n	2\) Use the default for the system Cell Name.\n	3\) Create a new alternate cell for the system Cell Name."
46 "echo Starting the first glbd daemon in this cell" 
48 "echo Starting the glbd from replica on $hostn" 
50 "The netlsd daemon will be started"
53 "  echo Starting the netlsd daemon" 
56 "  echo The GLB server is not responding;" 
57 "  echo iFOR/LS server cannot be started."
59 "The NCS setup completed successfully"
60 "Normally logging is turned off for 'netlsd'. (except for error events)"
61 "Do you want logging to be turned on?"
$
$ end of netls_config
$
100 "You do not have X11R5 installed and are trying to run the X version of this command"
101 "Please run this command with some arguments or install X11R5"


$
$delset 2
$set 2
$
$ The messages in this set are for the file auto_config.
$ All messages in this set are ADMIN messages.
$ All messages in this set should be translated.
$ 
$ Do not translate path names, file names, or command names.
$ Do not translate the following words: netlsd, glbd, llbd, GLB, iFOR/LS
$
1 "You must be root to run this program.  Login as \"root\" or use the \"su\" command\nbefore running this program."

2 "The iFOR/LS license server is not installed.  Please refer to your installation\ndocumentation for instructions on how to install your license server."
$ A license server is a server that contains user licenses for applications.
$ If an application needs to be licensed to run, it can get a license from the
$ license server.

3 "Your machine will now be configured to be a standalone iFOR/LS license server."
$ A "standalone" server is a server that only grants services to clients on
$ the same machine.

4 "Stopping the netsld daemon..."

5 "Stopping the glbd daemon..."

6 "Stopping the llbd daemon..."

7 "Removing the current configuration..."

8 "  removing /etc/ncs/glb.e"

9 "  removing /etc/ncs/glb.p"

10 "  removing /etc/ncs/glb_log"

11 "  removing /etc/ncs/glb_obj.txt"

12 "  removing /etc/ncs/glb_site.txt"

13 "  removing /tmp/llbdbase.dat"

14 "Beginning the new standalone configuration..."

15 "Starting the llbd daemon..."

16 "Starting the glbd daemon..."

17 "Starting the netlsd daemon..."

18 "The GLB server is not responding;"

19 "iFOR/LS server cannot be started."

20 "Updating the /etc/inittab file to automatically start your server whenever\nthe system is re-booted..."

21 "Configuration has completed.  Only applications that are started on this\nmachine will be granted licenses by the license server on this machine.\n\nTo perform a more advanced configuration on this machine, run the program\n\"/usr/lib/netls/conf/netls_config\"."

