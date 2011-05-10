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

/*

 ping assignments

pa0		- xmem
pa1		- xmem
pa2		- xmem
pa3		- xmem
pa4		- xmem
pa5		- xmem
pa6		- xmem
pa7		- xmem

pb0/ss		-
pb1/sck		-
pb2/mosi	-
pb3/miso	-
pb4/oc2a	- sd_cs/alt
pb5/oc1a	-
pb6/oc1b	-
pb7/oc0a/oc1c	-

pc0		- xmem
pc1		- xmem
pc2		- xmem
pc3		- xmem
pc4		- xmem
pc5		- xmem
pc6		- xmem
pc7		- xmem

pd0/scl		-
pd1/sda		-
pd2/rxd1	-
pd3/txd1	-
pd4/icp1	-
pd5/xck1	-
pd6/t1		-
pd7/t0		-

pe0/pdi/rxd0	-
pe1/pdo/txd0	-
pe2/ain0/xck0	-
pe3/ain1/oc3a	- ads_busy
pe4/int4/oc3b	- sd_detect
pe5/int5/oc3c	- ads_irq
pe6/int6/t3	-
pe7/int7/icp3	-

pf0/adc0	-
pf1/adc1	-
pf2/adc2	-
pf3/adc3	-
pf4/adc4/tck	-
pf5/adc5/tms	-
pf6/adc6/tdo	-
pf7/adc7/tdi	-

pg0/wr		- xmem
pg1/rd		- xmem
pg2/ale		- xmem
pg3/tosc2	-
pg4/tosc1	-
pg5/oc0b	- sd_cs

pg6/xck
pg7/xck

ph0/rxd2	-
ph1/txd2	-
ph2/xck2	-
ph3/oc4a	- ads_cs
ph4/oc4b	- lcd_cs
ph5/oc4c	- lcd_rst
ph6/oc2b	- lcd_led
ph7/t4		-

pj0/rxd3	-
pj1/txd3	-
pj2/xck3	-
pj3/pcint12	- cad_int
pj4/pcint13	- ped_int
pj5/pcint14	- hb_cs
pj6/pcint15	- mp3_int
pj7		- mp3_cs

pk0/adc8	-
pk1/adc9	-
pk2/adc10	-
pk3/adc11	-
pk4/adc12	-

pl0/icp4	-
pl1/icp5	-
pl2/t5		-
pl3/oc5a	-
pl4/oc5b	-
pl5/oc5c	-
pl6		-
pl7		-

msd:
2/pe4 sd_detect
3/pe5 ads_irq
4/pg5 sd_cs (10/pb4)
5/pe3 ads_busy
6/ph3 ads_cs
7/ph4 lcd_cs
8/ph5 lcd_rst
9/ph6 lcd_led

 */

int
main()
{

	return 0;
}
