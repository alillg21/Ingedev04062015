/// \file tcnonuta.h

/** \addtogroup tcnongroup
 * @{
*/
void tcvp0006(void);    ///<RS485 send (non UTA)
void tcvp0007(void);    ///<RS485 receive (non UTA)

void tcab0166(void);
void tcab0180(void);
void tcab0188(void);		///<LZW compression
void tcab0224(void);		///<chinese font printing

void tcml0005(void);

void tcik0068(void);    //FTP via GCL (non UTA)
void tcik0145(void);
void tcik0146(void);
void tcik0147(void);

void tcek0003(void); //USB Master

void tcns0015(void); ///<Contactless Mifare card: read factory information
void tcab0184(void); //graphics font 2 pbm
void tcrf0002(void);
void tcjs0000(void);
void tcik0148(void); ///<MAC calculation (non	uta)
void tcrm0003(void); ///<ftp
void tcik0163(void); ///<cmm
void tcik0178(void); ///<SSL implementation for Telium platform.
void tcik0180(void); ///<color graphics for EFT930BL2

void tckd0005(void); ///<pprintf8859
void tcbb0001(void); ///<printing various telium fonts 
void tcns0059(void); ///<reading driver card (telium)
void tcns0062(void); ///<reading driver card (unicapt)
void tcns0063(void); ///<IAC: client
void tcns0064(unsigned char* in, unsigned char* out); ///<IAC: server

void tcbkw001(void); ///<signature capture
void tcbkw002(void); ///<multimedia
void tcbkw003(void); ///<alphanumeric input
void tcbkw004(void); ///<swipe card
void tcbkw005(void); ///<pin entry without CGUI
void tcbkw006(void); ///<pin entry with CGUI
void tcbkw007(void); ///<idle display
void tcbkw008(void); ///<list box
void tcbkw009(void); ///<dynamic list box
void tcbkw010(void); ///<keypad mask
void tcbkw011(void); ///<enable/disable button
void tcbkw012(void); ///<multimedia advertisement
void tcbkw013(void); ///<adress input
void tcbkw014(void); ///<terms and conditions list box
void tcbkw015(void); ///<animated GIF
void tcbkw016(void); ///<numeric entry
void tcbkw017(void); ///<birthday entry
void tcbkw018(void); ///<SSN entry
void tcbkw019(void); ///<Amount entry
void tcbkw020(void); ///<Phone entry
void tcbkw021(void); ///<Y/N entry
void tcbkw022(void); ///<Bitmap buttons
void tcbkw023(void); ///<Amount OK inquiry
void tcbkw024(void); ///<Idle screen
void tcbkw025(void); ///<Cashback inquiry
void tcbkw026(void); ///<Survey question
void tcbkw027(void); ///<Signature form
void tcbkw028(void); ///<E-mail entry
void tcbkw029(void); ///<Idle with Multimedia
void tcbkw030(void); ///<PIN entry
void tcbkw031(void); ///<Text review form
void tcbkw032(void); ///<Birthday form
void tcbkw033(void); ///<Loop Images
void tcbkw034(void); ///<Ajax line display
void tcbkw035(void); ///<Alternative Birthday entry
void tcbkw036(void); ///<Alternative Phone entry
void tcbkw037(void); ///<Line display using div tag
void tcbkw038(void); ///<Radio buttons
void tcbkw039(void); ///<Checkbox buttons
void tcbkw040(void); ///<PGN file signature verification and unpacking

void tcgkz001(void); ///<test UIA com handling

/** @} */
