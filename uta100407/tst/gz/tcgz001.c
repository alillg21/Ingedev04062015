/** \file
 * Unitary non UTA test case for UIA com task
 *
 * Subversion reference
 *
 * $HeadURL:  $
 *
 * $Id:  $
 */

#include "basearm.h"
#include "sdk30.h"
#include "TlvTree.h"
#include "LinkLayer.h"
#include "sys.h"


#ifdef __cplusplus
extern "C"
{
#endif
#if 0
// com task states 
#define COM_INIT        1
#define COM_SEND        2
#define COM_RECEIVE     3
#define COM_EXIT        4
   
// com task errors
#define COM_SUCCESS        0
#define COM_ERR_CONNECT   -1
#define COM_ERR_RECV      -2
#define COM_ERR_SEND      -3
   
// define more meaningfull names for semaphore functions
#define REQ_SEMAPHORE OS_rtc_P
#define REL_SEMAPHORE OS_rtc_V
   
typedef struct {
   int port;
   int baudRate;
   int bitsPerByte;
   int stopBits;
   int flowCtrl;
   int parity;
} ComData_t;

typedef struct {
   int len;
   int status;
   byte buffer[4096];
   byte command;
} comMbMsg_t;

static t_topstack* hostComTaskPtr;

// mailboxes for com task
static word mbToComTask;
static word mbFromComTask;

// semaphores to protect com in/out buffers
static word semBufIn;
static word semBufOut;

// com in/out buffers - access only with semaphore protection
static comMbMsg_t comInMbBuf;
static comMbMsg_t comOutMbBuf;

//*****************************************************************************
static word createMailbox(word size) {
   word mailbox;
   tStatus status;
   
   mailbox = GetMailboxUser();
   CHECK(mailbox != 0xFFFF, lblKO);
   
   status = OS_rtc_InitMailBox(mailbox, size);
   CHECK(status ==  cOK, lblKO);

   return mailbox;
lblKO:
   return 0xFFFF;
}

//*****************************************************************************
static word createSemaphore(word len) {
   word semaphore;
   tStatus status;
   
   semaphore = GetSemaphoreUser();
   CHECK(semaphore != 0xFFFF, lblKO);
   
   status = OS_rtc_InitSemaphore(semaphore, len);
   CHECK(status ==  cOK, lblKO);

   return semaphore;
lblKO:
   return 0xFFFF;
}

//*****************************************************************************
static int get_LL_BaudRate(int baudRate) {
   int ret;
   
   switch (baudRate) {
      case 300:
         ret = LL_PHYSICAL_V_BAUDRATE_300;
         break;  
      case 1200:
         ret = LL_PHYSICAL_V_BAUDRATE_1200;
         break;  
      case 2400:
         ret = LL_PHYSICAL_V_BAUDRATE_2400;
         break;  
      case 4800:
         ret = LL_PHYSICAL_V_BAUDRATE_4800;
         break;  
      case 9600:
         ret = LL_PHYSICAL_V_BAUDRATE_9600;
         break;  
      case 19200:
         ret = LL_PHYSICAL_V_BAUDRATE_19200;
         break;  
      case 38400:
         ret = LL_PHYSICAL_V_BAUDRATE_38400;
         break;  
      case 57600:
         ret = LL_PHYSICAL_V_BAUDRATE_57600;
         break;  
      case 115200:
      default:
         ret = LL_PHYSICAL_V_BAUDRATE_115200;
         break;  
   }
   return ret;
}

//*****************************************************************************
static int get_LL_BitsPerByte(int bitsPerByte) {
   int ret;
   
   if (bitsPerByte == 7) 
      ret = LL_PHYSICAL_V_7_BITS;
   else
      ret = LL_PHYSICAL_V_8_BITS;
   return ret;
}

//*****************************************************************************
static int get_LL_StopBits(int stopBits) {
   int ret;
   
   if (stopBits == 2) 
      ret = LL_PHYSICAL_V_2_STOP;
   else
      ret = LL_PHYSICAL_V_1_STOP;
   return ret;
}

//*****************************************************************************
static int get_LL_FlowCtrl(int flowCtrl) {
   int ret;
   
   if (flowCtrl) 
      ret = LL_PHYSICAL_V_HARD_FLOW_CTRL;
   else
      ret = LL_PHYSICAL_V_NO_FLOW_CTRL;
   return ret;
}

//*****************************************************************************
static int get_LL_Parity(int parity) {
   int ret;
   
   if (parity == 1) 
      ret = LL_PHYSICAL_V_ODD_PARITY;
   else if (parity == 2)
      ret = LL_PHYSICAL_V_EVEN_PARITY;
   else
      ret = LL_PHYSICAL_V_NO_PARITY;
   return ret;
}

//*****************************************************************************
static int configHostCom(LL_HANDLE* phSession, ComData_t cD) {
   int retCode;
   TLV_TREE_NODE hConfig         = NULL;
   TLV_TREE_NODE hPhysicalConfig = NULL;
   
   // Create the Link Layer configuration parameter tree
   hConfig = TlvTree_New(LL_TAG_LINK_LAYER_CONFIG);
   
   // Physical layer parameters
   hPhysicalConfig = TlvTree_AddChild(hConfig, LL_TAG_PHYSICAL_LAYER_CONFIG, NULL, 0);
  
   TlvTree_AddChildInteger(hPhysicalConfig, LL_PHYSICAL_T_LINK, LL_PHYSICAL_V_COM0,  LL_PHYSICAL_L_LINK);

   TlvTree_AddChildInteger(hPhysicalConfig, LL_PHYSICAL_T_BAUDRATE, get_LL_BaudRate(cD.baudRate),  LL_PHYSICAL_L_BAUDRATE);
   
   TlvTree_AddChildInteger(hPhysicalConfig, LL_PHYSICAL_T_BITS_PER_BYTE, get_LL_BitsPerByte(cD.bitsPerByte), LL_PHYSICAL_L_BITS_PER_BYTE);
   
   TlvTree_AddChildInteger(hPhysicalConfig, LL_PHYSICAL_T_STOP_BITS, get_LL_StopBits(cD.stopBits),  LL_PHYSICAL_L_STOP_BITS);
   
   TlvTree_AddChildInteger(hPhysicalConfig, LL_PHYSICAL_T_PARITY, get_LL_Parity(cD.parity),  LL_PHYSICAL_L_PARITY);
   
//   TlvTree_AddChildInteger(hPhysicalConfig, LL_PHYSICAL_T_FLOW_CTRL, get_LL_FlowCtrl(cD.flowCtrl),  LL_PHYSICAL_L_FLOW_CTRL);   

   // Configure the Link layer
   retCode = LL_Configure(phSession, hConfig);
   
   printf("LL_Configure returned %d", retCode);
   // Release the configuration TLV tree 
   TlvTree_Release(hConfig);
   
   return retCode;
}

//*****************************************************************************
static word hostComTask() {
   LL_HANDLE* phSession = NULL;
   ComData_t comData = {0, 115200, 8, 1, 0, 0};
   comMbMsg_t* msg;
   Boolean done = FALSE;
   int retCode;
   tStatus status; 
   
   
   while (!done) {
//      status = OS_rtc_Receive(mbToComTask, (DoubleWord*) msg, 0);
      
//      memset(&comOutMbBuf, 0, sizeof(comOutMbBuf));
//      comOutMbBuf.command = msg->command;
      
//      switch (msg->command) {
//         case COM_INIT:
            retCode = configHostCom(phSession, comData);
            if (retCode != LL_ERROR_OK) break;
            comOutMbBuf.status = LL_Connect(*phSession);
            break;
#if 0
        case COM_SEND:
            REQ_SEMAPHORE(semBufOut, 0);
            comOutMbBuf.len = LL_Send(*phSession, msg->len, msg->buffer, LL_INFINITE);
            REL_SEMAPHORE(semBufOut);
            comOutMbBuf.status = LL_GetLastError(*phSession);
            break;
         case COM_RECEIVE:
            REQ_SEMAPHORE(semBufIn, 0);
            comOutMbBuf.len = LL_Receive(*phSession, sizeof(msg->buffer), comOutMbBuf.buffer, LL_INFINITE);
            REL_SEMAPHORE(semBufIn);
            comOutMbBuf.status = LL_GetLastError(*phSession);
            break;
         case COM_EXIT:
            comOutMbBuf.status = LL_Disconnect(*phSession);
            done = TRUE;
            break;
         default:
            break;
      }
#endif
      
//      status = OS_rtc_Send(mbFromComTask, (DoubleWord) &comOutMbBuf);
      
   }
   return 0;
}
#endif
//*****************************************************************************
void tcgkz001(void) {
#if 0
   comMbMsg_t* msg;
   int priority = 0;
   int i;
   tStatus status;
   
   mbToComTask = createMailbox(20);
   CHECK(mbToComTask != 0xFFFF, lblKO);
   
   
   mbFromComTask = createMailbox(20);
   CHECK(mbFromComTask != 0xFFFF, lblKO);
   
   semBufIn = createSemaphore(1);
   CHECK(semBufIn != 0xFFFF, lblKO);   

   semBufOut = createSemaphore(1);
   CHECK(semBufOut != 0xFFFF, lblKO);   

   hostComTaskPtr = fork(hostComTask, NULL, priority);
   
   REQ_SEMAPHORE(semBufIn, 0);
   memset(&comInMbBuf, 0, sizeof(comInMbBuf));
   comInMbBuf.command = COM_INIT;
   REL_SEMAPHORE(semBufIn);
   status = OS_rtc_Send(mbToComTask, (DoubleWord) &comInMbBuf);
   
   status = OS_rtc_Receive(mbFromComTask, (DoubleWord*) msg, 0);
   
   for (i = 1; i < 5; i++) {
      
      REQ_SEMAPHORE(semBufIn, 0);
      memset(&comInMbBuf, 0, sizeof(comMbMsg_t));
      comInMbBuf.command = COM_RECEIVE;
      comInMbBuf.len = sizeof(comInMbBuf.buffer);
      REL_SEMAPHORE(semBufIn);
      status = OS_rtc_Send(mbToComTask, (DoubleWord) &comInMbBuf);
      
      status = OS_rtc_Receive(mbFromComTask, (DoubleWord*) msg, 0);
      REQ_SEMAPHORE(semBufIn, 0);
      REQ_SEMAPHORE(semBufOut, 0);
      memcpy(&comInMbBuf, msg, sizeof(comMbMsg_t));
      REL_SEMAPHORE(semBufOut);
      comInMbBuf.command = COM_SEND;
      comInMbBuf.status = 0;
      REL_SEMAPHORE(semBufIn);
      status = OS_rtc_Send(mbToComTask, (DoubleWord) &comInMbBuf);
   }
   
   REQ_SEMAPHORE(semBufIn, 0);
   memset(&comInMbBuf, 0, sizeof(comMbMsg_t));
   comInMbBuf.command = COM_EXIT;
   REL_SEMAPHORE(semBufIn);
   status = OS_rtc_Send(mbToComTask, (DoubleWord) &comInMbBuf);

lblKO:
   FreeMailboxUser(mbToComTask);
   FreeMailboxUser(mbFromComTask);
   FreeSemaphoreUser(semBufIn);
   FreeSemaphoreUser(semBufOut);
#endif
   return;
}

#ifdef __cplusplus
}
#endif
