# @(#)90	1.1  src/packages/bos/diagboot/proto.awk.S, diagboot, pkg411, GOLD410 2/22/94 12:56:31
#
#   COMPONENT_NAME: DIAGBOOT
#
#   FUNCTIONS: findfile
#
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#

BEGIN {
  ship = ENVIRON["SHIP_PATH"]
  npath = split(ship,shipdir,":")
}

function findfile(file) {
  s = "/tmp/"
  f = file
  npos = index(f,s)
  if (npos > 0) {
    file2 = substr(f,npos+5)
    if (!system("test -r ./"file2)) {
      return "./"file2
    }
  }
  for (i = 1; i <= npath; i++) {
    file1 = shipdir[i] file
    flag = 0

    if (!system("test -r " file1)) {
      flag = 1
      return file1
      break
    }
  }
  if (flag == 0) {
    print "File " file " not found"
    exit 1
  }
}

{
if (NF == 6 && $2 !~ /^[Ll]/) {
	# if the second field begins with an upper or lowercase "L",
	# then that entry in the proto file describes a link, and
	# should not be mapped to a file in the ship tree
    pos = index($0,$6)
    print substr($0,1,pos-1) findfile($6)
  }
  else {
    print $0
  }
}
