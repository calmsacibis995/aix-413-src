/* @(#)18	1.5  src/tenplus/helpers/dir/statmod.h, tenplus, tenplus411, GOLD410 7/18/91 12:40:43 */

/*
 * COMPONENT_NAME: (INED) INed Editor
 * 
 * ORIGINS:  9, 10
 * 
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * 
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
 
/*****************************************************************************
* File: statmod.sh - defines for accessing stat information.                 *
*****************************************************************************/

/* Macros for determining the type of a file */

#define is_file(f)      (ST_TYPE( f ) != -1)
#define is_not_file(f)  (ST_TYPE( f ) == -1)
#define is_directory(d) (ST_TYPE( d ) == 4)
#define is_regular(f)   (!(ST_TYPE( f ) & 07))


/* Macros for determining the accessibility of a file */
#define is_executable(f) check_bits( f, S_IEXEC )
#define is_readable(f)   check_bits( f, S_IREAD )
#define is_writable(f)   check_bits( f, S_IWRITE )




/* Macros updating stat information */

#define unstat(f)  if (f == NULL || (strcmp( f, Last_stat_file)==0))  \
		       Last_stat_file[0] = '\0';


#define _upstat(f) ((strcmp( f, Last_stat_file )==0) ? Result_of_stat :      \
		       (((f == NULL) || (*f == '\0')) ? -1 :             \
			  (strcpy( Last_stat_file, f),               \
			   Result_of_stat = stat( f, &Stat_buffer ))))



/* Macros for accessing stat information */

#define _file_ok( f ) (_upstat( f ) == 0)

#define ST_MODE(f)  (_file_ok( f ) ? Stat_buffer.st_mode  : ((ushort) ERROR))
#define ST_INO(f)   (_file_ok( f ) ? Stat_buffer.st_ino   : ((ino_t) ERROR))
#define ST_DEV(f)   (_file_ok( f ) ? Stat_buffer.st_dev   : ((dev_t) ERROR))
#define ST_RDEV(f)  (_file_ok( f ) ? Stat_buffer.st_rdev  : ((dev_t) ERROR))
#define ST_NLINK(f) (_file_ok( f ) ? Stat_buffer.st_nlink : ((short) ERROR))
#define ST_UID(f)   (_file_ok( f ) ? Stat_buffer.st_uid   : ((ushort) ERROR))
#define ST_GID(f)   (_file_ok( f ) ? Stat_buffer.st_gid   : ((ushort) ERROR))
#define ST_SIZE(f)  (_file_ok( f ) ? Stat_buffer.st_gid   : ((off_t) ERROR))
#define ST_ATIME(f) (_file_ok( f ) ? Stat_buffer.st_atime : ((time_t) ERROR))
#define ST_MTIME(f) (_file_ok( f ) ? Stat_buffer.st_mtime : ((time_t) ERROR))
#define ST_CTIME(f) (_file_ok( f ) ? Stat_buffer.st_ctime : ((time_t) ERROR))
#define ST_TYPE(f)  (_file_ok( f ) ?                                       \
		    ((signed)((unsigned)(Stat_buffer.st_mode & S_IFMT))>>12):  \
		    (-1))
