/* @(#)55  1.42.1.56  src/bos/usr/sbin/install/ckprereq/check_prereq.h, cmdinstl, bos41J, 9517A_all 4/20/95 19:18:20 */

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS:
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1991,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_CHECK_PREREQ

#define _H_CHECK_PREREQ

#include <ckp_common_defs.h>
#include <cmdinstl_e_msg.h>
#include <commonmsg.h>
#include <ckprereqmsg.h>
#include <search.h>

/*
 * GLOBAL VARS:
 */

/*
 * The following will be used to specify the destination of errors, successes,
 * warnings and progress printed by inu_ckreq.
 */
int ckp_errs; 
int ckp_succ;
int ckp_warn;
int ckp_prog;

/*
 * GLOBAL MACROS:
 */
#define RETURN_ON_ERROR { if (* error) return; }
#define RETURN_VALUE_ON_ERROR(arg) { if (* error) return (arg); }
#define DEBUG_DUMP(arg) { if (check_prereq.debug_mode) graph_dump (arg); }

#define A_IS_PREREQ_OF_B(A, B)                                            \
          ((B->requisites != NIL (requisite_list_type)) &&                \
           (B->requisites->fix_ptr == A))

#define USR_ROOT_PKG_W_NO_ROOT_INSTLD(fix) (                                  \
                             (fix->parts_in_fix == (LPP_USER | LPP_ROOT))   && \
                             (fix->parts_applied == LPP_USER)               && \
                             (check_prereq.parts_to_operate_on & LPP_ROOT))

#define IS_AUTO_APPLIABLE(fix)                                                \
            ((fix->apply_state == IN_TOC)                          \
                                  ||                                          \
             ((fix->apply_state == PARTIALLY_APPLIED) &&           \
              ((fix->parts_applied == LPP_USER) &&                            \
               (check_prereq.parts_to_operate_on & LPP_ROOT))))  

#define IS_AUTO_COMMITTABLE(fix)                                               \
            (((fix->apply_state == ALL_PARTS_APPLIED) &&           \
              (fix->commit_state == UNCOMMITTED))                  \
                                   ||                                         \
             ((fix->commit_state == PARTIALLY_COMMITTED) &&        \
              ((fix->parts_committed == LPP_USER) &&                          \
               (check_prereq.parts_to_operate_on & LPP_ROOT))))

#define IS_AUTO_REJECTABLE(fix)                                               \
             ((fix->reject_state == NOT_REJECTED) ||               \
              (check_prereq.deinstall_submode &&                              \
                                       fix->reject_state == BROKEN))

#define IS_STILL_SUPS_CHAIN_LR_SCAN(old, new)                              \
          ((new != NIL (fix_info_type))                     &&              \
           (!IF_INSTALL (new->op_type))                     &&              \
           (!A_IS_PREREQ_OF_B (old, new))                   &&              \
           (strcmp (old->name, new->name) == 0))

#define IS_STILL_SUPS_CHAIN_RL_SCAN(old, new)                              \
          ((!IF_INSTALL (old->op_type))                     &&              \
           (!A_IS_PREREQ_OF_B (old, new))                   &&              \
           (strcmp (old->name, new->name) == 0))
/*
 * used to detect a superseded entry created by inu_ckreq().  COMPAT() check
 * should be enough, but additional checks for origin and base level are 
 * provided in case another notion of a compat entry is employed or evolves.
 */
#define IS_SUPD_COMPAT_ENTRY(fix)                                           \
              (IF_INSTALL(fix->op_type)   &&                                \
               (fix->origin == DUMMY_FIX) &&                                \
               (COMPAT(fix->cp_flag)))

#define IS_SUPERSEDED(fix)                                                   \
             (fix->apply_state == SUPD_BY_NEWER_BASE            ||           \
              fix->apply_state == SUPD_BY_NEWER_UPDT            ||           \
              fix->apply_state == TO_BE_OVERWRITTEN_BY_SUP_BASE ||           \
              IS_SUPD_COMPAT_ENTRY (fix)                        ||           \
              (fix -> apply_state == AVAILABLE &&                            \
               fix -> superseded_by != NIL (supersede_list_type)))

#define IS_UNRESOLVED_UPDATE(op, parts_to_operate_on)                        \
       (((op->Status == STAT_NOT_FOUND_ON_MEDIA) ||                          \
         (((op->Status == STAT_NUTTIN_TO_APPLY)                              \
             || (op->Status == STAT_NO_USR_MEANS_NO_ROOT)) &&                \
          parts_to_operate_on == LPP_ROOT))                                  \
                     &&                                                      \
        (op->level.ver != -1)                                                \
                     &&                                                      \
        ((op->level.fix !=0) ||                                              \
         ((op->level.ptf[0] != '\0')       &&                             \
          (strcmp (op->level.ptf, "all") !=0) &&                             \
          (strcmp (op->level.ptf, "ALL") !=0))))


/*
 * Global Variables.
 */
extern char * inu_cmdname;  /* Used by printfs.  Set to installp or ckprereq.*/

#define ROOT_VPD_PATH "/etc/objrepos"
#define USR_VPD_PATH "/usr/lib/objrepos"
#define SHARE_VPD_PATH "/usr/share/lib/objrepos"

#define NULL_CHAR '\0'     /* Used to indicate end-of-file on prereq file. */

/* The following are used to switch between operations in the routine
   ck_base_lev_list() */
#define CBLL_UNRES_IFREQ  1
#define CBLL_SUCC         2
#define CBLL_SUCC_CAND    3
#define CBLL_MIGRATING    4
#define CBLL_MISSING      5

/*
 * Global Prototypes.
 */
int inu_ckreq (Option_t * SOP_Ptr,
               Option_t * Fail_SOP_Ptr,
               TOC_t    * TOC_Ptr,
               boolean    verbose,
               boolean    Auto_Include,
               boolean    installp_update,
               short      parts_to_operate_on,
               boolean    called_from_cmd_line);

void build_graph (boolean * error);

void build_subgraph (fix_info_type * parent_fix,
                     boolean       * error);

void determine_order_of_fixes (Option_t * option_list,
                               boolean  * error);

void report_failures (
boolean  instp_non_req_failures,
boolean  ckp_non_req_failures,
boolean  requisite_failures,
boolean  non_req_auto_sups_failure,
boolean  req_auto_sups_failure,
boolean * error);

void report_succeses (Option_t * sop, boolean * error);

void report_warnings (boolean fix_info_warnings,
                      boolean failsop_warnings,
                      boolean * error);

void process_dummy_maintenance_packages (boolean * error);

/* Debug code. */

void graph_dump (char * description);

void dump_fix_info (fix_info_type * fix_info);

void dump_sop (char * description);

void dump_option (Option_t * sop);

/***********************************************************************
 ************************** Declarations for ***************************
 ************************** build_supersede.c **************************
 ***********************************************************************/

void build_expl_supersede_info (boolean * error);

/***********************************************************************
 ************************** Declarations for ***************************
 ************************** copy_utils.c *******************************
 ***********************************************************************/

char * dupe_string (char * source, boolean * error);

void copy_TOC_to_fix_info (Option_t         * option,
                           fix_info_type   ** fix,
                           fix_origin_type    origin,
                           boolean          * error);

void copy_vpd_to_fix_info (prod_t        * product_info,
                           fix_info_type * fix_list,
                           short           vpd_source,
                           boolean       * error);
/***********************************************************************
 ************************** Declarations for ***************************
 ************************** cycle_utils.c    ***************************
 ***********************************************************************/

void check_for_prereq_cycles (fix_info_type * node,
                              boolean       * error);

/***********************************************************************
 ************************** Declarations for ***************************
 ************************** determine_order.c **************************
 ***********************************************************************/

typedef struct fix_set fix_set_type;

struct fix_set
       {
         fix_info_type * fix_ptr;
         fix_set_type  * next_fix;
         fix_set_type  * previous_fix;
       };

/***********************************************************************
 ************************** Declarations for ***************************
 ************************ evaluate_base_levels.c ***********************
 ***********************************************************************/

void disable_older_pkgs (
fix_info_type * new_best_base,
fix_info_type * start_node,
char          * reference_name,
boolean         disabling_expl_supd_option,
boolean       * error);

void evaluate_base_levels (boolean * error);

/***********************************************************************
 ************************** Declarations for ***************************
 ************************ evaluate_failures.c **************************
 ***********************************************************************/

boolean evaluate_failures_and_warnings (
boolean * ckp_non_req_failures,
boolean * instp_non_req_failures,
boolean * requisite_failures,
boolean * non_req_auto_sups_failure,
boolean * req_auto_sups_failure,
boolean * fix_info_warnings,
boolean * fail_sop_warnings,
boolean * error);

void evaluate_requisite_failure_subgraph (
fix_info_type       * subgraph_root,
requisite_list_type * req_node,
boolean             * error);

/***********************************************************************
 ************************** Declarations for ***************************
 ************************** evaluate_graph.c ***************************
 ***********************************************************************/

void evaluate_base_levels (boolean * error);

boolean evaluate_graph (boolean * error);

boolean evaluate_subgraph (requisite_list_type * requisite,
                           fix_info_type       * fix,
                           boolean             * error);

void add_auto_include_nodes_to_graph (boolean * error);

void consider_ifreqs (boolean * error);

void consider_unresolved_ifreqs (boolean * error);

/***********************************************************************
 ************************** Declarations for ***************************
 ************************** evaluate_requisite.c ***********************
 ***********************************************************************/
 
void evaluate_requisite (
fix_info_type  * parent,
char           * lpp_name,
fix_info_type ** fix_chosen,
criteria_type  * criteria,
criteria_type  * ifreq_updt_lev,
requisite_type   type,
boolean          building_41_base_prq,
boolean        * error);

/***********************************************************************
 ************************** Declarations for ***************************
 ************************ evaluate_supersedes.c ************************
 ***********************************************************************/

void evaluate_superseding_updates (
boolean * error);

/***********************************************************************
 ************************** Declarations for ***************************
 ************************** fix_state_utils.c **************************
 ***********************************************************************/

void figure_out_state_of_fix (fix_info_type * fix,
                              boolean       * error);

void inconsistent_state_error_check (fix_info_type * fix,
                                     boolean       * error);

void reset_state_of_medialess_fixes (void);

void set_implicit_states (void);

/***********************************************************************
 ************************** Declarations for ***************************
 ************************** hash_utils.c *******************************
 ***********************************************************************/

void create_hash_table (boolean * const error);

void destroy_hash_table (void);

void load_fix_hash_table (const fix_info_type * const fix_list,
                          boolean             * const error);

void add_to_fix_hash_table (fix_info_type const * const fix,
                            boolean             * const duplicate,
                            boolean             * const error);

ENTRY * locate_hash_entry (char * const lpp_name,
                           char * const ptf);

/***********************************************************************
 ************************** Declarations for ***************************
 ************************** list_utils.c *******************************
 ***********************************************************************/
void add_unique_node_to_base_lev_list (
requisite_list_type * req_node,
fix_info_type       * new_base_lev,
boolean             * error);


boolean ck_base_lev_list (requisite_list_type * req_node,
                          short                 operation,
                          boolean               cld_by_rpt_routine);

void delete_unreferenced_nodes (boolean * const error);

void purge_fix_list (possible_fixes_type * const fail_list);

void add_fix_to_fix_info_table (fix_info_type * const fix_info,
                                boolean       * const error);

fix_info_type * fix_info_lookup (char    * lpp_name,
                                 Level_t * level,
                                 boolean   use_fix_id);

fix_info_type * get_base_level (fix_info_type *);

fix_info_type * impl_sups_lookup (char    * lpp_name,
                                  Level_t * level);

boolean is_B_req_of_A (fix_info_type * A, fix_info_type * B);

fix_info_type * locate_fix (char    * const lpp_name,
                            Level_t * const level,
                            boolean   use_fixid);

requisite_list_type * link_requisite (
fix_info_type  * const parent_fix,
fix_info_type  * const fix_chosen,
criteria_type  * const criteria,
requisite_type   const type,
boolean        * const error);

Option_t * move_failed_sop_entry_to_fail_sop (fix_info_type * fix_ptr,
                                              int             failure_status);


void remove_groups_from_graph (fix_info_type *, fix_info_type *, boolean *);

void unmark_SUBGRAPH (fix_info_type * const node,
                      int             const bit_pattern);

void unmark_graph (int const bit_pattern);

void unmark_requisite_visited (void);

void mark_initial_CANDIDATE_NODE (void);

void put_command_line_args_in_graph (boolean * const error);

void tag_subgraph_SUCCESSFUL_NODEs (requisite_list_type * link_from_parent,
                                    fix_info_type * const fix,
                                    boolean       * const error);
void free_graph (void);

fix_info_type * locate_fix_entry (char * lpp_name, Level_t * level);

void delete_supersedes_info ();

/***********************************************************************
 ************************** Declarations for ***************************
 ************************** load_fix_info.c * **************************
 ***********************************************************************/

void load_fix_info_table (
boolean   called_without_sop,
boolean * error);

void read_current_product_vpd (
prod_t        * product_info,
int             flags,
fix_info_type * fix_header,
short           vpd_source,
boolean       * error);

/***********************************************************************
 ************************** Declarations for ***************************
 ************************** mem_utils.c ********************************
 ***********************************************************************/

base_lev_list_type * create_base_lev_list_node (boolean * error);

void * my_malloc (size_t size, boolean * error);

void * my_realloc (void * source, size_t size, boolean * error);

void my_free (void * ptr);

void free_fix_info_node (fix_info_type * node);

void free_possible_fix_node (possible_fixes_type * node);

fix_info_type * create_fix_info_node (boolean * error);

possible_fixes_type * create_fix_list_header (boolean * error);

possible_fixes_type * create_possible_fix_node (boolean * error);

void create_dummy_group (fix_info_type ** group_node,
                         boolean        * error);

void create_DUMMY_FIX (
fix_info_type ** dummy,
char           * option_name,
Level_t        * level,
boolean        * error);

void create_relation (criteria_type * criteria,
                      token_type      VPD_field,
                      token_type      operator,
                      short           int_value,
                      char          * PTF_value,
                      boolean       * error);

void free_relations (relation_type * relation);

void dupe_relations (criteria_type * source,
                     criteria_type * destination,
                     boolean       * error);

requisite_list_type * create_requisite (criteria_type * criteria,
                                        boolean       * error);

void clear_requisite_flags (requisite_flags_type * requisite_flag);

void free_requisite (requisite_list_type * requisite);

void free_options (Option_t * Option_Ptr);

/***********************************************************************
 ************************** Declarations for ***************************
 ************************** parser.c ***********************************
 ***********************************************************************/

#define MAX_LEX_STRING_LENGTH 81
#define MAX_PREREQ_LINE_LENGTH 1024
#define INITIAL_NUMBER_OF_LINES_IN_PREREQ_FILE 100   /* Used for malloc. */

token_type get_token (boolean * error);

void initialize_lexical_analyzer (fix_info_type * prereq_node);

void print_output_file (void);

#define INSTALL_OR_UPDATE  FALSE
#define INSTALL_LEVEL_ONLY TRUE

void set_failure_code (char      failure_code,
                       boolean   major_error);


/* We have some data private to the parser/lexical analyzer that is best to
   keep global for them. */

typedef struct
         {
           char *  start_of_line;
           char    error_code;
         } error_info_type;

typedef struct
         {
           fix_info_type   * prereq_node;
           token_type        token;
           int               column;  /* for error reporting */
           int               line_number;
           int               start_of_current_expression; /* Line number of exp */
           error_info_type * error_info;
           int               error_info_buffer_size;
           char            * prereq_file;
           char            * virtual_file;
           char              tokens_string_value[MAX_LEX_STRING_LENGTH];
           int               tokens_number_value;
           int               dotted_number_count;  /* used to check correct #
                                                      of digits specified. */
         } parser_data_type;

parser_data_type  parsers;
/***********************************************************************
 ************************** Declarations for ***************************
 ************************** process_unres_updts.c **********************
 ***********************************************************************/

void
process_unresolved_updates_on_failsop (Option_t *, boolean *, boolean *);

/***********************************************************************
 ************************** Declarations for ***************************
 ************************** utils.c ************************************
 ***********************************************************************/

#define USE_FIXID     TRUE
#define IGNORE_FIXID  FALSE

void 
check_failsop_for_special_cases (
Option_t * failsop,
boolean  * unresolved_updates_on_failsop,
boolean  * refine_failsop_errors,
short      parts_to_operate_on);

short convert_content_type_to_cp_flag_type (char content);

short convert_action_type_to_cp_flag_type (int action);

int convert_cp_flag_type_to_content_type (short cp_flag);

boolean same_fix (fix_info_type * fix_1,
                  fix_info_type * fix_2);

boolean same_level (Level_t * level_1,
                    Level_t * level_2);

boolean is_lpp_applied (possible_fixes_type * fixes_for_this_lpp,
                        boolean               base_match_required);

boolean is_compat_levels (
char    * opt_name,
char    * supersedes_string,
Level_t * target);

char * get_barrier_from_sups_str (
char * opt_name,
char * sups_string,
char * barrier_lev);

char * get_level_from_fix_info (fix_info_type * node,
                                char          * level);

char * get_fix_name_from_fix_info (fix_info_type * const node,
                                   char          * const buffer);

char * construct_fix_name (char    * const name,
                           Level_t * const level,
                           char    * const buffer);

void  format_lpp_level (Level_t * const level,
                        char    * const buffer);

void  Quit (int const return_code);

boolean apply_superseded_root_for_consistency (fix_info_type * fix_info,
                                               fix_info_type * supersede);

boolean strcmp_non_wsp (char * str1, char * str2);

boolean is_dummy_supd_vpd_rec (fix_info_type * fix);

static criteria_type EMPTY_CRITERIA = {0,
                                       0,
                                       NIL (relation_type),
                                       NIL (relation_type),
                                       NIL (relation_type),
                                       NIL (relation_type),
                                       NIL (relation_type)};

fix_info_type * get_superseding_fix (fix_info_type * fix);

char * get_superseding_name_or_lev (
fix_info_type * superseded_fix,
char          * buf);

/***********************************************************************
 **************************  Declarations for **************************
 *****  report_failures.c, report_successes.c, report_utils.c, *********
 ***********************************************************************/

#define IS_EXPL_REQUESTED_PKG(sop_ptr)                        \
                     (IF_SELECTED (sop_ptr -> flag))

/*
 * Macro determines if sop_ptr is a pkg being auto-installed/committed.
 * Criteria:  MC type, not a 4.1 pkg, neither explicitly requested
 * NOR an auto supersedes of an explicitly requested pkg.
 */ 
#define IS_AUTO_MAINT_PKG(sop_ptr)                             \
                     ((IF_C_UPDT (sop_ptr->op_type)) &&        \
                      (! IF_4_1 (sop_ptr->op_type))  &&        \
                      ! (IS_EXPL_REQUESTED_PKG (sop_ptr) ||    \
                         IS_AUTO_SUPERSEDE_PKG (sop_ptr)))

#define IS_AUTO_IFREQ_PKG(sop_ptr)                            \
                     (sop_ptr->Status == AUTO_IFREQ)

#define IS_AUTO_MANDATORY_PKG(sop_ptr)                        \
                     ((sop_ptr->Status == MANDATORY) &&       \
                       ! IS_EXPL_REQUESTED_PKG (sop_ptr))

#define IS_AUTO_SUPERSEDE_PKG(sop_ptr)                        \
                      (sop_ptr->Status == AUTO_SUPERSEDE)

#define IS_IMPL_REQUESTED_PKG(sop_ptr)                        \
                     ( (check_prereq.mode == OP_REJECT &&     \
                        ! IS_EXPL_REQUESTED_PKG (sop_ptr))    \
                                 ||                           \
                       ! (IS_EXPL_REQUESTED_PKG (sop_ptr) ||  \
                          IS_AUTO_SUPERSEDE_PKG (sop_ptr) ||  \
                          IS_AUTO_MANDATORY_PKG (sop_ptr) ||  \
                          IS_AUTO_MAINT_PKG (sop_ptr)     ||  \
                          IS_AUTO_IFREQ_PKG (sop_ptr)))

#define TYPE_C_FAILURE(failure_sym)                                       \
                        (((check_prereq.mode == OP_REJECT) &&             \
                          ((failure_sym == APPLD_PRQ_SYM) ||              \
                              (failure_sym == REJECTABLE_REQ_SYM)))       \
                                            ||                            \
                         ((check_prereq.mode != OP_REJECT) &&             \
                          ((failure_sym == AVLBL_SYM) ||                  \
                               (failure_sym == UNAVLBL_REQ_OF_REQ_SYM))))

typedef struct
   {
      index_list_type * superseded_index_hdr;
      index_list_type * superseded_index_tail;
      index_list_type * misc_warnings_hdr;
      index_list_type * misc_warnings_tail;
      boolean           instreq_failure_in_subgraph;
      boolean           non_instreq_failure_in_subgraph;
   } report_failures_type;

report_failures_type report_anchor;

/*
 * The following are used to communicate the type of a package between
 * the ordering routines and the success reporting routines.
 */
#define EXPLICIT       1
#define IMPLICIT       2
#define MANDATORY      3
#define AUTO_MC        4
#define AUTO_IFREQ     5
#define AUTO_SUPERSEDE 6

#define DESC1ST TRUE
#define DESC2ND FALSE

/* The following #defined symbols are used to report requisite failures. */

#define BLANK_SYM                  ' '

/* Apply only */
#define CONFL_LEVEL_SYM            '='
#define SUPRSD_CONSIST_SYM         '<'
#define APPLIED_ROOT_ONLY_SYM      '}'
#define COMMITTED_ROOT_ONLY_SYM    '{'
#define NOT_APPLD_CMD_LINE_SYM     '*'
#define UPDATES_ONLY_SYM           '@'
#define BROKEN_SUPS_SYM            '/'
#define INSTREQ_SYM                'I'
#define RENAMED_SUPD_PKG_SYM       'R'    /* flags a pkg that  fails to 
                                             re-install over it's superseding,
                                             newly named, pkg. */
#define DISABLING_INT_ERR_SYM      'D'    /* flags a pkg which was disabled
                                             for the current installation due
                                             to a internal error condition
                                             detected. */
#define OEM_BASELEVEL_SYM          'o'    
#define OEM_MISMATCH_SYM           'O'    
#define OEM_SPECIFIC_SYM           'S'    

/* Commit only */
#define COM_PARTLY_APPLD_SYM       '#'
#define AVLBL_NOT_APPLD_SYM        '@'
#define CANT_COMT_ROOT_PART_SYM    '&'
#define COM_PARTLY_COMTD_SYM       '%'
#define SUPRSD_USR_COMMIT_SYM      '<'

/* Reject only */
#define PARTLY_COM_SYM             '%'
#define COM_PRQ_SYM                '@'
#define APPLD_ROOT_SYM             '&'
#define APPLD_USR_SYM              '$'
#define APPLD_USR_ROOT_SYM         '#'
#define APPLD_SHARE_SYM            '^'
#define APPLD_PRQ_SYM              '!'
#define REJECTABLE_REQ_SYM         '~'
#define DEINSTBLTY_FAIL_SYM        '*'

/* Apply & Commit */

#define UNAVLBL_SYM                '*'
#define PARTLY_APPLD_SYM           '#'
#define AVLBL_SYM                  '!'
#define UNAVLBL_REQ_OF_REQ_SYM     '~'
#define NO_USR_PART_RQSTD_SYM      '$'
#define NO_USR_ROOT_PART_RQSTD_SYM '+'
#define NO_SHARE_PART_RQSTD_SYM    '^'
#define BROKEN_REQ_SYM             '-'
#define UNKNOWN_SYM                '?'
#define NO_PRQ_BASE_SYM            'x'
#define MIGRATING_SYM              'M'

/**
 ** MAX_LEGEND_SZ is used to define the size of the report_failure legend.
 **/
#define MAX_LEGEND_SZ  41

void add_entry_to_index_list (index_list_type  * hdr,
                              fix_info_type    * fix_ptr,
                              Option_t         * sop_ptr,
                              criteria_type    * criteria,
                              boolean          * error);

void add_failures_to_fail_sop (void);

void add_fix_to_cmd_line_fail_list (fix_info_type * node, boolean * error);

void add_fix_to_failed_requisites_list (fix_info_type         * subgraph_root, 
                                        fix_info_type         * node,
                                        criteria_type         * criteria,
                                        requisite_flags_type    req_flags,
                                        boolean               * error);

void add_subgraph_root_to_dependents_list (
                               fix_info_type        * subgraph_root,
                               requisite_list_type  * req_node,
                               boolean              * error);

void add_sym_to_rf_legend (char failure_symbol);

boolean bypass_redundant_group_reporting (fix_info_type * node);

boolean check_for_cycle_or_ifreq_subgraph (
                                     fix_info_type       * current_fix,
                                     requisite_list_type * req_node,
                                     boolean               cld_by_rpt_routine);

void create_fix_info_index_from_subgraph (
                                index_list_type * index_hdr,
                                fix_info_type   * subgraph_root,
                                fix_info_type   * current_fix,
                                boolean         * error);

index_list_type * create_index_node (criteria_type * criteria,
                                     boolean       * error);

char determine_failure_reason (fix_info_type        * node,
                               requisite_list_type  * req_node,
                               boolean                doing_command_line_fails);

boolean flag_rename_conflicts (fix_info_type * fix);
boolean flag_superseding_TYPE_C_apply_failures (fix_info_type * fix);

char * format_pkg_name_and_desc (boolean    base_lev,
                                 char     * return_string,
                                 char     * pkg_string,
                                 char     * desc_string,
                                 boolean  * error);

void get_criteria_string (criteria_type * criteria, char * level_buf);

void get_group_req_stats (fix_info_type * node,
                          short         * satisfied,
                          short         * satisfiable,
                          short         * total,
                          boolean       * nested_group);

boolean ignore_ifreq_subgraph (boolean                     doing_successes,
                               requisite_list_type       * requisite,
                               boolean                     cld_by_rpt_routine);

void init_fix_info_index (index_list_type ** index_hdr,
                                index_list_type ** index_tail,
                                boolean          * error);

void init_sop_index (index_list_type ** index_hdr,
                     index_list_type ** index_tail,
                     boolean          * error);

boolean is_2nd_greater (char    * name1, 
                        char    * name2, 
                        Level_t * level1, 
                        Level_t * level2,
                        char    * desc1,
                        char    * desc2);

void check_failures_on_Fail_SOP (boolean * failures,
                                 boolean * warnings);

void print_alrdy_instlds_or_comtds (void);

void print_ckp_non_req_failures_hdr (boolean auto_sups_failure);

void print_dependent_ref_msg ();

void print_failed_pkg (fix_info_type      * failed_pkg,
                       boolean              print_failure_sym,
                       criteria_type      * criteria,
                       boolean            * error);

void print_failures_hdr (void);

void print_group_requisites (
fix_info_type        * parent,
short                  indent_lev,
requisite_list_type  * req_node,
boolean              * error);


void  print_index_list_msg (
index_list_type * index_hdr,
index_list_type * index_tail,
flags_type const  warning_type,
boolean         * error);

void print_instp_failures_and_warnings (boolean * error);

void print_instp_non_req_failures (Option_t * Fail_SOP);

void print_non_req_preinst_failures (Option_t     * failsop, 
                                     boolean const  print_end_msg);

void print_requisite_and_dependent_list (
fix_info_type        * failed_requisite,
boolean                print_failure_sym,
char                 * specl_lppchk_output,
boolean              * error);

void print_subgraph_requisite (fix_info_type       * fix_ptr,
                               requisite_list_type * req_node,
                               short                 indent_lev,
                               short                 num_spaces);

void print_success_hdr_by_request_type (short request_type);

void print_successful_requisite (fix_info_type * fix_ptr);

void print_successful_sop_entry ( Option_t * sop_ptr, boolean  * error);

void print_success_key (void);

void print_success_section_header (void);

void cleanup_report_failures (void);

void create_sop_index (Option_t         * source_sop_hdr,
                       index_list_type ** index_hdr,
                       index_list_type ** index_tail,
                       boolean          * error);

void print_sop_list_msg (
Option_t *failsop,
int       target_status);

void print_TYPE_A_FAILURE_hdr (void);
void print_TYPE_B_FAILURE_hdr (void);
void print_TYPE_C_FAILURE_hdr (void);
void print_TYPE_C_FAILURE_msg (short count);
void print_TYPE_Z_FAILURE_hdr (void);

void print_requisite_failures_hdr (boolean auto_sups_failure);


void print_report_failure_legend (boolean print_requisite_key);

void print_requisite_fix (fix_info_type        * node,
                      criteria_type        * criteria,
                      fix_info_type        * parent,
                      short                  indent_lev,
                      requisite_flags_type   req_flags);

void print_warnings_hdr (void);

void reset_report_failure_legend (void);

void
refine_failsop_errors_for_reporting (Option_t * failsop);

void 
print_ifreq_warnings (boolean * error);

void
print_superseded_pkgs (void);

/***********************************************************************
 **************************  Miscellaneous  ****************************
 **************************   Declarations  ****************************
 ***********************************************************************/

/* prereq_errors */

typedef enum {GROUP_OR_REQUISITE_TYPE, LPP_NAME_EXPECTED, OR_INVALID,
              VRMFP_EXPECTED, VERSION_SEEN_AGAIN, RELEASE_SEEN_AGAIN,
              MODIFICATION_SEEN_AGAIN, PTF_SEEN_AGAIN, FIX_SEEN_AGAIN,
              VPD_FIELDS_MUST_MATCH, MUST_USE_EQUALS_ON_PTF,
              EXPECTED_RELATION, ILLEGAL_PTF, NUMBER_EXPECTED,
              NUMBER_OR_PTF_EXPECTED, OPEN_BRACE_EXPECTED,
              CLOSE_BRACE_EXPECTED, CLOSE_PAREN_EXPECTED, PASS_COUNT_REQUIRED,
              PASS_COUNT_TOO_SMALL, PTF_WITH_MOD_OR_FIX,
              BASE_LEVEL_AND_VRMF_ILLEGAL, DOTTED_NUMBER_EXPECTED
              } parser_error_type;

typedef enum {ILLEGAL_NAME, ILLEGAL_NUMBER, REQUISITE_EXPECTED,
              UNEXPECTED_CHARACTER, NOT_ASCII, NAME_TOO_LONG,
              ILLEGAL_DOTTED_NUMBER
              } lex_error_type;

#define FATAL_ERROR_RC         (-1)
#define CAN_NOT_REJECT_RC         1
#define MALLOC_ERROR_RC           2
#define UNEXPECTED_LPP_MICRO      3
#define ILLEGAL_SHARE_PART_RC     4
#define VPD_SYS_ERROR_RC          5
#define VPD_INTERNAL_ERROR_RC     6
#define UNEXPECTED_STATE_RC       7
#define LEXICAL_ERROR_RC          8
#define PARSING_ERROR_RC          9
#define INTERNAL_LEX_ERROR_RC    10
#define INTERNAL_ERROR_RC        11
#define PREREQ_CYCLE_RC          12
#define COREQ_SELF_RC            13
#define ILLEGAL_ROOT_RC          14
#define SEMANTIC_ERROR_RC        15
#define HALT_INSTALLP            99
#define UNKNOWN_ERROR_RC         100

#define CKPREREQ_ERROR_FLAG      'c'
#define FIX_ERROR_FLAG           'f'
#define IFREQ_ERROR_FLAG         'i'
#define MODIFICATION_ERROR_FLAG  'm'
#define PREREQ_ERROR_FLAG        'n'
#define PTF_ERROR_FLAG           'p'
#define RELEASE_ERROR_FLAG       'r'
#define SYNTAX_ERROR_FLAG        's'
#define VERSION_ERROR_FLAG       'v'


#endif
