/** \file
 * Unitary test case tcab0123.
 * Index search
 * \sa
 *  - idxDel()
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/tst/ab/tcab0123.c $
 *
 * $Id: tcab0123.c 2609 2010-04-07 07:46:19Z abarantsev $
 */

#include <string.h>
#include "pri.h"
#include "tst.h"

typedef struct sPerson {
    char first[prtW + 1];
    char last[prtW + 1];
} tPerson;

static tPerson uta[] = {
    {"Kaveh", "NEMATIPUR"},
    {"Leonce", "MEKINDA"},
    {"Lamine", "NDIAYE"},
    {"Ahmad", "JAMAL"},
    {"Talin", "THOMAS"},
    {"Nooshin", "LOTFDAR"},
    {"Melineh", "TABIBZADEH"},
    {"Feresteh", "ZABIHI"},
    {"Tamas", "SARAI"},
    {"Tamas", "DANI"},
    {"Gabor", "SAROSPATAKI"},
    {"Bruno", "KASSOVIC"},
    {"Ibrahima", "TRAORE"},
    {"Mohamed", "LACHGUER"},
    {"Benedek", "BOTLIK"},
    {"Pal", "SURANYI"},
    {"Robert", "FEHER"},
    {"Enguerran", "BAZONCOURT"},
    {"Fabrice", "SCHUWEY"},
    {"Ibrahim", "EL KHOURY"},
    {"Matta", "MATTA"},
    {"Fadi", "TARABAY"},
    {"Johny", "TORBEY"},
    {"Mahdi", "ESNAASHARI"},
    {"Babak", "TOOTOONCHI"},
    {"Abdelatif", "BOUMHALI"},
    {"Colette", "MERIAN"},
    {"Puneet", "AGRAWAL"},
    {"Sapana", "JAISWAL"},
    {"Erosha", "RATHNAYAKE"},
    {"Kamruzzaman", "SHARDER"},
    {"Nirmala", "SRINIVASAN"},
    {"Rajesh", "BHOOPATHY"},
    {"Jai Sai", "KRISHNA"},
    {"Sathish", "KUMAR"},
    {"Murugan", "DEVARAJ"},
    {"Durai", "MURUGAN"},
    {"Aswini", "SRINIVASAN"},
    {"Deepa", "SAMBANDAM"},
    {"Roman", "MAHRER"},
    {"Bruno", "WIDMANN"},
    {"Christophe", "LEHRKINDER"},
    {"Tadej", "GREGORCIC"},
    {"David", "JAPARIDZE"},
    {"Irakli", "KHOKHIASHVILI"},
    {"Nick", "GUDUSHAURI"},
    {"Ashoka", "LIYANAGE"},
    {"Puvirajan", "JEEVANANDAM"},
    {"Vengata", "SUBRAMANIYAN"},
    {"Alireza", "PAYAMI"},
    {"Firas", "ABU-SEIR"},
    {"Dhanya", "ABDUL LAISE"},
    {"Budi", "HERMANSYAH"},
    {"Iqbal", "CHALID"},
    {"Fouad", "SELMANE"},
    {"Song Wee", "GAN"},
    {"Inna", "KOUZMINA"},
    {"Oleg", "LYTOVCHENKO"},
    {"Amaury", "ROBERT DE RANCHER"},
    {"Jerome", "CUSTODIO"},
    {"Jeffrey", "QUEVEDO"},
    {"Amina", "CHOUISAINOVA-TATSI"},
    {"Panagiotis", "VASILOPOULOS"},
    {"Ekta", "RUSTAGI"},
    {"Himani", "GUPTA"},
    {"Qasim", "SHAIKH"},
    {"Rostislav", "KUPRIENKO"},
    {"Alexander", "ZAKUSILO"},
    {"Dmitry", "ROMANYAK"},
    {"Jean Philippe", "DUBOIS"},
    {"Mark", "LITOV"},
    {"Gordana", "JOVANOVIC"},
    {"Milos", "LAZAREVIC"},
    {"Sasa", "MISIC"},
    {"Mary Kunjumary", "RAJU"},
    {"Jay", "KUMAR"},
    {"Ali", "MEHRPOOR"},
    {"Shahab", "YOUSEFI"},
    {"Maziyar", "MAVVAJ"},
    {"Anahita", "NAGHILOU"},
    {"Bindia", "THOMAS"},
    {"Nikolay", "SOBOLEV"},
    {"Naima", "NAKABI"},
    {"Ranel", "RIVERO"},
    {"Janairo", "JIAO"},
    {"Vladimir", "ILIJEVSKI"},
    {"Radomir", "SUMIC"},
    {"Vincent", "POLLAERT"},
    {"Marvin", "San PEDRO"},
    {"Eric", "KOH"},
    {"Hamdan", "ZAINAL"},
    {"Archie", "OLA"},
    {"Milan", "STANEK"},
    {"Martin", "KUBECEK"},
    {"Radek", "STASTNY"},
    {"Petr", "LHOTSKY"},
    {"Tomas", "PARIZEK"},
    {"Karrel", "DELA CRUZ"},
    {"John Jeferson", "ELEMIA"},
    {"Jennifer Rose", "LAPARA"},
    {"Mark Allen", "PAMINTUAN"},
    {"Jaime", "TY"},
    {"Roman Angelo", "TRIA"},
    {"Emir", "MERCADO"},
    {"Natalie", "KILSHTEIN"},
    {"Itamar", "LASK"},
    {"Dmitry", "ZINGER"},
    {"Arie", "ABRAMOVITCH"},
    {"Zvi", "RAM"},
    {"Branislav", "TODOSIJEVIC"},
    {"Franc", "HORVAT"},
    {"Zdenko", "MEZGEC"},
    {"Kagan", "SENYUZ"},
    {"Paola", "BELIBI"},
    {"Friedrich", "HAEUPL"},
    {"Samy", "EL SAGHIR"},
    {"Edmund", "PIETERSE"},
    {"Slavina", "SOBADZHIEVA"},
    {"Paul", "WAUDBY"},
    {"Dragan", "BUGARSKI"},
    {"Dan", "MIHALACHE"},
    {"Viktor", "NAGORNY"},
    {"Bahare", "GHADERI"},
    {"Konstantin", "CHERNYAK"},
    {"Alexander", "BOCHAGOV"},
    {"Alexei", "BOROVKOV"},
    {"Konstantin", "SOLOVIEV"},
    {"Mikhail", "GUSARENKO"},
    {"Oleg", "ZAITSEV"},
    {"Dmitry", "STEFANENKO"},
    {"Victor", "PESKOVATSKOV"},
    {"Pavel", "CHERNOV"},
    {"Alexander", "KHREBTOVICH"},
    {"Nazarena", "BUHAT"},
    {"Aesel James", "LORETO"},
    {"Violet", "DY"},
    {"Rose Anne", "BAUTISTA"},
    {"Kresimir", "ALERIC"},
    {"Conrad", "MICALLEF"},
    {"Katarina", "ROSKAR"},
    {"Tara", "FLYNN"},
    {"Dabisa", "KELAVA"},
    {"Paul", "DEBORO"},
    {"Brijesh", "PATEL"},
    {"Thanasis", "SOLTADIS"},
    {"Irfan", "FAUZI"},
    {"Johannes", "FILANDOW"},
    {"Nofalia", ""},
    {"Apiwat", "SAENGDEEJING"},
    {"Edwin", "GARCIA"},
    {"Rico", "SANTOS"},
    {"Harsasongko", "BAYUADJI"},
    {"David", "PERIANG HENDRA"},
    {"Scott", "GAO"},
    {"Adrian", "LO"},
    {"Pua", "ENG LE"},
    {"Daniel", "LAI KWONG LUN"},
    {"Rully", "BUDHIANA"},
    {"Sylvain", "TCHAMOKOUEN"},
    {"Stephane", "KANA"},
    {"Eiad", "TAHA"},
    {"Francis Michael", "DELAINO"},
    {"Berlene", "MORES"},
    {"Alexander", "SARMATOV"},
    {"Dmitry", "LUKASEVICH"},
    {"Kevin Njenga", "WAINAINA"},
    {"Alexander Mwau", "MUEMA"},
    {"Richard Ndambuki", "MUIA"},
    {"James Kithome", "KILALI"},
    {"Catherine Wanjiku", "KIGURU"},
    {"Carol", "NJAU"},
    {"Emmanuel", "SEKA"},
    {"Roslyn Conda", "MINA"},
    {"Mark Kenneth", "DELA CRUZ"},
    {"Brian", "RUPTASH"},
    {"Joaquim", "ROVIRA"},
    {"Ian", "BUTTIMER"},
    {"Martin", "CHITUNDU"},
    {"Frank", "SIKAZWE"},
    {"Lionel", "TOPART"},
    {"Jean Jacques", "ARNAUD"},
    {"Jerome", "GRANDEMENGE"},
    {"Denys", "DOBRIVECHER"},
    {"Anton", "KHOMENKO"},
    {"Yury", "FOMENKO"},
    {"Dmitry", "OSTROPICKY"},
    {"Richard", "CAMATO"},
    {"Myleen Joy", "CASTRO"},
    {"Manuel", "ADEA"},
    {"Li Chun", "LIANG"},
    {"David", "HSU"},
    {"Thomas", "KRONER"},
    {"Adelino", "JUNIOR"},
    {"Milot", "SHALA"},
    {"Oliver", "TAYLOR"},
    {"Mary Ann Sharon", "AGDA"},
    {"Tarik", "DAHNI"},
    {"Said", "OURACHE"},
    {"Zouhair", "CHAHID"},
    {"Mouhamadou", "ABIBALLAH GUEYE"},
    {"Raphael", "WALY SENGHOR"},
    {"Jeff Paolo", "ORTIGA"},
    {"Sheila", "MUSA"},
    {"Lea Jean ", "ONG"},
    {"Elvis", "QUITO"},
    {"Marie Grace", "PAGUIRIGAN"},
    {"Anas", "MONAFFAL"},
    {"David", "SITORUS"},
    {"Agustian", "ROBBYTUA"},
    {"Andry Susanto", "HADI"},
    {"Luffi", "ANDRAWIGUNA"},
    {"Rahendra Anda", "SETIA"},
    {"Fronita", "FRONITA"},
    {"Chuah", "YEN HARN"},
    {"Yang", "MING HSIANG"},
    {"Erizal", "ERIZAL"},
    {"Wara Asri", "WIDYAWATI"},
    {"Ade", "IRWANSIAH"},
    {"Delhy", "MUSENGA"},
    {"Guillaume", "LARDILLIER"},
    {"Osama", "MUQIL"},
    {"Muhammad", "SAQIB"},
    {"Konstantinas", "ILJINAS"},
    {"Revaz", "TATISHVILI"},
    {"George", "GORELISHVILI"},
    {"Valentin", "JOJUA"},
    {"Revielle", "PILLERVA"},
    {"Maria Kristina", "BORLONGAN"},
    {"Ahmed", "CHKIOUA"},
    {"Richard", "CANAPI"},
    {"Jason", "CERBAS"},
    {"Reina Mora", "GONZALEZ"},
    {"Astrix Plasencia", "CASTRO"},
    {"Ariel Rodriguez", "ALVAREZ"},
    {"Seiji", "HAYASHI"},
    {"Henry", "SENG"},
    {"Tara", "WALKER"},
    {"Jacques", "ARNAUD"},
    {"Colin", "MACKENZIE"},
    {"Christine", "WEI"},
    {"Vaishali", "DHANDHUKIA"},
    {"Qiang", "ZHANG"},
    {"Gordon", "ZIMMERMAN"},
    {"Justin", "ALLEN"},
    {"Charles", "CIVILS"},
    {"Amit", "VARMA"},
    {"Benson", "KWOK"},
    {"David", "KRISS"},
    {"Donghong", "HUNTER"},
    {"Johnie", "BASSON"},
    {"Darko", "LAZOVIC"},
    {"Lucjan", "FOIK"},
    {"Zbigniew", "TOMICA"},
    {"Rehan", "AHMED KHAN"},
    {"Mustufa", "AMIN SHAH"},
    {"Lilija", "ALECHNO"},
    {"Andrew", "LO"},
    {"Jack", "CHEUNG"},
    {"Zhi Rong", "GUO"},
    {"Paul", "LEE"},
    {"Andrew", "CHAN"},
    {"CL", "MAK"},
    {"Johnie", "BASSON"},
    {"Darko", "LAZOVIC"},
    {"Lucjan", "FOIK"},
    {"Zbigniew", "TOMICA"},
    {"", ""}
};

#define DIM 256
enum eFld {                     //multirecord fields
    fldNum,                     //record number
    fldFirst,                   //first name
    fldLast,                    //last name
    fldEnd                      //end sentinel
};

static tRecord rec;
static word fldLen[fldEnd] = {
    sizeof(word),
    prtW,
    prtW
};
static word fldMap[fldEnd];
static tTable tab;

static tContainer cnt;          //container built around rec

static int init(void) {
    int ret;
    word len;

    ret = dspLS(0, "init");
    CHECK(ret >= 0, lblKO);

    ret = recInit(&rec, 0, 0, fldEnd, fldLen, 0, fldMap);
    VERIFY(ret == (int) recSize(&rec));
    len = ret;

    ret = tabInit(&tab, 0, 0, len, DIM, 0);
    VERIFY(ret == (int) tabSize(&tab));

    ret = cntInit(&cnt, 'r', &rec);
    CHECK(ret >= 0, lblKO);

    ret = 1;

    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    return ret;
}

static int generate(void) {
    int ret;
    word idx;
    char buf[256];

    ret = dspLS(0, "generate...");
    CHECK(ret >= 0, lblKO);

    for (idx = 0; idx < tabDim(&tab); idx++) {
        if(uta[idx].first[0] == 0)
            break;

        num2dec(buf, idx, 4);
        ret = dspLS(1, buf);
        CHECK(ret >= 0, lblKO);

        ret = dspLS(2, uta[idx].first);
        CHECK(ret >= 0, lblKO);

        ret = dspLS(3, uta[idx].last);
        CHECK(ret >= 0, lblKO);

        recMove(&rec, &tab, idx);

        ret = recPutWord(&rec, fldNum, idx);
        CHECK(ret >= 0, lblKO);

        ret = recPutStr(&rec, fldFirst, uta[idx].first);
        CHECK(ret >= 0, lblKO);

        ret = recPutStr(&rec, fldLast, uta[idx].last);
        CHECK(ret >= 0, lblKO);
    }
    ret = dspLS(1, "Done.");
    CHECK(ret >= 0, lblKO);

    ret = idx;
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    dspClear();
    return ret;
}

static int buildIndexFL(tIndex * idx, card ofs, word dim) {
    int ret;
    word pos;
    char buf[256];
    static word num[DIM];
    static char key[DIM * (prtW + 1)];
    tLocator arg[3];            //arguments for combo
    tCombo cmbFL;               //concatenates First and Last names
    tContainer cntFL;

    VERIFY(idx);

    ret = dspLS(0, "buildIndex...");
    CHECK(ret >= 0, lblKO);

    arg[0].cnt = &cnt;
    arg[0].key = fldFirst;
    arg[1].cnt = &cnt;
    arg[1].key = fldLast;
    arg[2].cnt = 0;
    arg[2].key = 0;
    ret = cmbInit(&cmbFL, oprCat, arg, prtW + 1);
    CHECK(ret >= 0, lblKO);

    ret = cntInit(&cntFL, 'x', &cmbFL);
    CHECK(ret >= 0, lblKO);

    ret = idxInit(idx, 1, ofs, &tab, &rec, &cntFL, num, prtW + 1, key);
    CHECK(ret >= 0, lblKO);

    ret = idxReset(idx);
    CHECK(ret >= 0, lblKO);

    for (pos = 0; pos < dim; pos++) {
        num2dec(buf, pos, 4);
        ret = dspLS(1, buf);
        CHECK(ret >= 0, lblKO);
        recMove(&rec, &tab, pos);
        ret = idxIns(idx);
        CHECK(ret >= 0, lblKO);
    }
    ret = idxSave(idx);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "Done.");
    CHECK(ret >= 0, lblKO);
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    return ret;
}

static int buildIndexLF(tIndex * idx, card ofs, word dim) {
    int ret;
    word pos;
    char buf[256];
    static word num[DIM];
    static char key[DIM * (prtW + 1)];
    tLocator arg[3];            //arguments for combo
    tCombo cmbLF;               //concatenates Last and First names
    tContainer cntLF;

    VERIFY(idx);

    ret = dspLS(0, "buildIndex...");
    CHECK(ret >= 0, lblKO);

    arg[0].cnt = &cnt;
    arg[0].key = fldLast;
    arg[1].cnt = &cnt;
    arg[1].key = fldFirst;
    arg[2].cnt = 0;
    arg[2].key = 0;
    ret = cmbInit(&cmbLF, oprCat, arg, prtW + 1);
    CHECK(ret >= 0, lblKO);

    ret = cntInit(&cntLF, 'x', &cmbLF);
    CHECK(ret >= 0, lblKO);

    ret = idxInit(idx, 1, ofs, &tab, &rec, &cntLF, num, prtW + 1, key);
    CHECK(ret >= 0, lblKO);

    ret = idxReset(idx);
    CHECK(ret >= 0, lblKO);

    for (pos = 0; pos < dim; pos++) {
        num2dec(buf, pos, 4);
        ret = dspLS(1, buf);
        CHECK(ret >= 0, lblKO);
        recMove(&rec, &tab, pos);
        ret = idxIns(idx);
        CHECK(ret >= 0, lblKO);
    }
    ret = idxSave(idx);
    CHECK(ret >= 0, lblKO);

    ret = dspLS(1, "Done.");
    CHECK(ret >= 0, lblKO);
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    return ret;
}

static int prtRec(const tIndex * idx) {
    int ret;
    word num;
    char buf[prtW + 1];
    char tmp[prtW + 1];

    strcpy(buf, "Record ");
    num2dec(tmp, idx->cur, 0);
    strcat(buf, tmp);
    strcat(buf, ":");
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = recGetWord(&rec, fldNum, num);
    CHECK(ret >= 0, lblKO);
    strcpy(buf, "Num: ");
    num2dec(tmp, num, 0);
    strcat(buf, tmp);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = recGet(&rec, fldFirst, buf, prtW + 1);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = recGet(&rec, fldLast, buf, prtW + 1);
    CHECK(ret >= 0, lblKO);
    ret = prtS(buf);
    CHECK(ret >= 0, lblKO);

    ret = prtS(" === === ===");
    CHECK(ret >= 0, lblKO);

    ret = 1;

    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    return ret;
}

static int delete(tIndex * idx) {
    int ret;

    VERIFY(idx);

    ret = prtS(" === Delete: ===");
    CHECK(ret >= 0, lblKO);

    idxSet(idx, 10);            //choose some row inside to delete
    ret = prtRec(idx);
    CHECK(ret >= 0, lblKO);

    ret = idxDel(idx);          //delete insider row
    CHECK(ret >= 0, lblKO);

    idxSet(idx, 0);             //choose first row
    ret = prtRec(idx);
    CHECK(ret >= 0, lblKO);

    ret = idxDel(idx);          //delete from the beginning of the list
    CHECK(ret >= 0, lblKO);

    ret = sizeof(uta) / sizeof(tPerson) - 1;    //number of persons in the list
    ret -= 2;                   //two of them are already deleted
    idxSet(idx, (word) (ret - 1));  //move to the end of list
    ret = prtRec(idx);
    CHECK(ret >= 0, lblKO);

    ret = idxDel(idx);          //delete the last row
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    return ret;
}

static int browse(tIndex * idx) {
    int ret;
    word pos;

    VERIFY(idx);

    dspClear();
    ret = dspLS(0, "Printing...");
    CHECK(ret >= 0, lblKO);

    ret = prtS(" === Browse ===");
    CHECK(ret >= 0, lblKO);

    for (pos = 0; pos < idxDim(idx); pos++) {
        ret = idxSet(idx, pos);
        CHECK(ret >= 0, lblKO);

        ret = prtRec(idx);
        CHECK(ret >= 0, lblKO);
    }

    ret = 1;
    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    ret = -1;
  lblEnd:
    tmrPause(1);
    dspClear();
    return ret;
}

void tcab0123(void) {
    int ret;
    byte vol[256];
    tIndex idx;
    card ofs;
    word dim;

    nvmStart();
    dbaSetPtr(vol);

    ret = dspStart();
    CHECK(ret >= 0, lblKO);
    ret = prtStart();
    CHECK(ret >= 0, lblKO);

    ret = init();
    CHECK(ret >= 0, lblKO);
    ret = generate();
    CHECK(ret >= 0, lblKO);
    dim = ret;

    ofs = 0;
    ret = buildIndexFL(&idx, ofs, dim);
    trcFN("buildIndexFL ret=%d", ret);
    CHECK(ret >= 0, lblKO);
    ofs += ret;
    ret = browse(&idx);
    trcFN("browse ret=%d", ret);
    CHECK(ret >= 0, lblKO);

    ret = buildIndexLF(&idx, ofs, dim);
    trcFN("buildIndexLF ret=%d", ret);
    CHECK(ret >= 0, lblKO);
    ofs += ret;
    ret = browse(&idx);
    trcFN("browse ret=%d", ret);
    CHECK(ret >= 0, lblKO);
    ret = delete(&idx);
    trcFN("delete ret=%d", ret);
    CHECK(ret >= 0, lblKO);
    ofs += ret;
    ret = browse(&idx);
    trcFN("browse ret=%d", ret);
    CHECK(ret >= 0, lblKO);

    goto lblEnd;
  lblKO:
    dspLS(0, "KO");
    tmrPause(3);
  lblEnd:
    prtStop();
    dspStop();
}
