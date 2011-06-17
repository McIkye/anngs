;
; Copyright (c) 2011 Michael Shalayeff
; All rights reserved.
;
; Permission to use, copy, modify, and distribute this software for any
; purpose with or without fee is hereby granted, provided that the above
; copyright notice and this permission notice appear in all copies.
;
; THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
; WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
; MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
; ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
; WHATSOEVER RESULTING FROM LOSS OF MIND, USE, DATA OR PROFITS, WHETHER IN
; AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
; OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
;

	.globl	usdelay, msdelay
	.text
	.align	1

	; this is 1+4 cycles too many for each call
usdelay:
	rjmp	2f		; 2
1:
#if F_CPU == 20000000
	nop			; 13
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
#elif F_CPU == 16000000
	nop			; 9
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
#elif F_CPU == 12000000
	nop			; 5
	nop
	nop
	nop
	nop
#elif F_CPU == 8000000
	nop
#else
#error "fix usdelay()"
#endif

2:	sbiw	r24, 1		; 2
	ldi	r18, 0xff	; 1
	cpi	r24, 0xff	; 1
	cpc	r25, r18	; 1
	brne	1b		; 1/2
	ret			; 4
	.size usdelay, .-usdelay

	; there is 13+5 cycles for each loop with usdelay()
msdelay:
	movw	r26, r24	; 1
	rjmp	2f		; 2
1:
#if F_CPU == 20000000
	ldi	r24, 0xE7	; 1	#999
	nop			; 20-5 leftover - 13 overhead
	nop
#elif F_CPU == 16000000
	ldi	r24, 0xE6	; 1	#998
	nop			; 32-5 leftover - 13 overhead
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
	nop
#elif F_CPU == 12000000
	ldi	r24, 0xE6	; 1	#998
	nop			; 24-5 leftover - 13 overhead
	nop
	nop
	nop
	nop
	nop
#elif F_CPU == 8000000
	ldi	r24, 0xE5	; 1	#997
	nop			; 24-5 leftover - 13 overhead
	nop
	nop
	nop
	nop
	nop
#else
#error "fix msdelay()"
#endif
	ldi	r25, 0x03	; 1
	call	usdelay		; 4
2:	sbiw	r26, 1		; 2
	ldi	r24, 0xff	; 1
	cpi	r26, 0xff	; 1
	cpc	r27, r24	; 1
	brne	1b		; 1/2
	ret			; 4
	.size msdelay, .-msdelay

	.end
