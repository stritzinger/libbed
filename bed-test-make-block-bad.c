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

#include "bed-test.h"

#include <string.h>

void bed_test_make_block_bad(
	const bed_partition *part,
	uint32_t *page_buffer_0,
	uint8_t *oob_buffer_0,
	uint32_t *page_buffer_1,
	uint8_t *oob_buffer_1,
	bed_test_mark_block_bad_stats *stats
)
{
	bed_status status = BED_SUCCESS;
	const uint32_t block_size = bed_block_size(part);
	const uint16_t page_size = bed_page_size(part);
	const uint16_t oob_free_size = bed_oob_free_size(part);
	const bed_oob_request write_oob = {
		.mode = BED_OOB_MODE_AUTO,
		.offset = 0,
		.size = oob_free_size,
		.data = oob_buffer_0
	};
	const bed_oob_request read_oob = {
		.mode = BED_OOB_MODE_AUTO,
		.offset = 0,
		.size = oob_free_size,
		.data = oob_buffer_1
	};
	const bed_address block = 0;
	const bed_address next_block = block + block_size;
	int pattern = 0xa5;

	memset(stats, 0, sizeof(*stats));

	while (status == BED_SUCCESS) {
		++stats->erase_counter;
		status = bed_erase(part, block, BED_ERASE_MARK_BAD_ON_ERROR);
		if (status == BED_SUCCESS) {
			bed_address page;

			memset(page_buffer_0, pattern, page_size);
			memset(oob_buffer_0, pattern, oob_free_size);

			for (page = block; status == BED_SUCCESS && page != next_block; page += page_size) {
				++stats->write_counter;
				status = bed_write_oob(
					part, page,
					page_buffer_0,
					page_size,
					&write_oob
				);
				if (status == BED_SUCCESS) {
					bed_status read_status = bed_read_oob(
						part,
						page,
						page_buffer_1,
						page_size,
						&read_oob
					);

					switch (read_status) {
						case BED_SUCCESS:
							break;
						case BED_ERROR_ECC_FIXED:
							++stats->read_ecc_fixed;
							break;
						case BED_ERROR_ECC_UNCORRECTABLE:
							++stats->read_ecc_uncorrectable;
							break;
						default:
							++stats->read_other_errors;
							break;
					}

					stats->read_data_errors += memcmp(page_buffer_0, page_buffer_1, page_size) != 0;
					stats->read_oob_errors += memcmp(oob_buffer_0, oob_buffer_1, oob_free_size) != 0;
				} else {
					++stats->write_errors;
					bed_mark_block_bad(part, block);
				}
			}

			pattern = ~pattern;
		} else {
			++stats->erase_errors;
		}
	}
}
