# @(#)96  1.29  src/packages/bos/sysmgt/serv_aid/bos.sysmgt.serv_aid.il, pkgserv_aid, pkg41J, 9514A_all 3/20/95 21:13:31 
#
# COMPONENT_NAME: pkgserv_aid
#
# FUNCTIONS:  inslist
#
# ORIGINS: 27
#
# (C) COPYRIGHT International Business Machines Corp. 1993,1994
# All Rights Reserved
# Licensed Materials - Property of IBM
#
# US Government Users Restricted Rights - Use, duplication or
# disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
# -------------------------------------------------------------------------
#
# LPP_NAME: bos.sysmgt.serv_aid
#
D 0 3 755	/usr/lib/ras/dmprtns errlog
D 2 2 755	/usr/lbin/tty dir
F 0 0 555	/usr/lib/ras/dmprtns/tty errlog
F 0 0 555	/usr/lbin/tty/crash-ldterm errlog
F 0 0 555	/usr/lbin/tty/crash-sptr errlog
F 0 0 555	/usr/lbin/tty/crash-nls errlog
F 0 0 555	/usr/lbin/tty/crash-ptydd errlog
F 0 0 555	/usr/lbin/tty/crash-rs errlog
F 0 0 555	/usr/lbin/tty/crash-shtty errlog
F 0 0 555	/usr/lbin/tty/crash-lion errlog
F 0 0 555	/usr/lbin/tty/crash-cxma errlog
F 0 0 555	/usr/lbin/tty/crash-sf errlog
# Mirrored Console
F 2 2 555	/usr/lib/drivers/pse/mirror errlog
F 2 2 555	/usr/sbin/mirrord errlog
F 2 2 555	/usr/lib/mir_tty errlog
F 2 2 555	/usr/sbin/cfgmir errlog
# errlog & dump & crash commands
F 2 2 544	/usr/bin/errclear errlog
F 0 2 544	/usr/bin/errlogger errlog
F 0 2 544	/usr/bin/errmsg errlog
FT 0 3 4555	/usr/bin/errpt errlog
F 2 2 544	/usr/bin/sysdumpstart errlog
F 0 2 544	/usr/lib/errdead errlog
D 0 3 755	/usr/lib/ras errlog
F 2 2 555	/usr/lib/ras/cpydmpdskt errlog
F 2 2 555	/usr/lib/ras/dmprtns/dmpfmt errlog
H 2 2 777	/usr/lib/ras/dmprtns/_default_dmp_fmt /usr/lib/ras/dmprtns/dmpfmt errlog
# dump formatter for the 3com Ethernet device driver
F 2 2 555	/usr/lib/ras/dmprtns/en3com errlog
# dump formatter for the Monterey Token Ring device driver
F 2 2 555	/usr/lib/ras/dmprtns/trmon errlog
# dump formatter for the fddi device driver
F 2 2 555	/usr/lib/ras/dmprtns/fddi_dd errlog
# dump formatter for the integrate ethernet device driver
F 2 2 555	/usr/lib/ras/dmprtns/ient_dd errlog
# dump formatter for the pci ethernet device driver
F 2 2 555	/usr/lib/ras/dmprtns/kent_dd errlog
# dump format routine for the Wildwood token ring device driver
F 2 2 555	/usr/lib/ras/dmprtns/trmps errlog
# dump format routine for the Sandalfoot ethernet device driver
F 2 2 555	/usr/lib/ras/dmprtns/ientdd errlog
# dump format routine for the Sandalfoot token ring device driver
F 2 2 555	/usr/lib/ras/dmprtns/itok_dd errlog
F 2 2 555	/usr/lib/ras/dump_smutil errlog
F 2 2 555	/usr/lib/ras/errlg_smutil errlog
F 2 2 555	/usr/sbin/crash errlog
H 2 2 777	/usr/sbin/pstat /usr/sbin/crash errlog
F 2 2 500	/usr/sbin/survd errlog
D 2 2 755	/usr/lpp/bos.sysmgt errlog 
D 2 2 755	/usr/lpp/bos.sysmgt/serv_aid errlog 
F 2 2 555	/usr/lpp/bos.sysmgt/serv_aid/bos.sysmgt.serv_aid.codept errlog
F 2 2 555	/usr/lpp/bos.sysmgt/serv_aid/bos.sysmgt.serv_aid.cron errlog
# Sample to find core dumps
D 2 2 755	/usr/samples errlog
D 2 2 755	/usr/samples/findcore errlog
F 2 2 644	/usr/samples/findcore/README errlog
F 2 2 744	/usr/samples/findcore/corepath errlog
F 2 2 744	/usr/samples/findcore/getvfsname errlog
F 2 2 644	/usr/samples/findcore/getvfsname.c errlog

# dump pseudoprobe
F 2 2 555	/usr/lib/ras/logsymptom errlog
