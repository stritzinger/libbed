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

#include <gtest/gtest.h>

#include <string.h>

static int onesPerByte(uint8_t byte)
{
	int ones = 0;

	for (int i = 0; i < 8; ++i) {
		ones += (byte & (1 << i)) != 0;
	}

	return ones;
}

TEST(BED, OnesPerByte)
{
	for (int i = 0; i < 256; ++i) {
		uint8_t byte = static_cast<uint8_t>(i);

		EXPECT_EQ(bed_ones_per_byte(byte), onesPerByte(byte));
	}
}

TEST(BED, ECCHamming256)
{
	static const uint8_t data [256] = {
		20, 88, 45, 216, 153, 203, 145, 31, 12, 113, 23, 13, 160, 130, 42,
		211, 76, 133, 123, 222, 203, 244, 211, 158, 99, 31, 51, 102, 61, 41,
		80, 81, 130, 125, 42, 27, 72, 187, 58, 85, 45, 81, 99, 205, 212,
		141, 161, 33, 19, 29, 255, 222, 17, 210, 125, 117, 242, 176, 219, 47,
		218, 43, 129, 93, 169, 171, 120, 242, 103, 178, 71, 148, 4, 171, 98,
		217, 57, 3, 250, 76, 32, 250, 42, 50, 205, 167, 168, 192, 88, 131,
		239, 51, 175, 113, 144, 89, 29, 8, 75, 132, 187, 147, 25, 192, 62,
		123, 153, 119, 127, 148, 195, 160, 142, 238, 210, 91, 150, 123, 28, 238,
		254, 11, 34, 174, 125, 178, 7, 155, 187, 83, 32, 118, 230, 57, 55,
		37, 181, 209, 156, 52, 101, 96, 212, 244, 79, 167, 80, 229, 34, 108,
		212, 33, 120, 246, 208, 245, 169, 215, 145, 100, 42, 177, 219, 17, 235,
		18, 54, 160, 228, 211, 213, 73, 52, 170, 62, 131, 81, 0, 105, 116,
		250, 61, 149, 114, 52, 101, 104, 221, 61, 249, 66, 104, 171, 29, 121,
		150, 48, 176, 55, 20, 132, 12, 94, 184, 183, 156, 60, 8, 42, 165,
		125, 37, 227, 18, 152, 23, 120, 1, 245, 181, 250, 55, 29, 166, 85,
		151, 61, 133, 72, 116, 154, 204, 129, 248, 132, 56, 149, 193, 65, 192,
		102, 190, 229, 73, 209, 126, 97, 73, 127, 86, 255, 122, 142, 29, 32, 227
	};

	uint8_t tmp [256];
	uint8_t calcECC [BED_ECC_HAMMING_256_SIZE];
	uint8_t readECC [BED_ECC_HAMMING_256_SIZE];

	// No error
	memcpy(tmp, data, sizeof(tmp));
	bed_ecc_hamming_256_calculate(tmp, calcECC);
	bed_status status = bed_ecc_hamming_256_correct(tmp, calcECC, calcECC);
	EXPECT_EQ(BED_SUCCESS, status);

	for (int i = 0; i < 256 * 8; ++i) {
		int byte = i >> 3;
		int bit = i & 0x7;

		// Single bit error in data
		memcpy(tmp, data, sizeof(tmp));
		bed_ecc_hamming_256_calculate(tmp, calcECC);
		tmp [byte] ^= static_cast<uint8_t>(1 << bit);
		bed_ecc_hamming_256_calculate(tmp, readECC);
		status = bed_ecc_hamming_256_correct(tmp, readECC, calcECC);
		EXPECT_EQ(BED_ERROR_ECC_FIXED, status);
		EXPECT_EQ(0, memcmp(tmp, data, sizeof(tmp)));
	}

	for (int i = 2; i < 24; ++i) {
		int byte = i >> 3;
		int bit = i & 0x7;

		// Single bit error in ECC
		memcpy(tmp, data, sizeof(tmp));
		bed_ecc_hamming_256_calculate(tmp, calcECC);
		memcpy(readECC, calcECC, sizeof(readECC));
		readECC [byte] ^= static_cast<uint8_t>(1 << bit);
		status = bed_ecc_hamming_256_correct(tmp, readECC, calcECC);
		EXPECT_EQ(BED_ERROR_ECC_FIXED, status);
		EXPECT_EQ(0, memcmp(tmp, data, sizeof(tmp)));
	}

	// Double bit error
	memcpy(tmp, data, sizeof(tmp));
	bed_ecc_hamming_256_calculate(tmp, calcECC);
	tmp [13] ^= static_cast<uint8_t>(1 << 7);
	tmp [31] ^= static_cast<uint8_t>(1 << 2);
	bed_ecc_hamming_256_calculate(tmp, readECC);
	status = bed_ecc_hamming_256_correct(tmp, readECC, calcECC);
	EXPECT_EQ(BED_ERROR_ECC_UNCORRECTABLE, status);
	EXPECT_NE(0, memcmp(tmp, data, sizeof(tmp)));
}
