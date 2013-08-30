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

#include "bed.h"

#include <string.h>

const const char parity [256] = {
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
	0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0
};

void bed_ecc_hamming_256_calculate(const void *data, uint8_t *ecc)
{
	const uint8_t *byte = data;
	uint8_t par = 0;
	uint8_t rp [16];
	uint8_t cur;
	int i;

	memset(rp, 0, sizeof(rp));

	for (i = 0; i < 256; ++i, ++byte) {
		uint8_t cur = *byte;
		int j;

		par ^= cur;

		for (j = 0; j < 8; ++j) {
			if ((i & (1 << j)) == 0) {
				rp [j << 1] ^= cur;
			} else {
				rp [(j << 1) + 1] ^= cur;
			}
		}
	}

	ecc [0] = 0;
	for (i = 0; i < 8; ++i) {
		ecc [0] |= (uint8_t) (parity [rp [i]] << i);
	}

	ecc [1] = 0;
	for (i = 0; i < 8; ++i) {
		ecc [1] |= (uint8_t) (parity [rp [i + 8]] << i);
	}

	ecc [2] = 0;
	ecc [2] |= (uint8_t) (parity [par & 0xf0] << 7);
	ecc [2] |= (uint8_t) (parity [par & 0x0f] << 6);
	ecc [2] |= (uint8_t) (parity [par & 0xcc] << 5);
	ecc [2] |= (uint8_t) (parity [par & 0x33] << 4);
	ecc [2] |= (uint8_t) (parity [par & 0xaa] << 3);
	ecc [2] |= (uint8_t) (parity [par & 0x55] << 2);

	ecc[0] = (uint8_t) (~ecc[0]);
	ecc[1] = (uint8_t) (~ecc[1]);
	ecc[2] = (uint8_t) (~ecc[2]);
}
