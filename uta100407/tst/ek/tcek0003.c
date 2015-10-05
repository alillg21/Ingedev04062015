#include "sdk30.h"
#include "oem.h"
#include "sys.h"
//#include "hid.h"

const KEY tabkey = {
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {'a', 'A', ',', '?'},
    {'b', 'B', 'b', 'B',},
    {'c', 'C', 'c', 'C'},
    {'d', 'D', 'd', 'D'},
    {'e', 'E', 'e', 'E'},
    {'f', 'F', 'f', 'F'},
    {'g', 'G', 'g', 'G'},
    {'h', 'H', 'h', 'H'},
    {'i', 'I', 'i', 'I'},
    {'j', 'J', 'j', 'J'},
    {'k', 'K', 'k', 'K'},
    {'l', 'L', 'l', 'L'},
    {0, 0},
    {'n', 'N'},
    {'o', 'O'},
    {'p', 'P'},
    {'a', 'A'},
    {'r', 'R'},
    {'s', 'S'},
    {'t', 'T'},
    {'u', 'U'},
    {'v', 'V'},
    {'z', 'Z'},
    {'x', 'X'},
    {'y', 'Y'},
    {'w', 'W'},
    {'&', '1'},
    {'é', '2'},
    {'"', '3'},
    {' ', '4'},
    {'(', '5'},
    {'-', '6'},
    {'è', '7'},
    {'_', '8'},
    {'ç', '9'},
    {'à', '0'},
    {T_VAL, T_VAL},
    {0x17, 0x17},               //esc
    {0x18, 0x18},               //corr
    {0, 0},                     // TAB
    {' ', ' '},
    {0, 0},
    {0, 0},
    {0, 0},
    {'$', '$'},
    {0, 0},
    {'*', '*'},
    {'m', 'M'},
    {0, 0},
    {0, 0},
    {';', '.'},
    {0, 0},
    {0, 0},
    {0, 0},                     //MAJ
    {T_SK1, T_SK1},
    {T_SK2, T_SK2},
    {T_SK3, T_SK3},
    {T_SK4, T_SK4},
    {0, 0},                     //F5
    {0, 0},                     //F6
    {0, 0},                     //F7
    {0, 0},                     //F8
    {0, 0},                     //F9
    {0, 0},                     //F10
    {0, 0},                     //F11
    {0, 0},                     //F12
    {0, 0},                     //Imp ecran
    {0, 0},                     //Defil
    {0, 0},                     // Pause
    {0, 0},
    {'F', 'F'},                 //home
    {0, 0},
    {0, 0},
    {0, 0},
    {0, 0},
    {T_SKVAL, T_SKVAL},         //Fl droit
    {T_SKCLEAR, T_SKCLEAR},     //Fl gauche
    {T_SKBAS, T_SKBAS},         //Fl bas
    {T_SKHAUT, T_SKHAUT},       //Fl haut
    {0, 0},
    {'/', ':'},                 ///
    {'*', '*'},                 //*
    {'-', '-'},                 //Moins
    {'+', '+'},                 //Plus
    {T_VAL, T_VAL},             //Enter
    {'1', '1'},
    {'2', '2'},
    {'3', '3'},
    {'4', '4'},
    {'5', '5'},
    {'6', '6'},
    {'7', '7'},
    {'8', '8'},
    {'9', '9'},
    {'0', '0'},
    {'.', '.'},
    {'<', '>'}
};

void tcek0003(void) {
    FILE *fdkeybusb, *fdmouse, *fdhost;

#ifndef _SIMULPC_
    FILE *fdfunc;
#endif
    unsigned int event;
    unsigned char tab[4] = { 0, 0, 0, 0 };
    unsigned char keys[8];
    unsigned int receive = 0;
    unsigned char buffer[2048];
    byte key;
    int x = 8, y = 8;
    int Events = MOUSE | KEYBUSB | COM6;

    trcS("#BEG tcek0003()#\n");

#ifndef _SIMULPC_
    Events |= COM5;
    StartRetroEclairage(0, 1, 0xFFFF);
#endif

    //fopen("KEYBOARD","r");
    dspStart();
    kbdStart(1);
    cnvStart();

    fdhost = fopen("COM6", "rw");

#ifndef _SIMULPC_
    fdfunc = fopen("COM5", "rw");
#endif

    fdkeybusb = fopen("KEYBUSB", "r");
    fdmouse = fopen("MOUSE", "r");

    // mask event with SPECIAL_KEY for have an event when a special key will pressed.
    mask_event(fdkeybusb, SPECIAL_KEY | COM_REC_NOT_EMPTY);
#ifndef _SIMULPC_
    mask_event(fdfunc, COM_REC_NOT_EMPTY);
#endif

    startupcv20(8, 16);

    trcS("LOOP USB\n");

    // Define a new map of the keyboard
    fioctl(0x8002, &tabkey[0][0], fdkeybusb);

    StateHeader(0);
    _clrscr();

    while(1) {
        event = ttestall(Events, 100);
        if(event & MOUSE) {
            cnvBar(x, y, x + 4, y + 4, 0);
            cnvDraw();

            // get the informations of the mouse
            fioctl(GET_MOUSE, tab, fdmouse);
            trcFN("BUT = %d", tab[0]);
            trcFN("  WH = %d", tab[1]);
            trcFN("  DX = %d", tab[3]);
            trcFN("  DY = %d\n", tab[2]);

            if(tab[1] > 200) {
                if(x >= 9)
                    x -= 8;
            } else if(tab[1] < 100) {
                if(x <= 127 - 8)
                    x += 8;
            }
            if(tab[2] > 200) {
                if(y >= 9)
                    y -= 8;
            } else if(tab[2] < 100) {
                if(y <= 63 - 8)
                    y += 8;
            }
            cnvBar(x, y, x + 4, y + 4, 1);
            cnvDraw();

        } else if(event & KEYBUSB) {
            // read the keys
            fread(tab, 1, 1, fdkeybusb);
            // read the state of the special keys ( and all the keys)
            fioctl(GET_KEYS, keys, fdkeybusb);
            trcS("VAL = ");
            trcFN("0x%02X", tab[0]);
            trcS("  SP = ");
            trcFN("0x%02X", keys[0]);
            trcFN("  [%c]", tab[0]);
            trcS("\n");
        } else if(event & COM6) {
            receive += fread(buffer, 1, sizeof(buffer), fdhost);
            trcFN("V = 0x%02X", buffer[0]);
            trcFN("  R = %d\n", receive);
            fwrite(buffer, 1, receive, fdhost);
        }
#ifndef _SIMULPC_
        else if(event & COM5) {
            receive += fread(buffer, 1, sizeof(buffer), fdfunc);
            trcS("value receive : ");
            trcBN(&buffer[0], 1);
            trcFN("  Nb rec = %d", receive);
            fwrite(buffer, 1, receive, fdfunc);
        }
#endif
        else {
            key = kbdKey();
            CHECK(key != kbdANN, lblEND);
        }
    }

  lblEND:
    StateHeader(1);
    cnvStop();
    kbdStop();
    dspStop();
    fclose(fdhost);
    fclose(fdkeybusb);
    fclose(fdmouse);
    trcS("#END tcek0003()#\n");
#ifndef _SIMULPC_
    fclose(fdfunc);
#endif
}
