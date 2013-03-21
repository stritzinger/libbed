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

#include "bed-impl.h"

#include <gtest/gtest.h>

static const bed_oob_request null_oob = {
	BED_OOB_MODE_AUTO,
	0,
	0,
	NULL
};

TEST(BED, ErrorOpNotSupported)
{
	bed_partition nonZeroSizeNullPartition = bed_null_partition;
	nonZeroSizeNullPartition.size = 1;
	const bed_partition *part = &nonZeroSizeNullPartition;
	bed_address address = 0;
	size_t size = 1;
	bed_status status = bed_read(part, address, NULL, size);
	EXPECT_EQ(BED_ERROR_OP_NOT_SUPPORTED, status);

	status = bed_read_oob(part, address, NULL, size, &null_oob);
	EXPECT_EQ(BED_ERROR_OP_NOT_SUPPORTED, status);

	status = bed_write(part, address, NULL, size);
	EXPECT_EQ(BED_ERROR_OP_NOT_SUPPORTED, status);

	status = bed_write_oob(part, address, NULL, size, &null_oob);
	EXPECT_EQ(BED_ERROR_OP_NOT_SUPPORTED, status);

	status = bed_erase(part, address, BED_ERASE_NORMAL);
	EXPECT_EQ(BED_ERROR_OP_NOT_SUPPORTED, status);

	status = bed_is_block_valid(part, address);
	EXPECT_EQ(BED_ERROR_OP_NOT_SUPPORTED, status);

	status = bed_mark_block_bad(part, address);
	EXPECT_EQ(BED_ERROR_OP_NOT_SUPPORTED, status);
}

TEST(BED, SuccessWithZeroSize)
{
	bed_partition nonZeroSizeNullPartition = bed_null_partition;
	nonZeroSizeNullPartition.size = 1;
	const bed_partition *part = &nonZeroSizeNullPartition;
	bed_address address = 0;
	size_t size = 0;

	bed_status status = bed_read(part, address, NULL, size);
	EXPECT_EQ(BED_SUCCESS, status);

	status = bed_read_oob(part, address, NULL, size, &null_oob);
	EXPECT_EQ(BED_SUCCESS, status);

	status = bed_write(part, address, NULL, size);
	EXPECT_EQ(BED_SUCCESS, status);

	status = bed_write_oob(part, address, NULL, size, &null_oob);
	EXPECT_EQ(BED_SUCCESS, status);
}

TEST(BED, ErrorInvalidAddress)
{
	const bed_partition *part = &bed_null_partition;
	bed_address validAddress = 0;
	bed_address invalidAddress = 1;
	size_t validSize = 0;
	size_t invalidSize = 1;

	bed_status status = bed_read(part, invalidAddress, NULL, validSize);
	EXPECT_EQ(BED_ERROR_INVALID_ADDRESS, status);

	status = bed_read(part, validAddress, NULL, invalidSize);
	EXPECT_EQ(BED_ERROR_INVALID_ADDRESS, status);

	status = bed_read_oob(part, invalidAddress, NULL, validSize, &null_oob);
	EXPECT_EQ(BED_ERROR_INVALID_ADDRESS, status);

	status = bed_read_oob(part, validAddress, NULL, invalidSize, &null_oob);
	EXPECT_EQ(BED_ERROR_INVALID_ADDRESS, status);

	status = bed_write(part, invalidAddress, NULL, validSize);
	EXPECT_EQ(BED_ERROR_INVALID_ADDRESS, status);

	status = bed_write(part, validAddress, NULL, invalidSize);
	EXPECT_EQ(BED_ERROR_INVALID_ADDRESS, status);

	status = bed_write_oob(part, invalidAddress, NULL, validSize, &null_oob);
	EXPECT_EQ(BED_ERROR_INVALID_ADDRESS, status);

	status = bed_write_oob(part, validAddress, NULL, invalidSize, &null_oob);
	EXPECT_EQ(BED_ERROR_INVALID_ADDRESS, status);

	status = bed_erase(part, invalidAddress, BED_ERASE_NORMAL);
	EXPECT_EQ(BED_ERROR_INVALID_ADDRESS, status);

	status = bed_is_block_valid(part, invalidAddress);
	EXPECT_EQ(BED_ERROR_INVALID_ADDRESS, status);

	status = bed_mark_block_bad(part, invalidAddress);
	EXPECT_EQ(BED_ERROR_INVALID_ADDRESS, status);
}

TEST(BED, PartitionCreate)
{
	bed_partition nonZeroSizeNullPartition = bed_null_partition;
	nonZeroSizeNullPartition.size = 1;
	const bed_partition *part = &nonZeroSizeNullPartition;

	bed_status status = bed_partition_create(NULL, part, 1, 0);
	EXPECT_EQ(BED_ERROR_INVALID_ADDRESS, status);

	status = bed_partition_create(NULL, part, 0, 2);
	EXPECT_EQ(BED_ERROR_INVALID_ADDRESS, status);
}

TEST(BED, IsPowerOfTwo)
{
	EXPECT_FALSE(bed_is_power_of_two(0));
	EXPECT_TRUE(bed_is_power_of_two(1));
	EXPECT_TRUE(bed_is_power_of_two(2));
	EXPECT_FALSE(bed_is_power_of_two(3));
}

TEST(BED, PowerOfTwo)
{
	int n = sizeof(bed_address) == 4 ? 32 : 64;

	for (int i = 1; i < n; ++i) {
		bed_address one = 1;
		bed_address address = one << i;
		EXPECT_EQ(i, bed_power_of_two(address));
	}
}
