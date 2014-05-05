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

#define __STDC_FORMAT_MACROS

#include "bed-test.h"
#include "bed-lpc32xx.h"

#ifdef __rtems__
	#include <bsp.h>
#endif

#ifdef LIBBSP_ARM_LPC32XX_BSP_H

#include <gtest/gtest.h>

#include <stdio.h>
#include <inttypes.h>

#include <local/mzx-flash.h>

TEST(BED, LPC32XXSLCWriteAndReadAndErase)
{
	bed_status status;

	const bed_partition *part = mzx_flash_slc_without_mutex(true);
	ASSERT_TRUE(part != NULL);

	size_t pageSize = bed_page_size(part);
	ASSERT_GE(2048, pageSize);

	uint32_t data [2048 / 4];
	uint8_t oobData [64];
	bed_oob_request oob = { BED_OOB_MODE_RAW, 0, bed_oob_size(part), oobData };

	bed_print_bad_blocks(part, reinterpret_cast<bed_printer>(fprintf), stdout);

	bed_partition testPartition;
	status = bed_partition_create(&testPartition, part, 3 * bed_block_size(part), bed_block_size(part));
	EXPECT_EQ(BED_SUCCESS, status);

	status = bed_read_oob(&testPartition, 0, data, pageSize, &oob);
	EXPECT_EQ(BED_SUCCESS, status);

	uint32_t dataValue = 0;
	uint8_t oobValue = 0;
	for (int i = 0; i < 10; ++i) {
		printf("test write/read: %" PRIu32 "\n", dataValue);
		status = bed_test_write_and_read(&testPartition, &dataValue, &oobValue, data, oobData);
		EXPECT_EQ(BED_SUCCESS, status);
	}

	status = bed_erase_all(&testPartition, BED_ERASE_MARK_BAD_ON_ERROR);
	EXPECT_EQ(BED_SUCCESS, status);

	status = bed_test_if_erased(&testPartition, data, oobData);
	EXPECT_EQ(BED_SUCCESS, status);
}

TEST(BED, LPC32XXSLCBadBlocks)
{
	bed_status status;

	const bed_partition *part = mzx_flash_slc_without_mutex(true);
	ASSERT_TRUE(part != NULL);

	size_t pageSize = bed_page_size(part);
	ASSERT_GE(2048, pageSize);

	uint8_t data [2048];
	uint8_t oobData [64];
	bed_oob_request oob = { BED_OOB_MODE_AUTO, 0, bed_oob_free_size(part), oobData };

	bed_address address = 0;
	bed_address goodAddress = static_cast<bed_address>(-1);
	do {
		status = bed_is_block_valid(part, address);
		if (status == BED_SUCCESS) {
			goodAddress = address;
		} else if (status == BED_ERROR_BLOCK_IS_BAD) {
			printf("bad block: %" PRIu32 "\n", address / bed_block_size(part));
			status = BED_SUCCESS;
		}

		address += bed_block_size(part);
	} while (status == BED_SUCCESS);

	printf("good block: %" PRIu32 "\n", goodAddress / bed_block_size(part));

	status = bed_is_block_valid(part, goodAddress);
	EXPECT_EQ(BED_SUCCESS, status);

	status = bed_read_oob(part, goodAddress, data, pageSize, &oob);
	EXPECT_EQ(BED_SUCCESS, status);

	memset(data, 0, pageSize);
	status = bed_write_oob(part, goodAddress, data, pageSize, &oob);
	EXPECT_EQ(BED_SUCCESS, status);

	status = bed_read_oob(part, goodAddress, data, pageSize, &oob);
	EXPECT_EQ(BED_SUCCESS, status);

	status = bed_erase(part, goodAddress, BED_ERASE_NORMAL);
	EXPECT_EQ(BED_SUCCESS, status);

	status = bed_read_oob(part, goodAddress, data, pageSize, &oob);
	EXPECT_EQ(BED_SUCCESS, status);

	status = bed_mark_block_bad(part, goodAddress);
	EXPECT_EQ(BED_SUCCESS, status);

	status = bed_is_block_valid(part, goodAddress);
	EXPECT_EQ(BED_ERROR_BLOCK_IS_BAD, status);

	status = bed_erase(part, goodAddress, BED_ERASE_FORCE);
	EXPECT_EQ(BED_SUCCESS, status);
}

TEST(BED, LPC32XXSLCCheckECC)
{
	bed_status status;

	const bed_partition *part = mzx_flash_slc_without_mutex(true);
	ASSERT_TRUE(part != NULL);

	size_t pageSize = bed_page_size(part);
	uint32_t data [2048 / 4];
	uint8_t oobData [64];
	bed_oob_request oob = { BED_OOB_MODE_AUTO, 0, bed_oob_free_size(part), oobData };

	bed_partition testPartition;
	status = bed_partition_create(&testPartition, part, 3 * bed_block_size(part), bed_block_size(part));
	EXPECT_EQ(BED_SUCCESS, status);

	status = bed_erase_all(&testPartition, BED_ERASE_MARK_BAD_ON_ERROR);
	EXPECT_EQ(BED_SUCCESS, status);

	status = bed_test_if_erased(&testPartition, data, oobData);
	EXPECT_EQ(BED_SUCCESS, status);

	memset(data, 0x5a, sizeof(data));
	memset(oobData, 0, sizeof(oobData));
	status = bed_write_oob(&testPartition, 0, data, pageSize, &oob);
	EXPECT_EQ(BED_SUCCESS, status);

	status = bed_read_oob(&testPartition, 0, data, pageSize, &oob);
	EXPECT_EQ(BED_SUCCESS, status);

	data [0] = 0xdeadbeef;
	status = bed_write_oob(&testPartition, 0, data, pageSize, &oob);
	EXPECT_EQ(BED_SUCCESS, status);

	status = bed_read_oob(&testPartition, 0, data, pageSize, &oob);
	EXPECT_EQ(BED_ERROR_ECC_UNCORRECTABLE, status);
}

// Be careful
#if 0
TEST(BED, LPC32XXSLCMakeBlockBad)
{
	bed_status status;

	const bed_partition *part = mzx_flash_slc_without_mutex(true);
	ASSERT_TRUE(part != NULL);

	size_t pageSize = bed_page_size(part);
	ASSERT_GE(2048, pageSize);

	bed_partition testPartition;
	status = bed_partition_create(&testPartition, part, 4 * bed_block_size(part), bed_block_size(part));
	EXPECT_EQ(BED_SUCCESS, status);

	uint32_t data [2048 / 4] [2];
	uint8_t oobData [64] [2];
	uint32_t eraseCounter;
	uint32_t writeCounter;
	uint32_t readErrors;
	bed_test_mark_block_bad_stats stats;
	bed_test_make_block_bad(
		&testPartition,
		data [0],
		oobData [0],
		data [1],
		oobData [1],
		&stats
	);
	fprintf(
		stdout,
		"erase counter = %" PRIu32
		"erase errors = %" PRIu32
			", write counter = %" PRIu32
			", write errors = %" PRIu32
			", read ecc fixed = %" PRIu32 "\n"
			", read ecc uncorrectable = %" PRIu32 "\n"
			", read other errors = %" PRIu32 "\n"
			", read data errors = %" PRIu32 "\n"
			", read oob errors = %" PRIu32 "\n",
		stats.erase_counter,
		stats.erase_errors,
		stats.write_counter,
		stats.write_errors,
		stats.read_ecc_fixed,
		stats.read_ecc_uncorrectable,
		stats.read_other_errors,
		stats.read_data_errors,
		stats.read_oob_errors
	);
}
#endif

#endif /* LIBBSP_ARM_LPC32XX_BSP_H */
