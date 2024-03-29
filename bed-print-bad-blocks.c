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

#include <inttypes.h>

void bed_print_bad_blocks(const bed_partition *part, bed_printer printer, void *printer_arg)
{
	bed_status status;
	bed_address address = 0;
	bed_device *bed = part->bed;
	uint32_t block_size = bed->block_size;
	uint32_t bad_block_count = 0;
	uint32_t valid_block_count = 0;

	do {
		status = bed_is_block_valid(part, address);
		if (status == BED_ERROR_BLOCK_IS_BAD) {
			uint32_t block = (address >> bed->block_shift) & (bed->block_size - 1);
			uint32_t chip = address >> bed->chip_shift;

			++bad_block_count;

			(*printer)(
				printer_arg,
				"bad block: address = 0x%08" PRIx32 ", chip = %" PRIu32 ", block = %" PRIu32 "\n",
				address,
				chip,
				block
			);
			status = BED_SUCCESS;
		} else if (status == BED_SUCCESS) {
			++valid_block_count;
		}

		address += block_size;
	} while (status == BED_SUCCESS);

	(*printer)(
		printer_arg,
		"total blocks = %" PRIu32 ", bad blocks = %" PRIu32 ", valid blocks = %" PRIu32 "\n",
		bad_block_count + valid_block_count,
		bad_block_count,
		valid_block_count
	);
}
