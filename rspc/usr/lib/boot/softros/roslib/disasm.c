static char sccsid[] = "@(#)19	1.1  src/rspc/usr/lib/boot/softros/roslib/disasm.c, rspc_softros, rspc411, 9432A411a 8/5/94 16:38:54";
/*
 *   COMPONENT_NAME: rspc_softros
 *
 *   FUNCTIONS: disassemble
 *
 *   ORIGINS: 27
 *
 *   IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 *   combined with the aggregated modules for this product)
 *                    SOURCE MATERIALS
 *
 *   (C) COPYRIGHT International Business Machines Corp. 1993,1994
 *   All Rights Reserved
 *   US Government Users Restricted Rights - Use, duplication or
 *   disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

// <rb> #pragma options MAXMEM=10000

 /*
  */
#ifdef DEBUG

 /*
  * Maintenance History:
  *
  *  08 FEB 93  M. McLemore  Original version (0.1).
  *  11 FEB 93  M. McLemore  Add support for extended mnemonics (0.3).
  *  17 FEB 93  M. McLemore  Add priviledged instructions (0.4).
  *  19 FEB 93  P. Kufeldt   Add new command line argument parsing mechanism.
  *                         Add new args -n, -s <start>, -c <count>.
  *                         Can handle straight, unheadered chunks of code.
  *                         Das now defaults to a.out file (0.5).
  *  20 APR 93  M. McLemore  Fixed mtspr decode and rlwimi MB field (0.6).
  */


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NODISASM

int disassemble ( startaddr, image, extmnem, size )
        int     startaddr;
        int     image[];
        int     extmnem;
        int     size;
     {
        register int    ins;
        int             aalk, aa, lk, bo, bi, rt, ra, rb, oe;
        int             rc, mb, oerc, spr, fxm, mask, not;
        short           si, d;
        unsigned short  ui, crf;
        unsigned int    li, crbit;
        int             bt, ba, bb, bf, bfa, j, l, me, rs, sr, sh;
        long            line;
        long            addr, newaddr;
        char            *mnem, *comment, *btype;
        register int    opcode, subop, i;
        int             extended;
        int             rval;


        size = size / 4;                        /* number of words */

        rval = 0;

        for ( i = 0; (i < size) ; i++ )            /* for each instruction */
            {
            ins = image[ i ];
            line = startaddr + ( i * 4 );               /* line number */
            bo  = bt = rt = rs = (ins >> 21) & 0x1F;    /* fields: */
            bi  = ba = ra = (ins >> 16) & 0x1F;
            sr  = ba & 0xF;
            bb  = rb = sh = (ins >> 11 ) & 0x1F;
            spr = (ins >> 11 ) & 0x3FF;
            spr = (( spr >> 5 ) & 0x1F) | (( spr & 0x1F ) << 5 );
            fxm = (ins >> 12) & 0xFF;
            bf  = (ins >> 23) & 0x07;
            bfa = (ins >> 18) & 0x07;
            oe  = (ins >> 10) & 1;
            mb  = (ins >>  6) & 0x1F;
            me  = (ins >>  1) & 0x1F;
            rc  = ins & 1;
            d   = ins & 0xFFFF;
            l   = (ins >> 21) & 1;
            ui  = (unsigned short)ins;          /* unsigned immediate data */
            si  = (short)(ins);                 /* signed immediate data */
            oerc  = ( oe  <<  1 ) | rc;
            opcode =  (ins >> 26) & 0x3F;       /* get opcode */

            mask = 0;                           /* decode rotate mask */
            if ( me < mb )                      /* end less than start? */
                j = me, me = mb, mb = j;        /* swap me and mb */
            for ( j = ( 31 - me ); j <= ( 31 - mb ); j++ )
                mask |= ( 1 << j );

            crf   = bi >> 2;                    /* condition reg field */
            crbit = 0x80000000 >> bi;           /* cond reg bit */
            crbit = (crbit >> ((7 - crf) * 4)); /* shift to lo nibble */

            switch ( opcode )
                {
                case 0:                         /* pad word */
                    comment = "# pad word: ";
                    printf("\n  %08x:\t\t\t\t%s0x%08x", line, comment, ins);
                    continue;

                case 2:                         /* trap doubleword imm */
                    mnem = "tdi";
                    comment = "# trap doubleword immediate";
                    printf("\n  %08x:\t%s\tr%d,r%d,0x%x  \t%s %d", line,
                            mnem, rt, ra, ui, comment, si);
                    continue;

                case 3:                         /* trap word imm */
                    mnem = "twi";
                    comment = "# trap word immediate";
                    printf("\n  %08x:\t%s\tr%d,r%d,0x%x  \t%s %d", line,
                        mnem, rt, ra, ui, comment, si);
                    continue;

                case 7:                         /* multiply lo immediate */
                    mnem = "mulli";
                    comment = "# multiply low immediate";
                    printf("\n  %08x:\t%s\tr%d,r%d,0x%x  \t%s %d", line,
                            mnem, rt, ra, ui, comment, si);
                    continue;

                case 8:                         /* subtract imm carry */
                    mnem = "subfic";
                    comment = "# subtract immediate";
                    printf("\n  %08x:\t%s\tr%d,r%d,0x%x  \t%s %d, carrying",
                        line, mnem, rt, ra, ui, comment, si);
                    continue;

                case 10:                        /* compare logical immediate */
                    mnem = "cmpli";
                    comment = "# compare logical";
                    printf("\n  %08x:\t%s\t%d,%d,r%d,0x%x\t%s r%d to %d",
                        line, mnem, bf, l, ra, ui, comment, ra, si);
                    continue;

                case 11:                        /* compare immediate */
                    mnem = "cmpi";
                    comment = "# compare";
                    printf("\n  %08x:\t%s\t%d,%d,r%d,0x%x\t%s r%d to %d",
                        line, mnem, bf, l, ra, ui, comment, ra, si);
                    continue;

                case 12:                        /* add immediate carry */
                    mnem = "addic";
                    comment = "# add";
                    if ( ra )
                        {
                        printf("\n  %08x:\t%s\tr%d,r%d,0x%x\t",
                            line, mnem, rt, ra, ui);
                        if ( ui )
                            printf("%s %d to r%d", comment, si, ra);
                        else
                            printf("# move r%d to r%d", ra, rt);
                        }
                    else
                        printf("\n  %08x:\t%s\tr%d,0,0x%x\t%s %d to r%d",
                            line, mnem, rt, ui, comment, si, rt);
                    continue;

                case 13:                        /* add imm carry record */
                    mnem = "addic.";
                    comment = "# add";
                    if ( ra )
                        {
                        printf("\n  %08x:\t%s\tr%d,r%d,0x%x  \t",
                            line, mnem, rt, ra, ui);
                        if ( ui )
                            printf("%s %d to r%d", comment, si, ra);
                        else
                            printf("# move r%d to r%d", ra, rt);
                        }
                    else
                        printf("\n  %08x:\t%s\tr%d,0,0x%x  \t%s %d to r%d",
                            line, mnem, rt, ui, comment, si, rt);
                    continue;

                case 14:                        /* add immediate */
                    mnem = "addi";
                    comment = "# add";
                    if ( ra )
                        {
                        printf("\n  %08x:\t%s\tr%d,r%d,0x%x  \t",
                            line, mnem, rt, ra, ui);
                        if ( ui )
                            printf("%s %d to r%d", comment, si, ra);
                        else
                            printf("# move r%d to r%d", ra, rt);
                        }
                    else
                        printf("\n  %08x:\t%s\tr%d,0,0x%x  \t%s %d to r%d",
                            line, mnem, rt, ui, comment, si, rt);
                    continue;

                case 15:                        /* add immediate shifted */
                    mnem = "addis";
                    comment = "# add";
                    if ( ra )
                        {
                        printf("\n  %08x:\t%s\tr%d,r%d,0x%x  \t",
                            line, mnem, rt, ra, ui);
                        if ( ui )
                            printf("%s %d to r%d", comment, si << 16, ra);
                        else
                            printf("# move r%d to r%d", ra, rt);
                        }
                    else
                        printf("\n  %08x:\t%s\tr%d,0,0x%x  \t%s %d to r%d",
                            line, mnem, rt, ui, comment, si << 16, rt);
                    continue;

                case 18:                        /* branch I-form */
                    aalk = ins & 3;             /* absolute and link bits */
                    li = (ins >> 2) & 0xFFFFFF; /* encoded address */
                    addr = (signed)(li << 8);   /* sign extend */
                    addr = addr >> 6;           /* actual offset */
                    newaddr = line + addr;      /* absolute address */
                    switch ( aalk )
                    {
                        case 0:                 /* no AA, no LK */
                            mnem = "b";         /* regular branch */
                            comment = "# branch to";
                            printf("\n  %08x:\t%s\t%08x\t%s %08x",
                                line, mnem, addr, comment, newaddr);
                            break;
                        case 1:                 /* no AA, but LK */
                            mnem = "bl";        /* branch and link */
                            comment = "# branch and link to";
                            printf("\n  %08x:\t%s\t%08x\t%s %08x",
                                line, mnem, addr, comment, newaddr);
                            rval = 0x01;
                            break;
                        case 2:                 /* AA, but no LK */
                            mnem = "ba";        /* branch absolute */
                            comment = "# branch to";
                            printf("\n  %08x:\t%s\t%08x\t%s %08x",
                                line, mnem, addr, comment, addr);
                            break;
                        case 4:                 /* AA, LK */
                            mnem = "bla";       /* branch,link absolute */
                            comment = "# branch and link to";
                            printf("\n  %08x:\t%s\t%08x\t%s %08x",
                                line, mnem, addr, comment, addr);
                            rval = 0x01;
                            break;
                    }
                    break;

                case 16:                        /* branch conditional */
                    aalk = ins & 3;             /* absolute and link bits */
                    li = (ins >> 2) & 0x3FFF;   /* encoded address */
                    addr = (signed)(li << 18);  /* sign extend */
                    addr = addr >> 16;          /* actual offset */
                    newaddr = line + addr;      /* next address */

                    if ( extmnem )              /* if use extended mnem: */
                        {                       /* complement condition? */
                        if (( bo == 4 ) || ( bo == 5 ))
                            not = TRUE;
                        else
                            not = FALSE;
                        extended = TRUE;        /* optimism */
                        switch ( crbit )        /* type of branch: */
                            {
                            case 1:             /* branch summary overflow */
                                if ( not )
                                    {
                                    mnem = "bns";
                                    comment = "# branch not summary overflow";
                                    }
                                else
                                    {
                                    mnem = "bso";
                                    comment = "# branch summary overflow";
                                    }
                                break;
                            case 2:             /* branch equal */
                                if ( not )
                                    {
                                    mnem = "bne";
                                    comment = "# branch, if not equal,";
                                    }
                                else
                                    {
                                    mnem = "beq";
                                    comment = "# branch, if equal,";
                                    }
                                break;
                            case 4:             /* branch greater than */
                                if ( not )
                                    {
                                    mnem = "bng";
                                    comment = "# branch if not greater";
                                    }
                                else
                                    {
                                    mnem = "bgt";
                                    comment = "# branch if greater";
                                    }
                                break;
                            case 8:             /* branch less than */
                                if ( not )
                                    {
                                    mnem = "bnl";
                                    comment = "# branch if not less";
                                    }
                                else
                                    {
                                    mnem = "blt";
                                    comment = "# branch if less";
                                    }
                                break;

                            default:            /* not extended */
                                mnem = "bc";
                                comment = "# branch conditional";
                                extended = FALSE;
                                break;
                            }
                        switch ( aalk )
                            {
                            case 0:             /* no AA, no LK */
                                btype = "";     /* regular branch cond */
                                break;
                            case 1:             /* no AA, but LK */
                                btype = "l";    /* branch cond and link */
                                rval = 0x01;
                                break;
                            case 2:             /* AA, but no LK */
                                btype = "a";    /* branch absolute */
                                break;
                            case 4:             /* AA, LK */
                                btype = "la";   /* branch,link absolute */
                                rval = 0x01;
                                break;
                            }
                        if ( extended )
                            {
                            if ( crf )          /* cr1 - cr7 */
                               printf("\n  %08x:\t%s%s\tcr%d,%04x\t%s to %08x",
                                    line, mnem, btype, crf, addr, comment,
                                    newaddr);
                            else                /* don't show cr0 */
                               printf("\n  %08x:\t%s%s\t%08x    \t%s to %08x",
                                    line, mnem, btype, addr, comment, newaddr);
                            }
                        else                    /* not extended */
                            printf("\n  %08x:\t%s%s\t%d,%d,%04x\t%s to %08x",
                                line, mnem, btype, bo, bi, addr, comment,
                                newaddr);
                        }
                    else
                        {                       /* else, primary mnemonics */
                        switch ( aalk )
                            {
                            case 0:             /* no AA, no LK */
                                mnem = "bc";    /* regular branch cond */
                                comment = "# cond branch to";
                                printf("\n  %08x:\t%s\t%d,%d,%04x\t%s %08x",
                                    line, mnem, bo, bi, addr, comment, newaddr);
                                break;
                            case 1:             /* no AA, but LK */
                                mnem = "bcl";   /* branch cond and link */
                                comment = "# cond branch and link to";
                                printf("\n  %08x:\t%s\t%d,%d,%04x\t%s %08x",
                                    line, mnem, bo, bi, addr, comment, newaddr);
                                rval = 0x01;
                                break;
                            case 2:             /* AA, but no LK */
                                mnem = "bca";   /* branch absolute */
                                comment = "# cond branch to";
                                printf("\n  %08x:\t%s\t%d,%d,%04x\t%s %08x",
                                    line, mnem, bo, bi, addr, comment, addr);
                                break;
                            case 4:             /* AA, LK */
                                mnem = "bcla";  /* branch,link absolute */
                                comment = "# cond branch and link to";
                                printf("\n  %08x:\t%s\t%d,%d,%04x\t%s %08x",
                                    line, mnem, bo, bi, addr, comment, addr);
                                rval = 0x01;
                                break;
                        }
                    }
                    continue;

                case 19:                        /* XL instruction */
                    subop = (ins >> 1) & 0x3FF; /* subopcode */
                    lk = ins & 1;               /* absolute and link bits */
                    switch ( subop )
                        {
                        case 0:                 /* cond register field */
                            mnem = "mcrf";
                            comment = "# move condition register field";
                            printf("\n  %08x:\t%s\t%d,%d\t\t%s", line, mnem,
                                bf, bfa, comment);
                            continue;

                        case 16:                /* branch cond to LR */
                            if ( lk )
                                {
                                mnem = "bclrl";
                                comment = "# cond branch to LR, link";
                                rval = 0x01;
                                }
                            else
                                {
                                mnem = "bclr";
                                comment = "# cond branch to LR";
                                }
                            printf("\n  %08x:\t%s\t%d,%d\t\t%s", line, mnem,
                                bo, bi, comment);
                            continue;

                        case 33:                /* cond register nor */
                            mnem = "crnor";
                            comment = "# cond reg NOR bits ";
                            printf("\n  %08x:\t%s\t%d,%d,%d\t\t%s%d, %d",
                                line, mnem, bt, ba, bb, comment, ba, bb);
                            continue;

                        case 50:                /* return from interrupt */
                            mnem = "rfi";
                            comment = "# return from interrupt ";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            rval = 0x02;        /* Cannot step rfi      */
                            continue;

                        case 129:               /* cond register and comp */
                            mnem = "crandc";
                            comment = "# cond reg AND complement bits ";
                            printf("\n  %08x:\t%s\t%d,%d,%d     \t%s%d, %d",
                                line, mnem, bt, ba, bb, comment, ba, bb);
                            continue;

                        case 150:               /* cond register nor */
                            mnem = "isync";
                            comment = "# Instruction Sync";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 193:               /* cond register xor */
                            if ( extmnem && ( ba == bb ) && ( ba == bt ))
                                {
                                mnem = "crclr";
                                comment = "# clear cr bit";
                                printf("\n  %08x:\t%s\t%d\t\t%s %d",
                                    line, mnem, ba, comment, ba);
                                }
                            else
                                {
                                mnem = "crxor";
                                comment = "# cond register XOR bits ";
                                printf("\n  %08x:\t%s\t%d,%d,%d\t\t%s%d, %d",
                                    line, mnem, bt, ba, bb, comment, ba, bb);
                                }
                            continue;

                        case 225:               /* cond register nand */
                            mnem = "crnand";
                            comment = "# condition register NAND bits ";
                            printf("\n  %08x:\t%s\t%d,%d,%d\t\t%s%d, %d",
                                line, mnem, bt, ba, bb, comment, ba, bb);
                            continue;

                        case 257:               /* cond register and */
                            mnem = "crand";
                            comment = "# cond reg AND bits ";
                            printf("\n  %08x:\t%s\t%d,%d,%d\t\t%s%d, %d",
                                line, mnem, bt, ba, bb, comment, ba, bb);
                            continue;

                        case 289:               /* cond register equ */
                            mnem = "creqv";
                            comment = "# cond reg equivalent bits ";
                            printf("\n  %08x:\t%s\t%d,%d,%d\t\t%s%d, %d",
                                line, mnem, bt, ba, bb, comment, ba, bb);
                            continue;

                        case 417:               /* cond register or comp */
                            mnem = "crorc";
                            comment = "# cond reg OR complement bits ";
                            printf("\n  %08x:\t%s\t%d,%d,%d\t%s%d, %d",
                                line, mnem, bt, ba, bb, comment, ba, bb);
                            continue;

                        case 449:               /* cond register or */
                            if ( extmnem && ( ba == bb ))
                                {
                                if ( ba == bt )
                                    {
                                    mnem = "nop";
                                    comment = "# null operation";
                                    printf("\n  %08x:\t%s\t\t\t%s",
                                        line, mnem, comment);
                                    }
                                else
                                    {
                                    mnem = "crmove";
                                    comment = "# move cr bit";
                                    printf("\n  %08x:\t%s\t%d,%d\t\t%s",
                                        line, mnem, bt, bb, comment);
                                    printf(" %d to bit %d", bt, bb);
                                    }
                                }
                            else
                                {
                                mnem = "cror";
                                comment = "# cond register or bits ";
                                printf("\n  %08x:\t%s\t%d,%d,%d\t%s%d, %d",
                                    line, mnem, bt, ba, bb, comment, ba, bb);
                                }
                            continue;

                        case 528:               /* branch cond to CTR */
                            if ( lk )
                                {
                                mnem = "bcctrl";
                                comment = "# conditional branch to CTR, link";
                                }
                            else
                                {
                                mnem = "bcctr";
                                comment = "# conditional branch to CTR";
                                }
                            printf("\n  %08x:\t%s\t%d,%d\t\t%s", line, mnem,
                                bo, bi, comment);
                            continue;

                        default:
                            printf("\n  %08x:\t%08x", line, ins);
                            printf("\t\t# unrecognized opcode");
                            continue;
                        }
                case 17:                        /* SC instruction */
                    printf("\n  %08x:\tsc", line);
                    continue;

                case 20:                        /* rotate left immediate */
                    if ( rc )
                        mnem = "rlwimi.";
                    else
                        mnem = "rlwimi";
                    printf("\n  %08x:\t%s\tr%d,r%d,%d,%d,%d\t",
                        line, mnem, ra, rs, sh, mb, me );
                    printf("# rotate r%d left %d, insert %08x",
                        rs, sh, mask);
                    continue;

                case 21:                        /* rotate left immediate */
                    if ( rc )
                        mnem = "rlwinm.";
                    else
                        mnem = "rlwinm";
                    printf("\n  %08x:\t%s\tr%d,r%d,%d,%d,%d\t",
                        line, mnem, ra, rs, sh, mb, me );
                    if ( sh )
                        printf("# rotate r%d left %d, mask with %08x",
                            rs, sh, mask);
                    else
                        printf("# mask r%d with %08x", rs, mask);
                    continue;

                case 23:                        /* rotate left */
                    if ( rc )
                        mnem = "rlwnm.";
                    else
                        mnem = "rlwnm";
                    printf("\n  %08x:\t%s\tr%d,r%d,r%d,%d,%d\t",
                        line, mnem, ra, rs, rb, mb, me );
                    printf("# rotate r%d left, mask %08x", rs, mask);
                    continue;

                case 24:                        /* or immediate */
                    mnem = "ori";
                    comment = "# or";
                    if ( ui )
                        printf("\n  %08x:\t%s\tr%d,r%d,0x%x\t%s r%d with 0x%x",
                            line, mnem, ra, rs, ui, comment, rs, ui);
                    else
                        printf("\n  %08x:\t%s\tr%d,r%d,0x%x\t# move r%d to r%d",
                            line, mnem, ra, rs, ui, rs, ra);
                    continue;

                case 25:                        /* or immediate shifted */
                    mnem = "oris";
                    comment = "# or";
                    if ( ui )
                        printf("\n  %08x:\t%s\tr%d,r%d,0x%x\t%s r%d with 0x%x",
                            line, mnem, ra, rs, ui, comment, rs, ui << 16);
                    else
                        printf("\n  %08x:\t%s\tr%d,r%d,0x%x\t# move r%d to r%d",
                            line, mnem, ra, rs, ui, rs, ra);
                    continue;

                case 26:                        /* xor immediate */
                    mnem = "xori";
                    comment = "# xor";
                    if ( ui )
                        printf("\n  %08x:\t%s\tr%d,r%d,0x%x\t%s r%d with 0x%x",
                            line, mnem, ra, rs, ui, comment, rs, ui);
                    else
                        printf("\n  %08x:\t%s\tr%d,r%d,0x%x\t# move r%d to r%d",
                            line, mnem, ra, rs, ui, rs, ra);
                    continue;

                case 27:                        /* xor immediate shifted */
                    mnem = "xoris";
                    comment = "# xor";
                    if ( ui )
                        printf("\n  %08x:\t%s\tr%d,r%d,0x%x\t%s r%d with 0x%x",
                            line, mnem, ra, rs, ui, comment, rs, ui << 16);
                    else
                        printf("\n  %08x:\t%s\tr%d,r%d,0x%x\t# move r%d to r%d",
                            line, mnem, ra, rs, ui, rs, ra);
                    continue;

                case 28:                        /* and immediate */
                    mnem = "andi.";
                    comment = "# and";
                    if ( ui )
                        printf("\n  %08x:\t%s\tr%d,r%d,0x%x\t%s r%d with 0x%x",
                            line, mnem, ra, rs, ui, comment, rs, ui);
                    else
                        printf("\n  %08x:\t%s\tr%d,r%d,0x%x\t# clear r%d",
                            line, mnem, ra, rs, ui, ra );
                    continue;

                case 29:                        /* and immediate shifted */
                    mnem = "andis.";
                    comment = "# and";
                    printf("\n  %08x:\t%s\tr%d,r%d,0x%x\t%s r%d with 0x%x",
                            line, mnem, ra, rs, ui, comment, rs, ui << 16);
                    continue;

                case 31:                        /* Extended instruction: */
                    subop = ( ins >>  1 ) & 0x3FF;
                    switch ( subop )
                        {
                        case 0:                 /* compare logical immediate */
                            mnem = "cmp";
                            comment = "# compare";
                            printf("\n  %08x:\t%s\t%d,%d,r%d,r%d\t",
                                line, mnem, bf, l, ra, rb);
                            printf("%s r%d to r%d", comment, ra, rb);
                            continue;


                        case 4:               /* cond register nor */
                            mnem = "tw";
                            comment = "# Trap Word - disasm incomplete";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 8:                 /* subtract carry XO form */
                        case 520:
                            switch (oerc)
                                {
                                case 0:
                                    comment = ", carry";
                                    mnem = "subfc";
                                    break;

                                case 1:
                                    comment = ", carry";
                                    mnem = "subfc.";
                                    break;

                                case 2:
                                    comment = ", carry, overflow";
                                    mnem = "subfco";
                                    break;

                                case 3:
                                    comment = ", carry, overflow";
                                    mnem = "subfco.";
                                    break;
                                }
                            printf("\n  %08x:\t%s\tr%d,r%d,r%d\t# subtract",
                                    line, mnem, rt, ra, rb );
                            printf(" r%d from r%d%s", ra, rb, comment);
                            continue;

                        case 10:                /* add carry XO form */
                        case 522:
                            switch (oerc)
                                {
                                case 0:
                                    comment = "# add";
                                    mnem = "addc";
                                    break;

                                case 1:
                                    comment = "# add";
                                    mnem = "addc.";
                                    break;

                                case 2:
                                    comment = "# add, overflow,";
                                    mnem = "addco";
                                    break;

                                case 3:
                                    comment = "# add, overflow,";
                                    mnem = "addco.";
                                    break;
                                }
                            printf("\n  %08x:\t%s\tr%d,r%d,r%d\t%s",
                                    line, mnem, rt, ra, rb, comment );
                            printf(" r%d to r%d", ra, rb);
                            continue;

                        case 11:                /* multiply high unsigned */
                        case 523:
                            if ( rc )
                                mnem = "mulhwu.";
                            else
                                mnem = "mulhwu";
                            comment = " unsigned";
                            printf("\n  %08x:\t%s\tr%d,r%d,r%d\t# multiply hi",
                                    line, mnem, rt, ra, rb );
                            printf(" r%d times r%d%s", ra, rb, comment);
                            continue;

                        case 19:               /* move from cr */
                            mnem = "mfcr";
                            comment = "# Move from condition - disasm incomplete";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 20:                /* Load word and reserve idx */
                            comment = "# load word and reserve indexed";
                            mnem = "lwarx";
                            printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s", line,
                                mnem, rt, ra, rb, comment);
                            continue;

                        case 21:                /* Load doubleword idx */
                            comment = "# load doubleword indexed";
                            mnem = "ldx";
                            printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s", line,
                                mnem, rt, ra, rb, comment);
                            continue;

                        case 23:                /* Load word zero indexed */
                            comment = "# load word and zero indexed";
                            mnem = "lwzx";
                            printf("\n  %08x:\t%s\tr%d,r%d,r%d\t%s", line,
                                mnem, rt, ra, rb, comment);
                            continue;

                        case 24:                /* shift left word */
                            if ( rc )
                                mnem = "slw.";
                            else
                                mnem = "slw";
                            printf("\n  %08x:\t%s\tr%d,r%d,r%d\t# shift",
                                    line, mnem, ra, rs, rb );
                            printf(" r%d left by (r%d)", rs, rb );
                            continue;

                        case 26:                /* count leading zeroes */
                            if ( rc )
                                mnem = "cntlzw.";
                            else
                                mnem = "cntlzw";
                            comment = "# count leading zeroes in";
                            printf("\n  %08x:\t%s\tr%d,r%d\t\t%s r%d",
                                    line, mnem, ra, rs, comment, rs );
                            continue;

                        case 28:                /* and */
                            if ( rc )
                                mnem = "and.";
                            else
                                mnem = "and";
                            printf("\n  %08x:\t%s\tr%d,r%d,r%d\t# and ",
                                    line, mnem, ra, rs, rb );
                            printf("r%d with r%d", rs, rb);
                            continue;

                        case 32:                /* compare logical */
                            mnem = "cmpl";
                            comment = "# compare logical";
                            printf("\n  %08x:\t%s\t%d,%d,r%d,r%d\t",
                                line, mnem, bf, l, ra, rb);
                            printf("%s r%d to r%d", comment, ra, rb);
                            continue;

                        case 40:                /* subtract XO form */
                        case 552:
                            switch (oerc)
                                {
                                case 0:
                                    comment = "";
                                    mnem = "subf";
                                    break;

                                case 1:
                                    comment = "";
                                    mnem = "subf.";
                                    break;

                                case 2:
                                    comment = ", overflow";
                                    mnem = "subfo";
                                    break;

                                case 3:
                                    comment = ", overflow";
                                    mnem = "subfo.";
                                    break;
                                }
                            printf("\n  %08x:\t%s\tr%d,r%d,r%d\t# subtract",
                                    line, mnem, rt, ra, rb );
                            printf(" r%d from r%d%s", ra, rb, comment);
                            continue;

                        case 53:                /* Load double idx update */
                            comment = "# load doubleword with update indexed";
                            mnem = "ldux";
                            printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s", line,
                                mnem, rt, ra, rb, comment);
                            continue;

                        case 54:               /* data cache block store */
                            mnem = "dcbst";
                            comment = "# Data Cache Block Store - disasm incomplete";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 55:                /* Load wd zero idx update */
                            comment = "# load word and zero update indexed";
                            mnem = "lwzux";
                            printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s", line,
                                mnem, rt, ra, rb, comment);
                            continue;

                        case 60:                /* and with complement */
                            if ( rc )
                                mnem = "andc.";
                            else
                                mnem = "andc";
                            printf("\n  %08x:\t%s\tr%d,r%d,r%d\t# and ",
                                    line, mnem, ra, rs, rb );
                            printf("r%d with complement r%d", rs, rb);
                            continue;

                        case 75:                /* multiply high */
                        case 587:
                            if (rc)
                                mnem = "mulhw.";
                            else
                                mnem = "mulhw";
                            printf("\n  %08x:\t%s\tr%d,r%d,r%d\t# multiply hi",
                                    line, mnem, rt, ra, rb );
                            printf(" r%d times r%d", ra, rb);
                            continue;

                        case 83:                /* Move from msr */
                            mnem = "mfmsr";
                            printf("\n  %08x:\t%s\tr%d\t\t", line, mnem, rt);
                            printf("# move msr to r%d", rt );
                            continue;

                        case 84:                /* Load dword and reserve idx */
                            comment = "# load doubleword and reserve indexed";
                            mnem = "ldarx";
                            printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s", line,
                                mnem, rt, ra, rb, comment);
                            continue;

                        case 86:               /* data cache block flush */
                            mnem = "dcbf";
                            comment = "# Data Cache Block Flush - disasm incomplete";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 87:                /* Load byte zero idx */
                            comment = "# load byte and zero indexed";
                            mnem = "lbzx";
                            printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s", line,
                                mnem, rt, ra, rb, comment);
                            continue;

                        case 104:               /* negate XO form */
                        case 616:
                            switch (oerc)
                                {
                                case 0:
                                    comment = "# negate";
                                    mnem = "neg";
                                    break;

                                case 1:
                                    comment = "# negate";
                                    mnem = "neg.";
                                    break;

                                case 2:
                                    comment = "# negate, overflow";
                                    mnem = "nego";
                                    break;

                                case 3:
                                    comment = "# negate, overflow";
                                    mnem = "nego.";
                                    break;
                                }
                            printf("\n  %08x:\t%s\tr%d,r%d\t%s", line,
                                mnem, rt, ra, comment);
                            continue;

                        case 136:               /* subtract extended XO form */
                        case 648:
                            switch (oerc)
                                {
                                case 0:
                                    comment = "";
                                    mnem = "subfe";
                                    break;

                                case 1:
                                    comment = "";
                                    mnem = "subfe.";
                                    break;

                                case 2:
                                    comment = ", overflow";
                                    mnem = "subfeo";
                                    break;

                                case 3:
                                    comment = ", overflow";
                                    mnem = "subfeo.";
                                    break;
                                }
                            printf("\n  %08x:\t%s\tr%d,r%d,r%d\t# subtract",
                                    line, mnem, rt, ra, rb );
                            printf(" r%d from r%d%s", ra, rb, comment);
                            continue;

                        case 115:              /* move from pmr */
                            mnem = "mfpmr";
                            comment = "# Move From Program Mode Register - disasm incomplete";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 119:               /* Load byte zero update idx */
                            comment = "# load byte and zero update indexed";
                            mnem = "lbzux";
                            printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s", line,
                                mnem, rt, ra, rb, comment);
                            continue;

                        case 124:               /* nor */
                            if ( extmnem && ( rs == rb ))
                                {
                                if ( rc )
                                    mnem = "not.";
                                else
                                    mnem = "not";
                                printf("\n  %08x:\t%s\tr%d,r%d\t",
                                        line, mnem, ra, rs );
                                printf("# move complement r%d to r%d",
                                        rb, ra);
                                }
                            else
                                {
                                if ( rc )
                                    mnem = "nor.";
                                else
                                    mnem = "nor";
                                printf("\n  %08x:\t%s\tr%d,r%d,r%d\t",
                                        line, mnem, ra, rs, rb );
                                printf("# complement r%d ored with r%d",
                                        rs, rb);
                                }
                            continue;

                        case 138:              /* Add Extended */
                            mnem = "addex";
                            comment = "# Add Extended - disasm incomplete";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 144:               /* move to cr */
                            mnem = "mtcrf";
                            comment = "# move to cond register field";
                            printf("\n  %08x:\t%s\t%d,r%d\t\t%s",
                                    line, mnem, fxm, rs, comment );
                            continue;

                        case 146:               /* Move to msr */
                            mnem = "mtmsr";
                            printf("\n  %08x:\t%s\tr%d\t\t", line,
                                mnem, rs);
                            printf("# move r%d to msr", rs );
                            rval = 0x02;        /* Cannot step mtmsr    */
                            continue;

                        case 149:               /* Store dword indexed */
                            comment = "# store doubleword indexed";
                            mnem = "stdx";
                            printf("\n  %08x:\t%s\tr%d,r%d,r%d\t%s", line,
                                mnem, rs, ra, rb, comment);
                            continue;

                        case 150:               /* Store word cond indexed */
                            comment = "# store word conditional indexed";
                            mnem = "stwcx.";
                            printf("\n  %08x:\t%s\tr%d,r%d,r%d\t%s", line,
                                mnem, rs, ra, rb, comment);
                            continue;

                        case 151:               /* Store word indexed */
                            comment = "# store word indexed";
                            mnem = "stwx";
                            printf("\n  %08x:\t%s\tr%d,r%d,r%d\t%s", line,
                                mnem, rs, ra, rb, comment);
                            continue;

                        case 178:              /* mtpmr */
                            mnem = "mtpmr";
                            comment = "# Move To Program Mode Register - disasm incomplete";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 181:               /* Store dword update idx */
                            comment = "# store doubleword update indexed";
                            mnem = "stdux";
                            printf("\n  %08x:\t%s\tr%d,r%d,r%d\t%s", line,
                                mnem, rs, ra, rb, comment);
                            continue;

                        case 183:               /* Store word update indexed */
                            comment = "# store word with update indexed";
                            mnem = "stwux";
                            printf("\n  %08x:\t%s\tr%d,r%d,r%d\t%s", line,
                                mnem, rs, ra, rb, comment);
                            continue;

                        case 202:               /* add to zero XO form */
                        case 714:
                            switch (oerc)
                                {
                                case 0:
                                    comment = "# add to 0";
                                    mnem = "addze";
                                    break;

                                case 1:
                                    comment = "# add to 0";
                                    mnem = "addze.";
                                    break;

                                case 2:
                                    comment = "# add to 0, overflow";
                                    mnem = "addzeo";
                                    break;

                                case 3:
                                    comment = "# add to 0, overflow";
                                    mnem = "addzeo.";
                                    break;
                                }
                            printf("\n  %08x:\t%s\tr%d,r%d\t\t%s", line,
                                mnem, rt, ra, comment);
                            continue;

                        case 210:               /* Move to segreg */
                            mnem = "mtsr";
                            printf("\n  %08x:\t%s\t%d,r%d\t\t", line,
                                mnem, sr, rs);
                            printf("# move r%d to sr%d", rs, sr );
                            continue;

                        case 214:               /* Store dword cond indexed */
                            comment = "# store doubleword conditional indexed";
                            mnem = "stdcx.";
                            printf("\n  %08x:\t%s\tr%d,r%d,r%d\t%s", line,
                                mnem, rs, ra, rb, comment);
                            continue;

                        case 215:               /* Store byte indexed */
                            comment = "# store byte indexed";
                            mnem = "stbx";
                            printf("\n  %08x:\t%s\tr%d,r%d,r%d\t%s", line,
                                mnem, rs, ra, rb, comment);
                            continue;

                        case 200:               /* subt from 0 ext XO form */
                        case 712:
                            switch (oerc)
                                {
                                case 0:
                                    comment = "";
                                    mnem = "subfze";
                                    break;

                                case 1:
                                    comment = "";
                                    mnem = "subfze.";
                                    break;

                                case 2:
                                    comment = ", overflow";
                                    mnem = "subfzeo";
                                    break;

                                case 3:
                                    comment = ", overflow";
                                    mnem = "subfzeo.";
                                    break;
                                }
                            printf("\n  %08x:\t%s\tr%d,r%d\t# subtract",
                                    line, mnem, rt, ra );
                            printf(" r%d from 0%s", ra, comment);
                            continue;

                        case 232:               /* subt from -1 ext XO form */
                        case 744:
                            switch (oerc)
                                {
                                case 0:
                                    comment = "";
                                    mnem = "subfme";
                                    break;

                                case 1:
                                    comment = "";
                                    mnem = "subfme.";
                                    break;

                                case 2:
                                    comment = ", overflow";
                                    mnem = "subfmeo";
                                    break;

                                case 3:
                                    comment = ", overflow";
                                    mnem = "subfmeo.";
                                    break;
                                }
                            printf("\n  %08x:\t%s\tr%d,r%d\t\t# subtract",
                                    line, mnem, rt, ra );
                            printf(" r%d from -1%s", ra, comment);
                            continue;

                        case 234:               /* add to minus 1 XO form */
                        case 746:
                            switch (oerc)
                                {
                                case 0:
                                    comment = "";
                                    mnem = "addme";
                                    break;

                                case 1:
                                    comment = "";
                                    mnem = "addme.";
                                    break;

                                case 2:
                                    comment = ", overflow";
                                    mnem = "addmeo";
                                    break;

                                case 3:
                                    comment = ", overflow";
                                    mnem = "addmeo.";
                                    break;
                                }
                            printf("\n  %08x:\t%s\tr%d,r%d\t# add",
                                    line, mnem, rt, ra );
                            printf(" r%d to -1%s", ra, comment);
                            continue;

                        case 235:               /* multiply low XO form */
                        case 747:
                            switch (oerc)
                                {
                                case 0:
                                    comment = "";
                                    mnem = "mull";
                                    break;

                                case 1:
                                    comment = "";
                                    mnem = "mull.";
                                    break;

                                case 2:
                                    comment = ", overflow";
                                    mnem = "mullo";
                                    break;

                                case 3:
                                    comment = ", overflow";
                                    mnem = "mullo.";
                                    break;
                                }
                            printf("\n  %08x:\t%s\tr%d,r%d,r%d\t# multiply",
                                    line, mnem, rt, ra, rb );
                            printf(" r%d times r%d%s", ra, rb, comment);
                            continue;

                        case 242:               /* Move to segreg indirect */
                            mnem = "mtsrin";
                            printf("\n  %08x:\t%s\tr%d,r%d\t\t", line,
                                mnem, rs, rb );
                            printf("# move r%d to seg reg indirect", rs );
                            continue;

                        case 246:              /* dcbtst */
                            mnem = "dcbtst";
                            comment = "# Data Cache Block Touch Store - disasm incomplete";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 247:               /* Store byte update indexed */
                            comment = "# store byte with update indexed";
                            mnem = "stbux";
                            printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s", line,
                                mnem, rt, ra, rb, comment);
                            continue;

                        case 266:               /* add XO form */
                        case 778:
                            switch (oerc)
                                {
                                case 0:
                                    comment = "# add";
                                    mnem = "add";
                                    break;

                                case 1:
                                    comment = "# add";
                                    mnem = "add.";
                                    break;

                                case 2:
                                    comment = "# add, overflow exception";
                                    mnem = "addo";
                                    break;

                                case 3:
                                    comment = "# add, overflow exception";
                                    mnem = "addo.";
                                    break;
                                }
                            printf("\n  %08x:\t%s\tr%d,r%d,r%d\t%s", line,
                                mnem, rt, ra, rb, comment);
                            continue;

                        case 275:              /* mftb */
                            mnem = "mftb";
                            comment = "# Move From Time Base - disasm incomplete";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 278:              /* dcbt */
                            mnem = "dcbt";
                            comment = "# Data Cache Block Touch - disasm incomplete";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 279:               /* Load hword zero idx */
                            comment = "# load halfword and zero indexed";
                            mnem = "lhzx";
                            printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s", line,
                                mnem, rt, ra, rb, comment);
                            continue;

                        case 284:               /* equivalent */
                            if ( rc )
                                mnem = "eqv.";
                            else
                                mnem = "eqv";
                            printf("\n  %08x:\t%s\tr%d,r%d,r%d\t# equivalent ",
                                    line, mnem, ra, rs, rb );
                            printf("r%d and r%d", rs, rb);
                            continue;

                        case 306:              /* tlbie */
                            mnem = "tlbie";
                            comment = "# TLB Invalidate Entry - disasm incomplete";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 307:              /* mftbu */
                            mnem = "mftbu";
                            comment = "# Move From Time Base Upper - disasm incomplete";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 310:              /* eciwx */
                            mnem = "eciwx";
                            comment = "# Extended Control Input Word indexed - disasm incomplete";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 311:               /* Load hword zero update idx */
                            comment = "# load halfword and zero update indexed";
                            mnem = "lhzux";
                            printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s", line,
                                mnem, rt, ra, rb, comment);
                            continue;

                        case 316:               /* xor */
                            if ( rc )
                                mnem = "xor.";
                            else
                                mnem = "xor";
                            if ( rs == rb )
                                {
                                printf("\n  %08x:\t%s\tr%d,r%d,r%d\t",
                                    line, mnem, ra, rs, rb );
                                printf("# clear r%d", ra );
                                }
                            else
                                {
                                printf("\n  %08x:\t%s\tr%d,r%d,r%d\t# xor ",
                                    line, mnem, ra, rs, rb );
                                printf("r%d and r%d", rs, rb);
                                }
                            continue;

                        case 339:               /* move from SPR */
                            if ( extmnem )
                                {
                                switch ( spr )
                                    {
                                    case 1:
                                        mnem = "mfxer";
                                        printf("\n  %08x:\t%s\tr%d\t\t",
                                                line, mnem, rt);
                                        printf("# move from XER to r%d", rt);
                                        break;
                                    case 8:
                                        mnem = "mflr";
                                        printf("\n  %08x:\t%s\tr%d\t\t",
                                                line, mnem, rt);
                                        printf("# move from LR to r%d", rt);
                                        break;
                                    case 9:
                                        mnem = "mfctr";
                                        printf("\n  %08x:\t%s\tr%d\t\t",
                                                line, mnem, rt);
                                        printf("# move from CTR to r%d", rt);
                                        break;
                                    case 18:
                                        mnem = "mfdsisr";
                                        printf("\n  %08x:\t%s\tr%d\t\t",
                                                line, mnem, rt);
                                        printf("# move from DSISR to r%d", rt);
                                        break;
                                    case 19:
                                        mnem = "mfdar";
                                        printf("\n  %08x:\t%s\tr%d\t\t",
                                                line, mnem, rt);
                                        printf("# move from DAR to r%d", rt);
                                        break;
                                    case 22:
                                        mnem = "mfdec";
                                        printf("\n  %08x:\t%s\tr%d\t\t",
                                                line, mnem, rt);
                                        printf("# move from DEC to r%d", rt);
                                        break;
                                    case 25:
                                        mnem = "mfsdr1";
                                        printf("\n  %08x:\t%s\tr%d\t\t",
                                                line, mnem, rt);
                                        printf("# move from SDR 1 to r%d", rt);
                                        break;
                                    case 26:
                                        mnem = "mfsrr0";
                                        printf("\n  %08x:\t%s\tr%d\t\t",
                                                line, mnem, rt);
                                        printf("# move from SRR 0 to r%d", rt);
                                        break;
                                    case 27:
                                        mnem = "mfsrr1";
                                        printf("\n  %08x:\t%s\tr%d\t\t",
                                                line, mnem, rt);
                                        printf("# move from SRR 1 to r%d", rt);
                                        break;
                                    case 272:
                                        mnem = "mfsprg";
                                        printf("\n  %08x:\t%s\tr%d,0\t\t",
                                                line, mnem, rt);
                                        printf("# move from SPRG0 to r%d", rt);
                                        break;
                                    case 273:
                                        mnem = "mfsprg";
                                        printf("\n  %08x:\t%s\tr%d,1\t\t",
                                                line, mnem, rt);
                                        printf("# move from SPRG1 to r%d", rt);
                                        break;
                                    case 274:
                                        mnem = "mfsprg";
                                        printf("\n  %08x:\t%s\tr%d,2\t\t",
                                                line, mnem, rt);
                                        printf("# move from SPRG2 to r%d", rt);
                                        break;
                                    case 275:
                                        mnem = "mfsprg";
                                        printf("\n  %08x:\t%s\tr%d,3\t\t",
                                                line, mnem, rt);
                                        printf("# move from SPRG3 to r%d", rt);
                                        break;
                                    case 280:
                                        mnem = "mfasr";
                                        printf("\n  %08x:\t%s\tr%d\t\t",
                                                line, mnem, rt);
                                        printf("# move from ASR to r%d", rt);
                                        break;
                                    case 282:
                                        mnem = "mfear";
                                        printf("\n  %08x:\t%s\tr%d\t\t",
                                                line, mnem, rt);
                                        printf("# move from EAR to r%d", rt);
                                        break;
                                    case 287:
                                        mnem = "mfpvr";
                                        printf("\n  %08x:\t%s\tr%d\t\t",
                                                line, mnem, rt);
                                        printf("# move from PVR to r%d", rt);
                                        break;
                                    case 528:
                                        mnem = "mfibatu";
                                        printf("\n  %08x:\t%s\tr%d,0\t\t",
                                                line, mnem, rt);
                                        printf("# move from IBAT0U to r%d", rt);
                                        break;
                                    case 529:
                                        mnem = "mfibatl";
                                        printf("\n  %08x:\t%s\tr%d,0\t\t",
                                                line, mnem, rt);
                                        printf("# move from IBAT0L to r%d", rt);
                                        break;
                                    case 530:
                                        mnem = "mfibatu";
                                        printf("\n  %08x:\t%s\tr%d,1\t\t",
                                                line, mnem, rt);
                                        printf("# move from IBAT1U to r%d", rt);
                                        break;
                                    case 531:
                                        mnem = "mfibatl";
                                        printf("\n  %08x:\t%s\tr%d,1\t\t",
                                                line, mnem, rt);
                                        printf("# move from IBAT1L to r%d", rt);
                                        break;
                                    case 532:
                                        mnem = "mfibatu";
                                        printf("\n  %08x:\t%s\tr%d,2\t\t",
                                                line, mnem, rt);
                                        printf("# move from IBAT2U to r%d", rt);
                                        break;
                                    case 533:
                                        mnem = "mfibatl";
                                        printf("\n  %08x:\t%s\tr%d,2\t\t",
                                                line, mnem, rt);
                                        printf("# move from IBAT2L to r%d", rt);
                                        break;
                                    case 534:
                                        mnem = "mfibatu";
                                        printf("\n  %08x:\t%s\tr%d,3\t\t",
                                                line, mnem, rt);
                                        printf("# move from IBAT3U to r%d", rt);
                                        break;
                                    case 535:
                                        mnem = "mfibatl";
                                        printf("\n  %08x:\t%s\tr%d,3\t\t",
                                                line, mnem, rt);
                                        printf("# move from IBAT3L to r%d", rt);
                                        break;

                                    case 536:
                                        mnem = "mfdbatu";
                                        printf("\n  %08x:\t%s\tr%d,0\t\t",
                                                line, mnem, rt);
                                        printf("# move from DBAT0U to r%d", rt);
                                        break;
                                    case 537:
                                        mnem = "mfdbatl";
                                        printf("\n  %08x:\t%s\tr%d,0\t\t",
                                                line, mnem, rt);
                                        printf("# move from DBAT0L to r%d", rt);
                                        break;
                                    case 538:
                                        mnem = "mfdbatu";
                                        printf("\n  %08x:\t%s\tr%d,1\t\t",
                                                line, mnem, rt);
                                        printf("# move from DBAT1U to r%d", rt);
                                        break;
                                    case 539:
                                        mnem = "mfdbatl";
                                        printf("\n  %08x:\t%s\tr%d,1\t\t",
                                                line, mnem, rt);
                                        printf("# move from DBAT1L to r%d", rt);
                                        break;
                                    case 540:
                                        mnem = "mfdbatu";
                                        printf("\n  %08x:\t%s\tr%d,2\t\t",
                                                line, mnem, rt);
                                        printf("# move from DBAT2U to r%d", rt);
                                        break;
                                    case 541:
                                        mnem = "mfdbatl";
                                        printf("\n  %08x:\t%s\tr%d,2\t\t",
                                                line, mnem, rt);
                                        printf("# move from DBAT2L to r%d", rt);
                                        break;
                                    case 542:
                                        mnem = "mfdbatu";
                                        printf("\n  %08x:\t%s\tr%d,3\t\t",
                                                line, mnem, rt);
                                        printf("# move from DBAT3U to r%d", rt);
                                        break;
                                    case 543:
                                        mnem = "mfdbatl";
                                        printf("\n  %08x:\t%s\tr%d,3\t\t",
                                                line, mnem, rt);
                                        printf("# move from DBAT3L to r%d", rt);
                                        break;
                                    }
                                }
                            else
                                {
                                mnem = "mfspr";
                                comment = "from spr to";
                                printf("\n  %08x:\t%s\tr%d,%d\t\t# move %s r%d",
                                    line, mnem, rs, spr, comment, rs );
                                }
                            continue;

                        case 341:               /* Load word algebraic idx */
                            comment = "# load word algebraic indexed";
                            mnem = "lwax";
                            printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s", line,
                                mnem, rt, ra, rb, comment);
                            continue;

                        case 343:               /* Load hword algebraic idx */
                            comment = "# load halfword algebraic indexed";
                            mnem = "lhax";
                            printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s", line,
                                mnem, rt, ra, rb, comment);
                            continue;

                        case 373:               /* Load word al idx update */
                            comment = "# load word algebraic indexed & update";
                            mnem = "lwaux";
                            printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s", line,
                                mnem, rt, ra, rb, comment);
                            continue;

                        case 375:               /* Load hword al update idx */
                            comment = "# load halfwd algebraic update indexed";
                            mnem = "lhaux";
                            printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s", line,
                                mnem, rt, ra, rb, comment);
                            continue;

                        case 403:              /* mttb */
                            mnem = "mttb";
                            comment = "# Move To Time Base - disasm incomplete";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 407:               /* Store halfword indexed */
                            comment = "# store halfword indexed";
                            mnem = "sthx";
                            printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s", line,
                                mnem, rt, ra, rb, comment);
                            continue;

                        case 412:               /* or with complement */
                            if ( rc )
                                mnem = "orc.";
                            else
                                mnem = "orc";
                            printf("\n  %08x:\t%s\tr%d,r%d,r%d\t# or ",
                                    line, mnem, ra, rs, rb );
                            printf("r%d with complement r%d", rs, rb);
                            continue;

                        case 435:              /* mttbu */
                            mnem = "mttbu";
                            comment = "# Move To Time Base Upper - disasm incomplete";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 438:              /* ecowx */
                            mnem = "ecowx";
                            comment = "# External Control Output Word indexed - disasm incomplete";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 439:               /* Store halfword idx update */
                            comment = "# store halfword with update indexed";
                            mnem = "sthux";
                            printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s", line,
                                mnem, rt, ra, rb, comment);
                            continue;

                        case 444:               /* move register? */
                            if ( extmnem && ( rs == rb ))
                                {
                                if ( rc )
                                    mnem = "mr.";
                                else
                                    mnem = "mr";
                                printf("\n  %08x:\t%s\tr%d,r%d\t\t# move ",
                                    line, mnem, ra, rs );
                                printf("r%d into r%d", rs, ra);
                                }
                            else                /* else, regular or */
                                {
                                if ( rc )
                                    mnem = "or.";
                                else
                                    mnem = "or";
                                printf("\n  %08x:\t%s\tr%d,r%d,r%d\t# or ",
                                    line, mnem, ra, rs, rb );
                                printf("r%d with r%d", rs, rb);
                                }
                            continue;

                        case 459:               /* divide word unsigned */
                        case 971:
                            switch (oerc)
                                {
                                case 0:
                                    comment = " unsigned";
                                    mnem = "divwu";
                                    break;

                                case 1:
                                    comment = " unsigned, record";
                                    mnem = "divwu.";
                                    break;

                                case 2:
                                    comment = " unsigned, overflow";
                                    mnem = "divwou";
                                    break;

                                case 3:
                                    comment = " unsigned, record, overflow";
                                    mnem = "divwou.";
                                    break;
                                }
                            printf("\n  %08x:\t%s\tr%d,r%d,r%d\t# divide",
                                    line, mnem, rt, ra, rb );
                            printf(" r%d by r%d%s", ra, rb, comment);
                            continue;

                        case 467:               /* move to SPR */
                            if ( extmnem )
                                {
                                switch ( spr )
                                    {
                                    case 1:
                                        mnem = "mtxer";
                                        printf("\n  %08x:\t%s\tr%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to XER", rs);
                                        break;
                                    case 8:
                                        mnem = "mtlr";
                                        printf("\n  %08x:\t%s\tr%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to LR", rs);
                                        break;
                                    case 9:
                                        mnem = "mtctr";
                                        printf("\n  %08x:\t%s\tr%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to CTR", rs);
                                        break;
                                    case 18:
                                        mnem = "mtdsisr";
                                        printf("\n  %08x:\t%s\tr%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to DSISR", rs);
                                        break;
                                    case 19:
                                        mnem = "mtdar";
                                        printf("\n  %08x:\t%s\tr%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to DAR", rs);
                                        break;
                                    case 22:
                                        mnem = "mtdec";
                                        printf("\n  %08x:\t%s\tr%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to DEC", rs);
                                        break;
                                    case 25:
                                        mnem = "mtsdr1";
                                        printf("\n  %08x:\t%s\tr%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to SDR 1", rs);
                                        break;
                                    case 26:
                                        mnem = "mtsrr0";
                                        printf("\n  %08x:\t%s\tr%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to SRR 0", rs);
                                        break;
                                    case 27:
                                        mnem = "mtsrr1";
                                        printf("\n  %08x:\t%s\tr%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to SRR 1", rs);
                                        break;
                                    case 272:
                                        mnem = "mtsprg";
                                        printf("\n  %08x:\t%s\t0,r%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to SPRG0", rs);
                                        break;
                                    case 273:
                                        mnem = "mtsprg";
                                        printf("\n  %08x:\t%s\t1,r%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to SPRG1", rs);
                                        break;
                                    case 274:
                                        mnem = "mtsprg";
                                        printf("\n  %08x:\t%s\t2,r%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to SPRG2", rs);
                                        break;
                                    case 275:
                                        mnem = "mtsprg";
                                        printf("\n  %08x:\t%s\t3,r%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to SPRG3", rs);
                                        break;
                                    case 280:
                                        mnem = "mtasr";
                                        printf("\n  %08x:\t%s\tr%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to ASR", rs);
                                        break;
                                    case 282:
                                        mnem = "mtear";
                                        printf("\n  %08x:\t%s\tr%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to EAR", rs);
                                        break;
                                    case 284:
                                        mnem = "mttb";
                                        printf("\n  %08x:\t%s\tr%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to TB", rs);
                                        break;
                                    case 285:
                                        mnem = "mttbu";
                                        printf("\n  %08x:\t%s\tr%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to TBU", rs);
                                        break;
                                    case 528:
                                        mnem = "mtibatu";
                                        printf("\n  %08x:\t%s\t0,r%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to IBAT0U", rs);
                                        break;
                                    case 529:
                                        mnem = "mtibatl";
                                        printf("\n  %08x:\t%s\t0,r%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to IBAT0L", rs);
                                        break;
                                    case 530:
                                        mnem = "mtibatu";
                                        printf("\n  %08x:\t%s\t1,r%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to IBAT1U", rs);
                                        break;
                                    case 531:
                                        mnem = "mtibatl";
                                        printf("\n  %08x:\t%s\t1,r%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to IBAT1L", rs);
                                        break;
                                    case 532:
                                        mnem = "mtibatu";
                                        printf("\n  %08x:\t%s\t2,r%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to IBAT2U", rs);
                                        break;
                                    case 533:
                                        mnem = "mtibatl";
                                        printf("\n  %08x:\t%s\t2,r%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to IBAT2L", rs);
                                        break;
                                    case 534:
                                        mnem = "mtibatu";
                                        printf("\n  %08x:\t%s\t3,r%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to IBAT3U", rs);
                                        break;
                                    case 535:
                                        mnem = "mtibatl";
                                        printf("\n  %08x:\t%s\t3,r%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to IBAT3L", rs);
                                        break;

                                    case 536:
                                        mnem = "mtdbatu";
                                        printf("\n  %08x:\t%s\t0,r%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to DBAT0U", rs);
                                        break;
                                    case 537:
                                        mnem = "mtdbatl";
                                        printf("\n  %08x:\t%s\t0,r%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to DBAT0L", rs);
                                        break;
                                    case 538:
                                        mnem = "mtdbatu";
                                        printf("\n  %08x:\t%s\t1,r%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to DBAT1U", rs);
                                        break;
                                    case 539:
                                        mnem = "mtdbatl";
                                        printf("\n  %08x:\t%s\t1,r%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to DBAT1L", rs);
                                        break;
                                    case 540:
                                        mnem = "mtdbatu";
                                        printf("\n  %08x:\t%s\t2,r%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to DBAT2U", rs);
                                        break;
                                    case 541:
                                        mnem = "mtdbatl";
                                        printf("\n  %08x:\t%s\t2,r%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to DBAT2L", rs);
                                        break;
                                    case 542:
                                        mnem = "mtdbatu";
                                        printf("\n  %08x:\t%s\t3,r%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to DBAT3U", rs);
                                        break;
                                    case 543:
                                        mnem = "mtdbatl";
                                        printf("\n  %08x:\t%s\t3,r%d\t\t",
                                                line, mnem, rs);
                                        printf("# move r%d to DBAT3L", rs);
                                        break;
                                    default:
                                        mnem = "mtspr";
                                        comment = "# move";
                                        printf("\n  %08x:\t%s\t%d,r%d\t\t%s",
                                            line, mnem, spr, rs, comment );
                                        printf(" r%d to spr", rs);
                                        break;
                                    }
                                }
                            else
                                {
                                mnem = "mtspr";
                                comment = "# move";
                                printf("\n  %08x:\t%s\t%d,r%d\t\t%s r%d to spr",
                                    line, mnem, spr, rs, comment, rs );
                                }
                            continue;

                        case 470:              /* dcbi */
                            mnem = "dcbi";
                            comment = "# Data Cache Block Invalidate - disasm incomplete";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 476:               /* nand */
                            if ( rc )
                                mnem = "nand.";
                            else
                                mnem = "nand";
                            printf("\n  %08x:\t%s\tr%d,r%d,r%d\t# complement ",
                                    line, mnem, ra, rs, rb );
                            printf("r%d anded with r%d", rs, rb);
                            continue;

                        case 491:               /* divide word */
                        case 1003:
                            switch (oerc)
                                {
                                case 0:
                                    comment = "";
                                    mnem = "divw";
                                    break;

                                case 1:
                                    comment = ", record";
                                    mnem = "divw.";
                                    break;

                                case 2:
                                    comment = ", overflow";
                                    mnem = "divwo";
                                    break;

                                case 3:
                                    comment = ", record, overflow";
                                    mnem = "divwo.";
                                    break;
                                }
                            printf("\n  %08x:\t%s\tr%d,r%d,r%d\t# divide",
                                    line, mnem, rt, ra, rb );
                            printf(" r%d by r%d%s", ra, rb, comment);
                            continue;

                        case 512:               /* move to cond reg from XER */
                            mnem = "mcrxr";
                            comment = "# move to cond reg from xer";
                            printf("\n  %08x:\t%s\t%d\t\t%s",
                                    line, mnem, bf );
                            continue;

                        case 533:               /* Load string word indexed */
                            comment = "# load string word indexed";
                            mnem = "lswx";
                            printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s", line,
                                mnem, rt, ra, rb, comment);
                            continue;

                        case 534:               /* Load word byte-reversed */
                            comment = "# load word byte-reversed indexed";
                            mnem = "lwbrx";
                            printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s", line,
                                mnem, rt, ra, rb, comment);
                            continue;

                        case 535:              /* lfsx */
                            mnem = "lfsx";
                            comment = "# Load Float, Single, Indexed - disasm incomplete";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 536:               /* shift right word */
                            if ( rc )
                                mnem = "srw.";
                            else
                                mnem = "srw";
                            printf("\n  %08x:\t%s\tr%d,r%d,r%d\t# shift",
                                    line, mnem, ra, rs, rb );
                            printf(" r%d right by (r%d)", rs, rb );

                        case 567:              /* lfsux */
                            mnem = "lfsux";
                            comment = "# Load Float, Single, with Update Indexed - disasm incomplete";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;
                            continue;

                        case 595:               /* Move from segreg */
                            mnem = "mfsr";
                            printf("\n  %08x:\t%s\t%d,r%d\t\t", line,
                                mnem, rt, sr );
                            printf("# move sr%d to r%d", sr, rt );
                            continue;

                        case 597:               /* Load string word immediate */
                            comment = "# load string word immediate";
                            mnem = "lswi";
                            printf("\n  %08x:\t%s\tr%d,r%d,%d  \t%s", line,
                                mnem, rt, ra, rb, comment);
                            continue;

                        case 598:              /* sync */
                            mnem = "sync";
                            comment = "# Synchronize";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 599:              /* lfdx */
                            mnem = "lfdx";
                            comment = "# Load Float, Double, Indexed - disasm incomplete";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 631:              /* lfdux */
                            mnem = "lfdux";
                            comment = "# Load Float, Double, with Update Indexed - disasm incomplete";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 659:               /* Move from segreg indirect */
                            mnem = "mfsrin";
                            printf("\n  %08x:\t%s\tr%d,r%d\t\t", line,
                                mnem, rt, rb);
                            printf("# move seg reg indirect to r%d", rt );
                            continue;

                        case 661:               /* Store string word indexed */
                            comment = "# store string word indexed";
                            mnem = "stswx";
                            printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s", line,
                                mnem, rt, ra, rb, comment);
                            continue;

                        case 662:               /* Store word byte-reversed */
                            comment = "# store word byte-reversed indexed";
                            mnem = "stwbrx";
                            printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s", line,
                                mnem, rt, ra, rb, comment);
                            continue;

                        case 663:              /* stfsx */
                            mnem = "stfsx";
                            comment = "# Store Float, Single, Indexed - disasm incomplete";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 695:              /* stfsux */
                            mnem = "stfsux";
                            comment = "# Store Float, Single, with Update Indexed - disasm incomplete";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 725:               /* Store str word immediate */
                            comment = "# store string word immediate";
                            mnem = "stswi";
                            printf("\n  %08x:\t%s\tr%d,r%d,%d  \t%s", line,
                                mnem, rs, ra, rb, comment);
                            continue;

                        case 727:              /* stfdx */
                            mnem = "stfdx";
                            comment = "# Store Float, Double, Indexed - disasm incomplete";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 759:              /* stfdux */
                            mnem = "stfdux";
                            comment = "# Store Float, Double, with Update Indexed - disasm incomplete";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 792:               /* shift right al word */
                            if ( rc )
                                mnem = "sraw.";
                            else
                                mnem = "sraw";
                            printf("\n  %08x:\t%s\tr%d,r%d,r%d\t# shift",
                                    line, mnem, ra, rs, rb );
                            printf(" r%d right algebraic", rs );
                            continue;

                        case 824:               /* shift right al word */
                            if ( rc )
                                mnem = "srawi.";
                            else
                                mnem = "srawi";
                            printf("\n  %08x:\t%s\tr%d,r%d,%d  \t# shift",
                                    line, mnem, ra, rs, rb );
                            printf(" r%d right algebraic %d bits", rs, rb );
                            continue;

                        case 854:              /* eieio */
                            mnem = "eieio";
                            comment = "# Enforce in-order Execution of I/O";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 922:               /* extend sign hword */
                            if ( rc )
                                mnem = "extsh.";
                            else
                                mnem = "extsh";
                            comment = "# extend sign halfword";
                            printf("\n  %08x:\t%s\tr%d,r%d\t\t%s",
                                    line, mnem, ra, rs, comment );
                            continue;

                        case 954:               /* extend sign byte */
                            if ( rc )
                                mnem = "extsb.";
                            else
                                mnem = "extsb";
                            comment = "# extend sign byte";
                            printf("\n  %08x:\t%s\tr%d,r%d\t%s",
                                    line, mnem, ra, rs, comment );
                            continue;

                        case 982:              /* eieio */
                            mnem = "icbi";
                            comment = "# Instructino Cache Block Invalidate";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 983:              /* stfiwx */
                            mnem = "stfiwx";
                            comment = "# Store Float as Integer Word Indexed - disasm incomplete";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        case 986:               /* extend sign word */
                            if ( rc )
                                mnem = "extsw.";
                            else
                                mnem = "extsw";
                            comment = "# extend sign word";
                            printf("\n  %08x:\t%s\tr%d,r%d\t%s",
                                    line, mnem, ra, rs, comment );
                            continue;

                        case 1014:              /* dcbz */
                            mnem = "dcbz";
                            comment = "# Data Cache Block Zero - disasm incomplete";
                            printf("\n  %08x:\t%s\t\t\t%s",
                                line, mnem, comment);
                            continue;

                        default:
                            printf("\n  %08x:\t%08x", line, ins);
                            printf("\t\t# unrecognized opcode");
                            continue;
                        }

                case 32:                        /* load word and zero D */
                    comment = "# load word,";
                    mnem = "lwz";
                    printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s offset 0x%04x",
                        line, mnem, rt, d, ra, comment, d);
                    continue;

                case 33:                        /* load wd and zero update D */
                    comment = "# load word, update,";
                    mnem = "lwzu";
                    printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s offset 0x%04x",
                        line, mnem, rt, d, ra, comment, d);
                    continue;

                case 34:                        /* load byte and zero D */
                    comment = "# load byte,";
                    mnem = "lbz";
                    printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s offset 0x%04x",
                        line, mnem, rt, d, ra, comment, d);
                    continue;

                case 35:                        /* load byte and zero upd D */
                    comment = "# load byte, update,";
                    mnem = "lbzu";
                    printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s offset 0x%04x",
                        line, mnem, rt, d, ra, comment, d);
                    continue;

                case 36:                        /* store word */
                    comment = "# store word,";
                    mnem = "stw";
                    printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s offset 0x%04x",
                        line, mnem, rs, d, ra, comment, d);
                    continue;

                case 37:                        /* store word */
                    comment = "# store word, update,";
                    mnem = "stwu";
                    printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s offset 0x%04x",
                        line, mnem, rs, d, ra, comment, d);
                    continue;

                case 38:                        /* store byte */
                    comment = "# store byte,";
                    mnem = "stb";
                    printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s offset 0x%04x",
                        line, mnem, rs, d, ra, comment, d);
                    continue;

                case 39:                        /* store byte with update */
                    comment = "# store byte, update,";
                    mnem = "stbu";
                    printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s offset 0x%04x",
                        line, mnem, rs, d, ra, comment, d);
                    continue;

                case 40:                        /* load hword and zero D */
                    comment = "# load halfword,";
                    mnem = "lhz";
                    printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s offset 0x%04x",
                        line, mnem, rt, d, ra, comment, d);
                    continue;

                case 41:                        /* load hword and zero upd D */
                    comment = "# load halfword, update,";
                    mnem = "lhzu";
                    printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s offset 0x%04x",
                        line, mnem, rt, d, ra, comment, d);
                    continue;

                case 42:                        /* load hword algebraic D */
                    comment = "# load halfword,";
                    mnem = "lha";
                    printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s offset 0x%04x",
                        line, mnem, rt, d, ra, comment, d);
                    continue;

                case 43:                        /* load hword alge update */
                    comment = "# load halfword, update,";
                    mnem = "lhau";
                    printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s offset 0x%04x",
                        line, mnem, rt, d, ra, comment, d);
                    continue;

                case 44:                        /* store halfword */
                    comment = "# store halfword,";
                    mnem = "sth";
                    printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s offset 0x%04x",
                        line, mnem, rs, d, ra, comment, d);
                    continue;

                case 45:                        /* store halfword upd */
                    comment = "# store halfword, update,";
                    mnem = "sthu";
                    printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s offset 0x%04x",
                        line, mnem, rs, d, ra, comment, d);
                    continue;

                case 46:                        /* load mult word */
                    comment = "# load mult word,";
                    mnem = "lmw";
                    printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s offset 0x%04x",
                        line, mnem, rt, d, ra, comment, d);
                    continue;

                case 47:                        /* store mult word */
                    comment = "# store mult word,";
                    mnem = "stmw";
                    printf("\n  %08x:\t%s\tr%d,%d(r%d)\t%s offset 0x%04x",
                        line, mnem, rs, d, ra, comment, d);
                    continue;

                case 48:                        /* store mult word */
                    comment = "# load float single,";
                    mnem = "lfs";
                    printf("\n  %08x:\t%s\t\t\t%s",
                        line, mnem, comment);
                    continue;

                case 49:
                    comment = "# load float single, with update,";
                    mnem = "lfsu";
                    printf("\n  %08x:\t%s\t\t\t%s",
                        line, mnem, comment);
                    continue;

                case 50:
                    comment = "# load float double,";
                    mnem = "lfd";
                    printf("\n  %08x:\t%s\t\t\t%s",
                        line, mnem, comment);
                    continue;

                case 51:
                    comment = "# load float double, with update,";
                    mnem = "lfdu";
                    printf("\n  %08x:\t%s\t\t\t%s",
                        line, mnem, comment);
                    continue;

                case 52:
                    comment = "# store float single,";
                    mnem = "stfs";
                    printf("\n  %08x:\t%s\t\t\t%s",
                        line, mnem, comment);
                    continue;

                case 53:
                    comment = "# store float single, with update,";
                    mnem = "stfsu";
                    printf("\n  %08x:\t%s\t\t\t%s",
                        line, mnem, comment);
                    continue;

                case 54:
                    comment = "# store float double,";
                    mnem = "stfd";
                    printf("\n  %08x:\t%s\t\t\t%s",
                        line, mnem, comment);
                    continue;

                case 55:
                    comment = "# store float double, with update,";
                    mnem = "stfdu";
                    printf("\n  %08x:\t%s\t\t\t%s",
                        line, mnem, comment);
                    continue;

                case 59:
                    comment = "# Floating Point Something - disasm incomplete";
                    mnem = "f? (op59)";
                    printf("\n  %08x:\t%s\t\t\t%s",
                        line, mnem, comment);
                    continue;

                case 63:
                    comment = "# Floating Point Something - disasm incomplete";
                    mnem = "f? (op63)";
                    printf("\n  %08x:\t%s\t\t\t%s",
                        line, mnem, comment);
                    continue;

                default:
                    printf("\n  %08x:\t%08x", line, ins);
                    printf("\t\t# unrecognized opcode");
                    continue;
                }
            }
        printf("\n");

        return rval;
     }

#else
int disassemble ( startaddr, image, extmnem, size )
        int     startaddr;
        int     image[];
        int     extmnem;
        int     size;
     {
        printf("\nDisassembler removed from this version to conserve space.\n");

   return 0;
}
#endif
#endif /*DEBUG*/
