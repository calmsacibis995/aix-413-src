$ ENGLISH_SOURCE_VERSION= 1.3
$ @(#)81	1.1  src/msg/bos/base/dwbnword.msg, bosmsg, msg411, GOLD410 2/9/94 08:57:33


$
$ COMPONENT_NAME: (CMDTEXT) Text Formatting
$
$ FUNCTIONS: dwbnword.msg
$
$ ORIGINS: 3,27,28
$
$ This module contains IBM CONFIDENTIAL code. -- (IBM
$ Confidential Restricted when combined with the aggregated
$ modules for this product)
$                  SOURCE MATERIALS
$ (C) COPYRIGHT International Business Machines Corp. 1989
$ All Rights Reserved
$
$ US Government Users Restricted Rights - Use, duplication or
$ disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
$
$quote "         define quote character.

$len 
$set MS_nwords        Define initial set#
$ Do not translate output formats used with C programs such as:
$ 	%d %i %x %X %u %c %s %f %e %E %g %G %p %n %ld
$ Do not translate shell variables such as:
$ 	$# $? $$ $! $- {$0 $1 $2 .... $some number}

M_MSG_0  "style: 1004-501 Parts (part.l): Sentence %1$d has too many words.\n\
\tOnly the first %2$d words will be used.\n"
$ Refers to sentence number and words used.
M_MSG_1  "style: 1004-502 Parts (part.l): Sentence %1$d has too many characters.\n\
\tOnly the first %2$d characters will be used.\n"
$ Refers to sentence number and characters used.
M_MSG_8  "style: 1004-503 Cannot find or open %s.\n"
$ Refers to input file. 
M_MSG_10 "Readability grades:\n\
	(Kincaid) %4.1f  (auto) %4.1f  (Coleman-Liau) %4.1f  (Flesch) %4.1f (%4.1f)\n"
$ The above indices estimate the reading skills needed by the reader to 
$ understand a document. Where reading skills equal grade level (0-12).
M_MSG_11 "sentence information:\n"
M_MSG_12 "	number of sentence %1$d number of words  %2$ld\n"
M_MSG_13 "	average sentence length %4.1f average word length %4.2f\n"
M_MSG_14 "	number of questions %1$d number of imperatives %2$d\n"
M_MSG_15 " 	number of content words %ld  %4.1f%%   average length %4.2f\n"
M_MSG_16 " 	short sentence (<%d)%3.0f%% (%d) long sentence (>%d) %3.0f%% (%d)\n"
M_MSG_17 " 	longest sentence %1$d words at sentence %2$d; shortest sentence %3$d words at sentence %4$d\n"
M_MSG_18 " sentence types:\n"
M_MSG_19 " 	simple %3.0f%% (%d) complex %3.0f%% (%d)\n"
M_MSG_20 " 	compound %3.0f%% (%d) compound-complex %3.0f%% (%d)\n"
M_MSG_21 " word usage:\n"
M_MSG_22 " 	verb types as %% of total verbs (%d)\n"
M_MSG_23 " 	tobe %3.0f%% (%d) auxiliary %3.0f%% (%d) infinitives %3.0f%% (%d)\n"
M_MSG_24 " 	passives as %% of non-infinitives verbs %3.0f%% (%d)\n"
M_MSG_25 " 	types as %% of total\n"
M_MSG_26 " 	prepositional %3.1f%% (%d) conjunctions %3.1f%% (%d) adverbs %3.1f%% (%d)\n"
M_MSG_27 " 	noun %3.1f%% (%d) adjectives %3.1f%% (%d) pronouns %3.1f%% (%d)\n"
M_MSG_28 " 	nominalizations %3.0f %% (%d)\n"
M_MSG_29 " sentence beginnings:\n"
M_MSG_30 " 	subject opener: noun (%d) pronoun (%d) possessive (%d) adjective (%d) article (%d) tot %3.0f%%\n"
M_MSG_31 " 	preposition %3.0f%% (%d) adverbs %3.0f%% (%d) \n"
M_MSG_32 " 	verb %3.0f%% (%d) "
M_MSG_33 "  sub_conjunctions %3.0f%% (%d) conjunctions %3.0f%% (%d)\n"
M_MSG_34 " 	expletives %3.0f%% (%d)\n"
M_MSG_43 "style: 1004-537 The %s is not valid for part.l\n"
