/*
 * Copyright (c) 2013 embedded brains GmbH.  All rights reserved.
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

#include "bed-nor.h"

#include <gtest/gtest.h>

static const size_t DEVICE_SIZE = 8;

static const size_t BLOCK_COUNT = 2;

static const size_t BLOCK_SIZE = DEVICE_SIZE / BLOCK_COUNT;

enum Action {
	NOTHING,
	WRITE,
	ERASE
};

struct TestCase {
	Action action;
	bed_address addr;
	size_t size;
	uint8_t out[DEVICE_SIZE];
	uint8_t in[DEVICE_SIZE];
};

static const TestCase testCases[] = {
	{
		NOTHING,
		0,
		0,
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
	}, {
		WRITE,
		0,
		DEVICE_SIZE,
		{ 0x00, 0xa5, 0x5a, 0x7e, 0x18, 0xff, 0x81, 0xff},
		{ 0x00, 0xa5, 0x5a, 0x7e, 0x18, 0xff, 0x81, 0xff}
	}, {
		WRITE,
		0,
		DEVICE_SIZE,
		{ 0x00, 0x5a, 0xa5, 0x7e, 0x10, 0xf0, 0x80, 0x0f},
		{ 0x00, 0x00, 0x00, 0x7e, 0x10, 0xf0, 0x80, 0x0f}
	}, {
		ERASE,
		0,
		BLOCK_SIZE,
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{ 0xff, 0xff, 0xff, 0xff, 0x10, 0xf0, 0x80, 0x0f}
	}, {
		ERASE,
		BLOCK_SIZE,
		BLOCK_SIZE,
		{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff}
	}
};

TEST(BED, NORSimulator)
{
	bed_partition *part = bed_nor_simulator_create(BLOCK_COUNT, BLOCK_SIZE);
	ASSERT_TRUE(part != NULL);

	for (size_t i = 0; i < sizeof(testCases) / sizeof(testCases[0]); ++i) {
		const TestCase &t(testCases[i]);
		bed_status status;

		switch (t.action) {
			case WRITE:
				status = bed_write(part, t.addr, &t.out[t.addr], t.size);
				EXPECT_EQ(BED_SUCCESS, status);
				break;
			case ERASE:
				status = bed_erase(part, t.addr, BED_ERASE_NORMAL);
				EXPECT_EQ(BED_SUCCESS, status);
				break;
			default:
				break;
		}

		for (bed_address addr = 0; addr < DEVICE_SIZE; ++addr) {
			uint8_t in;
			status = bed_read(part, addr, &in, sizeof(in));
			EXPECT_EQ(BED_SUCCESS, status);

			EXPECT_EQ(in, t.in[addr]);
		}
	}

	bed_nor_simulator_destroy(part);
}
