//Subversion reference
//$HeadURL: https://svn.ingenico.com/UTAPROJECTS/sea-partner/5W/trunk/ISO5WPH/str/str.h $
//$Id: str.h 2047 2009-05-22 03:58:55Z ajloreto $

#ifndef __STR_H

/** \defgroup strgroup STR Structure components
 *
 * @{
 */

#include "ctx.h"

#ifdef __TEST__
#define __mnuTEST__ "? Test>           "
#else
#define __mnuTEST__ "? Test>           "
#endif

#ifdef _TELIUM_2_
#define __DFT_CON_TYPE__ 2
#else //_TELIUM_2_
#ifdef CONN_LAN
#define __DFT_CON_TYPE__ 2
#else//CONN_LAN
#define __DFT_CON_TYPE__ 4
#endif//CONN_LAN
#endif//_TELIUM_2_

#define __DFTAPN__ "|banksimas|||"
//#define __DFTCHN__ "M8N2400"
//#define __DFTSRV__ "7519220"
#ifdef _TELIUM_2_ //@@AS0.0.20
#define __DFTCHN__ "T"
#else //_TELIUM_2_ @@AS0.0.20
#ifdef CONN_LAN
#define __DFTCHN__ "T"
#else//CONN_LAN
#define __DFTCHN__ "G" //@@AS0.0.20
#endif//CONN_LAN
#endif //_TELIUM_2_ @@AS0.0.20
//#define __DFTSRV__ "127.000.000.001|2000"
//#define __DFTSRV__ "010.017.032.031|2000"
//#define __DFTSRV__  "192.168.001.061|5030"                         //setting edc IP
#define __DFTPPP__ "080989999|telkomnet@instan|telkom|60"

//#define __DFTSRV__ "192.168.002.035|5050"
//#define __DFTSRV__ "192.168.002.059|9999"

//#define __DFTSRV__ "172.21.64.17|2000"
#ifdef SAMBUNG_KE_NAC
//#define __DFTSRV__ "10.35.65.42|8888" //NAC RAGUNAN

//#define __DFTSRV__ "10.35.65.43|5050" //NAC RAGUNAN

//#define __DFTSRV__ "6107"// Dial Up

#define __DFTSRV__ "192.168.002.097|1401" //TLE RAGUNAN
//#define __DFTSRV__ "10.35.65.173|7888" //TLE RAGUNAN CREDIT TEST
//#define __DFTSRV__ "10.35.65.209|1402" //Test PLN ragunan


//#define __DFTSRV__ "10.35.65.173|5050" //PERMINTAAN TEST UAT

//#define __DFTSRV__ "172.021.064.057|8888" //NAC RAGUNAN
//#define __DFTSRV__ "172.21.64.57|8888" //NAC SUDIRMAN
//#define __DFTSRV__ "172.21.64.53|5050" //TLE SUDIRMAN
#else
//#define __DFTSRV__ "192.168.1.7|9999"
//#define __DFTSRV__ "6107"// Dial Up
//#define __DFTSRV__ "192.168.1.11|9999"
#define __DFTSRV__ "192.168.2.64|9999"
//#define __DFTSRV__ "192.168.2.35|6060"
//#define __DFTSRV__ "192.168.2.59|9999"
#endif
//#define __DFTSRV__ "192.168.1.2|9999"
//#define __DFTSRV__ "192.168.1.1|9999"
//#define __DFTSRV__ "192.168.2.98|1402"

#ifdef USE_DIAL_UP

//#define __DFTSRV__ "6107"// Dial Up
#define __DFTSRV__ "108"// Dial Up

#endif

#define __DFTTLE__ "0000000000000000"
#define __DFTUMK__ "00000000000000000000000000000000"
#define __DFTPPPCONN__ "080989999|telkomnet@instan|telkom|60"
#define __DFTPPPDEST__ "117.102.089.091|9999"

#define __DFTMODULUS__ "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"

#define __DFTMINPREPAIDINIT__ "20000"
#define __DFTMAXPREPAIDINIT__ "20000000"

//#define __DFT_TID__ "11000018"
//#define __DFT_MID__ "000001210000018"
//#define __DFT_TID__ "11000020" //ICT
//#define __DFT_MID__ "000001202060001"  //ICT
//#define __DFT_TID__ "11000021" //EFT
//#define __DFT_MID__ "000001202060018" //EFT
//#define __DFT_TID__ "00000068" //EFT

//#define __DFT_TID__ "00000007" //EFT
//#define __DFT_MID__ "000001210000020" //EFT

//#define __DFT_TID__ "00000062" //EFT
//#define __DFT_MID__ "000001203300018" //EFT


#define __DFT_TID__ "80210031" //EFT
#define __DFT_MID__ "800211401000021" //EFT


//#define __DFT_TID__ "17890038" //EFT
//#define __DFT_MID__ "001019000148" //EFT

//#define __DFT_TID__ "17890037" //ICT CREDIT
//#define __DFT_MID__ "000001019100009" //ICT CREDIT




//#define __DFT_TID__ "00000061" //EFT
//#define __DFT_MID__ "000001203300018" //EFT

//#define __DFT_TID__ "11000121" //EFT
//#define __DFT_MID__ "000001104780000" //EFT

//#define __DFT_TID__ "90000002" //EFT
//#define __DFT_MID__ "000001299990002" //EFT

//#define __DFT_TID_CREDIT__ "20000002"
//#define __DFT_MID_CREDIT__ "000001019000147"

//#define __DFT_TID_CREDIT__ "02500114"
#define __DFT_TID_CREDIT__ "80210031"
#define __DFT_MID_CREDIT__ "800211401000021"

//#define __DFT_TID_CREDIT__ "17890037"  //NII uat 51
//#define __DFT_MID_CREDIT__ "000001019100009"

#define __DFT_BRIZZI_TID __DFT_TID__ //EFT
#define __DFT_BRIZZI_MID __DFT_MID__ //EFT



enum binLogoType {
    binLogoBeg,
    binLogoVisa,
    binLogoMC,
    binLogoJCB,
    binLogoAmex,
    binLogoCirrus,
    binLogoMaestro,
    binLogoEnd
};

#include "pri.h"
#include "key.h"

int traReset(void);             ///<reset volatile memory: transaction related data = tra section
int strFormatDatTim(word key);  ///<get date & time from logDatTim and format it for printing into traFmtDate

int emvIsApproved(void);        ///<check if emv transaction approved
int usrInfo(word key);          ///<retrieve user screen from database and display it
word mapKeyTag(card tag);
int scrDlg(word infKey, word prmKey);
int isReversibleSend(void);     //is current transaction reversible one?
int isReversibleRecv(void);     //is current transaction reversible one?
int incCard(word key);          ///< increments the inputted key of card data type with max value of 999999

#define MAPGET(KEY,BUF,LBL) { ret= mapGet(KEY,BUF,sizeof(BUF)); CHECK(ret>=0,LBL);}
#define MAPGETBYTE(KEY,VAR,LBL) { ret= mapGetByte(KEY,VAR); CHECK(ret>=0,LBL);}
#define MAPGETWORD(KEY,VAR,LBL) { ret= mapGetWord(KEY,VAR); CHECK(ret>=0,LBL);}
#define MAPGETCARD(KEY,VAR,LBL) { ret= mapGetCard(KEY,VAR); CHECK(ret>=0,LBL);}

#define MAPPUTSTR(KEY,VAR,LBL) { ret= mapPutStr(KEY,VAR); CHECK(ret>=0,LBL);}
#define MAPPUTBYTE(KEY,VAR,LBL) { ret= mapPutByte(KEY,VAR); CHECK(ret>=0,LBL);}
#define MAPPUTWORD(KEY,VAR,LBL) { ret= mapPutWord(KEY,VAR); CHECK(ret>=0,LBL);}
#define MAPPUTCARD(KEY,VAR,LBL) { ret= mapPutCard(KEY,VAR); CHECK(ret>=0,LBL);}

#define __BOLD__        "\x1B\x1A"
#define __COMPRESSED__  "\x1B\x20"
#define __DBLSIZE__  	"\x1b\x1e"
#define __DBLHEIGHT__	"\x1b\x1d"
#define __DBLWIDTH__	"\x1b\x1c"
#define __RIGHT__		"\x1b\x18"
#define __CENTER__	    "\x1b\x17"

#define __COMPRESSED_RIGHT__	"\x1B\x20\x1b\x18"

#define __BOLD_COMPR__    "\x1B\x1A\x1B\x20"
#define __BOLD_CENTER__	  "\x1B\x1A\x1b\x17"
#define __COMPR_CENTER__  "\x1B\x20\x1b\x17"
#define __DBLHEIGHT_BOLD__	"\x1b\x1d\x1B\x1A"
#define __SPACES__	  "\x1b\x13"

/** @} */
#define MEM_ZERO(VAR) memset(VAR, 0, sizeof(VAR));
#define MEM_0x20(VAR) memset(VAR, 0x20, sizeof(VAR));
#define MEM_0x30(VAR) memset(VAR, 0x30, sizeof(VAR));

/* @} */
#define RUN_FUNCTION(F, LBL) {ret = F(); CHECK(ret > 0, LBL);}
#define dsp_ALIGN_LEFT 		0
#define dsp_ALIGN_CENTER 	1
#define dsp_ALIGN_RIGHT 	2

#define kbdF1_kbdF4 		1
#define kbdVAL_kbdANN 		0


#define __STR_H
#endif
