//Subversion reference
//$HeadURL: https://svn.ingenico.com/UTAPROJECTS/sea-partner/5W/trunk/ISO5WPH/log/iso.c $
//$Id: iso.c 1490 2009-02-05 09:39:05Z jelemia $

#include "sys.h"
#include "iso.h"

#define LLBCD (-2)
#define LLLBCD (-3)
#define LLASC (-4)
#define LLLASC (-5)

static int fmt[isoBitEnd - isoBitBeg - 1] = {
            1,                          //001 Bit Map, Secondary
            LLBCD,                      //002 Primary Account Number (PAN)
            6,                          //003 Processing Code
            12,                         //004 Amount, Transaction
            0,                          //-005 not used
            0,                          //-006 not used
            10,                         //-007 Transmission Date and Time
            8,                          //-008 TMS max buffer rcv size
            0,                          //-009 not used
            0,                          //-010 not used
            6,                          //011 System Trace Audit Number
            6,                          //012 Time, Local Transaction
            4,                          //013 Date, Local Transaction
            4,                          //014 Date, Expiration
            4,                          //015 Date, Settlement
            0,                          //-016 not used
            0,                          //-017 not used
            0,                          //-018 not used
            0,                          //-019 not used
            0,                          //-020 not used
            0,                          //-021 not used
            3,                          //022 Point-of-Service Entry Mode
            3,                          //-023 not used
            3,                          //024 Network International Identifier
            2,                          //025 POS Condition Code
            2,                          //-026 Card Acceptor Business Code
            0,                          //-027 not used
            6,                          //-028 Reconciliation Date
            0,                          //-029 not used
            12,                         //-030 Original Amount
            0,                          //-031 not used
            LLASC,                      //-032 Acquirer Institution Identification Code
            0,                          //-033 not used
            0,                          //-034 not used
            LLBCD,                      //035 Track 2 Data
            0,                          //-036 not used
            24,                         //037 Retrieval Reference Number
            12,                         //038 Authorization Identification Response
            4,                          //039 Response Code
            0,                          //-040 not used
            16,                         //041 Card Acceptor Terminal Identification
            30,                         //042 Card Acceptor Identification Code
            0,                          //-043 not used in SAMA
            0,                          //-044 not used in SAMA
            0,                          //-045 not used in SAMA
            0,                          //-046 not used in SAMA
            LLLASC,                     //-047 Private Card Scheme Sponser ID
            LLLASC,                     //048 Pin/MAC session keys
            3,                          //049 Currency Code, Transaction
            3,                          //-050 Currency Code, Settlement
            0,                          //-051 not used in SAMA
            16,                         //052 Personal Identification Number (PIN) Data
            16,                         //-053 Security Related Control Information
            LLLASC,                     //054 Additional Amounts
            LLLASC,                     //-055 ICC System related data
            LLASC,                      //-056 Original Data Elements
            LLLASC,                     // 057 - @agmr - BRI
#ifdef LINE_ENCRYPTION
            LLLASC, //@@AS0.0.10        //-058 not used
#else
            0,                          //-058 not used
#endif //LINE_ENCRYPTION
            0,                          //-059 not used
            LLLASC,                     //060 Private use, Batch Number
            LLLASC,  //@agmr - BRI6     //061 Amount original transaction
            LLLASC,                     //062 Reconciliation Totals
            LLLASC,                     //063 Additional Data
            16,                         //064 Message Authentication Code (MAC)
        };
int isoFmt(byte bit)
{
	VERIFY(isoBitBeg < bit);
	VERIFY(bit < isoBitEnd);
	return fmt[bit - 1];
}
