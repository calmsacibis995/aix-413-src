#!/bin/bsh
# @(#)20        1.25  src/bos/usr/sbin/install/client/instlclient.sh, cmdinstl, bos411, 9428A410j 5/20/93 10:59:42
# COMPONENT_NAME: (CMDINSTL) diskless install
#
# FUNCTIONS: instlclient, lslppclient, lppchkclient
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
# NAME: instlclient, lslppclient, lppchkclient
#
# FUNCTION: Set up a replica of a client's filesystem on the local machine
#   and execute a command in that filesystem.
#
# USAGE:
#   instlclient  {-S spot | -K client [-K client]} [-M mnt,mntpt] [-J]
#                <installp options>
#   lslppclient  {-S spot | -K client [-K client]} [-M mnt,mntpt] [-J]
#                <lslpp options>
#   lppchkclient {-S spot | -K client [-K client]} [-M mnt,mntpt] [-J]
#                <lppchk options>
#
# ARGUMENTS:
#   -S            Spot name to operate on
#   -K            Client name to operate on
#   -M mnt,mntpt  Specifies an additional mount to be performed
#   -J            Specifies that we were called from SMIT
#   All other options are passed on to the command that is executed.
#
# RETURN VALUE DESCRIPTION:
#   0 if everything succeeded
#   !0 if something failed; the return code will be the return code
#      from whatever command failed.

#***********************************************************************
# Defines for some commands, operands, and directories that are used.
#***********************************************************************
MOUNT=/usr/sbin/mount
UMOUNT=/usr/sbin/umount
MAINTCLIENT=/usr/sbin/maintclient
INSTALLP=/usr/sbin/installp
LPPCHK=/usr/bin/lppchk
LSLPP=/usr/bin/lslpp
FGREP=/usr/bin/fgrep
DF=/usr/bin/df
DIRNAME=/usr/bin/dirname
BASENAME=/usr/bin/basename
INSTALLPFLAGS="?AaBCcDd:Ff:gIiJlNO:PqRrsT:tUvXxz:"
LPPCHKFLAGS="fclvum:t:O:"
LSLPPFLAGS="lhpdifqcAaBIO:mJ?"
TEMPDIR=/tmp/inuclienttmp$$
CLIENTLOCK=/tmp/client_LOCK
HOSTNAME=`hostname`
HOST=`host $HOSTNAME | awk '{print $1}'`
INUCLTSTAT=$TEMPDIR/status
export INUCLTSTAT

blocksize=0

#***********************************************************************
# Messages.
#***********************************************************************
MS_INSTLCLIENT=12
CATALOG=cmdinstl_e.cat

MC_INUSE_E=40
MC_MARG_E=23
IC_USAGE1_I=41
IC_USAGE2_I=42
IC_USAGE3_I=43
IC_INVALID_SPOT_E=30
IC_EMPTY_SPOT_E=31
IC_INVALID_CLIENT_E=32
IC_CLIENT_MATCH_E=33
IC_OARG_E=34
IC_ONE_FLAG_E=35
IC_ONE_SPOT_E=36
IC_MUTUAL_E=37
IC_NO_CLIENTS_E=38
IC_INVALID_CMD_E=39
IC_MUTUALO_E=45
IC_SAVEDIR_E=46

DWM_AKA=	# initialize NULL
IS_LOCAL=FALSE	# initialize FALSE

MC_INUSE_D="\
0503-720 %s:  The %s command is currently in use.\n\
\tPlease try again later.  If the %s command is not in use\n\
\tthen delete %s.\n"
MC_MARG_D="\
0503-721 %s:  The argument to -M must be mnt,mntpt where mnt is a\n\
\tdirectory to mount and mntpt is the mount point (in the client's\n\
\tfile tree) for that directory.\n"
IC_USAGE1_D="\
%s instlclient {-S SPOTname | -K client1 [-K client2 ...]}\n\
\t[-M [host1:]dir1,mountpt1 [-M [host2:]dir2,mountpt2 ...]]\n\
\t{installp_flags}\n"
IC_USAGE2_D="\
%s lslppclient {-S SPOTname | -K client1 [-K client2 ...]}\n\
\t[-M [host1:]dir1,mountpt1 [-M [host2:]dir2,mountpt2 ...]]\n\
\t{lslpp_flags}\n"
IC_USAGE3_D="\
%s lppchkclient {-S SPOTname | -K client1 [-K client2 ...]}\n\
\t[-M [host1:]dir1,mountpt1 [-M [host2:]dir2,mountpt2 ...]]\n\
\t{lppchk_flags}\n"
IC_INVALID_SPOT_D="\
0503-724 %s:  %s is not a valid SPOT name.\n"
IC_EMPTY_SPOT_D="\
0503-725 %s:  The given SPOT must contain at least one client.\n"
IC_INVALID_CLIENT_D="\
0503-726 %s:  %s is not a valid client name.\n"
IC_CLIENT_MATCH_D="\
0503-727 %s:  All clients specified with the -K flag\n\
\tmust have the same SPOT.\n"
IC_OARG_D="\
0503-728 %s:  A %s is an invalid argument to the -O flag.\n"
IC_ONE_FLAG_D="\
0503-729 %s:  The %s flag may only be used once on the command line.\n"
IC_ONE_SPOT_D="\
0503-730 %s:  Only one SPOT (-S) may be specified.\n"
IC_MUTUAL_D="\
0503-731 %s:  The -S and -K flags are mutually exclusive.\n"
IC_NO_CLIENTS_D="\
0503-732 %s:  Either a SPOT (-S) or a client (-K) must be specified.\n"
IC_INVALID_CMD_D="\
0503-733 %s is not a valid command name.\n"
IC_MUTUALO_D="\
0503-734 %s:  The -Os and -Or flags are mutually exclusive.\n"
IC_SAVEDIR_D="\
0503-735 %s:  The -t save-directory flag is not supported.\n"


#***********************************************************************
# Print out a usage message.
#***********************************************************************
Usage () {
  USAGE=0
  if [ $INU_CMDNAME = instlclient ]
  then
    USAGE=1
  fi
  if [ $INU_CMDNAME = lslppclient ]
  then
    USAGE=2
  fi
  if [ $INU_CMDNAME = lppchkclient ]
  then
    USAGE=3
  fi
  Usage="Usage:"
  if [ $USAGE = 1 -o $USAGE = 0 ]
  then
    dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_USAGE1_I "$IC_USAGE1_D" "$Usage"
    Usage="      "
  fi
  if [ $USAGE = 2 -o $USAGE = 0 ]
  then
    dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_USAGE2_I "$IC_USAGE2_D" "$Usage"
    Usage="      "
  fi
  if [ $USAGE = 3 -o $USAGE = 0 ]
  then
    dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_USAGE3_I "$IC_USAGE3_D" "$Usage"
    Usage="      "
  fi
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

#***********************************************************************
#  get tape block size
#***********************************************************************
inu_ls_blocksize()
{

	# make sure it's a tape drive
	lsdev -C -c tape | cut -f1 -d" " | grep "^$1$" > /dev/null 2>&1
	rc=$?
	if test $rc -ne 0 ; then
		blocksize=0
		return $rc
	fi
	
	# get it's block size
	blocksize=`lsattr -l $1 -a block_size -EO 2>/dev/null` 
	rc=$?
	if test $rc -ne 0 ; then
		blocksize=0
		return $rc
	fi
	blocksize=`echo "$blocksize" | sed -e "/^#/d"`
	return 0
}

#***********************************************************************
#  change tape block size
#***********************************************************************
inu_chg_blocksize()
{
	tape=`echo $1 | cut -c3-`
	tape=`basename $tape`
	tape=`echo $tape | cut -f1 -d.`
	inu_ls_blocksize $tape
	if test $? -ne 0; then
		return 0
	fi

	if test $blocksize -ne $2; then
		ODMDIR=/etc/objrepos chdev -l $tape -a block_size=$2 1>/dev/null
	fi
	return 0
}

#***********************************************************************
#  change tape block size
#***********************************************************************
inu_blocksize()
{
	if [ x"$dOPT" != x ]
	then
		if [ x"$zOPT" = x ]
		then
			zOPT=512;
		fi
		if [ $zOPT != $blocksize ]
		then
			inu_chg_blocksize $dOPT $zOPT
		fi
	fi
}
#***********************************************************************
#  change tape to it's original size
#***********************************************************************
inu_oldblocksize()
{
	if [ x"$dOPT" != x ]
	then
		if [ x"$zOPT" != x ]
		then
			if [ $zOPT != $blocksize ]
			then
				t_blocksize=$blocksize
				inu_chg_blocksize $dOPT $blocksize
				blocksize=$t_blocksize
			fi
		fi
	fi
}
#***********************************************************************
# Determine the name of the SPOT used by the specified client
#***********************************************************************

sname () {
echo `lsdclient -cL $1 2>/dev/null | awk -F: '

	BEGIN {sname=0}
	{
		if (NR == 1) {
			for (i=1; i<=NF; i++) {
				if ($i == "sname") {
					sname = i
					break
				}
			}
		}
		else {
			if (sname != 0)
				print $sname
		}
	} '`
}

#***********************************************************************
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
#***********************************************************************

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
   
#########################################################
#
#    If there is more than one match ( $7 == dir ), since
#     each match is being concatanated, we should use the 
#     last match. 
#
#######################################################
num_of_matches=`echo $dfdir | wc -w`		   #How many? 
dfdir=`echo $dfdir | cut -f$num_of_matches -d' '`  #Get the last match 

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
            break
        fi
  fi
done
 
echo ${p1}

}

#***********************************************************************
# Check if the specified SPOT is the server's /usr filesystem.  If it
# is, add the server's fully qualified name to the INUCLIENTS environment
# variable.
#***********************************************************************
CheckServer () {
  SPOTHOST=`lsspot -E $1 | awk -F: '{print $1}'` 
  SPOTDIR=`lsspot -E $1 | awk -F: '{print $2}'` 
  if [ "$SPOTHOST" = "$HOST" -a "$SPOTDIR" = "/usr" ]
    then
	INUCLIENTS="$INUCLIENTS $HOST"
  fi
} 

#***********************************************************************
# Check the validity of the specified SPOT name and determine the
# directories associated with it.
#***********************************************************************
CheckSpot () {
  SPOTNAME=$1
  if [ x`lsspot | grep "^${1}\$" ` = x ]
  then
    dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_INVALID_SPOT_E \
      "$IC_INVALID_SPOT_D" $INU_CMDNAME $1
    Usage
  fi
  SHARE=`lsspot -cL $1 | awk -F: '{
    if ( NR == 1 ) {
      for ( i = 1; i <= NF; i++ ) {
        if ( $i == "shost" ) shost = i
        if ( $i == "share" ) share = i
      }
    } else {
      print "-S " $shost ":" $share
    } }'`
  USR="-U `lsspot -E $1`"
  ROOT=
  CheckServer $1
  for root in `lsdclient -E $1`
  do
    INUCLIENTS="$INUCLIENTS `host $root 2>/dev/null | awk '{print $1}'`"
    ROOT="$ROOT -R `lsdclient -R $root`"
  done
  if [ "x$ROOT" = x ]
  then
    dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_EMPTY_SPOT_E "$IC_EMPTY_SPOT_D" \
      $INU_CMDNAME
    Usage
  fi
  ONEROOT=`echo $ROOT | awk '{print $1 $2}'`
}

#***********************************************************************
# Check the validity of the specified client names and determine the
# directories associated with them.
#***********************************************************************
CheckClients () {
  if [ $# = 1 ]
  then
    if [ "x`lsdclient | grep \"^${1}\$\"`" = "x" ]
    then
      if [ -f $1 ]
      then
        set -- `cat $1`
      fi
    fi
  fi
  if [ "x`lsdclient | grep \"^${1}\$\"`" = "x" ]
  then
    dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_INVALID_CLIENT_E \
      "$IC_INVALID_CLIENT_D" $INU_CMDNAME $1
    Usage
  fi
  TEMP=`lsdclient -cL $1 | awk -F: '
    BEGIN { sname = shost = spot = shhost = share = 0 }
    { if ( NR == 1 ) {
        for ( i = 1; i <= NF; i++ ) {
          if ( $i == "sname"  ) sname  = i
          if ( $i == "shost"  ) shost  = i
          if ( $i == "spot"   ) spot   = i
          if ( $i == "shhost" ) shhost = i
          if ( $i == "share"  ) share  = i
        }
      } else {
        if ( sname != 0 && \
             shost != 0 && \
             spot != 0 && \
             shhost != 0 && \
             share != 0 )
          print $sname " " $shost ":" $spot " " $shhost ":" $share
      } }'`
  if [ "x$TEMP" = "x" ]
  then
    dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_INVALID_CLIENT_E \
      "$IC_INVALID_CLIENT_D" $INU_CMDNAME $1
    Usage
  fi
  SPOTNAME=`echo $TEMP | awk '{print $1}'`
  SHARE=`echo $TEMP | awk '{print "-S " $3}'`
  USR=`echo $TEMP | awk '{print "-U " $2}'`
  #
  # Determine the names of all clients that use the spot SPOTNAME.
  # I will not use lsdclient -E $SPOTNAME because this can block
  # on systems with two or more network adapter and the proper 
  # aliases are not set in /usr/lib/dwm/dwm_platform (see defect
  # 39754 for more details).
  #
  spot=
  for client in $*
  do
    spottemp=`sname $client`
    if [ "x${spottemp}" = "x${SPOTNAME}" ]
    then
       if [ "x${spot}" = "x" ]
       then  
          spot="${client}"
       else
          spot="${spot} ${client}"
       fi
    fi
  done

  CheckServer $SPOTNAME
  ROOT=
  for client in $*
  do
    if [ "x`lsdclient | grep \"^${client}\$\"`" != "x$client" ]
    then
      dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_INVALID_CLIENT_E \
        "$IC_INVALID_CLIENT_D" $INU_CMDNAME $client
      Usage
    fi
    if [ "x`echo $spot | fgrep $client`" = "x" ]
    then
      dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_CLIENT_MATCH_E \
        "$IC_CLIENT_MATCH_D" $INU_CMDNAME
      Usage
    fi
    INUCLIENTS="$INUCLIENTS `host $client 2>/dev/null | awk '{print $1}'`"
    ROOT="$ROOT -R `lsdclient -R $client`"
  done
  ONEROOT=`echo $ROOT | awk '{print $1 $2}'`
}

#***********************************************************************
# Check the parts specified by the -O flag on the command line.
#***********************************************************************
CheckOflag () {
  RPART=false
  UPART=false
  SPART=false
  oopt=`echo $OOPT | awk '{for ( i = 1; i <= length($0); i++ )
                             print substr($0, i, 1) " "}'`
  for part in $oopt
  do
    case $part in
      r) RPART=true
         ;;
      u) UPART=true
         ;;
      s) SPART=true
         ;;
      *) dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_OARG_E "$IC_OARG_D" \
           $INU_CMDNAME $part
         Usage
         ;;
    esac
  done
  if [ $SPART = true -a $UPART = false -a $RPART = true ]
  then
    dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_MUTUALO_E "$IC_MUTUALO_D" \
      $INU_CMDNAME
    Usage
  fi
}

#***********************************************************************
# Do the specified mount.  Check for the NFS specified mount directory
# being on the current host; if so, then do a local mount instead of an
# NFS mount
#***********************************************************************
DoMount () {
  mounthost=`echo $1 | awk -F: '{print $1}'`
  check_aka $mounthost
  if [ "x$mounthost" = "x$HOST" -o $IS_LOCAL = TRUE ]
  then
    MNTDIR=`echo $1 | awk -F: '{print $2}'`
  else
    MNTDIR=$1
  fi
  $MOUNT $MNTDIR $2
  return $?
}

#***********************************************************************
# Check to see that the boot image in the /tftpboot directory for the
# current SPOT is the most recent boot image for the SPOT.  (The boot
# image could be changed by applying/rejecting the SPOT part of lpps.)
#***********************************************************************
CheckBootImage () {
  spotboot=`echo $USR | awk '{print $2}'`/lib/boot

  spotfile=$TEMPDIR/nblv.image/net.image

  tftpboot=`lsspot -cL $SPOTNAME | awk -F: '{
    if ( NR == 1 ) {
      for ( i = 1; i <= NF; i++ ) {
        if ( $i == "bhost" ) bhost = i
        if ( $i == "boot"  ) boot  = i
      }
    } else {
      print $bhost ":" $boot
    } }'`
  tftpfile=$TEMPDIR/tftpboot.image/$SPOTNAME

  mkdir $TEMPDIR/nblv.image
  mkdir $TEMPDIR/tftpboot.image
  DoMount $spotboot $TEMPDIR/nblv.image
  if [ $? = 0 ]
  then
    DoMount $tftpboot $TEMPDIR/tftpboot.image
    if [ $? = 0 ]
    then
      if [ -s $spotfile ]
      then
        NBLVSUM=`sum $spotfile | awk '{print $1 ":" $2}'`
        TFTPSUM=`sum $tftpfile | awk '{print $1 ":" $2}'`
        if [ $NBLVSUM != $TFTPSUM ]
        then
          cp $spotfile $tftpfile
        fi
      fi
      $UMOUNT $TEMPDIR/tftpboot.image
    fi
    $UMOUNT $TEMPDIR/nblv.image
  fi
}

#***********************************************************************
# Check the status file and determine which lpps were successfully
# applied that have a USR and a ROOT part; these are the lpps which
# need to be applied on the ROOT part of the remaining clients.
#***********************************************************************
CheckStatusFile () {
  if [ -f $INUCLTSTAT ]
  then
    cat $INUCLTSTAT | sort -u | \
      while read name ver rel mod fix ptf content op type status
      do
        if [ x$op = x1 ]
        then
          op=0
        fi
        if [ x$status = x0 -a x$op = x$1 -a x$content = xB ]
        then
          if [ x$ptf != xNONE ]
          then
            echo $name $ver.$rel.$mod.$fix.$ptf
          else
            echo $name $ver.$rel.$mod.$fix
          fi
        fi
      done
  fi
}

#***********************************************************************
# Check the status file and determine if any operations was unsuccessful.
#***********************************************************************
CheckInstallStatus () {
  if [ -f $INUCLTSTAT ]
  then
    cat $INUCLTSTAT | sort -u | \
      while read name ver rel mod fix ptf content op type status
      do
        if [ x$status != x0 ]
        then
          echo -n "1"
        fi
      done
  #
  # if no status file exists, then it's a failure (if we're called)
  #
  else
      echo "1"
  fi
}

#***********************************************************************
# This takes care of doing an install for a ROOT part of an lpp.
#***********************************************************************
InstallRoot () {
  FLAGS="$aOPT $cOPT $rOPT $sOPT $COPT"
  INU_INSTALLING_ROOT=yes    #for CDROM
  export INU_INSTALLING_ROOT
  $MAINTCLIENT $ROOT $USR $SHARE $MOPTS $JOPT \
               $INSTALLP $FLAGS -Or $REMAINDER $LPPS
  INU_INSTALLING_ROOT=
  RC=`CheckInstallStatus`
  if [ "x$RC" != "x" ]
  then
    RC=1
  else
    RC=0
  fi
  return $RC
}

#***********************************************************************
# This takes care of doing an install for a USR part of an lpp.
#***********************************************************************
InstallUsr () {
  FLAGS="$aOPT $cOPT $rOPT $sOPT $COPT $DOPT $dOPT $qOPT"
  ROOTtemp=$ROOT
  ROOT=`echo $ROOT | awk '{for ( i = 3; i <= NF; i++ ) print $i " "}'`
  inu_blocksize
  if [ x$aOPT != x ]
  then
    $MAINTCLIENT $ONEROOT $USR $SHARE $MOPTS $JOPT \
                 $INSTALLP $FLAGS -O$OOPT $REMAINDER $LPPS
    if [ $RPART = true -a "x$ROOT" != "x" ]
    then
      LPPS=`CheckStatusFile 0`
      if [ "x$LPPS" != "x" ]
      then
        InstallRoot
      fi
    fi
  elif [ x$cOPT != x ]
  then
    $MAINTCLIENT $ONEROOT $USR $SHARE $MOPTS $JOPT \
                 $INSTALLP $FLAGS -O$OOPT $REMAINDER $LPPS
    if [ $RPART = true -a "x$ROOT" != "x" ]
    then
      LPPS=`CheckStatusFile 2`
      if [ "x$LPPS" != "x" ]
      then
        InstallRoot
      fi
    fi
  elif [ x$COPT != x ]
  then
    if [ $RPART = true -a "x$ROOT" != "x" ]
    then
      InstallRoot
    fi
    $MAINTCLIENT $ONEROOT $USR $SHARE $MOPTS $JOPT \
                 $INSTALLP $FLAGS -O$OOPT $REMAINDER $LPPS
  elif [ x$rOPT != x ]
  then
    if [ $RPART = true -a "x$ROOT" != "x" ]
    then
      InstallRoot
    fi
    $MAINTCLIENT $ONEROOT $USR $SHARE $MOPTS $JOPT \
                 $INSTALLP $FLAGS -O$OOPT $REMAINDER $LPPS
  else
    $MAINTCLIENT $ONEROOT $USR $SHARE $MOPTS $JOPT \
                 $INSTALLP $FLAGS -O$OOPT $REMAINDER $LPPS
    if [ $RPART = true -a "x$ROOT" != "x" ]
    then
      InstallRoot
    fi
  fi
  if [ x$aOPT != x -o x$cOPT != x -o x$COPT != x -o x$rOPT != x ]
  then
    RC=`CheckInstallStatus`
    if [ "x$RC" != "x" ]
    then
      RC=1
    else
      RC=0
    fi
  else
    RC=0
  fi
  ROOT=$ROOTtemp
  inu_oldblocksize
  return $RC
}

#***********************************************************************
# This takes care of doing an install for a SHARE part of an lpp.
#***********************************************************************
InstallShare () {
  FLAGS="$aOPT $cOPT $rOPT $sOPT $COPT $DOPT $dOPT $qOPT"
  inu_blocksize
  $MAINTCLIENT $ONEROOT $USR $SHARE $MOPTS $JOPT \
               $INSTALLP $FLAGS -Os $REMAINDER $LPPS
  inu_oldblocksize
  RC=`CheckInstallStatus`
  if [ "x$RC" != "x" ]
  then
    RC=1
  else
    RC=0
  fi
  return $RC
}

#***********************************************************************
# Run lppchk for each lpp which was successfully installed.
#***********************************************************************
InstallLppchk () {

 #
 # Basically unconditionally execute "lppchk -ul" to handle the
 # case where we have a symlink from the root to the usr, but the
 # image has only a usr part.  This call to lppchk will create
 # such a link.
 #
  $MAINTCLIENT $ROOT $USR $SHARE $MOPTS $JOPT \
                    $LPPCHK -ul 


#
# Comment out existing code -- in case we ever need it again.
  #if [ -f $INUCLTSTAT ]
 # then
 #   cat $INUCLTSTAT | sort -u | \
 #     while read name ver rel mod fix ptf content op type status
 #     do
 #       if [ x$op = x1 ]
 #       then
 #         op=0
 #       fi
 #       if [ x$status = x0 -a x$op = x0 ]
 #       then
 #         $MAINTCLIENT $ROOT $USR $SHARE $MOPTS $JOPT \
 #                      $LPPCHK -ul $name
 #       fi
 #     done
 # else
 #      $MAINTCLIENT $ROOT $USR $SHARE $MOPTS $JOPT \
 #                   $LPPCHK -ul 
 # fi
  return 0
}

#***********************************************************************
# Do the actual install of the lpps.
#***********************************************************************
DoInstallp () {
  mkdir $TEMPDIR
  mkdir $TEMPDIR/inst.images
  if [ x$AOPT = x -a x$COPT = x -a x$aOPT = x -a x$cOPT = x -a \
       x$iOPT = x -a x$lOPT = x -a x$rOPT = x -a x$sOPT = x ]
  then
    aOPT=-a
  fi
  if [ x$aOPT != x -o \
       x$cOPT != x -o \
       x$rOPT != x -o \
       x$sOPT != x -o \
       x$COPT != x ]
  then
    RC=0
    if [ x$OOPT = x ]
    then
      OOPT=sur
      CheckOflag
    fi
    if [ $SPART = true -a $UPART = false -a $RPART = false ]
    then
      InstallShare
      RC=$?
    elif [ $SPART = false -a $UPART = false -a $RPART = true ]
    then
      REMAINDER="$REMAINDER $dOPT"
      InstallRoot
      RC=$?
    else
      InstallUsr
      RC=$?
    fi
    InstallLppchk
  else
    inu_blocksize
    if [ x$OOPT = x ]
    then
      $MAINTCLIENT $ONEROOT $USR $SHARE $MOPTS $JOPT \
                   $INSTALLP $dOPT $DOPT $qOPT $REMAINDER $LPPS
      RC=$?
    else
      $MAINTCLIENT $ONEROOT $USR $SHARE $MOPTS $JOPT \
                   $INSTALLP $dOPT $DOPT $qOPT -O$OOPT $REMAINDER $LPPS
      RC=$?
    fi
    inu_oldblocksize
  fi
  #
  #  We put the check for an updated boot image here because
  #  of the case where installp was ran previous to this 
  #  invocation of instlclient which 1.) updated instlclient
  #  and 2.) updated the boot image(s) in the spot(s).
  #  NOTE:  CheckBootImage depends on the existence of $TEMPDIR 
  #         below
  #  
  if [ $STATUS = 0 -a \( x$aOPT != x -o x$rOPT != x \) ]
  then
      CheckBootImage
  fi
  #--------------------------------------------------------
  # If we used a CDROM as the input device, then the original
  # directory over which the CD was mounted would have been umounted.
  # Now, we should mount it back
  #-------------------------------------------------------- 

  if [ x$CD_MOUNTED != x ]; then	
     mount -o $OPTION -v $VFS $CD_DEV $M_OVER
     #if [ xNEED_TO_CD_BACK = xyes ]; then   # we were temporarily relocated
     #	  cd $temp1
     #       fi 
  fi
  rm -rf $TEMPDIR
  return $RC
}

#***********************************************************************
# Execute lppchk for the given client(s).
#***********************************************************************
DoLppchk () {
  $MAINTCLIENT $ROOT $USR $SHARE $MOPTS $JOPT \
               $LPPCHK $REMAINDER
  return $?
}

#***********************************************************************
# Execute lslpp for the given client(s).
#***********************************************************************
DoLslpp () {
  $MAINTCLIENT $ROOT $USR $SHARE $MOPTS $JOPT \
               $LSLPP $REMAINDER
  return $?
}

#***********************************************************************
# Clean up behind ourself before exiting.
#***********************************************************************
Cleanup () {
  inu_oldblocksize
  rm -f $CLIENTLOCK
}

#
# ###################################################################
#
# This is is to find out if cdrom is to be mounted when
#   1. cdrom was mounted first, and one or more normal directoried were 
#      mounted on top of it. In this case the mounted fs/dir is used. 
#
#   2. A normanl dir was mounted on cdrfs, and then the cdrom was mounted
#      on top of that.  In this case cdrom is used.
#
# ####################################################################

is_cd_preferred()
{
CD_ON_LINE=`mount | grep $TARGET | awk  -f $TMP_CD_DEVFILE`
TOTAL_LINES=`mount | grep $TARGET | wc -l | awk '{print $1}'`
if [ $CD_ON_LINE = $TOTAL_LINES ] ; then
  return 0
else
  return 1
fi
}

#***********************************************************************
# Make sure we were called with a valid command name.
#***********************************************************************
INU_CMDNAME=`basename $0`
if [ $INU_CMDNAME != instlclient -a \
     $INU_CMDNAME != lslppclient -a \
     $INU_CMDNAME != lppchkclient ]
then
  dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_INVALID_CMD_E "$IC_INVALID_CMD_D" \
    $INU_CMDNAME
  Usage
fi
export INU_CMDNAME

#***********************************************************************
# Create a lock to make sure only one {instl|lslpp|lppchk}client is
# running at a time.
#***********************************************************************
echo $$ >> $CLIENTLOCK
if [ x`cat $CLIENTLOCK | head -1` != x$$ ]
then
  dspmsg -s $MS_INSTLCLIENT $CATALOG $MC_INUSE_E "$MC_INUSE_D" \
    $INU_CMDNAME $INU_CMDNAME $INU_CMDNAME $CLIENTLOCK
  exit 1
fi

#***********************************************************************
# Set up trap handlers for signals that will cause us to exit.
#***********************************************************************
trap 'Cleanup; exit 2' 1 2 9 15

#***********************************************************************
# Initially, the flag arguments are empty.
#***********************************************************************
CLIENT=
SPOT=
MOPTS=
JOPT=
REMAINDER=

#***********************************************************************
# export DWM_AKA variable
#***********************************************************************
export_aka

#***********************************************************************
# Determine which command we were executed as and parse the command
# line options accordingly.
#***********************************************************************
case $INU_CMDNAME in
  #*********************************************************************
  # Command is instlclient.
  #*********************************************************************
  instlclient)  AOPT=
                COPT=
                DOPT=
                OOPT=
                aOPT=
                cOPT=
                dOPT=
                iOPT=
                lOPT=
                qOPT=
                rOPT=
                sOPT=
		zOPT=
                set -- `getopt JS:K:M:$INSTALLPFLAGS $*`
                if [ $? != 0 ]
                then
                  Usage
                fi
                while [ $1 != -- ]
                do
                  case $1 in
                    #***************************************************
                    # -A => List APAR information.
                    #***************************************************
                    -A) if [ x$AOPT != x ]
                        then
                          dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_ONE_FLAG_E \
                            "$IC_ONE_FLAG_D" $INU_CMDNAME "-A"
                          Usage
                        fi
                        AOPT=-A
			REMAINDER="$REMAINDER -A"
                        shift
                        ;;
                    #***************************************************
                    # -C => Cleanup a failed install.
                    #***************************************************
                    -C) if [ x$COPT != x ]
                        then
                          dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_ONE_FLAG_E \
                            "$IC_ONE_FLAG_D" $INU_CMDNAME "-C"
                          Usage
                        fi
                        COPT=-C
                        shift
                        ;;
                    #***************************************************
                    # -D => ???
                    #***************************************************
                    -D) if [ x$DOPT != x ]
                        then
                          dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_ONE_FLAG_E \
                            "$IC_ONE_FLAG_D" $INU_CMDNAME "-D"
                          Usage
                        fi
                        DOPT=-D
                        shift
                        ;;
                    #***************************************************
                    # -J => called from SMIT
                    #***************************************************
                    -J) if [ x$JOPT != x ]
                        then
                          dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_ONE_FLAG_E \
                            "$IC_ONE_FLAG_D" $INU_CMDNAME "-J"
                          Usage
                        fi
                        JOPT=-J
                        REMAINDER="$REMAINDER -J"
                        shift
                        ;;
                    #***************************************************
                    # -K client => Which client to act on.
                    #***************************************************
                    -K) CLIENT="$CLIENT $2"
                        shift; shift
                        ;;
                    #***************************************************
                    # -M mnt,mntpt => Additional client mounts to be
                    #                 executed.
                    #***************************************************
                    -M) if [ x`echo $2 | grep ,` != x$2 ]
                        then
                          dspmsg -s $MS_INSTLCLIENT $CATALOG $MC_MARG_E \
                            "$MC_MARG_D" $INU_CMDNAME
                          Usage
                        fi
                        MOPTS="$MOPTS -M $2"
                        shift; shift
                        ;;
                    #***************************************************
                    # -S spot => Which spot to act on.
                    #***************************************************
                    -S) if [ x$SPOT != x ]
                        then
                          dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_ONE_SPOT_E \
                            "$IC_ONE_SPOT_D" $INU_CMDNAME
                          Usage
                        fi
                        SPOT=$2
                        shift; shift
                        ;;
                    #***************************************************
                    # -O parts => Which parts to install.
                    #***************************************************
                    -O) if [ x$OOPT != x ]
                        then
                          dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_ONE_FLAG_E \
                            "$IC_ONE_FLAG_D" $INU_CMDNAME "-O"
                          Usage
                        fi
                        OOPT=$2
                        CheckOflag
                        shift; shift
                        ;;
                    #***************************************************
                    # -X => Expand file systems - ignored.
                    #***************************************************
                    -X) shift
                        ;;
                    -z) if [ x$zOPT != x ]
                        then
                          dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_ONE_FLAG_E \
                            "$IC_ONE_FLAG_D" $INU_CMDNAME "-z"
                          Usage
                        fi
			zOPT=$2
			REMAINDER="$REMAINDER -z $zOPT"
			shift; shift
			;;
                    #***************************************************
                    # -a => Do an apply.
                    #***************************************************
                    -a) if [ x$aOPT != x ]
                        then
                          dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_ONE_FLAG_E \
                            "$IC_ONE_FLAG_D" $INU_CMDNAME "-a"
                          Usage
                        fi
                        aOPT=-a
                        shift
                        ;;
                    #***************************************************
                    # -c => Do a commit.
                    #***************************************************
                    -c) if [ x$cOPT != x ]
                        then
                          dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_ONE_FLAG_E \
                            "$IC_ONE_FLAG_D" $INU_CMDNAME "-c"
                          Usage
                        fi
                        cOPT=-c
                        shift
                        ;;
                    #***************************************************
                    # -d device => Device to install from.
                    #***************************************************
                    -d) if [ x$dOPT != x ]
                        then
                          dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_ONE_FLAG_E \
                            "$IC_ONE_FLAG_D" $INU_CMDNAME "-d"
                          Usage
                        fi
			#--------------------------------------------
			# Variables to support the CDROM device
			#

			CDROM_IS_INPUT=	     #Is CDROM the input device?
			CD_MOUNTED=          #Is CD Mounted on a cdrfs?	
			INU_CD_TMPDIR=tmp/inu_cdtmp$$    #temp cdrfs
			INU_CD_DEV=			 # cdX
			export CDROM_IS_INPUT CD_MOUNTED INU_CD_TMPDIR \
			       INU_CD_DEV

			#
                        # ---------------------------------------------
                        # make sure arg to -d has a leading '/'
                        # ---------------------------------------------
                        if [ x`echo $2 | grep '^/'` = x ]
                        then
			  if [ `pwd` = / ]
			  then
			    temp=/$2
			  else
                            temp=`pwd`/$2
			  fi
                        else
                          temp=$2
                        fi

                        # ---------------------------------------------
			# At this point, temp contains the argument 
			# that the user specified with the -d flag to
			# the instlclient command.
			#
			# This device could be one of several types of 
			# devices:
			#  1.) a tape device or other device that contains
			#     "/dev" in its name, /dev/rfd0, for instance.
			#  2.) a local directory containing bffs
			#      -- could also include a single bff image 
			#         specified as a device.
			#  3.) a remotely mounted dir containing bffs
			#      -- could also include a single bff image 
			#         specified as a device.
			# 
			# The case we are specifically interested in is
			# the remotely mounted dir case.  In such cases
			# we want to prepend the fullpath name of the
			# dir with "machine name:", as in:
			#   "machine1:/usr/sys/inst.images"
			# We want to do this so we can DIRECTLY mount
			# the directory -- so as to avoid double mounts.
                        # ---------------------------------------------

                        # ---------------------------------------------
                        # if $temp is readable and does NOT contain /dev 
			# or /tmp as a substring
                        # ---------------------------------------------
                        if [ -r $temp -a \
                             x`echo $temp | grep '^/dev/'` = x -a \
                             x`echo $temp | grep '^/tmp/'` = x ]
                        then
                           # ---------------------------------------------
			   #  if temp is a directory, see if it's remote 
                           # ---------------------------------------------
			   IS_A_BFF=		#used for processing CDROM
			   if [ -d $temp ]; then
                              temp1=`FindDir $temp`
                           else  
                              # ---------------------------------------------
			      #  Else temp must be a filename (bff)
			      #  So, remove the filename (get its dir name)
			      #  and see if it is remotely mounted.
                              # ---------------------------------------------
			      IS_A_BFF=yes
			      bff_name=`basename $temp`
			      temp3=`${DIRNAME} $temp`
                              temp1=`FindDir $temp3`
                           fi

                           temp2=`echo $temp | awk -F/ '{print $NF}'`
                           MOPTS="$MOPTS -M$temp1,$TEMPDIR/inst.images"

			   #-------------------------------------------
			   # If we are dealing with a directory which is 
			   # a mount point for a CDROM, then we need to
			   # find out if we have to mount the CD
			   #-------------------------------------------
			   # Is it a CDROM device?
			   CD_MOUNTED_ON_TARGET=
			   CD_PREFFERED=
			   (lsfs -c $temp1 | fgrep cdrfs ) >/dev/null 2>&1
			   if [ $? -eq 0 ]; then
		              CD_DEV=`lsfs $temp1 | grep "^/" | cut -f1 -d' '`
			      (mount | grep $CD_DEV | grep $temp1) >/dev/null \
									2>&1
			      if [ $? -eq 0 ]; then
				CD_MOUNTED_ON_TARGET=yes
				TARGET=$temp1
			        INU_CD_DEV=`echo $CD_DEV | cut -f3 -d/` 

				########################################
				#
				# AWK can't handle $ variables, so we
			        #  need to create an awk script
				#
				########################################
			    	TMP_CD_DEVFILE=/tmp/tmpCD_DEVFILE.$$ 
			        echo "/\/dev\/$INU_CD_DEV/ {print NR}" > $TMP_CD_DEVFILE
			 				   
			          CD_PREFFERED=no   #To CD or not to CD 
			          is_cd_preferred   # latest mount on $temp? 
			          if [ $? -eq 0 ]; then
  			             CD_PREFFERED=yes   # To CD!!          
			          fi
 			          rm $TMP_CD_DEVFILE >/dev/null 2>&1 
			      fi
			    fi
			   if [ x$CD_PREFFERED != xyes ]; then # Normal process
                             if [ -d $temp ]; then
                                # ---------------------------------------------
                                # if temp is a dir, pass it along to maintclient
			        # as it is.
                                # ---------------------------------------------
                                dOPT=-d$TEMPDIR/inst.images
                             else  
                                # ---------------------------------------------
                                # if temp is a bff name, we need to add the    
			        # bffname to the fully qualified path to the
			        # dir that the bff resides in.
                                # ---------------------------------------------
                                dOPT=-d$TEMPDIR/inst.images/$temp2
                             fi
		           else # Processing directory, or bff on cdrfs
			     #
			     # We should unmount the cdrom locally since
		 	     # the device can not be muliply mounted (i.e.
			     # the local mount, and the 
			     # /home/maintclients/tmp/tmp_inucd mount)
			     # 
				CDROM_IS_INPUT=yes
			     if [ x$CD_MOUNTED_ON_TARGET = xyes ]; then
				#
    				# remember the mount options 
				#
				CD_MOUNTED=yes
			        M_OVER=`mount| fgrep $CD_DEV | awk '{print $2}'`
				VFS=`mount | fgrep $CD_DEV | awk '{print $3}'` 
				OPTION=`mount | fgrep $CD_DEV | awk '{print $7}'`
				#
				# If the user specifies the current directory 
				# as the device, we have to cd to some other
			        # directory before we can umount it, say /tmp.
				#
				# NEED_TO_CD_BACK=
				# DIR_TO_CD=/tmp	 # where do I go?
				# if [ $temp1 = `pwd` ]; then
				#   NEED_TO_CD_BACK=yes
				#   cd $DIR_TO_CD
				# fi

				#
	  		        # If it is mounted we umount it if we can
	  			# If someone is accessing it, we get  error
				# about the impending failure. We let installp
				# handle the failure. 
				#
  	  			umount $CD_DEV
	  			#if [ $? -ne 0 ]
	  			#then
	    			   #exit BADRC
				   # echo "can not umount cd device"
			   	#fi
  			       fi
				MOPTS=    # we don't need this, so blank it
				if [ x$IS_A_BFF = x ]; then
				   dOPT=-d/$INU_CD_TMPDIR  #we are ready now
				 else  #bff
				   dOPT=-d/$INU_CD_TMPDIR/$bff_name  
				 fi
			 fi # Processing for CD with dirname/ bff

                        else
                           # ---------------------------------------------
			   #  temp contains "/dev" or "/tmp" which aren't
			   #  remote.  So pass along as specified.
                           # ---------------------------------------------

			   #----------------------------------------------
			   # For CDROM we need special processing 
			   # If CDROM then
			   #    If it is mounted already
			   #	   remember the mount options
			   #	   umount it (CDROM can be mounted only once)
			   #	   set a variable so that we can remount CDROM
			   #    else
			   #	   CDROM not mounted, no problemo
			   # else
			   #    either rmt , or fd, or ...
			   #------------------------------------------------ 

			  CDROM=      # Assume that we are not 
			   	      # using a CDROM device
			  echo $temp | fgrep  -q "/dev/cd" > /dev/null 2>&1 
			  if [ $? -eq 0 ]; then
  			    #get the actual device name, cd0,cd1,etc.
    			    INU_CD_DEV=`echo $temp | cut -f3 -d/`  
    			    # determine if it is a CDROM
    			    DEVTYPE=`lsdev -C -F 'name class'  | fgrep 	$INU_CD_DEV | awk ' {print $2} '`
    			   if [ x$DEVTYPE = x ]; then 
       			      #return NODEVICE
       			      #echo "The specified device does not exist"
       			      DEVTYPE=nodevice
    			    fi
    			    if [ $DEVTYPE = cdrom ]; then
      			       CDROM_IS_INPUT=yes  # Used by maintclient
      			       temp=/$INU_CD_TMPDIR 
      			       (mount | fgrep -q /dev/$INU_CD_DEV )>/dev/null \
									  2>&1 
      			      if [ $? -eq 0 ] ; then 
  	  			CD_MOUNTED=yes 
	  			M_OVER=` mount | fgrep /dev/$INU_CD_DEV| awk \
							          '{print $2}'` 
				VFS=`mount | fgrep /dev/$INU_CD_DEV | awk \
							          '{print $3}'` 
				OPTION=`mount | fgrep /dev/$INU_CD_DEV | awk \
								  '{print $7}'`
								 
			        #
	  			# If it is mounted we umount it if we can.
	  			# If someone is accessing it, we should get 
			        # a bad ret code 
				#
  	  			umount /dev/$INU_CD_DEV
	  			#if [ $? -ne 0 ]; then
	    		          # exit BADRC
	    		          # echo "could not umount cdrom " 
          			#fi
       			     else
         			CD_MOUNTED=   #redundant 
      			     fi   # CD mounted? 
                     fi
		fi
    
#--------------------------------------------------------------------------
# Check for a CDROM is already done  at this point
                dOPT=-d$temp
                fi
                 shift; shift
                     ;;
                    #***************************************************
                    # -i => List supplemental information.
                    #***************************************************
                    -i) if [ x$iOPT != x ]
                        then
                          dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_ONE_FLAG_E \
                            "$IC_ONE_FLAG_D" $INU_CMDNAME "-i"
                          Usage
                        fi
                        iOPT=-i
			REMAINDER="$REMAINDER -i"
                        shift
                        ;;
                    #***************************************************
                    # -l => List products on media.
                    #***************************************************
                    -l) if [ x$lOPT != x ]
                        then
                          dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_ONE_FLAG_E \
                            "$IC_ONE_FLAG_D" $INU_CMDNAME "-l"
                          Usage
                        fi
                        lOPT=-l
			REMAINDER="$REMAINDER -l"
                        shift
                        ;;
                    #***************************************************
                    # -q => Do not prompt for the first volume.
                    #***************************************************
                    -q) if [ x$qOPT != x ]
                        then
                          dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_ONE_FLAG_E \
                            "$IC_ONE_FLAG_D" $INU_CMDNAME "-q"
                          Usage
                        fi
                        qOPT=-q
                        shift
                        ;;
                    #***************************************************
                    # -r => Do a reject.
                    #***************************************************
                    -r) if [ x$rOPT != x ]
                        then
                          dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_ONE_FLAG_E \
                            "$IC_ONE_FLAG_D" $INU_CMDNAME "-r"
                          Usage
                        fi
                        rOPT=-r
                        shift
                        ;;
                    #***************************************************
                    # -s => List uncommitted software.
                    #***************************************************
                    -s) if [ x$sOPT != x ]
                        then
                          dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_ONE_FLAG_E \
                            "$IC_ONE_FLAG_D" $INU_CMDNAME "-s"
                          Usage
                        fi
                        sOPT=-s
                        shift
                        ;;
                    #***************************************************
                    # -t => Do not allow -t, not supported.
                    #***************************************************
                    -t) dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_SAVEDIR_E \
                          "$IC_SAVEDIR_D" $INU_CMDNAME
                        Usage
                        ;;
                    #***************************************************
                    # Everyting else get clumped together in here.
                    #***************************************************
                     *) REMAINDER="$REMAINDER $1"
                        shift
                        ;;
                  esac
                done
                shift
                LPPS=$*
                ;;
  #*********************************************************************
  # Command is lslppclient.
  #*********************************************************************
  lslppclient)  set -- `getopt JS:K:M:$LSLPPFLAGS $*`
                if [ $? != 0 ]
                then
                  Usage
                fi
                while [ $1 != -- ]
                do
                  case $1 in
                    #***************************************************
                    # -J => called from SMIT
                    #***************************************************
                    -J) if [ x$JOPT != x ]
                        then
                          dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_ONE_FLAG_E \
                            "$IC_ONE_FLAG_D" $INU_CMDNAME "-J"
                          Usage
                        fi
                        JOPT=-J
                        REMAINDER="$REMAINDER -J"
                        shift
                        ;;
                    #***************************************************
                    # -K client => Which client to act on.
                    #***************************************************
                    -K) CLIENT="$CLIENT $2"
                        shift; shift
                        ;;
                    #***************************************************
                    # -M mnt,mntpt => Additional client mounts to be
                    #                 executed.
                    #***************************************************
                    -M) if [ x`echo $2 | grep ,` != x$2 ]
                        then
                          dspmsg -s $MS_INSTLCLIENT $CATALOG $MC_MARG_E \
                            "$MC_MARG_D" $INU_CMDNAME
                          Usage
                        fi
                        MOPTS="$MOPTS -M $2"
                        shift; shift
                        ;;
                    #***************************************************
                    # -S spot => Which spot to act on.
                    #***************************************************
                    -S) if [ x$SPOT != x ]
                        then
                          dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_ONE_SPOT_E \
                            "$IC_ONE_SPOT_D" $INU_CMDNAME
                          Usage
                        fi
                        SPOT=$2
                        shift; shift
                        ;;
                    #***************************************************
                    # Everyting else get clumped together in here.
                    #***************************************************
                     *) REMAINDER="$REMAINDER $1"
                        shift
                        ;;
                  esac
                done
                shift
                REMAINDER="$REMAINDER $*"
                ;;
  #*********************************************************************
  # Command is lppchkclient.
  #*********************************************************************
  lppchkclient) set -- `getopt JS:K:M:$LPPCHKFLAGS $*`
                if [ $? != 0 ]
                then
                  Usage
                fi
                while [ $1 != -- ]
                do
                  case $1 in
                    #***************************************************
                    # -J => called from SMIT
                    #***************************************************
                    -J) if [ x$JOPT != x ]
                        then
                          dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_ONE_FLAG_E \
                            "$IC_ONE_FLAG_D" $INU_CMDNAME "-J"
                          Usage
                        fi
                        JOPT=-J
                        shift
                        ;;
                    #***************************************************
                    # -K client => Which client to act on.
                    #***************************************************
                    -K) CLIENT="$CLIENT $2"
                        shift; shift
                        ;;
                    #***************************************************
                    # -M mnt,mntpt => Additional client mounts to be
                    #                 executed.
                    #***************************************************
                    -M) if [ x`echo $2 | grep ,` != x$2 ]
                        then
                          dspmsg -s $MS_INSTLCLIENT $CATALOG $MC_MARG_E \
                            "$MC_MARG_D" $INU_CMDNAME
                          Usage
                        fi
                        MOPTS="$MOPTS -M $2"
                        shift; shift
                        ;;
                    #***************************************************
                    # -S spot => Which spot to act on.
                    #***************************************************
                    -S) if [ x$SPOT != x ]
                        then
                          dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_ONE_SPOT_E \
                            "$IC_ONE_SPOT_D" $INU_CMDNAME
                          Usage
                        fi
                        SPOT=$2
                        shift; shift
                        ;;
                    #***************************************************
                    # Everyting else get clumped together in here.
                    #***************************************************
                     *) REMAINDER="$REMAINDER $1"
                        shift
                        ;;
                  esac
                done
                shift
                REMAINDER="$REMAINDER $*"
                ;;
esac

#***********************************************************************
# Check that either a single -S or one or more -K flags were given on
# the command line.
#***********************************************************************
if [ x$SPOT != x -a "x$CLIENT" != "x" ]
then
  dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_MUTUAL_E "$IC_MUTUAL_D" $INU_CMDNAME
  Usage
fi

if [ x$SPOT = x -a "x$CLIENT" = "x" ]
then
  dspmsg -s $MS_INSTLCLIENT $CATALOG $IC_NO_CLIENTS_E "$IC_NO_CLIENTS_D" \
    $INU_CMDNAME
  Usage
fi

#*******************************************************************************
# Export INUCLIENTS.  This environment variable contains the names of all
#                     hosts that will be updated by the instlclient
#                     command.
#*******************************************************************************

INUCLIENTS=
export INUCLIENTS

#***********************************************************************
# Check the validity of the given spot/client names and get the
# directories associated with them.
#***********************************************************************
if [ x$SPOT != x ]
then
  CheckSpot $SPOT
else
  CheckClients $CLIENT
fi

#***********************************************************************
# Determine which command to call (installp, lppchk, or lslpp) and call
# the appropriate procedure.
#***********************************************************************
STATUS=0
case $INU_CMDNAME in
  instlclient)  DoInstallp
                STATUS=$?
                ;;
  lslppclient)  DoLslpp
                STATUS=$?
                ;;
  lppchkclient) DoLppchk
                STATUS=$?
                ;;
esac

#***********************************************************************
# All done; remove our lock and return our status to the calling process.
#***********************************************************************
Cleanup
exit $STATUS

