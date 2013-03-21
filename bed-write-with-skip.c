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

#include <string.h>

#ifndef BED_CONFIG_READ_ONLY
static bed_status write_with_skip(
	bed_device *bed,
	bed_address area_begin,
	bed_address area_size,
	const void *data,
	size_t n,
	void *page_buffer
)
{
	bed_status status = BED_SUCCESS;
	bed_address area_end = area_begin + area_size;
	uint32_t block_size = bed->block_size;
	uint16_t page_size = bed->page_size;
	bed_address block = area_begin;
	const uint8_t *out = data;
	const uint8_t *end = out + n;

	while (out != end && block != area_end) {
		bed_address next_block = block + block_size;;
		const uint8_t *out_at_block_begin = out;

		status = bed_device_erase(bed, block, BED_ERASE_MARK_BAD_ON_ERROR);
		if (status == BED_SUCCESS) {
			bed_address page;

			for (page = block; out != end && page != next_block; page += page_size) {
				size_t r = (uintptr_t) end - (uintptr_t) out;
				size_t m = r < page_size ? r : page_size;
				const uint8_t *p = out;

				if (m < page_size) {
					uint8_t *q = page_buffer;

					memcpy(q, p, m);
					memset(q + m, 0xff, page_size - m);
					p = q;
				}

				status = (*bed->write)(bed, page, p, page_size);
				if (status == BED_SUCCESS) {
					out += m;
				} else if (status == BED_ERROR_WRITE) {
					(*bed->mark_block_bad)(bed, block);
					out = out_at_block_begin;
					break;
				}
			}

		}

		block = next_block;
	}

	if (out == end) {
		status = BED_SUCCESS;
	} else {
		status = BED_ERROR_UNSATISFIED;
	}

	return status;
}
#endif /* BED_CONFIG_READ_ONLY */

bed_status bed_write_with_skip(
	const bed_partition *part,
	const void *data,
	size_t n,
	void *page_buffer
)
{
#ifndef BED_CONFIG_READ_ONLY
	bed_status status = BED_SUCCESS;
	bed_device *bed = part->bed;

	(*bed->obtain)(bed);
	status = write_with_skip(bed, part->begin, part->size, data, n, page_buffer);
	(*bed->release)(bed);

	return status;
#else /* BED_CONFIG_READ_ONLY */
	return BED_ERROR_READ_ONLY;
#endif /* BED_CONFIG_READ_ONLY */
}
