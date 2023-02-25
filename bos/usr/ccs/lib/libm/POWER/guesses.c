#if (!( _FRTINT | _SVID))
static char sccsid[] = "@(#)83	1.4  src/bos/usr/ccs/lib/libm/POWER/guesses.c, libm, bos411, 9428A410j 6/15/90 17:57:40";
#endif
/*
 * COMPONENT_NAME: (LIBM) math library
 *
 * FUNCTIONS: none
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 *                  SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* Define data table */
/*  sqrt table static ext, used for acos, asin*/

short guesses[256] ={
    27497,27752,28262,28517,28772,29282,29537,29792,30302,30557,31067,31322,
    31577,32088,32343,32598,33108,33363,33618,34128,34384,34639,35149,35404,
    35659,35914,36425,36680,36935,37446,37701,37956,38211,38722,38977,39232,
    39487,39998,40253,40508,40763,41274,41529,41784,42040,42550,42805,43061,
    43316,43571,44082,44337,44592,44848,45103,45358,45869,46124,46379,46635,
    46890,47401,47656,47911,48167,48422,48677,48933,49443,49699,49954,50209,
    50465,50720,50976,51231,51742,51997,52252,52508,52763,53019,53274,53529,
    53785,54296,54551,54806,55062,55317,55573,55828,56083,56339,56594,56850,
    57105,57616,57871,58127,58382,58638,58893,59149,59404,59660,59915,60170,
    60426,60681,60937,61192,61448,61703,61959,62214,62470,62725,62981,63236,
    63492,63747,64003,64258,64514,64769,65025,65280,511,510,764,1018,1272,1526,
    1780,2034,2288,2542,2796,3050,3305,3559,3813,4067,4321,4576,4830,5084,5338,
    5593,5847,6101,6101,6356,6610,6864,7119,7373,7627,7882,8136,8391,8391,8645,
    8899,9154,9408,9663,9917,10172,10172,10426,10681,10935,11190,11444,11699,
    11699,11954,12208,12463,12717,12972,13226,13226,13481,13736,13990,14245,
    14245,14500,14754,15009,15264,15518,15518,15773,16028,16282,16537,16537,
    16792,17047,17301,17556,17556,17811,18066,18320,18575,18575,18830,19085,
    19339,19339,19594,19849,20104,20104,20359,20614,20868,21123,21123,21378,
    21633,21888,21888,22143,22398,22653,22653,22907,23162,23417,23417,23672,
    23927,23927,24182,24437,24692,24692,24947,25202,25457,25457,25712,25967,
    25967,26222,26477,26732,26732,26987,27242 };