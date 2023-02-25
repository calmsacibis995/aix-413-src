/* @(#)57	1.2  src/bos/kernext/dlc/lan/lanhasht.h, sysxdlcg, bos411, 9428A410j 10/19/93 11:21:05 */
/*
 * COMPONENT_NAME: (SYSXDLCG) Generic Data Link Control
 *
 * FUNCTIONS: lanhasht.h
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1989, 1993
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
#ifndef _h_LANHASHT
#define _h_LANHASHT

/**********************************************************************/
/* receive hashing constants declaration */
/**********************************************************************/
u_char rcv_hash_tbl[] = {                                       
        0x00, 0x2c, 0x58, 0x74, 0xb0, 0x9c, 0xe8, 0xc4,         
        0x7d, 0x51, 0x25, 0x09, 0xcd, 0xe1, 0x95, 0xb9,         
        0xfa, 0xd6, 0xa2, 0x8e, 0x4a, 0x66, 0x12, 0x3e,         
        0x87, 0xab, 0xdf, 0xf3, 0x37, 0x1b, 0x6f, 0x43,         
        0xe9, 0xc5, 0xb1, 0x9d, 0x59, 0x75, 0x01, 0x2d,         
        0x94, 0xb8, 0xcc, 0xe0, 0x24, 0x08, 0x7c, 0x50,         
        0x13, 0x3f, 0x4b, 0x67, 0xa3, 0x8f, 0xfb, 0xd7,         
        0x6e, 0x42, 0x36, 0x1a, 0xde, 0xf2, 0x86, 0xaa,         
        0xcf, 0xe3, 0x97, 0xbb, 0x7f, 0x53, 0x27, 0x0b,         
        0xb2, 0x9e, 0xea, 0xc6, 0x02, 0x2e, 0x5a, 0x76,         
        0x35, 0x19, 0x6d, 0x41, 0x85, 0xa9, 0xdd, 0xf1,         
        0x48, 0x64, 0x10, 0x3c, 0xf8, 0xd4, 0xa0, 0x8c,         
        0x26, 0x0a, 0x7e, 0x52, 0x96, 0xba, 0xce, 0xe2,         
        0x5b, 0x77, 0x03, 0x2f, 0xeb, 0xc7, 0xb3, 0x9f,         
        0xdc, 0xf0, 0x84, 0xa8, 0x6c, 0x40, 0x34, 0x18,         
        0xa1, 0x8d, 0xf9, 0xd5, 0x11, 0x3d, 0x49, 0x65,         
        0x83, 0xaf, 0xdb, 0xf7, 0x33, 0x1f, 0x6b, 0x47,         
        0xfe, 0xd2, 0xa6, 0x8a, 0x4e, 0x62, 0x16, 0x3a,         
        0x79, 0x55, 0x21, 0x0d, 0xc9, 0xe5, 0x91, 0xbd,         
        0x04, 0x28, 0x5c, 0x70, 0xb4, 0x98, 0xec, 0xc0,         
        0x6a, 0x46, 0x32, 0x1e, 0xda, 0xf6, 0x82, 0xae,         
        0x17, 0x3b, 0x4f, 0x63, 0xa7, 0x8b, 0xff, 0xd3,         
        0x90, 0xbc, 0xc8, 0xe4, 0x20, 0x0c, 0x78, 0x54,         
        0xed, 0xc1, 0xb5, 0x99, 0x5d, 0x71, 0x05, 0x29,         
        0x4c, 0x60, 0x14, 0x38, 0xfc, 0xd0, 0xa4, 0x88,         
        0x31, 0x1d, 0x69, 0x45, 0x81, 0xad, 0xd9, 0xf5,         
        0xb6, 0x9a, 0xee, 0xc2, 0x06, 0x2a, 0x5e, 0x72,         
        0xcb, 0xe7, 0x93, 0xbf, 0x7b, 0x57, 0x23, 0x0f,         
        0xa5, 0x89, 0xfd, 0xd1, 0x15, 0x39, 0x4d, 0x61,         
        0xd8, 0xf4, 0x80, 0xac, 0x68, 0x44, 0x30, 0x1c,         
        0x5f, 0x73, 0x07, 0x2b, 0xef, 0xc3, 0xb7, 0x9b,         
        0x22, 0x0e, 0x7a, 0x56, 0x92, 0xbe, 0xca, 0xe6
};        
#endif /* _h_LANHASHT */
