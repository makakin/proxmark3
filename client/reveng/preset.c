/* preset.c
 * Greg Cook, 26/Jul/2016
 */

/* CRC RevEng: arbitrary-precision CRC calculator and algorithm finder
 * Copyright (C) 2010, 2011, 2012, 2013, 2014, 2015, 2016  Gregory Cook
 *
 * This file is part of CRC RevEng.
 *
 * CRC RevEng is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * CRC RevEng is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with CRC RevEng.  If not, see <https://www.gnu.org/licenses/>.
 */

/* 2016-07-26: added array order checking code
 * 2016-07-25: added 5 new algorithms
 * 2016-07-14: added CRC-16/CMS
 * 2016-07-08: added CRC-16/PROFIBUS
 * 2016-02-23: added 11 new algorithms, 4 new aliases
 * 2016-02-22: split off from model.c
 * 2016-02-22: preset points to primary alias, eliminated strcmp()
 * 2016-02-22: eliminated bogus calls to bsearch()
 * 2015-07-29: eliminated struct malias.isprimry
 * 2014-01-14: added CRC-8/DVB-S2
 * 2014-01-11: corrected CRC-40/GSM, added alias CRC-8/AES
 * 2013-10-14: added CRC-13/BBC and six cdma2000 algorithms
 * 2013-06-11: ensure BMP_BIT is an integer constant to compile presets
 * 2013-01-20: big polynomials autogenerated, corrected CRC-82/DARC
 * 2012-07-19: added CRC-8/EBU
 * 2012-07-16: added CRC-15/MPT1327
 * 2012-05-25: removed CRC-1/PARITY-EVEN, CRC-1/PARITY-ODD
 * 2012-04-12: added model CRC-31/PHILIPS
 * 2012-02-20: corrected model CRC-6/DARC
 * 2011-08-28: added model CRC-64/XZ
 * 2011-04-30: added models CRC-16/TMS37157 and CRC-A, and alias CRC-B
 * 2011-02-10: made preset models ANSI C compliant
 * 2011-01-17: fixed ANSI C warnings (except preset models)
 * 2011-01-01: added mbynum(), mcount()
 * 2010-12-26: renamed CRC RevEng
 * 2010-12-18: minor change to mtostr() output format
 * 2010-12-15: added mcmp(), mmatch()
 * 2010-12-14: finished mbynam(), mnames()
 * 2010-12-13: restarted with PCONST macros
 * 2010-12-12: was having so much fun I didn't think to try compiling. :(
 * 2010-12-12: started models.c
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reveng.h"

/* Private declarations */

/* incomplete type declaration to permit cross-reference */
struct malias;

struct mpreset {
	const unsigned long width;		/* width of CRC algorithm */
	const bmp_t *const bspoly;		/* polynomial with highest-order term removed. length determines CRC width */
	const bmp_t *const binit;		/* initial register value. length == poly.length */
	const int flags;			/* P_REFIN and P_REFOUT indicate reflected input/output */
	const bmp_t *const bxorout;		/* final register XOR mask. length == poly.length */
	const bmp_t *const bcheck;		/* optional check value, the CRC of the UTF-8 string "123456789" */
	const struct malias *const alias;	/* optional canonical name of the model */
};

struct malias {
	const char *name;			/* name of alias */
	const struct mpreset *const model;	/* corresponding model */
};

#ifdef PRESETS
#  if BMP_BIT < 32
#    error config.h: BMP_BIT must be an integer constant macro to compile presets
#  else /* BMP_BIT */

/* Big polynomial constants. */

/* Directives for relink.pl */
/* CONSTANT b40  = (40, 0x0004820009) */
/* CONSTANT b40a = (40, 0xffffffffff) */
/* CONSTANT b40b = (40, 0xd4164fc646) */
/* CONSTANT b64  = (64, 0x42f0e1eba9ea3693) */
/* CONSTANT b64a = (64, 0x6c40df5f0b497347) */
/* CONSTANT b64b = (64, 0xffffffffffffffff) */
/* CONSTANT b64c = (64, 0x62ec59e3f1a4f00a) */
/* CONSTANT b64d = (64, 0x995dc9bbdf1939fa) */
/* CONSTANT b82  = (82, 0x0308c0111011401440411) */
/* CONSTANT b82a = (82, 0x09ea83f625023801fd612) */

/* The next section was generated by relink.pl from the directives above. */

/* DO NOT EDIT the section below, INCLUDING the next comment. */
/* BEGIN AUTO-GENERATED CONSTANTS */
#    if BMP_BIT >= 40
static const bmp_t b40[] = {
	BMP_C(0x0004820009) << (BMP_BIT - 40),
};
static const bmp_t b40a[] = {
	BMP_C(0xffffffffff) << (BMP_BIT - 40),
};
static const bmp_t b40b[] = {
	BMP_C(0xd4164fc646) << (BMP_BIT - 40),
};
#    else /* BMP_BIT */
static const bmp_t b40[] = {
	BMP_C(0x00048200) << (BMP_BIT - 32) | BMP_C(0x04) >> (39 - BMP_BIT),
	BMP_C(0x09) << (BMP_BIT * 2 - 40),
};
static const bmp_t b40a[] = {
	BMP_C(0xffffffff) << (BMP_BIT - 32) | BMP_C(0x7f) >> (39 - BMP_BIT),
	BMP_C(0xff) << (BMP_BIT * 2 - 40),
};
static const bmp_t b40b[] = {
	BMP_C(0xd4164fc6) << (BMP_BIT - 32) | BMP_C(0x23) >> (39 - BMP_BIT),
	BMP_C(0x46) << (BMP_BIT * 2 - 40),
};
#    endif /* BMP_BIT */

#    if BMP_BIT >= 64
static const bmp_t b64[] = {
	BMP_C(0x42f0e1eba9ea3693) << (BMP_BIT - 64),
};
static const bmp_t b64a[] = {
	BMP_C(0x6c40df5f0b497347) << (BMP_BIT - 64),
};
static const bmp_t b64b[] = {
	BMP_C(0xffffffffffffffff) << (BMP_BIT - 64),
};
static const bmp_t b64c[] = {
	BMP_C(0x62ec59e3f1a4f00a) << (BMP_BIT - 64),
};
static const bmp_t b64d[] = {
	BMP_C(0x995dc9bbdf1939fa) << (BMP_BIT - 64),
};
#    else /* BMP_BIT */
static const bmp_t b64[] = {
	BMP_C(0x42f0e1eb) << (BMP_BIT - 32) | BMP_C(0x54f51b49) >> (63 - BMP_BIT),
	BMP_C(0xa9ea3693) << (BMP_BIT * 2 - 64),
};
static const bmp_t b64a[] = {
	BMP_C(0x6c40df5f) << (BMP_BIT - 32) | BMP_C(0x05a4b9a3) >> (63 - BMP_BIT),
	BMP_C(0x0b497347) << (BMP_BIT * 2 - 64),
};
static const bmp_t b64b[] = {
	BMP_C(0xffffffff) << (BMP_BIT - 32) | BMP_C(0x7fffffff) >> (63 - BMP_BIT),
	BMP_C(0xffffffff) << (BMP_BIT * 2 - 64),
};
static const bmp_t b64c[] = {
	BMP_C(0x62ec59e3) << (BMP_BIT - 32) | BMP_C(0x78d27805) >> (63 - BMP_BIT),
	BMP_C(0xf1a4f00a) << (BMP_BIT * 2 - 64),
};
static const bmp_t b64d[] = {
	BMP_C(0x995dc9bb) << (BMP_BIT - 32) | BMP_C(0x6f8c9cfd) >> (63 - BMP_BIT),
	BMP_C(0xdf1939fa) << (BMP_BIT * 2 - 64),
};
#    endif /* BMP_BIT */

#    if BMP_BIT >= 82
static const bmp_t b82[] = {
	BMP_C(0x0308c0111011401440411) << (BMP_BIT - 82),
};
static const bmp_t b82a[] = {
	BMP_C(0x09ea83f625023801fd612) << (BMP_BIT - 82),
};
#    elif BMP_BIT >= 41
static const bmp_t b82[] = {
	BMP_C(0x01846008880) << (BMP_BIT - 41) | BMP_C(0x08a00a20208) >> (81 - BMP_BIT),
	BMP_C(0x11401440411) << (BMP_BIT * 2 - 82),
};
static const bmp_t b82a[] = {
	BMP_C(0x04f541fb128) << (BMP_BIT - 41) | BMP_C(0x011c00feb09) >> (81 - BMP_BIT),
	BMP_C(0x023801fd612) << (BMP_BIT * 2 - 82),
};
#    else /* BMP_BIT */
static const bmp_t b82[] = {
	BMP_C(0x0c230044) << (BMP_BIT - 32) | BMP_C(0x040) >> (40 - BMP_BIT),
	BMP_C(0x40450051) << (BMP_BIT * 2 - 64) | BMP_C(0x00104) >> (80 - BMP_BIT * 2),
	BMP_C(0x00411) << (BMP_BIT * 3 - 82),
};
static const bmp_t b82a[] = {
	BMP_C(0x27aa0fd8) << (BMP_BIT - 32) | BMP_C(0x094) >> (40 - BMP_BIT),
	BMP_C(0x9408e007) << (BMP_BIT * 2 - 64) | BMP_C(0x0f584) >> (80 - BMP_BIT * 2),
	BMP_C(0x3d612) << (BMP_BIT * 3 - 82),
};
#    endif /* BMP_BIT */

/* END AUTO-GENERATED CONSTANTS */
/* DO NOT EDIT the section above, INCLUDING the previous comment. */

/* Array of the polynomial bitmaps used in the model table. */
static const bmp_t b32[] = {
	BMP_C(0x00000000) << (BMP_BIT - 32),	/*   0 --  5,      00 */
	BMP_C(0x000000af) << (BMP_BIT - 32),	/*   1 -- 32,000000af */
	BMP_C(0x00010000) << (BMP_BIT - 32),	/*   2 -- 16,    0001 */
	BMP_C(0x00020000) << (BMP_BIT - 32),	/*   3 -- 15,    0001 */
	BMP_C(0x00065b00) << (BMP_BIT - 32),	/*   4 -- 24,  00065b */
	BMP_C(0x007e0000) << (BMP_BIT - 32),	/*   5 -- 16,    007e */
	BMP_C(0x007f0000) << (BMP_BIT - 32),	/*   6 -- 16,    007f */
	BMP_C(0x03400000) << (BMP_BIT - 32),	/*   7 -- 11,     01a */
	BMP_C(0x0376e6e7) << (BMP_BIT - 32),	/*   8 -- 32,0376e6e7 */
	BMP_C(0x04c11db7) << (BMP_BIT - 32),	/*   9 -- 32,04c11db7 */
	BMP_C(0x05890000) << (BMP_BIT - 32),	/*  10 -- 16,    0589 */
	BMP_C(0x07000000) << (BMP_BIT - 32),	/*  11 --  8,      07 */
	BMP_C(0x09823b6e) << (BMP_BIT - 32),	/*  12 -- 31,04c11db7 */
	BMP_C(0x0b3c0000) << (BMP_BIT - 32),	/*  13 -- 15,    059e */
	BMP_C(0x0c000000) << (BMP_BIT - 32),	/*  14 --  6,      03 */
	BMP_C(0x0c200000) << (BMP_BIT - 32),	/*  15 -- 11,     061 */
	BMP_C(0x0fb30000) << (BMP_BIT - 32),	/*  16 -- 16,    0fb3 */
	BMP_C(0x10210000) << (BMP_BIT - 32),	/*  17 -- 16,    1021 */
	BMP_C(0x12000000) << (BMP_BIT - 32),	/*  18 --  7,      09 */
	BMP_C(0x130d2afc) << (BMP_BIT - 32),	/*  19 -- 30,04c34abf */
	BMP_C(0x15000000) << (BMP_BIT - 32),	/*  20 --  8,      15 */
	BMP_C(0x1697d06a) << (BMP_BIT - 32),	/*  21 -- 32,1697d06a */
	BMP_C(0x18000000) << (BMP_BIT - 32),	/*  22 --  6,      06 */
	BMP_C(0x19d3c8d8) << (BMP_BIT - 32),	/*  23 -- 31,0ce9e46c */
	BMP_C(0x1c000000) << (BMP_BIT - 32),	/*  24 --  6,      07 */
	BMP_C(0x1d000000) << (BMP_BIT - 32),	/*  25 --  8,      1d */
	BMP_C(0x1d0f0000) << (BMP_BIT - 32),	/*  26 -- 16,    1d0f */
	BMP_C(0x1dcf0000) << (BMP_BIT - 32),	/*  27 -- 16,    1dcf */
	BMP_C(0x1edc6f41) << (BMP_BIT - 32),	/*  28 -- 32,1edc6f41 */
	BMP_C(0x1f23b800) << (BMP_BIT - 32),	/*  29 -- 24,  1f23b8 */
	BMP_C(0x20140000) << (BMP_BIT - 32),	/*  30 -- 14,    0805 */
	BMP_C(0x20b40000) << (BMP_BIT - 32),	/*  31 -- 14,    082d */
	BMP_C(0x20fe0000) << (BMP_BIT - 32),	/*  32 -- 16,    20fe */
	BMP_C(0x21890000) << (BMP_BIT - 32),	/*  33 -- 16,    2189 */
	BMP_C(0x21cf0200) << (BMP_BIT - 32),	/*  34 -- 24,  21cf02 */
	BMP_C(0x23ef5200) << (BMP_BIT - 32),	/*  35 -- 24,  23ef52 */
	BMP_C(0x25000000) << (BMP_BIT - 32),	/*  36 --  8,      25 */
	BMP_C(0x26b10000) << (BMP_BIT - 32),	/*  37 -- 16,    26b1 */
	BMP_C(0x27d00000) << (BMP_BIT - 32),	/*  38 -- 13,    04fa */
	BMP_C(0x28000000) << (BMP_BIT - 32),	/*  39 --  5,      05 */
	BMP_C(0x29b10000) << (BMP_BIT - 32),	/*  40 -- 16,    29b1 */
	BMP_C(0x2f000000) << (BMP_BIT - 32),	/*  41 --  8,      2f */
	BMP_C(0x30000000) << (BMP_BIT - 32),	/*  42 --  4,       3 */
	BMP_C(0x3010bf7f) << (BMP_BIT - 32),	/*  43 -- 32,3010bf7f */
	BMP_C(0x31000000) << (BMP_BIT - 32),	/*  44 --  8,      31 */
	BMP_C(0x31c30000) << (BMP_BIT - 32),	/*  45 -- 16,    31c3 */
	BMP_C(0x328b6300) << (BMP_BIT - 32),	/*  46 -- 24,  328b63 */
	BMP_C(0x34000000) << (BMP_BIT - 32),	/*  47 --  6,      0d */
	BMP_C(0x340bc6d9) << (BMP_BIT - 32),	/*  48 -- 32,340bc6d9 */
	BMP_C(0x38000000) << (BMP_BIT - 32),	/*  49 --  5,      07 */
	BMP_C(0x39000000) << (BMP_BIT - 32),	/*  50 --  8,      39 */
	BMP_C(0x3d650000) << (BMP_BIT - 32),	/*  51 -- 16,    3d65 */
	BMP_C(0x3e000000) << (BMP_BIT - 32),	/*  52 --  8,      3e */
	BMP_C(0x44c20000) << (BMP_BIT - 32),	/*  53 -- 16,    44c2 */
	BMP_C(0x48000000) << (BMP_BIT - 32),	/*  54 --  5,      09 */
	BMP_C(0x4acc0000) << (BMP_BIT - 32),	/*  55 -- 15,    2566 */
	BMP_C(0x4b000000) << (BMP_BIT - 32),	/*  56 --  8,      4b */
	BMP_C(0x4b370000) << (BMP_BIT - 32),	/*  57 -- 16,    4b37 */
	BMP_C(0x4c060000) << (BMP_BIT - 32),	/*  58 -- 16,    4c06 */
	BMP_C(0x55000000) << (BMP_BIT - 32),	/*  59 --  8,      55 */
	BMP_C(0x55555500) << (BMP_BIT - 32),	/*  60 -- 24,  555555 */
	BMP_C(0x59350000) << (BMP_BIT - 32),	/*  61 -- 16,    5935 */
	BMP_C(0x5d380000) << (BMP_BIT - 32),	/*  62 -- 16,    5d38 */
	BMP_C(0x5d6dcb00) << (BMP_BIT - 32),	/*  63 -- 24,  5d6dcb */
	BMP_C(0x60000000) << (BMP_BIT - 32),	/*  64 --  3,       3 */
	BMP_C(0x60e00000) << (BMP_BIT - 32),	/*  65 -- 11,     307 */
	BMP_C(0x63d00000) << (BMP_BIT - 32),	/*  66 -- 16,    63d0 */
	BMP_C(0x64000000) << (BMP_BIT - 32),	/*  67 --  6,      19 */
	BMP_C(0x66400000) << (BMP_BIT - 32),	/*  68 -- 10,     199 */
	BMP_C(0x6f630000) << (BMP_BIT - 32),	/*  69 -- 16,    6f63 */
	BMP_C(0x6f910000) << (BMP_BIT - 32),	/*  70 -- 16,    6f91 */
	BMP_C(0x70000000) << (BMP_BIT - 32),	/*  71 --  4,       7 */
	BMP_C(0x70a00000) << (BMP_BIT - 32),	/*  72 -- 11,     385 */
	BMP_C(0x755b0000) << (BMP_BIT - 32),	/*  73 -- 16,    755b */
	BMP_C(0x765e7680) << (BMP_BIT - 32),	/*  74 -- 32,765e7680 */
	BMP_C(0x7979bd00) << (BMP_BIT - 32),	/*  75 -- 24,  7979bd */
	BMP_C(0x7e000000) << (BMP_BIT - 32),	/*  76 --  8,      7e */
	BMP_C(0x80006300) << (BMP_BIT - 32),	/*  77 -- 24,  800063 */
	BMP_C(0x80050000) << (BMP_BIT - 32),	/*  78 -- 16,    8005 */
	BMP_C(0x800d0000) << (BMP_BIT - 32),	/*  79 -- 16,    800d */
	BMP_C(0x80c2e71c) << (BMP_BIT - 32),	/*  80 -- 30,2030b9c7 */
	BMP_C(0x80f00000) << (BMP_BIT - 32),	/*  81 -- 12,     80f */
	BMP_C(0x814141ab) << (BMP_BIT - 32),	/*  82 -- 32,814141ab */
	BMP_C(0x864cfb00) << (BMP_BIT - 32),	/*  83 -- 24,  864cfb */
	BMP_C(0x87315576) << (BMP_BIT - 32),	/*  84 -- 32,87315576 */
	BMP_C(0x89ec0000) << (BMP_BIT - 32),	/*  85 -- 16,    89ec */
	BMP_C(0x8a000000) << (BMP_BIT - 32),	/*  86 --  7,      45 */
	BMP_C(0x8b320000) << (BMP_BIT - 32),	/*  87 -- 15,    4599 */
	BMP_C(0x8bb70000) << (BMP_BIT - 32),	/*  88 -- 16,    8bb7 */
	BMP_C(0x8cc00000) << (BMP_BIT - 32),	/*  89 -- 10,     233 */
	BMP_C(0x906e0000) << (BMP_BIT - 32),	/*  90 -- 16,    906e */
	BMP_C(0x97000000) << (BMP_BIT - 32),	/*  91 --  8,      97 */
	BMP_C(0x98000000) << (BMP_BIT - 32),	/*  92 --  6,      26 */
	BMP_C(0x9b000000) << (BMP_BIT - 32),	/*  93 --  8,      9b */
	BMP_C(0x9c000000) << (BMP_BIT - 32),	/*  94 --  6,      27 */
	BMP_C(0x9e000000) << (BMP_BIT - 32),	/*  95 --  7,      4f */
	BMP_C(0x9ecf0000) << (BMP_BIT - 32),	/*  96 -- 16,    9ecf */
	BMP_C(0xa0970000) << (BMP_BIT - 32),	/*  97 -- 16,    a097 */
	BMP_C(0xa1000000) << (BMP_BIT - 32),	/*  98 --  8,      a1 */
	BMP_C(0xa6000000) << (BMP_BIT - 32),	/*  99 --  7,      53 */
	BMP_C(0xa8000000) << (BMP_BIT - 32),	/* 100 --  5,      15 */
	BMP_C(0xa8190000) << (BMP_BIT - 32),	/* 101 -- 16,    a819 */
	BMP_C(0xa833982b) << (BMP_BIT - 32),	/* 102 -- 32,a833982b */
	BMP_C(0xabcdef00) << (BMP_BIT - 32),	/* 103 -- 24,  abcdef */
	BMP_C(0xaee70000) << (BMP_BIT - 32),	/* 104 -- 16,    aee7 */
	BMP_C(0xb0000000) << (BMP_BIT - 32),	/* 105 --  4,       b */
	BMP_C(0xb2aa0000) << (BMP_BIT - 32),	/* 106 -- 16,    b2aa */
	BMP_C(0xb4600000) << (BMP_BIT - 32),	/* 107 -- 11,     5a3 */
	BMP_C(0xb4c80000) << (BMP_BIT - 32),	/* 108 -- 16,    b4c8 */
	BMP_C(0xb4f3e600) << (BMP_BIT - 32),	/* 109 -- 24,  b4f3e6 */
	BMP_C(0xb704ce00) << (BMP_BIT - 32),	/* 110 -- 24,  b704ce */
	BMP_C(0xbb3d0000) << (BMP_BIT - 32),	/* 111 -- 16,    bb3d */
	BMP_C(0xbc000000) << (BMP_BIT - 32),	/* 112 --  8,      bc */
	BMP_C(0xbd0be338) << (BMP_BIT - 32),	/* 113 -- 32,bd0be338 */
	BMP_C(0xbdf40000) << (BMP_BIT - 32),	/* 114 -- 16,    bdf4 */
	BMP_C(0xbf050000) << (BMP_BIT - 32),	/* 115 -- 16,    bf05 */
	BMP_C(0xc0000000) << (BMP_BIT - 32),	/* 116 --  3,       6 */
	BMP_C(0xc2000000) << (BMP_BIT - 32),	/* 117 --  7,      61 */
	BMP_C(0xc25a5600) << (BMP_BIT - 32),	/* 118 -- 24,  c25a56 */
	BMP_C(0xc2b70000) << (BMP_BIT - 32),	/* 119 -- 16,    c2b7 */
	BMP_C(0xc6c60000) << (BMP_BIT - 32),	/* 120 -- 16,    c6c6 */
	BMP_C(0xc8000000) << (BMP_BIT - 32),	/* 121 --  5,      19 */
	BMP_C(0xc8670000) << (BMP_BIT - 32),	/* 122 -- 16,    c867 */
	BMP_C(0xcbf43926) << (BMP_BIT - 32),	/* 123 -- 32,cbf43926 */
	BMP_C(0xcde70300) << (BMP_BIT - 32),	/* 124 -- 24,  cde703 */
	BMP_C(0xd0000000) << (BMP_BIT - 32),	/* 125 --  8,      d0 */
	BMP_C(0xd02a0000) << (BMP_BIT - 32),	/* 126 -- 15,    6815 */
	BMP_C(0xd0db0000) << (BMP_BIT - 32),	/* 127 -- 16,    d0db */
	BMP_C(0xd4d00000) << (BMP_BIT - 32),	/* 128 -- 12,     d4d */
	BMP_C(0xd5000000) << (BMP_BIT - 32),	/* 129 --  8,      d5 */
	BMP_C(0xd64e0000) << (BMP_BIT - 32),	/* 130 -- 16,    d64e */
	BMP_C(0xda000000) << (BMP_BIT - 32),	/* 131 --  8,      da */
	BMP_C(0xdaf00000) << (BMP_BIT - 32),	/* 132 -- 12,     daf */
	BMP_C(0xdf000000) << (BMP_BIT - 32),	/* 133 --  8,      df */
	BMP_C(0xe0000000) << (BMP_BIT - 32),	/* 134 --  3,       7 */
	BMP_C(0xe3069283) << (BMP_BIT - 32),	/* 135 -- 32,e3069283 */
	BMP_C(0xe5cc0000) << (BMP_BIT - 32),	/* 136 -- 16,    e5cc */
	BMP_C(0xe7a80000) << (BMP_BIT - 32),	/* 137 -- 13,    1cf5 */
	BMP_C(0xea000000) << (BMP_BIT - 32),	/* 138 --  7,      75 */
	BMP_C(0xea820000) << (BMP_BIT - 32),	/* 139 -- 16,    ea82 */
	BMP_C(0xec000000) << (BMP_BIT - 32),	/* 140 --  6,      3b */
	BMP_C(0xf0000000) << (BMP_BIT - 32),	/* 141 --  4,       f */
	BMP_C(0xf1300000) << (BMP_BIT - 32),	/* 142 -- 12,     f13 */
	BMP_C(0xf4000000) << (BMP_BIT - 32),	/* 143 --  8,      f4 */
	BMP_C(0xf4acfb13) << (BMP_BIT - 32),	/* 144 -- 32,f4acfb13 */
	BMP_C(0xf5b00000) << (BMP_BIT - 32),	/* 145 -- 12,     f5b */
	BMP_C(0xf6400000) << (BMP_BIT - 32),	/* 146 -- 10,     3d9 */
	BMP_C(0xf8000000) << (BMP_BIT - 32),	/* 147 --  5,      1f */
	BMP_C(0xfc000000) << (BMP_BIT - 32),	/* 148 --  6,      3f */
	BMP_C(0xfc891918) << (BMP_BIT - 32),	/* 149 -- 32,fc891918 */
	BMP_C(0xfd000000) << (BMP_BIT - 32),	/* 150 --  8,      fd */
	BMP_C(0xfe000000) << (BMP_BIT - 32),	/* 151 --  7,      7f */
	BMP_C(0xfedcba00) << (BMP_BIT - 32),	/* 152 -- 24,  fedcba */
	BMP_C(0xfee80000) << (BMP_BIT - 32),	/* 153 -- 16,    fee8 */
	BMP_C(0xff000000) << (BMP_BIT - 32),	/* 154 --  8,      ff */
	BMP_C(0xffc00000) << (BMP_BIT - 32),	/* 155 -- 10,     3ff */
	BMP_C(0xfff00000) << (BMP_BIT - 32),	/* 156 -- 12,     fff */
	BMP_C(0xffff0000) << (BMP_BIT - 32),	/* 157 -- 16,    ffff */
	BMP_C(0xffffff00) << (BMP_BIT - 32),	/* 158 -- 24,  ffffff */
	BMP_C(0xfffffffc) << (BMP_BIT - 32),	/* 159 -- 30,3fffffff */
	BMP_C(0xfffffffe) << (BMP_BIT - 32),	/* 160 -- 31,7fffffff */
	BMP_C(0xffffffff) << (BMP_BIT - 32),	/* 161 -- 32,ffffffff */
};

static const struct malias aliases[];

/* Table of preset CRC models.
 * Sorted by left-justified polynomial for bsearch().
 */
static const struct mpreset models[] = {
	{32UL, b32+  1, 0,       P_BE,   0,       b32+113, aliases+123},	/*  0 */
	{40UL, b40,     0,       P_BE,   b40a,    b40b,    aliases+ 80},	/*  1 */
	{24UL, b32+  4, b32+ 60, P_LE,   0,       b32+118, aliases+ 54},	/*  2 */
	{32UL, b32+  9, 0,       P_BE,   b32+161, b32+ 74, aliases+ 74},	/*  3 */
	{32UL, b32+  9, b32+161, P_BE,   0,       b32+  8, aliases+ 73},	/*  4 */
	{32UL, b32+  9, b32+161, P_BE,   b32+161, b32+149, aliases+ 68},	/*  5 */
	{32UL, b32+  9, b32+161, P_LE,   0,       b32+ 48, aliases+115},	/*  6 */
	{32UL, b32+  9, b32+161, P_LE,   b32+161, b32+123, aliases+ 64},	/*  7 */
	{16UL, b32+ 10, 0,       P_BE,   0,       b32+  6, aliases+ 28},	/*  8 */
	{16UL, b32+ 10, 0,       P_BE,   b32+  2, b32+  5, aliases+ 27},	/*  9 */
	{ 8UL, b32+ 11, 0,       P_BE,   0,       b32+143, aliases+ 94},	/* 10 */
	{ 8UL, b32+ 11, 0,       P_BE,   b32+ 59, b32+ 98, aliases+102},	/* 11 */
	{ 8UL, b32+ 11, b32+154, P_LE,   0,       b32+125, aliases+106},	/* 12 */
	{31UL, b32+ 12, b32+160, P_BE,   b32+160, b32+ 23, aliases+ 63},	/* 13 */
	{ 6UL, b32+ 14, 0,       P_LE,   0,       b32+ 22, aliases+ 87},	/* 14 */
	{82UL, b82,     0,       P_LE,   0,       b82a,    aliases+109},	/* 15 */
	{16UL, b32+ 17, 0,       P_BE,   0,       b32+ 45, aliases+124},	/* 16 */
	{16UL, b32+ 17, 0,       P_LE,   0,       b32+ 33, aliases+116},	/* 17 */
	{16UL, b32+ 17, b32+ 26, P_BE,   0,       b32+136, aliases+ 18},	/* 18 */
	{16UL, b32+ 17, b32+ 85, P_LE,   0,       b32+ 37, aliases+ 49},	/* 19 */
	{16UL, b32+ 17, b32+106, P_LE,   0,       b32+ 66, aliases+ 45},	/* 20 */
	{16UL, b32+ 17, b32+120, P_LE,   0,       b32+115, aliases+110},	/* 21 */
	{16UL, b32+ 17, b32+157, P_BE,   0,       b32+ 40, aliases+ 21},	/* 22 */
	{16UL, b32+ 17, b32+157, P_BE,   b32+157, b32+130, aliases+ 32},	/* 23 */
	{16UL, b32+ 17, b32+157, P_LE,   0,       b32+ 70, aliases+ 41},	/* 24 */
	{16UL, b32+ 17, b32+157, P_LE,   b32+157, b32+ 90, aliases+120},	/* 25 */
	{ 7UL, b32+ 18, 0,       P_BE,   0,       b32+138, aliases+ 91},	/* 26 */
	{ 6UL, b32+ 24, b32+148, P_BE,   0,       b32+140, aliases+ 85},	/* 27 */
	{ 8UL, b32+ 25, b32+150, P_BE,   0,       b32+ 76, aliases+101},	/* 28 */
	{ 8UL, b32+ 25, b32+154, P_BE,   b32+154, b32+ 56, aliases+107},	/* 29 */
	{ 8UL, b32+ 25, b32+154, P_LE,   0,       b32+ 91, aliases+100},	/* 30 */
	{16UL, b32+ 27, b32+157, P_BE,   b32+157, b32+101, aliases+ 44},	/* 31 */
	{32UL, b32+ 28, b32+161, P_LE,   b32+161, b32+135, aliases+ 75},	/* 32 */
	{14UL, b32+ 30, 0,       P_LE,   0,       b32+ 31, aliases+ 12},	/* 33 */
	{ 5UL, b32+ 39, b32+147, P_LE,   b32+147, b32+121, aliases+ 83},	/* 34 */
	{ 8UL, b32+ 41, 0,       P_BE,   0,       b32+ 52, aliases+105},	/* 35 */
	{ 8UL, b32+ 41, b32+154, P_BE,   b32+154, b32+133, aliases+ 96},	/* 36 */ 
	{ 4UL, b32+ 42, 0,       P_LE,   0,       b32+ 71, aliases+ 79},	/* 37 */
	{ 4UL, b32+ 42, b32+141, P_BE,   b32+141, b32+105, aliases+ 78},	/* 38 */
	{ 8UL, b32+ 44, 0,       P_LE,   0,       b32+ 98, aliases+104},	/* 39 */
	{24UL, b32+ 46, b32+158, P_BE,   b32+158, b32+109, aliases+ 57},	/* 40 */
	{ 8UL, b32+ 50, 0,       P_LE,   0,       b32+ 20, aliases+ 98},	/* 41 */
	{16UL, b32+ 51, 0,       P_BE,   b32+157, b32+119, aliases+ 30},	/* 42 */
	{16UL, b32+ 51, 0,       P_LE,   b32+157, b32+139, aliases+ 29},	/* 43 */
	{64UL, b64,     0,       P_BE,   0,       b64a,    aliases+ 88},	/* 44 */
	{64UL, b64,     b64b,    P_BE,   b64b,    b64c,    aliases+ 89},	/* 45 */
	{64UL, b64,     b64b,    P_LE,   b64b,    b64d,    aliases+ 90},	/* 46 */
	{ 5UL, b32+ 54, b32+ 54, P_BE,   0,       b32+  0, aliases+ 81},	/* 47 */
	{16UL, b32+ 61, 0,       P_BE,   0,       b32+ 62, aliases+ 42},	/* 48 */
	{24UL, b32+ 63, b32+103, P_BE,   0,       b32+ 29, aliases+ 56},	/* 49 */
	{24UL, b32+ 63, b32+152, P_BE,   0,       b32+ 75, aliases+ 55},	/* 50 */
	{ 3UL, b32+ 64, b32+134, P_LE,   0,       b32+116, aliases+ 61},	/* 51 */
	{11UL, b32+ 65, 0,       P_BE,   0,       b32+ 15, aliases+  6},	/* 52 */
	{ 6UL, b32+ 67, 0,       P_LE,   0,       b32+ 92, aliases+ 86},	/* 53 */
	{16UL, b32+ 69, 0,       P_BE,   0,       b32+114, aliases+ 38},	/* 54 */
	{11UL, b32+ 72, b32+  7, P_BE,   0,       b32+107, aliases+  5},	/* 55 */
	{16UL, b32+ 73, 0,       P_BE,   0,       b32+ 32, aliases+ 43},	/* 56 */
	{24UL, b32+ 77, 0,       P_BE,   0,       b32+ 35, aliases+ 59},	/* 57 */
	{16UL, b32+ 78, 0,       P_BE,   0,       b32+153, aliases+ 19},	/* 58 */
	{16UL, b32+ 78, 0,       P_LE,   0,       b32+111, aliases+  0},	/* 59 */
	{16UL, b32+ 78, 0,       P_LE,   b32+157, b32+ 53, aliases+ 40},	/* 60 */
	{16UL, b32+ 78, b32+ 79, P_BE,   0,       b32+ 96, aliases+ 26},	/* 61 */
	{16UL, b32+ 78, b32+157, P_BE,   0,       b32+104, aliases+ 24},	/* 62 */
	{16UL, b32+ 78, b32+157, P_LE,   0,       b32+ 57, aliases+117},	/* 63 */
	{16UL, b32+ 78, b32+157, P_LE,   b32+157, b32+108, aliases+ 51},	/* 64 */
	{30UL, b32+ 80, b32+159, P_BE,   b32+159, b32+ 19, aliases+ 62},	/* 65 */
	{12UL, b32+ 81, 0,       P_BE,   0,       b32+145, aliases+  9},	/* 66 */
	{12UL, b32+ 81, 0,       P_BELE, 0,       b32+132, aliases+ 10},	/* 67 */
	{32UL, b32+ 82, 0,       P_BE,   0,       b32+ 43, aliases+ 77},	/* 68 */
	{24UL, b32+ 83, 0,       P_BE,   0,       b32+124, aliases+ 58},	/* 69 */
	{24UL, b32+ 83, b32+110, P_BE,   0,       b32+ 34, aliases+ 53},	/* 70 */
	{ 7UL, b32+ 86, 0,       P_BE,   0,       b32+117, aliases+ 93},	/* 71 */
	{15UL, b32+ 87, 0,       P_BE,   0,       b32+ 13, aliases+ 13},	/* 72 */
	{16UL, b32+ 88, 0,       P_BE,   0,       b32+127, aliases+ 47},	/* 73 */
	{10UL, b32+ 89, 0,       P_BE,   0,       b32+ 68, aliases+  3},	/* 74 */
	{ 8UL, b32+ 93, 0,       P_BE,   0,       b32+138, aliases+103},	/* 75 */
	{ 8UL, b32+ 93, 0,       P_LE,   0,       b32+ 36, aliases+108},	/* 76 */
	{ 8UL, b32+ 93, b32+154, P_BE,   0,       b32+131, aliases+ 97},	/* 77 */
	{ 6UL, b32+ 94, b32+148, P_BE,   0,       b32+ 47, aliases+ 84},	/* 78 */
	{ 7UL, b32+ 95, b32+151, P_LE,   0,       b32+ 99, aliases+ 92},	/* 79 */
	{16UL, b32+ 97, 0,       P_BE,   0,       b32+ 16, aliases+ 48},	/* 80 */
	{ 5UL, b32+100, 0,       P_LE,   0,       b32+ 49, aliases+ 82},	/* 81 */
	{32UL, b32+102, b32+161, P_LE,   b32+161, b32+ 84, aliases+ 76},	/* 82 */
	{16UL, b32+122, b32+157, P_BE,   0,       b32+ 58, aliases+ 23},	/* 83 */
	{15UL, b32+126, 0,       P_BE,   b32+  3, b32+ 55, aliases+ 14},	/* 84 */
	{ 8UL, b32+129, 0,       P_BE,   0,       b32+112, aliases+ 99},	/* 85 */
	{13UL, b32+137, 0,       P_BE,   0,       b32+ 38, aliases+ 11},	/* 86 */
	{12UL, b32+142, b32+156, P_BE,   0,       b32+128, aliases+  8},	/* 87 */
	{32UL, b32+144, b32+161, P_LE,   b32+161, b32+ 21, aliases+ 67},	/* 88 */
	{10UL, b32+146, b32+155, P_BE,   0,       b32+ 89, aliases+  4},	/* 89 */
	{ 0UL, 0,       0,       P_BE,   0,       0,       NULL       },	/* terminating entry */
};
#    define NPRESETS 90

/* List of names with pointers to models, pre-sorted for use with bsearch() */
static const struct malias aliases[] = {
	{"ARC",			models+59},	/*   0 */
	{"B-CRC-32",		models+ 5},	/*   1 */
	{"CKSUM",		models+ 3},	/*   2 */
	{"CRC-10",		models+74},	/*   3 */
	{"CRC-10/CDMA2000",	models+89},	/*   4 */
	{"CRC-11",		models+55},	/*   5 */
	{"CRC-11/UMTS",		models+52},	/*   6 */
	{"CRC-12/3GPP",		models+67},	/*   7 */
	{"CRC-12/CDMA2000",	models+87},	/*   8 */
	{"CRC-12/DECT",		models+66},	/*   9 */
	{"CRC-12/UMTS",		models+67},	/*  10 */
	{"CRC-13/BBC",		models+86},	/*  11 */
	{"CRC-14/DARC",		models+33},	/*  12 */
	{"CRC-15",		models+72},	/*  13 */
	{"CRC-15/MPT1327",	models+84},	/*  14 */
	{"CRC-16",		models+59},	/*  15 */
	{"CRC-16/ACORN",	models+16},	/*  16 */
	{"CRC-16/ARC",		models+59},	/*  17 */
	{"CRC-16/AUG-CCITT",	models+18},	/*  18 */
	{"CRC-16/BUYPASS",	models+58},	/*  19 */
	{"CRC-16/CCITT",	models+17},	/*  20 */
	{"CRC-16/CCITT-FALSE",	models+22},	/*  21 */
	{"CRC-16/CCITT-TRUE",	models+17},	/*  22 */
	{"CRC-16/CDMA2000",	models+83},	/*  23 */
	{"CRC-16/CMS",		models+62},	/*  24 */
	{"CRC-16/DARC",		models+23},	/*  25 */
	{"CRC-16/DDS-110",	models+61},	/*  26 */
	{"CRC-16/DECT-R",	models+ 9},	/*  27 */
	{"CRC-16/DECT-X",	models+ 8},	/*  28 */
	{"CRC-16/DNP",		models+43},	/*  29 */
	{"CRC-16/EN-13757",	models+42},	/*  30 */
	{"CRC-16/EPC",		models+23},	/*  31 */
	{"CRC-16/GENIBUS",	models+23},	/*  32 */
	{"CRC-16/I-CODE",	models+23},	/*  33 */
	{"CRC-16/IBM-SDLC",	models+25},	/*  34 */
	{"CRC-16/IEC-61158-2",	models+31},	/*  35 */
	{"CRC-16/ISO-HDLC",	models+25},	/*  36 */
	{"CRC-16/LHA",		models+59},	/*  37 */
	{"CRC-16/LJ1200",	models+54},	/*  38 */
	{"CRC-16/LTE",		models+16},	/*  39 */
	{"CRC-16/MAXIM",	models+60},	/*  40 */
	{"CRC-16/MCRF4XX",	models+24},	/*  41 */
	{"CRC-16/OPENSAFETY-A",	models+48},	/*  42 */
	{"CRC-16/OPENSAFETY-B",	models+56},	/*  43 */
	{"CRC-16/PROFIBUS",	models+31},	/*  44 */
	{"CRC-16/RIELLO",	models+20},	/*  45 */
	{"CRC-16/SPI-FUJITSU",	models+18},	/*  46 */
	{"CRC-16/T10-DIF",	models+73},	/*  47 */
	{"CRC-16/TELEDISK",	models+80},	/*  48 */
	{"CRC-16/TMS37157",	models+19},	/*  49 */
	{"CRC-16/UMTS",		models+58},	/*  50 */
	{"CRC-16/USB",		models+64},	/*  51 */
	{"CRC-16/VERIFONE",	models+58},	/*  52 */
	{"CRC-24",		models+70},	/*  53 */
	{"CRC-24/BLE",		models+ 2},	/*  54 */
	{"CRC-24/FLEXRAY-A",	models+50},	/*  55 */
	{"CRC-24/FLEXRAY-B",	models+49},	/*  56 */
	{"CRC-24/INTERLAKEN",	models+40},	/*  57 */
	{"CRC-24/LTE-A",	models+69},	/*  58 */
	{"CRC-24/LTE-B",	models+57},	/*  59 */
	{"CRC-24/OPENPGP",	models+70},	/*  60 */
	{"CRC-3/ROHC",		models+51},	/*  61 */
	{"CRC-30/CDMA",		models+65},	/*  62 */
	{"CRC-31/PHILIPS",	models+13},	/*  63 */
	{"CRC-32",		models+ 7},	/*  64 */
	{"CRC-32/AAL5",		models+ 5},	/*  65 */
	{"CRC-32/ADCCP",	models+ 7},	/*  66 */
	{"CRC-32/AUTOSAR",	models+88},	/*  67 */
	{"CRC-32/BZIP2",	models+ 5},	/*  68 */
	{"CRC-32/CASTAGNOLI",	models+32},	/*  69 */
	{"CRC-32/DECT-B",	models+ 5},	/*  70 */
	{"CRC-32/INTERLAKEN",	models+32},	/*  71 */
	{"CRC-32/ISCSI",	models+32},	/*  72 */
	{"CRC-32/MPEG-2",	models+ 4},	/*  73 */
	{"CRC-32/POSIX",	models+ 3},	/*  74 */
	{"CRC-32C",		models+32},	/*  75 */
	{"CRC-32D",		models+82},	/*  76 */
	{"CRC-32Q",		models+68},	/*  77 */
	{"CRC-4/INTERLAKEN",	models+38},	/*  78 */
	{"CRC-4/ITU",		models+37},	/*  79 */
	{"CRC-40/GSM",		models+ 1},	/*  80 */
	{"CRC-5/EPC",		models+47},	/*  81 */
	{"CRC-5/ITU",		models+81},	/*  82 */
	{"CRC-5/USB",		models+34},	/*  83 */
	{"CRC-6/CDMA2000-A",	models+78},	/*  84 */
	{"CRC-6/CDMA2000-B",	models+27},	/*  85 */
	{"CRC-6/DARC",		models+53},	/*  86 */
	{"CRC-6/ITU",		models+14},	/*  87 */
	{"CRC-64",		models+44},	/*  88 */
	{"CRC-64/WE",		models+45},	/*  89 */
	{"CRC-64/XZ",		models+46},	/*  90 */
	{"CRC-7",		models+26},	/*  91 */
	{"CRC-7/ROHC",		models+79},	/*  92 */
	{"CRC-7/UMTS",		models+71},	/*  93 */
	{"CRC-8",		models+10},	/*  94 */
	{"CRC-8/AES",		models+30},	/*  95 */
	{"CRC-8/AUTOSAR",	models+36},	/*  96 */
	{"CRC-8/CDMA2000",	models+77},	/*  97 */
	{"CRC-8/DARC",		models+41},	/*  98 */
	{"CRC-8/DVB-S2",	models+85},	/*  99 */
	{"CRC-8/EBU",		models+30},	/* 100 */
	{"CRC-8/I-CODE",	models+28},	/* 101 */
	{"CRC-8/ITU",		models+11},	/* 102 */
	{"CRC-8/LTE",		models+75},	/* 103 */
	{"CRC-8/MAXIM",		models+39},	/* 104 */
	{"CRC-8/OPENSAFETY",	models+35},	/* 105 */
	{"CRC-8/ROHC",		models+12},	/* 106 */
	{"CRC-8/SAE-J1850",	models+29},	/* 107 */
	{"CRC-8/WCDMA",		models+76},	/* 108 */
	{"CRC-82/DARC",		models+15},	/* 109 */
	{"CRC-A",		models+21},	/* 110 */
	{"CRC-B",		models+25},	/* 111 */
	{"CRC-CCITT",		models+17},	/* 112 */
	{"CRC-IBM",		models+59},	/* 113 */
	{"DOW-CRC",		models+39},	/* 114 */
	{"JAMCRC",		models+ 6},	/* 115 */
	{"KERMIT",		models+17},	/* 116 */
	{"MODBUS",		models+63},	/* 117 */
	{"PKZIP",		models+ 7},	/* 118 */
	{"R-CRC-16",		models+ 9},	/* 119 */
	{"X-25",		models+25},	/* 120 */
	{"X-CRC-12",		models+66},	/* 121 */
	{"X-CRC-16",		models+ 8},	/* 122 */
	{"XFER",		models+ 0},	/* 123 */
	{"XMODEM",		models+16},	/* 124 */
	{"ZMODEM",		models+16},	/* 125 */
	{NULL,			NULL     },	/* terminating entry */
};
#    define NALIASES 126

#  endif /* BMP_BIT */
#else /* PRESETS */

static const struct mpreset models[] = {
	{ 0UL, 0,       0,       P_BE,   0,       0,       NULL       },	/* terminating entry */
};
#  define NPRESETS 0

static const struct malias aliases[] = {
	{NULL,			NULL     },	/* terminating entry */
};
#  define NALIASES 0

#endif /* PRESETS */

static void munpack(model_t *, const struct mpreset *);

/* copy a parameter of a preset to a poly */
#define PUNPACK(poly, preset, field) {\
	unsigned long iter, idx; \
	praloc((poly), ((preset)->field ? (preset)->width : 0UL)); \
	for(iter=0UL, idx=0UL; iter < (poly)->length; iter += BMP_BIT, ++idx) \
		(poly)->bitmap[idx] = (preset)->field[idx]; \
}

/* copy a parameter of a preset into a model */
#define MUNPACK(parm) PUNPACK(&dest->parm, src, b##parm)

/* Definitions */

int
mbynam(model_t *dest, const char *key) {
	/* Sets parameters in dest according to the model named by key.
	 */
	size_t left = 0, right = NALIASES, middle = 0;
	int cmp = 1;
	char *ukey, *uptr;

	if(!aliases->name)
		return(-1);
	if(!(ukey = malloc((size_t) 1 + strlen(key)))) {
		uerror("cannot allocate memory for comparison string");
			return(0);
	}
	uptr = ukey;
	do
		*uptr++ = toupper(*key);
	while(*key++);

	while(left < right && cmp) {
		middle = (left >> 1) + (right >> 1);
		cmp = strcmp(ukey, aliases[middle].name);
		if(cmp < 0) right = middle;
		else if(cmp > 0) left = middle + 1;
	}
	free(ukey);

	if(cmp)
		return(0);
	munpack(dest, aliases[middle].model);
	return(1);
}

void
mbynum(model_t *dest, int num) {
	/* Sets parameters in dest according to the model indexed by num. */
	if(num > NPRESETS)
		num = NPRESETS;
	munpack(dest, num+models);
}

int
mcount(void) {
	/* Returns the number of preset models. */
	return(NPRESETS);
}

char *
mnames(void) {
	/* Returns a malloc()-ed string of the names of all preset
	 * models, separated by newlines and terminated by NULL.
	 * Aliases are not listed.
	 */
	size_t size = 0;
	char *string, *sptr;
	const struct malias *aptr = aliases;

	while(aptr->name) {
		if(aptr == aptr->model->alias)
			size += strlen(aptr->name) + 1;
		++aptr;
	}
	if(!size) return(NULL);
	if((string = malloc(size))) {
		aptr = aliases;
		sptr = string;
		while(aptr->name) {
			if(aptr == aptr->model->alias) {
				strcpy(sptr, aptr->name);
				sptr += strlen(aptr->name);
				*sptr++ = '\n';
			}
			++aptr;
		}
		*--sptr = '\0';
	} else
		uerror("cannot allocate memory for list of models");

	return(string);
}

void
mmatch(model_t *model, int flags) {
	/* searches models[] for a model matching the argument, and links a name if found
	 * if flags & M_OVERWR, copies the found model onto the argument. */
	size_t left = 0, right = NPRESETS, middle = 0;
	poly_t poly = PZERO;
	int cmp = 1;
	if(!model) return;

	while(left < right && cmp) {
		middle = (left >> 1) + (right >> 1);
		PUNPACK(&poly, models+middle, bspoly);
		cmp = psncmp(&model->spoly, &poly);
		if(!cmp) {
			PUNPACK(&poly, models+middle, binit);
			cmp = psncmp(&model->init, &poly);
		}
		if(!cmp) {
			if((model->flags & P_REFIN) && (~models[middle].flags & P_REFIN))
				cmp = 1;
			else if((~model->flags & P_REFIN) && (models[middle].flags & P_REFIN))
				cmp = -1;
			else if((model->flags & P_REFOUT) && (~models[middle].flags & P_REFOUT))
				cmp = 1;
			else if((~model->flags & P_REFOUT) && (models[middle].flags & P_REFOUT))
				cmp = -1;
			else {
				PUNPACK(&poly, models+middle, bxorout);
				cmp = psncmp(&model->xorout, &poly);
			}
		}
		if(cmp < 0) right = middle;
		else if(cmp > 0) left = middle + 1;
	}
	pfree(&poly);

	if(!cmp) {
		model->name = models[middle].alias->name;
		if(flags & M_OVERWR)
			munpack(model, models+middle);
	}
}

/* Private functions */

static void
munpack(model_t *dest, const struct mpreset *src) {
	/* Copies the parameters of src to dest.
	 * dest must be an initialised model.
	 */
	if(!dest || !src) return;
	MUNPACK(spoly);
	MUNPACK(init);
	MUNPACK(xorout);
	MUNPACK(check);
	dest->flags = src->flags;
	/* link to the name as it is static */
	dest->name = src->alias->name;
}
