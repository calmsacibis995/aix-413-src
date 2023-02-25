# COMPONENT_NAME: pkgpci

# ORIGINS: 13, 27

# @(#)87        1.2  src/packages/bos/pci/usr/lpp.doc.S, pkgpci, pkg410 3/30/94 # 10:15:40

#   Copyright (c) 1994 Locus Computing Corporation
#   All Rights Reserved
#
#   (C) COPYRIGHT International Business Machines Corp. 1994
#   All Rights Reserved
#   Licensed Materials - Property of IBM
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.

>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SPECIAL NOTE: DOS Server for AIX Version 4.1 supports only Ethernet and Token 
Ring connection between the server and AIX Access For DOS Users Version
3.1.  Asynchronous connection to DOS Server is not supported in AIX Version 4.1.
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

This README covers two topics: 1) a statement that PC scan mode is not
supported in AIX Version 4 and 2) notification that PTY format has changed
in the pciptys file.

1) EM2 PC scan mode is not supported in DOS Server for AIX Version 4.
 
2) For the new format of the pciptys file, the section called
"Configuring Pseudo-Terminal (PTY) Ports" in the Installation and Server
Notes will be incorrect.  This is a sample "readme" file we talked
about to update the information in this section.  This "readme" file
is preliminary and may need some minor updates when we determine the
exact format of the getty command and after we actually try it for
terminal emulation.  Is there a getty man page you could fax us?
Anyway, here is the file:

Configuring Pseudo-Terminal (PTY) Ports for Terminal Emulation
--------------------------------------------------------------

The file, /usr/pci/pciptys is accessed by
the DOS Server to determine which pseudo-ttys (ptys) are available for
use in terminal emulation.  The number of entries in the file determines
the number of possible simultaneous terminal emulation sessions.

The file lists the ptys used for terminal emulation in the format:
	control_half:slave_half:[optional getty command]

An example of entries in the pciptys file appears below:

ptyp0:ttyp0:/usr/sbin/getty ttyp0 9600
ptyp1:ttyp1:/usr/sbin/getty ttyp1 9600
ptyp2:ttyp2:/usr/sbin/getty ttyp2 9600
ptyp3:ttyp3:/usr/sbin/getty ttyp3 9600
ptyp4:ttyp4:/usr/sbin/getty ttyp4 9600
ptyp5:ttyp5:/usr/sbin/getty ttyp5 9600
ptyp10:ttyp10

The first lines in the example above, which are used for LAN terminal emulation,
include the getty command which informs the DOS Server to start 
a getty on that pty.  The last line in the pciptys file is used for RS-232
terminal emulation and doesn't contain the getty command since gettys must
be started in /etc/inittab instead.

For both the LAN and RS-232 versions of the DOS Server, special pty
device nodes must exist in the /dev directory corresponding to the entries in
the pciptys file.  There are separate devices for the control half and
slave half of each pty.  The number of pty devices in /dev available for
the DOS Server terminal emulation may vary since pty devices are used
by other programs.  If a given pty is in use when the DOS Server tries
to open it, the DOS Server tries to open each successive pty
listed in /usr/pci/pciptys until it succeeds in opening one.

For the LAN version of the DOS Server, to increase the number of terminal
emulation ports available, follow these steps:

1.  Log in as root
2.  Make sure that you have enough PTY devices nodes.  The number of nodes
    is handled through the "Device/PTY" menu items in SMIT.
3.  Inform the DOS Server about these new ptys by adding new lines to the
    pciptys file, including the getty command on the line, in the following
    format:
	ptypx:ttypx:/usr/sbin/getty ttypx 9600H
    where ptypx and ttypx (x is an appropriate pty number) are the newly
    created pty nodes.  You can use the existing entries in the pciptys
    file as a guide.

For the RS-232 version of the DOS Server, to increase the number of terminal
emulation ports available, follow these steps:

1.  Log in as root
2.  Make sure that you have enough PTY devices nodes.  The number of nodes
    is handled through the "Device/PTY" menu items in SMIT.
3.  Add the appropriate entries to the /etc/inittab file in the following
    format, using the existing entry as a guide:
	ptypx:2:respawn:/usr/sbin/getty /dev/ttypx
    where ptypx and ttypx (x is an appropriate pty number) are the newly
    created pty nodes.
4.  Inform the DOS Server about this new entry by adding a new line to the
    pciptys file without the getty command, in the following format:
	ptypx:ttypx
    where ptypx and ttypx (x is an appropriate pty number) are the newly
    created pty nodes.  You can use the existing entry in the pciptys file
    as a guide.
   
When you have installed PTYs for RS-232 terminal emulation in this manner,
these PTYs are no longer available for general service and are devoted to
log-ins from the DOS Server.
