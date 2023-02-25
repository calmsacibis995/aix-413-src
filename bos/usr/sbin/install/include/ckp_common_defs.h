/* @(#)71  1.25  src/bos/usr/sbin/install/include/ckp_common_defs.h, cmdinstl, bos41J, 9510A_all 2/27/95 12:15:20 */
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS:
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1989,1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_CKP_COMMON_DEFS
#define _H_CKP_COMMON_DEFS

#include <inulib.h>
#include <instl_options.h>

typedef enum {ALL_PARTS_APPLIED, TO_BE_EXPLICITLY_APPLIED, 
              TO_BE_IMPLICITLY_APPLIED, APPLY_TO_BE_COMPLETED,
              PARTIALLY_APPLIED, IN_TOC, SUPD_BY_NEWER_BASE, 
              SUPD_BY_NEWER_UPDT, TO_BE_OVERWRITTEN_BY_SUP_BASE, 
              CANNOT_COMPLETE_APPLY, CANNOT_APPLY, CANNOT_APPLY_CONFL_LEVEL,
              TO_BE_OVERWRITTEN_BY_CONFL_LEVEL, AVAILABLE, BROKEN,
              APPLYING, CANCELLED, FAILED, UNKNOWN}
              apply_state_type;

  /* ALL_PARTS_APPLIED         means that a package is completely applied.

     TO_BE_EXPLICITLY_APPLIED  means that this package came from the command
                               line.  If there is something wrong with the
                               package, the state may change to CAN_NOT_APPLY.

     TO_BE_IMPLICITLY_APPLIED  used to flag a package which is being 
                               pulled in as a requisite of another.
                               Set AFTER all evaluation is done and is used
                               primarily for reporting purposes.

     APPLY_TO_BE_COMPLETED     used by the ordering routines as an intermediate 
                               state when doing root part applies.

     PARTIALLY_APPLIED         means that the ROOT part of this package has not
                               been applied.  

     IN_TOC                    means that this package is on the media.

     SUPD_BY_NEWER_BASE        base leve or updt pkg on the media which is 
                               implicitly superseded by a base level pkg.

     SUPD_BY_NEWER_UPDT        update on the media which is implicitly or 
                               explicitly superseded by an update pkg.
     
     TO_BE_OVEWRITTEN_BY_SUP_BASE
                               an installed pkg which is about to be overwritten
                               by a superseding base level.
                               
     CANNOT_COMPLETE_APPLY     Indicates that a PARTIALLY_APPLIED package can
                               not be applied because the -O flag prevents that
                               part from being worked on.

     CANNOT_APPLY              something is preventing this pkg from applying.
                               Perhaps the wrong parts were requested (-O flag).

     CANNOT_APPLY_CONFL_LEVEL  pkg which was not requested explcitly which is 
                               in conflict with another base level that was
                               either requested or is already installed.
                     
          
     TO_BE_OVERWRITTEN_BY_CONFL_LEVEL
                               installed pkg that is about to be overwritten
                               by an older or newer incompatible base level.

     AVAILABLE                 The package (or part of a package) is marked as
                               AVAILABLE in the VPD.  May be a dummy superseded
                               record or a pkg that was rejected/de-installed.

     BROKEN                    corresponds to BROKEN state in the VPD.  

     APPLYING,CANCELLED,       used by the "in-progress" requisite checking
     FAILED                    mechanism in installp  (ie. After inu_ckreq
                               tells installp what to apply).

     UNKNOWN                   The state of the package is unknown.  This is
                               an initilization/error state. */


typedef enum {ALL_PARTS_COMMITTED, TO_BE_EXPLICITLY_COMMITTED, 
              TO_BE_IMPLICITLY_COMMITTED, COMMIT_TO_BE_COMPLETED, 
              PARTIALLY_COMMITTED, UNCOMMITTED, CANNOT_COMPLETE_COMMIT, 
              CANNOT_COMMIT, COMMIT_UNKNOWN} 
              commit_type;

 /* The commit states mirror the apply states.

    ALL_PARTS_COMMITTED              Means that a package is completely
                                     committed.

    TO_BE_EXPLICITLY_COMMITTED       Means that the package was on the command
                                     line.  It is possible for the state to
                                     change to CANNOT_COMMIT in some, unusual
                                     circumstances.

     TO_BE_IMPLICITLY_COMMITTED      used to flag a package which is being 
                                     pulled in as a requisite of another.
                                     Set AFTER all evaluation is done and is 
                                     used primarily for reporting purposes.

    COMMIT_TO_BE_COMPLETED           Same as TO_BE_EXPLICITLY_COMMITTED,
                                     except that only the ROOT part needs to
                                     be committed.

    PARTIALLY_COMMITTED              Means that the USR part is committed, but
                                     not the ROOT part.  If a requisite
                                     requires that this package be committed,
                                     then the state will change to either
                                     COMMIT_TO_BE_COMPLETED or
                                     CANNOT_COMPLETE_COMMIT, depending on the
                                     -O flag.

    UNCOMITTED                       Indicates that the package (or this part
                                     of a package) is APPLIED and not yet
                                     COMMITTED.

    CANNOT_COMPLETE_COMMIT          Indicates that a PARTIALLY_COMMITTED
                                     package can not have it's other part
                                     committed because of the -O flag.

    CANNOT_COMMIT                   May indicate an error state.

    COMMIT_UNKNOWN                   Initial commit_state.  Is an error state.
  */


typedef enum {CANNOT_REJECT, NOT_REJECTABLE, NOT_REJECTED,
              TO_BE_EXPLICITLY_REJECTED, TO_BE_IMPLICITLY_REJECTED, REJECTED,
              DONT_NEED_TO_REJECT, REJECT_UNKNOWN} reject_type;


  /* CAN_NOT_REJECT             means that we want to reject this guy, but can
                                not.

     NOT_REJECTABLE             means that this node (or subgraph) is not
                                eligable to be rejected; and no one has yet
                                tried.

     NOT_REJECTED               means that we are eligable to be rejected, but
                                no one has yet tried.

     TO_BE_EXPLICITLY_REJECTED  means that this node (or subgraph) has
                                explictily been asked to be rejected from the
                                command line.

     TO_BE_IMPLICITLY_REJECTED  used to flag a package which is being 
                                pulled in as a dependent of another.
                                Set AFTER all evaluation is done and is 
                                used primarily for reporting purposes.

     REJECTED                   This is only used at the tail end, when the
                                ordered SOP is being built to send back to
                                installp.  As each node has it's SOP created,
                                it is marked as REJECTED.

     DONT_NEED_TO_REJECT        means that this item is not even installed.
                                This is used for superseded AVAILABLE packages.

     REJECT_UNKNOWN             This is an initial state.  It is also an error
                                state. */


/*
 * Origins:  provide information about where the package came from and
 *           what its status is.
 */
typedef enum {COMMAND_LINE, TOC, RQD_UPDATE_FROM_TOC, AUTO_MC_FROM_VPD, VPD, 
              DUMMY_SUPERSEDED_FIX, DUMMY_FIX, DUMMY_GROUP, UNKNOWN_ORIGIN} 
              fix_origin_type;

typedef enum {UNKNOWN_REQUISITE, A_PREREQ, A_COREQ, AN_IFREQ, AN_IFFREQ,
              AN_INSTREQ}
              requisite_type;


typedef enum {A_NAME, A_NUMBER, IFREQ, PREREQ, COREQ, INSTREQ, FIX, 
              MODIFICATION, OR, PTF, RELEASE, VERSION, LESS_THAN, GREATER_THAN,
              EQUALS, GTRTHAN_OR_EQUALS, OPEN_BRACE, CLOSE_BRACE, OPEN_PAREN, 
              CLOSE_PAREN, A_DOTTED_NUMBER, END_OF_FILE, ILLEGAL_TOKEN} 
              token_type;

typedef long flags_type;

#define VISITING_THIS_NODE     BIT(0)   /* Cycle detection.  Keeps us from
                                           getting hung in recursion. */

#define MEMBER_OF_SOP          BIT(1)   /* Indicates that a node will be placed
                                           on installp's SOP. */

#define CONFL_TOC_BASE_LEVEL   BIT(2)   /* Flags a conflict in versions of
                                           a given product in the TOC.  
                                           Info transferred from toc->flag
                                           entry at copy_toc_to_fix_info.  */

#define VISITING_THIS_SUBGRAPH BIT(3)   /* Has the same function as
                                           VISITING_THIS_NODE.  Allows for two
                                           unrelated walks of the graph to
                                           occur without collision.  Used in
                                           conjunction with SUBGRAPH_VISITED. */


#define SUBGRAPH_VISITED       BIT(4)   /* Since there can be cycles, and cycles
                                           of cycles, a lot of time can be
                                           wasted revisiting the same nodes
                                           over and over.  This flag marks
                                           each subgraph that has already been
                                           worked on. */

#define SUCCESSFUL_NODE        BIT(5)   /* Flags a node that was successfully
                                           evaluated (w.r.t req checks) */

#define DEBUG_CYCLE_DETECTION  BIT(6)  /* Used Only during debugging. */


#define MEMBER_OF_SUCCESS_SUBGRAPH BIT(7)  /* Set when evaluating failures
                                              and successes.  Used as a 
                                              sanity check when deleting
                                              unreferenced nodes. */

#define UPDATES_ONLY           BIT(8)  /* Can't apply Install levels when
                                           -B (updates only) flag specified.
                                           Used for Error Reporting. */

#define CANDIDATE_NODE         BIT(9)  /* Flags nodes which could satisfy
					   a requisite or which could be 
                                           installed, committed etc., but 
                                           which haven't been evaluated as 
                                           part of a SUCCESSFUL subgraph. */

#define FAILED_NODE            BIT(10)  /* Node cannot satisfy a requisite. */

#define FIX_HAS_IFREQS         BIT(11)  /* Used by consider_unresolved_ifreqs
                                           to determine if fix has ifreqs. */

#define UNRESOLVED_IFREQ       BIT(12)  /* Used for reporting unresolved
                                           ifreqs on apply operation. */

#define SUPERSEDES_BROKEN_PTF BIT(13)   /* Flags a ptf which supersedes another
                                           which is broken.  Used for error
                                           reporting. */

#define AUTO_C_UPDATE          BIT(14)  /* Flags a MC type update pkg which is
                                           being automatically applied/committed
                                         */

#define EXPL_REQUESTED_PKG     BIT(15)  /* Success/Error reporting.  Explicitly
                                           requested pkg. */

#define MANDATORY_UPDATE       BIT(16)  /* Not explicitly requested, dragged in
                                           automatically. */

#define REPT_CMD_LINE_FAILURE  BIT(17)  /* Flags a pkg requested from the cmd
                                           line which failed for reasons other
                                           than requisite failures. */

#define REPT_REQUISITE_FAILURE BIT(18)  /* Flags a pkg requested from cmd line
                                           which failed because of requisite 
                                           problems. */

#define REPT_IFREQ_WARNING     BIT(19)  /* Flags a pkg which is a missing 
                                           ifreq of one that is already 
                                           installed: pkg fails to install*/

#define REPT_FAILED_REQUISITE  BIT(20)  /* Flags a pkg which is a failed 
                                           requisite of one that was 
                                           explicitly requested. */

#define AUTO_SUPERSEDES_PKG    BIT(21)  /* Flags a superseding pkg which was
                                           selected by the auto supersedes
                                           rules. */

#define WARN_NO_USR_SUPS_INFO  BIT(22)  /* Next 5 are warnings flags. */
#define WARN_NO_USR_PRQ_INFO   BIT(23)
#define WARN_NO_PRQ_BASE       BIT(24)  
#define WARN_MISS_USR_APPLD_ROOT BIT(25)
#define WARN_MISS_USR_COMTD_ROOT BIT(26)

#define FAILED_DEINST_CHECK    BIT(27)  /* Flags a deinstallability check 
                                           failure. */

#define CKP_ST_APPLYING            BIT(28)  /* The next 4 are used ONLY when */
#define CKP_ST_COMMITTING          BIT(29)  /* load_fix_info() is called by  */
#define CKP_ST_REJECTING           BIT(30)  /* lppchk.  Needed since ckprq   */
#define CKP_ST_DEINSTALLING        BIT(31)  /* normally doesn't process INGs */

/* RECYCLED BITS FOLLOW: The following bits are used above but should not 
   cause conflicts during the same installation */

#define DISABLING_INT_ERR_FLAG     BIT(31)  /* Flags nodes "disabled" by 
                                               internal error recovery. */

#define RENAMED_CONFLICTING_BASE   BIT(30)  /* Flags a conflicting base req.
                                               which has a different name to 
                                               the superseded base it is 
                                               conflicting with. */

#define FAILED_OEM_MISMATCH        BIT(29)  /* Flags an OEM manufactured
					       fileset attempting to apply on
					       a system not specific to that
					       OEM. */

#define FAILED_OEM_SPECIFIC        BIT(28)  /* Flags a non-OEM specific update
					       attempting to apply on top of
					       an OEM-specific fileset. */

#define FAILED_OEM_BASELEVEL       BIT(27)  /* Flags a non-OEM specific base
					       level attempting to apply on
					       top of an OEM-specific fileset.*/



/* NOTE!!! If the need arises to use BIT(32), the flags field will have to
   be split into 2 -- suggested: c_flags for control flags 
   (VISTING_THIS_NODE, SUBGRAPH_VISITED, etc.) and r_flags for report flags
 */
typedef struct
   {                        /* Define some single bit fields. */
     boolean requisite_visited            : 1;
     boolean selected_member_of_group     : 1; /* To show which elements of a
                                                * group are chosen. */

     boolean superseded_consistency_apply : 1; /* Used to flag root part fixes
                                                * being applied to be
                                                * consistent with the usr
                                                * part.  The report routines
                                                * use the flag. */

     boolean superseded_usr_commit        : 1; /* Used to flag a superseded usr
                                                * part that will be committed
                                                * even though it's root part
                                                * is not applied. (We use the
                                                * same value as
                                                * SUPERSEDED_CONSISTENCY_APPLY
                                                * since the two ops. are
                                                * mutually exclusive.) */

     boolean supersedes_fix               : 1; /* Used to flag the prereq/coreq
                                                * node of a superseding fix
                                                * that was selected by
                                                * pick_best_fix, instead of
                                                * the superseded fix. */

     boolean old_selected_member_of_group : 1; /* Used to mark a selected
                                                * member of group which has
                                                * been unselected temporarily
                                                * in evaluate_group_node. */
    } requisite_flags_type;

typedef struct fix_info              fix_info_type;
typedef struct dependants            dependants_type;
typedef struct requisite_list        requisite_list_type;
typedef struct supersede_list        supersede_list_type;
typedef struct relation_list         relation_type;
typedef struct index_node            index_list_type;
typedef struct base_lev_list         base_lev_list_type;

struct relation_list
 {
   token_type      operator;
   int             int_value;
   char            ptf_value[PTF_LEN];
   relation_type * next_relation;
 };

typedef struct
 {
   short           flags;
   int             hash_id;    /* used for speedy req_expr comparisons. */
   relation_type * version;
   relation_type * release;
   relation_type * modification;
   relation_type * fix;
   relation_type * ptf;
 } criteria_type;

/*
 * Bit values associated with criteria structure 
 */
#define EXACT_MATCH_REQUIRED  BIT (0)   /* tells the requisite lookup routines 
                                           not to do implicit supersedes
                                           lookups:
                                           -- when < in req expression
                                           -- when 4.1 update prereqs pkg from
                                              same option (normally its base)
                                           -- when looking up base level for
                                              ifreq processing.
                                         */

#define OLD_STYLE_REQ_EXPR    BIT (1)   /* the criteria structure, contains
				           "old style" requisite information
                                           of the form v=, m=, r>, etc. 
                                           Set in parse_assertion().
                                         */
                                     
struct dependants
 {
   fix_info_type    * fix_ptr;
   dependants_type  * next_dependant;
 };

struct base_lev_list
 {
   fix_info_type       * fix_ptr;
   base_lev_list_type  * next_base_lev;
 };

struct requisite_list
 {
   fix_info_type        * fix_ptr;
   base_lev_list_type   * base_lev_list;  /* Used for ifreq processing AND
                                             for reporting deinstal dependency
                                             errors when two products have
                                             "hard" requisites between updates
                                             but not between base levels.
                                           */
   criteria_type          criteria;
   requisite_type         type;
   requisite_flags_type   flags;
   requisite_list_type  * next_requisite;
 };

struct index_node
 {
   criteria_type          criteria;
   fix_info_type        * fix_ptr;
   Option_t             * sop_ptr;
   requisite_flags_type   flags;
   index_list_type      * next_index_node;
   boolean                unique_criteria;
 };

struct supersede_list
 {
   fix_info_type       * superseded_fix;
   fix_info_type       * superseding_fix;
   supersede_list_type * previous_supersede;
   supersede_list_type * next_supersede;
 };
/* The fix_info type is used to store prereq info.

   We keep a list of all unapplied fixes/installs that we have queried
   so far.  Chained off of each element is a list (prereqs) which can be used
   to figure out our prereqs.  This list is NOT simply a list of prereqs.
   See the comments for prereq_list_type for a better explanation.

   Second, this list is used when we are trying to evaluate an assertion_list.
   A assertion_list is a test specified in a prereq file, it has the format:

          <lpp_name> relation_expression

   Where a relation_expression can test combinations of the Version, Release,
   etc. for an lpp.

   To evaluate an assertion_list, we gather all known info about the LPP
   (from the VPD and possible fixes/installs from the TOC).  All of this info
   is placed in a list of this type.  Entries which fail the various relations
   are removed from the list.

   To make things easier for delete operations, heavily used during
   relation evaluation, a header node is used to evaluate the assertion_list.

 */
struct fix_info
 {
   char                 * name;
   char                 * product_name;
   Level_t                level;
   apply_state_type       apply_state;
   commit_type            commit_state;
   reject_type            reject_state;
   flags_type             flags;
   int                    op_type;         /* Same as Option_t op_type */
   fix_origin_type        origin;
   short                  parts_applied;   /* LPP_USER, LPP_ROOT, LPP_SHARE */
   short                  parts_committed; /* LPP_USER, LPP_ROOT, LPP_SHARE */
   short                  parts_in_fix;    /* LPP_USER, LPP_ROOT, LPP_SHARE */
   short                  passes_required;
   long                   cp_flag;         /* LPP_USER ... LPP_MICRO */
   short                  num_rejectable_requisites; /* Used for reject of */
   short                  num_rejected_requisites;   /*   group requisites. */
   requisite_list_type  * requisites;
   char                   superseding_ptf[MAX_PROD_SCED];
   char                 * supersedes_string;
   supersede_list_type  * supersedes;
   supersede_list_type  * superseded_by;
   char                 * prereq_file;
   Option_t             * TOC_Ptr;
   fix_info_type        * next_fix;
   fix_info_type        * prev_fix;
   fix_info_type        * collision_list;  /* Used for hash table. */

/* These fields are used during the prereq ordering process only (after all
   fixes have been decided on. */

   int                   number_of_requisites;
   dependants_type     * dependants;

   char                * apar_info;   /* Used by installp -l & lslpp */
   char                * description; /* Used by installp -l & lslpp */
  
   index_list_type     * rept_dependents;    /* Used by report_failures() */
   short                 rept_list_position; /* So is this... */
   char                  failure_sym;        /* Keeps track of requisite 
                                                failure type -- for reporting */
                                             
 };

typedef struct possible_fixes possible_fixes_type;

struct possible_fixes
 {
   fix_info_type       * fix_ptr;
   possible_fixes_type * next_fix;
 };

typedef struct
 {
   boolean          Auto_Include;          /* g flag from installp */
   boolean          Auto_Supersedes;       /* currently g flag */
   boolean          Force_Install;         /* F flag from installlp */
   boolean          Updates_Only_Install;  /* B flag from installp */
   boolean          instp_preview;
   V_flag_type      instp_verbosity;
   boolean          suppress_instreq_failures;  /* corresponds to -Q flag from
                                                   installp */
   int              instp_verify;
   boolean          instp_V0_verb;  /* special level of verbosity equivalent
                                       to V1 in inu_ckreq() in all but a few
                                       cases. */ 
   boolean          commit;
   boolean          no_save;
   FILE           * debug_file;
   boolean          called_from_command_line;
   boolean          called_from_ls_programs;
   boolean          called_from_lslpp;
   boolean          ignore_AVAILABLE_entries;
   boolean          installp_update;
   boolean          keep_apar_info;
   boolean          keep_description_info;
   Option_t       * First_Option;
   Option_t       * SOP;
   Option_t       * Fail_SOP;
   fix_info_type  * fix_info_anchor;
   int              mode;
   boolean          deinstall_submode;
   int              number_of_errors;
   int              function_return_code; /* Signals major errors. */
   FILE           * output_file;
   char           * output_file_name;
   short            parts_to_operate_on; /* LPP_USER LPP_ROOT LPP_SHARE */
   boolean          verbose;
   boolean          debug_mode;
   boolean          dump_on_error;
   boolean          consider_ifreqs;
   boolean          consider_unresolved_ifreqs;
   boolean          decision_reversal_occurred;
   int              graph_dump_code;
   int              media_type;
   boolean          mark_prereq_file;
   boolean          printed_finished_msg;
   boolean          successful_auto_supersedes;
   boolean          deinst_dependencies_via_updates; /* used to report cross-
                                                        prod depencies created
                                                        by updates for deinstl 
                                                        failure msgs.  */
 } check_prereq_type;

/* Internal Error Codes.  Used for internal errors and we don't want to create
   a new one to be translated.  */

#define FIX_ALREADY_IN_FINFO_TABLE    2   /* Trying to add fix to fix_info
                                             table but already there. */

#define INTERNAL_ERROR_FIX_NOT_ON_MEDIA 3

#define FIX_SHOULD_NOT_BE_IN_GRAPH    4   /* This essentially replaces the
                                             "reference count should be zero"
                                             message in the supersedes drop.
                                             A product has a state which doesn't
                                             agree with the SUCCESSFUL_NODE or 
                                             the MEMBER_OF_SOP bit.  There 
                                             should also be a product name 
                                             printed below this message. */

#define GROUP_SHOULDNT_HAVE_REQS      5   /* A group has a passes_required
                                             count of 0 but no prereqs or
                                             coreqs. */

#define SOP_ENTRY_NOT_IN_GRAPH        6    /* An entry in the sop was expected 
                                             in the graph passed back from 
                                             inu_ckreq to installp. */

#define EXPECTED_NODE_TO_BE_MARKED    7    /* A node in the graph should have
                                             been marked as either successful
                                             or candidate but it wasn't. */

                                           /* 7 through 11 pertain to hash 
                                              table creation and search
					      activities. */
#define HASH_TABLE_FULL               8 
#define HASH_TABLE_ACTIVE             9
#define HASH_TABLE_COLLISION         10
#define HASH_TABLE_DUPLICATE         11 
#define CANT_FIND_FIX                12

#define BAD_FAIL_CODE                13      /* unexpected failure code 
                                                passed to failure code  
						calculation routine. */

#define INCONSISTNT_PARTS_CODE       14      /* Database indicates that a 
						package has a share part
 						and a root part OR perhaps 
						installp didn't catch a request
						to install share parts only on 
						a package which doesn't contain
						share parts (often a result of 
						a messed up cp_flag in the 
						database). */

#define ORDER_APPLY_LOOP_CODE        15      /* A "residual" pkg was detected
						in the list of items to apply.
          					Usually means an illegal 
						prereq loop was detected.
						Could be a real prereq loop
						but more than likely a result
						of memory corruption.  
						(struct was freed when it 
						wasn't supposed to be.)  This
						only applies to "medialess"
						apply ops (ie. root ops).*/

#define ORDER_COMT_REJ_LOOP_CODE     16      /* Same as 14 but applies to
						commit and reject ops. */

#define ORDER_APPLY_LOOP2_CODE       17      /* Same as 14, but applies to
						apply ops with media. */

#define STACK_UFLOW_CODE             18      /* Cycle detection stack 
						underflow */

#define UNEXP_MICRO_CODE             19      /* Even though op_type says
						LPP_MICRO, it does not say
						its a 31 formatted pkg. */

#define COMT_REJ_OPT_MISSING_CODE    20      /* None of the pkgs passed to
						inu_ckreq() on sop for 
						non-apply op were in the vpd.*/

#define LOOKING_FOR_SUPS_IN_REQ_MATCH 21     /* Failed to find a superseding
                                                fix when one was supposed to
                                                be there.  See 
                                                evaluate_base_levels() and
                                                evaluate_superseding_updates()
                                                for potential cause. */

#define PICKED_WRONG_BEST_BASE        22     /* Picked a base level which does
                                                not appear to be the best 
                                                choice. */

#define NO_PRQ_FOR_41_UPDT            23     /* Couldn't find a prereq for
                                                a 4.1 update when verifying
                                                graph. */

#define FOUND_UNVISITED_NODE_IN_EVALUATE_BASE_LEVELS 24
                                             /* A node should have been marked
                                                with the VISITING_THIS_NODE
                                                tag bit but wasn't. */

#ifdef DEFINE_EXTERNALS
  check_prereq_type check_prereq;

#else
  extern check_prereq_type check_prereq;

#endif

#endif  /* _H_CKP_COMMON_DEFS */
