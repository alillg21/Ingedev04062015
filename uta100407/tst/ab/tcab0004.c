/** \file
 * Unitary test case tcab0004.
 * \sa
 * - num2dec()
 * - dec2num()
 * - num2hex()
 * - hex2num()
 * - bin2hex()
 * - hex2bin()
 * - bin2num()
 * - num2bin()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0004.c $
 *
 * $Id: tcab0004.c 2534 2010-01-06 10:35:08Z abarantsev $
 */

#include <string.h>
#include "sys.h"
#include "tst.h"

static int tstNum2Dec(void) {
    int ret;
    char msg[prtW + 1];
    char buf[16];

    /*! \section TestNum2Dec Steps 1.1 to 1.4
     * \par Call num2dec(char * dec, card num, byte len)
     * this function convert a numeric value num into a decimal string dec of length len.
     * The zero value of len means that the length is to be calculated as the number of decimal digits in num.
     * If the argument len is too small, zero is returned; otherwise the number of characters converted is returned.
     * - dec  (O) Destination zero-ended string to be filled by (len+1) characters.
     * - num  (I) Numeric value to be converted.
     * - len  (I) Number of characters in the destination.
     * \n
     * \par Step 1.1
     * Conversion from numeric to decimal format in normal conditions
     * with length bigger or equal to number of caracters in value to be converted
     */
    //Normal conditions
    strcpy(msg, "num2dec 1234 5: ");
    ret = num2dec(buf, 1234, 5);
    CHECK(ret == 5, lblErr);
    CHECK(strcmp(buf, "01234") == 0, lblErr);
    strcat(msg, buf);
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 1.2:
     *  Conversion from numeric to decimal format in zero length argument conditions
     * in this case the length is to be calculated as the number of decimal digits in num
     */
    //Check zero length argument
    strcpy(msg, "num2dec 321 0: ");
    ret = num2dec(buf, 321, 0);
    CHECK(ret == 3, lblErr);
    CHECK(strcmp(buf, "321") == 0, lblErr);
    strcat(msg, buf);
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 1.3:
     *  Conversion from numeric to decimal format in boundary conditions
     * with zero length and zero value to be converted
     */
    //Boundary condition: zero
    strcpy(msg, "num2dec 0 0: ");
    ret = num2dec(buf, 0, 0);
    CHECK(ret == 1, lblErr);
    CHECK(strcmp(buf, "0") == 0, lblErr);
    strcat(msg, buf);
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 1.4:
     *  Conversion from numeric to decimal format in special conditions
     * when the length argument is too small
     * in this case zero is returned
     */
    //Exception: the length argument is too small
    strcpy(msg, "num2dec 234 2: ");
    ret = num2dec(buf, 234, 2);
    CHECK(ret == 0, lblErr);
    strcat(msg, "OK");
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblErr:
    strcat(msg, "KO");
    prtS(msg);
  lblKO:
    trcErr(ret);
    ret = 0;
  lblEnd:
    return ret;
}

static int tstDec2Num(void) {
    int ret;
    char msg[prtW + 1];
    card num;

    /*! \section TestDec2Num Steps 2.1 to 2.6
     * \par Call dec2num(card * num, const char * dec, byte len)
     * this function convert a numeric value num into a decimal string dec of length len.
     * The zero value of len means that the length is to be calculated as the number of decimal digits in num.
     * If the argument len is too small, zero is returned; otherwise the number of characters converted is returned.
     * - dec  (O) Destination zero-ended string to be filled by (len+1) characters.
     * - num  (I) Numeric value to be converted.
     * - len  (I) Number of characters in the destination.
     * \n
     * \par Step 2.1
     * Conversion from decimal to numeric format in normal conditions
     *
     */
    //Normal conditions
    strcpy(msg, "dec2num 1234 0: ");
    ret = dec2num(&num, "1234", 0);
    CHECK(ret == 4, lblErr);
    CHECK(num == 1234, lblErr);
    strcat(msg, "1234");
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 2.2:
     *  Conversion from decimal to numeric format with spaces at original string
     * in this case spaces are ignored.
     */
    //Blanks processing
    strcpy(msg, "dec2num ..1 0: ");
    ret = dec2num(&num, "  1", 0);
    CHECK(ret == 3, lblErr);
    CHECK(num == 1, lblErr);
    strcat(msg, "1");
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 2.3:
     *  Conversion from decimal to numeric format in non-zero length conditions
     * string will be cut to defined length before conversion
     */
    //Non-zero length processing
    strcpy(msg, "dec2num 12345 3: ");
    ret = dec2num(&num, "12345", 3);
    CHECK(ret == 3, lblErr);
    CHECK(num == 123, lblErr);
    strcat(msg, "123");
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 2.4:
     * Conversion from decimal to numeric format with invalid character in original string
     * Converion will return 0, as zero characters converted.
     */
    //Invalid character
    strcpy(msg, "dec2num 1A2 0: ");
    ret = dec2num(&num, "1A2", 0);
    CHECK(ret == 1, lblErr);
    CHECK(num == 1, lblErr);
    strcat(msg, "OK");
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 2.5:
     * Conversion from decimal to numeric format with empty string
     * Converion will return 0, as zero characters converted.
     */
    //Empty string
    strcpy(msg, "dec2num . 0: ");
    ret = dec2num(&num, "", 0);
    CHECK(ret == 0, lblErr);
    strcat(msg, ".");
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 2.6:
     * Conversion from decimal to numeric format with string containing spaces only
     * Converion will return 0, as zero characters converted.
     */
    //Empty string
    strcpy(msg, "dec2num . 0: ");
    ret = dec2num(&num, " ", 0);
    CHECK(ret == 1, lblErr);
    strcat(msg, ".");
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 2.7:
     * Conversion from decimal to numeric format with very long original string
     * Converion will return 0, as zero characters converted.
     */
    //Very long string
    strcpy(msg, "dec2num 123..789 0: ");
    ret = dec2num(&num, "1234567890123456789", 0);
    CHECK(ret == 0, lblErr);
    strcat(msg, "0");
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 2.8:
     * Conversion from decimal to numeric format with the minimum boundary range value
     */
    //Acceptable range: <0;4294967295>=<0,0xFFFFFFFF>
    strcpy(msg, "dec2num 0 0: ");
    ret = dec2num(&num, "0", 0);
    CHECK(ret == 1, lblErr);
    CHECK(num == 0, lblErr);
    strcat(msg, "0");
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 2.9:
     * Conversion from decimal to numeric format with the maximum boundary range value
     */
    //Acceptable range: <0;4294967295>=<0,0xFFFFFFFF>
    strcpy(msg, "dec2num 4294967295 0: ");
    ret = dec2num(&num, "4294967295", 0);
    CHECK(ret == 10, lblErr);
    CHECK(num == 4294967295LL, lblErr);
    ret = prtS(msg);
    strcpy(msg, "4294967295");
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 2.10:
     * Conversion from decimal to a value over the maximum boundary range value
     */
    //Acceptable range: <0;4294967295>=<0,0xFFFFFFFF>
    strcpy(msg, "dec2num 4294967296 0: ");
    ret = dec2num(&num, "4294967296", 0);
    CHECK(ret == 0, lblErr);
    strcat(msg, "0");
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblErr:
    strcat(msg, "KO");
    prtS(msg);
  lblKO:
    trcErr(ret);
    ret = 0;
  lblEnd:
    return ret;
}

static int tstNum2Hex(void) {
    int ret;
    char msg[prtW + 1];
    char buf[16];

    /*! \section TestNum2Hex Steps 3.1 to 3.4
     * \par Call num2hex(char * hex, card num, byte len)
     * this function convert a numeric value num into a hexadecimal string hex of length len.
     * The zero value of len means that the length is to be calculated as the number of hexadecimal digits in num.
     * If the argument len is too small, zero is returned; otherwise the number of characters converted is returned.
     * - hex  (O) Destination zero-ended string to be filled by (len+1) characters.
     * - num  (I) Numeric value to be converted.
     * - len  (I) Number of characters in the destination.
     * \n
     * \par Step 3.1
     * Conversion from numeric to hexadecimal format in normal conditions
     * fonction return number of characters converted.
     */
    //Normal conditions
    strcpy(msg, "num2hex 12AB 5: ");
    ret = num2hex(buf, 0x12AB, 5);
    CHECK(ret == 5, lblErr);
    CHECK(strcmp(buf, "012AB") == 0, lblErr);
    strcat(msg, buf);
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 3.2:
     *  Conversion from numeric to hexadecimal format with zero in length argument conditions
     * in this case the length is to be calculated as the number of hexadecimal digits in num.
     */
    //Check zero length argument
    strcpy(msg, "num2hex 321 0: ");
    ret = num2hex(buf, 0x321, 0);
    CHECK(ret == 3, lblErr);
    CHECK(strcmp(buf, "321") == 0, lblErr);
    strcat(msg, buf);
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 3.3:
     * Conversion from numeric to hexadecimal format with boundary condition: zero
     * in this case result of conversion will be zero.
     */
    //Boundary condition: zero
    strcpy(msg, "num2hex 0 0: ");
    ret = num2hex(buf, 0, 0);
    CHECK(ret == 1, lblErr);
    CHECK(strcmp(buf, "0") == 0, lblErr);
    strcat(msg, buf);
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 3.4:
     *  Conversion from numeric to hexadecimal format with the length argument is too small
     * Converion will return 0, as zero characters converted.
     */
    //Exception: the length argument is too small
    strcpy(msg, "num2hex abc 2: ");
    ret = num2hex(buf, 0x234, 2);
    CHECK(ret == 0, lblErr);
    strcat(msg, "OK");
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblErr:
    strcat(msg, "KO");
    prtS(msg);
  lblKO:
    trcErr(ret);
    ret = 0;
  lblEnd:
    return ret;
}

static int tstHex2Num(void) {
    int ret;
    char msg[prtW + 1];
    card num;

    /*! \section TestHex2Num Steps 4.1 to 4.6
     * \par Call hex2num(card * num, const char * hex, byte len)
     * this function convert a hexadecimal string hex of length len into a numeric value num.
     * The zero value of len means that the length is to be calculated as the strlen(hex).
     * The spaces and tabs at the beginning of hex are ignored.
     * If operation is performed successfully, the number of characters converted is returned.
     * In case of invalid input (empty string, too long string, invalid character) zero value is returned.
     * - num  (O) Pointer to the result numeric value.
     * - hex  (I) Source string containing hexadecimal digits only.
     * - len  (I) Number of characters in the destination.
     * \n
     * \par Step 4.1
     * Conversion from hexadecimal to numeric format in normal conditions
     * fonction return number of characters in the destination.
     */
    //Normal conditions
    strcpy(msg, "hex2num 12AB 0: ");
    ret = hex2num(&num, "12AB", 0);
    CHECK(ret == 4, lblErr);
    CHECK(num == 0x12AB, lblErr);
    strcat(msg, "12AB");
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 4.2:
     *  Conversion from hexadecimal to numeric format in blanks processing conditions
     * in this case all spaces are ignored.
     */
    //Blanks processing
    strcpy(msg, "hex2num ..1 0: ");
    ret = hex2num(&num, "  1", 0);
    CHECK(ret == 3, lblErr);
    CHECK(num == 1, lblErr);
    strcat(msg, "1");
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 4.3:
     *  Conversion from hexadecimal to numeric format in non-zero length processing conditions
     * in this case string will be cut to defined length before conversion.
     */
    //Non-zero length processing
    strcpy(msg, "hex2num 12EF5 3: ");
    ret = hex2num(&num, "12EF5", 3);
    CHECK(ret == 3, lblErr);
    CHECK(num == 0x12E, lblErr);
    strcat(msg, "12E");
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 4.4:
     *  Conversion from hexadecimal to numeric format with invalid character in original string
     * Converion will return 0, as zero characters converted.
     */
    //Invalid character
    strcpy(msg, "hex2num 1G2 0: ");
    ret = hex2num(&num, "1G2", 0);
    CHECK(ret == 0, lblErr);
    strcat(msg, "OK");
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 4.5:
     *  Conversion from hexadecimal to numeric format with empty string
     * Converion will return 0, as zero characters converted.
     */
    //Empty string
    strcpy(msg, "hex2num . 0: ");
    ret = hex2num(&num, "", 0);
    CHECK(ret == 0, lblErr);
    strcat(msg, ".");
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 4.6:
     *  Conversion from hexadecimal to numeric format with very long string
     * Converion will return 0, as zero characters converted.
     */
    //Very long string
    strcpy(msg, "hex2num 12..EF 0: ");
    ret = hex2num(&num, "123456789ABCDEF0123", 0);
    CHECK(ret == 0, lblErr);
    strcat(msg, "0");
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblErr:
    strcat(msg, "KO");
    prtS(msg);
  lblKO:
    trcErr(ret);
    ret = 0;
  lblEnd:
    return ret;
}

static int tstBin2Hex(void) {
    int ret;
    char msg[prtW + 1];
    char buf[16];

    /*! \section TestBin2Hex Step 5.1
     * \par Call bin2hex(char * hex, const byte * bin, int len)
     * this function convert a binary buffer bin of length len into a hexadecimal string hex.
     * The return value is always len*2.
     * - hex  (O) Destination zero-ended string to be filled by (2*len+1) characters.
     * - bin  (I) Pointer to the binary buffer to be converted.
     * - len  (O) Number of bytes to convert.
     */
    //Normal conditions
    strcpy(msg, "bin2hex 1234 2: ");
    ret = bin2hex(buf, (byte *) "\x12\x34", 2);
    CHECK(ret == 4, lblErr);
    CHECK(strcmp(buf, "1234") == 0, lblErr);
    strcat(msg, buf);
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblErr:
    strcat(msg, "KO");
    prtS(msg);
  lblKO:
    trcErr(ret);
    ret = 0;
  lblEnd:
    return ret;
}

static int tstHex2Bin(void) {
    int ret;
    char msg[prtW + 1];
    byte buf[16];

    /*! \section TestHex2Bin Steps 6.1 to 6.4
     * \par Call hex2bin(byte * bin, const char * hex, int len)
     * this function convert a hexadecimal string hex into a binary buffer bin of length len.
     * The zero value of len means that the length is to be calculated as the strlen(hex).
     * The spaces and tabs at the beginning of hex are not allowed.
     * If operation is performed successfully, the number of bytes converted is returned.
     * In case of invalid input (invalid character) zero value is returned.
     * - bin  (O) Pointer to the buffer to be filled.
     * - hex  (I) Source string containing hexadecimal digits only.
     * - len  (I) Number of characters in the destination.
     * \n
     * \par Step 6.1
     * Conversion from hexadecimal to binary format in normal conditions
     * fonction return number of characters in the destination.
     */
    //Normal conditions
    strcpy(msg, "hex2bin 12AB 0: ");
    ret = hex2bin(buf, "12AB", 0);
    CHECK(ret == 2, lblErr);
    CHECK(memcmp(buf, "\x12\xAB", 2) == 0, lblErr);
    strcat(msg, "12AB");
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 6.2:
     *  Conversion from hexadecimal to binary format in non-zero length processing conditions
     * in this case hexadecimal string will be cut to defined length before conversion.
     */
    //Non-zero length processing
    strcpy(msg, "hex2bin 12EF 2: ");
    ret = hex2bin(buf, "12EF", 2);
    CHECK(ret == 2, lblErr);
    CHECK(memcmp(buf, "\x12\xEF", 2) == 0, lblErr);
    strcat(msg, "12EF");
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 6.3:
     *  Conversion from hexadecimal to binary format with invalid character in original hexadecimal string
     * Converion will return 0, as zero characters converted.
     */
    //Invalid character
    strcpy(msg, "hex2bin 1G23 0: ");
    ret = hex2bin(buf, "1G23", 0);
    CHECK(ret == 0, lblErr);
    strcat(msg, "OK");
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 6.4:
     *  Conversion from hexadecimal to binary format with invalid hex length
     * Converion will return 0, as zero characters converted.
     */
    //Invalid hex length
    strcpy(msg, "hex2bin 123 0: ");
    ret = hex2bin(buf, "123", 0);
    CHECK(ret == 0, lblErr);
    strcat(msg, "OK");
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblErr:
    strcat(msg, "KO");
    prtS(msg);
  lblKO:
    trcErr(ret);
    ret = 0;
  lblEnd:
    return ret;
}

static int tstBin2Num(void) {
    int ret;
    char msg[prtW + 1];
    card num;

    /*! \section TestBin2Num Step 7.1
     * \par Call bin2num()
     * this function convert a binary buffer bin of length len into a numeric value num using direct byte order.
     * The zero value of len is not accepted.
     * The return value is always len.
     * - num  (O) pointer to a numeric value to be calculated.
     * - bin  (I) pointer to the binary buffer to be converted.
     * - len  (I) number of bytes to convert.
     */
    //Normal conditions
    strcpy(msg, "bin2num 123456 3: ");
    ret = bin2num(&num, (byte *) "\x12\x34\x56", 3);
    CHECK(ret == 3, lblErr);
    CHECK(num == 0x123456L, lblErr);
    strcat(msg, "123456");
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblErr:
    strcat(msg, "KO");
    prtS(msg);
  lblKO:
    trcErr(ret);
    ret = 0;
  lblEnd:
    return ret;
}

static int tstNum2Bin(void) {
    int ret;
    char msg[prtW + 1];
    byte buf[256];

    /*! \section TestNum2Bin Steps 8.1 to 8.2
     * \par Call num2bin(byte *  bin, card num, byte len)
     * this function convert a numeric value num into a binary buffer bin of length len using direct byte order.
     * The zero value of len is not accepted.
     * - bin  (O) Pointer to the buffer to be filled.
     * - num  (I) Numeric value to be converted.
     * - len  (I) Number of bytes in the destination.
     * \n
     * \par Step 8.1
     * Conversion from numeric to binary format in normal conditions
     * fonction return number of bytes in the destination.
     */
    //Normal conditions
    strcpy(msg, "num2bin 123456 4: ");
    ret = num2bin(buf, 0x123456, 4);
    CHECK(ret == 4, lblErr);
    CHECK(memcmp(buf, "\x00\x12\x34\x56", 4) == 0, lblErr);
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);
    ret = prtS("00123456");
    CHECK(ret >= 0, lblKO);

    /*!
     * \par Step 8.2:
     *  Conversion from numeric to binary format with too small length
     * Converion will return 0, as zero bytes converted.
     */
    //len too small
    strcpy(msg, "num2bin 123456 2: ");
    ret = num2bin(buf, 0x123456, 2);
    CHECK(ret == 0, lblErr);
    strcat(msg, "OK");
    ret = prtS(msg);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;

  lblErr:
    strcat(msg, "KO");
    prtS(msg);
  lblKO:
    trcErr(ret);
    ret = 0;
  lblEnd:
    return ret;
}

void tcab0004(void) {
    int ret;

    trcS("tcab0004 Beg\n");

    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = dspStart();
    CHECK(ret >= 0, lblKO);

    /*! TEST SEQUENCE :
     * \par Step 1:  test of conversion from numeric to decimal format
     * \ref TestNum2Dec
     */

    dspLS(0, "num2dec");
    ret = tstNum2Dec();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    /*!
     * \par Step 2: test of conversion from decimal to numeric format
     * \ref TestDec2Num
     */

    dspLS(1, "dec2num");
    ret = tstDec2Num();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    /*!
     * \par Step 3: test of conversion from numeric to hexadecimal format
     * \ref TestNum2Hex
     */

    dspLS(2, "num2hex");
    ret = tstNum2Hex();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    /*!
     * \par Step 4: test of conversion from hexadecimal to numeric format
     * \ref TestHex2Num
     */

    dspLS(3, "hex2num");
    ret = tstHex2Num();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    /*!
     * \par Step 5: test of conversion from binary to hexadecimal format
     * \ref TestBin2Hex
     */

    dspLS(0, "bin2hex");
    ret = tstBin2Hex();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    /*!
     * \par Step 6: test of conversion from hexadecimal to binary format
     * \ref TestHex2Bin
     */

    dspLS(1, "hex2bin");
    ret = tstHex2Bin();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    /*!
     * \par Step 7: test of conversion from binary to numeric format
     * \ref TestBin2Num
     */

    dspLS(2, "bin2num");
    ret = tstBin2Num();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    /*!
     * \par Step 8: test of conversion from numeric to binary format
     * \ref TestNum2Bin
     */

    dspLS(3, "num2bin");
    ret = tstNum2Bin();
    CHECK(ret >= 0, lblKO);
    tmrPause(1);

    goto lblEnd;

  lblKO:
    trcErr(ret);
  lblEnd:
    tmrPause(3);
    prtStop();
    dspStop();                  //close resources
    trcS("tcab0004 End\n");
}
