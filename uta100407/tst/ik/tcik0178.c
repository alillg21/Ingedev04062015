/** \file
 * non-UTA Unitary test case tcik0178 for SSL implementation for Telium platform.
 */

#include <string.h>
#include "sdk30.h"
#include "lib_iapp.h"
#include "sys.h"
#include "tst.h"
#include <SSL_.h>
#include <X509_.h>
#include "ssl_profile.h"

void tcik0178(void) {
    //#ifdef __CHN_SSL__
    char szName[80];
    char szBuffer[100];
    int Port;
    int ret;
    static char szHostName[40];
    char szTemp[100];
    byte Load;
    byte RcvBuf[128];

    SSL_HANDLE hSSL;
    SSL_PROFILE_HANDLE hProfile;

    FILE *hPrt = 0;

    Load = 0;
    hSSL = 0;

    trcS("ssllib_open:\n");

    ssllib_open();

    hPrt = fopen("PRINTER", "w");
    SetDefaultFont(_MEDIUM_);

    //strcpy( szHostName, "10.110.1.120" );
    //strcpy( szHostName, "217.109.89.164" );
    strcpy(szHostName, "10.10.203.231");
    Port = 443;

    memset(szName, 0, sizeof(szName));
    memcpy(szName, "PROFILE_00", PROFILE_NAME_SIZE);

    sprintf(szBuffer, "Creation Profile: %s\n", szName);
    pprintf(szBuffer);

    //ret= SSL_DeleteProfile(szName);

    hProfile = SSL_NewProfile(szName, NULL);
    if(hProfile == NULL)
        goto lblLoad;

    pprintf("Profile created\n");

    ret = SSL_ProfileSetProtocol(hProfile, TLSv1);  //SSLv3 );
    if(ret != 0)
        goto lblKO;
    ret = SSL_ProfileSetCipher(hProfile,
                               SSL_RSA | SSL_DES | SSL_3DES | SSL_RC4 | SSL_RC2
                               | SSL_MAC_MASK | SSL_SSLV3,
                               SSL_HIGH | SSL_NOT_EXP);
    if(ret != 0)
        goto lblKO;
    ret = SSL_ProfileSetKeyFile(hProfile, "/SYSTEM/CLIENT_KEY.PEM", FALSE);
    if(ret != 0)
        goto lblKO;
    ret = SSL_ProfileSetCertificateFile(hProfile, "/SYSTEM/CLIENT.CRT");
    if(ret != 0)
        goto lblKO;
    ret = SSL_ProfileAddCertificateCA(hProfile, "/SYSTEM/CA.CRT");
    if(ret != 0)
        goto lblKO;
    // Save the profile.
    ret = SSL_SaveProfile(hProfile);
    if(ret != 0)
        goto lblKO;
//      SSL_UnloadProfile( hProfile );

  lblLoad:
    pprintf("Loading SSL Profile\n");

    hProfile = SSL_LoadProfile(szName);
    if(hProfile == NULL)
        goto lblKO;

    ret = SSL_New(&hSSL, hProfile);
    if(ret != 0)
        goto lblKO;

    ret = SSL_Connect(hSSL, szHostName, Port, 2000);
    sprintf(szBuffer, "Connect: %d\n", ret);
    pprintf(szBuffer);
    if(ret == 0) {
        pprintf("Connected SSL OK !\n");
        //ttestall( 0, 300 );

        ret = SSL_Write(hSSL, "hello", 5, 300);
        sprintf(szTemp, "SSL_Write = %d\n", ret);
        pprintf(szTemp);

        ret = SSL_Read(hSSL, RcvBuf, sizeof(RcvBuf), 300);
        sprintf(szTemp, "SSL_Read = %d\n", ret);
        pprintf(szTemp);

        pprintf("Disconnecting SSL...\n");
        SSL_Disconnect(hSSL);
    } else {
        // Get the connection error.
        int nError;

        SSL_ProfileGetLastError(hProfile, &nError);
        sprintf(szTemp, "Error = %d\n", nError);
        pprintf("Connecting SSL...\n");
        pprintf(szTemp);
        ttestall(0, 300);
    }

    ret = SSL_Free(hSSL);
    sprintf(szTemp, "SSL_Free = %d\n", ret);
    pprintf(szTemp);

//          ret = SSL_UnloadProfile(szName);
//      sprintf( szTemp, "SSL_UnloadProfile = %d\n", ret );
//          pprintf(szTemp);

    ret = SSL_DeleteProfile(szName);
    sprintf(szTemp, "SSL_DeleteProfile = %d\n", ret);
    pprintf(szTemp);

    goto lblEnd;
  lblKO:
    pprintf("tcik0178 fail\n");
  lblEnd:
    ttestall(PRINTER, 0);
    fclose(hPrt);
    //#endif
}
