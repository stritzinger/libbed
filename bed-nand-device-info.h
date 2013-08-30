/*
 * Copyright (c) 2012 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Dornierstr. 4
 *  82178 Puchheim
 *  Germany
 *  <rtems@embedded-brains.de>
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rtems.com/license/LICENSE.
 */

#include "bed-nand.h"

#define POWER_OF_TWO(x) \
	((x) & (1UL << 31) ? 31 : \
	(x) & (1UL << 30) ? 30 : \
	(x) & (1UL << 29) ? 29 : \
	(x) & (1UL << 28) ? 28 : \
	(x) & (1UL << 27) ? 27 : \
	(x) & (1UL << 26) ? 26 : \
	(x) & (1UL << 25) ? 25 : \
	(x) & (1UL << 24) ? 24 : \
	(x) & (1UL << 23) ? 23 : \
	(x) & (1UL << 22) ? 22 : \
	(x) & (1UL << 21) ? 21 : \
	(x) & (1UL << 20) ? 20 : \
	(x) & (1UL << 19) ? 19 : \
	(x) & (1UL << 18) ? 18 : \
	(x) & (1UL << 17) ? 17 : \
	(x) & (1UL << 16) ? 16 : \
	(x) & (1UL << 15) ? 15 : \
	(x) & (1UL << 14) ? 14 : \
	(x) & (1UL << 13) ? 13 : \
	(x) & (1UL << 12) ? 12 : \
	(x) & (1UL << 11) ? 11 : \
	(x) & (1UL << 10) ? 10 : \
	(x) & (1UL << 9) ? 9 : \
	(x) & (1UL << 8) ? 8 : \
	(x) & (1UL << 7) ? 7 : \
	(x) & (1UL << 6) ? 6 : \
	(x) & (1UL << 5) ? 5 : \
	(x) & (1UL << 4) ? 4 : \
	(x) & (1UL << 3) ? 3 : \
	(x) & (1UL << 2) ? 2 : \
	(x) & (1UL << 1) ? 1 : \
	0)

#define POWER_OF_TWO_ADJ(x, c) \
	((x) != 0 ? (POWER_OF_TWO((x) / (1UL << (c))) + 1) : 0)

#define DEV(xid, ps, bs, cs, bw16) \
	{ \
		.id = (xid), \
		.page_size_code = POWER_OF_TWO_ADJ(ps, 8), \
		.block_size_code = POWER_OF_TWO_ADJ(bs, 12), \
		.chip_size_code = POWER_OF_TWO_ADJ(cs, 0), \
		.bus_width_16 = (bw16), \
	}

#ifdef BED_NAND_DEVICE_INFO_8_BIT_1_8_V
	#define DEV_8_BIT_1_8_V(xid, ps, bs, cs) DEV(xid, ps, bs, cs, 0),
#endif

#ifdef BED_NAND_DEVICE_INFO_8_BIT_3_3_V
	#define DEV_8_BIT_3_3_V(xid, ps, bs, cs) DEV(xid, ps, bs, cs, 0),
#endif

#ifdef BED_NAND_DEVICE_INFO_16_BIT_1_8_V
	#define DEV_16_BIT_1_8_V(xid, ps, bs, cs) DEV(xid, ps, bs, cs, 1),
#endif

#ifdef BED_NAND_DEVICE_INFO_16_BIT_3_3_V
	#define DEV_16_BIT_3_3_V(xid, ps, bs, cs) DEV(xid, ps, bs, cs, 1),
#endif

#ifndef DEV_8_BIT_1_8_V
	#define DEV_8_BIT_1_8_V(xid, ps, bs, cs)
#endif

#ifndef DEV_8_BIT_3_3_V
	#define DEV_8_BIT_3_3_V(xid, ps, bs, cs)
#endif

#ifndef DEV_16_BIT_1_8_V
	#define DEV_16_BIT_1_8_V(xid, ps, bs, cs)
#endif

#ifndef DEV_16_BIT_3_3_V
	#define DEV_16_BIT_3_3_V(xid, ps, bs, cs)
#endif

const bed_nand_device_info BED_NAND_DEVICE_INFO [] = {
	/* Small page chips */

	/* 16MiB */
	DEV_8_BIT_1_8_V(0x33, 512, 16384, 16)
	DEV_8_BIT_3_3_V(0x73, 512, 16384, 16)
	DEV_16_BIT_1_8_V(0x43, 512, 16384, 16)
	DEV_16_BIT_3_3_V(0x53, 512, 16384, 16)

	/* 32MiB */
	DEV_8_BIT_1_8_V(0x35, 512, 16384, 32)
	DEV_8_BIT_3_3_V(0x75, 512, 16384, 32)
	DEV_16_BIT_1_8_V(0x45, 512, 16384, 32)
	DEV_16_BIT_3_3_V(0x55, 512, 16384, 32)

	/* 64MiB */
	DEV_8_BIT_1_8_V(0x36, 512, 16384, 64)
	DEV_8_BIT_3_3_V(0x76, 512, 16384, 64)
	DEV_16_BIT_1_8_V(0x46, 512, 16384, 64)
	DEV_16_BIT_3_3_V(0x56, 512, 16384, 64)

	/* 128MiB */
	DEV_8_BIT_1_8_V(0x78, 512, 16384, 128)
	DEV_8_BIT_1_8_V(0x39, 512, 16384, 128)
	DEV_8_BIT_3_3_V(0x79, 512, 16384, 128)
	DEV_16_BIT_1_8_V(0x72, 512, 16384, 128)
	DEV_16_BIT_1_8_V(0x49, 512, 16384, 128)
	DEV_16_BIT_3_3_V(0x74, 512, 16384, 128)
	DEV_16_BIT_3_3_V(0x59, 512, 16384, 128)

	/* 256MiB */
	DEV_8_BIT_3_3_V(0x71, 512, 16384, 256)

	/*
	 * Large page chips.  Some values are initialized through the extended
	 * ID.
	 */

	/* 64MiB */
	DEV_8_BIT_1_8_V(0xa2, 0, 0, 64)
	DEV_8_BIT_1_8_V(0xa0, 0, 0, 64)
	DEV_8_BIT_3_3_V(0xf2, 0, 0, 64)
	DEV_8_BIT_3_3_V(0xd0, 0, 0, 64)
	DEV_8_BIT_3_3_V(0xf0, 0, 0, 64)
	DEV_16_BIT_1_8_V(0xb2, 0, 0, 64)
	DEV_16_BIT_1_8_V(0xb0, 0, 0, 64)
	DEV_16_BIT_3_3_V(0xc2, 0, 0, 64)
	DEV_16_BIT_3_3_V(0xc0, 0, 0, 64)

	/* 128MiB */
	DEV_8_BIT_1_8_V(0xa1, 0, 0, 128)
	DEV_8_BIT_3_3_V(0xf1, 0, 0, 128)
	DEV_8_BIT_3_3_V(0xd1, 0, 0, 128)
	DEV_16_BIT_1_8_V(0xb1, 0, 0, 128)
	DEV_16_BIT_3_3_V(0xc1, 0, 0, 128)
	DEV_16_BIT_1_8_V(0xad, 0, 0, 128)

	/* 256MiB */
	DEV_8_BIT_1_8_V(0xaa, 0, 0, 256)
	DEV_8_BIT_3_3_V(0xda, 0, 0, 256)
	DEV_16_BIT_1_8_V(0xba, 0, 0, 256)
	DEV_16_BIT_3_3_V(0xca, 0, 0, 256)

	/* 512MiB */
	DEV_8_BIT_1_8_V(0xac, 0, 0, 512)
	DEV_8_BIT_3_3_V(0xdc, 0, 0, 512)
	DEV_16_BIT_1_8_V(0xbc, 0, 0, 512)
	DEV_16_BIT_3_3_V(0xcc, 0, 0, 512)

	/* 1GiB */
	DEV_8_BIT_1_8_V(0xa3, 0, 0, 1024)
	DEV_8_BIT_3_3_V(0xd3, 0, 0, 1024)
	DEV_16_BIT_1_8_V(0xb3, 0, 0, 1024)
	DEV_16_BIT_3_3_V(0xc3, 0, 0, 1024)

	/* 2GiB */
	DEV_8_BIT_1_8_V(0xa5, 0, 0, 2048)
	DEV_8_BIT_3_3_V(0xd5, 0, 0, 2048)
	DEV_16_BIT_1_8_V(0xb5, 0, 0, 2048)
	DEV_16_BIT_3_3_V(0xc5, 0, 0, 2048)

	/* 4GiB */
	DEV_8_BIT_1_8_V(0xa7, 0, 0, 4096)
	DEV_8_BIT_3_3_V(0xd7, 0, 0, 4096)
	DEV_16_BIT_1_8_V(0xb7, 0, 0, 4096)
	DEV_16_BIT_3_3_V(0xc7, 0, 0, 4096)

	/* 8GiB */
	DEV_8_BIT_1_8_V(0xae, 0, 0, 8192)
	DEV_8_BIT_3_3_V(0xde, 0, 0, 8192)
	DEV_16_BIT_1_8_V(0xbe, 0, 0, 8192)
	DEV_16_BIT_3_3_V(0xce, 0, 0, 8192)

	/* 16GiB */
	DEV_8_BIT_1_8_V(0x1a, 0, 0, 16384)
	DEV_8_BIT_3_3_V(0x3a, 0, 0, 16384)
	DEV_16_BIT_1_8_V(0x2a, 0, 0, 16384)
	DEV_16_BIT_3_3_V(0x4a, 0, 0, 16384)

	/* 32GiB */
	DEV_8_BIT_1_8_V(0x1c, 0, 0, 32768)
	DEV_8_BIT_3_3_V(0x3c, 0, 0, 32768)
	DEV_16_BIT_1_8_V(0x2c, 0, 0, 32768)
	DEV_16_BIT_3_3_V(0x4c, 0, 0, 32768)

	/* 64GiB */
	DEV_8_BIT_1_8_V(0x1e, 0, 0, 65536)
	DEV_8_BIT_3_3_V(0x3e, 0, 0, 65536)
	DEV_16_BIT_1_8_V(0x2e, 0, 0, 65536)
	DEV_16_BIT_3_3_V(0x4e, 0, 0, 65536)

	/* Terminal */
	{
		.id = 0,
		.page_size_code = 0,
		.block_size_code = 0,
		.chip_size_code = 0,
		.bus_width_16 = 0
	}
};
