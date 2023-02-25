static char sccsid[] = "@(#)62  1.4  src/bos/usr/sbin/install/ckprereq/hash_utils.c, cmdinstl, bos411, 9428A410j 5/25/94 08:45:24";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS:
 *            add_to_fix_hash_table 
 *            create_hash_table 
 *            destroy_hash_table
 *            grow_hash_list 
 *            grow_hash_table 
 *            load_fix_hash_table
 *            locate_hash_entry
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Includes
 */

#include <check_prereq.h>

static void grow_hash_table (boolean * const error);

static void grow_hash_list (boolean * const error);

static ENTRY * hash_list;
static int     hash_index;
static int     hash_table_size;

#ifndef STRESS_MALLOC
  #define HASH_TABLE_CHUNK 4096
#else
  #define HASH_TABLE_CHUNK 4
#endif

/*--------------------------------------------------------------------------*
**
** NAME: create_hash_table
**
** FUNCTION:  Sets up memory for a hash table of a given size.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void create_hash_table (boolean * const error)
 {
   if (hash_table_size != 0)
    {
      inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_INT_CODED_MSG_E,
               CKP_INT_CODED_MSG_D), inu_cmdname, HASH_TABLE_ACTIVE);
      * error = TRUE;
      check_prereq.function_return_code = INTERNAL_ERROR_RC;
      return;
    }
   hash_table_size = HASH_TABLE_CHUNK;

   if (hcreate (hash_table_size) == 0)
    {
      inu_msg (ckp_errs, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
               CMN_MALLOC_FAILED_D), inu_cmdname);
      check_prereq.function_return_code = MALLOC_ERROR_RC;
      * error = TRUE;
      return;
    }

   hash_list = my_malloc (hash_table_size * sizeof (ENTRY), error);
   RETURN_ON_ERROR;

   hash_index = 0;
 } /* end create_hash_table */

/*--------------------------------------------------------------------------*
**
** NAME: destroy_hash_table
**
** FUNCTION:  Frees up all memory for the current hash_table.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void destroy_hash_table (void)
 {
   hdestroy ();
   hash_table_size = 0;
   my_free (hash_list);
   hash_list = NIL (ENTRY);
   hash_index = 0;
 } /* end destroy_hash_table */

/*--------------------------------------------------------------------------*
**
** NAME: grow_hash_table
**
** FUNCTION:  Gets a bigger hash table and reloads it.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void grow_hash_table (boolean * const error)
 {
   ENTRY * hash_entry;
   int     local_hash_index;
   int     number_of_elements;

   hdestroy ();
   number_of_elements = hash_table_size + HASH_TABLE_CHUNK;
   if (hcreate (number_of_elements) == 0)
    {
      inu_msg (ckp_errs, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
               CMN_MALLOC_FAILED_D), inu_cmdname);
      check_prereq.function_return_code = MALLOC_ERROR_RC;
      * error = TRUE;
      return;
    }

   /* Now reload hash table.

      Just rip through our hash_list and rehash them into the table.  We don't
      have to worry about collisions this time because they were taken care
      of the first go-round and only the unique keys were added to the
      hash_list. */

   for (local_hash_index = 0;
        local_hash_index <= hash_index;
        local_hash_index++)
    {
      hash_entry = hsearch (hash_list[local_hash_index], ENTER);

      if (hash_entry == NIL (ENTRY) )
       {
         /* The table is full, should not be able to happen! */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_INT_CODED_MSG_E,
                  CKP_INT_CODED_MSG_D), inu_cmdname, HASH_TABLE_FULL);
         * error = TRUE;
         check_prereq.function_return_code = INTERNAL_ERROR_RC;
         return;
       }

      /* Make sure we don't think there is a collision. */

      if (hash_entry -> data != hash_list[local_hash_index].data)
       {
         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_INT_CODED_MSG_E,
                  CKP_INT_CODED_MSG_D), inu_cmdname, HASH_TABLE_COLLISION);
         * error = TRUE;
         check_prereq.function_return_code = INTERNAL_ERROR_RC;
         return;
       }
    } /* end for */

 } /* end grow_hash_table */

/*--------------------------------------------------------------------------*
**
** NAME: grow_hash_list
**
** FUNCTION:  Gets more memory for the hash_list.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

static void grow_hash_list (boolean * const error)
 {
   /* We grow the hash_list and hash table seperately because the hash table
      is padded by some factor by hcreate.  Unfortunately, we don't know the
      pad value.  So... we can run out of room in our hash_list before the
      hash table is full. */

   hash_table_size += HASH_TABLE_CHUNK;
   hash_list = my_realloc (hash_list, hash_table_size * sizeof (ENTRY),
                           error);
   RETURN_ON_ERROR;

 } /* end grow_hash_list */

/*--------------------------------------------------------------------------*
**
** NAME: load_fix_hash_table
**
** FUNCTION:  Puts the given fix list into a hash table.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void load_fix_hash_table (fix_info_type * const fix_list,
                          boolean       * const error)
 {
   boolean         duplicate;
   fix_info_type * fix;
   fix_info_type * previous_fix;

   previous_fix = fix_list;
   fix = fix_list -> next_fix;
   while (fix != NIL (fix_info_type) )
    {
       add_to_fix_hash_table (fix, & duplicate, error);
       RETURN_ON_ERROR;

       if (duplicate)
        {
          previous_fix -> next_fix = fix -> next_fix;
          if (fix->next_fix != NIL (fix_info_type))
             fix->next_fix->prev_fix = fix->prev_fix;
          free_fix_info_node (fix);
        }
       else
        {
          previous_fix = fix;
        } /* end if */
      fix = previous_fix -> next_fix;
    } /* end while */


 } /* end load_fix_hash_table */

/*--------------------------------------------------------------------------*
**
** NAME: add_to_fix_hash_table
**
** FUNCTION:  Puts the given fix into a hash table.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void add_to_fix_hash_table (fix_info_type * const fix,
                            boolean       * const duplicate,
                            boolean       * const error)
 {
   fix_info_type * collision;
   fix_info_type * found_fix;
   ENTRY         * hash_entry;
   fix_info_type * previous_fix;

   /* We do not have a unique key for our hash table.  We could generate
      one by converting lpp_name.ver.rel.mod.fix.ptf into a string, but this
      is pretty expensive, so just use ptf or lpp_name as appropriate and go
      through the pain of handling collisions. */

   /* Use the PTF field if it has something in it, otherwise use name. */

   if ((hash_index + 1) >= hash_table_size)
    {
      grow_hash_list (error);
      RETURN_ON_ERROR;
    }

   if (fix -> level.ptf[0] == '\0')
      hash_list[hash_index].key = fix -> name;
    else
      hash_list[hash_index].key = fix -> level.ptf;

   /* Make sure that our list for collision handling is clean. */

   fix -> collision_list = NIL (fix_info_type);
   hash_list[hash_index].data = (void *) fix;
   hash_entry = hsearch (hash_list[hash_index], ENTER);

   if (hash_entry == NIL (ENTRY) )
    {
      /* The table is full, get a bigger hash table. */

      grow_hash_table (error);
      RETURN_ON_ERROR;

      hash_entry = hsearch (hash_list[hash_index], ENTER);
      if (hash_entry == NIL (ENTRY) )
       {
         /* The table is full, should not be able to happen! */

         inu_msg (ckp_errs, MSGSTR (MS_CKPREREQ, CKP_INT_CODED_MSG_E,
                  CKP_INT_CODED_MSG_D), inu_cmdname, HASH_TABLE_FULL);
         * error = TRUE;
         check_prereq.function_return_code = INTERNAL_ERROR_RC;
         return;
       }
    }

   /* If we have a duplicate key:
         * Ignore if for the same_fix
         * Otherwise, add to collison list. */

   if ((fix_info_type *) (hash_entry -> data) == fix)
    {
      hash_index++;  /* This hash_list entry is unique (no collisions) */
    }
   else
    {
      * duplicate = FALSE;

      for (collision = (fix_info_type *) hash_entry -> data;
           collision != NIL (fix_info_type);
           collision = collision -> collision_list)
       {
         if (same_fix (fix, collision) )
          {
            * duplicate = TRUE;
            return;
          }
       } /* end for */

      /* Not in list, go ahead and add after first entry. */

      found_fix = (fix_info_type *) (hash_entry -> data);
      fix -> collision_list = found_fix -> collision_list;
      found_fix -> collision_list = fix;
    }
   * duplicate = FALSE;
   return;
 } /* end add_fix_to_hash_table */

/*--------------------------------------------------------------------------*
**
** NAME: locate_hash_entry
**
** FUNCTION:  locates the given lpp_name and level in the hash table.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

ENTRY * locate_hash_entry (char * const lpp_name,
                           char * const ptf)
 {
   ENTRY hash_entry;

    /* Use the PTF field if it has something in it, otherwise use name. */

    if (ptf[0] == '\0')
       hash_entry.key = lpp_name;
     else
       hash_entry.key = ptf;

    return (hsearch (hash_entry, FIND) );

 } /* end locate_hash_entry */
