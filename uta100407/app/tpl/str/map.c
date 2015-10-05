#include <string.h>
#include "map.h"

/** Find a key subspace corresponding to a given key
 * \param  key (I) Key to be located. 
 * \return
 *  - starting sentinel of a subspace that the key belongs to
 *  - negative if the input key is out of space
 * 
 * Used by other map functions to identify a data structure which the
 * data element belongs to
 */
static int begKey(word key){ //find starting sentinel of key subspace
    if(isSorted(appBeg,key,appEnd)) return appBeg; //application parameters record
    if(isSorted(mnuBeg,key,mnuEnd)) return mnuBeg; //menu tree
    if(isSorted(msgBeg,key,msgEnd)) return msgBeg; //screen messages
    if(isSorted(rptBeg,key,rptEnd)) return rptBeg; //receipt formatting strings
    if(isSorted(scrBeg,key,scrEnd)) return scrBeg; //user screen structure
    if(isSorted(regBeg,key,regEnd)) return regBeg; //registry record
    if(isSorted(traBeg,key,traEnd)) return traBeg; //transaction related data situated in volatile memory
    return -1;
}

/** Find relative position of a key within its subspace
 * \param  key (I) Key to be adjusted. 
 * \return - relative position of a key within its subspace
 * \pre key should be a valid key of a data element (inside key enum)
 * Used by other map functions to find field numbder within a record,
 * or row number within a table, etc
 */
word relKey(word key){ //return relative key position in key subspace
    int beg;
    beg= begKey(key);
    VERIFY(beg>0);
    VERIFY(beg<key);
    return key-beg-1;
}

/** Find a container corresponding to a given key
 * \param  key (I) Key whose contatiner should be identified. 
 * \return pointer to the container that contains the key
 * \pre key should be a valid key of a data element (inside key enum)
 * A given key can belong to a record, a table, a converter or a combo.
 * For each of these data structures a surrounding container is constructed.
 * The derivated elements (combos and converters) use these containers
 * to perform operations.
 */
const tContainer *getContainer(word key){
    int beg;
    beg= begKey(key);
    VERIFY(beg>0);
    VERIFY(beg<key);
    switch(beg){
        case appBeg: return &cntApp;
        case mnuBeg: return &cntMnu;
        case msgBeg: return &cntMsg;
        case rptBeg: return &cntRpt;
        case scrBeg: return &cntScr;
        case regBeg: return &cntReg;
        case traBeg: return &cntTra;
        default: break;
    }
    return 0;
}

/** Call the constructors for all UTA data base structures:
 * - tables
 * - records
 * - multirecords
 * - converters
 * - combos
 * - containers
 * - volatile record
 * - search descriptors
 * - indices and keys
 * \return
 * - full size of DFS in bytes occupied by UTA data base
 * - negative if failure
 * 
 * The data elements are saved in DFS in the same order as they are described
 * in dbs.xml file.
 * 
 * In this function the descriptors are filled.
 * All the operations are performed in RAM; it is fast.
 * Usually this function is called during POWER ON event treatment.
 * 
 * The size of each data structure is traced.
 */
int mapInit(void){
    int ret;
    byte sec;
    card ofs;
    word len;
    card tot= 0;

    trcS("mapInit: Beg\n");
    //page0000.nvm
    ofs= 0;
    sec= 0x00;

    //key subspace:app
    ret= recInit(&recApp,sec,ofs,DIM(appBeg,appEnd),appLen,appMap);
    VERIFY(ret==(int)recSize(&recApp));
    trcFN("app record size: %d\n",ret);
    ofs+= ret;
    ret= cntInit(&cntApp,'r',&recApp); CHECK(ret>=0,lblKO);

    //key subspace:mnu
    ret= tabInit(&tabMnu,sec,ofs,lenMnu,DIM(mnuBeg,mnuEnd)
);
    VERIFY(ret==(int)tabSize(&tabMnu));
    trcFN("mnu table size: %d\n",ret);
    ofs+= ret;
    ret= cntInit(&cntMnu,'t',&tabMnu); VERIFY(ret>=0);

    //key subspace:msg
    ret= tabInit(&tabMsg,sec,ofs,dspW,DIM(msgBeg,msgEnd)
);
    VERIFY(ret==(int)tabSize(&tabMsg));
    trcFN("msg table size: %d\n",ret);
    ofs+= ret;
    ret= cntInit(&cntMsg,'t',&tabMsg); VERIFY(ret>=0);

    //key subspace:rpt
    ret= tabInit(&tabRpt,sec,ofs,prtW,DIM(rptBeg,rptEnd)
);
    VERIFY(ret==(int)tabSize(&tabRpt));
    trcFN("rpt table size: %d\n",ret);
    ofs+= ret;
    ret= cntInit(&cntRpt,'t',&tabRpt); VERIFY(ret>=0);

    //key subspace:scr
    ret= recInit(&recScr,sec,ofs,DIM(scrBeg,scrEnd),scrLen,scrMap);
    VERIFY(ret==(int)recSize(&recScr));
    trcFN("scr record size: %d\n",ret);
    len= ret; //len is used to initiate table

    ret= tabInit(&tabScr,sec,ofs,len,DIM(infBeg,infEnd));
    VERIFY(ret==(int)tabSize(&tabScr));
    trcFN("scr table size: %d\n",ret);
    ofs+= ret;
    ret= cntInit(&cntScr,'r',&recScr); CHECK(ret>=0,lblKO);

    //key subspace:reg
    ret= recInit(&recReg,sec,ofs,DIM(regBeg,regEnd),regLen,regMap);
    VERIFY(ret==(int)recSize(&recReg));
    trcFN("reg record size: %d\n",ret);
    ofs+= ret;
    ret= cntInit(&cntReg,'r',&recReg); CHECK(ret>=0,lblKO);

    trcFN("section page0000.nvm size: %d\n",ofs);
    tot+= ofs;

    //volatile memory
    dbaSetPtr(&vol);
    ofs= 0;
    sec= 0xFF;

    //key subspace:tra
    ret= recInit(&recTra,sec,ofs,DIM(traBeg,traEnd),traLen,traMap);
    VERIFY(ret==(int)recSize(&recTra));
    trcFN("tra record size: %d\n",ret);
    ofs+= ret;
    ret= cntInit(&cntTra,'r',&recTra); CHECK(ret>=0,lblKO);


    trcFN("total DFS size: %d\n",tot);
    ret= tot;
    goto lblEnd;
lblKO:
    trcErr(ret);
    ret= -1;
lblEnd:
    trcFN("mapInit: ret=%d\n",ret);
    return ret;
}

/** Fill a data structure by zeroes.
 * \param  key (I) Starting sentinel of the data structure to be reset.
 * \return
 * - the size of data structure to be reset
 * - negative if failure
 * 
 * If the parameter is keyBeg (zero) all data structures are reset.
 * If the data structure is in DFS it can take some time.
 * Usually volatile record is reset before any event treatment.
 */
int mapReset(word key){
    int ret= 0;
    int beg;
    trcFN("mapReset: key=%d\n",key);
    if(key==keyBeg){
        ret= recReset(&recApp); CHECK(ret>0,lblKO);
        ret= tabReset(&tabMnu); CHECK(ret>0,lblKO);
        ret= tabReset(&tabMsg); CHECK(ret>0,lblKO);
        ret= tabReset(&tabRpt); CHECK(ret>0,lblKO);
        ret= tabReset(&tabScr); CHECK(ret>0,lblKO);
        ret= recReset(&recReg); CHECK(ret>0,lblKO);
        ret= recReset(&recTra); CHECK(ret>0,lblKO);
        goto lblEnd;
    }
    beg= begKey(key);
    if(beg < 0) beg= begKey((word)(key+1));
    switch(beg){
        case appBeg: ret= recReset(&recApp); break;
        case mnuBeg: ret= tabReset(&tabMnu); break;
        case msgBeg: ret= tabReset(&tabMsg); break;
        case rptBeg: ret= tabReset(&tabRpt); break;
        case scrBeg: ret= tabReset(&tabScr); break;
        case regBeg: ret= recReset(&recReg); break;
        case traBeg: ret= recReset(&recTra); break;
        default: ret= -1;
            VERIFY(ret>=0);
            break;
    }
    CHECK(ret>0,lblKO);
    goto lblEnd;
lblKO:
    trcErr(ret);
    ret= -1;
lblEnd:
    trcFN("mapReset: ret=%d\n",ret);
    return ret;
}

/** Retrieve a data element from a table.
 * \param  tab (I) Pointer to table descriptor.
 * \param  key (I) Index of a row within the table.
 * \param  ptr (O) Pointer to buffer where to write the data to.
 * \param  len (I) Buffer length.
 * \return
 * - size of the data element retrieved
 * - negative if failure
 * 
 * The parameter len ensures that the memory is not overwritten.
 * If it is too small the exception condition is arised.
 * Zero length is treated as table width.
 * This way of calling the function is not recommended.
 * 
 * \pre tab!=0
 * \pre ptr!=0
 * \sa
 *  - mapGet()
 *  - mapRecGet()
 */
static int mapTabGet(const tTable *tab,word key,void *ptr,word len){
    VERIFY(tab);
    VERIFY(ptr);
    if(!len) len= tabWdt(tab);
    VERIFY(len>=tabWdt(tab));
    if(len<tabWdt(tab)) return -1;
    return tabGet(tab,key,ptr,len);
}

/** Retrieve a data element from a record.
 * \param  rec (I) Pointer to record descriptor.
 * \param  key (I) Index of a field within the record.
 * \param  ptr (O) Pointer to buffer where to write the data to.
 * \param  len (I) Buffer length.
 * \return
 * - size of the data element retrieved
 * - negative if failure
 * 
 * The parameter len ensures that the memory is not overwritten.
 * If it is too small the exception condition is arised.
 * Zero length is treated as field length.
 * This way of calling the function is not recommended.
 * 
 * \pre rec!=0
 * \pre ptr!=0
 * \sa
 *  - mapGet()
 *  - mapTabGet()
 */
static int mapRecGet(const tRecord *rec,word key,void *ptr,word len){
    int ret;
    VERIFY(rec);
    VERIFY(ptr);
    if(!len) len= rec->len[key];
    VERIFY(len>=rec->len[key]);
    if(len<rec->len[key]) return -1;
    ret= recGet(rec,key,ptr,len);
    VERIFY(ret);
    return ret;
}

/** Retrieve a data element from a data structure.
 * \param  key (I) Index of a data element.
 * \param  ptr (O) Pointer to buffer where to write the data to.
 * \param  len (I) Buffer length.
 * \return
 * - size of the data element retrieved
 * - negative if failure
 * 
 * The parameter len ensures that the memory is not overwritten.
 * If it is too small the exception condition is arised.
 * Zero length is treated as field length.
 * This way of calling the function is not recommended.
 * 
 * \pre ptr!=0
 * \pre key belongs to the key space
 * 
 * At first the key subspace is identified that contains the data element.
 * Then the relative position of the key within the subspace is calculated.
 * Depending on the subspace the retrieval function is called for related descriptor.
 * \sa
 *  - mapTabGet()
 *  - mapRecGet()
 *  - mapPut()
 *  - mapMove()
 */
int mapGet(word key,void *ptr,word len){
    int beg;
    VERIFY(ptr);
    VERIFY(isSorted(keyBeg,key,keyEnd));

    beg= begKey(key);
    VERIFY(beg>0);
    key-= beg+1;
    switch(beg){
        case appBeg: return mapRecGet(&recApp,key,ptr,len);
        case mnuBeg: return mapTabGet(&tabMnu,key,ptr,len);
        case msgBeg: return mapTabGet(&tabMsg,key,ptr,len);
        case rptBeg: return mapTabGet(&tabRpt,key,ptr,len);
        case scrBeg: return mapRecGet(&recScr,key,ptr,len);
        case regBeg: return mapRecGet(&recReg,key,ptr,len);
        case traBeg: return mapRecGet(&recTra,key,ptr,len);
        default: break;
    }
    return -1;
}

/** Save a data element into a table of rows.
 * \param  tab (I) Pointer to table descriptor.
 * \param  key (I) Index of a row within the table.
 * \param  ptr (I) Pointer to buffer containing the data to be saved.
 * \param  len (I) Buffer length.
 * \return
 * - size of the data element saved
 * - negative if failure
 * 
 * \pre tab!=0
 * \pre ptr!=0
 * 
 * If len is to big the buffer is truncated to the size of data element.
 * \sa
 *  - mapPut()
 *  - mapRecPut()
 */
static int mapTabPut(tTable *tab,word key,const void *ptr,word len){
    VERIFY(ptr);
    //VERIFY(len<=tabWdt(tab));
    //if(len>tabWdt(tab)) return -1;
    if(len>tabWdt(tab)) len= tabWdt(tab);
    return tabPut(tab,key,ptr,len);
}

/** Save a data element into a record.
 * \param  rec (I) Pointer to record descriptor.
 * \param  key (I) Index of a field within the record.
 * \param  ptr (I) Pointer to buffer containing the data to be saved.
 * \param  len (I) Buffer length.
 * \return
 * - size of the data element saved
 * - negative if failure
 * 
 * \pre rec!=0
 * \pre ptr!=0
 * 
 * If len is to big the buffer is truncated to the size of data element.
 * \sa
 *  - mapPut()
 *  - mapTabPut()
 */
static int mapRecPut(tRecord *rec,word key,const void *ptr,word len){
    int ret;
    VERIFY(rec);
    VERIFY(ptr);
    //VERIFY(len<=rec->len[key]);
    //if(len>rec->len[key]) return -1;
    if(len>rec->len[key]) len= rec->len[key];
    ret= recPut(rec,key,ptr,len);
    return ret;
}

/** Save a data element into a data structure.
 * \param  key (I) Index of a data element.
 * \param  ptr (I) Pointer to buffer containing the data to be saved.
 * \param  len (I) Buffer length.
 * \return
 * - size of the data element saved
 * - negative if failure
 * 
 * \pre ptr!=0
 * \pre key belongs to the key space
 * At first the key subspace is identified that contains the data element.
 * Then the relative position of the key within the subspace is calculated.
 * Depending on the subspace the save function is called for related descriptor.
 * 
 * If len is to big the buffer is truncated to the size of data element.
 * \sa
 *  - mapTabPut()
 *  - mapRecPut()
 *  - mapGet()
 *  - mapMove()
 */
int mapPut(word key,const void *ptr,word len){
    int beg;
    VERIFY(ptr);
    VERIFY(isSorted(keyBeg,key,keyEnd));

    beg= begKey(key);
    VERIFY(beg>0);
    key-= beg+1;
    if(!len) len= strlen((char *)ptr);

    switch(beg){
        case appBeg: return mapRecPut(&recApp,key,ptr,len);
        case mnuBeg: return mapTabPut(&tabMnu,key,ptr,len);
        case msgBeg: return mapTabPut(&tabMsg,key,ptr,len);
        case rptBeg: return mapTabPut(&tabRpt,key,ptr,len);
        case scrBeg: return mapRecPut(&recScr,key,ptr,len);
        case regBeg: return mapRecPut(&recReg,key,ptr,len);
        case traBeg: return mapRecPut(&recTra,key,ptr,len);
        default: break;
    }
    return -1;
}

/** Move the cursor within a multirecord.
 * \param  key (I) Starting sentinel of the multirecord.
 * \param  idx (I) Location where to move cursor.
 * \return
 * - cursor position
 * - negative if failure
 * 
 * \pre key belongs to the key space
 * \pre idx is less than multirecord dimension
 * 
 * At first the multirecord key subspace is identified.
 * For this key subspace there are.
 *  - table descriptor, each row containing an instance of the multirecord
 *  - record descriptor that overlays a row in the table
 * 
 * The record descriptor is moved to point to the record with index idx
 * within the multirecord.
 * The first position has index 0.
 * 
 * It is a very fast operation, no DFS access is used.
 * \sa
 *  - mapGet()
 *  - mapPut()
 */
int mapMove(word key,word idx){
    int beg;
    tTable *tab;
    tRecord *rec;

    beg= begKey(key);
    if(beg < 0) beg= begKey((word)(key+1));
    VERIFY(beg>0);

    tab= 0;
    rec= 0;
    switch(beg){
        case scrBeg: tab= &tabScr; rec= &recScr; break;
        default: break;
    }
    VERIFY(tab);
    VERIFY(rec);
    return recMove(rec,tab,idx);
}

/** Enable/disable cache buffer for a data structure.
 * \param  key (I) Starting sentinel of a data structure.
 * \return
 * - the size of allocated cache buffer
 * - negative if failure
 * 
 * \pre cache buffer should be already allocated for the data structure
 * 
 * A cache buffer can be allocated for
 *  - a table: the size of cache buffer is equal to table size
 *  - a single record: the size of cache buffer is equal to record size
 *  - a multirecord: the size of cache buffer is equal to single record size
 * 
 * Cache buffer can be allocated for data residing in non-volatile memory to accelerate access to these data.
 * 
 * The cache buffer is allocated for a data structure if its attribute
 * uta:cache is set to "yes".
 * By default it is disabled.
 * 
 * If cache buffer is enabled all mapGet()/mapPut() calls operate with
 * the data in cache buffer at a related offset.
 * Otherwise the real read/write operations are performed.
 * 
 * To enable a cache buffer the function mapCache should be called
 * with a parameter key equal to the starting sentinel of the data structure, for example, mnuBeg.
 * 
 * To disable cache buffer, the negative value should be provided, for example, (-mnuBeg).
 * 
 * Cache buffers can be used in the following context:
 *  - for essentially read/only data structures like menu or message table
 *    it can be loaded into cache buffer during POWER ON event treatment using mapLoad function.
 *    After that mapPut operations will operate with this RAM copy of
 *    the data structure.
 *    Note that for dynamical menu management a call to mapSave() or mapFlush()
 *    can be needed if the menu is modified.
 *  - For multirecord saving like transaction log save all the fields
 *    in the current record can be saved at first into cache buffer.
 *    When the record is filled in RAM it the mapSave() function is called to perform
 *    write operation.
 *    The low performance write operation is called only once in this case instead of
 *    performing it for each field of the record.
 * 
 * \sa
 *  - mapLoad()
 *  - mapSave()
 *  - mapFlush()
 */
int mapCache(int key){
    int beg;
    word tmp;

    tmp= (word)((key>0) ? key : (-key));
    beg= begKey(tmp);
    if(beg < 0) beg= begKey((word)(tmp+1));
    VERIFY(beg>0);

    switch(beg){
        case mnuBeg:
            if(key>0)
                return tabCache(&tabMnu,mnuCache,sizeof(mnuCache));
            return tabCache(&tabMnu,0,0);
        case msgBeg:
            if(key>0)
                return tabCache(&tabMsg,msgCache,sizeof(msgCache));
            return tabCache(&tabMsg,0,0);
        case scrBeg:
            if(key>0)
                return recCache(&recScr,scrCache,sizeof(scrCache));
            return recCache(&recScr,0,0);

        default: break;
    }
    return -1;
}

/** Depending on the argument the function does:
 *  - For data structures where cache is enabled:
 *    load data from non-volatile memory to a cache buffer.
 *  - For indices: load index file into volatile memory.
 * \param  key (I) Starting sentinel of a data structure.
 * \return
 *  - the size of buffer
 *  - negative if failure
 * 
 * \pre cache buffer should be enabled for the data structure
 * 
 * \sa
 *  - mapFlush()
 *  - mapSave()
 *  - mapCache()
 *  - mapFind()
 *  - mapIns()
 *  - mapDel()
 */
int mapLoad(word key){
    int beg;

    beg= begKey(key);
    if(beg < 0) beg= begKey((word)(key+1));
    VERIFY(beg>0);

    switch(beg){
        case mnuBeg: return tabLoad(&tabMnu);
        case msgBeg: return tabLoad(&tabMsg);
        case scrBeg: return recLoad(&recScr);
        default: break;
    }
    return -1;
}

/** Depending on the argument the function does:
 *  - For data structures where cache is enabled:
 *    save all the data from a cache buffer to non-volatile memory.
 *  - For indices: save index from volatile memory into a file.
 * \param  key (I) Starting sentinel of a data structure.
 * \return
 *  - the size of buffer
 *  - negative if failure
 * 
 * \pre cache buffer should be enabled for the data structure
 * 
 * \sa
 *  - mapFlush()
 *  - mapLoad()
 *  - mapCache()
 *  - mapFind()
 *  - mapIns()
 *  - mapDel()
 */
int mapSave(word key){
    int beg;

    beg= begKey(key);
    if(beg < 0) beg= begKey((word)(key+1));
    VERIFY(beg>0);

    switch(beg){
        case mnuBeg: return tabSave(&tabMnu);
        case msgBeg: return tabSave(&tabMsg);
        case scrBeg: return recSave(&recScr);
        default: break;
    }
    return -1;
}

/** Save the modified data from a cache buffer to non-volatile memory.
 * \param  key (I) Starting sentinel of a data structure.
 * \return
 * - the size of allocated cache buffer
 * - negative if failure
 * 
 * \pre cache buffer should be enabled for the data structure
 * 
 * If the data structure was loaded into cache memory before calling mapFlush()
 * the operation has the same effect as mapSave() but it can be faster for data structures of big size.
 * 
 * If a single instance of multirecord is filled by some data mapSave() function
 * should be called to fill all the bytes in the record.
 * 
 * \sa
 *  - mapLoad()
 *  - mapSave()
 *  - mapCache()
 */
int mapFlush(word key){
    int beg;

    beg= begKey(key);
    if(beg < 0) beg= begKey((word)(key+1));
    VERIFY(beg>0);

    switch(beg){
        case mnuBeg: return tabFlush(&tabMnu);
        case msgBeg: return tabFlush(&tabMsg);
        case scrBeg: return recFlush(&recScr);
        default: break;
    }
    return -1;
}

/** Find a pattern pat according to search descriptor key.
 * \param  key (I) Starting sentinel of the search/index descriptor
 * \param  pat (I) pattern to look for
 * \param  beg (I) starting position for search
 * \param  end (I) ending position for search
 * \return
 * - the position of the record found
 * - negative if failure
 * 
 * Four types of search operations are implemented in UTA:
 * - linear: (uta:find/\@uta:how="linear") sequential search in a multirecord.
 *   All records are browsed starting from beg until to end.
 *   For each single record the pattern is compared with a data element
 *   described by uta:what attribute in uta:find tag.
 *   This method can be applied for any multirecord. It is the slowest one.
 * - binary: (uta:find/\@uta:how="binary") binary search in a multirecord.
 *   This method can be applied for multirecords sorted according to the uta:what key.
 *   It is faster than linear but it is limited to sorted multirecords only.
 * - index: (uta:len missing in uta:index descriptor) binary search in index array.
 *   An index is an array of row numbers sorted according to a key uta:what.
 *   It has the same speed as binary search but it requires resources:
 *   a buffer should be allocated in volatile and non-volatile memory
 *   This method can be applied for multirecords sorted according to the uta:what key.
 * - key: (non-zero uta:len in uta:index descriptor) binary search in key array.
 *   A key array contains not only the row numbers but also the key values.
 *   The search is performed in RAM buffer, only one record at the end is required
 *   It is the fastest method, but it requires more resources than index array.
 *   
 * 
 * \pre the key should reference an index
 * \pre pat!=0
 * \pre beg and end should be inside the range
 * 
 * \sa
 *  - mapIns()
 *  - mapDel()
 *  - mapLoad()
 *  - mapSave()
 */
int mapFind(word key,const char *pat,word beg,word end){
    switch(key){
        default: break;
    }
    return -1;
}

/** Insert the current position of a multirecord into the index.
 * \param  key (I) Starting sentinel of the index.
 * \return
 * - the position of the current record
 * - (-1) if failure
 * - (-2) if key is duplicated
 * 
 * \pre the key should reference an index
 * 
 * \sa
 *  - mapFind()
 *  - mapDel()
 */
int mapIns(word key){
    word rel;

    rel= relKey(key);
    switch(key){
        default: break;
    }
    return -1;
}

/** Remove the current position of a multirecord from the index.
 * \param  key (I) Starting sentinel of the index.
 * \return
 * - the position of the current record
 * - negative if failure
 * 
 * \pre the key should reference an index
 * 
 * \sa
 *  - mapFind()
 *  - mapIns()
 */
int mapDel(word key){
    word rel;

    rel= relKey(key);
    switch(key){
        default: break;
    }
    return -1;
}

/** Export a data element to a non-volatile data section.
 * \param  key (I) Valid non-deriveted data element key.
 * \param  sec (I) Data section where to export.
 * \return
 * - the length of string exported
 * - negative if failure
 * 
 * \pre the key should reference a row of a table or a field of a record
 * \pre the key be described in dbs.xml with the attribute uta:export
 * 
 * \sa
 *  - dbaImport()
 *  - dbaExport()
 *  - tabExport()
 *  - recExport()
 */ 
int mapExport(word key,byte sec){
    word rel;

    if(key!=begKey(key)){
        rel= relKey(key);
        switch(key){
            default: break;
        }
    }else{
        int ret;
        word idx,beg,end,num;
        char exp;
        char *str;
        tTable *tab;
        char tmp[256];
        char buf[256];

        beg= 0;
        end= 0;
        exp= 0;
        tab= 0;
        str= 0;
        switch(begKey(key)){
            default:
                break;
        }
        VERIFY(beg);
        VERIFY(end);
        VERIFY(exp);
        VERIFY(tab);
        VERIFY(str);
        for(idx= beg+1; idx<end; idx++){
            num= idx-beg-1;;
            strcpy(buf,str);
            strcat(buf,"[");
            num2dec(tmp,num,0);
            strcat(buf,tmp);
            strcat(buf,"]");
            ret= tabExport(tab,num,sec,'S',buf);
            if(ret<0) return -1;
        }
        return end-beg-1;
    }
    return -1;
}

