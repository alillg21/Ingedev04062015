/** \file
 * Drawings implementation
 *
 * Subversion reference
 *
 * $HeadURL: https://svn.ingenico.com/UTA/core/utaYYMMDD/pri/drw.c $
 *
 * $Id: drw.c 2322 2009-04-22 11:50:57Z abarantsev $
 */

#include "pri.h"
#include <string.h>

#ifdef __CANVAS__
#undef trcOpt
//static const card trcOpt = TRCOPT & BIT(trcDRW);
#define trcOpt (TRCOPT & BIT(trcDRW))

/** Pixel shape constructor.
 * \param shp (O) The pointer to the shape descriptor to be initialized
 * \param clr (I) Shape color
 * \pre 
 *    - shp!=0
 *    - clr<cnvP
 * \return 1
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
int drwPixelInit(tPixel * shp, word clr) {
    VERIFY(shp);
    //VERIFY(clr < cnvP);
    trcFN("drwPixelInit clr=%d\n", clr);
    shp->clr = clr;
    return 1;
}

/** Pixel shape draw method.
 * The shape is not drawn if it is not on the canvas
 * \param shp (I) The pointer to the shape descriptor to be drawn
 * \param x (I) The horizontal coordinate of the reference point (0..cnvW: left->right)
 * \param y (I) The vertical coordinate of the reference point (0..cnvH: top->bottom)
 * \pre 
 *    - shp!=0
 * \return 1 if drawn, 0 if not, negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
int drwPixelDraw(tPixel * shp, word x, word y) {
    trcFN("drwPixelDraw x=%d", x);
    trcFN(" y=%d", y);
    trcFN(" clr=%d\n", shp->clr);
    VERIFY(shp);
    if(x >= cnvW)
        return 0;
    if(y >= cnvH)
        return 0;
    return cnvPixel(x, y, shp->clr);
}

static card drwPixelRel(const tPixel * shp, int x, int y) {
    VERIFY(shp);
    if(x)
        return RELOUT;
    if(0)
        return RELOUT;
    return 0;
}

/** Line shape constructor.
 * The anchor point of the line is (0,0).
 * It is drawn from (0,0) to (x,y)
 * \param shp (O) The pointer to the shape descriptor to be initialized
 * \param x (I) Horizontal coordinate of line end (0..cnvW: left->right)
 * \param y (I) Vertical coordinate of line end (0..cnvH: top->bottom)
 * \param clr (I) Shape color
 * \pre 
 *    - shp!=0
 *    - x<cnvW
 *    - y<cnvH
 *    - clr<cnvP
 * \return 1
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
int drwLineInit(tLine * shp, word x, word y, word clr) {
    VERIFY(shp);
    VERIFY(x < cnvW);
    VERIFY(y < cnvH);
    //VERIFY(clr < cnvP);
    trcFN("drwLineInit x=%d", x);
    trcFN(" y=%d", y);
    trcFN(" clr=%d\n", clr);

    shp->x = x;
    shp->y = y;
    shp->clr = clr;

    return 1;
}

/** Line shape draw method.
 * The shape is not drawn if it is not on the canvas.
 * \param shp (I) The pointer to the shape descriptor to be drawn
 * \param x (I) The horizontal coordinate of the reference point (0..cnvW: left->right)
 * \param y (I) The vertical coordinate of the reference point (0..cnvW: left->right)
 * \pre 
 *    - shp!=0
 * \return 1 if drawn, 0 if not, negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
int drwLineDraw(tLine * shp, word x, word y) {
    word x1, y1, x2, y2;

    trcFN("drwLineDraw x=%d", x);
    trcFN(" y=%d", y);
    trcFN(" dx=%d", shp->x);
    trcFN(" dy=%d", shp->y);
    trcFN(" clr=%d\n", shp->clr);
    VERIFY(shp);
    x1 = x;
    y1 = y;
    x2 = x + shp->x;
    y2 = y + shp->y;
    if(x1 >= cnvW)
        return 0;
    if(y1 >= cnvH)
        return 0;
    if(x2 >= cnvW)
        return 0;
    if(y2 >= cnvH)
        return 0;
    return cnvLine(x1, y1, x2, y2, shp->clr);
}

static card drwLineRel(const tLine * shp, int x, int y) {
    VERIFY(shp);
    return RELOUT;              //not required
/*
    x2 = x + shp->x;
    y2 = y + shp->y;
    if(x*(shp->y)!=(shp->x)*y)
      return RELOUT;
    return CARDHL(x,y);
*/
}

/** Bar shape constructor.
 * \param shp (O) The pointer to the shape descriptor to be initialized
 * \param wdt (I) bar width (0..cnvW: left->right)
 * \param hgt (I) bar height (0..cnvH: top->bottom)
 * \param clr (I) Shape color
 * \pre 
 *    - shp!=0
 *    - wdt>0
 *    - hgt>0
 *    - wdt<cnvW
 *    - hgt<cnvH
 *    - clr<cnvP
 * \return 1
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
int drwBarInit(tBar * shp, word wdt, word hgt, word clr) {
    VERIFY(shp);
    VERIFY(wdt);
    VERIFY(hgt);
    VERIFY(wdt <= cnvW);
    VERIFY(hgt <= cnvH);
    //VERIFY(clr < cnvP);
    trcFN("drwBarInit wdt=%d", wdt);
    trcFN(" hgt=%d", hgt);
    trcFN(" clr=%d\n", clr);

    shp->wdt = wdt;
    shp->hgt = hgt;
    shp->clr = clr;

    return 1;
}

/** Bar shape draw method.
 * \param shp (I) The pointer to the shape descriptor to be drawn
 * \param x (I) The horizontal coordinate of the reference point (0..cnvW: left->right)
 * \param y (I) The vertical coordinate of the reference point (0..cnvW: left->right)
 * \pre 
 *    - shp!=0
  * \return 1 if drawn, 0 if not, negative in case of error
* \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
int drwBarDraw(tBar * shp, word x, word y) {
    word x1, y1, x2, y2;

    trcFN("drwBarDraw x=%d", x);
    trcFN(" y=%d", y);
    trcFN(" wdt=%d", shp->wdt);
    trcFN(" hgt=%d", shp->hgt);
    trcFN(" clr=%d\n", shp->clr);
    VERIFY(shp);
    x1 = x;
    y1 = y;
    x2 = x + shp->wdt - 1;
    y2 = y + shp->hgt - 1;
    if(x1 >= cnvW)
        return 0;
    if(y1 >= cnvH)
        return 0;
    if(x2 >= cnvW)
        return 0;
    if(y2 >= cnvH)
        return 0;
    return cnvBar(x1, y1, x2, y2, shp->clr);
}

static card drwBarRel(const tBar * shp, int x, int y) {
    VERIFY(shp);

    if(x < 0)
        return RELOUT;
    if(y < 0)
        return RELOUT;
    if(x > shp->wdt)
        return RELOUT;
    if(y > shp->hgt)
        return RELOUT;
    return CARDHL(x, y);
}

/** Text shape constructor.
 * \param shp (O) The pointer to the shape descriptor to be initialized
 * \param cnt (I) Pointer to a container that contains the text data element to be drawn
 * \param key (I) Key to access the data element from container
 * \param fnt (I) Font to be used
 * \param clr (I) Shape color
 * \pre 
 *    - shp!=0
 *    - cnt!=0
 * \return 1
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
int drwTextInit(tText * shp, const tContainer * cnt, word key, byte fnt,
                card clr) {
    VERIFY(shp);
    VERIFY(cnt);
    //VERIFY(clr < cnvP);
    trcFN("drwTextInit key=%d", key);
    trcFN(" fnt=%d", fnt);
    trcFN(" clr=%d\n", clr);

    shp->cnt = cnt;
    shp->key = key;
    shp->fnt = fnt;
    shp->clr = clr;

    return 1;
}

/** Clone a text shape.
 * \param shp (O) The pointer to an array of text shape descriptors to be initialized
 * \param cnt (I) Pointer to an array of  containers that contain the text data elements to be drawn
 * \param key (I) Pointer to an array of keys to access the data elements from containers
 * \param fnt (I) Pointer to an array of fonts to be used
 * \param clr (I) Pointer to an array of shape colors
 * \param dim (I) Number of elements in the arrays
 * \pre 
 *    - shp!=0
 *    - dim>1
 * \return 1
 *
 * The first shape in the array should be already initialized before cloning.
 * All other shapes in the array are initialized according to the first element of the array
 * For each initialization parameter its value is retrieved from the related array if its pointer is not zero
 * Otherwise it is copied from the sample element (the 1st one)
 *
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0198.c
*/
int drwTextClone(tText * shp, const tContainer * cnt, word * key, byte * fnt,
                 word * clr, byte dim) {
    byte idx;

    VERIFY(shp);
    VERIFY(dim > 1);
    trcFN("drwTextClone dim=%d", dim);

    for (idx = 1; idx < dim; idx++) {
        word _key = shp->key;
        byte _fnt = shp->fnt;
        byte _clr = shp->clr;
        const tContainer *_cnt = shp->cnt;

        if(cnt)
            _cnt = &cnt[idx];
        if(key)
            _key = key[idx];
        if(fnt)
            _fnt = fnt[idx];
        if(clr)
            _clr = clr[idx];

        drwTextInit(&shp[idx], _cnt, _key, _fnt, _clr);
    }

    return dim;
}

/** Text shape draw method.
 * \param shp (I) The pointer to the shape descriptor to be drawn
 * \param x (I) The horizontal coordinate of the reference point (0..cnvW: left->right)
 * \param y (I) The vertical coordinate of the reference point (0..cnvW: left->right)
 * \pre 
 *    - shp!=0
  * \return 1 if drawn, 0 if not, negative in case of error
* \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
int drwTextDraw(tText * shp, word x, word y) {
    int ret;
    char buf[256];

    VERIFY(shp);
    if(x >= cnvW)
        return 0;
    if(y >= cnvH)
        return 0;

    ret = cntGet(shp->cnt, shp->key, buf, 256);
    if(ret < 0)
        return ret;

    trcFN("drwTextDraw x=%d", x);
    trcFN(" y=%d", y);
    trcFN(" fnt=%d", shp->fnt);
    trcFN(" clr=%d", shp->clr);
    trcFS(" str=%s\n", buf);
    if(ret < 0)
        return ret;
    return cnvText(x, y, buf, shp->fnt, shp->clr);
}

static card drwTextRel(const tText * shp, int x, int y) {
    int ret;
    char buf[256];
    card wh;
    word w, h;

    VERIFY(shp);

    if(y > 0)                   //the base point is left-bottom
        return RELOUT;

    if(x < 0)
        return RELOUT;

    ret = cntGet(shp->cnt, shp->key, buf, 256);
    if(ret < 0)
        return ret;
    wh = cnvTextSize(buf, shp->fnt);

    w = HWORD(wh);
    h = LWORD(wh);

    if(x > w)
        return RELOUT;
    if(y < -h)
        return RELOUT;
    return CARDHL(x, y);
}

/** Tile shape constructor.
 * \param shp (O) The pointer to the shape descriptor to be initialized
 * \param ptr (I) Pointer to tile/bitmap
 * \param wdt (I) Tile width (0..cnvW: left->right)
 * \param hgt (I) Tile height (0..cnvH: top->bottom)
 * \param clr (I) Shape color
 * \pre 
 *    - shp!=0
 *    - ptr!=0
 *    - wdt<cnvW
 *    - hgt<cnvH
 *    - clr<cnvP
 * \return 1
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
int drwTileInit(tTile * shp, const byte * ptr, word wdt, word hgt, int clr) {
    VERIFY(shp);
    VERIFY(ptr);
    VERIFY(wdt < cnvW);
    VERIFY(hgt < cnvH);
    VERIFY(clr < cnvP);

    if(!wdt) {
        shp->wdt = bmpWdt(ptr);
        shp->hgt = bmpHgt(ptr);
        shp->ptr = bmpDat(ptr);
    } else {
        shp->wdt = wdt;
        shp->hgt = hgt;
        shp->ptr = ptr;
    }
    shp->clr = clr;

    trcFN("drwTileInit wdt=%d", shp->wdt);
    trcFN(" hgt=%d", shp->hgt);
    trcFN(" clr=%d\n", shp->clr);
    return 1;
}

/** Tile shape draw method.
 * \param shp (I) The pointer to the shape descriptor to be drawn
 * \param x (I) The horizontal coordinate of the reference point (0..cnvW: left->right)
 * \param y (I) The vertical coordinate of the reference point (0..cnvW: left->right)
 * \pre 
 *    - shp!=0
  * \return 1 if drawn, 0 if not, negative in case of error
* \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
int drwTileDraw(tTile * shp, word x, word y) {
    word x1, y1, x2, y2;

    trcFN("drwTileDraw x=%d", x);
    trcFN(" y=%d", y);
    trcFN(" wdt=%d", shp->wdt);
    trcFN(" hgt=%d", shp->hgt);
    trcFN(" clr=%d\n", shp->clr);
    VERIFY(shp);
    x1 = x;
    y1 = y;
    x2 = x + shp->wdt - 1;
    y2 = y + shp->hgt - 1;
    if(x1 >= cnvW)
        return 0;
    if(y1 >= cnvH)
        return 0;
    if(x2 >= cnvW)
        return 0;
    if(y2 >= cnvH)
        return 0;
    return cnvTile(x1, y1, shp->wdt, shp->hgt, shp->clr, shp->ptr);
}

static card drwTileRel(const tTile * shp, int x, int y) {
    VERIFY(shp);

    if(x < 0)
        return RELOUT;
    if(y < 0)
        return RELOUT;
    if(x > shp->wdt)
        return RELOUT;
    if(y > shp->hgt)
        return RELOUT;
    return CARDHL(x, y);
}

/** Box shape constructor.
 * \param shp (O) The pointer to the shape descriptor to be initialized
 * \param wdt (I) box width (0..cnvW: left->right)
 * \param hgt (I) box height (0..cnvH: top->bottom)
 * \param clr (I) Shape color
 * \pre 
 *    - shp!=0
 *    - wdt<=cnvW
 *    - hgt<=cnvH
 *    - clr<cnvP
 * \return 1
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
int drwBoxInit(tBox * shp, word wdt, word hgt, word clr) {
    VERIFY(shp);
    VERIFY(wdt <= cnvW);
    VERIFY(hgt <= cnvH);
    //VERIFY(clr < cnvP);
    trcFN("drwBoxInit wdt=%d", wdt);
    trcFN(" hgt=%d", hgt);
    trcFN(" clr=%d\n", clr);

    shp->wdt = wdt;
    shp->hgt = hgt;
    shp->clr = clr;

    return 1;
}

/** Box shape draw method.
 * \param shp (I) The pointer to the shape descriptor to be drawn
 * \param x (I) The horizontal coordinate of the reference point (0..cnvW: left->right)
 * \param y (I) The vertical coordinate of the reference point (0..cnvW: left->right)
 * \pre 
 *    - shp!=0
  * \return 1 if drawn, 0 if not, negative in case of error
* \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
int drwBoxDraw(tBox * shp, word x, word y) {
    int ret;
    word x1, y1, x2, y2;

    trcFN("drwBoxDraw x=%d", x);
    trcFN(" y=%d", y);
    trcFN(" wdt=%d", shp->wdt);
    trcFN(" hgt=%d", shp->hgt);
    trcFN(" clr=%d\n", shp->clr);
    VERIFY(shp);

    x1 = x;
    y1 = y;
    x2 = x + shp->wdt - 1;
    y2 = y + shp->hgt - 1;

    if(x1 >= cnvW)
        return 0;
    if(y1 >= cnvH)
        return 0;
    if(x2 >= cnvW)
        return 0;
    if(y2 >= cnvH)
        return 0;

    ret = cnvLine(x1, y1, x2, y1, shp->clr);
    if(ret < 0)
        return ret;
    ret = cnvLine(x2, y1, x2, y2, shp->clr);
    if(ret < 0)
        return ret;
    ret = cnvLine(x1, y2, x2, y2, shp->clr);
    if(ret < 0)
        return ret;
    ret = cnvLine(x1, y1, x1, y2, shp->clr);
    return ret;
}

static card drwBoxRel(const tBox * shp, int x, int y) {
    VERIFY(shp);

    if(x < 0)
        return RELOUT;
    if(y < 0)
        return RELOUT;
    if(x > shp->wdt)
        return RELOUT;
    if(y > shp->hgt)
        return RELOUT;
    return CARDHL(x, y);
}

/** Circle shape constructor.
 * \param shp (O) The pointer to the shape descriptor to be initialized
 * \param rad (I) circle radius
 * \param clr (I) Shape color
 * \pre 
 *    - shp!=0
 *    - clr<cnvP
 * \return 1
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
int drwCircleInit(tCircle * shp, word rad, word clr) {
    VERIFY(shp);
    //VERIFY(clr < cnvP);
    trcFN("drwCircleInit rad=%d", rad);
    trcFN(" clr=%d\n", shp->clr);

    shp->rad = rad;
    shp->clr = clr;

    return 1;
}

static int relPlot(int x, int y, byte clr) {
    if(x < 0)
        return 0;
    if(x >= cnvW)
        return 0;
    if(y < 0)
        return 0;
    if(y >= cnvH)
        return 0;
    return cnvPixel((word) x, (word) y, clr);
}

static int cirPlot(int x0, int y0, int dx, int dy, byte clr) {
    int ret;

    ret = relPlot(x0 + dx, y0 + dy, clr);
    if(ret < 0)
        return ret;
    ret = relPlot(x0 - dx, y0 + dy, clr);
    if(ret < 0)
        return ret;
    ret = relPlot(x0 + dx, y0 - dy, clr);
    if(ret < 0)
        return ret;
    ret = relPlot(x0 - dx, y0 - dy, clr);
    if(ret < 0)
        return ret;

    ret = relPlot(x0 + dy, y0 + dx, clr);
    if(ret < 0)
        return ret;
    ret = relPlot(x0 - dy, y0 + dx, clr);
    if(ret < 0)
        return ret;
    ret = relPlot(x0 + dy, y0 - dx, clr);
    if(ret < 0)
        return ret;
    ret = relPlot(x0 - dy, y0 - dx, clr);
    if(ret < 0)
        return ret;

    return 1;
}

/** Circle shape draw method.
 * \param shp (I) The pointer to the shape descriptor to be drawn
 * \param x (I) The horizontal coordinate of the reference point (0..cnvW: left->right)
 * \param y (I) The vertical coordinate of the reference point (0..cnvW: left->right)
 * \pre 
 *    - shp!=0
  * \return 1 if drawn, 0 if not, negative in case of error
* \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
int drwCircleDraw(tCircle * shp, word x, word y) {
    int ret;
    int h, v, t, n;

    trcFN("drwCircleDraw x=%d", x);
    trcFN(" y=%d", y);
    trcFN(" rad=%d", shp->rad);
    trcFN(" clr=%d\n", shp->clr);
    VERIFY(shp);

    h = 0;
    v = shp->rad << 1;
    t = 0;
    n = 0;
    while(h <= v + 2) {
        if(!(h & 1)) {
            ret = cirPlot(x, y, h >> 1, (v + 1) >> 1, shp->clr);
            if(ret < 0)
                return ret;
            n++;
        }
        t += (h << 1) + 1;
        h++;
        if(t > 0) {
            t -= (v << 1) - 1;
            v--;
        }
    }
    return n;
}

static card drwCircleRel(const tCircle * shp, int x, int y) {
    VERIFY(shp);

    if((x * x + y * y) > (shp->rad) * (shp->rad))
        return RELOUT;
    return CARDHL(x, y);
}

/** Disc shape constructor.
 * \param shp (O) The pointer to the shape descriptor to be initialized
 * \param rad (I) disc radius
 * \param clr (I) Shape color
 * \pre 
 *    - shp!=0
 *    - clr<cnvP
 * \return 1
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
int drwDiscInit(tDisc * shp, word rad, word clr) {
    VERIFY(shp);
    //VERIFY(clr < cnvP);
    trcFN("drwDiscInit rad=%d", rad);
    trcFN(" clr=%d\n", shp->clr);

    shp->rad = rad;
    shp->clr = clr;

    return 1;
}

static int relPlotHor(int x1, int x2, int y, byte clr) {
    if(y < 0)
        return 0;
    if(y >= cnvH)
        return 0;
    if(x1 >= cnvW)
        return 0;
    if(x2 < 0)
        return 0;
    if(x1 < 0)
        x1 = 0;
    if(x2 >= cnvW)
        x2 = cnvW - 1;
    return cnvLine((word) x1, (word) y, (word) x2, (word) y, clr);
}

static int dscPlot(int x0, int y0, int dx, int dy, byte clr) {
    int ret;

    ret = relPlotHor(x0 - dx, x0 + dx, y0 + dy, clr);
    if(ret < 0)
        return ret;
    ret = relPlotHor(x0 - dx, x0 + dx, y0 - dy, clr);
    if(ret < 0)
        return ret;
    ret = relPlotHor(x0 - dy, x0 + dy, y0 + dx, clr);
    if(ret < 0)
        return ret;
    ret = relPlotHor(x0 - dy, x0 + dy, y0 - dx, clr);
    if(ret < 0)
        return ret;
    return 1;
}

/** Disc shape draw method.
 * \param shp (I) The pointer to the shape descriptor to be drawn
 * \param x (I) The horizontal coordinate of the reference point (0..cnvW: left->right)
 * \param y (I) The vertical coordinate of the reference point (0..cnvW: left->right)
 * \pre 
 *    - shp!=0
 * \return 1 if drawn, 0 if not, negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
int drwDiscDraw(tDisc * shp, word x, word y) {
    int ret;
    int h, v, t, n;

    trcFN("drwDiscDraw x=%d", x);
    trcFN(" y=%d", y);
    trcFN(" rad=%d", shp->rad);
    trcFN(" clr=%d\n", shp->clr);
    VERIFY(shp);

    h = 0;
    v = shp->rad << 1;
    t = 0;
    n = 0;
    while(h <= v + 2) {
        if(!(h & 1)) {
            ret = dscPlot(x, y, h >> 1, (v + 1) >> 1, shp->clr);
            if(ret < 0)
                return ret;
            n++;
        }
        t += (h << 1) + 1;
        h++;
        if(t > 0) {
            t -= (v << 1) - 1;
            v--;
        }
    }
    return n;
}

static card drwDiscRel(const tDisc * shp, int x, int y) {
    VERIFY(shp);

    if((x * x + y * y) > (shp->rad) * (shp->rad))
        return RELOUT;
    return CARDHL(x, y);
}

/** Bezier curve degree 3 shape constructor.
 * \param shp (O) The pointer to the shape descriptor to be initialized
 * \param ctl (I) array of control points packed in card
 * \param clr (I) Shape color
 * \pre 
 *    - shp!=0
 *    - clr<cnvP
 * \return 1
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
int drwBezierInit(tBezier * shp, const card * ctl, word clr) {
    VERIFY(shp);
    VERIFY(ctl);
    //VERIFY(clr < cnvP);
    trcFN("drwBezierInit clr=%d\n", clr);

    memcpy(shp->ctl, ctl, 4 * sizeof(card));
    shp->clr = clr;

    return 1;
}

#define ABS(X) (((X)>0)?(X):(-(X)))
static int bzr(word dx, word dy, const card * ctl, byte clr) {
    int ret;
    card lft[4];
    card rgt[4];
    word x0, y0, x1, y1, x2, y2, x3, y3, x, y;

    VERIFY(ctl);

    x0 = HWORD(ctl[0]);
    y0 = LWORD(ctl[0]);
    x1 = HWORD(ctl[1]);
    y1 = LWORD(ctl[1]);
    x2 = HWORD(ctl[2]);
    y2 = LWORD(ctl[2]);
    x3 = HWORD(ctl[3]);
    y3 = LWORD(ctl[3]);

    if(ABS(x0 - x3) <= 2 && ABS(y0 - y3) <= 2)
        return cnvLine((word) (dx + x0), (word) (dy + y0), x3, y3, clr);

    x = x0;
    y = y0;
    lft[0] = CARDHL(x, y);
    x = (x0 + x1) / 2;
    y = (y0 + y1) / 2;
    lft[1] = CARDHL(x, y);
    x = (x0 + 2 * x1 + x2) / 4;
    y = (y0 + 2 * y1 + y2) / 4;
    lft[2] = CARDHL(x, y);
    x = (x0 + 3 * x1 + 3 * x2 + x3) / 8;
    y = (y0 + 3 * y1 + 3 * y2 + y3) / 8;
    lft[3] = CARDHL(x, y);

    rgt[0] = lft[3];
    x = (x1 + 2 * x2 + x3) / 4;
    y = (y1 + 2 * y2 + y3) / 4;
    rgt[1] = CARDHL(x, y);
    x = (x2 + x3) / 2;
    y = (y2 + y3) / 2;
    rgt[2] = CARDHL(x, y);
    x = x3;
    y = y3;
    rgt[3] = CARDHL(x, y);

    ret = bzr(dx, dy, lft, clr);
    if(ret < 0)
        return ret;
    ret = bzr(dx, dy, rgt, clr);
    if(ret < 0)
        return ret;

    return 1;
}

/** Bezier curve degree 3 shape draw method.
 * \param shp (I) The pointer to the shape descriptor to be drawn
 * \param x (I) The horizontal coordinate of the reference point (0..cnvW: left->right)
 * \param y (I) The vertical coordinate of the reference point (0..cnvW: left->right)
 * \pre 
 *    - shp!=0
 * \return 1 if drawn, 0 if not, negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
int drwBezierDraw(tBezier * shp, word x, word y) {
    VERIFY(shp);
    trcFN("drwBezierDrawx=%d", x);
    trcFN(" y=%d", y);
    trcFN(" clr=%d\n", shp->clr);
    return bzr(x, y, shp->ctl, shp->clr);
}

static card drwBezierRel(const tBezier * shp, int x, int y) {
    VERIFY(shp);
    return RELOUT;              //not required
}

/** Shape constructor: 
 * Given an existing shape pointed to by ptr create an abstract shape of related type typ.
 * \param shp (O) Pointer to the shape to be initialized
 * \param typ (I) Shape type from eShp enum
 * \param ptr (I) Pointer to the shape body that depends on the value of typ argument
 *  - shpDrawing: tDrawing
 *  - shpPixel: tPixel
 *  - shpLine: tLine
 *  - shpBar: tBar
 *  - etc
 * \pre 
 *  - shp!=0
 *  - ptr!=0
 *  - shpBeg<typ<shpEnd
 * \return 1
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
int drwShapeInit(tShape * shp, byte typ, void *ptr) {
    VERIFY(shp);
    VERIFY(ptr);
    trcFN("drwShapeInit type=%d\n", typ);

    shp->loc = RELOUT;
    shp->key = 0;
    shp->typ = typ;
    switch (typ) {
      case shpPixel:
          shp->ptr.pix = (tPixel *) ptr;
          break;
      case shpLine:
          shp->ptr.lin = (tLine *) ptr;
          break;
      case shpBar:
          shp->ptr.bar = (tBar *) ptr;
          break;
      case shpText:
          shp->ptr.txt = (tText *) ptr;
          break;
      case shpTile:
          shp->ptr.til = (tTile *) ptr;
          break;
      case shpBox:
          shp->ptr.box = (tBox *) ptr;
          break;
      case shpCircle:
          shp->ptr.cir = (tCircle *) ptr;
          break;
      case shpDisc:
          shp->ptr.dsc = (tDisc *) ptr;
          break;
      case shpBezier:
          shp->ptr.bzr = (tBezier *) ptr;
          break;
      case shpButton:
          shp->ptr.btn = (tButton *) ptr;
          break;
      case shpSelector:
          shp->ptr.sel = (tSelector *) ptr;
          break;
      case shpSlider:
          shp->ptr.sld = (tSlider *) ptr;
          break;
      case shpVideo:
          shp->ptr.vid = (tVideo *) ptr;
          break;
      default:
          VERIFY(typ == shpDrawing);
          shp->ptr.drw = (tDrawing *) ptr;
          break;
    }
    VERIFY(typ);
    return typ;
}

/** Abstract shape draw method.
 * Depending on the shape type the related real draw method is called
 * \param shp (I) The pointer to the shape descriptor to be drawn
 * \param x (I) The horizontal coordinate of the reference point (0..cnvW: left->right)
 * \param y (I) The vertical coordinate of the reference point (0..cnvH: top->bottom)
 * \pre 
 *    - shp!=0
  * \return 1 if drawn, 0 if not, negative in case of error
* \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
int drwShapeDraw(tShape * shp, word x, word y) {
    VERIFY(shp);

    shp->loc = CARDHL(x, y);
    switch (shp->typ) {
      case shpPixel:
          return drwPixelDraw(shp->ptr.pix, x, y);
      case shpLine:
          return drwLineDraw(shp->ptr.lin, x, y);
      case shpBar:
          return drwBarDraw(shp->ptr.bar, x, y);
      case shpText:
          return drwTextDraw(shp->ptr.txt, x, y);
      case shpTile:
          return drwTileDraw(shp->ptr.til, x, y);
      case shpBox:
          return drwBoxDraw(shp->ptr.box, x, y);
      case shpCircle:
          return drwCircleDraw(shp->ptr.cir, x, y);
      case shpDisc:
          return drwDiscDraw(shp->ptr.dsc, x, y);
      case shpBezier:
          return drwBezierDraw(shp->ptr.bzr, x, y);
      case shpButton:
          return drwButtonDraw(shp->ptr.btn, x, y);
      case shpSelector:
          return drwSelectorDraw(shp->ptr.sel, x, y);
      case shpSlider:
          return drwSliderDraw(shp->ptr.sld, x, y);
      case shpVideo:
          return drwVideoDraw(shp->ptr.vid, x, y);
      default:
          VERIFY(shp->typ == shpDrawing);
          return drwDrawingDraw(shp->ptr.drw, x, y);
    }
}

//forward declarations:
static card drwDrawingRel(const tDrawing * shp, int x, int y);
static card drwButtonRel(const tButton * btn, int x, int y);

/** If the point is within the shape get its relative position
 * \param shp (I) The pointer to the shape descriptor
 * \param x (I) The horizontal coordinate of the point (0..cnvW: left->right)
 * \param y (I) The vertical coordinate of the point (0..cnvH: top->bottom)
 * \pre 
 *    - shp!=0
 * \return
 *  - relative position packed into the card if the point is within the shape
 *  - RELOUT otherwise
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0159
*/
card drwShapeRel(const tShape * shp, int x, int y) {
    int rx, ry;

    VERIFY(shp);

    if(shp->loc == RELOUT)
        return RELOUT;
    rx = HWORD(shp->loc);
    ry = LWORD(shp->loc);
    switch (shp->typ) {
      case shpPixel:
          return drwPixelRel(shp->ptr.pix, x - rx, y - ry);
      case shpLine:
          return drwLineRel(shp->ptr.lin, x - rx, y - ry);
      case shpBar:
          return drwBarRel(shp->ptr.bar, x - rx, y - ry);
      case shpText:
          return drwTextRel(shp->ptr.txt, x - rx, y - ry);
      case shpTile:
          return drwTileRel(shp->ptr.til, x - rx, y - ry);
      case shpBox:
          return drwBoxRel(shp->ptr.box, x - rx, y - ry);
      case shpCircle:
          return drwCircleRel(shp->ptr.cir, x - rx, y - ry);
      case shpDisc:
          return drwDiscRel(shp->ptr.dsc, x - rx, y - ry);
      case shpBezier:
          return drwBezierRel(shp->ptr.bzr, x - rx, y - ry);
      case shpButton:
          //return drwDrawingRel(&shp->ptr.btn->drw, x, y);
          return drwButtonRel(shp->ptr.btn, x - rx, y - ry);
      case shpSelector:
          return drwDrawingRel(&shp->ptr.sel->drw, x, y);
      case shpSlider:
          return drwDrawingRel(&shp->ptr.sld->drw, x, y);
      case shpVideo:
          return drwDrawingRel(shp->ptr.vid->drw, x, y);
      default:
          VERIFY(shp->typ == shpDrawing);
          return drwDrawingRel(shp->ptr.drw, x, y);
    }
}

/** Associate a keyboard key with the shape shp
 * \param shp (I) The pointer to the shape descriptor
 * \param key (I) The key to be associated with the shape
 * \pre 
 *    - shp!=0
 * 
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
*/
void drwShapeKey(tShape * shp, char key) {
    VERIFY(shp);
    shp->key = key;
}

/** Shape location getter
 * \param shp (I) The pointer to the shape descriptor
 * \pre 
 *    - shp!=0
 * \return the location where the shape was drawn last time.
 * 
 * The location is packed into a card as (x,y)
 *
 * The value RELOUT means that the shape was never drawn
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
*/
card drwShapeLoc(const tShape * shp) {
    return shp->loc;
}

/** Drawing constructor: 
 * Build a drawing from an array of shapes shp anchored at reference points rfp.
 *
 * The reference point coordinates are packed into card using CARDHL macro.
 * The highest word of card is horizontal coordinate of the reference point;
 * the lowest one is the vertical coordinate.
 *
 * The highest bit of a reference point card value is used as visibility attribute.
 * When it is set to 1 the related shape is not drawn.
 * \param drw (O) Pointer to the drawing to be initialized
 * \param shp (I) Array of shapes constituting the drawing, should be terminated by empty shape (typ=0)
 * \param rfp (I) Array of reference points
 * \pre 
 *  - drw!=0
 *  - shp!=0
 *  - rfp!=0
 * \return number of shapes in the drawing
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
int drwDrawingInit(tDrawing * drw, tShape * shp, card * rfp) {
    tShape *ptr;

    VERIFY(drw);
    VERIFY(shp);
    VERIFY(rfp);

    drw->shp = shp;
    drw->rfp = rfp;
    drw->dim = 0;

    ptr = shp;
    while(ptr->typ) {
        ptr++;
        drw->dim++;
    }
    drw->loc = RELOUT;
    trcFN("drwDrawingInit dim=%d\n", drw->dim);
    return drw->dim;
}

/** Drawing draw method.
 * For each shape in the drawing call the related draw method if the shape is not hidden
 * \param drw (I) The pointer to the drawing descriptor to be drawn
 * \param x (I) The horizontal coordinate of the reference point (0..cnvW: left->right)
 * \param y (I) The vertical coordinate of the reference point (0..cnvH: top->bottom)
 * \pre 
 *    - drw!=0
 * \return 1 if drawn, 0 if not, negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
int drwDrawingDraw(tDrawing * drw, word x, word y) {
    trcS("drwDrawingDraw\n");
    VERIFY(drw);
    drw->loc = CARDHL(x, y);
    return drwDrawingRefresh(drw, drw->dim);
}

/** Drawing refresh method.
 * Redraw it at the location drw->loc if already was drawn; otherwise do nothing.
 * Only the top dim shapes are redrawn.
 * \param drw (I) The pointer to the drawing descriptor to be drawn
 * \param num (I) Number of shapes to redraw; if dim is bigger than drawing dimension all is redrawn
 * \pre 
 *    - drw!=0
 * \return 1 if drawn, 0 if not, negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
*/
int drwDrawingRefresh(tDrawing * drw, byte num) {
    int ret;
    byte idx;
    word x, y;

    VERIFY(drw);

    trcFN("drwDrawingRefresh: num=%d\n", num);
    if(drw->loc == RELOUT)
        return 0;
    x = HWORD(drw->loc);
    y = LWORD(drw->loc);
    if(num > drw->dim)
        num = drw->dim;

    num = drw->dim - num;
    for (idx = num; idx < drw->dim; idx++) {
        if(!drwIsVisible(drw, idx))
            continue;
        ret =
            drwShapeDraw(&drw->shp[idx], (word) (x + HWORD(drw->rfp[idx])),
                         (word) (y + LWORD(drw->rfp[idx])));
        //cnvDraw();
        if(ret < 0)
            return ret;
    }
    trcS("drwDrawingRefresh: End\n");
    return drw->dim - num;
}

static word drwDrawingIdx(const tDrawing * drw, int x, int y) {
    card rel;
    int idx;

    VERIFY(drw);

    idx = drw->dim;
    while(idx--) {              //backward direction: upper layer -> background
        if(!drwIsVisible(drw, (byte) idx))
            continue;
        rel = drwShapeRel(&drw->shp[idx], x, y);
        if(rel != RELOUT)
            break;
    }
    if(idx < 0)
        return drw->dim;
    return idx;
}

static card drwDrawingRel(const tDrawing * drw, int x, int y) {
    word idx;
    int rx, ry;

    VERIFY(drw);

    idx = drwDrawingIdx(drw, x, y);
    VERIFY(idx <= drw->dim);

    if(idx >= drw->dim)
        return RELOUT;

    rx = HWORD(drw->loc);
    ry = HWORD(drw->loc);
    return CARDHL(x - rx, y - ry);
}

/** Drawing getter.
 * Return a pointer to a shape containing the given point
 * \param drw (I) The pointer to the drawing descriptor
 * \param x (I) Horizontal coordinate (left->right)
 * \param y (I) Vertical coordinate (top->bottom)
 * \pre 
 *    - drw!=0
 * \return pointer to a shape containing the given point; NULL if there is no such a shape
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
*/
tShape *drwDrawingShape(const tDrawing * drw, int x, int y) {
    word idx;

    VERIFY(drw);

    idx = drwDrawingIdx(drw, x, y);
    VERIFY(idx <= drw->dim);
    if(idx >= drw->dim)
        return 0;

    if(drw->shp[idx].typ != shpDrawing)
        return &drw->shp[idx];

    return drwDrawingShape(drw->shp[idx].ptr.drw, x, y);
}

/** Drawing getter.
 * Return the number of shapes in the drawing.
 * \param drw (I) The pointer to the drawing descriptor
 * \pre 
 *    - drw!=0
 * \return 1 if drawn, 0 if not, negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
byte drwDim(const tDrawing * drw) {
    VERIFY(drw);
    return drw->dim;
}

/** Make visible the shape number idx in the drawing
 * \param drw (I) The pointer to the drawing descriptor
 * \param idx (I) Index of the shape to be made visible
 * \pre 
 *    - drw!=0
 *    - idx<drw->dim
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
void drwShow(tDrawing * drw, byte idx) {
    VERIFY(drw);
    VERIFY(idx < drw->dim);
    drw->rfp[idx] &= 0x7FFFFFFFUL;
}

/** Make invisible the shape number idx in the drawing
 * \param drw (I) The pointer to the drawing descriptor
 * \param idx (I) Index of the shape to be made invisible
 * \pre 
 *    - drw!=0
 *    - idx<drw->dim
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
void drwHide(tDrawing * drw, byte idx) {
    VERIFY(drw);
    VERIFY(idx < drw->dim);
    drw->rfp[idx] |= 0x80000000UL;
}

/** Move the shape idx to the location (x,y)
 * \param drw (I) The pointer to the drawing descriptor
 * \param idx (I) Index of the shape to be move
 * \param x (I) horizontal coordinate of the new location
 * \param y (I) vertical coordinate of the new location
 * \pre 
 *    - drw!=0
 *    - idx<drw->dim
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
*/
void drwMove(tDrawing * drw, byte idx, word x, word y) {
    VERIFY(drw);
    VERIFY(idx < drw->dim);
    drw->rfp[idx] = CARDHL(x, y);
}

/** Return 1 if the shape idx is visible; 0 otherwise
 * \param drw (I) The pointer to the drawing descriptor
 * \param idx (I) Index of the shape
 * \pre 
 *    - drw!=0
 *    - idx<drw->dim
 * \return 1 if the shape idx is visible; 0 otherwise
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
byte drwIsVisible(const tDrawing * drw, byte idx) {
    VERIFY(drw);
    VERIFY(idx < drw->dim);
    return (drw->rfp[idx] & 0x80000000UL) ? 0 : 1;
}

/** Return the index of the shape in the drawing
 * \param drw (I) The pointer to the drawing descriptor
 * \param shp (I) Pointer to the shape
 * \pre 
 *    - drw!=0
 *    - shp!=0
 * \return the index of the shape in the drawing; drwDim(drw) otherwise
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
byte drwIdx(const tDrawing * drw, void *shp) {
    tShape *ptr;
    byte idx;

    VERIFY(drw);
    VERIFY(shp);

    ptr = drw->shp;
    idx = 0;
    while(ptr) {
        if(ptr->ptr.pix == shp)
            break;
        ptr++;
        idx++;
    }
    VERIFY(idx <= drw->dim);    //it is equal to dim if not found
    return idx;
}

/** Move a shape from the layer src in the drawing array to the layer dst
 *
 * The shapes at the beginning of the drawing array can be covered by the shapes at the end of the array while drawing
 * since they are drawn earlier.
 * This operation can be applied to put a shape on the top of the screen.
 * \param drw (I) The pointer to the drawing descriptor
 * \param src (I) Original index of the shape
 * \param dst (I) Destination index of the shape
 * \pre 
 *    - drw!=0
 *    - src<drw->dim
 *    - dst<drw->dim
 * \return the resulting index of the shape
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0127.c
*/
byte drwPush(tDrawing * drw, byte src, byte dst) {
    tShape p;
    card c;
    void *p1;
    void *p2;

    VERIFY(drw);
    VERIFY(src < drw->dim);
    VERIFY(dst < drw->dim);

    memcpy(&p, &drw->shp[src], sizeof(p));
    c = drw->rfp[src];
    if(src < dst) {
        p1 = &drw->shp[src];
        p2 = &drw->shp[src + 1];
        memmove(p1, p2, (dst - src) * sizeof(p));
        memcpy(&drw->shp[dst], &p, sizeof(p));

        p1 = &drw->rfp[src];
        p2 = &drw->rfp[src + 1];
        memmove(p1, p2, (dst - src) * sizeof(c));
        drw->rfp[dst] = c;
    }
    if(src > dst) {
        p1 = &drw->shp[dst];
        p2 = &drw->shp[dst + 1];
        memmove(p2, p1, (dst - src) * sizeof(p));
        memcpy(&drw->shp[dst], &p, sizeof(p));

        p1 = &drw->rfp[dst];
        p2 = &drw->rfp[dst + 1];
        memmove(p2, p1, (dst - src) * sizeof(c));
        drw->rfp[dst] = c;
    }
    return dst;
}

/** Button constructor: 
 * Build a button drawing.
 * \param shp (O) Pointer to the button drawing to be initialized
 * \param frm (I) Surrouding frame box (only dimensions are used, not color)
 * \param rfpBtn (I) Relative text position packed into card (CARDHL(WORDHL(xTxt,yTxt),WORDHL(xIco,yIco)))
 * \param txt (I) Text shape to be displayed on the button
 * \param ico (I) Tile shape to be displayed on the button
 * \param pal (I) button palette, array of colors: foreground, background, shadow, text
 * \pre 
 *  - shp!=0
 *  - frm!=0
 *  - frm->wdt>=4
 *  - frm->hgt>=4
 * \return 1 if OK; negative otherwise
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0128.c
*/
int drwButtonInit(tButton * shp, tBox * frm, card rfpBtn, tText * txt,
                  tTile * ico, const card * pal) {
    int ret;
    word wdt, hgt;
    word xTxt, yTxt, xIco, yIco;
    byte idx;
    static const card _pal[palBtnEnd] = { clrFgd, clrBgd, clrFgd, clrFgd };

    VERIFY(shp);
    VERIFY(frm);

    memset(shp, 0, sizeof(*shp));
    idx = 0;
    wdt = frm->wdt;
    hgt = frm->hgt;
    xTxt = HBYTE(HWORD(rfpBtn));
    yTxt = LBYTE(HWORD(rfpBtn));
    xIco = HBYTE(LWORD(rfpBtn));
    yIco = LBYTE(LWORD(rfpBtn));
    if(pal)
        shp->pal = pal;
    else
        shp->pal = _pal;

    ret = drwBarInit(&shp->shw, wdt, hgt, shp->pal[palBtnShw]);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp->shp[idx], shpBar, &shp->shw);
    CHECK(ret >= 0, lblKO);
    shp->rfp[idx++] = CARDHL(2, 2);

    ret = drwBarInit(&shp->bgd, wdt, hgt, shp->pal[palBtnBgd]);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp->shp[idx], shpBar, &shp->bgd);
    CHECK(ret >= 0, lblKO);
    shp->rfp[idx++] = CARDHL(0, 0);

    ret = drwBoxInit(&shp->frm, wdt, hgt, shp->pal[palBtnFgd]);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp->shp[idx], shpBox, &shp->frm);
    CHECK(ret >= 0, lblKO);
    shp->rfp[idx++] = CARDHL(0, 0);

    VERIFY(wdt >= 4);
    VERIFY(hgt >= 4);
    ret =
        drwBarInit(&shp->foc, (word) (wdt - 4), (word) (hgt - 4),
                   shp->pal[palBtnBgd]);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp->shp[idx], shpBar, &shp->foc);
    CHECK(ret >= 0, lblKO);
    shp->rfp[idx++] = CARDHL(2, 2);

    ret = drwShapeInit(&shp->shp[idx], shpTile, &shp->ico);
    CHECK(ret >= 0, lblKO);
    if(ico) {
        ret = drwTileInit(&shp->ico, ico->ptr, ico->wdt, ico->hgt, ico->clr);
        CHECK(ret >= 0, lblKO);
        shp->rfp[idx++] = CARDHL(xIco, yIco);
    } else
        shp->rfp[idx++] = 0x80000000UL; //hidden

    ret = drwShapeInit(&shp->shp[idx], shpText, &shp->txt);
    CHECK(ret >= 0, lblKO);
    if(txt) {
        ret =
            drwTextInit(&shp->txt, txt->cnt, txt->key, txt->fnt,
                        shp->pal[palBtnTxt]);
        CHECK(ret >= 0, lblKO);
        shp->rfp[idx++] = CARDHL(xTxt, yTxt);
    } else
        shp->rfp[idx++] = 0x80000000UL; //hidden

    ret = drwDrawingInit(&shp->drw, shp->shp, shp->rfp);
    CHECK(ret >= 0, lblKO);

    shp->rfpBtn = rfpBtn;
    shp->sta = 'n';

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

/** Button draw method.
 * Just draw the button drawing.
 * \param shp (I) The pointer to the button descriptor to be drawn
 * \param x (I) The horizontal coordinate of the reference point (0..cnvW: left->right)
 * \param y (I) The vertical coordinate of the reference point (0..cnvH: top->bottom)
 * \pre 
 *    - shp!=0
 * \return 1 if drawn, 0 if not, negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0128.c
*/
int drwButtonDraw(tButton * shp, word x, word y) {
    VERIFY(shp);
    return drwDrawingDraw(&shp->drw, x, y);
}

static card drwButtonRel(const tButton * btn, int x, int y) {
    VERIFY(btn);
    return drwBoxRel(&btn->frm, x, y);
}

/** Set button state.
 * Button is redrawn on the canvas then.
 * \param btn (I) The pointer to the button descriptor
 * \param sta (I) The new button state, one of the following:
 *  - 'n': normal state
 *  - 'f': focused button
 *  - 'p': pushed button
 * \pre 
 *    - btn !=0
 * \return 1 if drawn, 0 if not, negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0128.c
*/
int btnState(tButton * btn, char sta) {
    word xTxt, yTxt, xIco, yIco;
    card mskTxt, mskIco;

    VERIFY(btn);

    xTxt = HBYTE(HWORD(btn->rfpBtn));
    yTxt = LBYTE(HWORD(btn->rfpBtn));
    xIco = HBYTE(LWORD(btn->rfpBtn));
    yIco = LBYTE(LWORD(btn->rfpBtn));
    mskTxt = btn->rfp[idxBtnText] & 0x80000000UL;   //visibility bit for text
    mskIco = btn->rfp[idxBtnIcon] & 0x80000000UL;   //visibility bit for icon
    btn->sta = sta;
    switch (sta) {
      case 'f':
          if(btn->txt.cnt)
              btn->txt.clr = btn->pal[1];
          if(btn->ico.ptr)
              if(btn->ico.clr >= 0)
                  btn->ico.clr = btn->pal[1];
          btn->foc.clr = btn->pal[0];
          btn->rfp[idxBtnFrame] = CARDHL(0, 0);
          btn->rfp[idxBtnIcon] = CARDHL(xIco, yIco);
          btn->rfp[idxBtnText] = CARDHL(xTxt, yTxt);
          btn->rfp[idxBtnFocus] = CARDHL(2, 2);
          btn->shw.clr = btn->pal[2];
          break;
      case 'p':
          if(btn->txt.cnt)
              btn->txt.clr = btn->pal[1];
          if(btn->ico.ptr)
              if(btn->ico.clr >= 0)
                  btn->ico.clr = btn->pal[1];
          btn->foc.clr = btn->pal[0];
          btn->rfp[idxBtnFrame] = CARDHL(0 + 2, 0 + 2);
          btn->rfp[idxBtnIcon] = CARDHL(xIco + 2, yIco + 2);
          btn->rfp[idxBtnText] = CARDHL(xTxt + 2, yTxt + 2);
          btn->rfp[idxBtnFocus] = CARDHL(2 + 2, 2 + 2);
          btn->shw.clr = btn->pal[1];
          break;
      default:
          VERIFY(sta == 'n');
          if(btn->txt.cnt)
              btn->txt.clr = btn->pal[palBtnTxt];
          if(btn->ico.ptr)
              if(btn->ico.clr >= 0)
                  btn->ico.clr = btn->pal[0];
          btn->foc.clr = btn->pal[1];
          btn->rfp[idxBtnFrame] = CARDHL(0, 0);
          btn->rfp[idxBtnIcon] = CARDHL(xIco, yIco);
          btn->rfp[idxBtnText] = CARDHL(xTxt, yTxt);
          btn->rfp[idxBtnFocus] = CARDHL(2, 2);
          btn->shw.clr = btn->pal[2];
          break;
    }
    btn->rfp[idxBtnIcon] |= mskIco;
    btn->rfp[idxBtnText] |= mskTxt;
    return drwDrawingRefresh(&btn->drw, drwDim(&btn->drw));
}

static void dly(byte d) {
    tmrStart(0, d);
    while(tmrGet(0));
    tmrStop(0);
}

/** Push the button.
 *
 * At first, the focus is put on the button.
 *
 * Then it is pushed (in bottom right direction, to the shadow position).
 *
 * Then it is returned to the focused state.
 * \param btn (I) The pointer to the button descriptor
 * \pre 
 *    - btn !=0
 * \return 1 if OK, negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0128.c
*/
#ifdef WIN32
#define DLY 1
#else
#define DLY 1
#endif
int btnPush(tButton * btn) {
    int ret;

    VERIFY(btn);

    ret = btnState(btn, 'f');
    CHECK(ret >= 0, lblKO);
    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);
    dly(DLY);

    ret = btnState(btn, 'p');
    CHECK(ret >= 0, lblKO);
    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);
    dly(DLY);

    ret = btnState(btn, 'f');
    CHECK(ret >= 0, lblKO);
    ret = cnvDraw();
    CHECK(ret >= 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

/** redraw the button at the same location
 * \param btn (I) The pointer to the button descriptor to be redrawn
 * \pre 
 *    - shp!=0
 * \return 1 if drawn, 0 if not, negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0128.c
*/
int btnRefresh(tButton * btn) {
    VERIFY(btn);
    return drwDrawingRefresh(&btn->drw, drwDim(&btn->drw));
}

/** Selector drawing constructor: 
 * Build a selector drawing.
 * \param shp (O) Pointer to the selector drawing to be initialized
 * \param bgd (I) background bar
 * \param foc (I) focus box
 * \param pal (I) menu palette, array of colors: color palette: foreground, background, highlight bar, highlight text, focus box, item font
 * \param sld (I) associated slider
 *
 * \pre 
 *  - shp!=0
 *  - bgd!=0
 *  - foc!=0
 * \return 1 if OK; negative otherwise
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0128.c
 *
 * If pal is zero a default palette is used
 *
*/
int drwSelectorInit(tSelector * shp, tBar * bgd, tBox * foc, tSlider * sld,
                    const card * pal) {
    int ret;
    byte idx, row, dim;
    static const card _pal[] = { clrFgd, clrFgd, clrBgd, 0 };
    VERIFY(shp);
    VERIFY(bgd);
    VERIFY(foc);

    memset(shp, 0, sizeof(*shp));
    if(pal)
        shp->pal = pal;
    else
        shp->pal = _pal;
    shp->sld = sld;

    idx = 0;
    VERIFY(idx == idxSelBgd);
    ret = drwBarInit(&shp->bgd, bgd->wdt, bgd->hgt, bgd->clr);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp->shp[idx], shpBar, &shp->bgd);
    CHECK(ret >= 0, lblKO);
    shp->rfp[idx] = CARDHL(0, 0);

    idx++;
    VERIFY(idx == idxSelBak);
    ret = drwBarInit(&shp->bak, foc->wdt, foc->hgt, shp->pal[palSelBak]);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp->shp[idx], shpBar, &shp->bak);
    CHECK(ret >= 0, lblKO);
    shp->rfp[idx] = CARDHL(0, 0);

    idx++;
    VERIFY(idx == idxSelFoc);
    ret = drwBoxInit(&shp->foc, foc->wdt, foc->hgt, foc->clr);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp->shp[idx], shpBox, &shp->foc);
    CHECK(ret >= 0, lblKO);
    shp->rfp[idx] = CARDHL(0, 0);

    VERIFY(foc->hgt);
    dim = (bgd->hgt) / (foc->hgt);
    VERIFY(dim);
    for (row = 0; row < dim; row++) {
        idx++;
        ret = cntInit(&shp->cnt[row], 's', shp->itm[row]);
        CHECK(ret >= 0, lblKO);
        ret =
            drwTextInit(&shp->txt[row], &shp->cnt[row], 0, shp->pal[palSelFnt],
                        shp->pal[palSelFgd]);
        CHECK(ret >= 0, lblKO);
        ret = drwShapeInit(&shp->shp[idx], shpText, &shp->txt[row]);
        CHECK(ret >= 0, lblKO);
        shp->rfp[idx] = CARDHL(1, (row + 1) * (foc->hgt));
    }

    ret = drwDrawingInit(&shp->drw, shp->shp, shp->rfp);
    CHECK(ret >= 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

/** Selector draw method.
 * Just draw the selector drawing.
 * \param shp (I) The pointer to the selector descriptor to be drawn
 * \param x (I) The horizontal coordinate of the reference point (0..cnvW: left->right)
 * \param y (I) The vertical coordinate of the reference point (0..cnvH: top->bottom)
 * \pre 
 *    - shp!=0
 * \return 1 if drawn, 0 if not, negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0128.c
*/
int drwSelectorDraw(tSelector * shp, word x, word y) {
    VERIFY(shp);
    return drwDrawingDraw(&shp->drw, x, y);
}

byte selDim(const tSelector * sel) {
    VERIFY(sel);
    VERIFY(sel->foc.hgt);
    return (sel->bgd.hgt) / (sel->foc.hgt);
}

//calculate associated slider dimensions based on selector and the size of item list
static int setSelectorSlider(tSelector * sel, word dim, card pos) {
    int ret;
    word lstDim, lstTop, lstNum;    //number of items, index of the item displayed at the top, number of items displayed
    word sldDim, sldTop, sldNum;    //slider background height, upper edge of slider, slider foreground height
    tBar beg, end;
    tSlider *sld;
    card loc;

    VERIFY(sel);
    //The result predicates:
    // lstNum/lstDim == sldNum/sldDim (slider foreground size)
    // lstTop/lstDim == sldTop/sldDim (slider foreground position)
    sld = sel->sld;

    if(!sld)
        return 0;

    trcS("setSelectorSlider:");
    VERIFY(sel->foc.hgt);
    //retrieve input values
    lstDim = dim;               //total number of items to select
    lstTop = HWORD(pos);        //top item index
    lstNum = (sel->bgd.hgt) / (sel->foc.hgt);   //number of items that are displayed
    sldDim = sld->bgd.hgt;      //slider background height

    VERIFY(lstDim);
    //calculate output values
    sldTop = (lstTop * sldDim) / lstDim;
    sldNum = (lstNum * sldDim) / lstDim;

    trcFN(" lstDim=%d", lstDim);
    trcFN(" lstNum=%d", lstNum);
    trcFN(" lstTop=%d", lstTop);
    trcFN(" sldDim=%d", sldDim);
    trcFN(" sldNum=%d", sldNum);
    trcFN(" sldTop=%d\n", sldTop);

    if(!sldNum)
        sldNum = 1;
    VERIFY(sldNum <= sldDim);
    //set output values
    ret = drwBarInit(&beg, sld->fgd.wdt, sldNum, sld->fgd.clr);
    CHECK(ret >= 0, lblKO);
    ret = drwBarInit(&end, sld->fgd.wdt, sldNum, sld->bgd.clr);
    CHECK(ret >= 0, lblKO);
    loc = sld->drw.loc;
    drwSliderInit(sld, &beg, &end, 0, (word) (sldDim - sldNum));
    sld->drw.loc = loc;

    ret = sldPos(sld, sldTop);
    CHECK(ret >= 0, lblKO);
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

static byte selClear(tSelector * sel, byte reset) {
    byte idx, dim;

    dim = selDim(sel);          //maximum number of items that can be displayed via selector
    VERIFY(dim <= dspH);
    for (idx = 0; idx < dim; idx++) {
        if(reset)
            memset(sel->itm[idx], 0, dspW + 1); //reset items
        sel->txt[idx].clr = sel->pal[palSelFgd];    //reset colors
    }
    return dim;
}

/** Set selector position.
 * The selector items array is provided to be displayed via selector.
 * \param sel (I) The pointer to the selector descriptor
 * \param str (I) The array of selector items (zero-ended)
 * \param pos (I) Selector position = (top,cur)
 *
 * The str array contains the full set of items in the menu.
 * Only selDim(sel) of them are displayed via selector.
 *
 * The position consists of two bytes:
 *  - HBYTE(pos) is the row in str to be displayed at the top of the selector
 *  - LBYTE(pos) is the row to be highlighted
 * \pre 
 *    - shp!=0
 *    - str!=0
 *    - str should contain at least one item
 *    - top and idx should be in the range
 *    - top<=idx
 *    - (idx-top) should be less than or equal to selector height
 * \return 1 if OK, 0 if not, negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0128.c
 * 
*/
int selPos(tSelector * sel, const Pchar * str, word pos) {
    int ret;
    byte dim, top, idx, num, row;

    VERIFY(sel);
    VERIFY(str);

    dim = selClear(sel, 1);     //returns maximum number of items that can be displayed via selector
    num = 0;                    //calculate number of items in str array
    while(str[num])
        num++;
    VERIFY(num);

    if(dim > num)
        dim = num;              //short menu, only a part of the window will be filled

    top = HBYTE(pos);
    idx = LBYTE(pos);
    VERIFY(top <= idx);
    VERIFY(idx < top + dim);

    row = idx - top;            //the index of the row to be highlighted
    sel->txt[row].clr = sel->pal[palSelCur];    //modify highlighted item color
    sel->rfp[idxSelBak] = CARDHL(0, row * (sel->foc.hgt));  //move highlight bar
    sel->rfp[idxSelFoc] = CARDHL(0, row * (sel->foc.hgt));  //move focus box
    sel->pos = pos;

    VERIFY(dim <= dspH);
    for (idx = 0; idx < dim; idx++) {
        VERIFY(strlen(str[top + idx]) <= dspW);
        strcpy(sel->itm[idx], str[top + idx]);
    }

    setSelectorSlider(sel, num, CARDHL(top, idx));

    ret = drwDrawingRefresh(&sel->drw, (byte) drwDim(&sel->drw));
    CHECK(ret >= 0, lblKO);

    if(sel->sld) {
        ret = drwDrawingRefresh(&sel->sld->drw, (byte) drwDim(&sel->sld->drw));
        CHECK(ret >= 0, lblKO);
    }

    return row;
  lblKO:
    return -1;
}

/** Set browser position.
 * \param sel (I) The pointer to the selector descriptor
 * \param pos (I) Selector position = (top,cur)
 * \param max (I) maximal number of items in browser
 *
 * Only selDim(sel) of them are displayed via selector.
 *
 * The position consists of two bytes:
 *  - HWORD(pos) is the row number to be displayed at the top of the selector
 *  - LWORD(pos) is the row number to be highlighted
 * \pre 
 *    - shp!=0
 *    - str should contain at least one item
 *    - top and idx should be in the range
 *    - top<=idx
 *    - (idx-top) should be less than or equal to selector height
 * \return 1 if OK, 0 if not, negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0194.c
 * 
*/
int brwPos(tSelector * sel, card pos, word max) {
    int ret;
    word dim, top, idx, num, row;

    VERIFY(sel);

    dim = selClear(sel, 0);     //returns maximum number of items that can be displayed via selector
    num = max;                  //number of items in browser
    //VERIFY(num);

    if(dim > num)
        dim = num;              //short menu, only a part of the window will be filled

    top = HWORD(pos);
    idx = LWORD(pos);
    if(dim) {
        VERIFY(top <= idx);
        VERIFY(idx < top + dim);
    } else {
        VERIFY(top == 0);
        VERIFY(idx == 0);
        VERIFY(sel->sld == 0);
    }

    row = idx - top;            //the index of the row to be highlighted
    sel->txt[row].clr = sel->pal[palSelCur];    //modify highlighted item color
    sel->rfp[idxSelBak] = CARDHL(0, row * (sel->foc.hgt));  //move highlight bar
    sel->rfp[idxSelFoc] = CARDHL(0, row * (sel->foc.hgt));  //move focus box
    sel->pos = (word) pos;

    setSelectorSlider(sel, num, CARDHL(top, idx));

    ret = drwDrawingRefresh(&sel->drw, (byte) drwDim(&sel->drw));
    CHECK(ret >= 0, lblKO);

    if(sel->sld) {
        ret = drwDrawingRefresh(&sel->sld->drw, (byte) drwDim(&sel->sld->drw));
        CHECK(ret >= 0, lblKO);
    }

    return row;
  lblKO:
    return -1;
}

word selWhere(tSelector * sel, card loc) {
    word idx;

    VERIFY(sel);
    loc = drwShapeRel(&sel->shp[idxSelBgd], HWORD(loc), LWORD(loc));
    if(loc == RELOUT)           //touch point out of bgd
        return selDim(sel);
    VERIFY(sel->foc.hgt);
    idx = LWORD(loc) / sel->foc.hgt;
    VERIFY(idx < selDim(sel));
    return idx;
}

/** redraw the selector at the same location
 * \param sel (I) The pointer to the selector descriptor to be redrawn
 * \pre 
 *    - shp!=0
 * \return 1 if drawn, 0 if not, negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0128.c
*/
int selRefresh(tSelector * sel) {
    VERIFY(sel);
    return drwDrawingRefresh(&sel->drw, drwDim(&sel->drw));
}

#undef MIN
#undef MAX
#define MIN(A,B) ((A)<(B))? (A):(B)
#define MAX(A,B) ((A)<(B))? (B):(A)
/** Slider constructor: 
 * Build a slider drawing.
 * \param shp (O) Pointer to the slider drawing to be initialized
 * \param beg (I) Initial slider state
 * \param end (I) Final slider state
 * \param dx (I) Horizontal shift
 * \param dy (I) Vertical shift
 *
 * \remark the color of beg is used for slider color; the color of end is used as background color
 *
 * Slider moves from its initial state beg to the state end shifted to (dx,dy)
 * \pre 
 *  - shp!=0
 *  - beg!=0
 *  - end!=0
 * \return 1 if OK; negative otherwise
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0128.c
*/
int drwSliderInit(tSlider * shp, tBar * beg, tBar * end, int dx, int dy) {
    int ret;
    int x1, y1, x2, y2;         //surrounding corners
    int sx, sy;                 //normalizing shift
    word wdt, hgt;              //background width and height

    VERIFY(shp);
    VERIFY(beg);
    VERIFY(end);

    trcS("drwSliderInit\n");

    memset(shp, 0, sizeof(*shp));

    //corners of initial slider
    shp->bx1 = 0;
    shp->by1 = 0;
    shp->bx2 = shp->bx1 + beg->wdt - 1;
    shp->by2 = shp->by1 + beg->hgt - 1;

    //corners of final slider
    shp->ex1 = dx;
    shp->ey1 = dy;
    shp->ex2 = shp->ex1 + end->wdt - 1;
    shp->ey2 = shp->ey1 + end->hgt - 1;

    //corners of surrounding rectangle
    x1 = MIN(shp->bx1, shp->ex1);
    y1 = MIN(shp->by1, shp->ey1);
    x2 = MAX(shp->bx2, shp->ex2);
    y2 = MAX(shp->by2, shp->ey2);

    VERIFY(x1 <= x2);
    VERIFY(y1 <= y2);

    //slider dimension
    ret = shp->ex1 - shp->bx1;
    if(ret < 0)
        ret = -ret;
    shp->dim = MAX(shp->dim, ret);

    ret = shp->ex2 - shp->bx2;
    if(ret < 0)
        ret = -ret;
    shp->dim = MAX(shp->dim, ret);

    ret = shp->ey1 - shp->by1;
    if(ret < 0)
        ret = -ret;
    shp->dim = MAX(shp->dim, ret);

    ret = shp->ey2 - shp->by2;
    if(ret < 0)
        ret = -ret;
    shp->dim = MAX(shp->dim, ret);

    //shift to the origin: (x1,y1,x2,y2)->(0,0,wdt-1,hgt-1)
    sx = -x1;
    sy = -y1;
    VERIFY(x1 <= x2);
    VERIFY(y1 <= y2);

    x2 += sx;
    y2 += sy;
    wdt = x2 - x1 + 1;
    hgt = y2 - y1 + 1;

    shp->bx1 += sx;
    shp->by1 += sy;
    shp->bx2 += sx;
    shp->by2 += sy;

    VERIFY(shp->bx1 >= 0);
    VERIFY(shp->by1 >= 0);
    VERIFY(shp->bx2 >= 0);
    VERIFY(shp->by2 >= 0);

    shp->ex1 += sx;
    shp->ey1 += sy;
    shp->ex2 += sx;
    shp->ey2 += sy;

    VERIFY(shp->ex1 >= 0);
    VERIFY(shp->ey1 >= 0);
    VERIFY(shp->ex2 >= 0);
    VERIFY(shp->ey2 >= 0);

    //initialize shapes
    ret = drwBarInit(&shp->bgd, wdt, hgt, end->clr);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp->shp[sldBgd], shpBar, &shp->bgd);
    CHECK(ret >= 0, lblKO);
    shp->rfp[sldBgd] = CARDHL(0, 0);

    ret = drwBarInit(&shp->fgd, beg->wdt, beg->hgt, beg->clr);
    CHECK(ret >= 0, lblKO);
    ret = drwShapeInit(&shp->shp[sldFgd], shpBar, &shp->fgd);
    CHECK(ret >= 0, lblKO);
    shp->rfp[sldFgd] = CARDHL(shp->bx1, shp->by1);

    ret = drwDrawingInit(&shp->drw, shp->shp, shp->rfp);
    CHECK(ret >= 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

/** Slider draw method.
 * Just draw the slider drawing.
 * \param shp (I) The pointer to the slider descriptor to be drawn
 * \param x (I) The horizontal coordinate of the reference point (0..cnvW: left->right)
 * \param y (I) The vertical coordinate of the reference point (0..cnvH: top->bottom)
 * \pre 
 *    - shp!=0
 * \return 1 if drawn, 0 if not, negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0128.c
*/
int drwSliderDraw(tSlider * shp, word x, word y) {
    VERIFY(shp);
    trcFN("drwSliderDraw x=%d", x);
    trcFN(" y=%d\n", y);
    return drwDrawingDraw(&shp->drw, x, y);
}

/** Getter: return slider dimension
 * \param sld (I) The pointer to the slider descriptor
 * \pre 
 *    - sld!=0
 * \return slider dimension
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0128.c
*/
word sldDim(const tSlider * sld) {
    VERIFY(sld);
    return sld->dim;
}

/** Move slider to a given position
 * \param sld (I) The pointer to the slider descriptor
 * \param pos (I) Position to move to
 * \pre 
 *    - sld!=0
 *    - pos<=sldDim(sld)
 * \return 1 if drawn, 0 if not, negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0128.c
*/
word sldPos(tSlider * sld, word pos) {
    int ret;
    word x, y, x1, y1, x2, y2;

    VERIFY(sld);
    VERIFY(pos <= sld->dim);
    trcFN("sldPos pos=%d\n", pos);

    x = HWORD(sld->drw.loc);
    y = LWORD(sld->drw.loc);

    //erase previous state    
    ret = drwSliderDraw(sld, x, y);
    CHECK(ret >= 0, lblKO);

    //interpolate corners
    VERIFY(sld->dim);
    x1 = sld->bx1 + ((sld->ex1 - sld->bx1) * pos) / sld->dim;
    y1 = sld->by1 + ((sld->ey1 - sld->by1) * pos) / sld->dim;
    x2 = sld->bx2 + ((sld->ex2 - sld->bx2) * pos) / sld->dim;
    y2 = sld->by2 + ((sld->ey2 - sld->by2) * pos) / sld->dim;

    //resize and position the slider bar
    sld->fgd.wdt = x2 - x1 + 1;
    sld->fgd.hgt = y2 - y1 + 1;
    sld->rfp[sldFgd] = CARDHL(MIN(x1, x2), MIN(y1, y2));

    ret = drwDrawingRefresh(&sld->drw, sldEnd);
    CHECK(ret >= 0, lblKO);

    ret = 1;
    goto lblEnd;
  lblKO:
    ret = -1;
  lblEnd:
    return ret;
}

/** redraw the slider at the same location
 * \param sld (I) The pointer to the slider descriptor to be redrawn
 * \pre 
 *    - sld!=0
 * \return 1 if drawn, 0 if not, negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0128.c
*/
int sldRefresh(tSlider * sld) {
    VERIFY(sld);
    return drwDrawingRefresh(&sld->drw, sldEnd);
}

/** Video constructor.
 * Video is a drawing where only two shapes are visible:
 *  - the first shape (0) is used as a background
 *  - the current shape
 *
 * Usually all shapes have the same size, and the first one is a bar of background color.
 * \param shp (O) Pointer to the slider drawing to be initialized
 * \param drw (I) Drawing containing backround image and animation shapes
 * \pre 
 *  - shp!=0
 *  - drw!=0
 *  - drwDim(drw)>=2
 * \return drawing dimension if OK; negative otherwise
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0128.c
*/
int drwVideoInit(tVideo * shp, tDrawing * drw) {
    byte idx;

    VERIFY(shp);
    VERIFY(drw);
    VERIFY(drwDim(drw) >= 2);

    shp->drw = drw;
    drwShow(drw, 0);
    drwShow(drw, 1);
    shp->pos = 1;
    idx = drwDim(drw);
    while(idx > 1)
        drwHide(drw, --idx);

    return drwDim(drw);
}

/** Video draw method.
 * Just draw the video drawing.
 * \param shp (I) The pointer to the button descriptor to be drawn
 * \param x (I) The horizontal coordinate of the reference point (0..cnvW: left->right)
 * \param y (I) The vertical coordinate of the reference point (0..cnvH: top->bottom)
 * \pre 
 *    - shp!=0
 * \return 1 if drawn, 0 if not, negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0128.c
*/
int drwVideoDraw(tVideo * shp, word x, word y) {
    VERIFY(shp);
    trcFN("drwVideoDraw x=%d", x);
    trcFN(" y=%d\n", y);
    return drwDrawingDraw(shp->drw, x, y);
}

/** Getter: return video drawing dimension
 * \param vid (I) The pointer to the video descriptor
 * \remark the video position can take values from 1 to vidDim(vid); the zero frame being background
 * \pre 
 *    - vid!=0
 * \return video drawing dimension
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0128.c
*/
word vidDim(const tVideo * vid) {
    VERIFY(vid);
    return drwDim(vid->drw);
}

/** Set video to a given position
 * \param vid (I) The pointer to the video descriptor
 * \param pos (I) Position to move to
 * \remark the video position can take values from 0 to vidDim(vid)-1; the zero frame being background
 * \pre 
 *    - vid!=0
 *    - pos<=vidDim(vid)
 *    - pos<256
 * \return 1 if drawn, 0 if not, negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0128.c
*/
word vidPos(tVideo * vid, word pos) {
    VERIFY(vid);
    //VERIFY(pos);
    VERIFY(pos < vidDim(vid));
    VERIFY(pos < 256);

    if(vid->pos)
        drwHide(vid->drw, vid->pos);    //background is never hidden
    vid->pos = (byte) pos;
    drwShow(vid->drw, vid->pos);

    return pos;
}

/** redraw the video at the same location
 * \param vid (I) The pointer to the video descriptor to be redrawn
 * \pre 
 *    - vid!=0
 * \return 1 if drawn, 0 if not, negative in case of error
 * \sa Header: pri\\pri.h
 * \sa Source: pri\\drw.c
 * \test tcab0128.c
*/
int vidRefresh(tVideo * vid) {
    VERIFY(vid);
    return drwDrawingRefresh(vid->drw, drwDim(vid->drw));
}
#endif
