/* @(#)78  1.41  src/bos/usr/sbin/install/include/installp.h, cmdinstl, bos411, 9436D411a 9/7/94 15:04:27 */
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_INSTALLP
#define _H_INSTALLP

#include <fcntl.h>
#include <inulib.h>

typedef struct stats_table_values  stats_table;

/*--------------------------------------------------------------------------*
 * EXTERNAL FUNCTIONS
 *--------------------------------------------------------------------------*/

/* Not sure what is needed for audit for 3.2, (stage 2).
 * extern int
 * auditproc();
 * extern int      auditwrite();
 * extern int      kleenup(); */

char *NIM_usr_spot();

void inu_vpd_history (Option_t * op,
                      int        state,
                      int        event);

void inu_vpd_lpp_prod (Option_t * sop,
                       int        state);

void inu_vpd_set_states (Option_t * op,
                         int        action);

void inu_sig_abort (int);

void convert_vrmf_to_string (Option_t * lppopt);

int find_inv_file (char * inv_file,
                   char * lppname,
                   char * lppopt_name,
                   int    inv_file_type);

int inu_ckreq (Option_t * Option_Ptr,
               Option_t * Fail_SOP_Ptr,
               TOC_t    * TOC_ptr,
               boolean    verbose,
               boolean    Auto_Include,
               boolean    installp_update,
               short      parts_to_operate_on,
               boolean    called_from_cmd_line);

int inu_apply_opts (Option_t *, Option_t *);

int inu_mk_al_fil (Option_t *, Option_t *);

int inu_valid_al (Option_t *, char *);

int inu_bld_root_sop (Option_t **, Option_t *);

int inu_bld_sop (TOC_t * toc, Option_t *, Option_t *, Option_t *);

int inu_bld_sop_toc (TOC_t    * toc,
                     Option_t * sop,
                     char     * i_name,
                     Level_t  * level,
                     Option_t * failsop);

void inu_quit (int code);

void inu_remove_save_files (Option_t * sop,
                            Option_t * next_op);

void inu_remove_all_save_files (int tree,
                                Option_t * sop, 
                                Option_t * next_op);

void inu_clean_libdir (Option_t * sop,
                       Option_t * next_op);

void inu_create_anchors ();

boolean inu_remove_opts (TOC_t   * toc,
                     Option_t * sop,
                     Option_t * next_op);

int inu_cleanup_opts (Option_t *, Option_t *);

int inu_exec_installp (char * argv[]);

int inu_get_cntl_files (Option_t *, Option_t *);

int inu_get_31cntl_files (Option_t *, Option_t *);

void inu_get_optlevname_from_Option_t (Option_t *, char *);

void inu_get_savdir (char, Option_t *, char *);

int inu_level_ok (Option_t *);

void inu_31level_convert (char *, Level_t *);

int inu_level_chk_3_1 (Level_t *);

int inu_ls_apars (Option_t *, Option_t *);

void inu_ls_op_apars (Option_t * op);

void inu_ls_missing_info (Option_t * sop);

int inu_ls_instr (Option_t *);

int inu_ls_toc (Option_t *);

int inu_mk_opt_fil (Option_t *, Option_t *, int, char *);

int inu_putenv (char *);

int inu_run_sysck (char *);

int inu_reject_opts (Option_t *, Option_t *);

void inu_set_cur_oper (Option_t *, Option_t **);

int inu_set_env_vars (void);

void inu_set_libdir (char, Option_t *);

void inu_set_savdir (char, Option_t *);

void inu_set_vpdtree (char);

void inu_vpd_apply (Option_t *, int);

void inu_ck_vpd_ok     (int rc, char *, ... );
void inu_ck_vpd_sys_rc (int rc);

void get_corr_svn (hist_t * hist);

void get_prereq_file (Option_t * op,
                      prod_t   * prod);

void get_fixinfo_file (Option_t * op,
                       prod_t   * prod);

void get_productid_file (Option_t * op,
                         prod_t   * prod);

void inu_vpd_ch_sceded_by (char * sceded_by,
                           char * new_sceded_by,
                           char * lpp_name);

int inu_check_one (char *, int *, int);

int inu_valid_args (int *);

int inu_instl_args (int, char **, char *);

int inu_get_list (char *, char *, int);

int correct_usage (void);

int inu_do_status (Option_t *);

void inu_init_strings (void);

void inu_sort_sop (Option_t * sop, boolean  * error);

void inu_add_supersedes (Option_t * op);

Option_t  * grep_option_toc (Option_t *, char *, int, Level_t *, int *, char *);
Option_t  * conv_prod_t_Option_t (prod_t *);

int installp_eval_size (Option_t * sop, char     * Command);

void  inu_mark_dups_in_toc          (TOC_t * tp);
int   mv_F_ptfs_from_sop_to_savesop (Option_t *, Option_t *);
int   pull_in_ptfs_from_toc         (TOC_t *, Option_t *, int);
int   set_operation                 (Option_t *);
void  inu_rm_dups_from_sop          (Option_t *, int, TOC_t *, Option_t *);

int inu_run_command(char *, char **, int *);
int inu_bosboot_perform(void);
int inu_bosboot_verify(void);
int inu_bosboot_isneeded(Option_t *);
int inu_bosboot_ispending(void);
int inu_bosboot_setpending(int);

void inu_pseudo_ckreq (Option_t *  SopPtr,
                       Option_t ** start_op,
                       Option_t *  next_op);

void inu_set_graph_status           (Option_t * op, Option_t * next_op);
void inu_rm_sum_log                 (char *);
void rm_biron_files                 (Option_t *, Option_t *, int);
int  nothing_still_installed        (Option_t * op, int );
int  is_biron_set                   ();

void inu_calc_stats_table_values    (Option_t *, Option_t *, Option_t *,
                                     stats_table *);
void inu_show_stats_table           (Option_t *, Option_t *, Option_t *,
                                     stats_table *);
void inu_display_section_hdr        (int, int);
void inu_print_fail_stats           (Option_t *, Option_t *);

int inurid_is_running();
int inu_different_level (Option_t *op);
void inu_set_graph_status (Option_t * start_op, Option_t * next_op);
void write_command_line (int argc, char **argv);
Level_t * is_it_applied_or_committed (char * op_name);

void prune_dups_between_sops (Option_t *sop, Option_t *savesop);

/*--------------------------------------------------------------------------*
 * DEFINES
 *--------------------------------------------------------------------------*/

#define FD        3     /* close all fd's >3 at kleenup */
#define PID       0     /* auditproc() on this process */
#define INST_EVENT  "INSTALLP_Inst"     /* name of event to audit log */
#define EXEC_EVENT  "INSTALLP_Exec"     /* name of event to audit log */
#define MIN_SPACE_TMP     100      /* Minimum Kblocks require in /tmp */
#define BOSBOOT_SPACE_TMP 7800     /* Kblocks required in /tmp for a bosboot */
#define MIN_SPACE_USR     1000     /* Minimum Kblocks require in /usr */

/* define default scripts */
#define DEF_UPDATE_SCRIPT    "/usr/lib/instl/update"
#define DEF_INSTAL_SCRIPT    "/usr/lib/instl/instal"
#define DEF_CLEANUP_SCRIPT   "/usr/lib/instl/cleanup"
#define DEF_REJECT_SCRIPT    "/usr/lib/instl/reject"
#define DEF_DEINSTAL_SCRIPT  "/usr/lib/instl/deinstal"

#define USER_LIST         "/user.list"              /* user list */
#define SOP_LL            0                         /* sop linked list */
#define TOC_LL            1                         /* toc linked list */

#define NEWER_LEVEL       1
#define SAME_LEVEL        0
#define OLDER_LEVEL      -1


#define BOSBOOT_VERIFY    "/usr/lib/instl/inubosboot -v"
#define BOSBOOT_PERFORM   "/usr/lib/instl/inubosboot"
#define LPP_CTL_NAME      "__SWVPD_CTL__"

#define INSTALLP_SUMMARY_DIR        "/var/adm/sw"
#define INSTALLP_SUMMARY_FILE       "installp.summary"
#define INTERMED_SUM_LOG_FILE       "/tmp/_inu_intermediate.summary.file"
#define INSTALLP_SUMMARY_DELIMITER  ":"
#define STORED_CFGFILES_LIST        "/var/adm/sw/cfgfiles.stored"

 /* for lpp migration */
#define ROOT_MIGSAVE                "/lpp/save.config"
#define USR_MIGSAVE                 "/usr/lpp/save.config"  
#define SHARE_MIGSAVE               "/usr/share/lpp/save.config"  
#define CFGFILES                    "cfgfiles"  

struct stats_table_values
{
   int selected;
   int passed;
   int failed;
   int to_be_seded;
   int deferred;
   int requisites;
   int already_done;
   int unsel_mand_failed;
   int unsel_mand_passed;
};

#define SECT_HDR_PI_PROC      1
#define SECT_HDR_INSTLLING    2
#define SECT_HDR_POST_PROC    3
#define SECT_HDR_SUMMARIES    4

#define INU_TIME_SZ           36
#define INU_TIME_FORMAT "%m\
%d\
%H\
%M\
%S\
%y"

#define CMD_LINE_LT  80


/*--------------------------------------------------------------------------*
 * GLOBAL VARIABLES
 *--------------------------------------------------------------------------*/

extern TOC_t   *TocPtr;            /* Pointer to the toc structure */
extern nl_catd  catd;              /* File descriptor for the catalog */
extern int      audit_log;         /* if auditing */
extern int      Prior_Audit_State; /* audit state to return */

/* for more info about the alternate save directory, see inu_instl_args.c */
/* The tdirinfo fields and values are for present/future requirements */
#define T_SINIT         'I'        /* record initialized, but unused   */
#define T_SUSED         'U'        /* the save directory has been used */
#define T_USE_CAUTION   'X'        /* cpu planar id has changed        */
#define T_NOCHANGE      'X'        /* don't allow dirpath to change    */
#define T_OK            'O'        /* cpu planar id unchanged          */
#define T_ALLOW_CHG     'O'        /* allow dirpath to change          */

/* identification for alternate save location */
struct tsavdir_id {
    char    status;                     /* I = initialized,      U = Used */
    char    use_caution;                /* X = cpu changed,      O = OK   */
    char    allow_path_change;          /* X = no (or not sure), O = OK   */
    char    *time;                      /* time in milliseconds           */
    char    *machine_id;                /* machine id number (planar id)  */
    char    *tsavdir;                   /* alternate save directory       */
} tdirinfo;				/* info about alternate save dir */
#endif  /* _H_INSTALLP */
