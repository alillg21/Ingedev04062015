/** \file
 * APACS40 oriented functions
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/pri/a40.c $
 *
 * $Id: a40.c 2217 2009-03-20 14:52:31Z abarantsev $
 */

#include <string.h>
#include "pri.h"

#undef trcOpt
//static const card trcOpt = TRCOPT & BIT(trcA40);
#define trcOpt (TRCOPT & BIT(trcA40))

/** calculate fields A,B,C,D according to APACS40 specification
 * \todo To be documented
 * \test tcab0069.c 
*/
int a40ABCD(char *abcd, const char *src) {
    enum { cSS = ';', cDS = '=', cES = '?' };   //Start Sentinel, Data field Separator, End Sentinel
    int n, nSS, nDS, nES;       //SS,DS,ES offsets within src, -1 if missing
    int nA, nB, nC, nD, lA, lB, lC, lD;
    char *p;

    VERIFY(abcd);
    VERIFY(src);
    nSS = -1;
    nDS = -1;
    nES = -1;
    n = 0;
    while(src[n]) {
        switch (src[n]) {
          case cSS:
              nSS = n;
              break;
          case cDS:
              nDS = n;
              break;
          case cES:
              nES = n;
              break;
          default:
              break;
        }
        n++;
    }
    nA = nB = nC = nD = -1;
    lA = lB = lC = lD = -1;
    if(nSS >= 0) {              //Card Based Transactions
        if(nES < nSS)
            return -1;
        if(nDS > 0) {           //Card conforming ISO7810/7811/7812/7813
            if(nDS > 8)
                lB = 8;
            else
                lB = nDS;
            nB = nDS - lB;
            if(nB > 9)
                lA = 8;
            else
                lA = nB - 1;
            nA = nB - lA;
            nC = nDS + 5;
            if(nC + 7 <= nES)
                lC = 8;
            else
                lC = nES - nC + 1;
            nD = nC + lC;
            if(nD + 7 <= nES)
                lD = 8;
            else
                lD = nES - nD + 1;
        } else {                //non ISO cards
            if(nES - nSS >= 16) {   //long card data
                nA = nSS + 1;
                lA = 8;
                nB = nA + lA;
                lB = 8;
                nD = nES - 8;
                lD = 8;
                nC = nD - lD;
                lC = 8;
            } else {            //short card data
                VERIFY(nSS <= nES);
                if(nES < 8)
                    lB = nES;
                else
                    lB = 8;
                nB = nES - lB;
                if(nB > 9)
                    lA = 8;
                else
                    lA = nB - 1;
                nA = nB - lA;
                nC = nSS + 1;
                if(nC + 7 <= nES)
                    lC = 8;
                else
                    lC = nES - nC + 1;
                nD = nC + lC;
                if(nD + 7 <= nES)
                    lD = 8;
                else
                    lD = nES - nD + 1;
            }
        }
    } else {                    //Administrative transactions and manually entered details
        VERIFY(nSS == -1);
        VERIFY(nDS == -1);
        VERIFY(nES == -1);
        nES = strlen(src);
        if(nES >= 8)
            lB = 8;
        else
            lB = nES;
        nB = nES - lB;
        if(nB >= 8)
            lA = 8;
        else
            lA = nB;
        nA = nB - lA;
        nC = nD = lC = lD = 0;
    }
    VERIFY(nB >= 0);
    VERIFY(nA >= 0);
    VERIFY(nC >= 0);
    VERIFY(nD >= 0);
    VERIFY(lA >= 0);
    VERIFY(lB >= 0);
    VERIFY(lC >= 0);
    VERIFY(lD >= 0);
    VERIFY(lA <= 8);
    VERIFY(lB <= 8);
    VERIFY(lC <= 8);
    VERIFY(lD <= 8);
    VERIFY(nA <= nB);
    VERIFY(nC <= nD);
    p = abcd;
    memset(p, 0, 8 + 8 + 8 + 8 + 1);
    memcpy(p, src + nA, lA);
    fmtPad(p, -8, '0');
    p += 8;
    memcpy(p, src + nB, lB);
    fmtPad(p, -8, '0');
    p += 8;
    memcpy(p, src + nC, lC);
    fmtPad(p, 8, '0');
    p += 8;
    memcpy(p, src + nD, lD);
    fmtPad(p, 8, '0');
    p += 8;

    n = 0;
    while(abcd[n]) {
        if(abcd[n] < 0x3A) {
            n++;
            continue;
        }
        if(0x3F < abcd[n]) {
            n++;
            continue;
        }
        abcd[n] -= 0x3A;
        abcd[n] += 'A';
    }
    return 8 + 8 + 8 + 8;
}

#ifdef __OWF__
static int a40GenMacKey(const byte * A, const byte * B, byte locReg,
                        byte locMac) {
    byte dataLeft[8];
    byte dataRight[8];
    int ret;

    memcpy(dataLeft, B, 4);
    memcpy(dataLeft + 4, A, 4);
    memcpy(dataRight, B, 4);
    memcpy(dataRight + 4, A, 4);

    // From KeyReg on position (x), and KeyReg on position (x+1) => Double KeyReg
    // Get  MacKey on position (y), and MacKey on position (y+1) => Double MacKey with property MAC_KEY
    ret = cryOwfInt(dataLeft, dataRight, locReg, locMac, 'm');
    CHECK(ret >= 0, lblKO);
    return 0;
  lblKO:
    return -1;
}

// Done by bkassovic 
static int a40GenKeyVal(const byte * A, const byte * B, byte locReg,
                        byte locVal) {
    byte dataLeft[8];
    byte dataRight[8];
    int ret;

    memcpy(dataLeft, A, 4);
    memcpy(dataLeft + 4, B, 4);
    memcpy(dataRight, A, 4);
    memcpy(dataRight + 4, B, 4);

    // From KeyReg on position (x), and KeyReg on position (x+1) => Double KeyReg
    // Get  KeyVal on position (z), and KeyVal on position (z+1) => Double KeyVal with property GEN_KEY
    ret = cryOwfInt(dataLeft, dataRight, locReg, locVal, 'g');
    CHECK(ret >= 0, lblKO);
    return 0;
  lblKO:
    return -1;
}

static void a40GenCardKey(const byte * A, const byte * B, const byte * C,
                          const byte * D, byte * key) {
    byte dat[8];
    byte reg[8];

    memcpy(dat, B, 4);
    memcpy(dat + 4, D, 4);
    memcpy(reg, A, 4);
    memcpy(reg + 4, C, 4);
    stdOWF(key, dat, reg);
}

// Done by bkassovic
static int a40GenPinKey(const byte * crdKey, byte locPinKey) {
    byte dataLeft[8];
    byte dataRight[8];
    int ret;

    memcpy(dataLeft, crdKey, 8);    // Left  part : CardKey (Generate by function genCardKey)
    memcpy(dataRight, crdKey, 8);   // Right part : CardKey (Generate by function genCardKey)   

    // From KeyVal on position (z), and KeyVal on position (z+1) => Double KeyVal
    // Get  PinKey on position (z), and PinKey on position (z+1) => Double PinKey with property TRIPLE_DES_PIN_KEY
    ret = cryOwfInt(dataLeft, dataRight, locPinKey, locPinKey, 'p');
    CHECK(ret >= 0, lblKO);
    return 0;
  lblKO:
    return -1;
}

/** APACS40 key generation
 * \todo To be documented
 * \test TCTT0016.C
*/
int a40GenA40Keys(const byte * ABCD, byte locPK, byte locMK, byte locRK) {
    byte CardKey[8];
    int ret;

    const byte *A = ABCD + 0;
    const byte *B = ABCD + 4;
    const byte *C = ABCD + 8;
    const byte *D = ABCD + 12;

    // Generation of MAC KEY (saved in SSA)
    // From locRK => RegisterKey position (x) and position (x+1) (double Register Key)
    // get  locMK => MacKey position (y) and position (y+1) (double Mac key)
    ret = a40GenMacKey(A, B, locRK, locMK);
    CHECK(ret >= 0, lblKO);

    // Generation of intermediate KEY VAL (saved in SSA temporaly)
    // From locRK => RegisterKey position (x) and position (x+1) (double Register Key)
    // get  locPK => KeyVal position (z) and position (z+1) (double Key Val) 
    ret = a40GenKeyVal(A, B, locRK, locPK);
    CHECK(ret >= 0, lblKO);

    // Calculation of CARD KEY
    a40GenCardKey(A, B, C, D, CardKey);

    // Generation of PIN KEY (saved in SSA overwrites KeyVal)
    // From locPK => KeyVal position (z), KeyVal position (z+1) (double Key Val)  
    // get locPK =>  PinKey position (z), PinKey position (z+1) (double Pin Key) PinKey overwrites KeyVal
    ret = a40GenPinKey(CardKey, locPK);
    CHECK(ret >= 0, lblKO);

    return 0;
  lblKO:
    return -1;
}

static int a40GenReceivedMac(tBuffer * buf, const byte * TextStr,
                             byte loc, byte * ResponseResidue,
                             byte * ResponseMac) {
    int ret;
    byte RunningMac[8];
    char tmp[8 + 1];
    word DataSize;
    byte Index;

    memset(RunningMac, '\0', 8);
    DataSize = strlen((char *) TextStr) - 8;    //length of the received message excluding the MAC      
    ret = cryMac(buf, loc, (byte *) tmp);
    if(ret > 0)
        ret = 5;                //DE_FAIL

    memcpy(RunningMac, tmp, 8);
    bin2hex(tmp, RunningMac, 4);
    memcpy(ResponseMac, tmp, 8);

    // Check against the last eight bytes of the received message.
    ret = 4;                    //DE_OK
    for (Index = 0; Index < 8; Index++) {
        if(tmp[Index] != TextStr[DataSize + Index])
            ret = 5;            //DE_FAIL
    }

    // if the MAC is OK update the response residue and return success.
    if(ret == 4)                //DE_OK
        memcpy(ResponseResidue, &RunningMac[4], 4); //store the response residue  

    return ret;
}

/** 
 * \todo To be documented
 * \test tctt0020.c
*/
int a40GenerateAuthParameter(const byte TransactionType,
                             const byte AuthParameterType,
                             const byte * TransactionData,
                             byte locKey, byte * AuthParameter) {

    int ret;
    byte Index_GenAuth;
    byte Offset;
    byte TransDataBlock[8];
    byte Length;
    byte marker;

    Length = 0;
    if(TransactionData != NULL)
        Length = (byte) strlen((char *) TransactionData);
    if(AuthParameterType == 1)  //DE_NO_KEY
        memset(AuthParameter, 0, 8);
    else {                      //Initialise the transaction data block with 0xff
        memset(TransDataBlock, 0xFF, 8);
        //If a credit or debit transaction then verify transaction data length.
        marker = TransactionType - '0';
        switch (TransactionType) {
          case '9':            //credit transaction
              //Place 9 in the most significant nibble and the length of the transaction 
              //amount in the least significant nibble of the most significant byte of the
              //transaction data.
              TransDataBlock[0] = (byte) ((marker << 4) | Length);
              break;
          case '8':            //debit transaction
              //Place 8 in the most significant nibble and the length of the transaction 
              //amount in the least significant nibble of the most significant byte of the
              //transaction data.
              TransDataBlock[0] = (byte) ((marker << 4) | Length);
              break;
          case '7':            //administrative transaction
              //Place 7 in the upper nibble and zero in the  
              //lower nibble of the most significant byte of the transaction data.
              TransDataBlock[0] = marker << 4;
              break;
        }
        if(TransactionType != '7') {    //non-administrative transaction
            Offset = 1;
            //For credit or debit transaction load the amount
            for (Index_GenAuth = 0; Index_GenAuth < Length; Index_GenAuth++) {
                if(Index_GenAuth & 1) { // Pack the lower digit
                    TransDataBlock[Offset] = ((TransDataBlock[Offset] & 0xF0) |
                                              (TransactionData[Index_GenAuth] &
                                               0x0F));
                    Offset++;
                } else {        //Index_GenAuth is an even number. Pack the upper digit
                    TransDataBlock[Offset] = (TransDataBlock[Offset] & 0x0F) |
                        ((TransactionData[Index_GenAuth] << 4) & 0xF0);

                }
            }
        }
        //The resulting transaction data block is the data register of the OWF. 
        //The key is written into the key register of the OWF. 
        //The result of the OWF is the authorisation parametr. 
        memcpy(AuthParameter, TransDataBlock, 8);

        ret =
            cryOwfExt(TransDataBlock, TransDataBlock, locKey, AuthParameter,
                      AuthParameter);
        CHECK(ret >= 0, lblKO);
    }                           // End of key required
    return 0;

  lblKO:
    return -1;
}

static int a40ChkMac(const byte TransactionType,
                     const byte AuthParameterType, const byte * TextStr,
                     byte * AuthParameter,
                     byte * TxnAmount,
                     const byte * RequestResidue,
                     byte locMacKey,
                     byte locPinKey,
                     tBuffer * Resbuf,
                     byte * ResponseResidue, byte * ResponseMac) {

    int ret;
    byte GenAuthParameter[8 + 1];

//    byte ReturnValue;
    byte TxnAmountCopy[11 + 1]; //MAX_TXN_AMOUNT_SIZE+1
    tBuffer Totbuf;
    byte dOutMsg[256] = "";

    // make sure the amount is at least two digits 
    while(strlen((char *) TxnAmount) < 2) { //MIN_AMOUNT_DIGITS
        // add a '0' to the start of the string 
        strcpy((char *) TxnAmountCopy, (char *) TxnAmount);
        TxnAmount[0] = '0';
        strcpy((char *) &TxnAmount[1], (char *) TxnAmountCopy);
    };

    // Calculate the authorisation parameter with MAC key first. 
    if(TransactionType != '7') {    //administrative transaction
        if(strlen((char *) AuthParameter) == 0) {
            ret = a40GenerateAuthParameter(TransactionType,
                                           AuthParameterType,
                                           TxnAmount, locMacKey, AuthParameter);
            CHECK(ret >= 0, lblKO);
            memcpy(GenAuthParameter, AuthParameter, 8);
        } else

            ret =
                cryOwfExt(AuthParameter, AuthParameter, locMacKey,
                          GenAuthParameter, GenAuthParameter);
        CHECK(ret >= 0, lblKO);
    } else {
        if(strlen((char *) AuthParameter) == 0) {
            ret = a40GenerateAuthParameter(TransactionType,
                                           AuthParameterType,
                                           NULL, locMacKey, AuthParameter);
            CHECK(ret >= 0, lblKO);
            memcpy(GenAuthParameter, AuthParameter, 8);
        } else

            ret =
                cryOwfExt(AuthParameter, AuthParameter, locMacKey,
                          GenAuthParameter, GenAuthParameter);
        CHECK(ret >= 0, lblKO);
    }
    bufInit(&Totbuf, dOutMsg, 256);
    bufApp(&Totbuf, RequestResidue, 4);
    bufApp(&Totbuf, TextStr, strlen((char *) TextStr) - 8);

    while(bufLen(&Totbuf) % 8) {
        bufApp(&Totbuf, (byte *) "\x00", 1);
    }
    bufApp(&Totbuf, GenAuthParameter, 8);

    //ReturnValue =

    ret = a40GenReceivedMac(&Totbuf, TextStr, locMacKey, ResponseResidue,
                            ResponseMac);

    // If mac wrong try again using the pin processing key
    if(ret == 5) {              //DE_FAIL
        if(TransactionType != '7') {    //administrative transaction
            if(strlen((char *) AuthParameter) == 0) {
                ret = a40GenerateAuthParameter(TransactionType,
                                               AuthParameterType,
                                               TxnAmount, locPinKey,
                                               AuthParameter);
                CHECK(ret >= 0, lblKO);
                memcpy(GenAuthParameter, AuthParameter, 8);
            } else

                ret =
                    cryOwfExt(AuthParameter, AuthParameter, locPinKey,
                              GenAuthParameter, GenAuthParameter);
            CHECK(ret >= 0, lblKO);
        } else {
            if(strlen((char *) AuthParameter) == 0) {
                a40GenerateAuthParameter(TransactionType,
                                         AuthParameterType,
                                         NULL, locPinKey, AuthParameter);
                CHECK(ret >= 0, lblKO);
                memcpy(GenAuthParameter, AuthParameter, 8);
            } else

                ret =
                    cryOwfExt(AuthParameter, AuthParameter, locPinKey,
                              GenAuthParameter, GenAuthParameter);
            CHECK(ret >= 0, lblKO);
        }

        ret = a40GenReceivedMac(&Totbuf, TextStr, locPinKey, ResponseResidue,
                                ResponseMac);
    }
    return (ret);

  lblKO:
    return -1;
}

/** 
 * \todo To be documented
 * \test tctt0019.c
*/
int a40CheckMac(byte TransactionType, byte AuthParameterType,
                byte * ConstructedRspPacket, byte * AuthParam, byte * TxnAmount,
                byte * ReqMacResidue, byte * ResMacResidue, byte * ResMac,
                byte MacKeyLoc, byte PinKeyLoc) {
    int ret;
    byte returnVal;
    byte dOutMsg[256];
    tBuffer Resbuf;

    trcS("cryCheckMac: Beg\n");
    trcFN("MacKeyLoc=%02X ", MacKeyLoc);
    trcFN("PinKeyLoc=%02X ", PinKeyLoc);

    bufInit(&Resbuf, dOutMsg, 256);
    bufApp(&Resbuf, ReqMacResidue, strlen((char *) ReqMacResidue));
    bufApp(&Resbuf, ConstructedRspPacket,
           strlen((char *) ConstructedRspPacket) - 8);
    while(bufLen(&Resbuf) % 8) {
        ret = bufApp(&Resbuf, (byte *) "\x00", 1);
    }

    returnVal =
        a40ChkMac(TransactionType, AuthParameterType, ConstructedRspPacket,
                  AuthParam, TxnAmount, ReqMacResidue, MacKeyLoc, PinKeyLoc,
                  &Resbuf, ResMacResidue, ResMac);

    if(returnVal == 5)
        ret = 5;                //Mac Does not Match
    else
        ret = 1;                //Mac Matches

    trcFN("cryCheckMac: ret=%d\n", ret);
    return ret;

}

/** 
 * \todo To be documented
 * \test tctt0019.c
*/
int a40UpdateKeyRegister(const byte * RequestMacResidue,
                         const byte * ResponseMacResidue, byte locKeyReg) {
    byte dataLeft[8];
    byte dataRight[8];

    byte resLeft[8];            // For Debug
    byte resRight[8];           // For Debug

    int ret;

    memcpy(dataLeft, RequestMacResidue, 4);
    memcpy(&dataLeft[4], ResponseMacResidue, 4);
    memcpy(dataRight, RequestMacResidue, 4);
    memcpy(&dataRight[4], ResponseMacResidue, 4);

    // From    KeyReg on position (x), and KeyReg on position (x+1) => Double KeyReg
    // Get new KeyReg on position (x), and new KeyReg on position (x+1) => Double KeyReg with property GEN_KEY
    ret = cryOwfExt(dataLeft, dataRight, locKeyReg, resLeft, resRight);
    CHECK(ret >= 0, lblKO);
    ret = cryOwfInt(dataLeft, dataRight, locKeyReg, locKeyReg, 'g');
    CHECK(ret >= 0, lblKO);
    return 0;
  lblKO:
    return -1;
}
#endif
