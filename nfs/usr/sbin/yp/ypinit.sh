#!/bin/bsh
# @(#)13	1.23  src/nfs/usr/sbin/yp/ypinit.sh, cmdnfs, nfs411, GOLD410 6/9/94 08:57:55
# 
# COMPONENT_NAME: (CMDNFS) Network File System Commands
# 
# FUNCTIONS: 
#
# ORIGINS: 24 27 
#
# (C) COPYRIGHT International Business Machines Corp. 1988, 1989, 1990
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# Copyright (c) 1988 by Sun Microsystems, Inc.
#
#
#  (#)ypinit.sh	1.2 88/06/02 4.0NFSSRC SMI
#
#
# set -xv
maps="bootparams ethers.byaddr ethers.byname group.bygid \
group.byname hosts.byaddr hosts.byname mail.aliases netgroup \
netgroup.byuser netgroup.byhost networks.byaddr networks.byname \
passwd.byname passwd.byuid protocols.byname protocols.bynumber \
rpc.bynumber services.byname ypservers"

yproot_dir=/var/yp
yproot_exe=/usr/sbin
yproot_exe2=/usr/lib/netsvc/yp
hf=/tmp/ypinit.hostlist.$$
XFR=${YPXFR-ypxfr}

masterp=F
slavep=F
host=""
def_dom=""
master=""
got_host_list=F
#exit_on_error=F
errors_in_setup=F
quiet_mode=F

PATH=/bin:/usr/bin:$yproot_exe:$yproot_exe2:$PATH
export PATH 

USAGE="usage: 
\typinit -m\n
\typinit -s master_server\n
\typinit -q [-o] [-n] {-m [slave1 [slave2]...] | -s master_server}\n"

set -- `getopt qms:on $* 2>/dev/null`
if [ $? != 0 ] ; then
	dspmsg -s 46 cmdnfs.cat 1 "$USAGE"
	exit 1
fi

while [ $1 != -- ]
do
	case "$1" in
		"-s")		# Setting up a slave server
		  if [ "${masterp}" = "T" ] ; then dspmsg -s 46 cmdnfs.cat 1 "$USAGE"; exit 1 ; fi
		  slavep=T
		  if [ -z "$2" ] ; then dspmsg -s 46 cmdnfs.cat 1 "$USAGE" ; exit 1 ; fi
		  master=$2
		  shift ; shift
		  ;;
		"-m")
		  if [ "${slavep}" = "T" ] ; then dspmsg -s 46 cmdnfs.cat 1 "$USAGE"; exit 1 ; fi
		  masterp=T
		  shift
		  ;;
		"-o")
		  over_write=yes
		  shift
		  ;;
		"-n")
		  exit_on_error=F
		  shift
		  ;;
		"-q")
		  quiet_mode="T"
		  shift
		  ;;
		*)
		  dspmsg -s 46 cmdnfs.cat 1 "$USAGE" ; exit 1
		  shift
		  ;;
	esac
done

# Set this variable if the user is desiring to run this command in quiet mode
# We assume that they have supplied all of the needed information with the
# command line options.
if [ $quiet_mode = T ]
then
    setup="yes"
else
    setup="no"
fi

#Choose a default for overwritting the maps.
if [ -z "${over_write}" ] ; then
	over_write=no
fi

if [ "$setup" != "yes" ] ; then
	if [ -z "${exit_on_error}" ] ; then
		exit_on_error=T
	fi
else
	exit_on_error=F
fi

# get rid of the --
shift
# Check to make sure that the user specified a master or slave server
# If it is a slave, check to see if there was a master server specified.
# Already checked to make sure that both were not specified.
if [ "${slavep}" = F -a "${masterp}" = F ] ; then dspmsg -s 46 cmdnfs.cat 1 "$USAGE"; exit 1 ; fi
if [ "${slavep}" = T -a -n "$1" ] ; then dspmsg -s 46 cmdnfs.cat 1 "$USAGE"; exit 1 ; fi



if [ $slavep = T ]
then
	maps=`ypwhich -m | egrep -i $master | awk '{ printf("%s ",$1) }' -`
	if [ -z "$maps" ]
	then
		dspmsg -s 46 cmdnfs.cat 2 "Can't enumerate maps from $master. Please check that it is running.\n" $master
		exit 1
	fi
fi

host=`hostname`

if [ $? -ne 0 ]
then 
	dspmsg -s 46 cmdnfs.cat 3 "Can't get local host's name.  Please check your path.\n"
	exit 1
fi

if [ -z "$host" ]
then
	dspmsg -s 46 cmdnfs.cat 4 "The local host's name hasn't been set.  Please set it.\n"
	exit 1
fi

def_dom=`domainname`

if [ $? -ne 0 ]
then 
	dspmsg -s 46 cmdnfs.cat 5 "Can't get local host's domain name.  Please check your path.\n"
	exit 1
fi

if [ -z "$def_dom" ]
then
	dspmsg -s 46 cmdnfs.cat 6 "The local host's domain name hasn't been set.  Please set it.\n"
	exit 1
fi

domainname $def_dom

if [ $? -ne 0 ]
then 
	dspmsg -s 46 cmdnfs.cat 7 "\
You have to be the superuser to run this.  Please log in as root.\n"
	exit 1
fi

if [ ! -d $yproot_dir -o -f $yproot_dir ]
then
    dspmsg -s 46 cmdnfs.cat 8 "\
The directory $yproot_dir doesn't exist.  Restore it from the distribution.\n" $yproot_dir
	exit 1
fi

if [ $slavep = T ]
then
	if [ $host = $master ]
	then
		dspmsg -s 46 cmdnfs.cat 9 "\
The host specified should be a running master NIS server, not this machine.\n"
		exit 1
	fi
fi

if [ "$setup" != "yes" ]; then
	dspmsg -s 46 cmdnfs.cat 10 "Installing the NIS data base will require that you answer a few questions.\n"
	dspmsg -s 46 cmdnfs.cat 11 "Questions will all be asked at the beginning of the procedure.\n"
	echo ""
	dspmsg -s 46 cmdnfs.cat 12 "Do you want this procedure to quit on non-fatal errors? [y/n: n]   "
	read doexit
else
	doexit=yes
fi

case $doexit in
y*)	exit_on_error=T;;
Y*)	exit_on_error=T;;
*)	dspmsg -s 46 cmdnfs.cat 13 "\
OK, please remember to go back and redo manually\nwhatever fails.  If you don't, some part of the system\n(perhaps the NIS itself) won't work.\n";;
esac

echo ""

for dir in $yproot_dir/$def_dom
do

	if [ -d $dir ]; then
		if [ "$setup" != "yes" ]; then
			dspmsg -s 46 cmdnfs.cat 14 "Can we destroy the existing $dir and its contents? [y/n: n]   " $dir
			read kill_old_dir
		#
		# don't remove old directory if we can't overwrite
		#
		elif [ "${over_write}" = "no" ]
		then
			kill_old_dir=no
		else 
			kill_old_dir=yes
		fi

		case $kill_old_dir in
		y*)	rm -rf $dir

			if [ $?  -ne 0 ]
			then
			dspmsg -s 46 cmdnfs.cat 15 "Can't clean up old directory $dir.\nFatal error.\n" $dir
				exit 1
			fi;;

		Y*)	rm -rf $dir

			if [ $?  -ne 0 ]
			then
			dspmsg -s 46 cmdnfs.cat 15 "Can't clean up old directory $dir.\nFatal error.\n" $dir
				exit 1
			fi;;

		*) if [ "$setup" != "yes" ]; then
			echo "OK, please clean it up by hand and start again.  Bye"
			exit 0
                   else
			#
			# in quite mode so can't ask question but they gave
			# option to not remove maps (kill_old_dir != y*),
			# therefore error
			#
			dspmsg -s 46 cmdnfs.cat 15 "Can't clean up old directory $dir.\nFatal error.\n" $dir
			echo "OK, please clean it up by hand and start again.  B
ye"
				exit 1
		   fi;;
		esac
	fi

	#
	# only make it if it's not there (since we
	# could've skipped the rm -rf above)
	#
	if [ ! -d $dir ]; then
		mkdir $dir

		if [ $?  -ne 0 ]
		then
			dspmsg -s 46 cmdnfs.cat 17 "Can't make new directory $dir.\nFatal error.\n" $dir
			exit 1
		fi
	fi

done

if [ $slavep = T ]
then
	if [ "$setup" != "yes" ] ; then
		dspmsg -s 46 cmdnfs.cat 18 "\
	There will be no further questions.\nThe remainder of the procedure should take a few minutes,\nto copy the data bases from $master.\n" $master
	fi

	for dom in  $def_dom
	do
		for map in $maps
		do
			dspmsg -s 46 cmdnfs.cat 19 "Transferring $map...\n" $map
			#
			# don't transfer if we didn't remove it
			#
			if [ "${over_write}" = "yes" -o \
			     ! -f $yproot_dir/$def_dom/$map.dir -o \
			     ! -f $yproot_dir/$def_dom/$map.pag ]; then
				$XFR -h $master -c -d $dom $map

				if [ $?  -ne 0 ]
				then
					errors_in_setup=T

					if [ $exit_on_error = T ]
					then
						exit 1
					fi
				fi
			fi
		done
	done

	echo ""

	if [ $errors_in_setup = T ]
	then
		dspmsg -s 46 cmdnfs.cat 20 "${host}'s NIS data base has been\nset up with errors.  Please remember to figure out what went\nwrong, and fix it.\n" ${host}
	else
		dspmsg -s 46 cmdnfs.cat 21 "${host}'s NIS data base has been\nset up without any errors.\n" ${host}
	fi

	echo ""
	dspmsg -s 46 cmdnfs.cat 22 "\
At this point, make sure that /etc/passwd, /etc/hosts,\n\t/etc/networks, /etc/group, /etc/protocols, /etc/services/,\n\t/etc/rpc and /etc/netgroup have been edited so that when\n\tNIS is activated, the data bases you have just created will be\n\tused, instead of the /etc ASCII files.\n"

	exit 0
else

	rm -f $yproot_dir/*.time


	while [ $got_host_list = F ]; do
		echo $host >$hf
		if [ "$setup" != "yes" ]; then
			echo ""
			dspmsg -s 46 cmdnfs.cat 23 "\
At this point, we have to construct a list of the\nhosts which will run NIS servers.  $host is in the list of NIS\nserver hosts.  Please continue to add the names for the other\nhosts, one per line.  When you are done with the list, type a <control D>.\n" $host
			dspmsg -s 46 cmdnfs.cat 24 "next host to add:  $host\n" $host
			dspmsg -s 46 cmdnfs.cat 25 "next host to add:  "

			while read h
			do
				dspmsg -s 46 cmdnfs.cat 25 "next host to add:  "
				echo $h >>$hf
			done

			echo ""
			dspmsg -s 46 cmdnfs.cat 26 "The current list of NIS servers looks like this:\n"
			echo ""

			cat $hf
			echo ""
			dspmsg -s 46 cmdnfs.cat 27 "Is this correct?  [y/n: y]    "
			read hlist_ok

			case $hlist_ok in
			n*)	got_host_list=F
				dspmsg -s 46 cmdnfs.cat 28 "Let's try the whole thing again...\n";;
			N*)	got_host_list=F
				dspmsg -s 46 cmdnfs.cat 28 "Let's try the whole thing again...\n";;
			*)	got_host_list=T;;
			esac
		else 
			# Build the host list for the master server
			while [ -n "$1" ]
			do
				echo $1 >> $hf
				shift
			done
			got_host_list=T
		fi
	done

	if [ "$setup" != "yes" ] ; then
		dspmsg -s 46 cmdnfs.cat 29 "\
There will be no further questions. The remainder\nof the procedure should take 5 to 10 minutes.\n"
	fi

	dspmsg -s 46 cmdnfs.cat 30 "Building $yproot_dir/$def_dom/ypservers...\n" $yproot_dir $def_dom
	cat $hf | awk '{print $0, $0}' \
	    | /usr/sbin/makedbm - $yproot_dir/$def_dom/ypservers

	if [ $?  -ne 0 ]
	then
		dspmsg -s 46 cmdnfs.cat 31 "\
Couldn't build NIS data base\n$yproot_dir/ypservers.\n" $yproot_dir
		errors_in_setup=T

		if [ $exit_on_error = T ]
		then
			exit 1
		fi
	fi

	rm $hf

	in_pwd=`pwd`
	cd $yproot_dir
	dspmsg -s 46 cmdnfs.cat 32 "Running $yproot_dir/Makefile...\n" $yproot_dir
	make NOPUSH=1 

	if [ $?  -ne 0 ]
	then
		dspmsg -s 46 cmdnfs.cat 33 "\
Error running Makefile.\n"
		errors_in_setup=T
		
		if [ $exit_on_error = T ]
		then
			exit 1
		fi
	fi

	cd $in_pwd
	echo ""

	if [ $errors_in_setup = T ]
	then
	dspmsg -s 46 cmdnfs.cat 34 "\
$host has been set up as a NIS master\n\tserver with errors.\n\tPlease remember to figure out what went wrong, and fix it.\n" $host
	else
	        dspmsg -s 46 cmdnfs.cat 36 "\
$host has been set up as a NIS master\n\tserver without any errors.\n" $host
	fi

	echo ""
	dspmsg -s 46 cmdnfs.cat 35 "\
If there are running slave NIS servers, run yppush\nnow for any data bases which have been changed.\nIf there are no running slaves, run ypinit on those\nhosts which are to be slave servers.\n"

fi
