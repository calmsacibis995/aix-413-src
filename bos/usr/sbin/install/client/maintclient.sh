#!/bin/bsh
# @(#)21        1.17  src/bos/usr/sbin/install/client/maintclient.sh, cmdinstl, bos411, 9428A410j 6/28/93 14:29:32
# COMPONENT_NAME: (CMDINSTL) diskless install
#
#
# FUNCTIONS: maintclient
#
# ORIGINS: 27
#
# (C) COPYRIGHT Internation Business Machines Corp. 1991
# All Rights Reserved
# Licensed Material - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#
# NAME: maintclient
#
# FUNCTION: Set up a replica of a client's filesystem on the local machine
#   and execute a command in that filesystem.
#
# USAGE:
#   maintclient [-C] -R root [-R root] -U usr -S share [-M mnt,mntpt] [-J]
#               command
#
# ARGUMENTS:
#   -C            Do not verify client information
#   -R root       The directory that holds the root part of a client
#   -U usr        The direcotry that holds the usr part of a client
#   -S share      The directory that holds the share part of a client
#   -M mnt,mntpt  Specifies an additional mount to be performed before
#                   executing the specified command
#   -J            Specifies that we were called from SMIT
#   command       The command to be executed
#
# RETURN VALUE DESCRIPTION:
#   0 if everything succeeded
#   1 if something failed
#
# This script will executed the given command for each specified client.
# If an error occurs for a specific client, the error is noted and the
# next client is processed.

#***********************************************************************
# Defines for some commands and directories that are used.
#***********************************************************************
CHROOT=/usr/sbin/chroot
MOUNT=/usr/sbin/mount
UMOUNT=/usr/sbin/umount
FGREP=/usr/bin/fgrep
DF=/usr/bin/df
DIRNAME=/usr/bin/dirname
BASENAME=/usr/bin/basename
DEV=/dev
TMP=/tmp
VAR=/var
MOUNTROOT=/home/maintclients
MOUNTUSR=$MOUNTROOT/usr
MOUNTSHARE=$MOUNTROOT/usr/share
MOUNTDEV=$MOUNTROOT/dev
MOUNTTMP=$MOUNTROOT/tmp
MOUNTVAR=$MOUNTROOT/var
HOSTNAME=`hostname`
HOST=`host $HOSTNAME | awk '{print $1}'`
CLIENTLOCK=/tmp/maintclient_LOCK
CMDNAME=${INU_CMDNAME-maintclient}

#***********************************************************************
# Messages.
#***********************************************************************
MS_INSTLCLIENT=12
CATALOG=cmdinstl_e.cat

MC_USAGE_I=21
MC_HEADER_I=44
MC_INUSE_E=40
MC_MARG_E=23
MC_ONESHARE_E=25
MC_ONESPOT_E=26

DWM_AKA=        # initialize NULL
IS_LOCAL=FALSE    # initialize FALSE

MC_USAGE_D="\
Usage: maintclient -R root [-R root] -U usr -S share \
[-M mnt,mntpt] command\n"
MC_HEADER_D="\
%s\n\
%s%s:  The output from %s with\n\
%s\t%s as the root file system,\n\
%s\t%s as the usr file system, and\n\
%s\t%s as the share file system.\n\
%s\n"
MC_INUSE_D="\
0503-720 %s:  The %s command is currently in use.\n\
\tPlease try again later.  If the %s command is not in use\n\
\tthen delete %s.\n"
MC_MARG_D="\
0503-721 %s:  The argument to -M must be mnt,mntpt where mnt is a\n\
\tdirectory to mount and mntpt is the mount point (in the client's\n\
\tfile tree) for that directory.\n"
MC_ONESHARE_D="\
0503-722 maintclient:  Only one SHARE (-S) directory may be specified.\n"
MC_ONESPOT_D="\
0503-723 maintclient:  Only one SPOT (-U) directory may be specified.\n"

#***********************************************************************
# Print out a usage message.
#***********************************************************************
Usage () {
  dspmsg -s $MS_INSTLCLIENT $CATALOG $MC_USAGE_I "$MC_USAGE_D"
  Cleanup
  exit 1
}

#***********************************************************************
# Check the hostname is one of the DWM_AKA hostname or not
# This will make sure instlclient works fine for multi-hosted server
#***********************************************************************
check_aka()
{
	CK_HOST=$1
        for i in $DWM_AKA
        do
                if [ $i = $CK_HOST ]
                then	# Yes, this is Another Known Alias
                        IS_LOCAL=TRUE
			return
                fi
        done
        IS_LOCAL=FALSE
	return
}


#***********************************************************************
# emport DWM_AKA variable if it's definded in dwm_platform
#***********************************************************************
export_aka()
{
	# find out where the dwm files reside & cd there
	# was there a directory when invoked ???
        dirs=`dirname $0`
        PWD=`pwd`
        if [ "$dirs" = "." ]
        then
                dirs="$PWD:$PATH"
        else
                dirs="$dirs:$PATH"
        fi
        dirs=`echo $dirs | sed -e "s/:/ /g"`
        FOUND_DWM=FALSE
        for i in $dirs
        do
		if [ -f $i/dwm_platform ]
		then # include dwm_platform which difines DWM_AKA
			FOUND_DWM=TRUE
			. $i/dwm_platform
                        break
	fi
        done

	if [ FOUND_DWM = FALSE ]
	then
		echo "\nDWM software not found.\n"
		Cleanup
		exit 1
	fi


        DWM_AKA=`echo $DWM_AKA | sed -e "s/:/ /g" | sed -e "s/,/ /g"`

	return
}

#
# Determine what should be mounted to get access
# to the files in the specified directory.  Uses df to determine 
# whether the directory is local or remote.  If its remote, a directory 
# of the form:
#
#           <hostname>:<directory path name>
#
# is returned. If its local a directory of the form:
#
#            <directory path name>
#
# is returned.
#

FindDir () {
p1=${1}
savename=
curdir=`pwd`
cd ${p1}
if [ $? -ne 0 ]
then
   cd ${curdir}
   echo ${p1}
   return 1
fi

p1=`pwd`
basedir=${p1}
cd ${curdir}

while :
do
  mydf="${basedir}
        `${DF}`"
  dfdir=`echo "${mydf}" | awk '{
                               if ( NR == 1 )
                                  dir = $1
                               else
                                  if ( $7 == dir )
                                     print $1
                              }'`
   
  if [ x${dfdir} = x ]
    then
       if [ ${basedir} = / ]
         then
            break 
         else
	    savename=/`${BASENAME} ${basedir}`${savename}
            basedir=`${DIRNAME} ${basedir}`
       fi

    else
       rmtdir=`echo ${dfdir} | ${FGREP} :`
       if [ x${rmtdir} = x ]
         then
            break

         else
            p1=${rmtdir}${savename}
#           p1=`echo ${rmtdir} | awk -F: '{print $1}'`:${p1}
            break
        fi
  fi
done
 
echo ${p1}

}

#***********************************************************************
# Check the arguments given with -R; if only one -R argument was given
# and a file exists with that name, use the contents of that file as the
# list of root parts for the command to be executed.
#***********************************************************************
GetRootFile () {
  if [ `echo $ROPTS | wc -w` -eq 1 ]
  then
    if [ -f $ROPTS ]
    then
      FILE=$ROPTS
      ROPTS=
      cat $FILE | while read x
      do
        ROPTS="$ROPTS $x"
      done
    fi
  fi
}

#***********************************************************************
# Check the mount directory for an NFS path that is actually local to
# this machine and convert it to a local path.
#***********************************************************************
CheckMount () {

  F2=`echo $1 | awk -F: '{print $2}'`
  host=`echo $1 | awk -F: '{print $1}'`

# DDDDDDD
  # Support AKA (Also Known As) in multi-homed server
  check_aka $host

  if [ $host = $HOSTNAME -o $host = $HOST -o $IS_LOCAL = TRUE -a "x$F2" != 'x' ]
  then
    echo $1 | awk -F: '{print $2}'
  else
    echo $1
  fi
}

#***********************************************************************
# Makes a list of directories to be mounted along with their mount
# points; this list of directories is mounted over each root directory
# in succession before the command is executed.
#***********************************************************************
MakeMountList () {
  MOUNTS="$UOPT $MOUNTUSR $SOPT $MOUNTSHARE $DEV $MOUNTDEV $TMP $MOUNTTMP"
  for mount in $MOPTS
  do
    mnt=`echo $mount | awk -F, '{print $1}'`
    mnt=`CheckMount $mnt`
    mntpt=`echo $mount,$MOUNTROOT | awk -F, '{print $3 $2}'`
    MOUNTS="$MOUNTS $mnt $mntpt"
  done
}

#***********************************************************************
# Verify that the root directory mount point exists; if it does not,
# then create it.
#***********************************************************************
MakeMountPoint () {
  if [ ! -d $MOUNTROOT ]
  then
    TMP=
    echo $MOUNTROOT | \
      awk -F/ '{for(i=2; i<=NF; i++) print $i " "}' | \
      while read a
      do
        TMP=$TMP/$a
        if [ ! -d $TMP ]
        then
          mkdir $TMP
        fi
      done
  fi
}

#***********************************************************************
# Mount the specified directories onto the specified mount points.  A
# list of the directories that are mounted is created so that DoUmounts
# can unmount the directories mounted by this procedure.  If a mount
# error occurs along the way, the directories that have already been
# mounted are unmounted and an error code is returned.
# The first argument is a directory to be mounted, the second is the
# mount point for the 1st, the 3rd is another directory, the 4th is it's
# mount point, etc.
#***********************************************************************
DoMounts () {
  MOUNTDIRS=
  UMOUNTLIST=
  while [ x$1 != x ]
  do
    $MOUNT $1 $2
    if [ $? != 0 ]
    then
      DoUmounts
      return 1
    else
      MOUNTDIRS="$MOUNTDIRS $1"
      UMOUNTLIST="$2 $UMOUNTLIST"
    fi
    shift; shift
  done
  return 0
}

#***********************************************************************
# Unmounts the list of directories mounted by the DoMounts procedure.
#***********************************************************************
DoUmounts () {
  for i in $UMOUNTLIST
  do
    $UMOUNT $i
  done
  UMOUNTLIST=
}

#***********************************************************************
# Prints out the mounted directories.
#***********************************************************************
PrintMounts () {
  command=`echo $COMMAND | awk '{print $1}'`
  command=`basename $command`
  root=`echo $MOUNTDIRS | awk '{printf ("%s", $1)}'`
  usr=`echo $MOUNTDIRS | awk '{printf ("%s", $2)}'`
  share=`echo $MOUNTDIRS | awk '{printf ("%s", $3)}'`
  dspmsg -s $MS_INSTLCLIENT $CATALOG $MC_HEADER_I "$MC_HEADER_D" \
    "$JOPT" "$JOPT" maintclient $command "$JOPT" $root "$JOPT" $usr \
    "$JOPT" $share "$JOPT"
}

#***********************************************************************
# Clean up behind ourself before exiting.
#***********************************************************************
Cleanup () {
  DoUmounts
  rm -f $CLIENTLOCK
}

#***********************************************************************
# Create a lock to make sure only one maintclient is being run at a time.
#***********************************************************************
echo $$ >> $CLIENTLOCK
if [ x`cat $CLIENTLOCK | head -1` != x$$ ]
then
  dspmsg -s $MS_INSTLCLIENT $CATALOG $MC_INUSE_E "$MC_INUSE_D" \
    $CMDNAME $CMDNAME $CMDNAME $CLIENTLOCK
#  rm -f $CLIENTLOCK
  exit 1
fi

#***********************************************************************
# Set up trap handlers for signals that will cause us to exit.
#***********************************************************************
trap 'Cleanup; exit 2' 1 2 9 15

#***********************************************************************
# Determine if the /dev and /tmp directories are part of a remote 
# filesystem.
#***********************************************************************
DEV=`FindDir ${DEV}`
TMP=`FindDir ${TMP}`

#***********************************************************************
# Parse the command line with getopt and check for errors.
#***********************************************************************
set -- `getopt JR:U:S:M: $*`
if [ $? != 0 ]
then
  Usage
fi

#***********************************************************************
# Initially, the flag arguments are empty.
#***********************************************************************
ROPTS=
UOPT=
SOPT=
MOPTS=
JOPT=' '

#***********************************************************************
# export DWM_AKA variable
#***********************************************************************
export_aka

#***********************************************************************
# Loop through the flags found on the command line and process them.
#***********************************************************************
while [ $1 != -- ]
do
  case $1 in
    #*******************************************************************
    # -C ==> Don't check client information
    #*******************************************************************
    -C) shift
        ;;
    #*******************************************************************
    # -J ==> We were called from SMIT
    #*******************************************************************
    -J) JOPT='#'
        shift
        ;;
    #*******************************************************************
    # -M path,path ==> Path and mount point for a special mount for a
    #                  client.
    #*******************************************************************
    -M) if [ x`echo $2 | grep ,` != x$2 ]
        then
          dspmsg -s $MS_INSTLCLIENT $CATALOG $MC_MARG_E "$MC_MARG_D" \
            maintclient
          Usage
        fi
        MOPTS="$MOPTS $2"
        shift; shift
        ;;
    #*******************************************************************
    # -R path ==> Path for the root part of a client.
    #*******************************************************************
    -R) ROPTS="$ROPTS `CheckMount $2`"
        shift; shift
        ;;
    #*******************************************************************
    # -S path ==> Path for the share part of a client.
    #*******************************************************************
    -S) if [ x$SOPT != x ]
        then
          dspmsg -s $MS_INSTLCLIENT $CATALOG $MC_ONESHARE_E "$MC_ONESHARE_D"
          Usage
        fi
        SOPT=`CheckMount $2`
        shift; shift
        ;;
    #*******************************************************************
    # -U path ==> Path for the usr part of a client.
    #*******************************************************************
    -U) if [ x$UOPT != x ]
        then
          dspmsg -s $MS_INSTLCLIENT $CATALOG $MC_ONESPOT_E "$MC_ONESPOT_D"
          Usage
        fi
        UOPT=`CheckMount $2`
        shift; shift
        ;;
  esac
done

#***********************************************************************
# Ignore the '--' that getopt places on the command line and take the
# remainder of the command line as the command to be executed.
#***********************************************************************
shift
COMMAND=$*

#***********************************************************************
# Make sure that the required options (at least one -R, a -U, a -S, and
# a command to execute) have been found on the command line.
#***********************************************************************
if [ "x$ROPTS" = x -o x$UOPT = x -o x$SOPT = x -o "x$COMMAND" = x ]
then
  Usage
fi

#***********************************************************************
# Process the arguments to the -R flag; if only one was given and it is
# a path to a file, use the contents of that file as the list of root
# paths for the command to execute.
#***********************************************************************
GetRootFile

#***********************************************************************
# Make a list of the directories to be mounted after the root directory
# has been mounted.
#***********************************************************************
MakeMountList

#***********************************************************************
# Make the root directory mount point, if necessary.
#***********************************************************************
MakeMountPoint

#***********************************************************************
# Loop through the list of root directories specified and do the
# following:
#   1) mount the various directories to create a replica of the client's
#      file tree in a directory of this machine's file tree
#   2) execute the given command in the newly created file tree
#   3) unmount the various directories
#***********************************************************************
STATUS=0                # Assume success
for root in $ROPTS
do
  root=`CheckMount $root`
  DoMounts $root $MOUNTROOT $MOUNTS
  DMRC=$?

  # ----------------------------------------------------------------------- #
  #   Need to assure that after we do a chroot to /home/maintclients a 
  #  symlink (called maintclients) exists in the client's home dir (which
  #  should be a subdirectory off of root after the chroot) to "..".  
  #  In other words the symlink will look like this:
  #         /home/maintclients/home/maintclients -> ".."
  #  This is so the df command will work after we do a chroot.
  #  The bosboot cmd (which is called when the boot image is updated) is
  #  vitally dependant on df working.
  # ----------------------------------------------------------------------- #

  if [ ! -r /home/maintclients/home/maintclients ]; then
     ln -s .. /home/maintclients/home/maintclients > /dev/null 2>&1
  else
     rm -rf /home/maintclients/home/maintclients > /dev/nul 2>&1
     ln -sf .. /home/maintclients/home/maintclients > /dev/null 2>&1
  fi

  #
  ################################################################
  #
  # Inorder to mount the CDROM device on to /home/maintclients/$INU_CD_TMPDIR
  # we have to create that cdrfs. If we do that in instlclient so that
  # it is created only once, the /etc/filesystems entry for the cdrfs won't
  # available after CHROOT. So, we gotta do it in maintclient. 
  # Since MAINTCLIENT is called by lppchkclient also, we should detect
  # that case too. 
  #
  ##############################################################
  #
  CMD=`echo $COMMAND | cut -f1 -d' ' | cut -f4 -d/` 
  CRFS_RC=0
  MT_RC=0
  INU_CDRFS_CREATED=
  if [ x$CDROM_IS_INPUT != x -a ! -d /home/maintclients/$INU_CD_TMPDIR -a \
	 x$CMD = xinstallp -a x$INU_INSTALLING_ROOT = x ]; then
     crfs -v cdrfs -p ro -d $INU_CD_DEV -m/home/maintclients/$INU_CD_TMPDIR \
          -Ano
     CRFS_RC=$?
     mount /home/maintclients/$INU_CD_TMPDIR
     MT_RC=$?
     INU_CDRFS_CREATED=yes
  fi
  if [ $DMRC = 0 -a $CRFS_RC = 0 -a $MT_RC = 0 ]
  then
    PrintMounts
    $CHROOT $MOUNTROOT $COMMAND
    if [ $? != 0 ]
    then
      STATUS=1          # There has been a failure
    fi
  ###############################################################
  #
  # In DoUmounts everything mounted in DoMounts is umounted,
  # But, since $INU_CD_TMPDIR is mounted on /home/maintclients/tmp
  # it can't be umounted and that work is being done below.  
  # It could have been added to DoUmounts, but since the mounting
  # of cdrfs is done outside of DoMounts, to be consistent,
  # it is done here
  #
  ###############################################################
    if [ x$CDROM_IS_INPUT != x -a x$CMD = xinstallp -a $CRFS_RC = 0 -a x$INU_CDRFS_CREATED != x  ] ; then
      if [ $MT_RC = 0 ] ; then
         umount /home/maintclients/$INU_CD_TMPDIR
         # if [ $? -ne 0 ]; then
         #
         # fi
      fi
      rmfs /home/maintclients/$INU_CD_TMPDIR 
       # if [ $? -ne 0 ]; then
       #
       # fi
      #-------------------------------------------------------
      #
      #  rmfs removes entries from /etc/filesystems, but
      #  does not get rid of the mount point. So, here we go
      #
      #--------------------------------------------------------
      rmdir /home/maintclients/$INU_CD_TMPDIR   >/dev/null  2>&1
    fi
    DoUmounts
  else
    STATUS=1            # There has been a failure
  fi
done
#***********************************************************************
# All finished; return our status to the calling process.
#***********************************************************************
Cleanup
exit $STATUS
