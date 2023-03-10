#!/bin/ksh
# @(#)75        1.3  src/bos/usr/sbin/dtappintegrate/dtappintegrate.sh, dtappintegrate, bos410, 9412A410a 3/7/94 10:03:43
#
#   COMPONENT_NAME: dtappintegrate
#
#   FUNCTIONS: none
#
#   ORIGINS: 27
#
#
#   (C) COPYRIGHT International Business Machines Corp. 1993
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

#####################################################################
#                                                                   #
#  dtappintegrate                                                   #
#                                                                   #
#                                                                   #
#  This is the dt  application integration script to assist         #
#  in integrating applications into COSE Desktop Environment.       #
#                                                                   #
#  syntax: dtappintegrate -s <source> [-t <target>] [-l <lang>] [-u]#
#  where                                                            #
#        -s           indicates application's root location.        #
#        source       the path name of the application's root.      #
#        -t           indicates a new location for the              #
#                       application's group files.                  #
#        target       the path name of the target location.         #
#        -l           indicates the language for this application.  #
#        language     the language name as used by $LANG            #
#                       environment variable.                       #
#        -u           indicates to unintegrate the application.     #
#                                                                   #
#                                                                   #
#####################################################################

#-------------------------------------------------------------------#
#  ShowSyntax                                                       #
#                                                                   #
#       This routine is used to echo the command line syntax.       #
#                                                                   #
#       input:                                                      #
#         none                                                      #
#                                                                   #
#       return codes:                                               #
#         0   - success                                             #
#                                                                   #
#-------------------------------------------------------------------#
function ShowSyntax
{
  #echo "Usage: $SCRIPT_NAME -s <source> [-t <target>] [-l <language>] [-u]" | tee -a $LOGFILE
  dspmsg dtappintegrate.cat -s 1 23 "Usage: %s -s <source> [-t <target>] [-l <language>] [-u]\n" $SCRIPT_NAME | tee -a $LOGFILE
  return 0
}
#-------------------------------------------------------------------#
#  GetAbsolutePath                                                  #
#                                                                   #
#       This routine is to resolve a path to its actual path,       #
#       following links, etc.                                       #
#                                                                   #
#       input:                                                      #
#         $1 = path                                                 #
#                                                                   #
#       output:                                                     #
#         absolute path                                             #
#                                                                   #
#-------------------------------------------------------------------#
function GetAbsolutePath
{
  if [ "/" = "$1" ]; then
     echo $2
  elif [ -L $1 ]; then
     GetAbsolutePath `ls -l $1 | awk '{print $NF}'` $2
  else
     GetAbsolutePath `dirname $1` /`basename $1`$2
  fi
}
#-------------------------------------------------------------------#
#  GetRelativePath                                                  #
#                                                                   #
#       This routine is used to determine the relative path of      #
#       of the source path from the target path.                    #
#                                                                   #
#       input:                                                      #
#         $1 = absolute source path                                 #
#         $2 = absolute target path                                 #
#                                                                   #
#       return codes:                                               #
#         0   - success                                             #
#         1   - error                                               #
#                                                                   #
#-------------------------------------------------------------------#
function GetRelativePath
{
  $AWK 'BEGIN {
          src = ARGV[1]
          dest = ARGV[2]
          a = split(src, A, "/");
          b = split(dest, B, "/");

          s = 0;
           for (i = 2; i < a && i < b; i++) {
             if (match(A[i],B[i])) {
                ++s;
             } else {
                break;
             }
           }
          for (i = 0 ; i <= (a - s - 2); i++) {
             printf("../")
          }
          for (i = 2 + s; i <= b; i++) {
             printf("%s%s", B[i], (i < b) ? "/":"\n");
          }
  }' $2 $1
}
#-------------------------------------------------------------------#
#  LinkCfgs                                                         #
#                                                                   #
#       This routine creates the actual links from the application's#
#       root config files to the target location.                   #
#                                                                   #
#       input:                                                      #
#                                                                   #
#       return codes:                                               #
#         n   - number of files integrated or unintegrated          #
#                                                                   #
#-------------------------------------------------------------------#
function LinkCfgs
{
  typeset source=$1 target=$2 torf=$3 spath="" tpath="" cfgfile="" rpath=""
  typeset pattern="" files=""

  shift;shift;shift
  if [[ -L $source || -L $(dirname $source) ]] then
    spath=$(GetAbsolutePath $source)
  else
    spath=$source
  fi
  if [[ -L $target || -L $(dirname $target) ]] then
    tpath=$(GetAbsolutePath $target)
  else
    tpath=$target
  fi
  rpath=""
  for pattern in "$@"
  do
    if [[ $pattern = "(*)" ]] then
      files=$(ls -d $source/* 2>/dev/null)
    else
      files=$(ls -d $source/$pattern 2>/dev/null)
    fi
    if [[ $? = 0 ]] then
      count=$(echo $files | wc -w)
      for cfgfile in $files
      do
        basecfg=$(basename $cfgfile)
        if [[ $torf = TRUE ]] then
          if [[ $rpath = "" ]] then
            rpath=$(GetRelativePath $spath $tpath)
          fi
          rm -f $tpath/$basecfg
          ln -sf $rpath/$basecfg $tpath/$basecfg
          echo "ln $rpath/$basecfg -> $tpath/$basecfg" >> $LOGFILE
        else
          rm $tpath/$basecfg >/dev/null 2>&1
          if [[ $? = 0 ]] then
            echo "rm $tpath/$basecfg" >> $LOGFILE
          fi
        fi
      done
    fi
  done
  return $count
}

#-------------------------------------------------------------------#
#  IntegrateUnintegrate                                             #
#                                                                   #
#       This routine integrates the files into the cose desktop     #
#       environment or unintegrates them depending on the boolean   #
#       input parameter.                                            #
#                                                                   #
#       input:                                                      #
#         $1 = Integrate or not. TRUE=integrate. FALSE=unintegrate. #
#                                                                   #
#       return codes:                                               #
#         0   - work was done                                       #
#         1   - no work done                                        #
#                                                                   #
#-------------------------------------------------------------------#
function IntegrateUnintegrate
{
  typeset torf=$1 srcs="" trgs="" fpats="" langs="" tpath="" spath="" rpath="" k="" languages="" lang=""
  typeset cfgs="" srcabs="" trgabs=""
  integer i=0 icons=0 types=1 help=2 appmgr=3

  srcabs=$(GetAbsolutePath $APP_ROOT)
  trgabs=$(GetAbsolutePath $APP_TARGET)

  srcs[0]=$srcabs$ICONS$APP_LANG
  srcs[1]=$srcabs$TYPES$APP_LANG
  srcs[2]=$srcabs$HELP$APP_LANG
  srcs[3]=$srcabs$APPMANAGER$APP_LANG

  trgs[0]=$trgabs$ICONS$APP_LANG
  trgs[1]=$trgabs$TYPES$APP_LANG
  trgs[2]=$trgabs$HELP$APP_LANG
  trgs[3]=$trgabs$APPMANAGER$APP_LANG

  fpats[0]="$PIXMAP_FILES $BITMAP_FILES"
  fpats[1]="$ACTIONDB_FILES"
  fpats[2]="$HELPVOLUME_FILES_OLD $HELPVOLUME_FILES_NEW $HELPFAMILY_FILES"
  fpats[3]="$APPMAN_FILES"

  rc=1
  while (( i < 4 ))
  do
    if [[ $APP_LANG = "" ]] then
      languages=$(ls -d ${srcs[i]}/* 2>/dev/null)
      if [[ $? = 0 ]] then
        for lang in $languages
        do
          baselang=$(basename $lang)
          if [[ -d $lang ]] then
            if [[ $torf = TRUE ]] then
              if [[ ! -d ${trgs[i]}/$baselang ]] then
                mkdir -p ${trgs[i]}/$baselang
              fi
            fi
            LinkCfgs ${srcs[i]}/$baselang ${trgs[i]}/$baselang $torf ${fpats[i]}
            if [[ $? != 0 ]] then
              rc=0
            fi
          fi
        done
      fi
    else
      LinkCfgs ${srcs[i]} ${trgs[i]} $torf ${fpats[i]}
      if [[ $? != 0 ]] then
        rc=0
      fi
    fi
    i=i+1
  done
  return $rc
}
#-------------------------------------------------------------------#
#  ExitOut                                                          #
#                                                                   #
#       Exit the program.                                           #
#                                                                   #
#       input:                                                      #
#         $1 = return code                                          #
#                                                                   #
#-------------------------------------------------------------------#
function ExitOut
{
  typeset retcode=$1
  #echo "<<<<<<< END  OF  APPLICATION INTEGRATION >>>>>>>" >> $LOGFILE
  dspmsg dtappintegrate.cat -s 1 2 "<<<<<<< END  OF  APPLICATION INTEGRATION >>>>>>>\n" >> $LOGFILE

  #echo "See $LOGFILE file for more information"
  dspmsg dtappintegrate.cat -s 1 29 "See %s file for more information\n" $LOGFILE
  exit $retcode
}
#  ----<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>-----
#  ----<<<<<<<<<<<.-------------------------.>>>>>>>>>>>-----
#  ----<<<<<<<<<<<|                         |>>>>>>>>>>>-----
#  ----<<<<<<<<<<<|  START OF MAIN ROUTINE  |>>>>>>>>>>>>-----
#  ----<<<<<<<<<<<|_________________________|>>>>>>>>>>>-----
#  ----<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>-----

#--------------------------------------------------------------------
#  Initialize variables
#--------------------------------------------------------------------
sFLAG=0
tFLAG=0
lFLAG=0
uFLAG=0

TYPES=/types
APPMANAGER=/appmanager
ICONS=/icons
HELP=/help
APPCONFIG=/dt/appconfig
CONFIG_TOP=/etc/dt
DT=`basename $CONFIG_TOP`
APP_TARGET=${CONFIG_TOP%/$DT}$APPCONFIG

PIXMAP_FILES=*.pm
BITMAP_FILES=*.bm
HELPVOLUME_FILES_OLD=*.hv
HELPVOLUME_FILES_NEW=*.sdl
HELPFAMILY_FILES=*.hf
ACTIONDB_FILES=*.dt
FRONTPANEL_FILES=*.fp
APPMAN_FILES="(*)"

ID=$(id)
LOGFILE=/tmp/dtappint.log
PATH=/usr/dt/bin:$PATH

#--------------------------------------------------------------------
#  Save application's name in variable.
#--------------------------------------------------------------------
SCRIPT_NAME=$0

#--------------------------------------------------------------------
#  Check if root user.  Exit if not.
#--------------------------------------------------------------------
ID=${ID##*uid=}
ID=${ID#*\(}
ID=${ID%%\)*}
if [[ $ID != root ]] then
   #echo "Error: Must be root user to run $0!" >&2
   dspmsg dtappintegrate.cat -s 1 24 "Error: Must be root user to run %s!\n" $0 >&2
   exit 3
fi

#--------------------------------------------------------------------
#  Put prolog into log file.
#--------------------------------------------------------------------
#echo "<<<<<<< START OF APPLICATION INTEGRATION >>>>>>>" > $LOGFILE
dspmsg dtappintegrate.cat -s 1 1 "<<<<<<< START OF APPLICATION INTEGRATION >>>>>>>\n" > $LOGFILE

#--------------------------------------------------------------------
#  Put the date of application integration into the log file.
#--------------------------------------------------------------------
echo $(date) >> $LOGFILE

#--------------------------------------------------------------------
#  Put the command line into the log file.
#--------------------------------------------------------------------
echo "$SCRIPT_NAME $*" >> $LOGFILE

#--------------------------------------------------------------------
#  Check if there are no command line arguments.
#  If none, then display the command syntax.
#--------------------------------------------------------------------
if [[ $# = 0 ]] then
  ShowSyntax
  ExitOut 0
fi

#--------------------------------------------------------------------
#  Parse the command line into flags and variables.
#--------------------------------------------------------------------
while getopts s:t:l:u  flag
do
  case $flag in
     s)   sFLAG=1
          APP_ROOT="$OPTARG";;
     t)   tFLAG=1
          APP_TARGET="$OPTARG";;
     l)   lFLAG=1
          APP_LANG="$OPTARG";;
     u)   uFLAG=1;;
     ?)   echo " "
          ShowSyntax
          ExitOut 2;;
  esac
done

#--------------------------------------------------------------------
#  Check if application's root was specified and is valid.
#--------------------------------------------------------------------
if [[ $sFLAG = 0 ]] then
  #echo "Error: Did not specify -s option!" >&2
  dspmsg dtappintegrate.cat -s 1 6 "Error: Did not specify -s option!\n" >&2
  ExitOut 4
else
  if [[ ! -d $APP_ROOT ]] then
    APP_PATH=$APP_ROOT
    #echo "Error: $APP_PATH is not a directory!" >&2
    dspmsg dtappintegrate.cat -s 1 27 "Error:  %s  is not a directory!\n" $APP_PATH >&2
    ExitOut 4
  fi
  if [[ ! -r $APP_ROOT ]] || [[ ! -x $APP_ROOT ]] then
    APP_PATH=$APP_ROOT
    #echo "Error: Can not read $APP_PATH directory!" >&2
    dspmsg dtappintegrate.cat -s 1 28 "Error: Can not read  %s  directory!\n" $APP_PATH >&2
    ExitOut 4
  fi
fi

if [[ ${APP_ROOT%%/*} != "" ]] then
  APP_ROOT=$(pwd)/$APP_ROOT
fi

#--------------------------------------------------------------------
#  If target is specified, do some sanity checks on this path.
#--------------------------------------------------------------------
if [[ $tFLAG = 1 ]] then
  if [[ ! -d $APP_TARGET ]] then
    APP_PATH=$APP_TARGET
    #echo "Error: $APP_PATH is not a directory!" >&2
    dspmsg dtappintegrate.cat -s 1 27 "Error:  %s  is not a directory!\n" $APP_PATH >&2
    ExitOut 4
  fi
  if [[ ! -r $APP_TARGET ]] || [[ ! -x $APP_TARGET ]] then
    APP_PATH=$APP_TARGET
    #echo "Error: Can not read $APP_PATH directory!" >&2
    dspmsg dtappintegrate.cat -s 1 28 "Error: Can not read  %s  directory!\n" $APP_PATH >&2
    ExitOut 4
  fi

  if [[ ${APP_TARGET%%/*} != "" ]] then
    APP_TARGET=$(pwd)/$APP_TARGET
  fi
fi

#--------------------------------------------------------------------
#  Set up variables.
#--------------------------------------------------------------------
APP_ROOT=$APP_ROOT$APPCONFIG

if [[ $APP_LANG != "" ]] then
  APP_LANG=/$APP_LANG
fi

#--------------------------------------------------------------------
#  Unintegrate the application by un-doing the integration steps.
#--------------------------------------------------------------------
if [[ $uFLAG = 1 ]] then
  IntegrateUnintegrate FALSE
  if [[ $? = 0 ]] then
    #echo "Unintegration Complete"
    dspmsg dtappintegrate.cat -s 1 10 "Unintegration Complete\n"
  else
    #echo "No files to unintegrate"
    dspmsg dtappintegrate.cat -s 1 11 "No files to unintegrate\n"
  fi
  ExitOut 0
fi

#--------------------------------------------------------------------
#  See if these directories exist.  If they don't, then create them.
#--------------------------------------------------------------------
for i in $APP_TARGET$ICONS$APP_LANG $APP_TARGET$TYPES$APP_LANG $APP_TARGET$APPMANAGER$APP_LANG $APP_TARGET$HELP$APP_LANG
 do
  if [[ ! -d $i ]] then
    mkdir -p $i
  fi
 done

#--------------------------------------------------------------------
#  Determine which awk to use.
#--------------------------------------------------------------------
$(type nawk > /dev/null 2>&1)
if [[ $? != 0 ]] then
  AWK="awk"
else
  AWK="nawk"
fi

#--------------------------------------------------------------------
#  Integrate the application.
#--------------------------------------------------------------------
IntegrateUnintegrate TRUE
if [[ $? = 0 ]] then
  #echo "Integration Complete"
  dspmsg dtappintegrate.cat -s 1 12 "Integration Complete\n"
else
  #echo "No files to integrate"
  dspmsg dtappintegrate.cat -s 1 13 "No files to integrate\n"
fi

#--------------------------------------------------------------------
#  Exit
#--------------------------------------------------------------------
ExitOut 0
