#!/bin/ksh
# @(#)82	1.18  src/packages/bos/rte/usr/bos.rte.post_i.sh, pkgbos, pkg411, 9439C411b 9/30/94 14:49:42
#
#   COMPONENT_NAME: pkgbos
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

# Note:  bos.rte.post_i is only called during the migration path of BOS install.

# /dev/hd4 is mounted over /
# /dev/hd2 is mounted over /usr
# /dev/hd9var is mounted over /var
# /dev/<tmplv> is mounted over /tmp
# /dev/hd1 is mounted over /home
# and any other file systems listed in bosinst.data

if [ -n "$SETX" -a "$SETX" -eq 1 ]
then
	set -x
fi

# Perform special processing for 3.2->4.1 migration
# It is best to do any removal before the migration scripts get called
# in order to reduce space requirements

# Check to see if we had any existing directories in conflict with bos.rte
# Directories moved to a new location
if [ -s /tmp/bos/dirs.moved ]
then
	dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 76 \
	  "The following directories conflicted with BOS installation files and\n\
had to be renamed:\n"
	cat /tmp/bos/dirs.moved | sed 's:^/mnt::' | sed 's: /mnt: :' |
	while read oldloc newloc
	do
		echo "  $oldloc -> $newloc"
	done
fi

# Directories totally removed because they couldn't be moved
if [ -s /tmp/bos/dirs.removed ]
then
	echo
	dspmsg /usr/lib/nls/msg/$LANG/BosMenus.cat -s 10 77 \
	  "The following directories conflicted with BOS installation files and\n\
had to be removed:\n"
	cat /tmp/bos/dirs.removed | sed 's:^/mnt::' |
	while read oldloc
	do
		echo "  $oldloc"
	done
fi

if [ $BOSLEVEL = "3.2" ]
then
    # Remove usr/root PTF data for the AIX 3.2 filesets.
    for odmdir in /tmp/bos/usr/lib/objrepos \
		  /tmp/bos/etc/objrepos \
		  /tmp/bos/usr/share/lib/objrepos
    do 
    {
       ODMDIR=$odmdir
       for subprod in bos.\* bosext1\* bosext2\* bosnet\* bsl\* bsm\* bosadt\* \
	   X11rte\* X11dev\* X11fnt\* bosnet\* txtfmt\* pci.obj
       do
           odmdelete -o product -q"ptf!='' and lpp_name like $subprod" >/dev/null 2>&1
	   odmget -q"name like $subprod" lpp 2>/dev/null | grep lpp_id | awk '{print $3}'|
	   while read obs_lpp
	   do
	     odmdelete -o history -q"lpp_id=$obs_lpp and ptf !=''" >/dev/null 2>&1
	   done
       done
    }
    done

    # If 3.2->4.1 migration, remove any "prime" message catalogs which might
    # be left hanging around.  They should really be removed when the 
    # base messages are deinstalled in bi_main following the restoration of
    # the bos.rte image.  They might not be removed if the deinstallation 
    # failed (possibly due to some requisite) or if the unpost_i script was
    # not available to be run (possibly due to ptfdir_clean).
    # The prime messages need to be removed because they are obsolete...

    baseprimemsgs=`ls /usr/lpp/bsm*/PRIMESET 2>/dev/null | head -1`
    if [ -n "$baseprimemsgs" ]
    then
       cat $baseprimemsgs | xargs -i rm -f {}
    fi

    # Now we can remove the package directories for the base messages
    grep bsm.*\.msg /tmp/bos/filesets.gone |
    while read fileset description
    do
       basemsgdir=`basename $fileset .msg`
       rm -rf /usr/lpp/$basemsgdir
    done

    # Remove the software vital product data information for those
    # filesets which had files removed from the system during the
    # pre-installation script.
    for odmdir in /tmp/bos/usr/lib/objrepos /tmp/bos/etc/objrepos
    do
    {
    ODMDIR=$odmdir
	 while read fileset description
	 do
	    odmget -q "name = $fileset and state != 1" lpp 2>/dev/null |
	    grep "lpp_id =" | awk '{print $3}' |
	    while read lpp_id
	    do
	       odmdelete -o history -q "lpp_id=$lpp_id" >/dev/null 2>&1
	       odmdelete -o lpp -q "lpp_id=$lpp_id" >/dev/null 2>&1
	       odmdelete -o inventory -q "lpp_id=$lpp_id" >/dev/null 2>&1
	       odmdelete -o product -q "lpp_name=$fileset" >/dev/null 2>&1
	    done
	 done < /tmp/bos/filesets.gone
    }
    done

    # If the size of the / filesystem is 4MB, then we are in great
    # danger of running out of inodes in / during the installation.
    # Increase / by one partition to increase the number of available inodes.
    # This code is unnecessary during 4.1->4.1 migration since the inode
    # ratio is higher on 4.1
    ROOT_SIZE=`df /dev/hd4 | tail +2 | awk '{print $2}'`
    if [ $ROOT_SIZE -eq 8192 ]
    then
       chfs -a size=+1 /dev/hd4 >/dev/null 2>&1
    fi

fi

# Process the bos.rte.cfgfiles file in order to do the merge of config files.

export RC=0

cat /tmp/bos/bos.rte.cfgfiles | \
    while read FILE_NAME KEYWORD MERGE_METHOD
    do
	# If this line begins with a comment character, continue.
	case "$FILE_NAME" in
	\# )	continue ;;
	esac

	# If the merge method is not null, run it.  Otherwise, do the default
	# processing for each keyword.
	if [ -n "$MERGE_METHOD" ]
	then
		$MERGE_METHOD || RC="$?"
		# Return code of two is fatal.  Break and exit.
		if [ "$RC" -eq 2 ]
		then
			break
		fi
	fi

	# Check to ensure that the FILE_NAME is not null before processing.
	# and check to make sure that the file exists
	if [ -n "$FILE_NAME" -a -r "/tmp/bos$FILE_NAME" ]
	then
	    case "$KEYWORD" in
		preserve | auto_merge )
			# The default action for preserve and auto_merge is to
			# move the previous version of the file to its place
			# in the BOS file system.
			mv /tmp/bos$FILE_NAME $FILE_NAME || RC=1
		;;
		v4preserve )
			# The default action for v4preserve is to
			# move the previous version of the file to its place
			# in the BOS file system if this is not a 3.2->4.1
			# migration; if it is a 3.2->4.1 migration, just leave
			# it saved off 
			if [ "$BOSLEVEL" != "3.2" ]
			then
			    mv /tmp/bos$FILE_NAME $FILE_NAME || RC=1
			fi
		;;
		hold_new )
			# The default action for hold_new is to swap the
			# new version of the file with the old version.
			if mv $FILE_NAME /tmp$FILE_NAME
			then if mv /tmp/bos$FILE_NAME $FILE_NAME
				then if mv /tmp$FILE_NAME /tmp/bos$FILE_NAME
					then :
					else RC=1
					fi
				else RC=1
				fi
			else RC=1
			fi
		;;
		user_merge | safe_merge | other )
			# The default action for user_merge, safe_merge, and
			# other  is to leave the file in place.
			:
		;;
	    esac
	fi
    done

    # Remove obsolete SRC entries
    if [ "$BOSLEVEL" = "3.2" ]
    then
       for i in biod gated inetd iptrace keyserv llbd named nfsd nrglbd \
		portmap routed rpc.lockd rpc.mountd rps.statd rwhod \
		sendmail snmpd syslogd timed uxserverd uxserverd12 \
		ypbind yppasswdd ypserv ypupdated 

	do
	   /usr/bin/rmssys -s $i >/dev/null 2>&1
	done

	# Fix up the paths to the three default SRC entries
	for i in qdaemon writesrv lpd
	do
	   cat << !! >/../tmp/SRC.change
SRCsubsys:
	path = "/usr/sbin/$i"
!!
	   ODMDIR=/etc/objrepos odmchange -q "subsysname = $i" -o SRCsubsys /../tmp/SRC.change >/dev/null 2>&1 
	done
	rm -f /../tmp/SRC.change
    fi

    # If it exists, call the migration script for install_assist
    if [ -x /usr/lib/assist/assist_migrate -a "$BOSLEVEL" = "3.2" ]
    then
	/usr/lib/assist/assist_migrate >/dev/null 2>&1
    fi

    # Clean out empty directories that got created by preserved files
    LANG=C find /tmp/bos -type d -print | sort -r | xargs -i rmdir {} 2>/dev/null

    # If the maximum number of logical partitions in /usr
    # is less than 512, then increase the maximum to 512
    # If this fails, it's probably okay - it's just a safety measure in case
    # we need to expand /usr past 512KB.
    USRLVID=`getlvodm -l hd2`
    USRMAXLPS=`lquerylv -L $USRLVID -n 2>/dev/null`
    if [ -n "$USRMAXLPS" ] && [ $USRMAXLPS -lt 512 ]
    then
       lchangelv -l $USRLVID -s 512
    fi

    # Remove the list of files used to delete from the 3.2 system
    rm -f /tmp/bos/bos.rte.*.rmlist >/dev/null 2>&1

    # Figure out what needs to be reinstalled at the latest level
    if [ "$BOSLEVEL" != "3.2" ]
    then
         case "$BOOTTYPE" in
		$CDROM | $NETWORK )
			/usr/sbin/installp -qLd /SPOT/usr/sys/inst.images |
			    cut -d: -f2,3 | sed 's/:/ /' > /../tmp/toc
			;;
		$TAPE )
			tctl -f /dev/$BOOTDEV.1 rewind
			/usr/sbin/installp -qLd /dev/$BOOTDEV.1 |
			    cut -d: -f2,3 | sed 's/:/ /' > /../tmp/toc
			tctl -f /dev/$BOOTDEV.1 rewind
			;;
         esac
	 # Loop through the installed options to determine which install options
	 # have newer versions on the installation media
	 while read installedfs installedlevel
	 do
		 grep $installedfs' ' /../tmp/toc |
		 while read tocfs toclevel
		 do
			 if [[ $installedlevel < $toclevel ]]
			 then
				echo $tocfs
			 fi
		 done
	 done < /../tmp/installed.list | sort -u > /../tmp/mig.pkgs
	 rm -f /../tmp/installed.list /../tmp/toc 2>/dev/null
    fi

exit $RC
