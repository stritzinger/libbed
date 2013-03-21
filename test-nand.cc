/*
 * Copyright (c) 2012 embedded brains GmbH.  All rights reserved.
 *
 *  embedded brains GmbH
 *  Obere Lagerstr. 30
 *  82178 Puchheim
 *  Germany
 *  <rtems@embedded-brains.de>
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rtems.com/license/LICENSE.
 */

#include "bed-nand.h"

#include <gtest/gtest.h>

#include <stdio.h>

static const size_t CHIP_COUNT = 2;

static const size_t BLOCK_COUNT = 2;

static const size_t BLOCK_SIZE = 1024;

static const size_t PAGE_SIZE = 512;

static const size_t PAGES_PER_BLOCK = BLOCK_SIZE / PAGE_SIZE;

static const size_t OOB_SIZE = 16;

static const size_t OOB_FREE_SIZE = 8;

static const size_t CHIP_SIZE = CHIP_COUNT * BLOCK_COUNT * BLOCK_SIZE;

static bed_address address(bed_address chip, bed_address block, bed_address page)
{
	return chip * (BLOCK_SIZE * BLOCK_COUNT) + block * BLOCK_SIZE + page * PAGE_SIZE;
}

static uint32_t createDataWithSize(uint32_t *data, size_t n, uint32_t value)
{
	for (size_t i = 0; i < n / sizeof(uint32_t); ++i) {
		data [i] = value;
		++value;
	}

	return value;
}

static uint32_t createData(uint32_t *data, uint32_t value)
{
	return createDataWithSize(data, PAGE_SIZE, value);
}

static uint8_t createOOB(uint8_t *oob, uint8_t value)
{
	for (size_t i = 0; i < OOB_FREE_SIZE; ++i) {
		oob [i] = value;
		++value;
	}

	return value;
}

static bool checkIfErased(bed_partition *part)
{
	for (size_t chip = 0; chip < CHIP_COUNT; ++chip) {
		for (size_t block = 0; block < BLOCK_COUNT; ++block) {
			for (size_t page = 0; page < PAGES_PER_BLOCK; ++page) {
				uint32_t data [PAGE_SIZE / sizeof(uint32_t)];
				uint8_t oobData [OOB_FREE_SIZE];
				const bed_oob_request oob = {
					BED_OOB_MODE_AUTO,
					0,
					OOB_FREE_SIZE,
					oobData
				};
				bed_status status = bed_read_oob(
					part,
					address(chip, block, page),
					data,
					PAGE_SIZE,
					&oob
				);
				if (status != BED_SUCCESS) {
					return false;
				}

				uint8_t expectedData [PAGE_SIZE];
				memset(expectedData, 0xff, PAGE_SIZE);
				if (memcmp(expectedData, data, PAGE_SIZE) != 0) {
					return false;
				}

				uint8_t expectedOOB [OOB_FREE_SIZE];
				memset(expectedOOB, 0xff, OOB_FREE_SIZE);
				if (memcmp(expectedOOB, oobData, OOB_FREE_SIZE) != 0) {
					return false;
				}
			}
		}
	}

	return true;
}

TEST(BED, NANDSimulator)
{
	bed_status status;

	bed_partition *part = bed_nand_simulator_create(CHIP_COUNT, BLOCK_COUNT, BLOCK_SIZE, PAGE_SIZE);
	ASSERT_TRUE(part != NULL);

	EXPECT_TRUE(checkIfErased(part));

	uint32_t dataValue = 0;
	uint8_t oobValue = 0;
	for (size_t chip = 0; chip < CHIP_COUNT; ++chip) {
		for (size_t block = 0; block < BLOCK_COUNT; ++block) {
			for (size_t page = 0; page < PAGES_PER_BLOCK; ++page) {
				uint32_t expectedData [PAGE_SIZE / sizeof(uint32_t)];
				uint8_t expectedOOBData [OOB_FREE_SIZE];
				const bed_oob_request expectedOOB = {
					BED_OOB_MODE_AUTO,
					0,
					OOB_FREE_SIZE,
					expectedOOBData
				};
				dataValue = createData(expectedData, dataValue);
				oobValue = createOOB(expectedOOBData, oobValue);
				status = bed_write_oob(
					part,
					address(chip, block, page),
					expectedData,
					PAGE_SIZE,
					&expectedOOB
				);
			}
		}
	}

	dataValue = 0;
	oobValue = 0;
	for (size_t chip = 0; chip < CHIP_COUNT; ++chip) {
		for (size_t block = 0; block < BLOCK_COUNT; ++block) {
			for (size_t page = 0; page < PAGES_PER_BLOCK; ++page) {
				uint32_t expectedData [PAGE_SIZE / sizeof(uint32_t)];
				uint8_t expectedOOBData [OOB_FREE_SIZE];
				dataValue = createData(expectedData, dataValue);
				oobValue = createOOB(expectedOOBData, oobValue);

				uint32_t data [PAGE_SIZE / sizeof(uint32_t)];
				uint8_t oobData [OOB_FREE_SIZE];
				const bed_oob_request oob = {
					BED_OOB_MODE_AUTO,
					0,
					OOB_FREE_SIZE,
					oobData
				};
				status = bed_read_oob(
					part,
					address(chip, block, page),
					data,
					PAGE_SIZE,
					&oob
				);
				EXPECT_EQ(BED_SUCCESS, status);

				EXPECT_EQ(0, memcmp(expectedData, data, PAGE_SIZE));
				EXPECT_EQ(0, memcmp(expectedOOBData, oobData, OOB_FREE_SIZE));
			}
		}
	}

	for (size_t chip = 0; chip < CHIP_COUNT; ++chip) {
		for (size_t block = 0; block < BLOCK_COUNT; ++block) {
			status = bed_erase(part, address(chip, block, 0), BED_ERASE_NORMAL);
			EXPECT_EQ(BED_SUCCESS, status);
		}
	}

	EXPECT_TRUE(checkIfErased(part));

	for (size_t chip = 0; chip < CHIP_COUNT; ++chip) {
		for (size_t block = 0; block < BLOCK_COUNT; ++block) {
			bed_address blockAddress = address(chip, block, 0);

			status = bed_is_block_valid(part, blockAddress);
			EXPECT_EQ(BED_SUCCESS, status);

			status = bed_mark_block_bad(part, blockAddress);
			EXPECT_EQ(BED_SUCCESS, status);

			status = bed_is_block_valid(part, blockAddress);
			EXPECT_EQ(BED_ERROR_BLOCK_IS_BAD, status);

			status = bed_erase(part, blockAddress, BED_ERASE_NORMAL);
			EXPECT_EQ(BED_ERROR_BLOCK_IS_BAD, status);

			status = bed_erase(part, blockAddress, BED_ERASE_FORCE);
			EXPECT_EQ(BED_SUCCESS, status);
		}
	}

	bed_nand_simulator_destroy(part);
}

class ReadProcess {
	public:
		ReadProcess(void *data, size_t n)
			: mData(static_cast<uint8_t *>(data)), mSize(n), mIndex(0)
		{
			// VOID
		}

		bool complete() const
		{
			return mIndex == mSize;
		}

		static bool process(
			void *arg,
			bed_address address,
			void *data,
			size_t n,
			void *oob,
			size_t m
		)
		{
			ReadProcess *self = static_cast<ReadProcess *>(arg);

			return self->doProcess(data, n);
		}

	private:
		uint8_t *const mData;

		const size_t mSize;

		size_t mIndex;

		bool doProcess(void *data, size_t n)
		{
			bool done = mSize - mIndex < n;

			if (!done) {
				done = memcmp(data, mData + mIndex, n) != 0;
				mIndex += n;
			}

			return done;
		}
};

TEST(BED, WriteAndReadWithSkip)
{
	bed_partition *part = bed_nand_simulator_create(CHIP_COUNT, BLOCK_COUNT, BLOCK_SIZE, PAGE_SIZE);
	ASSERT_TRUE(part != NULL);

	uint32_t chipData [CHIP_SIZE / sizeof(uint32_t)];
	createDataWithSize(chipData, CHIP_SIZE, 0);
	uint8_t pageBuffer [PAGE_SIZE];
	bed_status status = bed_write_with_skip(
		part,
		chipData,
		CHIP_SIZE,
		pageBuffer
	);
	EXPECT_EQ(BED_SUCCESS, status);

	ReadProcess readProcess(chipData, CHIP_SIZE);
	uint8_t oobBuffer [OOB_SIZE];
	status = bed_read_with_skip(
		part,
		ReadProcess::process,
		&readProcess,
		pageBuffer,
		oobBuffer
	);
	EXPECT_EQ(BED_SUCCESS, status);
	EXPECT_TRUE(readProcess.complete());

	bed_nand_simulator_destroy(part);
}

TEST(BED, PrintBadBlocks)
{
	bed_partition *part = bed_nand_simulator_create(CHIP_COUNT, BLOCK_COUNT, BLOCK_SIZE, PAGE_SIZE);
	ASSERT_TRUE(part != NULL);

	bed_status status = bed_mark_block_bad(part, address(0, 0, 0));
	EXPECT_EQ(BED_SUCCESS, status);

	status = bed_mark_block_bad(part, address(1, 1, 0));
	EXPECT_EQ(BED_SUCCESS, status);

	bed_print_bad_blocks(part, reinterpret_cast<bed_printer>(fprintf), stdout);

	bed_print_bad_blocks(part, bed_vprintf_printer, reinterpret_cast<void *>(vprintf));

	bed_nand_simulator_destroy(part);
}

TEST(BED, NANDDevice)
{
	const bed_nand_device_info *info = bed_nand_device_info_all;

	while (!bed_nand_device_is_terminal(info)) {
		if (info->id == 0x33) {
			EXPECT_EQ(512, bed_nand_device_page_size(info));
			EXPECT_EQ(16384, bed_nand_device_block_size(info));
			EXPECT_EQ(16777216, bed_nand_device_chip_size(info));
		} else if (info->id == 0xa2) {
			EXPECT_EQ(0, bed_nand_device_page_size(info));
			EXPECT_EQ(0, bed_nand_device_block_size(info));
			EXPECT_EQ(67108864, bed_nand_device_chip_size(info));
		}

		++info;
	}
}
