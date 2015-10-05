/*
 * rsagen.c
 *
 *  Created on: Aug 30, 2011
 *      Author: Andry@H
 */
#include "prsa.h"

int RSAKeyPair(unsigned char *pubMod,int *pubModLen,unsigned char *privExp,int *privExpLen,unsigned char *pubExp,int *pubExpLen) {
	int ret;
	ret = genRSAKeyPair(1024,65537,pubMod,pubModLen,privExp,privExpLen,pubExp,pubExpLen);
	return (ret);
}


