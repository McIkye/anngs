/*
 * Copyright (c) 2011 Michael Shalayeff
 * All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER IN
 * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* himax hx8347-d(t) 240RGBx320 tft driver */

#define	CSEL	(PORTH &= ~_BV(PORTH4))
#define	DSEL	(PORTH |= _BV(PORTH4))

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "spi.h"
#include "hx8347.h"
#include "util.h"

enum hx8347_regs {
	/* page 0 registers */
	ID=0x00,
	NOP=0x00,		/* 00: pseudo reg for delays */
	DMODE,			/* 01: display mode */
	SCRH, SCRL,		/* 02: start column */
	ECRH, ECRL,		/* 04: end column */
	SPRH, SPRL,		/* 06: start row */
	EPRH, EPRL,		/* 08: end row */
	PSLH, PSLL,		/* 0a: partial area start */
	PELH, PELL,		/* 0c: partial area end */
	TFAH, TFAL,		/* 0e: top vertial scroll area */
	VSAH, VSAL,		/* 10: vertical scroll height */
	BFAH, BFAL,		/* 12: vertial scroll button area */
	VSPH, VSPL,		/* 14: vertical scroll start */
	MAC,			/* 16: memory access control */
	COLMOD,			/* 17: colour mode */
	OSCH, OSCL,		/* 18: oscylator conrol */
	PWC1, PWC2, PWC3, PWC4, PWC5, PWC6,
				/* 1a: power control */
	U0x21 = 0x21,
	SRAMWC,			/* 22: sram write control */
	VMF, VMH, VML,		/* 23: VCOM control */
	DC1, DC2, DC3,		/* 26: display control */
	FRC1, FRC2, FRC3, FRC4,	/* 29: frame rate control */
	GDON, GDOFF,		/* 2d: cycle control */
	DINV,			/* 2f: display inversion */
	U0x30,
	RGB1, RGB2, RGB3, RGB4,	/* 31: RGB control */
	U0x35,
	PCH,			/* 36: panel characteristics */
	U0x37,
	OTPC1, OTPC2, OTPC3,	/* 38: OTP control */
	U0x3b,
	CABC1, CABC2, CABC3, CABC4,
				/* 3c: CABC control */
				/* 40: skip gamma control */
	TEC = 0x60,		/* 60: TE control */
	U0xe3 = 0xe3,
	PS1, PS2, PS3, PS4,	/* e4: power savings control */
	OPONN, OPONI,		/* e8: source OP control */
	STBAH, STBAL,		/* ea: power control internal */
	PTBAH, PTBAL,		/* ec: source control internal */
			/* are the two above mistakenly flipped in the man? */

	/* page 1 registers */
	CABC5 = 0xc3, CABC6 = 0xc5, CABC7 = 0xc7,
				/* CABC control */
	U0xca = 0xca,
	DBG0, DBG1, DBG2, DBG3, DBG4, DBG5, DBG6, DBG7, DBG8,
				/* gain select */

	PGSEL = 0xff,		/* page select */
};

/*
 * this is basically i2c interface with two addresses;
 * 0x38 is the register index and 0x39 for the data
 *
 * dunno why the hell they wire it this way on spi bus;
 * an alternative is to use 4-pin spi mode (with d/c pin)
 *
 * also would be much faster in command mode but the manual
 * (HX8347-D(N)) is not available for some reason...
 */
void
hx8347_cmd(char cmd, char data)
{
	CSEL;
	spi_wrb(0x70);
	spi_wrb(cmd);
	DSEL;

	CSEL;
	spi_wrb(0x72);
	spi_wrb(data);
	DSEL;
}

const char hx8347_idata[] PROGMEM = {
	STBAH, 0x00,
	STBAL, 0x20,
	PTBAH, 0x0c,
	PTBAL, 0xc4,
	OPONN, 0x40,
	OPONI, 0x38,
	0xf1,  0x01,
	0xf2,  0x10,
	DC2,   0xa3,

	PWC2,  0x1b,
	PWC1,  0x01,
	VMH,   0x2f,
	VML,   0x57,
	VMF,   0x8d,

	OSCH,  0x36,
	OSCL,  0x01,
	DMODE, 0x00,
	PWC6,  0x88,
	NOP,   5,
	PWC6,  0x80,
	NOP,   5,
	PWC6,  0x90,
	NOP,   5,
	PWC6,  0xd0,
	NOP,   5,

	COLMOD, 0x05,	/* 16b/pixel */

	PCH,   0x00,

	DC3,   0x38,
	NOP,   40,
	DC3,   0x3c,

	MAC,   0xa8,

	SCRH,  0,
	SCRL,  0,
	ECRH,  1,
	ECRL,  63,
	SPRH,  0,
	SPRL,  0,
	EPRH,  0,
	EPRL,  239,
};

void
hx8347_init(void)
{
	const char *p, *ep;
	int i;

	DSEL;
	PORTH &= ~_BV(PORTH5);
	msdelay(50);
	PORTH |= _BV(PORTH5);
	msdelay(50);

	for (p = hx8347_idata, ep = p + sizeof hx8347_idata; p < ep; p += 2) {
		char c = pgm_read_byte(p), d = pgm_read_byte(p+1);
		if (c == NOP)
			msdelay(d);
		else
			hx8347_cmd(c, d);
	}

	/* clear screen */

	CSEL;
	spi_wrb(0x70);
	spi_wrb(SRAMWC);
	DSEL;

	CSEL;
	spi_wrb(0x72);
	for (i = 320 / 8 * 240; i--; ) {
		spi_wrw(0xe718);
		spi_wrw(0xe718);
		spi_wrw(0xe718);
		spi_wrw(0xe718);
		spi_wrw(0xe718);
		spi_wrw(0xe718);
		spi_wrw(0xe718);
		spi_wrw(0xe718);
	}
	DSEL;

	PORTH |= _BV(PORTH6);	/* turn on backlight */
}

void
hx8347_clip(char x0, char y0, char x1, char y1)
{
	hx8347_cmd(SCRH, x0 >> 8);
	hx8347_cmd(SCRL, x0);
	hx8347_cmd(ECRH, x1 >> 8);
	hx8347_cmd(ECRL, x1);
	hx8347_cmd(SPRH, y0 >> 8);
	hx8347_cmd(SPRL, y0);
	hx8347_cmd(EPRH, y1 >> 8);
	hx8347_cmd(EPRL, y1);
}
/*
 * convert 3-3-2 to 5-6-5 palette colouring
 * from:
 * awk 'BEGIN {printf("\t");for(i=0;i<256;i++)printf("0x%04x,%s",int(int(i%4)*8+(int(i/4)%8)*256+(int(i/32)%8)*8192),(i+1)%8?" ":"\n\t")} END {printf("\n")}' < /dev/null
 */
const uint16_t
hx8347_palette[256] PROGMEM = {
	0x0000, 0x0008, 0x0010, 0x0018, 0x0100, 0x0108, 0x0110, 0x0118,
	0x0200, 0x0208, 0x0210, 0x0218, 0x0300, 0x0308, 0x0310, 0x0318,
	0x0400, 0x0408, 0x0410, 0x0418, 0x0500, 0x0508, 0x0510, 0x0518,
	0x0600, 0x0608, 0x0610, 0x0618, 0x0700, 0x0708, 0x0710, 0x0718,
	0x2000, 0x2008, 0x2010, 0x2018, 0x2100, 0x2108, 0x2110, 0x2118,
	0x2200, 0x2208, 0x2210, 0x2218, 0x2300, 0x2308, 0x2310, 0x2318,
	0x2400, 0x2408, 0x2410, 0x2418, 0x2500, 0x2508, 0x2510, 0x2518,
	0x2600, 0x2608, 0x2610, 0x2618, 0x2700, 0x2708, 0x2710, 0x2718,
	0x4000, 0x4008, 0x4010, 0x4018, 0x4100, 0x4108, 0x4110, 0x4118,
	0x4200, 0x4208, 0x4210, 0x4218, 0x4300, 0x4308, 0x4310, 0x4318,
	0x4400, 0x4408, 0x4410, 0x4418, 0x4500, 0x4508, 0x4510, 0x4518,
	0x4600, 0x4608, 0x4610, 0x4618, 0x4700, 0x4708, 0x4710, 0x4718,
	0x6000, 0x6008, 0x6010, 0x6018, 0x6100, 0x6108, 0x6110, 0x6118,
	0x6200, 0x6208, 0x6210, 0x6218, 0x6300, 0x6308, 0x6310, 0x6318,
	0x6400, 0x6408, 0x6410, 0x6418, 0x6500, 0x6508, 0x6510, 0x6518,
	0x6600, 0x6608, 0x6610, 0x6618, 0x6700, 0x6708, 0x6710, 0x6718,
	0x8000, 0x8008, 0x8010, 0x8018, 0x8100, 0x8108, 0x8110, 0x8118,
	0x8200, 0x8208, 0x8210, 0x8218, 0x8300, 0x8308, 0x8310, 0x8318,
	0x8400, 0x8408, 0x8410, 0x8418, 0x8500, 0x8508, 0x8510, 0x8518,
	0x8600, 0x8608, 0x8610, 0x8618, 0x8700, 0x8708, 0x8710, 0x8718,
	0xa000, 0xa008, 0xa010, 0xa018, 0xa100, 0xa108, 0xa110, 0xa118,
	0xa200, 0xa208, 0xa210, 0xa218, 0xa300, 0xa308, 0xa310, 0xa318,
	0xa400, 0xa408, 0xa410, 0xa418, 0xa500, 0xa508, 0xa510, 0xa518,
	0xa600, 0xa608, 0xa610, 0xa618, 0xa700, 0xa708, 0xa710, 0xa718,
	0xc000, 0xc008, 0xc010, 0xc018, 0xc100, 0xc108, 0xc110, 0xc118,
	0xc200, 0xc208, 0xc210, 0xc218, 0xc300, 0xc308, 0xc310, 0xc318,
	0xc400, 0xc408, 0xc410, 0xc418, 0xc500, 0xc508, 0xc510, 0xc518,
	0xc600, 0xc608, 0xc610, 0xc618, 0xc700, 0xc708, 0xc710, 0xc718,
	0xe000, 0xe008, 0xe010, 0xe018, 0xe100, 0xe108, 0xe110, 0xe118,
	0xe200, 0xe208, 0xe210, 0xe218, 0xe300, 0xe308, 0xe310, 0xe318,
	0xe400, 0xe408, 0xe410, 0xe418, 0xe500, 0xe508, 0xe510, 0xe518,
	0xe600, 0xe608, 0xe610, 0xe618, 0xe700, 0xe708, 0xe710, 0xe718,
};

void
hx8347_puckmap(const char *p, char fg, char bg, char bf)
{
	uint16_t f, b;
	char m;

	if (!bf) {
		CSEL;
		spi_wrb(0x70);
		spi_wrb(SRAMWC);
		DSEL;

		CSEL;
		spi_wrb(0x72);
	}

	f = pgm_read_word(&hx8347_palette[(int)fg]);
	b = pgm_read_word(&hx8347_palette[(int)bg]);

	for (m = 0x80; m; m >>= 1)
		spi_wrw(pgm_read_byte(p) &m? f : b);
		
	if (bf == 0xff)
		DSEL;
}

void
hx8347_bitmap(char x, char y, const uint16_t *p, char w, char h)
{
	const uint16_t *ep;

	hx8347_clip(x, y, x + w - 1, y + h - 1);

	CSEL;
	spi_wrb(0x70);
	spi_wrb(SRAMWC);
	DSEL;

	CSEL;
	spi_wrb(0x72);
	for (ep = p + w * h; p < ep; p++)
		spi_wrw(*p);
	DSEL;
} 

void
antialias(uint16_t *bmap, char x, char y)
{

}

#include "font.h"
#include "sony8x16.h"
#include "sony12x24.h"

const char hx8347_fonts[] PROGMEM = {
	0, 8,
	8, 16,
	12, 24,
	24, 32
};

void
hx8347_putchar(char x, char y, char ch, char attr)
{
	const char *p = font + pgm_read_word(&fidx[(int)ch]);
	uint16_t *q, bmap[24 * 32];	/* the biggest char we ever handle */
	uint16_t f, b;
	char i, c, w, h;

	f = pgm_read_word(&hx8347_palette[(int)0x00]);
	b = pgm_read_word(&hx8347_palette[(int)0xff]);

	/* expand the char */
	i = (attr & FONTMASK) << 1;
	w = i? pgm_read_byte(&hx8347_fonts[(int)i]) : pgm_read_byte(p);
	h = pgm_read_byte(&hx8347_fonts[(int)i + 1]);

	switch (i) {
	case FONT0 << 1:
		for (p++; i < w; p++, i++) {
			uint16_t *eq;
			q = bmap + i;
			eq = q + w * 8;
			for (c = pgm_read_byte(p); q < eq; q += w, c >>= 1)
				*q = c & 1? f : b;
		}
		break;
	case FONT1 << 1:
		for (q = bmap, p = sony8x16_data + ch * 16, i = 0;
		    i < 16; i++, p++) {
			c = pgm_read_byte(p);
			*q++ = c & 0x80? f : b;
			*q++ = c & 0x40? f : b;
			*q++ = c & 0x20? f : b;
			*q++ = c & 0x10? f : b;
			*q++ = c & 0x08? f : b;
			*q++ = c & 0x04? f : b;
			*q++ = c & 0x02? f : b;
			*q++ = c & 0x01? f : b;
		}
		break;
	case FONT2 << 1:
		for (q = bmap, p = sony12x24_data + ch * 48, i = 0;
		    i < 48; i++, p++) {
			c = pgm_read_byte(p);
			*q++ = c & 0x80? f : b;
			*q++ = c & 0x40? f : b;
			*q++ = c & 0x20? f : b;
			*q++ = c & 0x10? f : b;
			*q++ = c & 0x08? f : b;
			*q++ = c & 0x04? f : b;
			*q++ = c & 0x02? f : b;
			*q++ = c & 0x01? f : b;
			p++;
			c = pgm_read_byte(p);
			*q++ = c & 0x80? f : b;
			*q++ = c & 0x40? f : b;
			*q++ = c & 0x20? f : b;
			*q++ = c & 0x10? f : b;
		}
		break;
	case FONT3 << 1:
		for (q = bmap, p = sony12x24_data + ch * 48, i = 0;
		    i < 96; i++, p++) {
			c = pgm_read_byte(p);
		}
	}

	hx8347_bitmap(x, y, bmap, w, h);   
}

/*
 * this assumes we do not print string longer than 256 pixels (;
 */
char
hx8347_length(const char *s, char attr)
{
	char l = 0;
	  
	switch (attr & FONTMASK) {
	case FONT0:
		for (; *s; s++)
			l += pgm_read_byte(font +
			    pgm_read_word(&fidx[(int)*s]));
			break;
	case FONT1:
		for (; *s; s++)
			l += 8;
		break;
	case FONT2:
		for (; *s; s++)
			l += 12;
		break;
	case FONT3:
		for (; *s; s++)
			l += 24;
		break;
	}

	return l;
}

void
hx8347_putstr(char x, char y, const char *s, char attr)
{
	char c;

	c = 8;
	for (; *s ; s++, x += c) {
		switch (attr & FONTMASK) {
		case FONT0:
			c = pgm_read_byte(font +
			    pgm_read_word(&fidx[(int)*s]));
			break;
		case FONT1:
			c = 8;
			break;
		case FONT2:
			c = 12;
			break;
		case FONT3:
			c = 24;
			break;
		}
		hx8347_putchar(x, y, *s, attr);
	}
} 

const char hexd[] PROGMEM = "0123456789abcdef";

#ifdef DEBUG
void
puthexb(char x, char y, char b)
{

	hx8347_putchar(x, y, pgm_read_byte(&hexd[b >> 4]), 0);
	hx8347_putchar(x, y + 6, pgm_read_byte(&hexd[b & 0xf]), 0);
}
#endif

