#ifndef __STR_H

#include "def.h"

#ifdef __TEST__
#define __mnuTEST__ "1 Test>           "
#else
#define __mnuTEST__ "? Test>           "
#endif

#include "pri.h"
#include "key.h"

int traReset(void); //reset volatile memory: transaction related data = tra section
int usrInfo(word key); //retrieve user screen from database and display it

#define __STR_H
#endif
