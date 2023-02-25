static char sccsid[] = "@(#)76  1.4.1.17  src/bos/usr/sbin/install/ckprereq/mem_utils.c, cmdinstl, bos411, 9437B411a 9/11/94 16:02:48";

/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: 
 *            create_base_lev_list_node
 *            create_dummy_group
 *            create_DUMMY_FIX
 *            create_fix_info_node
 *            create_possible_fix_node 
 *            create_relation 
 *            create_requsite 
 *            dupe_relations 
 *            free_fix_info_node
 *            free_options
 *            free_possible_fix_node 
 *            free_relations
 *            free_requisite 
 *            my_free 
 *            my_malloc 
 *            my_realloc 
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

/*
 * Includes
 */

#include <check_prereq.h>

static char Unknown_Name[] = "unknown name";

/*--------------------------------------------------------------------------*
**
** NAME: create_base_lev_list_node
**
** FUNCTION:  mallocs and initializes (to NULL) base_level_list type structure.
**
** RETURNS:   Returns a character pointer to the storage malloced.
**
**--------------------------------------------------------------------------*/

base_lev_list_type * create_base_lev_list_node (boolean * error)
{
   base_lev_list_type * node;

   node = (base_lev_list_type *) my_malloc (sizeof (base_lev_list_type), error);
   if (* error)
      return (NIL (base_lev_list_type) );
 
   node->next_base_lev = NIL (base_lev_list_type);
   node->fix_ptr = NIL (fix_info_type);
   return (node);

}  /* end create_base_lev_list_node */

/*--------------------------------------------------------------------------*
**
** NAME: my_malloc
**
** FUNCTION:  This routine provides a wrapper around malloc so that we can
**            easily trap any errors.  Any errors cause us to terminate.
**
** RETURNS:   Returns a character pointer to the storage malloced.
**
**--------------------------------------------------------------------------*/

void * my_malloc (size_t size, boolean * error)

 {
    char * ptr;

    ptr = malloc (size);
    if (ptr == NIL (char))
     {
       inu_msg (ckp_errs, MSGSTR (MS_INUCOMMON, CMN_MALLOC_FAILED_E, 
                                                CMN_MALLOC_FAILED_D),
                inu_cmdname);
       check_prereq.function_return_code = MALLOC_ERROR_RC;
       * error = TRUE;
     }
    return (ptr);

 }  /* end my_malloc */

/*--------------------------------------------------------------------------*
**
** NAME: my_realloc
**
** FUNCTION:  This routine provides a wrapper around realloc so that we
**            can easily trap any errors.  Any errors cause us to
**            terminate.
**
** RETURNS:   Returns a character pointer to the storage realloced.
**
**--------------------------------------------------------------------------*/

void * my_realloc (void * source, size_t size, boolean * error)

 {
    char * ptr;

    ptr = realloc (source, size);
    if (ptr == NIL (char))
     {
       inu_msg (ckp_errs, MSGSTR (MS_INUCOMMON, CMN_REALLOC_FAILED_E,
                CMN_REALLOC_FAILED_D), inu_cmdname);
       check_prereq.function_return_code = MALLOC_ERROR_RC;
       * error = TRUE;
     }
    return (ptr);
 }  /* end my_realloc */

/*--------------------------------------------------------------------------*
**
** NAME: my_free
**
** FUNCTION:  This routine provides a wrapper around free so that we
**            can easily trap any errors.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void my_free (void * ptr)
 {
    free (ptr);
 } /* end my_free */

/*--------------------------------------------------------------------------*
**
** NAME: free_fix_info_node
**
** FUNCTION:  This routine frees a fix_info_type node.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void free_fix_info_node (fix_info_type * node)
 {
   dependants_type     * dependant;
   requisite_list_type * next_requisite;
   dependants_type     * next_dependant;
   supersede_list_type * next_supersede;
   requisite_list_type * requisite;
   supersede_list_type * supersede;

   if ( (node -> name != NIL (char))
                  &&
        (node -> name != Unknown_Name) )
      my_free (node -> name);

   if ( (node -> product_name != NIL (char))
                  &&
        (node -> product_name != Unknown_Name) )
      my_free (node -> product_name);

   if (node -> prereq_file != NIL (char))
      my_free (node -> prereq_file);

   node -> prereq_file = NIL (char);

   if (node -> apar_info != NIL (char))
      my_free (node -> apar_info);

   if (node -> description != NIL (char))
      my_free (node -> description);

   if (node -> supersedes_string != NIL (char))
      my_free (node -> supersedes_string);

   requisite = node -> requisites;
   while (requisite != NIL (requisite_list_type))
    {
      next_requisite = requisite -> next_requisite;
      free_requisite (requisite);
      requisite = next_requisite;
    } /* end while */
   node -> requisites = NIL (requisite_list_type);

   supersede = node -> supersedes;
   while (supersede != NIL (supersede_list_type))
    {
      next_supersede = supersede -> next_supersede;
      my_free (supersede);
      supersede = next_supersede;
    }
   node -> supersedes = NIL (supersede_list_type);

   supersede = node -> superseded_by;
   while (supersede != NIL (supersede_list_type))
    {
      next_supersede = supersede -> next_supersede;
      my_free (supersede);
      supersede = next_supersede;
    }
   node -> superseded_by = NIL (supersede_list_type);

   dependant = node -> dependants;
   while (dependant != NIL (dependants_type))
    {
      next_dependant = dependant -> next_dependant;
      my_free (dependant);
      dependant = next_dependant;
    }
   node -> dependants = NIL (dependants_type);

   /* Let's set a few more pointers to NIL.  We don't want to delete them
      because this is not a unique reference, I just want them NIL so that
      we don't pretend that this node is still somehow useful. */

   node -> TOC_Ptr = NIL (Option_t);
   node -> next_fix = NIL (fix_info_type);
   node -> prev_fix = NIL (fix_info_type);

   my_free (node);

 } /* end free_fix_info_node */

/*--------------------------------------------------------------------------*
**
** NAME: free_possible_fix_node
**
** FUNCTION:  This routine frees a fix_possible_fix node.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void free_possible_fix_node (possible_fixes_type * node)
 {
   my_free (node);

 } /* end free_possible_fix_node */

/*--------------------------------------------------------------------------*
**
** NAME: create_fix_info_node
**
** FUNCTION:  Creates an initialized node of fix_info_type.
**
** RETURNS:   Returns a pointer to the node created.
**
**--------------------------------------------------------------------------*/

fix_info_type * create_fix_info_node (boolean * error)
{
  fix_info_type * node;

  node = (fix_info_type *) my_malloc (sizeof (fix_info_type), error);
  if (* error)
     return (NIL (fix_info_type) );

  node -> name = Unknown_Name;
  node -> product_name = Unknown_Name;
  node -> level.sys_ver = 0;
  node -> level.sys_rel = 0;
  node -> level.sys_mod = 0;
  node -> level.sys_fix = 0;
  node -> level.ver = 0;
  node -> level.rel = 0;
  node -> level.mod = 0;
  node -> level.fix = 0;
  node -> level.ptf[0] = '\0';
  node -> apply_state = UNKNOWN;  /* Don't know if we can apply */
  node -> commit_state = COMMIT_UNKNOWN;
  node -> reject_state = REJECT_UNKNOWN;
  node -> flags = 0;
  node -> origin = UNKNOWN_ORIGIN;
  node -> parts_applied = 0;
  node -> parts_committed = 0;
  node -> parts_in_fix = 0;
  node -> cp_flag = 0;
  node -> op_type = 0;
  node -> passes_required = 0;
  node -> requisites = NIL (requisite_list_type);
  node -> superseding_ptf[0] = '\0';
  node -> supersedes_string = NIL (char);
  node -> supersedes = NIL (supersede_list_type);
  node -> superseded_by = NIL (supersede_list_type);
  node -> prereq_file = NIL (char);
  node -> TOC_Ptr = NIL (Option_t);
  node -> next_fix = NIL (fix_info_type);
  node -> prev_fix = NIL (fix_info_type);
  node -> number_of_requisites = 0;
  node -> dependants = NIL (dependants_type);
  node -> apar_info = NIL (char);
  node -> description = NIL (char);
  node -> collision_list = NIL (fix_info_type);
  node -> num_rejected_requisites = 0;
  node -> num_rejectable_requisites = 0;
  node -> rept_dependents = NIL (index_list_type);
  node -> failure_sym = '\0';

  return (node);
} /* end create_fix_info_node */

/*--------------------------------------------------------------------------*
**
** NAME: create_possible_fix_node
**
** FUNCTION:  Creates an initialized node of possible_fixes_type.
**
**
** RETURNS:   Returns a pointer to the node created.
**
**--------------------------------------------------------------------------*/

possible_fixes_type * create_possible_fix_node (boolean * error)

 {
   possible_fixes_type  * possible_fix;

   possible_fix =
            (possible_fixes_type *) my_malloc (sizeof (possible_fixes_type),
                                               error);
   if (* error)
      return (NIL (possible_fixes_type));

   possible_fix -> next_fix = NIL (possible_fixes_type);
   return (possible_fix);

 } /* end create possible_fix_node */

/*--------------------------------------------------------------------------*
**
** NAME: create_dummy_group
**
** FUNCTION:  creates a dummy group node
**
** RETURNS:   Returns a pointer to the created node.
**
**--------------------------------------------------------------------------*/

void create_dummy_group (fix_info_type ** group_node,
                         boolean        * error)
 {
   static int             group_id;
          fix_info_type * group_result;
          char            group_name[20];

   * group_node = create_fix_info_node (error);
   RETURN_ON_ERROR;

   group_result = * group_node;
   sprintf (group_name, "Group %d", group_id++);
   group_result -> name = dupe_string (group_name, error);
   RETURN_ON_ERROR;

   group_result -> apply_state = ALL_PARTS_APPLIED;
   group_result -> commit_state = ALL_PARTS_COMMITTED;
   group_result -> reject_state = NOT_REJECTED;
   group_result -> flags |= CANDIDATE_NODE;
   group_result -> origin = DUMMY_GROUP;

   add_fix_to_fix_info_table (group_result, error);
   RETURN_ON_ERROR;

   return;
 } /* end create_dummy_group */

/*--------------------------------------------------------------------------*
**
** NAME: create_DUMMY_FIX
**
** FUNCTION:  creates a dummy fix_info_structure with origin == DUMMY_FIX
**
** RETURNS:   Returns a pointer to the created node.
**
**--------------------------------------------------------------------------*/

void create_DUMMY_FIX (fix_info_type ** dummy,
                         char           * option_name,
                         Level_t        * level,
                         boolean        * error)
 {
   (*dummy) = create_fix_info_node (error);
   RETURN_ON_ERROR;
   (*dummy)->name = dupe_string (option_name, error);
   RETURN_ON_ERROR;
   (*dummy)->level = (*level);
   (*dummy)->apply_state  = CANNOT_APPLY;
   (*dummy)->commit_state = CANNOT_COMMIT;
   (*dummy)->reject_state = REJECT_UNKNOWN;
   (*dummy)->origin       = DUMMY_FIX;

   return;

 } /* end create_DUMMY_FIX */

/*--------------------------------------------------------------------------*
**
** NAME: create_requisite
**
** FUNCTION:  This routine creates a requisite node and initializes all of the
**            proper fields.
**
** RETURNS:   Returns a pointer to the created node.
**
**--------------------------------------------------------------------------*/

requisite_list_type * create_requisite (criteria_type * criteria,
                                        boolean       * error)
 {
   requisite_list_type * requisite;

   requisite = (requisite_list_type *) my_malloc (sizeof (requisite_list_type),
                                                  error);
   if (* error)
      return (NIL (requisite_list_type) );

   requisite -> fix_ptr = NIL (fix_info_type);
   requisite -> base_lev_list = NIL (base_lev_list_type);
   requisite -> criteria.flags = criteria -> flags;
   requisite -> criteria.hash_id = criteria -> hash_id;
   requisite -> criteria.version = criteria -> version;
   requisite -> criteria.release = criteria -> release;
   requisite -> criteria.modification = criteria -> modification;
   requisite -> criteria.fix = criteria -> fix;
   requisite -> criteria.ptf = criteria -> ptf;

   clear_requisite_flags (& requisite -> flags);
   requisite -> type = UNKNOWN_REQUISITE;
   requisite -> next_requisite = NIL (requisite_list_type);

   return (requisite);

 } /* end create_requisite */

/*--------------------------------------------------------------------------*
**
** NAME: free_requisite
**
** FUNCTION:  This routine frees a requisite node and all of its criteria
**            fields.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void free_requisite (requisite_list_type * requisite)
 {
   if (requisite -> criteria.version != NIL (relation_type) )
      free_relations (requisite -> criteria.version);
   requisite -> criteria.version = NIL (relation_type);

   if (requisite -> criteria.release != NIL (relation_type) )
      free_relations (requisite -> criteria.release);
   requisite -> criteria.release = NIL (relation_type);

   if (requisite -> criteria.modification != NIL (relation_type) )
      free_relations (requisite -> criteria.modification);
   requisite -> criteria.modification = NIL (relation_type);

   if (requisite -> criteria.fix != NIL (relation_type) )
      free_relations (requisite -> criteria.fix);
   requisite -> criteria.fix = NIL (relation_type);

   if (requisite -> criteria.ptf != NIL (relation_type) )
      free_relations (requisite -> criteria.ptf);
   requisite -> criteria.ptf = NIL (relation_type);
   my_free (requisite);

 } /* free_requisite */

/*--------------------------------------------------------------------------*
**
** NAME: free_relations
**
** FUNCTION:  This routine frees a list of relations.
**
** RETURNS:   This is a void function.
**
**--------------------------------------------------------------------------*/

void free_relations (relation_type * relation)
 {
   relation_type * next_relation;

   while (relation != NIL (relation_type) )
    {
      next_relation = relation -> next_relation;
      my_free (relation);
      relation = next_relation;
    }

 } /* free_relations */

/*--------------------------------------------------------------------------*
**
** NAME: clear_requisite_flags
**
** FUNCTION:  Sets all of the flag fields to zero.
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

void clear_requisite_flags (requisite_flags_type * requisite_flag)
 {
   memset (requisite_flag, 0, sizeof (requisite_flags_type));
   requisite_flag -> selected_member_of_group = FALSE;
   requisite_flag -> superseded_consistency_apply  = FALSE;
   requisite_flag -> superseded_usr_commit  = FALSE;
   requisite_flag -> supersedes_fix  = FALSE;
   requisite_flag -> old_selected_member_of_group  = FALSE;
 }

/*--------------------------------------------------------------------------*
**
** NAME: free_options
**
** FUNCTION:  This routine frees every option, except for the header, in this
**            linked list of options.
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

void free_options (Option_t * Option_Ptr)
 {
   Option_t * option;
   Option_t * previous_option;

   option = Option_Ptr -> next;
   previous_option = Option_Ptr;
   while (option != NIL (Option_t))
    {
      previous_option -> next = option -> next;
      my_free (option);
      option = previous_option -> next;
    } /* end while */

   Option_Ptr -> next = NIL (Option_t);

 } /* end free_options */

/*--------------------------------------------------------------------------*
**
** NAME: create_relation
**
** FUNCTION:  This routine creates a relation and links it into the appropriate
**            list of the criteria that it is associated with.
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

void create_relation (criteria_type * criteria,
                      token_type      VPD_field,
                      token_type      operator,
                      short           int_value,
                      char          * PTF_value,
                      boolean       * error)
{
   relation_type * relation;
   relation_type * rel;
   char          * ptr;

   relation = my_malloc (sizeof (relation_type), error);
   RETURN_ON_ERROR;

   relation -> operator = operator;
   if (operator == LESS_THAN)
      criteria->flags |= EXACT_MATCH_REQUIRED;

   relation -> int_value = int_value;
   relation -> ptf_value[0] = '\0';
   relation -> next_relation = NIL (relation_type);

   /*
    * Calculate a (cumulative) hash value for the requisite expression
    * which can be used to efficiently check for equality between 
    * requisite expressions (see same_criteria()).
    * For non-ptfs, use the sum of the current relations operator (its 
    * numeric value) and the integer part of the relation.
    * (ex. req expression: v=2 r=1 o>1 
    *   hash_id = (int) EQUALS + 2 (int) EQUALS + 1 + (int) GREATHER_THAN + 1);
    * For ptfs, use the summation of the ascii value of each character.
    */  
   if (VPD_field != PTF) 
   {
      criteria->hash_id += ((int) operator + (int) int_value);
   }
   else
   {
      ptr = PTF_value;
      while ((*ptr) != '\0')
      { 
           criteria->hash_id += (int) *ptr;
           ptr++;
      }
      criteria->hash_id += (int) operator;
   }
   switch (VPD_field)
    {
      case VERSION :
         rel = criteria -> version;
         if (rel != NIL (relation_type))
         {
            while (rel -> next_relation != NIL (relation_type))
            {
               rel = rel -> next_relation;
            }
            rel -> next_relation = relation;
            relation -> next_relation = NIL (relation_type);
         }
         else
         {
            relation -> next_relation = criteria -> version;
            criteria -> version = relation;
         }
         break;

      case RELEASE :
         rel = criteria -> release;
         if (rel != NIL (relation_type))
         {
            while (rel -> next_relation != NIL (relation_type))
            {
               rel = rel -> next_relation;
            }
            rel -> next_relation = relation;
            relation -> next_relation = NIL (relation_type);
         }
         else
         {
            relation -> next_relation = criteria -> release;
            criteria -> release = relation;
         }
         break;

      case MODIFICATION :
         rel = criteria -> modification;
         if (rel != NIL (relation_type))
         {
            while (rel -> next_relation != NIL (relation_type))
            {
               rel = rel -> next_relation;
            }
            rel -> next_relation = relation;
            relation -> next_relation = NIL (relation_type);
         }
         else
         {
            relation -> next_relation = criteria -> modification;
            criteria -> modification = relation;
         }
         break;

      case FIX :
         rel = criteria -> fix;
         if (rel != NIL (relation_type))
         {
            while (rel -> next_relation != NIL (relation_type))
            {
               rel = rel -> next_relation;
            }
            rel -> next_relation = relation;
            relation -> next_relation = NIL (relation_type);
         }
         else
         {
            relation -> next_relation = criteria -> fix;
            criteria -> fix = relation;
         }
         break;

      case PTF :
         relation -> next_relation = criteria -> ptf;
         strcpy (relation -> ptf_value, PTF_value);
         relation -> int_value = -1;
         criteria -> ptf = relation;
         break;

      default :  /* Should never happen. */
         break;
    } /* end switch */

} /* end create_relation */

/*--------------------------------------------------------------------------*
**
** NAME: dupe_relations
**
** FUNCTION:  This routine copies all of the relations for the source criteria
**            and links to the destination criteria.
**
** RETURNS:   This is a void function.
**
**
**--------------------------------------------------------------------------*/

void dupe_relations (criteria_type * source,
                     criteria_type * destination,
                     boolean       * error)
 {
    relation_type * relation;

    *destination = EMPTY_CRITERIA;

    destination->flags = source->flags;
    destination->hash_id = source->hash_id;
    for (relation = source -> version;
         relation != NIL (relation_type);
         relation = relation -> next_relation)
     {
       create_relation (destination, VERSION, EQUALS, relation -> int_value,
                        NIL (char), error);
       RETURN_ON_ERROR;
     }

    for (relation = source -> release;
         relation != NIL (relation_type);
         relation = relation -> next_relation)
     {
       create_relation (destination, RELEASE, EQUALS, relation -> int_value,
                        NIL (char), error);
       RETURN_ON_ERROR;
     }

    for (relation = source -> modification;
         relation != NIL (relation_type);
         relation = relation -> next_relation)
     {
       create_relation (destination, MODIFICATION, EQUALS,
                        relation -> int_value, NIL (char), error);
       RETURN_ON_ERROR;
     }

    for (relation = source -> fix;
         relation != NIL (relation_type);
         relation = relation -> next_relation)
     {
       create_relation (destination, FIX, EQUALS, relation -> int_value,
                        NIL (char), error);
       RETURN_ON_ERROR;
     }

    for (relation = source -> ptf;
         relation != NIL (relation_type);
         relation = relation -> next_relation)
     {
       create_relation (destination, PTF, EQUALS, -1, relation -> ptf_value,
                        error);
       RETURN_ON_ERROR;
     }
 } /* end dupe_relations */
