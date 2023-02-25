$ ENGLISH_SOURCE_VERSION= 1.4
$ @(#)50        1.1 2/9/94 09:01:24
$
$ COMPONENT_NAME: CMDSCCS       source code control system
$
$ FUNCTIONS:
$
$ ORIGINS: 27
$
$ IBM CONFIDENTIAL -- (IBM Confidential Restricted when
$ combined with the aggregated modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$

$quote %
$set 49
1 %\tList of Data Keywords Recognized by the "prs" Command\n\
\n\
 Data                             SCCS File          Value         Format\n\
Keyword       Data Item            Section           Type           Type\n\n%

2 % :Dt:   Whole delta line       Delta Table   Delta information   Single\n\
 :DL:   Delta line statistics         "         :Li:/:Ld:/:Lu:       S\n\
 :Li:   Lines inserted by Delta       "              nnnnn           S\n\
 :Ld:   Lines deleted by Delta        "              nnnnn           S\n\
 :Lu:   Lines unchanged by Delta      "              nnnnn           S\n\
 :DT:   Delta type                    "              D or R          S%
3 %  :I:   SCCS ID string (SID)          "          :R:.:L:.:B:.:S:     S\n\
  :R:   Release number                "              nnnn            S\n\
  :L:   Level number                  "              nnnn            S\n\
  :B:   Branch number                 "              nnnn            S\n\
  :S:   Sequence number               "              nnnn            S%
4 %  :D:   Date Delta created            "          :Dy:/:Dm:/:Dd:      S\n\
 :Dy:   Year Delta created            "               nn             S\n\
 :Dm:   Month Delta created           "               nn             S\n\
 :Dd:   Day Delta created             "               nn             S\n\
  :T:   Time Delta created            "          :Th:::Tm:::Ts:      S%
5 % :Th:   Hour Delta created            "               nn             S\n\
 :Tm:   Minute Delta created          "               nn             S\n\
 :Ts:   Second Delta created          "               nn             S\n\
  :P:   Programmer who created Delta  "             logname          S\n\
 :DS:   Delta sequence number         "              nnnn            S%
6 % :DP:   Previous Delta number         "              nnnn            S\n\
 :DI:   Deltas incl,excl,ignored      "         :Dn:/:Dx:/:Dg:       S\n\
 :Dn:   Deltas included               "             :DS: ...         S\n\
 :Dx:   Deltas excluded               "             :DS: ...         S\n\
 :Dg:   Deltas ignored                "             :DS: ...         S\n\
 :MR:   MR numbers for Delta          "              text            Multiline%
7 %  :C:   Comments for Delta            "              text            M\n\
 :UN:   User names                User Names         text            M\n\
 :FL:   Header flag list            Flags            text            M\n\
  :Y:   Module type flag              "              text            S\n\
 :MF:   MR validation flag            "            yes or no         S%
8 % :MP:   MR validation pgm.            "              text            S\n\
 :KF:   Kywd error/warning flag       "            yes or no         S\n\
 :BF:   Branch flag                   "            yes or no         S\n\
  :J:   Joint edit flag               "            yes or no         S\n\
 :LK:   Locked releases               "             :R: ...          S%
9 %  :Q:   User defined keyword          "              text            S\n\
  :M:   Module name flag              "           module name        S\n\
 :FB:   Floor boundary                "               :R:            S\n\
 :CB:   Ceiling boundary              "               :R:            S\n\
 :Ds:   Default SID                   "               :I:            S%
10 % :ND:   Null delta flag set           "            yes or no         S\n\
 :FD:   Descriptive text           Comments          text            M\n\
 :BD:   Body                         Body            text            M\n\
 :GB:   Text in a g-file              "              text            M\n\
  :W:   A "what" string              N/A          :Z::M:\\t:I:      S%
11 %  :A:   A "what" string              N/A       :Z::Y: :M: :I::Z:     S\n\
  :Z:   A "what" string delimiter    N/A             @(#)            S\n\
  :F:   SCCS file name               N/A             text            S\n\
 :PN:   SCCS path name               N/A           path name         S%

