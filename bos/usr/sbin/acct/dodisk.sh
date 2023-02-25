#! /usr/bin/bsh
# @(#)22 1.2.1.3  src/bos/usr/sbin/acct/dodisk.sh, cmdacct, bos411, 9428A410j 6/10/94 09:51:45
#
#  COMPONENT_NAME: (CMDACCT) Command Accounting
# 
#  FUNCTIONS: none
# 
#  ORIGINS: 3, 9, 27
# 
#  (C) COPYRIGHT International Business Machines Corp. 1985, 1994
#  All Rights Reserved
#  Licensed Materials - Property of IBM
# 
#  US Government Users Restricted Rights - Use, duplication or
#  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
# 
# 'perform disk accounting'
ACCTDIR=${ACCTDIR:-/var/adm}
_nite=${ACCTDIR}/acct/nite
PATH=/usr/sbin/acct:/bin:/usr/bin:/etc:
set -- `getopt o $*`
if [ $? -ne 0 ]
then
	if dspmsg  acct.cat 9  "Usage: dodisk [ -o ] [ filesystem ... ]\n"
	then :
	else echo "Usage: dodisk [ -o ] [ filesystem ... ]\n"
	fi
	exit 1
fi
for i in $*; do
	case $i in
	-o)	SLOW=1; shift;;
	--)	shift; break;;
	esac
done

cd ${ACCTDIR}
date

if [ "$SLOW" = "" ]
then
	if [ $# -lt 1 ]
	then
		args=`awk '
		BEGIN   { true = 1; false = 0; keepdefault = 0; keepcomment = 0 }

		/^\*.*$/{   # pass comments unchanged
			    if (debug) printf "DEBUG: comment: <%s>\n", $0
			    if (keepcomment)
				print;
			    next
			}

		/^.*:$/  {   # found stanza name
			    if (debug) printf "DEBUG: stanza name: <%s>\n", $0
			    name = $1
			    instanza = true
			    if (name == "default:") {
				split("", defaults)             # empty out the array
				if (keepdefault)
				    print;
			    }
			    else {
				split("", values)               # empty out the array
				for (i in defaults) {
				    values[i] = defaults[i]     # init the values array
				}
				print
			    }
			    next
			}

		/^ *$/  {   # found blank line, maybe end of stanza
			    if (debug) printf "DEBUG: blank line:<%s>\n", $0
			    if (! instanza) {
				next                            # skip ordinary white space
			    }
			    else {                              # dump the current stanza
				instanza = false
				if (name == "default:") {
				    if (keepdefault) {
					for (i in defaults) {
					    printf "%s=%s\n", i, defaults[i]
					}
				    }
				}
				else {
				    for (i in values) {
					printf "%s=%s\n", i, values[i]
				    }
				}
			    }
			    print
			    next
			}

		/^.*=.*$/ { # must be an attribute line
			    if (debug) printf "DEBUG: atribute line: <%s>\n", $0
			    n = index($0, "=")
			    attr = substr($0, 1, n-1)
			    value= substr($0, n+1)
			    if (name == "default:") {
				defaults[ attr ] = value
			    }
			    else {
				values[ attr ] = value
			    }
			    next
			}

		/^.*$/  {   # looks like garbage to me
			    if (debug) printf "DEBUG: garbage line: <%s>\n", $0
			    next
			}

		END     {   # dump last stanza if need be.
			    if (instanza)
				if (name == "default:") {       # unless it was
				    for (i in defaults) {
					printf "%s=%s\n", i, defaults[i]
				    }
				}
				else {
				    for (i in values) {
					printf "%s=%s\n", i, values[i]
				    }
				}
			    printf "\n"
			}
		' /etc/filesystems | grep -p 'account.*true' | \
				  grep '^[^a-zA-Z]dev.*=' | sed 's/dev.*=//' `
	else
		args="$*"
	fi
	#
	# diskusg has a built-in user limit.  See if this system exceeds
	# that limit and increase it if so.
	#
	users=`/bin/wc -l < /etc/passwd`
	if [ $users -gt 5000 ]; then
		users=`/bin/expr $users + 100`
		args="$args -U ${users}"
	fi
	diskusg $args > dtmp
else
	if [ $# -lt 1 ]
	then
		args="/"
	else
		args="$*"
	fi
	for i in $args; do
		if [ ! -d $i ]
		then
			echo "$0: $i \c"
			if dspmsg acct.cat 10 "is not a directory -- ignored\n"
			then :
			else echo "is not a directory -- ignored"
			fi
		else
			dir="$i $dir"
		fi
	done
	if [ "$dir" = "" ]
	then
		if dspmsg -s 1 acct.cat 11 "dodisk: No data" > dtmp
		then :
		else echo "dodisk: No data" > dtmp
		fi
	else
		find $dir -print | acctdusg > dtmp
	fi
fi

date
sort +0n +1 -o dtmp dtmp
acctdisk <dtmp >${_nite}/dacct
chmod 644 ${_nite}/dacct
chown adm.adm ${_nite}/dacct
