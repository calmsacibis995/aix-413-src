static char sccsid[] = "@(#)76	1.24.2.8  src/bos/usr/sbin/lvm/liblv/lvm_msg.c, cmdlvm, bos411, 9428A410j 3/28/94 15:39:45";
/*
 * COMPONENT_NAME: (cmdlvm) Logical Volume Commands
 *
 * FUNCTIONS: lvm_msg
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

/*
 * FILENAME: lvm_msg.c
 *
 * FILE DESCRIPTION: Function to return all cmdlvm NLS messages.
 */


#include <stdio.h>
#include <lvm.h>

/* include file for message texts */
#include "cmdlvm_msg.h"
#include "cmdlvm_def.h"

/* Global Variables */

char *Prog;	        /* Program name */
nl_catd  scmc_catd;	/* NLS Cat Descriptor */


char *lvm_msgstr (msg_num)
int msg_num;
{
  char *msg;
  static char error_str[100];

  switch (msg_num) {
	case VGDELETED_LVM:      msg=MSG_VGDELETED_LVM;      break;
	case CHKVVGOUT_LVM:      msg=MSG_CHKVVGOUT_LVM;      break;
	case REMRET_INCOMP_LVM:  msg=MSG_REMRET_INCOMP_LVM;  break;
	case UNKNERR_LVM:        msg=MSG_UNKNERR_LVM;        break;
	case OFFLINE_LVM:        msg=MSG_OFFLINE_LVM;        break;
	case LVOPEN_LVM:         msg=MSG_LVOPEN_LVM;         break;
	case VGMEMBER_LVM:       msg=MSG_VGMEMBER_LVM;       break;
	case PARTFND_LVM:        msg=MSG_PARTFND_LVM;        break;
	case VGFULL_LVM:         msg=MSG_VGFULL_LVM;         break;
	case LVEXIST_LVM:        msg=MSG_LVEXIST_LVM;        break;
	case INVAL_PARAM_LVM:    msg=MSG_INVAL_PARAM_LVM;    break;
	case PVOPNERR_LVM:       msg=MSG_PVOPNERR_LVM;       break;
	case NOALLOCLP_LVM:      msg=MSG_NOALLOCLP_LVM;      break;
	case MAPFOPN_LVM:        msg=MSG_MAPFOPN_LVM;        break;
	case LPNUM_INVAL_LVM:    msg=MSG_LPNUM_INVAL_LVM;    break;
	case PPNUM_INVAL_LVM:    msg=MSG_PPNUM_INVAL_LVM;    break;
	case DALVOPN_LVM:        msg=MSG_DALVOPN_LVM;        break;
	case INVAL_MIN_NUM_LVM:  msg=MSG_INVAL_MIN_NUM_LVM;  break;
	case PVDAREAD_LVM:       msg=MSG_PVDAREAD_LVM;       break;
	case PVSTATE_INVAL_LVM:  msg=MSG_PVSTATE_INVAL_LVM;  break;
	case MAPFRDWR_LVM:       msg=MSG_MAPFRDWR_LVM;       break;
	case NODELLV_LVM:        msg=MSG_NODELLV_LVM;        break;
	case PVMAXERR_LVM:       msg=MSG_PVMAXERR_LVM;       break;
	case VGDASPACE_LVM:      msg=MSG_VGDASPACE_LVM;      break;
	case NOQUORUM_LVM:       msg=MSG_NOQUORUM_LVM;       break;
	case MISSPVNAME_LVM:     msg=MSG_MISSPVNAME_LVM;     break;
	case MISSINGPV_LVM:      msg=MSG_MISSINGPV_LVM;      break;
	case ALLOCERR_LVM:       msg=MSG_ALLOCERR_LVM;       break;
	case RDPVID_LVM:         msg=MSG_RDPVID_LVM;         break;
	case LVMRECERR_LVM:      msg=MSG_LVMRECERR_LVM;      break;
	case WRTDAERR_LVM:       msg=MSG_WRTDAERR_LVM;       break;
	case NOTVGMEM_LVM:       msg=MSG_NOTVGMEM_LVM;       break;
	case NOTSYNCED_LVM:      msg=MSG_NOTSYNCED_LVM;      break;
	case PROGERR_LVM:        msg=MSG_PROGERR_LVM;        break;
	case BADBBDIR_LVM:       msg=MSG_BADBBDIR_LVM;       break;
	case INRESYNC_LVM:       msg=MSG_INRESYNC_LVM;       break;
	case INVLPRED_LVM:       msg=MSG_INVLPRED_LVM;       break;
	case INVCONFIG_LVM:      msg=MSG_INVCONFIG_LVM;      break;
	case NOTCHARDEV_LVM:     msg=MSG_NOTCHARDEV_LVM;     break;
	case NOTCHARDEVLVM:      msg=MSG_NOTCHARDEVLVM;      break;
	case INV_DEVENT_LVM:     msg=MSG_INV_DEVENT_LVM;     break;
	case INVDEVENTLVM:       msg=MSG_INVDEVENTLVM;       break;
	case BELOW_QRMCNT_LVM:   msg=MSG_BELOW_QRMCNT_LVM;   break;
	case VGDA_BB_LVM:        msg=MSG_VGDA_BB_LVM;        break;
	case MIGRATE_FAIL_LVM:   msg=MSG_MIGRATE_FAIL_LVM;   break;
	case MEMACTVVG_LVM:      msg=MSG_MEMACTVVG_LVM;      break;
	case FORCEOFF_LVM:       msg=MSG_FORCEOFF_LVM;       break;
	case NOVGDAS_LVM:        msg=MSG_NOVGDAS_LVM;        break;
	case ALRDYMEM_LVM:       msg=MSG_ALRDYMEM_LVM;       break;
	case NOKMID_LVM:         msg=MSG_NOKMID_LVM;         break;
	case MAPFBSY_LVM:        msg=MSG_MAPFBSY_LVM;        break;
	case VONOTHMAJ_LVM:      msg=MSG_VONOTHMAJ_LVM;      break;
	case BADVERSION_LVM:     msg=MSG_BADVERSION_LVM;     break;
	case MISSING_LVID:       msg=MSG_MISSING_LVID;       break;
	case BADLVID:            msg=MSG_BADLVID;            break;
	case MISSING_VGID:       msg=MSG_MISSING_VGID;       break;
	case MISSING_PVID:       msg=MSG_MISSING_PVID;       break;
	case MISSING_LVNM:       msg=MSG_MISSING_LVNM;       break;
	case MISSING_MINOR:      msg=MSG_MISSING_MINOR;      break;
	case MISSING_MAJ:        msg=MSG_MISSING_MAJ;        break;
	case MISSING_PVNM:       msg=MSG_MISSING_PVNM;       break;
	case MISSING_MAXLV:      msg=MSG_MISSING_MAXLV;      break;
	case MISSING_DASZ:       msg=MSG_MISSING_DASZ;       break;
	case MISSING_PPSZ:       msg=MSG_MISSING_PPSZ;       break;
	case MISSING_ESZ:        msg=MSG_MISSING_ESZ;        break;
	case MISSING_MID:        msg=MSG_MISSING_MID;        break;
	case MISSING_FN:         msg=MSG_MISSING_FN;         break;
	case MISSING_MAP:        msg=MSG_MISSING_MAP;        break;
	case EXPECT_MAP:         msg=MSG_EXPECT_MAP;         break;
	case EXPECT_INPT:        msg=MSG_EXPECT_INPT;        break;
	case RECEIVED:           msg=MSG_RECEIVED;           break;
	case RDERR_MAP:          msg=MSG_RDERR_MAP;          break;
	case MISSING_VGNAME:     msg=MSG_MISSING_VGNAME;     break;
	case CHLV_USAGE:         msg=MSG_CHLV_USAGE;         break;
	case CHPV_USAGE:         msg=MSG_CHPV_USAGE;         break;
	case CRLV_USAGE:         msg=MSG_CRLV_USAGE;         break;
	case CRVG_USAGE:         msg=MSG_CRVG_USAGE;         break;
	case DLV_USAGE:          msg=MSG_DLV_USAGE;          break;
	case DPV_USAGE:          msg=MSG_DPV_USAGE;          break;
	case EXLV_USAGE:         msg=MSG_EXLV_USAGE;         break;
	case MISSING_SZ:         msg=MSG_MISSING_SZ;         break;
	case IPV_USAGE:          msg=MSG_IPV_USAGE;          break;
	case MPP_USAGE:          msg=MSG_MPP_USAGE;          break;
	case MISSING_OPVID:      msg=MSG_MISSING_OPVID;      break;
	case MISSING_OPPN:       msg=MSG_MISSING_OPPN;       break;
	case MISSING_NPVID:      msg=MSG_MISSING_NPVID;      break;
	case MISSING_NPPN:       msg=MSG_MISSING_NPPN;       break;
	case QLV_USAGE:          msg=MSG_QLV_USAGE;          break;
	case QPV_USAGE:          msg=MSG_QPV_USAGE;          break;
	case MISSING_VGPV:       msg=MSG_MISSING_VGPV;       break;
	case QVG_USAGE:          msg=MSG_QVG_USAGE;          break;
	case QVGS_USAGE:         msg=MSG_QVGS_USAGE;         break;
	case RLV_USAGE:          msg=MSG_RLV_USAGE;          break;
	case RSLP_USAGE:         msg=MSG_RSLP_USAGE;         break;
	case MISSING_LPN:        msg=MSG_MISSING_LPN;        break;
	case RSLV_USAGE:         msg=MSG_RSLV_USAGE;         break;
	case RSPV_USAGE:         msg=MSG_RSPV_USAGE;         break;
	case VOFF_USAGE:         msg=MSG_VOFF_USAGE;         break;
	case VON_USAGE:          msg=MSG_VON_USAGE;          break;
	case VON_SUCCESS:        msg=MSG_VON_SUCCESS;        break;
	case GETODM_USE:         msg=MSG_GETODM_USE;         break;
	case BAD_VGID:           msg=MSG_BAD_VGID;           break;
	case ID_NOT_FOUND:       msg=MSG_ID_NOT_FOUND;       break;
        case DEV_NOT_FOUND:      msg=MSG_DEV_NOT_FOUND;      break;
	case ODM_DOWN:           msg=MSG_ODM_DOWN;           break;
        case CH_NOT_FOUND:       msg=MSG_CH_NOT_FOUND;       break;
	case ATTR_NOT_FOUND:     msg=MSG_ATTR_NOT_FOUND;     break;
	case ATTRNOTFOUND:       msg=MSG_ATTRNOTFOUND;       break;
	case LV_NOT_FOUND:       msg=MSG_LV_NOT_FOUND;       break;
	case LV_NOT_FOUND2:      msg=MSG_LV_NOT_FOUND2;      break;
	case VG_NOT_FOUND:       msg=MSG_VG_NOT_FOUND;       break;
	case NO_FREE_PVS:        msg=MSG_NO_FREE_PVS;        break;
        case NO_VGS_NOW:         msg=MSG_NO_VGS_NOW;         break;
	case PV_NOT_CONF:        msg=MSG_PV_NOT_CONF;        break;
	case PV_NOT_IN_VG:       msg=MSG_PV_NOT_IN_VG;       break;
	case PVS_NOT_FOUND:      msg=MSG_PVS_NOT_FOUND;      break;
	case NO_PVS_ON_VG:       msg=MSG_NO_PVS_ON_VG;       break;
	case PUTODM_USE:         msg=MSG_PUTODM_USE;         break;
	case MISSING_ID:         msg=MSG_MISSING_ID;         break;
	case BAD_FLAG:           msg=MSG_BAD_FLAG;           break;
	case BAD_FLAG2:          msg=MSG_BAD_FLAG2;          break;
	case MAJ_NUM_MISSING:    msg=MSG_MAJ_NUM_MISSING;    break;
	case NAME_TRUNC:         msg=MSG_NAME_TRUNC;         break;
	case LVNAME_USE:         msg=MSG_LVNAME_USE;         break;
	case SEQ_ERROR:          msg=MSG_SEQ_ERROR;          break;
	case VGNAME_USE:         msg=MSG_VGNAME_USE;         break;
	case GENMAJ_USE:         msg=MSG_GENMAJ_USE;         break;
	case GENMAJ_ERROR:       msg=MSG_GENMAJ_ERROR;       break;
	case GENMIN_USE:         msg=MSG_GENMIN_USE;         break;
	case MISSING_PARMS:      msg=MSG_MISSING_PARMS;      break;
	case MISSING_MAJNUM:     msg=MSG_MISSING_MAJNUM;     break;
	case GENMINOR_ERR:       msg=MSG_GENMINOR_ERR;       break;
	case RELSEQ_ERR:         msg=MSG_RELSEQ_ERR;         break;
	case RELMIN_USE:         msg=MSG_RELMIN_USE;         break;
	case RELMINOR_ERR:       msg=MSG_RELMINOR_ERR;       break;
	case PREFIX_EXISTS:      msg=MSG_PREFIX_EXISTS;      break;
	case NAME_EXISTS:        msg=MSG_NAME_EXISTS;        break;
	case LVODM_ERROR:        msg=MSG_LVODM_ERROR;        break;
	case ATTR_NOT_ADDED:     msg=MSG_ATTR_NOT_ADDED;     break;
	case VG_LOCKED:          msg=MSG_VG_LOCKED;          break;
	case REL_MIN_FAIL:       msg=MSG_REL_MIN_FAIL;       break;
	case GEN_MIN_FAIL:       msg=MSG_GEN_MIN_FAIL;       break;
	case GET_KMID_FAIL:      msg=MSG_GET_KMID_FAIL;      break;
        case CHKMAJ_ERROR:       msg=MSG_CHKMAJ_ERROR;       break;
        case CHKMAJ_PARMS:       msg=MSG_CHKMAJ_PARMS;       break;
        case CHKMAJ_USE:         msg=MSG_CHKMAJ_USE;         break;
        case LSTMAJ_ERROR:       msg=MSG_LSTMAJ_ERROR;       break;
        case LSTMAJ_USE:         msg=MSG_LSTMAJ_USE;         break;
        case ALLOCP_MAP_NOFREE:  msg=MSG_ALLOCP_MAP_NOFREE;  break;
	case ALLOCP_SCANFAIL:    msg=MSG_ALLOCP_SCANFAIL;    break;
	case ALLOCP_NORESOURCE:  msg=MSG_ALLOCP_NORESOURCE;  break;
	case ALLOCP_STRICTERROR: msg=MSG_ALLOCP_STRICTERROR; break;
	case ALLOCP_PARSELVID:   msg=MSG_ALLOCP_PARSELVID;   break;
	case ALLOCP_PARSECOPY:   msg=MSG_ALLOCP_PARSECOPY;   break;
	case ALLOCP_PARSESIZE:   msg=MSG_ALLOCP_PARSESIZE;   break;
	case ALLOCP_PARSEBOUND:  msg=MSG_ALLOCP_PARSEBOUND;  break;
	case ALLOCP_PARMCOMB:    msg=MSG_ALLOCP_PARMCOMB;    break;
	case ALLOCP_LINERROR:    msg=MSG_ALLOCP_LINERROR;    break;
	case ALLOCP_INTRAERROR:  msg=MSG_ALLOCP_INTRAERROR;  break;
	case ALLOCP_USAGE:       msg=MSG_ALLOCP_USAGE;       break;
	case ALLOCP_MAPFILELOW:  msg=MSG_ALLOCP_MAPFILELOW;  break;
	case 500:                msg=MSG_500;                break;
	case 502:                msg=MSG_502;                break;
	case 504:                msg=MSG_504;                break;
	case 506:                msg=MSG_506;                break;
	case 508:                msg=MSG_508;                break;
	case 510:                msg=MSG_510;                break;
	case 520:                msg=MSG_520;                break;
	case 521:                msg=MSG_521;                break;
        case 522:                msg=MSG_522;                break;
	case 524:                msg=MSG_524;                break;
	case 526:                msg=MSG_526;                break;
	case 528:                msg=MSG_528;                break;
	case 530:                msg=MSG_530;                break;
	case 532:                msg=MSG_532;                break;
        case 540:                msg=MSG_540;                break;
	case 542:                msg=MSG_542;                break;
	case 544:                msg=MSG_544;                break;
	case 546:                msg=MSG_546;                break;
	case 548:                msg=MSG_548;                break;
	case 550:                msg=MSG_550;                break;
	case 552:                msg=MSG_552;                break;
	case 556:                msg=MSG_556;                break;
	case 558:                msg=MSG_558;                break;
	case 560:                msg=MSG_560;                break;
	case 562:                msg=MSG_562;                break;
	case 564:                msg=MSG_564;                break;
	case 566:                msg=MSG_566;                break;
	case 567:                msg=MSG_567;                break;
        case 568:                msg=MSG_568;                break;
	case 570:                msg=MSG_570;                break;
	case 572:                msg=MSG_572;                break;
	case 574:                msg=MSG_574;                break;
	case 576:                msg=MSG_576;                break;
	case 602:                msg=MSG_602;                break;
	case 604:                msg=MSG_604;                break;
	case 606:                msg=MSG_606;                break;
	case 608:                msg=MSG_608;                break;
	case 610:                msg=MSG_610;                break;
	case 612:                msg=MSG_612;                break;
	case 613:                msg=MSG_613;                break;
	case 614:                msg=MSG_614;                break;
	case 615:                msg=MSG_615;                break;
	case 616:                msg=MSG_616;                break;
	case 618:                msg=MSG_618;                break;
	case 620:                msg=MSG_620;                break;
	case 621:                msg=MSG_621;                break;
	case 622:                msg=MSG_622;                break;
	case 624:                msg=MSG_624;                break;
	case 626:                msg=MSG_626;                break;
	case 628:                msg=MSG_628;                break;
	case 630:                msg=MSG_630;                break;
	case 631:                msg=MSG_631;                break;
        case 632:                msg=MSG_632;                break;
        case 633:                msg=MSG_633;                break;  
	case 634:                msg=MSG_634;                break;
        case 635:                msg=MSG_635;                break;
        case 636:                msg=MSG_636;                break;
        case 637:                msg=MSG_637;                break;
        case 638:                msg=MSG_638;                break;
        case 639:                msg=MSG_639;                break;
        case 640:                msg=MSG_640;                break;
        case 641:                msg=MSG_641;                break;
        case 642:                msg=MSG_642;                break;
        case 643:                msg=MSG_643;                break;
        case 644:                msg=MSG_644;                break;
        case 645:                msg=MSG_645;                break;
        case 646:                msg=MSG_646;                break;
        case 647:                msg=MSG_647;                break;
        case 648:                msg=MSG_648;                break;
        case 649:                msg=MSG_649;                break;
        case 650:                msg=MSG_650;                break;
        case 651:                msg=MSG_651;                break; 
        case 652:                msg=MSG_652;                break;
	case 654:                msg=MSG_654;                break;
	case 656:                msg=MSG_656;                break;
	case 658:                msg=MSG_658;                break;
	case 660:                msg=MSG_660;                break;
	case 662:                msg=MSG_662;                break;
	case 664:                msg=MSG_664;                break;
	case 666:                msg=MSG_666;                break;
	case 668:                msg=MSG_668;                break;
	case 670:                msg=MSG_670;                break;
	case 672:                msg=MSG_672;                break;
	case 674:                msg=MSG_674;                break;
	case 676:                msg=MSG_676;                break;
	case 678:                msg=MSG_678;                break;
	case 680:                msg=MSG_680;                break;
	case 681:                msg=MSG_681;                break;
	case 682:                msg=MSG_682;                break;
	case 684:                msg=MSG_684;                break;
	case 686:                msg=MSG_686;                break;
	case 688:                msg=MSG_688;                break;
	case 690:                msg=MSG_690;                break;
	case 692:                msg=MSG_692;                break;
	case 693:                msg=MSG_693;                break;
	case 694:                msg=MSG_694;                break;
	case 696:                msg=MSG_696;                break;
	case 697:                msg=MSG_697;                break;
        case 698:                msg=MSG_698;                break;
        case 699:                msg=MSG_699;                break; 
        case 702:                msg=MSG_702;                break;
	case 704:                msg=MSG_704;                break;
	case 706:                msg=MSG_706;                break;
	case 708:                msg=MSG_708;                break;
	case 710:                msg=MSG_710;                break;
        case 711:                msg=MSG_711;                break;
        case 712:                msg=MSG_712;                break;
	case 720:                msg=MSG_720;                break;
	case 722:                msg=MSG_722;                break;
	case 724:                msg=MSG_724;                break;
        case 730:                msg=MSG_730;                break;
	case 732:                msg=MSG_732;                break;
	case 740:                msg=MSG_740;                break;
        case 741:                msg=MSG_741;                break;
	case 742:                msg=MSG_742;                break;
        case 743:                msg=MSG_743;                break;
	case 744:                msg=MSG_744;                break;
	case 746:                msg=MSG_746;                break;
	case 747:                msg=MSG_747;                break;
	case 748:                msg=MSG_748;                break;
	case 750:                msg=MSG_750;                break;
	case 752:                msg=MSG_752;                break;
        case 753:                msg=MSG_753;                break;
	case 754:                msg=MSG_754;                break;
	case 760:                msg=MSG_760;                break;
	case 762:                msg=MSG_762;                break;
	case 764:                msg=MSG_764;                break;
	case 766:                msg=MSG_766;                break;
	case 768:                msg=MSG_768;                break;
	case 770:                msg=MSG_770;                break;
	case 772:                msg=MSG_772;                break;
	case 774:                msg=MSG_774;                break;
	case 776:                msg=MSG_776;                break;
	case 778:                msg=MSG_778;                break;
	case 780:                msg=MSG_780;                break;
	case 782:                msg=MSG_782;                break;
	case 785:                msg=MSG_785;                break;
        case 786:                msg=MSG_786;                break;
	case 787:                msg=MSG_787;                break;
	case 788:                msg=MSG_788;                break;
	case 789:                msg=MSG_789;                break;
	case 790:                msg=MSG_790;                break;
	case 792:                msg=MSG_792;                break;
	case 794:                msg=MSG_794;                break;
	case 796:                msg=MSG_796;                break;
	case 800:                msg=MSG_800;                break;
	case 804:                msg=MSG_804;                break;
	case 806:                msg=MSG_806;                break;
	case 808:                msg=MSG_808;                break;
	case 810:                msg=MSG_810;                break;
	case 812:                msg=MSG_812;                break;
	case 814:                msg=MSG_814;                break;
        case 816:                msg=MSG_816;                break;
        case 817:                msg=MSG_817;                break;
        case 818:                msg=MSG_818;                break;
        case 820:                msg=MSG_820;                break;
	case 822:                msg=MSG_822;                break;
	case 823:                msg=MSG_823;                break;
        case 824:                msg=MSG_824;                break;
        case 825:                msg=MSG_825;                break;
	case 828:                msg=MSG_828;                break;
	case 830:                msg=MSG_830;                break;
        case 832:                msg=MSG_832;                break;
	case 834:		 msg=MSG_834;		     break;
        case 840:                msg=MSG_840;                break;
	case 842:                msg=MSG_842;                break;
	case 844:                msg=MSG_844;                break;
	case 846:                msg=MSG_846;                break;
	case 848:                msg=MSG_848;                break;
	case 850:                msg=MSG_850;                break;
	case 852:                msg=MSG_852;                break;
	case 860:                msg=MSG_860;                break;
	case 862:                msg=MSG_862;                break;
	case 864:                msg=MSG_864;                break;
	case 866:                msg=MSG_866;                break;
	case 868:                msg=MSG_868;                break;
	case 870:                msg=MSG_870;                break;
	case 872:                msg=MSG_872;                break;
	case 874:                msg=MSG_874;                break;
        case 880:                msg=MSG_880;                break;
	case 882:                msg=MSG_882;                break;
	case 884:                msg=MSG_884;                break;
	case 886:                msg=MSG_886;                break;
	case 888:                msg=MSG_888;                break;
	case 894:                msg=MSG_894;                break;
	case 896:                msg=MSG_896;                break;
	case 910:                msg=MSG_910;                break;
	case 912:                msg=MSG_912;                break;
	case 913:                msg=MSG_913;                break;
	case 914:                msg=MSG_914;                break;
        case 916:                msg=MSG_916;                break;
	case 918:                msg=MSG_918;                break;
	case 920:                msg=MSG_920;                break;
        case 921:                msg=MSG_921;                break;
        case 922:                msg=MSG_922;                break;
	case 930:                msg=MSG_930;                break;
	case 932:                msg=MSG_932;                break;
	case 934:                msg=MSG_934;                break;
        case 936:                msg=MSG_936;                break;
        case 937:                msg=MSG_937;                break;
        case 938:                msg=MSG_938;                break;
        case 940:                msg=MSG_940;                break;
	case 942:                msg=MSG_942;                break;
	case 950:                msg=MSG_950;                break;
	case 952:                msg=MSG_952;                break;
	case 954:                msg=MSG_954;                break;
	case 956:                msg=MSG_956;                break;
	case 957:                msg=MSG_957;                break;
	case 958:                msg=MSG_958;                break;
	case 959:                msg=MSG_959;                break;
	case 960:                msg=MSG_960;                break;
	case 962:                msg=MSG_962;                break;
	case 964:                msg=MSG_964;                break;
	case 966:                msg=MSG_966;                break;
	case 968:                msg=MSG_968;                break;
	case 970:                msg=MSG_970;                break;
	case 972:                msg=MSG_972;                break;
	case 982:                msg=MSG_982;                break;
	case 984:                msg=MSG_984;                break;
	case 988:                msg=MSG_988;                break;
	case 990:                msg=MSG_990;                break;
	case LS_BADOPS:          msg=MSG_LS_BADOPS;          break;
	case LS_BADLP:           msg=MSG_LS_BADLP;           break;
	case LS_LVUSAGE:         msg=MSG_LS_LVUSAGE;         break;
	case LS_PVUSAGE:         msg=MSG_LS_PVUSAGE;         break;
	case LS_VGUSAGE:         msg=MSG_LS_VGUSAGE;         break;
	case NLS_PVUSAGE:        msg=MSG_NLS_PVUSAGE;        break;
	case NLS_VGUSAGE:        msg=MSG_NLS_VGUSAGE;        break;
	case 1002:               msg=MSG_1002;               break;
	case MAJINUSE_LVM:       msg=MSG_MAJINUSE_LVM;       break;
	case LOSTPP_LVM:         msg=MSG_LOSTPP_LVM;         break;
	case EXTRAPP_LVM:        msg=MSG_EXTRAPP_LVM;        break;
	case RESYNC_FAILED_LVM:  msg=MSG_RESYNC_FAILED_LVM;  break;
	case 1003:               msg=MSG_1003;               break;
	case 1004:               msg=MSG_1004;               break;
	case 1005:               msg=MSG_1005;               break;
	case 1006:               msg=MSG_1006;               break;
	case 1008:               msg=MSG_1008;               break;
	case LMKTEMP_USAGE:      msg=MSG_LMKTEMP_USAGE;      break;
	case 1010:               msg=MSG_1010;               break;
	case 1011:               msg=MSG_1011;               break;
	case 1012:               msg=MSG_1012;               break;
	case 1013:               msg=MSG_1013;               break;
	case LVEXISTLVM:	 msg=MSG_LVEXISTLVM;	     break;
	case DALVOPNLVM:	 msg=MSG_DALVOPNLVM;	     break;
	case INVCONFIGLVM:	 msg=MSG_INVCONFIGLVM;	     break;
	case 1022:               msg=MSG_1022;               break;
	case 1023:               msg=MSG_1023;               break;
	case 1024:               msg=MSG_1024;               break;
	case 1025:               msg=MSG_1025;               break;
	case 1026:               msg=MSG_1026;               break;
	case 1027:               msg=MSG_1027;               break;
	case 1028:               msg=MSG_1028;               break;
	case 1029:               msg=MSG_1029;               break;
	case 1030:               msg=MSG_1030;               break;
	case 1031:               msg=MSG_1031;               break;
	case 1032:               msg=MSG_1032;               break;
	case 1033:               msg=MSG_1033;               break;
	case 1034:               msg=MSG_1034;               break;
	case 1035:               msg=MSG_1035;               break;
	case 1036:               msg=MSG_1036;               break;
	case 1037:               msg=MSG_1037;               break;
	case 1038:               msg=MSG_1038;               break;
	case 1039:               msg=MSG_1039;               break;
	case 1040:               msg=MSG_1040;               break;
	case 1041:               msg=MSG_1041;               break;
	case 1042:               msg=MSG_1042;               break;
	case 1043:               msg=MSG_1043;               break;
	case 1044:               msg=MSG_1044;               break;
	case 1046:               msg=MSG_1046;               break;
	case 1048:               msg=MSG_1048;               break;
	case 1049:               msg=MSG_1049;               break;
	case 1100:               msg=MSG_1100;               break;
	case 1101:               msg=MSG_1101;               break;
	case PVINTRO:            msg=MSG_PVINTRO;               break;
	case VGINTRO:            msg=MSG_VGINTRO;               break;
	
	default:
		sprintf(error_str,"Unknown LVM message: %d\n", msg_num);
		msg=error_str;
		break;
  }

  return(msg);

}


char *lvm_msg (msg_num)
int msg_num;
{

  return(catgets(scmc_catd, LVMSET, msg_num, lvm_msgstr(msg_num)));

}
