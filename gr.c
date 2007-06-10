/**
 * gr.c
 */
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include "gr.h"
#include "fixed8.h"

#define ABS(a) (((a) < 0) ? (-(a)) : (a))
#define VPSTKDEP 4

#ifdef __arm__
# define INVERTFB	1
#endif

static SDL_Surface *sf = NULL;
#ifdef INVERTFB
static void *pixelsend = NULL;
#endif /* INVERTFB */
static rect_t ua, vps[VPSTKDEP], *vp = vps;
static unsigned grCurColor = 0;
static struct {
	int x, y;
} pos;

/**
 * Swaps two integers.
 */
static void
isw(int *a, int *b)
{
	int c = *a;
	*a = *b;
	*b = c;
}

/**
 * Makes a color according to the current pixel format from the
 * specified RGB values.
 */
int (*grRGB)(int r, int g, int b) = NULL;

static int
grRGB15(int r, int g, int b)
{
	return	(((r >> 3) & 0x1f) << 10) |
		(((g >> 3) & 0x1f) << 5) |
		(((b >> 3) & 0x1f));
}

static int
grRGB16(int r, int g, int b)
{
	return	(((r >> 3) & 0x1f) << 11) |
		(((g >> 2) & 0x3f) << 5) |
		(((b >> 3) & 0x1f));
}

static int
grRGB24(int r, int g, int b)
{
	return	((r & 0xff) << 16) |
		((g & 0xff) << 8) |
		((b & 0xff));
}

/**
 * Set up a viewport over the drawing area.
 * After setting up a viewport, all drawing routines will interpret
 * their parameters as relative coorditates to the left upper corner
 * of the current viewport.
 */
void
grSetViewPort(int x1, int y1, int x2, int y2)
{
	if (!x1 && !x2 && !y1 && !y2) {
		x2 = sf->w - 1;
		y2 = sf->h - 1;
	}
	if (x1 > x2) isw(&x1, &x2);
	if (y1 > y2) isw(&y1, &y2);
	if (x2 < 0 || y2 < 0 || x1 >= sf->w || y1 >= sf->h) return;
	if (x1 < 0) x1 = 0;
	if (y1 < 0) y1 = 0;
	if (x2 >= sf->w) x2 = sf->w - 1;
	if (y2 >= sf->h) y2 = sf->h - 1;
	vp->x1 = x1;
	vp->y1 = y1;
	vp->x2 = x2;
	vp->y2 = y2;
}

int
grPushViewPort()
{
	if (vp < vps + VPSTKDEP - 1) {
		++vp;
		*vp = vp[-1];
		return !0;
	}
	return 0;
}

int
grPopViewPort()
{
	if (vp > vps) {
		--vp;
		return !0;
	}
	return 0;
}

static void
grExtendUpdateArea(int x, int y)
{
	if (x < ua.x1) ua.x1 = x;
	if (x > ua.x2) ua.x2 = x;
	if (y < ua.y1) ua.y1 = y;
	if (y > ua.y2) ua.y2 = y;
}

/* SetPixel */
static void
grSetPixel8(unsigned offs)
{
#ifdef INVERTFB
	char *p = pixelsend - offs;
#else /* INVERTFB */
	char *p = sf->pixels + offs;
#endif /* INVERTFB */
	*p = grCurColor;
}

static void
grSetPixel16(unsigned offs)
{
#ifdef INVERTFB
	short *p = pixelsend - (offs << 1);
#else /* INVERTFB */
	short *p = sf->pixels + (offs << 1);
#endif /* INVERTFB */
	*p = grCurColor;
}

static void
grSetPixel24(unsigned offs)
{
#ifdef INVERTFB
	char *p = pixelsend - (offs * 3);
#else /* INVERTFB */
	char *p = sf->pixels + (offs * 3);
#endif /* INVERTFB */
	*p = grCurColor & 0xffffff;
}

static void
grSetPixel32(unsigned offs)
{
#ifdef INVERTFB
	int *p = pixelsend - (offs << 2);
#else /* INVERTFB */
	int *p = sf->pixels + (offs << 2);
#endif /* INVERTFB */
	*p = grCurColor;
}

/* XorPixel */
static void
grXorPixel8(unsigned offs)
{
#ifdef INVERTFB
	char *p = pixelsend - offs;
#else /* INVERTFB */
	char *p = sf->pixels + offs;
#endif /* INVERTFB */
	*p ^= grCurColor;
}

static void
grXorPixel16(unsigned offs)
{
#ifdef INVERTFB
	short *p = pixelsend - (offs << 1);
#else /* INVERTFB */
	short *p = sf->pixels + (offs << 1);
#endif /* INVERTFB */
	*p ^= grCurColor;
}

static void
grXorPixel24(unsigned offs)
{
#ifdef INVERTFB
	char *p = pixelsend - (offs * 3);
#else /* INVERTFB */
	char *p = sf->pixels + (offs * 3);
#endif /* INVERTFB */
	*p ^= grCurColor & 0xffffff;
}

static void
grXorPixel32(unsigned offs)
{
#ifdef INVERTFB
	int *p = pixelsend - (offs << 2);
#else /* INVERTFB */
	int *p = sf->pixels + (offs << 2);
#endif /* INVERTFB */
	*p ^= grCurColor;
}

typedef void (*grDrawPixelFunc_t)(unsigned);
typedef grDrawPixelFunc_t grDrawPixelSet_t[PMD_LAST];

static grDrawPixelSet_t grDrawPixelSet8 = {
	grSetPixel8, grXorPixel8
};

static grDrawPixelSet_t grDrawPixelSet16 = {
	grSetPixel16, grXorPixel16
};

static grDrawPixelSet_t grDrawPixelSet24 = {
	grSetPixel24, grXorPixel24
};

static grDrawPixelSet_t grDrawPixelSet32 = {
	grSetPixel32, grXorPixel32
};

static grDrawPixelSet_t *grDrawPixelSet = NULL;
static grDrawPixelFunc_t grDrawPixel = NULL;
static pixelmode_t grPixelMode = PMD_SET;

void
grSetPixelMode(pixelmode_t pm)
{
	if (pm < PMD_FIRST) pm = PMD_FIRST;
	if (pm >= PMD_LAST) pm = PMD_LAST - 1;
	grPixelMode = pm;
	if (sf == NULL) return;
	grDrawPixel = (*grDrawPixelSet)[grPixelMode];
}

void
grPutPixel(int x, int y)
{
	x += vp->x1;  y += vp->y1;
	if (sf == NULL || x < vp->x1 || y < vp->y1 || x > vp->x2 || y > vp->y2) return;
	grDrawPixel(y * sf->w + x);
	grExtendUpdateArea(x, y);
}

void
grBar(int x1, int y1, int x2, int y2)
{
	x1 += vp->x1;  y1 += vp->y1;
	x2 += vp->x1;  y2 += vp->y1;
	if (x1 > x2) isw(&x1, &x2);
	if (y1 > y2) isw(&y1, &y2);
	if (sf == NULL || x1 > vp->x2 || y1 > vp->y2 || x2 < vp->x1 || y2 < vp->y1) return;
	if (x1 < vp->x1) x1 = vp->x1;
	if (y1 < vp->y1) y1 = vp->y1;
	if (x2 > vp->x2) x2 = vp->x2;
	if (y2 > vp->y2) y2 = vp->y2;
	unsigned d1 = x2 - x1 + 1;
	unsigned d2 = sf->w - d1;
	unsigned offs = y1 * sf->w + x1;
	unsigned x, y;
	for (y = y2 - y1 + 1; y; --y) {
		for (x = d1; x; --x) grDrawPixel(offs++);
		offs += d2;
	}
	grExtendUpdateArea(x1, y1);
	grExtendUpdateArea(x2, y2);
}

void
grHorizLine(int x1, int y, int x2)
{
	x1 += vp->x1;
	y += vp->y1;
	x2 += vp->x1;
	if (x1 > x2) isw(&x1, &x2);
	if (sf == NULL || x2 < vp->x1 || x1 > vp->x2 || y < vp->y1 || y > vp->y2) return;
	if (x1 < vp->x1) x1 = vp->x1;
	if (x2 > vp->x2) x2 = vp->x2;
	unsigned offs = y * sf->w + x1;
	int x;
	for (x = x2 - x1 + 1; x; --x) grDrawPixel(offs++);
	grExtendUpdateArea(x1, y);
	grExtendUpdateArea(x2, y);
}

void
grVertLine(int x, int y1, int y2)
{
	x += vp->x1;
	y1 += vp->y1;
	y2 += vp->y1;
	if (y1 > y2) isw(&y1, &y2);
	if (sf == NULL || y2 < vp->y1 || y1 > vp->y2 || x < vp->x1 || x > vp->x2) return;
	if (y1 < vp->y1) y1 = vp->y1;
	if (y2 > vp->y2) y2 = vp->y2;
	unsigned offs = y1 * sf->w + x;
	int y;
	for (y = y2 - y1 + 1; y; --y) {
		grDrawPixel(offs);
		offs += sf->w;
	}
	grExtendUpdateArea(x, y1);
	grExtendUpdateArea(x, y2);
}

int
grLine45(int x1, int y1, int x2, int y2)
{
	x1 += vp->x1;  y1 += vp->y1;
	x2 += vp->x1;  y2 += vp->y1;
	if ((x1 + y1 != x2 + y2) && (x1 - y1 != x2 - y2)) return 0;
	if (x1 > x2) {
		isw(&x1, &x2);
		isw(&y1, &y2);
	}
	if (sf == NULL) return 1;
	if (y2 > y1) { /* lefele megy */
		if (x1 < vp->x1) {
			y1 += vp->x1 - x1;
			x1 = vp->x1;
		}
		if (y1 < vp->y1) {
			x1 += vp->y1 - y1;
			y1 = vp->y1;
		}
		if (x2 > vp->x2) {
			y2 -= x2 - vp->x2;
			x2 = vp->x2;
		}
		if (y2 > vp->y2) {
			x2 -= y2 - vp->y2;
			y2 = vp->y2;
		}
		if (y2 < vp->y1 || y1 > vp->y2 || x2 < vp->x1 || x1 > vp->x2) return 1;
		unsigned offs = y1 * sf->w + x1;
		int x;
		for (x = x2 - x1 + 1; x; --x) {
			grDrawPixel(offs);
			offs += sf->w + 1;
		}
	} else { /* felfele megy */
		if (x1 < vp->x1) {
			y1 -= vp->x1 - x1;
			x1 = vp->x1;
		}
		if (y1 > vp->y2) {
			x1 += y1 - vp->y2;
			y1 = vp->y2;
		}
		if (x2 > vp->x2) {
			y2 += x2 - vp->x2;
			x2 = vp->x2;
		}
		if (y2 < vp->y1) {
			x2 -= vp->y1 - y2;
			y2 = vp->y1;
		}
		if (y1 < vp->y1 || y2 > vp->y2 || x2 < vp->x1 || x1 > vp->x2) return 1;
		unsigned offs = y1 * sf->w + x1;
		int x;
		for (x = x2 - x1 + 1; x; --x) {
			grDrawPixel(offs);
			offs -= sf->w - 1;
		}
	}
	grExtendUpdateArea(x1, y1);
	grExtendUpdateArea(x2, y2);
	return 1;
}

void
grLine(int x1, int y1, int x2, int y2)
{
	if (y2 == y1) {
		grHorizLine(x1, y1, x2);
	} else if (x2 == x1) {
		grVertLine(x1, y1, y2);
	} else if (!grLine45(x1, y1, x2, y2)) {
		if (ABS(x2 - x1) > ABS(y2 - y1)) {
			if (x2 < x1) {
				isw(&x1, &x2);
				isw(&y1, &y2);
			}
			int y = y1 * 65536 + 32768, dy = 0;
			if (x1 != x2) dy = (y2 - y1) * 65536 / (x2 - x1);
			for (; x1 <= x2; ++x1, y += dy) grPutPixel(x1, y / 65536);
		} else {
			if (y2 < y1) {
				isw(&x1, &x2);
				isw(&y1, &y2);
			}
			int x = x1 * 65536 + 32768, dx = 0;
			if (y1 != y2) dx = (x2 - x1) * 65536 / (y2 - y1);
			for (; y1 <= y2; ++y1, x += dx) grPutPixel(x / 65536, y1);
		}
	}
}

void
grLineTo(int x, int y)
{
	grLine(pos.x, pos.y, x, y);
	grSetPos(x, y);
}

void
grClear()
{
	if (sf == NULL) return;
	grSetColor(0);
	grBar(vp->x1, vp->y1, vp->x2, vp->y2);
}

void
grSetColor(int c)
{
	grCurColor = c;
}

void
grBegin()
{
	if (sf == NULL) return;
	SDL_LockSurface(sf);
	ua.x1 = sf->w;
	ua.y1 = sf->h;
	ua.x2 = ua.y2 = 0;
}

void
grEnd()
{
	if (sf == NULL) return;
	SDL_UnlockSurface(sf);
	if (ua.x2 >= ua.x1 && ua.y2 >= ua.y1) {
		if (ua.x2 >= vp->x1 && ua.y2 >= vp->y1 && ua.x1 <= vp->x2 && ua.y1 <= vp->y2) {
			if (ua.x1 < vp->x1) ua.x1 = vp->x1;
			if (ua.y1 < vp->y1) ua.y1 = vp->y1;
			if (ua.x2 > vp->x2) ua.x2 = vp->x2;
			if (ua.y2 > vp->y2) ua.y2 = vp->y2;
#ifdef INVERTFB
			ua.y1 = sf->h - ua.y1;
			ua.y2 = sf->h - 1 - ua.y2;
			isw(&ua.y1, &ua.y2);
			ua.x1 = sf->w - ua.x1;
			ua.x2 = sf->w - 1 - ua.x2;
			isw(&ua.x1, &ua.x2);
			if (ua.x2 >= sf->w) ua.x2 = sf->w - 1;
			if (ua.y2 >= sf->h) ua.y2 = sf->h - 1;
#endif /* INVERTFB */
			unsigned w = ua.x2 - ua.x1 + 1;
			unsigned h = ua.y2 - ua.y1 + 1;
			SDL_UpdateRect(sf, ua.x1, ua.y1, w, h);
		}
	}
}

static struct {
	int bpp;
	int (*rgbfunc)(int, int, int);
	grDrawPixelSet_t *drawpixelset;
} bitdep[] = {
	{  8, NULL, &grDrawPixelSet8 },
	{ 15, grRGB15, &grDrawPixelSet16 },
	{ 16, grRGB16, &grDrawPixelSet16 },
	{ 24, grRGB24, &grDrawPixelSet24 },
	{ 32, grRGB24, &grDrawPixelSet32 },
	{ 0 }
};

int
grSetSurface(SDL_Surface *s)
{
	int i;

	if (s == NULL) return 0;
	sf = s;
	for (i = 0; bitdep[i].bpp; ++i) {
		if (bitdep[i].bpp == sf->format->BitsPerPixel) {
			grRGB = bitdep[i].rgbfunc;
			grDrawPixelSet = bitdep[i].drawpixelset;
		}
	}
	grSetPixelMode(grPixelMode);
	grSetViewPort(0, 0, sf->w - 1, sf->h - 1);
#ifdef INVERTFB
	pixelsend = sf->pixels +
		((sf->w * sf->h - 1) * sf->format->BytesPerPixel);
#endif /* INVERTFB */
	return !0;
}

void
grSetPos(int x, int y)
{
	pos.x = x;
	pos.y = y;
}

void
grPutChar(char ch)
{
	if (pos.x > -8 && pos.x < vp->x2-vp->x1 && pos.y > -16 && pos.y < vp->y2-vp->y1) {
		ch -= 32;
		if ((unsigned char)ch >= 96) ch = '?';
		int y;
		for (y = 0; y < 16; ++y) {
			int sor = fixed8[(unsigned)ch][y];
			int x;
			for (x = 0; x < 8; ++x)
				if ((sor << x) & 0x80)
					grPutPixel(pos.x + x, pos.y - y + 16);
		}
	}
	pos.x += 8;
}

void
grPutStr(const char *s)
{
	for (; *s; ++s) {
		grPutChar(*s);
	}
}

void
grprintf(const char *fmt, ...)
{
	int n, size = 128;
	char *p, *np;
	va_list ap;

	p = (char *)malloc(size);
	if (p == NULL) return;
	for (;;) {
		va_start(ap, fmt);
		n = vsnprintf(p, size, fmt, ap);
		va_end(ap);
		if (n > -1) {
			if (n < size) {
				break;
			}
			size = n + 1;
		} else {
			size *= 2;
		}
		np = (char *)realloc(p, size);
		if (np == NULL) {
			free(p);
			return;
		}
		p = np;
	}
	grPutStr(p);
	free(p);
}

void
grOctagon(int x, int y, int a)
{
	int a2 = a / 2;
	int c = a2 + 1000 * a / 1414;
	int x1 = x + a2;
	int y1 = y - c;
	int x2 = x + c;
	int y2 = y - a2;
	grLine(x1, y1, x2, y2);
	x1 = x2;
	y1 = y + a2;
	grLine(x1, y1, x2, y2);
	x2 = x + a2;
	y2 = y + c;
	grLine(x1, y1, x2, y2);
	x1 = x - a2;
	y1 = y2;
	grLine(x1, y1, x2, y2);
	x2 = x - c;
	y2 = y + a2;
	grLine(x1, y1, x2, y2);
	x1 = x2;
	y1 = y - a2;
	grLine(x1, y1, x2, y2);
	x2 = x - a2;
	y2 = y - c;
	grLine(x1, y1, x2, y2);
	x1 = x + a2;
	y1 = y2;
	grLine(x1, y1, x2, y2);
}

void
grRectangle(int x1, int y1, int x2, int y2)
{
	grHorizLine(x1, y1, x2);
	grHorizLine(x1, y2, x2);
	grVertLine(x1, y1, y2);
	grVertLine(x2, y1, y2);
}

void
grAlignMouse(int *mx, int *my)
{
	if (*mx < vp->x1) *mx = -1;
	if (*my < vp->y1) *my = -1;
	if (*mx > vp->x2) *mx = -1;
	if (*my > vp->y2) *my = -1;
	*mx -= vp->x1;
	*my -= vp->y1;
}

