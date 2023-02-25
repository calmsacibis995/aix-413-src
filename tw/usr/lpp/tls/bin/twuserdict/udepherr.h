/* @(#)75	1.1  src/tw/usr/lpp/tls/bin/twuserdict/udepherr.h, tw, tw411, GOLD410 7/10/92 14:01:43 */
/*
 * COMPONENT_NAME :     (CMDTW) - Traditional Chinese Dictionary Utility
 *
 * FUNCTIONS :          udepherr.h
 *
 * ORIGINS :            27
 *
 * (C) COPYRIGHT International Business Machines Corp.  1991, 1992
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Error message for phonetic dictionary editor */

#ifdef ENGLISH

#define MSG_ERR_WR        "Dictionary Read/Write Error !"
#define MSG_ERR_PHRASE    "Phrase codes are error !"
#define MSG_ERR_PHONETIC  "Phonetic codes are error !"
#define MSG_EX_IN_SYS     "word is in system dictionary !"
#define MSG_EX_IN_USR     "word is in user dictionary !"
#define MSG_ERR_GF        "Group was full !"
#define MSG_EX_IN_NON     "word is not in user dictionary !"
#define MSG_SAME_US       "User and system dictionary have same name !"
#define MSG_SAME_MS       "Merge and system dictionary have same name !"
#define MSG_SAME_UM       "User and merge dictionary have same name !"
#define MSG_ERR_IF        "Invalid dictionary format !"
#define MSG_ERR_AP        "Dictionary Access not permitted !"
#define MSG_ERR_OP        "dictionary open error !"
#define MSG_ERR_IV        "dictionary name is invalid"
#define MSG_ERR_SO        "system dictionary open error !"
#define MSG_ERR_MF        "MI was full !"
#define MSG_ERR_IC        "data of dictionary was inconsistent !"
#define MSG_ERR_LOCK      "Dictionary had locked by another process !"
#define MSG_ERR_UNLOCK    "Failed to unlock dictionary !"
#define MSG_ERR_PHRASE_EMPTY    "Please input phrase code !"
#define MSG_ERR_PHONETIC_EMPTY  "Please input phonetic code !"
#define MSG_ERR_EMPTY     "No highlight phrase was selected !"
#define MSG_PHONETIC_LONG "Phonetic code too long !"
#define MSG_PHRASE_LONG   "Phrase code too long !"
#define MSG_PHRASE_SYS_DUP      "All phrase are duplicate with system dict!"
#define MSG_FILE_EMPTY    "Please input file name !"
#else

#define MSG_ERR_WR        "������������/����� !"
#define MSG_ERR_PHRASE    "��������������� !"
#define MSG_ERR_PHONETIC  "��������������� !"
#define MSG_EX_IN_SYS     "����������˷���������������� !"
#define MSG_EX_IN_USR     "������������������������������ !"
#define MSG_ERR_GF        "���������������������������."
#define MSG_EX_IN_NON     "����������˷���������������������� !"
#define MSG_SAME_US       "������������������˷����������������������� !"
#define MSG_SAME_MS       "��������������������˷��������������������� !"
#define MSG_SAME_UM       "��������������������������������������������� !"
#define MSG_ERR_IF        "������������������֪Ȣ !"
#define MSG_ERR_AP        "��ݷ������̽������������������ !"
#define MSG_ERR_OP        "����������������������� !"
#define MSG_ERR_IV        "������������������� !"
#define MSG_ERR_SO        "˷������������������� !"
#define MSG_ERR_MF        "������������������Ϩ������,�������������������ī !"
#define MSG_ERR_IC        "�����������������ݷ̧���ʩԺ��,��ȩ�������� !"
#define MSG_ERR_LOCK      "��������������������ݷ̧���ʩ���� !"
#define MSG_ERR_UNLOCK    "���������������������������� !"
#define MSG_ERR_PHRASE_EMPTY   " ��ǿ��ī���� !"
#define MSG_ERR_PHONETIC_EMPTY " ��ǿ��ī���� !"
#define MSG_ERR_EMPTY     "��ǿ����������������ġԶ��� !"
#define MSG_PHONETIC_LONG "�������Ŵ�� !"
#define MSG_PHRASE_LONG   "�������Ŵ�� !"
#define MSG_PHRASE_SYS_DUP     "��������������������,��������˷��������� !"
#define MSG_FILE_EMPTY    "��ǿ��ī������������� !"

#endif
/* DON'T ADD ANYTHING AFTER THIS #endif */
