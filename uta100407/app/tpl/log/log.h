/// \file log.h
#ifndef __LOG_H

/** \defgroup sysgroup LOG Logical components
 *
 * The logical component functions are declared in the header file log.h in log directory.
 * They are implemented in the files:
 *  - HDL.C: (HanDLer) external event handler functions.
 *    The messages coming from master application (GMA) are treated here.
 *    If GMA is used these functions are called from gma.c that makes interface between GMA and UTA.
 *  - VAL.C: (VALidation) validation functions that are used to implement the fincancial logics of the application
 *    as well as administrative verifications (chacking passwords etc)
 * @{
 */

#include "str.h"

/** \weakgroup hdlgroup HDL HanDLer
 * HDL component contains the functions processing the messages coming from master application.
 * It is the entry point component of the UTA.
 * @{
 */
void hdlBefore(void); ///<called before processing any event
void hdlAfter(void); ///<called after processing any event
void hdlPowerOn(void); //POWER ON even handler
void hdlMainMenu(void); ///<main menu
int hdlTimer(card id,card *date,card *time); ///<scheduled wakeup call event
void hdlWakeUp(void); ///<this function is called by hdlTimer() once per minute in the default configuration
int hdlAskMagStripe(const char *buf); ///<asks whether the card swiped is treated by the application
void hdlMagStripe(const char *buf); ///<process magnetic stripe card if validated by hdlAskMagStripe()
void hdlSmartCard(const byte *buf); ///<process EMV card inserted
/** @} */

/** \weakgroup valgroup VAL VALidate
 * VAL component contains the functions that verify the validity of input data,
 * make decisions on whether to continue and retrieve parameters concerning the rtransaction flow
 * @{
 */
int valOpr(void); ///<validate operator
int valAmount(void); ///<validate amount
int valPan(void); ///<validate PAN
int valExpDat(void); ///<validate expired date
int valCard(void); ///<validate card
/** @} */

#define __LOG_H
#endif
