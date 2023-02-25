$ @(#)78	1.73.5.2  src/bos/usr/sbin/install/messages/cmdinstl_e.msg, cmdinstl, bos41J, 9510A_all 2/27/95 12:14:51
$
$
$ COMPONENT_NAME:  (CMDINSTL) High Level Install Command
$
$ FUNCTIONS:  Message catalog for CMDINSTL
$
$ ORIGINS:  27
$
$  (C) COPYRIGHT International Business Machines Corp. 1991, 1994
$   All Rights Reserved
$   Licensed Materials - Property of IBM
$   US Government Users Restricted Rights - Use, duplication or
$   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$ --------------------------------------------------------------------


$quote "
$ -------------------------------------------------
$set MS_BFF
$ These are the messages for the bffcreate command.
$ Message numbers 0503-600 through 0503-619.
$ -------------------------------------------------

BFF_FAILED_CREATE_E "\
0503-600 bffcreate:  Failed creating the backup format file for package\n\
\t%s.\n"

BFF_NOLPPS_E "\
0503-601 bffcreate:  No package names were specified on the command line.\n"

BFF_NO_SIZE_IN_TOC_I "\
bffcreate:  Warning: important size information is missing from the table\n\
\tof contents file.  Consequently, there may not be enough free\n\
\tfile system space to successfully create the backup format file\n\
\timages.  Continuing anyway ...\n\n"

BFF_USAGE_E "\
Usage:  bffcreate [-qvX] [-d device] [-t savedir] [-w workdir]\n\
                 {-l | package1 [level1][, package2 [level2], ...] | all}\n\
       where\n\
           -d  <device>   input (d)evice containing source media\n\
                                 default is /dev/rfd0\n\
           -l             (l)ist media contents\n\
           -q             (q)uiet mode, don't needlessly prompt for device\n\
           -t  <savedir>  (t)arget directory in which images will be created\n\
                                 default is /usr/sys/inst.images\n\
           -v             (v)erbose mode, echoes image name\n\
           -w  <workdir>  (w)ork directory used for temporary work space\n\
                                 default is /tmp\n\
           -X             autoe(X)pand\n\n\
           package        is the package name\n\
           level          is of the form vv.rr.mmmm.ffff.ppppppppp\n\
                                        (version.release.mod.fix.ptf_id)\n"

$ Do not translate bffcreate.
$ Words in the syntax which may be translated are:  savedir, device,
$ workdir, and package.

BFF_NO_LPPS_W_L_FLAG_E  "\
0503-602 bffcreate:  A package list is not permitted with the -l flag.\n"

BFF_INVALID_LEVEL_E  "\
0503-603 bffcreate:  An invalid level was given for package\n\
\t%s:  %s\n\
\tALL levels must be in one of the following formats:  \n\
\t    vv.rr.mmmm.ffff   OR   vv.rr.mmmm.fffff.ppppppppp\n"

BFF_PKG_NOT_FOUND_E  "\
0503-604 bffcreate:  The specified software package %s\n\
\twas not found on device %s\n"

BFF_PKG_W_LEVEL_NOT_FOUND_E "\
0503-605 bffcreate:  The specified software package %s %s\n\
\twas not found on device %s\n"

BFF_PKG_LIST_HDR_I_OLD "\
   Package Name                   Level\n"

BFF_NO_ALL_W_PKG_LIST_E  "\
0503-606 bffcreate:  The \"all\" keyword cannot be given with a package list.\n"

BFF_PKG_LIST_HDR_I "\
   Package Name                   Level                     Content\n"


$ ------------------------------------------------
$set MS_CKPREREQ
$ These are the messages for the ckprereq command.
$ Message numbers 0503-200 through 0503-300.
$ ------------------------------------------------

$ !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
$ NOTES TO TRANSLATORS..........
$ 1. When indentation is visible in the CKP_ messages, please preserve that 
$    indentation when possible and appropriate.  (See CKP_AUTO_SUPS_WARNING1_W
$    for example.)
$ 2. Please ensure that dashed underlines span the length of the entire
$    translated strings.  (See CKP_MISC_FAILURES_TITLE_I for example.)
$ 3. Do not translate any occurrences of \"usr\", \"root\", or \"share\"
$ !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

CKP_USAGE_E "\
Usage:  ckprereq [-f PrereqFile | -l fileset] [-O {r|u|s}] [-v]\n\
\tChecks status information held in either the PrereqFile or the\n\
\tSoftware Vital Product Data about installed filesets.\n\
\t\t-f Specifies the name of the prerequisite file.\n\
\t\t\tThe default name is prereq.\n\
\t\t-l Specifies the name of the fileset for which to get \n\
\t\t\tinformation from the Software Vital Product Data.\n\
\t\t-O Specifies which Software Vital Product Data \n\
\t\t\tdatabase to search:  root, usr, or share.\n\
\t\t-v Specifies verbose mode.\n"
$ PrereqFile represents the name of the prerequisite file.
$ Do not translate ckprereq, prereq, -f, -l, -O, r,u,s, or -v.

CKP_BAD_INUTREE_E "\
0503-200 %s:  The environment variable INUTREE must be one of \n\
\tthe following:  %c, %c or %c.\n"
$ Do not translate INUTREE.

CKP_BAD_OFLAG_E "\
0503-201 %s:  One of the following must be specified with the \n\
\t-O flag:  %c, %c or %c.\n"
$ Do not translate -O.

CKP_EXIT_E "\
0503-202 %s:  The ckprereq routine is exiting with an error.\n"
$ Do not translate ckprereq.

CKP_INCONSISTNT_REQS_E "\
0503-203 %s:  Error in the Software Vital Product Data. The \"usr\"\n\
\tpart of a fileset does not have the same requisites as the \"root\"\n\
\tpart. The fileset is:  %s\n"

CKP_INCONSISTNT_SDEDBY_E "\
0503-204 %s:  Error in the Software Vital Product Data. The \"usr\"\n\
\tpart of an update is not superseded by the same fileset update as the\n\
\t\"root\" part. The update is:  %s\n"

CKP_INSTP_REQ_CANCL_E "\
0503-205 %s:  The installation of the following filesets will be\n\
\tCANCELLED because filesets upon which they depend are not installed\n\
\t(or not completely installed):\n"

CKP_INSTP_REQ_CANCL_ROOT_E "\
0503-206 %s:  The installation of the \"root\" part of the following\n\
\tfilesets will be CANCELLED because filesets upon which they\n\
\tdepend are not installed (or not completely installed):\n"

CKP_INT_CODED_MSG_E "\
0503-207 %s:  Internal error. Code %d.\n\
\tUse local problem reporting procedures.\n"

CKP_INT_UNEXP_LPPMICRO_E "\
0503-208 %s:  Internal error. Unexpected OP_TYPE for MICRO_CODE fileset.\n\
\tFileset is %s\n\
\tUse local problem reporting procedures.\n"

CKP_INVALID_MC_REQ1_E "\
0503-209 %s:  Error.  A fileset maintenance level has an illegal\n\
\trequisite.  The only valid requisite type for a maintenance\n\
\tlevel is \"prereq.\"\n\
\tThe fileset is:  %s\n"

CKP_INVALID_MC_REQ2_E "\
0503-210 %s:  Error. A 3.2-formatted fileset maintenance level cannot\n\
\thave a requisite on a fileset with a different name than itself.\n\
\tThe fileset in error is:  %s\n"

CKP_LEX_ILLBASE_E "\
0503-211 %s:  A lexical error occurred for %s \n\
\ton line %d, column %d.\n\
\tThere is an illegal base level:  %s.\n\
\tUse local problem reporting procedures.\n"

CKP_LEX_ILLCHR_E "\
0503-212 %s:  A lexical error occurred for %s \n\
\ton line %d, column %d.\n\
\tAn illegal character, %c, was found (hexadecimal value:  %x).\n\
\tAll characters must be ASCII.\n\
\tThe character will be treated as a SPACE.\n\
\tUse local problem reporting procedures.\n"
$ The %c is the illegal character, %x the hexadecimal value.

CKP_LEX_ILLNAME_E "\
0503-213 %s:  A lexical error occurred for %s \n\
\ton line %d, column %d.\n\
\tThere is an illegal name, PTF ID, requisite, \n\
\t\tor field name:  %s.\n\
\tNames must be more than one character in length\n\
\t\tand begin with a letter or '_'.\n\
\tSubsequent characters in a name may be a letter, a digit, \n\
\t\ta '_', a '-', or a '.'.\n\
\tA PTF ID has the same syntax as a name.\n\
\tA requisite is one of:  *prereq, *coreq, *instreq, or *ifreq.\n\
\tA field name is one of:  'V', 'R', 'M', 'F', 'P', or 'O'.\n\
\tUse local problem reporting procedures.\n"
$ The first %s is the command name.
$ The second %s is the file or fileset name for which the error occurred.
$ The third %s is the illegal name.
$ Do not translate prereq, coreq, ifreq, instreq.
$ Do not translate field names V, R, M, F, P, or O.

CKP_LEX_ILLNUM_E "\
0503-214 %s:  A lexical error occurred for %s \n\
\ton line %d, column %d.\n\
\tThere is an illegal number:  %s.\n\
\tUse local problem reporting procedures.\n"

CKP_LEX_LNGNAME_E "\
0503-215 %s:  A lexical error occurred for %s\n\
\ton line %d, column %d.\n\
\tThe name %s is too long.\n\
\tMaximum name length is %d. The name has been truncated.\n\
\tUse local problem reporting procedures.\n"

CKP_LEX_REQEXP_E "\
0503-216 %s:  A lexical error occurred for %s \n\
\ton line %d, column %d.\n\
\tOne of the following was expected instead of %s:\n\
\t\t*prereq, *coreq, *instreq, or *ifreq.\n\
\tUse local problem reporting procedures.\n"
$ Do not translate prereq, coreq, ifreq, instreq.

CKP_LEX_UNXCHR_E "\
0503-217 %s:  A lexical error occurred for %s\n\
\ton line %d, column %d.\n\
\tAn unexpected character, %c, was found (hexadecimal value:  %x).\n\
\tThe character will be treated as a SPACE.\n\
\tUse local problem reporting procedures.\n"
$ The %c is the unexpected character, %x the hexadecimal value.

CKP_NOSWVPD_E "\
0503-218 %s:  Fileset %s\n\
\tis not in the Software Vital Product Data. \n\
\tUse local problem reporting procedures.\n"

CKP_PARS_CLOSEB_E "\
0503-219 %s:  A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tThere is a missing close brace '}'.\n\
\tUse local problem reporting procedures.\n"

CKP_PARS_CLOSEP_E "\
0503-220 %s:  A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tMissing close parenthesis ')' after base level in an *ifreq expression.\n\
\tUse local problem reporting procedures.\n"

CKP_PARS_DOT_NUMB_E "\
0503-221 %s:  A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tA base level was expected following an open parenthesis '(' in an\n\
\t*ifreq expression instead of %s.\n\
\tUse local problem reporting procedures.\n"

CKP_PARS_EXPNAM_E "\
0503-222 %s:  A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tA fileset name was expected instead of %s.\n\
\tUse local problem reporting procedures.\n"

CKP_PARS_EXPVRM_E "\
0503-223 %s:  A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tA 'V', 'R', 'M', 'F', or 'P' was expected\n\
\t\tinstead of '%s'.\n\
\tUse local problem reporting procedures.\n"
$ Do not translate V, R, M, F, and P.

CKP_PARS_FIXRPT_E "\
0503-224 %s:  A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tThe fix field, 'F', has already been found.\n\
\tUse the OR expression, 'O', to compare multiple values\n\
\t\tof the fix field.\n\
\tUse local problem reporting procedures.\n"
$ Do not translate F and O.

CKP_PARS_ILLPTF_E "\
0503-225 %s:  A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tAn illegal PTF ID value, %s, was found.\n\
\tUse local problem reporting procedures.\n"

CKP_PARS_INVOR_E "\
0503-226 %s:  A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tAn OR expression, 'O', is invalid at this point.\n\
\tUse local problem reporting procedures.\n"
$ Do not translate O.

CKP_PARS_MODRPT_E "\
0503-227 %s:  A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tThe modification field, 'M', has already been found.\n\
\tUse the OR expression, 'O', to compare multiple values\n\
\t\tof the modification field.\n\
\tUse local problem reporting procedures.\n"
$ Do not translate M and O.

CKP_PARS_NONUMB_E "\n\
0503-228 %s:  A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tA version, release, modification, or fix number\n\
\twas expected where %s was found.\n\
\tUse local problem reporting procedures.\n"

CKP_PARS_NOPTFN_E "\
0503-229 %s:  A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tA number or PTF ID was expected where %s was found.\n\
\tUse local problem reporting procedures.\n"

CKP_PARS_OPENB_E "\
0503-230 %s:  A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tThere is a missing open brace '{'.\n\
\tUse local problem reporting procedures.\n"

CKP_PARS_OROPER_E "\
0503-231 %s:  A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tAll operands of an OR expression, 'O', must operate \n\
\t\ton the same field.\n\
\tUse local problem reporting procedures.\n"
$ Do not translate O.

CKP_PARS_PASSC_E "\
0503-232 %s:  A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tThe number of items in the group required to pass\n\
\t\twas expected instead of %s.\n\
\tUse local problem reporting procedures.\n"

CKP_PARS_PASSLESS_E "\
0503-233 %s:  A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tThe group has fewer items than was specified for the number \n\
\t\trequired to pass.\n\
\tUse local problem reporting procedures.\n"

CKP_PARS_PTFRPT_E "\
0503-234 %s:  A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tThe PTF ID field, 'P', has already been found.\n\
\tUse the OR expression, 'O', to compare multiple values\n\
\t\tof the PTF ID field.\n\
\tUse local problem reporting procedures.\n"
$ Do not translate P and O.

CKP_PARS_RLSRPT_E "\
0503-235 %s:  A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tThe release field, 'R', has already been found.\n\
\tUse the OR expression, 'O', to compare multiple values\n\
\t\tof the release field.\n\
\tUse local problem reporting procedures.\n"
$ Do not translate R and O.

CKP_PARS_UNXTYP_E "\
0503-236 %s:  A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tInstead of %s, one of the following was expected:\n\
\t\ta '>' (to start a group) or \n\
\t\t*prereq, *instreq, *coreq, *ifreq or\n\
\ta fileset name.\n\
\tUse local problem reporting procedures.\n"
$ Do not translate ifreq, prereq, coreq, instreq.

CKP_PARS_VRMF_LEVEL_E "\
0503-237 %s:  A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tIt is illegal to specify version, release, modification or fix fields\n\
\twhen a base level is given in an *ifreq expression.\n\
\tUse local problem reporting procedures.\n"

CKP_PARS_VRSRPT_E "\
0503-238 %s:  A parsing error occurred for %s\n\
\ton line %d, column %d.\n\
\tThe version field, 'V', has already been found.\n\
\tUse the OR expression, 'O', to compare multiple values\n\
\t\tof the version field.\n\
\tUse local problem reporting procedures.\n"
$ Do not translate V and O.

CKP_SELF_COREQ_E "\
0503-239 %s:  Error. %s is a corequisite of itself.\n"

CKP_SELF_PREREQ_E "\
0503-240 %s:  Error. %s is a prerequisite of itself.\n"

CKP_SUP_FIX_NOT_IN_VPD_E "\
0503-241 %s:  Cannot find superseding update in Software Vital \n\
\tProduct Data database for:  %s. \n\
\tSuperseding update is:  %s.\n"

CKP_SWVPD_2PARTS_E "\
0503-242 %s:  The Software Vital Product Data indicates\n\
\tthat %s has a \"share\" and \"root\" part\n\
\tor a \"share\" and \"usr\" part.\n\
\tUse local problem reporting procedures.\n"
$ Do not translate share, root, usr.

CKP_ILLEGAL_OLD_IFREQ_E "\
0503-243 %s:  Error:  A 4.1-formatted package must not contain an\n\
\t\"ifreq\" to a fileset update without also specifying the base\n\
\tlevel of the fileset.  The ifreq expression must contain a\n\
\tdotted, numeric level as in:\n\
\t\t*ifreq <fileset_name> (v.r.m.f) v.r.m.f\n\
\t\t\tor\n\
\t\t*ifreq <fileset_name> v.r.m.f\n\
\t\t\t(if the requisite fileset is in 4.1 format) or \n\
\t\t*ifreq <fileset_name> (v.r.m.f) p=<ptf_id>\n\
\t\t\t(if the requisite fileset is in 3.2 format)\n\
\twhere \"(v.r.m.f)\" is the base fileset level of the requisite\n\
\tand \"v.r.m.f\" or \"p=<ptf_id>\" is the level of the requisite\n\
\tupdate itself.  If a single, non-parenthetic level is provided,\n\
\tthe base level is implied from the level of the update itself.\n\
\tThe update in error is:  %s\n\
\tThe prereq information is:\n%s\n\n"

CKP_NO_UPDATE_INSTREQS_E "\
0503-244 %s:  Error:  A fileset update must not contain the requisite type\n\
\t\"instreq\".\n\
\tThe update in error is:  %s\n\n"

CKP_41_UPDT_NO_PRQ_SAME_NAME_E "\n\
0503-245 %s:  Error:  A 4.1-formatted fileset update has an illegal\n\
\trequisite on a fileset with the same name as itself.  The requisite\n\
\ttype MUST be \"*prereq\".\n\
\tThe fileset in error is:\n\
\t%s\n\n"

CKP_ILLEGAL_LEVEL_FOR_BASE_E "\n\
0503-246 %s:  Error:  Expecting a base level prereq expression using a dotted,\n\
\tnumeric level (e.g., 1.1.0.0) for the fileset update:\n\
\t%s\n\
\tInstead, the following was found:\n\
\t%s\n\n"

CKP_NO_NEWER_32_THAN_41_E "\n\
0503-247 %s:  Error:  A fileset contained in a 4.1-formatted package\n\
\tcannot have a lower level (ver.rel.mod.fix) than a fileset with the\n\
\tsame name in a 3.2 or 3.1-formatted package.\n\
\tThe filesets in error are:\n\
\t%s\n\
\tand\n\
\t%s\n\n"

CKP_ILLEGAL_SUPS_PARTS_E "\n\
0503-248 %s:  Error:  A fileset has parts that are different from one or\n\
\tmore of the filesets it supersedes.  It is NOT valid for a fileset with\n\
\ta \"usr\" part only to supersede a fileset with both \"usr\" and \"root\"\n\
\tparts.  A fileset with a \"share\" part may only supersede another fileset\n\
\twith a \"share\" part.\n\
\tThe fileset in error is:\n\
\t%s\n\n"

CKP_ILLEGAL_BASE_IFREQ1_E "\n\
0503-249 %s:  Error:  The base level implied or specified in an \"*ifreq\"\n\
\texpression is not a base level fileset.  The fileset in error is:\n\
\t%s\n\n"

CKP_ILLEGAL_BASE_IFREQ2_E "\n\
0503-250 %s:  Error:  A fileset has an update level in an \"*ifreq\"\n\
\texpression that is not greater than the base level specified.  The\n\
\tfileset in error is:\n\
\t%s\n\n"

CKP_TOO_MANY_REQS_ON_SAME_FILESET1_E "\n\
0503-251 %s:  Error:  A 4.1-formatted fileset has more than one requisite\n\
\ton filesets with its own name.  The fileset in error is:\n\
\t%s\n\n"

CKP_TOO_MANY_REQS_ON_SAME_FILESET2_E "\n\
0503-252 %s:  Error:  A 4.1-formatted fileset has more than one requisite of\n\
\tthe type \"*prereq\", \"*coreq\", or \"*instreq\" that are not contained\n\
\tin a group requisite expression.  The fileset in error is:\n\
\t%s\n\n"

CKP_NO_MIX_N_MATCH_E "\n\
0503-253 %s:  Error:  A fileset update prereqs a base level fileset with a\n\
\tdifferent package format.  (A 4.1-formatted update cannot prereq a\n\
\t3.2-formatted base level and vice versa.)  The update in error is:\n\
\t%s\n\n"

CKP_NO_PRQ_BASE_ERR_E "\n\
0503-254 %s:  Error:  An update does not prereq its base level fileset with\n\
\tthe same version and release, the same or lower modification number, or\n\
\tlower fix number.  The update in error is:\n\
\t%s\n\n"

CKP_WHERE_I "\
where:\n"

CKP_AUTO_SUPS_WARNING1_W "\
  NOTE:  One or more fileset updates in this list are flagged with \"*\".\n\
         These updates supersede (i.e., replace) updates which you\n\
         selected.  Newer level fileset updates will always be \n\
         automatically chosen instead of the fileset udpates they\n\
         supersede when the \"auto-install\" option (i.e., \"AUTOMATICALLY\n\
         install requisite software\" or -g flag) is specified.\n"

CKP_MISC_FAILURES_TITLE_I "\
  Miscellaneous Failures\n\
  ----------------------\n"

CKP_MISC_APPLY_FAILURES_HDR_I "\
  The following is a list of filesets that you asked to install.  They failed\n\
  pre-installation checks for one of several reasons.  See the \"Failure Key\"\n\
  for failure reasons and possible recovery hints.\n\n"

CKP_MISC_COMMIT_FAILURES_HDR_I "\
  The following is a list of filesets that you asked to commit.  They failed\n\
  pre-commit checks for one of several reasons.  See the \"Failure Key\"\n\
  for failure reasons and possible recovery hints.\n\n"

CKP_MISC_REJECT_FAILURES_HDR_I "\
  The following is a list of filesets that you asked to reject.  They failed\n\
  pre-reject checks for one of several reasons.  See the \"Failure Key\"\n\
  for failure reasons and possible recovery hints.\n\n"

CKP_MISC_DEINST_FAILURES_HDR_I "\
  The following is a list of filesets that you asked to remove.  They failed\n\
  pre-deinstall checks for one of several reasons.  See the \"Failure Key\"\n\
  for failure reasons and possible recovery hints.\n\n"

CKP_ALRDY_INSTLD_HDR_PREVIEW_I "\
  Already Installed\n\
  -----------------\n\
  The following filesets which you selected are either already installed\n\
  or effectively installed through superseding filesets.\n\n"

CKP_ALRDY_INSTLD_HDR_I "\
  Already Installed\n\
  -----------------\n\
  The number of selected filesets that are either already installed\n\
  or effectively installed through superseding filesets is %d.  See\n\
  the summaries at the end of this installation for details.\n\n"

CKP_ALRDY_COMTD_HDR_I "\
  Already Committed\n\
  -----------------\n\
  The number of selected filesets that are already committed is %d.\n\n"

CKP_MAIN_FAILURES_TITLE_I "\
FAILURES\n\
--------\n"

CKP_APPLY_FAILURES_HDR_I "\
  Filesets listed in this section failed pre-installation verification\n\
  and will not be installed.\n\n"

CKP_COMT_FAILURES_HDR_I "\
  Filesets listed in this section failed pre-commit verification\n\
  and will not be committed.\n\n"

CKP_DEINST_FAILURES_HDR_I "\
  Filesets listed in this section failed pre-deinstall verification\n\
  and will not be removed.\n\n"

CKP_REJ_FAILURES_HDR_I "\
  Filesets listed in this section failed pre-reject verification\n\
  and will not be rejected.\n\n"

CKP_NO_SUPS_IN_USR_W "\
  No Supersedes Information in USR Part\n\
  -------------------------------------\n\
  The following fileset updates have supersedes information in the \"root\"\n\
  part but not the \"usr\" part.  The information from the \"root\" part will\n\
  be used if necessary.\n\n"

CKP_NO_PRQ_IN_USR_W "\
  No Prereq Information in USR Part\n\
  ---------------------------------\n\
  The following have prereq information in the \"root\" part but not the\n\
  \"usr\" part.  The information from the \"root\" part will be used if\n\
  necessary.\n\n"

CKP_NO_PRQ_BASE_WARN_W "\
  No Prereq on Base Level Fileset\n\
  -------------------------------\n\
  The following fileset updates do not prereq their base level fileset at all\n\
  or do not prereq a base level fileset with the same version and release,\n\
  the same or lower modification number, or lower fix number.  This could\n\
  cause dependency checks on these updates to fail.\n\n"

CKP_MISS_USR_APPLD_ROOT_W "\
  No USR Part for Applied ROOT Part\n\
  ---------------------------------\n\
  The following are applied on the \"root\" part but not on the \"usr\"\n\
  part.  To correct their state, try to reject the root part.  If desired,\n\
  you may attempt to remove the fileset with the \"Remove Software\n\
  Products\" or deinstall facility (-u flag).  Alternatively, you may attempt\n\
  to re-install the fileset using \"FORCE\" (-F flag).\n\n"

CKP_MISS_USR_COMTD_ROOT_W "\
  No USR Part for Committed ROOT Part\n\
  -----------------------------------\n\
  The following are committed on the \"root\" part but not installed on the\n\
  \"usr\" part.  To correct their state, you may attempt to remove the\n\
  fileset with the \"Remove Software Products\" or deinstall\n\
  facility (-u flag).  Alternatively, you may attempt to re-install the\n\
  fileset using \"FORCE\" (-F flag).\n\n"

CKP_AVLBL_IFREQ_HDR_W "\
  Available Requisites of Previously Installed Software\n\
  -----------------------------------------------------\n\
  The following filesets are requisites of software that is already installed.\n\
  These filesets are not currently installed but should be to ensure that the\n\
  system functions correctly.  You did not select these filesets for\n\
  installation, but they are available on the installation media.\n"

CKP_AVLBL_IFREQ_FAIL_W "\
  An attempt would have been made to install these filesets, but they appear\n\
  to have missing or inconsistent requisites of their own.  For further\n\
  details, select these filesets explicitly for installation.\n\n"

CKP_AVLBL_IFREQ_USE_GFLAG_W "\
  They may be explicitly selected, or they will be automatically installed\n\
  whenever you specify options to the installation commands to automatically\n\
  include requisite software (-g flag).\n\n"

CKP_MSNG_IFREQ_TITLE_W "\
  Missing Requisites of Previously Installed Software\n\
  ---------------------------------------------------\n"

CKP_MSNG_IFREQ_CLIENT_W "\
  The following filesets are requisites of software that is already installed.\n\
  They should be installed to ensure that the system functions correctly.\n\
  These filesets are not installed on the \"usr\" (server) part and,\n\
  therefore, they cannot be installed on the \"root\" (client) part.\n\n"

CKP_MSNG_IFREQ_W "\
  The following fileset updates are requisites of software that is already\n\
  installed.  They are not currently installed but should be to ensure that\n\
  the system functions correctly.  These updates are not on the current\n\
  installation media.\n\n"

CKP_RESULTS_I "\
Results...\n\n"

CKP_E_O_WARNINGS_I "\
  << End of Warning Section >>\n\n"

CKP_E_O_FAILURES_I "\
  << End of Failure Section >>\n\n"

CKP_E_O_SUCCESS_I "\
  << End of Success Section >>\n\n"

CKP_OTHER_BROKENS_I "\
  Updates to BROKEN Filesets\n\
  --------------------------\n\
  The following selected fileset updates are updates to filesets which have\n\
  one or more updates in the BROKEN state.  Before installing any other\n\
  updates to these filesets, you must correct the state of the BROKEN\n\
  updates.\n\n"

CKP_BROKEN_PKGS_I "\
  BROKEN Filesets\n\
  ---------------\n\
  The following selected fileset updates are currently in the BROKEN state.\n\n"

CKP_MISSING_PKGS_TITLE_I "\
  Missing Filesets\n\
  ----------------\n"

CKP_MISSING_PKGS_HDR_I "\
  The following filesets could not be found on the installation media.\n\
  If you feel these filesets really are on the media, check for typographical\n\
  errors in the name specified or, if installing from directory, check for\n\
  discrepancies between the Table of Contents file (.toc) and the images that\n\
  reside in the directory.\n\n"

CKP_CONFL_VERSIONS_I "\
  Conflicting Versions of Filesets\n\
  --------------------------------\n\
  The following filesets are conflicting versions of filesets for which there\n\
  are multiple versions on the installation media.  Since a specific version\n\
  was not selected, the newest installable version has been selected.\n\n"

CKP_MSNG_SUPERSEDED_TITLE_I "\
  Missing Superseded Fileset Updates\n\
  ----------------------------------\n"

CKP_MSNG_SUPERSEDED_HDR_I "\
  The following selected fileset updates could not be found on the installation\n\
  media.  Newer updates which supersede those selected were detected on\n\
  the media and are indicated in parentheses.  You may explicitly select the\n\
  newer fileset updates for installation or use the \"auto-install\" facility\n\
  (i.e., \"AUTOMATICALLY install requisite software\" or -g flag), and the\n\
  newer updates will be installed instead of those selected.\n\n"

CKP_MSNG_SUPERSEDED_ROOT_HDR_I "\
  Missing Superseded Fileset Updates\n\
  ----------------------------------\n\
  The \"usr\" parts (i.e., \"server\" parts) of the following fileset updates\n\
  are not currently installed, and, therefore, the \"root\" parts (i.e.,\n\
  \"client\" parts) cannot be installed.  The \"usr\" parts of newer\n\
  updates which supersede those selected are installed.  You may\n\
  explicitly select installation of the \"root\" part of the newer\n\
  fileset updates, or you may use the \"auto-install\" facility (-g flag),\n\
  and the newer updates will be installed instead of those selected.\n\n"

CKP_MISSING_PKGS_CLIENT_I "\
  No ROOT Part to Install\n\
  -----------------------\n\
  The following selected filesets do not have a \"root\" part that can be\n\
  installed.  Either the fileset's corresponding \"usr\" part has yet to be\n\
  installed or the fileset does not have a \"root\" part.\n\n"

CKP_ROOT_PART_NOT_RQSTD_I "\
  Root Part Not Specified\n\
  -----------------------\n\
  The following contain both \"usr\" and \"root\" parts.  You only asked\n\
  to install the \"usr\" part.  You cannot install the \"usr\" part of a \n\
  fileset without its \"root\" part.\n\n"

CKP_NUTTIN_TO_DEINSTL_I "\
  Not Installed\n\
  -------------\n\
  No software could be found on the system that could be deinstalled for the\n\
  following requests:\n\n"

CKP_NUTTIN_TO_COMMIT_I "\
  Not Committable\n\
  ---------------\n\
  No software was found in the APPLIED state that could be committed for\n\
  the following requests:\n\n"
          
CKP_COMTD_NOT_REJCTBL_I "\
  Committed:  Not Rejectable\n\
  -------------------------\n\
  The following have been committed and cannot be rejected:\n\n"

CKP_NUTTIN_TO_REJECT_I "\
  Not Rejectable\n\
  --------------\n\
  No software could be found installed on the system that could be rejected\n\
  for the following requests:\n\n"

CKP_31_UPDT_REQ_FAILURE_I "\
  Different Base Level Installed\n\
  ------------------------------\n\
  The following selected filesets are updates to a base level which is at\n\
  a level different from that currently installed (or selected to be\n\
  installed during this installation).  Use software listing utilities\n\
  (lslpp command) for details of the installed base level.\n\n"

CKP_NO_DEINST_TITLE_I "\
  Non-Deinstallable Filesets\n\
  --------------------------\n"

CKP_NO_DEINST_BOSRTE_I "\
  You cannot deinstall bos.rte.  This is the base operating system.\n"
                           
CKP_SUPERSEDED_BY1_I "\
      (superseded by:  %s)\n"

CKP_SUPERSEDED_BY2_I "\
  %-49s (Superseded by %s)\n"
$ %-49s is the name of a fileset update (e.g., LPP1.obj 1.2.0.0)
$ %s is a PTF id (e.g., U412345)

CKP_NUTTIN_TO_DEINSTL_TRLR_I "\n\
  (The fileset may not be currently installed, or you may have made a\n\
   typographical error.)\n"

CKP_NUTTIN_TO_COMMIT_TRLR_I "\n\
  (Possible reasons for failure:  1. the selected software is already\n\
   committed, 2. the selected software is not installed, or 3. a typographical\n\
   error was made.)\n"

CKP_COMTD_NOT_REJCTBL_TRLR_I "\n\
  (The only way to remove a committed fileset or fileset update is\n\
   to deinstall the fileset.  Use the \"Remove Software Products\"\n\
   facility (-u flag) if this is desired.)\n"

CKP_NUTTIN_TO_REJECT_TRLR_I "\n\
  (Possible reasons for failure:  1. the selected software has been\n\
   committed, i.e., cannot be rejected, 2. the selected software is not\n\
   installed, or 3. a typographical error was made.)\n"

CKP_REQ_FAIL_KEY_HDR_I "\
  Requisite Failure Key:\n"

CKP_DEP_FAIL_KEY_HDR_I "\
  Dependent Failure Key:\n"
   
CKP_FAIL_KEY_HDR_I "\
  Failure Key:\n"

CKP_REQ_FAIL_APPLY_TITLE_I "\
  Requisite Failures\n\
  ------------------\n"

CKP_REQ_FAIL_APPLY_HDR_I "\
  SELECTED FILESETS:  The following is a list of filesets that you asked to\n\
  install.  They cannot be installed until all of their requisite filesets\n\
  are also installed.  See subsequent lists for details of requisites.\n"

CKP_REQ_FAIL_COMT_HDR_I "\
  Requisite Failures\n\
  (Commit Operation)\n\
  ------------------\n\
  SELECTED FILESETS:  The following is a list of filesets that you asked to\n\
  commit.  They cannot be committed until all of their requisite filesets\n\
  are also committed.  See subsequent lists for details of requisites.\n\n"

CKP_DEP_FAIL_DEINST_HDR_I "\
  Dependency Failures\n\
  (Deinstall Operation)\n\
  ---------------------\n\
  SELECTED FILESETS:  The following is a list of filesets that you asked to\n\
  remove.  They cannot be removed until all of their dependent filesets\n\
  are also removed.  See subsequent lists for details of dependents.\n\n"
         
CKP_DEP_FAIL_REJECT_HDR_I "\
  Dependency Failures\n\
  (Reject Operation)\n\
  -------------------\n\
  SELECTED FILESETS:  The following is a list of filesets that you asked to\n\
  reject.  They cannot be rejected until all of their dependent filesets\n\
  are also rejected.  See subsequent lists for details of dependents.\n\n"
       
CKP_SUCC_RQSTD_PKGS_HDR_I "\
  Selected Filesets\n\
  -----------------\n"

CKP_AUTO_SUPS_WARNING2_W "\
  (\"*\" indicates a newer update chosen automatically over a selected\n\
   fileset update)\n"

CKP_SUCC_REQUISITES_TITLE_I "\
  Requisites\n\
  ----------\n"
           
CKP_SUCC_REQS_APPLY_HDR_I "\
  (being installed automatically;  required by filesets listed above)\n"

CKP_SUCC_REQS_COMT_HDR_I "\
  (being committed automatically as requisites of selected filesets)\n"

CKP_SUCC_DEPS_TITLE_I "\
  Dependents\n\
  ----------\n"
            
CKP_SUCC_DEPS_DEINST_HDR_I "\
  (being deinstalled so that selected filesets may be removed; will not\n\
   function correctly when selected filesets are removed.)\n"
            
CKP_SUCC_DEPS_REJECT_HDR_I "\
  (being rejected so that selected filesets may be rejected;  will not\n\
   function correctly when selected filesets are removed.)\n"
   
CKP_SUCC_AUTO_IFREQS_HDR_I "\
  Requisites from Prior Installations\n\
  -----------------------------------\n\
  (being installed automatically to ensure system functions correctly)\n"
   
CKP_SUCC_AUTO_MLEV_TITLE_I "\
  Maintenance Level Fileset Updates\n\
  ---------------------------------\n"
         
CKP_SUCC_AUTO_MLEV_APPLY_HDR_I "\
  (being installed automatically to reflect level of system)\n"
         
CKP_SUCC_AUTO_MLEV_COMT_HDR_I "\
  (being committed automatically to preserve level of system)\n"

CKP_SUCC_AUTO_MAND_HDR_I "\
  Mandatory Fileset Updates\n\
  -------------------------\n\
  (being installed automatically due to their importance)\n"

CKP_SUCC_REQ_TO_BE_INST_I "\
      %s %s (TO BE INSTALLED)\n" 

CKP_SUCC_REQ_TO_BE_COMTD_I "\
      %s %s (TO BE COMMITTED)\n"

CKP_SUCC_REQ_INSTLD_I "\
      %s %s (INSTALLED)\n" 

CKP_SUCC_REQ_COMTD_I "\
      %s %s (COMMITTED)\n"

CKP_SUCCESS_KEY_HDR_I "\
  Success Key:\n"

CKP_SUCC_RQSTD_CODE_I "\
Selected"

CKP_SUCC_MAINT_CODE_I "\
Maintenance"

CKP_SUCC_MAND_CODE_I "\
Mandatory"

CKP_SUCC_P_REQ_CODE_I "\
P_Requisite"
$ The 'P_' part of this is an abbreviated code for "previously installed"

CKP_SUCC_SUPS_CODE_I "\
Supersedes"

CKP_SUCC_REQ_CODE_I "\
Requisite"

CKP_SUCC_UNK_CODE_I "\
Unknown"

CKP_SUCC_DEP_CODE_I "\
Dependent"

CKP_APPLY_SUCCESS_KEY_I "\
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
$ The 'P_' part of P_Requisite is an abbreviated code for "previously installed"

CKP_COMT_SUCCESS_KEY_I "\
  Selected    -- Explicitly selected by user to be committed.\n\
  Maintenance -- Maintenance Level fileset update; being committed\n\
                 automatically to preserve the ability to track the level\n\
                 of the system (prevents rejecting).\n\
  Requisite   -- Requisite of filesets being committed; requisites are always\n\
                 committed when \"auto-commit\" (-g flag) is specified.\n\n"

CKP_DEINST_SUCCESS_KEY_I "\
  Selected  -- Explicitly selected by user to be deinstalled.\n\
  Dependent -- Dependent of other filesets being deinstalled; dependents\n\
               are always deinstalled when \"auto-deinstall\" (-g flag) is\n\
               specified.\n\n"

CKP_REJECT_SUCCESS_KEY_I "\
  Selected  -- Explicitly selected by user to be rejected.\n\
  Dependent -- Dependent of other filesets being rejected; dependents are\n\
               always rejected when \"auto-reject\" (-g flag) is specified.\n\n"

CKP_SUCCESS_TITLE_I "\
SUCCESSES\n\
---------\n"

CKP_SUCCESS_APPLY_HDR_I "\
  Filesets listed in this section passed pre-installation verification\n\
  and will be installed.\n"

CKP_SUCCESS_APPLY_HDR_GTR_V0_I "\
    -- Filesets are listed in the order in which they will be installed.\n\
    -- The reason for installing each fileset is indicated with a keyword\n\
       in parentheses and explained by a \"Success Key\" following this list.\n"

CKP_SUCCESS_APPLY_HDR_GTR_V1_I "\
    -- If a fileset has requisites they are listed (indented)\n\
       beneath the fileset.\n"

CKP_SUCCESS_COMT_HDR_I "\
  Filesets listed in this section passed pre-commit verification\n\
  and will be committed.\n"

CKP_SUCCESS_COMT_HDR_GTR_V0_I "\
    -- Filesets are listed in the order in which they will be committed.\n\
    -- The reason for committing each fileset is indicated with a keyword\n\
       in parentheses and explained by a \"Success Key\" following this list.\n"

CKP_SUCCESS_COMT_HDR_GTR_V1_I "\
    -- If a fileset has requisites they are listed (indented)\n\
       beneath the fileset.\n"

CKP_SUCCESS_DEINST_HDR_I "\
  Filesets listed in this section passed pre-deinstall verification\n\
  and will be removed.\n"

CKP_SUCCESS_DEINST_HDR_GTR_V0_I "\
    -- Filesets are listed in the order in which they will be removed.\n\
    -- The reason for deinstalling each fileset is indicated with a keyword\n\
       in parentheses and explained by a \"Success Key\" following this list.\n"

CKP_SUCCESS_REJECT_HDR_I "\
  Filesets listed in this section passed pre-reject verification\n\
  and will be rejected.\n"

CKP_SUCCESS_REJECT_HDR_GTR_V0_I "\
    -- Filesets are listed in the order in which they will be rejected.\n\
    -- The reason for rejecting each fileset is indicated with a keyword\n\
       in parentheses and explained by a \"Success Key\" following this list.\n"

CKP_SUCCESS_REMOVE_HDR_GTR_V1_I "\
    -- If a fileset has dependents they are listed (indented)\n\
       beneath the fileset.\n"
  
CKP_REQUIRED_PKGS_I "\
  REQUIRED FILESETS:  The fileset under test cannot be installed without\n\
  the following filesets.  (These filesets are not currently installed.)\n\n"

CKP_MISS_REQ_APPLY_I "\
  MISSING REQUISITES:  The following filesets are required by one or more\n\
  of the selected filesets listed above.  They are not currently installed\n\
  and could not be found on the installation media.\n"

CKP_MISS_REQ_COMT_I "\
  MISSING REQUISITES:  The following filesets are requisites of one or more\n\
  of the selected filesets listed above.  They are not currently installed\n\
  on the system.  You should install these requisites to ensure that the\n\
  selected filesets function correctly.  You MUST install these requisites\n\
  before committing the selected filesets.\n"
    
CKP_COMTD_DEPENDENTS_I "\
  COMMITTED DEPENDENTS:  The following filesets depend upon one or more of\n\
  the selected filesets listed above.  These dependents must be rejected\n\
  before the selected filesets.  Since these dependents are in a COMMITTED\n\
  state, they cannot be rejected.  Therefore, the selected filesets cannot\n\
  be rejected.  (Committed software can be removed with the deinstall\n\
  facility, i.e., \"Remove Software Products\" or -u flag.  NOTE:  This\n\
  will remove an entire fileset.)\n"

CKP_REF_TO_DEP_LIST1_I "\
  (Selected filesets which depend upon these requisites are referenced in\n\
   parentheses.)\n\n"

CKP_REF_TO_DEP_LIST2_I "\
  (Selected filesets upon which these depend are referenced in parentheses.)\n\n"

CKP_GROUP_FAIL_HDR1_I "\
  GROUP REQUISITE FAILURES:  The fileset under test cannot be installed\n\
  until the following group requisite is satisfied.  A group requisite must\n\
  pass a specified number of requisite tests.  See the \"Requisite Failure Key\"\n\
  below for details of group member failures.\n\n"

CKP_GROUP_FAIL_HDR2_I "\
  GROUP REQUISITES:  The dependencies of one or more of the selected filesets\n\
  listed above are defined by a group requisite.  A group requisite must pass\n\
  a specified number of requisite tests.  The following describe group\n\
  requisite failures for filesets that you selected.  (See the \"Requisite\n\
  Failure Key\" below for details of group member failures.)\n\n"

CKP_AVLBL_REQS_HDR1_I "\
  AVAILABLE REQUISITES:  You specified that requisite software should be\n\
  automatically installed.  The following filesets are requisites of one\n\
  or more of the selected filesets listed above.  These requisites would\n\
  have been installed automatically had the selected filesets passed all\n\
  requisite checks.\n"

CKP_AVLBL_REQS_HDR2_I "\
  AVAILABLE REQUISITES:  The following filesets are requisites of one or\n\
  more of the selected filesets listed above.  They are available on\n\
  the installation media.  To install these requisites with the selected\n\
  filesets, specify the option to automatically install requisite\n\
  software (-g flag).\n"

CKP_APPLD_REQS_HDR1_I "\
  APPLIED REQUISITES:  You specified that requisite software should be\n\
  automatically committed.  The following filesets are APPLIED requisites\n\
  of one or more of the selected filesets listed above.  These filesets\n\
  would have been committed automatically had the selected filesets passed\n\
  all requisite checks.\n"

CKP_APPLD_REQS_HDR2_I "\
  APPLIED REQUISITES:  The following filesets are requisites of one or more\n\
  of the selected filesets listed above.  These must be committed before\n\
  or with the filesets that you selected.  To commit these requisites with\n\
  the selected filesets, specify the option to automatically commit requisite\n\
  software (-g flag).\n"

CKP_APPLD_DEPS_HDR1_I "\
  APPLIED DEPENDENTS:  You specified that dependent fileset updates should\n\
  be automatically rejected.  The following updates are dependents of one or\n\
  more of the selected filesets listed above.  These filesets would have\n\
  been rejected automatically had the selected filesets passed all dependency\n\
  checks.\n"
         
CKP_APPLD_DEPS_HDR2_I "\
  APPLIED DEPENDENTS:  The following fileset updates are dependents of one\n\
  or more of the selected filesets listed above.  These dependents must be\n\
  rejected before or with the updates that you selected.  To reject these\n\
  dependents with the selected filesets, specify the option to automatically\n\
  reject dependent updates (-g flag).\n"

CKP_INSTLD_DEPS_HDR1_I "\
  INSTALLED DEPENDENTS:  You specified that dependent software should be\n\
  automatically removed.  The following filesets are dependents of one or\n\
  more of the selected filesets listed above.  These dependents would have\n\
  been removed automatically had the selected filesets passed all dependency\n\
  checks.\n"

CKP_INSTLD_DEPS_HDR2_I "\
  INSTALLED DEPENDENTS:  The following filesets are dependents of one or more\n\
  of the selected filesets listed above.  These must be removed before\n\
  or with the filesets that you selected.  To remove these dependents with\n\
  the selected filesets, specify the option to automatically remove dependent\n\
  software (-g flag).\n"
                  
CKP_DEINST_DEPS_VIA_UPDTS_W "\
  WARNING:  The fileset updates listed below have created dependencies from\n\
  their filesets to one or more of the filesets that you are trying to\n\
  remove.  If you choose to automatically remove dependent software, the\n\
  entire dependent fileset will be removed (not just the update).  You\n\
  may be able to reject the update to remove the dependency.\n"

$ ----
$ In the following 4 messages, %d is the number of failing filesets to which 
$ the message refers.
$ ----
CKP_AVLBL_REQ_MSG_I "\
  AVAILABLE REQUISITES:  You specified that requisite software should be\n\
  automatically installed.  Additional filesets (%d) would have been\n\
  installed automatically had the selected filesets passed all requisite\n\
  checks.\n\n"

CKP_APPLD_REQ_MSG_I "\
  APPLIED REQUISITES:  You specified that requisite software should be\n\
  automatically committed.  Additional filesets (%d) would have been\n\
  committed automatically had the selected filesets passed all requisite\n\
  checks.\n\n"

CKP_INSTLD_DEP_MSG_I "\
  INSTALLED DEPENDENTS:  You specified that dependent software should be\n\
  automatically removed.  Additional filesets (%d) would have been\n\
  removed automatically had the selected filesets passed all dependency\n\
  checks.\n\n"
               
CKP_APPLD_DEP_MSG_I "\
  APPLIED DEPENDENTS:  You specified that dependent software should be\n\
  automatically rejected.  Additional filesets (%d) would have been\n\
  rejected automatically had the selected filesets passed all dependency\n\
  checks.\n\n"

CKP_CMD_LINE_MISC_FAILING_REQ_HDR_I "\
  MISCELLANEOUS FAILING REQUISITES:  The fileset under test cannot be\n\
  installed without the following filesets.  These filesets have various\n\
  problems associated with them.  See the \"Requisite Failure Key\" for\n\
  failure reasons and possible recovery hints.\n"

CKP_MISC_FAILING_REQ_APPLY_HDR_I "\
  MISCELLANEOUS FAILING REQUISITES:  The following filesets are requisites\n\
  of one or more of the selected filesets listed above.  Various problems\n\
  associated with these requisites are preventing the selected filesets\n\
  from installing.  See the \"Requisite Failure Key\" for failure reasons and\n\
  possible recovery hints.\n"

CKP_MISC_FAILING_REQ_COMT_HDR_I "\
  MISCELLANEOUS FAILING REQUISITES:  The following filesets are requisites\n\
  of one or more of the selected filesets listed above.  Various problems\n\
  associated with these requisites are preventing the selected filesets from\n\
  being committed.  See the \"Requisite Failure Key\" for failure reasons and\n\
  possible recovery hints.\n"

CKP_MISC_FAILING_REQ_REMOVE_HDR_I "\
  MISCELLANEOUS FAILING DEPENDENTS:  The following filesets depend upon one or\n\
  more of the selected filesets listed above.  As a result, these dependents\n\
  must be removed before or with the selected filesets.  Various problems\n\
  associated with these dependents are preventing the selected filesets from\n\
  being removed.  See the \"Dependent Failure Key\" for failure reasons and\n\
  possible recovery hints.\n"

CKP_WARNING_TITLE_I "\
WARNINGS\n\
--------\n"

CKP_WARNING_HDR_I "\
  Problems described in this section are not likely to be the source of any\n\
  immediate or serious failures, but further actions may be necessary or\n\
  desired.\n\n"

CKP_CYCLE_HDR_I "\
\tFilesets involved in the cycle:\n\n"

CKP_VERIFYING_REQ_I "\
Verifying requisites..."

CKP_REQ_CODES_I "\
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
$ Do not translate prereq, coreq, instreq, ifreq, or the alphanumeric 
$ codes 'p','c','I','i', 'oi', 'a','pa','c','pc','b'

CKP_REQ_CODES_APPLY_ONLY_I "\
      o -- on the installation media, not installed\n\
      m -- \"missing:\" not installed, not on installation media\n"
$ Do not translate the alphanumeric codes 'o' or 'm'

CKP_NEEDS_I "\
   #%d needs:\n"
$ %d is the number in a list of filesets of a fileset that requires other
$ filesets that need to be installed.  The list which follows this message 
$ is the list of "other" filesets.

CKP_SUPERSEDES_I "\
        (supersedes:  %s"

CKP_DEPENDENTS_COLON_I "\
     Dependents:\n"

CKP_REQUISITES_COLON_I "\
     Requisites:\n"

CKP_DEP_ABBREV_I "\
         (dep #s:  "
$ dep is an abbreviation for the word dependent -- please limit translated
$ abbreviation to 3 characters if possible. 

CKP_SUCC_UPDATES_I "\
  %s %d.%d.%d.%d updates:\n"
$ %s is a fileset name, the %d's are version, release, modification, fix:
$ example "LPP1.obj 1.2.0.0 updates:"

CKP_SUPERSEDED_UPDATES_HDR_I "\
  Superseded Fileset Updates\n\
  --------------------------\n\
  Fileset updates listed in this section will not be installed.  Newer\n\
  updates which supersede (replace) these were selected instead (either by\n\
  you or automatically by the installation program).  Make sure that the\n\
  superseding updates listed passed pre-installation verification.\n\n"

CKP_PROD_AT_LEV_REQRS_I "\n\
Fileset %s %s requires:\n"

CKP_AT_LEAST_FOLLOWNG_I "\
%s   At least %d of the following:\n"

CKP_INSTP_FAILED_REQ_I "\
\"*\" indicates a requisite fileset that probably failed to install\n\
   during this installation session.\n\n"

$ ----
$ %c in the following messages is an alphanumeric character (examples:  *, !, ~,
$ $, #, ....)
$ ----
CKP_NO_REMOVE_USR_B4_ROOT_I "\
  \"%c\" fileset has both \"usr\" and \"root\" parts installed.  Both parts\n\
      were not selected for this invocation of installp.  The \"usr\" part\n\
      cannot be removed separately when a fileset has both parts installed.\n"

CKP_NO_REAPPLY_INSTLD_ROOT_I "\
  \"%c\" fileset is applied on the \"root\" part but not on the \"usr\" part.\n\
      Before attempting to re-apply this fileset you must remove its \"root\"\n\
      part.  (Use the reject facility if the fileset is an update.  Remove\n\
      the fileset via the deinstall facility if it is a base level fileset.)\n"

CKP_NO_COMT_ROOT_B4_USR_I "\
  \"%c\" fileset contains both \"usr\" and \"root\" parts.  The \"usr\" part\n\
      is still in the applied state.  A fileset's \"root\" part cannot be\n\
      committed before its \"usr\" part.\n"

CKP_COMMITTED_ROOT_ONLY_I "\
  \"%c\" fileset is committed on the \"root\" part but not installed on\n\
      the \"usr\" part.  Deinstall the fileset (using -u flag) or \"force\n\
      re-install\" the fileset (using -F flag).\n"

CKP_NO_REJ_COMMTD_PKG_I "\
  \"%c\" fileset is in the committed state.  A committed fileset cannot be\n\
      rejected.  (Committed software can be removed with the deinstall\n\
      facility, i.e., \"Remove Software Products\" or -u flag.  NOTE:  This\n\
      will remove an entire fileset.)\n"

CKP_NO_COMT_PARTLY_APPLD_I "\
  \"%c\" partially applied fileset (i.e., the \"usr\" and \"root\" parts are\n\
      not both applied).  A fileset must be fully applied before it can be\n\
      committed.\n"

CKP_RECOVER_PARTLY_INSTLD_I "\
      Use software listing facilities (lslpp command) to determine which\n\
      parts are missing.  If the \"usr\" part is missing, reject (or remove/\n\
      deinstall) the fileset so that it may be re-installed.  If the \"root\"\n\
      part is missing, request that the fileset be installed again, and the\n\
      \"root\" part will be installed.\n"

CKP_NON_UNIQUE_VRMF_PRQ_I "\
  \"%c\" base level fileset for which there is another version that is either\n\
      already installed or that was selected to be installed during this\n\
      installation session.\n"

CKP_SHARE_PART_NOT_RQSTD_I "\
  \"%c\" fileset contains a \"share\" part.  (\"share\" parts were not\n\
      selected for this invocation of installp.)\n"

CKP_SUPD_MUST_APP_FOR_CONSIST_I "\
  \"%c\" superseded fileset that is applied on the \"usr\" part which must\n\
      also be applied on the \"root\" part for consistency.  Select this\n\
      fileset explicitly or use the option to automatically include\n\
      requisite software (-g flag).\n"

CKP_REQ_AVAIL_OTHER_PROBLEMS_I "\
  \"%c\" fileset is available on the installation media but is not currently\n\
      installed.  This fileset is failing to install because the requirements\n\
      of the group requisite, of which the fileset is a member, cannot be\n\
      met or because the fileset has requisites of its own which are causing\n\
      problems.  Request this fileset for installation by itself to determine\n\
      the actual reason.  (Use the \"Preview\" option (-p flag) to prevent\n\
      the installation from actually occurring.)\n"

CKP_UNCLEAR_FAILURE_I "\
  \"%c\" unclear why this fileset failed.  Use the software listing (lslpp\n\
      command) and software verification facilities (lppchk command) to try\n\
      to establish further information on this fileset.\n"

CKP_UPDATES_ONLY_I "\
  \"%c\" this is the base level for a fileset.  You asked to install\n\
      updates only (with the -B flag).  The base level fileset will not\n\
      be automatically installed.\n"

CKP_NO_REJ_PRTLY_COMMTD_I "\
  \"%c\" fileset is in a partially committed state (i.e., the \"usr\" or\n\
      \"root\" part is committed).  A committed fileset cannot be rejected.\n\
      (Committed software can be removed with the deinstall facility, i.e.,\n\
      \"Remove Software Products\" or -u flag.  NOTE:  This will remove an\n\
      entire fileset.)\n"

CKP_BROKEN_PKG_I "\
  \"%c\" fileset is installed but in the BROKEN state.\n"

CKP_REQS_AVLBL_NO_G_I "\
  \"%c\" fileset is available on the installation media but not currently\n\
      installed.  Explicitly request that the fileset be installed, or use\n\
      the option to automatically install requisite software (-g flag).\n"

CKP_REQS_NOT_AVAILABL_I "\
  \"%c\" fileset is not installed and not available on the current\n\
      installation media.\n"

CKP_REQS_PARTLY_APPLD_I "\
  \"%c\" partially installed fileset (i.e., the \"usr\" and \"root\" parts\n\
      are not both installed).\n"

CKP_REQS_PARTLY_COMTD_I "\
  \"%c\" fileset is in a partially committed state (i.e., the \"usr\" and\n\
      \"root\" parts are not both committed but must be to satisfy the\n\
      requisite check).\n"

CKP_USR_PART_NOT_RQSTD_I "\
  \"%c\" fileset contains a \"usr\" part.  (\"usr\" parts were not selected\n\
      for this invocation of installp.)\n"

CKP_USR_ROOT_PARTS_NOT_RQSTD_I "\
  \"%c\" fileset contains both \"usr\" and \"root\" parts.  Both parts were\n\
      not selected.  (If installing this fileset on a client, the \"usr\"\n\
      (server) part is more than likely not yet installed.)\n"

CKP_BROKEN_SUPS_I "\
  \"%c\" fileset update supersedes a BROKEN update.  In order to install\n\
      this fileset or to fix the BROKEN update, you must specify options to\n\
      the installation commands that will apply, commit, and automatically\n\
      include requisite software without saving replaced files (-acgN flags).\n"

CKP_NO_PRQ_BASE_I "\
  \"%c\" fileset update does not prereq its base level fileset.  An update\n\
      must prereq its base level fileset directly or indirectly (by prereqing\n\
      another update which prereqs the base fileset).\n"

CKP_INSTREQ_FAILURE_I "\
  \"%c\" fileset must be installed prior to or at the same time as its\n\
      dependent filesets.  This requisite cannot be installed automatically\n\
      (using -g option).  It must be explicitly selected for installation.\n"

CKP_PKG_NOT_INSTLD_I "\
  \"%c\" currently not installed on the system.\n"

CKP_BROKEN_REQUISITES_I "\
  BROKEN REQUISITES:  The following filesets are requisites of one or more\n\
  of the selected filesets listed above.  They are currently installed\n\
  on the system but are in the BROKEN state.\n"

CKP_RECOVER_BROKEN_I "\
  Alternative actions that can be used to correct broken filesets are:\n\
  1. If the broken fileset is an update, re-install the update with options\n\
     to the installation program that will apply, commit and automatically\n\
     install requisite software without saving replaced files:  -acgN flags.\n\
  2. Re-install the entire fileset, i.e., the base level and any desired\n\
     updates (may need to use the \"FORCE\" option: -F flag).\n\
  3. Deinstall the entire fileset (using \"Remove Software Products\":\n\
     -u flag).\n"

CKP_NON_DEINST_PKG_I "\
  The following filesets were selected for deinstallation.  Deinstallability\n\
  checks indicate that they should not be removed from the system.\n\n"

CKP_NON_DEINST_DEP_I "\
  NON-DEINSTALLABLE DEPENDENTS:  The following filesets depend upon one or\n\
  more of the selected filesets listed above.  These dependents should\n\
  be removed before the selected filesets.  Deinstallability checks indicate\n\
  that these dependents should not be removed from the system; therefore, the\n\
  selected filesets cannot be removed.\n"

CKP_CONFL_BASE_LEV_I "\
  CONFLICTING REQUISITES:  The following filesets are required by one or\n\
  more of the selected filesets listed above.  There are other versions of\n\
  these filesets which are already installed (or which were selected to be\n\
  installed during this installation session).  A base level fileset cannot\n\
  be installed automatically as another fileset's requisite when a different\n\
  version of the requisite is already installed.  You must explicitly select\n\
  the base level requisite for installation.\n"

CKP_NOTHING_TO_COMMIT_I "\
  Nothing to Commit\n\
  -----------------\n\
  There is nothing in the APPLIED state that needs to be committed.\n\n"

CKP_NOTHING_TO_APPLY_I "\
  Nothing to Install\n\
  ------------------\n\
  There are no fileset \"root\" parts (i.e., client specific software)\n\
  which need to be installed at this time.\n\n"

CKP_CANT_FORCE_APPLY_PTF_I "\
  Force Apply Failures\n\
  --------------------\n\
  The following is a list of fileset updates.  Updates cannot be specified\n\
  from the command line when the force flag (-F) is used in combination\n\
  with the apply flag (-a).\n\n"

CKP_WILL_PULL_IN_NEWER_I "\
  NOTE:  There is a newer version of one or more of these requisite filesets\n\
	 on the installation media.  If you choose to automatically install\n\
	 these requisites, the newer version will be selected instead.\n\
	 Do not use the auto-install option (-g flag) if you want the older\n\
	 version to be installed.\n"

CKP_DEINST_3_1_I "\
  3.1 Formatted Filesets: \n\
  ----------------------- \n\
  The following is a list of 3.1 formatted filesets.  Deinstallation of \n\
  3.1 formatted filesets may not be complete.\n\n"

CKP_DEINST_3_2_I "\
  Filesets installed on 3.2: \n\
  -------------------------- \n\
  The following filesets were originally installed on an AIX 3.2 system \n\
  and, therefore, may not be completely deinstalled.\n\n"

CKP_NO_DEINST_COMPAT_I "\
  Compatibility Entries:  Not Deinstallable\n\
  -----------------------------------------\n\
  The following filesets are compatibility entries used by installp and,\n\
  therefore, cannot be deinstalled.\n\n"

CKP_NO_DEINST_MIGRATING_I "\
  Partially Migrated Filesets:  Not Deinstallable\n\
  -----------------------------------------------\n\
  The following filesets were originally installed on an AIX 3.2 system. \n\
  They have only been partially migrated from AIX 3.2 to AIX 4.1 and, \n\
  therefore, cannot be deinstalled.\n\n"

CKP_SELECTED_SUPS_BROKEN_I "\
  Supersedes of BROKEN Filesets \n\
  ----------------------------- \n\
  The following selected fileset updates supersede others that are installed\n\
  but in the BROKEN state.  In order to install the superseding fileset, use\n\
  options to the installation program that will apply, commit and\n\
  automatically install requisite software without saving replaced files:\n\
  -acgN flags.\n\n"

CKP_MIGRATING_REQ_I "\
  \"%c\" fileset is partially migrated from an earlier installation.\n\
      A partially migrated fileset cannot be updated.\n"

CKP_MIGRATING_DEP_I "\
  \"%c\" fileset is partially migrated from an earlier installation.\n\
      A partially migrated fileset cannot be deinstalled.  (Try again\n\
      when the fileset has been fully migrated.)\n"

CKP_LPPCHK_INCONSIST_FAILURE_HDR_I "\
%s:  The following filesets need to be installed or corrected to bring\n\
	 the system to a consistent state:\n\n"

CKP_LPPCHK_DEP_FAILURE_HDR_I "\
%s:  The following describes dependencies from installed filesets\n\
	 on one or more of the filesets listed above:\n"

CKP_LPPCHK_NOT_INSTALLED_REQ_FILESET_I "\
not installed; requisite fileset"

CKP_LPPCHK_NOT_INSTALLED_I "\
not installed"

CKP_RENAMED_SUPD_PKG_I "\
  \"%c\" older version of a fileset that has been renamed.  The newer,\n\
      renamed fileset, which supersedes this one, is already installed (or\n\
      was selected to be installed during this installation).  A fileset\n\
      which has undergone a name change cannot be re-installed over the newly\n\
      named fileset.  The newer fileset must first be removed.\n"

CKP_NO_MULT_BARR_E "\n\
0503-255 %s:  Error:  A fileset cannot reference its own name more than\n\
\tonce in its supersedes data.  The fileset in error is:\n\t%s\n\n"

CKP_NO_COMPAT_MATCH_E "\n\
0503-256 %s:  Error:  A compatibility entry in the supersedes data of\n\
\ta fileset should have the same name but not the same level as the\n\
\tcompatible fileset.  The level should be greater than all base levels\n\
\tand updates released for the compatible (superseded) fileset.  The\n\
\tfileset in error is:\n\t%s\n\n"

CKP_NO_SUPPORT_MANY_COMPAT_E "\n\
0503-257 %s:  Error:  Multiple compatibilty entries in the supersedes\n\
\tdata of a fileset are not currently supported.  A fileset may only have\n\
\tone name different than its own in its supersedes data.\n\
\tThe fileset in error is:\n\t%s\n\n"

CKP_DOTTED_NUM_MUST_HAVE_4_PTS_E "\n\
0503-258 %s:  Error:  A dotted number in a requisite expression must\n\
\tcontain four parts to specify version, release, modification and fix\n\
\tnumber.  The fileset in error is:\n\
\t%s\n\n"

CKP_ALRDY_INSTLD_NOTE_I "\
  NOTE:  Base level filesets may be reinstalled using the \"Force\"\n\
  option (-F flag), or they may be removed, using the deinstall or\n\
  \"Remove Software Products\" facility (-u flag), and then reinstalled.\n\n"

CKP_INT_ERR_DISABLING_PKGS_E "\n\
0503-259 %s:  Internal error.  Unable to resolve conflicts between\n\
\tbase level filesets which are already installed and/or which are on\n\
\tthe installation media.  Use local problem reporting procedures.\n\
\t(NOTE:  This error has not affected the current installation\n\
\tif the two filesets listed here do not appear in subsequent failure\n\
\tmessages AND if there are no references to \"disabled\" filesests\n\
\tin subsequent failure messages.)\n\
\tfileset1: %s %s\n\
\tfileset2: %s %s\n\n"

CKP_DISABLING_INT_ERR_I "\
  \"%c\" fileset was disabled for the current installation due to an internal\n\
      error which occurred.  Use local problem reporting procedures.\n"

CKP_CONFL_BASE_LEV_NOTE_I "\
  NOTE:  A requisite listed below supersedes (ie. replaces) the fileset with\n\
  which it is in conflict but the two have different names.  The installed\n\
  fileset is displayed beneath the conflicting fileset.\n\n"

CKP_CONFL_WITH_I "\
      (Conflicts with: %s)\n"

CKP_DEINST_MIG_I "\
  Partially Migrated Filesets: \n\
  ---------------------------- \n\
  The following filesets were originally installed on an AIX 3.2 system \n\
  and have only been partially migrated from AIX 3.2 to AIX 4.1. \n\
  Deinstallation of these filesets is allowed, however, no unconfiguration\n\
  will be performed. \n\n"

CKP_OEM_MISMATCH_ERR_I "\
  \"%c\" Operating system mismatch.  Fileset or one of its requisites is\n\
      designated to be applied only on a system supplied by a different\n\
      system manufacturer.\n"

CKP_OEM_SPECIFIC_ERR_I "\
  \"%c\"  Generic fileset update cannot be applied onto a fileset which has\n\
      been modified or would be modified during this installation to operate\n\
      only on a system supplied by this specific system manufacturer.\n"

CKP_OEM_BASELEVEL_ERR_I "\
  \"%c\"  Generic fileset cannot be applied over the installed version of\n\
      the fileset which has been modified to operate only on a system\n\
      supplied by this specific system manufacturer unless the \"FORCE\"\n\
      option (-F) is specified.\n"

$ Dashed lines in message should be same length as first line
CKP_OEM_BASELEVEL_HDR_I "\
  Filesets Already Modified by System Manufacturer\n\
  ------------------------------------------------\n\
  The following generic filesets can not be applied because the\n\
  installed filesets have been modified by the system manufacturer\n\
  to be specific to this system.  The \"FORCE\" option (-F) must be\n\
  used in order to overwrite the modified version of the fileset.\n\n"

CKP_OEM_MISMATCH_HDR_I "\
  Filesets Specific to Different System Manufacturer\n\
  --------------------------------------------------\n\
  The following filesets can not be applied because this system\n\
  was manufactured by a different system manufacturer than that required by\n\
  the filesets or the requisites of the filesets.\n\n"

$ Dashed lines in message should be same length as first line
CKP_OEM_REPLACED_HDR_I "\
  Filesets Already Modified by System Manufacturer\n\
  ------------------------------------------------\n\
  The following generic fileset updates can not be applied because the\n\
  installed filesets have been modified or have requisites which have been\n\
  modified by the system manufacturer to be specific to this system.\n\n"


$ --------------------------------------------------------
$set MS_INUCOMMON
$ These are the messages that are common between commands.
$ Message numbers 0503-000 through 0503-100.
$ --------------------------------------------------------

CMN_ADDVPD_E "\
0503-000 %s:  Unable to add an entry to the Software Vital \n\
\tProduct Data database.\n"
$ %s is the name of the command.

CMN_ARFAIL_E "\
0503-001 %s:  The archive of a constituent member into \n\
\t file %s failed.\n"
$ The first %s is the name of the command.
$ The second %s is the name of the archive file.

CMN_BADMV_E "\
0503-002 %s:  File %s\n\
\tcould not be moved to %s.\n"
$ The first %s is the name of the command.
$ The second %s is the name of the file to be moved.
$ The third %s is the file to move to.

CMN_BAD_DEV_E "\
0503-003 %s:  The specified device %s\n\
\tis not a valid device or file.\n"
$ The first %s is the name of the command.
$ The second %s is the name of the installation device.

CMN_BAD_FL_FORMAT_E "\
0503-004 %s:  File %s is not in the correct format.\n"
$ The first %s is the name of the command.
$ The second %s is the name of the file.

CMN_BAD_TOC_E "\
0503-005 %s:  The format of the toc file is invalid.\n"
$ %s is the name of the command.

CMN_BD_CNTFLD_W "\
%s:  Incorrect content field of \'%c\' in the file lpp_name for \n\
\tsoftware package \'%s\'. Assuming \'%c\' and continuing.\n"

CMN_CANT_CHDIR_E "\
0503-006 %s:  Cannot change to directory %s.\n\
\tCheck path name and permissions.\n"
$ The first %s is the name of the command.
$ The second %s is the path name of the directory.

CMN_CANT_CREATE_E "\
0503-007 %s:  Cannot create file %s.\n\
\tCheck path name and permissions.\n"
$ The first %s is the name of the command.
$ The second %s is a file path name.

CMN_CANT_EXPAND_E "\
0503-008 %s:  There is not enough free disk space in filesystem\n\
\t%s (%d more 512-byte blocks are required).\n\
\tAn attempt to extend this filesystem was unsuccessful.\n\
\tMake more space available then retry this operation.\n\n"
$ The first %s is the name of the command.
$ The second %s is the name of the file system.
$ %d is the number of blocks needed.

CMN_CANT_OPEN_E "\
0503-009 %s:  Cannot access or open file %s. \n\
\tCheck path name and permissions.\n"
$ The first %s is the name of the command.
$ The second %s is a file name.

CMN_CPFAIL_E "\
0503-010 %s:  The copy of file %s\n\
\tto %s failed.\n"
$ The first %s is the name of the command.
$ The second %s is the name of the file being copied from.
$ The third %s is the name of the file being copied to.

CMN_CREATE_TOC_E "\
0503-011 %s:  An error occurred while attempting to build the \n\
\ttable of contents file in the %s directory.\n"
$ %s is the command that received the error.

CMN_DEV_INFORM_CORRECTION_I "\
%s:  Using device %s instead of %s\n"

CMN_DEV_INFORM_WAIT_I "\
%s:  Obtaining device information, please wait.\n"

CMN_EMPTY_PREREQ_FILE_E "\
0503-012 %s:  The prereq file is empty for the fileset\n\
\tmaintenance level %s.\n"
$ The second %s is the name of a fileset.

CMN_ERR_FUNCCALL_E "\
0503-013 %s:  %s function call failed.\n"
$ The first %s is the name of the command.
$ The second %s is the name of the system call.

CMN_ERR_SYSCALL_E "\
0503-014 %s:  %s system call failed.\n"
$ The first %s is the name of the command.
$ The second %s is the name of the system call.

CMN_INSRT_VOL_I "\
%s:  Please mount volume %d on %s. \n\
\tPress the enter key to continue.\n"
$ The first %s is the name of the command.
$ %d is the volume number of the installation media.
$ The second %s is the name of the installation media device.

CMN_INTERNAL_E "\
0503-015 %s:  An internal error has occurred. \n\
\tUse local problem reporting procedures.\n"
$ %s is the name of the command.

CMN_INVALID_OPT_NAME_E "\
0503-016 %s:  The fileset name %s does not\n\
\tbegin with its package name of %s.\n"

CMN_INV_DSKT_E "\
0503-017 %s:  Please mount volume %d and restart the command.\n"

CMN_IVPDERR_E "\
0503-018 %s:  An internal error occurred while attempting \n\
\tto access the Software Vital Product Data.\n\
\tUse local problem reporting procedures.\n"

CMN_LEVEL_NUM_E "\
0503-019 %s:  %s is an invalid level. \n\
\tALL levels must be in one of the following formats:  \n\
\t\tvv.rr.mmmm.ffff OR vv.rr.mmmm.fffff.ppppppppp\n"
$ The first %s is the name of the command.
$ The second %s is the level number.

CMN_MALLOC_FAILED_E "\
0503-020 %s:  Received an error while trying to allocate space \n\
\tusing malloc. Use local problem reporting procedures.\n"
$ %s is the name of the command.

CMN_MISS_PROD_NAME_E "\
0503-021 %s:  The package name field required in a 4.1-formatted package\n\
\tis missing from the lpp_name file.\n"

CMN_MIXT_LEVELS_E "\
0503-022 %s:  Installable filesets %s and\n\
\t%s have different levels.\n \
\tAll filesets within the same package must have identical\n\
\tlevels (version, release, modification, and fix fields).  (For\n\
\t3.2-formatted update packages, the PTF ID must also be identical.)\n"

CMN_MSG_SIZE_FILE_E "\
0503-023 %s:  The package containing installable fileset\n\
\t%s does not have a size file for the fileset.\n\
\tThe size file is required for 4.1-formatted packages.\n"

CMN_NOSORT_E "\
0503-024 %s:  The %s file could not be sorted.\n"
$ The first %s is the name of the command.
$ The second %s is the name of the file.

CMN_NOSPACE_E "\
0503-025 %s:  There is not enough free disk space in filesystem\n\
\t%s (%d more 512-byte blocks are required).\n\
\tMake more disk space available or use the option to automatically\n\
\textend filesystems (-X flag).\n\n"
$ The first %s is the name of the command.
$ The second %s is the name of a file system.
$ %d is the number of blocks needed.
$ Do not translate -X.

CMN_NOSVF_E "\
0503-026 %s:  File %s, specified in the list of saved files,\n\
\tdoes not exist and could not be recovered.\n"
$ The first %s is the name of the command.
$ The second %s is the name of a saved file.

CMN_NOT_DIR_E "\
0503-027 %s:  The %s option requires a directory.\n\
\t%s is a file.\n"
$ The first %s is the command that received the error.
$ The second %s is the option to that command that expected a directory input.
$ The third %s is the file name given by the user.

CMN_NOT_ENOUGH_W "\
%s:  Warning. Continuing even though there may not be enough\n\
\tspace to complete the operation.\n"
$ %s is the name of the command.

CMN_NO_MK_DIR_E "\
0503-028 %s:  Cannot create the directory %s.\n\
\tCheck path name and permissions.\n"
$ The first %s is the name of the command.
$ The second %s is the name of the directory.

CMN_NO_MK_TMPDIR_E "\
0503-029 %s:  Cannot create a directory in the /tmp directory.\n\
\tCheck permissions or available space.\n"
$ The first %s is the name of the command.

CMN_NO_SPACE_E "\
0503-030 %s:  There is not enough space on the system to continue.\n"
$ %s is the name of the command.

CMN_ODMDIR_W "\
%s:  Warning. The directory for the Object Data Manager object \n\
\trepository was not specified.\n"
$ %s is the name of the command.

CMN_PGSPACE_MEG_E "\
0503-031 %s:  The page space could not be increased by %d megabytes.\n"
$ %s is the name of the command.
$ %d is the number of megabytes needed.

CMN_READONLY_FS_E "\
0503-032 %s:  The file system %s is mounted read only.\n"
$ The first %s is the name of the command.
$ The second %s is the name of the file system.

CMN_READ_ERROR_E "\
0503-033 %s:  An error occurred while attempting to read from \n\
\tthe device %s.\n"
$ %s is the command that received the error.

CMN_REALLOC_FAILED_E "\
0503-034 %s:  Received an error while trying to reallocate space.\n\
\tUse local problem reporting procedures.\n"
$ %s is the command that received the error.

CMN_REMOTE_FS_E "\
0503-035 %s:  There is not enough free disk space in a filesystem.\n\
\tThe remote filesystem %s:%s,\n\
\tmounted over %s, \n\
\tneeds an additional %d 512-byte blocks.\n\
\tThis filesystem cannot be extended automatically by the installation\n\
\tprogram.  Make more space available then retry the installation.\n\n"
$ The first %s is the name of the remote host
$ The second %s is the name of the filesystem on the remote host.
$ The third %s is the name of the filesystem on the local host (mount point).
$ The %d is the number of blocks required

CMN_RENAME_W "\
%s:  Warning. File %s\n\
\tcould not be renamed to %s.\n"
$ The first %s is the name of the command.
$ The second %s is the name of the file.
$ The third %s is the name to which the file was to be renamed.

CMN_SET_ENV_VAR_E "\
0503-036 %s:  Environment variable %s could not be set.\n"
$ The first %s is the name of the command.
$ The second %s is the name of the environment variable.

CMN_SYSCOM_E "\
0503-037 %s:  Failure on system call to execute command %s.\n"
$ The first %s is the name of the command.
$ The second %s is the name of the command which was being executed by
$ the system command.

CMN_TOO_FW_E "\
0503-038 %s:  Not enough parameters were passed on the \n\
\tcommand line.\n"
$ %s is the name of the command.

CMN_TOO_MN_E "\
0503-039 %s:  Too many parameters were passed on the \n\
\tcommand line.\n"
$ %s is the name of the command.

CMN_VPDERR_E "\
0503-040 %s:  A system error occurred while attempting \n\
\tto access the Software Vital Product Data.\n\
\tUse local problem reporting procedures.\n"
$ %s is the name of the command.

CMN_PLEASE_WAIT_I "\
Please wait...\n"

CMN_VERIF_SELS_I "\
Verifying selections..."

CMN_DONE_I "\
done\n"

CMN_NO_EXPAND_ALLOWED_E "\
0503-041 %s:  There is not enough free disk space in filesystem\n\
\t%s (%d more 512-byte blocks are required).\n\
\tThis filesystem cannot be extended automatically by the installation\n\
\tprogram.  Make more space available then retry the installation.\n\n"

$ -----------------------------------------------------------------
$set MS_INUCONVERT
$ These are the messages for the conversion routines which are part
$ of the installp command.
$ Message numbers 0503-750 through 0503-799.
$ -----------------------------------------------------------------

ICV_CONV_E "\
0503-750 %s:  An error occurred while running the %s \n\
\tconversion shell script.\n\
\tThe return code is %d.\n"
$ The second %s is the name of the conversion shell script.
$ The %d is the return code value.

ICV_GETWD_E "\
0503-751 %s:  An error occurred while running the getwd function.\n\
\tUse local problem reporting procedures.\n"
$ Do not translate getwd.

ICV_MISS_FILE_E "\
0503-752 %s:  One or more root inventory files exist for the fileset \n\
\t%s,\n\
\tbut the file %s is missing.\n"

ICV_MKDIR_E "\
0503-753 %s:  Cannot make the directory %s \n\
\tspecified in the inventory file %s.\n\
\tCheck path name and permissions.\n"

ICV_MRKBRK_READ_E "\
0503-754 %s:  An error occurred while trying to mark the fileset\n\
\t%s broken.\n\
\tThe Software Vital Product Data database could not be read. \n\
\tThe return code is %d.\n\
\tUse local problem reporting procedures.\n"

ICV_MRKBRK_UPDT_E "\
0503-755 %s:  An error occurred while trying to mark the fileset\n\
\t%s broken.\n\
\tThe Software Vital Product Data database could not be updated. \n\
\tThe return code is %d.\n\
\tUse local problem reporting procedures.\n"

ICV_MV_FAIL_E "\
0503-756 %s:  An error occurred while processing the \n\
\tinventory file.\n\
\tThe file move of %s to %s failed.\n\
\tThe return code is %d.\n\
\tThe inventory file is %s\n\
\tCheck path name and permissions.\n"

ICV_MV_NOEXP_E "\
0503-757 %s:  An error occurred while processing the \n\
\tinventory file.\n\
\tAttempted to move %s to %s\n\
\tbut the destination filesystem was not large enough \n\
\tand could not be expanded.\n\
\tThe return code is %d.\n\
\tThe inventory file is %s.\n"

ICV_NODEL_SWVPD_E "\
0503-758 %s:  Cannot delete %s\n\
\tfrom the Software Vital Product Data database.\n\
\tThe return code is %d.\n\
\tUse local problem reporting procedures.\n"

ICV_NOFILE_INVEN_E "\
0503-759 %s:  An error occurred while processing the \n\
\tinventory file.\n\
\tNeither the newfile %s\n\
\tnor the oldpath %s exists.\n\
\tThe inventory file is %s.\n"

ICV_NOINFO_SWVPD_E "\
0503-760 %s:  Cannot get information about %s\n\
\tfrom the Software Vital Product Data database.\n\
\tThe return code is %d.\n\
\tUse local problem reporting procedures.\n"

ICV_OLDPATH_E "\
0503-761 %s:  An error occurred while reading the oldpath attribute.\n\
\tThe inventory file is %s.\n\
\tThe stanza is %s.\n"

ICV_OLDP_DIR_E "\
0503-762 %s:  An error occurred while processing the \n\
\tinventory file.\n\
\tThe stanza %s incorrectly specifies that\n\
\toldpath %s is a directory.\n\
\tThe inventory file is %s.\n"

ICV_OLDP_FIL_E "\
0503-763 %s:  An error occurred while processing the \n\
\tinventory file.\n\
\tThe stanza %s incorrectly specifies that\n\
\toldpath %s is a file.\n\
\tThe inventory file is %s.\n"

ICV_OLDP_NOINF_E "\
0503-764 %s:  An error occurred while processing the \n\
\tinventory file.\n\
\tCannot get information about the oldpath %s.\n\
\tThe inventory file is %s.\n\
\tThe stanza is %s.\n"

ICV_RETURN_DIR_E "\
0503-765 %s:  Cannot return to directory %s.\n\
\tThe return code is %d.\n\
\tUse local problem reporting procedures.\n"

ICV_RMDIR_E "\
0503-766 %s:  Cannot remove the directory %s. \n\
\tThe inventory file is %s.\n\
\tThe stanza is %s.\n\
\tThe directory must be empty to be removed.\n\
\tCheck path name and permissions.\n"

ICV_RMV_LINK_E "\
0503-767 %s:  An error occurred while processing the \n\
\tinventory file.\n\
\tThe link %s could not be removed. \n\
\tThe return code is %d.\n\
\tThe inventory file is %s.\n\
\tThe stanza is %s.\n\
\tCheck path name and permissions.\n"

ICV_STANZA_OLDPATH_E "\
0503-768 %s:  The inventory file is not formatted correctly.\n\
\tThe oldpath %s is not an absolute path.\n\
\tThe inventory file is %s.\n\
\tThe stanza is %s.\n"

ICV_STANZA_PATH_E "\
0503-769 %s:  The inventory file is not formatted correctly.\n\
\tThe stanza name %s is not an absolute path name. \n\
\tThe inventory file is %s.\n"

ICV_STANZA_TYPE_E "\
0503-770 %s:  The inventory file is not formatted correctly.\n\
\tThe type attribute is %s.\n\
\tIt must be either a file name or directory name.\n\
\tThe inventory file is %s.\n\
\tThe stanza is %s.\n"

ICV_TYPE_E "\
0503-771 %s:  An error occurred while reading the type attribute.\n\
\tThe inventory file is %s.\n\
\tThe stanza is %s.\n"

ICV_UNARC_E "\
0503-772 %s:  The file %s could not be unarchived.\n\
\tThe return code is %d.\n"

ICV_VPD_PATH_E "\
0503-773 %s:  A path cannot be set to the /usr/lib/objrepos directory.\n\
\tThe return code is %d.\n\
\tUse local problem reporting procedures.\n"
$ Do not translate /usr/lib/objrepos.
$ The first %s is the command name for all following messages.


$ ------------------------------------------------
$set MS_INSTALLP
$ These are the messages for the installp command.
$ Message numbers 0503-400 through 0503-500.
$ ------------------------------------------------

INP_ACTION_E "\
0503-400 installp:  Only one of the following flags may be used on \n\
\tthe command line at a time:  \n\
\t\t-a, -ac, -c, -r, -u, -C, -A, -i, -l,or -s.\n"
$ Do not translate installp.

INP_ALL_EX_E "\
0503-401 installp:  The -%c flag and a list of installable filesets\n\
\tare mutually exclusive.\n"
$ Do not translate installp.

INP_AN_INVAL_E "\
0503-402 installp:  The NO SAVE (-N) option is invalid without \n\
\tthe COMMIT (-c) option. \n"
$ Do not translate installp.

INP_APAR_INFO_I "\
\tFix information for %s\n"

INP_APPLY_BAD_SAVDIR_SYM_LINK_E "\
installp:  Using the existing save directory for the update\n\
\t%s.\n"

INP_APPLY_DANGLING_SAVDIR_SYM_LINK_E "\
0503-403 installp:  Cannot access the save directory,\n\
\t%s,\n\
\tfrom the symbolic link,\n\
\t%s.\n"

INP_A_ASSUMED_W "\
installp:  No action was indicated. \n \
\tThe -a (apply) flag is being assumed. \n"
$ Do not translate installp.
$ Do not translate -a.

INP_BADNAM_E "\
0503-404 installp:  %s is an invalid fileset name.\n\
\tUse the -l flag to see a list of available software.\n"
$ Do not translate installp.
$ %s is the name of the fileset.

INP_BAD_RESTORE_E "\
0503-405 installp:  An error occurred while running the restore command.\n\
\tUse local problem reporting procedures.\n"
$ Do not translate installp.

INP_BAD_USE_E "\
0503-406 installp:  The -%s flag is incompatible as given.\n"
$ Do not translate installp.

INP_BEGIN_COPYRIGHT_I "\n\
. . . . . << Copyright notice for %s >> . . . . . . .\n"
$ Where %s is a package name

INP_END_COPYRIGHT_I "\
. . . . . << End of copyright notice for %s >>. . . . \n\n"
$ Where %s is a package name

INP_BOSBOOT_FAILED_E "\
0503-407 %s:  An error occurred during bosboot processing.\n\
\tPlease correct the problem and rerun %s.\n"
$ Do not translate bosboot

INP_BOSBOOT_PERFORM_COMPLETE_I "\
%s:  bosboot process completed.\n"

INP_BOSBOOT_PERFORM_E "\
0503-408 %s:  bosboot process starting...\n"

INP_BOSBOOT_VERIFY_COMPLETE_I "\
%s:  bosboot verification completed.\n"
$ Do not translate bosboot

INP_BOSBOOT_VERIFY_E "\
0503-409 %s:  bosboot verification starting...\n"
$ Do not translate bosboot

INP_CANT_SYM_LINK_E "\
0503-411 installp:  Cannot create a symbolic link\n\
\tnamed %s\n\
\tto file or directory %s.\n\
\tCheck path name and permissions.\n"
$ Do not translate installp.
$ %s is the name of the symbolic link that was not created.
$ %s is the pathname to the file or directory.

INP_CLEANING_I "\n\
installp:  Cleaning up software for:\n"
$ Do not translate installp.
$ to be followed by a list of filesets.

INP_CLEANUP_COMMIT_I "\n\
installp:  Cleaning up software by finishing the commit operation for:\n"
$ Do not translate installp.
$ to be followed by a list of filesets.

INP_CLEANUP_NOSTAT_E "\
0503-412 installp:  The status file was not found. \n\
\tThis could be caused by an internal error within the \n\
\tcleanup script or could be caused by using Ctrl-c to \n\
\tinterrupt the installation processing.\n"
$ Do not translate installp.

INP_CLEANUP_NOTFND_E "\
0503-413 installp:  The cleanup script, %s,\n\
\tcould not be found for fileset %s.\n\
\tThe fileset has been marked as BROKEN since cleanup could not\n\
\tbe performed.\n"
$ Do not translate installp.

INP_COMMIT_DANGLING_SAVDIR_SYM_LINK_E "\
installp:  Unable to access the save directory,\n\
\t%s,\n\
\tfrom the symbolic link,\n\
\t%s.\n\
\tCommitting without deleting saved files.\n"

INP_COMMIT_ERROR_E "\n\
0503-414 installp:  Due to errors unable to commit fileset\n\
\t%s.\n"
$ Do not translate installp.

INP_DEINST_NOSTAT_E "\
0503-415 installp:  The status file was not found. \n\
\tThis could be caused by an internal error within the \n\
\tdeinstall script or could be caused by using Ctrl-c to \n\
\tinterrupt the installation processing.\n"
$ Do not translate installp.

INP_EXCLUS_E "\
0503-416 installp:  The -%s and -%s flags are mutually exclusive.\n"
$ Do not translate installp.

INP_EXCL_ALL_E "\
0503-417 installp:  The -%s flag and the keyword \"all\" are mutually \n\
\texclusive.\n"
$ Do not translate installp.
$ Do not translate "all".

INP_EXEC_FAIL_E "\
\n0503-418 installp:  The reinvocation of the installp command has failed.\n\
\tUse local problem reporting procedures.\n"

INP_FAIL_CR_SUM_LOG_E "\
0503-419 installp:  Could not create the %s\n\
\tinstallp summary log file.\n"

INP_INSTRUCTIONS_I "\
\tInstructions for %s\nfrom file:  %s\n"
$ The first %s is for the fileset name and level.
$ The second %s is a file name.

INP_INTRN_FLNOTFND_E "\
0503-420 installp:  The status file was not found. \n\
\tThis could be caused by an internal error within the \n\
\tinstallation scripts or could be caused by using Ctrl-c \n\
\tto interrupt the installation processing.\n"
$ Do not translate installp.

INP_INTRN_INVSTAT_E "\
0503-421 installp:  Invalid status character found in the status file. \n\
\tThis could be caused by an internal error within the \n\
\tinstallation scripts or could be caused by using Ctrl-c \n\
\tto interrupt the installation processing.\n"
$ Do not translate installp.

INP_INTRN_MISSVPDI_E "\
0503-422 installp:  An internal error has occurred. Expected \n\
\tinformation was not found in the Software Vital Product Data.\n"
$ Do not translate installp.

INP_INTRN_OPTNOTFND_E "\
0503-423 installp:  Fileset %s was not found in the status file.\n\
\tThis could be caused by an internal error within the \n\
\tinstallation scripts or could be caused by using Ctrl-c \n\
\tto interrupt the installation processing.\n"
$ Do not translate installp.
$ %s is the name of the fileset which could not be found.

INP_INUPDT_MSG_W "\
instupdt:  The use of the -%s flag implies the use of the \n\
\t-%s flag(s). Please reissue the command or refer to the \n\
\tdocumentation on the installp command.\n"
$ Do not translate instupdt or installp.

INP_INURID_RUNNING_E "\
0503-424 installp:  The installp command cannot execute while the \n\
\tinurid command is running. If the inurid command is not really \n\
\trunning, remove the \"/usr/lpp/inurid_LOCK\" file and rerun the command.\n"

INP_INVAL_E "\
0503-425 installp:  A %s is an invalid argument to the -%s flag.\n"
$ Do not translate installp.

INP_INVAL_LPP_E "\
0503-426 installp:  %s is an invalid fileset name.\n"
$ Do not translate installp.
$ %s is the name of the fileset.

INP_INVAL_SCED_OPT_E "\
0503-427 installp:  Invalid superseding fileset name. The superseding\n\
\tfileset name %s\n\
\tmust match the current fileset.\n"
$ Do not translate installp.

INP_INVAL_SCED_SCED_E "\
0503-428 installp:  Invalid superseding fileset update. The superseding\n\
\tupdate %s\n\
\tis more than %d bytes.\n\
\tThe superseding update may only be %d bytes.\n\n"
$ Do not translate installp.

INP_LEVEL_E "\
0503-429 installp:  %s is invalid.\n \
\tAll levels must be preceded by the name of a fileset at \n\
\tthat level.\n"
$ Do not translate installp.

INP_LINE_E "\
Usage installp:  Valid Flags are AaBbCcDdeFfgIilLNOpqrstuVvXz where:\n\n\
       -A  List fixes                     -l  List software on media\n\
       -a  Apply software                 -L  List software (colon separated)\n\
       -B  Consider updates only          -N  Do not save replaced files\n\
       -b  No bosboot                     -O  {[r][s][u]}  root, share, usr\n\
       -C  Cleanup install failure        -p  Preview\n\
       -c  Commit update(s)               -q  Quiet (no prompt)\n\
       -D  Delete install image           -r  Reject update(s)\n\
       -d  {install device}               -s  List applied software updates\n\
       -e  {log file}                     -t  {alternate save directory}\n\
       -F  Force installation             -u  Deinstall (Remove) fileset(s)\n\
       -f  {file containing fileset(s)}   -V  {verbosity level [2-4]}\n\
       -g  Include requisite software     -v  Verify installation\n\
       -I  Consider base levels only      -X  Expand filesystem if necessary\n\
       -i  List supplemental information  -z  {media block size}\n\n"
$ Do not translate installp.

INP_LISTHEAD_UL_I "\
===================================================================="

INP_LIST_UL_I "\
--------------------------------------------------------------------"

INP_LOCKPENDING_E "\
0503-430 installp:  Either there is an installp process currently \n\
\trunning or there is a previously failed installation. \n\
\tWait for the process to complete or run installp -C to \n\
\tcleanup a failed installation.\n"
$ Do not translate installp.

INP_LPPALL_I "\
%c Description:  Will include ALL the images for %s.\n"
$ %c is the comment character in SMIT.
$ %s is the name of the fileset.

INP_LPP_ACTION_I "\
I/U"
$ An install or an update.
$ Maximum character length of 3

INP_LPP_CONTENT2_I "\
Content"
$ The contents of the fileset, e.g., o for object, etc.
$ Maximum character length of 8

INP_LPP_LABEL_I "\
Fileset Name"
$ The name of the fileset.
$ Maximum character length of 35

INP_LPP_LEVEL_I "\
Level"
$ The level at which the fileset should be installed.
$ Maximum character length of 26

INP_LPP_QUISCENT2_I "\
Q"
$ Whether or not the system should be quiesced to install this fileset.
$ Maximum character length of 1

INP_MISS_NETLS_PID_E "\
0503-431 installp:  The product ID data for fileset\n \
\t%s is missing from the iFOR/LS\n\
\tinformation within the table of contents on the installation media.\n"
$ Do not translate installp.

INP_MISS_NETLS_PVER_E "\
0503-432 installp:  The product version data for fileset\n \
\t%s is missing from the iFOR/LS\n \
\tinformation within the table of contents on the installation media.\n"
$ Do not translate installp.

INP_MUST_HAVE_E "\
0503-433 installp:  The -%s flag is only valid when the -%s flag is \n\
\talso used.\n"
$ Do not translate installp.

INP_NEEDCLEANUP_E "\
0503-434 installp:  There are incomplete installation operations \n\
\ton the following filesets.  Run installp -C to clean up \n\
\tthe previously failed installations before continuing.\n\n"
$ Do not translate installp.
$ Do not translate -C.
$ This message is followed by a list of filesets.

INP_NOARG_E "\
0503-435 installp:  The -%s flag requires an argument.\n"
$ Do not translate installp.

INP_NODEV_E "\
0503-436 installp:  Device %s could not be accessed.\n\
\tSpecify a valid device name.\n"
$ Do not translate installp.
$ %s is a device name.

INP_NOLIBLPP_E "\
0503-437 installp:  Cannot install the specified\n\
\tfileset %s.\n\
\tCould not access the installation control files.\n\
\tUse local problem reporting procedures.\n"
$ Do not translate installp.
$ %s is the name of the fileset.

INP_NOLIST_E "\
0503-438 installp:  This function of installp requires a list of \n\
                    installable filesets to be provided.\n\n"
$ Do not translate installp.

INP_NOTHING_TO_CLEAN_ALL_E "\n\
0503-439 installp:  No filesets were found in the Software Vital\n\
\tProduct Database that could be cleaned up.\n"

INP_NOTINSTL_E "\
0503-440 installp:  %s is not a standard calling routine.\n"
$ Do not translate installp.

INP_NOT_DISKFILE_E "\
0503-441 installp:  The -D flag is not valid because the installation\n\
\timage file is not on the file system.\n"
$ Do not translate installp or -D .

INP_NO_DEINST_PTF_E "\n\
0503-442 installp:  %s is NOT a valid\n\
\tbase level fileset.  You may only specify base level filesets\n\
\t(no updates) to be deinstalled.\n\n"

INP_NO_DEINST_SCRIPT_E "\n\
0503-443 installp:  Unable to access the default deinstall script, \n\
\t%s. Deinstallation attempt CANCELLED.\n\n"

INP_NO_INFO_I "\
\tThere was no information for the following:\n"

INP_NO_ROOT_TO_APPLY_E "\n\
0503-444 installp:  No filesets were found in the Software Vital\n\
\tProduct Database that could be applied to the \"root\"\n\
\tpart of %s.\n"

INP_ONE_E "\
0503-445 installp:  The -%s flag may only be used once on the command \n\
\tline.\n"
$ Do not translate installp.

INP_ONE_TDIR_E "\
0503-446 installp:  The alternate save directory\n\
\tis %s.\n\
\tCommit or reject all fileset updates before changing the alternate\n\
\tsave directory to %s.\n"
$ %s is a directory path.
$ %s is a different directory path.

INP_PERFORMING_OPER_I "\n\
installp:  %s software for:\n"
$ Do not translate installp.
$ %s is the operation being performed (i.e., APPLYING, COMMITTING, etc...)
$ to be followed by a list of filesets.

INP_PERFORMING_OPER_PART_I "\n\
installp:  %s software for the \"%s\" part of fileset\n\
\t%s.\n"
$ Do not translate installp.
$ First %s is the operation being performed (i.e., APPLYING, COMMITTING, etc...)
$ Second %s is the part (i.e., usr, root or share)
$ Third %s is the fileset name.

INP_OPER_FAIL_E "\n\
0503-447 installp:  The %s operation has failed for the following \n\
\tfilesets:\n"
$ Do not translate installp.
$ %s is the operation (i.e., apply, commit, reject, deinstall or cleanup)
$ This message is followed by a list of filesets.

INP_OPER_FAIL_PART_E "\n\
0503-448 installp:  The %s operation has failed for the \"%s\" part \n\
\tof the following filesets:\n"
$ Do not translate installp.
$ First %s is the operation (i.e., apply, commit, reject, deinstall or cleanup)
$ Second %s is the part (i.e., usr, root or share)
$ This message is followed by a list of filesets.

INP_OPER_SUCC_I "\n\
installp:  The %s operation was successful for the following \n\
\tfilesets:\n"
$ Do not translate installp.
$ %s is the operation (i.e., apply, commit, reject, deinstall or cleanup)
$ This message is followed by a list of filesets.

INP_OPER_SUCC_PART_I "\n\
installp:  The %s operation was successful for the \"%s\" part \n\
\tof the following filesets:\n"
$ Do not translate installp.
$ First %s is the operation (i.e., apply, commit, reject, deinstall or cleanup)
$ Second %s is the part (i.e., usr, root or share)
$ This message is followed by a list of filesets.

INP_PART_COMMIT_W "\
0503-449 installp:  Warning. Some of the saved files may not have been\n\
\tcleaned up during the commit operation of %s.\n\
\tThe save directory is %s.\n\n"
$ Do not translate installp.

INP_PATH_TOO_BIG_E "\
0503-450 installp:  The save directory name for the symbolic link is too long.\n"

INP_PREINST_FAIL_E "\
0503-451 installp:  After completion of pre-installation processing, \n\
\tthere are no valid filesets left to process.  No processing\n\
\thas occurred.\n"
$ Do not translate installp.

INP_PTF_CANT_SEDE_SELF_E "\
0503-452 installp:  Fatal error:  %s\n\
\terroneously supersedes itself. Processing cannot continue.\n"

INP_REINVOKE_I "\
\ninstallp:  The installp command has been updated. Reinvoking installp\n\
\tto process the remaining items.\n"

INP_REJECT_DANGLING_SAVDIR_SYM_LINK_E "\
0503-453 installp:  The reject has been CANCELLED.\n\
\tProcessing cannot continue.\n\
\tA directory for saved files cannot be accessed.\n\
\tA symbolic link, %s,\n\
\tsuggests that a save directory is\n\
\t %s,\n\
\tand that the alternate save directory is\n\
\t %s.\n\
\tPlease check that the alternate save directory\n\
\tis mounted or available.\n\
\tA save directory is not accessible for\n\
\tthe following filesets:\n"

INP_REJECT_NOSTAT_E "\
0503-454 installp:  The status file was not found. \n\
\tThis could be caused by an internal error within the \n\
\treject script or could be caused by using Ctrl_c to \n\
\tinterrupt the installation processing.\n"
$ Do not translate installp.

INP_REM_XFLAG_W "\
installp:  Warning. The functionality for the -x flag has been \n\
\tremoved due to its destructive nature. The installp command \n\
\twill continue.\n"
$ Do not translate installp. Do not translate -x.

INP_SHARE_READONLY_E "\
0503-455 installp:  \"share\" part is on a read only file system.\n\
\tSpecify parts with the -O[r][u] option.\n"

INP_SMIT_STATUS_HEAD_I "\
%sName                                Level\n"

INP_STATUS_HEAD_I "\
%sName                                Part    Level                   State\n"

INP_STATUS_HEAD_LINE_I "\
%s---------------------------------------------------------------------------\n"

INP_STATUS_I "\
%sInstallp Status\n"

INP_STATUS_LINE_I "\
%s---------------\n"

INP_SYSCK_ERROR_E "\
0503-456 installp:  The program \"sysck\" detected verification errors with\n\
\tfileset %s.\n\
\tThe fileset may still be usable but will be marked as BROKEN.\n"
$ Do not translate installp.
$ Do not translate sysck.

INP_TFLAG_LOCK_E "\
0503-457 installp:  The alternate save directory %s\n\
\tis not available. If the alternate save\n\
\tdirectory is available, then delete\n\
\t%s/%s\n\
\tor specify a different alternate save directory.\n\
\tIf the file is on a remote filesystem,\n\
\tverify that the remote server allows root access to\n\
\tthe remote directory for this host.\n"
$ Do not translate root (the name of the super user).
$ The first %s is the alternate save directory path.
$ The second/third %s is the lock file in the alternate save directory.

INP_USAGE_A_I "\
Usage installp:  Install software:  Apply Only (-a):\n\
      installp -a [-e LogFile] [-V Number] [-d Device] [-bBDIpqvX]\n\
               [-F | -g] [-O{[r][s][u]}] [-t SaveDirectory] [-z BlockSize] \n\
               { Fileset [Level] ... | -f ListFile | all }\n\n"
$ Do not translate:  installp, all
$ Words which may be translated are:  Device, BlockSize, SaveDirectory, 
$ LogFile, ListFile, Fileset, and Level

INP_USAGE_A_C_I "\
Usage installp:  Install Software:  Apply and Commit (-ac):\n\
      installp -ac[N] [-e LogFile] [-V Number] [-d Device] [-bBDIpqvX]\n\
               [-F | -g] [-O{[r][s][u]}] [-t SaveDirectory] [-z BlockSize] \n\
               { FileSet [Level] ... | -f ListFile | all }\n\n"
$ Do not translate:  installp, all
$ Words which may be translated are:  Device, BlockSize, SaveDirectory, 
$ LogFile, ListFile, FileSet, and Level

INP_USAGE_A_AC_I "\
Usage installp:  Install Software:  Apply Only (-a) or Apply/Commit (-ac):\n\
      installp [-a | -ac[N]] [-e LogFile] [-V Number] [-d Device]\n\
               [-bBDIpqvX] [-F | -g] [-O{[r][s][u]}] [-t SaveDirectory]\n\
               [-z BlockSize] { FileSet [Level] ... | -f ListFile | all }\n\n"
$ Do not translate:  installp, all
$ Words which may be translated are:  Device, BlockSize, SaveDirectory, 
$ LogFile, ListFile, FileSet, and Level

INP_USAGE_AL_I "\
Usage installp:  List Fixes (-A):\n\
      installp -A [-e LogFile] [-d Device] [-BIq] [-O{[s][u]}] [-z BlockSize]\n\
               { FileSet [Level] ... | -f ListFile | all }\n\n"
$ Do not translate:  installp, all
$ Words which may be translated are:  Device, BlockSize, LogFile, 
$ ListFile, FileSet, and Level

INP_USAGE_A_I_I "\
Usage installp:  List Fixes (-A) or Supplemental Information (-i):\n\
      installp {-A | -i} [-e LogFile] [-d Device] [-BIq] [-O{[s][u]}]\n\
               [-z BlockSize] { FileSet [Level] ... | -f ListFile | all }\n\n"
$ Do not translate:  installp, all
$ Words which may be translated are:  Device, BlockSize, LogFile, 
$ ListFile, FileSet, and Level

INP_USAGE_CL_I "\
Usage installp:  Clean Up Install Failure (-C):\n\
      installp -C [-e LogFile] [-b] \n\n"
$ Do not translate installp.
$ Words which may be translated are:  LogFile

INP_USAGE_CO_I "\
Usage installp:  Commit Applied Software Updates (-c):\n\
      installp -c [-e LogFile] [-V Number] [-bgpvX] [-O{[r][s][u]}]\n\
               { FileSet [Level] ... | -f ListFile | all }\n\n"
$ Do not translate:  installp, all
$ Words which may be translated are:  LogFile, ListFile, FileSet, and Level

INP_USAGE_C_R_U_I "\
Usage installp:  Commit (-c), Reject (-r), or Deinstall (-u) Software:\n\
      installp {-c | -r | -u} [-e LogFile] [-V Number] [-bgpvX]\n\
               [-O{[r][s][u]}] { FileSet [Level] ... | -f ListFile | all }\n\
      Note:  The keyword \"all\" is not valid with the -r or -u flag.\n\n"
$ Do not translate:  installp, all
$ Words which may be translated are:  LogFile, ListFile, FileSet, and Level

INP_USAGE_I_I "\
Usage installp:  List Supplemental Information (-i):\n\
      installp -i [-e LogFile] [-d Device] [-BIq] [-O{[s][u]}] [-z BlockSize]\n\
               { FileSet [Level] ... | -f ListFile | all }\n\n"
$ Do not translate:  installp, all
$ Words which may be translated are:  Device, BlockSize, LogFile, 
$ ListFile, FileSet, and Level

INP_USAGE_L_I "\
Usage installp:  List Software on Media (-l or -L):\n\
      installp [-l | -L ] [-e LogFile] [-d Device] [-BIq] [-O{[s][u]}]\n\
               [-z BlockSize]\n\n"
$ Do not translate installp.
$ Words which may be translated are:  Device, BlockSize and LogFile

INP_USAGE_R_I "\
Usage installp:  Reject Applied Software Updates (-r):\n\
      installp -r [-e LogFile] [-V Number] [-bgpvX] [-O{[r][s][u]}]\n\
               { FileSet [Level] ... | -f ListFile }\n\n"
$ Do not translate installp.
$ Words which may be translated are:  LogFile, ListFile, FileSet, and Level

INP_USAGE_S_I "\
Usage installp:  List Applied Software Updates (-s):\n\
      installp -s [-e LogFile] [-O{[r][s][u]}] \n\
               [ FileSet [Level] ... | -f ListFile | all ]\n\n"
$ Do not translate:  installp, all
$ Words which may be translated are:  LogFile, ListFile, FileSet, and Level

INP_USAGE_T_I "\
Usage installp:  Specifying alternate save directory with the -t flag:\n\
      installp [-a | -ac[N]] [-t SaveDirectory] [-e LogFile] [-V Number]\n\
               [-d Device] [-bBDIpqvX] [-F | -g] [-O{[r][s][u]}]\n\
               [-z BlockSize] { FileSet [Level] ... | -f ListFile | all }\n\n"
$ Do not translate:  installp, all
$ Words within the syntax which may be translated are:  Device, 
$ BlockSize, SaveDirectory, LogFile, ListFile, FileSet, and Level

INP_USAGE_U_I "\
Usage installp:  Deinstall (Remove) Software:\n\
      installp -u [-e LogFile] [-V Number] [-bgpvX] [-O{[r][s][u]}]\n\
               { FileSet [Level] ... | -f ListFile }\n\n"
$ Do not translate installp.
$ Words within the syntax which may be translated are:  LogFile
$ ListFile, FileSet, and Level

INP_USR_READONLY_E "\
0503-458 installp:  \"usr\" part is on a read only file system.\n\
\tSpecify parts with the -O[r][s] option.\n"

INP_PREV_INST_NOT_I "\
installp PREVIEW:  installation will not actually occur.\n"

INP_PREV_COMMIT_NOT_I "\
installp PREVIEW:  commit operation will not actually occur.\n"

INP_PREV_REJECT_NOT_I "\
installp PREVIEW:  reject operation will not actually occur.\n"

INP_PREV_DEINST_NOT_I "\
installp PREVIEW:  deinstall operation will not actually occur.\n"

INP_VERIF_INST_I "\
installp:  Pre-installation verification may take several minutes.\n"

INP_VERIF_COMMIT_I "\
installp:  Pre-commit verification may take several minutes.\n"

INP_VERIF_REJECT_I "\
installp:  Pre-reject verification may take several minutes.\n"

INP_VERIF_DEINST_I "\
installp:  Pre-deinstall verification may take several minutes.\n"

INP_PREV_END_I "\
End of installp PREVIEW.  "

INP_PREV_NO_APPLY_I "\
No apply operation has actually occurred.\n"

INP_PREV_NO_COMMIT_I "\
No commit operation has actually occurred.\n"

INP_PREV_NO_REJECT_I "\
No reject operation has actually occurred.\n"

INP_PREV_NO_DEINST_I "\
No deinstall operation has actually occurred.\n"

INP_PIV_I "\
\t\t    Pre-installation Verification...\n"

INP_PCV_I "\
\t\t\tPre-commit Verification...\n"

INP_PRV_I "\
\t\t\tPre-reject Verification...\n"

INP_PDV_I "\
\t\t    Pre-deinstall Verification...\n"

INP_INST_SW_I "\
\t\t\t Installing Software...\n"

INP_COMMIT_SW_I "\
\t\t\t  Committing Software...\n"

INP_REJ_SW_I "\
\t\t\t  Rejecting Software...\n"

INP_DEINST_SW_I "\
\t\t\t   Deinstalling Software...\n"

INP_PIP_I "\
\t\t     Post-installation Processing...\n"

INP_PCP_I "\
\t\t        Post-commit Processing...\n"

INP_PRP_I "\
\t\t        Post-reject Processing...\n"

INP_PDP_I "\
\t\t       Post-deinstall Processing...\n"

INP_SUMMARIES_I "\
\t\t\t\tSummaries:\n"

INP_CANT_REJ_COMMITEDS_I "\
Committed:  cannot be rejected\n"

INP_INCOST_UR_PARTS_I "\
Inconsistent usr and root parts\n"

INP_NO_SW_PRODS_IN_VPD_E "\
0503-459 installp:  No filesets were found in the Software\n\
\tVital Product Database in the APPLIED state.\n"

INP_PREV_AND_MAJOR_FLAG_E "\
0503-460 installp:  The -p flag (preview operation) must be specified with\n\
\tone of the -a (apply), -c (commit), -r (reject), or\n\
\t-u (deinstall).\n\n"

INP_ONE_MAJ_PREV_FLAG_E "\
0503-461 installp:  The -p flag (preview operation) must be specified with\n\
\tonly one of the following flags:  -a (apply), -c (commit),\n\
\t-r (reject), or -u (deinstall).\n\n"

INP_INCMPLT_SUMMARY_E "\
0503-462 installp:  This may result in the installation summary \n\
\tbeing incomplete.\n"

INP_FINIS_PROC_ALL_PKGS_I "\
Finished processing all filesets."

INP_PROC_XOFY_I "\
Filesets processed:  %d of %d"

INP_TT_HMS_I "\
  (Total time:  %d hrs %d mins %d secs).\n"

INP_TT_MS_I "\
  (Total time:  %d mins %d secs).\n"

INP_TT_S_I "\
  (Total time:  %d secs).\n"

INP_RESOURCES_TITLE_I "\
RESOURCES\n\
---------\n"

INP_NO_RESOURCE_EST_I "\
  Resource requirements could not be estimated.  It appears that size\n\
  information was not provided for the filesets being installed.\n\n"

INP_RESOURCES_HDR_I "\
  Estimated system resource requirements for filesets being installed:\n\
                (All sizes are in 512-byte blocks)\n"

INP_RESOURCES_COL_HDR_I "\
      Filesystem                     Needed Space             Free Space\n"
$ The beginning of each word needs to start in the same byte location.

INP_TOTAL_RESOURCE_I "\
      TOTAL: %34d %22d\n\n"

INP_PAGE_SPACE_HDR_I "\
  Paging Space Requirements:    %10d\n\n"

INP_NEEDED_SPACE_I "\
  NOTE:  \"Needed Space\" values are calculated from data available prior\n\
  to installation.  These are the estimated resources required for the\n\
  entire operation.  Further resource checks will be made during\n\
  installation to verify that these initial estimates are sufficient.\n"

INP_USE_X_I "\
  Use the option to extend filesystems (-X flag) to ensure that any\n\
  additional resources which may be needed are automatically allocated.\n\n"

INP_BOSBOOT_DONTBOOT_E "\
0503-463 %s:  An error occurred during bosboot processing.\n\
\tDo NOT reboot your machine until you have successfully run\n\
\tthe installp -C command to cleanup.\n"
$ Do not translate bosboot
$ Do not translate installp

INP_BOSBOOT_WASRUN_I "\
installp:  The bosboot command has successfully rebuilt the boot image.\n\
\tIt may be necessary to reboot your system before the changes \n\
\twill take effect.\n"
$ Do not translate bosboot

INP_APPLY_FAIL_PART_E "\n\
0503-464 installp:  The installation has FAILED for the \"%s\" part \n\
\tof the following filesets:\n"
$ Do not translate installp.
$ %s is the part (usr, root, or share)
$ This message is followed by a list of filesets.

INP_REJECT_FAIL_PART_E "\n\
0503-467 installp:  The reject operation has FAILED for the \"%s\" part \n\
\tof the following filesets (the software has been marked \n\
\tBROKEN and may need to be reinstalled):\n"
$ Do not translate installp.
$ %s is the part (usr, root, or share)
$ This message is followed by a list of filesets.

INP_CLEAN_FAIL_PART_E "\n\
0503-470 installp:  The cleanup operation has FAILED for the \"%s\" part \n\
\tof the following filesets:\n"
$ Do not translate installp.
$ %s is the part (usr, root, or share)
$ This message is followed by a list of filesets.

INP_DEINSTALL_FAIL_PART_E "\n\
0503-473 installp:  The deinstall operation has FAILED for the \"%s\" part\n\
\tof the following filesets:\n"
$ Do not translate installp.
$ %s is the part (usr, root, or share)
$ This message is followed by a list of filesets.

INP_NO_CMD_LINE_IN_NIM_E "\
0503-476 installp:  The installp command cannot be run directly from\n\
\tthe command line in the Network Installation Management (NIM)\n\
\tenvironment.  Please use the NIM commands.\n"

INP_VERBOSITY_USAGE_E "\
0503-477 installp:  The -V flag (detail/verbosity specifier for installp \n\
\toutput) may only be used in combination with one of the following: \n\
\t-a (apply), -c (commit), -ac (apply and commit), -r (reject), or \n\
\t-u (deinstall).\n\n"
$ Do not translate installp.

INP_FORCE_APPLY_USR_I "\
installp:  You cannot force re-apply (-aF) the USR part without also \n\
specifying the ROOT part.\n"

INP_41_UPDT_HAS_SEDES_E "\
0503-478 installp:  The 4.1-formatted fileset update %s\n\
\tincorrectly contains supersedes information.\n"

INP_NO_SEDES_IN_41_UPDT_E "\
0503-479 installp:  4.1-formatted fileset updates cannot contain\n\
\tsupersedes information.\n"

INP_NO_FIX_ID_E "\
0503-480 installp:  Base level fileset %s %d.%d.%d.%d.%s\n\
\tincorrectly contains a PTF ID field.\n"

INP_NO_FIX_IN_41_BASE_E "\
0503-481 installp:  Base level fileset %s %d.%d.%d.%d.%s\n\
\tincorrectly contains a nonzero fix value.\n"

INP_FIX_MUST_BE_0_E "\
0503-482 installp:  All 4.1-formatted base level filesets must contain a\n\
\tfix value of zero.\n"

INP_41_UPDT_HAS_FIX_ID_E "\
0503-483 installp:  Fileset update %s %d.%d.%d.%d.%s\n\
\tincorrectly contains a PTF ID field.\n"

INP_NO_FIX_ID_IN_41_UPDT_E "\
0503-484 installp:  4.1-formatted fileset updates cannot contain a PTF ID\n\
\tfield.\n"

INP_41_UPDT_HAS_ZERO_MF_E "\
0503-485 installp:  The 4.1-formatted fileset update\n\
\t%s %d.%d.%d.%d.%s\n\
\tincorrectly contains zero modification and zero fix values.\n"

INP_NO_ZERO_MOD_AND_FIX_E "\
0503-486 installp:  4.1-formatted fileset updates cannot contain zero\n\
\tmodification and zero fix values.\n"

INP_SEDES_PARSING_E "\n\
0503-487 installp:  Error parsing supersedes information for\n\
\t%s.\n"

INP_BAD_LEVEL_IN_SEDES_E "\n\
0503-488 installp:  An invalid level of %s was specified in the\n\
\tsupersedes information for %s.\n"

INP_BAD_NAME_IN_SEDES_E "\n\
0503-489 installp:  An invalid fileset name was found in the supersedes\n\
\tinformation for %s.\n"

INP_LEVEL_TOO_HIGH_E "\
0503-490 installp:  The level field specified in the supersedes information\n\
\tof a base level fileset must not be greater than the level of the base level\n\
\tfileset itself.\n"

INP_SEDES_NAME_MUST_MATCH_E "\n\
0503-491 installp:  Any fileset name specified in the supersedes\n\
\tinformation of a base level fileset must match the name of the \n\
\tbase level fileset itself.\n"

INP_DEINST_3_1_E "\
0503-492 installp:  %s is a 3.1-formatted fileset.\n\
\tDeinstallation of this fileset requires the use of the FORCE \n\
\toption (-F flag).\n\n"

INP_DEINST_3_2_E "\
0503-493 installp:  The fileset, %s, \n\
\twas originally installed on an AIX 3.2 system.  Deinstallation\n\
\tof this fileset requires the use of the FORCE option (-F flag).\n\n"

INP_DEINST_MIGRATING_E "\
0503-494 installp:  The fileset, %s, \n\
\twas originally installed on an AIX 3.2 system.  It has only been\n\
\tpartially migrated from 3.2 to 4.1 and, therefore, cannot be\n\
\tdeinstalled.\n\n"

INP_DEINST_COMPAT_E "\
0503-495 installp:  The fileset, %s,\n\
\tis a compatibility entry used by installp and, therefore, cannot be\n\
\tdeinstalled.\n\n"

INP_REBOOT_USR_I "\n\
installp:  * * *  A T T E N T I O N ! ! ! \n\
\tSoftware changes processed during this session require this system \n\
\tand any of its diskless/dataless clients to be rebooted in order \n\
\tfor the changes to be made effective.\n"

INP_REBOOT_ROOT_I "\n\
installp:  * * *  A T T E N T I O N ! ! ! \n\
\tSoftware changes processed during this session require the system \n\
\tto be rebooted in order for the changes to be made effective.\n"


$
$ The following set of messages appear in the FILESET STATISTICS section.
$ If possible, keep each translation to one line.  If that cannot be done,
$ then indent following lines (to column 8 for messages that begin with
$ %5d and to column 12 for messages that begin with %9d).
$

INP_STAT_HDR_I "\
FILESET STATISTICS \n\
------------------\n"

INP_NUM_SELECTED_I "\
%5d  Selected to be installed, of which:\n"

INP_NUM_PASSED_PIV_I "\
%9d  Passed pre-installation verification\n"

INP_NUM_FAILED_PIV_I "\
%9d  FAILED pre-installation verification\n"

INP_NUM_SUBSTITUTED_I "\
%9d  Replaced by superseding updates\n"

INP_NUM_ALREADY_INST_I "\
%9d  Already installed (directly or via superseding filesets)\n"

INP_ADDTL_REQS_I "\
%5d  Additional requisites to be automatically installed\n"

INP_ADDTL_MAND_I "\
%5d  Unselected mandatory filesets that FAILED pre-installation verification\n"

INP_TOT_TB_INST_I "\
%5d  Total to be installed\n"

INP_TOT_SEL_REJ_I "\
%5d  Selected to be rejected, of which:\n"

INP_TOT_PASS_PRV_I "\
%9d  Passed pre-reject verification\n"

INP_TOT_FAIL_PRV_I "\
%9d  FAILED pre-reject verification\n"

INP_ADTL_DEPEND_I "\
%5d  Additional dependents to be automatically rejected\n"

INP_TOT_TBR_I "\
%5d  Total to be rejected\n"

INP_TOT_SEL_TBD_I "\
%5d  Selected to be deinstalled, of which:\n"

INP_TOT_PASS_PDV_I "\
%9d  Passed pre-deinstall verification\n"

INP_TOT_FAIL_PDV_I "\
%9d  FAILED pre-deinstall verification\n"

INP_ADTL_DEP_DEINST_I "\
%5d  Additional dependents to be automatically deinstalled\n"

INP_TOT_TBD_I "\
%5d  Total to be deinstalled\n"

INP_TOT_SELECTED_TBC_I "\
%5d  Selected to be committed, of which:\n"

INP_NUM_ALREADY_COMMITTED_I "\
%9d  Already in the committed state\n"

INP_ADTL_REQ_COMMIT_I "\
%5d  Additional requisites to be automatically committed\n"

INP_TOT_TBC_I "\
%5d  Total to be committed\n"

INP_NUM_DEFERRED_I "\
%9d  Deferred (see *NOTE below)\n"

$ This note can follow the STATISTICS section.
INP_DEFERREDS_NOTE_I "\
*NOTE  The deferred filesets mentioned above will be processed after the\n\
       installp update and its requisites are successfully installed.\n"

$
$ The following set of messages appear in the pre-installation summary.
$ The first two are headers; the remaining ones are messages that will
$ appear under the Pre-installation Failure/Warning column.  If possible,
$ keep the translation to 37 characters so that each entry will fit on one
$ line.  If the translation cannot be kept that short, then indent
$ following lines so they will start in the Pre-installation Failure
$ column.
$

INP_PIFWS_HDR1_I "\
Pre-installation Failure/Warning Summary\n\
----------------------------------------\n"

INP_PIFWS_HDR2_I "\
Name                      Level           Pre-installation Failure/Warning\n\
--------------------------------------------------------\
-----------------------\n"
$ The beginning of each word needs to start in the same byte location.

INP_PIF_NOM_TRY_SEDES_I "\
Not found; try %s\n"

INP_PIF_USR_NOT_INSTALLED_I "\
USR not instld; try %s\n"

INP_PIF_ALREADY_INST_I "\
Already installed\n"

INP_PIF_ALREADY_COMMIT_I "\
Already COMMITTED\n"

INP_PIF_FAILURE_I "\
Pre-installation failure\n"

INP_PIF_NO_USR_MEANS_NO_ROOT_I  "\
No root part or usr not installed\n"

INP_PIF_FORCE_APPLY_PTF_I "\
Cannot force apply fileset updates\n"

INP_PIF_FORCE_DEINST_I "\
Must use -F flag (force) to deinstall\n"

INP_PIF_PART_MIG_DEINST_I "\
Partially migrated, not deinstallable\n"

INP_PIF_COMPAT_DEINST_I "\
Compatibility, not deinstallable\n"

INP_PIF_ALREADY_INSTALLED_I "\
Already installed\n"

INP_PIF_ALREADY_SUPERSEDED_I "\
Already superseded by %s\n"

INP_PIF_BASE_MUST_BE_COMMITTED_I "\
Must commit before reinstalling\n"

INP_PIF_BROKEN_NEEDS_COMMIT_I "\
BROKEN, requires the -acgN flags\n"

INP_PIF_MUST_APPLY_ROOT_TOO_I "\
Must apply both usr and root parts\n"

INP_PIF_NOT_FOUND_ON_MEDIA_I "\
Not found on the installation media\n"

INP_PIF_NO_DEINST_BOS_I "\
bos.rte is not deinstallable\n"

INP_PIF_NUTTIN_TO_APPLY_I "\
Nothing by this name to apply\n"

INP_PIF_NUTTIN_TO_COMMIT_I "\
Must be in APPLIED state to commit\n"

INP_PIF_NUTTIN_TO_DEINSTL_I "\
Nothing by this name to deinstall\n"

INP_PIF_NUTTIN_TO_REJECT_I "\
Must be in APPLIED state to reject\n"

INP_PIF_OTHER_BROKENS_NEED_COMMIT_I "\
First apply BROKENs for this fileset\n"

INP_PIF_REQUISITE_FAILURE_I "\
Requisite failure\n"

INP_PIF_TO_BE_SUPERSEDED_I "\
To be superseded by %s\n"

INP_PIF_FAILED_PRE_D_I "\
Failed pre-deinstallation check\n"

INP_PIF_SUPS_A_BROKEN_FILESET_I "\
Supersedes a BROKEN fileset\n"

INP_SUMM_LINE_I "\
Installation Summary\n\
--------------------\n"

INP_SUM_HEAD_I "\
Name                        Level           Part        Event       Result\n\
--------------------------------------------------------\
-----------------------\n"
$ The beginning of each word needs to start in the same byte location.

INP_PIW_SEE_WARNING_SECTION_I "\
See WARNINGS section above\n"

INP_EXCLUS_F_G_E "\
0503-496 installp:  The \"AUTOMATICALLY install requisite software\" option \n\
\tand the \"OVERWRITE same or newer versions\" option from the \n\
\tprevious menu may not be selected together.\n"

$ The following message appears in the post-processing section if there are
$ any user-configurable files which were not preserved in their original
$ location.
INP_CFGFILES_STORED_I "\
Some configuration files could not be automatically merged into the system \n\
during the installation.  The previous versions of these files have been \n\
saved in a configuration directory as listed below.  Compare the saved files \n\
and the newly installed files to determine if you need to recover \n\
configuration data.  Consult product documentation to determine how to \n\
merge the data. \n\
"

$ The variable below is a pathname, as in /lpp/save.config
INP_CFGLIST_I "\
\n\
 Configuration files which were saved in %s:\n\
"

$ the variable in the following message is the name of the SPOT
INP_NIM_USR_SPOT_WARNING "\
Warning: the /usr filesystem on this machine has been converted into\n\
a Network Install Manager (NIM) SPOT.  If there are any diskless or\n\
dataless NIM clients using this SPOT, you should perform the following\n\
NIM operations when this invocation of installp finishes:\n\
\tnim -o sync_roots %s\n"

$ The following two messages are messages that will
$ appear under the Pre-installation Failure/Warning column.  If possible,
$ keep the translation to 37 characters so that each entry will fit on one
$ line.  If the translation cannot be kept that short, then indent
$ following lines so they will start in the Pre-installation Failure
$ column.
INP_OEM_MISMATCH_I "\
Incorrect system manufacturer\n"

INP_OEM_REPLACED_I "\
System manufacturer fileset required\n"

INP_OEM_BASELEVEL_I "\
System manufacturer fileset installed\n"

$ ---------------------------------------------
$set MS_INUCP
$ These are the messages for the inucp command.
$ Message numbers 0503-620 through 0503-629.
$ ---------------------------------------------

INUCP_USAGE_E "\
Usage:  inucp -s start_dir [-e final_dir] listfile package\n\
\twhere \n\
\t\tstart_dir is the directory from which files \n\
\t\t\twill be copied.\n\
\t\tfinal_dir is the directory to which files \n\
\t\t\twill be copied.\n\
\t\tlistfile is a file containing relative paths of files\n\
\t\t\tto be copied. \n\
\t\tpackage is the name of the package to be copied.\n"
$ Do not translate inucp.
$ Do not translate -s and -e.

INUCP_WRG_FMT_W "\
inucp:  Warning. inucp expects the listfile to contain relative path names \n\
\tto the file. inucp will attempt to complete anyway.\n"

INUCP_INST_ROOT_I "\
\t\t\t Copying files from\n %s...\n"

$ ----------------------------------------------
$set MS_INUTOC
$ These are the messages for the inutoc command.
$ Message numbers 0503-630 through 0503-639.
$ ----------------------------------------------

INUTOC_BADIR_E "\
0503-630 inutoc:  %s either does not exist or is not a directory.\n"
$ Do not translate inutoc.

INUTOC_SYSTIME_W "\
inutoc:  Warning. Could not read the system time. \n\
\tThe toc header will contain 000000000000 in the timestamp \n\
\tlocation.\n"
$ Do not translate inutoc.

INUTOC_UNKNOWN_TYPE_W "\
%s:  Warning. The installation package image in %s\n\
\thas an unrecognized package type of \"%s\". This may indicate a\n\
\tbad image or a new package type requiring a newer version of the\n\
\tinstallation commands.\n\
\tThe installation image was added to the .toc file anyway.\n"

INUTOC_USAGE_E "\
Usage:  inutoc [dirname]\n\
\twhere dirname is the name of the directory for which a \n\
\t./.toc file will be created.\n"
$ Do not translate inutoc.
$ Do not translate ./.toc.

$
$set MS_INSTLCLIENT
$
$ Leave this set number in file for now because deletion may cause
$ errors in messages being displayed from script files (where set  
$ values could be wrong if they are changed without also making
$ changes to the scripts).
$


$ -------------
$set MS_STRINGS
$ -------------
UNKNOWN_STR       "UNKNOWN"
UPDT_3_1_STR      "3.1updt"
USR_STR           "USR    "
ROOT_STR          "ROOT   "
SHARE_STR         "SHARE  "
USR2_STR          "usr     "
ROOT2_STR         "root    "
SHARE2_STR        "share   "
BOTH_STR          "usr,root"
APPLY_STR         "APPLY      "
APPLYING_STR      "APPLYING   "
APPLIED_STR       "APPLIED    "
COMMIT_STR        "COMMIT     "
COMMITTING_STR    "COMMITTING "
COMMITTED_STR     "COMMITTED  "
REJECT_STR        "REJECT     "
REJECTING_STR     "REJECTING  "
CLEANUP_STR       "CLEANUP    "
DEINSTALL_STR     "DEINSTALL  "
DEINSTALLING_STR  "DEINSTALLING"
CANCELLED_STR     "CANCELLED  "
AVAILABLE_STR     "AVAILABLE  "
BROKEN_STR        "BROKEN     "
SUCCESS_STR       "SUCCESS    "
FAILURE_STR       "FAILED     "
$ all messages in this set cannot exceed the number of bytes in length
$ as what is defined here.


$ -----------------------------------------------
$set MS_INURECV
$ These are the messages for the inurecv command.
$ -----------------------------------------------

RCV_USAGE_E "\
Usage:  inurecv package [filesets_file]\n\
\twhere \n\
\t\tpackage is the name of the software package to be recovered.\n\
\t\tfilesets_file is the full path name of a file that specifies\n\
\t\t\twhich filesets within the package to recover.\n"
$ Do not translate inurecv.


$ -----------------------------------------------
$set MS_INURDSD
$ These are the messages for the inurdsd command.
$ -----------------------------------------------

RSD_USAGE_E "\
Usage:  inurdsd [-d dev] [-v v_n] [-s v_i] [-b blkn] [-n cnt] [-t n]\n\
\twhere \n\
\t\tdev is the name of the input device (default is /dev/rfd0).\n\
\t\tv_n is the volume of where to start reading (default is 1).\n\
\t\tv_i is the volume ID string mmddhhMMssyy (default is none).\n\
\t\tblkn is the first block number to read (default is 1).\n\
\t\tcnt is the number of characters to read (default is 9999999).\n\
\t\tn is the trace code digit, from 0 to 9 (default is 0).\n"


$ -------------------------------------------
$set MS_INUREST
$ These are the messages for inurest command.
$ Message numbers 0503-700 through 0503-709.
$ -------------------------------------------

RST_FAILURE_E "\
0503-700 inurest:  Error in restoring files\n"

RST_INFORM_WAIT_I "\
Restoring files, please wait.\n"

RST_PROGRESS_1_I "\
%d files out of %d restored\n"

RST_PROGRESS_2_I "\
%d files restored.\n"

RST_USAGE_E "\
Usage:  inurest [-d device] [-q] listfile package \n\
\twhere \n\
\t\tdevice is the input device.\n\
\t\tlistfile is a file containing relative paths of files to\n\
\t\t\tbe restored.\n\
\t\tpackage is the name of the software package to be restored.\n"
$ Do not translate inurest.
$ Do not translate -d and -q.

$ RST_FAILURE_TAPE_E is displayed in combination with the RST_FAILURE_E
$ Translators: check is used like examine for this message
RST_FAILURE_TAPE_E "\
The restore failure may be due to a tape error or SCSI bus reset.\n\
Please check your physical SCSI configuration, the tape, the tape drive,\n\
and then try again.\n"


$ -------------------------------------------
$set MS_INUSAVE
$ These are the messages for inusave command.
$ Message numbers 0503-710 through 0503-719.
$ -------------------------------------------

SAV_DELVPD_W "\
inusave:  Warning. Unable to delete an entry from the Software \n\
\tVital Product Data database.\n"
$ Do not translate inusave.

SAV_USAGE_E "\
Usage:  inusave listfile package \n\
\twhere \n\
\t\tlistfile is a file containing relative paths of files to \n\
\t\t\tbe saved.\n\
\t\tpackage is the name of the software package to be saved.\n"
$ Do not translate inusave.

SAV_VPDNF_E "\
0503-710 inusave:  Software Vital Product Data information was not\n\
\tfound for apply list entry %s.\n"
$ Do not translate inusave.

$set MS_ADD_DEL_VPD
ADEL_EXISTS_E "\
0503-960 %s:  %s already exists\n"
$ First %s is the name of the command, second %s is the fileset name

ADEL_ODMDIR_E "\
0503-961 %s:  ODMDIR not set\n"
$ %s is the name of the command.

ADEL_ODMDIR_USR_SHARE_E "\
0503-962 %s:  ODMDIR should be set to USR or SHARE instead of %s\n"
$ First %s is the name of the command, second %s is ODMDIR value.

ADEL_USAGE_E "\
\tUsage:  %s package fileset v.r.m.f\n"
$ %s is the name of the command.

ADEL_CREATE_REC_E "\
0503-963 %s:  Cannot create record for SWVPD file:  %s\n"
$ First %s is the name of the command.
$ Second %s is either "lpp" or "product".


ADEL_NOT_COMPAT_ENTRY_E "\
0503-964 %s:  %s is not a compatability entry or is missing\n"
$ First %s is the name of the command, second %s is the fileset name.

$set MS_CONVERT_VPD
CONVERT_USAGE_E "\
\tUsage:  %s <32_vpd_path> <41_vpd_path>\n"
$ %s is the name of the command.

CONVERT_ADDVPD_E "\
0503-970%s:  Could not merge %s database\n"
$ %s is the name of the command.
$ %s is the name of the vpd.

CONVERT_MIGRATE_USAGE_E "\
\tUsage:  %s <32_vpd_path> \n"
$ %s is the name of the command.

$ ---------------------------------------------------------
$set MS_GET_LICENSE_INFO
$ These are the messages for the get_license_info command.
$ Message numbers 0503-810 through 0503-825.
$ ---------------------------------------------------------

GLI_USAGE_E "\
 Usage:  get_license_info [-l][-f file]\n\
\tThis lists iFOR/LS-enabled products which have been registered\n\
\twith iFOR/LS server(s) on a network.\n\n\
 -f logs status of execution to a file (for communication with caller).\n\
 -l lists vendor names, vendor id's, product name, product id's and\n\
    product versions in a colon separated format.\n\
 default:\n\
    lists vendor id's, product id's and product version in a colon\n\
    separated format.\n"

GLI_NO_STATUS_FILE_E "\
0503-810 get_license_info:  No name for the status file was entered.\n"

GLI_INVALID_ARGUMENT_E "\
0503-811 get_license_info:  Invalid argument.\n"

GLI_LS_TV_ERROR_E "\
0503-812 get_license_info:  ls_tv did not return successfully.\n"

GLI_NO_SERVER_NAMES_FOUND_E "\
0503-813 get_license_info:  Could not get any server names from ls_tv.\n"

GLI_LS_ADMIN_FAILED_E "\
0503-814 get_license_info:  ls_admin failed as no vendor names were obtained.\n"

GLI_NO_VENDORS_FOUND_E "\
0503-815 get_license_info:  Could not get any vendors from the server.\n"


$ ---------------------------------------------------------
$set MS_INSTFIX
$ These are the messages for the instfix command.
$ Message numbers 0503-826 through 0503-840.
$ ---------------------------------------------------------

ISF_VPD_ERROR_E "\
0503-826: %s: Unknown VPD error occurred while processing the fix database.\n"
$ %s is the command name.

ISF_NO_KEYWORD_DATA_I "\
%s: There are no filesets on the media for %s.\n"
$ %s is the command name.
$ %s is a keyword from the command line or from an input file

ISF_NO_KEYWORD_LIST_I "\
%s: There are no filesets on the media for the requested Fix IDs.\n"
$ %s is the command name.

ISF_PROC_QUERY_I "\
\n\
%s: Searching for installed filesets associated with %s.\n\
\n"
$ %s is the command name.
$ %s is a keyword from the command line or from an input file

ISF_FILESET_NOT_FOUND_I "\
Fileset %s is not applied on the system.\n"
$ %s is a fileset name

ISF_FILESET_FOUND_I "\
Fileset %s is applied on the system.\n"
$ %s is a fileset name

ISF_ALL_FILESETS_FOUND_I "\
All filesets for %s were found.\n"
$ %s is a keyword from the command line or from an input file

ISF_NOT_ALL_FILESETS_FOUND_I "\
Not all filesets for %s were found.\n"
$ %s is a keyword from the command line or from an input file

ISF_NO_FIX_DATA_I "\
There was no data for %s in the fix database.\n"
$ %s is a keyword from the command line or from an input file

ISF_HELP_I_OLD "\
Usage: instfix [-T] [-s string] [ -k keyword | -f file ] [-p]\n\
          [-i[a]] [-d device] \n\
\n\
Function: Installs filesets associated with keywords or fixes.\n\
\n\
\t-T Display entire Table of Contents from the media.\n\
\t-s Search for and Display Table of Contents entries containing a string.\n\
\t-k Install filesets for a keyword or fix.\n\
\t-f Install filesets for multiple keywords or fixes using an input file.\n\
\t\tNote that the output of the -T option produces a suitable input\n\
\t\tfile format. -f- will result in instfix using standard input.\n\
\t-i Use with -k or -f option to display whether fixes or \n\
\t   keywords are installed.  This option is for information only.\n\
\t   Installation is not attempted when this option is used.\n\
\t-a Use only with -i to optionally display the symptom text associated \n\
\t   with a fix.\n\
\t-d Specify the input device (required for all but -i).\n\
\t-p Used with -k or -f to Print (Preview) filesets associated with \n\
\t   keywords.  This options is for information only.  Installation is \n\
\t   not attempted when this options is used.\n\
\n\
Examples:\n\
\n\
instfix -k IX38794 -d /dev/rmt0.1\n\
\twill install all filesets associated with fix IX38794 from the\n\
\ttape mounted on /dev/rmt0.1\n\
\n\
instfix -s SCSI -d /dev/rmt0.1\n\
\twill list all keyword entries on the tape containing the string \"SCSI\".\n\
\n\
instfix -i -k \"IX38794 IX48523\"\n\
\twill inform the user on whether fixes IX38794 and IX48523 are installed.\n\
\n\
\tNote: Available fixes are stored in the ODM fix database.\n"

ISF_ABSTRACT_I "\
%s Abstract: %s \n\n"
$ %s is the fix number (apar number).
$ %s is the abstract text which cannot be translated.

ISF_SYMPTOM_I "\
Symptom Text: \n%s \n\n"
$ %s is the untranslatable symptom text.

ISF_HELP_I "\
Usage: instfix [-T] [-s string] [ -k keyword | -f file ] [-p] [-d device]\n\
          [-i [-c] [-q] [-t type] [-v] [-F]] [-a] \n\
\n\
Function: Installs or queries filesets associated with keywords or fixes.\n\
\n\
\t-T Display fix information for complete fixes present on the media.\n\
\t-s Search for and display fixes on media containing a specified string.\n\
\t-k Install filesets for a keyword or fix.\n\
\t-f Input file containing keywords or fixes. Use '-' for standard input.\n\
\t\t The -T option produces a suitable input file format for -f.\n\
\t-p Use with -k or -f to print filesets associated with keywords.\n\
\t   Installation is not attempted when -p is used.\n\
\t-d Input device (required for all but -i and -a).\n\n\
\t-i Use with -k or -f option to display whether specified fixes or \n\
\t   keywords are installed.  Installation is not attempted.\n\
\t   If neither -k nor -i is specified, all known fixes are displayed.\n\
\t-c Colon-separated output for use with -i. Output includes keyword\n\
\t   name, fileset name, required level, installed level, status, and\n\
\t   abstract.  Status values are < (down level), = (correct level),\n\
\t   + (superseded), and ! (not installed).\n\
\t-q Quiet option for use with -i.  If -c is specified, no heading is \n\
\t   displayed.  Otherwise, no output is displayed.\n\
\t-t Use with -i option to limit search to a given type.  Currently\n\
\t   valid types are 'f' (fix) and 'p' (preventive maintenance).\n\
\t-v Verbose option for use with -i.  Gives information about each\n\
\t   fileset associated with a fix or keyword.\n\
\t-F Returns failure unless all filesets associated with the fix\n\
\t   are installed.\n\
\t-a Display the symptom text (can be combined with -i, -k, or -f).\n"

ISF_C_OUTPUT_HDR_I "\
#Keyword:Fileset:ReqLevel:InstLevel:Status:Abstract\n"

ISF_NO_RELEVANT_FILESETS_I "\
No filesets which have fixes for %s are currently installed.\n"

$ Do not translate instfix or any flag designations.
$ Do not translate commands given in examples.
 
