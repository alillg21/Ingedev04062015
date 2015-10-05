#ifndef _TC_H
#define _TC_H

// tc specific application functions
//
const char *tcIdleMsg(void);
const char *tcIdleApp(void);


#define TC_BUF_LEN ((__DSPH__- 2) * __DSPW__)  // 13 lines / 15 for a 8550 terminal

#define __CANCEL_KEY__ 59
#define __DEL_KEY__    61
#define __VALID_KEY__  58
#define __00_KEY__     60
#define __F1_KEY__     64
#define __F2_KEY__     65
#define __BUT_KEY__    69
#define __UP_BUT_KEY__ 67
#define __DN_BUT_KEY__ 71

// tclib functions
//
//void  tcStart(void);
//void  tcStop(void);

// Return the next parameter as a string,
// from the parameter array defined in tcapp.c
const char *tcStrParam(void);

// Return the next parameter as an integer,
// from the parameter array defined in tcapp.c
int tcIntParam(void);

// Set 'ret' as the return code for the test case,
// as read in tclib
void tcSetRet(int ret);

// Efface l'�cran, positionne la position d'affichage � lnVal
void tcDspInit(unsigned int lnVal);

// Renvoie le buffer d'affichage pour �tre adress� directement par l'applicatif
char *tcDspBuf(void);

// d�finit la prochaine position d'affichage une ligne plus bas
// ou en premi�re ligne lorsque le bas d'�cran est atteint.
// A cette position est affich�e une ligne de s�paration
// compos�e du caract�re fill (exemple : "-------" pour fill = '-')
void tcLnFeed(char fill);

// Remember the current display position on screen
void tcPushLn(void);

// Return to the remembered display position on screen
void tcPopLn(void);

// Affiche la cha�ne de caract�res str.
// Si celle-ci est plus longue que l'�cran, la fin de l'affichage est
// report�e sur la(es) ligne(s) suivante(s).
// Si str est NULLe, c'est le buffer par d�faut qui est affich�.
// Ce buffer contient le dernier affichage effectu�.
// Il peut aussi avoir �t� pr�c�demment r�cup�r� par tcDspBuf() pour �tre
// initialis� dans le test case (par exemple pour un format sprintf
// non support� par tclib)
int tcDspS(const char *str);

// Affiche la cha�ne de caract�res str en �crasant le dernier affichage
int tcDspPS(const char *str);

// Affiche la cha�ne de caract�res str selon le format fmt (voir sprintf)
int tcDspFS(const char *fmt, const char *str);

// Affiche la valeur val selon le format fmt (voir sprintf)
int tcDspFN(const char *fmt, int val);

// Affiche les valeurs val1 et val2 selon le format fmt (voir sprintf)
int tcDspF2N(const char *fmt, int val1, int val2);

// Affiche le buffer buf de longueur len. Les caract�res alphanum�riques
// sont affich�s en clair, les autres au format hexad�cimal. Si existe une s�quence
// d'un de ces caract�res d'une longueur > 3, 10 fois '\0' par exemple,
// on affichera "\0(10x)" au lieu de "\0\0\0\0\0\0\0\0\0\0".
int tcDspAN(const unsigned char *buf, int len);

// Display the title and subtitle of the test case
int tcDspTitle(const char *title, const char *subTitle);

// Imprime la cha�ne de caract�res str.
// Si str est NULLe, c'est le buffer pr�c�demment r�cup�r� par tcDspBuf()
// pour initialisation, ou contenant le dernier affichage, qui est imprim�.
void tcPrt(const char *str);

// Renvoie au test case en cours d'ex�cution la valeur d'it�ration courante
int tcLoop(void);

// Displays the prompt msg and waits for a keyboard entry
// if ret < 0, the terminal makes a bip
// if ret > 0
char tcWait(const char *msg, int ret);

// Affiche l'invitation prompt et attend les entr�es clavier
// qui composeront le nombre num.
// num contient la valeur initiale par d�faut, puis la valeur entr�e par l'utilisateur
// minVal et maxVal d�finissent l'intervalle de validit� de la valeur saisie
// to est la limite de temps pour chaque entr�e clavier.( 0 pour pas de limite)
int tcWaitNum(const char *prompt, int *num, int minVal, int maxVal, int to);

// Affiche l'invitation prompt et attend les entr�es clavier
// qui composeront la cha�ne de caract�res dans dst.
// dst contient la cha�ne initiale, puis la cha�ne entr�e par l'utilisateur
void tcWaitS(char *prompt, char *dst, int len);

// Set the way displays are performed
// Return 1 if the dspMode option is enabled, 0 otherwise
int tcDspMode(int option);

// Enable the dspMode option if option is positive,
// disable it if option is negative
unsigned long tcDspModeSet(int option);

// Reset dspMode to 0, enable the dspMode option
unsigned long tcDspModeReset(int option);

// stat may be chosen among dspModes :
typedef enum
{
	DSP_ENABLE   = 0x01,
	SCR_BY_SCR   = 0x2,
	STP_BY_STP   = 0x4,
	TRC_2_HMI    = 0x8,   // Enable/disable the redirection of traces to the screen
	MSG_2_PRT    = 0x10,  // Enable/Disable the printing of the displays
	MSG_2_SER    = 0x20,  // Enable/disable the copying of displays to the serial port 
	KBD_CHECK    = 0x40,
	BEEP_ON      = 0x80,
	SHOW_TI      = 0x100, // Enable/disable the showing of time stamp while displaying traces on screen or printer
	IN_MENU      = 0x200
} dspModes;

// D�finit si un contr�le d'entr�e clavier est effectu� � l'occasion de
// chaque affichage. Cela est utile lorsque une boucle dans un test case
// risque d'�tre infinie. L'inconv�nient est que ce contr�le retarde chaque affichage
// d'un centi�me de seconde
int tcKbdCheck(int onOff);

// Returns the time interval in hundreds of seconds since the last call
int tcTimeInterval(void);

// Print msg followed by current date and time
// msg should be less than 100 char
void tcPrtDateTime(const char *msg);

// Test the current value of stack pointer (stackPtr).
// If this is a max, print it followed by the
// function name (fnc) where this max was reached
void tcCheckStack(void *stackPtr, const char *fnc);

// Copy ipVal to ip in the form "XX.XX.XX.XX"
void tcDecodeIpAdrs(char *ip, int ipVal);

// Return connection info into any (or all) from connected, localIpAddr, dns1, dns2
int tcConnectInfo(unsigned char *connected, unsigned long *localIpAddr,
				  unsigned long *dns1, unsigned long *dns2);

// Decode UART error err,
// writing symbolic value for this error into errBuf
int tcDecodeComError(int err);

// Decode GSM error err,
// writing symbolic value for this error into errBuf
// (GSM_ERROR_START = -13312)
int tcDecodeGsmError(int err);

// Decode GCL error err,
// writing symbolic value for this error into errBuf
// GCL_ERROR_BASE = -14848
int tcDecodeGclError(int err);

// Check wether the CANCEL key was pressed
int tcCancel(void);

// Entry point of the test case application, called in gma.c
void selectMain(void);

// Functions for the redirection of the traces to the display
int trcDspS(const char *str);
int trcDspAN(const unsigned char *buf, int len);
int trcDspFN(const char *fmt, int val);
int trcDspFS(const char *fmt, const char *str);

// Redirection of the traces to the screen
//
#if defined __TRC_2_HMI__ | defined __MSG_2_PRT__

#undef trcS
#undef trcFS
#undef trcFN
#undef trcAN
#undef trcBN
#undef trcBAN

#define trcS   trcDspS
#define trcFS  trcDspFS
#define trcFN  trcDspFN
#define trcAN  trcDspAN
#define trcBN  trcDspAN
#define trcBAN trcDspAN

#endif


// Original type0 test cases : no prameter, no return code
typedef void (tc0_t)(void);
typedef tc0_t *tc0p_t;

tc0_t tcjd0001;  // ECR test load article
tc0_t tcjd0002;  // ECR test online sale w/ finish sale (simulate total key pressed)
tc0_t tcjd0003;  // ECR test online sale and close online sale to finish sale or complete sale.
tc0_t tcjd0004;  // ECR test programming Taxes (retrieve ECR taxes list, update it and reload it into ECR).
tc0_t tcjd0005;  // ECR test load list items.
tc0_t tcjd0006;  // ECR stress test.
tc0_t tcjd0007;  // ECR Handshake.
tc0_t tcjd0008;  // ECR test online sale and refund w/ finish sale (simulate total key pressed).
tc0_t tcjd0009;  // ECR test finish sale (simulate total key pressed).
tc0_t tcjd0010;  // ECR test daily sales reporting.
tc0_t tcjd0011;  // ECR test sales done on ECR and reported on Terminal.
tc0_t tcjd0012;  // ECR financial reports : X and Z reports.

tc0_t tcjd0025;  // BARE CODE Printing.
tc0_t tcjd0026;  // BARE CODE Reading.

tc0_t tcjd0020;  // INGESTATE schedule testing

tc0_t tcjd0039;  // GPRS Connection
tc0_t tcjd0040;  // GPRS Connection
tc0_t tcik0041;  // GPRS Test IP
tc0_t tcjd0041;  // GPRS Test IP
tc0_t tcjd0045;  // GPRS Test parsing functions
tc0_t tcjd0050;  // GPRS IP connection
tc0_t tcjd0051;  // GPRS Get IMSI-IMEI
tc0_t tcjd0052;  // GPRS Send msg
tc0_t tcjd0066;  // GPRS Send msg
tc0_t tcjd0183;  // buffer primitives testing

tc0_t tcjd0046;  // tclib testing

// Test GCL lib and GSM plugin of GMA
tc0_t tcjd0177;  // GCL connection : require libs : GCL, GCL_ETH, GCL_PGSUP, gmaLib, gmaLibPgComm
tc0_t tcik0099;  // GCL connection non UTA, GMA plugin, handshake test
tc0_t tcjd0178;  // GCL tcp session

tc0_t tcab0009;  // buffer primitives testing
tc0_t tcab0120;
tc0_t tcab0121;
tc0_t tcab0122;
tc0_t tcab0123;
tc0_t tcab0143;
tc0_t tcab0150;


// menu structures
//
#define NO_WAIT    0  // no wait between test iterations
#define EOT_WAIT   1  // wait for a key press at the end of the test
#define ITER_WAIT  2  // wait for a key press at the end of every iteration


#define PARAM_NUM 5

typedef struct
{
	int wait;   // EOT_WAIT  : wait for a key press at the end of the test
				// ITER_WAIT : wait for a key press at the end of every iteration
				// or NO_WAIT
	int pause;	// pause in seconds between two test iterations
	
	void *tc;
	int loop;   // number of test iterations
	
	const char *param[PARAM_NUM]; 

} tcCfg_t;


typedef enum
{
	NO_CHECK,
	DO_CHECK
} checkEntry_t;


typedef struct mnuCfg
{
	const char   *label;
	unsigned char choice;
	
	void         *mnTc;
	int loop; // number of test iterations (used for all typ except TC_MNU)

	const char *param[PARAM_NUM];

} mnuCfg_t;

mnuCfg_t *tcMenu(void);

#endif // _TC_H

