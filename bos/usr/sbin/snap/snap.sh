#! /bin/ksh
# @(#)63  1.25  src/bos/usr/sbin/snap/snap.sh, cmdsnap, bos41J, 9518A_all 5/1/95 09:46:01 
#
# COMPONENT_NAME: (cmdsnap) SNAP Determination Tool
#
# FUNCTIONS: snap.sh
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1991, 1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
## [End of PROLOG]

##
# WARNING!  If you add a file or directory to the destdir path,
# be sure to include it in the dflist in state_func14.
##

#
# Set the LANG variable to C.  Snap only works under
# the C locale.  It's not internationalized.
#
export LANG=C

#
# Define external commands.
#

#
# Define constants
#
bootinfo=/usr/sbin/bootinfo     # Boot information command


#
# Define variable defaults.
#
devtype=""              # Default device type
destdir=/tmp/ibmsupt    # Default destination directory
device=n                # Device required arg
action=n                # Action flag required arg
state=""
memsize="$bootinfo -r"  # Determine real memory size
complist="async dump filesys general kernel lang nfs printer x25 sna tcpip"
dirlist_all="$complist testcase other"
total_bytes=0    # Initialize space byte counter
sleep_time=5            # Sleep time before waking up to check netcmds
netcmdlist="df enq netstat arp rpcinfo exportfs"
			# List of commands that may hang on the network so that
			# we can handle them differently

#
# Define default files.
#
kernel=/unix

#
# Set flags.
#
doall=n                 # Gather all
dogen=n                 # Gather general
dopred=n		# Include Pd* ODM database files in general info
dotcp=n                 # Gather tcpip
donfs=n                 # Gather nfs 
dokern=n                # Gather kernel
doprt=n                 # Gather printer
dodump=n                # Gather dump and /unix
dosna=n                 # Gather sna
dosec=n			# Include security files in general information
dofs=n			# Gather file system 
doasync=n               # Gather async (tty)
dolang=n                # Gather lang info 
dox25=n			# Gather x.25 info
dotar=n                 # Create snap.tar.Z file
dodev=n                 # Put snap.tar.Z onto diskette or tape
doclean=n               # Remove snap gathered files
doview=n		# View component snap files
badargs=n		# Flag for other args with view option
docheck=y		# Flag to check for free space

#
# Working functions
#

# Function: intr_action
intr_action () {
	echo "\n\nYou have chosen to interrupt the current process."
	echo "Press 'enter' for no action, 's' to attempt to kill"
	echo "the current operation, or 'q' to quit out altogether: \c"
	read response

	if [ "$response" = q -o "$response" = Q ]
	then
		exit 10
	else
		if [ "$response" = s -o "$response" = S ]
		then
			cmdtokill=`echo $cmd | awk '{print $1}'`
			pids=`ps -ef | fgrep $cmdtokill | fgrep -v fgrep | awk '{print $2}'`
			numpids=`echo $pids | awk '{print NF}'`
			if [ "$numpids" = 1 ]
			then
				echo "\nKilling process '$cmdtokill' with a process id of $pids...\c"
				kill -9 $pids >/dev/null 2>&1
			else
				counter=1
				gotit=n
				while [ "$counter" -le "$numpids" ]
				do
					pid=`echo $pids | cut -d' ' -f$counter`
					ppid=`ps -ef | fgrep $pid | fgrep -v fgrep | awk '{print $3}'`
					ps -ef | fgrep $ppid | fgrep -v fgrep | fgrep snap >/dev/null 2>&1
					if [ "$?" = 0 ]
					then
						echo "\nKilling process '$cmdtokill' with a process id of $pid...\c"
						gotit=y
						kill -9 $pid >/dev/null 2>&1
						echo " done.\n"
						return
					fi
					((counter += 1))
				done
				[ "$gotit" = n ] && echo "\nProcess '$cmdtokill' has already exited... continuing."
			fi
		else
			echo "\nNo action taken, skipping current pass.... continuing"
			return
		fi
	fi
} # End of intr_action


# Function: ckspace
ckspace () {
# $1 = how much free space is needed
#
#
	total_bytes=`expr $total_bytes / 512`
	tempdir=$destdir
	while [ "$tempdir" != "" ]
	do

	df -k $tempdir >/dev/null 2>&1
	if [ "$?" = 0 ]
	then
		cmd="df -k"
		freespace=`df -k $tempdir` 
		freespace=`echo $freespace | awk '{print $11}'`
		if [ "$freespace" -lt "$total_bytes" ]
		then
			echo "\n\nsnap: not enough space in $tempdir, free space is $freespace kbytes,"
			echo "       $total_bytes kbytes are needed.  Please correct and retry."
			exit 11
		else
			echo " done."
			return;
		fi
	else
		numpaths=`echo $tempdir | awk -F/ '{print NF}'`
 		count=2
		newdir=
		while [ "$count" != "$numpaths" ]
		do
			atom=/`echo $tempdir | cut -d/ -f$count`
			newdir=$newdir$atom
			count=`expr $count + 1`
		done
		tempdir=$newdir
	fi
	done

}  # End ckspace



# Function valid_dir
valid_dir () {
#
#
case "$1" in
        /*      )     echo "Destination directory set to $1"
                        ;;
        * )             echo "Invalid -d argument.  You must specify an absolute path."
                        exit 12
                        ;;          # Not valid path.     
esac

} # End of valid_dir


# Function valid_dev
valid_dev () {
#
#
case "$1" in
        /dev/rmt* )     echo "\nSetting output device to $1...\c"
			bsname=`basename $1 | awk -F . '{print $1}'`
			blksz=`lsattr -l $bsname -a "block_size" -F value` > /dev/null 2>&1
			if [ $? != 0 ]
			then
				exit 99
			fi
			tape=y
			echo " done."
                        ;;
        /dev/*fd* )     echo "\nSetting output device to $1...\c"
			flop=y
			echo " done."
                        ;;
        * )             echo "Invalid output device.\nValid devices are: \c"
                        echo "tape drives, \c"
                        echo "diskette drives "
                        exit 13
                        ;;          # Not output device
esac

} # End of valid_dev


# Function: Initial setup.
initial_setup () {
# Create the directory tree that will hold the gathered information

echo "********Checking and initializing directory structure"
if [ ! -d "$destdir" ]
then
        echo "Creating $destdir directory tree...\c"
        mkdir $destdir  > /dev/null 2>&1  
	echo " done."
fi

dirlist="testcase other"
[ "$doall" = y ] && dirlist=$dirlist_all
[ "$dogen" = y ] && dirlist="general $dirlist"
[ "$dotcp" = y ] && dirlist="tcpip $dirlist"
[ "$donfs" = y ] && dirlist="nfs $dirlist"
[ "$dokern" = y ] && dirlist="kernel $dirlist"
[ "$doprt" = y ] && dirlist="printer $dirlist"
[ "$dodump" = y ] && dirlist="dump $dirlist"
[ "$dosna" = y ] && dirlist="sna $dirlist"
[ "$dofs" = y ] && dirlist="filesys $dirlist"
[ "$doasync" = y ] && dirlist="async $dirlist"
[ "$dolang" = y ] && dirlist="lang $dirlist"
[ "$dox25" = y ] && dirlist="x25 $dirlist"

for i in $dirlist
do
	if [ ! -d "$destdir/$i" ]
	then
        	echo "Creating $destdir/$i directory tree...\c"
		mkdir $destdir/$i > /dev/null 2>&1
		echo " done."
	else
		echo "Directory $destdir/$i already exists... skipping"
	fi
done

echo "********Finished setting up directory $destdir\n"
}  # End of initial_setup()


##############################################################################
# Function: run_cmd
#
# >>>>>>>IMPORTANT<<<<<<<<<>>>>>>>>>>IMPORTANT<<<<<<<<<>>>>>>>IMPORTANT<<<<<<<
# To utilize this facility and add new function to collect more data from a
# system you must use one of three methods to redirect the output
#
# 1) single redirection in the form of " > "
#     Notice the spaces surrounding the greater than sign
# 2) append redirection in the form of "  >> "
#     Notice the spaces surrounding the greater than signs
# 3) The copy command in the form if "cp -p file1 file2 file3 file4...destfile"
#     Notice the "cp -p" with a single space in between the letters
#
#   All of these forms are very important for supporting conversion of commands
#   to forms that will pipe their output to the "wc -c" command to count the
#   number of bytes for the particular operation.
#   If the commands are not in this form, you will get an error message.
#   Again, this is important to understand when adding new functionality into
#   the script for collecting more documentation
#
#   Each command that you would like to run must be in the form of a 3 line
#   sequence to perform the appropriate space checking.
#
#           string="lsdev -C"
#           cmd="lsdev -C >> $snapfile >/dev/null 2>&1"
#           run_cmd
#    
#    string  => This is the string that will be put into the various *.snap
#               files for the specified command.  This should match the command
#               that you actually want to run.  This string should be set to
#               null when output is not put into the *.snap file.
#    cmd     => This is the actual command you would like to run in one of the
#               three appropriate formats as noted above.  Notice the " >> " in
#               this form of the command.  This can be set to null if you just
#               want some informational output "string" in the *.snap file but
#               don't want to run any command.
#    run_cmd => This is just the call to this function to take appropriate 
#               action depending on which pass we are on.
#
#   If you are adding a command that needs to use network facilities at all 
#   and may hang when run, please add the command into the variable list
#   "netcmdlist" at the top of the file !!!
#
# >>>>>>>IMPORTANT<<<<<<<<<>>>>>>>>>>IMPORTANT<<<<<<<<<>>>>>>>IMPORTANT<<<<<<<
#
##############################################################################

run_cmd() {
#
	cmdto_ck=`echo $cmd | sed 's/\( .*\)//'`
	netcmd=n
	for com in $netcmdlist
	do
		if [ "$com" = "$cmdto_ck" ]
		then
			netcmd=y
			break;
		fi
	done

	if [ "$passno" = 1 ]   # calculate total_bytes
	then
		# Section to convert commands in form of "lsdev -C >> $snapfile"
		echo "$cmd" | fgrep " >> " >/dev/null 2>&1
		if [ "$?" = 0 ]
		then

			exec 2>/dev/null
			new_cmd=`echo $cmd | sed 's/\(.*\)\>\>.*/\1/'`
			new_cmd="$new_cmd | wc -c"
			if [ "$netcmd" = y ]
			then
				eval $new_cmd >/dev/null 2>&1 &
				pid=$!
				sleep $sleep_time
				ps -ef | fgrep $pid | fgrep -v fgrep >/dev/null 2>&1
				if [ "$?" = 0 ] 
				then
					echo "\n\nWARNING: Command '$cmdto_ck' is hanging due to an unresponsive server."
					echo "This will affect the filesystem requirement calculation and invalidate it."
					echo "If 'snap' fails due to lack of space it is due to this problem.\n"
				else
					temp_bytes=`eval $new_cmd`
					total_bytes=`expr $total_bytes + $temp_bytes`
				fi
			else
				temp_bytes=`eval $new_cmd` 
				total_bytes=`expr $total_bytes + $temp_bytes`
			fi
			exec 2>&1
			echo ".\c"
		else
			# Section to convert commands in form of "lsdev -C > $snapdir"
			echo "$cmd" | fgrep " > " >/dev/null 2>&1
			if [ "$?" = 0 ]
			then
				exec 2>/dev/null
				new_cmd=`echo $cmd | sed 's/\(.*\)\>.*/\1/'`
                        	new_cmd="$new_cmd | wc -c"
				if [ "$netcmd" = y ]
				then
					eval $new_cmd >/dev/null 2>&1 &
					pid=$!
					sleep $sleep_time
					ps -ef | fgrep $pid | fgrep -v fgrep >/dev/null 2>&1
					if [ "$?" = 0 ] 
					then
						echo "\n\nWARNING: Command '$cmdto_ck' is hanging due to an unresponsive server.\n"
						echo "This will affect the filesystem requirement calculation and invalidate it."
						echo "If 'snap' fails due to lack of space it is due to this problem.\n"
					else
						temp_bytes=`eval $new_cmd`
						total_bytes=`expr $total_bytes + $temp_bytes`
					fi
				else
					temp_bytes=`eval $new_cmd` 
					total_bytes=`expr $total_bytes + $temp_bytes`
				fi

				exec 2>&1
                        	echo ".\c"
			else
				# Section to convert "cp -p" type commands
				echo "$cmd" | fgrep "cp -p" >/dev/null 2>&1
				if [ "$?" = 0 ]
				then
					exec 2>/dev/null
					new_cmd=`echo $cmd | sed 's/\(.*\)\>.*/\
1/'`
					set $new_cmd
					numfields=$#
	
					cat_counter=3
					while [ "$cat_counter" -lt "$numfields" ]
					do
						file_to_cat=`echo $new_cmd | cut -d' ' -f$cat_counter`
       		                        	temp_bytes=`cat $file_to_cat | wc -c`
       		                        	total_bytes=`expr $total_bytes + $temp_bytes`
						cat_counter=`expr $cat_counter + 1`
					done
       		                       	echo ".\c"
					exec 2>&1
				else
					if [ "$cmd" != "" ]
					then
						echo ". failed."
						echo "\nsnap: invalid operation in function \"run_cmd\": \n\tcmd='$cmd'"
						echo "\tcomp='$comp'\n"
						exit 14
					fi
				fi
			fi
		fi


	else
		exec 2>/dev/null
		if [ "$string" = "" ]
		then
			# No string specified, just run command
			if [ "$netcmd" = y ]
			then
				echo "$cmd" | fgrep " >> " >/dev/null 2>&1
				if [ "$?" = 0 ]
				then
					tempcmd=`echo $cmd | sed 's/\(.*\)\>\>.*
/\1/'`
					tempcmd="$tempcmd >/dev/null 2>&1"
				else
					echo "$cmd" | fgrep " > " >/dev/null 2>&1
					if [ "$?" = 0 ]
					then
						tempcmd=`echo $cmd | sed 's/\(.*
\)\>.*/\1/'`
						tempcmd="$tempcmd >/dev/null 2>&1"
					else
						tempcmd=cmd
					fi
				fi

				eval $tempcmd >/dev/null 2>&1 &
				pid=$!
				sleep $sleep_time
				ps -ef | fgrep $pid | fgrep -v fgrep >/dev/null 2>&1
				if [ "$?" = 0 ] 
				then
					echo "\n\nWARNING: Command '$cmdto_ck' is hanging due to an unresponsive server."
					echo "There will not be any 'snap' output for this command.\n"
					kill -9 $pid >/dev/null 2>&1
				else
					eval $cmd
					echo ".\c"
				fi
			else
				eval $cmd
				echo ".\c"
			fi
		else
			# Put string in *.snap file and run command
			dots $comp "$string"
			if [ "$netcmd" = y ]
			then
				echo "$cmd" | fgrep " >> " >/dev/null 2>&1
				if [ "$?" = 0 ]
				then
					tempcmd=`echo $cmd | sed 's/\(.*\)\>\>.*
/\1/'`
					tempcmd="$tempcmd >/dev/null 2>&1"
				else
					echo "$cmd" | fgrep " > " >/dev/null 2>&1
					if [ "$?" = 0 ]
					then
						tempcmd=`echo $cmd | sed 's/\(.*
\)\>.*/\1/'`
						tempcmd="$tempcmd >/dev/null 2>&1"
					else
						tempcmd=cmd
					fi
				fi

				eval $tempcmd >/dev/null 2>&1 &
				pid=$!
				sleep $sleep_time
				ps -ef | fgrep $pid | fgrep -v fgrep >/dev/null 2>&1
				if [ "$?" = 0 ] 
				then
					echo "\n\nWARNING: Command '$cmdto_ck' is hanging due to an unresponsive server."
					echo "There will not be any 'snap' output for this command.\n"
					kill -9 $pid >/dev/null 2>&1
				else
					eval $cmd
					echo ".\c"
				fi
			else
				eval $cmd
				echo ".\c"
			fi
		fi
		exec 2>&1
	fi

} # End of run_cmd

# Function: dots
dots () {
#
        echo "\n....." >> $destdir/$1/$1.snap  2>&1
        echo ".....    $2" >> $destdir/$1/$1.snap  2>&1
        echo ".....\n" >> $destdir/$1/$1.snap  2>&1

} # End of dots
        


#       State functions:
#
#               1. Gather general Information           -g
#               2. Gather tcpip information             -t
#               3. Gather nfs information               -n
#               4. Gather kernel information            -k
#               5. Gather printer information           -p
#               6. Gather dump and /unix                -D
#               7. Gather sna information               -s
#               8. Gather file system information       -f
#               9. Gather async (tty) information       -A
#               10. Gather programming lang information -l
#               11. Gather X.25 information		-x
#               12. Create snap.tar.Z file              -c
#               13. Put tar file to dskt or tape        -o
#               14. Cleanup files                       -r
#
#       Action  |                                       |
#               |---------------------------------------|
#               |                       |               |
#       --------|-------|-------|-------|-------|-------|
#       dogen   | 1                     |               |
#       --------|-------|-------|-------|-------|-------|
#       dotcp   | 2                     |               |
#       --------|-------|-------|-------|-------|-------|
#       donfs   | 3                     |               |
#       --------|-------|-------|-------|-------|-------|
#       dokern  | 4                     |               |
#       --------|-------|-------|-------|-------|-------|
#       doprt   | 5                     |               |
#       --------|-------|-------|-------|-------|-------|
#       dodump  | 6                     |               |
#       --------|-------|-------|-------|-------|-------|
#       dosna   | 7                     |               |
#       --------|-------|-------|-------|-------|-------|
#       dofs    | 8                     |               |
#       --------|-------|-------|-------|-------|-------|
#       doasync | 9                     |               |
#       --------|-------|-------|-------|-------|-------|
#       dolang  | 10                    |               |
#       --------|-------|-------|-------|-------|-------|
#       dox25   | 11                    |               |
#       --------|-------|-------|-------|-------|-------|
#       dotar   | 12                    |               |
#       --------|-------|-------|-------|-------|-------|
#       dodev   | 13                    |               |
#       --------|-------|-------|-------|-------|-------|
#       doclean | 14                    |               |
#       --------|-------|-------|-------|-------|-------|
#       doview  | 15                    |               |
#       --------|-------|-------|-------|-------|-------|
#
# Begin defining state functions.
#

# Function state_func1
state_func1 () {
#            
# 

  comp=general
  snapfile=$destdir/$comp/$comp.snap
  snapdir=$destdir/$comp

  if [ "$passno" = 1 ]
  then
  	echo "Checking space requirement for general information\c"
  else
  	echo "Gathering general system information\c"
  fi

  string="creation date"
  cmd="date >> $snapfile 2>&1"
  run_cmd  

  string=""
  cmd="cp -p /var/adm/ras/trcfile $snapdir >/dev/null 2>&1"
  run_cmd

  if [ -x /usr/bin/errpt ]
  then
  	string=""
  	cmd="errpt -a > $snapdir/errpt.out 2>&1"
  	run_cmd
  else
	LOGFILE=`/usr/lib/errdemon -l | grep "/" | awk '{print $3}'`
	string=""
	cmd="cp -p $LOGFILE $snapdir/errlog >/dev/null 2>&1"
	run_cmd 
  fi	

  string="env"
  cmd="env >> $snapfile 2>&1"
  run_cmd

  string="bootinfo -r"
  cmd="bootinfo -r >> $snapfile 2>&1"
  run_cmd

  string="lsps -a"
  cmd="lsps -a >> $snapfile 2>&1"
  run_cmd

  string=""
  cmd="lslpp -hac > $snapdir/lslpp.hac 2>&1"
  run_cmd

  string="lslpp -lc"
  cmd="lslpp -lc >> $snapfile 2>&1"
  run_cmd

  string="lslpp -La"
  cmd="lslpp -La >> $snapfile 2>&1"
  run_cmd

  lsdev -C -F name | \
  while read name
  do
	string="lsattr -El $name"
	cmd="lsattr -El $name >> $snapfile 2>&1"
	run_cmd
  done

  if [ -x /usr/sbin/lscfg ]
  then
  	string="lscfg -v"
  	cmd="lscfg -v >> $snapfile 2>&1"
  	run_cmd
  fi

  if [ -x /usr/sbin/lsdclient ]
  then
  	string="lsdclient -l"
  	cmd="lsdclient -l >> $snapfile 2>&1"
  	run_cmd
  fi

  if [ -x /usr/sbin/lsspot ]
  then
  	string="lsspot -l"
  	cmd="lsspot -l >> $snapfile 2>&1"
  	run_cmd
  fi

  if [ -f /var/perf/tmp/.SM ]
  then
  	string=""
  	cmd="cp -p /var/perf/tmp/.SM $snapdir > /dev/null 2>&1"
  	run_cmd
  fi

  cwd=`pwd`
  cd /etc/objrepos
  filelist=`ls Cu*`
  for files in $filelist
  do
	string=""
  	cmd="odmget $files > $snapdir/$files.add 2>/dev/null"
	run_cmd
  done
  cd $cwd

  [ "$passno" != 1 ] && echo " done."

  if [ "$dopred" = "y" ] 
  then
	[ "$passno" != 1 ] && echo "Including Pd* ODM files in general information...\c"
	cwd=`pwd`
	cd /etc/objrepos
	filelist=`ls Pd*`
	for files in $filelist
	do
		string=""
 		cmd="odmget $files > $snapdir/$files.add 2>/dev/null"
		run_cmd
	done
  	cd $cwd
	
	[ "$passno" != 1 ] && echo " done."
  fi

  if [ "$dosec" = "y" ]
  then
	[ "$passno" != 1 ] && echo "Including security files in general information\c"
	string=""
	cmd="cp -p /etc/passwd $snapdir/passwd.etc >/dev/null 2>&1"
	run_cmd

	string=""
	cmd="cp -p /etc/group $snapdir/group.etc >/dev/null 2>&1"
	run_cmd

	filelist=`ls /etc/security`
	for file in $filelist
	do
		if [ ! -d "/etc/security/$file" ] 
		then
			string=""
			cmd="cp -p /etc/security/$file $snapdir >/dev/null 2>&1"
			run_cmd
		fi
	done
	[ "$passno" != 1 ] && echo " done."
  fi

  [ "$passno" = 1 ] && echo " done."

} # End of state_func1

# Function state_func2
state_func2 () {
#            
# 

  comp=tcpip
  snapfile=$destdir/$comp/$comp.snap
  snapdir=$destdir/$comp

  if [ "$passno" = 1 ]
  then
  	echo "Checking space requirement for $comp information\c"
  else
  	echo "Gathering $comp system information\c"
  fi

  string="creation date"
  cmd="date >> $snapfile 2>&1"
  run_cmd

  string="Note: If you want the /var/adm/ras/trcfile, snap -g will get it"
  cmd=""
  run_cmd

  string="lssrc -a"
  cmd="lssrc -a >> $snapfile 2>&1"
  run_cmd

  string="netstat -m"
  cmd="netstat -m >> $snapfile 2>&1"
  run_cmd

  string="netstat -in"
  cmd="netstat -in >> $snapfile 2>&1"
  run_cmd
  
  string="netstat -v"
  cmd="netstat -v >> $snapfile 2>&1"
  run_cmd

  string="netstat -s"
  cmd="netstat -s >> $snapfile 2>&1"
  run_cmd

  string="netstat -an"
  cmd="netstat -an >> $snapfile 2>&1"
  run_cmd

  string="netstat -sr"
  cmd="netstat -sr >> $snapfile 2>&1"
  run_cmd

  string="netstat -nr"
  cmd="netstat -nr >> $snapfile 2>&1"
  run_cmd

  string="arp -a"
  cmd="arp -a >> $snapfile 2>&1"
  run_cmd

  string="Interface Configs"
  cmd="lsdev -Cc if -Fname | xargs -n1 ifconfig >> $snapfile 2>&1"
  run_cmd

  string="Name Server "
  cmd="lsnamsv -C >> $snapfile 2>&1"
  run_cmd
 
  # Check if we are nameserver

  [ -r /etc/resolv.conf ] && numbytes=`ls -al /etc/resolv.conf | awk '{ print $5 }'` || numbytes=1

  if [ "$numbytes" = "0" ] 
  then

	# See if named running

	ps -ef | fgrep named | fgrep -v fgrep >/dev/null 2>&1
	if [ "$?" = "0" ]
	then
		# See if -b flag specified with named for alternate boot file
		# otherwise assume /etc/named.boot
		bootfile=`ps -ef | fgrep named |fgrep -v fgrep | awk '
			{ 
     				for (i = 0; i <= NF; ++i) 
 					if ($i == "-b") {            
     						print $(i + 1);         
						exit 15;
					}
			}'`
	else
		# /etc/resolv.conf = 0 bytes, named not running, assume 
		# /etc/named.boot as bootfile
		bootfile=/etc/named.boot
	fi
	
	# named is running but no -b flag so assume /etc/named.boot
	[ ! -s "$bootfile" ] && bootfile=/etc/named.boot

	# copy named database files to snap directory

	string=""
	cmd="cp -p $bootfile `awk '
		$1 == "primary" || $1 == "cache" || $1 == "secondary" {
		print $NF
		}' $bootfile` $snapdir >/dev/null 2>&1"
        if [ "$?" != "0" ]
        then
                echo "\nAssumed named is running without -b flag so assumed /etc/named.boot is the boot file for the nameserver.  Could be that nameserver is not configured correctly.\n"
        fi
	run_cmd
  fi



  string=""
  cmd="cp -p /etc/sendmail.cf $snapdir >/dev/null 2>&1"
  run_cmd
  string=""
  cmd="cp -p /etc/aliases $snapdir >/dev/null 2>&1"
  run_cmd


  echo " done."


} # End of state_func2

# Function state_func3
state_func3 () {
#            
# 
  comp=nfs
  snapfile=$destdir/$comp/$comp.snap
  snapdir=$destdir/$comp

  if [ "$passno" = 1 ]
  then
  	echo "Checking space requirement for $comp information\c"
  else
  	echo "Gathering $comp system information\c"
  fi


  string="creation date"
  cmd="date >> $snapfile 2>&1"
  run_cmd

  string="Note: If you want the /var/adm/ras/trcfile, snap -g will get it"
  cmd=""
  run_cmd

  string="lssrc -a"
  cmd="lssrc -a >> $snapfile 2>&1"
  run_cmd

  string="rpcinfo -p"
  cmd="rpcinfo -p  >> $snapfile 2>&1"
  run_cmd

  string="lsnfsexp"
  cmd="lsnfsexp >> $snapfile 2>&1"
  run_cmd

  string="exportfs"
  cmd="exportfs >> $snapfile 2>&1"
  run_cmd

  if [ -f /etc/exports ]
  then
	string=""
  	cmd="cp -p /etc/exports $snapdir/exports.file 2>&1"
  	run_cmd
  fi

  if [ -f /etc/hanfs ]
  then
  	string=""
  	cmd="cp -p /etc/hanfs $snapdir/hanfs.file 2>&1"
  	run_cmd
  fi

  string="lsnfsmnt"
  cmd="lsnfsmnt >> $snapfile 2>&1"
  run_cmd

  string="nfsstat -z "
  cmd="nfsstat -z >> $snapfile 2>&1"
  run_cmd

  string="nfsstat -n "
  cmd="nfsstat -n >> $snapfile 2>&1"
  run_cmd

  string="nfsstat -r "
  cmd="nfsstat -r >> $snapfile 2>&1"
  run_cmd

  string="nfsstat -s "
  cmd="nfsstat -s >> $snapfile 2>&1"
  run_cmd

  string="nfsstat -c "
  cmd="nfsstat -z >> $snapfile 2>&1"
  run_cmd

  echo " done."


} # End of state_func3

# Function state_func4
state_func4 () {
#            
# 
  comp=kernel
  snapfile=$destdir/$comp/$comp.snap
  snapdir=$destdir/$comp

  if [ "$passno" = 1 ]
  then
  	echo "Checking space requirement for $comp information\c"
  else
  	echo "Gathering $comp system information\c"
  fi


  string="creation date"
  cmd="date >> $snapfile 2>&1"
  run_cmd

  string="Note: If you want the /var/adm/ras/trcfile, snap -g will get it"
  cmd=""
  run_cmd

  string="bootinfo -r"
  cmd="bootinfo -r >> $snapfile 2>&1"
  run_cmd

  string="vmstat"
  cmd="vmstat >> $snapfile 2>&1"
  run_cmd

  string="vmstat -s"
  cmd="vmstat -s >> $snapfile 2>&1"
  run_cmd

  string="env"
  cmd="env >> $snapfile 2>&1"
  run_cmd

  string="lssrc -a"
  cmd="lssrc -a >> $snapfile 2>&1"
  run_cmd

  string="ps -ef"
  cmd="ps -ef >> $snapfile 2>&1"
  run_cmd

  string="ps -leaf"
  cmd="ps -leaf >> $snapfile 2>&1"
  run_cmd

  string="sum on /etc/methods/*"
  cmd="sum /etc/methods/* >> $snapfile 2>&1"
  run_cmd

  string="sum on /etc/drivers/*"
  cmd="sum /etc/drivers/* >> $snapfile 2>&1"
  run_cmd

  echo " done."

} # End of state_func4

# Function state_func5
state_func5 () {
#            
# 
  comp=printer
  snapfile=$destdir/$comp/$comp.snap
  snapdir=$destdir/$comp

  if [ "$passno" = 1 ]
  then
  	echo "Checking space requirement for $comp information\c"
  else
  	echo "Gathering $comp system information\c"
  fi


  string="creation date"
  cmd="date >> $snapfile 2>&1"
  run_cmd

  string="enq -AL"
  cmd="enq -AL >> $snapfile 2>&1"
  run_cmd

  string="lsdev -Ccprinter"
  cmd="lsdev -Ccprinter >> $snapfile 2>&1"
  run_cmd

  for j in `lsdev -Cc printer -F name`
  do
    string="lsattr -El $j"
    cmd="lsattr -El $j >> $snapfile 2>&1"
    run_cmd
  done

  string=""
  cmd="cp -p /etc/qconfig $snapdir 2>&1"
  run_cmd

  echo " done."


} # End of state_func5

# Function state_func6
state_func6 () {
#            
# 
  comp=dump
  snapfile=$destdir/$comp/$comp.snap
  snapdir=$destdir/$comp

  if [ "$passno" = 1 ]
  then
  	echo "Checking space requirement for $comp information\c"
  else
  	echo "Gathering $comp system information\c"
  fi

  string="creation date"
  cmd="date >> $snapfile 2>&1"
  run_cmd

  string="uname -a"
  cmd="uname -a >> $snapfile 2>&1"
  run_cmd

  string="sum of /unix"
  cmd="sum /unix >> $snapfile 2>&1"
  run_cmd

  if [ "$bad_unix" != 1 ]
  then
  	string=""
  	cmd="cat /unix | compress > $snapdir/unix.Z"
  	run_cmd
  fi

  sysdumpdev -L >/dev/null 2>&1
  if [ $? -eq 2 ]	
  then
	nodump=1	
	string="Status of Dump Copy"
	cmd="echo 'No dump was copied because a recent dump does not exist.\n' >> $snapfile 2>&1"
	run_cmd 
  else
  	dumpdata=`sysdumpdev -L 2>/dev/null`
  	dumpdev=`echo $dumpdata | awk '{print $3}'`
	dumpsize=`echo $dumpdata | awk '{print $13}'`

	# If the dump is zero bytes, we don't wont to
        # copy it.  Just put out a warning.
	if [ "$dumpsize" = 0 ]
	then
		zero_bytes=1
		string="Status of Dump Copy"
		cmd="echo 'The dump was not copied, because the dump that\nwas taken is zero bytes.\n' >> $snapfile 2>&1"
		run_cmd 
	else
		ddcount=`expr $dumpsize / 65536`
		oflw=`expr $dumpsize % 65536`
		if [ "$oflw" != "0" ]
		then
			ddcount=`expr $ddcount + 1`
		fi
		if [ "$dumpdev" = "/dev/hd6" ]
		then
			# The dump device is paging space.  Get the name
			# of the file the dump was copied to.
			dump_copy=`echo $dumpdata | awk '{print $24}'`
			if [ "$dump_copy" = "Dump" ]
			then
				# The dump was copied to a file successfully.
				dump_file=`echo $dumpdata | awk '{print $27}'`
				if [ "$bad_unix" != 1 ]
				then
					string=""
					cmd="dd if=$dump_file bs=128b count=$ddcount | compress > $snapdir/dump.Z"
					run_cmd 
				fi
				dumpdev=`echo $dump_file`
			else
				# The attempt to copy the dump on boot failed.
				invalid=1	
				string="Status of Dump Copy"
				cmd="echo 'A dump was taken to /dev/hd6, but an attempt to copy the dump\nto a file on boot failed.' >> $snapfile 2>&1" 
				run_cmd
			fi
		else
			# The dump device is NOT paging space.
			echo $dumpdev | grep "^/" >/dev/null 2>&1
			if [ $? -eq 0 ]
			then
				# The dump device is a local logical volume.
				if [ "$bad_unix" != 1 ]
				then
					string=""
					cmd="dd if=$dumpdev bs=128b count=$ddcount | compress > $snapdir/dump.Z"	
					run_cmd
				fi
	 		else
				# The dump device is remote.
				hostname=`echo $dumpdev | cut -d":" -f1`
				full_filename=`echo $dumpdev | cut -d":" -f2-`
				file_dirname=`dirname $full_filename`
				file_basename=`basename $full_filename`
			
				mount $hostname:$file_dirname /mnt >/dev/null 2>&1
			
				if [ $? -eq 0 ]
				then
					mounted=1
					if [ "$bad_unix" != 1 ]
					then
						string=""
						cmd="dd if=/mnt/$file_basename bs=128b count=$ddcount | compress > $snapdir/dump.Z"
						run_cmd
					fi	
					dumpdev=`echo /mnt/$file_basename`
				else
					badmount=1	
					string="Status of Dump Copy"
					cmd="echo 'Unable to copy the dump from $hostname:$full_filename.\n\
The mount on the directory $file_dirname from the host $hostname failed.\n' >> $snapfile 2>&1"
					run_cmd
				fi  
			fi  
		fi  
     fi  
  fi  

  echo " done."

  if [ "$invalid" = 1 -a "$passno" = 2 ]
  then		
	echo "\nA dump was taken to /dev/hd6, but an attempt to copy the dump to\n\
a file during boot failed.  The dump is no longer valid.\n\
To insure that the next system dump will be copied, check that the\n\
'copy directory' has enough space and that the 'forced copy flag'\n\
for the sysdumpdev command is set to 'TRUE'.  To check these attributes,\n\
run the command 'sysdumpdev -l'.\n"
  fi

  if [ "$nodump" = 1  -a "$passno" = 2 ]
  then
	echo "\nNo dump was copied because a recent dump does not exist.\n"
  fi		

  if [ "$badmount" = 1  -a "$passno" = 2 ]
  then
	echo "\nUnable to copy the dump from $hostname:$full_filename.\n\
The mount on the directory $file_dirname from the host $hostname failed.\n"
  fi

  if [ "$passno" = 1 -a "$badmount" != 1 -a "$invalid" != 1 -a "$nodump" != 1 -a "$zero_bytes" != 1 ]
  then
  	/usr/lib/ras/check_unix $dumpdev
  	if [ $? -eq 1 ]
  	then
		bad_unix=1
  	fi	 
  fi

  if [ "$passno" = 2 -a "$bad_unix" = 1 ]
  then
	echo "\nWARNING: The dump and /unix file were not copied.
The /unix file does not match the latest dump on your
system.  The /unix must be the same unix, or linked to
the same unix, that was running when the dump occurred.
Possible Causes:
1) The /unix file does not exist.
2) The dump that was taken is a partial dump.
3) The /unix file on your system was replaced,
   and the bosboot command was not run.  If this is the
   case, then run the bosboot command and reboot your
   system.\n"
  fi
	
  if [ "$passno" = 2 -a "$zero_bytes" = 1 ]
  then
	echo "\nWARNING: The dump was not copied, because the dump that\nwas taken is zero bytes.\n"
  fi

  if [ "$mounted" = 1 ]
  then
	umount /mnt
  fi

} # End of state_func6

# Function state_func7
state_func7 () {
#            
# 
  comp=sna
  snapfile=$destdir/$comp/$comp.snap
  snapdir=$destdir/$comp

  if [ "$passno" = 1 ]
  then
  	echo "Checking space requirement for $comp information\c"
  else
  	echo "Gathering $comp system information\c"
  fi


  string="creation date"
  cmd="date >> $snapfile 2>&1"
  run_cmd

  string="See file exportsna.out"
  cmd=""
  run_cmd

  string="Note: If you want the /var/adm/ras/trcfile, snap -g will get it"
  cmd=""
  run_cmd

  string=""
  cmd="exportsna > $snapdir/exportsna.out 2>&1"
  run_cmd

  if [ -r /etc/objrepos/X25xlate ]
  then
        string=""
	cmd="odmget X25xlate > $snapdir/X25xlate.add >/dev/null 2>&1"
	run_cmd
  fi

  filelist=`ls /var/sna`
  for file in $filelist
  do
	if [ ! -d "/var/sna/$file" ] 
	then
		string=""
		cmd="cp -p /var/sna/$file $snapdir >/dev/null 2>&1"
		run_cmd
	fi
  done

  echo " done."

} # End of state_func7

# Function state_func8
state_func8 () {
#            
# 
  comp=filesys
  snapfile=$destdir/$comp/$comp.snap
  snapdir=$destdir/$comp

  if [ "$passno" = 1 ]
  then
  	echo "Checking space requirement for $comp information\c"
  else
  	echo "Gathering $comp system information\c"
  fi


  string="creation date"
  cmd="date >> $snapfile 2>&1"
  run_cmd

  string="df -k"
  cmd="df -k >> $snapfile 2>&1"
  run_cmd

  string="mount"
  cmd="mount >> $snapfile 2>&1"
  run_cmd

  string="lsfs -l"
  cmd="lsfs -l >> $snapfile 2>&1"
  run_cmd

  string="lsvg"
  cmd="lsvg  >> $snapfile 2>&1"
  run_cmd

  for j in `lsvg`
  do
    string="lsvg -p $j"
    cmd="lsvg -p $j >> $snapfile 2>&1"
    run_cmd
  done

  string="lsvg | xargs lsvg -l"
  cmd="lsvg | xargs lsvg -l >> $snapfile 2>&1"
  run_cmd

  for j in `lsdev -Cc disk -F name`
  do
    string="lspv -l $j"
    cmd="lspv -l $j >> $snapfile 2>&1"
    run_cmd
  done

  for j in `lsdev -C -t lvtype -Fname`
  do
    string="lslv -l $j"
    cmd="lslv -l $j >> $snapfile 2>&1"
    run_cmd
  done

  for j in `lsdev -Cc disk -F name`
  do
    string="lsattr -El $j"
    cmd="lsattr -El $j >> $snapfile 2>&1"
    run_cmd
  done

  echo " done."

} # End of state_func8

# Function state_func9
state_func9 () {
#            
# 
  comp=async
  snapfile=$destdir/$comp/$comp.snap
  snapdir=$destdir/$comp 

  if [ "$passno" = 1 ]
  then
  	echo "Checking space requirement for $comp information\c"
  else
  	echo "Gathering $comp system information\c"
  fi


  string="creation date"
  cmd="date >> $snapfile 2>&1"
  run_cmd
  
  string="Note: If you want the /var/adm/ras/trcfile, snap -g will get it"
  cmd=""
  run_cmd

  string="Slip Interface Configs"
  cmd="lsdev -Cc if -Fname | grep sl | xargs -n1 ifconfig >> $snapfile 2>&1"
  run_cmd

  string="pdisable"
  cmd="pdisable >> $snapfile 2>&1"
  run_cmd

  string="penable"
  cmd="penable >> $snapfile 2>&1"
  run_cmd

  string="pdelay"
  cmd="pdelay >> $snapfile 2>&1"
  run_cmd

  string="pshare"
  cmd="pshare >> $snapfile 2>&1"
  run_cmd

  string="phold"
  cmd="phold >> $snapfile 2>&1"
  run_cmd

  for j in `lsdev -Cc tty -F name`
  do
    string="lsattr -El $j"
    cmd="lsattr -El $j >> $snapfile 2>&1"
    run_cmd
  done

  if [ -d /etc/uucp ]
  then
	string=""
  	cmd="cp -p /etc/uucp/Devices $snapdir >/dev/null 2>&1"
	run_cmd

	string=""
  	cmd="cp -p /etc/uucp/Dialcodes $snapdir >/dev/null 2>&1"
	run_cmd

	string=""
  	cmd="cp -p /etc/uucp/Dialers $snapdir >/dev/null 2>&1"
	run_cmd

	string=""
  	cmd="cp -p /etc/uucp/Maxuuscheds $snapdir >/dev/null 2>&1"
	run_cmd

	string=""
  	cmd="cp -p /etc/uucp/Maxuuxqts $snapdir >/dev/null 2>&1"
	run_cmd

	string=""
  	cmd="cp -p /etc/uucp/Permissions $snapdir >/dev/null 2>&1"
	run_cmd

	string=""
  	cmd="cp -p /etc/uucp/Poll $snapdir >/dev/null 2>&1"
	run_cmd

	string=""
  	cmd="cp -p /etc/uucp/Systems $snapdir >/dev/null 2>&1"
	run_cmd
  fi
  echo " done."

} # End of state_func9

# Function state_func10
# Gather programming language information
state_func10 () {
#            
# 
  comp=lang
  snapfile=$destdir/$comp/$comp.snap

  if [ "$passno" = 1 ]
  then
  	echo "Checking space requirement for $comp information\c"
  else
  	echo "Gathering $comp system information\c"
  fi


  string="creation date"
  cmd="date >> $snapfile 2>&1"
  run_cmd

  string="env"
  cmd="env >> $snapfile 2>&1"
  run_cmd

  # xlc Compiler
  lslpp -h xlc* >/dev/null 2>&1 

  if [ $? -eq 0 ]
  then
	string="lslpp -h xlc*"
	cmd="lslpp -h xlc* >> $snapfile 2>&1"
	run_cmd
  fi

  # xlf Compiler		
  lslpp -h xlf* >/dev/null 2>&1 

  if [ $? -eq 0 ]
  then
	string="lslpp -h xlf*"
	cmd="lslpp -h xlf* >> $snapfile 2>&1"
	run_cmd
  fi

  # xlp Compiler		
  lslpp -h xlp* >/dev/null 2>&1 

  if [ $? -eq 0 ]
  then
	string="lslpp -h xlp*"
	cmd="lslpp -h xlp* >> $snapfile 2>&1"
	run_cmd
  fi

  # xlC Compiler		
  lslpp -h xlC* >/dev/null 2>&1 

  if [ $? -eq 0 ]
  then
	string="lslpp -h xlC*"
	cmd="lslpp -h xlC* >> $snapfile 2>&1"
	run_cmd
  fi

  # COBOL Compiler
  lslpp -h cobol* >/dev/null 2>&1 

  if [ $? -eq 0 ]
  then
	string="lslpp -h cobol*"
	cmd="lslpp -h cobol* >> $snapfile 2>&1"
	run_cmd
  fi

  # COBOL Compiler

  echo " done."

} # End of state_func10


# Function state_func11
state_func11 () {
#
#
  comp=x25
  snapfile=$destdir/$comp/$comp.snap
  snapdir=$destdir/$comp

  if [ "$passno" = 1 ]
  then
        echo "Checking space requirement for $comp information\c"
  else
        echo "Gathering $comp system information\c"
  fi

  string="creation date"
  cmd="date >> $snapfile 2>&1"
  run_cmd

  string="what /usr/lib/drivers/x25dd"
  cmd="what /usr/lib/drivers/x25dd >> $snapfile 2>&1"
  run_cmd

  string="sum /usr/lib/drivers/x25dd"
  cmd="sum /usr/lib/drivers/x25dd >> $snapfile 2>&1"
  run_cmd

  string="sum /usr/lib/microcode/icaaim.com"
  cmd="sum /usr/lib/microcode/icaaim.com >> $snapfile 2>&1"
  run_cmd

  string="X25 ucode version"
  cmd="strings /usr/lib/asw/x25a.exe  | grep VERSION >> $snapfile 2>&1"
  run_cmd

  string="sum /usr/lib/asw/x25a.exe"
  cmd="sum /usr/lib/asw/x25a.exe >> $snapfile 2>&1"
  run_cmd

  string="what /usr/lib/drivers/if_xt"
  cmd="what /usr/lib/drivers/if_xt >> $snapfile 2>&1"
  run_cmd
  string="sum /usr/lib/drivers/if_xt"
  cmd="sum /usr/lib/drivers/if_xt >> $snapfile 2>&1"
  run_cmd

  string="what /etc/x25xlate"
  cmd="what /etc/x25xlate >> $snapfile 2>&1"
  run_cmd
  string="sum /etc/x25xlate"
  cmd="sum /etc/x25xlate >> $snapfile 2>&1"
  run_cmd

  string="what /lib/libx25s.a"
  cmd="what /lib/libx25s.a >> $snapfile 2>&1"
  run_cmd
  string="sum /lib/libx25s.a"
  cmd="sum /lib/libx25s.a >> $snapfile 2>&1"
  run_cmd

  string="what /bin/xmanage"
  cmd="what /bin/xmanage >> $snapfile 2>&1"
  run_cmd
  string="sum /bin/xmanage"
  cmd="sum /bin/xmanage >> $snapfile 2>&1"
  run_cmd

  string="what /bin/xtalk"
  cmd="what /bin/xtalk >> $snapfile 2>&1"
  run_cmd
  string="sum /bin/xtalk"
  cmd="sum /bin/xtalk >> $snapfile 2>&1"
  run_cmd

  string="what /etc/xroute"
  cmd="what /etc/xroute >> $snapfile 2>&1"
  run_cmd
  string="sum /etc/xroute"
  cmd="sum /etc/xroute >> $snapfile 2>&1"
  run_cmd

  string="what /bin/xmonitor"
  cmd="what /bin/xmonitor >> $snapfile 2>&1"
  run_cmd
  string="sum /bin/xmonitor"
  cmd="sum /bin/xmonitor >> $snapfile 2>&1"
  run_cmd

  string="lsdev -C |grep x25"
  cmd="lsdev -C |grep x25 >> $snapfile 2>&1"
  run_cmd

  string="lsattr -l x25s0 -E"
  cmd="lsattr -l x25s0 -E >> $snapfile 2>&1"
  run_cmd

  string="lsattr -l x25s1 -E"
  cmd="lsattr -l x25s1 -E >> $snapfile 2>&1"
  run_cmd

  string="lsattr -l x25s2 -E"
  cmd="lsattr -l x25s2 -E >> $snapfile 2>&1"
  run_cmd

  string="lsattr -l x25s3 -E"
  cmd="lsattr -l x25s3 -E >> $snapfile 2>&1"
  run_cmd

  string="IP/X.25 host config"
  cmd="odmget X25xlate >> $snapfile 2>&1"
  run_cmd

  string="xroute config"
  cmd="cat /etc/xrt.names >> $snapfile 2>&1"
  run_cmd

  string="x25 statistics"
  cmd=""
  run_cmd

  if [ "$passno" = 2 ]
  then
  echo "\n"
  echo "For all of the info below put this information in $destdir/other"
  echo "\n"
  echo "Please obtain all X.25 (frame and packet layer) config settings"
  echo "for DCE (i.e. system the workstation is connected to). This cannot"
  echo "be done by snap. Contact your X.25 administrator."
  echo " "
  echo "Please provide a diagram indicating your network setup. (e.g."
  echo "DCE to DTE connections"
  echo " "
  echo "Please provide a list of all non-IBM X.25 applications running"
  echo "and who the contact is for technical support of those applications"
  echo " "
  echo "If the problem you are trying to diagnose involves X.25 failures"
  echo "between hosts gather this additional information"
  echo "   - xmonitor frame and pkt layer traces  on both ends of connection"
  echo "     during the time of failure"
  echo "     e.g. xmonitor -packet -frame x25s0"
  echo "   - AIX system trace for x.25 components on both ends of connection"
  echo "     e.g. trace -a -j 225,258,251"
  echo " "
  fi
  echo "done."


} # End of state_func11


# Function state_func12
state_func12 () {
# 
  echo "\nCreating compressed tar file... "
#  echo "Changing directory to $destdir... "
  cd $destdir >/dev/null 2>&1
  if [ $? != 0 ]
  then
        echo "snap: Encountered an error.  Suspect $destdir does not exist."
        echo "snap: Please gather the information first.\n"
        exit 16
  fi

  cd $destdir/other > /dev/null 2>&1
  if [ $? != 0 ]
  then
        echo "snap: Can't find the snap information."
        echo "snap: Please gather the information first.\n"
        exit 17
  fi
  cd $destdir > /dev/null 2>&1

  echo "Starting tar/compress process... Please wait...\c"

  dirlist="$complist testcase other"
  totaldirs=""
  for dirs in $dirlist
  do
	[ -d "$destdir/$dirs" ] && totaldirs="$totaldirs ./$dirs"
  done

  tar -cf- $totaldirs | compress > $destdir/snap.tar.Z

  #tar -cf- ./async ./dump ./filesys ./general ./kernel ./lang ./nfs ./other ./printer ./sna ./tcpip ./testcase | compress > $destdir/snap.tar.Z 

  if [ $? = 1 ]
  then
        echo "snap: Encountered an error.  Removing snap.tar.Z file."
        rm -f snap.tar.Z > /dev/null 2>&1
        echo "snap: Exiting utility."
        exit 18
  fi
  echo " done."
  echo ""
  ls -ln snap.tar.Z

} # End of state_func12

# Function state_func13
state_func13 () {
#
#
#  echo "\nChanging directory to $destdir... "
  cd $destdir >/dev/null 2>&1
  if [ $? != 0 ]
  then
        echo "snap: Encountered an error.  Suspect $destdir does not exist."
        echo "snap: Please gather the information first.\n"
        exit 19
  fi

  cd $destdir/other > /dev/null 2>&1
  if [ $? != 0 ]
  then
        echo "snap: Can't find the snap information."
        echo "snap: Please gather the information first.\n"
        exit 20
  fi
  cd $destdir > /dev/null 2>&1


  #echo "\nLooking for compressed tar file... \c"
  #if [ -f "snap.tar.Z" ]
  #then   
    #echo "found it." 
  #else
    #echo "\nCouldn't find snap.tar.Z file in specified directory."
    #exit 2
  #fi

  echo "\nCopying information to $devtype... Please wait...\c"
  dirlist="$complist testcase other"
  totaldirs=""
  for dirs in $dirlist
  do
	[ -d "$destdir/$dirs" ] && totaldirs="$totaldirs ./$dirs"
  done

  tar -cf$devtype $totaldirs
  if [ $? != 0 ]
  then
        echo "\n*****"
        echo "***** Problem encountered while attempt to send output to device."
        echo "*****"
        echo "***** Please correct problem."
        echo "***** When problem corrected, run ' snap -o$devtype [-d] '\n"
        exit 21
  fi

  echo " done."
  echo "\n****************************************************************"
  echo "******"
  echo "****** Please Write-Protect the output device now..."
  echo "******"
  echo "****************************************************************"

  if [ "$tape" = y ]
  then
    echo ""
    echo "****************************************************************"
    echo "******"
    echo "****** Please label your tape(s) as follows:"
    echo "****** snap                    blocksize=$blksz"
    echo "****** problem: xxxxx          `date`"
    echo "****** 'your name or company's name here'"
    echo "******"
    echo "****************************************************************"
  else
    echo "****************************************************************"
    echo "******"
    echo "****** Please label your diskette(s) as follows:"
    echo "****** snap                    "
    echo "****** problem: xxxxx          `date`"
    echo "******     'your name or company's name here'"
    echo "******                                       Diskette xx of xx"
    echo "****************************************************************"
  fi

} # End of state_func13


# Function state_func14
state_func14 () {
# Cleanup function.  
        found=""
        dflist="async filesys dump general kernel lang nfs other printer sna tcpip testcase x25 snap.tar snap.tar.Z"
        for i in $dflist
        do
                if [ -a "$destdir/$i" ]
                then
                	found=$found"$destdir/$i "
                fi
        done
        if [ -n "$found" ]
        then
          echo "\nThe following directories and files will be deleted:"
          echo "-----------------------------------------------------------"
	  for i in $found
	  do # Enumerate those about to die.
	    echo "$i \c"
	    if [ -d $i ]
	    then echo "(directory)"
	    else echo "(file)"
	    fi
	  done

          echo "\nDo you want me to remove these directories (y/n)? \c"
          read jnk
          if [ "$jnk" = y ]
          then
                echo "Removing...\c"
                rm -r $found >/dev/null 2>&1
                echo " done."
                exit 22
          fi
          echo "Aborting deletion process..."
          exit 23
        fi
        echo "Nothing to clean up"
        exit 24

} # End of state_func14


state_func15 () {
# View function
	
snapfile=$destdir/$component/$component.snap

found=n

if [ "$badargs" = n ]
then
	for choice in $complist
	do
		if [ "$component" = "$choice" ]
		then found=y; break;
		fi
	done

	if [ "$found" = y ] 
	then 
		if [ -r "$destdir/$component/$component.snap" ]
		then
			more $destdir/$component/$component.snap 
		else
			echo "snap: $destdir/$component/$component.snap not found"
			exit 25
		fi
	else
		usage
		exit 26
	fi
else
	usage
	exit 27
fi

} # End of state_func15

usage () {
echo "Usage: snap [-gGStnfkDpsAl] [-o outputdevice] [-d dir]" 
echo "       snap [-a] [-o outputdevice] [-d dir]" 
echo "       snap [-gGStnfkDpsAl] [-c] [-d dir]" 
echo "       snap [-o outputdevice] [-d dir]" 
echo "       snap [-v component]"
echo "       snap [-c] [-d dir]" 
echo "       snap [-r] [-d dir]" 
echo "       snap [-gGS ]" 
echo "       snap [-gG ]" 
echo "       snap [-gS ]\n" 
echo "\t-a Gather all information."
echo "\t-g Gather general information."
echo "\t-G Include Pd* ODM files in general information."
echo "\t-t Gather tcpip information."
echo "\t-n Gather nfs information."
echo "\t-f Gather file system information."
echo "\t-k Gather kernel information."
echo "\t-D Gather dump and /unix."
echo "\t-p Gather printer information."
echo "\t-s Gather sna information."
echo "\t-S Include security files in general information."
echo "\t-A Gather async (tty) information."
echo "\t-x Gather x.25 information."
echo "\t-l Gather programming language information.\n"
echo "\t-c Create snap.tar.Z file."
echo "\t-o Send information to removable output device (/dev/rfd0)."
echo "\t-d Directory to put information (/tmp/ibmsupt)."
echo "\t-r Remove directory (/tmp/ibmsupt).\n"
echo "\t-v Output component snap file to stdout"
echo "\t    Current component choices are:\n\t\t '$complist'\n"
}

# ----------------------------------------------------------
# ---------------------  M  A  I  N  -----------------------
# ----------------------------------------------------------

trap intr_action 2

set -- `getopt AaDd:fgGklcnNo:prv:sStx $*`

if [ $? != 0 ]
then
        usage
        exit 1
fi
 
osver=`uname -v` 
osrev=`uname -r`
if [ "$osrev" != 1 -a "$osver" != 4 ]
then
	echo "This utility designed to run on AIX 4.1 systems only."
	exit 2
fi

userid=`id -ru`
if [ "$userid" != 0 ]
then
        echo "Must be root user [0] to use this utility."
        exit 2
fi


while [ $1 != -- ]
do
        case $1 in

                -A)     doasync=y       # Gather async (tty) information
                        action=y
                        shift;;
                -a)     doall=y         # Gather all information
			dopred=y
			dosec=y
                        action=y
                        shift;;
                -d)     destdir=$2      # Directory to put information
                        valid_dir $destdir
                        shift;shift;;
                -f)     dofs=y          # Gather file system information
                        action=y
                        shift;;
                -g)     dogen=y         # Gather general information
                        action=y
                        shift;;
		-G)	dopred=y	# Include predefined ODM database files
			shift;;
                -k)     dokern=y        # Gather kernel information 
                        action=y
                        shift;;
                -D)     dodump=y        # Gather dump and /unix 
                        action=y
                        shift;;
                -l)     dolang=y        # Gather language information
                        action=y
                        shift;;
                -c)     dotar=y         # Create info.tar.Z file only
                        action=y
                        shift;;
                -n)     donfs=y         # Gather nfs information
                        action=y
                        shift;;
		-N)	docheck=n	# Suppress free space check
			shift;;
                -o)     devtype=$2      # Output Device (/dev/rfd0)
                        valid_dev $devtype
                        dodev=y
                        shift;shift;;
                -p)     doprt=y         # Gather printer information
                        action=y
                        shift;;
                -r)     doclean=y       # Remove directory (/tmp/ibmsupt)
                        action=y
                        shift;;
                -s)     dosna=y         # Gather sna information
                        action=y
                        shift;;
		-S)	dosec=y		# Include security files in general info
			shift;;
                -t)     dotcp=y         # Gather tcpip information
                        action=y
                        shift;;
		-x)     dox25=y        # Gather x.25 information
			action=y
                        shift;;
		-v)	doview=y
			action=y
			component=$2
			shift;shift;;
                *)      usage; exit 3

        esac
done

# Check for any flags or args
if [ "$action" = n -a "$dodev" = n ]
then
        usage
        exit 4
fi

if [ "$dotar" = y -a "$dodev" = y ]
then
	echo "-c and -o are mutually exclusive options."
	exit 5
fi

if [ "$doclean" = y ]
then
	if [ "$doall" = y -o "$dogen" = y -o "$dotcp" = y -o "$donfs" = y -o "$dokern" = y -o "$doprt" = y -o "$dodump" = y -o "$dosna" = y -o "$dofs" = y -o "$doasync" = y -o "$dolang" = y -o "$dotar" = y -o "$dodev" = y ]
	then
		echo "Use -r only with -d."
		exit 6
	fi
fi

#
# Set state action array with appropriate functional actions derived
# from action input arguments. There must be a function associated with 
# each index number.
#

if [ "$doall" = y ]
then
	if [ "$doview" = y -o "$dogen" = y -o "$dotcp" = y -o "$donfs" = y -o "$dokern" = y -o "$doprt" = y -o "$dosna" = y -o "$dofs" = y -o "$doasync" = y -o "$dolang" = y -o "$dodump" = y -o "$doclean" = y -o "$dox25" = y ]
	then
		usage
		exit 7
	else
        	state=" 1 2 3 4 5 6 7 8 9 10 11" 
        	if [ "$dotar" = y -a "$dodev" != y ]
        	then state="$state 12"
        	fi
        	if [ "$dodev" = y ]
        	then state="$state 13"
        	fi
	fi
fi

if [ "$dogen" = y -a "$doall" != y ]
then state=" 1"
        if [ "$dotcp" = y ]
        then state="$state 2"
        fi
        if [ "$donfs" = y ]
        then state="$state 3"
        fi
        if [ "$dokern" = y ]
        then state="$state 4"
        fi
        if [ "$doprt" = y ]
        then state="$state 5"
        fi
        if [ "$dodump" = y ]
        then state="$state 6"
        fi
        if [ "$dosna" = y ]
        then state="$state 7"
        fi
        if [ "$dofs" = y ]
        then state="$state 8"
        fi
        if [ "$doasync" = y ]
        then state="$state 9"
        fi
        if [ "$dolang" = y ]
        then state="$state 10"
        fi
        if [ "$dox25" = y ]
        then state="$state 11"
        fi
        if [ "$dotar" = y -a "$dodev" != y ]
        then state="$state 12"
        fi
        if [ "$dodev" = y ]
        then state="$state 13"
        fi
fi

if [ "$dotcp" = y -a "$doall" != y -a "$dogen" != y ]
then state=" 2"
        if [ "$donfs" = y ]
        then state="$state 3"
        fi
        if [ "$dokern" = y ]
        then state="$state 4"
        fi
        if [ "$doprt" = y ]
        then state="$state 5"
        fi
        if [ "$dodump" = y ]
        then state="$state 6"
        fi
        if [ "$dosna" = y ]
        then state="$state 7"
        fi
        if [ "$dofs" = y ]
        then state="$state 8"
        fi
        if [ "$doasync" = y ]
        then state="$state 9"
        fi
        if [ "$dolang" = y ]
        then state="$state 10"
        fi
        if [ "$dox25" = y ]
        then state="$state 11"
        fi
        if [ "$dotar" = y -a "$dodev" != y ]
        then state="$state 12"
        fi
        if [ "$dodev" = y ]
        then state="$state 13"
        fi
fi

if [ "$donfs" = y -a "$doall" != y -a "$dogen" != y -a "$dotcp" != y ]
then state=" 3"
        if [ "$dokern" = y ]
        then state="$state 4"
        fi
        if [ "$doprt" = y ]
        then state="$state 5"
        fi
        if [ "$dodump" = y ]
        then state="$state 6"
        fi
        if [ "$dosna" = y ]
        then state="$state 7"
        fi
        if [ "$dofs" = y ]
        then state="$state 8"
        fi
        if [ "$doasync" = y ]
        then state="$state 9"
        fi
        if [ "$dolang" = y ]
        then state="$state 10"
        fi
        if [ "$dox25" = y ]
        then state="$state 11"
        fi
        if [ "$dotar" = y -a "$dodev" != y ]
        then state="$state 12"
        fi
        if [ "$dodev" = y ]
        then state="$state 13"
        fi
fi
 
if [ "$dokern" = y -a "$doall" != y -a "$dogen" != y -a "$dotcp" != y -a "$donfs" != y ] 
then state=" 4"
        if [ "$doprt" = y ]
        then state="$state 5"
        fi
        if [ "$dodump" = y ]
        then state="$state 6"
        fi
        if [ "$dosna" = y ]
        then state="$state 7"
        fi
        if [ "$dofs" = y ]
        then state="$state 8"
        fi
        if [ "$doasync" = y ]
        then state="$state 9"
        fi
        if [ "$dolang" = y ]
        then state="$state 10"
        fi
        if [ "$dox25" = y ]
        then state="$state 11"
        fi
        if [ "$dotar" = y -a "$dodev" != y ]
        then state="$state 12"
        fi
        if [ "$dodev" = y ]
        then state="$state 13"
        fi

fi

if [ "$doprt" = y -a "$doall" != y -a "$dogen" != y -a "$dotcp" != y -a "$donfs" != y -a "$dokern" != y ]
then state=" 5"
        if [ "$dodump" = y ]
        then state="$state 6"
        fi
        if [ "$dosna" = y ]
        then state="$state 7"
        fi
        if [ "$dofs" = y ]
        then state="$state 8"
        fi
        if [ "$doasync" = y ]
        then state="$state 9"
        fi
        if [ "$dolang" = y ]
        then state="$state 10"
        fi
        if [ "$dox25" = y ]
        then state="$state 11"
        fi
        if [ "$dotar" = y -a "$dodev" != y ]
        then state="$state 12"
        fi
        if [ "$dodev" = y ]
        then state="$state 13"
        fi
fi

if [ "$dodump" = y -a "$doall" != y -a "$dogen" != y -a "$dotcp" != y -a "$donfs" != y -a "$dokern" != y -a "$doprt" != y ]
then state=" 6"
        if [ "$dosna" = y ]
        then state="$state 7"
        fi
        if [ "$dofs" = y ]
        then state="$state 8"
        fi
        if [ "$doasync" = y ]
        then state="$state 9"
        fi
        if [ "$dolang" = y ]
        then state="$state 10"
        fi
        if [ "$dox25" = y ]
        then state="$state 11"
        fi
        if [ "$dotar" = y -a "$dodev" != y ]
        then state="$state 12"
        fi
        if [ "$dodev" = y ]
        then state="$state 13"
        fi
fi

if [ "$dosna" = y -a "$doall" != y -a "$dogen" != y -a "$dotcp" != y -a "$donfs" != y -a "$dokern" != y -a "$doprt" != y -a "$dodump" != y ]
then state=" 7"
        if [ "$dofs" = y ]
        then state="$state 8"
        fi
        if [ "$doasync" = y ]
        then state="$state 9"
        fi
        if [ "$dolang" = y ]
        then state="$state 10"
        fi
        if [ "$dox25" = y ]
        then state="$state 11"
        fi
        if [ "$dotar" = y -a "$dodev" != y ]
        then state="$state 12"
        fi
        if [ "$dodev" = y ]
        then state="$state 13"
        fi
fi

if [ "$dofs" = y -a "$doall" != y -a "$dogen" != y -a "$dotcp" != y -a "$donfs" != y -a "$dokern" != y -a "$doprt" != y -a "$dodump" != y -a "$dosna" != y ]
then state=" 8"
        if [ "$doasync" = y ]
        then state="$state 9"
        fi
        if [ "$dolang" = y ]
        then state="$state 10"
        fi
        if [ "$dox25" = y ]
        then state="$state 11"
        fi
        if [ "$dotar" = y -a "$dodev" != y ]
        then state="$state 12"
        fi
        if [ "$dodev" = y ]
        then state="$state 13"
        fi
fi

if [ "$doasync" = y -a "$doall" != y -a "$dogen" != y -a "$dotcp" != y -a "$donfs" != y -a "$dokern" != y -a "$doprt" != y -a "$dodump" != y -a "$dosna" != y -a "$dofs" != y ]
then state=" 9"
        if [ "$dolang" = y ]
        then state="$state 10"
        fi
        if [ "$dox25" = y ]
        then state="$state 11"
        fi
        if [ "$dotar" = y -a "$dodev" != y ]
        then state="$state 12"
        fi
        if [ "$dodev" = y ]
        then state="$state 13"
        fi
fi

if [ "$dolang" = y -a "$doall" != y -a "$dogen" != y -a "$dotcp" != y -a "$donfs" != y -a "$dokern" != y -a "$doprt" != y -a "$dodump" != y -a "$dosna" != y -a "$dofs" != y -a "$doasync" != y ]
then state=" 10"
        if [ "$dox25" = y ]
        then state="$state 11"
        fi
        if [ "$dotar" = y -a "$dodev" != y ]
        then state="$state 12"
        fi
        if [ "$dodev" = y ]
        then state="$state 13"
        fi
fi

if [ "$dox25" = y -a "$doall" != y -a "$dogen" != y -a "$dotcp" != y -a "$donfs" != y -a "$dokern" != y -a "$doprt" != y -a "$dodump" != y -a "$dosna" != y -a "$dofs" != y -a "$doasync" != y -a "$dolang" != y  ]
then state=" 11"
        if [ "$dotar" = y -a "$dodev" != y ]
        then state="$state 12"
        fi
        if [ "$dodev" = y ]
        then state="$state 13"
        fi
fi

if [ "$dotar" = y -a "$doall" != y -a "$dogen" != y -a "$dotcp" != y -a "$donfs" != y -a "$dokern" != y -a "$doprt" != y -a "$dodump" != y -a "$dosna" != y -a "$dofs" != y -a "$doasync" != y -a "$dolang" != y -a "$dodev" != y -a "$dox25" != y ] 
then state=" 12"
fi

if [ "$dodev" = y -a "$doall" != y -a "$dogen" != y -a "$dotcp" != y -a "$donfs" != y -a "$dokern" != y -a "$doprt" != y -a "$dodump" != y -a "$dosna" != y -a "$dofs" != y -a "$doasync" != y -a "$dotar" != y -a "$dox25" != y ]
then state=" 13"
fi

if [ "$doclean" = y ]
then state=" 14"
fi

if [ "$doview" = y ]
then
	if [ "$doall" = y -o "$dogen" = y -o "$dotcp" = y -o "$donfs" = y -o "$dokern" = y -o "$doprt" = y -o "$dosna" = y -o "$dofs" = y -o "$doasync" = y -o "$dolang" = y -o "$dotar" = y -o "$dopred" = y -o "$dosec" = y -o "$dodump" = y -o "$devtype" != "" -o "$doclean" = y ]
	then 
		usage
		exit 8
	else
		state=" 15"
	fi
fi


#
# Build state array of functions.
#
for i in $state
do
        state_action[$i]=$i
done

# Ensure that on first pass only component gathering functions get called
no_run="12 13 14 15"
for i in $no_run
do
	state_action[$i]=0
done


#
# State machine is built. First check for enough free space.
# This is pass 1 on state functions.
#
# If any gathering flags then check space requirements
if [ "$docheck" = y ]
then
	if [ "$doall" = y -o "$dogen" = y -o "$dotcp" = y -o "$donfs" = y -o "$dokern" = y -o "$doprt" = y -o "$dodump" = y -o "$dosna" = y -o "$dofs" = y -o "$doasync" = y -o "$dolang" = y -o "$dox25" = y ]
	then
		act=0   # State 0 is for cleanup.
		passno=1
		while [ "$act" -le "$i" ]
		do
       			if [ "${state_action[$act]}" -ne 0 ]
       	 		then
       	         		state_func${state_action[$act]} 
       	 		fi
       	 	((act += 1))
		done
		echo "Checking for enough free space in filesystem...\c"
# adding an extra 20000 bytes into total number of bytes needed in order to
# keep filesystem from being 100 percent full on systems tight on space
		total_bytes=`expr $total_bytes + 20000`
		ckspace 
	fi
else
	if [ "$doall" = y -o "$dogen" = y -o "$dotcp" = y -o "$donfs" = y -o "$dokern" = y -o "$doprt" = y -o "$dodump" = y -o "$dosna" = y -o "$dofs" = y -o "$doasync" = y -o "$dolang" = y  -o "$dox25" = y ]
	then

		echo "\nSpace checking has been suppressed... continuing"
		passno=2
	else
		usage
		exit 9
	fi
fi

# If any gathering flags then initialize the destination directory 
if [ "$doall" = y -o "$dogen" = y -o "$dotcp" = y -o "$donfs" = y -o "$dokern" = y -o "$doprt" = y -o "$dodump" = y -o "$dosna" = y -o "$dofs" = y -o "$doasync" = y -o "$dolang" = y  -o "$dox25" = y ]
then
	echo 
       	initial_setup
fi

#
# Rebuild state array of functions.
#
for i in $state
do
        state_action[$i]=$i
done

#
# Now proceed to call the associated functions for real.
# This is pass 2 on state functions.
#
act=0   # State 0 is for cleanup.
passno=2
while [ "$act" -le "$i" ]
do
        if [ "${state_action[$act]}" -ne 0 ]
        then
                state_func${state_action[$act]}
        fi
        ((act += 1))
done

exit 0
# End of snap
