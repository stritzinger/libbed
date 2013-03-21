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

#include "bed-test.h"
#include "bed-impl.h"

bed_status bed_test_if_erased(
	const bed_partition *part,
	uint32_t *page_buffer,
	uint8_t *oob_buffer
)
{
	bed_status status = BED_SUCCESS;
	bed_address block = 0;
	bed_address end = bed_size(part);
	uint32_t block_size = bed_block_size(part);
	uint16_t page_size = bed_page_size(part);
	uint16_t oob_free_size = bed_oob_free_size(part);
	const bed_oob_request oob = {
		.mode = BED_OOB_MODE_AUTO,
		.offset = 0,
		.size = oob_free_size,
		.data = oob_buffer
	};

	while (block != end) {
		bed_address next_block = block + block_size;

		status = bed_is_block_valid(part, block);
		if (status == BED_SUCCESS) {
			bed_address page;

			for (page = block; page != next_block; page += page_size) {
				status = bed_read_oob(part, page, page_buffer, page_size, &oob);
				if (status == BED_SUCCESS) {
					size_t i;

					for (i = 0; i < page_size / sizeof(*page_buffer); ++i) {
						if (page_buffer [i] != 0xffffffff) {
							status = BED_ERROR_UNSATISFIED;
							goto done;
						}
					}

					for (i = 0; i < oob_free_size; ++i) {
						if (oob_buffer [i] != 0xff) {
							status = BED_ERROR_UNSATISFIED;
							goto done;
						}
					}
				} else {
					goto done;
				}
			}
		} else {
			status = BED_SUCCESS;
		}

		block = next_block;
	}

done:

	return status;
}
