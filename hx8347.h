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

enum hx8347_attr {
	FONT0, FONT1, FONT2, FONT3, FONTMASK = 3,
	NORMAL =0x00,	/* black/white */
	STATUS =0x04,	/* white/blue */
	ALERT  =0x08,	/* red/white */
	MESG   =0x0c,	/* black/gray */
	REVERSE=0x80,	/* flag to reverse any of the above */

};

void hx8347_init(void);
void hx8347_clip(char, char, char, char);
void hx8347_puckmap(const char *, char, char, char);
void hx8347_putchar(char, char, char, char);
char hx8347_length(const char *, char);
void hx8347_putstr(char, char, const char *, char);

