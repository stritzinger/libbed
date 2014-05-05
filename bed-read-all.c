/*
 * Copyright (c) 2012-2014 embedded brains GmbH.  All rights reserved.
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

#include <string.h>

static bed_status read_all(
	bed_device *bed,
	bed_address area_begin,
	bed_address area_size,
	bed_oob_mode oob_mode,
	bed_read_all_process process,
	void *process_arg,
	void *page_buffer,
	void *oob_buffer
)
{
	bed_status status = BED_SUCCESS;
	bed_address area_end = area_begin + area_size;
	uint32_t block_size = bed->block_size;
	uint16_t page_size = bed->page_size;
	uint16_t oob_size = (uint16_t) (oob_mode == BED_OOB_MODE_AUTO ? bed->oob_free_size : bed->oob_size);
	bed_oob_request oob = {
		.mode = oob_mode,
		.offset = 0,
		.size = oob_size,
		.data = oob_buffer
	};
	bed_address block = area_begin;

	while (status == BED_SUCCESS && block != area_end) {
		bed_address next_block = block + block_size;
		bed_status is_block_valid_status;
		bed_address page;

		is_block_valid_status = (*bed->is_block_valid)(bed, block);

		for (page = block; status == BED_SUCCESS && page != next_block; page += page_size) {
			bed_status page_read_status = (*bed->read_oob)(bed, page, page_buffer, page_size, &oob);
			bool done = (*process)(
				process_arg,
				page,
				is_block_valid_status,
				page_read_status,
				page_buffer,
				page_size,
				oob_buffer,
				oob_size
			);

			status = done ? BED_ERROR_STOPPED : BED_SUCCESS;
		}

		block = next_block;
	}

	return status;
}

bed_status bed_read_all(
	const bed_partition *part,
	bed_oob_mode oob_mode,
	bed_read_all_process process,
	void *process_arg,
	void *page_buffer,
	void *oob_buffer
)
{
	bed_status status = BED_SUCCESS;
	bed_device *bed = part->bed;

	(*bed->obtain)(bed);
	status = read_all(
		bed,
		part->begin,
		part->size,
		oob_mode,
		process,
		process_arg,
		page_buffer,
		oob_buffer
	);
	(*bed->release)(bed);

	return status;
}
