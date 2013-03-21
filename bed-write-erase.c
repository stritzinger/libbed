
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

bed_status bed_write(const bed_partition *part, bed_address addr, const void *data, size_t n)
{
#ifdef BED_CONFIG_READ_ONLY
	return BED_ERROR_READ_ONLY;
#else
	bed_status status = BED_SUCCESS;
	bed_device *bed = part->bed;

	if (bed_is_range_valid(part, addr, n)) {
		if (n > 0) {
			(*bed->obtain)(bed);
			status = (*bed->write)(bed, part->begin + addr, data, n);
			(*bed->release)(bed);
		}
	} else {
		status = BED_ERROR_INVALID_ADDRESS;
	}

	return status;
#endif
}

bed_status bed_write_oob(
	const bed_partition *part,
	bed_address addr,
	const void *data,
	size_t n,
	const bed_oob_request *oob
)
{
#ifdef BED_CONFIG_READ_ONLY
	return BED_ERROR_READ_ONLY;
#else
	bed_status status = BED_SUCCESS;
	bed_device *bed = part->bed;

	if (bed_is_range_valid(part, addr, n) && bed_is_oob_request_valid(bed, oob)) {
		if (n > 0) {
			(*bed->obtain)(bed);
			status = (*bed->write_oob)(bed, part->begin + addr, data, n, oob);
			(*bed->release)(bed);
		}
	} else {
		status = BED_ERROR_INVALID_ADDRESS;
	}

	return status;
#endif
}

bed_status bed_device_erase(
	bed_device *bed,
	bed_address addr,
	bed_erase_mode mode
)
{
#ifdef BED_CONFIG_READ_ONLY
	return BED_ERROR_READ_ONLY;
#else
	bed_status status = mode == BED_ERASE_FORCE ?
		BED_SUCCESS : (*bed->is_block_valid)(bed, addr);

	if (status == BED_SUCCESS || status == BED_ERROR_OP_NOT_SUPPORTED) {
		status = (*bed->erase)(bed, addr);

		if (
			status == BED_ERROR_ERASE
				&& mode == BED_ERASE_MARK_BAD_ON_ERROR
		) {
			(*bed->mark_block_bad)(bed, addr);
		}
	}

	return status;
#endif
}

bed_status bed_erase(
	const bed_partition *part,
	bed_address addr,
	bed_erase_mode mode
)
{
#ifdef BED_CONFIG_READ_ONLY
	return BED_ERROR_READ_ONLY;
#else
	bed_status status = BED_SUCCESS;
	bed_device *bed = part->bed;

	if (bed_is_address_valid(part, addr)) {
		(*bed->obtain)(bed);
		status = bed_device_erase(bed, part->begin + addr, mode);
		(*bed->release)(bed);
	} else {
		status = BED_ERROR_INVALID_ADDRESS;
	}

	return status;
#endif
}

bed_status bed_erase_all(const bed_partition *part, bed_erase_mode mode)
{
#ifdef BED_CONFIG_READ_ONLY
	return BED_ERROR_READ_ONLY;
#else
	bed_status status = BED_SUCCESS;
	bed_device *bed = part->bed;
	bed_address block = part->begin;
	bed_address end = part->begin + part->size;
	uint32_t block_size = bed->block_size;

	(*bed->obtain)(bed);

	while (block != end) {
		bed_status erase_status = bed_device_erase(bed, block, mode);

		if (
			status == BED_SUCCESS
				&& erase_status != BED_SUCCESS
				&& erase_status != BED_ERROR_BLOCK_IS_BAD
		) {
			status = erase_status;
		}

		block += block_size;
	}

	(*bed->release)(bed);

	return status;
#endif
}

bed_status bed_mark_block_bad(const bed_partition *part, bed_address addr)
{
#ifdef BED_CONFIG_READ_ONLY
	return BED_ERROR_READ_ONLY;
#else
	bed_status status = BED_SUCCESS;
	bed_device *bed = part->bed;

	if (bed_is_address_valid(part, addr)) {
		(*bed->obtain)(bed);
		status = (*bed->mark_block_bad)(bed, part->begin + addr);
		(*bed->release)(bed);
	} else {
		status = BED_ERROR_INVALID_ADDRESS;
	}

	return status;
#endif
}
