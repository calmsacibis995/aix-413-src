/* @(#)71       1.18  src/bos/usr/sbin/install/include/inulib.h, cmdinstl, bos41J, 9513A_all 3/23/95 12:30:56 */

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


#ifndef _H_INULIB
#define _H_INULIB

#define _ILS_MACROS
#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/limits.h>
#include <unistd.h>
#include <inudef.h>
#include <inu_toc.h>
#include <toc0.h>
#include <swvpd.h>
#include <swvpd1.h>
#include <IN/AFdefs.h>  /* This guy does NOT have #ifndefs */
#include <errno.h>
#include <inuerr.h>
#include <instlmsg.h>
#include <commonmsg.h>
#include <installpmsg.h>
#include <ctype.h>
/* #include <nl_types.h> */


#ifdef MAIN_PROGRAM
   char * inu_cmdname;
   char * NullString = "";
   char * dashed_line =
"  ----------------------------------------------------------------------------\n";
#else
   extern char * inu_cmdname;
   extern char * NullString;
   extern char * dashed_line;
#endif


char * decapitate (char option_name[],
                   char first_name[]);

void inu_fget_fs_sizes (FILE   * fp,
                        size_t   total[],
                        size_t   max_temp[]);


void inu_fget_file_sizes (FILE   * fp,
                          size_t   total[],
                          int      active);

int inu_fcopy (FILE * fd1,
               FILE * fd2);

int inu_expand_ps (int needed_pagesize);

void trequest_vol (int    vol,
                   char * media);

int topen (char * media,
           int    vol,
           int    mode,
           int    quiet_flag);

void tclose (int tape);

int trewind (int tape);

int tfile_mark (int tape);

int twrite_record (int    tape,
                   char * record,
                   int    rec_len);

int tread_record (int    tape,
                  char * record,
                  int    n_bytes);

int tcopy_file (char * file_name,
                int    tape);

int tseek_file (int tape,
                int nfiles,
                int seek_code);

int tseek_record (int tape,
                  int nrecs,
                  int seek_code);

int ttape_size (int tape);

int tape_err (int err);

int inu_cp (char source[],
            char target[]);

int inu_cat (char * infile,
             char * outfile,
             char * mode);

int inu_archive (FILE * acf_fp,
                 FILE * apply_fp,
                 FILE * del_fp);

int enough_space (char * Command);

int expand_lv (char   * i_filesystem,
               size_t   total_blox_requested);

int in_al (char * member,
           FILE * al);

char * getbasename (char * path);

int get_stat (struct vmount ** vmountpp);

int get_lppname (FILE * fp,
                 char * str);

int find_pfs (char * file);

int find_lfs (char * dir);

int expand_lv (char   * i_filesystem,
               size_t   total_blox_requested);

int inu_level_compare (Level_t * a,
                       Level_t * b);

int open_toc (TOC_t ** toc,
              char   * media,
              int      q_flag,
              int      caller);


void insert_bffref (BffRef_t *, BffRef_t *);

int load_tape_toc (TOC_t * toc,
                   char *  media,
                   int     tape);

int load_stack_disk_toc (TOC_t * toc,
                         char  * media);

int load_bff_toc (TOC_t * toc,
                  char  * media,
                  int     class,
                  int     q_flag,
                  int     caller);

int load_disk_toc (TOC_t * toc,
                   char  * media);

int toc_size (TOC_t * toc);

int set_hdr_fmt (int  * hdr_fmt,
                 char * string);

int set_content (Option_t * op);

void set_vpdtree (Option_t *op);

void set_op_type (Option_t * op);

int get_opt_info (Option_t * op,
                  FILE     * media_ptr);

int read_opt_info (char ** mbuf,
                   FILE  * media_ptr);

void inu_dump_lvl_msg (Option_t * op,
                       char     * level);

int inu_del_op (Option_t * sop,
                Option_t *next_op,
                Option_t *ptr);

void inu_free_op (Option_t * optptr);

void inu_free_bff (BffRef_t * bffptr);

int free_option_list (Option_t * optptr);

void inu_signal (int sig, void (*funky) ());

void inu_set_abort_handler (void (*funky) ());

void inu_ignore_all_sigs (void);

void inu_setall_sigs (void (*funky) ());

void inu_init_sigs (void);

void inu_default_sigs (void);

void inu_before_system_sigs (void);

void inu_rm_save_dir (char * path_name);

int inu_rm (char * filename);

int inu_restore (char     * device,
                 int        quiet,
                 int        type,
                 Option_t * sop,
                 char     * file_names);

int inu_read_file (FILE  * fp,
                   char ** mem_buf);


int inu_rcvr_or_rm (char * optionfile,
                    int    caller,
                    char * lppname);

int inu_position_tape (Option_t * sop);

void insert_bffref (BffRef_t * L,
                    BffRef_t * ref);

void insert_optionref (OptionRef_t * L,
                       OptionRef_t * ref);

void insert_option (Option_t * L,
                    Option_t * option);

TOC_t * create_toc (BffRef_t * bff_list,
                    Option_t * op_list);

BffRef_t * create_bffref (int    vol,
                          int    off,
                          int    size,
                          char   fmt,
                          char   platform,
                          char * media,
                          int    crc,
                          int    action);

Option_t * create_option (char     * name,
                          int        op_checked,
                          int        quiesce,
                          char       content,
                          char     * lang,
                          Level_t  * level,
                          char     * desc,
                          BffRef_t * bff);

OptionRef_t * create_optionref (Option_t * option);

Option_t * InsertSop_toc (Option_t * L,
                          Option_t * op);

Option_t * copy_option (Option_t * op);

int inu_mv (char * from,
            char * to);

void inu_msg (int   output_indicator,
              char * format,
              ... );

int inu_mk_mstr_al (char * apply_list,
                    char * sorted_al);

int inu_mk_dir (char * dir);

int inu_testsetlock (char * path);

void inu_unsetlock (int fildes);

int inu_level_convert (char    * i_level,
                       Level_t * level);

void inu_31level_convert (char    * i_level,
                          Level_t * level);

int is_file (char * device);

int mk_fs_list (void);

void inu_get_lppname (char * op_name,
                      char * lppname);

void inu_get_prodname (Option_t * op,
                       char     * prodname);

void inu_get_optlevname_from_Option_t (Option_t *op,
                                        char name_lev[]);

void inu_get_lpplevname_from_Option_t (Option_t *op,
                                       char name_lev[]);

void inu_prepend_str (char * prestr,
                      char * rootstr,
                      char * deststr);

int inurddk (char * dev,
             int    vol,
             char * volid,
             int    beg,
             int    num,
             int  (* wfunc) (),
             int    prompt);

int inu_file_eval_size (char * topath,
                        char * frompath,
                        int    active);

void rm_1_opt (char * svdir,
               char * option);

void inu_get_fs_sizes (char   * ptr,
                       size_t   total[],
                       size_t   max_temp[]);

void set_media_field (prod_t * prod_rec,
                      TOC_t  * head);

int inu_do_exec (char * cmd);


boolean path_exists (char *path);

boolean vappend_file   (char *file, char *format, ...); 

char * inu_tree_string (char);

char * inu_unpad (char *);

int inu_deinst_chk (char *, char *, char, int);

char * get_migdir_prefix (Option_t *op);

char * get_fsname (char *path);


/* A few function definitions that don't exist elsewhere. */

char * mktemp (char * template);

char * basename (char * path);

char * getwd (char * path);

char * AFread (AFILE_t af);

char * regcmp (char * string, ...);

char * regex (char * pattern,
              char * subject,
              char * ret, ...);

nl_catd  catd;


#endif  /* _H_INULIB */

