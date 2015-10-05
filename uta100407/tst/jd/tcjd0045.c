#include <stdio.h>
#include <unicapt.h>
#include <string.h>
#include "sys.h"
#include "tc.h"

//Unitary test case tcjd0045.
//Function testing of GPRS parsing functions
// 		parseStr
// 		parseIp

static code const char *srcFile= __FILE__;

extern const char *parseStr(char *dst, const char *src, int dim);
extern const char *parseIp(char *adr, const char *str);


int tcjd0045(int pI1, int pI2, char *pS1, char *pS2)
{
	int ret;
{
//comSetTcp
	const char *init = "10.10.59.27|2000|";   //AB
	char adr[15 + 1];
	char msk[15 + 1];
	char gateway[15 + 1];
	char port[10];

	init = parseStr(adr, init, sizeof(adr));
	VERIFY(*init);
	CHECK(strcmp(adr, "10.10.59.27") == 0, lblKO);
			
	// parse port
	init = parseStr(port, init, sizeof(port));
	CHECK(strcmp(port, "2000") == 0, lblKO);

	// parse msk
	init = parseStr(msk, init, sizeof(msk));
	CHECK(strcmp(msk, "") == 0, lblKO);

	// parse default gateway
	init = parseStr(gateway, init, sizeof(gateway));
	CHECK(strcmp(gateway, "") == 0, lblKO);
	tcDsp("comSetTcp OK");
}
{
//comSetGprs
	char usr[32];
	char pwd[32];
	char pin[8 + 1];
	char m_apn[64 + 1];
    char ip[20];

	const char *init = "0000|orange.fr|danfr|mq36noqt|";

	init = parseStr(pin, init, sizeof(pin));      // extract pin
	CHECK(strcmp(pin, "0000") == 0, lblKO);
	init = parseStr(m_apn, init, sizeof(m_apn));  // extract apn
	CHECK(strcmp(m_apn, "orange.fr") == 0, lblKO);
	init = parseStr(usr, init, sizeof(usr));      // extract usr
	CHECK(strcmp(usr, "danfr") == 0, lblKO);
	init = parseStr(pwd, init, sizeof(pwd));      // extract password
	CHECK(strcmp(pwd, "mq36noqt") == 0, lblKO);
	(void) parseStr(ip, init, sizeof(ip));        // extract IP
	CHECK(strcmp(ip, "") == 0, lblKO);
	tcDsp("comSetGprs OK");
}
{
//comDialIp
	char adr[16];  //tcp address NNN.NNN.NNN.NNN
	byte ladr[4];
	char RemotePort[10];
	char LocalPort[10];

//	const char *srv = "10.10.59.27-2000";   //AB
//	const char *srv = "10.10.59.27|2000|127.12.13.14|2100";
	const char *srv = "||127.12.13.14|2100";

	srv = parseStr(adr, srv, sizeof(adr));
//	CHECK(strcmp(adr, "10.10.59.27") == 0, lblKO);
	CHECK(strcmp(adr, "") == 0, lblKO);

	srv = parseStr(RemotePort, srv, sizeof(RemotePort));
//	CHECK(strcmp(RemotePort, "2000") == 0, lblKO);
	CHECK(strcmp(RemotePort, "") == 0, lblKO);

	// GET lOCAL IP
	srv = parseIp(ladr, srv);
	CHECK(ladr[0] == 127
		  && ladr[1] == 12
		  && ladr[2] == 13
		  && ladr[3] == 14, lblKO);
	// extract Local Port
	CHECK(*srv, lblKO); // Check srv is not finished
	(void) parseStr(LocalPort, srv, sizeof(LocalPort));
	CHECK(strcmp(LocalPort, "2100") == 0, lblKO);
	tcDsp("comDialIp OK");
}
{
//comListen
	const char *srv = "192.1.1.2|8000|255.255.255.0";
	byte adr[15 + 1];  // tcp address NNN.NNN.NNN.NNN
	char port[4 + 1];
	char msk[15 + 1];

	srv = parseStr(adr, srv, sizeof(adr));
	CHECK(strcmp(adr, "192.1.1.2") == 0, lblKO);

	srv = parseStr(port, srv, sizeof(port));  // start port
	CHECK(strcmp(port, "8000") == 0, lblKO);

	(void) parseStr(msk, srv, sizeof(msk));    // start mask
	CHECK(strcmp(msk, "255.255.255.0") == 0, lblKO);
	tcDsp("comListen OK");
}

	goto lblEnd;
	
lblKO:
	tcDsp(" KO!");
	goto lblEnd;
lblEnd:
}
