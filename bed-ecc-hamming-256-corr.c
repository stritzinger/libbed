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

#include "bed-impl.h"

bed_status bed_ecc_hamming_256_correct(
	void *data,
	const uint8_t *read_ecc,
	const uint8_t *calc_ecc
)
{
	bed_status status = BED_SUCCESS;
	uint8_t error [BED_ECC_HAMMING_256_SIZE];

	error [0] = read_ecc [0] ^ calc_ecc [0];
	error [1] = read_ecc [1] ^ calc_ecc [1];
	error [2] = read_ecc [2] ^ calc_ecc [2];

	if ((error [0] | error [1] | error [2]) != 0) {
		int ones = 0;

		ones += bed_ones_per_byte(error [0]);
		ones += bed_ones_per_byte(error [1]);
		ones += bed_ones_per_byte(error [2]);

		if (ones == 11) {
			/* Correctable error in data */
			uint8_t *bytes = data;
			int cp = error [2];
			int rp = error [0] | (error [1] << 8);
			int bit = 0;
			int byte = 0;

			bit |= (cp & 0x08) >> 3;
			bit |= (cp & 0x20) >> 4;
			bit |= (cp & 0x80) >> 5;

			byte |= (rp & 0x0002) >> 1;
			byte |= (rp & 0x0008) >> 2;
			byte |= (rp & 0x0020) >> 3;
			byte |= (rp & 0x0080) >> 4;
			byte |= (rp & 0x0200) >> 5;
			byte |= (rp & 0x0800) >> 6;
			byte |= (rp & 0x2000) >> 7;
			byte |= (rp & 0x8000) >> 8;

			/* Bit flip */
			bytes [byte] ^= (uint8_t) (1 << bit);

			status = BED_ERROR_ECC_FIXED;
		} else if (ones == 1) {
			/* Correctable error in read ECC */
			status = BED_ERROR_ECC_FIXED;
		} else {
			status = BED_ERROR_ECC_UNCORRECTABLE;
		}
	}

	return status;
}
