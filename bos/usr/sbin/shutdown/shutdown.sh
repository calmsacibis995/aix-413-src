#!/usr/bin/ksh
# @(#)39        1.37.2.35  src/bos/usr/sbin/shutdown/shutdown.sh, cmdoper, bos41J, 9515A_all 4/7/95 18:39:33
#
# COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
#
# FUNCTIONS: 
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1985, 1995
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

####################################################################
#                                                                   
# SHUTDOWN
#	Possible cases:
#
# 	halt 		- shuts down all processes
# 	maint 		- shuts down to single-user (maintenance) mode
# 	reboot		- reboots system
#	multi		- brings system down from distribute mode to multi mode
# 	interactive 	- asks for confirmation
#       nohalt		- pretend to halt but do not actually halt
#
####################################################################
trap "exec > /dev/null ; exec 2> /dev/null" 13


####################################################################
# NAME: usage
# FUNCTION: display the usage statement
# ARGUMENTS: none
# RETURN VALUE: none
####################################################################
usage()
{
        dspmsg -s 1 shutdown.cat 58 'Usage: shutdown [ -c][ -d][ -F][ -i][ -k]\
[ -m][ -r][ -hv | -t mmddHHMM[yy]][ + Time][ Message]\n'
}

####################################################################
# NAME: illegal
# FUNCTION: display the usage statement
# ARGUMENTS: none
# RETURN VALUE: none
####################################################################
illegal()
{
        dspmsg -s 1 shutdown.cat 53 'shutdown -t mmddHHMM[yy] is not valid with\
 the -v or -h options\n'
}


####################################################################
# NAME: bad_date
# FUNCTION: display the usage statement
# ARGUMENTS: none
# RETURN VALUE: none
####################################################################
bad_date()
{
        dspmsg -s 1 shutdown.cat 55 'Input date (mmddHHMM[yy])\
 must include valid numbers.\n mm(1-12), dd(1-31), HH(0-23), MM(0-59),\
 yy([19]70-99 or [20]00-38)\n'
}


# NAME: lowerf
# FUNCTION: display usage statement and inform the user of the change to c or F
lowerf()
{
	usage
	dspmsg -s 1 shutdown.cat 2 'Please choose either -c (same as -f BSD) \
or -F (same as -f old AIX)\n'
	exit 1;
}

# NAME: synced
# FUNCTION: sync all of the files systems but if hung up then continue
synced()
{
	sync&
	sync&
	sync&
	a=`date +%d:%H:%M:%S | sed 's/://g'`
	a=`expr $a + 20`
	ps | grep $! | grep sync > /dev/null 2>&1
	while [ $? -eq 0 ]
	do
		b=`date +%d:%H:%M:%S | sed 's/://g'`
		if [ $a -lt $b ]; then
			break
		fi
		ps | grep $! | grep sync > /dev/null 2>&1
	done
}
# NAME: tabmnt
# FUNCTION: collect the mount information and force every field to be separated
#    by a tab, so that awk can look at the different fields.
tabmnt()
{
	mount 2>/dev/null |awk '{ line[i] = "-"$0; i++; }
		END { while ( i >= 4 ) { i--; print line[i]; } }' - >/tmp/mount.a
	tab /tmp/mount.a
# remove extra tabs and blanks
	sed "/ /s//	/g" /tmp/mount.a \
	|sed "/			/s//	/g" \
	|sed "/			/s//	/g" \
	|sed "/		/s//	/g" >/tmp/mount.t

	rm -f /tmp/mount.a 2>/dev/null
}

# NAME: dismntall
# FUNCTION: unmount all mounted filesystems in reverse order to insure that all
#  filesystems are unmounted
dismntall()
{
	if [ -x /usr/bin/tab ]
	then
		tabmnt
# get list of directories or files that need unmounted
                awk '{ print $3 }' /tmp/mount.t | \
                grep -v "^/tmp$" >/mount.a

		rm -f /tmp/mount.t /tmp/mount.a 2>/dev/null
		cat /mount.a |while read POINT 
		do 
			if [ $POINT != /usr -a  $POINT != / ]
			then		
				#do not umount /var if state is maint.
				if [ $POINT != /var -o $state != maint ]
				then
					umount $POINT 
				fi
			fi
		done
		rm -f /mount.a
                umount /tmp > /dev/null 2>&1
	fi
}

# NAME: dismntrmt
# FUNCTION: unmount the remote filesystems
#  first compile a list of all the remote filesystems and all filesystems
#    that were mounted after any remote filesystem.
#  second determine which filesystems are mounted over the remote filesystems.
#  then unmount all remote filesystems and all filesystems mounted over them.
dismntrmt()
{
        REMOTEFS=0
        rm -f /tmp/remote
	if [ -x /usr/bin/tab ]
	then
	  tabmnt
	  sed '/^#/D' /etc/vfs |sed '/^%/D'|sed -n '/remote$/p' >/tmp/vfs.t
	  rm -f /tmp/vfs
	  cat /tmp/vfs.t|while read VFS
	  do
		set $VFS
		vfs=$1
		echo $VFS >>/tmp/vfs
	  done
	  awk 'BEGIN { i=0; j=0; v=0 }
		{ if ( FILENAME == "/tmp/vfs" ) {
			v++; vfs[v] = $1; } 
		  else { 
			i++; line[i] = $3; type[i] = $4;
			for ( m=1; m <= v; m++ )
			  if ( $4 == vfs[m] ) { j=i; } } }
		END { k=1;
		  while ( j > 0 ) {
		    yes=0;
		    for ( m=1; m <= v; m++ )
			if ( type[k] == vfs[m] ) {
			  print line[k]; yes=1; }
		    if ( yes != 1 )
				  for ( n=k+1; n <= i; n++ )
			    if ( line[k] == line[n] )
			      for ( m=1; m <= v; m++ )
				if ( type[n] == vfs[m] )
				  print line[k];
		    j--; k++
		  } }' /tmp/vfs /tmp/mount.t >/tmp/mount.a
	  cat /tmp/mount.a |while read POINT 
	  do 
		if [ $POINT != /usr -a  $POINT != / ]
		then		
			umount $POINT  2>/dev/null
		else
			touch /tmp/remote
		fi
	  done
	  rm -f /tmp/vfs.t /tmp/vfs /tmp/mount.[ta] 2>/dev/null
	fi
}

# NAME: converttime
# FUNCTION: convert time (hh:mm) into seconds
#
converttime()
{
	if [ $time != 0 ]
	then
		TIME=$time
		time=`echo $time |sed 's/:/ /'`
		set $time
		hour=$1
		min=$2
		pretm=`date +%H:%M |sed 's/:/ /'`
		set $pretm
		prehour=$1
		premin=$2	
		if [ $prehour -gt 24 ]
		then
			usage
			dspmsg -s 1 shutdown.cat 3 'Time must be based on a \
24 hour clock\n'
			exit 1	
		fi
		if [ $prehour -gt $hour ]
		then
			hour=`expr $hour + 24`
			tmmsg=1
		else
			tmmsg=2
		fi
		if [ $min -lt $premin ]
		then
			hour=`expr $hour - 1`
			min=`expr $min + 60`
		fi
		HOUR=`expr $hour - $prehour`
		MINUTE=`expr $min - $premin`
		grace=`expr \( $HOUR \* 3600 \) + \( $MINUTE \* 60 \)`
	fi
}

################################################################
# NAME: checkdate
# FUNCTION: check date (mmddHHMM[yy])
# ARGUMENTS:  input date
# RETURN:  only if date is valid
################################################################
checkdate()
{

# Initialize NUM
NUM=0

#
# Set the current date
#
ldate=`date +"%D"`
ldate1=`date +"%H:%M"`
ccdate=`date +%m:%d:%H:%M | sed 's/://g`
cydate=`date +%y:%m:%d:%H:%M | sed 's/://g`

# Check for 8 or 10 digits and
# compare the date to the current date and ensure it's greater.
#

if [[ $itime = [0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9] ]]
then
        NUM=8
        if [ $ccdate -gt $itime ]
        then
                dspmsg -s 1 shutdown.cat 54 "Input date (mmddHHMM[yy]) must\
 be after %s %s\n" $ldate $ldate1
                usage
                exit 1
        fi
elif [[ $itime = [0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9] ]]
then
        NUM=10
# set up date in this format yymmddHHMM
        t=${itime##[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]}
        t2=${itime%%[0-9][0-9]}
        itime1=`echo $t$t2`
	c2=`date +%y`

        if [ $cydate -gt $itime1 ]
        then
      # check for the change between a 1900 year and a 2000 year
           if [[ $t -ge 70 && $t -le 99 ]]
           then
                inyr=19
           else
                inyr=20
           fi
           if [[ $c2 -ge 70 && $c2 -le 99 ]]
           then
                curyr=19
           else
                curyr=20
           fi
           if [ $inyr -le $curyr ]
           then
                dspmsg -s 1 shutdown.cat 54 "Input date (mmddHHMM[yy]) must\
 be after %s %s\n" $ldate $ldate1
                usage
                exit 1
           fi
        fi
else
        usage
        exit 1
fi

#
# Validate the following:
#       mm is between 1 and 12
#       dd is between 1 and 31
#       HH is between 0 and 23
#       MM is between 0 and 59
#       yy is between 0 and 99 (no check since it's 2 digits)
#

val=1
while [ $val -lt $NUM ]
do
        val1=`expr $val + 1`
        num=`echo $itime | cut -c$val-$val1`
        case $val in
                1 )  if [ $num -lt 1 -o $num -gt 12 ]
                     then
                        bad_date
                        usage
                        exit 1
                     fi ;;
                3 )  if [ $num -lt 1 -o $num -gt 31 ]
                     then
                        bad_date
                        usage
                        exit 1
                     fi ;;
                5 )  if [ $num -lt 0 -o $num -gt 23 ]
                     then
                        bad_date
                        usage
                        exit 1
                     fi ;;
                7 )  if [ $num -lt 0 -o $num -gt 59 ]
                     then
                        bad_date
                        usage
                        exit 1
                     fi ;;
                9)   if [ $num -lt 70 -a $num -gt 38 ]
                     then
                        bad_date
                        usage
                        exit 1
                     fi ;;
        esac
        val=`expr $val + 2`
done

}

# NAME: nologin
# FUNCTION: creates a /etc/nologin file
#
nologin()
{
	dspmsg -s 1 shutdown.cat 6 '\n\nSystem maintenance about to begin.\n\
Try again later.\n' >/etc/nologin
}

# NAME: prtmsg
# FUNCTION: sends warning message to all users
prtmsg()
{
  if [ $HOUR = 0 ]
  then
    case $state in
      multi) 
          wall `dspmsg -s 1 shutdown.cat 7 'Remote files will become \
unaccessable in %s minutes.\n' $MINUTE`
        ;;
      *)
          wall `dspmsg -s 1 shutdown.cat 8 "The system will be brought down \
in %s minutes.\n" $MINUTE`
        ;;
    esac
  else
    case $state in
      multi)
           wall `dspmsg -s 1 shutdown.cat 9 "Remote files will become \
unaccessable in %s hours and %s minutes.\n" $HOUR $MINUTE`
         ;;
      *) 
  	 wall `dspmsg -s 1 shutdown.cat 10 "The system will be brought down \
in %s hours and %s minutes.\n" $HOUR $MINUTE`
        ;;
    esac
  fi
}

# NAME: pause
# FUNCTION: sleep for n seconds, wake up to deliver warning messages as
#    apocalypse approaches.
pause()
{
	trap "" 1 3 4 6 8 13 15 17 21 22 30 31
	while [ $HOUR -gt 0 ]
	do
		HOUR=`expr $HOUR - 1`
		grace=`expr $grace - 3600`
		sleep 3600
		prtmsg
	done
	while [ $HOUR = 0 -a $MINUTE -gt 30 ]
	do
		MINUTE=`expr $MINUTE - 10`
		grace=`expr $grace - 600`
		sleep 600
		prtmsg
	done
	while [ $HOUR = 0 -a $MINUTE -lt 30 -a $MINUTE -gt 5 ]
	do
		MINUTE=`expr $MINUTE - 5`
		grace=`expr $grace - 300`
		sleep 300
		prtmsg
	done
	if [ $grace -lt 0 ]
	then
		grace=0
	fi
	sleep $grace
}


# NAME: getmsg
# FUNCTION: concatenate the shutdown message
getmsg()
{
	while [ -n "$1" ]
	do
		validflag=1
		case $1 in
			-* | now | NOW | +*)
				;;
			[0-9]*)
				echo $1 | grep ":" > /dev/null
				if [ $? -ne 0 ]
				then
					validflag=0
				fi;;
			*)
				validflag=0;;
		esac
		if [ $validflag -eq 1 ]
		then
			shift
		else
			while [ -n "$1" ]
			do
			    msg=`echo $msg $1`
			    shift
			done
		fi
	done
	echo $msg
}

#FUNCTION: killnonroot
#   kills all non-root processes; exclude the ancestors of 'shutdown'.
killnonroot()
{
	trap "" 1
# Get all the non-root processes from the list
        ps -le | awk 'NR > 1 && $3 != 0 {print $4}' > /tmp.proclist

#get the shutdown process and all its parents off the list
        temp_ppid=$$
        while [ $temp_ppid -gt 1 ]
        do
                sed "s/$temp_ppid$//" /tmp.proclist > /tmp.proclistX
                temp_ppid=`ps -fp $temp_ppid | awk 'NR > 1 { print $3 }' `
                mv /tmp.proclistX /tmp.proclist
        done
        rm -f /tmp.proclistX

# Kill all the processes still on the list.
        for i in `cat /tmp.proclist`
        do
                kill -TERM $i > /dev/null 2>&1
        done
        sleep 30
        for i in `cat /tmp.proclist`
        do
                kill -KILL $i > /dev/null 2>&1
        done
        rm -f /tmp.proclist

}

#FUNCTION: killsome
#   kills all processes except kernel processes,
#   the ones starting 'shutdown', init, srcmstr and 
#   daemons reqd by dwm
killsome()
{
	trap "" 1
# ps -eF gets all prcesses except kernel processes
	ps -eF "%p %a" | awk 'NR > 1 \
			      { if ( $1 != 0 && \
				     $1 != 1  && \
                                     $2 != "/etc/srcmstr" && \
                                     $2 != "/usr/sbin/srcmstr" && \
                                     $2 != "/usr/etc/portmap"  && \
                                     $2 != "/usr/sbin/portmap"  && \
                                     $2 != "/etc/syncd"  && \
                                     $2 != "/usr/sbin/syncd"  && \
                                     $2 != "/usr/etc/rpc.statd"  && \
                                     $2 != "/usr/sbin/rpc.statd"  && \
                                     $2 != "/usr/etc/rpc.lockd"  && \
                                     $2 != "/usr/sbin/rpc.lockd"  && \
                                     $2 != "/usr/etc/biod" && \
                                     $2 != "/usr/sbin/biod" ) \
                         print $1 }' > /tmp.proclist
#get the processes starting shutdown and exclude them
	temp_ppid=$$
	while [ $temp_ppid -gt 1 ]
	do
		sed "s/$temp_ppid$//" /tmp.proclist > /tmp.proclistX
		temp_ppid=`ps -fp $temp_ppid | awk 'NR > 1 { print $3 }' `
		mv /tmp.proclistX /tmp.proclist
		
	done
	rm -f /tmp.proclistX
	for i in `cat /tmp.proclist`
	do
		kill -TERM $i > /dev/null 2>&1
	done
	sleep 30
	for i in `cat /tmp.proclist`
	do
		kill -KILL $i > /dev/null 2>&1
	done
	rm -f /tmp.proclist
}
# main routine

sync; sync; sync		

PATH=/usr/sbin:/bin:/usr/bin:/etc:
trap "init q; rm -f /etc/nologin /tmp/mount.* /tmp/vfs* 2>/dev/null;exit" 0
trap ":" 3 4 6 8 13 15 17 21 22 30 31
trap "contflag=on" 1		# continues when shutdown receives signal 1


state=halt			# state of shutdown
interactive=off			# get confirmation from user
fast=off			# do not send warning messages and shutdown now
filecheck=yes                   # check filesystem during reboot
nohalt=off			# pretend to shutdown system 
grace=-1			# number of seconds until shutdown
time=0				# actual clock time of shutdown
message=no			# message to broadcast to users
tmmsg=0				# message specifying time to shutdown
HOUR=0				# number of hours until shutdown
MINUTE=0			# number of minutes until shutdown
contflag=off			# continues when shutdown receives signal 1
tflag=0                         # shutdown -t can't be used with -v or -h
                                # tflag=0  not used yet; tflag=1 -t input;
                                # tflag=2 -h or -v input;
                                # tflag=3  mmddHHMM[yy] input
itime=0                         # mmddHHMM[yy] input
REMOTEFS=0

#	Process arguments
#
#	Save the last argument (possibly the shutdown message), since after
#	getopt it will be split up into separate words (arguments).  This
#	save will fail if there are more than 9 arguments to shutdown.
#
savmsg=`getmsg $*`
set -- `getopt cdFfhikmrvt: $*`
if [ $? != 0 ]
then
	usage
	exit 1
fi

y=`locale yesstr | cut -f1 -d:`
n=`locale nostr | cut -f1 -d:`

for i
do	
	case $i in
		-c) state=reboot	# reboot system
		    filecheck=no;;	# don't check filesytem during reboot
		-d) state=multi;;	# go to multi mode from distribute mode
		-F) grace=0		# shutdown now
		    HOUR=0
		    MINUTE=0
		    fast=on;;
		-f) lowerf;;		# inform user of new flags
                -h) if [[ $tflag = 1 || $tflag = 3 ]]
                    then                # default shutdown
                        illegal
                        exit 1
                    fi
                    state=halt
                    tflag=2;;
		-i) interactive=on;;	# get confirmation from user
		-k) nohalt=on;;		# pretend to shutdown
		-m) state=maint;;	# go to maintenance mode
		-r) state=reboot;;	# reboot system
                -t) if [ $tflag = 2 ]   # powerup system (at date input)
                    then
                        illegal
                        exit 1
                    else
			dspmsg -s 1 shutdown.cat 59 "WARNING:  The -t option is only supported on systems that have\n\
a power supply which automatically turns power off at shutdown\n\
and an alarm to allow reboot at a later time.  Systems without\n\
this capability may hang or may reboot immediately after shutdown.\n"
			dspmsg -s 1 shutdown.cat 36 "Do you wish to continue? \
[%s or %s]:\n" $y $n

			read ans
			if [ ${ans:-"n"} != $y ]
			then
				exit 0
			fi
                        tflag=1
                        state=reboot
                    fi ;;
                -v) if [[ $tflag = 1 || $tflag = 3 ]]
                    then                # default shutdown
                        illegal
	                exit 1
                    fi
                    state=halt
                    tflag=2;;
		+[0-9]*) 
                   if [ $tflag != 1 ]
                   then
			plus=`/usr/bin/echo $i |sed 's/.//'`
			echo $plus | grep "[^0-9]" > /dev/null
			if [ $? -eq 0 ]
			then
				usage
				exit 1
			fi
			HOUR=`expr $plus / 60`
			if [ $? = 2 ]
			then
				usage
				exit 1
			fi
			MINUTE=`expr $plus % 60`
			if [ $? = 2 ]
			then
				usage
				exit 1
			fi
		        grace=`expr $plus \* 60`
			if [ $? = 2 ]
			then
				usage
				exit 1
			fi
			if [ $plus -eq 0 ]
			then
				tmmsg=4
			else
				tmmsg=3
			fi
		     fi;;
                [0-9][0-9]:[0-9][0-9])
                   if [ $tflag != 1 ]
                   then
                        time=$i
                        converttime
                   fi;;
                [0-9]:[0-9][0-9])
                   if [ $tflag != 1 ]
                   then
                        time=$i
                        converttime
                   fi;;
                [0-9]*)
                  if [ $tflag = 1 ]
                  then
                        itime=$i
                        tflag=3
                  fi;;
                now) grace=0
                    HOUR=0
                    MINUTE=0
                    tmmsg=4
                        ;;
		NOW) grace=0
		    HOUR=0
		    MINUTE=0
	   	    tmmsg=4
			;;
		--) ;;
		-*) usage
		   	exit 1;;
		*) message=$savmsg
		   if [ $grace -lt 0 ]
		   then
                        if [ $tflag = 1 ]
                        then
                                dspmsg -s 1 shutdown.cat 56 "Specify time\
 as mmddHHMM[yy].\n"
                        else
                                dspmsg -s 1 shutdown.cat 11 "Specify time as \
+number or hh:mm.\n"
                        fi
                        usage
                        exit 1
		   fi
		   break;;
	esac
done

if [ $tflag = 1 ]
then
        usage
        exit 1
fi

if [ $itime != 0 ]
then
        checkdate
fi

if [ $grace -lt 0 ]
then
	MINUTE=1
	grace=60
	tmmsg=5
fi
	

dspmsg -s 1 shutdown.cat 15 "\nSHUTDOWN PROGRAM\n"
date

cd /

if [ $state = halt -o $state = reboot  -o $state = maint ]
then
	if [ $interactive = on ]
	then
		case $state in
		  halt)
		    dspmsg -s 1 shutdown.cat 16 "The system will now halt - \
do you want to do this? (%s/%s)\n" $y $n
		    ;;
		  reboot)
		    dspmsg -s 1 shutdown.cat 17 "The system will now reboot - \
do you want to do this? (%s/%s)\n" $y $n
		    ;;
		  maint)
		    dspmsg -s 1 shutdown.cat 18 "The system will now go to \
maintenance mode - do you want to do this? (%s/%s)\n" $y $n
		    ;;
		esac
		read ans
		if [ ${ans:-"n"} != $y ]
		then
			exit 0
		fi
	fi

	if [ $fast = off ]
	then
                msg1=`dspmsg -s 1 shutdown.cat 20 "System maintenance in \
progress."`
 		msg0=""
	    	case $tmmsg in
		      1)
		      msg2=`dspmsg -s 1 shutdown.cat 43 "All processes will \
be killed tomorrow at %s.\n" $TIME`;;
		      2)
		      msg2=`dspmsg -s 1 shutdown.cat 44 "All processes will \
be killed at %s.\n" $TIME`;;
		      3)
		      msg2=`dspmsg -s 1 shutdown.cat 45 "All processes will \
be killed in %s minutes.\n" $plus`;;
		      4)
		      msg0=`dspmsg -s 1 shutdown.cat 19 "PLEASE LOG \
OFF NOW ! ! !"`
		      msg2=`dspmsg -s 1 shutdown.cat 46 "All processes will \
be killed now.\n"`;;
                      5)
                      ;;
		esac
		if [ "${message}" = no ]
		then
			msg3=""
		else
			msg3=$message
		fi

#      Display only if tmmsg <=4;  no message needed for 5
                if [ $tmmsg -le 4 ]
                then
		    wall << ! 
$msg0
$msg1
$msg2 
$msg3
!
                 fi
	fi

#    If not halt now  then subtract 60 seconds from the time out and pause for
#    the timeout period.  Issue final messages, and sleep 60 more seconds, then
#    continue.

	if [ $tmmsg -ne 4 -a $fast = off ]
        then
            grace=`expr $grace - 60`
	    pause
            msg0=`dspmsg -s 1 shutdown.cat 19 "PLEASE LOG OFF \
NOW ! ! !"`
            msg2=`dspmsg -s 1 shutdown.cat 47 "All processes will be \
killed in 1 minute.\n"`
            wall <<!
$msg0
$msg2
!
            sleep 60
        fi
	trap ":" 2 18

	if [ $fast = off ]
	then
		wall `dspmsg -s 1 shutdown.cat 22 "! ! ! SYSTEM \
BEING BROUGHT DOWN NOW ! ! !\n"`
	fi

	if [ $nohalt = off ]
	then

# No one but root will be able to login
		nologin

		case $state in
		  halt)
		    dspmsg -s 1 shutdown.cat 26 "\nWait for \'....Halt \
completed....\' before halting.\n"
		    ;;
		  reboot)
                    dspmsg -s 1 shutdown.cat 57 "Wait for \'Rebooting...\'\
 before stopping.\n"
		    ;;
		  maint)
		    dspmsg -s 1 shutdown.cat 28 "\nWait for \'INIT: \
SINGLE-USER MODE\' before continuing.\n"
		    ;;
		esac

		init N     # tell init to stop respawning
		if [ -x /usr/sbin/nimclient ]
		then
			/usr/sbin/nimclient -S shutdown > /dev/null 2>&1
		fi

		if [ -x /usr/lib/errstop ]; then
			/usr/lib/errstop
			dspmsg -s 1 shutdown.cat 31 "Error logging stopped...\n"
		fi

		# if /etc/netware.clean exists then execute it
		if [ -f /etc/netware.clean ]
		then
			sh /etc/netware.clean
		fi

	# unmount remote filesystems, clean NFS, clean TCP/IP

		# Before trying to dismount anything kill all processes with a
		# uid not equal to 0.  Next, turn off accounting to make
		# /var/adm un-busy.  Finally, dismount filesystems.
		for i in `lssrc -a | awk 'NR > 1  { print $1 }'`
		do
			if [ "`lssrc -S -s $i|awk 'NR >1 {print }'|cut -d: -f5`" != "0" ]
			then
				stopsrc -cs $i > /dev/null 2>&1
			fi
		done

                killnonroot

		if [ -x /usr/lib/acct/shutacct ]
		then
			/usr/lib/acct/shutacct
			dspmsg -s 1 shutdown.cat 29 "Process accounting \
stopped...\n"
		fi

		dismntrmt   # unmount the remote filesytems


		# if /etc/dfs.clean exists then execute it
		if [ -f /etc/dfs.clean ]
		then
			sh /etc/dfs.clean
		fi

		# if /etc/dce.clean exists then execute it
		if [ -f /etc/dce.clean ]
		then
			sh /etc/dce.clean
		fi


		# if /, /usr and /tmp are remote file systems then
		# nfs.clean and tcp.clean can not be executed
		# without the -d option
		if [ -f /tmp/remote ]; then
			REMOTEFS=1
			rm -f /tmp/remote
	        else
			REMOTEFS=0
		fi
		if [ $REMOTEFS -eq 1 ]
		then
			if [ -f /etc/nfs.clean ]
			then
				sh /etc/nfs.clean -d -y
			fi

		else
			if [ -f /etc/nfs.clean ]
			then
				sh /etc/nfs.clean
			fi
		fi

		if [ -f /etc/tcp.clean ]
		then
			sh /etc/tcp.clean
		fi

# Stop all SRC subsystems before killall
#               stopsrc -ca
# Selective stopsrc: stop some active SRC subsystems
		lssrc -a | awk 'NR > 1  { \
				  if ( $1 != "portmap" && \
				       $1 != "rpc.lockd" && \
				       $1 != "rpc.statd" && \
				       $1 != "syncd" && \
				       $1 != "tftp" && \
				       $1 != "bootp" && \
				       $1 != "biod" ) \
				print $1 }' > /tmp.srcsubsf
		for i in  `cat /tmp.srcsubsf`
		do         
			stopsrc -cs $i > /tmp.errmsgf
			if [ $? -eq 0 ]
			then
				cat /tmp.errmsgf
			fi
		done
		rm -f /tmp.srcsubsf
		rm -f /tmp.errmsgf
			
		rm -f /etc/nologin 2>/dev/null
# send all processes SIGTERM signal; 30 seconds later send all processes SIGKILL
		dspmsg -s 1 shutdown.cat 32 "All processes currently running \
will now be killed...\n"
#selective kill
 		killsome
		synced
# Unmount the file systems
		if [ $contflag = on ]; then
		    dspmsg -s 1 shutdown.cat 33 "Unmounting the file \
systems...\n" > /dev/console 2>&1
		else
		    dspmsg -s 1 shutdown.cat 33 "Unmounting the file \
systems...\n"
		fi
		dismntall

		if [ $REMOTEFS -eq 0 ]; then
		     # Bring down all network interfaces
		     netw_if=`lsdev -C -c if | awk '{ print $1 }'`
		     if [ -n "$netw_if" ] ; then 
			# there are network interfaces
			echo "\nBringing down network interfaces:\c" > /dev/console
			for i in $netw_if ; do
				echo " $i\c" >  /dev/console
				/usr/sbin/ifconfig $i detach 2> /dev/null
			done
			echo "\n"
		     fi
		fi

		case $state in
		  halt)
			if [ $contflag = on -o -n "$WINDOWID" ]; then
				halt >/dev/console 2>&1
			else
			#  Check if the shutdown has been started from the
			#  cron. If it has then redirect the halt completed
			#  message to the console.
				( echo "" > /dev/tty ) 2> /dev/null
				if [ $? -eq 0 ]
				then
					halt > /dev/tty 2>&1
				else
					echo "\n" > /dev/console
					halt > /dev/console 2>&1
				fi
			fi;;
		  reboot)

			#Get the current date
			ccdate=`date +%m:%d:%H:%M | sed 's/://g`
			cydate=`date +%y:%m:%d:%H:%M | sed 's/://g`

                        if [[ ( $NUM = 8 && $itime > $ccdate ) || ( $NUM = 10 && $itime1 > $cydate ) ]]; then
                                if [ $contflag = on ]; then
                                        reboot -t $itime >/dev/console 2>&1
                                else
                                        reboot -t $itime
                                fi
                        else
                                if [ $contflag = on ]; then
                                        reboot >/dev/console 2>&1
                                else
                                        reboot
                                fi
                        fi;;
		  maint)
			killall - >/dev/null 2>&1
			synced
			init s
			sleep 10;;
		esac
	fi

	if [ $nohalt = on ]
	then
		dspmsg -s 1 shutdown.cat 42 "\nshutdown -k is finished.\n\
The system is still up.\n"
	fi

	exit 0
fi

if [ $state = multi ]
then
	if [ $interactive = on ]
	then
	  dspmsg -s 1 shutdown.cat 35 "Remote files will become unaccessable.\n"
	  dspmsg -s 1 shutdown.cat 36 "Do you wish to continue? \
[%s or %s]:\n" $y $n
	  read b
	  if [ ${b:="y"} != $y ]
	  then
	  	exit 0
	  fi
	fi

	msg0=`dspmsg -s 1 shutdown.cat 37 "System maintenance about \
to begin."`
	case $tmmsg in
	      1)
	      msg2=`dspmsg -s 1 shutdown.cat 48 "Remote files will \
become unaccessable tomorrow at %s.\n" $TIME`;;
	      2)
	      msg2=`dspmsg -s 1 shutdown.cat 49 "Remote files will \
become unaccessable at %s.\n" $TIME`;;
	      3)
	      msg2=`dspmsg -s 1 shutdown.cat 50 "Remote files will \
become unaccessable in %s minutes.\n" $plus`;;
	      4)
	      msg2=`dspmsg -s 1 shutdown.cat 51 "Remote files will \
become unaccessable now.\n"`;;
	      5)
	      msg2=`dspmsg -s 1 shutdown.cat 52 "Remote files will \
become unaccessable in 1 minute.\n "`;;
    	esac 
	wall << ! 
$msg0
$msg2
!
	pause

	msg0=`dspmsg -s 1 shutdown.cat 37 "System maintenance about \
to begin."`
	msg1=`dspmsg -s 1 shutdown.cat 39 "Remote files will become \
unaccessable now."`
	wall << ! 
$msg0
$msg1
!
	dspmsg -s 1 shutdown.cat 40 "\nUnmounting the remote file systems...\n"
	dismntrmt

	exit 0;
fi
