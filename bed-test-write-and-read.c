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

bed_status bed_test_write_and_read(
	const bed_partition *part,
	uint32_t *page_value,
	uint8_t *oob_value,
	uint32_t *page_buffer,
	uint8_t *oob_buffer
)
{
	bed_status status = BED_SUCCESS;
	const bed_address end = bed_size(part);
	const uint32_t block_size = bed_block_size(part);
	const uint16_t page_size = bed_page_size(part);
	const uint16_t oob_free_size = bed_oob_free_size(part);
	const bed_oob_request oob = {
		.mode = BED_OOB_MODE_AUTO,
		.offset = 0,
		.size = oob_free_size,
		.data = oob_buffer
	};
	const uint32_t start_page_value = *page_value;
	const uint8_t start_oob_value = *oob_value;
	bed_address block = 0;
	uint32_t current_page_value = start_page_value;
	uint8_t current_oob_value = start_oob_value;

	while (block != end) {
		bed_address next_block = block + block_size;

		status = bed_erase(part, block, BED_ERASE_MARK_BAD_ON_ERROR);
		if (status == BED_SUCCESS) {
			bed_address page;

			for (page = block; page != next_block; page += page_size) {
				size_t i;

				for (i = 0; i < page_size / sizeof(*page_buffer); ++i) {
					page_buffer [i] = current_page_value;
					++current_page_value;
				}

				for (i = 0; i < oob_free_size; ++i) {
					oob_buffer [i] = current_oob_value;
					++current_oob_value;
				}

				status = bed_write_oob(part, page, page_buffer, page_size, &oob);
				if (status != BED_SUCCESS) {
					goto error;
				}
			}
		} else {
			status = BED_SUCCESS;
		}

		block = next_block;
	}

	block = 0;
	current_page_value = start_page_value;
	current_oob_value = start_oob_value;
	while (block != end) {
		bed_address next_block = block + block_size;

		status = bed_is_block_valid(part, block);
		if (status == BED_SUCCESS || status == BED_ERROR_OP_NOT_SUPPORTED) {
			bed_address page;

			for (page = block; page != next_block; page += page_size) {
				status = bed_read_oob(part, page, page_buffer, page_size, &oob);
				if (status == BED_SUCCESS) {
					size_t i;

					for (i = 0; i < page_size / sizeof(*page_buffer); ++i) {
						if (page_buffer [i] != current_page_value) {
							status = BED_ERROR_UNSATISFIED;
							goto error;
						}

						++current_page_value;
					}

					for (i = 0; i < oob_free_size; ++i) {
						if (oob_buffer [i] != current_oob_value) {
							status = BED_ERROR_UNSATISFIED;
							goto error;
						}

						++current_oob_value;
					}
				} else {
					goto error;
				}
			}
		} else {
			status = BED_SUCCESS;
		}

		block = next_block;
	}

	*page_value = current_page_value;
	*oob_value = current_oob_value;

error:

	return status;
}
