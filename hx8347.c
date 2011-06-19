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
		spi_wrw(0xffff);
		spi_wrw(0);
		spi_wrw(0xffff);
		spi_wrw(0);
		spi_wrw(0xffff);
		spi_wrw(0);
		spi_wrw(0xffff);
		spi_wrw(0);
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

uint16_t
hx8347_palette[256] = {
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xffff,
};

void
hx8347_pgbitmap(char x, char y, const char *p, char l, char fg, char bg)
{
	char i, j, m;
	uint16_t f, b;

	hx8347_clip(x, y, x + l - 1, y + 7);
	f = hx8347_palette[(int)fg];
	b = hx8347_palette[(int)bg];

	CSEL;
	spi_wrb(0x70);
	spi_wrb(SRAMWC);
	DSEL;

	CSEL;
	spi_wrb(0x72);
	for (m = 0x01, i = 8; i--; m <<= 1)
		for (j = 0; j < l; j++)
			spi_wrw(pgm_read_byte(p + j) & m? f : b);
	DSEL;
} 

#include "font.h"

void
hx8347_putchar(char x, char y, char c, char attr)
{
	const char *p = font + pgm_read_word(&fidx[(int)c]);

	c = pgm_read_byte(p);
	hx8347_pgbitmap(x, y, p + 1, c, 0x00, 0xff);   
}

/*
 * this assumes we do not print string lnger than 256 pixels (;
 */
char
hx8347_length(const char *s, char attr)
{
	char l;
	  
	for (l = 0; *s; s++)
		l += pgm_read_byte(font + pgm_read_word(&fidx[(int)*s]));

	return l;
}

void
hx8347_putstr(char x, char y, const char *s, char attr)
{
	char c;

	for (; *s && y < 240; s++, x += c) {
		const char *p = font + pgm_read_word(&fidx[(int)*s]);
		c = pgm_read_byte(p);
		hx8347_pgbitmap(x, y, p + 1, c, 0, 0xff);
	}
} 
