/* @(#)62  1.16.2.21  src/bos/usr/sbin/install/messages/ckprereqmsg.h, cmdinstl, bos41J, 9510A_all 2/27/95 12:14:58 */
/*
 *   COMPONENT_NAME: CMDINSTL
 *
 * FUNCTIONS: Default messages for ckprereq command.
 *
 * ORIGINS: 27
 *
 *   (C) COPYRIGHT International Business Machines Corp.  1991,1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifndef _H_CKPREREQMSG
#define _H_CKPREREQMSG

#define CKP_USAGE_D "\
Usage: ckprereq [-f PrereqFile | -l fileset] [-O {r|u|s}] [-v]\n\
\tChecks status information held in either the PrereqFile or the\n\
\tSoftware Vital Product Data about installed filesets.\n\
\t\t-f Specifies the name of the prerequisite file.\n\
\t\t\tThe default name is prereq.\n\
\t\t-l Specifies the name of the fileset for which to get \n\
\t\t\tinformation from the Software Vital Product Data.\n\
\t\t-O Specifies which Software Vital Product Data \n\
\t\t\tdatabase to search: root, usr, or share.\n\
\t\t-v Specifies verbose mode.\n"

#define CKP_BAD_INUTREE_D "\
%s: The environment variable INUTREE must be one of \n\
\tthe following: %c, %c or %c.\n"

#define CKP_BAD_OFLAG_D "\
%s: One of the following must be specified with the \n\
\t-O flag: %c, %c or %c.\n"

#define CKP_EXIT_D "\
%s: The ckprereq routine is exiting with an error.\n"

#define CKP_INCONSISTNT_REQS_D "\
%s: Error in the Software Vital Product Data. The \"usr\"\n\
\tpart of a fileset does not have the same requisites as the \"root\"\n\
\tpart. The fileset is: %s\n"

#define CKP_INCONSISTNT_SDEDBY_D "\
%s: Error in the Software Vital Product Data. The \"usr\"\n\
\tpart of an update is not superseded by the same fileset update as the\n\
\t\"root\" part. The update is: %s\n"

#define CKP_INSTP_REQ_CANCL_D "\
%s: The installation of the following filesets will be\n\
\tCANCELLED because filesets upon which they depend are not installed\n\
\t(or not completely installed):\n"

#define CKP_INSTP_REQ_CANCL_ROOT_D "\
%s: The installation of the \"root\" part of the following\n\
\tfilesets will be CANCELLED because filesets upon which they\n\
\tdepend are not installed (or not completely installed):\n"

#define CKP_INT_CODED_MSG_D "\
%s: Internal error. Code %d.\n\
\tUse local problem reporting procedures.\n"

#define CKP_INT_UNEXP_LPPMICRO_D "\
%s: Internal error. Unexpected OP_TYPE for MICRO_CODE fileset.\n\
\tFileset is %s\n\
\tUse local problem reporting procedures.\n"

#define CKP_INVALID_MC_REQ1_D "\
%s: Error.  A fileset maintenance level has an illegal\n\
\trequisite.  The only valid requisite type for a maintenance\n\
\tlevel is \"prereq.\"\n\
\tThe fileset is: %s\n"

#define CKP_INVALID_MC_REQ2_D "\
%s:  Error.  A 3.2-formatted fileset maintenance level cannot\n\
\thave a requisite on a fileset with a different name than itself.\n\
\tThe fileset in error is:  %s\n"

#define CKP_LEX_ILLBASE_D "\
%s: A lexical error occurred for %s \n\
\ton line %d, column %d.\n\
\tThere is an illegal base level: %s.\n\
\tUse local problem reporting procedures.\n"

#define CKP_LEX_ILLCHR_D "\
%s: A lexical error occurred for %s \n\
\ton line %d, column %d.\n\
\tAn illegal character, %c, was found (hexadecimal value: %x).\n\
\tAll characters must be ASCII.\n\
\tThe character will be treated as a SPACE.\n\
\tUse local problem reporting procedures.\n"

#define CKP_LEX_ILLNAME_D "\
%s: A lexical error occurred for %s \n\
\ton line %d, column %d.\n\
\tThere is an illegal name, PTF ID, requisite, \n\
\t\tor field name: %s.\n\
\tNames must be more than one character in length\n\
\t\tand begin with a letter or '_'.\n\
\tSubsequent characters in a name may be a letter, a digit, \n\
\t\ta '_', a '-', or a '.'.\n\
\tA PTF ID has the same syntax as a name.\n\
\tA requisite is one of: *prereq, *coreq, *instreq, or *ifreq.\n\
\tA field name is one of: 'V', 'R', 'M', 'F', 'P', or 'O'.\n\
\tUse local problem reporting procedures.\n"

#define CKP_LEX_ILLNUM_D "\
%s: A lexical error occurred for %s \n\
\ton line %d, column %d.\n\
\tThere is an illegal number: %s.\n\
\tUse local problem reporting procedures.\n"

#define CKP_LEX_LNGNAME_D "\
%s: A lexical error occurred for %s\n\
\ton line %d, column %d.\n\
\tThe name %s is too long.\n\
\tMaximum name length is %d. The name has been truncated.\n\
\tUse local problem reporting procedures.\n"

#define CKP_LEX_REQEXP_D "\
%s: A lexical error occurred for %s \n\
\ton line %d, column %d.\n\
\tOne of the following was expected instead of %s:\n\
\t\t*prereq, *coreq, *instreq, or *ifreq.\n\
\tUse local problem reporting procedures.\n"

#define CKP_LEX_UNXCHR_D "\
%s: A lexical error occurred for %s\n\
\ton line %d, column %d.\n\
\tAn unexpected character, %c, was found (hexadecimal value: %x).\n\
\tThe character will be treated as a SPACE.\n\
\tUse local problem reporting procedures.\n"

#define CKP_NOSWVPD_D "\
%s: Fileset %s\n\
\tis not in the Software Vital Product Data. \n\
\tUse local problem reporting procedures.\n"

#define CKP_PARS_CLOSEB_D "\
%s: A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tThere is a missing close brace '}'.\n\
\tUse local problem reporting procedures.\n"

#define CKP_PARS_CLOSEP_D "\
%s: A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tMissing close parenthesis ')' after base level in an *ifreq expression.\n\
\tUse local problem reporting procedures.\n"

#define CKP_PARS_DOT_NUMB_D "\
%s: A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tA base level was expected following an open parenthesis '(' in an\n\
\t*ifreq expression instead of %s.\n\
\tUse local problem reporting procedures.\n"

#define CKP_PARS_EXPNAM_D "\
%s: A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tA fileset name was expected instead of %s.\n\
\tUse local problem reporting procedures.\n"

#define CKP_PARS_EXPVRM_D "\
%s: A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tA 'V', 'R', 'M', 'F', or 'P' was expected\n\
\t\tinstead of '%s'.\n\
\tUse local problem reporting procedures.\n"

#define CKP_PARS_FIXRPT_D "\
%s: A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tThe fix field, 'F', has already been found.\n\
\tUse the OR expression, 'O', to compare multiple values\n\
\t\tof the fix field.\n\
\tUse local problem reporting procedures.\n"

#define CKP_PARS_ILLPTF_D "\
%s: A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tAn illegal PTF ID value, %s, was found.\n\
\tUse local problem reporting procedures.\n"

#define CKP_PARS_INVOR_D "\
%s: A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tAn OR expression, 'O', is invalid at this point.\n\
\tUse local problem reporting procedures.\n"

#define CKP_PARS_MODRPT_D "\
%s: A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tThe modification field, 'M', has already been found.\n\
\tUse the OR expression, 'O', to compare multiple values\n\
\t\tof the modification field.\n\
\tUse local problem reporting procedures.\n"

#define CKP_PARS_NONUMB_D "\n\
%s: A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tA version, release, modification, or fix number\n\
\twas expected where %s was found.\n\
\tUse local problem reporting procedures.\n"

#define CKP_PARS_NOPTFN_D "\
%s: A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tA number or PTF ID was expected where %s was found.\n\
\tUse local problem reporting procedures.\n"

#define CKP_PARS_OPENB_D "\
%s: A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tThere is a missing open brace '{'.\n\
\tUse local problem reporting procedures.\n"

#define CKP_PARS_OROPER_D "\
%s: A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tAll operands of an OR expression, 'O', must operate \n\
\t\ton the same field.\n\
\tUse local problem reporting procedures.\n"

#define CKP_PARS_PASSC_D "\
%s: A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tThe number of items in the group required to pass\n\
\t\twas expected instead of %s.\n\
\tUse local problem reporting procedures.\n"

#define CKP_PARS_PASSLESS_D "\
%s: A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tThe group has fewer items than was specified for the number \n\
\t\trequired to pass.\n\
\tUse local problem reporting procedures.\n"

#define CKP_PARS_PTFRPT_D "\
%s: A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tThe PTF ID field, 'P', has already been found.\n\
\tUse the OR expression, 'O', to compare multiple values\n\
\t\tof the PTF ID field.\n\
\tUse local problem reporting procedures.\n"

#define CKP_PARS_RLSRPT_D "\
%s: A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tThe release field, 'R', has already been found.\n\
\tUse the OR expression, 'O', to compare multiple values\n\
\t\tof the release field.\n\
\tUse local problem reporting procedures.\n"

#define CKP_PARS_UNXTYP_D "\
%s: A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tInstead of %s, one of the following was expected:\n\
\t\ta '>' (to start a group) or \n\
\t\t*prereq, *instreq, *coreq, *ifreq or\n\
\ta fileset name.\n\
\tUse local problem reporting procedures.\n"

#define CKP_PARS_VRMF_LEVEL_D "\
%s: A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tIt is illegal to specify version, release, modification or fix fields\n\
\twhen a base level is given in an *ifreq expression.\n\
\tUse local problem reporting procedures.\n"

#define CKP_PARS_VRSRPT_D "\
%s: A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tThe version field, 'V', has already been found.\n\
\tUse the OR expression, 'O', to compare multiple values\n\
\t\tof the version field.\n\
\tUse local problem reporting procedures.\n"

#define CKP_SELF_COREQ_D "\
%s: Error. %s is a corequisite of itself.\n"

#define CKP_SELF_PREREQ_D "\
%s: Error. %s is a prerequisite of itself.\n"

#define CKP_SUP_FIX_NOT_IN_VPD_D "\
%s: Cannot find superseding update in Software Vital \n\
\tProduct Data database for: %s. \n\
\tSuperseding update is: %s.\n"

#define CKP_SWVPD_2PARTS_D "\
%s: The Software Vital Product Data indicates\n\
\tthat %s has a \"share\" and \"root\" part\n\
\tor a \"share\" and \"usr\" part.\n\
\tUse local problem reporting procedures.\n"

#define CKP_ILLEGAL_OLD_IFREQ_D "\
%s:  Error:  A 4.1-formatted package must not contain an\n\
\t\"ifreq\" to a fileset update without also specifying the base\n\
\tlevel of the fileset.  The ifreq expression must contain a\n\
\tdotted, numeric level as in:\n\
\t\t*ifreq <fileset_name> (v.r.m.f) v.r.m.f\n\
\t\t\tor\n\
\t\t*ifreq <fileset_name> v.r.m.f\n\
\t\t\t(if the requisite fileset is in 4.1 format) or\n\
\t\t*ifreq <fileset_name> (v.r.m.f) p=<ptf_id>\n\
\t\t\t(if the requisite fileset is in 3.2 format)\n\
\twhere \"(v.r.m.f)\" is the base fileset level of the requisite\n\
\tand \"v.r.m.f\" or \"p=<ptf_id>\" is the level of the requisite\n\
\tupdate itself.  If a single, non-parenthetic level is provided,\n\
\tthe base level is implied from the level of the update itself.\n\
\tThe update in error is: %s\n\
\tThe prereq information is:\n%s\n\n"

#define CKP_NO_UPDATE_INSTREQS_D "\
%s:  Error:  A fileset update must not contain the requisite type\n\
\t\"instreq\".\n\
\tThe update in error is: %s\n\n"

#define CKP_41_UPDT_NO_PRQ_SAME_NAME_D "\n\
%s:  Error:  A 4.1-formatted fileset update has an illegal\n\
\trequisite on a fileset with the same name as itself.  The requisite\n\
\ttype MUST be \"*prereq\".\n\
\tThe fileset in error is:\n\
\t%s\n\n"

#define CKP_ILLEGAL_LEVEL_FOR_BASE_D "\n\
%s:  Error:  Expecting a base level prereq expression using a dotted,\n\
\tnumeric level (e.g., 1.1.0.0) for the fileset update:\n\
\t%s\n\
\tInstead, the following was found:\n\
\t%s\n\n"

#define CKP_NO_NEWER_32_THAN_41_D "\n\
%s:  Error:  A fileset contained in a 4.1-formatted package\n\
\tcannot have a lower level (ver.rel.mod.fix) than a fileset with the\n\
\tsame name in a 3.2 or 3.1-formatted package.\n\
\tThe filesets in error are:\n\
\t%s\n\
\tand\n\
\t%s\n\n"

#define CKP_ILLEGAL_SUPS_PARTS_D "\n\
%s:  Error:  A fileset has parts that are different from one or\n\
\tmore of the filesets it supersedes.  It is NOT valid for a fileset with\n\
\ta \"usr\" part only to supersede a fileset with both \"usr\" and \"root\"\n\
\tparts.  A fileset with a \"share\" part may only supersede another fileset\n\
\twith a \"share\" part.\n\
\tThe fileset in error is:\n\
\t%s\n\n"


#define CKP_ILLEGAL_BASE_IFREQ1_D "\n\
%s:  Error:  The base level implied or specified in an \"*ifreq\"\n\
\texpression is not a base level fileset.  The fileset in error is:\n\
\t%s\n\n"

#define CKP_ILLEGAL_BASE_IFREQ2_D "\n\
%s:  Error:  A fileset has an update level in an \"*ifreq\" expression\n\
\tthat is not greater than the base level specified.  The fileset in\n\
\terror is:\n\
\t%s\n\n"

#define CKP_TOO_MANY_REQS_ON_SAME_FILESET1_D "\n\
%s:  Error:  A 4.1-formatted fileset has more than one requisite\n\
\ton filesets with its own name.  The fileset in error is:\n\
\t%s\n\n"

#define CKP_TOO_MANY_REQS_ON_SAME_FILESET2_D "\n\
%s:  Error:  A 4.1-formatted fileset has more than one requisite of\n\
\tthe type \"*prereq\", \"*coreq\", or \"*instreq\" that are not contained\n\
\tin a group requisite expression.  The fileset in error is:\n\
\t%s\n\n"

#define CKP_NO_MIX_N_MATCH_D "\n\
%s:  Error:  A fileset update prereqs a base level fileset with a\n\
\tdifferent package format.  (A 4.1-formatted update cannot prereq a\n\
\t3.2-formatted base level and vice versa.)  The update in error is:\n\
\t%s\n\n"

#define CKP_NO_PRQ_BASE_ERR_D "\n\
%s:  Error:  An update does not prereq its base level fileset with\n\
\tthe same version and release, the same or lower modification number, or\n\
\tlower fix number.  The update in error is:\n\
\t%s\n\n"

#define CKP_WHERE_D "\
where:\n"

#define CKP_AUTO_SUPS_WARNING1_D "\
  NOTE:  One or more fileset updates in this list are flagged with \"*\".\n\
         These updates supersede (i.e., replace) updates which you\n\
         selected.  Newer level fileset updates will always be\n\
         automatically chosen instead of the fileset updates they\n\
         supersede when the \"auto-install\" option (i.e., \"AUTOMATICALLY\n\
         install requisite software\" or -g flag) is specified.\n"

#define CKP_MISC_FAILURES_TITLE_D "\
  Miscellaneous Failures\n\
  ----------------------\n"

#define CKP_MISC_APPLY_FAILURES_HDR_D "\
  The following is a list of filesets that you asked to install.  They failed\n\
  pre-installation checks for one of several reasons.  See the \"Failure Key\"\n\
  for failure reasons and possible recovery hints.\n\n"

#define CKP_MISC_COMMIT_FAILURES_HDR_D "\
  The following is a list of filesets that you asked to commit.  They failed\n\
  pre-commit checks for one of several reasons.  See the \"Failure Key\"\n\
  for failure reasons and possible recovery hints.\n\n"

#define CKP_MISC_REJECT_FAILURES_HDR_D "\
  The following is a list of filesets that you asked to reject.  They failed\n\
  pre-reject checks for one of several reasons.  See the \"Failure Key\"\n\
  for failure reasons and possible recovery hints.\n\n"

#define CKP_MISC_DEINST_FAILURES_HDR_D "\
  The following is a list of filesets that you asked to remove.  They failed\n\
  pre-deinstall checks for one of several reasons.  See the \"Failure Key\"\n\
  for failure reasons and possible recovery hints.\n\n"

#define CKP_ALRDY_INSTLD_HDR_PREVIEW_D "\
  Already Installed\n\
  -----------------\n\
  The following filesets which you selected are either already installed\n\
  or effectively installed through superseding filesets.\n\n"

#define CKP_ALRDY_INSTLD_HDR_D "\
  Already Installed\n\
  -----------------\n\
  The number of selected filesets that are either already installed\n\
  or effectively installed through superseding filesets is %d.  See\n\
  the summaries at the end of this installation for details.\n\n"

#define CKP_ALRDY_COMTD_HDR_D "\
  Already Committed\n\
  -----------------\n\
  The number of selected filesets that are already committed is %d.\n\n"

#define CKP_MAIN_FAILURES_TITLE_D "\
FAILURES\n\
--------\n"

#define CKP_APPLY_FAILURES_HDR_D "\
  Filesets listed in this section failed pre-installation verification\n\
  and will not be installed.\n\n"

#define CKP_COMT_FAILURES_HDR_D "\
  Filesets listed in this section failed pre-commit verification\n\
  and will not be committed.\n\n"

#define CKP_DEINST_FAILURES_HDR_D "\
  Filesets listed in this section failed pre-deinstall verification\n\
  and will not be removed.\n\n"

#define CKP_REJ_FAILURES_HDR_D "\
  Filesets listed in this section failed pre-reject verification\n\
  and will not be rejected.\n\n"

#define CKP_NO_SUPS_IN_USR_D "\
  No Supersedes Information in USR Part\n\
  -------------------------------------\n\
  The following fileset updates have supersedes information in the \"root\"\n\
  part but not the \"usr\" part.  The information from the \"root\" part will\n\
  be used if necessary.\n\n"

#define CKP_NO_PRQ_IN_USR_D "\
  No Prereq Information in USR Part\n\
  ---------------------------------\n\
  The following have prereq information in the \"root\" part but not the\n\
  \"usr\" part.  The information from the \"root\" part will be used if\n\
  necessary.\n\n"

#define CKP_NO_PRQ_BASE_WARN_D "\
  No Prereq on Base Level Fileset\n\
  -------------------------------\n\
  The following fileset updates do not prereq their base level fileset at all\n\
  or do not prereq a base level fileset with the same version and release,\n\
  the same or lower modification number, or lower fix number.  This could\n\
  cause dependency checks on these updates to fail.\n\n"

#define CKP_MISS_USR_APPLD_ROOT_D "\
  No USR Part for Applied ROOT Part\n\
  ---------------------------------\n\
  The following are applied on the \"root\" part but not on the \"usr\"\n\
  part.  To correct their state, try to reject the root part.  If desired,\n\
  you may attempt to remove the fileset with the \"Remove Software\n\
  Products\" or deinstall facility (-u flag).  Alternatively, you may attempt\n\
  to re-install the fileset using \"FORCE\" (-F flag).\n\n"

#define CKP_MISS_USR_COMTD_ROOT_D "\
  No USR Part for Committed ROOT Part\n\
  -----------------------------------\n\
  The following are committed on the \"root\" part but not installed on the\n\
  \"usr\" part.  To correct their state, you may attempt to remove the\n\
  fileset with the \"Remove Software Products\" or deinstall\n\
  facility (-u flag).  Alternatively, you may attempt to re-install the\n\
  fileset using \"FORCE\" (-F flag).\n\n"

#define CKP_AVLBL_IFREQ_HDR_D "\
  Available Requisites of Previously Installed Software\n\
  -----------------------------------------------------\n\
  The following filesets are requisites of software that is already installed.\n\
  These filesets are not currently installed but should be to ensure that the\n\
  system functions correctly.  You did not select these filesets for\n\
  installation, but they are available on the installation media.\n"

#define CKP_AVLBL_IFREQ_FAIL_D "\
  An attempt would have been made to install these filesets, but they appear\n\
  to have missing or inconsistent requisites of their own.  For further\n\
  details, select these filesets explicitly for installation.\n\n"

#define CKP_AVLBL_IFREQ_USE_GFLAG_D "\
  They may be explicitly selected, or they will be automatically installed\n\
  whenever you specify options to the installation commands to automatically\n\
  include requisite software (-g flag).\n\n"

#define CKP_MSNG_IFREQ_TITLE_D "\
  Missing Requisites of Previously Installed Software\n\
  ---------------------------------------------------\n"

#define CKP_MSNG_IFREQ_CLIENT_D "\
  The following filesets are requisites of software that is already installed.\n\
  They should be installed to ensure that the system functions correctly.\n\
  These filesets are not installed on the \"usr\" (server) part and,\n\
  therefore, they cannot be installed on the \"root\" (client) part.\n\n"

#define CKP_MSNG_IFREQ_D "\
  The following fileset updates are requisites of software that is already\n\
  installed.  They are not currently installed but should be to ensure that\n\
  the system functions correctly.  These updates are not on the current\n\
  installation media.\n\n"

#define CKP_RESULTS_D "\
Results...\n\n"

#define CKP_E_O_WARNINGS_D "\
  << End of Warning Section >>\n\n"

#define CKP_E_O_FAILURES_D "\
  << End of Failure Section >>\n\n"

#define CKP_E_O_SUCCESS_D "\
  << End of Success Section >>\n\n"

#define CKP_OTHER_BROKENS_D "\
  Updates to BROKEN Filesets\n\
  --------------------------\n\
  The following selected fileset updates are updates to filesets which have\n\
  one or more updates in the BROKEN state.  Before installing any other\n\
  updates to these filesets, you must correct the state of the BROKEN\n\
  updates.\n\n" 

#define CKP_BROKEN_PKGS_D "\
  BROKEN Filesets\n\
  ---------------\n\
  The following selected fileset updates are currently in the BROKEN state.\n\n"

#define CKP_MISSING_PKGS_TITLE_D "\
  Missing Filesets\n\
  ----------------\n"

#define CKP_MISSING_PKGS_HDR_D "\
  The following filesets could not be found on the installation media.\n\
  If you feel these filesets really are on the media, check for typographical\n\
  errors in the name specified or, if installing from directory, check for\n\
  discrepancies between the Table of Contents file (.toc) and the images that\n\
  reside in the directory.\n\n"

#define CKP_CONFL_VERSIONS_D "\
  Conflicting Versions of Filesets\n\
  --------------------------------\n\
  The following filesets are conflicting versions of filesets for which there\n\
  are multiple versions on the installation media.  Since a specific version\n\
  was not selected, the newest installable version has been selected.\n\n"

#define CKP_MSNG_SUPERSEDED_TITLE_D "\
  Missing Superseded Fileset Updates\n\
  ----------------------------------\n"

#define CKP_MSNG_SUPERSEDED_HDR_D "\
  The following selected fileset updates could not be found on the installation\n\
  media.  Newer updates which supersede those selected were detected on\n\
  the media and are indicated in parentheses.  You may explicitly select the\n\
  newer fileset updates for installation or use the \"auto-install\" facility\n\
  (i.e., \"AUTOMATICALLY install requisite software\" or -g flag), and the\n\
  newer updates will be installed instead of those selected.\n\n"

#define CKP_MSNG_SUPERSEDED_ROOT_HDR_D "\
  Missing Superseded Fileset Updates\n\
  ----------------------------------\n\
  The \"usr\" parts (i.e., \"server\" parts) of the following fileset updates\n\
  are not currently installed, and, therefore, the \"root\" parts (i.e.,\n\
  \"client\" parts) cannot be installed.  The \"usr\" parts of newer\n\
  updates which supersede those selected are installed.  You may\n\
  explicitly select installation of the \"root\" part of the newer\n\
  fileset updates, or you may use the \"auto-install\" facility (-g flag),\n\
  and the newer updates will be installed instead of those selected.\n\n"

#define CKP_MISSING_PKGS_CLIENT_D "\
  No ROOT Part to Install\n\
  -----------------------\n\
  The following selected filesets do not have a \"root\" part that can be\n\
  installed.  Either the fileset's corresponding \"usr\" part has yet to be\n\
  installed or the fileset does not have a \"root\" part.\n\n"

#define CKP_ROOT_PART_NOT_RQSTD_D "\
  Root Part Not Specified\n\
  -----------------------\n\
  The following contain both \"usr\" and \"root\" parts.  You only asked\n\
  to install the \"usr\" part.  You cannot install the \"usr\" part of a \n\
  fileset without its \"root\" part.\n\n"

#define CKP_NUTTIN_TO_DEINSTL_D "\
  Not Installed\n\
  -------------\n\
  No software could be found on the system that could be deinstalled for the\n\
  following requests:\n\n"

#define CKP_NUTTIN_TO_COMMIT_D "\
  Not Committable\n\
  ---------------\n\
  No software was found in the APPLIED state that could be committed for\n\
  the following requests:\n\n"
          
#define CKP_COMTD_NOT_REJCTBL_D "\
  Committed: Not Rejectable\n\
  -------------------------\n\
  The following have been committed and cannot be rejected:\n\n"

#define CKP_NUTTIN_TO_REJECT_D "\
  Not Rejectable\n\
  --------------\n\
  No software could be found installed on the system that could be rejected\n\
  for the following requests:\n\n"

#define CKP_31_UPDT_REQ_FAILURE_D "\
  Different Base Level Installed\n\
  ------------------------------\n\
  The following selected filesets are updates to a base level which is at\n\
  a level different from that currently installed (or selected to be\n\
  installed during this installation).  Use software listing utilities\n\
  (lslpp command) for details of the installed base level.\n\n"

#define CKP_NO_DEINST_TITLE_D "\
  Non-Deinstallable Filesets\n\
  --------------------------\n"

#define CKP_NO_DEINST_BOSRTE_D "\
  You cannot deinstall bos.rte.  This is the base operating system.\n"
                           
#define CKP_DEINST_3_1_D "\
  3.1 Formatted Filesets: \n\
  ----------------------- \n\
  The following is a list of 3.1 formatted filesets.  Deinstallation of \n\
  3.1 formatted filesets may not be complete.\n\n"

#define CKP_DEINST_3_2_D "\
  Filesets installed on 3.2: \n\
  -------------------------- \n\
  The following filesets were originally installed on an AIX 3.2 system \n\
  and, therefore, may not be completely deinstalled.\n\n"

#define CKP_DEINST_MIG_D "\
  Partially Migrated Filesets: \n\
  ---------------------------- \n\
  The following filesets were originally installed on an AIX 3.2 system \n\
  and have only been partially migrated from AIX 3.2 to AIX 4.1. \n\
  Deinstallation of these filesets is allowed, however, no unconfiguration\n\
  will be performed. \n\n"

#define CKP_OEM_BASELEVEL_ERR_D "\
  \"%c\" System-specific fileset exists.  The currently installed version\n\
      of the fileset has been modified by the system manufacturer.  The\n\
      generic version of the fileset cannot be installed unless the \"FORCE\"\n\
      option (-F) is specified. \n\"

#define CKP_OEM_MISMATCH_ERR_D "\
  \"%c\" Operating system mismatch.  Fileset or one of its requisites is\n\
      designated to be applied only on a system supplied by a different\n\
      system manufacturer.\n"

#define CKP_OEM_SPECIFIC_ERR_D "\
  \"%c\" Generic fileset update cannot be applied onto a fileset which has\n\
      been modified or would be modified during this installation to operate\n\
      only on a system supplied by this specific system manufacturer.\n"

#define CKP_OEM_BASELEVEL_HDR_D "\
  Filesets Already Modified by System Manufacturer\n\
  ------------------------------------------------\n\
  The following generic filesets can not be applied because the\n\
  installed filesets have been modified by the system manufacturer\n\
  to be specific to this system.  The \"FORCE\" option (-F) must be\n\
  used in order to overwrite the modified version of the fileset.\n\n"

#define CKP_OEM_MISMATCH_HDR_D "\
  Filesets Specific to Different System Manufacturer\n\
  --------------------------------------------------\n\
  The following filesets can not be applied because this system\n\
  was manufactured by a different system manufacturer than that required by\n\
  the filesets or the requisites of the filesets.\n\n"

#define CKP_OEM_REPLACED_HDR_D "\
  Filesets Already Modified by System Manufacturer\n\
  ------------------------------------------------\n\
  The following generic fileset updates can not be applied because the\n\
  installed filesets have been modified or have requisites which have been\n\
  modified by the system manufacturer to be specific to this system.\n\n"

#define CKP_SUPERSEDED_BY1_D "\
      (superseded by: %s)\n"

#define CKP_SUPERSEDED_BY2_D "\
  %-49s (Superseded by %s)\n"

#define CKP_NUTTIN_TO_DEINSTL_TRLR_D "\n\
  (The fileset may not be currently installed, or you may have made a\n\
   typographical error.)\n"

#define CKP_NUTTIN_TO_COMMIT_TRLR_D "\n\
  (Possible reasons for failure:  1. the selected software is already\n\
   committed, 2. the selected software is not installed, or 3. a typographical\n\
   error was made.)\n"

#define CKP_COMTD_NOT_REJCTBL_TRLR_D "\n\
  (The only way to remove a committed fileset or fileset update is\n\
   to deinstall the fileset.  Use the \"Remove Software Products\"\n\
   facility (-u flag) if this is desired.)\n"

#define CKP_NUTTIN_TO_REJECT_TRLR_D "\n\
  (Possible reasons for failure:  1. the selected software has been\n\
   committed, i.e., cannot be rejected, 2. the selected software is not\n\
   installed, or 3. a typographical error was made.)\n"

#define CKP_REQ_FAIL_KEY_HDR_D "\
  Requisite Failure Key:\n"

#define CKP_DEP_FAIL_KEY_HDR_D "\
  Dependent Failure Key:\n"
   
#define CKP_FAIL_KEY_HDR_D "\
  Failure Key:\n"

#define CKP_REQ_FAIL_APPLY_TITLE_D "\
  Requisite Failures\n\
  ------------------\n"

#define CKP_REQ_FAIL_APPLY_HDR_D "\
  SELECTED FILESETS:  The following is a list of filesets that you asked to\n\
  install.  They cannot be installed until all of their requisite filesets\n\
  are also installed.  See subsequent lists for details of requisites.\n"

#define CKP_REQ_FAIL_COMT_HDR_D "\
  Requisite Failures\n\
  (Commit Operation)\n\
  ------------------\n\
  SELECTED FILESETS:  The following is a list of filesets that you asked to\n\
  commit.  They cannot be committed until all of their requisite filesets\n\
  are also committed.  See subsequent lists for details of requisites.\n\n"

#define CKP_DEP_FAIL_DEINST_HDR_D "\
  Dependency Failures\n\
  (Deinstall Operation)\n\
  ---------------------\n\
  SELECTED FILESETS:  The following is a list of filesets that you asked to\n\
  remove.  They cannot be removed until all of their dependent filesets\n\
  are also removed.  See subsequent lists for details of dependents.\n\n"

#define CKP_DEP_FAIL_REJECT_HDR_D "\
  Dependency Failures\n\
  (Reject Operation)\n\
  -------------------\n\
  SELECTED FILESETS:  The following is a list of filesets that you asked to\n\
  reject.  They cannot be rejected until all of their dependent filesets\n\
  are also rejected.  See subsequent lists for details of dependents.\n\n"
       
#define CKP_SUCC_RQSTD_PKGS_HDR_D "\
  Selected Filesets\n\
  -----------------\n"

#define CKP_AUTO_SUPS_WARNING2_D "\
  (\"*\" indicates a newer update chosen automatically over a selected\n\
   fileset update)\n"

#define CKP_SUCC_REQUISITES_TITLE_D "\
  Requisites\n\
  ----------\n"
           
#define CKP_SUCC_REQS_APPLY_HDR_D "\
  (being installed automatically;  required by filesets listed above)\n"

#define CKP_SUCC_REQS_COMT_HDR_D "\
  (being committed automatically as requisites of selected filesets)\n"

#define CKP_SUCC_DEPS_TITLE_D "\
  Dependents\n\
  ----------\n"
            
#define CKP_SUCC_DEPS_DEINST_HDR_D "\
  (being deinstalled so that selected filesets may be removed; will not\n\
   function correctly when selected filesets are removed.)\n"
            
#define CKP_SUCC_DEPS_REJECT_HDR_D "\
  (being rejected so that selected filesets may be rejected;  will not\n\
   function correctly when selected filesets are removed.)\n"
   
#define CKP_SUCC_AUTO_IFREQS_HDR_D "\
  Requisites from Prior Installations\n\
  -----------------------------------\n\
  (being installed automatically to ensure system functions correctly)\n"
   
#define CKP_SUCC_AUTO_MLEV_TITLE_D "\
  Maintenance Level Fileset Updates\n\
  ---------------------------------\n"
         
#define CKP_SUCC_AUTO_MLEV_APPLY_HDR_D "\
  (being installed automatically to reflect level of system)\n"
         
#define CKP_SUCC_AUTO_MLEV_COMT_HDR_D "\
  (being committed automatically to preserve level of system)\n"

#define CKP_SUCC_AUTO_MAND_HDR_D "\
  Mandatory Fileset Updates\n\
  -------------------------\n\
  (being installed automatically due to their importance)\n"

#define CKP_SUCC_REQ_TO_BE_INST_D "\
      %s %s (TO BE INSTALLED)\n" 

#define CKP_SUCC_REQ_TO_BE_COMTD_D "\
      %s %s (TO BE COMMITTED)\n"

#define CKP_SUCC_REQ_INSTLD_D "\
      %s %s (INSTALLED)\n" 

#define CKP_SUCC_REQ_COMTD_D "\
      %s %s (COMMITTED)\n"

#define CKP_SUCCESS_KEY_HDR_D "\
  Success Key:\n"

#define CKP_SUCC_RQSTD_CODE_D "\
Selected"

#define CKP_SUCC_MAINT_CODE_D "\
Maintenance"

#define CKP_SUCC_MAND_CODE_D "\
Mandatory"

#define CKP_SUCC_P_REQ_CODE_D "\
P_Requisite"

#define CKP_SUCC_SUPS_CODE_D "\
Supersedes"

#define CKP_SUCC_REQ_CODE_D "\
Requisite"

#define CKP_SUCC_UNK_CODE_D "\
Unknown"

#define CKP_SUCC_DEP_CODE_D "\
Dependent"

#define CKP_APPLY_SUCCESS_KEY_D "\
  Selected    -- Explicitly selected by user for installation.\n\
  Maintenance -- Maintenance Level fileset update; being installed\n\
                 automatically to enable the level of the system to be\n\
                 tracked.\n\
  Mandatory   -- Considered to be important to the system; will always\n\
                 be installed when detected on the installation media.\n\
  Requisite   -- Requisite of other filesets being installed.\n\
  P_Requisite -- Previously installed fileset's requisite; being installed\n\
                 automatically now to ensure system's consistency.  (Only\n\
                 installed automatically when \"auto-install\" (-g flag)\n\
                 is specified.)\n\
  Supersedes  -- Superseding fileset update; not selected, chosen instead\n\
                 of an older, selected update.  (Only chosen in this fashion\n\
                 when \"auto-install\" is specified (-g flag)).\n\n"

#define CKP_COMT_SUCCESS_KEY_D "\
  Selected    -- Explicitly selected by user to be committed.\n\
  Maintenance -- Maintenance Level fileset update; being committed\n\
                 automatically to preserve the ability to track the level\n\
                 of the system (prevents rejecting).\n\
  Requisite   -- Requisite of filesets being committed; requisites are always\n\
                 committed when \"auto-commit\" (-g flag) is specified.\n\n"

#define CKP_DEINST_SUCCESS_KEY_D "\
  Selected  -- Explicitly selected by user to be deinstalled.\n\
  Dependent -- Dependent of other filesets being deinstalled; dependents\n\
               are always deinstalled when \"auto-deinstall\" (-g flag) is\n\
               specified.\n\n"

#define CKP_REJECT_SUCCESS_KEY_D "\
  Selected  -- Explicitly selected by user to be rejected.\n\
  Dependent -- Dependent of other filesets being rejected; dependents are\n\
               always rejected when \"auto-reject\" (-g flag) is specified.\n\n"

#define CKP_SUCCESS_TITLE_D "\
SUCCESSES\n\
---------\n"

#define CKP_SUCCESS_APPLY_HDR_D "\
  Filesets listed in this section passed pre-installation verification\n\
  and will be installed.\n"

#define CKP_SUCCESS_APPLY_HDR_GTR_V0_D "\
    -- Filesets are listed in the order in which they will be installed.\n\
    -- The reason for installing each fileset is indicated with a keyword\n\
       in parentheses and explained by a \"Success Key\" following this list.\n"

#define CKP_SUCCESS_APPLY_HDR_GTR_V1_D "\
    -- If a fileset has requisites they are listed (indented)\n\
       beneath the fileset.\n"

#define CKP_SUCCESS_COMT_HDR_D "\
  Filesets listed in this section passed pre-commit verification\n\
  and will be committed.\n"

#define CKP_SUCCESS_COMT_HDR_GTR_V0_D "\
    -- Filesets are listed in the order in which they will be committed.\n\
    -- The reason for committing each fileset is indicated with a keyword\n\
       in parentheses and explained by a \"Success Key\" following this list.\n"

#define CKP_SUCCESS_COMT_HDR_GTR_V1_D "\
    -- If a fileset has requisites they are listed (indented)\n\
       beneath the fileset.\n"

#define CKP_SUCCESS_DEINST_HDR_D "\
  Filesets listed in this section passed pre-deinstall verification\n\
  and will be removed.\n"

#define CKP_SUCCESS_DEINST_HDR_GTR_V0_D "\
    -- Filesets are listed in the order in which they will be removed.\n\
    -- The reason for deinstalling each fileset is indicated with a keyword\n\
       in parentheses and explained by a \"Success Key\" following this list.\n"

#define CKP_SUCCESS_REJECT_HDR_D "\
  Filesets listed in this section passed pre-reject verification\n\
  and will be rejected.\n"

#define CKP_SUCCESS_REJECT_HDR_GTR_V0_D "\
    -- Filesets are listed in the order in which they will be rejected.\n\
    -- The reason for rejecting each fileset is indicated with a keyword\n\
       in parentheses and explained by a \"Success Key\" following this list.\n"

#define CKP_SUCCESS_REMOVE_HDR_GTR_V1_D "\
    -- If a fileset has dependents they are listed (indented)\n\
       beneath the fileset.\n"
  
#define CKP_REQUIRED_PKGS_D "\
  REQUIRED FILESETS:  The fileset under test cannot be installed without\n\
  the following filesets.  (These filesets are not currently installed.)\n\n"

#define CKP_MISS_REQ_APPLY_D "\
  MISSING REQUISITES:  The following filesets are required by one or more\n\
  of the selected filesets listed above.  They are not currently installed\n\
  and could not be found on the installation media.\n"

#define CKP_MISS_REQ_COMT_D "\
  MISSING REQUISITES:  The following filesets are requisites of one or more\n\
  of the selected filesets listed above.  They are not currently installed\n\
  on the system.  You should install these requisites to ensure that the\n\
  selected filesets function correctly.  You MUST install these requisites\n\
  before committing the selected filesets.\n"

#define CKP_COMTD_DEPENDENTS_D "\
  COMMITTED DEPENDENTS:  The following filesets depend upon one or more of\n\
  the selected filesets listed above.  These dependents must be rejected\n\
  before the selected filesets.  Since these dependents are in a COMMITTED\n\
  state, they cannot be rejected.  Therefore, the selected filesets cannot\n\
  be rejected.  (Committed software can be removed with the deinstall\n\
  facility, i.e., \"Remove Software Products\" or -u flag.  NOTE: This\n\
  will remove an entire fileset.)\n"

#define CKP_REF_TO_DEP_LIST1_D "\
  (Selected filesets which depend upon these requisites are referenced in\n\
   parentheses.)\n\n"

#define CKP_REF_TO_DEP_LIST2_D "\
  (Selected filesets upon which these depend are referenced in parentheses.)\n\n"

#define CKP_GROUP_FAIL_HDR1_D "\
  GROUP REQUISITE FAILURES:  The fileset under test cannot be installed\n\
  until the following group requisite is satisfied.  A group requisite must\n\
  pass a specified number of requisite tests.  See the \"Requisite Failure Key\"\n\
  below for details of group member failures.\n\n"

#define CKP_GROUP_FAIL_HDR2_D "\
  GROUP REQUISITES:  The dependencies of one or more of the selected filesets\n\
  listed above are defined by a group requisite.  A group requisite must pass\n\
  a specified number of requisite tests.  The following describe group\n\
  requisite failures for filesets that you selected.  (See the \"Requisite\n\
  Failure Key\" below for details of group member failures.)\n\n"

#define CKP_AVLBL_REQS_HDR1_D "\
  AVAILABLE REQUISITES:  You specified that requisite software should be\n\
  automatically installed.  The following filesets are requisites of one\n\
  or more of the selected filesets listed above.  These requisites would\n\
  have been installed automatically had the selected filesets passed all\n\
  requisite checks.\n"

#define CKP_AVLBL_REQS_HDR2_D "\
  AVAILABLE REQUISITES:  The following filesets are requisites of one or\n\
  more of the selected filesets listed above.  They are available on\n\
  the installation media.  To install these requisites with the selected\n\
  filesets, specify the option to automatically install requisite\n\
  software (-g flag).\n"

#define CKP_APPLD_REQS_HDR1_D "\
  APPLIED REQUISITES:  You specified that requisite software should be\n\
  automatically committed.  The following filesets are APPLIED requisites\n\
  of one or more of the selected filesets listed above.  These filesets\n\
  would have been committed automatically had the selected filesets passed\n\
  all requisite checks.\n"

#define CKP_APPLD_REQS_HDR2_D "\
  APPLIED REQUISITES:  The following filesets are requisites of one or more\n\
  of the selected filesets listed above.  These must be committed before\n\
  or with the filesets that you selected.  To commit these requisites with\n\
  the selected filesets, specify the option to automatically commit requisite\n\
  software (-g flag).\n"

#define CKP_APPLD_DEPS_HDR1_D "\
  APPLIED DEPENDENTS:  You specified that dependent fileset updates should\n\
  be automatically rejected.  The following updates are dependents of one or\n\
  more of the selected filesets listed above.  These filesets would have\n\
  been rejected automatically had the selected filesets passed all dependency\n\
  checks.\n"

#define CKP_APPLD_DEPS_HDR2_D "\
  APPLIED DEPENDENTS:  The following fileset updates are dependents of one\n\
  or more of the selected filesets listed above.  These dependents must be\n\
  rejected before or with the updates that you selected.  To reject these\n\
  dependents with the selected filesets, specify the option to automatically\n\
  reject dependent updates (-g flag).\n"

#define CKP_INSTLD_DEPS_HDR1_D "\
  INSTALLED DEPENDENTS:  You specified that dependent software should be\n\
  automatically removed.  The following filesets are dependents of one or\n\
  more of the selected filesets listed above.  These dependents would have\n\
  been removed automatically had the selected filesets passed all dependency\n\
  checks.\n"

#define CKP_INSTLD_DEPS_HDR2_D "\
  INSTALLED DEPENDENTS:  The following filesets are dependents of one or more\n\
  of the selected filesets listed above.  These must be removed before\n\
  or with the filesets that you selected.  To remove these dependents with\n\
  the selected filesets, specify the option to automatically remove dependent\n\
  software (-g flag).\n"

#define CKP_DEINST_DEPS_VIA_UPDTS_D "\
  WARNING:  The fileset updates listed below have created dependencies from\n\
  their filesets to one or more of the filesets that you are trying to\n\
  remove.  If you choose to automatically remove dependent software, the\n\
  entire dependent fileset will be removed (not just the update).  You\n\
  may be able to reject the update to remove the dependency.\n"

#define CKP_AVLBL_REQ_MSG_D "\
  AVAILABLE REQUISITES:  You specified that requisite software should be\n\
  automatically installed.  Additional filesets (%d) would have been\n\
  installed automatically had the selected filesets passed all requisite\n\
  checks.\n\n"

#define CKP_APPLD_REQ_MSG_D "\
  APPLIED REQUISITES:  You specified that requisite software should be\n\
  automatically committed.  Additional filesets (%d) would have been\n\
  committed automatically had the selected filesets passed all requisite\n\
  checks.\n\n"

#define CKP_INSTLD_DEP_MSG_D "\
  INSTALLED DEPENDENTS:  You specified that dependent software should be\n\
  automatically removed.  Additional filesets (%d) would have been\n\
  removed automatically had the selected filesets passed all dependency\n\
  checks.\n\n"

#define CKP_APPLD_DEP_MSG_D "\
  APPLIED DEPENDENTS:  You specified that dependent software should be\n\
  automatically rejected.  Additional filesets (%d) would have been\n\
  rejected automatically had the selected filesets passed all dependency\n\
  checks.\n\n"

#define CKP_CMD_LINE_MISC_FAILING_REQ_HDR_D "\
  MISCELLANEOUS FAILING REQUISITES:  The fileset under test cannot be\n\
  installed without the following filesets.  These filesets have various\n\
  problems associated with them.  See the \"Requisite Failure Key\" for\n\
  failure reasons and possible recovery hints.\n"

#define CKP_MISC_FAILING_REQ_APPLY_HDR_D "\
  MISCELLANEOUS FAILING REQUISITES:  The following filesets are requisites\n\
  of one or more of the selected filesets listed above.  Various problems\n\
  associated with these requisites are preventing the selected filesets\n\
  from installing.  See the \"Requisite Failure Key\" for failure reasons and\n\
  possible recovery hints.\n"

#define CKP_MISC_FAILING_REQ_COMT_HDR_D "\
  MISCELLANEOUS FAILING REQUISITES:  The following filesets are requisites\n\
  of one or more of the selected filesets listed above.  Various problems\n\
  associated with these requisites are preventing the selected filesets from\n\
  being committed.  See the \"Requisite Failure Key\" for failure reasons and\n\
  possible recovery hints.\n"

#define CKP_MISC_FAILING_REQ_REMOVE_HDR_D "\
  MISCELLANEOUS FAILING DEPENDENTS:  The following filesets depend upon one or\n\
  more of the selected filesets listed above.  As a result, these dependents\n\
  must be removed before or with the selected filesets.  Various problems\n\
  associated with these dependents are preventing the selected filesets from\n\
  being removed.  See the \"Dependent Failure Key\" for failure reasons and\n\
  possible recovery hints.\n"

#define CKP_WARNING_TITLE_D "\
WARNINGS\n\
--------\n"

#define CKP_WARNING_HDR_D "\
  Problems described in this section are not likely to be the source of any\n\
  immediate or serious failures, but further actions may be necessary or\n\
  desired.\n\n"

#define CKP_CYCLE_HDR_D "\
\tFilesets involved in the cycle:\n\n"

#define CKP_VERIFYING_REQ_D "\
Verifying requisites..."

#define CKP_REQ_CODES_D "\
  Requisite codes\n\
  ---------------\n\
    Requisite type:\n\
      p -- prereq\n\
      c -- coreq\n\
      I -- instreq (only shows up on apply operations)\n\
      i -- ifreq\n\
     oi -- \"old-style\" ifreq (no level in expression)\n\
    Installation State:\n\
      a -- applied (fully installed) \n\
     pa -- partially applied (usr or root part not installed)\n\
      c -- committed\n\
     pc -- partially committed (usr or root part not committed)\n\
      b -- broken\n"

#define CKP_REQ_CODES_APPLY_ONLY_D "\
      o -- on the installation media, not installed\n\
      m -- \"missing:\" not installed, not on installation media\n"

#define CKP_NEEDS_D "\
   #%d needs:\n"

#define CKP_SUPERSEDES_D "\
        (supersedes: %s"

#define CKP_DEPENDENTS_COLON_D "\
     Dependents:\n"

#define CKP_REQUISITES_COLON_D "\
     Requisites:\n"

#define CKP_DEP_ABBREV_D "\
         (dep #s: "

#define CKP_SUCC_UPDATES_D "\
  %s %d.%d.%d.%d updates:\n"

#define CKP_SUPERSEDED_UPDATES_HDR_D "\
  Superseded Fileset Updates\n\
  --------------------------\n\
  Fileset updates listed in this section will not be installed.  Newer\n\
  updates which supersede (replace) these were selected instead (either by\n\
  you or automatically by the installation program).  Make sure that the\n\
  superseding updates listed passed pre-installation verification.\n\n"

#define CKP_PROD_AT_LEV_REQRS_D "\n\
Fileset %s %s requires:\n"

#define CKP_AT_LEAST_FOLLOWNG_D "\
%s   At least %d of the following:\n"

#define CKP_INSTP_FAILED_REQ_D "\
\"*\" indicates a requisite fileset that probably failed to install\n\
   during this installation session.\n\n"

#define CKP_NO_REMOVE_USR_B4_ROOT_D "\
  \"%c\" fileset has both \"usr\" and \"root\" parts installed.  Both parts\n\
      were not selected for this invocation of installp.  The \"usr\" part\n\
      cannot be removed separately when a fileset has both parts installed.\n"

#define CKP_NO_REAPPLY_INSTLD_ROOT_D "\
  \"%c\" fileset is applied on the \"root\" part but not on the \"usr\" part.\n\
      Before attempting to re-apply this fileset you must remove its \"root\"\n\
      part.  (Use the reject facility if the fileset is an update.  Remove\n\
      the fileset via the deinstall facility if it is a base level fileset.)\n"

#define CKP_NO_COMT_ROOT_B4_USR_D "\
  \"%c\" fileset contains both \"usr\" and \"root\" parts.  The \"usr\" part\n\
      is still in the applied state.  A fileset's \"root\" part cannot be\n\
      committed before its \"usr\" part.\n"

#define CKP_COMMITTED_ROOT_ONLY_D "\
  \"%c\" fileset is committed on the \"root\" part but not installed on\n\
      the \"usr\" part.  Deinstall the fileset (using -u flag) or \"force\n\
      re-install\" the fileset (using -F flag).\n"

#define CKP_NO_REJ_COMMTD_PKG_D "\
  \"%c\" fileset is in the committed state.  A committed fileset cannot be\n\
      rejected.  (Committed software can be removed with the deinstall\n\
      facility, i.e., \"Remove Software Products\" or -u flag.  NOTE: This\n\
      will remove an entire fileset.)\n"

#define CKP_NO_COMT_PARTLY_APPLD_D "\
  \"%c\" partially applied fileset (i.e., the \"usr\" and \"root\" parts are\n\
      not both applied).  A fileset must be fully applied before it can be\n\
      committed.\n"

#define CKP_RECOVER_PARTLY_INSTLD_D "\
      Use software listing facilities (lslpp command) to determine which\n\
      parts are missing.  If the \"usr\" part is missing, reject (or remove/\n\
      deinstall) the fileset so that it may be re-installed.  If the \"root\"\n\
      part is missing, request that the fileset be installed again, and the\n\
      \"root\" part will be installed.\n"

#define CKP_NON_UNIQUE_VRMF_PRQ_D "\
  \"%c\" base level fileset for which there is another version that is either\n\
      already installed or that was selected to be installed during this\n\
      installation session.\n"

#define CKP_SHARE_PART_NOT_RQSTD_D "\
  \"%c\" fileset contains a \"share\" part.  (\"share\" parts were not\n\
      selected for this invocation of installp.)\n"

#define CKP_SUPD_MUST_APP_FOR_CONSIST_D "\
  \"%c\" superseded fileset that is applied on the \"usr\" part which must\n\
      also be applied on the \"root\" part for consistency.  Select this\n\
      fileset explicitly or use the option to automatically include\n\
      requisite software (-g flag).\n"

#define CKP_REQ_AVAIL_OTHER_PROBLEMS_D "\
  \"%c\" fileset is available on the installation media but is not currently\n\
      installed.  This fileset is failing to install because the requirements\n\
      of the group requisite, of which the fileset is a member, cannot be\n\
      met or because the fileset has requisites of its own which are causing\n\
      problems.  Request this fileset for installation by itself to determine\n\
      the actual reason.  (Use the \"Preview\" option (-p flag) to prevent\n\
      the installation from actually occurring.)\n"

#define CKP_UNCLEAR_FAILURE_D "\
  \"%c\" unclear why this fileset failed.  Use the software listing (lslpp\n\
      command) and software verification facilities (lppchk command) to try\n\
      to establish further information on this fileset.\n"

#define CKP_UPDATES_ONLY_D "\
  \"%c\" this is the base level for a fileset.  You asked to install\n\
      updates only (with the -B flag).  The base level fileset will not\n\
      be automatically installed.\n"

#define CKP_NO_REJ_PRTLY_COMMTD_D "\
  \"%c\" fileset is in a partially committed state (i.e., the \"usr\" or\n\
      \"root\" part is committed).  A committed fileset cannot be rejected.\n\
      (Committed software can be removed with the deinstall facility, i.e.,\n\
      \"Remove Software Products\" or -u flag.  NOTE: This will remove an\n\
      entire fileset.)\n"

#define CKP_BROKEN_PKG_D "\
  \"%c\" fileset is installed but in the BROKEN state.\n"

#define CKP_REQS_AVLBL_NO_G_D "\
  \"%c\" fileset is available on the installation media but not currently\n\
      installed.  Explicitly request that the fileset be installed, or use\n\
      the option to automatically install requisite software (-g flag).\n"

#define CKP_REQS_NOT_AVAILABL_D "\
  \"%c\" fileset is not installed and not available on the current\n\
      installation media.\n"

#define CKP_REQS_PARTLY_APPLD_D "\
  \"%c\" partially installed fileset (i.e., the \"usr\" and \"root\" parts\n\
      are not both installed).\n"

#define CKP_REQS_PARTLY_COMTD_D "\
  \"%c\" fileset is in a partially committed state (i.e., the \"usr\" and\n\
      \"root\" parts are not both committed but must be to satisfy the\n\
      requisite check).\n"

#define CKP_USR_PART_NOT_RQSTD_D "\
  \"%c\" fileset contains a \"usr\" part.  (\"usr\" parts were not selected\n\
      for this invocation of installp.)\n"

#define CKP_USR_ROOT_PARTS_NOT_RQSTD_D "\
  \"%c\" fileset contains both \"usr\" and \"root\" parts.  Both parts were\n\
      not selected.  (If installing this fileset on a client, the \"usr\"\n\
      (server) part is more than likely not yet installed.)\n"

#define CKP_BROKEN_SUPS_D "\
  \"%c\" fileset update supersedes a BROKEN update.  In order to install\n\
      this fileset or to fix the BROKEN update, you must specify options to\n\
      the installation commands that will apply, commit, and automatically\n\
      include requisite software without saving replaced files (-acgN flags).\n"

#define CKP_NO_PRQ_BASE_D "\
  \"%c\" fileset update does not prereq its base level fileset.  An update\n\
      must prereq its base level fileset directly or indirectly (by prereqing\n\
      another update which prereqs the base fileset).\n"

#define CKP_INSTREQ_FAILURE_D "\
  \"%c\" fileset must be installed prior to or at the same time as its\n\
      dependent filesets.  This requisite cannot be installed automatically\n\
      (using -g option).  It must be explicitly selected for installation.\n"

#define CKP_PKG_NOT_INSTLD_D "\
  \"%c\" currently not installed on the system.\n"

#define CKP_BROKEN_REQUISITES_D "\
  BROKEN REQUISITES:  The following filesets are requisites of one or more\n\
  of the selected filesets listed above.  They are currently installed\n\
  on the system but are in the BROKEN state.\n"

#define CKP_RECOVER_BROKEN_D "\
  Alternative actions that can be used to correct broken filesets are:\n\
  1. If the broken fileset is an update, re-install the update with options\n\
     to the installation program that will apply, commit and automatically\n\
     install requisite software without saving replaced files: -acgN flags.\n\
  2. Re-install the entire fileset, i.e., the base level and any desired \n\
     updates (may need to use the \"FORCE\" option: -F flag). \n\
  3. Deinstall the entire fileset (using \"Remove Software Products\": \n\
     -u flag).\n"

#define CKP_NON_DEINST_PKG_D "\
  The following filesets were selected for deinstallation.  Deinstallability\n\
  checks indicate that they should not be removed from the system.\n\n"

#define CKP_NON_DEINST_DEP_D "\
  NON-DEINSTALLABLE DEPENDENTS:  The following filesets depend upon one or\n\
  more of the selected filesets listed above.  These dependents should\n\
  be removed before the selected filesets.  Deinstallability checks indicate\n\
  that these dependents should not be removed from the system; therefore, the\n\
  selected filesets cannot be removed.\n"

#define CKP_CONFL_BASE_LEV_D "\
  CONFLICTING REQUISITES:  The following filesets are required by one or\n\
  more of the selected filesets listed above.  There are other versions of\n\
  these filesets which are already installed (or which were selected to be\n\
  installed during this installation session).  A base level fileset cannot\n\
  be installed automatically as another fileset's requisite when a different\n\
  version of the requisite is already installed.  You must explicitly select\n\
  the base level requisite for installation.\n"

#define CKP_NOTHING_TO_COMMIT_D "\
  Nothing to Commit\n\
  -----------------\n\
  There is nothing in the APPLIED state that needs to be committed.\n\n"

#define CKP_NOTHING_TO_APPLY_D "\
  Nothing to Install\n\
  ------------------\n\
  There are no fileset \"root\" parts (i.e., client specific software)\n\
  which need to be installed at this time.\n\n"

#define CKP_CANT_FORCE_APPLY_PTF_D "\
  Force Apply Failures\n\
  --------------------\n\
  The following is a list of fileset updates.  Updates cannot be specified\n\
  from the command line when the force flag (-F) is used in combination\n\
  with the apply flag (-a).\n\n"

#define CKP_WILL_PULL_IN_NEWER_D "\
  NOTE:  There is a newer version of one or more of these requisite filesets\n\
	 on the installation media.  If you choose to automatically install\n\
	 these requisites, the newer version will be selected instead.\n\
	 Do not use the auto-install option (-g flag) if you want the older\n\
	 version to be installed.\n"

#define CKP_SELECTED_SUPS_BROKEN_D "\
  Supersedes of BROKEN Filesets \n\
  ----------------------------- \n\
  The following selected fileset updates supersede others that are installed\n\
  but in the BROKEN state.  In order to install the superseding fileset, use\n\
  options to the installation program that will apply, commit and\n\
  automatically install requisite software without saving replaced files:\n\
  -acgN flags.\n\n"

#define CKP_MIGRATING_REQ_D "\
  \"%c\" fileset is partially migrated from an earlier installation.\n\
      A partially migrated fileset cannot be updated.\n"

#define CKP_MIGRATING_DEP_D "\
  \"%c\" fileset is partially migrated from an earlier installation.\n\
      A partially migrated fileset cannot be deinstalled.  (Try again\n\
      when the fileset has been fully migrated.)\n"

#define CKP_LPPCHK_INCONSIST_FAILURE_HDR_D "\
%s:  The following filesets need to be installed or corrected to bring\n\
	 the system to a consistent state:\n\n"

#define CKP_LPPCHK_DEP_FAILURE_HDR_D "\
%s:  The following describes dependencies from installed filesets\n\
	 on one or more of the filesets listed above:\n"

#define CKP_LPPCHK_NOT_INSTALLED_REQ_FILESET_D "\
not installed; requisite fileset"

#define CKP_LPPCHK_NOT_INSTALLED_D "\
not installed"

#define CKP_RENAMED_SUPD_PKG_D "\
  \"%c\" older version of a fileset that has been renamed.  The newer,\n\
      renamed fileset, which supersedes this one, is already installed (or\n\
      was selected to be installed during this installation).  A fileset\n\
      which has undergone a name change cannot be re-installed over the newly\n\
      named fileset.  The newer fileset must first be removed.\n"

#define CKP_NO_MULT_BARR_D "\n\
%s:  Error:  A fileset cannot reference its own name more than\n\
\tonce in its supersedes data.  The fileset in error is:\n\t%s\n\n"

#define CKP_NO_COMPAT_MATCH_D "\n\
%s:  Error:  A compatibility entry in the supersedes data of\n\
\ta fileset should have the same name but not the same level as the\n\
\tcompatible fileset.  The level should be greater than all base levels\n\
\tand updates released for the compatible (superseded) fileset.  The\n\
\tfileset in error is:\n\t%s\n\n"

#define CKP_NO_SUPPORT_MANY_COMPAT_D "\n\
%s:  Error:  Multiple compatibilty entries in the supersedes\n\
\tdata of a fileset are not currently supported.  A fileset may only have\n\
\tone name different than its own in its supersedes data.\n\
\tThe fileset in error is:\n\t%s\n\n"

#define CKP_DOTTED_NUM_MUST_HAVE_4_PTS_D "\n\
%s:  Error:  A dotted number in a requisite expression must\n\
\tcontain four parts to specify version, release, modification and fix\n\
\tnumber.  The fileset in error is:\n\
\t%s\n\n"

#define CKP_ALRDY_INSTLD_NOTE_D "\
  NOTE:  Base level filesets may be reinstalled using the \"Force\"\n\
  option (-F flag), or they may be removed, using the deinstall or\n\
  \"Remove Software Products\" facility (-u flag), and then reinstalled.\n\n"

#define CKP_INT_ERR_DISABLING_PKGS_D "\n\
%s:  Internal error.  Unable to resolve conflicts between\n\
\tbase level filesets which are already installed and/or which are on\n\
\tthe installation media.  Use local problem reporting procedures.\n\
\t(NOTE:  This error has not affected the current installation\n\
\tif the two filesets listed here do not appear in subsequent failure\n\
\tmessages AND if there are no references to \"disabled\" filesests\n\
\tin subsequent failure messages.)\n\
\tfileset1: %s %s\n\
\tfileset2: %s %s\n\n"

#define CKP_DISABLING_INT_ERR_D "\
  \"%c\" fileset was disabled for the current installation due to an internal\n\
      error which occurred.  Use local problem reporting procedures.\n"

#define CKP_CONFL_BASE_LEV_NOTE_D "\
  NOTE:  A requisite listed below supersedes (ie. replaces) the fileset with\n\
  which it is in conflict but the two have different names.  The installed\n\
  fileset is displayed beneath the conflicting fileset.\n\n"

#define CKP_CONFL_WITH_D "\
      (Conflicts with: %s)\n"

#endif /* _H_CKPREREQMSG */
