#include "unicapt.h"
#include "gmaDefines.h"
#include "menuLib.h"
#include "tst.h"

static uint32 hHmi = 0;
static uint8 dat1[L_BUF];
static uint8 dat2[L_BUF];
static uint8 dat3[L_BUF];
static uint8 dat4[L_BUF];

//Simple shortcuts
static void hmiStart(void) {
    hmiOpenShared("DEFAULT", &hHmi, HMI_CH_DISPLAY | HMI_CH_KEYBOARD, 0);
}

static void hmiStop(void) {
    hmiClose(hHmi);
}

static void makeMenu(const char *hdr, const char *mnu[]) {
    int i;

    mnuReset(hHmi, 0);
    mnuSetTitle(hdr);
    mnuSetTimeout(60 * 100);
    i = 0;
    while(mnu[i]) {
        mnuAddItem(mnu[i], TRUE, i);
        i++;
    }
}

//Test case runners
#define RUN(TC) hmiStop(); TC; hmiStart();

static void mnuSYS_ToolBox(const char *hdr) {    // SYS - ToolBox
    const char *mnu[] = {
        "Conversion",
        "Buffer",
        "Queue",
        0
    };
    const char *ptr;
    int i;

    makeMenu(hdr, mnu);

    i = 0;
    while(42) {
        i = mnuRun(i);
        if(i < 0)
            break;
        ptr = mnu[i];
        switch (i) {
          case 0:
        	  RUN(tcab0004()); // SYS - ToolBox - Conversion
              break;
          case 1:
        	  RUN(tcab0009()); // SYS - ToolBox - Buffer
              break;
          case 2:
        	  RUN(tcab0010()); // SYS - ToolBox - Queue
              break;
          default:
              break;
        }
    }
}

static void mnuSYS_Hmi(const char *hdr) {    // SYS - Hmi
    const char *mnu[] = {
        "Printer",
        "Display",
        "Canvas",
        "Keyboard",        
        0
    };
    const char *ptr;
    int i;

    makeMenu(hdr, mnu);

    i = 0;
    while(42) {
        i = mnuRun(i);
        if(i < 0)
            break;
        ptr = mnu[i];
        switch (i) {
          case 0:
              RUN(tcab0000());
              break;
          case 1:
              RUN(tcab0002());
              break;
          case 2:
              RUN(tcab0117());
              break;
          case 3:
              RUN(tcab0003());
              break;              
          default:
              break;
        }
    }
}

static void mnuSYS_Ingenico(const char *hdr) {    // SYS - Ingenico
    const char *mnu[] = {
        "Timer",
        "Magnetic",
        "Smart",
        "PinPad",
        "System",        
        0
    };
    const char *ptr;
    int i;

    makeMenu(hdr, mnu);

    i = 0;
    while(42) {
        i = mnuRun(i);
        if(i < 0)
            break;
        ptr = mnu[i];
        switch (i) {
          case 0:
              RUN(tcab0001());
              break;
          case 1:
              RUN(tcab0006());
              break;
          case 2:
              RUN(tcab0007());
              break;
          case 3:
              RUN(tcab0003());
              break;
          case 4:
              RUN(tcab0050());
              break;              
          default:
              break;
        }
    }
}

static void mnuSYS_Memory(const char *hdr) {    // SYS - Memory
    const char *mnu[] = {
        "Nvm",
        "Ism",
        0
    };
    const char *ptr;
    int i;

    makeMenu(hdr, mnu);

    i = 0;
    while(42) {
        i = mnuRun(i);
        if(i < 0)
            break;
        ptr = mnu[i];
        switch (i) {
          case 0:
              RUN(tcab0005());
              break;
          case 1:
              RUN(tcnl0022());
              break;
          default:
              break;
        }
    }
}

static void mnuSYS_Gcl(const char *hdr) { // SYS - Gcl
    const char *mnu[] = {
        "GclCom",
        "GclEth",        
        0
    };
    const char *ptr;
    int i;

    makeMenu(hdr, mnu);

    i = 0;
    while(42) {
        i = mnuRun(i);
        if(i < 0)
            break;
        ptr = mnu[i];
        switch (i) {
          case 0:
              RUN(tcab0078());
              break;
          case 1:
              RUN(tcml0003());
              break;
          default:
              break;
        }
    }
}

static void mnuSYS_Crypto(const char *hdr) {    // SYS - Crypto
    const char *mnu[] = {
        "LoadTestKey",
        "LoadDkuDk",
        "GetPin",
        "Mac",
        "Dukpt",        
        "Owf",
        0
    };
    const char *ptr;
    int i;

    makeMenu(hdr, mnu);

    i = 0;
    while(42) {
        i = mnuRun(i);
        if(i < 0)
            break;
        ptr = mnu[i];
        switch (i) {
          case 0:
              RUN(tcab0021());
              break;
          case 1:
              RUN(tcab0022());
              break;
          case 2:
              RUN(tcab0023());
              break;
          case 3:
              RUN(tcab0061());
              break;
          case 4:
              RUN(tcbk0001());
              break;
          case 5:
              RUN(tcbk0005());
              break;              
          default:
              break;
        }
    }
}


static void mnuSYS_EMV(const char *hdr) {    // SYS - Crypto
    const char *mnu[] = {
    				"Init",
    		        "Select",
    		        "FinalSelect",
    		        "Context",
    		        "Prepare",
    		        "Authenticate",
    		        "CVM",
    		        "Validate",
    		        "Analyze",
    		        "Complete",
    		        0
    };
    const char *ptr;
    int i;

    makeMenu(hdr, mnu);

    i = 0;
    while(42) {
        i = mnuRun(i);
        if(i < 0)
            break;
        ptr = mnu[i];
        switch (i) {
                 case 0:
                     RUN(tcab0011(1, dat1));
                     break;
                 case 1:
                     RUN(tcab0012(1, dat1, dat2));
                     break;
                 case 2:
                     RUN(tcab0013(1, dat1, dat2));
                     break;
                 case 3:
                     RUN(tcab0014(1, dat1, dat2));
                     break;
                 case 4:
                     RUN(tcab0015(1, dat1, dat2, dat3, dat4));
                     break;
                 case 5:
                     RUN(tcab0016(1, dat1, dat2, dat3, dat4));
                     break;
                 case 6:
                     RUN(tcab0017(1, dat1, dat2, dat3, dat4));
                     break;
                 case 7:
                     RUN(tcab0018(1, dat1, dat2, dat3, dat4));
                     break;
                 case 8:
                     RUN(tcab0019(1, dat1, dat2, dat3, dat4));
                     break;
                 case 9:
                     RUN(tcab0020(1, dat1, dat2, dat3, dat4));
                     break;
                 default:
                     break;
               }
    }
}


static void mnuSYS(const char *hdr) { // SYS
    const char *mnu[] = {
        "ToolBox",
        "Hmi",
        "Ingenico",
        "Memory",
        "GCL",
        "Task",
        "Crypto",
        "Emv",
        0
    };
    const char *ptr;
    int i;

    makeMenu(hdr, mnu);

    i = 0;
    while(42) {
        i = mnuRun(i);
        if(i < 0)
            break;
        ptr = mnu[i];
        switch (i) {
          case 0:
              mnuSYS_ToolBox(ptr);
              break;
          case 1:
              mnuSYS_Hmi(ptr);
              break;
          case 2:
              mnuSYS_Ingenico(ptr);
              break;
          case 3:
              mnuSYS_Memory(ptr);
              break;
          case 4:
              mnuSYS_Gcl(ptr);
              break;
          case 5:
        	  RUN(tcab0056());
              break;
          case 6:
              mnuSYS_Crypto(ptr);
              break;
          case 7:
                       mnuSYS_EMV(ptr);
                       break;
          default:
              break;
        }
    }
}

static void mnuPRI_Standard(const char *hdr) {    // PRI - Standard
    const char *mnu[] = {   		
        "CheckLuhn",
        "Strings",
        "Algo",
        0
    };
    const char *ptr;
    int i;

    makeMenu(hdr, mnu);

    i = 0;
    while(42) {
        i = mnuRun(i);
        if(i < 0)
            break;
        ptr = mnu[i];
        switch (i) {
          case 0:
        	  RUN(tcab0053()); // PRI - Standard - CheckLuhn
              break;
          case 1:
        	  RUN(tcab0049()); // PRI - Standard - Strings
              break;
          case 2:
        	  RUN(tcab0057()); // PRI - Standard - Algo
              break;
          default:
              break;
        }
    }
}

static void mnuPRI_Dialog(const char *hdr) {    // PRI - Dialog 
    const char *mnu[] = {		
        "Menu",
        "EnterStr",
        "EnterTxt",
        "EnterCard",
        "EnterDate",
        "Info",        
        0
    };
    const char *ptr;
    int i;

    makeMenu(hdr, mnu);

    i = 0;
    while(42) {
        i = mnuRun(i);
        if(i < 0)
            break;
        ptr = mnu[i];
        switch (i) {
          case 0:
              RUN(tcab0025()); // PRI - Dialog - Menu
              break;
          case 1:
              RUN(tcab0026()); // PRI - Dialog - EnterStr
              break;
          case 2:
              //RUN(tcnl0007()); // PRI - Dialog - EnterTxt !!!!!!!!!!!!
              break;
          case 3:
              RUN(tcab0027()); // PRI - Dialog - EnterCard
              break;
          case 4:
              RUN(tcfa0001()); // PRI - Dialog - EnterDate
              break;
          case 5:
              RUN(tcab0028()); // PRI - Dialog - Info
              break;              
          default:
              break;
        }
    }
}

static void mnuPRI_DataBase(const char *hdr) {    // PRI - DataBase 
    const char *mnu[] = {    		
        "Table",
        "Record",
        "MultiRecord",
        "LinearSearch",
        "BinarySearch",        
        0
    };
    const char *ptr;
    int i;

    makeMenu(hdr, mnu);

    i = 0;
    while(42) {
        i = mnuRun(i);
        if(i < 0)
            break;
        ptr = mnu[i];
        switch (i) {
          case 0:
              RUN(tcab0029()); // PRI - DataBase - Table
              break;
          case 1:
              RUN(tcab0030()); // PRI - DataBase - Record
              break;
          case 2:
              RUN(tcab0031()); // PRI - DataBase - MutliRecord
              break;
          case 3:
              RUN(tcab0120()); // PRI - DataBase - LinearSearch
              break;
          case 4:
              RUN(tcab0121()); // PRI - DataBase - BinarySearch !!!!!!!!!!!!
              break;              
          default:
              break;
        }
    }
}

static void mnuPRI_Derivated(const char *hdr) {    // PRI - Derivated 
    const char *mnu[] = {
        "Combo",
        "Converter",        
        0
    };
    const char *ptr;
    int i;

    makeMenu(hdr, mnu);

    i = 0;
    while(42) {
        i = mnuRun(i);
        if(i < 0)
            break;
        ptr = mnu[i];
        switch (i) {
          case 0:
              RUN(tcab0032()); // PRI - Derivated - Combo
              break;
          case 1:
              RUN(tcab0033()); // PRI - Derivated - Converter
              break;              
          default:
              break;
        }
    }
}

static void mnuPRI(const char *hdr) { // PRI
    const char *mnu[] = {	
        "Format",
        "Standard",
        "Communication",
        "Dialog",
        "DataBase",
        "Derivated",
        0
    };
    const char *ptr;
    int i;

    makeMenu(hdr, mnu);

    i = 0;
    while(42) {
        i = mnuRun(i);
        if(i < 0)
            break;
        ptr = mnu[i];
        switch (i) {
          case 0:
        	  RUN(tcab0024()); // PRI - Format
              break;
          case 1:
              mnuPRI_Standard(ptr); // PRI - Standard
              break;
          case 2:
        	  RUN(tcab0059()); // PRI - Communication
              break;
          case 3:
              mnuPRI_Dialog(ptr); // PRI - Dialog
              break;
          case 4:
              mnuPRI_DataBase(ptr); // PRI - DataBase
              break;
          case 5:
        	  mnuPRI_Derivated(ptr); // PRI - Derivated
              break;
          default:
              break;
        }
    }
}

void treatMenu(void) {
    const char *hdr = "MAIN MENU";
    const char *mnu[] = {
        "SystemGroup",
        "PrimitiveGroup",
        0
    };
    const char *ptr;
    int i;

    hmiStart();
    makeMenu(hdr, mnu);

    i = 0;
    while(42) {
        i = mnuRun(i);
        if(i < 0)
            break;
        ptr = mnu[i];
        switch (i) {
          case 0:
              mnuSYS(ptr);
              break;
          case 1:
              mnuPRI(ptr);
              break;
          default:
              break;
        }
    }
    hmiStop();
}
