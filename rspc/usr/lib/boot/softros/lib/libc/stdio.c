static char sccsid[] = "@(#)08	1.2  src/rspc/usr/lib/boot/softros/lib/libc/stdio.c, rspc_softros, rspc411, 9432A411a 8/5/94 16:38:45";
/*
 *   COMPONENT_NAME: RSPC_SOFTROS
 *
 *   FUNCTIONS: CONVERT
 *		INCHAR
 *		OUTCHAR
 *		_Inchar
 *		_Outchar
 *		_Printf
 *		_Scanf
 *		getchar
 *		gets
 *		perror
 *		printf
 *		putchar
 *		puts
 *		scanf
 *		sprintf
 *		sscanf
 *
 *   ORIGINS: 27
 *
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1994
 *   All Rights Reserved
 *   Licensed Materials - Property of IBM
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
#ifdef DEBUG

#include <ctype.h>
#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <errno.h>
#include <types.h>
#include <string.h>

#include <stand.h>


/* Internal functions: */

static void _Outchar( char c, long addr );      /* print char out routine */
static char _Inchar( long addr );               /* scanf char in routine */
                                                /* print/scan engines: */
static int  _Printf( const char *format, va_list ap, long where );
static int  _Scanf( const char *format, va_list ap, long where );

/*-----------------------------------------------------------------------
 *  External function declarations:
 */

extern void ConsoleWrite( int fildes, char c ); /* console write routine */
extern char ConsoleRead( int fildes );          /* console read routine */

/*-----------------------------------------------------------------------
 *  I/O functions:
 */

int putchar( int c )
    {
        char output_char;

        output_char = c;
        write( stdout, &output_char, 1);        /* Output char to stdout */             return( c );
    }

int getchar( void )
    {
        char c;
        int returned_char;

        read( stdin, &c, 1);            /* Input char from stdin */
        returned_char = c;
        return( returned_char );
    }

int puts( const char *s )
    {
        register char *str = (char *)s;
        int     newline = '\n';
                                                /* write up to NULL */
        while ( *str != '\0' )
        {
            write( stdout, str, 1);
            str++;
        }
        write( stdout, &newline, 1);
        return( str - s + 1 );                  /* return # chars written */
    }

char *gets( char *s )
    {
        register char *str = (char *)s;
        char c;
        int cnt = 0;

        c = getchar();
        while( c != '\n')                       /* until newline read   */
        {
            if ((c == 0x04) || (c == 0xff))     /* or EOF               */
              {
              str = s;             /* Reset to beginning of string      */
              *str++ = EOF;
              break;
              }

            if (c == 0x08)         /* Is it a back space?                    */
              {
              if (cnt)             /* If we aren't at the beginning then...  */
                {
                cnt--;             /* Forget one character                   */
                str--;             /* Backup to write over the one character */
                putchar(' ');      /* Delete the character                   */
                putchar(0x08);     /*   and move the cursor back             */
                }
              }
            else
                {
                *str++ = c;                     /* copy to string       */
                cnt++;
                }
            c = getchar();
        }
        *str = '\0';                            /* append end null      */
        return( s );                            /* return string        */
    }

int errno;                                      /* as good a place as any */

void perror( const char *s )
    {
        register char   *str;

        if ( *s != '\0' )                       /* if not null string */
            printf("%s: ", s);                  /* print it out */
        str = strerror( errno );                /* decode errno */

        printf("%s\n", str);                    /* print error string */
    }


/*-----------------------------------------------------------------------
 *  Print/scan functions:
 */

int printf( const char *format, ... )
    {
        va_list ap;
        register int    rc;                     /* return code */

        va_start( ap, format );                 /* set ap to 1st unnamed arg */
        rc = _Printf( format, ap, (long)NULL);  /* print to stdout */
        va_end( ap );                           /* cleanup */

        return( rc );                           /* return count */
    }


int sprintf( char *s, const char *format, ... )
    {
        va_list ap;
        register int    rc;                     /* return code */

        va_start( ap, format );                 /* set ap to 1st unnamed arg */
        rc = _Printf( format, ap, (long)&s);    /* print to memory */
        va_end( ap );                           /* cleanup */
        *s = '\0';                              /* add string terminator */

        return( rc );                           /* return count */
    }

int scanf( const char *format, ... )
    {
        va_list ap;
        register int    rc;                     /* return code */

        va_start( ap, format );                 /* set ap to 1st unnamed arg */
        rc = _Scanf( format, ap, (long)NULL);   /* read from stdin */
        va_end( ap );                           /* cleanup */

        return( rc );                           /* return count */
    }

int sscanf( const char *s, const char *format, ... )
    {
        va_list ap;
        register int    rc;                     /* return code */

        va_start( ap, format );                 /* set ap to 1st unnamed arg */
        rc = _Scanf( format, ap, (long)&s);     /* print from memory */
        va_end( ap );                           /* cleanup */

        return( rc );                           /* return count */
    }


/*-----------------------------------------------------------------------
 *  PRINT ENGINE:
 *
 *  This internal function "_Printf" performs POSIX-compliant printf
 *  functionality minus floating-point conversions.  The following
 *  directives are supported:
 *
 *      flags:          -       left justify
 *                      +       positive numbers signed with '+'.
 *                      space   positive numbers signed with space.
 *                      #       0x, 0X, (hex) or 0 (octal) preface.
 *                      0       pad with zeroes (non-POSIX extension)
 *      options:        h       convert to short
 *                      l       convert to long
 *      formats:        i, d    decimal string
 *                      o       octal string
 *                      u       unsigned decimal string
 *                      x, X    hexadecimal string
 *                      c       character
 *                      s       string
 *                      p       pointer
 *                      n       return current count.
 *
 *  The floating point directives 'f', 'e', 'E', 'g', and 'G' are unsupported.
 *
 *  Output from _Printf occurs thru the macro OUTCHAR, which maintains the
 *  character count and calls a low-level character routine to output
 *  characters either to a device or to memory.  The variable "where"
 *  can contain a value or an address of a state block used by the
 *  low-level output routine.
 *
 */

# define CONVERT( val, type )                                   \
        if (( type ) == 'h' ) (val) = (short)(val);             \
            else if (( type ) == 'l' ) (val) = (long)(val);     \
                else (val) = (int)(val)

/* Max digits is calculated to hold the longest signed binary string */

#define MAXDIGITS       (1 + sizeof( long ) * 8)

#define OUTCHAR( c )                                            \
        count++, _Outchar(( c ), ( where ))

static int _Printf( const char *format, va_list ap, long where )
    {
        char            c, *p, *sval;
        register long   n;
        unsigned long   m;
        register int    i, len, d, left_justify, pound;
        register int    prec_mode, precision, width, gap, count = 0;
        register char   pad, type, sign;
        char            digits[ MAXDIGITS ];

        for ( p = (char *)format; *p; p++ )     /* for each char in string */
            {
            if ( *p != '%' )                    /* not a conversion? */
                {
                OUTCHAR( *p );                  /* output character */
                continue;                       /* next victim */
                }
                                                /* Conversion Defaults: */
            left_justify = FALSE;               /* right justify */
            pound        = FALSE;               /* no alternates */
            sign         = '\0';                /* no sign for positives */
            width        = 0;                   /* width not specified */
            precision    = 0;                   /* precision not specified */
            prec_mode    = FALSE;               /* start in width mode */
            pad          = ' ';                 /* default pad is space */
            type         = 'i';                 /* default type is int */
            i            = 0;                   /* counter is zero */

            while ( *++p )                      /* conversion loop */
                {
                switch ( *p )
                    {                           /* OPTIONS: */
                    case '-':                   /* left-justify */
                        left_justify = TRUE;
                        continue;

                    case '+':                   /* use plus sign */
                        sign = '+';
                        continue;

                    case ' ':                   /* use space as plus */
                        if ( sign != '+' )
                            sign = ' ';
                        continue;

                    case '#':                   /* alternate form */
                        pound = TRUE;
                        continue;

                    case 'h':                   /* type conversion */
                    case 'l':
                    case 'L':
                        type = *p;
                        continue;

                    case '.':                   /* precision */
                        prec_mode = TRUE;
                        pad = '0';              /* use zeroes */
                        continue;

                    case '0':                   /* width or precision */
                        if (!( width + precision ))
                            pad = '0';          /* pad specification */
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        if ( prec_mode )
                            precision = precision * 10 + ( *p - '0' );
                        else
                            width = width * 10 + ( *p - '0' );
                        continue;

                    case 'n':                   /* number chars written */
                        *(va_arg( ap, long *)) = count;
                        break;                  /* exit conversion loop */

                    case 'd':                   /* DECIMAL */
                    case 'i':
                    case 'u':

                        n = va_arg( ap, long );
                        CONVERT( n, type );     /* type conversion */

                        if ( pad == '0' && !width && !n )
                            continue;           /* if 0 width, n = 0; skip */

                        if ( n < 0 )            /* negative? */
                            sign = '-', n = -n;

                        if ( *p == 'u' )
                            sign = '\0';        /* no sign if unsigned */

                        do  {                   /* fill string */
                            digits[ i++ ] = (char)( n % 10 + '0' );
                            n = n / 10;
                            } while ( n );

                        if ( sign )             /* need sign? */
                            digits[ i++ ] = sign;

                        width = MAX( precision, MAX( width, i ));

                        if ( left_justify )     /* left justify? */
                            {
                            gap = width - i;    /* determine gap */
                            len = i;            /* length of string */
                            i += gap;           /* include gap */
                            while ( len-- )     /* move chars left */
                                digits[ len + gap ] = digits[ len ];
                            while ( gap-- )     /* add pad */
                                digits[ gap ] = pad;
                            }
                        else                    /* right justify */
                            while ( i < width )
                                digits[ i++ ] = pad;

                        while ( i-- )           /* spit it out */
                            OUTCHAR( digits[ i ] );
                        break;                  /* exit conversion loop */

                    case 'x':                   /* HEXADECIMAL */
                    case 'X':
                    case 'p':
                        n = va_arg( ap, int );
                        CONVERT( n, type );     /* type conversion */

                        if ( pad == '0' && !width && !n )
                            continue;           /* if 0 width, n = 0; skip */

                        m = (unsigned long)n;   /* unsigned version */

                        do  {                   /* fill string */
                            d = m & 0xF;        /* nibble at a time */
                            if ( d > 9 )        /* a - f, A - F */
                                digits[ i++ ] =
                                    (char)( d - 10 + ( *p == 'X' ? 'A' : 'a' ));
                            else                /* 0 - 9 */
                                digits[ i++ ] = (char)( d + '0' );
                            m = m >> 4;
                            } while ( m );

                        if ( n && pound )       /* 0x needed? */
                            {
                            digits[ i++ ] = ( *p == 'X' ? 'X' : 'x' );
                            digits[ i++ ] = '0';
                            }

                        width = MAX( precision, MAX( width, i ));

                        if ( left_justify )     /* left justify? */
                            {
                            gap = width - i;    /* determine gap */
                            len = i;            /* length of string */
                            i += gap;           /* include gap */
                            while ( len-- )     /* move chars left */
                                digits[ len + gap ] = digits[ len ];
                            while ( gap-- )     /* add pad */
                                digits[ gap ] = pad;
                            }
                        else                    /* right justify */
                            while ( i < width )
                                digits[ i++ ] = pad;

                        while ( i-- )           /* spit it out */
                            OUTCHAR( digits[ i ] );
                        break;                  /* exit conversion loop */

                    case 'o':                   /* OCTAL */
                        n = va_arg( ap, int );
                        CONVERT( n, type );     /* type conversion */

                        if ( pad == '0' && !width && !n )
                            continue;           /* if 0 width, n = 0; skip */

                        m = (unsigned long)n;   /* unsigned version */

                        do  {                   /* fill string */
                            d = m & 0x7;        /* three bits at a time */
                            digits[ i++ ] = (char)( d + '0' );
                            m = m >> 3;
                            } while ( m );

                        if ( pound )            /* force leading zero? */
                            digits[ i++ ] = '0';

                        width = MAX( precision, MAX( width, i ));

                        if ( left_justify )     /* left justify? */
                            {
                            gap = width - i;    /* determine gap */
                            len = i;            /* length of string */
                            i += gap;           /* include gap */
                            while ( len-- )     /* move chars left */
                                digits[ len + gap ] = digits[ len ];
                            while ( gap-- )     /* add pad */
                                digits[ gap ] = pad;
                            }
                        else                    /* right justify */
                            while ( i < width )
                                digits[ i++ ] = pad;

                        while ( i-- )           /* spit it out */
                            OUTCHAR( digits[ i ] );
                        break;                  /* exit conversion loop */

                    case 's':                   /* STRING */
                        sval = va_arg( ap, char * );
                        if (!sval)
                            break;              /* no string? punt */
                        len = strlen( sval );   /* get length */
                        if ( precision )        /* truncate string? */
                            len = MIN( len, precision );

                        if ( !width )           /* width not specified? */
                            width = len;        /* then don't limit */

                        if ( left_justify )     /* left justify? */
                            {                   /* output string */
                            while ( *sval && width-- )
                                OUTCHAR( *sval++ );
                            if ( width > len )  /* right padding needed? */
                                while ( width-- > len )
                                    OUTCHAR( pad );
                            }
                        else
                            {
                            if ( width > len )  /* left padding needed? */
                                while ( width-- > len )
                                    OUTCHAR( pad );
                            width++;
                            while ( *sval && width-- )
                                OUTCHAR( *sval++ );
                            }
                        break;                  /* exit conversion loop */

                    case 'c':                   /* CHARACTER */
                        c = va_arg( ap, char );
                        width = MAX( 1, width );
                        if ( left_justify )     /* left justify? */
                            {
                            OUTCHAR( c );
                            while( --width )
                                OUTCHAR( pad );
                            }
                        else
                            {
                            while( --width )
                                OUTCHAR( pad );
                            OUTCHAR( c );
                            }

                        break;                  /* exit conversion loop */

                    case '\0':                  /* premature string end */
                        return( -1 );           /* so error */

                    default:
                        OUTCHAR( *p );          /* default, no conversion */
                        break;                  /* exit conversion loop */
                    }                           /* end switch */
                    break;                      /* exit conversion loop */
                }                               /* end conversion loop */
            }                                   /* end char read loop */
        return( count );                        /* return char count */
    }


static void _Outchar( char c, long addr )
    {
        register char   *s;

        if ( addr == (long)NULL )               /* no address? */
            write( stdout, &c, 1);              /* out to console */
        else
            {
            s = (char *)*(long *)addr;          /* get string address */
            *s++ = c;                           /* store character */
            *(long *)addr = (long)s;            /* update address */
            }
    }


/*-----------------------------------------------------------------------
 *  SCAN ENGINE:
 *
 *  This internal function "_Scanf" performs POSIX-compliant scanf
 *  functionality minus floating-point conversions.  The following
 *  directives are supported:
 *
 *      flags:          -       left justify
 *                      +       positive numbers signed with '+'.
 *                      space   positive numbers signed with space.
 *                      #       0x, 0X, (hex) or 0 (octal) preface.
 *                      0       pad with zeroes (non-POSIX extension)
 *      options:        h       convert to short
 *                      l       convert to long
 *      formats:        i, d    decimal string
 *                      o       octal string
 *                      u       unsigned decimal string
 *                      x, X    hexadecimal string
 *                      c       character
 *                      s       string
 *                      p       pointer
 *                      n       return current count.
 *
 *  The floating point directives 'f', 'e', 'E', 'g', and 'G' are unsupported,
 *  however, precision fields are parsed (if preceded by .) should floating
 *  point support be added to this code.
 *
 *  Input to _Scanf occurs thru the macro INCHAR, which calls a low-level
 *  character routine to input characters either from a device or from
 *  memory.  The variable "where" can contain a value or an address of a
 *  state block used by the low-level output routine.
 *
 */

# define INCHAR()       _Inchar( where ); if ( c == EOF ) return(EOF)

# define MAXDEC         ( sizeof( long ) * 8 / 3 )
# define MAXOCT         ( 1 + sizeof( long ) * 8 / 3 )
# define MAXHEX         ( sizeof( long ) * 8 / 4 )
# define MAXSTRING      1024
# define MAXSCANSET     ( '~' - ' ' + 2 )

# define DECIMAL        1
# define OCTAL          2
# define HEX            3

static int _Scanf( const char *format, va_list ap, long where )
    {
        char            c, *p, *str;
        int             *ptr, mode;
        register long   n;
        unsigned long   m;
        register int    i, j, limit, width, sign;
        register int    negate, suppress, count = 0;
        register char   type;
        char            digits[ MAXDIGITS ];
        char            scanset[ MAXSCANSET ];  /* all graph chars */

        c = INCHAR();                           /* prime the pump */
        for ( p = (char *)format; *p; p++ )     /* for each char in string */
            {
            if ( *p != '%' )                    /* not a conversion? */
                {
                if ( c == *p )                  /* if match, continue */
                    {
                    c = INCHAR();               /* next character */
                    continue;
                    }
                else
                    return( count );            /* doesn't match, end */
                }
                                                /* Conversion Defaults: */
            suppress = FALSE;                   /* no alternates */
            width    = 0;                       /* width not specified */
            type     = 'i';                     /* default type is int */
            mode     = DECIMAL;                 /* default decimal integer */
            limit    = MAXDEC;                  /* max number of digits */
            negate   = FALSE;                   /* "^" on scan set */
            sign     = 1;
            i        = 0;                       /* counter is zero */
            m        = 0;                       /* value is zero */

            while ( *++p )                      /* conversion loop */
                {
                switch ( *p )
                    {                           /* OPTIONS: */
                    case '*':                   /* suppress assignment */
                        suppress = TRUE;
                        continue;

                    case 'h':                   /* type conversion */
                    case 'l':
                    case 'k':
                    case 'L':
                        type = *p;
                        continue;

                    case '0':                   /* width  */
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        width = width * 10 + ( *p - '0' );
                        continue;

                    case 'o':                   /* OCTAL */
                        mode = OCTAL;
                        limit = MAXOCT;         /* maximum octal digits */
                        goto readin;            /* God, I hate gotos . . . */
                    case 'x':                   /* HEX */
                        mode = HEX;
                        limit = MAXHEX;         /* maximum hex digits */
                        goto readin;
                    case 'd':                   /* DECIMAL */
                    case 'u':
                        mode = DECIMAL;
                        limit = MAXDEC;         /* maximum decimal digits */
readin:
                    case 'i':                   /* INTEGER */
                        count++;                /* another one bytes the dust */
                        while( isspace( c ))
                            c = INCHAR();       /* strip whitespace */
                        if ( c == '-' )         /* sign? */
                            {
                            sign = -1;
                            c = INCHAR();       /* next char */
                            }
                        if ( *p == 'i' )        /* integer format? */
                            {
                            if ( c == '0' )     /* octal or hex? */
                                {
                                mode = OCTAL;   /* assume octal */
                                limit = MAXOCT;
                                c = INCHAR();
                                if ( c == 'x' || c == 'X' )
                                    {
                                    mode = HEX;         /* actually, hex */
                                    limit = MAXOCT;
                                    c = INCHAR();       /* next char */
                                    }
                                }
                            }
                        if ( width )            /* normalize width */
                            limit = MIN( width, limit );
                        if ( sign < 0 )         /* account for sign */
                            limit--;

                        switch ( mode )         /* read in digits */
                            {
                            case DECIMAL:
                                while ( isdigit( c ) && limit-- )
                                    {
                                    digits[ i++ ] = c;
                                    c = INCHAR();
                                    }
                                for ( j = 0; j < i; j++ )
                                    m = m * 10 + ( digits[ j ] - '0' );
                                break;

                            case OCTAL:
                                while( isdigit( c ) && c < '8' && limit-- )
                                    {
                                    digits[ i++ ] = c;
                                    c = INCHAR();
                                    }
                                for ( j = 0; j < i; j++ )
                                    m = m * 8 + ( digits[ j ] - '0' );
                                break;

                            case HEX:
                                if ( c == '0' ) /* possible 0x spec? */
                                    {
                                    c = INCHAR();
                                    if ( c == 'x' || c == 'X' )
                                        {
                                        c = INCHAR();
                                        }
                                    }
                                while( isxdigit( c ) && limit-- )
                                    {
                                    if ( islower( c ))
                                        c = (char)toupper( c );
                                    digits[ i++ ] = c;
                                    c = INCHAR();
                                    }
                                for ( j = 0; j < i; j++ )
                                    if ( isdigit( digits[ j ] ))
                                        m = m * 16 + digits[ j ] - '0';
                                    else
                                        m = m * 16 + 10 + digits[ j ] - 'A';
                            }
                        if (! suppress )        /* if we can store it, */
                            {
                            n = m * sign;       /* append sign */

                            CONVERT( n, type ); /* type conversion */
                            ptr = (int *)va_arg( ap, long );
                            *ptr = n;           /* store it */
                            }
                        break;                  /* exit conversion loop */

                    case 's':                   /* STRING */
                        count++;                /* another one bytes the dust */
                        while( isspace( c ))
                            c = INCHAR();       /* strip whitespace */
                        limit = MAXSTRING;      /* set limit */
                        if ( width )            /* truncate if width */
                            limit = MIN( width, limit );
                        if ( suppress )         /* if we can't store it, */
                            while ( !isspace( c ) && limit-- )
                                c = INCHAR();   /* read and ignore */
                        else                    /* else, read and store */
                            {
                            str = va_arg( ap, char * );
                            while ( !isspace( c ) && limit-- )
                                {
                                *str++ = c;
                                c = INCHAR();
                                }
                            *str = '\0';        /* add terminating null */
                            }
                        break;

                    case 'c':                   /* CHARACTERS */
                        count++;                /* another one bytes the dust */
                        limit = MAX( width, 1 );
                        if ( suppress )         /* if we can't store it, */
                            while ( limit-- )
                                c = INCHAR();   /* read and ignore */
                        else                    /* else, read and store */
                            {
                            str = va_arg( ap, char * );
                            while ( limit-- )
                                {
                                *str++ = c;
                                c = INCHAR();
                                }
                            }
                        break;

                    case '[':                   /* scan set */
                        limit = MAXSCANSET;
                        if ( *++p == '^' )      /* negation? */
                            negate = TRUE;
                        else                    /* read in scanset to ] */
                            scanset[ i++ ] = *p;
                        while ( *++p != ']' && --limit )
                            scanset[ i++ ] = *p;
                        scanset[ i ] = '\0';    /* null termination */

                        limit = MAXSTRING;      /* set limit */
                        if ( width )            /* truncate if width */
                            limit = MIN( width, limit );
                        if ( !suppress )        /* can store it? */
                            str = va_arg( ap, char * );

                        while ( c != '\0' && limit-- )
                            {
                            if ( strchr( scanset, c ))  /* member of set, */
                                {
                                if ( negate )           /* but [^ ] */
                                    {
                                    *str = '\0';        /* store null */
                                    return( count );    /* and quit */
                                    }
                                }
                            else                        /* not member, */
                                {
                                if ( !negate )          /* but [ ] */
                                    {
                                    *str = '\0';        /* store null */
                                    return( count );    /* and quit */
                                    }
                                }
                            if ( !suppress )            /* if not suppressed */
                                *str++ = c;             /* store character */
                            c = INCHAR();               /* next one */
                            }
                        break;

                    case '%':                   /* escaped percent */
                        count++;                /* another one bytes the dust */
                        if ( c != '%' )         /* should be read in */
                            return( count );
                        c = INCHAR();           /* next character */
                        break;

                    }                           /* end switch */
                    break;                      /* exit conversion loop */
                }                               /* end conversion loop */
            }                                   /* end char read loop */
        return( count );                        /* return char count */
    }

static char _Inchar( long addr )
    {
        register char   *s;
        char    c;

        if ( addr == (long)NULL )               /* no address? */
            read( stdin, &c, 1);                /* standard in */
        else
            {
            s = (char *)*(long *)addr;          /* get string address */
            c = *s++;                           /* get character */
            *(long *)addr = (long)s;            /* update address */
            }
        return( c );                            /* return character */
    }

#else
/*
 *  Need a printf defined even when not debug so we can build.
 */
int printf( const char *format, ... )
{
return 0;
}
#endif /*DEBUG */
