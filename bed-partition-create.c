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

bed_status bed_partition_create(
	bed_partition *child,
	const bed_partition *parent,
	bed_address begin,
	bed_address size
)
{
	bed_status status = BED_SUCCESS;
	bed_device *bed = parent->bed;

	if (
		bed_is_range_valid(parent, begin, size)
			&& bed_is_block_aligned(bed, begin)
			&& bed_is_block_aligned(bed, size)
	) {
		child->bed = parent->bed;
		child->begin = parent->begin + begin;
		child->size = size;
	} else {
		status = BED_ERROR_INVALID_ADDRESS;
	}

	return status;
}
