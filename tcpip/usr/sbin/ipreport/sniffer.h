/* @(#)95	1.1  src/tcpip/usr/sbin/ipreport/sniffer.h, tcpras, tcpip411, GOLD410 12/8/92 11:02:53 */

#define NG_IDENT	"TRSNIFF data"
#define NG_IDENT_LEN	17

/* standard header */
struct f_rec_struct {
    u_short type;
#define REC_VERS	1
#define REC_FRAME2	4
#define REC_EOF		3
    u_short length;
    u_short rsvd;
};

struct date_struct {
    u_short a;
    u_short b;
};

struct f_vers_struct {
    u_short maj_vers;		/* major version of sniffer */
    u_short min_vers;		/* minor    "    "     "    */
    struct date_struct date;	/* date and time, DOS format */
    char type;			/* what type of records follow */
    char network;		/* indicator of network type */
#define NG_ETHERNET	0x01
#define NG_TOKEN	0x00
    char format;		/* format version */
    char timeunit;		/* indicator of frame timestamp unit */
    u_short rsvd[3];
};

struct f_frame2_struct {
    u_short time_low;		/* low time in 0.838096 usec units */
    u_short time_mid;		/* mid time in 54.9255 msec units */
    u_short time_high;		/* High time in hours */
    u_short size;		/* Number of bytes actually written */
    char fs;			/* frame error status bits */
    char flags;			/* buffer flags */
    u_short true_size;		/* if non-zero, size of original frame */
    u_short rsvd3;
};

/* only for unsigned shorts */
#define SWAP_SHORT(a) (a) = ((a) << 8) + ((a) >> 8)
