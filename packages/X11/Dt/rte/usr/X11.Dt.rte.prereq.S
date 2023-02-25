# @(#)33        1.19  src/packages/X11/Dt/rte/usr/X11.Dt.rte.prereq.S, pkgxcde, pkg41J, 9523B_all 6/6/95 14:13:47
#
#   COMPONENT_NAME: pkgxcde
#
#   FUNCTIONS: prereq
#
#   ORIGINS: 27
#
#   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
#   combined with the aggregated modules for this product)
#                    SOURCE MATERIALS
#
#   (C) COPYRIGHT International Business Machines Corp. 1993, 1994
#   All Rights Reserved
#   US Government Users Restricted Rights - Use, duplication or
#   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
#
#       [type] - *prereq, *coreq, *ifreq
#       [option] - full name of option with requisite
#       [expression]    "=","<",">"
#               v - version
#               r - release
#               m - modification number
#               f - fix number
#               p - update of PTF number
#       {o [expression]} - or condition for the option
#[type] [option] [expression] {o [expression]
#
#*prereq bos 4.1.0.0
*prereq X11.base.rte 4.1.0.0
*coreq X11.base.lib 4.1.0.0
*prereq X11.motif.lib 4.1.3.0
*prereq X11.motif.mwm 4.1.3.0
*coreq X11.apps.aixterm 4.1.0.0
*coreq X11.apps.rte 4.1.0.0
*coreq X11.apps.xterm 4.1.0.0
*coreq X11.apps.clients 4.1.0.0
*coreq X11.apps.custom 4.1.0.0
*coreq X11.Dt.helprun 4.1.3.0
*coreq X11.Dt.lib 4.1.3.0
*coreq X11.Dt.ToolTalk 4.1.3.0
*coreq X11.Dt.bitmaps 4.1.3.0
*coreq X11.vsm.rte 4.1.0.0
*coreq bos.txt.tfs 4.1.0.0
*coreq bos.txt.spell 4.1.0.0
*coreq X11.fnt.iso1 4.1.0.0
*coreq X11.fnt.defaultFonts 4.1.0.0
*coreq bos.terminfo.ibm.data 4.1.0.0
*ifreq X11.msg.De_DE.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.msg.En_US.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.msg.Es_ES.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.msg.Fr_FR.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.msg.It_IT.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.msg.Ja_JP.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.msg.Nl_BE.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.msg.No_NO.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.msg.Sv_SE.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.msg.de_DE.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.msg.en_US.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.msg.es_ES.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.msg.fr_FR.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.msg.it_IT.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.msg.ja_JP.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.msg.ko_KR.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.msg.nl_BE.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.msg.no_NO.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.msg.sv_SE.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.msg.zh_TW.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.Ar_AA.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.Da_DK.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.De_CH.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.De_DE.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.En_GB.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.En_US.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.Es_ES.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.Fi_FI.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.Fr_BE.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.Fr_CA.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.Fr_CH.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.Fr_FR.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.Is_IS.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.It_IT.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.Iw_IL.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.Ja_JP.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.Nl_BE.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.Nl_NL.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.No_NO.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.Pt_PT.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.Sv_SE.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.ar_AA.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.bg_BG.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.cs_CZ.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.da_DK.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.de_CH.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.de_DE.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.el_GR.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.en_GB.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.en_US.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.es_ES.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.fi_FI.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.fr_BE.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.fr_CA.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.fr_CH.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.fr_FR.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.hr_HR.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.hu_HU.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.is_IS.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.it_IT.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.iw_IL.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.ja_JP.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.ko_KR.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.mk_MK.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.nl_BE.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.nl_NL.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.no_NO.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.pl_PL.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.pt_BR.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.pt_PT.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.ro_RO.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.ru_RU.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.sh_SP.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.sk_SK.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.sl_SI.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.sr_SP.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.sv_SE.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.tr_TR.Dt.rte (4.1.0.0) 4.1.3.0
*ifreq X11.loc.zh_TW.Dt.rte (4.1.0.0) 4.1.3.0
