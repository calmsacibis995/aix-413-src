# @(#)52        1.13.1.1  src/packages/bos/sysmgt/nim/client/bos.sysmgt.nim.client.il, pkgnim, pkg41J, 9522A_all 5/27/95 16:35:15
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
# LPP_NAME: bos.sysmgt.nim.client
#
# ----------------------- client directories --------------------------
#
d 0 0 775	/var/adm/nim dir
D 0 0 755	/usr/lpp/bos.sysmgt/nim nim-cmd
D 0 0 755	/usr/lpp/bos.sysmgt/nim/methods nim-cmd
D 0 0 755	/usr/lpp/bos.sysmgt/nim/awk nim-cmd
#
# ----------------------- client commands -----------------------------
#
F 0 0 550	/usr/sbin/niminit nim-cmd
F 0 0 550	/usr/sbin/nimclient nim-cmd
#
# ----------------------- client methods ------------------------------
#
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/cmdnim_errors.shh nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_add_routes nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_alloc_boot nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_at nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_bosinst_env nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_ch_nfsexp nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_ch_rhost nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_cklevel nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_ckros_emu nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_ckspot nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_dealloc_boot nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_errmsg nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_initiate_bootp nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_installp nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_instspot nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_mk_nimclient nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_mk_lpp_source nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_mkbooti nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_mkdir nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_mkdump nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_mkpaging nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_mkpfile nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_mkroot nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_mkspot nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_niminfo nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_nimpush nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_popxop nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_rmdir nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_rmspot nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_script nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_sh_lib nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_stat nim-method
F 0 0 555	/usr/lpp/bos.sysmgt/nim/methods/c_sync_root nim-method
#
# ----------------------- awk scripts    ------------------------------
#
F 0 0 555	/usr/lpp/bos.sysmgt/nim/awk/lsnim_l.awk nim-script
F 0 0 555	/usr/lpp/bos.sysmgt/nim/awk/lsnim_L.awk nim-script
F 0 0 555	/usr/lpp/bos.sysmgt/nim/awk/lsnim_P.awk nim-script
F 0 0 555	/usr/lpp/bos.sysmgt/nim/awk/lsnim_pa.awk nim-script
F 0 0 555	/usr/lpp/bos.sysmgt/nim/awk/popxop.awk nim-script
F 0 0 555	/usr/lpp/bos.sysmgt/nim/awk/cutfs.awk nim-script
F 0 0 555	/usr/lpp/bos.sysmgt/nim/awk/image_data.awk nim-script
#
