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

#include <bsp/lpc32xx.h>

#include <local/mzx-flash.h>
#include <local/mzx-config-nand.h>

TEST(BED, LPC32XXMLC)
{
	lpc32xx_select_nand_controller(LPC32XX_NAND_CONTROLLER_MLC);

	bed_lpc32xx_mlc_context ctx;
	bed_partition *part = &ctx.part;
	mzx_nand_config nand_config = MZX_NAND_SAMSUNG_KF91208ROC_JIBO_HCLK_104_MHZ;
	bed_lpc32xx_mlc_config config = {
		bed_nand_device_info_8_bit_1_8_V,
		reinterpret_cast<volatile bed_lpc32xx_mlc *>(&lpc32xx.nand_mlc),
		nand_config.mlc_time,
		static_cast<uint16_t>(nand_config.chip_count),
		bed_default_obtain,
		bed_default_release,
		mzx_flash_select_chip,
		NULL
	};
	bed_status status = bed_lpc32xx_mlc_init(&ctx, &config);

	EXPECT_EQ(0xec, bed_nand_manufacturer_id(part));
	EXPECT_EQ(0x36, bed_nand_model_id(part));

	bed_print_bad_blocks(part, reinterpret_cast<bed_printer>(fprintf), stdout);

	bed_partition testPartition;
	status = bed_partition_create(&testPartition, part, 2 * bed_block_size(part), bed_size(part) - 2 * bed_block_size(part));
	EXPECT_EQ(BED_SUCCESS, status);

	uint32_t data [512 / 4];
	uint8_t oobData [16];

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

	bed_oob_request oob = { BED_OOB_MODE_RAW, 0, sizeof(oobData), oobData };
	status = bed_read_oob(part, 0, data, sizeof(data), &oob);
	EXPECT_EQ(BED_SUCCESS, status);

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

	status = bed_read_oob(part, goodAddress, data, sizeof(data), &oob);
	EXPECT_EQ(BED_SUCCESS, status);

	memset(data, 0, sizeof(data));
	status = bed_write_oob(part, goodAddress, data, sizeof(data), &oob);
	EXPECT_EQ(BED_SUCCESS, status);

	status = bed_read_oob(part, goodAddress, data, sizeof(data), &oob);
	EXPECT_EQ(BED_SUCCESS, status);

	status = bed_erase(part, goodAddress, BED_ERASE_NORMAL);
	EXPECT_EQ(BED_SUCCESS, status);

	status = bed_read_oob(part, goodAddress, data, sizeof(data), &oob);
	EXPECT_EQ(BED_SUCCESS, status);

	status = bed_mark_block_bad(part, goodAddress);
	EXPECT_EQ(BED_SUCCESS, status);

	status = bed_is_block_valid(part, goodAddress);
	EXPECT_EQ(BED_ERROR_BLOCK_IS_BAD, status);

	status = bed_erase(part, goodAddress, BED_ERASE_FORCE);
	EXPECT_EQ(BED_SUCCESS, status);
}

#endif /* LIBBSP_ARM_LPC32XX_BSP_H */
