#ifndef __KEY_INJECT_H__
#define __KEY_INJECT_H__

int req4Key(void);
int storeWorkingKey(byte key_index,byte *key_value); //@@AS0.0.26 : export storeWorkingKey to the header file
//@@AS-SIMAS - Start
int store_acqTMK(byte *tmk);
int store_acqTMKRefNo(byte *tmkRefNo);
int store_acqTAKnTLE(byte *tak,byte *tle);
int store_acqTAKLogon(byte *tak);
#ifdef SIMAS_TMS
int storeKeyFromTMS(void);
#endif
//@@AS-SIMAS - End

#endif
