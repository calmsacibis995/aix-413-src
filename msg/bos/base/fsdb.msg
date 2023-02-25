$ ENGLISH_SOURCE_VERSION= 1.10.1.7
$ @(#)11	1.1  src/msg/bos/base/fsdb.msg, bosmsg, msg411, GOLD410 2/9/94 08:59:07  
$
$ COMPONENT_NAME: (CMDFS) commands that deal with the file system
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989 1994
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$
$quote "         define quote character.

$len 
$set MS_FSDB        Define initial set#

$ Do not translate fsdb.
$ Do not translate %s, %c, %d, etc.

DISK_MAP	"Cannot read disk map inode (inode number %d).\n"
$ inode is a technical term.  

INODE_MAP	"Cannot read inode map inode (inode number %d).\n"
$ inode is a technical term.  

USAGE		"Usage: fsdb Filesystem [-]\n"

INVALFS		"fsdb: 0506-148 %s is not a valid file system.\n"

CANTACCESS	"fsdb: 0506-149 Cannot access file system %s.\n"

MALLOC 		"fsdb: 0506-157 malloc failed"
$ malloc is a technical term.  

HEADERMSG	"\n\nFile System:"

ERRCHKOFF	"error checking off\n"

ERRCHKON	"error checking on\n"

HOOKNL		"?\n"

ALIGNMENT	"alignment\n"

OUTFORMAT	"0x%10.10llx%-2s:  0x%8.8lx (%ld)\n"

MAPBLOCKRANGE	"map block out of range\n"

INODERANGE	"inode out of range\n"
$ do not translate inode

BADDIROFF	"bad directory offset\n"

BADNUMBER	"bad number\n"

DIVIDEBY	"divide by 0\n"

READERROR1	"read error.\n"

DATABLOCKRANGE	"data block out of range: 0 <= %d < %d\n"

DATABLOCKALLOC	"warning:  data block not allocated\n"

BADMAPTYPE1	"bad map type {p,m}\n"

WORDIDXREQ	"word index required\n"

BADWORDIDX	"bad word index, 0<=n<%d\n"

SHELLNOTAVAIL	"Shell not available"

INVALIDSHELL	"Shell command is not valid.\n"

BANGNL		"!\n"

BADBLOCKNO	"bad block number\n"

INODENOTALLOC	"warning: inode not allocated\n"
$ do not translate inode

SPECIALDEV	"special device\n"

GETTOOLONG	"get(addr=%llx, lngth=%d) - %lld too long\n"

READERROR2	"read error : 0x%lx\n"

GETINVALLEN     "get(addr=%llx, lngth=%d) - invalid length\n"

WRITEERROR1	"write error : addr   = 0x%10.10llx\n"

TRUNCERROR	"truncation error\n"

LTOPFAIL	"fsdb: mark_blocks: ltop failed for i-block #%d of %s file.\n"

DIRSLOTRANGE	"directory slot %d out of range\n"

MUSTSPECRECLEN	"must specify record length\n"

MINDIRSIZE	"minimum directory size = %d\n"

MAXDIRSIZE	"maximum directory size = %d\n"

BLOCKOVERFLOW	"block overflow\n"

ASCIIASSIGN	"ascii assign in dirent's only to d_name\n"

REMEMBERTOFIX	"remember to fix d_namlen (nl) to = %d\n"

PCT22X		"%02.2X "

PCT44O		"%4.4o "

DIRFORMAT1	"d%d (slot=%u): %4d  "

DIRFORMAT2	" (d_reclen/d_namlen = %d/%d)\n"

ADDRFORM	"\n0x%10.10llx:  "

PCT66O		"  %6.6o"

PCT44X		"  %4.4X"

PCT6D		"  %6d"

MAPPAGEOFF	"%s map page @ %10.10llx\n"

NOTAMAPPAGE	"not a map %s map page\n"

MAPPAGENUM	"%s map page # %d (%10.10.llx)\n"

MAPFMT1		"mapsize (ms) = %ld   freecnt (mf) = %ld   agsize (ag) = %d\n"

INOFMT1		"inode at 0x%.2llx\n\tmd: "
$ do not translate inode

INOFMT2		"i#: %6ld  md: "

INOFMT3		"  ln:%5d  uid:%5d  gid:%5d"

INOFMT4		"  sz: %8lu\n"

INOFMT5		"maj:%3.3o  min:%3.3o  "

INOFMT6		"a%d: 0x%-10.2x"

INOFMT7		"at: "

INOFMT8		"mt: "

INOFMT9		"ct: "

NOSUCHPRINT	"no such print option\n"

CANTMODIFY	"cannot modify mounted file systems\n"

NOTANINODE	"not an inode\n"

NLHOOKNL	"\n?\n"

NOTCHARORBLOCK	"not char or block device\n"

BLOCKINILIST	"block in I-list or diskmaps\n"

LINKEDTO	"Linked to\t-> %s\n"

BLKISDOUBLE	"Double indirect block: 0x%-10.2x"

BLKISSINGLE	"Single indirect block: 0x%-10.2x"

MNTCTLFAIL	"mntctl failed"
$ Do not translate mntctl.  

NOMEMORY	"Out of memory.\n"

JFSVERSION	"\n\thas an unrecognized JFS version number."

MAGIC		"\n\thas an unrecognized JFS magic number."

NOREADSUPER	"\n\tcould not be read."

CORUPT		"\n\tis corrupted. One of s_fragsize, s_agsize, or s_iagsize is invalid."
$ Do not translate s_fragsize, s_agsize, nor  s_iagsize

BADSUPER	"\n\tis not recognized as JFS."

WARNING1	"\nWARNING:\n\tThe primary superblock for %s %s\n\tAttempting to read the secondary superblock...\n"

BADSECONDARY	"\nFATAL ERROR:\n\tThe secondary superblock for %s %s\n"

FIXPRIMARY	"\nWARNING:\n\tThe problems with the primary superblock\n\tmust be corrected prior to mounting %s.\n"

FIXSECONDARY	"\tThe secondary superblock should also be revalidated.\n\n"

FSIZEMSG	"\nFile System Size:"

DMSIZE		"\nDisk Map Size:"

IMSIZE		 "\nInode Map Size:"

AGINFO		"\nAllocation Group Size:"

INOINFO		"\nInodes per Allocation Group:"
$ do not translate inode

FRAGINFO	"\nFragment Size:"

INVALID_DIRENT	"Invalid directory entry\n"

NOTDIRINO	"Not a directory inode: inode address = 0x%lx\n"
$ do not translate inode

NOFREE		"There is not enough free space in block (0x%2x)\nfor a directory entry of %d bytes long.\n"

BLOCK4K		"(4K blocks)"

BLOCK512	"(512 byte blocks)"

BYTES		"(bytes)"

FRAGS		"(fragments)"

FIFO		"fifo\n"
$ do not translate fifo

DATE_FMT	"%a %b %d %T %Y"

TOTALINODE	"\nTotal Inodes:"

TOTALFRAG	"\nTotal Fragments:"
