/** \file
 * Unitary test cases for UTA system components
 */

#include "sys.h"

/**	\addtogroup	tcsysgroup
 * @{
 */

/**	\weakgroup TSTGEN Generic	tools
 * @{
 */
void tchs0000(void);
int tchs0003(void); ///<TSA Pin Entry
void tcab0004(void); ///<conversion	functions
void tcab0009(void); ///<Structure sBuffer testing
void tcab0010(void); ///<Structure sQueue	testing
int	 tcgs0034(void);	///<hex2bin
void tcab0072(void);	///tracing
void tcgs0021(void); //bufDel
void tcgs0022(void); //bar code	reader
void tcgs0023(void); //enterCard with	bar	code reading
int  tcab0066(card id,card *date,card *time); ///<//Testing timer event
void tcjs0001(void); ///<barcode
void tcar0000(void); //ECR	test load	article
void tcar0001(void); //ECR	test online	sale w/	finish sale	(simulate	total	key	pressed)
void tcar0002(void); //ECR	test online	sale and close online	sale to	finish sale	or complete	sale.
void tcar0003(void); //ECR	test programming Taxes (retrieve ECR taxes list, update	it and reload	it in	ECR).
void tcar0004(void); //ECR	test lorad articles	list
void tcar0005(void); //
void tcab0156(void); ///<ecrPut,ecrGet
void tcik0106(void); ///<bar code reader with GUI
void tcab0141(void); ///<bar code reader
void tcjd0001(void); // ECR test	load article
void tcjd0002(void); // ECR test	online sale	w/ finish	sale (simulate total key pressed)
void tcjd0003(void); // ECR test	online sale	and	close	online sale	to finish	sale or	complete sale.
void tcjd0004(void); // ECR test	programming	Taxes	(retrieve	ECR	taxes	list,	update it	and	reload it	into ECR).
void tcjd0005(void); // ECR test	load list	items.
void tcjd0006(void); // ECR stress	test.
void tcjd0007(void); // ECR Handshake.
void tcjd0008(void); // ECR test	online sale	and	refund w/	finish sale	(simulate	total	key	pressed).
void tcjd0009(void); // ECR test	finish sale	(simulate	total	key	pressed).

void tcik0166(void); ///<mfcBip mfcText
void tcns0016(void); ///<mfcSaveBlock
void tcns0017(void);
void tcns0018(void);
void tcns0019(void);
void tcns0020(void);
void tcns0021(void);
void tcns0022(void);
void tcns0037(void);
void tcns0031(void);
void tcns0035(void);
void tcns0036(void);
void tcab0087(void); ///<HDLC
/**	@} */

/**	\weakgroup	TSTPER Perypherals	test cases
 * @{
 */
void tcab0000(void); ///<printer perypheral
void tcbb0000(void); ///<printer perypheral, playing with fonts
void tcab0001(void); ///<timer processing
void tcab0002(void); ///<text	display	functions
void tcab0003(void); ///<keyboard	processing
void tcab0005(void); ///<non volatile	memory processing
void tcab0006(void); ///<magnetic	stripe driver	function wrappers
void tcab0007(void); ///<asynchronous	smartcard	commands
void tcab0008(void); ///<internal	modem	using	CHN
void tcab0062(void); ///<ethernet
void tcab0138(void); ///<touch screen
void tcab0117(void); ///<canvas functions
void tcik0144(void); ///<fonts
void tcab0115(void); ///<printing & display tests
void tcik0149(void); ///<HDLC	(uta)
void tcik0150(void); ///<cryDiag sys (uta)
void tcik0153(void); ///<chn TCP IP, Ethernet
void tcek0001(void); ///<USB Slave
void tcek0002(void); ///<RS232
void tcab0186(void); ///<nvm sequential access
void tcab0207(void); ///<asynchronous	smartcard	commands for the card	ISO7816	non	EMV
void tcab0208(void); ///<contactless Mifare	card processing
void tcab0111(void); ///<chn FTP connection
void tcab0177(void); ///<gclSck
void tcbt0002(void); ///<getTS time stamp
void tcda0004(void); ///<ppdDspKbdKey
void tcmp0000(void); ///<dt2ts ts2dt
void tcmp0001(void); ///getTS
void tcmp0002(void); ///getCard
void tcmp0005(void); ///com functions for IP
void tcab0071(void); ///<comms integration testing
void tcab0184(void); ///<graphics font 2 pbm

void tcab0131(void); ///<printing	arabic
void tcab0137(void); ///<Printing	an image by	chunks
void tcab0177(void); ///<gclSck
void tcab0178(void); ///<gclSck+ftp
void tcnl0018(void); ///<dayOfWeek
void tcnl0022(void); ///<ismSave
void tcgs0030(void); ///<NVM DFS functions:	nvmSize(), nvmMove()
void tcik0168(void); ///<comStart

void tcab0074(void); ///<gcl receiving of data
void tcab0078(void); ///<gcl comm
void tcab0078(void); ///<internal	modem	using	GCL
void tcik0136(void); ///<GCL:	receiving	characters until terminators 

void tcik0040(void); ///<GPRS par UTA
void tcik0041(void); ///<GPRS par ingedev lib
void tcik0060(void); ///<arabic fonts
void tcik0153(void); ///<Ethernet
void tcik0160(void); ///<Send 1K	...	16K	via	GPRS
void tcik0162(void); ///<Socket
void tcik0159(void); ///<TPass+ value operations
void tcik0161(void); ///<TPass+ interface
void tcik0171(void); ///<test stability of clsImg() function for TPass+
void tcik0179(void); ///<SSL connection with chn (TELIUM)
void tcmp0008(void); ///<printing various telium fonts 
void tcns0060(void); ///<reading driver card
void tcmp0009(void); ///<biometrics enroll to ref
void tcmp0010(void); ///<biometrics verify
void tcmp0011(void); ///<biometrics clear
void tcmp0012(void); ///<biometrics enroll to DB
void tcmp0013(void); ///<biometrics identify
void tcmp0014(void); ///<biometrics destroy DB
void tcmp0015(void); ///<biometrics remove record

void tcan0020(void); ///<PPP connection with chn (Telium)
void tcan0021(void); ///<TCP connection with chn  (Telium)
/**	@} */

/**	\weakgroup TSTEMV EMV	Kernel wrappers
 * The EMV application selection and transaction flow	functions	can	be called	only in	a	predefined order.
 * These test	cases	are	interdependent:	the	transaction	flow cannot	be performed is	application	is not selected.
 * The first parameter of	each test	case is	1	if it	is the last	test case	in a sequence.
 * Other parameters	contain	the	data structures	reserved at	the	beginning.
 * @{
 */
#define	L_BUF	2048
int	tcab0011(int stop,byte *dat);	///<emvInit	wrapper
int	tcab0012(int stop,byte *dat,byte *tmp);	///<emvSelect	wrapper
int	tcab0013(int stop,byte *dat1,byte	*dat2);	///<emvFinalSelect wrapper
int	tcab0014(int stop,byte *dVAK,byte	*dTKA);	///<emvContext wrapper
int	tcab0015(int stop,byte *dVAK,byte	*dTKA,byte *dVKA,byte	*dTAK);	///<emvPrepare wrapper
int	tcab0016(int stop,byte *dVAK,byte	*dTKA,byte *dVKA,byte	*dTAK);	///<emvAuthenticate	wrapper
int	tcab0017(int stop,byte *dVAK,byte	*dTKA,byte *dVKA,byte	*dTAK);	///<Card Holder	Verification Methods (CVM) wrappers
int	tcab0018(int stop,byte *dVAK,byte	*dTKA,byte *dVKA,byte	*dTAK);	///<emvValidate	wrapper
int	tcab0019(int stop,byte *dVAK,byte	*dTKA,byte *dVKA,byte	*dTAK);	///<emvAnalyse wrapper
int	tcab0020(int stop,byte *dVAK,byte	*dTKA,byte *dVKA,byte	*dTAK);	///<emvComplete	wrapper
void tcdk0000(void); ///<Full	emv	test for SAGEM
int	tcnl0029(int stop, byte	*	dat, byte	*	tmp);	///<emvSelectStatus
/**	@} */

/**	\weakgroup TSTCRY Crypto module	test cases
 * @{
 */
void tcab0021(void); ///<cryLoadTestKey	and	cryVerify
void tcab0022(void); ///<cryLoadDKuDK
void tcab0023(void); ///<cryGetPin and crySetPpd
void tcab0061(void); ///<mac calculation
void tcfa0002(void); ///<calculateSHA1 SHA1ToPKCS1 rsaSignData
int  tcgs0033(void); ///<cryEraseKeysArray
void tcab0070(void); ///<cryStart
void tcbk0000(void); ///<cryLoadDKuDK
void tcbk0001(void); ///<cryDukptGetPin
void tcbk0002(void); ///<test and working keys downloading
void tcbk0003(void); ///<cry3DESDukptGetPin
void tcbk0004(void); ///<cryLoadDKuDK
void tcbk0005(void); ///<cryOwfInt
void tcbk0006(void); ///<cryOwfExt
void tcjl0003(void); ///<dukpt
void tcjl0005(void); ///<dukpt
void tcik0177(void); ///<cryDukptInit cryDukptPinInput TELIUM
void tctd0003( void ); ///<ppdSap,ppdClear,ppdLS
/**	@} */

/**	\weakgroup TSTINF System Information module	test cases
 * @{
 */
void tcab0050(void); ///<getPid	getAppName getDateTime setDateTime getSapSer getTS 
void tcab0183(void); ///<get IMEI & IMSI
void tcjl0000(void); ///<getSapSer
void tcjl0001(void); ///<getAppName
void tcvp0016(void); ///<setDateTime
void tcjl0004(void); ///<get code file list
void tcjl0002(void); ///<getIMEI & IMSI
/**	@} */

/**	\weakgroup TSTTSK Task & semaphore test cases
 * @{
 */
void tcab0056(void); ///start a secondary task
/**	@} */

/**	\weakgroup TSTGCL GCL test cases
 * @{
 */
void tcml0000(void); //GCL HDLC
void tcml0001(void); //GCL PPP
void tcml0002(void); //GCL GPRS
void tcml0003(void); //GCL ETH
void tcml0004(void); //GCL WIFI
void tcap0021(void); ///<gclEth
void tcfa0005(void); ///<gclEth connection with no DHCP
/**	@} */

/**	\weakgroup TSTAPR APR test cases
 * @{
 */
void tcik0164(void); ///<cryStart cryStop cryLoadTestKey cryVerify
void tcik0165(void); ///<cryLoadDKuDK
void tcik0167(void); ///<cryGetPin
void tcje0030(void); ///<Beep Click
void tcmp0003(void); ///<chnHdlc
void tcmp0004(void); ///<chnTcp5100
void tcjj0000(void); ///<APR SSA

/**	@} */
/**	@} */
