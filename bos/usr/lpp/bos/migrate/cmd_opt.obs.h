/* @(#)05  1.5  src/bos/usr/lpp/bos/migrate/cmd_opt.obs.h, cmdsmit, bos411, 9428A410j 6/30/94 15:10:53 */
/*
 *   COMPONENT_NAME: CMDSMIT
 *
 *   FUNCTIONS: none
 *
 *   PURPOSE: obsolete sm_cmd_opt entries from 3.2
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

static struct cmd_opt_list
   {
        char *id;
   } obs_cmds [] =
        {
	"128psync_chg",
	"128psync_ln_opt",
	"128psync_parent_opt",
	"128psync_predef_opt",
	"3270_chg",
	"3270_ln_opt",
	"Amd_delete_args",
	"Apc_schedule_body",
	"Funct_opt",
	"RCS__chgslip",
	"RCS__chlnk_prof_slip_HCON",
	"RCS__chlnk_prof_slip_telnet",
	"RCS__chlnk_prof_sna_HCON",
	"RCS__chlnk_prof_tcp_HCON",
	"RCS__chlnk_prof_tcp_telnet",
	"RCS__disconnslip",
	"RCS__get_slip",
	"RCS__ibmprof_logon_slip_HCON",
	"RCS__ibmprof_logon_slip_telnet",
	"RCS__ibmprof_logon_sna",
	"RCS__ibmprof_logon_tcp_HCON",
	"RCS__ibmprof_logon_tcp_telnet",
	"RCS__link_emul_SNA",
	"RCS__linkcontype",
	"RCS__linkucfg_slip_HCON",
	"RCS__linkucfg_slip_tel",
	"RCS__linkucfg_sna",
	"RCS__linkucfg_tcp_HCON",
	"RCS__linkucfg_tcp_telnet",
	"RCS__makeconnection",
	"RCS__mkusrprof_1",
	"RCS__mkusrprof_2",
	"RCS__mkusrprof_ibm_slip_HCON",
	"RCS__mkusrprof_ibm_slip_telnet",
	"RCS__mkusrprof_ibm_tcp_HCON",
	"RCS__mkusrprof_ibm_tcp_telnet",
	"RCS__reconfig_slip",
	"RCS__showslips",
	"RCS__sup6contype",
	"RCS__sup6login_SLIP",
	"RCS__sup6login_TCP/IP",
	"RCS__sup6login_sel",
	"RCS__sup6ucfg_SLIP",
	"RCS__sup6ucfg_TCP/IP",
	"RCS__usrfuncquery",
	"RCS_ch_conn_sel",
	"RCS_chgslipcon",
	"RCS_chusr_name_sel",
	"RCS_conn_sel_lnk",
	"RCS_cs_usr_profs_slip",
	"RCS_cs_usr_profs_tcp",
	"RCS_echo_active_slips",
	"RCS_emul_link_slip",
	"RCS_getibmserv",
	"RCS_ghost_cmd_opt",
	"RCS_link_conn_type",
	"RCS_link_emul_TCP/IP",
	"RCS_link_emul_ibm_SLIP",
	"RCS_link_emul_sel",
	"RCS_mkprof_hc",
	"RCS_mkslipcon",
	"RCS_rcs_select_name",
	"RCS_rcs_serv_sel",
	"RCS_rcs_service",
	"RCS_rm_name_sel",
	"RCS_rm_serv_sel",
	"RCS_rm_this_slip",
	"RCS_rmusrprof1",
	"RCS_rmusrprof2",
	"RCS_rmusrprof3",
	"RCS_rmusrprof4",
	"RCS_rmusrprof5",
	"RCS_sel_emul_slip",
	"RCS_sel_emul_tcp",
	"RCS_select_slip",
	"RCS_sub_sys_disconn",
	"RCS_sup6conn",
	"RCS_what_slip_to_rm",
	"RCS_whichslip",
	"_add_snmp_comun",
	"_chen",
	"_chfc",
	"_chfi",
	"_chgated",
	"_chhostent",
	"_chhostent1",
	"_chif",
	"_chif1",
	"_chifdriveren",
	"_chifdriverfc",
	"_chifdriverfi",
	"_chifdrivertr",
	"_chifdriverx2",
	"_chifen",
	"_chiffc",
	"_chiffi",
	"_chifso",
	"_chifsx25",
	"_chiftr",
	"_chifx25",
	"_chinet",
	"_chinetca",
	"_chinetd",
	"_chinetdconf",
	"_chinetdconf1",
	"_chineten",
	"_chinetfc",
	"_chinetfi",
	"_chinetsl",
	"_chinetso",
	"_chinettr",
	"_chinetxs",
	"_chinetxt",
	"_chitab",
	"_chkbd",
	"_chnamed",
	"_chrhp",
	"_chrouted",
	"_chservices",
	"_chservices1",
	"_chsx2",
	"_chsx25",
	"_chsx25_1",
	"_chsx25_2",
	"_chsyslogd",
	"_chtimed",
	"_chtr",
	"_chx2",
	"_chx25",
	"_chx25_1",
	"_chx25_2",
	"_cmdlvmchlv",
	"_cmdlvmchpv",
	"_cmdlvmchvg",
	"_cmdlvmcplv",
	"_cmdlvmexportvg",
	"_cmdlvmextendlv",
	"_cmdlvmextendvg",
	"_cmdlvmimportvg",
	"_cmdlvmlslv",
	"_cmdlvmlspv",
	"_cmdlvmlsvg",
	"_cmdlvmlsvg2",
	"_cmdlvmlvns",
	"_cmdlvmmigratepv",
	"_cmdlvmmklv",
	"_cmdlvmmklvcopy",
	"_cmdlvmmkvg",
	"_cmdlvmpvns",
	"_cmdlvmreducevg",
	"_cmdlvmrenamelv",
	"_cmdlvmreorgvg",
	"_cmdlvmrmlv",
	"_cmdlvmrmlvcopy",
	"_cmdlvmrmvg",
	"_cmdlvmspvns",
	"_cmdlvmvaryoffvg",
	"_cmdlvmvaryonvg",
	"_cmdlvmvgns",
	"_fshrttbl",
	"_lshostent",
	"_mkdomain",
	"_mkftpusers",
	"_mkhostent",
	"_mkhostname",
	"_mkhostsequiv",
	"_mkhostslpd",
	"_mkinet1ca",
	"_mkinet1en",
	"_mkinet1et",
	"_mkinet1fc",
	"_mkinet1fi",
	"_mkinet1sl",
	"_mkinet1so",
	"_mkinet1tr",
	"_mkinet1xs",
	"_mkinet1xt",
	"_mkinetca",
	"_mkinetdconf",
	"_mkinetdconf1",
	"_mkineten",
	"_mkinetet",
	"_mkinetfc",
	"_mkinetfi",
	"_mkinetsl",
	"_mkinetso",
	"_mkinettr",
	"_mkinetxs",
	"_mkinetxt",
	"_mklque_opts",
	"_mknamerslv",
	"_mkquedev_opts",
	"_mkroute",
	"_mkrque_opts",
	"_mkservices",
	"_mksx25p",
	"_mksx25s",
	"_mktcpip1",
	"_mktcpipca",
	"_mktcpipen",
	"_mktcpiptr",
	"_mktcpipxt",
	"_mkx25p",
	"_mkx25s",
	"_nfs.chnfs",
	"_nfs.chnfsexp",
	"_nfs.chnfsexp.select",
	"_nfs.chnfsmnt",
	"_nfs.chnfsmnt.select",
	"_nfs.mkautomnt",
	"_nfs.mknfs",
	"_nfs.mknfsexp",
	"_nfs.mknfsmnt",
	"_nfs.rmnfs",
	"_nfs.rmnfsexp",
	"_nfs.rmnfsexp.select",
	"_nfs.rmnfsmnt",
	"_nfs.rmnfsmnt.select",
	"_off_chsnmp_status",
	"_on_chsnmp_status",
	"_qpri_printjob",
	"_rmen",
	"_rmfc",
	"_rmfi",
	"_rmftpusers",
	"_rmhostent1",
	"_rmhostsequiv",
	"_rmhostslpd",
	"_rmi1",
	"_rminet",
	"_rminetdconf",
	"_rminetdconf1",
	"_rmnamerslv",
	"_rmque_opts",
	"_rmquedev_opts",
	"_rmroute",
	"_rmservices",
	"_rmservices1",
	"_rmsx2",
	"_rmsx25",
	"_rmsx25_1",
	"_rmtr",
	"_rmx2",
	"_rmx25",
	"_rmx25_1",
	"_rpc.newkey",
	"_setbootup_option",
	"_shsnmp_comun",
	"_shsnmp_status",
	"_spnamerslv",
	"_startkeyserv",
	"_stnamerslv1",
	"_stnamerslv2",
	"_stopkeyserv",
	"_terminal",
	"_yp.chmaster",
	"_yp.chslave",
	"_yp.chypdom",
	"_yp.getmaps",
	"_yp.mkclient",
	"_yp.mkmaps",
	"_yp.mkmaster",
	"_yp.mkslave",
	"_yp.rmypclnt",
	"_yp.rmypserv",
	"_yp.transmaps",
	"acp_chg",
	"acp_ln_opt",
	"add_snmp_comun_chg_opt",
	"add_snmp_nms_cmd",
	"add_snmp_nms_cmd_opt",
	"add_snmp_nms_opt",
	"add_snmp_traps_cmd",
	"add_snmp_traps_cmd_opt",
	"add_snmp_traps_opt",
	"ascsi_chg",
	"atOpts",
	"backforg_2",
	"backupOpts",
	"bffcreate.cmd_option",
	"cat",
	"cat_chg",
	"cat_ln_opt",
	"catadap",
	"catalog_opt",
	"cdchfsCmdOpt",
	"cdchfsNameOpt",
	"cdcrfsOpt",
	"cdrmfsOpt",
	"cdrom_add",
	"cdrom_chg",
	"cdrom_common",
	"cdrom_ln_opt",
	"cdrom_oscd_common",
	"cdrom_predef_opt",
	"cfg128psync_opt",
	"cfg_fcs_dd_opt",
	"cfg_fcsdd_opt",
	"cfg_scbndd_nopt",
	"cfg_scbndd_opt",
	"cfg_sdlcmpa_cfg",
	"cfg_x25a_cfg",
	"cfgcdr_opt",
	"cfgdials_opt",
	"cfgdsk_opt",
	"cfgdskt_opt",
	"cfgmp_opt",
	"cfgmpdd_opt",
	"cfgmsla_opt",
	"cfgops",
	"cfgprt_opt",
	"cfgtmscsi_opt",
	"cfgtpe_opt",
	"cfgtty_opt",
	"ch_sdlcmpa_ch",
	"ch_sdlcmpa_l",
	"chclick_1",
	"chcons_1",
	"chcursor_1",
	"chdclient",
	"chdisp_1",
	"chdispsz_dial",
	"chdispsz_sel",
	"chdspot",
	"chfont_1",
	"chfont_n8",
	"chfontpl_8",
	"chg_fcs_dd_opt",
	"chg_fcsdd_opt",
	"chg_scbndd_nopt",
	"chg_scbndd_opt",
	"chgaioCmdOpt",
	"chgated",
	"chgcat",
	"chgcat_hdr_pca_mca",
	"chghdr_mpa_mca",
	"chgmpa",
	"chgops",
	"chgprt_hdr_parallel.opt",
	"chgprt_hdr_rs232.opt",
	"chgprt_hdr_rs422.opt",
	"chgsys_opt",
	"chgtty_hw",
	"chgtty_hw.opt",
	"chgtty_hw_lion_local",
	"chgtty_hw_rs_local",
	"chgtty_nls",
	"chgtty_nls_diag.opt",
	"chgtty_posix",
	"chgtty_posix_diag.opt",
	"chgtty_program",
	"chgtty_program_diag.opt",
	"chgtty_route_to_dialog.opt",
	"chgtty_task_selector.opt",
	"chhostent",
	"chhostent1",
	"chhwkbd_2",
	"chif",
	"chif1",
	"chifdriveren",
	"chifdriverfc",
	"chifdriverfi",
	"chifdrivertr",
	"chifdriverx2",
	"chifen",
	"chiffc",
	"chiffi",
	"chifso",
	"chiftr",
	"chifx25",
	"chinetca",
	"chinetd",
	"chinetdconf",
	"chinetdconf1",
	"chineten",
	"chinetfc",
	"chinetfi",
	"chinetsl",
	"chinetso",
	"chinettr",
	"chinetxt",
	"chinterface",
	"chinterface.ns",
	"chlang_env1",
	"chlicense",
	"chpsCmdOpt",
	"chpsNameOpt",
	"chqueNameOpt",
	"chqueOpts",
	"chquedevNameOpt0",
	"chquedevNameOpt1",
	"chquedevOpts",
	"chrefrate_dial",
	"chrefrate_sel",
	"chrhp_chg_ntx2m_opt",
	"chrhp_chg_ntx8m_opt",
	"chrouted",
	"chservices",
	"chservices1",
	"chsnmp_status",
	"chsound_2",
	"chsx25_1",
	"chsx25_2",
	"chsyslogd",
	"chtimed",
	"chtz_cmd_opt_a",
	"chtz_cmd_opt_no",
	"chtz_cmd_opt_yes",
	"chtz_opts",
	"chtz_opts_2",
	"chx25_1",
	"chx25_2",
	"client_select",
	"cmddlc_ether_infoc",
	"cmddlc_ether_infop",
	"cmddlc_ether_name_ls",
	"cmddlc_ether_name_mk",
	"cmddlc_ether_name_rm",
	"cmddlc_fddi_infoc",
	"cmddlc_fddi_infop",
	"cmddlc_fddi_name_ls",
	"cmddlc_fddi_name_mk",
	"cmddlc_fddi_name_rm",
	"cmddlc_qllc_infoc",
	"cmddlc_qllc_infop",
	"cmddlc_qllc_name_ls",
	"cmddlc_qllc_name_mk",
	"cmddlc_qllc_name_rm",
	"cmddlc_sdlc_infoc",
	"cmddlc_sdlc_infop",
	"cmddlc_sdlc_name_ls",
	"cmddlc_sdlc_name_mk",
	"cmddlc_sdlc_name_rm",
	"cmddlc_token_infoc",
	"cmddlc_token_infop",
	"cmddlc_token_name_ls",
	"cmddlc_token_name_mk",
	"cmddlc_token_name_rm",
	"commit",
	"commodev",
	"corvette_common_chg",
	"cowstop_id",
	"cx25str_dd_a_sel_cmd",
	"cx25str_dd_c_cmd",
	"cx25str_dd_c_opt",
	"cx25str_dd_l_cmd",
	"cx25str_dd_r_opt",
	"cx25str_mp_sel_parent",
	"dacc_adddev.do",
	"dacc_configdev.do",
	"dacc_pick_device",
	"dacc_pickparent_device",
	"dacc_remdev.select",
	"date",
	"dateOpts",
	"date_dial",
	"date_sel_opt",
	"ddaemon",
	"device",
	"dials_add",
	"dials_chg",
	"dials_ln_opt",
	"dials_predef_opt",
	"digi_tty_special",
	"disk_chg",
	"disk_ln_opt",
	"disk_mca_add",
	"disk_mca_chg",
	"disk_osdisk_scsi_add",
	"disk_osdisk_scsi_chg",
	"disk_predef_opt",
	"disk_scsi_add",
	"disk_scsi_chg",
	"disk_serial_chg",
	"diskless_cklevels",
	"diskless_cklevels.cmd_option",
	"diskless_cklevels.name_select",
	"diskless_cleanup",
	"diskless_commit",
	"diskless_dependents",
	"diskless_enable",
	"diskless_files",
	"diskless_history",
	"diskless_install",
	"diskless_install_commit.name_select",
	"diskless_install_install.cmd_option",
	"diskless_install_install.name_select1",
	"diskless_install_install.name_select2",
	"diskless_install_instupdt.cmd_option",
	"diskless_install_instupdt.name_select1",
	"diskless_install_instupdt.name_select2",
	"diskless_install_list_applied.name_select",
	"diskless_install_reject.name_select",
	"diskless_install_remove.name_select",
	"diskless_install_update.cmd_option",
	"diskless_install_update.name_select1",
	"diskless_install_update.name_select2",
	"diskless_instupdt",
	"diskless_instupdt_commit.name_select",
	"diskless_instupdt_install.name_select1",
	"diskless_instupdt_install.name_select2",
	"diskless_instupdt_instupdt.name_select1",
	"diskless_instupdt_instupdt.name_select2",
	"diskless_instupdt_pending.name_select",
	"diskless_instupdt_reject.name_select",
	"diskless_instupdt_remove.name_select",
	"diskless_instupdt_update.name_select1",
	"diskless_instupdt_update.name_select2",
	"diskless_list",
	"diskless_list_applied.cmd_option",
	"diskless_list_applied.name_select",
	"diskless_listupdt",
	"diskless_lslpp_dependents.cmd_option",
	"diskless_lslpp_dependents.name_select",
	"diskless_lslpp_files.cmd_option",
	"diskless_lslpp_files.name_select",
	"diskless_lslpp_history.cmd_option",
	"diskless_lslpp_history.name_select",
	"diskless_lslpp_id.cmd_option",
	"diskless_lslpp_id.name_select",
	"diskless_lslpp_installed.cmd_option",
	"diskless_lslpp_installed.name_select",
	"diskless_lslpp_maintenance.cmd_option",
	"diskless_lslpp_maintenance.name_select",
	"diskless_lslpp_prereq.cmd_option",
	"diskless_lslpp_prereq.name_select",
	"diskless_pending",
	"diskless_pending_vpd",
	"diskless_pid",
	"diskless_prereqs",
	"diskless_reject",
	"diskless_remove",
	"diskless_update",
	"diskless_verify",
	"diskless_verify.cmd_option",
	"diskless_verify.name_select",
	"diskless_vpd_cklevels.name_select",
	"diskless_vpd_dependents.name_select",
	"diskless_vpd_files.name_select",
	"diskless_vpd_history.name_select",
	"diskless_vpd_list.name_select",
	"diskless_vpd_listupdt.name_select",
	"diskless_vpd_pending.name_select",
	"diskless_vpd_pid.name_select",
	"diskless_vpd_prereqs.name_select",
	"diskless_vpd_verify.name_select",
	"dskt_add",
	"dskt_chg",
	"dskt_common",
	"dskt_ln_opt",
	"dump_change1",
	"dump_change2",
	"dump_copy_dskt",
	"dump_copy_dskt_not_2",
	"dump_copy_file",
	"dump_copy_file_not_2",
	"dump_format",
	"dumpfmt_filename",
	"dumpfmt_opt",
	"dumpfmt_printer",
	"enet_chg",
	"enet_chg_ethernet",
	"enet_ln_opt",
	"errclear",
	"errdemon",
	"errpt",
	"errpt_1",
	"errpt_2",
	"errpt_cprinter",
	"errpt_filename",
	"errpt_ghost",
	"errpt_printer",
	"errpt_sel.1st",
	"fcs_hdr_chg",
	"fcs_ln_opt",
	"fddi_chg",
	"fddi_ln_opt",
	"fsckOpts",
	"fshrttbl",
	"get_device",
	"get_device.bffcreate",
	"get_fware_fname",
	"get_ldevname",
	"get_ldevname2",
	"get_ldevname3",
	"get_ldevname4",
	"get_ldevname5",
	"group_add",
	"group_change",
	"group_change.name_select",
	"group_remove",
	"gsw_chg",
	"hia_chg",
	"hia_common",
	"hscsi_chg",
	"iconvOpts",
	"install_all.cmd_option",
	"install_commit",
	"install_enhancements.cmd_option",
	"install_latest.cmd_option",
	"install_list.cmd_option",
	"install_list_problems.cmd_option",
	"install_maintenance.cmd_option",
	"install_reject",
	"install_remove",
	"install_selectable_all.cmd_option",
	"install_subsystems.cmd_option",
	"install_verify.cmd_option",
	"instupdt",
	"instupdt.inst",
	"instupdt.install.name_select",
	"instupdt.instupdt.name_select",
	"instupdt.list",
	"instupdt.list.name_select",
	"instupdt.update.name_select",
	"instupdt.updt",
	"instupdt_commit",
	"instupdt_for_net",
	"instupdt_for_net.name_select",
	"instupdt_list_problems",
	"instupdt_list_problems.ns",
	"instupdt_reject",
	"instupdt_remove",
	"iostat_o",
	"jchfsCmdOpt",
	"jchfsNameOpt",
	"jcrfsCmdOpt",
	"jcrfsNameOpt",
	"jcrfslvCmdOpt",
	"jrmfsOpt",
	"killOpts",
	"listdclick_id",
	"ln_sdlcmpa_opt",
	"ln_sdlcmpa_opt2",
	"ln_sdlcmpa_opt3",
	"ln_x25a_opt",
	"lnopt",
	"lppchk",
	"ls_ac",
	"ls_ac2",
	"ls_array",
	"ls_array_2",
	"ls_array_3",
	"ls_array_4",
	"ls_dacs",
	"lsallqdevOpt",
	"lsattrd_hdr_opt",
	"lsattrd_opt",
	"lsattrs_1_opt",
	"lsattrs_2_opt",
	"lsattrs_hdr_opt",
	"lsattrs_opt",
	"lscat",
	"lsdswconfig.opt",
	"lslpp_dependents",
	"lslpp_dependents.cmd_option",
	"lslpp_files",
	"lslpp_files.cmd_option",
	"lslpp_history",
	"lslpp_history.cmd_option",
	"lslpp_id.cmd_option",
	"lslpp_installed.cmd_option",
	"lslpp_listupdt",
	"lslpp_maintenance.cmd_option",
	"lslpp_pid",
	"lslpp_prereq.cmd_option",
	"lslpp_prereqs",
	"lsmpa",
	"lsswconfig.opt",
	"makcdr_parent_opt",
	"makdials_parent_opt",
	"makdsk_parent_opt",
	"makprt_hdr_parallel.opt",
	"makprt_hdr_rs232.opt",
	"makprt_hdr_rs422.opt",
	"makprt_parent_opt",
	"maktmscsi_parent_opt",
	"maktpe_parent_opt",
	"maktty_hdr.opt",
	"maktty_parent_opt",
	"mk_fcs_dd_opt",
	"mk_fcsdd_opt",
	"mk_scbndd_nopt",
	"mk_scbndd_opt",
	"mk_sdlcmpa_mk",
	"mk_x25a_opt",
	"mkdclient",
	"mkdomain",
	"mkfont_2",
	"mkhostent",
	"mkhostname",
	"mkhostsequiv",
	"mkhostslpd",
	"mkinet1ca",
	"mkinet1en",
	"mkinet1et",
	"mkinet1fc",
	"mkinet1fi",
	"mkinet1sl",
	"mkinet1so",
	"mkinet1tr",
	"mkinet1xt",
	"mkinetca",
	"mkinetdconf",
	"mkinetdconf1",
	"mkineten",
	"mkinetet",
	"mkinetfc",
	"mkinetfi",
	"mkinetsl",
	"mkinetso",
	"mkinettr",
	"mkinetxt",
	"mkkbd_2",
	"mkpsCmdOpt",
	"mkpsNameOpt",
	"mkroute",
	"mkservices",
	"mkspot",
	"mksx25s",
	"mksysbOpts",
	"mkx25p",
	"mkx25s",
	"mountOpts",
	"mountgOpts",
	"mp_add",
	"mp_chg",
	"mp_common",
	"mp_ln_opt",
	"mp_lndd_opt",
	"mp_mk_parent",
	"mp_mkdd_parent",
	"mp_mv",
	"mp_mv_parent",
	"mpa",
	"mpa_add",
	"mpaadap",
	"mpachg",
	"msla_add",
	"msla_ln_opt",
	"msla_mk_mode",
	"msla_mk_parent",
	"mvdswconfig.opt",
	"mvswconfig.opt",
	"nice_o",
	"ntx_snmp_communities_menu",
	"ntx_snmp_nms_menu",
	"ntx_snmp_traps_menu",
	"off_chsnmp_status_chg",
	"off_chsnmp_status_chg_opt",
	"on_chsnmp_status_chg",
	"on_chsnmp_status_chg_opt",
	"palettevalues_16",
	"parity_chkrepair",
	"passwd",
	"pid.subsys.opt",
	"prereq",
	"prt_add",
	"prt_chg",
	"prt_common",
	"prt_ln_opt",
	"prt_mv",
	"prt_mv_parent",
	"prt_parallel",
	"prt_predef_class_opt",
	"prt_predef_opt",
	"prt_serial",
	"prt_serial_cxmadd",
	"pty_attr",
	"qcanOpts",
	"qchkOpts",
	"qprtOpts",
	"qstartOpts",
	"qstopOpts",
	"query.type.opt",
	"react_chshdev.select",
	"react_configdev.select",
	"react_pick_device",
	"react_remdev.select",
	"remove_snmp_nms_cmd",
	"remove_snmp_nms_cmd_opt",
	"remove_snmp_nms_lst",
	"remove_snmp_nms_lst_opt",
	"remove_snmp_nms_opt",
	"remove_snmp_traps_cmd",
	"remove_snmp_traps_cmd_opt",
	"remove_snmp_traps_lst",
	"remove_snmp_traps_lst_opt",
	"remove_snmp_traps_opt",
	"renice_o",
	"restoreOpts",
	"rm_fcs_dd_opt",
	"rm_fcsdd_opt",
	"rm_scbndd_nopt",
	"rm_scbndd_opt",
	"rm_sdlcmpa_rm",
	"rm_x25a_rm",
	"rmatOpts",
	"rmdclient",
	"rmftpusers",
	"rmhostent",
	"rmhostent1",
	"rmhostsequiv",
	"rmhostslpd",
	"rmi1",
	"rminetd_boot",
	"rminetd_both",
	"rminetd_now",
	"rminetdconf",
	"rminetdconf1",
	"rmnamerslv",
	"rmpsOpt",
	"rmquedevNameOpt",
	"rmroute",
	"rmservices",
	"rmservices1",
	"rmso",
	"rmspot",
	"rmsx25",
	"rmsx25_1",
	"rmtr",
	"rmvcdr_opt",
	"rmvdials_opt",
	"rmvdsk_opt",
	"rmvdskt_opt",
	"rmvmp_opt",
	"rmvmpdd_opt",
	"rmvmsla_opt",
	"rmvprt_opt",
	"rmvtmscsi_opt",
	"rmvtpe_opt",
	"rmvtty_opt",
	"rmx2",
	"rmx25",
	"rmx25_1",
	"rsdswconfig.opt",
	"rsdswconfig_device.name_select",
	"rsswconfig.opt",
	"rsswconfig_device.name_select",
	"rucklevels",
	"rulppchk",
	"sarOpts",
	"scsi_common_chg",
	"scsi_ln_opt",
	"select.subsvr.name.opt",
	"sh_snmp_nms_cmd",
	"sh_snmp_nms_cmd_opt",
	"sh_snmp_nms_opt",
	"sh_snmp_traps_cmd",
	"sh_snmp_traps_cmd_opt",
	"sh_snmp_traps_opt",
	"shsnmp_comun_sh_chg_opt",
	"shsnmp_comun_sh_opt",
	"shsnmp_status_chg",
	"shsnmp_status_chg_opt",
	"shutdownOpts",
	"spgated_boot",
	"spgated_both",
	"spgated_now",
	"spnamed_boot",
	"spnamed_both",
	"spnamed_now",
	"spnamerslv",
	"spot_select",
	"spotclient",
	"spotname",
	"sprouted_boot",
	"sprouted_both",
	"sprouted_now",
	"spsyslogd_boot",
	"spsyslogd_both",
	"spsyslogd_now",
	"sptimed_boot",
	"sptimed_both",
	"sptimed_now",
	"start.trace.opt",
	"stnamerslv1",
	"stnamerslv2",
	"stopserver.opt",
	"stopserver.trace.cmd",
	"stopssys.opt",
	"subsvr.name.opt",
	"subsys.name.opt",
	"subsys.pid.opt",
	"svdswconfig.name_select",
	"svdswconfig.opt",
	"svswconfig.name_select",
	"svswconfig.opt",
	"swaponOpt",
	"swcons_1",
	"swdisp_1",
	"swkbd_2",
	"tape_1200mb-c",
	"tape_3490e",
	"tape_4mm2gb",
	"tape_4mm4gb",
	"tape_525mb",
	"tape_8mm",
	"tape_8mm5gb",
	"tape_9trk",
	"tape_add",
	"tape_chg",
	"tape_common",
	"tape_ln_opt",
	"tape_ost",
	"tape_predef_opt",
	"tape_quarter",
	"telinitOpts",
	"timex_o",
	"tmscsi_add",
	"tmscsi_chg",
	"tmscsi_common",
	"tmscsi_ln_opt",
	"tmscsi_predef_opt",
	"tochar_5",
	"tocharnnsp_5",
	"tofunction_5",
	"tok_chg",
	"tok_ln_opt",
	"tostring_5",
	"trcrpt",
	"trcrpt_filename",
	"trcrpt_printer",
	"trcrpt_sel",
	"trcstart",
	"trcstop.N",
	"ts7318_cs_add_opt",
	"ts7318_cs_chg_opt",
	"ts7318_cs_chg_server_opt",
	"ts7318_cs_remove_opt",
	"ts7318_cs_remove_server_opt",
	"ts7318_port_buddy_opt",
	"ts7318_port_buddy_server_opt",
	"ts7318_port_hibaud_opt",
	"ts7318_port_hibaud_server_opt",
	"ts7318_port_slew_opt",
	"ts7318_port_slew_server_opt",
	"ts7318_prt_add_intf_opt",
	"ts7318_prt_add_opt",
	"ts7318_prt_add_par_opt",
	"ts7318_prt_add_par_port_opt",
	"ts7318_prt_add_printer_opt",
	"ts7318_prt_add_ser_opt",
	"ts7318_prt_add_ser_port_opt",
	"ts7318_prt_add_ser_type_opt",
	"ts7318_prt_add_server_opt",
	"ts7318_prt_add_tprt_opt",
	"ts7318_prt_chg_opt",
	"ts7318_prt_chg_printer_opt",
	"ts7318_prt_chg_server_opt",
	"ts7318_prt_chg_tprt_opt",
	"ts7318_prt_chg_type_opt",
	"ts7318_prt_remove_opt",
	"ts7318_prt_remove_printer_opt",
	"ts7318_prt_remove_server_opt",
	"ts7318_tty_add_TTY_opt",
	"ts7318_tty_add_opt",
	"ts7318_tty_add_port_opt",
	"ts7318_tty_add_server_opt",
	"ts7318_tty_add_type_opt",
	"ts7318_tty_chg_opt",
	"ts7318_tty_chg_server_opt",
	"ts7318_tty_chg_tty_opt",
	"ts7318_tty_remove_opt",
	"ts7318_tty_remove_server_opt",
	"ts7318_tty_remove_tty_opt",
	"tty_add",
	"tty_chg",
	"tty_common",
	"tty_ln_opt",
	"tty_mv",
	"tty_mv_parent",
	"tty_predef_opt",
	"umountfsOpts",
	"umountgOpts",
	"unameOpts",
	"user_add",
	"user_change",
	"user_change.name_select",
	"user_remove",
	"vca_chg",
	"vca_ln_opt",
	"vmstat_o",
	"vscsi_chg",
	"wallOpts",
	"x25_attr_fr",
	"x25_attr_gen",
	"x25_attr_init",
	"x25_attr_net",
	"x25_attr_pack",
	"x25_attr_pvc",
	"x25_attr_pvc_1",
	"x25_attr_pvc_10",
	"x25_attr_pvc_11",
	"x25_attr_pvc_12",
	"x25_attr_pvc_13",
	"x25_attr_pvc_14",
	"x25_attr_pvc_15",
	"x25_attr_pvc_16",
	"x25_attr_pvc_17",
	"x25_attr_pvc_18",
	"x25_attr_pvc_19",
	"x25_attr_pvc_2",
	"x25_attr_pvc_20",
	"x25_attr_pvc_21",
	"x25_attr_pvc_22",
	"x25_attr_pvc_23",
	"x25_attr_pvc_24",
	"x25_attr_pvc_25",
	"x25_attr_pvc_26",
	"x25_attr_pvc_27",
	"x25_attr_pvc_28",
	"x25_attr_pvc_29",
	"x25_attr_pvc_3",
	"x25_attr_pvc_30",
	"x25_attr_pvc_31",
	"x25_attr_pvc_32",
	"x25_attr_pvc_33",
	"x25_attr_pvc_34",
	"x25_attr_pvc_35",
	"x25_attr_pvc_36",
	"x25_attr_pvc_37",
	"x25_attr_pvc_38",
	"x25_attr_pvc_39",
	"x25_attr_pvc_4",
	"x25_attr_pvc_40",
	"x25_attr_pvc_41",
	"x25_attr_pvc_42",
	"x25_attr_pvc_43",
	"x25_attr_pvc_44",
	"x25_attr_pvc_45",
	"x25_attr_pvc_46",
	"x25_attr_pvc_47",
	"x25_attr_pvc_48",
	"x25_attr_pvc_49",
	"x25_attr_pvc_5",
	"x25_attr_pvc_50",
	"x25_attr_pvc_51",
	"x25_attr_pvc_52",
	"x25_attr_pvc_53",
	"x25_attr_pvc_54",
	"x25_attr_pvc_55",
	"x25_attr_pvc_56",
	"x25_attr_pvc_57",
	"x25_attr_pvc_58",
	"x25_attr_pvc_59",
	"x25_attr_pvc_6",
	"x25_attr_pvc_60",
	"x25_attr_pvc_61",
	"x25_attr_pvc_62",
	"x25_attr_pvc_63",
	"x25_attr_pvc_64",
	"x25_attr_pvc_7",
	"x25_attr_pvc_8",
	"x25_attr_pvc_9",
	"x25_list_pvc",
	"x25_listc",
	"x25str_a_cio_cmd",
	"x25str_a_cio_opt",
	"x25str_attr_fr",
	"x25str_attr_gen",
	"x25str_attr_pack",
	"x25str_attr_pvc",
	"x25str_attr_pvc_a",
	"x25str_attr_pvc_r",
	"x25str_attr_pvc_s",
	"x25str_dd_a_opt",
	"x25str_dd_a_sel_cmd",
	"x25str_dd_c_cmd",
	"x25str_dd_c_opt",
	"x25str_dd_l_cmd",
	"x25str_dd_r_opt",
	"x25str_mp_c_opt",
	"x25str_mp_common",
	"x25str_mp_dial_port",
	"x25str_mp_disp_parent",
	"x25str_mp_disp_port",
	"x25str_mp_r_opt",
	"x25str_mp_sel_dport",
	"x25str_mp_sel_parent",
	"x25str_mp_sel_port",
	"x25str_r_cio_cmd",
	"x25str_r_cio_opt",
	"x25str_r_if_cmd",
	"x25str_r_if_opt",
	"x_add_fs_fpe_one_d_opt",
	"x_add_nfs_fpe_one_d_opt",
	"x_add_trm_opt_120",
	"x_add_trm_opt_130",
	"x_add_trm_opt_140",
	"x_add_trm_opt_150",
	"x_add_xst_fpe_one_d_opt",
	"x_chg_def_set_opt_140",
	"x_chg_def_set_opt_150",
	"x_chg_net_d_opt",
	"x_chg_net_n_opt",
	"x_chg_trm_d_opt_120",
	"x_chg_trm_d_opt_130",
	"x_chg_trm_d_opt_140",
	"x_chg_trm_d_opt_150",
	"x_chg_trm_n_opt_120",
	"x_chg_trm_n_opt_130",
	"x_chg_trm_n_opt_140",
	"x_chg_trm_n_opt_150",
	"x_def_net_opt",
	"x_rest_def_opt",
	"x_rm_fpe_d_opt",
	"x_rm_fpe_n2_opt",
	"x_rm_fpe_n_opt",
	"x_rm_net_d_opt",
	"x_rm_net_n_opt",
	"x_rm_trm_d_opt",
	"x_rm_trm_n_opt"
	};

#define NUM_OBS_CMDS (sizeof(obs_cmds) / sizeof(struct cmd_opt_list))
