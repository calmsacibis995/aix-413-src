# @(#)58        1.31  src/packages/bos/sysmgt/nim/master/bos.sysmgt.nim.master.il, pkgnim, pkg41J, 9521B_all 5/24/95 17:46:28
#
# COMPONENT_NAME: pkgnim
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
# LPP_NAME: bos.sysmgt.nim.master
#
#
# ----------------------- NIM database   ------------------------------
#
V 0 0 550	/etc/objrepos/nim_pdattr nim-db
V 0 0 550	/etc/objrepos/nim_pdattr.vc nim-db
V 0 0 664	/etc/objrepos/nim_object nim-db
V 0 0 664	/etc/objrepos/nim_object.vc nim-db
V 0 0 664	/etc/objrepos/nim_attr nim-db
V 0 0 664	/etc/objrepos/nim_attr.vc nim-db
#
# ----------------------- NIM directories -----------------------------
#
D 0 0 775	/export nim-cmd
D 0 0 775	/export/nim nim-cmd
D 0 0 775	/export/nim/scripts nim-cmd
#
# ----------------------- NIM commands   ------------------------------
#
F 0 0 550	/usr/sbin/nim nim-cmd
F 0 0 550	/usr/sbin/lsnim nim-cmd
F 0 0 550	/usr/sbin/nimconfig nim-cmd
F 0 0 550	/usr/sbin/nimesis nim-cmd
#
# ----------------------- IPL ROM emulation  --------------------------
#
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/IPLROM.emulation nim-method
#
# ----------------------- NIM master README  --------------------------
#
F 0 0 440	/usr/lpp/bos.sysmgt/nim/README
#
# ----------------------- NIM master methods --------------------------
#
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_abort nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_alloc_boot nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_alloc_ns nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_alloc_pdir nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_alloc_spot nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_allocate nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_bos_inst nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_chattr nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_chmac nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_chmaster nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_chnet nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_chres nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_chstate nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_ckmac nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_ckspot nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_ck_lpp_source nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_cust nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_dealloc_boot nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_dealloc_ns nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_dealloc_pdir nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_dealloc_spot nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_deallocate nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_destroy_res nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_diag nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_dkls_inst nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_do_sop nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_doreboot nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_dtls_inst nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_fixquery nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_getdate nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_instspot nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_lslpp nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_ls_lpp_source nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_maint nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_mkbosi nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_mkmac nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_mknet nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_mkpdir nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_mkres nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_mk_lpp_source nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_mkspot nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_nnc_setup nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_reset_res nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_reboot nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_rmmac nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_rmnet nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_rmpdir nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_rmres nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_rmspot nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_schedule nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_spotmaint nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_sync_roots nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/m_unconfig nim-method
#
# ---------- Special non-nim client methods --------------------------
#
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/c_nnc_setup nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/c_nim_bootinfo32 nim-method
F 0 0 550	/usr/lpp/bos.sysmgt/nim/methods/c_nim_mkboot32 nim-method
